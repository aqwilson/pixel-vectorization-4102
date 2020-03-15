#pragma once

#include "opencv2/imgproc/imgproc.hpp"

#include "PixelGraph.h"

int generateSvg(const cv::Mat &src, const PixelGraph *graph, const char *outPath);