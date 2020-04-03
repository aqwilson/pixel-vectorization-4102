#include "EdgeDetection.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include "opencv2/highgui/highgui.hpp"

#include <string>

#include "GenerateSvg.h"
#include "PixelGraph.h"

extern const bool DEBUG_SHOW_GRAPH = 1;
extern const bool DEBUG_SHOW_GRID = 0;

int pixelToVector() {
    const char* windowName = "Pixel to Vector";
    cv::namedWindow(windowName, CV_WINDOW_AUTOSIZE);

    // Load the image
    cv::Mat src = cv::imread("./input/Watt_Original.png");

    PixelGraph* p = new PixelGraph(&src);

    p->generateGraph();
    p->brutePrune();
    p->runHeuristics();

    cv::Size s = cv::Size(src.cols * 4, src.rows * 4);
    cv::Size s2 = cv::Size(src.cols * 3, src.rows * 3);
    cv::Mat* voronoi = new cv::Mat(s, src.type());
    cv::Mat* voronoi2 = new cv::Mat(s2, src.type());

    p->computeVoronoi(voronoi);
    p->computeVoronoi2(voronoi2);
    cv::imwrite("voronoi.png", *voronoi);
    cv::imwrite("voronoi2.png", *voronoi2);

    generateSvg(src, p, "bitmap_to_svg.svg");

    // Show the image
    // cv::imshow(windowName, src);
    // cv::waitKey(0);

    delete p;
    return 0;
}

int main() {
    return pixelToVector();
}