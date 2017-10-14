//
// Created by Logan on 10/12/17.
//

#include "population.h"

Population::Population(int pop_size, int genome_size, const ElevationData& data) : _data(data) {

    // Save constants
    _pop_size = pop_size;
    _genome_size = genome_size;

    // Create the appropriate vectors
    _individuals = std::vector<glm::vec4>(pop_size * (genome_size + 1));
    _opencl_individuals =  boost::compute::vector<glm::vec4>(_individuals.size(), Kernel::getContext());

    // Make a dummy genome
    dummy_genome = new glm::vec4[genome_size];

    // Generate the population
    generatePopulation();

}

Population::~Population() { delete dummy_genome; }

Individual Population::getIndividual(int index) {

    Individual ind;
    ind.num_genes = _genome_size;

    // Calculate the location of the parts of the individual
    glm::vec4* header_loc = &_individuals[index * (_genome_size + 1)];
    ind.header = header_loc;
    ind.genome = header_loc + 1;


}

void Population::breedIndividuals() {

    // Get a random number of mother and fathers from the top 20%
    // Ensure there is 1 mother and 1 father every time though
    int mothers = glm::linearRand(1, (int)(_pop_size * 0.2));
    int fathers = glm::linearRand(1, (int)(_pop_size * 0.2));

    std::vector<glm::vec4> new_population = std::vector<glm::vec4>(_pop_size);

    // Breed 80% of the population from the mother and father
    int i;
    for (i = 0; i < (_pop_size * 0.8); i++) {

        glm::vec4* bred = crossoverIndividual(glm::linearRand(0, mothers), glm::linearRand(0, fathers));
        memcpy(&new_population[i * (_genome_size + 1) + 1], bred, sizeof(glm::vec4) * _genome_size);

    }

    // Generate new individuals (20%)
    for (int i = i; i < _pop_size; i++) {

        // Adds + 1 to ignore the header
        int individual_start = i * (_genome_size + 1) + 1;

        for (int g = 0; g < _genome_size; g++) {

            // Create a new random vector with
            glm::vec4 random_vec = glm::vec4(glm::linearRand(0.0, _data.getWidth()),
                                             glm::linearRand(0.0, _data.getHeight()),
                                             glm::linearRand(_data.getMinElevation(), _data.getMaxElevation()), 0.0);

            new_population[individual_start + g] = random_vec;

        }

    }

    // Save the new population
    _individuals = new_population;

    // Copy to GPU
    boost::compute::copy(_individuals.begin(), _individuals.end(), _opencl_individuals.begin(), Kernel::getQueue());

}

void Population::sortIndividuals() {

    // Get all of the individuals as Individuals for easier sorting
    std::vector<Individual> individuals_s = std::vector<Individual>(_pop_size);
    for (int i = 0; i < _pop_size; i++)
        individuals_s[i] = getIndividual(i);

    // Sort the array of individual structs
    std::sort(individuals_s.begin(), individuals_s.end(), [](Individual a, Individual b){

        // Compare costs in the header
        return (*a.header).x < (*b.header).x;

    });

    // Create a new vector so we don't destroy any data
    std::vector<glm::vec4> sorted_individuals = std::vector<glm::vec4>(_individuals.size());
    for (int i = 0; i < _pop_size; i++) {

        // Copy the sorted individual into the new array
        memcpy(&sorted_individuals[i * (_genome_size + 1)], individuals_s[i].header, sizeof(glm::vec4) * (_genome_size + 1));

    }

    // Save the sorted array
    _individuals = sorted_individuals;

}

void Population::generatePopulation() {

    // Go through each individual
    for (int i = 0; i < _pop_size; i++) {

        // Adds + 1 to ignore the header
        int individual_start = i * (_genome_size + 1) + 1;

        for (int g = 0; g < _genome_size; g++) {

            // Create a new random vector with
            glm::vec4 random_vec = glm::vec4(glm::linearRand(0.0, _data.getWidth()),
                                             glm::linearRand(0.0, _data.getHeight()),
                                             glm::linearRand(_data.getMinElevation(), _data.getMaxElevation()), 0.0);

            _individuals[individual_start + g] = random_vec;

        }

    }

    // Upload the population onto the GPU
    boost::compute::copy(_individuals.begin(), _individuals.end(), _opencl_individuals.begin(), Kernel::getQueue());

}

glm::vec4* Population::crossoverIndividual(int a, int b) {

    // Get memory location of a's genom
    glm::vec4* a_genome = &_individuals[a * (_genome_size + 1) + 1];
    glm::vec4* b_genome = &_individuals[b * (_genome_size + 1) + 1];

    // Cross over each gene
    for (int i = 0; i < _genome_size; i++) {

        // Get a random amount to cross the two by
        float random_mix = glm::linearRand(0.0, 1.0);
        dummy_genome[i] = glm::mix(*(a_genome + i), *(b_genome + i), random_mix);

    }

    return dummy_genome;

}