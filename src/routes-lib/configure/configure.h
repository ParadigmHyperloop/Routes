//
// Created by isaac on 8/14/18.
//

#ifndef ROUTES_CONFIGURE_H
#define ROUTES_CONFIGURE_H

#ifdef _MSC_VER
#include <boost/config/compiler/visualc.hpp>
#endif
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <cassert>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>

#define FILE_PATH "../params.json"


#endif //ROUTES_CONFIGURE_H

/**
 * A struct for representing the parameter information.
 */
struct Configuration {

    /**
     * The default size of the population for calculating a route
     */
    int population_size;

    /**
     * The default number of generations that the population should be bred
     */
    int num_generations;

    /**
     * The toggle for using the database
     */
    int use_db;

    /**
     * This value is used to help calculate the initial step size of the population.
     * and the Z sigma is the max elevation delta / INITIAL_SIGMA_DIVISOR.
     */
    float initial_sigma_divisor;

    /**
     * This serves as the initial value for the X and Y of all the points for sigma.
     * We use 5km as a pretty tight bounding around the straight line (initial mean)
     */
    float initial_sigma_xy;

    /**
     * Represents the dampening parameter for the step size. This value should be close to 1
     */
    float step_dampening;

    /**
     * The interval multiplier for the square root of _genome_size * 3 for the indicator function.
     */
    float alpha;

    /**
     * The number of threads that are used to sample from the multivariate normal distribution
     */
    int num_sample_threads;

    /**
     * The number of divisions that the route is split up into for evaluation on the GPU
     */
    int num_route_workers;

    /**
     * The constant that the track cost is multiplied by in the cost function
     */
    float track_weight;

    /**
     * The constant that the curve cost is multiplied by in the cost function
     */
    float curve_weight;

    /**
     * The constant that the grade cost is multiplied by in the cost function
     */
    float grade_weight;

    /**
     * The constant that the length cost is mulitplied by in the cost function
     */
    float length_weight;

};

class Configure {

public:

    /**
     * Construct the configuration for this run.
     */
    Configure();

    /**
     * Gets the population size
     *
     * @return
     * returns the population size
     *
     */
    int getPopulationSize();

    /**
     * Gets the number of generations
     *
     * @return
     * returns the number of generations
     */
    int getNumGenerations();

    /**
     * Gets the toggle for using the database
     *
     * @return
     * returns the toggle for the database
     */
    bool getUseDb();

    /**
     * Gets the initial Sigma Divisor for calculating the step size
     *
     * @return
     * returns the initial sigma divisor
     */
    float getInitialSigmaDivisor();

    /**
     * Gets the initial sigma for calculating the initial X and Y values for sigma
     *
     * @return
     * returns the initial sigma xy
     */
    float getInitialSigmaXY();

    /**
     * Gets the dampening parameter for step size
     *
     * @return
     * returns the step dampening parameter
     */
    static float getStepDampening();

    /**
     * Gets the interval multiplier for the indicator function
     *
     * @return
     * returns alpha
     */
    float getAlpha();

    /**
     * Gets the number of threads for sampling the multivariate normal distribution
     *
     * @return
     * returns the number of sample threads
     */
    int getNumSampleThreads();

    /**
     * Gets the number of workers for GPU eval
     *
     * @return
     * returns the number of route workers
     */
    int getNumRouteWorkers();

    /**
     * Gets the constant to multiply the track cost by
     *
     * @return
     * The track weight
     */
    float getTrackWeight();

    /**
     * Gets the constant to multiply the curve cost by
     *
     * @return
     * The curve weight
     */
    float getCurveWeight();

    /**
     * Gets the constant to multiply the grade cost by
     *
     * @return
     * The grade weight
     */
    float getGradeWeight();

    /**
     * Gets the constant to multiply the length cost by
     *
     * @return
     * The length weight
     */
    float getLengthWeight();

private:

    /**
     * Loads the params json file
     */
    void loadConfig();

    /**
     * The loaded configuration
     */
    static Configuration _config;

};


