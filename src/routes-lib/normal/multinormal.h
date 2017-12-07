//
//  multinormal.h
//  Routes
//
//  Created by Logan on 11/17/17.
//

#ifndef multinormal_h
#define multinormal_h

#include <chrono>
#include <glm/glm.hpp>
#include <vector>

#include "normal_gen.h"

/** */

/** This class is responsible for sampling from a multivariate normal distribution. */
class MultiNormal {
    
    public:
    
        /**
         * A simple default constructor for creating a multivariate normal distribution.
         * Once the covariance matrix and mean vector are set they cannot be changed.
         *
         * @param covariance_matrix
         * The covariance matrix of the multivariate normal distribution
         *
         * @param m
         * The mean vector of the multivariate normal distribution
         *
         * @param sigma
         * The step size of the distribution
         *
         */
        MultiNormal(const Eigen::MatrixXf& covariance_matrix, const Eigen::VectorXf& sigma);

        /**
         * Fills the out vector with samples from this distribution.
         *
         * @param out
         * The output vector for the samples. It is assumed that all elements of the vector are initialized to the correct dimensions
         *
         * @param sampler
         * A generator that we can pull standard normal random samples from.
         */
        void generateRandomSamples(std::vector<Eigen::VectorXf>& out, SampleGenerator& sampler);

        /**
         * Fills the out vector with samples from this distribution.
         * This version of the function utilizes multiple threads and multiple SampleGenerators
         *
         * @param out
         * The output vector for the samples. It is assumed that all elements of the vector are initialized to the correct dimensions
         *
         * @param samplers
         * Several generators that we can pull standard normal random samples from.
         * The size of this vector determines the number of threads used.
         */
         void generateRandomSamples(std::vector<Eigen::VectorXf>& out, std::vector<SampleGenerator*> samplers);
    
    private:
    
        /**
         * Generates a random sample from the distribution.
         *
         * @param out_sample
         * The resulting N-dimension vector that represents a random sample from the multivariate normal distribution.
         *
         * @param sampler
         * A generator that we can pull standard normal random samples from.
         *
         */
        void doSample(Eigen::VectorXf& out_sample, SampleGenerator& sampler);
    
        /**
         * Represents the decomposition of the covariance matrix. Multiplying a vector with independent random samples from the
         * standard normal distribution will yeild a sample from the multivariate normal distribution.
         */
        Eigen::MatrixXf _A;
    
        /** The step size of the distribution */
        Eigen::VectorXf _sigma;
    
};

#endif /* multinormal_h */
