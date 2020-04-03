#pragma once

#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include "Node.h"

class PixelGraph {

public:
    enum ConnType {CONNECTED, DISCONNECTED, INVALID, UNTESTED};

    const float ISLAND_NUM = 5.0f; // magic number 5 is from the paper

    std::vector<std::vector<Node*>*>* graph;
    cv::Mat* img;

    PixelGraph();
    PixelGraph(cv::Mat*);
    ~PixelGraph();

    Node* getIfExists(int r, int c);
    void generateGraph();
    void brutePrune();
    void runHeuristics();

    void computeVoronoi(cv::Mat*);
    void computeVoronoi2(cv::Mat*);

private:
    enum class FillType { FULL, NO_CORNER, THREE_CORNER, FOUR_CORNER, DIAGONAL, CARD_END, DIAG_END, NONE };

    void runCurveHeuristic(cv::Point, cv::Point2f*);
    void runSparseHeuristic(cv::Point, cv::Point2f*);
    void runIslandHeuristic(cv::Point, cv::Point2f*);

    int GetCurveLength(Node*, Node*);
    Node* FindSecondConnection(Node*, Node*);

    void connections(std::vector<std::vector<ConnType>>&, cv::Point);
    int calculateValence(cv::Point);

    bool comparePixels(Node*, Node*);
    void firstNeighbourhoodPrunePass(Node*);
    void flatPruning(Node*);

    void renderFullPixel(cv::Mat&, cv::Point);
    void renderFourDiag(cv::Mat&, cv::Point);
    void renderThreeDiag(cv::Mat&, cv::Point);
    void renderNoCornerPixel(cv::Mat&, cv::Point);
    void renderDiagonalPixel(cv::Mat&, cv::Point);
    void renderCardEndPixel(cv::Mat&, cv::Point);
    void renderDiagEndPixel(cv::Mat&, cv::Point);
    void renderNonePixel(cv::Mat&, cv::Point);
    void calculateFillType(std::vector<std::vector<FillType>>&);
    bool hasExternalDiagonals(Node*, int, int);
    int countExternalDiagonals(Node*, int, int);
};
