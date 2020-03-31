#pragma once

#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include "Node.h"

class PixelGraph {

public:
    enum ConnType {CONNECTED, DISCONNECTED, INVALID, UNTESTED};

    std::vector<std::vector<Node*>*>* graph;
    cv::Mat* img;

    PixelGraph();
    PixelGraph(cv::Mat*);
    ~PixelGraph();

    Node* getIfExists(int r, int c);
    void generateGraph();
    void brutePrune();
    void runHeuristics();

private:
    void runCurveHeuristic(cv::Point, cv::Point2f*);
    void runSparseHeuristic(cv::Point, cv::Point2f*);
    void runIslandHeuristic(cv::Point, cv::Point2f*);

    void connections(std::vector<std::vector<ConnType>>&, cv::Point);
    int calculateValence(cv::Point);

    bool comparePixels(Node*, Node*);
    void firstNeighbourhoodPrunePass(Node*);
    void flatPruning(Node*);
};
