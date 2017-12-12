//
// Created by Logan on 10/12/17.
//

#include "genetics.h"

std::vector<glm::vec3> Genetics::solve(Population& pop, Pod& pod, int generations) {

    // Run the simulation for then given amount of generations
    for (int i = 0; i < generations; i++) {

        pop.step();

        if (i % 20 == 0 && i != 0)
            std::cout << "Finished generation " << i << " Fittest: " << pop.getGlobalBest()[0].x << std::endl;

    }

    // Get the pointer to the best, making sure to skip the header
    glm::vec4* best = pop.getGlobalBest() + 1;

    // Transfer the bath over
    std::vector<glm::vec3> out_path = std::vector<glm::vec3>(pop.getGenomeSize() + 2);
    for (int i = 0; i < pop.getGenomeSize() + 2; i++)
        out_path[i] = glm::vec3(best[i].x, best[i].y, best[i].z);

    return out_path;

}
