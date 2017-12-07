//
//  multinormal.cpp
//  Routes
//
//  Created by Logan on 11/17/17.
//

#include "multinormal.h"

MultiNormal::MultiNormal(const Eigen::MatrixXf& covariance_matrix, const Eigen::VectorXf& sigma) : _sigma(sigma) {

    // Decompose the covariance matrix
    Eigen::LLT<Eigen::MatrixXf> decomp(covariance_matrix);
    
    if (decomp.info() == Eigen::Success)
        _A = decomp.matrixL();
    else {
      
        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> eigen_decomp(covariance_matrix);
        _A = eigen_decomp.eigenvectors() * eigen_decomp.eigenvalues().cwiseSqrt().asDiagonal();
        
    }
    
}

void MultiNormal::generateRandomSamples(std::vector<Eigen::VectorXf>& out, SampleGenerator& sampler) {

    // Overwrite everything in the vector
    for (int i = 0; i < out.size(); i++)
        doSample(out[i], sampler);

}

void MultiNormal::doSample(Eigen::VectorXf& out_sample, SampleGenerator& sampler) {
    
    // Grab a sample
    sampler.getSample(out_sample);
    
    // Transform the sample by the decomposed covariance and then add the mean vector
    out_sample = (_A * out_sample).cwiseProduct(_sigma);
    
}

void MultiNormal::generateRandomSamples(std::vector<Eigen::VectorXf>& out, std::vector<SampleGenerator*> samplers) {

    // Determine params
    int num_workers = samplers.size();
    int work_size = out.size() / num_workers;

    std::vector<std::thread> threads (num_workers);

    for (int i = 0; i < num_workers; i++) {

        threads[i] = std::thread([this, i, &out, &samplers, work_size] {

            int start = i * work_size;
            int end = glm::min(start + work_size, (int)out.size());

            for (int p = start; p < end; p++)
                doSample(out[p], *samplers[i]);

        });

    }

    // Make sure everything finishes
    for (int i = 0; i < num_workers; i++)
        threads[i].join();

}