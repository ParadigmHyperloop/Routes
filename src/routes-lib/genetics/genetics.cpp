//
// Created by Logan on 10/12/17.
//

#include "genetics.h"

double Genetics::_lat_start;
double Genetics::_long_start;
double Genetics::_lat_end;
double Genetics::_long_end;
int Genetics::_id;
std::string Genetics::_eval;

std::vector<glm::vec3> Genetics::solve(Population& pop, Pod& pod, int generations, const glm::dvec2& start, const glm::dvec2& dest, bool useDb) {

    ElevationData elev = ElevationData(start, dest);

    _lat_start = pop._start.y;
    _long_start = pop._start.x;
    _lat_end = pop._dest.y;
    _long_end = pop._dest.x;

    std::vector<std::string> controlValsToInsert;
    std::vector<std::string> genValsToInsert;
    std::vector<std::string> fitValsToInsert;

    //Initialize database connection
    Database db = Database("evie", "evie", "evolution");


    //Insert the starting positions and the type of optimization
    std::string init = "INSERT INTO \"Route\" (lat_start, lat_end, long_start, long_end) "
                       "values (" + std::to_string(_lat_start) + ", " + std::to_string(_lat_end) + ", "
                       + std::to_string(_long_start) + ", " + std::to_string(_long_end) + ")";

    if (useDb) {
        db.initRoute(init);
    }



    //get all the route_ids
    std::string nextRouteId = "SELECT route_id FROM \"Route\" ";

    std::string nextControlsId = "SELECT nextval('\"Controls_controls_id_seq\"')";

    std::string nextGenId = "SELECT nextval('\"Generation_generation_id_seq\"')";

    std::vector<std::string> getIds = {nextRouteId, nextControlsId, nextGenId};

    int route_id = 0;
    int controls_id = 0;
    int generation_id = 0;
    glm::vec3 idResults = {0,0,0};

    if (useDb) {
        idResults= db.getNextIds(getIds);
    }

    route_id = idResults.x;
    _id = route_id;
    controls_id = idResults.y;
    generation_id = idResults.z;


    // Run the simulation for then given amount of generations
    for (int i = 0; i < generations; i++) {

        //increment this at the beginning, since if the table is empty we want the first record to have id 1
        controls_id++;
        generation_id++;

        //Step through one generation
        pop.step(pod);

        Individual ind = pop.getIndividual(0);
        if (!ind.header->x)
            break;

        // Get the best solution at this generation (this is a vector of control points)
        std::vector<glm::vec3> sol = pop.getSolution();

        //evaluate the control points
        std::vector<glm::vec3> eval = Bezier::evaluateEntireBezierCurve(sol, 100);

        std::vector<std::string> toInsertControls;
        std::vector<std::string> toInsertEvaluated;

        //convert points in meters to points in longitude and latitude
        for (glm::vec3 point : sol) {

            double longitude = elev.metersToLongitudeLatitude({point.x, point.y})[0];
            double latitude = elev.metersToLongitudeLatitude({point.x, point.y})[1];

            toInsertControls.push_back("{" + std::to_string(longitude) + ", " + std::to_string(latitude) + "}");
        }

        for (glm::vec3 point : eval) {

            double longitude = elev.metersToLongitudeLatitude({point.x, point.y})[0];
            double latitude = elev.metersToLongitudeLatitude({point.x, point.y})[1];


            toInsertEvaluated.push_back("{" + std::to_string(longitude) + ", " + std::to_string(latitude) + "}");
        }

        //aggregate the points into one string
        std::string controlsToInsert = "{";

        for (std::string s : toInsertControls) {
            controlsToInsert.append(s);
            controlsToInsert.append(", ");
        }
        controlsToInsert.append("}");

        //get rid of the trailing comma
        controlsToInsert.erase(controlsToInsert.size() - 3, 2);

        //same as above
        std::string evalToInsert = "{";

        for (std::string s : toInsertEvaluated) {
            evalToInsert.append(s);
            evalToInsert.append(", ");
        }

        evalToInsert.append("}");

        evalToInsert.erase(evalToInsert.size() - 3, 2);

        //add a row to be inserted
        controlValsToInsert.push_back("(\'" + controlsToInsert + "\'"
                                      + ", \'" + evalToInsert + "\')");

        //wrap the vector in brackets and make it one string
        std::string evaluatedResult = evalToInsert;

        evaluatedResult.append("]");

        //get rid of the trailing comma
        evaluatedResult.erase(evaluatedResult.size() - 3, 1);

        //Since this is being passed to the front end through JSON, and curly braces
        //indicate an object in JSON, change curly braces to brackets.
        std::replace(evaluatedResult.begin(), evaluatedResult.end(), '{', '[');
        std::replace(evaluatedResult.begin(), evaluatedResult.end(), '}', ']');

        _eval = evaluatedResult;


        genValsToInsert.push_back("(" + std::to_string(i) + ", "
                                  + std::to_string(controls_id) + ", "
                                  + std::to_string(route_id) + ")");


        glm::vec4 fitness = pop.getFitness();

        double total = pop.totalFitness(fitness);

        fitValsToInsert.push_back("(" + std::to_string(total) + ", "
                                  + std::to_string(fitness.x) + ","
                                  + std::to_string(fitness.y) + ","
                                  + std::to_string(fitness.z) + ","
                                  + std::to_string(fitness.w) + ","
                                  + std::to_string(generation_id) + ")");

    }

    std::string controlString = "";

    //turn the vector into a comma delimited string
    for (std::string s : controlValsToInsert) {
        controlString.append(s);
        controlString.append(",");
    }
    //erase the last comma (it is the last character here so we can just use pop_back()
    controlString.pop_back();

    std::string genString = "";

    for (std::string s : genValsToInsert) {
        genString.append(s);
        genString.append(",");
    }

    genString.pop_back();

    std::string fitString = "";

    for (std::string s : fitValsToInsert) {
        fitString.append(s);
        fitString.append(",");
    }

    fitString.pop_back();

    //Insert into the Controls table
    std::string controlsInsert= "INSERT INTO \"Controls\" (controls, evaluated) values " + controlString;

    //Insert into the Generation table
    std::string genInsert = "INSERT INTO \"Generation\" (generation, controls_id, route_id) values " + genString;


    std::string fitInsert = "INSERT INTO \"Fitness\" (total_fitness, track_fitness, curve_fitness, grade_fitness,"
                            " length_fitness, generation_id) values " + fitString;


    std::vector<std::string> toExec = {controlsInsert, genInsert, fitInsert};

    if (useDb) {
        db.batchInsert(toExec);
    }

    // Transfer the bath over
    return pop.getSolution();

}

int Genetics::getRouteId() {

    return _id;

}

std::string Genetics::getEval() {
    return _eval;
}