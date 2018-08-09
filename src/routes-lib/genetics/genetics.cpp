//
// Created by Logan on 10/12/17.
//

#include "genetics.h"

double Genetics::_lat_start;
double Genetics::_long_start;
double Genetics::_lat_end;
double Genetics::_long_end;
std::vector<glm::vec3> Genetics::solve(Population& pop, Pod& pod, int generations, const glm::dvec2& start, const glm::dvec2& dest, std::string objectiveType) {

    ElevationData elev = ElevationData(start, dest);

    _lat_start = pop._start.y;
    _long_start = pop._start.x;
    _lat_end = pop._dest.y;
    _long_end = pop._dest.x;

    try {
        pqxx::connection c("dbname=routes user=isaac password=evolution");

        pqxx::work w(c);

        w.exec("INSERT INTO \"Route\" (lat_start, lat_end, long_start, long_end, objectives) "
               "values (" + std::to_string(_lat_start) + ", " + std::to_string(_lat_end) + ", "
               + std::to_string(_long_start) + ", " + std::to_string(_long_end)
               + ", " + "\'" + objectiveType + "\'" + ")");


        pqxx::result r = w.exec("SELECT route_id FROM \"Route\" ");

        int route_id;
        for (auto row: r) {
            route_id = std::stoi(row[0].c_str());
        }
        
        // Run the simulation for then given amount of generations
        for (int i = 0; i < generations; i++) {
            pop.step(pod, objectiveType);

            Individual ind = pop.getIndividual(0);
            if (!ind.header->x)
                break;

            std::vector<glm::vec3> sol = pop.getSolution();

            std::vector<std::string> toInsert;

            for (glm::vec3 point : sol) {

                double longitude = elev.metersToLongitudeLatitude({point.x, point.y})[0];
                double latitude = elev.metersToLongitudeLatitude({point.x, point.y})[1];


                toInsert.push_back("{" + std::to_string(longitude) + ", " + std::to_string(latitude) + "}");
            }

            std::string controlsToInsert = "{";

            for (std::string s: toInsert) {
                controlsToInsert.append(s);
                controlsToInsert.append(", ");
            }
            controlsToInsert.append("}");

            controlsToInsert.erase(controlsToInsert.size() - 3, 2);

            w.exec("INSERT INTO \"Controls\" (route_id, controls) "
                   "values (" + std::to_string(route_id) + ", \'" + controlsToInsert + "\')");

            pqxx::result c = w.exec("SELECT controls_id FROM \"Controls\"");

            int controls_id;
            for (auto row: c) {
                controls_id = std::stoi(row[0].c_str());
            }

            w.exec("INSERT INTO \"Generation\" (generation, controls_id, route_id) "
                   "values (" + std::to_string(i) + ", "
                   + std::to_string(controls_id) + ", "
                   + std::to_string(route_id) + ")");
        }

        w.commit();
        std::cout << "Writing to database succeeded" << std::endl;

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    // Transfer the bath over
    return pop.getSolution();

}
