//
// Created by logan on 11/3/2017.
//

#include <boost/filesystem.hpp>
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

            #ifdef BOOST_WINDOWS

                std::string build_cmd = "cmd /S /C gdalbuildvrt ../data/db.vtf ";

                // Open up the directory in the boost filesystem
                boost::filesystem::path data_path = boost::filesystem::path("../data/");

                for(auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(data_path), {})) {

                    // Make sure that this file is a .img file
                    std::string path_string = entry.path().string();

                    if (path_string.length() > 3 && !path_string.substr(path_string.length() - 4, 4).compare(".img"))
                        build_cmd += path_string + " ";

                }

                // Run a command to generate the vtf
                system(build_cmd.c_str());

                // Generate the stats for it
                system("cmd /S /C gdalinfo ../data/db.vtf -stats");

            #else

                // Run a command to generate the vtf
                system("gdalbuildvrt ../data/db.vtf ../data/*.img");

                // Generate the stats for it
                system("gdalinfo ../data/db.vtf -stats");

            #endif

            break;

    }

    return 0;
}
