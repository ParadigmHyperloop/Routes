//
// Created by logan on 12/6/2017.
//

#include "normal_gen.h"

std::hash<int> hasher;
SampleGenerator::SampleGenerator(int length, int retainer) : _queue(new boost::lockfree::spsc_queue<Eigen::VectorXf>(retainer)), _length(length),
                                                             _twister(hasher(std::chrono::high_resolution_clock::now().time_since_epoch().count())),
                                                             _standard_distro(0.0, 1.0) {

    // Start the sample thread
    _sample_thread = std::thread([this]{ generateSamples(); });
                                                                 
}

SampleGenerator::~SampleGenerator() {

    // Stop the sample thread and clean up the queue
    _should_gen_samples = false;
    _sample_thread.join();

    delete _queue;

}

void SampleGenerator::getSample(Eigen::VectorXf& to_place) {

    // More than likely we are going to get a sample, but we need to ensure it.
    // If we failed to get one generate one on the calling thread
    if (!_queue->pop(to_place)) {

        for (int i = 0; i < _length; i++)
            to_place(i) = _standard_distro(_twister);

    }

}

void SampleGenerator::generateSamples() {

    // Loop until our services are no longer required
    while (_should_gen_samples) {

        // Generate as many samples as we need
        // Because there is no way to see if we are full we generate samples until we get a failure
        while (true) {

            // Generate a new sample
            Eigen::VectorXf sample = Eigen::VectorXf(_length);
            for (int i = 0; i < _length; i++)
                sample(i) = _standard_distro(_twister);

            // Try to push and if we cant it means the queue is full and we can stop
            if (!_queue->push(sample))
                break;

        }

        // Sleep for a bit
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    }

}
