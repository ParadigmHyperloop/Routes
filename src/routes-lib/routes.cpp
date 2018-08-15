//
// Created by Logan on 10/30/17.
//

#include "routes.h"

float Routes::_time;
float Routes::_length;
std::vector<glm::vec2> Routes::_elevations;
std::vector<glm::vec2> Routes::_ground_elevations;
std::vector<glm::vec2> Routes::_speeds;
std::vector<glm::vec2> Routes::_grades;
int Routes::_route_id;
std::string Routes::_solutions;
int Routes::_pop_size;
int Routes::_num_generations;
Configure Routes::_config;

std::vector<glm::vec3> Routes::calculateRoute(glm::vec2 start, glm::vec2 dest) {

    configureParams();

    time_t now = time(0);

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

    _config = Configure();
    Population pop = Population(_pop_size, glm::vec4(start_meter.x, start_meter.y, start_meter.z + 10.0, 0.0),
                                          glm::vec4(dest_meter.x, dest_meter.y, dest_meter.z + 10.0, 0.0), data, _config);

    // Solve!
    // These points will be in meters so we need to convert them
    std::vector<glm::vec3> computed = Genetics::solve(pop, pod, _num_generations, start, dest, "single");

    std::vector<glm::vec3> points = Bezier::evaluateEntireBezierCurve(computed, 100);

    _time = pod.timeForCurve(points);
    _length = Bezier::bezierLength(points);

    _route_id = Genetics::getRouteId();

    std::vector<glm::vec2> elev;
    std::vector<float> velocities = pod.getVelocities(points);
    std::vector<glm::vec2> speeds;
    std::vector<glm::vec2> g_elev;
    std::vector<glm::vec2> grades;

    std::unordered_map<int, float> lengthMap = Bezier::bezierLengthMap(points);

    for (int i = 0; i < points.size(); i++) {
        elev.push_back({lengthMap[i], points[i].z});
        speeds.push_back({lengthMap[i], velocities[i]});
        glm::vec2 newPoint = {points[i].x, points[i].y};
        float newElev = data.metersToElevation(newPoint);
        g_elev.push_back({lengthMap[i], newElev});
    }

    float spacing = 0.0f;

    for (int i = 1; i < points.size(); i++) {
        spacing = sqrt(glm::pow(points[i].x - points[i-1].x, 2) + glm::pow(points[i].y - points[i-1].y, 2));

        grades.push_back({lengthMap[i], fabs(((points[i].z - points[i-1].z) * 100) / spacing)});
    }

    _elevations = elev;
    _ground_elevations = g_elev;
    _speeds = speeds;
    _grades = grades;

    // Convert to longitude, latitude and elevation
    for (int i = 0; i < computed.size(); i++) {

        glm::vec3& vec = computed[i];
        glm::vec2 conv = data.metersToLongitudeLatitude(glm::vec2(vec.x, vec.y));

        vec.x = conv.x;
        vec.y = conv.y;

    }

    time_t after = time(0);
    std::cout << "time to compute: " + std::to_string(after-now) << std::endl;

    return computed;

}

