#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include "opencv2/highgui/highgui.hpp"

#include <string>

#include "Polygon.h"
#include "GenerateSvg.h"
#include "PixelGraph.h"

extern const bool DEBUG_SHOW_GRAPH = 0;
extern const bool DEBUG_SHOW_GRID = 0;

int pixelToVector() {
    const char* windowName = "Pixel to Vector";
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);

    // Load the image
    cv::Mat src = cv::imread("./input/Watt_Original.png");

    PixelGraph* p = new PixelGraph(&src);

    p->generateGraph();
    p->brutePrune();
    p->runHeuristics();

    cv::Size s = cv::Size(src.cols * 4, src.rows * 4);
    cv::Mat* voronoi = new cv::Mat(s, src.type());

    //p->computeVoronoi(voronoi);
    //cv::imwrite("voronoi.png", *voronoi);

    std::vector<Polygon>* polygons = new std::vector<Polygon>();
    p->computeAllPolygons(*polygons);

    generateSvg(src.cols, src.rows, *polygons, p, "bitmap_to_svg.svg");

    // Show the image
    // cv::imshow(windowName, src);
    // cv::waitKey(0);

    delete p;
    return 0;
}

int main() {
    return pixelToVector();
}