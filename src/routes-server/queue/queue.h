//
// Created by Logan on 11/3/17.
//

#ifndef ROUTES_QUEUE_H
#define ROUTES_QUEUE_H

#include <boost/lockfree/queue.hpp>
#include <glm/glm.hpp>
#include <ctime>
#include <unordered_map>

#include <routes.h>
#include <bezier/bezier.h>

/** */

/**
 * This class is responsible for handling calculation of routes. It contains a queue of routes to be calculated.
 * Once the route is calculated it is placed into the completed map until it is retrieved.
 */
class RoutesQueue {

    public:

        /**
         * Queues a route to be calculated.
         *
         * @param start
         * The starting position of the route. X and Y are longitude and latitude respectively and Z is the elevation.
         * in meters above sea level.
         *
         * @param dest
         * The ending position of the route. X and Y are longitude and latitude respectively and Z is the elevation.
         * in meters above sea level.
         *
         * @return
         * A unique identifier that can be used to reference this route and check if it is complete.
         */
        static size_t queueRoute(const glm::vec3& start, const glm::vec3& dest);

        /**
         * A function that will calculate any routes that are in the queue. All routes that are calculated are then
         * moved to the completed map.
         */
        static void calculateRoutes();

        /**
         * Checks if the route has been calculated.
         *
         * @param id
         * The unique id of the route that was given by queueRoute.
         *
         * @return
         * A boolean that will be true if the route is completed and false if it was not.
         */
        static bool isRouteCompleted(size_t id);

        /**
         * Returns the evaluated points of a calculated path. This assumes that isRouteCompleted has been called so the
         * completed route exists.
         *
         * @param id
         * The unique id of the route that was given by queueRoute.
         *
         * @return
         * The control points of the calculated route. X and Y of the vectors are longitude and latitude respectively
         * and Z is the elevation.
         *
         */
        static std::vector<glm::vec3> getCompletedRoute(size_t id);


    private:

        /** A structure to store routes to be calculated */
        struct RouteItem {

            /** A unique identifier for this route */
            size_t id;

            /**
             * The starting position of the route. X and Y are longitude and latitude respectively and Z is the elevation
             * in meters above sea level.
             */
            glm::vec3 start;

            /**
             * The ending position of the route. X and Y are longitude and latitude respectively and Z is the elevation
             * in meters above sea level.
             */
            glm::vec3 dest;

        };

        /**
         * A queue (FIFO) of routes that need to be calculated.
         * We use Boost's lockfree queue so that we can read from the calculation thread and write from the
         * request handler thread.
         */
        static boost::lockfree::queue<RouteItem> _routes;

        /**
         * The map of the completed routes.
         * The key represents the unique identifier that was returned by the queueRoute function.
         * The value is the control points of the bezier curve of the route.
         * Unorded map is used because it fits the thread safety requirements.
         */
        static std::unordered_map<size_t, std::vector<glm::vec3>> _completed;


};


#endif //ROUTES_QUEUE_H
