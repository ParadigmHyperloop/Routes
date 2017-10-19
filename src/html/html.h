//
// Created by Logan on 10/18/17.
//

#ifndef ROUTES_HTML_H
#define ROUTES_HTML_H

#include <glm/glm.hpp>
#include <fstream>

#include "../elevation/elevation.h"
#include "../bezier/bezier.h"

/** */


/**
 * This class is responsible for creating html output for a calculated path.
 * It reads template.html out of the htdocs folder and creates an index.html.
 * This index.html file needs to be hosted with a local web server on port 8888.
 * This is a requirement of MapBoxJS for loading the stylesheet that we are using.
 */
class HTML {

    public:

        /**
         * Generates the index.html file which allows displaying the path in the web browser using the MapBoxJS api.
         *
         * @param path
         * The path that should be added to the map. This should be the output of a genetic cost function and
         * all of the points are measured in meters.
         *
         * @param data
         * The elevation data that this path was born from so it can be converted to longitude and latitude.
         */
        static void generateHTMLForPath(const std::vector<glm::vec3>& path, const ElevationData& data);

};


#endif //ROUTES_HTML_H
