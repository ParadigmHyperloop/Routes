//
// Created by Logan on 10/12/17.
//

#ifndef ROUTES_POPULATION_H
#define ROUTES_POPULATION_H

#include <chrono>
#include <boost/compute/container/vector.hpp>
#include <random>
#include <time.h>

#include "../bezier/bezier.h"
#include "../elevation/elevation.h"
#include "../normal/multinormal.h"
#include "../pod/pod.h"


/** */

/**
 * The max distance above or below the max and min of elevation data a population is allowed
 * to generate a track at.
 */
#define TRACK_ABOVE_BELOW_EXTREMA 10.0f

/**
 * Since we assume that paths are relatively close to straight lines, we decide on a maximum deviation from a straight
 * line that a path can initially generate from. This number represents the max deviation in meters.
 */
#define MAX_STRAIGHT_DEVIATION 15000.0f

/**
 * In order to get the number of points that a particular path should be evaluated along, we run a converstion.
 * This factor converts meters to number of points of evaluation.
 */
#define METERS_TO_POINT_CONVERSION 30.1867567568f

/**
 * To calculate the length of the genome we take the square root of the length of the route and then multiply it
 * by a constant. This constant is small so we get few points for a lot of meters.
 */
#define LENGTH_TO_GENOME 0.0274360619f

/**
 * Individual is a convenience so that individuals can be treated as units rather than
 * as a single float vector, which is how they are stored.
 * Individual is not used to store any new data, but rather reference other data.
 */
struct Individual {

    /**
     * A reference to this individual's header.
     * The header contains the computed cost of the individual, so long as it has been downloaded from the GPU.
     * Cost is stored in the X component and Y, Z and W are currently unused.
     */
    glm::vec4* header;

    /**
     * The pointer to the genome of the individual. This is an array of glm::vec4 with length equal
     * to num_genes.
     */
    glm::vec4* path;

    /** The pointer to the full path of the individual. This contains the start, the genome and the destination. */
    glm::vec4* genome;

    /**
     * The length of the genome (num glm::vec4's).
     * All individuals in the same population have the same number of genes.
     */
    size_t num_genes;

};

/**
 * This class is responsible for holding individuals and breeding them.
 * The current representation of an individual is as follows:
 *
 * individual[0] = glm::vec4
 * This vector stores the cost of the individual in the X component. Other components are blank
 * but may be used in the future.
 *
 * individual[1] = glm::vec4
 * This stores the start point of the individual for more simple computation on the GPU. This is constant for
 * every individual in the population.
 *
 * individual[2] to individual[gene_length + 1] = glm::vec4
 * These are the genes, they represent X, Y and Z of the bezier control points.
 * W is unused, however it is needed for OpenCL.
 *
 * individual[gene_length + 2] = glm::vec4
 * This stores the end point of the individual for more simple computation on the GPU. This is constant for
 * every individual in the population.
 *
 * Population stores all of the individuals in one std::vector<glm::vec4> for efficient uploading to the GPU.
 * A single individual can be accessed with the Individual struct convenience.
 */
class Population {

    public:

        /**
         * Generates a new population with the given parameters. Since the possible locations for the control
         * points of the population depend on the extents of the data, it is passed in here.
         *
         * @param pop_size
         * The amount of individuals that should be generated.
         *
         * @param start
         * The start location of the path. X, Y and Z are measured in meters.
         *
         * @param dest
         * The destination of the path. X, Y and Z are measured in meters.
         *
         * @param data
         * The elevation data that this population is path-finding on
         */
        Population(int pop_size, glm::vec4 start, glm::vec4 dest, const ElevationData& data);

        /**
         * Retrieves an individual from the population. This assumes that the population data
         * has already been copied back to the CPU.
         *
         * @param index
         * The index of the individual in the population that is desired.
         *
         * @return
         * An Individual struct; a convenience reference to the data in _individuals.
         */
        Individual getIndividual(int index);

        /**
         * This function sorts the individuals in ascending order based on the cost. _individuals[0] becomes
         * the most fit individual. Currently this is done on the CPU, which is not optimal, but it is simpler.
         */
        void sortIndividuals();

        /**
         * This function is what makes the genetic algorithm work.
         * For every individual a cost is evaluated. This represents how good their genome is as a solution.
         * This function performs this, using OpenCL.
         *
         * @param pod
         * The pod object containing the specs of the pod. Right now just uses max speed.
         *
         */
        void evaluateCost(const Pod& pod);

    private:

         /**
          * This function calculates the size of the genome.
          * This is based off the length of the route that this population represent and has a square root relationship.
          */
        void calcGenomeSize();

        /**
         * Generates the initial population using the parameters that were passed in via the constructor.
         * This is done on the CPU, but hopefully can be done on the GPU eventually.
         */
        void generatePopulation();
    
        /**
         * To evaluate the bezier curve, binomial coefficients are required.
         * These need factorials, so it would be slow to compute them. Instead we do it once, offline because
         * all paths have the same degree. This computes those coefficients.
         */
        void calcBinomialCoefficients();
    
        /** The number of individuals that should be in this population */
        int _pop_size;

        /** The number of "genes" (bezier curve control points) that each individual should have. */
        int _genome_size;

        /**
         * This number represents the size of glm::vec4s that an individual is.
         * This includes the genome size, the header and the start and destination
         */
        int _individual_size;

        /** The starting position of the path that this population is trying to "solve" */
        glm::vec4 _start;

        /** The ending position of the path that this population is trying to "solve" */
        glm::vec4 _dest;

        /**
         * The direction vector of the path that this population is built for.
         * Measured in meters.
         */
        glm::vec4 _direction;

        /**
         * Since the cost algorithm uses sampling we need to know how many samples to take. For one dataset this is a
         * trivial task. However some routes may span multiple datasets and therefore a calculation needs to be
         * performed to figure out how may samples should be taken.
         */
        int _num_evaluation_points;

        /** _num_evaluation_points - 1. This is a float because it is used for division in the cost function */
        float _num_evaluation_points_1;

        /** The CPU storage of the individuals.*/
        std::vector<glm::vec4> _individuals;

        /** The GPU uploaded version of the individual data */
        boost::compute::vector<glm::vec4> _opencl_individuals;

        /**
         * To evaluate the bezier curve, binomial coefficients are required.
         * These need factorials, so it would be slow to compute them. Instead we do it once, offline because
         * all paths have the same degree.
         * This stores the compute coefficients on the GPU.
         */
        boost::compute::vector<int> _opencl_binomials;

        /**
         * The reference to the elevation data that this population operates on.
         * Its stored as const because nothing should ever be done to the data except reading.
         */
        const ElevationData& _data;
    
/************************************************************************************************************************************************/
    
        /**
         * This represents the current mean vector of the populatio. In other words, this is the favorite solution the the population.
         * The vector is legnth _genome_size * 3, 3 components for each point in the bezier curve (X, Y, Z).
         */
        Eigen::VectorXf _mean;
    
        /**
         * The covariance matrix of the population. This contains the information about how the multivariate normal distribution is shaped.
         * This is a _genome_size * 3 X _genome_size * 3 matrix, 3 components for each point in the bezier curve (X, Y, Z).
         */
        Eigen::MatrixXf _covar_matrix;
    
        /**
         * The current step size. This is how far the next generation will move.
         * The vector is legnth _genome_size * 3, 3 components for each point in the bezier curve (X, Y, Z).
         */
        Eigen::VectorXf _sigma;

};

#endif //ROUTES_POPULATION_H
