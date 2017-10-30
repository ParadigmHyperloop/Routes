//
// Created by Logan on 10/23/17.
//

#include "elevation-stitch.h"

ElevationData ElevationStitch::stitch(const std::string& root, const std::string& stitcher) { return stitch({root, stitcher}); }

ElevationData ElevationStitch::stitch(const std::vector<std::string>& paths) {

    // Load up all of the GDAL data
    std::vector<GDALDataset*> datasets = std::vector<GDALDataset*>(paths.size());
    for (int i = 0; i < datasets.size(); i++)
        datasets[i] = (GDALDataset*)GDALOpen(paths[i].c_str(), GA_ReadOnly);

    // Get the transform of the first (root) dataset
    double transform[8];
    datasets[0]->GetGeoTransform(transform);

    // Make sure that every other data set is compatible with this one
    for (int i = 1; i < datasets.size(); i++) {

        double transform_t[8];
        datasets[i]->GetGeoTransform(transform_t);

        if (fabs(transform[1] - transform_t[1]) > EPSILON ||
            fabs(transform[5] - transform_t[5]) > EPSILON)
            throw std::runtime_error("The datasets could not be stitched because they did not have the same transform");

    }

    // Get the 2D buffer size required to contain all of the data
    DataRect root_rect = DataRect(datasets[0]);
    DataRect stitch_rect = DataRect(datasets[1]);
    DataRect buffer_rect = getRequiredRect(root_rect, stitch_rect);;

    // Ensure that the rect contains all sub-rects
    for (int i = 2; i < datasets.size(); i++)
        buffer_rect = getRequiredRect(DataRect(datasets[i]), buffer_rect);

    // Get the size we need in pixels now
    glm::ivec2 size_pixels = glm::ivec2(ceil(buffer_rect.extents.x / transform[1]),
                                        ceil(buffer_rect.extents.y / -transform[5]));

    // Create a 2D array of the size that we need in pixels
    float** stitched_buffer = new float*[size_pixels.y];
    for (int i = 0; i < size_pixels.y; i++)
        stitched_buffer[i] = new float[size_pixels.x];

    // Read in all of the data
    for (int i = datasets.size() - 1; i >= 0; i--) {

        GDALDataset* dataset = datasets[i];

        // Get the transform
        double transform_t[8];
        datasets[i]->GetGeoTransform(transform_t);

        // Figure out where the origin is in pixels
        glm::vec2 origin_t = glm::vec2(transform_t[0], transform_t[3]);

        // Get the offset in degrees and then convert it by pixels to get the insertion position
        glm::vec2 delta = origin_t - buffer_rect.origin;
        glm::ivec2 insertion = glm::ivec2(delta.x / transform[1], delta.y / -transform[5]);

        // Get the raster band for the data
        GDALRasterBand* raster = dataset->GetRasterBand(1);

        // For simplicity we do this in two loops. Doing the root second ensures that it will be the data that
        // is always present
        for (int i = 0; i < dataset->GetRasterYSize(); i++) {

            float* insert_pos = &stitched_buffer[insertion.y + i][insertion.x];
            raster->RasterIO(GF_Read, 0, i, dataset->GetRasterXSize(), 1, insert_pos,
                                            dataset->GetRasterXSize(), 1, GDT_Float32, 0, 0);

        }

    }

    // Turn the shiny new 2D array into a 1D one that we can upload to OpenCL
    std::vector<float> raw = std::vector<float>(size_pixels.x * size_pixels.y);
    for (int i = 0; i < size_pixels.y; i++) {

        // Copy over a row of data
        memcpy(raw.data() + i * size_pixels.x, &stitched_buffer[i][0], sizeof(float) * size_pixels.x);

    }

    // Now we leverage the fact that this is a friend class of ElevationData to create a new one from scratch
    ElevationData new_data = ElevationData();

    // Temporarily give it the root GDAL data so that we can calculate conversions using the built-in functions.
    // To make sure that it never deallocates it after we do, we take it away after
    new_data._gdal_dataset = datasets[0];
    new_data.calcConversions();

    new_data._gdal_dataset = nullptr;

    // Save width and height
    new_data._width  = size_pixels.x;
    new_data._height = size_pixels.y;

    new_data._width_meters  = size_pixels.x * new_data.pixelToMeterConversions[0];
    new_data._height_meters = size_pixels.y * new_data.pixelToMeterConversions[1];

    // Create the OpenCL image
    boost::compute::image_format format = boost::compute::image_format(CL_INTENSITY, CL_FLOAT);
    new_data._opencl_image = boost::compute::image2d(Kernel::getContext(), size_pixels.x, size_pixels.y,
                                                     format, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, raw.data());


    // Calculate the min and max
    double min_max[2]   = {0.0, 0.0};
    double min_max_t[2] = {0.0, 0.0};

    for (int i = 0; i < datasets.size(); i++) {

        // Get the raster band from each dataset
        GDALRasterBand* band = datasets[i]->GetRasterBand(1);

        // Compute min and max using GDAL
        int succ[2] = {0, 0};
        min_max_t[0] = band->GetMinimum(&succ[0]);
        min_max_t[1] = band->GetMaximum(&succ[1]);

        if (!succ[0] || !succ[1])
            band->ComputeRasterMinMax(false, min_max_t);

        // Check if this is the first one, if so save the results
        if (i == 0)
            memcpy(min_max, min_max_t, sizeof(double) * 2);

        // Save the min and max
        min_max[0] = glm::min(min_max_t[0], min_max[0]);
        min_max[1] = glm::max(min_max_t[1], min_max[1]);

    }

    new_data._elevation_min = min_max[0];
    new_data._elevation_max = min_max[1];

    // Delete the stitched buffer
    for (int i = 0; i < size_pixels.y; i++)
        delete stitched_buffer[i];
    delete [] stitched_buffer;

    // Close all of the data
    for (int i = 0; i < datasets.size(); i++)
        GDALClose(datasets[i]);

    return new_data;

}

