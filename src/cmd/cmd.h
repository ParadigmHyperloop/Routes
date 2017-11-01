//
// Created by Logan on 10/29/17.
//

#ifndef ROUTES_CMD_H
#define ROUTES_CMD_H

#include <boost/program_options.hpp>
#include <glm/glm.hpp>
#include <iostream>

/** A enum describing the various states that the program can be in based on arguments from the command line */
enum RoutesState {

    /** The state when the program should rebuild the data database */
    Rebuilding,

    /** The state when the program is calculating a route */
    Calculating

};

/** A simple class for processing the command line arguments */
class CMD {

    public:

        /**
         * This function takes in the arguments from the command line. It handles printing out the help message
         * as well as figuring out what state the program should be in.
         *
         * @param argc
         * The argument count, taken from the main function.
         *
         * @param argv
         * The value of the arguments, also taken from the main function.
         */
        static void parseArguments(int argc, const char* argv[]);

        /** Gets the state of the program based on the arguments on the command line */
        static RoutesState getState();

        /** The starting position of the route taken in from the command line */
        static glm::vec3 start;

         /** The ending position of the route taken in from the command line */
        static glm::vec3 dest;

    private:

        /** The state of the program according to the arguments passed in on the command line */
        static RoutesState _state;

};

#endif //ROUTES_CMD_H
