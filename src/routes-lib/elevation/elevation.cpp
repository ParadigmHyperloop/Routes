//
// Created by Logan on 10/12/17.
//

#include "elevation.h"
#include <stdio.h>

ElevationData::_StaticGDAL ElevationData::_init;

GDALDataset* ElevationData::_StaticGDAL::_gdal_dataset;
GDALRasterBand* ElevationData::_StaticGDAL::_gdal_raster_band;

double ElevationData::_StaticGDAL::_gdal_transform[6];

int ElevationData::_StaticGDAL::_width;
int ElevationData::_StaticGDAL::_height;

double ElevationData::_StaticGDAL::_width_meters;
double ElevationData::_StaticGDAL::_height_meters;
double ElevationData::_StaticGDAL::_pixelToMeterConversions[2];

ElevationData::_StaticGDAL::_StaticGDAL() {
    
    // Register all of the file formats for GDAL
    GDALAllRegister();
    
    // We don't throw an error here because this function is still called when we rebuild and we don't want to crash
    // Would love to use boost::filesystem::exists() here but it throws an exception and hasn't been fixed.
    if (std::ifstream(GDAL_DB_PATH)) {
        
        // Open up the GDAL dataset
        _gdal_dataset = (GDALDataset*)GDALOpenShared(GDAL_DB_PATH, GA_ReadOnly);
    
        std::cout << "Statically initializing the GDAL Data\n";
        
        // Get the raster band
        _gdal_raster_band = _gdal_dataset->GetRasterBand(1);
        
        calcConversions();
        calcStats();
        
    }
    
}

ElevationData::_StaticGDAL::~_StaticGDAL() {
    
    // Close the GDAL dataset if its open
//    if (GDALDumpOpenDatasets(stdout))
//        GDALClose(_gdal_dataset);
    
}

void ElevationData::_StaticGDAL::calcConversions() {
    
    // First read the transform from GDAL (we assume that we get something)
    _gdal_dataset->GetGeoTransform(_gdal_transform);
    
    // Calculate conversion factors based on the size of pixels in degrees.
    // This uses a simplified formula of degrees -> arcseconds -> meters
    double degrees_to_meters = EARTH_RADIUS * M_PI / 180.0;
    
    // X conversion
    _pixelToMeterConversions[0] = degrees_to_meters * (double)_gdal_transform[1];
    
    // Y Conversion - we use abs because GDAL can store it in degrees
    _pixelToMeterConversions[1] = degrees_to_meters * fabs((double)_gdal_transform[5]);
    
}

void ElevationData::_StaticGDAL::calcStats() {
    
    // Get the width and height in pixels
    _width =  _gdal_dataset->GetRasterXSize();
    _height = _gdal_dataset->GetRasterYSize();
    
    // Get the width and height in meters using the conversions
    _width_meters  = _width * _pixelToMeterConversions[0];
    _height_meters = _height * _pixelToMeterConversions[1];
    
}

/***********************************************************************************************************************************************/

ElevationData::ElevationData(const glm::dvec2& start, const glm::dvec2& dest) {

    if (!_StaticGDAL::_gdal_dataset)
        throw std::runtime_error("The database could not be loaded from the disk. Make sure to build it first.");
    
    // Before we do anything we do a sanity check
    if (!routeInsideData(start, dest)) {

        std::cout << "Attempted to compute a route that was not in the dataset\n";
        throw std::runtime_error("Attempted to compute a route that was not in the dataset");

    }

    // Get the size and then make the image
    calcCroppedSize(start, dest);
    createOpenCLImage();

}

int ElevationData::getWidth() const { return _StaticGDAL::_width; }
int ElevationData::getHeight() const { return _StaticGDAL::_height; }

double ElevationData::getWidthInMeters() const { return _StaticGDAL::_width_meters; }
double ElevationData::getHeightInMeters() const { return _StaticGDAL::_height_meters; }

double ElevationData::getMinElevation() const { return _elevation_min; }
double ElevationData::getMaxElevation() const { return _elevation_max; }

const boost::compute::image2d& ElevationData::getOpenCLImage() const { return _opencl_image; }

glm::dvec2 ElevationData::convertPixelsToMeters(const glm::ivec2& pos_pixels) const {

    // Multiply by the conversion factors
    return glm::dvec2((double)pos_pixels.x * _StaticGDAL::_pixelToMeterConversions[0],
                      (double)pos_pixels.y * _StaticGDAL::_pixelToMeterConversions[1]);

}

glm::ivec2 ElevationData::metersToPixels(const glm::dvec2 &pos_meters) const {

    // Divide by the conversion factors
    return glm::ivec2(pos_meters.x / _StaticGDAL::_pixelToMeterConversions[0],
                      pos_meters.y / _StaticGDAL::_pixelToMeterConversions[1]);

}

