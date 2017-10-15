//
// Created by Logan on 10/12/17.
//

#include "population.h"

Population::Population(int pop_size, int genome_size, const ElevationData& data) : _data(data) {

    // Save constants
    _pop_size = pop_size;
    _genome_size = genome_size;

    // Create the appropriate vectors
    _individuals = std::vector<glm::vec4>(pop_size * (genome_size + 1));
    _opencl_individuals =  boost::compute::vector<glm::vec4>(_individuals.size(), Kernel::getContext());

    // Calculate the binomial coefficients for evaluating the bezier paths
    calcBinomialCoefficients();

    // Make a dummy genome
    dummy_genome = new glm::vec4[genome_size];

    // Generate the population
    generatePopulation();

}

Population::~Population() { delete dummy_genome; }

Individual Population::getIndividual(int index) {

    Individual ind;
    ind.num_genes = _genome_size;

    // Calculate the location of the parts of the individual
    glm::vec4* header_loc = _individuals.data() + index * (_genome_size + 1);
    ind.header = header_loc;
    ind.genome = header_loc + 1;

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
        memcpy(sorted_individuals.data() + i * (_genome_size + 1), individuals_s[i].header, sizeof(glm::vec4) * (_genome_size + 1));

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
        memcpy(new_population.data() + (i * (_genome_size + 1) + 1), bred, sizeof(glm::vec4) * _genome_size);

    }

    // Generate new individuals (20%)
    for (; i < _pop_size; i++) {

        // Adds + 1 to ignore the header
        int individual_start = i * (_genome_size + 1) + 1;

        for (int j = 0; j < _genome_size; j++) {

            // Create a new random vector with
            glm::vec4 random_vec = glm::vec4(glm::linearRand(0.0f, _data.getWidthInMeters()),
                                             glm::linearRand(0.0f, _data.getHeightInMeters()),
                                             glm::linearRand(_data.getMinElevation(), _data.getMaxElevation()), 0.0);

            new_population[individual_start + j] = random_vec;

        }

    }

    // Save the new population
    _individuals = new_population;

    // Copy to GPU
    boost::compute::copy(new_population.begin(), new_population.end(), _opencl_individuals.begin(), Kernel::getQueue());

}

void Population::evaluateCost(glm::vec4 start, glm::vec4 dest, const Pod& pod) {

    // Get stuff we need to execute a kernel on
    const boost::compute::context& ctx =   Kernel::getContext();
    boost::compute::command_queue& queue = Kernel::getQueue();

    // Generate the program sounds
    static const std::string source = BOOST_COMPUTE_STRINGIZE_SOURCE(

            // Evaluates the bezier curve made by the given control points and start and dest at parametric value s
            float4 evaluateBezierCurve(__global float4* controls, int offset, int points, float s, float4 start, float4 dest, __global int* binomial_coeffs) {

                float one_minus_s = 1.0 - s;

                // Account for start and dest, so only add 1
                int degree = points + 1;

                // First term
                float4 out_point = start * pown(one_minus_s, degree);

                // Middle terms, iterate for num points
                for (int i = 1; i < points + 1; i++) {

                    // Evaluate for x y and z
                    float multiplier = pown(one_minus_s, degree - i) * pown(s, i);
                    out_point += controls[i + offset] * multiplier * (float)binomial_coeffs[i];

                }

                // Last term
                return out_point + dest * pown(s, degree);

            }

            // Computes the curvature implied by 3 control points of a bezier curve
            float curvature(float4 p0, float4 p1, float4 p2) {

                // Get the triangle side lengths
                float a = distance(p0, p1);
                float b = distance(p1, p2);
                float c = distance(p2, p0);

                // Fake check, this has never actually been a problem even though the paper includes it
                // Error if a + b == c || b + c == a || a + c == b

                // Do the curvature calculation
                float num = a * b * c;
                float denom = (a + b + c) * (b + c - a) * (a - b + c) * (a + b - c);

                return num / sqrt(denom);

            }

            // Computes the cost of a path
            __kernel void cost(__read_only image2d_t image, __global float4* individuals, int genome_size,
                               float max_grade_allowed, float min_curve_allowed, float excavation_depth, float width, float height, __global int* binomial_coeffs,
                               float4 start, float4 dest) {

                const sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
                const float pylon_cost = 116.0;
                const float tunnel_cost = 310000.0;
                const float num_points = 3700.0;

                // Get an offset to the gnome
                size_t i = get_global_id(0);
                int genome = i * (genome_size + 1) + 1;

                // Calculate the min curve
                // Start is special
                float min_curve = curvature(start, individuals[genome], individuals[genome + 1]);
                for (int p = 0; p < genome_size - 2; p++)
                    min_curve = min(min_curve, curvature(individuals[genome + p],
                                                         individuals[genome + p + 1],
                                                         individuals[genome + p + 2]));

                // Curvature for last
                min_curve = min(min_curve, curvature(individuals[genome + genome_size - 2],
                                                     individuals[genome + genome_size - 1],
                                                     dest));

                float curve_cost = 0.5 * (min_curve_allowed - min_curve + fabs(min_curve_allowed - min_curve)) + 1.0;
                float track_cost = 0.0;
                float steepest_grade = 0.0;

                float4 last_point = start;

                for (int p = 0; p <= num_points; p++) {

                    float4 bezier_point;

                    if (p == 0)
                        bezier_point = start;
                    else if (p == num_points)
                        bezier_point = dest;
                    else bezier_point = evaluateBezierCurve(individuals, genome, genome_size, (float)p / num_points,
                                                            start, dest, binomial_coeffs);

                    // Get pylon height with a sample from the image
                    float2 nrm_device = (float2)(bezier_point.x / width, bezier_point.y / height);
                    float height = read_imagef(image, sampler, nrm_device).x;

                    // Compute spacing, only x and y distance
                    float spacing = sqrt(pown(bezier_point.x - last_point.x, 2) + pown(bezier_point.y - last_point.y, 2));

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
                        float below_cost_den = 2.0 * pylon_height + 20.0;

                        below_cost = below_cost / below_cost_den * tunnel_cost;
                        track_cost += (above_cost + below_cost) * spacing;

                    }

                    last_point = bezier_point;

                }

                // Calculate grade cost
                float grade_cost = 100.0 * (steepest_grade - max_grade_allowed + fabs(max_grade_allowed - steepest_grade)) + 1.0;

                // Get total cost
                float total_cost = grade_cost * track_cost * curve_cost;

                // Set the individual's header to contain its cost
                individuals[genome - 1].x = total_cost;

            }

    );

    // Convert start and dest to float4
    boost::compute::float4_ start_4 = boost::compute::float4_(start.x, start.y, start.z, 0.0);
    boost::compute::float4_ dest_4 =  boost::compute::float4_(dest.x,  dest.y,  dest.z,  0.0);

    // Create a temporary kernel and execute it
    static Kernel kernel = Kernel(source, "cost");
    kernel.setArgs(_data.getOpenCLImage(), _opencl_individuals.get_buffer(), _genome_size,
                   MAX_SLOPE_GRADE, pod.minCurveRadius(), EXCAVATION_DEPTH, _data.getWidthInMeters(), _data.getHeightInMeters(),
                   _opencl_binomials.get_buffer(), start_4, dest_4);
    kernel.execute1D(0, _pop_size);

    // Download the data
    boost::compute::copy(_opencl_individuals.begin(), _opencl_individuals.end(), _individuals.begin(), queue);

}



