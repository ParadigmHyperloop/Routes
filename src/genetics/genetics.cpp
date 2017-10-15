//
// Created by Logan on 10/12/17.
//

#include "genetics.h"

void Genetics::solve(Population& pop, Pod& pod, glm::vec4 start, glm::vec4 dest, int generations) {

    // Run the simulation for then given amount of generations
    for (int i = 0; i < generations; i++) {

        // Sort the individuals to determine who is most fit
        pop.evaluateCost(start, dest, pod);

        // Sort the individuals by cost
        pop.sortIndividuals();

        // Generate a new population with the most fit being the mothers and father
        pop.breedIndividuals();

        if (i % 20 == 0)
            std::cout << "Finished generation " << i << std::endl;

    }

    // Sort one last time
    pop.sortIndividuals();

    // Format a path so we can print it in python
    Individual ind = pop.getIndividual(0);
    std::cout << "[[" << start.x << ", " << start.y << "], ";

    for (int i = 0; i < ind.num_genes; i++)
        std::cout << "[" << ind.genome[i].x << ", " << ind.genome[i].y << "], ";

    std::cout << "[" << dest.x << ", " << dest.y << "]]";

}