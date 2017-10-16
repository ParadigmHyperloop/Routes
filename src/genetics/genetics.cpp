//
// Created by Logan on 10/12/17.
//

#include "genetics.h"

std::vector<glm::vec3> Genetics::solve(Population& pop, Pod& pod, glm::vec4 start, glm::vec4 dest, int generations) {

    // Run the simulation for then given amount of generations
    for (int i = 0; i < generations; i++) {

        // Sort the individuals to determine who is most fit
        pop.evaluateCost(start, dest, pod);

        // Sort the individuals by cost
        pop.sortIndividuals();

        if (i % 20 == 0) {

            Individual ind = pop.getIndividual(0);
            std::cout << "Finished generation " << i << " Fittest: " << ind.header->x << std::endl;

        }

        // Generate a new population with the most fit being the mothers and father
        pop.breedIndividuals();

    }

    // Sort one last time
    pop.sortIndividuals();

    // Format a path so we can print it in python
    Individual ind = pop.getIndividual(0);
    std::cout << "[[" << start.x << ", " << start.y << "], ";

    for (int i = 0; i < ind.num_genes; i++)
        std::cout << "[" << ind.genome[i].x << ", " << ind.genome[i].y << "], ";

    std::cout << "[" << dest.x << ", " << dest.y << "]]\n";

    // Transfer the bath over
    std::vector<glm::vec3> out_path = std::vector<glm::vec3>(ind.num_genes);
    for (int i = 0; i < ind.num_genes; i++)
        out_path[i] = glm::vec3(ind.genome[i].x, ind.genome[i].y, ind.genome[i].z);

    return out_path;

}