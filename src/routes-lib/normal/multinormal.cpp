//
//  multinormal.cpp
//  Routes
//
//  Created by Logan on 11/17/17.
//

#include "multinormal.h"

MultiNormal::MultiNormal(const Eigen::MatrixXf& covariance_matrix, const Eigen::VectorXf& sigma, SampleGenerator& sampler) : _sigma(sigma), _sampler(sampler) {

    // Decompose the covariance matrix
    Eigen::LLT<Eigen::MatrixXf> decomp(covariance_matrix);
    
    if (decomp.info() == Eigen::Success)
        _A = decomp.matrixL();
    else {
      
        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> eigen_decomp(covariance_matrix);
        _A = eigen_decomp.eigenvectors() * eigen_decomp.eigenvalues().cwiseSqrt().asDiagonal();
        
    }
    
}

std::vector<Eigen::VectorXf> MultiNormal::generateRandomSamples(int count) {
 
    // Create a vector and fill it with _samples
    std::vector<Eigen::VectorXf> samples(count);
    
    for (int i = 0; i < count; i++) {
        
        // Make sure the vector is initialized to the right size
        samples[i] = Eigen::VectorXf(_sigma.size());
        doSample(samples[i]);
        
    }
    
    return samples;
    
}

void MultiNormal::generateRandomSamples(std::vector<Eigen::VectorXf>& out) {

    // Overwrite everything in the vector
    for (int i = 0; i < out.size(); i++)
        doSample(out[i]);

}

void MultiNormal::doSample(Eigen::VectorXf& out_sample) {
    
    // Grab a sample
    _sampler.getSample(out_sample);
    
    // Transform the sample by the decomposed covariance and then add the mean vector
    out_sample = (_A * out_sample).cwiseProduct(_sigma);
    
}
