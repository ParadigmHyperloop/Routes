//
// Created by Logan on 10/29/17.
//

#ifndef ROUTES_DB_H
#define ROUTES_DB_H

#include <boost/filesystem.hpp>
#include <gdal.h>

/** The class responsible for managing the data database.
 * The database is a simple .csv file that contains the origin and size in degrees of each .img file in the
 * data directory.
 */
class DB {

    public:

        /** Builds the database from scratch. This can be a slow process because every file needs to be opened. */
        static void build();

};

#endif //ROUTES_DB_H
