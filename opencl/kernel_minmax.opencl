__kernel void computeMinMax(__read_only image2d_t image, __global float* min_max, int width, int height) {

    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

    // Use this to exclude no data
    const float extreme = 1000000.0;

    size_t x = get_global_id(0);

    float min_v = 1000000.0;
    float max_v = -1000000.0;

    for (int y = 0; y < height; y++) {

        float sample = read_imagef(image, sampler, (float2)(x, y)).x;

        // Do a sanity check
        if (fabs(sample) < extreme) {

            min_v = min(sample, min_v);
            max_v = max(sample, max_v);

        }

    }

    min_max[0] = min(min_v, min_max[0]);
    min_max[1] = max(max_v, min_max[1]);

}
