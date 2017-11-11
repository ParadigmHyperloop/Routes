//
//  tests.cpp
//  Routes
//
//  Created by Logan on 11/10/17.
//

#define BOOST_TEST_MODULE Routes-TestSuite
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_test) {
    
    BOOST_CHECK_EQUAL(1, 1);

}

// This test is supposed to fail
BOOST_AUTO_TEST_CASE_EXPECTED_FAILURES(test_test1, 1)

BOOST_AUTO_TEST_CASE(test_test1) {
    
    BOOST_TEST_MESSAGE("Running failing test");
    BOOST_CHECK_EQUAL(1, 0);
    
}
