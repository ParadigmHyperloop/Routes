#include <vector>
#include "genetics/genetics.h"


int main() {

    // Test elevation object
    ElevationData* data = new ElevationData("../data/imgn35w119_1.img");
    std::cout << "Min: " << data->getMinElevation() << " Max:" << data->getMaxElevation() << std::endl;
    std::cout << data->convertPixelsToMeters(glm::ivec2(data->getWidth(), 0.0)).x << std::endl;

    Pod pod = Pod(339.0);
    Population pop = Population(200, 6, *data);

    std::vector<glm::vec3> computed = Genetics::solve(pop, pod, glm::vec4(0.0, 0.0, 550.0, 0.0), glm::vec4(70000.0, 80000.0, 1300.0, 0.0), 3000);

    delete data;

    return 0;
}