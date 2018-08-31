//
// Created by isaac on 8/31/18.
//

#ifndef ROUTES_DATABASE_H
#define ROUTES_DATABASE_H

#include <pqxx/pqxx>
#include <glm/glm.hpp>
#include <iostream>
#include <algorithm>



class Database {
public:

    /**
     * Initializes the database connection
     *
     * @param dbname
     * The name of the schema
     *
     * @param user
     * The name of the owner of the database
     *
     * @param password
     * The password for the database
     *
     */
    Database(std::string dbname, std::string user, std::string password);

    /**
     * Inserts the initial route parameters
     *
     * @param query
     * The query to be executed
     */
    void initRoute(std::string query);

    /**
     * Performs the next val queries in order to maintain foreign key integrity
     *
     * @param queries
     * A vector of queries to get the next vals
     *
     * @return
     * A vector of the ids that were returned by the queries
     */
    glm::vec3 getNextIds(std::vector<std::string> queries);

    /**
     * Inserts all the resulting information into the database
     *
     * @param queries
     * A vector of the queries to be executed
     */
    void batchInsert(std::vector<std::string> queries);

    /**
     * Selects the solutions from the db
     *
     * @param query
     * The query to select the solutions
     *
     * @return
     * The JSON string to do the select
     */
    std::string selectSolutions(std::string query);

    /**
     * Selects the total fitness from the db
     *
     * @param query
     * The query to select the totalFitness
     *
     * @return
     * The JSON string to do the select
     */
    std::string selectTotalFitness(std::string query);

    /**
     * Selects the track fitness from the db
     *
     * @param query
     * The query to select the trackFitness
     *
     * @return
     * The JSON string to do the select
     */
    std::string selectTrackFitness(std::string query);

    /**
     * Selects the curve fitness from the db
     *
     * @param query
     * The query to select the curve fitness
     *
     * @return
     * The JSON string to do the select
     */
    std::string selectCurveFitness(std::string query);

    /**
     * Selects the grade fitness from the db
     *
     * @param query
     * The query to select the grade fitness
     *
     * @return
     * The JSON string to do the select
     */
    std::string selectGradeFitness(std::string query);

    /**
     * Selects the length fitness from the db
     *
     * @param query
     * The query to select the length fitness
     *
     * @return
     * The JSON string to do the select
     */
    std::string selectLengthFitness(std::string query);

private:

    /** The name of the database */
    std::string _dbname;

    /** The name of the user that owns the database */
    std::string _user;

    /** The password for the database */
    std::string _password;

};

#endif //ROUTES_DATABASE_H
