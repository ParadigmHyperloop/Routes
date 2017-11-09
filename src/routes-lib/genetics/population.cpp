//
// Created by Logan on 10/12/17.
//

#include "population.h"

Population::Population(int pop_size, int genome_size, glm::vec4 start, glm::vec4 dest, const ElevationData& data) : _data(data) {

    // Save constants
    _pop_size = pop_size;
    _genome_size = genome_size;
    _start = start;
    _dest = dest;
    _direction = _dest - _start;

    // Figure out the number of vectors that make up the entire individual. This is the header, the start
    // and the destination
    _individual_size = _genome_size + 2 + 1;

    // Create the appropriate vectors
    _individuals = std::vector<glm::vec4>(pop_size * _individual_size);
    _opencl_individuals =  boost::compute::vector<glm::vec4>(_individuals.size(), Kernel::getContext());

    // Calculate the binomial coefficients for evaluating the bezier paths
    calcBinomialCoefficients();

    // Make a dummy genome
    dummy_genome = new glm::vec4[genome_size];

    // Figure out how many points this route should be evaluated on.
    // We also make sure it is a multiple of workers
    _num_evaluation_points = ceil(glm::max(data.getHeightInMeters() / METERS_TO_POINT_CONVERSION,
                                           data.getHeightInMeters() / METERS_TO_POINT_CONVERSION) / 50.0) * 50;
    
    std::cout << "Using " << _num_evaluation_points << " points of evaluation" << std::endl;
    _num_evaluation_points_1 = (float)_num_evaluation_points - 1.0f;

    // Generate the population
    generatePopulation();

}

Population::~Population() { delete dummy_genome; }

Individual Population::getIndividual(int index) {

    Individual ind;
    ind.num_genes = _genome_size;

    // Calculate the location of the parts of the individual
    glm::vec4* header_loc = _individuals.data() + index * _individual_size;
    ind.header = header_loc;

    // Account for the header
    ind.path = header_loc + 1;
    ind.genome = header_loc + 2;

    return ind;

}

void Population::sortIndividuals() {

    // Get all of the individuals as Individuals for easier sorting
    std::vector<Individual> individuals_s = std::vector<Individual>(_pop_size);
    for (int i = 0; i < _pop_size; i++)
        individuals_s[i] = getIndividual(i);

    // Sort the array of individual structs
    std::sort(individuals_s.begin(), individuals_s.end(), [](Individual a, Individual b){

        // Compare costs in the header
        return (*a.header).x < (*b.header).x;

    });

    // Create a new vector so we don't destroy any data
    std::vector<glm::vec4> sorted_individuals = std::vector<glm::vec4>(_individuals.size());
    for (int i = 0; i < _pop_size; i++) {

        // Copy the sorted individual into the new array
        memcpy(sorted_individuals.data() + i * _individual_size, individuals_s[i].header, sizeof(glm::vec4) * _individual_size);

    }

    // Save the sorted array
    _individuals = sorted_individuals;

}

void Population::breedIndividuals() {



    // Get a random number of mother and fathers from the top 20%
    // Ensure there is 1 mother and 1 father every time though
    int mothers = (int)(_pop_size * 0.2);
    int fathers = (int)(_pop_size * 0.2);


    std::vector<glm::vec4> new_population = std::vector<glm::vec4>(_individuals.size());

    // Breed 80% of the population from the mother and father
    int i;
    for (i = 0; i < (_pop_size * 0.8); i++) {


        int mom = glm::linearRand(0, mothers);
        int dad = glm::linearRand(0, fathers);

        glm::vec4* bred = crossoverIndividual(mom, dad);
        memcpy(new_population.data() + (i * _individual_size + 2), bred, sizeof(glm::vec4) * _genome_size);

        // Set the start and the destination
        new_population[i * _individual_size + 1] = _start;

        // Set the start and the destination
        new_population[i * _individual_size + 2 + _genome_size] = _dest;

    }

    // Generate new individuals (20%)
    for (; i < _pop_size; i++) {

        // Adds + 1 to ignore the header
        int individual_start = i * _individual_size + 2;

        for (int j = 0; j < _genome_size; j++)
            generateRandomPoint(new_population[individual_start + j]);

        // Set the start and the destination
        new_population[individual_start - 1] = _start;

        // Set the start and the destination
        new_population[individual_start + _genome_size] = _dest;

    }

    // Save the new population
    _individuals = new_population;

    // Copy to GPU
    boost::compute::copy(new_population.begin(), new_population.end(), _opencl_individuals.begin(), Kernel::getQueue());

}

