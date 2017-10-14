//
// Created by Logan on 10/12/17.
//

#ifndef ROUTES_GENETICS_H
#define ROUTES_GENETICS_H

#include "population.h"

/** */

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
         * @param start
         * The starting location in meters of the path.
         *
         * @param dest
         * The ending location in meters of the path.
         */
        static void solve(Population& pop, Pod& pod, glm::vec4 start, glm::vec4 dest, int generations);

};

#endif //ROUTES_GENETICS_H
