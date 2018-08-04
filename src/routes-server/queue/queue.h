//
// Created by Logan on 11/3/17.
//

#ifndef ROUTES_QUEUE_H
#define ROUTES_QUEUE_H

#include <boost/lockfree/queue.hpp>
#include <glm/glm.hpp>
#include <unordered_map>
#include <ctime>

#include <routes.h>
#include <bezier/bezier.h>

/** */

/**
 * This class is responsible for handling calculation of routes. It contains a queue of routes to be calculated.
 * Once the route is calculated it is placed into the completed map until it is retrieved.
 */
class RoutesQueue {

public:

    struct forJSON {

        /** The control points of the curve*/
        std::vector<glm::vec3> controls;

        /** The evaluated points of the curve*/
        std::vector<glm::vec3> evaluated;

        /** The time needed to traverse the route in seconds*/
        float time;

        /** The length of the track in meters*/
        float length;

        /** The elevations of the track. The x values are the current distance(m) on
         * the track and the y values are the elevations(m)
         */
        std::vector<glm::vec2> elevations;

        /** The elevations of the ground. The x values are the current distance(m) on
         * the track and the y values are the elevations(m)
         */
        std::vector<glm::vec2> ground_elevations;

        /**
         * The speed of the pod along the track. The x values are the current distance(m) on the
         * track and the y values are the speeds(m/s)
         */
        std::vector<glm::vec2> speeds;

        /**
         * The grade of the pod as it moves along the track. The x values are the current distance(m)
         * on the track and the y value are the grade.
         */
        std::vector<glm::vec2> grades;

    };

    /**
     * Queues a route to be calculated.
     *
     * @param start
     * The starting position of the route. X and Y are longitude and latitude respectively.
     *
     * @param dest
     * The ending position of the route. X and Y are longitude and latitude respectively.
     *
     * @return
     * A unique identifier that can be used to reference this route and check if it is complete.
     */
    static size_t queueRoute(const glm::vec2& start, const glm::vec2& dest);

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
     * Returns the control points of a calculated path. This assumes that isRouteCompleted has been called so the
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
    static forJSON getCompletedRoute(size_t id);




private:

    /** A structure to store routes to be calculated */
    struct _RouteItem {

        /** A unique identifier for this route */
        size_t id;

        /** The start latitude of the route */
        double start_lat;

        /** The start longitude of the route */
        double start_lon;

        /** The end latitude of the route */
        double dest_lat;

        /** The end longitude of the route */
        double dest_lon;

    };

    /**
     * A queue (FIFO) of routes that need to be calculated.
     * We use Boost's lockfree queue so that we can read from the calculation thread and write from the
     * request handler thread.
     */
    static boost::lockfree::queue<_RouteItem> _routes;

    /**
     * The map of the completed routes.
     * The key represents the unique identifier that was returned by the queueRoute function.
     * The value is the control points of the bezier curve of the route.
     * Unordered map is used because it fits the thread safety requirements.
     */
    static std::unordered_map<size_t, forJSON> _completed;


};


#endif //ROUTES_QUEUE_H