void Population::evaluateCost(const Pod& pod) {

    // Get stuff we need to execute a kernel on
    boost::compute::command_queue& queue = Kernel::getQueue();

    // Generate the program sounds
    static const std::string source = BOOST_COMPUTE_STRINGIZE_SOURCE(

        // Evaluates the bezier curve made by the given control points and start and dest at parametric value s
        float4 evaluateBezierCurve(__global float4* controls, int offset, int points, float s, __global int* binomial_coeffs) {

            float one_minus_s = 1.0 - s;

            // Degree is num points - 1
            int degree = points - 1;

            float4 out_point = (float4)(0.0, 0.0, 0.0, 0.0);

            // Middle terms, iterate for num points
            for (int i = 0; i < points; i++) {

                // Evaluate for x y and z
                float multiplier = pown(one_minus_s, degree - i) * pown(s, i);

                // We subtract one here so that we use the correct control point since i starts at 1
                out_point += controls[i + offset] * multiplier * (float)binomial_coeffs[i];

            }

            return out_point;

        }

        // This calculates the approximates curvature at a given point (p1)
        float curvature(float4 p0, float4 p1, float4 p2) {

            // Calculate the approximate first derivatives
            float4 der_first0 = p1 - p0;
            float4 der_first1 = p2 - p1;

            // Get the second derivative
            float4 der_second = der_first1 - der_first0;

            // Calculate the denominator and numerator
            float denom = length(cross(der_first0, der_second));
            float num = pown(length(der_first0), 3);

            return num / denom;

        }

        // Computes the cost of a path
        __kernel void cost(__read_only image2d_t image, __global float4* individuals, int path_length,
                           float max_grade_allowed, float min_curve_allowed, float excavation_depth, float width,
                           float height, __global int* binomial_coeffs,
                           float num_points_1, int points_per_worker) {

            const sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
            const float pylon_cost = 116.0;
            const float tunnel_cost = 310000.0;

            __local float min_curves [50];
            __local float max_grades [50];
            __local float track_costs[50];

            // Get an offset to the gnome
            size_t i = get_global_id(0);
            size_t w = get_local_id(1);

            int path = i * (path_length + 1) + 1;

            float min_curve = 10000000000000000.0;

            float track_cost = 0.0;
            float steepest_grade = 0.0;

            // Figure out where to start and end
            int start = w * points_per_worker;
            int end = start + points_per_worker;

            float4 last_last = individuals[path];
            float4 last_point = individuals[path];

            if (w) {

                last_last =  evaluateBezierCurve(individuals, path, path_length, (float)(start - 2) / num_points_1, binomial_coeffs);
                last_point = evaluateBezierCurve(individuals, path, path_length, (float)(start - 1) / num_points_1, binomial_coeffs);

            }

            for (int p = start; p <= end; p++) {

                float4 bezier_point = evaluateBezierCurve(individuals, path, path_length, (float)p / num_points_1, binomial_coeffs);

                // Get pylon height with a sample from the image
                float2 nrm_device = (float2)(bezier_point.x / width, bezier_point.y / height);
                float height = read_imagef(image, sampler, nrm_device).x;

                // Compute spacing, only x and y distance
                float spacing = sqrt(pown(bezier_point.x - last_point.x, 2) + pown(bezier_point.y - last_point.y, 2));

                // Get curvature
                if (p > 1)
                    min_curve = min(min_curve, curvature(last_last, last_point, bezier_point));

                // Compute grade if the points had spacing
                if (spacing) {

                    steepest_grade = max(steepest_grade, fabs(bezier_point.z - last_point.z) / spacing);

                    // Compute track cost
                    float pylon_height = bezier_point.z - height;

                    // Above cost
                    float above_cost = 0.5 * (fabs(pylon_height) + pylon_height);
                    above_cost = pown(above_cost, 2) * pylon_cost;

                    // Below cost
                    float below_cost = (-fabs(pylon_height + excavation_depth) + pylon_height + excavation_depth);
                    float below_cost_den = 2.0 * pylon_height + 2.0 * (excavation_depth);

                    below_cost = below_cost / below_cost_den * tunnel_cost;
                    track_cost += (above_cost + below_cost) * spacing;

            }

                last_last = last_point;
                last_point = bezier_point;

            }

            // Write to the buffer
            min_curves [w] = min_curve;
            max_grades [w] = min_curve_allowed;
            track_costs[w] = track_cost;

            barrier(CLK_LOCAL_MEM_FENCE);

            if (!w) {

                // Figure out real min curve and max grade
                for (int m = 1; m < 50; m++) {

                     min_curve = min(min_curve, min_curves[m]);
                     steepest_grade = max(steepest_grade, max_grades[m]);
                     track_cost += track_costs[m];

                }

                // Calculate grade cost
                float grade_cost = 100.0 * (steepest_grade - max_grade_allowed + fabs(max_grade_allowed - steepest_grade)) + 1.0;
                float curve_cost = 0.005 * (min_curve_allowed - min_curve + fabs(min_curve_allowed - min_curve)) + 1.0;

                // Get total cost
                float total_cost = grade_cost * track_cost * curve_cost;

                // Set the individual's header to contain its cost
                individuals[path - 1].x = total_cost;

            }
    });

    // Create a temporary kernel and execute it
    static Kernel kernel = Kernel(source, "cost");
    kernel.setArgs(_data.getOpenCLImage(), _opencl_individuals.get_buffer(), _genome_size + 2,
                   MAX_SLOPE_GRADE, pod.minCurveRadius(), EXCAVATION_DEPTH, _data.getWidthInMeters(),
                   _data.getHeightInMeters(), _opencl_binomials.get_buffer(),
                   _num_evaluation_points_1, _num_evaluation_points / 50);

    // Execute the 2D kernel with a work size of 5. 5 threads working on a single individual
    kernel.execute2D(glm::vec<2, size_t>(0, 0),
                     glm::vec<2, size_t>(_pop_size, 50),
                     glm::vec<2, size_t>(1, 50));

    // Download the data
    boost::compute::copy(_opencl_individuals.begin(), _opencl_individuals.end(), _individuals.begin(), queue);

}

