//
// Created by Logan on 10/12/17.
//

#include "genetics.h"

std::vector<glm::vec3> Genetics::solve(Population& pop, Pod& pod, int generations) {

    // Run the simulation for then given amount of generations
    for (int i = 0; i < generations; i++) {

        // Sort the individuals to determine who is most fit
        pop.evaluateCost(pod);

        // Sort the individuals by cost
        pop.sortIndividuals();

        if (i % 20 == 0) {

            Individual ind = pop.getIndividual(4);
            std::cout << "Finished generation " << i << " Fittest: " << ind.header->x << std::endl;

//            std::cout << "[";
//            for (int i = 0; i < ind.num_genes + 2; i++) {
//
//                glm::vec4& vec = ind.path[i];
//                std::cout << "[" << vec.x << "," << vec.y << "],";
//
//            }
//
//            std::cout << "]\n";

        }

        // Generate a new population with the most fit being the mothers and father
        pop.breedIndividuals();

    }

    // Sort one last time
    pop.sortIndividuals();

    // Format a path so we can print it in python
    Individual ind = pop.getIndividual(0);

    // Transfer the bath over
    std::vector<glm::vec3> out_path = std::vector<glm::vec3>(ind.num_genes + 2);
    for (int i = 0; i < ind.num_genes + 2; i++)
        out_path[i] = glm::vec3(ind.path[i].x, ind.path[i].y, ind.path[i].z);

    return out_path;

}