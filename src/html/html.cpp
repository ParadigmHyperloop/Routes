//
// Created by Logan on 10/18/17.
//

#include "html.h"

void HTML::generateHTMLForPath(const std::vector<glm::vec3>& path, const ElevationData& data) {

    // First we load up the template.html from the htdocs folder.
    std::ifstream template_file = std::ifstream("../htdocs/template.html");

    // Read the file into a string
    std::string template_contents;
    std::string line;

    while (std::getline(template_file, line))
        template_contents += line;

    template_file.close();

    // Evaluate the bezier curve
    std::vector<glm::vec3> evaluated = Bezier::evaluateEntireBezierCurve(path, 100);

    std::cout << "Evaluated " << path[0].x << std::endl;

    // Get the path as a string that we can insert to the GEOJson
    std::string points_json = "[";
    for (int i = 0; i < evaluated.size(); i++) {

        // Evaluate the bezier curve
        glm::vec3& vec = evaluated[i];
        glm::vec2 conv = data.metersToLongitudeLatitude(glm::vec2(vec.x, vec.y));
        points_json += "[" + std::to_string(conv.x) + ","+ std::to_string(conv.y) + "],";

    }

    points_json += "]\n";

    // Replace the marker in template.html with the points json
    template_contents.replace(template_contents.find("<POINTS GO HERE>"), 16, points_json);

    // Write out the new file
    std::ofstream out_file = std::ofstream("../htdocs/index.html");
    out_file << template_contents;
    out_file.close();

}