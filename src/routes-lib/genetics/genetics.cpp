//
// Created by Logan on 10/12/17.
//

#include "genetics.h"

std::vector<glm::vec3> Genetics::solve(Population& pop, Pod& pod, int generations) {

    // Run the simulation for then given amount of generations
    for (int i = 0; i < generations; i++) {
        
        if (i % 20 == 0 && i)
            std::cout << "Finished gen " << i << std::endl;
        
        pop.step(pod);
        
        Individual ind = pop.getIndividual(0);
        if (!ind.header->x)
            break;
        
    }

    // Transfer the bath over
    return pop.getSolution();

}
