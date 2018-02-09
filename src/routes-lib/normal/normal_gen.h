//
// Created by logan on 12/6/2017.
//

#ifndef ROUTES_NORMAL_GEN_H
#define ROUTES_NORMAL_GEN_H

#include <ctime>
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <thread>

/** */

/**
 * Because sampling from a multivariate normal distribution is so slow and all require samples from the standard
 * normal distribution, we can pull the standard random samples from another thread so they are always ready.
 * This class will store N random samples of the desired length from the standard normal distribution.
 *
 * It is assumed that there will only be a single reader asking for samples.
 *
 */
class SampleGenerator {

    public:

        /**
         * Creates a new sample generator and launches the thread for it.
         *
         * @param length
         *  The number of components each sample should have.
         *
         * @param retainer
         * The number of samples that should be retained at any time.
         *
         */
        SampleGenerator(int length, int retainer);

        /** Ensures that the samples are no longer generated when this is destroyed by stopping the thread */
        ~SampleGenerator();

        /**
         * Copies a sample that has already been generated to the destination vector. It is assumed that the
         * destination is the same length as _length.
         *
         * @param to_place
         * The reference to the vector that should be populated.
         */
        void getSample(Eigen::VectorXf& to_place);

    private:

        /* Handles the generation of new sample on another thread */
        void generateSamples();

        /**
         * The queue that the samples are placed into.
         * We use the boost lockfree spsc_queue because it is FIFO for a single reader and writer.
         */
        boost::lockfree::spsc_queue<Eigen::VectorXf>* _queue;

        /** The number of components each sample should have. */
        int _length;

        /** Whether or not samples should still be generated on the thread */
        std::atomic<bool> _should_gen_samples;

        /** The thread that sampling is performed on */
        std::thread _sample_thread;

        /** A Mersenne Twister object (a pseudorandom generator) that we pull the samples from. */
        boost::mt19937 _twister;

        /**
         * The normal distribution that we sample from, AKA the standard normal distribution.
         * This is by definition N(0, 1)
         */
        boost::normal_distribution<float> _standard_distro;

};

#endif //ROUTES_NORMAL_GEN_H
