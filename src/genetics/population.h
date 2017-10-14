//
// Created by Logan on 10/12/17.
//

#ifndef ROUTES_POPULATION_H
#define ROUTES_POPULATION_H

#include <glm/glm.hpp>
#include "../elevation/elevation.h"

/** */

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
    glm::vec4& header;

    /**
     * The pointer to the genome of the individual. This is an array of glm::vec4 with length equal
     * to num_genes.
     */
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
 * individual[1] to individual[gene_length + 1] = glm::vec4
 * These are the genes, they represent X, Y and Z of the bezier control points.
 * W is unused, however it is needed for OpenCL.
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
         * @param genome_size
         * The number of points that each individual should have in its genome.
         *
         * @param data
         * The elevation data that this population is path-finding on
         */
        Population(size_t pop_size, size_t genome_size, const ElevationData& data);

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
         * Breeds the top 20% of the individuals together to generate a new, more fit population. Only 80%
         * of the population is bred, 20% if randomly generated to avoid stagnation.
         */
        void breedIndividuals();

    private:

        /**
         * Generates the initial population using the parameters that were passed in via the constructor.
         * This is done on the GPU, so the after this is called the CPU does not receive the data back.
         */
        void generatePopulation();

        /** The number of individuals that should be in this population */
        size_t _pop_size;

        /** The number of "genes" (bezier curve control points) that each individual should have.
        size_t _genome_size;

        /** The CPU storage of the individuals.*/
        std::vector<glm::vec4> _individuals;

        /** The GPU uploaded version of the individual data */
        boost::compute::vector<glm::vec4> _opencl_individuals;

        /**
         * The reference to the elevation data that this population operates on.
         * Its stored as const because nothing should ever be done to the data except reading.
         */
        const ElevationData& _data;


};

#endif //ROUTES_POPULATION_H
