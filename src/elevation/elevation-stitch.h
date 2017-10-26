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
         * @param stitcher
         *  The path to the piece of data that should be added to the root. Data in here is subject to being
         * discarded depending on if there is overlap.
         *
         * @return
         * The stitched data
         */
        static ElevationData stitch(const std::string& root, const std::string& stitcher);

        /**
         * This function will load up two pieces of data from the disk and then stitch them together.
         * The original data on the disk will never be modified. In the case of overlap data from the stitcher
         * parameter will be discarded.
         * This function is slow and ineffecient so it should not be done in real time.
         * This particular version of the function will stitch together an arbitrary number of datasets.
         * The first string in the vector is analogous to the root in the other version of the function.
         * The last specified file's data is able to be overwritten by any before it, the second to last
         * will be able to be overwritten by anything before that and so on.
         * Assumes that all of the data being stitched together are adjacent.
         *
         * @param paths
         * A vector specifying the paths of the files on the disk that need to be stitched together.
         *
         * @return
         * The stitched data
         */
        static ElevationData stitch(const std::vector<std::string>& paths);

    private:

        /** A struct to store the origin and size of GDAL datasets internally */
        struct DataRect {

            /** The origin of the dataset in longitude latitude */
            glm::vec2 origin;

            /** The size of the dataset in degrees */
            glm::vec2 extents;

            /** Default constructor */
            DataRect();

            /** Constructs a DataRect based off of a dataset loaded from GDAL */
            DataRect(GDALDataset* dataset);

        };

        /**
         * This function can be used to get the required size in degrees to hold 2 pieces of data.
         *
         * @param a
         * The first dataset.
         *
         * @param b
         * The second dataset.
         *
         * @return
         * The required buffer size in degrees to contain both rects.
         */
        static DataRect getRequiredRect(DataRect a, DataRect b);

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
        static float getRequiredSizeAxis(float root_origin, float root_size, float sticher_origin, float sticher_size);


};

#endif //ROUTES_ELEVATION_STITCH_H
