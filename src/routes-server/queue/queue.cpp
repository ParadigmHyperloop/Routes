//
// Created by Logan on 11/3/17.
//

#include "queue.h"

boost::lockfree::queue<RoutesQueue::RouteItem> RoutesQueue::_routes(0);
std::unordered_map<size_t, std::vector<glm::vec3>> RoutesQueue::_completed;

size_t RoutesQueue::queueRoute(const glm::vec3& start, const glm::vec3& dest) {

    // Get the current time
    size_t identifier = time(0);

    // Make a new RouteItem and add it to the queue
    RouteItem item;

    item.id = identifier;
    item.start = start;
    item.dest = dest;

    _routes.push(item);

    return identifier;

}

void RoutesQueue::calculateRoutes() {

    RouteItem item;
    while (_routes.pop(item)) {

        // Calculate the route and insert it into the map
        // This can be done because the [] operator behaves like it is const for the purposes of thread safety
        _completed[item.id] = Routes::calculateRoute(item.start, item.dest);

    }

}

bool RoutesQueue::isRouteCompleted(size_t id) {

    // Return whether or not the completed map had the route that was asked for
    return (bool)_completed.count(id);

}

std::vector<glm::vec3> RoutesQueue::getCompletedRoute(size_t id) {

    // We assume that the route is finished because isRouteCompleted should have been called.
    std::vector<glm::vec3> controls = _completed[id];

    // Evaluate it
    std::vector<glm::vec3> evaluated = Bezier::evaluateEntireBezierCurve(controls, 100);

    // Erase it from the completed
    _completed.erase(id);

    return evaluated;

}