void Routes::configureParams() {

    _pop_size = _config.getPopulationSize();
    _num_generations = _config.getNumGenerations();

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

std::vector<glm::vec2> Routes::getGrades() {
    return _grades;
}

int Routes::getId() {
    return _route_id;
}

std::string Routes::getSolutions() {

    pqxx::result r;

    try {
        // Connect to the database
        pqxx::connection c("dbname=routes user=isaac password=evolution");

        pqxx::work w(c);

        // Get all the evaluated points with this iterations route_id
        r = w.exec("SELECT \"Controls\".evaluated FROM \"Controls\" "
                   "JOIN \"Generation\" ON (\"Controls\".controls_id = \"Generation\".controls_id) "
                   "JOIN \"Route\" ON (\"Route\".route_id = \"Generation\".route_id) "
                   "WHERE \"Route\".route_id = " + std::to_string(_route_id));

        w.commit();

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    std::vector<std::string> evaluatedStrings;


    //add all the rows into a vector and turn them into strings
    for (auto row : r) {
        evaluatedStrings.push_back(row[0].c_str());
    }

    //wrap the vector in brackets and make it one string
    std::string evaluatedResult = "[";

    for (std::string s : evaluatedStrings) {

        evaluatedResult.append(s);

        //newline for readability when debugging
        evaluatedResult.append(",\n");
    }

    evaluatedResult.append("]");

    //get rid of the trailing comma
    evaluatedResult.erase(evaluatedResult.size() - 3, 2);

    //Since this is being passed to the front end through JSON, and curly braces
    //indicate an object in JSON, change curly braces to brackets.
    std::replace(evaluatedResult.begin(), evaluatedResult.end(), '{', '[');
    std::replace(evaluatedResult.begin(), evaluatedResult.end(), '}', ']');

    return evaluatedResult;
}

std::string Routes::getTotalFitness() {

    pqxx::result r;

    try {
        pqxx::connection c("dbname=routes user=isaac password=evolution");

        pqxx::work w(c);

        // Get all the evaluated points with this iterations route_id
        r = w.exec("SELECT \"Fitness\".total_fitness FROM \"Fitness\" "
                   "JOIN \"Generation\" ON (\"Fitness\".generation_id = \"Generation\".generation_id) "
                   "JOIN \"Controls\" ON (\"Controls\".controls_id = \"Generation\".controls_id) "
                   "JOIN \"Route\" ON (\"Route\".route_id = \"Generation\".route_id) "
                   "WHERE \"Route\".route_id = " + std::to_string(_route_id)
                   + " ORDER BY \"Generation\".generation");

        w.commit();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    std::vector<std::string> evaluatedStrings;


    //add all the rows into a vector and turn them into strings
    for (auto row : r) {
        evaluatedStrings.push_back(row[0].c_str());
    }

    //wrap the vector in brackets and make it one string
    std::string evaluatedResult = "[";

    for (std::string s : evaluatedStrings) {

        evaluatedResult.append(s);

        //newline for readability when debugging
        evaluatedResult.append(",\n");
    }

    evaluatedResult.append("]");

    //get rid of the trailing comma
    evaluatedResult.erase(evaluatedResult.size() - 3, 2);

    return evaluatedResult;

}

std::string Routes::getTrackFitness() {

    pqxx::result r;

    try {
        pqxx::connection c("dbname=routes user=isaac password=evolution");

        pqxx::work w(c);

        // Get all the evaluated points with this iterations route_id
        r = w.exec("SELECT \"Fitness\".track_fitness FROM \"Fitness\" "
                   "JOIN \"Generation\" ON (\"Fitness\".generation_id = \"Generation\".generation_id) "
                   "JOIN \"Controls\" ON (\"Controls\".controls_id = \"Generation\".controls_id) "
                   "JOIN \"Route\" ON (\"Route\".route_id = \"Generation\".route_id) "
                   "WHERE \"Route\".route_id = " + std::to_string(_route_id)
                   + " ORDER BY \"Generation\".generation");

        w.commit();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    std::vector<std::string> evaluatedStrings;


    //add all the rows into a vector and turn them into strings
    for (auto row : r) {
        evaluatedStrings.push_back(row[0].c_str());
    }

    //wrap the vector in brackets and make it one string
    std::string evaluatedResult = "[";

    for (std::string s : evaluatedStrings) {

        evaluatedResult.append(s);

        //newline for readability when debugging
        evaluatedResult.append(",\n");
    }

    evaluatedResult.append("]");

    //get rid of the trailing comma
    evaluatedResult.erase(evaluatedResult.size() - 3, 2);

    return evaluatedResult;

}

std::string Routes::getCurveFitness() {

    pqxx::result r;

    try {
        pqxx::connection c("dbname=routes user=isaac password=evolution");

        pqxx::work w(c);

        // Get all the evaluated points with this iterations route_id
        r = w.exec("SELECT \"Fitness\".curve_fitness FROM \"Fitness\" "
                   "JOIN \"Generation\" ON (\"Fitness\".generation_id = \"Generation\".generation_id) "
                   "JOIN \"Controls\" ON (\"Controls\".controls_id = \"Generation\".controls_id) "
                   "JOIN \"Route\" ON (\"Route\".route_id = \"Generation\".route_id) "
                   "WHERE \"Route\".route_id = " + std::to_string(_route_id)
                   + " ORDER BY \"Generation\".generation");

        w.commit();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    std::vector<std::string> evaluatedStrings;


    //add all the rows into a vector and turn them into strings
    for (auto row : r) {
        evaluatedStrings.push_back(row[0].c_str());
    }

    //wrap the vector in brackets and make it one string
    std::string evaluatedResult = "[";

    for (std::string s : evaluatedStrings) {

        evaluatedResult.append(s);

        //newline for readability when debugging
        evaluatedResult.append(",\n");
    }

    evaluatedResult.append("]");

    //get rid of the trailing comma
    evaluatedResult.erase(evaluatedResult.size() - 3, 2);

    return evaluatedResult;

}

std::string Routes::getGradeFitness() {

    pqxx::result r;

    try {
        pqxx::connection c("dbname=routes user=isaac password=evolution");

        pqxx::work w(c);

        // Get all the evaluated points with this iterations route_id
        r = w.exec("SELECT \"Fitness\".grade_fitness FROM \"Fitness\" "
                   "JOIN \"Generation\" ON (\"Fitness\".generation_id = \"Generation\".generation_id) "
                   "JOIN \"Controls\" ON (\"Controls\".controls_id = \"Generation\".controls_id) "
                   "JOIN \"Route\" ON (\"Route\".route_id = \"Generation\".route_id) "
                   "WHERE \"Route\".route_id = " + std::to_string(_route_id)
                   + " ORDER BY \"Generation\".generation");

        w.commit();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    std::vector<std::string> evaluatedStrings;


    //add all the rows into a vector and turn them into strings
    for (auto row : r) {
        evaluatedStrings.push_back(row[0].c_str());
    }

    //wrap the vector in brackets and make it one string
    std::string evaluatedResult = "[";

    for (std::string s : evaluatedStrings) {

        evaluatedResult.append(s);

        //newline for readability when debugging
        evaluatedResult.append(",\n");
    }

    evaluatedResult.append("]");

    //get rid of the trailing comma
    evaluatedResult.erase(evaluatedResult.size() - 3, 2);

    return evaluatedResult;

}

std::string Routes::getLengthFitness() {

    pqxx::result r;

    try {
        pqxx::connection c("dbname=routes user=isaac password=evolution");

        pqxx::work w(c);

        // Get all the evaluated points with this iterations route_id
        r = w.exec("SELECT \"Fitness\".length_fitness FROM \"Fitness\" "
                   "JOIN \"Generation\" ON (\"Fitness\".generation_id = \"Generation\".generation_id) "
                   "JOIN \"Controls\" ON (\"Controls\".controls_id = \"Generation\".controls_id) "
                   "JOIN \"Route\" ON (\"Route\".route_id = \"Generation\".route_id) "
                   "WHERE \"Route\".route_id = " + std::to_string(_route_id)
                   + " ORDER BY \"Generation\".generation");

        w.commit();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    std::vector<std::string> evaluatedStrings;


    //add all the rows into a vector and turn them into strings
    for (auto row : r) {
        evaluatedStrings.push_back(row[0].c_str());
    }

    //wrap the vector in brackets and make it one string
    std::string evaluatedResult = "[";

    for (std::string s : evaluatedStrings) {

        evaluatedResult.append(s);

        //newline for readability when debugging
        evaluatedResult.append(",\n");
    }

    evaluatedResult.append("]");

    //get rid of the trailing comma
    evaluatedResult.erase(evaluatedResult.size() - 3, 2);

    return evaluatedResult;

}

bool Routes::validatePoint(const glm::vec3& point) {

    // Make sure that the z is not past a crazy number
    if (fabs(point.z) < 1000000.0)
        return true;

    return false;

}
