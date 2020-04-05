#pragma once

#include <vector>
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
    using PixelToPolygon = std::vector<std::vector<Polygon*>>;
    
    // Booleans determine which paths exist to traverse through this node, and also which corners are shifted
    class GridIntersection {
    public:
        // Corner shifts diagonally away from intersection by 1/4 pixel: applies to all turns -- CW and CCW
        //      / | ^
        //     / ^|\ \
        // <--+ / | \ +---
        // ----+  |  +--->
        // ===============
        // <---+  |  +----
        // ---+ \ | / +-->
        //     \ \|V /
        //      V | /
        bool topLeftShift;
        bool topRightShift;
        bool bottomLeftShift;
        bool bottomRightShift;

        // Walking in from the left: CW turn (polygon bottom left), straight (polygon bottom), CCW ("/" diagonal)
        //       | ^
        //       | |
        //       | |
        // ========|====
        // ----+---+--->
        //     | |
        //     V |
        bool leftToBottom;
        bool leftToRight;
        bool leftToTop;

        // Walking in from the top: CW turn (polygon top left), straight (polygon left), CCW ("\" diagonal)
        //     | |
        //     | |
        // <---+ |
        // ====|========
        //     +------->
        //     | |
        //     V |
        bool topToLeft;
        bool topToBottom;
        bool topToRight;

        // Walking in from the right: CW turn (polygon top right), straight (polygon top), CCW ("/" diagonal)
        //       | ^
        //       | |
        // <---+---+----
        // ====|========
        //     | |
        //     | |
        //     V |
        bool rightToTop;
        bool rightToLeft;
        bool rightToBottom;

        // Walking in from the bottom: CW turn (polygon bottom right), straight (polygon right), CCW ("\" diagonal)
        //       | ^
        //       | |
        // <-------+
        // ========|====
        //       | +--->
        //       | |
        //       | |
        bool bottomToRight;
        bool bottomToTop;
        bool bottomToLeft;
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

    void computePolygon(int startRow, int startCol, Polygon& polygon, PixelToPolygon& pixelToPolygon);

    void initGridIntersection(int row, int col, GridIntersection& intersection);
};
