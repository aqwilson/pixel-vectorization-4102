#pragma once

#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include "Node.h"

using Contour = std::vector<cv::Point2f>;

class Polygon {

public:
    class ControlAnchor {
        public:
            cv::Point2f anchorPoint;
            cv::Point2f previousControlPoint;
            cv::Point2f nextControlPoint;
    };

    Contour contour;
    cv::Vec3b color;
    int depth;

    Polygon();
    ~Polygon();

    template <class T>
    static T GetPoint(int index, std::vector<T> anchors){
        if (index > (int)anchors.size() - 1) {
            index = index - anchors.size();
        }
        else if (index < 0) {
            index = index + anchors.size();
        }

        return anchors[index];
    }

    static std::vector<Polygon::ControlAnchor> Polygon::GenerateControlAnchors(Contour anchors);

private:
    static ControlAnchor Polygon::CreateControlAnchor(cv::Point2f currentPoint, cv::Point2f previousPoint, cv::Point2f nextPoint);


};
