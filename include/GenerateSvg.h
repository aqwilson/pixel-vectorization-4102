#pragma once

#include "opencv2/imgproc/imgproc.hpp"

#include "PixelGraph.h"

int generateSvg(const cv::Mat &src, const PixelGraph *graph, const char *outPath);

int generateSvg(int width, int height, const std::vector<Polygon>& polygons, const PixelGraph* graph,
        const char* outPath);