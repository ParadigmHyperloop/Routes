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
                   float num_points_1, int points_per_worker, float origin_x, float origin_y, float straight_distance) {

    const sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
    const float pylon_cost = 1.16;
    const float tunnel_cost = 3100.0;

    __local int curve_penalties[100];
    __local int grade_penalties[100];
    __local float segment_lengths [100];
    __local float track_costs[100];

    // Get an offset to the gnome
    size_t i = get_global_id(0);
    size_t w = get_local_id(1);

    int path = i * (path_length + 1) + 1;

    float route_length;
    int curve_penalty = 0;
    int grade_penalty = 0;
    float track_cost = 0.0;

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

            if (curvature(last_last, last_point, bezier_point) < min_curve_allowed)
                ++curve_penalty;

        }

        // Compute grade and track cost if there was spacing
        if (spacing) {

            if (fabs(bezier_point.z - last_point.z) / spacing > max_grade_allowed)
                ++grade_penalty;

            // Add distance
            route_length += length(bezier_point - last_point);

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
    curve_penalties[w] = curve_penalty;
    grade_penalties[w] = grade_penalty;
    segment_lengths[w] = route_length;
    track_costs[w] = track_cost;

    barrier(CLK_LOCAL_MEM_FENCE);

    // This is only done on thread 0 so we don't do a million memory writes/reads
    if (!w) {

        // Figure out the final cost for everything. This would be equivalent to using one thread
        for (int m = 1; m < 100; m++) {

             curve_penalty += curve_penalties[m];
             grade_penalty += grade_penalties[m];
             route_length += segment_lengths[m];
             track_cost += track_costs[m];

        }

        // To normalize the track cost, divide by the distance of the entire route.
        // Then we divide by the max cost per segment of track, which we assume is the tunneling cost
        float track_cost_n = track_cost / (route_length * tunnel_cost);

        // To normalize the distance, divide by the straight line distance and then subtract 1.
        // This is equivalent to subtracting the straight line distance and then dividing since the min value is
        // the straight line distance.
        float route_length_n = clamp(route_length / straight_distance - 1.0, 0.0, 1.0);

        float curve_cost = (float)curve_penalty / (num_points_1 + 1.0);
        float grade_cost = (float)grade_penalty / (num_points_1 + 1.0);

        // Get total cost
        float total_cost = track_cost_n * 1.2 + curve_cost + grade_cost + route_length_n * 1.4;

        // Set the individual's header to contain its cost
        individuals[path - 1].x = total_cost;
        individuals[path - 1].y = route_length_n;

    }

}