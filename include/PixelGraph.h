#pragma once

#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include "Node.h"

class PixelGraph {

public:
    std::vector<std::vector<Node*>*>* graph;
    cv::Mat* img;

    PixelGraph();
    PixelGraph(cv::Mat*);
    ~PixelGraph();

    void generateGraph();
    void brutePrune();
    void runHeuristics();
};
