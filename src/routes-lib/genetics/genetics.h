//
// Created by Logan on 10/12/17.
//

#ifndef ROUTES_GENETICS_H
#define ROUTES_GENETICS_H

#include "population.h"

#include <pqxx/pqxx>

/** A simple class to manage the genetic cycle of a population */
class Genetics {

    public:

        /**
         * This function runs the genetic algorithm to compute the most efficient path from start to dest.
         *
         * @param pop
         * The population of individuals that represent solutions to the path. This should be a new population
         * but an old one can be reused assuming that it has been simulated with the same start and dest.
         *
         * @param pod
         * The information about the hyperloop pod. Determines how curved the track can be.
         *
         * @return
         * The points of the calculated path in meters. This will need to be converted to latitude and longitude
         * to be properly displayed.
         */
        static std::vector<glm::vec3> solve(Population& pop, Pod& pod, int generations, const glm::dvec2& start, const glm::dvec2& dest, std::string objectiveType);

    private:

        /**
         * The control points of the optimal solution at the current generation
         */
        static double _opt_control_points;

        /**
         * The starting latitude
         */
        static double _lat_start;

        /**
         * The ending latitude
         */
        static double _lat_end;

        /**
         * The starting longitude
         */
        static double _long_start;

        /**
         * The ending longitude
         */
        static double _long_end;

};

#endif //ROUTES_GENETICS_H
