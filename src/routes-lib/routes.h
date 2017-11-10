//
// Created by Logan on 10/30/17.
//

#ifndef ROUTES_ROUTES_H
#define ROUTES_ROUTES_H

#include "genetics/genetics.h"
#include "html/html.h"
#include "db/db.h"

/** */

/** The default size of the population for calculating a route */
#define POP_SIZE 200

/** The default number of generations that the population should be bred for */
#define NUM_GENERATIONS 400

/** This is a simple class to handle the complete calculation of a route. */
class Routes {

    public:

        /**
         * Performs the complete calculation of a route from one point to another given the start and destination.
         * This will calculate which datasets need to be stitched, stitch them, calculate the route and output
         * an HTML file.
         *
         * @param start
         * The start position in longitude latitude of the route.
         * The z component is the starting elevation of the route.
         *
         * @param dest
         * The end position in longitude latitude of the route.
         * z component is the ending elevation of the route.
         *
         * @return
         * The computed points of the route in longitude latitude and elevation.
         */
        static std::vector<glm::vec3> calculateRoute(glm::vec3 start, glm::vec3 dest);

};

#endif //ROUTES_ROUTES_H
