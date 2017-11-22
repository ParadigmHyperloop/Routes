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

    // Create the appropriate vectors
    _individuals = std::vector<glm::vec4>(pop_size * _individual_size);
    _opencl_individuals =  boost::compute::vector<glm::vec4>(_individuals.size(), Kernel::getContext());

    // Calculate the binomial coefficients for evaluating the bezier paths
    calcBinomialCoefficients();

    // Figure out how many points this route should be evaluated on.
    // We also make sure it is a multiple of workers
    glm::dvec2 cropped_size = data.getCroppedSizeMeters();
    _num_evaluation_points = ceil(glm::max(cropped_size.x / METERS_TO_POINT_CONVERSION,
                                           cropped_size.y / METERS_TO_POINT_CONVERSION) / 50.0) * 50;
    
    std::cout << "Using " << _num_evaluation_points << " points of evaluation" << std::endl;
    _num_evaluation_points_1 = (float)_num_evaluation_points - 1.0f;

    // First we init the params, then generate a starter population
    initParams();
    samplePopulation();

}

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

void Population::step() {
    
    
    
    
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

            return fabs(num / denom);

        }

        // Computes the cost of a path
        __kernel void cost(__read_only image2d_t image, __global float4* individuals, int path_length,
                           float max_grade_allowed, float min_curve_allowed, float excavation_depth, float width,
                           float height, __global int* binomial_coeffs,
                           float num_points_1, int points_per_worker, float origin_x, float origin_y) {

            const sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
            const float pylon_cost = 0.000116;
            const float tunnel_cost = 0.310;
            const float curve_weight = 300000.0;
            const float grade_weight = 100.0;

            __local float curve_sums [50];
//            __local float min_curves [50];
            __local float max_grades [50];
            __local float track_costs[50];

            // Get an offset to the gnome
            size_t i = get_global_id(0);
            size_t w = get_local_id(1);

            int path = i * (path_length + 1) + 1;

//            float min_curve = 10000000000000.0;
            float curve_sum = 0.0;

            float track_cost = 0.0;
            float steepest_grade = 0.0;

            // Figure out where to start and end
            int start = w * points_per_worker;
            int end = start + points_per_worker;

            float4 last_last = individuals[path];
            float4 last_point = individuals[path];

            // If we start at an offset, we need to make sure we "remember" the information that the last worker
            // calculated. The fastest way to do this is to just re-calculate it
            if (w) {

                last_last   = evaluateBezierCurve(individuals, path, path_length, (float)(start - 2) / num_points_1, binomial_coeffs);
                last_point  = evaluateBezierCurve(individuals, path, path_length, (float)(start - 1) / num_points_1, binomial_coeffs);

            }

            for (int p = start; p <= end; p++) {

                // Evaluate the bezier curve
                float4 bezier_point = evaluateBezierCurve(individuals, path, path_length, (float)p / num_points_1, binomial_coeffs);

                // Get the elevation of the terrain at this point. We do this with a texture sample
                float2 nrm_device = (float2)((bezier_point.x - origin_x) / width, (bezier_point.y - origin_y) / height);
                float height = read_imagef(image, sampler, nrm_device).x;

                // Compute spacing, only x and y distance, z delta is handled by the grade
                float spacing = sqrt(pown(bezier_point.x - last_point.x, 2) + pown(bezier_point.y - last_point.y, 2));

                // Get curvature
                if (p > 1) {

                    float curve = curvature(last_last, last_point, bezier_point);
                    curve_sum += 1.0 / curve * curve_weight;
//                    min_curve = min(min_curve, curve);

                }

                // Compute grade and track cost if there was spacing
                if (spacing) {

                    steepest_grade = max(steepest_grade, fabs(bezier_point.z - last_point.z) / spacing);

                    // Compute track cost
                    // First we get the pylon height which is the distance from the point on the track to the actual terrain
                    float pylon_height = bezier_point.z - height;

                    // Cost for the track being above the terrain. This is significantly less than if it was
                    // underground because no tunneling is needed
                    float above_cost = 0.5 * (fabs(pylon_height) + pylon_height);
                    above_cost = pown(above_cost, 2) * pylon_cost;

                    // Cost for the track being below the ground.
                    // For a delta of <= excavation_depth we don't count as tunneling because excavation will suffice
                    float below_cost = (-fabs(pylon_height + excavation_depth) + pylon_height + excavation_depth);
                    float below_cost_den = 2.0 * pylon_height + 2.0 * (excavation_depth);

                    below_cost = below_cost / below_cost_den * tunnel_cost;
                    track_cost += (above_cost + below_cost) * spacing;

            }

                // Keep track of the stuff that was computed so that we can calculate the delta at the next point
                last_last = last_point;
                last_point = bezier_point;

            }

            // Write to the buffer
            curve_sums [w] = curve_sum;
//            min_curves [w] = min_curve;
            max_grades [w] = steepest_grade;
            track_costs[w] = track_cost;

            barrier(CLK_LOCAL_MEM_FENCE);

            // This is only done on thread 0 so we don't do a million memory writes/reads
            if (!w) {

                // Figure out the final cost for everything. This would be equivalent to using one thread
                for (int m = 1; m < 50; m++) {

                     curve_sum += curve_sums[m];
//                     min_curve = min(min_curve, min_curves[m]);
                     steepest_grade = max(steepest_grade, max_grades[m]);
                     track_cost += track_costs[m];

                }

                // Now that we have all of the information about the route, we can calculate the cost.
                // Calculate grade cost, only apply a penalty if it is above 6%
                float grade_cost = grade_weight * (steepest_grade - max_grade_allowed + fabs(max_grade_allowed - steepest_grade)) + 1.0;

                // Calculate the curvature cost
                // Right now we are simply using the sum of curvature (not radius of curvature)
                float curve_cost = curve_sum;

                // Get total cost
                float total_cost = grade_cost + track_cost + curve_cost;

                // Set the individual's header to contain its cost
                individuals[path - 1].x = total_cost;

            }
    });

    // Get the data to allow for proper texture sampling
    glm::vec2 size_crop = _data.getCroppedSizeMeters();
    glm::vec2 origin = _data.getCroppedOriginMeters();

    // Create a temporary kernel and execute it
    static Kernel kernel = Kernel(source, "cost");
    kernel.setArgs(_data.getOpenCLImage(), _opencl_individuals.get_buffer(), _genome_size + 2,
                   MAX_SLOPE_GRADE, pod.minCurveRadius(), EXCAVATION_DEPTH, size_crop.x,
                   size_crop.y, _opencl_binomials.get_buffer(),
                   _num_evaluation_points_1, _num_evaluation_points / 50, origin.y, origin.y);

    // Execute the 2D kernel with a work size of 5. 5 threads working on a single individual
    kernel.execute2D(glm::vec<2, size_t>(0, 0),
                     glm::vec<2, size_t>(_pop_size, 50),
                     glm::vec<2, size_t>(1, 50));

    // Download the data
    boost::compute::copy(_opencl_individuals.begin(), _opencl_individuals.end(), _individuals.begin(), queue);

}

void Population::calcGenomeSize() {

    // The genome size has a square root relationship with the length of the route
    float sqrt_lenth = sqrt(glm::length(_direction));
    _genome_size = std::round(sqrt_lenth * LENGTH_TO_GENOME);
    std::cout << "Genome size: " << _genome_size << std::endl;

}

void Population::initParams() {
    
    
}

void Population::bestGuess() {
    
    
}

void Population::calcWeights() {
    
    
}

void Population::samplePopulation() {
    
    
}

void Population::updateParams() {
    
    
}

void Population::calcBinomialCoefficients() {

    // For degree we have _genome_size + 2 points, so we use that minus 1 for the degree
    const std::vector<int>& binomials = Bezier::getBinomialCoefficients(_genome_size + 1);
    _opencl_binomials = boost::compute::vector<int>(_genome_size + 2, Kernel::getContext());

    // Upload to the GPU
    boost::compute::copy(binomials.begin(), binomials.end(), _opencl_binomials.begin(), Kernel::getQueue());

}

