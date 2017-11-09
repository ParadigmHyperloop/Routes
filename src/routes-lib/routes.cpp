//
// Created by Logan on 10/30/17.
//

#include "routes.h"

std::vector<glm::vec3> Routes::calculateRoute(glm::vec3 start, glm::vec3 dest) {

    // Determine which datasets need to be stitched together using the database
    std::vector<std::string> required_data = DB::getRequiredDatasets(glm::vec2(start.x, start.y),
                                                                     glm::vec2(dest.x,  dest.y));

    for (int i = 0; i < required_data.size(); i++)
        std::cout << "Using:" << required_data[i] << std::endl;

    // Make sure there was data to compute this
    if (!required_data.size())
        throw std::runtime_error("Data to compute this route was missing or the database is corrupt.");

    // Stitch together the data
    ElevationData data = ElevationData(required_data[0]);
    
    if (required_data.size() > 1)
        data = ElevationStitch::stitch(required_data);

    // Figure out where the longitude and latitude are in meters
    glm::vec2 start_meter = data.longitudeLatitudeToMeters(glm::vec2(start.x, start.y));
    glm::vec2 dest_meter  = data.longitudeLatitudeToMeters(glm::vec2(dest.x,  dest.y));

    // Create a pod and a population
    Pod pod = Pod(DEFAULT_POD_MAX_SPEED);
    Population pop = Population(POP_SIZE, glm::vec4(start_meter.x, start_meter.y, start.z, 0.0), glm::vec4(dest_meter.x, dest_meter.y, dest.z, 0.0), data);

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