ElevationStitch::DataRect::DataRect() { origin = glm::vec2(0.0); extents = glm::vec2(0.0);}

ElevationStitch::DataRect::DataRect(GDALDataset* dataset) {

    // Get the transform of the dataset
    double transform[8];
    dataset->GetGeoTransform(transform);

    // Get the origin and extents
    origin = glm::vec2(transform[0], transform[3]);

    // We invert the y transform because it is negative and for calculation purposes we require it to be positive
    extents = glm::vec2(dataset->GetRasterXSize()    *  transform[1],
                        dataset->GetRasterYSize()    * -transform[5]);

}

ElevationStitch::DataRect ElevationStitch::getRequiredRect(DataRect a, DataRect b) {

    DataRect to_return;

    // Get extents
    to_return.extents = glm::vec2(getRequiredSizeAxis(a.origin.x, a.extents.x, b.origin.x, b.extents.x),
                                  getRequiredSizeAxis(a.origin.y, a.extents.y, b.origin.y, b.extents.y));

    // Get the new origin
    to_return.origin = glm::vec2(glm::min(a.origin.x, b.origin.x),
                                 glm::max(a.origin.y, b.origin.y));

    return to_return;

}

float ElevationStitch::getRequiredSizeAxis(float root_origin, float root_size, float sticher_origin, float sticher_size) {

    // Create two vectors for convenience
    glm::vec2 root    = glm::vec2(root_origin,    root_size);
    glm::vec2 sticher = glm::vec2(sticher_origin, sticher_size);

    // Determine which is greater so we have a negative number
    glm::vec2* greater = &root;
    glm::vec2* lesser = &sticher;
    if (sticher_origin > root_origin) {

        greater = &sticher;
        lesser = &root;

    }

    // The size is now the furthest extent of the largest minus the origin of the smallest
    float size = greater->x + greater->y - lesser->x;

    return size;

}