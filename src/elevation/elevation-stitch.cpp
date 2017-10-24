//
// Created by Logan on 10/23/17.
//

#include "elevation-stitch.h"

ElevationData ElevationStitch::stitch(const std::string& root, const std::string& sticher) {

    // First we load up both pieces of data from the disk.
    GDALDataset* root_set = (GDALDataset*)GDALOpen(root.c_str(), GA_ReadOnly);
    GDALDataset* sticher_set = (GDALDataset*)GDALOpen(sticher.c_str(), GA_ReadOnly);

    // Now we figure out how big of a buffer we need to allocate based on if there is overlap or not
    // First we get the origins of the two and their sizes in degrees
    // These transforms should both be the same
    double root_transform[8];
    double stitcher_transform[8];

    root_set->GetGeoTransform(root_transform);
    sticher_set->GetGeoTransform(stitcher_transform);

    // Make sure that the transforms are the same scale
    if (fabs(root_transform[1] - stitcher_transform[1]) > EPSILON ||
        fabs(root_transform[5] - stitcher_transform[5]) > EPSILON)
            throw std::runtime_error("The datasets could not be stitched because they did not have the same transform");


    // Origins
    glm::vec2 origin_root =  glm::vec2(root_transform[0],     root_transform[3]);
    glm::vec2 sticher_root = glm::vec2(stitcher_transform[0], stitcher_transform[3]);

    // Get the extents of the data in degrees.
    // We invert the y transform because it is negative and for calculation purposes we require it to be positive
    glm::vec2 extents_root =    glm::vec2(root_set->GetRasterXSize()    *  root_transform[1],
                                          root_set->GetRasterYSize()    * -root_transform[5]);

    glm::vec2 extents_sticher = glm::vec2(sticher_set->GetRasterXSize() *  root_transform[1],
                                          sticher_set->GetRasterYSize() * -root_transform[5]);

    // Get the size of the buffer required in degrees
    glm::vec2 extents = glm::vec2(getRequiredSize(origin_root.x, extents_root.x, sticher_root.x, extents_sticher.x),
                                  getRequiredSize(origin_root.y, extents_root.y, sticher_root.y, extents_sticher.y));

    // Get the size we need in pixels now
    glm::ivec2 size_pixels = glm::ivec2(extents.x / root_transform[1], extents.y / -root_transform[5]);

    // Create a 2D array of the size that we need in pixels
    float** stitched_buffer = new float*[size_pixels.y];
    for (int i = 0; i < size_pixels.y; i++)
        stitched_buffer[i] = new float[size_pixels.x];

    // Put the root buffer's contents into the new 2D buffer.
    // First we need to figure out where the root buffer belongs
    glm::ivec2 root_insert_pos = glm::ivec2(0);
    glm::ivec2 stitcher_insert_pos = glm::ivec2(size_pixels.x - sticher_set->GetRasterXSize(),
                                                size_pixels.y - root_set->GetRasterYSize());

    // Check if the root should be inserted after the stitcher
    if (origin_root.x - sticher_root.x > EPSILON) {

        root_insert_pos.x = size_pixels.x - root_set->GetRasterXSize();
        stitcher_insert_pos.x = 0;

    }

    if (origin_root.y - sticher_root.y > EPSILON) {

        root_insert_pos.y = size_pixels.y - root_set->GetRasterYSize();
        stitcher_insert_pos.y = 0;

    }

    // Get the raster band for the stitcher data
    GDALRasterBand* stitcher_raster = sticher_set->GetRasterBand(1);

    // This is inefficient but the documentation is unclear how the RasterIO function reads.
    // This operation shouldn't be done in real time anyways, so everything is Kosher.
    for (int i = 0; i < sticher_set->GetRasterYSize(); i++) {

        float* insert_pos = &stitched_buffer[stitcher_insert_pos.y + i][stitcher_insert_pos.x];
        stitcher_raster->RasterIO(GF_Read, 0, i, sticher_set->GetRasterXSize(), 1, insert_pos,
                                  sticher_set->GetRasterXSize(), 1, GDT_Float32, 0, 0);

    }

    // Get the raster band for the root data
    GDALRasterBand* root_raster = sticher_set->GetRasterBand(1);

    // For simplicity we do this in two loops. Doing the root second ensures that it will be the data that
    // is always present
    for (int i = 0; i < root_set->GetRasterYSize(); i++) {

        float* insert_pos = &stitched_buffer[root_insert_pos.y + i][root_insert_pos.x];
        root_raster->RasterIO(GF_Read, 0, i, root_set->GetRasterXSize(), 1, insert_pos,
                                  root_set->GetRasterXSize(), 1, GDT_Float32, 0, 0);

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
    new_data._gdal_dataset = root_set;
    new_data.calcConversions();
    new_data.calcStats();

    new_data._gdal_dataset = nullptr;

    // Save width and height
    new_data._width = size_pixels.x;
    new_data._height = size_pixels.y;

    // Create the OpenCL image
    boost::compute::image_format format = boost::compute::image_format(CL_INTENSITY, CL_FLOAT);
    new_data._opencl_image = boost::compute::image2d(Kernel::getContext(), size_pixels.x, size_pixels.y,
                                                     format, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, raw.data());

    // Calc the min and max
    new_data.calcMinMax();

    // Delete the stitched buffer
    for (int i = 0; i < size_pixels.y; i++)
        delete stitched_buffer[i];
    delete [] stitched_buffer;

    // Close GDAL buffers
    GDALClose(root_set);
    GDALClose(sticher_set);

    return new_data;

}

float ElevationStitch::getRequiredSize(float root_origin, float root_size, float sticher_origin, float sticher_size) {

    // Create two vectors for convenience
    glm::vec2 root =    glm::vec2(root_origin,    root_size);
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