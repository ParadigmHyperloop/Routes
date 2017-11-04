//
// Created by Logan on 11/3/17.
//

#include "server/server.h"


int main(const int argc, const char* argv[]) {

    // Load the databse
    DB::load();

    RoutesServer::startServer(8080);

}