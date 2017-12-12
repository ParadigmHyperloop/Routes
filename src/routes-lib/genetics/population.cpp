//
// Created by Logan on 10/12/17.
//

#include "population.h"

Population::Population(int pop_size, glm::vec4 start, glm::vec4 dest, const ElevationData& data, const Pod& pod) : _data(data), _pod(pod) {

    // Save constants
    _pop_size = pop_size;
    _start = start;
    _dest = dest;
    _direction = _dest -_start;

    // Figure out how many points we need for this route
    calcGenomeSize();

    // Figure out the number of vectors that make up the entire individual. This is the header, the start
    // and the destination
    _individual_size = _genome_size + 2 + 1;

    // Create the appropriate vectors
    _individuals      = std::vector<glm::vec4>(pop_size * _individual_size);
    _best_individuals = std::vector<glm::vec4>(pop_size * _individual_size);
    _global_best      = std::vector<glm::vec4>(_individual_size);
    _velocities       = std::vector<glm::vec4>(pop_size * _genome_size);
    
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

    // Init the params
    initParams();
    
    // Generate the population
    generatePopulation();

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

void Population::evaluateCost() {

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
         const float curve_weight = 0.3;
         const float curve_sum_weight = 0.4;
         const float grade_weight = 100.0;
         
         __local float curve_sums [100];
         __local float min_curves [100];
         __local float max_grades [100];
         __local float track_costs[100];
         
         // Get an offset to the gnome
         size_t i = get_global_id(0);
         size_t w = get_local_id(1);
         
         int path = i * (path_length + 1) + 1;
         
         float min_curve = 10000000000000.0;
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
                 min_curve = min(min_curve, curve);
                 
             }
             
             // Compute grade and track cost if there was spacing
             if (spacing) {
                 
                 // Add distance
                 curve_sum += length(bezier_point - last_point);
                 
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
         min_curves [w] = min_curve;
         max_grades [w] = steepest_grade;
         track_costs[w] = track_cost;
         
         barrier(CLK_LOCAL_MEM_FENCE);
         
         // This is only done on thread 0 so we don't do a million memory writes/reads
         if (!w) {
             
             // Figure out the final cost for everything. This would be equivalent to using one thread
             for (int m = 1; m < 100; m++) {
                 
                 curve_sum += curve_sums[m];
                 min_curve = min(min_curve, min_curves[m]);
                 steepest_grade = max(steepest_grade, max_grades[m]);
                 track_cost += track_costs[m];
                 
             }
             
             // Now that we have all of the information about the route, we can calculate the cost.
             // Calculate grade cost, only apply a penalty if it is above 6%
             float grade_cost = grade_weight * (steepest_grade - max_grade_allowed + fabs(max_grade_allowed - steepest_grade)) + 1.0;
             
             // Calculate the curvature cost
             // Right now we are simply using the sum of curvature (not radius of curvature)
             float curve_cost = curve_weight * (min_curve_allowed - min_curve + fabs(min_curve_allowed - min_curve)) + curve_sum * curve_sum_weight;
             
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
                   MAX_SLOPE_GRADE, _pod.minCurveRadius(), EXCAVATION_DEPTH, size_crop.x,
                   size_crop.y, _opencl_binomials.get_buffer(),
                   _num_evaluation_points_1, _num_evaluation_points / 50, origin.y, origin.y);

    // Execute the 2D kernel with a work size of 5. 5 threads working on a single individual
    kernel.execute2D(glm::vec<2, size_t>(0, 0),
                     glm::vec<2, size_t>(_pop_size, 50),
                     glm::vec<2, size_t>(1, 50));

    // Download the data
    boost::compute::copy(_opencl_individuals.begin(), _opencl_individuals.end(), _individuals.begin(), queue);

}

void Population::step() {
    
    // Upload the population onto the GPU and evaluate it
    boost::compute::copy(_individuals.begin(), _individuals.end(), _opencl_individuals.begin(), Kernel::getQueue());
    evaluateCost();
    
    // Update the best solutions
    updateBestSolutions();
    
    // Update all of the particles
    updatePositions();
    
    // Generate new velocities
    generateVelocities();
    
}

void Population::generateVelocities() {
    
    for (int i = 0; i < _pop_size; i++) {
    
        // Generate r1 and r2
        float r1 = generateRandomFloat(0.0, 1.0);
        float r2 = generateRandomFloat(0.0, 1.0);
        
        // Adds + 2 to ignore the header and start
        int individual_start = i * _individual_size + 2;
        
        // Go through each of the control points and create an updated velocity for them
        for (int p = 0; p < _genome_size; p++) {
            
            glm::vec4 global_diff = _global_best[2 + p]                     - _individuals[individual_start + p];
            glm::vec4 indivd_diff = _best_individuals[individual_start + p] - _individuals[individual_start + p];
            
            int velocity_pos = i * _genome_size + p;
            _velocities[velocity_pos] = _omega * _velocities[velocity_pos] + r1 * _c1 * indivd_diff + r2 * _c2 * global_diff;
            
            // Clamp down to make sure its not bigger than max velocity
            for (int c = 0; c < 3; c++)
                _velocities[velocity_pos][c] = glm::clamp(_velocities[velocity_pos][c], -_max_vel[c], _max_vel[c]);
            
        }
        
    }
    
}

void Population::updatePositions() {
    
    for (int i = 0; i < _pop_size; i++) {
         
        // Adds + 2 to ignore the header and start
        int individual_start = i * _individual_size + 2;
 
        // Add on velocity
        for (int p = 0; p < _genome_size; p++)
            _individuals[individual_start + p] += _velocities[_genome_size * i + p];
        
     }
    
}

void Population::updateBestSolutions() {
    
    for (int i = 0; i < _pop_size; i++) {
        
        // Adds + 2 to ignore the header and start
        int individual_start = i * _individual_size;
     
        // Check if this is a new personal best
        if (_individuals[individual_start].x < _best_individuals[individual_start].x) {
            
//            std::cout << "New personal best :D\n";
         
            // Copy over the new best
            memcpy(_best_individuals.data() + individual_start, _individuals.data() + individual_start, sizeof(glm::vec4) * _individual_size);
            
        }
        
        if (_individuals[individual_start].x < _global_best[0].x)
            memcpy(_global_best.data(), _individuals.data() + individual_start, sizeof(glm::vec4) * _individual_size);
        
    }
    
}

void Population::calcGenomeSize() {

    // The genome size has a square root relationship with the length of the route
    float sqrt_lenth = sqrt(glm::length(_direction));
    _genome_size = std::round(sqrt_lenth * LENGTH_TO_GENOME);
    std::cout << "Genome size: " << _genome_size << std::endl;

}

void Population::initParams() {
    
    // Equal balance between going to the global best and the inidividual best
    _c1 = 2.0;
    _c2 = 2.0;
    _omega = 0.8;
    
    // Calculate the max velocity
    glm::vec2 origin = _data.getCroppedOriginMeters();
    glm::vec2 size = _data.getCroppedSizeMeters();
    
    glm::vec3 low = glm::vec3(origin.x, origin.y, _data.getMinElevation());
    glm::vec3 high = glm::vec3(origin.x + size.x, origin.y + size.y, _data.getMaxElevation());
    
    _max_vel = KAPPA * (high - low) * 0.5f;
    
}

void Population::generatePopulation() {

    // Random seed
    std::hash<int> hasher;
    _twister = std::mt19937(hasher(std::chrono::high_resolution_clock::now().time_since_epoch().count()));

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

    // Evaluate the cost of the population
    evaluateCost();
    
    // Copy the entire vector over to the best solution one
    memcpy(_best_individuals.data(), _individuals.data(), sizeof(glm::vec4) * _individuals.size());
    
    // Find the global best solution
    int best_individual = 0;
    for (int i = 1; i < _pop_size; i++) {
        
        // Adds + 2 to ignore the header and start
        int individual_start = i         * _individual_size;
        int best_start = best_individual * _individual_size;
        
        // Check if this had a better fitness
        if (_individuals[individual_start].x < _individuals[best_start].x)
            best_individual = i;
        
    }
    
    std::cout << "Best starting individual was " << best_individual << std::endl;
    
    // Copy over the best individual
    memcpy(_global_best.data(), _individuals.data() + (best_individual * _individual_size), sizeof(glm::vec4) * _individual_size);
    
}

void Population::calcBinomialCoefficients() {

    // For degree we have _genome_size + 2 points, so we use that minus 1 for the degree
    const std::vector<int>& binomials = Bezier::getBinomialCoefficients(_genome_size + 1);
    _opencl_binomials = boost::compute::vector<int>(_genome_size + 2, Kernel::getContext());

    // Upload to the GPU
    boost::compute::copy(binomials.begin(), binomials.end(), _opencl_binomials.begin(), Kernel::getQueue());

}

void Population::generateRandomPoint(glm::vec4& to_gen) {

    // First we move along the direction vector by a random amount
    float percent = generateRandomFloat(0.0, 1.0);
    glm::vec4 progressed = _direction * percent + _start;

    // Generate a random deviation
    glm::vec4 deviation = progressed + glm::vec4(generateRandomFloat(-MAX_STRAIGHT_DEVIATION, MAX_STRAIGHT_DEVIATION),
                                                 generateRandomFloat(-MAX_STRAIGHT_DEVIATION, MAX_STRAIGHT_DEVIATION),
                                                 0.0f, 0.0f);

    // Get cropped info
    glm::vec2 cropped_origin = _data.getCroppedOriginMeters();
    glm::vec2 cropped_size   = _data.getCroppedSizeMeters();

    // Final vector, clamp to width and height
    to_gen = glm::vec4(glm::clamp(deviation.x, cropped_origin.x, cropped_origin.x + cropped_size.x),
                       glm::clamp(deviation.y, cropped_origin.y, cropped_origin.y + cropped_size.y),
                       generateRandomFloat(_data.getMinElevation() - TRACK_ABOVE_BELOW_EXTREMA,
                                           _data.getMaxElevation() + TRACK_ABOVE_BELOW_EXTREMA), 0.0);

}

float Population::generateRandomFloat(float low, float high) {
    
    // Get the twister value from [0,1]
    float random = (float)_twister() / (float)4294967296;
    
    // Convert and return
    return (random * (high - low)) + low;
    
}
