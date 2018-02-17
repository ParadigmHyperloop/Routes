//
// Created by Logan on 10/12/17.
//

#include "population.h"

Population::Population(int pop_size, glm::vec4 start, glm::vec4 dest, const ElevationData& data) : _pop_size(pop_size), _start(start),
    _dest(dest), _direction(_dest - _start), _data(data) {

    // Figure out how many points we need for this route
    calcGenomeSize();

    // Figure out the number of vectors that make up the entire individual. This is the header, the start
    // and the destination
    _individual_size = _genome_size + 2 + 1;

    // Calculate the binomial coefficients for evaluating the bezier paths
    calcBinomialCoefficients();
        
    // Get the data to allow for proper texture sampling
    _data_size   = _data.getCroppedSizeMeters();
    _data_origin = _data.getCroppedOriginMeters();

    determineEvalPoints();
        
    // First we init the params, then generate a starter population
    initParams();
    initSamplers();
    initSamples();
    samplePopulation();
        
    // Create the best sample vector
    _best_samples = std::vector<Eigen::VectorXf>((size_t)_mu);

}

Population::~Population() {

    // Delete the sample generators
    for (int i = 0; i < NUM_SAMPLE_THREADS; i++)
        delete _sample_gens[i];

}

Individual Population::getIndividual(int index) {

    Individual ind = Individual();
    ind.num_genes = (size_t)_genome_size;

    // Calculate the location of the parts of the individual
    glm::vec4* header_loc = _individuals.data() + index * _individual_size;
    ind.header = header_loc;

    // Account for the header
    ind.path = header_loc + 1;
    ind.genome = header_loc + 2;
    
    ind.index = index;

    return ind;

}

void Population::step(const Pod& pod) {

    // Evaluate the cost and sort so the most fit solutions are in the front
    evaluateCost(pod);

    sortIndividuals();

    // Update the params
    updateParams();

    // Sample a new generation
    samplePopulation();

}

void Population::sortIndividuals() {

    // Get all of the individuals as Individuals (struct) for easier sorting
    for (int i = 0; i < _pop_size; i++)
        _sorted_individuals[i] = getIndividual(i);

    // Sort the array of individual structs
    std::sort(_sorted_individuals.begin(), _sorted_individuals.end(), [](Individual a, Individual b){

        // Compare costs in the header
        return (*a.header).x < (*b.header).x;

    });
    
    // Save the fitness value of the best individual
    _fitness_over_generations.push_back(_sorted_individuals[0].header->x);
    
    // Copy the best samples into a sorted array
    for (int i = 0; i < _mu; i++)
        _best_samples[i] = _samples[_sorted_individuals[i].index];

}

void Population::evaluateCost(const Pod& pod) {

    // Get stuff we need to execute a kernel on
    boost::compute::command_queue& queue = Kernel::getQueue();

    // Create a temporary kernel and execute it
    static Kernel kernel = Kernel(std::ifstream("../opencl/kernel_cost.opencl"), "cost");
    kernel.setArgs(_data.getOpenCLImage(), _opencl_individuals.get_buffer(), _genome_size + 2,
                   MAX_SLOPE_GRADE, pod.minCurveRadius(), EXCAVATION_DEPTH, _data_size.x,
                   _data_size.y, _opencl_binomials.get_buffer(),
                   _num_evaluation_points_1, _num_evaluation_points / NUM_ROUTE_WORKERS, _data_origin.x, _data_origin.y, glm::length(_direction));

    // Upload the data
    boost::compute::copy(_individuals.begin(), _individuals.end(), _opencl_individuals.begin(), queue);

    // Execute the 2D kernel with a work size of NUM_ROUTE_WORKERS. NUM_ROUTE_WORKERS threads  will work on a single individual
    kernel.execute2D(glm::vec<2, size_t>(0, 0),
                     glm::vec<2, size_t>(_pop_size, NUM_ROUTE_WORKERS),
                     glm::vec<2, size_t>(1, NUM_ROUTE_WORKERS));

    // Download the data
    boost::compute::copy(_opencl_individuals.begin(), _opencl_individuals.end(), _individuals.begin(), queue);

}

