#include "Polygon.h"

Polygon::Polygon() {
    
}

Polygon::~Polygon() {
    
}



std::vector<Polygon::ControlAnchor> Polygon::GenerateControlAnchors(Contour anchors) {

    std::vector<Polygon::ControlAnchor> controlAnchors = std::vector<Polygon::ControlAnchor>();

    for (int i = 0; i < anchors.size(); i++) {
        controlAnchors.push_back(CreateControlAnchor(GetPoint(i, anchors), GetPoint(i - 1, anchors), GetPoint(i + 1, anchors)));
    }

    return controlAnchors;
}

Polygon::ControlAnchor Polygon::CreateControlAnchor(cv::Point2f currentPoint, cv::Point2f previousPoint, cv::Point2f nextPoint) {

    float tangentLength = sqrt(pow(nextPoint.x - previousPoint.x, 2) + pow(nextPoint.y - previousPoint.y, 2));
    float tangentAngle = atan2(nextPoint.y - previousPoint.y, nextPoint.x - previousPoint.x);

    float tangentAngleReverse = tangentAngle + CV_PI;

    float smoothingFactor = 0.2f;
    float smoothedLength = tangentLength * smoothingFactor;

    ControlAnchor ca = ControlAnchor();

    ca.nextControlPoint.x = currentPoint.x + cos(tangentAngle) * smoothedLength;
    ca.nextControlPoint.y = currentPoint.y + sin(tangentAngle) * smoothedLength;

    ca.previousControlPoint.x = currentPoint.x + cos(tangentAngleReverse) * smoothedLength;
    ca.previousControlPoint.y = currentPoint.y + sin(tangentAngleReverse) * smoothedLength;

    ca.anchorPoint = currentPoint;

    return ca;
}
