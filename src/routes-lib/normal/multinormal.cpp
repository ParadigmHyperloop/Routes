//
//  multinormal.cpp
//  Routes
//
//  Created by Logan on 11/17/17.
//

#include "multinormal.h"

std::mt19937 MultiNormal::_twister;
std::normal_distribution<float> MultiNormal::_standard_distro(0.0, 1.0);

MultiNormal::MultiNormal(const Eigen::MatrixXf& covariance_matrix, const Eigen::VectorXf& sigma) : _sigma(sigma) {
    
    // Update the twister to make sure we get consistantly random results
    // Hashing makes everything better :)
    std::hash<int> hasher;
    _twister = std::mt19937(hasher(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    
    // Decompose the covariance matrix
    Eigen::LLT<Eigen::MatrixXf> decomp(covariance_matrix);
    
    if (decomp.info()==Eigen::Success)
        _A = decomp.matrixL();
    else {
      
        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> eigen_decomp(covariance_matrix);
        _A = eigen_decomp.eigenvectors() * eigen_decomp.eigenvalues().cwiseSqrt().asDiagonal();
        
    }
    
}

std::vector<Eigen::VectorXf> MultiNormal::generateRandomSamples(int count) {
 
    // Create a vector and fill it with samples
    std::vector<Eigen::VectorXf> samples(count);
    
    for (int i = 0; i < count; i++) {
        
        // Make sure the vector is initialized to the right size
        samples[i] = Eigen::VectorXf(_sigma.size());
        doSample(samples[i]);
        
    }
    
    return samples;
    
}

void MultiNormal::doSample(Eigen::VectorXf& out_sample) {
    
    // First we fill the vector with randomly generated values from the standard distribution
    for (int i = 0; i < out_sample.size(); i++)
        out_sample(i) = _standard_distro(_twister);
    
    // Transform the sample by the decomposed covariance and then add the mean vector
    out_sample = (_A * out_sample).cwiseProduct(_sigma);
    
}
