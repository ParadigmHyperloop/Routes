#include "routes/routes.h"

int main(int argc, const char* argv[]) {

    CMD::parseArguments(argc, argv);

    switch (CMD::getState()) {

        case Calculating: {

            // Load the databse
            DB::load();

            // Calculate the route
            Routes::calculateRoute(glm::vec3(-118.873193359375, 34.8047829195724, 550.0),
                                   glm::vec3(-118.23173828125, 34.08877925439021, 550.0));

        } break;

        case Rebuilding:

            std::cout << "Rebuilding database\n";
            DB::build();

            break;

    }

    return 0;
}