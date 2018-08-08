//
// Created by Logan on 10/12/17.
//

#include "genetics.h"

std::vector<glm::vec3> Genetics::solve(Population& pop, Pod& pod, int generations) {


//    sql::Driver *driver;
//    sql::Connection *con;
//    sql::Statement *stmt;
//    sql::ResultSet *res;
//
//    /* Create a connection */
//    driver = get_driver_instance();
//
//    con = driver->connect("localhost:3306", "routes", "Evolution1!");
//    /* Connect to the MySQL test database */
//    //con->setSchema("routes");
//    std::cout << "woo" << std::endl;


//        stmt = con->createStatement();
//        res = stmt->executeQuery("SELECT 'Hello World!' AS _message");
//        while (res->next()) {
//            cout << "\t... MySQL replies: ";
//            /* Access column data by alias or column name */
//            cout << res->getString("_message") << endl;
//            cout << "\t... MySQL says it again: ";
//            /* Access column data by numeric offset, 1 is the first column */
//            cout << res->getString(1) << endl;
//        }
//        delete res;
//        delete stmt;
//        delete con;

    // Run the simulation for then given amount of generations
    for (int i = 0; i < generations; i++) {
        
        pop.step(pod);
        
        Individual ind = pop.getIndividual(0);
        if (!ind.header->x)
            break;
        
    }

    // Transfer the bath over
    return pop.getSolution();

}
