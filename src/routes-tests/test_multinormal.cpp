//
//  test_multinormal.cpp
//  Routes
//
//  Created by Logan on 11/17/17.
//

#include <normal/multinormal.h>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_decomp) {
 
    // Make a matrix and test the Cholesky decomposition
    boost::numeric::ublas::matrix<float> A = boost::numeric::ublas::matrix<float>(4, 4);
    
    A(0, 0) =  18;
    A(1, 0) =  22;
    A(2, 0) =  54;
    A(3, 0) =  42;
    
    A(0, 1) =  22;
    A(1, 1) =  70;
    A(2, 1) =  86;
    A(3, 1) =  62;
    
    A(0, 2) =  54;
    A(1, 2) =  86;
    A(2, 2) =  174;
    A(3, 2) =  134;
    
    A(0, 3) =  42;
    A(1, 3) =  62;
    A(2, 3) =  134;
    A(3, 3) =  106;
    
    // Get the decomposition matrix, L
    boost::numeric::ublas::matrix<float> L = MultiNormal::choleskyDecomposition(A);
    
    // Make sure that we got the right stuff back
    BOOST_CHECK_CLOSE(L(0, 0), 4.24264, 1);
    BOOST_CHECK_CLOSE(L(1, 0), 5.18545, 1);
    BOOST_CHECK_CLOSE(L(2, 0), 12.72792, 1);
    BOOST_CHECK_CLOSE(L(3, 0), 9.89949, 1);
    
    BOOST_CHECK_CLOSE(L(0, 1), 0.00000, 1);
    BOOST_CHECK_CLOSE(L(1, 1), 6.56591, 1);
    BOOST_CHECK_CLOSE(L(2, 1), 3.04604, 1);
    BOOST_CHECK_CLOSE(L(3, 1), 1.62455, 1);

    BOOST_CHECK_CLOSE(L(0, 2), 0.00000, 1);
    BOOST_CHECK_CLOSE(L(1, 2), 0.00000, 1);
    BOOST_CHECK_CLOSE(L(2, 2), 1.64974, 1);
    BOOST_CHECK_CLOSE(L(3, 2), 1.84971, 1);
    
    BOOST_CHECK_CLOSE(L(0, 3), 0.00000, 1);
    BOOST_CHECK_CLOSE(L(1, 3), 0.00000, 1);
    BOOST_CHECK_CLOSE(L(2, 3), 0.00000, 1);
    BOOST_CHECK_CLOSE(L(3, 3), 1.39262, 1);
    
}
