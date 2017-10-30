//
// Created by Logan on 10/29/17.
//

#include "cmd.h"

RoutesState CMD::_state = Calculating;

void CMD::parseArguments(int argc, const char* argv[]) {

    // Since this function is only supposed to be called once, we can get away with creating stuff in here that
    // Really should be created as a static member
    boost::program_options::options_description description("Allowed Args");
    description.add_options()
            ("help", "Prints the help message")
            ("rebuild", "Rebuilds the database from the contents of the data folder");

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

}

RoutesState CMD::getState() { return  _state; }