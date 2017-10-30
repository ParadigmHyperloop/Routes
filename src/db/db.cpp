//
// Created by Logan on 10/29/17.
//

#include <glm/vec2.hpp>
#include "db.h"

std::vector<DB::Entry> DB::_entries;

bool DB::load() {

    // Make sure tha the file exists
    if (!boost::filesystem::exists("../data/database.csv")) {

        std::cout << "Database not found! Make sure to run ./Routes --rebuild from the build directory.\n";
        exit(0);

    }

    std::ifstream database_file = std::ifstream("../data/database.csv");
    std::cout << "Loading database from disk...\n";

    // Run through each line of the file, discarding the first.
    // The first is discarded because it is the header for the csv files to make the database human readable/
    std::string line;
    std::getline(database_file, line);

    while (std::getline(database_file, line)) {

        // There are 5 entries on the csv, the first is the path
        // The second, third, fourth and fifth are the origin and size of the dataset
        std::istringstream line_stream = std::istringstream(line);
        std::string temp;

        Entry entry;
        std::getline(line_stream, entry.path, ',');

        // Get the origin
        std::getline(line_stream, temp, ',');
        entry.origin.x = (float)atof(temp.c_str());
        std::getline(line_stream, temp, ',');
        entry.origin.y = (float)atof(temp.c_str());

        // Get the size
        std::getline(line_stream, temp, ',');
        entry.size.x = (float)atof(temp.c_str());
        std::getline(line_stream, temp, ',');
        entry.size.y = (float)atof(temp.c_str());

        // Add it to the database
        _entries.push_back(entry);

    }

    return true;

}

void DB::build() {

    // Open up an out file stream
    std::ofstream out_file = std::ofstream("../data/database.csv");
    out_file << "name,origin-x,origin-y,size-x,size-y\n";

    // Open up the directory in the boost filesystem
    boost::filesystem::path data_path = boost::filesystem::path("../data/");

    for(auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(data_path), {})) {

        // Make sure that this is not the database file
        // Also on Mac no .DS_Store
        if (entry.path().compare("../data/database.csv") && entry.path().compare("../data/.DS_Store")) {

            // Load up the GDAL file
            GDALDataset* dataset = (GDALDataset*)GDALOpen(entry.path().c_str(), GA_ReadOnly);

            // Read the transform, this has everything we need
            double transform[8];
            dataset->GetGeoTransform(transform);

            // Get the origin of the dataset
            glm::vec2 origin = glm::vec2(transform[0], transform[3]);

            // Get the size in degree of the dataset
            // Invert y because its negative
            glm::vec2 size = glm::vec2((float)dataset->GetRasterXSize() * transform[1],
                                       (float)dataset->GetRasterYSize() * transform[5]);

            // Write out the entry
            std::string path_e = std::string(entry.path().c_str());
            out_file << path_e << "," << origin.x << "," << origin.y << "," << size.x << "," << size.y << "\n";

            GDALClose(dataset);

        }

    }

    out_file.close();

}

std::vector<std::string> DB::getRequiredDatasets(glm::vec2 start, glm::vec2 dest) {

    // First we calculate a vector to construct the 4 points of the box from the start and destination position
    glm::vec3 start_3 = glm::vec3(start.x, start.y, 0.0);
    glm::vec3 dest_3  = glm::vec3(dest.x,  dest.y, 0.0);

    glm::vec3 path_vec = dest_3 - start_3;

    // Get the vector that is perpendicular to the path vector and .5 degrees long
    glm::vec3 corner_vec = glm::cross(path_vec, glm::vec3(0.0, 0.0, 1.0));
    corner_vec = glm::normalize(corner_vec) * 0.5f;

    // Create two new starts for 2 new rays
    glm::vec3 start_a = start_3 + path_vec;
    glm::vec3 start_b = start_3 - path_vec;

    std::vector<std::string> intersected;

    // Check everything for three intersections
    for (int i = 0; i < _entries.size(); i++) {

        // Check for intersection with all three rays
        if (lineIntersectsDatSet(i, start_3, path_vec) /* ||
            lineIntersectsDatSet(i, start_a, path_vec) || */
            /* lineIntersectsDatSet(i, start_b, path_vec) */)
            intersected.push_back(_entries[i].path);

    }

    return intersected;

}

bool DB::lineIntersectsDatSet(int index, const glm::vec3 &start, const glm::vec3 &direction) {

    Entry& entry = _entries[index];

    // Calculate if the line intersects this dataset.
    // Algorithm taken from https://stackoverflow.com/questions/99353/how-to-test-if-a-line-segment-intersects-an-axis-aligned-rectange-in-2d, metamal's answer.

    float minx = glm::min(start.x, start.x + direction.x);
    float maxx = glm::max(start.x, start.x + direction.x);

    // Min x of the entry is the origin, max is origin + size
    if (maxx > entry.origin.x + entry.size.x)
        maxx = entry.origin.x + entry.size.x;

    if (minx < entry.origin.x)
        minx = entry.origin.x;

    // Check if there was no intersection on the x
    if (minx > maxx)
        return false;

    float miny = glm::min(start.y, start.y + direction.y);
    float maxy = glm::max(start.y, start.y + direction.y);

    // Get min y and max y
    float dx = direction.x;

    if (fabs(dx) > X_EPSILON) {

        float m = direction.y / dx;
        float k = start.y - m * start.x;

        miny = m * minx + k;
        maxy = m * maxx + k;

    }

    // Swap them if they are out of order
    if (miny > maxy) {

        float temp = miny;
        miny = maxy;
        maxy = temp;

    }

    // Check for intersection, min is the origin + size and max is the origin because y is negative
    if (miny < entry.origin.y + entry.size.y)
        miny =  entry.origin.y + entry.size.y;

    if (maxy > entry.origin.y)
        maxy = entry.origin.y;

    return maxy >= miny;

}