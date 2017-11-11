//
// Created by Logan on 10/30/17.
//

#include "routes.h"

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
    Population pop = Population(POP_SIZE, glm::vec4(start_meter.x, start_meter.y, start_meter.z, 0.0),
                                          glm::vec4(dest_meter.x, dest_meter.y, dest_meter.z, 0.0), data);

    // Solve!
    // These points will be in meters so we need to convert them
    std::vector<glm::vec3> computed = Genetics::solve(pop, pod, NUM_GENERATIONS);

    // Convert to longitude, latitude and elevation
    for (int i = 0; i < computed.size(); i++) {

        glm::vec3& vec = computed[i];
        glm::vec2 conv = data.metersToLongitudeLatitude(glm::vec2(vec.x, vec.y));

        vec.x = conv.x;
        vec.y = conv.y;

    }

    return computed;

}

bool Routes::validatePoint(const glm::vec3& point) {

    // Make sure that the z is not past a crazy number
    if (fabs(point.z) < 1000000.0)
        return true;

    return false;

}