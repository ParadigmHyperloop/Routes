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

        /**
         * Gets the time to traverse the calculated route.
         * Should always be called after calculateRoute.
         *
         * @return
         * Time to traverse route in seconds
         */
        static float getTime();

        /**
         * Gets the length of the calculated route.
         * Should always be called after calculateRoute.
         *
         * @return
         * Length of the track in meters
         */
        static float getLength();

        /**
         * Gets the elevations of the route.
         * Should always be called after calculateRoute.
         *
         * @return
         * Pairs of distance along track and the elevation at that point in meters
         */
        static std::vector<glm::vec2> getElevations();

        /**
         * Gets the elevations of the ground along the route.
         * Should always be called after calculateRoute.
         *
         * @return
         * Pairs of distance along track and the elevation at that point in meters
         */
        static std::vector<glm::vec2> getGElevations();

        /**
         * Gets the speeds along the route.
         * Should always be called after calculateRoute.
         *
         * @return
         * Pairs of distance along track and the speed at that point in meters / second
         */
        static std::vector<glm::vec2> getSpeeds();

        /**
         * Gets the grade of the track along the route.
         * Shoudl always be called after calculateRoute.
         *
         * @return
         * Pairs of distance along the track and the grade at that point
         */
        static std::vector<glm::vec2> getGrades();

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

        /**
         * Time to traverse route in seconds
         */
        static float _time;

        /**
         * Length of the track in meters
         */
        static float _length;

        /**
         * pairs of distance along track and the elevation at that point in meters
         */
        static std::vector<glm::vec2> _elevations;

        /**
         * pairs of distance along track and the elevation of the ground at that point in meters
         */
        static std::vector<glm::vec2> _ground_elevations;

        /**
         * pairs of distance along track and the speed at that point in meters / second
         */
        static std::vector<glm::vec2> _speeds;

        /**
         * pairs of distance along track and the grade at that point
         */
         static std::vector<glm::vec2> _grades;

};

#endif //ROUTES_ROUTES_H
