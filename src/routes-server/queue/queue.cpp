//
// Created by Logan on 11/3/17.
//

#include "queue.h"

boost::lockfree::queue<RoutesQueue::_RouteItem> RoutesQueue::_routes(0);
std::unordered_map<size_t, std::vector<glm::vec3>> RoutesQueue::_completed;

size_t RoutesQueue::queueRoute(const glm::vec2& start, const glm::vec2& dest) {

    // Get the current time
    size_t identifier = time(0);

    // Make a new _RouteItem and add it to the queue
    _RouteItem item;

    item.id = identifier;
    item.start_lat = start.x;
    item.start_lon = start.y;
    item.dest_lat = dest.x;
    item.dest_lon = dest.y;

    _routes.push(item);

    return identifier;

}

void RoutesQueue::calculateRoutes() {

    _RouteItem item;
    while (_routes.pop(item)) {

        try {

            // Calculate the route and insert it into the map
            // This can be done because the [] operator behaves like it is const for the purposes of thread safety
            glm::vec2 start = glm::vec2(item.start_lat, item.start_lon);
            glm::vec2 dest = glm::vec2(item.dest_lat, item.dest_lon);
            _completed[item.id] = Routes::calculateRoute(start, dest);

        } catch (std::runtime_error e) {

            // Print out that the server had an exception
            std::cout << "Exception: " << e.what() << std::endl;
            _completed[item.id] = {glm::vec3(std::numeric_limits<float>::max())};

        }

    }

}

bool RoutesQueue::isRouteCompleted(size_t id) {

    // Return whether or not the completed map had the route that was asked for
    return (bool)_completed.count(id);

}

std::vector<glm::vec3> RoutesQueue::getCompletedRoute(size_t id) {

    // We assume that the route is finished because isRouteCompleted should have been called.
    std::vector<glm::vec3> controls = _completed[id];

    // Erase it from the completed
    _completed.erase(id);

    return controls;

}