#pragma once

#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include "Node.h"

using Contour = std::vector<cv::Point2f>;

class Polygon {

public:
    Contour contour;
    std::vector<Polygon> holes;
    cv::Vec3b color;

    Polygon();
    ~Polygon();

private:

};
