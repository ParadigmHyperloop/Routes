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

// Ensure that E is defined on Windows
#ifndef M_E
    #define M_E 2.71828182845904523536f
#endif

/** */

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
 * This value is used to help calculate the initial step size of the population.
 * and the Z sigma is the max elevation delta / INITIAL_SIGMA_DIVISOR.
 */
#define INITIAL_SIGMA_DIVISOR 20.0f

/**
 * This serves as the initial value for the X and Y of all the points for sigma.
 * We use 5km as a pretty tight bounding around the straight line (initial mean)
 */
#define INITIAL_SIGMA_XY 2500.0f

/** Represents the dampening parameter for the step size. This value should be close to 1 */
#define STEP_DAMPENING 1.5f

/** The interval multiplier for the square root of _genome_size * 3 for the indicator function. */
#define ALPHA 1.5f

/** The number of threads that are used to sample from the multivariate normal distribution */
#define NUM_SAMPLE_THREADS 10

/** The number of divisions that the route is split up into for evaluation on the GPU */
#define NUM_ROUTE_WORKERS 100

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
    
    /** The individual's index in the _individuals vector */
    int index;

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

        /** Simple destructor to delete heap allocated things */
        ~Population();

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
         * Simulates the population for an entire generation. The steps are as follows:
         * - Rank the current population
         * - Update the CMA-ES params
         * - Sample a new population
         *
         * @param pod
         * The pod object containing the specs of the pod. Right now just uses max speed.
         */
        void step(const Pod& pod);

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
    
        /**
         * This returns the computed solution to the route (the mean).
         *
         * @return
         * The control points of the completed route containing the start and destination.
         */
        std::vector<glm::vec3> getSolution() const;
    
    private:

         /**
          * This function calculates the size of the genome.
          * This is based off the length of the route that this population represent and has a square root relationship.
          */
        void calcGenomeSize();

        /**
         * This function initializes the parameters for CMA-ES.
         * Parameters include the covariance matrix, the mean, the evolution path and the step size.
         */
        void initParams();

        /**
         * Becasue sampling from the multivariate normal distribution is so slow, we use some multithreading tricks to speed it up.
         * Firstly, since all MVND's require samples from the standard normal distribution we create objects that generate these samples in another thread.
         * Secondly we split the population up into NUM_SAMPLE_THREADS so that we can sample multiple individuals concurrently.
         * This function creates the prerequisites for this.
         */
        void initSamplers();

        /**
         * The samples vector is created once here. All samples that are done will overwrite the last ones to avoid memory copying.
         * All elements in the vector are also initialized here.
         */
        void initSamples();

        /**
         * When CMA-ES first starts, the mean is supposed to be the "best guess" for the desired result. In our case, our best is a straight
         * line from the starting position to the destination.
         * The mean vector generated by this function is a ordered distribution of linear steps toward the destination.
         */
        void bestGuess();

        /**
         * When we recalculate the mean vector from the best solutions, we do a weighting.
         * This function calculates the weights for mew (the selected population) once, as it remains constant.
         */
        void calcWeights();

        /**
         * Sigma represents how spread out the distribution will be, in other words the step size.
         * Because the sample space in Z is smaller than X and Y, sigma should reflect that initially
         */
        void calcInitialSigma();

        /** This function calculates some of the evolutionary parameters that remain constant */
        void calculateStratParameters();

        /**
         * This samples an entirely new population. We use the calculate / starting covariance matrix, the best solution (m) and the step size.
         * A multivariate normal distribution is temporarily constructed from these parameters and then the population is sampled from it.
         */
        void samplePopulation();

        /**
         * This function updates the parameters of the distribution including the covariance matrix and mean vector based on the current population.
         * We use "," selection, meaning we only sample from the current generation.
         */
        void updateParams();

        /**
         * This function re-calculates the mean vector of the population.
         * The only individuals that are used in this calculation are the _mu most fit ones.
         * Each individual is weighted by its corresponding _weights value based on its index.
         */
        void updateMean();

        /**
         * This function updates the evolutionary path for sigma.
         * We do this by using the covariance matrix and some evolutionary stratagy params.
         */
        void updatePSigma();

        /**
         * This function updates the evolutionary path for the covariance matrix.
         * We do this by using mostly the same factors as the sigma path but we also use the indicator function.
         */
        void updatePCovar();

        /**
         * Updates the covariance matrix for the population using path length control. This adapts the relationship between
         * each of the points to converge on the best solution.
         */
        void updateCovar();

        /**
         * Updates the step size for the population using path length control. This adapts the step size to quickly converge on a
         * solution.
         */
        void updateSigma();

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

        /** The origin of the data in meters relative to the database absolute origin */
        glm::vec2 _data_size;
    
        /** The size of the data in meters */
        glm::vec2 _data_origin;
    
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

        /** The number of individuals that are selected from each generation that are the best solutions */
        int _mu;

        /** This is the sum of 1/_weights^2 */
        float _mu_weight;
    
        /** The square root of _mu_weight */
        float _mu_weight_sqrt;
    
        /** The expectedc value of the standard normal distribution. We use this to determine if we should shrink or grow sigma */
        float _expected_value;

        /**
         * This represents the current mean vector of the population. In other words, this is the favorite solution the the population.
         * The vector is legnth _genome_size * 3, 3 components for each point in the bezier curve (X, Y, Z).
         */
        Eigen::VectorXf _mean;

        /** The difference between the new mean and the last mean divided by the step size */
        Eigen::VectorXf _mean_displacement;

        /**
         * The covariance matrix of the population. This contains the information about how the multivariate normal distribution is shaped.
         * This is a _genome_size * 3 X _genome_size * 3 matrix, 3 components for each point in the bezier curve (X, Y, Z).
         */
        Eigen::MatrixXf _covar_matrix;

        /**
         * The current step size. This is how far the next generation will move.
         * The vector is length _genome_size * 3, 3 components for each point in the bezier curve (X, Y, Z).
         */
        Eigen::VectorXf _sigma;

        /**
         * When we recalculate the mean vector from the best solutions, we do a weighting. This vector stores these weights where
         * _weights[0] is the highest weight, meant for the best solution.
         */
        std::vector<float> _weights;

        /**
         * The evolution path for the covariance matrix. This is used to adapt the covariance matrix each step
         * so that successful search steps vary closer to the favorable solution.
         */
        Eigen::VectorXf _p_covar;

        /**
         * The evolution path for the step size (sigma). This is used to adapt the step size each step
         * so that successful do not prematurely converge.
         */
        Eigen::VectorXf _p_sigma;

        /** The inverse of _c_sigma is the the backward time horizon for the evolution path for the step size. */
        float _c_sigma;

        /** The inverse of _c_covar is the the backward time horizon for the evolution path for the covariance matrix. */
        float _c_covar;

        /** The learning rate for the rank 1 update of the the covariance matrix */
        float _c1;

        /** The learning rate for the rank mu update of the covariance matrix */
        float _c_mu;

        /**
         * The samples from the distribution minus the mean. We separate this from the completed, evalulatable population to decrease
         * the chance of numerical error because we are dealing with large numbers.
         */
        std::vector<Eigen::VectorXf> _samples;

        /** Several multithreaded standard normal sample generator to speed up population sampling */
        std::vector<SampleGenerator*> _sample_gens;
    
        /** The _mu best multinormal samples from the population */
        std::vector<Eigen::VectorXf> _best_samples;
    
        /** The vector that is used to sort the individuals. We keep this around to avoid allocating every time we sort */
        std::vector<Individual> _sorted_individuals;
    
        /**
         * This vector contains the best fitness values of each generation. Every time step() is called, the value of the cost function
         * for the fittest individual is saved
         */
        std::vector<float> _fitness_over_generations;
    
};

#endif //ROUTES_POPULATION_H
