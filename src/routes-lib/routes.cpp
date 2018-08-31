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
bool Routes::_useDb;
Database Routes::_db = Database("routes", "isaac", "evolution");

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

    _useDb = _config.getUseDb();

    // Solve!
    // These points will be in meters so we need to convert them
    std::vector<glm::vec3> computed = Genetics::solve(pop, pod, _num_generations, start, dest, _useDb);

    std::vector<glm::vec3> points = Bezier::evaluateEntireBezierCurve(computed, 100);

    _solutions = Genetics::getEval();

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

    std::string result;

    if (_useDb) {

        std::string toExec = "SELECT \"Controls\".evaluated FROM \"Controls\" "
                             "JOIN \"Generation\" ON (\"Controls\".controls_id = \"Generation\".controls_id) "
                             "JOIN \"Route\" ON (\"Route\".route_id = \"Generation\".route_id) "
                             "WHERE \"Route\".route_id = " + std::to_string(_route_id);
        result = _db.selectSolutions(toExec);
    } else {
        result = "[[]]";
    }

    return result;

}

std::string Routes::getTotalFitness() {

    std::string result;

    if (_useDb) {

        std::string toExec = "SELECT \"Fitness\".total_fitness FROM \"Fitness\" "
                             "JOIN \"Generation\" ON (\"Fitness\".generation_id = \"Generation\".generation_id) "
                             "JOIN \"Controls\" ON (\"Controls\".controls_id = \"Generation\".controls_id) "
                             "JOIN \"Route\" ON (\"Route\".route_id = \"Generation\".route_id) "
                             "WHERE \"Route\".route_id = " + std::to_string(_route_id)
                             + " ORDER BY \"Generation\".generation";
        result = _db.selectTotalFitness(toExec);
    } else {
        result = "[[]]";
    }

    return result;

}

std::string Routes::getTrackFitness() {

    std::string result;

    if (_useDb) {

        std::string toExec = "SELECT \"Fitness\".track_fitness FROM \"Fitness\" "
                             "JOIN \"Generation\" ON (\"Fitness\".generation_id = \"Generation\".generation_id) "
                             "JOIN \"Controls\" ON (\"Controls\".controls_id = \"Generation\".controls_id) "
                             "JOIN \"Route\" ON (\"Route\".route_id = \"Generation\".route_id) "
                             "WHERE \"Route\".route_id = " + std::to_string(_route_id)
                             + " ORDER BY \"Generation\".generation";
        result = _db.selectTrackFitness(toExec);
    } else {
        result = "[[]]";
    }

    return result;

}

std::string Routes::getCurveFitness() {

    std::string result;

    if (_useDb) {

        std::string toExec = "SELECT \"Fitness\".curve_fitness FROM \"Fitness\" "
                             "JOIN \"Generation\" ON (\"Fitness\".generation_id = \"Generation\".generation_id) "
                             "JOIN \"Controls\" ON (\"Controls\".controls_id = \"Generation\".controls_id) "
                             "JOIN \"Route\" ON (\"Route\".route_id = \"Generation\".route_id) "
                             "WHERE \"Route\".route_id = " + std::to_string(_route_id)
                             + " ORDER BY \"Generation\".generation";
        result = _db.selectCurveFitness(toExec);
    } else {
        result = "[[]]";
    }

    return result;
}

std::string Routes::getGradeFitness() {

    std::string result;

    if (_useDb) {

        std::string toExec = "SELECT \"Fitness\".grade_fitness FROM \"Fitness\" "
                             "JOIN \"Generation\" ON (\"Fitness\".generation_id = \"Generation\".generation_id) "
                             "JOIN \"Controls\" ON (\"Controls\".controls_id = \"Generation\".controls_id) "
                             "JOIN \"Route\" ON (\"Route\".route_id = \"Generation\".route_id) "
                             "WHERE \"Route\".route_id = " + std::to_string(_route_id)
                             + " ORDER BY \"Generation\".generation";
        result = _db.selectGradeFitness(toExec);
    } else {
        result = "[[]]";
    }

    return result;

}

std::string Routes::getLengthFitness() {

    std::string result;

    if (_useDb) {

        std::string toExec = "SELECT \"Fitness\".length_fitness FROM \"Fitness\" "
                             "JOIN \"Generation\" ON (\"Fitness\".generation_id = \"Generation\".generation_id) "
                             "JOIN \"Controls\" ON (\"Controls\".controls_id = \"Generation\".controls_id) "
                             "JOIN \"Route\" ON (\"Route\".route_id = \"Generation\".route_id) "
                             "WHERE \"Route\".route_id = " + std::to_string(_route_id)
                             + " ORDER BY \"Generation\".generation";
        result = _db.selectLengthFitness(toExec);
    } else {
        result = "[[]]";
    }

    return result;

}

bool Routes::validatePoint(const glm::vec3& point) {

    // Make sure that the z is not past a crazy number
    if (fabs(point.z) < 1000000.0)
        return true;

    return false;

}
