//
// Created by logan on 11/3/2017.
//

#include <routes.h>
#include <stdlib.h>
#include "cmd/cmd.h"

int main(int argc, const char* argv[]) {

    CMD::parseArguments(argc, argv);

    switch (CMD::getState()) {

        case Calculating: {

            std::cout << "Computing route from " << glm::to_string(CMD::start) << " to " << glm::to_string(CMD::dest) <<
                      std::endl;

            // Calculate the route
            Routes::calculateRoute(CMD::start,
                                   CMD::dest);

        } break;

        case Rebuilding:

            std::cout << "Rebuilding database\n";

            // Run a command to generate the vtf
            system("gdalbuildvrt ../data/db.vtf ../data/*.img");
            
            // Generate the stats for it
            system("gdalinfo ../data/db.vtf -stats");

            break;

    }

    return 0;
}
