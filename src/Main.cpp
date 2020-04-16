#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include "opencv2/highgui/highgui.hpp"

#include <string>

#include "Polygon.h"
#include "GenerateSvg.h"
#include "PixelGraph.h"
#include "AntiAliasClean.h"

extern const bool DEBUG_SHOW_GRAPH = 0;
extern const bool DEBUG_SHOW_GRID = 0;

void useAntiAlias(cv::Mat& cleanImg) {
    cv::Mat antiImg = cv::imread("./input/Tank_Anti.png");
    cleanImg = antiImg.clone(); 

    antiAliasCleanup(antiImg, cleanImg);
}

int pixelToVector(cv::Mat& src) {
    const char* windowName = "Pixel to Vector";
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);

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

    generateSplineSvg(src.cols, src.rows, *polygons, p, "bitmap_to_svg.svg");

    // Show the image
    // cv::imshow(windowName, src);
    // cv::waitKey(0);

    delete p;
    return 0;
}

int main() {
    bool applyAntiCleanup = true;

    if (applyAntiCleanup)
    {
        cv::Mat src;
        useAntiAlias(src);
        return pixelToVector(src);
    }
    else 
    {
        cv::Mat src = cv::imread("./input/Ocarina_Original.png");
        return pixelToVector(src);
    }
}