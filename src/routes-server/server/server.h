//
// Created by Logan on 11/3/17.
//

#ifndef ROUTES_SERVER_H
#define ROUTES_SERVER_H

#include <map>
#include <thread>
#include <memory>
#include <functional>

#include <restbed>

#include "../queue/queue.h"

/**
 * This class sets up a HTTP (not HTTPS!) REST server for computing routes.
 *
 * Acceptable requests:
 * GET server_addr/compute?start=lat,lon,h&dest=lat,lon,h where lat and lon are latitude and longitude respectively
 * and h is the elevation above sea level in meters. This returns a unique identifier that needs to be saved for
 * later to get the finished route.
 *
 * GET server_addr/retrieve?id=unique where unique is the identifier returned by compute/ This will either return
 * the computed route's control points or false
 */
class RoutesServer {

    public:


        /** Starts the server listening for route calculation requests.
         *
         * @param port
         * The port that the server should listen on. Since this is not a regular HTTP server this should not really be
         * 80 so that it can run on the same machine that the frontend is hosted on.
         */
        static void startServer(int port);

    private:

        /**
         * This function is responsible for handling the OPTIONS call. This is essentially what enables CORS
         * for the api.This function shouldn't be called from anywhere, restbed calls it.
         *
         * @param session
         * The session input from restbed.
         */
        static void handleCORS(const std::shared_ptr<restbed::Session> session);

        /**
         * This function handles the GET request for computation of the route. This function shouldn't be called from
         * anywhere, restbed calls it.
         *
         * @param session
         * The session input from restbed.
         *
         */
        static void handleCompute(const std::shared_ptr<restbed::Session> session);

        /**
         * This function handles the GET request for retrieval of the route. This function shouldn't be called from
         * anywhere, restbed calls it.
         *
         * @param session
         * The session input from restbed.
         *
         */
        static void handleRetrieval(const std::shared_ptr <restbed::Session> session);

        /**
         * Since the restbed server is blocking, this is called by the server to give a thread that can continue
         * to do things when the server is running. This should never be called by anything but restbed.
         *
         * @param service
         * The restbed service.
         */
        static void onServerReady(restbed::Service &service);

        /** A thread to perform route calculation on. */
        static std::thread calculation_thread;

};

#endif //ROUTES_SERVER_H
