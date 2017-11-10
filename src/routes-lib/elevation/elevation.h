//
// Created by Logan on 10/12/17.
//

#ifndef ROUTES_ELEVATION_H
#define ROUTES_ELEVATION_H

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <gdal_priv.h>
#include "../opencl/kernel.h"

/** */

/**
 * The radius of the Earth in meters.
 * Data pulled from NASA https://nssdc.gsfc.nasa.gov/planetary/factsheet/earthfact.html
 */
#define EARTH_RADIUS 6378137.0

/**
* When determining the cropped rect for a route we add a little bit of padding.
* This gives slightly more sample space to explore to perhaps find a better solution.
*/
#define ROUTE_PADDING 0.1

/**
 * ElevationData is a class that converts elevation data from GDAL into
 * a format that can be read in OpenCL.
 */
class ElevationData {

    public:

        /**
         * Loads up an external file with GDAL and then converts it to
         * an OpenCL image and stores it inside a new ElevationData object.
         *
         * @param file_path
         * The path on the disk where the elevation data should be loaded from.
         *
         * @param start
         * The starting position in longitude latitude of the route.
         *
         * @param dest
         * The ending position in longitude latitude of the route.
         */
        ElevationData(const std::string& file_path, const glm::dvec3& start, const glm::dvec3& dest);

        /**
         * GDAL obejects are created on the heap so these are de-allocated here.
         * Also deletes the texture so it doesnt run away.
         */
        ~ElevationData();

        /** Gets the width of the raster image in pixels */
        int getWidth() const;

        /** Gets the height of the raster image in pixels */
        int getHeight() const;

        /** Gets the width of the raster image in meters */
        double getWidthInMeters() const;

        /** Gets the height of the raster image in meters */
        double getHeightInMeters() const;

        /** Gets the minimum height found in the raster image in meters */
        double getMinElevation() const;

        /** Gets the maximum height found in the raster image in meters */
        double getMaxElevation() const;

        /** Gets the size of the cropped dataset in meters */
        glm::dvec2 getCroppedSizeMeters() const;
    
        /** Gets the uploaded OpenCL data */
        const boost::compute::image2d& getOpenCLImage() const;

        /**
         * Takes in a location inside the raster image (measured in pixels) and converts that
         * to meters. The origin remains 0,0 in the upper left corner.
         *
         * @param pos_pixels
         * The position in pixels to be converted.
         *
         * @return
         * The output position in meters.
         */
        glm::dvec2 convertPixelsToMeters(const glm::ivec2& pos_pixels) const;

        /**
         * Takes in a location inside the raster image (measured in meters) and converts that
         * to pixels. The origin remains 0,0 in the upper left corner.
         *
         * @param pos_meters
         * The position in meters to be converted
         *
         * @return
         * The output position in pixels
         */
        glm::ivec2 metersToPixels(const glm::dvec2 &pos_meters) const;

        /**
         * Takes in a location inside the raster image (measured in meters) and converts that
         * to longitude and latitude. This gives an absolute position representative of
         * its corresponding location in the real world.
         *
         * @param pos_meters
         * The position in the raster image (in meters) to be converted
         *
         * @return
         * The output longitude and latitude.
         * return[0] is longitude and return[1] is latitude.
         */
        glm::dvec2 metersToLongitudeLatitude(const glm::dvec2& pos_meters) const;

        /**
         * Takes in a location inside the raster image (measured in pixels) and converts that
         * to longitude and latitude. This gives an absolute position representative of
         * its corresponding location in the real world.
         *
         * @param pos_pixels
         * The position in the raster image (in pixels) to be converted
         *
         * @return
         * The output longitude and latitude.
         * return[0] is longitude and return[1] is latitude.
         */
        glm::dvec2 pixelsToLongitudeLatitude(const glm::ivec2& pos_pixels) const;

        /**
         * Takes in a position in longitude latitude and converts it to meters.
         *
         * @param lat_lon
         * The position in the raster image in longitude and latitude.
         *
         * @return
         * The converted position on the raster image in meters.
         */
        glm::dvec2 longitudeLatitudeToMeters(const glm::dvec2 lat_lon) const;

        /**
         * Takes in a position in longitude latitude and converts it to pixels.
         *
         * @param lat_lon
         * The position in the raster image in longitude and latitude.
         *
         * @return
         * The converted position on the raster image in pixels.
         */
        glm::ivec2 longitudeLatitudeToPixels(const glm::dvec2 lat_lon) const;

