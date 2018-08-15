//
// Created by Logan on 10/12/17.
//

#include "genetics.h"

double Genetics::_lat_start;
double Genetics::_long_start;
double Genetics::_lat_end;
double Genetics::_long_end;
int Genetics::_id;

std::vector<glm::vec3> Genetics::solve(Population& pop, Pod& pod, int generations, const glm::dvec2& start, const glm::dvec2& dest) {

    ElevationData elev = ElevationData(start, dest);

    _lat_start = pop._start.y;
    _long_start = pop._start.x;
    _lat_end = pop._dest.y;
    _long_end = pop._dest.x;

    std::vector<std::string> controlValsToInsert;
    std::vector<std::string> genValsToInsert;
    std::vector<std::string> fitValsToInsert;

    try {
        //Start a connection to the database
        pqxx::connection c("dbname=routes user=isaac password=evolution");

        pqxx::work w(c);

        //Insert the starting positions and the type of optimization
        w.exec("INSERT INTO \"Route\" (lat_start, lat_end, long_start, long_end) "
               "values (" + std::to_string(_lat_start) + ", " + std::to_string(_lat_end) + ", "
               + std::to_string(_long_start) + ", " + std::to_string(_long_end) + ")");


        //get all the route_ids
        pqxx::result r = w.exec("SELECT route_id FROM \"Route\" ");

        int route_id;

        //find the last route_id (since it is auto_increment)
        for (auto row: r) {
            route_id = std::stoi(row[0].c_str());
        }

        _id = route_id;

        //same as above but for controls. Start this at 0 in case the tables were just truncated.
        pqxx::result con = w.exec("SELECT nextval('\"Controls_controls_id_seq\"')");

        int controls_id;
        for (auto row: con) {
            controls_id = std::stoi(row[0].c_str());
        }

        pqxx::result conFit = w.exec("SELECT nextval('\"Generation_generation_id_seq\"')");

        int generation_id;
        for (auto row: conFit) {
            generation_id = std::stoi(row[0].c_str());
        }

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


            genValsToInsert.push_back("(" + std::to_string(i) + ", "
                                      + std::to_string(controls_id) + ", "
                                      + std::to_string(route_id) + ")");


            glm::vec4 fitness = pop.getFitness();

            double total = Population::totalFitness(fitness);

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
        w.exec("INSERT INTO \"Controls\" (controls, evaluated) "
               "values " + controlString);

        //Insert into the Generation table
        w.exec("INSERT INTO \"Generation\" (generation, controls_id, route_id) "
               "values " + genString);


        w.exec("INSERT INTO \"Fitness\" (total_fitness, track_fitness, curve_fitness, grade_fitness, length_fitness, generation_id) "
               "values " + fitString);

        w.commit();
        std::cout << "Writing to database succeeded" << std::endl;

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    // Transfer the bath over
    return pop.getSolution();

}

int Genetics::getRouteId() {

    return _id;

}