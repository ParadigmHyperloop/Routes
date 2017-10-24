#include <vector>
#include "genetics/genetics.h"
#include "elevation/elevation-stitch.h"
#include "html/html.h"

int main() {

    ElevationData data = ElevationStitch::stitch("../data/imgn35w119_1.img", "../data/imgn35w118_1.img");

    // Test elevation object
//    ElevationData data = ElevationData("../data/imgn35w119_1.img");
//    std::cout << "Min: " << data.getMinElevation() << " Max:" << data.getMaxElevation() << std::endl;
//    std::cout << data.convertPixelsToMeters(glm::ivec2(data.getWidth(), 0.0)).x << std::endl;
//
    Pod pod = Pod(339.0);
    Population pop = Population(200, 10, glm::vec4(0.0, 0.0, 550.0, 0.0), glm::vec4(170000.0, 80000.0, 1300.0, 0.0), data);

    std::vector<glm::vec3> computed = Genetics::solve(pop, pod, 400);

    std::cout << computed[0].x << std::endl;

    // Write out some html
    HTML::generateHTMLForPath(computed, data);

    return 0;
}