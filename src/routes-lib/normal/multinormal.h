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
         * The covarience matrix of the multivariate normal distribution
         *
         * @param m
         * The mean vector of the multivariate normal distribution
         */
        MultiNormal(const Eigen::MatrixXf& covariance_matrix, Eigen::VectorXf m);
    
        /**
         * Samples from the distribution count number of times.
         *
         * @param count
         * The number of samples to return in the resulting vector
         */
        std::vector<Eigen::VectorXf> generateRandomSamples(int count);
    
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
         * independant standard normal distributions, we only need one normal distribution to do any sampling.
         * It has mean 0 and a standard deviation of 1.
         */
        static std::normal_distribution<float> _standard_distro;
    
        /**
         * Represents the decomposition of the covariance matrix. Multiplying a vector with independent random samples from the
         * standard normal distribution will yeild a sample from the multivariate normal distribution.
         */
        Eigen::MatrixXf _A;
    
        /** The mean vector of the multivariate normal distribution. */
        Eigen::VectorXf _m;
    
};

#endif /* multinormal_h */
