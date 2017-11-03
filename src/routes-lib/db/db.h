//
// Created by Logan on 10/29/17.
//

#ifndef ROUTES_DB_H
#define ROUTES_DB_H

#include <boost/range/iterator_range.hpp>
#include <boost/filesystem.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <gdal_priv.h>

/** The minimum dx for the line intersection to not be treated as a vertical line */
#define X_EPSILON 0.000000001

/** The class responsible for managing the data database.
 * The database is a simple .csv file that contains the origin and size in degrees of each .img file in the
 * data directory.
 */
class DB {

    public:

        /** Builds the database from scratch. This can be a slow process because every file needs to be opened. */
        static void build();

        /**
         * This function computes the datasets that are required to calculate a route from one point to another.
         * We create a oriented bounding box that is a degree wide and check which datasets intersect with it.
         *
         * @param start
         * The starting position in longitude latitude of the path.
         *
         * @param dest
         * The ending position in longitude latitude of the path.
         *
         * @return
         * All of the datasets that are required to calculate a path from the start to the destination.
         */
        static std::vector<std::string> getRequiredDatasets(glm::vec2 start, glm::vec2 dest);

        /** Loads the database from the disk. If it cannot be loaded, the program will exit */
        static bool load();

    private:

        /**
         * Determines if a ray with a given start and direction intersects the entry at index.
         *
         * @param index
         * The index of the dataset to check against
         *
         * @param start
         * The starting position in latitude longitude
         *
         * @param direction
         * The direction in latitude longitude
         *
         * @return
         * The result of the hit detection; true if the ray intersects, false if it does not.
         */
        static bool lineIntersectsDatSet(int index, const glm::vec3 &start, const glm::vec3 &direction);

        /** A struct for storing each entry in the databsae once it has been loaded */
        struct Entry {

            /** The path that the file exists at */
            std::string path;

            /** The origin in longitude latitude of the dataset */
            glm::vec2 origin;

            /** The size in degrees of the dataset */
            glm::vec2 size;

        };

        /** All of the entries that were in the database file */
        static std::vector<Entry> _entries;

};

#endif //ROUTES_DB_H