std::vector<glm::vec3> Population::getSolution() const {
    
    std::vector<glm::vec3> solution = std::vector<glm::vec3>((size_t)_genome_size + 2);
    solution[0]                     = glm::vec3(_start.x, _start.y, _start.z);
    solution[_genome_size + 1]      = glm::vec3(_dest.x, _dest.y, _dest.z);
    
    for (int i = 0; i < _genome_size; i++)
        solution[i + 1] = glm::vec3(_mean(i * 3    ),
                                    _mean(i * 3 + 1),
                                    _mean(i * 3 + 2));
    
    return solution;
    
}

void Population::calcGenomeSize() {

    // The genome size has a square root relationship with the length of the route
    float sqrt_length = sqrtf(glm::length(_direction));
    _genome_size = (int)std::round(sqrt_length * LENGTH_TO_GENOME);

    std::cout << "Genome: " << _genome_size << std::endl;

}

void Population::determineEvalPoints() {
    
    // Figure out how many points this route should be evaluated on.
    // We also make sure it is a multiple of workers
    _num_evaluation_points = glm::max((int)ceil(glm::max(_data_size.x / METERS_TO_POINT_CONVERSION,
                                                         _data_size.y / METERS_TO_POINT_CONVERSION)
                                                                      / (float)NUM_ROUTE_WORKERS) * NUM_ROUTE_WORKERS,
                                                         2400);
    
    _num_evaluation_points_1 = (float)_num_evaluation_points - 1.0f;
    
}

void Population::initParams() {

    // Choose mu to be a fixed number of individuals
    _mu = (int)(_pop_size * 0.15);

    // Init the mean to the best guess (a straight line)
    bestGuess();

    // Calculate the weights for the selected population (mu)
    calcWeights();

    // Covariance matrix starts as the identity matrix
    // Multiply by three to make sure that we have room for X, Y and Z
    _covar_matrix = Eigen::MatrixXf::Identity(_genome_size * 3, _genome_size * 3);

    // Calculate the initial step size
    calcInitialSigma();

    // Calculate strat params
    calculateStratParameters();

    // Set the two evolution paths to the zero vector
    _p_covar = Eigen::VectorXf::Zero(_genome_size * 3);
    _p_sigma = Eigen::VectorXf::Zero(_genome_size * 3);
    
    // Calculate the expected value of the standard normal distribution.
    // This is purely based of the number of dimensions.
    float N = _mean.size();
    _expected_value = sqrtf(N) * (1.0f - 1.0f / (N * 4.0f) + 1.0f / (21.0f * glm::pow(N, 2.0f)));

}

void Population::initSamplers() {

    // Create the standard normal samplers
    _sample_gens = std::vector<SampleGenerator*>(NUM_SAMPLE_THREADS);
    
    for (int i = 0; i < NUM_SAMPLE_THREADS; i++)
        _sample_gens[i] = new SampleGenerator(_genome_size * 3, _pop_size / NUM_SAMPLE_THREADS);

}

void Population::initSamples() {

    // Create the appropriate vectors
    _individuals = std::vector<glm::vec4>((size_t)_pop_size * _individual_size);
    _sorted_individuals = std::vector<Individual>((size_t)_pop_size);

    for (int i = 0; i < _individuals.size(); i++)
        _individuals[i] = glm::vec4(0.0);

    _opencl_individuals =  boost::compute::vector<glm::vec4>(_individuals.size(), Kernel::getContext());

    // Samples should be the same size as the population
    _samples = std::vector<Eigen::VectorXf>((size_t)_pop_size);

    for (int i = 0; i < _pop_size; i++)
        _samples[i] = Eigen::VectorXf::Zero(_genome_size * 3);

    // Make sure that individuals have the start and destination all set up. This will never change so we can do it
    // once.
    for (int i = 0; i < _pop_size; i++) {

        _individuals[i * _individual_size + 1] = _start;
        _individuals[i * _individual_size + 2 + _genome_size] = _dest;

    }

}

