//
// Created by Logan on 10/30/17.
//

#include "routes.h"

float Routes::_time;
float Routes::_length;
std::vector<glm::vec2> Routes::_elevations;
std::vector<glm::vec2> Routes::_ground_elevations;
std::vector<glm::vec2> Routes::_speeds;

std::vector<glm::vec3> Routes::calculateRoute(glm::vec2 start, glm::vec2 dest) {

    std::cout << "Calculating a route\n";

    // Stitch together the data
    ElevationData data = ElevationData(start, dest);

    // Figure out where the longitude and latitude are in meters
    glm::dvec3 start_meter = data.metersToMetersAndElevation(data.longitudeLatitudeToMeters(start));
    glm::dvec3 dest_meter  = data.metersToMetersAndElevation(data.longitudeLatitudeToMeters(dest));

    std::cout << "Final path was: " << glm::to_string(start_meter) << " -> " << glm::to_string(dest_meter) << std::endl;

    if (!validatePoint(start_meter) || !validatePoint(dest_meter))
        throw std::runtime_error("The start or destination was on a NODATA value");
    
    // Create a pod and a population
    Pod pod = Pod(DEFAULT_POD_MAX_SPEED);
    Population pop = Population(POP_SIZE, glm::vec4(start_meter.x, start_meter.y, start_meter.z + 10.0, 0.0),
                                          glm::vec4(dest_meter.x, dest_meter.y, dest_meter.z + 10.0, 0.0), data);

    // Solve!
    // These points will be in meters so we need to convert them
    std::vector<glm::vec3> computed = Genetics::solve(pop, pod, NUM_GENERATIONS);

    std::vector<glm::vec3> points = Bezier::evaluateEntireBezierCurve(computed, 100);

    _time = pod.timeForCurve(points);
    _length = Bezier::bezierLength(points);

    std::vector<glm::vec2> elev;
    std::vector<float> velocities = pod.getVelocities(points);
    std::vector<glm::vec2> speeds;
    std::vector<glm::vec2> g_elev;

    std::unordered_map<int, float> lengthMap = Bezier::bezierLengthMap(points);

    for (int i = 0; i < points.size(); i++) {
        elev.push_back({lengthMap[i], points[i].z});
        speeds.push_back({lengthMap[i], velocities[i]});
        glm::vec2 newPoint = {points[i].x, points[i].y};
        float newElev = data.metersToElevation(newPoint);
        g_elev.push_back({lengthMap[i], newElev});
    }

    _elevations = elev;
    _ground_elevations = g_elev;
    _speeds = speeds;

    // Convert to longitude, latitude and elevation
    for (int i = 0; i < computed.size(); i++) {

        glm::vec3& vec = computed[i];
        glm::vec2 conv = data.metersToLongitudeLatitude(glm::vec2(vec.x, vec.y));

        vec.x = conv.x;
        vec.y = conv.y;

    }

    return computed;

}

float Routes::getTime() {
    return _time;
}

float Routes::getLength() {
    return _length;
}

std::vector<glm::vec2> Routes::getElevations() {
    return _elevations;
}

std::vector<glm::vec2> Routes::getSpeeds() {
    return _speeds;
}

std::vector<glm::vec2> Routes::getGElevations() {
    return _ground_elevations;
}


bool Routes::validatePoint(const glm::vec3& point) {

    // Make sure that the z is not past a crazy number
    if (fabs(point.z) < 1000000.0)
        return true;

    return false;

}
