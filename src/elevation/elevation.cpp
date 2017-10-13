//
// Created by Logan on 10/12/17.
//

#include "elevation.h"

bool ElevationData::_is_initialized = ElevationData::initGDAL();

bool ElevationData::initGDAL() {



}

ElevationData::ElevationData(const std::string& file_path) {



}

int ElevationData::getWidth() const { return _width; }
int ElevationData::getHeight() const { return _height; }

float ElevationData::getWidthInMeters() const { return _width_meters; }
float ElevationData::getHeightInMeters() const { return _height_meters; }

float ElevationData::getMinElevation() const { return _elevation_min; }
float ElevationData::getMaxElevation() const { return _elevation_max; }


glm::vec2 ElevationData::convertPixelsToMeters(const glm::ivec2& pos_pixels) const {

    return glm::vec2();

}

glm::ivec2 ElevationData::convertMetersToPixels(const glm::vec2& pos_meters) const {

    return glm::ivec2();

}

glm::vec2 ElevationData::metersToLongitudeLatitude(const glm::vec2& pos_meters) const {

    return glm::vec2();

}

glm::vec2 ElevationData::pixelsToLongitudeLatitude(const glm::ivec2& pos_pixels) const {

    return glm::vec2();

}

glm::vec3 ElevationData::metersToMetersAndElevation(const glm::vec2& pos_meters) const {

    return glm::vec3();

}


glm::vec3 ElevationData::pixelsToMetersAndElevation(const glm::ivec2& pos_pixels) const {

    return glm::vec3();

}

