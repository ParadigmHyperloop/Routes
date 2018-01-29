//
// Created by Logan on 10/30/17.
//

#ifndef ROUTES_ROUTES_H
#define ROUTES_ROUTES_H

#include "genetics/genetics.h"

/** */

/** The default size of the population for calculating a route */
#define POP_SIZE 1000

/** The default number of generations that the population should be bred for */
#define NUM_GENERATIONS 300

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
         *
         * @param dest
         * The end position in longitude latitude of the route.
         *
         * @return
         * The computed points of the route in longitude latitude and elevation.
         */
        static std::vector<glm::vec3> calculateRoute(glm::vec2 start, glm::vec2 dest);

    private:

        /**
         * In some areas we have a no data value in the elevation data. In this case we have no idea what to compute
         * the Z of the route to be. For now we check for this case and throw a std::runtime_error if this happens.
         *
         * @param point
         * The point to be checked for validity.
         *
         * @return
         * True if the point was valid, false if otherwise.
         */
        static bool validatePoint(const glm::vec3& point);

};

#endif //ROUTES_ROUTES_H
