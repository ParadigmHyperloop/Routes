//
// Created by Logan on 11/3/17.
//

#include "server.h"

std::thread RoutesServer::calculation_thread;

void RoutesServer::startServer(int port) {

    // Create a resource for the compute
    auto compute_resource = std::make_shared<restbed::Resource>();
    compute_resource->set_path("/compute");
    compute_resource->set_method_handler("GET", handleCompute);
    compute_resource->set_method_handler("OPTIONS", handleCORS);

    // Make the resource for the retrial of the routes
    auto retrieve_resource = std::make_shared<restbed::Resource>();
    retrieve_resource->set_path("/retrieve");
    retrieve_resource->set_method_handler("GET", handleRetrieval);
    retrieve_resource->set_method_handler("OPTIONS", handleCORS);
    
    // Make the resource for the querying the max route length
    auto length_resource = std::make_shared<restbed::Resource>();
    length_resource->set_path("/max-route-length");
    length_resource->set_method_handler("GET", handleMaxRoute);
    length_resource->set_method_handler("OPTIONS", handleCORS);

    // Make the settings to start up the server
    auto settings = std::make_shared<restbed::Settings>();
    settings->set_port(port);

    // Create the service and add the resources and start handler onto it
    auto service = std::make_shared<restbed::Service>();

    service->publish(compute_resource);
    service->publish(retrieve_resource);
    service->publish(length_resource);
    service->set_ready_handler(onServerReady);

    // Start the server
    service->start(settings);

}

void RoutesServer::handleCORS(const std::shared_ptr<restbed::Session>& session) {

    // Send back a response with the proper CORS headers
    session->close(restbed::OK, "", {{"Access-Control-Allow-Origin",  "*"},
                                     {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
                                     {"Access-Control-Allow-Headers", "Content-Type, Accept, Access-Control-Allow-Headers, Access-Control-Allow-Methods, Access-Control-Allow-Origin"}} );
}

void RoutesServer::handleCompute(const std::shared_ptr<restbed::Session>& session) {

    // Get the request
    auto request = session->get_request();

    // Parse the arguments
    std::string start_s = request->get_query_parameter("start", "");
    std::string dest_s = request->get_query_parameter("dest", "");

    glm::vec2 start, dest;
    sscanf(start_s.c_str(), "%f,%f", &start.x, &start.y);
    sscanf(dest_s.c_str(),  "%f,%f", &dest.x, &dest.y);

    // Add the route to the queue
    size_t id = RoutesQueue::queueRoute(start, dest);
    std::string id_as_string =  std::to_string(id);

    sendResponse(session, id_as_string);

}

void RoutesServer::handleRetrieval(const std::shared_ptr<restbed::Session>& session) {

    auto request = session->get_request();

    // Parse the arguments
    size_t id = request->get_query_parameter("id", 0);

    // Check that the route is finished
    if (RoutesQueue::isRouteCompleted(id)) {

        // Get the control points
        RoutesQueue::forJSON ans = RoutesQueue::getCompletedRoute(id);

        std::vector<glm::vec3> controls = ans.controls;
        float time = ans.time;

        // Check if there was an exception
        if (controls[0].x == std::numeric_limits<float>::max()) {

            // Send some error text
            sendResponse(session, "An error was encountered calculating the route");

        } else {

            // Evaluate it
            std::vector<glm::vec3> evaluated = ans.evaluated;

            std::string timeJSON = std::to_string(time);

            std::string distanceJSON = std::to_string(ans.length);

            // Convert to a  JSON string
            std::string JSON = "{\"controls\":\n" + vector3ToJSON(controls) +
                    + ", \n\"evaluated\":\n" + vector3ToJSON(evaluated) +
                    + ", \n\"timeForCurve\":\n    " + timeJSON +
                    + ", \n\"distance\":\n    " + distanceJSON +
                    + ", \n\"elevations\":\n" + vector2ToJSON(ans.elevations) +
                    + ", \n\"groundElevations\":\n" + vector2ToJSON(ans.ground_elevations) +
                    + ", \n\"speeds\":\n" + vector2ToJSON(ans.speeds) +
                    + ", \n\"grades\":\n" + vector2ToJSON(ans.grades) +
                    + ", \n\"route_id\":\n" + std::to_string(ans.route_id) +
                    + ", \n\"solutions\":\n" + ans.solutions +
                    + ", \n\"totalFitness\":\n" + ans.total_fitness +
                    + ", \n\"trackFitness\":\n" + ans.track_fitness +
                    + ", \n\"curveFitness\":\n" + ans.curve_fitness +
                    + ", \n\"gradeFitness\":\n" + ans.grade_fitness +
                    + ", \n\"lengthFitness\":\n" + ans.length_fitness +"}";


            sendResponse(session, JSON);

        }

    } else {

        // Route was not done, return "false"
        sendResponse(session, "false");

    }

}

void RoutesServer::handleMaxRoute(const std::shared_ptr <restbed::Session>& session) {
    
    // Send the longest distance as a string
    std::string max_length = std::to_string(ElevationData::getLongestAllowedRoute());
    sendResponse(session, max_length);
    
}

void RoutesServer::onServerReady(restbed::Service &service) {

    fprintf( stderr, "Rest server for route calculation is running.\n");

    // Add a new thread so that the server doesnt get locked
    calculation_thread = std::thread([] {

        // Loop infinitely
        while (true) {

            // Sleep for a single second and then try to calculate some new routes
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            RoutesQueue::calculateRoutes();
        }

    });

}

void RoutesServer::sendResponse(const std::shared_ptr<restbed::Session>& session, const std::string& message) {

    session->close(restbed::OK, message.c_str(),
                   {{"Access-Control-Allow-Origin",  "*"},
                    {"Content-Length", std::to_string(message.length())},
                    {"Connection", "close"}});
    
}

std::string RoutesServer::vector3ToJSON(const std::vector<glm::vec3> points) {
    
    std::string JSON = "   [";
    
    for (int i = 0; i < points.size(); i++) {
        
        // Add an indent to make it a little easier for humans to read
        if (i)
            JSON += "    ";
        
        JSON += "[" + std::to_string(points[i].x) + ", " +
        std::to_string(points[i].y) + ", " +
        std::to_string(points[i].z) + "]";
        
        // Make sure there aren't trailing commas
        if (i != points.size() - 1)
            JSON += ", \n";
        
    }
    
    return JSON + "]";
    
}

std::string RoutesServer::vector2ToJSON(const std::vector<glm::vec2> points) {

    std::string JSON = "   [";

    for (int i = 0; i < points.size(); i++) {

        // Add an indent to make it a little easier for humans to read
        if (i)
            JSON += "    ";

        JSON += "[" + std::to_string(points[i].x) + ", " +
                std::to_string(points[i].y) + "]";

        // Make sure there aren't trailing commas
        if (i != points.size() - 1)
            JSON += ", \n";

    }

    return JSON + "]";

}