void Population::bestGuess() {

    // We choose a linear spacing of points along a straight line from the start to destination
    _mean = Eigen::VectorXf(_genome_size * 3);

    for (int i = 1; i <= _genome_size; i++) {

        // Figure out how far along the direction line we are
        float percent = (float)i / (float)(_genome_size + 1);

        // Set the mean
        int i_adjusted = (i - 1) * 3;
        _mean(i_adjusted    ) = _start.x + _direction.x * percent;
        _mean(i_adjusted + 1) = _start.y + _direction.y * percent;
        _mean(i_adjusted + 2) = _start.z + _direction.z * percent;

    }

}

void Population::calcWeights() {

    // When we update the mean and covariance matrix, we weight each solution unevenly.
    // Sum of all values in _weights should equal 1
    // Sum of 1/pow(_weights, 2) should be about _pop_size / 4
    _weights = std::vector<float>((size_t)_mu);
    float sum = 0.0f;

    for (int i = 0; i < _mu; i++) {

        _weights[i] = log(_mu + 0.5f) - log(i + 1.0f);
        sum += _weights[i];

    }

    // Normalize to make sure that it adds up to 1
    for (int i = 0; i < _mu; i++)
        _weights[i] /= sum;

    // Calculate _mu_weight to be the sum of 1 / pow(_weights, 2)
    sum = 0.0f;

    for (int i = 0; i < _mu; i++)
        sum += glm::pow(_weights[i], 2.0f);

    // Save some constant params we need to keep
    _mu_weight = 1.0f / sum;
    _mu_weight_sqrt = sqrtf(_mu_weight);

}

void Population::calcInitialSigma() {

    // Initialize sigma
    _sigma = Eigen::VectorXf(_genome_size * 3);

    // First we figure out what the actual values should be
    glm::vec3 sigma_parts = glm::vec3(INITIAL_SIGMA_XY,
                                      INITIAL_SIGMA_XY,
                                      (_data.getMaxElevation() - _data.getMinElevation()) / INITIAL_SIGMA_DIVISOR);

    // Apply it to the X Y and Z for each point
    for (int i = 0; i < _genome_size; i++) {

        _sigma(i * 3    ) = sigma_parts.x;
        _sigma(i * 3 + 1) = sigma_parts.y;
        _sigma(i * 3 + 2) = sigma_parts.z;

    }

}

void Population::calculateStratParameters() {

    float N = _mean.size();

    // Calc c_sigma
    _c_sigma = 3.0 / N;

    // Calc c_covar
    _c_covar = 4.0 / N;

    // Calculate a few other params for the covariance matrix updating
    _c1 = 2.0f / (N * N);
    _c_mu = _mu_weight / (N * N);

}

void Population::samplePopulation() {

    // Create a MND
    MultiNormal dist = MultiNormal(_covar_matrix, _sigma);
    dist.generateRandomSamples(_samples, _sample_gens);
    
    // Convert the _samples over to a set of glm vectors and update the population
    // Do so with multiple threads
    std::vector<std::thread> threads = std::vector<std::thread>(NUM_SAMPLE_THREADS);
    int worker_size = _pop_size / NUM_SAMPLE_THREADS;
    
    for (int i = 0; i < NUM_SAMPLE_THREADS; i++) {
        
        threads[i] = std::thread([this, i, worker_size] {

            int start = worker_size * i;
            int end = glm::min(start + worker_size, _pop_size);

            for (int u = start; u < end; u++) {

                // Add the mean because the samples don't have it
                Eigen::VectorXf actual = _samples[u] + _mean;

                // Use memory copies to put the right data in the the _individuals vector because its slightly faster
                // We need to do it in a for loop because the _individuals is vec4 and there are only 3 components for each control point
                // In the Eigen vectors that we build
                for (int p = 0; p < _genome_size; p++)
                    memcpy(&_individuals[u * _individual_size + 2 + p][0], actual.data() + p * 3, sizeof(float) * 3);

            }

        });
        
    }
    
    // Make sure all of the threads finish
    for (int i = 0; i < NUM_SAMPLE_THREADS; i++)
        threads[i].join();

}

