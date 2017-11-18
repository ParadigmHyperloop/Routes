//
//  multinormal.cpp
//  Routes
//
//  Created by Logan on 11/17/17.
//

#include "multinormal.h"

boost::numeric::ublas::matrix<float> MultiNormal::choleskyDecomposition(const boost::numeric::ublas::matrix<float>& A) {
    
    // First create the output matrix L
    boost::numeric::ublas::matrix<float> L = boost::numeric::ublas::zero_matrix<float>(A.size1(), A.size2());
    
    // Loop through each of the rows
    for (int i = 0; i < A.size1(); i++) {
        
        // Go through every number in the row
        for (int j = 0; j <= i; j++) {
            
            // First formula, perform sumnation
            float sum = 0.0;
            
            for (int k = 0; k < j; k++)
                sum += L(i, k) * L(j, k);
            
            // Two different things are done depending on if this is a diagonal, that is i == j
            float result;
            if (i == j)
                result = sqrt(A(j, j) - sum);
            else
                result = (A(i, j) - sum) * (1.0 / L(j, j));
            
            L(i, j) = result;
            
        }
        
    }
    
    return L;

}