        /**
         * Takes in a location on the image in meters and samples from the raster data, essentially
         * turning a 2D position into a 3D position that corresponds to the point on the terrain.
         *
         * @param pos_meters
         * The position on the raster image in meters where the elevation should be sampled from.
         *
         * @return
         * The position on the terrain in meters. Z is the up direction on this vector.
         */
        glm::dvec3 metersToMetersAndElevation(const glm::dvec2& pos_meters) const;

        /**
         * Takes in a location on the image in pixels and samples from the raster data, essentially
         * turning a 2D position into a 3D position that corresponds to the point on the terrain.
         * This function takes an input in pixels and converts it to meters.
         *
         * @param pos_meters
         * The position on the raster image in meters where the elevation should be sampled from.
         *
         * @return
         * The position on the terrain in meters. Z is the up direction on this vector.
         */
        glm::dvec3 pixelsToMetersAndElevation(const glm::ivec2& pos_pixles) const;

    private:

        /**
        * When creating the OpenCL texture we only want to take as little data as we need. This function takes in the start and end of a route
        * and computes _crop_origin and _crop_extent to encapsulate the route with padding equal to ROUTE_PADDING.
        *
        * @param start
        * The starting position in longitude latitude of the route.
        *
        * @param dest
        * The ending position in longitude latitude of the route.
        */
        void calcCroppedSize(const glm::dvec3& start, const glm::dvec3& dest);
    
        /** Calculate the conversion factors to translate GDAL pixels to meters */
        void calcConversions();

        /** Construction helper function for calculating the min, max and width an height */
        void calcStats();

        /**
         * This is a workaround to a current bug with GDAL.
         * Some GIS files have "NoData" values which are massive numbers.
         * GDAL is supposed to ignore these, but for some reason when computing min and max it does not.
         * This function uses an OpenCL Kernel to get the min and max manually, ignoring these values.
         */
        void calcMinMax();

        /** Translate the GDAL data into an OpenCL texture */
        void createOpenCLImage();
    
        /**
         * This function computes the minimum bounding rectangle on the dataset for the given route. We do this so we can create an OpenCL
         * texture that is specially fit to the route. This is padded by DATASET_ROUTE_PADDING so that there is slightly more samplespace than required
         *
         * @param start
         * The starting position in longitude latitude of the route.
         *
         * @param dest
         * The ending position in longitude latitude of the route.
         */
        void calcCoordinates(const glm::dvec3& start, const glm::dvec3& dest);

        /**
         * The GDAL data that is loaded from the disk.
         * This contains all of the elevation data including how to interpret it.
         */
        GDALDataset* _gdal_dataset;

        /**
         * The location where GDAL stores all of its actual data.
         * In the data downloaded from USGS, there is only one raster band
         */
        GDALRasterBand* _gdal_raster_band;

        /** The information about the pixel size and location of the GDAL data */
        double _gdal_transform[6];

        /** The width in pixels of the image */
        int _width;

        /** The height in pixels of the image */
        int _height;

        /** The width in meters of the image */
        double _width_meters;

        /** The height in meters of the image */
        double _height_meters;

        /** The minimum elevation in meters of the terrain in the raster image in meters */
        double _elevation_min;

        /** The maximum elevation in meters of the terrain in the raster image in meters */
        double _elevation_max;

        /** The origin of the subset of data that was taken to encapsulate the route in longitude latitude */
        glm::dvec2 _crop_origin;
    
        /** The extent (origin + size) of the subset of data that was taken to encapsulate the route in longitude latitude */
        glm::dvec2 _crop_extent;

        /**
         * Two conversions factors that translate pixels to meters for this particular image.
         * pixelToMeterConversions[0] corresponds to X, pixelToMeterConversions[1] corresponds to y.
         */
        double pixelToMeterConversions[2];

        /** The OpenCL image that is created once the data is loaded up from GDAL */
        boost::compute::image2d _opencl_image;
    
        /** The origin of the dataset adjusted for the route in longitude latitude */
        glm::vec2 _bound_origin;
    
        /** The furthest extent of the dataset adjusted for the route in longitude latitude  */
        glm::vec2 _bound_extent;

    private:

        /**
        *  Statically initializes GDAL.
        *  Should never be called by a ElevationData object.
        */
        static bool initGDAL();

        /**
         *  Dummy boolean to allow initGDAL() during static initialization.
         *  Should never be used by an ElevationData object.
         */
        static bool _is_initialized;

};

#endif //ROUTES_ELEVATION_H
