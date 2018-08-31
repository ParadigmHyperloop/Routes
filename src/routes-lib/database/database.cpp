//
// Created by isaac on 8/31/18.
//

#include "database.h"

std::string _dbname;
std::string _user;
std::string _password;


Database::Database(std::string dbname, std::string user, std::string password) {

    _dbname = dbname;
    _user = user;
    _password = password;

}

void Database::initRoute(std::string query) {

    try {
        pqxx::connection c("dbname=" + _dbname + " user=" + _user + " password=" + _password);

        pqxx::work w(c);

        w.exec(query);

        w.commit();

        std::cout << "Initial database write succeeded" << std::endl;

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

}

glm::vec3 Database::getNextIds(std::vector<std::string> queries) {

    try {
        pqxx::connection c("dbname=" + _dbname + " user=" + _user + " password=" + _password);

        pqxx::work w(c);

        pqxx::result r1 = w.exec(queries[0]);

        int route_id;

        //find the last route_id (since it is auto_increment)
        for (auto row: r1) {
            route_id = std::stoi(row[0].c_str());
        }

        pqxx::result r2 = w.exec(queries[1]);

        int controls_id;
        for (auto row: r2) {
            controls_id = std::stoi(row[0].c_str());
        }

        pqxx::result r3 = w.exec(queries[2]);

        int generation_id;
        for (auto row: r3) {
            generation_id = std::stoi(row[0].c_str());
        }

        w.commit();

        return {route_id, controls_id, generation_id};

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

}

void Database::batchInsert(std::vector<std::string> queries) {

    try {
        pqxx::connection c("dbname=" + _dbname + " user=" + _user + " password=" + _password);

        pqxx::work w(c);

        w.exec(queries[0]);
        w.exec(queries[1]);
        w.exec(queries[2]);

        w.commit();

        std::cout << "Writing to database succeeded" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

}

std::string Database::selectSolutions(std::string query) {

    pqxx::result r;

    try {
        pqxx::connection c("dbname=" + _dbname + " user=" + _user + " password=" + _password);

        pqxx::work w(c);

        r = w.exec(query);

        w.commit();

        std::cout << "Writing to database succeeded" << std::endl;
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

std::string Database::selectTotalFitness(std::string query) {

    pqxx::result r;

    try {
        pqxx::connection c("dbname=" + _dbname + " user=" + _user + " password=" + _password);

        pqxx::work w(c);

        r = w.exec(query);

        w.commit();

        std::cout << "Writing to database succeeded" << std::endl;
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

std::string Database::selectTrackFitness(std::string query) {

    pqxx::result r;

    try {
        pqxx::connection c("dbname=" + _dbname + " user=" + _user + " password=" + _password);

        pqxx::work w(c);

        r = w.exec(query);

        w.commit();

        std::cout << "Writing to database succeeded" << std::endl;
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

std::string Database::selectCurveFitness(std::string query) {

    pqxx::result r;

    try {
        pqxx::connection c("dbname=" + _dbname + " user=" + _user + " password=" + _password);

        pqxx::work w(c);

        r = w.exec(query);

        w.commit();

        std::cout << "Writing to database succeeded" << std::endl;
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

std::string Database::selectGradeFitness(std::string query) {

    pqxx::result r;

    try {
        pqxx::connection c("dbname=" + _dbname + " user=" + _user + " password=" + _password);

        pqxx::work w(c);

        r = w.exec(query);

        w.commit();

        std::cout << "Writing to database succeeded" << std::endl;
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

std::string Database::selectLengthFitness(std::string query) {

    pqxx::result r;

    try {
        pqxx::connection c("dbname=" + _dbname + " user=" + _user + " password=" + _password);

        pqxx::work w(c);

        r = w.exec(query);

        w.commit();

        std::cout << "Writing to database succeeded" << std::endl;
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