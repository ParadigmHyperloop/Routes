//
// Created by Logan on 10/12/17.
//

#include "genetics.h"

std::vector<glm::vec3> Genetics::solve(Population& pop, Pod& pod, int generations) {

    // Run the simulation for then given amount of generations
    for (int i = 0; i < generations; i++) {
        
        if (i % 20 == 0)
            std::cout << "Finished gen " << i << std::endl;
        
        pop.step(pod);
        
        
    }

    // Sort one last time
    pop.evaluateCost(pod);
    pop.sortIndividuals();

    // Format a path so we can print it in python
    Individual ind = pop.getIndividual(0);

    // Transfer the bath over
    std::vector<glm::vec3> out_path = std::vector<glm::vec3>(ind.num_genes + 2);
    for (int i = 0; i < ind.num_genes + 2; i++)
        out_path[i] = glm::vec3(ind.path[i].x, ind.path[i].y, ind.path[i].z);

    return out_path;

}