glm::dvec2 ElevationData::metersToLongitudeLatitude(const glm::dvec2& pos_meters) const {

    // Convert back to pixels
    glm::ivec2 pos_pixels = metersToPixels(pos_meters);

    // Just return the pixels to long lat conversion
    return pixelsToLongitudeLatitude(pos_pixels);

}

glm::dvec2 ElevationData::pixelsToLongitudeLatitude(const glm::ivec2& pos_pixels) const {

    // Convert using the GDAL transform
    // Formula can be found in the GDAL tutorial
    return glm::dvec2(_StaticGDAL::_gdal_transform[0] + (double)pos_pixels.x * _StaticGDAL::_gdal_transform[1],
                      _StaticGDAL::_gdal_transform[3] + (double)pos_pixels.y * _StaticGDAL::_gdal_transform[5]);

}

glm::dvec2 ElevationData::longitudeLatitudeToMeters(const glm::dvec2 lat_lon) const {

    // First convert to pixels using the gdal transform
    glm::ivec2 pixels = longitudeLatitudeToPixels(lat_lon);

    // Convert pixels to meters
    return convertPixelsToMeters(pixels);

}

glm::ivec2 ElevationData::longitudeLatitudeToPixels(const glm::dvec2 lat_lon) const {

    // First convert to pixels using the gdal transform
    return glm::ivec2((lat_lon.x - _StaticGDAL::_gdal_transform[0]) / _StaticGDAL::_gdal_transform[1],
                      (lat_lon.y - _StaticGDAL::_gdal_transform[3]) / _StaticGDAL::_gdal_transform[5]);

}

glm::dvec3 ElevationData::metersToMetersAndElevation(const glm::dvec2& pos_meters) const {

    glm::dvec3 pos_meters_sample = glm::dvec3(pos_meters.x, pos_meters.y, 0.0);
    float z = 0.0;

    // Get the pixel position
    glm::ivec2 pos_pixels = metersToPixels(pos_meters);

    // Do the sample
    CPLErr err = _StaticGDAL::_gdal_raster_band->RasterIO(GF_Read, pos_pixels.x, pos_pixels.y, 1, 1, &z, 1, 1, GDT_Float32, 0, 0);
    if (err)
        throw std::runtime_error("There was an error reading from the dataset");

    pos_meters_sample.z = (double)z;

    return pos_meters_sample;

}


glm::dvec3 ElevationData::pixelsToMetersAndElevation(const glm::ivec2& pos_pixels) const {

    // First convert pixels to meters
    glm::dvec2 pos_meters = convertPixelsToMeters(pos_pixels);
    glm::dvec3 pos_meters_sample = glm::dvec3(pos_meters.x, pos_meters.y, 0.0);

    // Now get the sample instead of calling metersToMetersAndElevation because GDAL samples in pixels
    CPLErr err = _StaticGDAL::_gdal_raster_band->RasterIO(GF_Read, pos_pixels.x, pos_pixels.y, 1, 1, &pos_meters_sample.z, 1, 1, GDT_Float32, 0, 0);
    if (err)
        throw std::runtime_error("There was an error reading from the dataset");

    return pos_meters_sample;

}

bool ElevationData::routeInsideData(const glm::dvec2& start, const glm::dvec2& dest) {

    // Get the origin and extent as points
    glm::dvec2 origin = pixelsToLongitudeLatitude(glm::ivec2(0));
    glm::dvec2 extent = pixelsToLongitudeLatitude(glm::ivec2(_StaticGDAL::_gdal_dataset->GetRasterXSize(),
                                                             _StaticGDAL::_gdal_dataset->GetRasterYSize()));

    // Check if start is outside
    if (start.x < origin.x || start.x > extent.x ||
            start.y > origin.y || start.y < extent.y)
        return false;

    // Check if dest is outside
    if (dest.x < dest.x || dest.x > dest.x ||
            dest.y > dest.y || dest.y < dest.y)
        return false;

    return true;

}

void ElevationData::calcCroppedSize(const glm::dvec2& start, const glm::dvec2& dest) {

    // Get the origin of the cropped data.
    // On the X this is the min because as you go left, X gets more negative
    // On the Y this is the max because as you go up, Y gets more positive
    _crop_origin = glm::dvec2(glm::min(start.x, dest.x),
                              glm::max(start.y, dest.y));

    // Get the extent of the cropped data.
    // On the X this is the max because as you go right, X gets more positive
    // On the Y this is the min because as you go down, Y gets more negative
    _crop_extent = glm::dvec2(glm::max(start.x, dest.x),
                              glm::min(start.y, dest.y));

    // Add padding on
    _crop_origin += glm::dvec2(-ROUTE_PADDING,
                                ROUTE_PADDING);

    _crop_extent += glm::dvec2( ROUTE_PADDING,
                                -ROUTE_PADDING);

}

