//
// Created by Logan on 10/23/17.
//

#ifndef ROUTES_ELEVATION_STITCH_H
#define ROUTES_ELEVATION_STITCH_H

#include "elevation.h"

/** */

/**
 * The transforms aren't exact on the data we are loading so we estimate that they are the same.
 * This value represents the furthest apart they can be.
 */
#define EPSILON 0.00001f

/**
 * This class is responsible for stitching together two pieces of data loaded from GDAL.
 * Because the data we are using (from the USGS) is slightly overlapping stitching needs to be done.
 * This class will take in various ElevationData objects and stitch them together.
 */
class ElevationStitch {

    public:

        /**
         * This function will load up two pieces of data from the disk and then stitch them together.
         * The original data on the disk will never be modified. In the case of overlap data from the stitcher
         * parameter will be discarded.
         * This function is slow and ineffecient so it should not be done in real time.
         *
         * @param root
         * The path to the data to treat as the root for the stitch. Any data in here will not be discarded.
         *
         * @param sticher
         *  The path to the piece of data that should be added to the root. Data in here is subject to being
         * discarded depending on if there is overlap.
         *
         * @return
         */
        static ElevationData stitch(const std::string& root, const std::string& sticher);

    private:

        /**
         * This function is used to help calculate how large the buffer needs to be to contain two elevation data.
         * This function works on one axis and determines the size in that axis needed to contain the two
         * data pieces.
         *
         * @param root_origin
         * The origin in latitude / longitude of the root dataset
         *
         * @param root_size
         * The size in degrees of the root dataset
         *
         * @param sticher_origin
         * The origin in latitude / longitude of the sticher dataset
         *
         * @param sticher_size
         * The size in degrees of the root dataset
         *
         * @return
         * The size in degrees required to contain the two datasets on the given axis
         */
        static float getRequiredSize(float root_origin, float root_size, float sticher_origin, float sticher_size);


};

#endif //ROUTES_ELEVATION_STITCH_H
