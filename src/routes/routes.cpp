//
// Created by Logan on 10/30/17.
//

#include "routes.h"

void Routes::calculateRoute(glm::vec3 start, glm::vec3 dest) {

    // Determine which datasets need to be stitched together using the database
    std::vector<std::string> required_data = DB::getRequiredDatasets(glm::vec2(start.x, start.y),
                                                                     glm::vec2(dest.x,  dest.y));

    for (int i = 0; i < required_data.size(); i++)
        std::cout << "Stitching :" << required_data[i] << std::endl;

    // Stitch together the data
    ElevationData data = ElevationData(required_data[0]);

    // Figure out where the longitude and latitude are in meters
    glm::vec2 start_meter = data.longitudeLatitudeToMeters(glm::vec2(start.x, start.y));
    glm::vec2 dest_meter  = data.longitudeLatitudeToMeters(glm::vec2(dest.x,  dest.y));

    // Create a pod and a population
    Pod pod = Pod(339.0);
    Population pop = Population(200, 8, glm::vec4(start_meter.x, start_meter.y, start.z, 0.0), glm::vec4(dest_meter.x, dest_meter.y, dest.z, 0.0), data);

    // Solve!
    std::vector<glm::vec3> computed = Genetics::solve(pop, pod, 200);

    // Write out some HTML
    HTML::generateHTMLForPath(computed, data);

}