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

    // Make the settings to start up the server
    auto settings = std::make_shared<restbed::Settings>();
    settings->set_port(port);

    // Create the service and add the resources and start handler onto it
    auto service = std::make_shared<restbed::Service>();

    service->publish(compute_resource);
    service->publish(retrieve_resource);
    service->set_ready_handler(onServerReady);

    // Start the server
    service->start(settings);


}

void RoutesServer::handleCORS(const std::shared_ptr<restbed::Session> session) {

    // Send back a response with the proper CORS headers
    session->close(restbed::OK, "", {{"Access-Control-Allow-Origin",  "*"},
                                     {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
                                     {"Access-Control-Allow-Headers", "Content-Type, Accept, Access-Control-Allow-Headers, Access-Control-Allow-Methods, Access-Control-Allow-Origin"}} );
}

void RoutesServer::handleCompute(const std::shared_ptr<restbed::Session> session) {

    // Get the request
    auto request = session->get_request();

    // Parse the arguments
    std::string start_s = request->get_query_parameter("start", "");
    std::string dest_s = request->get_query_parameter("dest", "");

    glm::vec3 start, dest;
    sscanf(start_s.c_str(), "%f,%f,%f", &start.x, &start.y, &start.z);
    sscanf(dest_s.c_str(),  "%f,%f,%f", &dest.x, &dest.y, &dest.z);

    // Add the route to the queue
    size_t id = RoutesQueue::queueRoute(start, dest);
    std::string id_as_string =  std::to_string(id);

    session->close(restbed::OK, id_as_string.c_str(),
                   {{"Access-Control-Allow-Origin",  "*"},
                    {"Content-Length", std::to_string(id_as_string.length())},
                    { "Connection", "close" }});

}

void RoutesServer::handleRetrieval(const std::shared_ptr<restbed::Session> session) {

    auto request = session->get_request();

    // Parse the arguments
    size_t id = request->get_query_parameter("id", 0);

    // Check that the route is finished
    if (RoutesQueue::isRouteCompleted(id)) {

        // Get the control points
        std::vector<glm::vec3> points = RoutesQueue::getCompletedRoute(id);

        // Convert to a  JSON string
        std::string JSON = "{\"controls\": [";

        for (int i = 0; i < points.size(); i++)
            JSON += "[" + std::to_string(points[i].x) + ", " +
                          std::to_string(points[i].y) + ", " +
                          std::to_string(points[i].z) + "],";

        JSON += "]}";

        session->close(restbed::OK, JSON.c_str(),
                       {{"Access-Control-Allow-Origin",  "*"},
                        {"Content-Length", std::to_string(JSON.length())},
                        { "Connection", "close" }});

    } else {

        // Route was not done, return "false"
        session->close(restbed::OK, "false", {{"Access-Control-Allow-Origin",  "*"},
                                              { "Content-Length", "5" }} );

    }


}

void RoutesServer::onServerReady(restbed::Service &service) {

    fprintf( stderr, "Rest server for route calculation is running.\n");

    // Add a new thread so that the server doesnt get locked
    calculation_thread = std::thread([] {

        // Loop infinitely
        while (true) {

            // Sleep for a single second and then try to calculate some new routes
            CPLSleep(1);
            RoutesQueue::calculateRoutes();

        }

    });

}
