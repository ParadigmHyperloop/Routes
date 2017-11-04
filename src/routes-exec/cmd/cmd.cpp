//
// Created by Logan on 10/29/17.
//

#include "cmd.h"

RoutesState CMD::_state = Calculating;

glm::vec3 CMD::start = glm::vec3(0.0);
glm::vec3 CMD::dest = glm::vec3(0.0);

void CMD::parseArguments(int argc, const char* argv[]) {

    // Since this function is only supposed to be called once, we can get away with creating stuff in here that
    // Really should be created as a static member
    boost::program_options::options_description description("Allowed Args");
    description.add_options()
            ("help", "Prints the help message")
            ("rebuild", "Rebuilds the database from the contents of the data folder")
            ("start", boost::program_options::value<std::string>(),
             "Example: start=X,Y,Z where X and Y are the longitude and latitude of the start of the route and Z is the elevation in meters")
            ("dest", boost::program_options::value<std::string>(),
             "Example: dest=X,Y,Z where X and Y are the longitude and latitude of the destination of the route and Z is the elevation in meters")
            ;

    boost::program_options::variables_map var_map;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, description), var_map);
    boost::program_options::notify(var_map);

    // Check if the help message was requested. If it was, print it and then exit
    if (var_map.count("help")) {

        std::cout << description << std::endl;
        exit(0);

    }

    // Determine what state the program is in
    if (var_map.count("rebuild"))
        _state = Rebuilding;

    // If the state is calculating, make sure that the start and dest exsit
    if (_state == Calculating) {

        if (var_map.count("start") && var_map.count("dest")) {

            // Parse the command line arguments
            std::string start_string = var_map["start"].as<std::string>();
            std::string dest_string  = var_map["dest"].as<std::string>();

            sscanf(start_string.c_str(), "%f,%f,%f", &start.x, &start.y, &start.z);
            sscanf(dest_string.c_str(),  "%f,%f,%f", &dest.x, &dest.y, &dest.z);

        } else {

            // Tell the user there was an error
            std::cout << "Missing a start and destination via the command line arguments\n";
            exit(0);

        }

    }

}

RoutesState CMD::getState() { return  _state; }