//
//  multinormal.h
//  Routes
//
//  Created by Logan on 11/17/17.
//

#ifndef multinormal_h
#define multinormal_h

#include <boost/numeric/ublas/matrix.hpp>

/** */

/** This class is responsible for sampling from a multivariate normal distribution. */
class MultiNormal {
    
    public:
    
        /*
         * Performs the Cholesky decomposition decomposition on a matrix; that is finding a matrix that when multiplied by the transpose of that matrix
         * produces the original matrix.
         * Algorithm details can be found at https://en.wikipedia.org/wiki/Cholesky_decomposition#The_Cholesky.E2.80.93Banachiewicz_and_Cholesky.E2.80.93Crout_algorithms
         *
         * @param A
         * The input matrix, A in the formula.
         *
         * @return
         * The output matrix such that return * transpose(return) will result in A.
         */
        static boost::numeric::ublas::matrix<float> choleskyDecomposition(const boost::numeric::ublas::matrix<float>& A);
    
    
    
    
    
    
    
};

#endif /* multinormal_h */
