//
//  multinormal.h
//  Routes
//
//  Created by Logan on 11/17/17.
//

#ifndef multinormal_h
#define multinormal_h

#include <chrono>
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <random>
#include <vector>

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
         */
        MultiNormal(const Eigen::MatrixXf& covariance_matrix, const Eigen::VectorXf& sigma);
    
        /**
         * Samples from the distribution count number of times.
         *
         * @param count
         * The number of samples to return in the resulting vector
         *
         * @return
         * The a vector of size count containing samples from the distribution
         */
        std::vector<Eigen::VectorXf> generateRandomSamples(int count);

        /**
         * Samples from the distribution count number of times.
         *
         * @param out
         * The output vector for the samples. It is assumed that all elements of the vector are initialized to the correct dimensions
         */
        void generateRandomSamples(std::vector<Eigen::VectorXf>& out);
    
    private:
    
        /**
         * Generates a random sample from the distribution.
         *
         * @param out_sample
         * The resulting N-dimension vector that represents a random sample from the multivariate normal distribution.
         */
        void doSample(Eigen::VectorXf& out_sample);
    
        /**
         * A Mersenne Twister object that is shared between all MultiNormal objects. The seed is updated whenever a new
         * instance of MultiNormal is created.
         */
        static std::mt19937 _twister;
    
        /**
         * Since sampling from a multivariate normal distribution basically is a transformation of a bunch of samples from
         * independent standard normal distributions, we only need one normal distribution to do any sampling.
         * It has mean 0 and a standard deviation of 1.
         */
        static std::normal_distribution<float> _standard_distro;
    
        /**
         * Represents the decomposition of the covariance matrix. Multiplying a vector with independent random samples from the
         * standard normal distribution will yeild a sample from the multivariate normal distribution.
         */
        Eigen::MatrixXf _A;
    
        /** The step size of the distribution */
        Eigen::VectorXf _sigma;
    
};

#endif /* multinormal_h */
