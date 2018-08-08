//
// Created by Logan on 10/12/17.
//

#ifndef ROUTES_GENETICS_H
#define ROUTES_GENETICS_H

#include "population.h"

#include <stdlib.h>
#include <iostream>

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

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
         * @return
         * The points of the calculated path in meters. This will need to be converted to latitude and longitude
         * to be properly displayed.
         */
        static std::vector<glm::vec3> solve(Population& pop, Pod& pod, int generations);

};

#endif //ROUTES_GENETICS_H
