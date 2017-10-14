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
        //pop.sortIndividuals();

        // Print out the cost of the most fit
        std::cout << "Most fit cost: " << pop.getIndividual(0).header->x << std::endl;

        // Generate a new population with the most fit being the mothers and father
        //pop.breedIndividuals();

    }

}