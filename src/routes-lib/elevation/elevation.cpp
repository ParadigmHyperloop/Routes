//
// Created by Logan on 10/12/17.
//

#include "elevation.h"

ElevationData::_StaticGDAL ElevationData::_init;

GDALDataset* ElevationData::_StaticGDAL::_gdal_dataset;
GDALRasterBand* ElevationData::_StaticGDAL::_gdal_raster_band;

double ElevationData::_StaticGDAL::_gdal_transform[6];

int ElevationData::_StaticGDAL::_width;
int ElevationData::_StaticGDAL::_height;

double ElevationData::_StaticGDAL::_width_meters;
double ElevationData::_StaticGDAL::_height_meters;
double ElevationData::_StaticGDAL::_elevation_min;
double ElevationData::_StaticGDAL::_elevation_max;
double ElevationData::_StaticGDAL::_pixelToMeterConversions[2];

ElevationData::_StaticGDAL::_StaticGDAL() {
    
    // Register all of the file formats for GDAL
    GDALAllRegister();
    
    // Open up the GDAL dataset
    _gdal_dataset = (GDALDataset*)GDALOpenShared(GDAL_DB_PATH, GA_ReadOnly);
    if (!_gdal_dataset)
        throw std::runtime_error("The databse could not be loaded from the disk. Make sure to build it first.");
    
    std::cout << "Statically initializing the GDAL Data\n";
    
    // Get the raster band
    _gdal_raster_band = _gdal_dataset->GetRasterBand(1);
    
    calcConversions();
    calcStats();
    calcMinMax();
    
}

ElevationData::_StaticGDAL::~_StaticGDAL() {
    
    // Close the GDAL dataset
    GDALClose(_gdal_dataset);
    
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

void ElevationData::_StaticGDAL::calcMinMax() {
    
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

/***********************************************************************************************************************************************/

ElevationData::ElevationData(const glm::dvec2& start, const glm::dvec2& dest) {

    // Get the size and then make the image
    calcCroppedSize(start, dest);
    createOpenCLImage();

}

int ElevationData::getWidth() const { return _StaticGDAL::_width; }
int ElevationData::getHeight() const { return _StaticGDAL::_height; }

double ElevationData::getWidthInMeters() const { return _StaticGDAL::_width_meters; }
double ElevationData::getHeightInMeters() const { return _StaticGDAL::_height_meters; }

double ElevationData::getMinElevation() const { return _StaticGDAL::_elevation_min; }
double ElevationData::getMaxElevation() const { return _StaticGDAL::_elevation_max; }

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

}

glm::dvec2 ElevationData::getCroppedSizeMeters() const {

    // Convert to meters and subtract
    return longitudeLatitudeToMeters(_crop_extent) - longitudeLatitudeToMeters(_crop_origin);

}

glm::vec2 ElevationData::getCroppedOriginMeters() const { return longitudeLatitudeToMeters(_crop_origin); }

double ElevationData::getLongestAllowedRoute() {
    
    // Querry OpenCL for the max texutre height. We assume that the max texture size is a square.
    size_t max_size = Kernel::getDevice().get_info<CL_DEVICE_IMAGE2D_MAX_WIDTH>();
    
    // Multiply it by both of the meter conversions and return the smaller one
    return glm::min((float)max_size * _StaticGDAL::_pixelToMeterConversions[0],
                    (float)max_size * _StaticGDAL::_pixelToMeterConversions[1]);

}