void Population::generatePopulation() {

    // Random seed
    srand(time(0));

    // Go through each individual
    for (int i = 0; i < _pop_size; i++) {

        // Adds + 1 to ignore the header
        int individual_start = i * (_genome_size + 1) + 1;

        for (int j = 0; j < _genome_size; j++) {

            // Create a new random vector with
            glm::vec4 random_vec = glm::vec4(glm::linearRand(0.0f, _data.getWidthInMeters()),
                                             glm::linearRand(0.0f, _data.getHeightInMeters()),
                                             glm::linearRand(_data.getMinElevation(), _data.getMaxElevation()), 0.0);

            _individuals[individual_start + j] = random_vec;

        }

    }

    // Upload the population onto the GPU
    boost::compute::copy(_individuals.begin(), _individuals.end(), _opencl_individuals.begin(), Kernel::getQueue());

}

glm::vec4* Population::crossoverIndividual(int a, int b) {

    // Get memory location of a's genom
    glm::vec4* a_genome = _individuals.data() + (a * (_genome_size + 1) + 1);
    glm::vec4* b_genome = _individuals.data() + (b * (_genome_size + 1) + 1);

    // Cross over each gene
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

    // TODO Mutation needs to be properly implemented. Bit flipping is slow and produces terrible results.

}

void Population::calcBinomialCoefficients() {

    std::vector<int> binomials = std::vector<int>(_genome_size);
    _opencl_binomials = boost::compute::vector<int>(_genome_size, Kernel::getContext());

    for (int i = 0; i < _genome_size; i++)
        binomials[i] = calcBinomialCoefficient(_genome_size - 1, i);

    // Upload to the GPU
    boost::compute::copy(binomials.begin(), binomials.end(), _opencl_binomials.begin(), Kernel::getQueue());

}

int Population::calcBinomialCoefficient(int n, int i) {

    // Special case to prevent divide by zero
    if (!i || n == i)
        return 1;

    // Formula taken from https://en.wikipedia.org/wiki/Binomial_coefficient#Multiplicative_formula
    int ni_falling = n;
    int i_fac = i;

    // Calculate i!
    for (int j = 1; j < i - 1; j++)
        i_fac *= (i - j);

    // Calculate n falling factorial i
    for (int j = 1; j < i; j++)
        ni_falling *= (n - j);

    return ni_falling / i_fac;

}