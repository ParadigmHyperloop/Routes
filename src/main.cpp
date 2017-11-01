#include "routes/routes.h"

int main(int argc, const char* argv[]) {

    CMD::parseArguments(argc, argv);

    switch (CMD::getState()) {

        case Calculating: {

            // Load the databse
            DB::load();

            std::cout << "Computing route from " << glm::to_string(CMD::start) << " to " << glm::to_string(CMD::dest) <<
                      std::endl;

            // Calculate the route
            Routes::calculateRoute(CMD::start,
                                   CMD::dest);

        } break;

        case Rebuilding:

            std::cout << "Rebuilding database\n";
            DB::build();

            break;

    }

    return 0;
}