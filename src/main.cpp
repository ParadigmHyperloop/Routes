#include <vector>
#include "genetics/genetics.h"
#include "elevation/elevation-stitch.h"
#include "html/html.h"
#include "cmd/cmd.h"
#include "db/db.h"

namespace po = boost::program_options;

int main(int argc, const char* argv[]) {

    CMD::parseArguments(argc, argv);

    switch (CMD::getState()) {

        case Calculating: {

            // Load the databse
            DB::load();

            std::vector<std::string> results = DB::getRequiredDatasets(glm::vec2(-120.0016667, 35.0016667),
                                                                       glm::vec2(-118.5000000, 34.5000000));

            for (int i = 0; i < results.size(); i++)
                std::cout << results[i] << std::endl;

            // Calculate a route
//            ElevationData data = ElevationStitch::stitch({"../data/imgn35w119_1.img", "../data/imgn35w118_1.img", "../data/imgn35w117_1.img"});
//
//            Pod pod = Pod(339.0);
//            Population pop = Population(200, 10, glm::vec4(0.0, 0.0, 550.0, 0.0), glm::vec4(350000.0, 80000.0, 1300.0, 0.0), data);
//
//            std::vector<glm::vec3> computed = Genetics::solve(pop, pod, 200);
//
//            std::cout << computed[0].x << std::endl;
//
//            // Write out some html
//            HTML::generateHTMLForPath(computed, data);

        } break;

        case Rebuilding:

            std::cout << "Rebuilding database\n";
            DB::build();

            break;

    }

    return 0;
}