void ElevationData::createOpenCLImage() {

    // Convert the cropped rect to pixels
    glm::ivec2 crop_origin_p = longitudeLatitudeToPixels(_crop_origin);
    glm::ivec2 crop_extent_p = longitudeLatitudeToPixels(_crop_extent);

    // Make sure that the coordinates are inside the raster image
    glm::ivec2 crop_origin_c = glm::ivec2(glm::clamp(crop_origin_p.x, 0, _StaticGDAL::_gdal_dataset->GetRasterXSize()),
                                          glm::clamp(crop_origin_p.y, 0, _StaticGDAL::_gdal_dataset->GetRasterYSize()));
    glm::ivec2 crop_extent_c = glm::ivec2(glm::clamp(crop_extent_p.x, 0, _StaticGDAL::_gdal_dataset->GetRasterXSize()),
                                          glm::clamp(crop_extent_p.y, 0, _StaticGDAL::_gdal_dataset->GetRasterYSize()));

    // Calculate the adjusted width and height
    glm::ivec2 size = crop_extent_c - crop_origin_c;

    static size_t max_size = Kernel::getDevice().get_info<CL_DEVICE_IMAGE2D_MAX_WIDTH>();

    // Ensure that we can actually fit all of the data required for this route
    // Otherwise throw and exception
    if (size.x >= max_size || size.y >= max_size)
        throw std::runtime_error("Route was too large to calculate");


    // OpenCL needs image data to be in a 1D double array.
    // Go line by line and read GDAL data to get the data in the format we need
    std::vector<float> image_data = std::vector<float>(size.x * size.y);

    for (int i = 0; i < size.y; i++) {

        CPLErr err = _StaticGDAL::_gdal_raster_band->RasterIO(GF_Read, crop_origin_c.x, i + crop_origin_c.y, size.x,
                                                                       1, &image_data[size.x * i], size.x, 1, GDT_Float32, 0, 0);
        if (err)
            throw std::runtime_error("There was an error reading from the dataset");

    }

    // Create the OpenCL image
    boost::compute::image_format format = boost::compute::image_format(CL_INTENSITY, CL_FLOAT);
    _opencl_image = boost::compute::image2d(Kernel::getContext(), size.x, size.y, format, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &image_data[0]);

    // Figure out the min and max elevations
    // Get stuff we need to execute a kernel on
    const boost::compute::context& ctx =   Kernel::getContext();
    boost::compute::command_queue& queue = Kernel::getQueue();

    std::vector<float>            min_max_host(2);
    boost::compute::vector<float> min_max_device(2, ctx);

    // Generate the program sounds
    const std::string source = BOOST_COMPUTE_STRINGIZE_SOURCE(

            __kernel void computeMinMax(__read_only image2d_t image, __global float* min_max, int width, int height) {

                const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

                // Use this to exclude no data
                const float extreme = 1000000.0;

                size_t x = get_global_id(0);
                size_t y = get_global_id(1);

                float sample = read_imagef(image, sampler, (float2)(x, y)).x;

                // Do a sanity check
                if (fabs(sample) < extreme) {

                    min_max[0] = min(sample, min_max[0]);
                    min_max[1] = max(sample, min_max[1]);

                }

            }

    );

    // Create a temporary kernel and execute it
    Kernel extrema_kernel = Kernel(source, "computeMinMax");
    extrema_kernel.setArgs(_opencl_image, min_max_device.get_buffer(), size.x, size.y);
    extrema_kernel.execute2D(glm::vec<2, size_t>(0, 0),
                             glm::vec<2, size_t>(size.x, size.y),
                             glm::vec<2, size_t>(1, 1));

    boost::compute::copy(min_max_device.begin(), min_max_device.end(), min_max_host.begin(), queue);

    _elevation_min = min_max_host[0];
    _elevation_max = min_max_host[1];

    std::cout << _elevation_min << " " << _elevation_max << std::endl;

}

glm::dvec2 ElevationData::getCroppedSizeMeters() const {

    // Convert to meters and subtract
    return longitudeLatitudeToMeters(_crop_extent) - longitudeLatitudeToMeters(_crop_origin);

}

glm::vec2 ElevationData::getCroppedOriginMeters() const { return longitudeLatitudeToMeters(_crop_origin); }

double ElevationData::getLongestAllowedRoute() {
    
    // Querry OpenCL for the max texutre height. We assume that the max texture size is a square.
    static size_t max_size = Kernel::getDevice().get_info<CL_DEVICE_IMAGE2D_MAX_WIDTH>();
    
    // Multiply it by both of the meter conversions and return the smaller one
    return glm::min((float)max_size * _StaticGDAL::_pixelToMeterConversions[0],
                    (float)max_size * _StaticGDAL::_pixelToMeterConversions[1]);

}