void Population::generatePopulation() {

    // Random seed
    std::hash<int> hasher;
    srand(hasher(time(0)));

    // Go through each individual
    for (int i = 0; i < _pop_size; i++) {

        // Adds + 2 to ignore the header and start
        int individual_start = i * _individual_size + 2;

        // Set the start and the destination
        _individuals[individual_start - 1] = _start;

        // Set the start and the destination
        _individuals[individual_start + _genome_size] = _dest;

        for (int j = 0; j < _genome_size; j++)
            generateRandomPoint(_individuals[individual_start + j]);

    }

    // Upload the population onto the GPU
    boost::compute::copy(_individuals.begin(), _individuals.end(), _opencl_individuals.begin(), Kernel::getQueue());

}

glm::vec4* Population::crossoverIndividual(int a, int b) {

    // Get memory location of a's genom, add 2 to ignore the header and start
    glm::vec4* a_genome = _individuals.data() + (a * _individual_size + 2);
    glm::vec4* b_genome = _individuals.data() + (b * _individual_size + 2);

    // Crossover each gene
    for (int i = 0; i < _genome_size; i++) {

        // Get a random amount to cross the two by
        dummy_genome[i] = glm::vec4(glm::mix(a_genome[i].x, b_genome[i].x, glm::linearRand(0.0, 1.0)),
                                    glm::mix(a_genome[i].y, b_genome[i].y, glm::linearRand(0.0, 1.0)),
                                    glm::mix(a_genome[i].z, b_genome[i].z, glm::linearRand(0.0, 1.0)), 0.0);

    }

    // Do some mutation
    mutateGenome(dummy_genome);

    return dummy_genome;

}

void Population::mutateGenome(glm::vec4* genome) {

    // 81% chance that this genome will be mutated
    if (glm::linearRand(0.0, 1.0) > 0.19) {
        
        // Choose a single random point to mutate and then choose a random component
        int point = glm::linearRand(0, _genome_size - 1);
        
        glm::vec4* point_ptr = genome + point;
        
        // Do the mutation
        generateRandomPoint((*point_ptr));
        
    }

}

void Population::calcBinomialCoefficients() {

    // For degree we have _genome_size + 2 points, so we use that minus 1 for the degree
    const std::vector<int>& binomials = Bezier::getBinomialCoefficients(_genome_size + 1);
    _opencl_binomials = boost::compute::vector<int>(_genome_size + 2, Kernel::getContext());

    // Upload to the GPU
    boost::compute::copy(binomials.begin(), binomials.end(), _opencl_binomials.begin(), Kernel::getQueue());

}

void Population::generateRandomPoint(glm::vec4& to_gen) const {

    // First we move along the direction vector by a random amount
    float percent = glm::linearRand(0.0, 1.0);
    glm::vec4 progressed = _direction * percent + _start;

    // Generate a random deviation
    glm::vec4 deviation = progressed + glm::vec4(glm::linearRand(0.0f, MAX_STRAIGHT_DEVIATION),
                                                 glm::linearRand(0.0f, MAX_STRAIGHT_DEVIATION),
                                                 0.0f, 0.0f);

    // Final vector, clamp to width and height
    to_gen = glm::vec4(glm::clamp(deviation.x, 0.0f, _data.getWidthInMeters()),
                       glm::clamp(deviation.y, 0.0f, _data.getHeightInMeters()),
                       glm::linearRand(_data.getMinElevation() - TRACK_ABOVE_BELOW_EXTREMA,
                                       _data.getMaxElevation() + TRACK_ABOVE_BELOW_EXTREMA), 0.0f);

}
