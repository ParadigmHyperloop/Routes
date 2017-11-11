//
// Created by Logan on 10/12/17.
//

#ifndef ROUTES_ELEVATION_H
#define ROUTES_ELEVATION_H

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <boost/filesystem.hpp>
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

/** The location of the virtual dataset that references all of the data */
#define GDAL_DB_PATH "../data/db.vtf"

/**
 * ElevationData is a class that converts elevation data from GDAL into
 * a format that can be read in OpenCL.
 */
class ElevationData {

    public:

        /**
         * Creates an OpenCL image that is the proper size to contain the route specified by the given start and destination.
         *
         * @param start
         * The starting position in longitude latitude of the route.
         *
         * @param dest
         * The ending position in longitude latitude of the route.
         */
        ElevationData(const glm::dvec2& start, const glm::dvec2& dest);

        /** Gets the width of the entire raster image in pixels */
        inline int getWidth() const;

        /** Gets the height of the entire raster image in pixels */
        inline int getHeight() const;

        /** Gets the entire width of the raster image in meters */
        double getWidthInMeters() const;

        /** Gets the entire height of the raster image in meters */
       double getHeightInMeters() const;

        /** Gets the minimum height found in the entire raster image in meters */
        double getMinElevation() const;

        /** Gets the maximum height found in the entire raster image in meters */
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

        /**
         * Gets the cropped origin's location in meters.
         * Z and W are unused.
         *
         * @return
         * The origin of the cropped portion of the raster in meters.
         */
        glm::vec2 getCroppedOriginMeters() const;
    
        /**
        * Returns the longest route that can be computed with this graphics card in meters. This is due to a maximum texture size limit on GPUs.
        * This number does not guranteee that a route of that length can actually be calculated due to a max texture buffer limit too.
        */
        static double getLongestAllowedRoute();

    private:

       /*
        * Returns a boolean whether or not this route can be computed by the dataset that is on the disk.
        *
        * @param start
        * The starting position in longitude latitude of the route.
        *
        * @param dest
        * The ending position in longitude latitude of the route.
        *
        * @return
        * true if the route is contained inside the data, false otherwise.
        *
        */
        bool routeInsideData(const glm::dvec2& start, const glm::dvec2& dest);

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
        void calcCroppedSize(const glm::dvec2& start, const glm::dvec2& dest);

        /**
         * Translate the GDAL data into an OpenCL texture.
         * This function also uses an OpenCL kernel to calculate the local min and max elevation instead of
         * the global min and max to decrease the sample space.
         */
        void createOpenCLImage();

        /** The minimum elevation in meters of the terrain in the raster image in meters */
        double _elevation_min;

        /** The maximum elevation in meters of the terrain in the raster image in meters */
        double _elevation_max;

        /** The origin of the subset of data that was taken to encapsulate the route in longitude latitude */
        glm::dvec2 _crop_origin;

        /** The extent (origin + size) of the subset of data that was taken to encapsulate the route in longitude latitude */
        glm::dvec2 _crop_extent;

        /** The OpenCL image that is created once the data is loaded up from GDAL */
        boost::compute::image2d _opencl_image;
    
/***********************************************************************************************************************************************/

        /**
         * Since all of the GDAL data can be referneced by a single file with the same attributes all of that is handled in this class.
         * It is a private class of ElevationData and is used as a static object to essentially perform static destruction and construction.
         *
         */
        class _StaticGDAL {
            
            public:
            
                /** Initializes GDAL, loads the database from the disk and calculates the conversions, min and max, size, etc. */
                _StaticGDAL();
            
                /** Cloes the GDAL dataset */
                ~_StaticGDAL();
            
                /** Calculate the conversion factors to translate GDAL pixels to meters */
                void calcConversions();
            
                /** Construction helper function for calculating the min, max and width an height */
                void calcStats();

                /**
                 * The GDAL data that is loaded from the disk.
                 * This contains all of the elevation data including how to interpret it.
                 */
                static GDALDataset* _gdal_dataset;
            
                /**
                 * The location where GDAL stores all of its actual data.
                 * In the data downloaded from USGS, there is only one raster band
                 */
                static GDALRasterBand* _gdal_raster_band;
            
                /** The information about the pixel size and location of the GDAL data */
                static double _gdal_transform[6];
            
                /** The width in pixels of the image */
                static int _width;
            
                /** The height in pixels of the image */
                static int _height;
            
                /** The width in meters of the image */
                static double _width_meters;
            
                /** The height in meters of the image */
                static double _height_meters;
            
                /**
                 * Two conversions factors that translate pixels to meters for this particular image.
                 * pixelToMeterConversions[0] corresponds to X, _pixelToMeterConversions[1] corresponds to y.
                 */
                static double _pixelToMeterConversions[2];
            
        };
    
        /** The static instance of _StaticGDAL responsible for managing all of the data from GDAL */
        static _StaticGDAL _init;

};

#endif //ROUTES_ELEVATION_H
