#include "EdgeDetection.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include "opencv2/highgui/highgui.hpp"

#include "PixelGraph.h"

int main() {


    cv::Mat src;

    // Attempt to load the image (as greyscale). Ask user to re-enter path on failure.
    src = cv::imread("./input/Cactuar_Original.png");
    cv::namedWindow("thingy", CV_WINDOW_AUTOSIZE);

    PixelGraph* p = new PixelGraph(&src);

    p->generateGraph();
    p->brutePrune();
    p->runHeuristics();

    // Show the image
    cv::imshow("thingy", src);
    cv::waitKey(0);
    return 0;
    //edgeDetection::edgeDetectionMain();
}