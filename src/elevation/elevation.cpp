//
// Created by Logan on 10/12/17.
//

#include "elevation.h"

bool ElevationData::_is_initialized = ElevationData::initGDAL();

bool ElevationData::initGDAL() {

    // Register all of the file formats for GDAL
    GDALAllRegister();

    return true;

}

ElevationData::ElevationData(const std::string& file_path) {

    // Get the dataset from the disk
    _gdal_dataset = (GDALDataset*)GDALOpen(file_path.c_str(), GA_ReadOnly);
    if (_gdal_dataset) {

        // Get the raster band
        _gdal_raster_band = _gdal_dataset->GetRasterBand(1);

        calcConversions();
        calcStats();
        createOpenCLImage();
        calcMinMax();


    } else std::cout << "Error loading the GDAL dataset from the disk\n";

}

ElevationData::~ElevationData() {

    // Raster band is owned by the dataset so it doesnt need to be deallocated
    // Only get rid of it if it exists. There is a default constructor for stitching where it will not.
    if (_gdal_dataset)
        GDALClose(_gdal_dataset);

}

int ElevationData::getWidth() const { return _width; }
int ElevationData::getHeight() const { return _height; }

float ElevationData::getWidthInMeters() const { return _width_meters; }
float ElevationData::getHeightInMeters() const { return _height_meters; }

float ElevationData::getMinElevation() const { return _elevation_min; }
float ElevationData::getMaxElevation() const { return _elevation_max; }

const boost::compute::image2d& ElevationData::getOpenCLImage() const { return _opencl_image; }

glm::vec2 ElevationData::convertPixelsToMeters(const glm::ivec2& pos_pixels) const {

    // Multiply by the conversion factors
    return glm::vec2((float)pos_pixels.x * pixelToMeterConversions[0],
                     (float)pos_pixels.y * pixelToMeterConversions[1]);

}

glm::ivec2 ElevationData::metersToPixels(const glm::vec2 &pos_meters) const {

    // Divide by the conversion factors
    return glm::ivec2(pos_meters.x / pixelToMeterConversions[0],
                      pos_meters.y / pixelToMeterConversions[1]);

}

glm::vec2 ElevationData::metersToLongitudeLatitude(const glm::vec2& pos_meters) const {

    // Convert back to pixels
    glm::ivec2 pos_pixels = metersToPixels(pos_meters);

    // Just return the pixels to long lat conversion
    return pixelsToLongitudeLatitude(pos_pixels);

}

glm::vec2 ElevationData::pixelsToLongitudeLatitude(const glm::ivec2& pos_pixels) const {

    // Convert using the GDAL transform
    // Formula can be found in the GDAL tutorial
    return glm::vec2(_gdal_transform[0] + (float)pos_pixels.x * _gdal_transform[1],
                     _gdal_transform[3] + (float)pos_pixels.y * _gdal_transform[5]);

}

glm::vec3 ElevationData::metersToMetersAndElevation(const glm::vec2& pos_meters) const {

    glm::vec3 pos_meters_sample = glm::vec3(pos_meters.x, pos_meters.y, 0.0);

    // Get the pixel position
    glm::ivec2 pos_pixels = metersToPixels(pos_meters);

    // Do the sample
    _gdal_raster_band->RasterIO( GF_Read, pos_pixels.x, pos_pixels.y, 1, 1, &pos_meters_sample[2], 1, 0, GDT_Float32, 0, 0);

    return pos_meters_sample;

}


glm::vec3 ElevationData::pixelsToMetersAndElevation(const glm::ivec2& pos_pixels) const {

    // First convert pixels to meters
    glm::vec2 pos_meters = convertPixelsToMeters(pos_pixels);
    glm::vec3 pos_meters_sample = glm::vec3(pos_meters.x, pos_meters.y, 0.0);

    // Now get the sample instead of calling metersToMetersAndElevation because GDAL samples in pixels
    _gdal_raster_band->RasterIO( GF_Read, pos_pixels.x, pos_pixels.y, 1, 1, &pos_meters_sample[2], 1, 0, GDT_Float32, 0, 0);

    return pos_meters_sample;

}

void ElevationData::calcConversions() {

    // First read the transform from GDAL (we assume that we get something)
    _gdal_dataset->GetGeoTransform(_gdal_transform);

    // Calculate conversion factors based on the size of pixels in degrees.
    // This uses a simplified formula of degrees -> arcseconds -> meters
    float degrees_to_meters = 1.0 * EARTH_RADIUS * M_PI / 180.0;

    // X conversion
    pixelToMeterConversions[0] = degrees_to_meters *      (float)_gdal_transform[1];

    // Y Conversion - we use abs because GDAL can store it in degrees
    pixelToMeterConversions[1] = degrees_to_meters * fabs((float)_gdal_transform[5]);

}

void ElevationData::calcStats() {

    // Get the width and height in pixels
    _width =  _gdal_dataset->GetRasterXSize();
    _height = _gdal_dataset->GetRasterYSize();

    // Get the width and height in meters using the conversions
    _width_meters  = _width * pixelToMeterConversions[0];
    _height_meters = _height * pixelToMeterConversions[1];

}

void ElevationData::calcMinMax() {

    // Get stuff we need to execute a kernel on
    const boost::compute::context& ctx =   Kernel::getContext();
    boost::compute::command_queue& queue = Kernel::getQueue();

    std::vector<float>            min_max_host(2);
    boost::compute::vector<float> min_max_device(2, ctx);

    // Generate the program sounds
    static const std::string source = BOOST_COMPUTE_STRINGIZE_SOURCE(

            __kernel void computeMinMax(__read_only image2d_t image, __global float* min_max, int width, int height) {

                const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

                // Use this to exclude no data
                const float extreme = 1000000.0;

                size_t i = get_global_id(0);

                // Convert i to x and y
                float x = (float)(i % width);
                float y = (float)(i / width);

                float sample = read_imagef(image, sampler, (float2)(x, y)).x;

                // Do a sanity check
                if (fabs(sample) < extreme) {

                    min_max[0] = min(sample, min_max[0]);
                    min_max[1] = max(sample, min_max[1]);

                }

            }

    );

    // Create a temporary kernel and execute it
    static Kernel kernel = Kernel(source, "computeMinMax");
    kernel.setArgs(_opencl_image, min_max_device.get_buffer(), _width, _height);
    kernel.execute1D(0, _width * _height);

    boost::compute::copy(min_max_device.begin(), min_max_device.end(), min_max_host.begin(), queue);

    // Save the min and max
    _elevation_min = min_max_host[0];
    _elevation_max = min_max_host[1];

}

void ElevationData::createOpenCLImage() {

    // OpenCL needs image data to be in a 1D float array.
    // Go line by line and read GDAL data to get the data in the format we need
    std::vector<float> image_data = std::vector<float>(_width * _height);

    for (int i = 0; i < _height; i++)
        _gdal_raster_band->RasterIO( GF_Read, 0, i, _width, 1, &image_data[_width * i], _width, 1, GDT_Float32, 0, 0);

    // Create the OpenCL image
    boost::compute::image_format format = boost::compute::image_format(CL_INTENSITY, CL_FLOAT);
    _opencl_image = boost::compute::image2d(Kernel::getContext(), _width, _height, format, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &image_data[0]);


}
