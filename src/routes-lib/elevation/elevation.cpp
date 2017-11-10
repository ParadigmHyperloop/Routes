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

ElevationData::ElevationData(const std::string& file_path, const glm::dvec3& start, const glm::dvec3& dest) {

    // Get the dataset from the disk
    _gdal_dataset = (GDALDataset*)GDALOpenShared(file_path.c_str(), GA_ReadOnly);
    if (_gdal_dataset) {

        // Get the raster band
        _gdal_raster_band = _gdal_dataset->GetRasterBand(1);

        calcCroppedSize(start, dest);

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

double ElevationData::getWidthInMeters() const { return _width_meters; }
double ElevationData::getHeightInMeters() const { return _height_meters; }

double ElevationData::getMinElevation() const { return _elevation_min; }
double ElevationData::getMaxElevation() const { return _elevation_max; }

const boost::compute::image2d& ElevationData::getOpenCLImage() const { return _opencl_image; }

glm::dvec2 ElevationData::convertPixelsToMeters(const glm::ivec2& pos_pixels) const {

    // Multiply by the conversion factors
    return glm::dvec2((double)pos_pixels.x * pixelToMeterConversions[0],
                      (double)pos_pixels.y * pixelToMeterConversions[1]);

}

glm::ivec2 ElevationData::metersToPixels(const glm::dvec2 &pos_meters) const {

    // Divide by the conversion factors
    return glm::ivec2(pos_meters.x / pixelToMeterConversions[0],
                      pos_meters.y / pixelToMeterConversions[1]);

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
    return glm::dvec2(_gdal_transform[0] + (double)pos_pixels.x * _gdal_transform[1],
                      _gdal_transform[3] + (double)pos_pixels.y * _gdal_transform[5]);

}

glm::dvec2 ElevationData::longitudeLatitudeToMeters(const glm::dvec2 lat_lon) const {

    // First convert to pixels using the gdal transform
    glm::ivec2 pixels = longitudeLatitudeToPixels(lat_lon);

    // Convert pixels to meters
    return convertPixelsToMeters(pixels);

}

glm::ivec2 ElevationData::longitudeLatitudeToPixels(const glm::dvec2 lat_lon) const {

    // First convert to pixels using the gdal transform
    return glm::ivec2((lat_lon.x - _gdal_transform[0]) / _gdal_transform[1],
                      (lat_lon.y - _gdal_transform[3]) / _gdal_transform[5]);

}

glm::dvec3 ElevationData::metersToMetersAndElevation(const glm::dvec2& pos_meters) const {

    glm::dvec3 pos_meters_sample = glm::dvec3(pos_meters.x, pos_meters.y, 0.0);
    float z = 0.0;

    // Get the pixel position
    glm::ivec2 pos_pixels = metersToPixels(pos_meters);

    // Do the sample
    CPLErr err = _gdal_raster_band->RasterIO(GF_Read, pos_pixels.x, pos_pixels.y, 1, 1, &z, 1, 1, GDT_Float32, 0, 0);
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
    CPLErr err = _gdal_raster_band->RasterIO(GF_Read, pos_pixels.x, pos_pixels.y, 1, 1, &pos_meters_sample.z, 1, 1, GDT_Float32, 0, 0);
    if (err)
        throw std::runtime_error("There was an error reading from the dataset");

    return pos_meters_sample;

}

void ElevationData::calcCroppedSize(const glm::dvec3& start, const glm::dvec3& dest) {

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

void ElevationData::calcConversions() {

    // First read the transform from GDAL (we assume that we get something)
    _gdal_dataset->GetGeoTransform(_gdal_transform);

    // Calculate conversion factors based on the size of pixels in degrees.
    // This uses a simplified formula of degrees -> arcseconds -> meters
    double degrees_to_meters = EARTH_RADIUS * M_PI / 180.0;

    // X conversion
    pixelToMeterConversions[0] = degrees_to_meters * (double)_gdal_transform[1];

    // Y Conversion - we use abs because GDAL can store it in degrees
    pixelToMeterConversions[1] = degrees_to_meters * fabs((double)_gdal_transform[5]);

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

    double gdal_min_max[2] = {0.0, 0.0};
    int succ[2] = {0, 0};

    gdal_min_max[0] = _gdal_raster_band->GetMinimum(&succ[0]);
    gdal_min_max[1] = _gdal_raster_band->GetMaximum(&succ[1]);

    if (!succ[0] || !succ[1])
        _gdal_raster_band->ComputeRasterMinMax(true, gdal_min_max);

    // Save the min and max
    _elevation_min = gdal_min_max[0];
    _elevation_max = gdal_min_max[1];

}

void ElevationData::createOpenCLImage() {

    // Convert the cropped rect to pixels
    glm::ivec2 crop_origin_p = longitudeLatitudeToPixels(_crop_origin);
    glm::ivec2 crop_extent_p = longitudeLatitudeToPixels(_crop_extent);

    // Make sure that the coordinates are inside the raster image
    glm::ivec2 crop_origin_c = glm::ivec2(glm::clamp(crop_origin_p.x, 0, _gdal_dataset->GetRasterXSize()),
                                          glm::clamp(crop_origin_p.y, 0, _gdal_dataset->GetRasterYSize()));
    glm::ivec2 crop_extent_c = glm::ivec2(glm::clamp(crop_extent_p.x, 0, _gdal_dataset->GetRasterXSize()),
                                          glm::clamp(crop_extent_p.y, 0, _gdal_dataset->GetRasterYSize()));

    // Calculate the adjusted width and height
    glm::ivec2 size = crop_extent_c - crop_origin_c;

    // OpenCL needs image data to be in a 1D double array.
    // Go line by line and read GDAL data to get the data in the format we need
    std::vector<float> image_data = std::vector<float>(size.x * size.y);

    for (int i = 0; i < size.y; i++) {

        CPLErr err = _gdal_raster_band->RasterIO(GF_Read, crop_origin_c.x, i + crop_origin_c.y, size.x,
                                                 1, &image_data[size.x * i], size.x, 1, GDT_Float32, 0, 0);
        if (err)
            throw std::runtime_error("There was an error reading from the dataset");

    }

    std::cout << image_data[10000] << std::endl;

    // Create the OpenCL image
    boost::compute::image_format format = boost::compute::image_format(CL_INTENSITY, CL_FLOAT);
    _opencl_image = boost::compute::image2d(Kernel::getContext(), size.x, size.y, format, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &image_data[0]);

}

glm::dvec2 ElevationData::getCroppedSizeMeters() const {

    // Convert to meters and subtract
    return longitudeLatitudeToMeters(_crop_extent) - longitudeLatitudeToMeters(_crop_origin);

}