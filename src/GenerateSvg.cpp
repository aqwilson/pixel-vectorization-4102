// This code get an image as input and threshold the pixels less than 50 to 0 values.
// You can apply multiple thesholding on image using several conditions on image pixels.

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>

#include "GenerateSvg.h"

extern const bool DEBUG_SHOW_GRAPH;
extern const bool DEBUG_SHOW_GRID;

int generateSvg(const cv::Mat& src, const PixelGraph* graph, const char* outPath) {
    const float strokeWidth = 0.07f;

    std::stringstream sstr = std::stringstream();
    sstr << "<svg version=\"1.1\" baseProfile = \"full\" width = \"" << src.cols << "\" height = \""
        << src.rows << "\" xmlns = \"http://www.w3.org/2000/svg\">" << std::endl;
    for (int r = 0; r < src.rows; r++) {
        for (int c = 0; c < src.cols; c++) {
            cv::Vec3b bgr = src.at<cv::Vec3b>(r, c);

            std::string colorStr = (std::stringstream() << "rgb(" << +bgr[2] << ", " << +bgr[1] << ", "
                << +bgr[0] << ")").str();

            sstr << "    <rect x = \"" << c << "\" y = \"" << r << "\" width = \"1\" height = \"1\" fill = \""
                << colorStr << "\" stroke = \"" << colorStr << "\" stroke-width = \"" << strokeWidth <<"\"/>"
                << std::endl;

            /*if (graph->graph->at(r)->at(c)->topLeft) {
                sstr << "    <polygon points = \""
                    << (c) << "," << (r + 1) << " "
                    << (c - 1) << "," << (r) << " "
                    << (c) << "," << (r - 1) << " "
                    << (c + 1) << "," << (r) << "\" fill = \""
                    << colorStr << "\" stroke = \"" << colorStr << "\" stroke-width = \"0\"/>"
                    << std::endl;
            }*/
        }
    }

    if (DEBUG_SHOW_GRID) {
        for (int r = 0; r < src.rows; r++) {
            for (int c = 0; c < src.cols; c++) {
                sstr << "    <rect x = \"" << c << "\" y = \"" << r
                    << "\" width = \"1\" height = \"1\" fill = \"none\" stroke = \"red\" stroke-width = \"0.02\"/>"
                    << std::endl;
            }
        }
    }

    if (DEBUG_SHOW_GRAPH) {
        for (int r = 0; r < src.rows; r++) {
            for (int c = 0; c < src.cols; c++) {
                if (graph->graph->at(r)->at(c)->top) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c + 0.5f) << "\" y2 = \"" << (r - 0.5f)
                        << "\" stroke = \"red\" stroke-width = \"0.2\"/>" << std::endl;
                }
                if (graph->graph->at(r)->at(c)->bottom) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c + 0.5f) << "\" y2 = \"" << (r + 1.5f)
                        << "\" stroke = \"green\" stroke-width = \"0.2\"/>" << std::endl;
                }

                if (graph->graph->at(r)->at(c)->left) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c - 0.5f) << "\" y2 = \"" << (r + 0.5f)
                        << "\" stroke = \"orange\" stroke-width = \"0.2\"/>" << std::endl;
                }
                if (graph->graph->at(r)->at(c)->right) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c + 1.5f) << "\" y2 = \"" << (r + 0.5f)
                        << "\" stroke = \"purple\" stroke-width = \"0.2\"/>" << std::endl;
                }

                if (graph->graph->at(r)->at(c)->topLeft) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c - 0.5f) << "\" y2 = \"" << (r - 0.5f)
                        << "\" stroke = \"blue\" stroke-width = \"0.2\"/>" << std::endl;
                }
                if (graph->graph->at(r)->at(c)->bottomRight) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c + 1.5f) << "\" y2 = \"" << (r + 1.5f)
                        << "\" stroke = \"cyan\" stroke-width = \"0.2\"/>" << std::endl;
                }

                if (graph->graph->at(r)->at(c)->topRight) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c + 1.5f) << "\" y2 = \"" << (r - 0.5f)
                        << "\" stroke = \"yellow\" stroke-width = \"0.2\"/>" << std::endl;
                }
                if (graph->graph->at(r)->at(c)->bottomLeft) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c - 0.5f) << "\" y2 = \"" << (r + 1.5f)
                        << "\" stroke = \"magenta\" stroke-width = \"0.2\"/>" << std::endl;
                }
            }
        }
    }
    
    sstr << "</svg>" << std::endl;

    std::ofstream outputFile;
    outputFile.open(outPath);
    outputFile << sstr.rdbuf();
    outputFile.close();

    return 0;
}