void Population::updateParams() {
    
    // Update the mean
    updateMean();
    
    // Update the step size path
    updatePSigma();

    // Update the covariance matrix path
    updatePCovar();

    // Update the covariance matrix
    updateCovar();
    
    // Update the step size
    updateSigma();
    
}

void Population::updateMean() {

    // Save the mean from the last gen so we can use it to update the paths
    Eigen::VectorXf _mean_prime = _mean;

    _mean = Eigen::VectorXf::Zero(_mean.size());

    for (int i = 0; i < _mu; i++)
        _mean += _best_samples[i] * _weights[i];
    
    _mean += _mean_prime;

    // Calculate mean displacement
    _mean_displacement = (_mean - _mean_prime);
    
    // Divide by sigma
    for (int i = 0; i < _mean_displacement.size(); i++)
        _mean_displacement(i) = _mean_displacement(i) / _sigma(i);
    
}

void Population::updatePSigma() {

    // Calculate the discount factor and its complement
    float discount = 1.0f - _c_sigma;
    float discount_comp = sqrtf(1.0f - (discount * discount));

    // Get the inverse square root of the covariance matrix
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> solver(_covar_matrix);
    Eigen::MatrixXf inv_sqrt_C(solver.operatorInverseSqrt());

    _p_sigma = discount * _p_sigma + discount_comp * _mu_weight_sqrt * inv_sqrt_C * _mean_displacement;

}

void Population::updatePCovar() {

    // Calculate the discount factor and its complement
    float discount = 1.0f - _c_covar;
    float discount_comp = sqrtf(1.0f - (discount * discount));

    // Figure out the indicator function
    float indicator = 0.0f;
    if (_p_sigma.norm() <= sqrtf((float)_mean.size()) * ALPHA)
        indicator = 1.0;

    _p_covar = discount * _p_covar + indicator * discount_comp * _mu_weight_sqrt * _mean_displacement;

}

void Population::updateCovar() {

    // Calculate the actual new covariance matrix
    Eigen::MatrixXf covariance_prime = Eigen::MatrixXf::Zero(_mean.size(), _mean.size());

    for (int i = 0; i < _mu; i++) {

        Eigen::VectorXf adjusted = _best_samples[i];
        
        // Divide by sigma
        for (int k = 0; k < adjusted.size(); k++)
            adjusted(k) = adjusted(k) / _sigma(k);
        
        covariance_prime += adjusted * adjusted.transpose() * _weights[i];

    }

    // Calculate the rank one matrix
    Eigen::MatrixXf rank_one = _c1 * _p_covar * _p_covar.transpose();

    // Calculate cs
    float indicator = 0.0f;
    if (_p_sigma.norm() * _p_sigma.norm() <= sqrtf((float)_mean.size()) * ALPHA)
        indicator = 1.0;

    float cs = (1.0f - indicator) * _c1 * _c_covar * (2.0f - _c_covar);

    // Calculate the discount factor
    float discount = 1.0f - _c1 - _c_mu + cs;

    // Update the MatrixXf
    _covar_matrix = discount * _covar_matrix + rank_one + _c_mu * covariance_prime;

}

void Population::updateSigma() {

    // Calculate ratio between the decay and the dampening
    float ratio = _c_sigma / STEP_DAMPENING;

    // Get the magnitude of the sigma path
    float mag = _p_sigma.norm();

    // Update sigma
    _sigma = _sigma * glm::pow(M_E, ratio * (mag / _expected_value - 1.0f));

}

void Population::calcBinomialCoefficients() {

    // For degree we have _genome_size + 2 points, so we use that minus 1 for the degree
    const std::vector<int>& binomials = Bezier::getBinomialCoefficients(_genome_size + 1);
    _opencl_binomials = boost::compute::vector<int>((size_t)_genome_size + 2, Kernel::getContext());

    // Upload to the GPU
    boost::compute::copy(binomials.begin(), binomials.end(), _opencl_binomials.begin(), Kernel::getQueue());

}
