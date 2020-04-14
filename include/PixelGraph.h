#pragma once

#include <vector>
#include <unordered_map>
#include <opencv2/imgproc/imgproc.hpp>
#include "Node.h"
#include "Polygon.h"

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

    void computeAllPolygons(std::vector<Polygon> &polygons);

private:
    enum class FillType { FULL, NO_CORNER, THREE_CORNER, FOUR_CORNER, DIAGONAL, CARD_END, DIAG_END, NONE };

    class GridIntersection;
    class WalkInfo {
    public:
        WalkInfo();
        WalkInfo(GridIntersection* destination, cv::Point2f cornerPosition);

        GridIntersection* destination;
        cv::Point2f cornerPosition;
    };
    
    // Contains information about how to walk along edges to generate contours, and which corners are shifted according
    // to the rules for simplified voronoi
    class GridIntersection {
    public:
        cv::Point2i pos;

        // Maps the source node to a tuple containing the destination node, whether the corner is shifted,
        // and the corner position
        std::unordered_map<GridIntersection*, WalkInfo> contourWalkMap;
    };

    std::vector<std::vector<GridIntersection>> grid;

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

    GridIntersection* getIntersectionIfExists(int r, int c);
    void initGridIntersection(int row, int col, GridIntersection& intersection);
};
