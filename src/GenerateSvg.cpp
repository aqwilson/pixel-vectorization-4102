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
        }
    }

    for (int r = 0; r < src.rows; r++) {
        for (int c = 0; c < src.cols; c++) {
            cv::Vec3b bgr = src.at<cv::Vec3b>(r, c);

            std::string colorStr = (std::stringstream() << "rgb(" << +bgr[2] << ", " << +bgr[1] << ", "
                << +bgr[0] << ")").str();

            // top left diagonals
            if (graph->graph->at(r)->at(c)->topLeft) {
                sstr << "    <polygon points = \""
                    << (c) << "," << (r + 0.5f) << " "
                    << (c - 0.5f) << "," << (r) << " "
                    << (c) << "," << (r - 0.5f) << " "
                    << (c + 0.5f) << "," << (r) << "\" fill = \""
                    << colorStr << "\" stroke = \"" << colorStr << "\" stroke-width = \"0\"/>"
                    << std::endl;
            }

            // top right diagonals
            if (graph->graph->at(r)->at(c)->topRight) {
                sstr << "    <polygon points = \""
                    << (c + 0.5f) << "," << (r) << " "
                    << (c + 1) << "," << (r - 0.5f) << " "
                    << (c + 1.5f) << "," << (r) << " "
                    << (c + 1) << "," << (r + 0.5f) << "\" fill = \""
                    << colorStr << "\" stroke = \"" << colorStr << "\" stroke-width = \"0\"/>"
                    << std::endl;
            }
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
                const float graphStrokeWidth = 0.05f;

                if (graph->graph->at(r)->at(c)->top) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c + 0.5f) << "\" y2 = \"" << (r - 0.5f)
                        << "\" stroke = \"red\" stroke-width = \"" << graphStrokeWidth << "\"/>" << std::endl;
                }
                if (graph->graph->at(r)->at(c)->bottom) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c + 0.5f) << "\" y2 = \"" << (r + 1.5f)
                        << "\" stroke = \"green\" stroke-width = \"" << graphStrokeWidth << "\"/>" << std::endl;
                }

                if (graph->graph->at(r)->at(c)->left) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c - 0.5f) << "\" y2 = \"" << (r + 0.5f)
                        << "\" stroke = \"orange\" stroke-width = \"" << graphStrokeWidth << "\"/>" << std::endl;
                }
                if (graph->graph->at(r)->at(c)->right) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c + 1.5f) << "\" y2 = \"" << (r + 0.5f)
                        << "\" stroke = \"purple\" stroke-width = \"" << graphStrokeWidth << "\"/>" << std::endl;
                }

                if (graph->graph->at(r)->at(c)->topLeft) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c - 0.5f) << "\" y2 = \"" << (r - 0.5f)
                        << "\" stroke = \"blue\" stroke-width = \"" << graphStrokeWidth << "\"/>" << std::endl;
                }
                if (graph->graph->at(r)->at(c)->bottomRight) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c + 1.5f) << "\" y2 = \"" << (r + 1.5f)
                        << "\" stroke = \"cyan\" stroke-width = \"" << graphStrokeWidth << "\"/>" << std::endl;
                }

                if (graph->graph->at(r)->at(c)->topRight) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c + 1.5f) << "\" y2 = \"" << (r - 0.5f)
                        << "\" stroke = \"yellow\" stroke-width = \"" << graphStrokeWidth << "\"/>" << std::endl;
                }
                if (graph->graph->at(r)->at(c)->bottomLeft) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c - 0.5f) << "\" y2 = \"" << (r + 1.5f)
                        << "\" stroke = \"magenta\" stroke-width = \"" << graphStrokeWidth << "\"/>" << std::endl;
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

int generateSvg(int width, int height, const std::vector<Polygon>& polygons, const PixelGraph* graph,
        const char* outPath) {
    const float strokeWidth = 0.05f;

    std::stringstream sstr = std::stringstream();
    sstr << "<svg version=\"1.1\" baseProfile = \"full\" width = \"" << width << "\" height = \""
        << height << "\" xmlns = \"http://www.w3.org/2000/svg\">" << std::endl;
    for (const Polygon& poly : polygons) {
        cv::Vec3b bgr = poly.color;
        std::string colorStr = (std::stringstream() << "rgb(" << +bgr[2] << ", " << +bgr[1] << ", "
            << +bgr[0] << ")").str();

        sstr << "    <polygon points = \"";
        for (cv::Point2f p : poly.contour) {
            sstr << p.x << "," << p.y << " ";
        }
        sstr << "\" fill = \"" << colorStr << "\" stroke = \"" << colorStr << "\" stroke-width = \"" << strokeWidth
            << "\"/>" << std::endl;
    }

    if (DEBUG_SHOW_GRID) {
        for (int r = 0; r < height; r++) {
            for (int c = 0; c < width; c++) {
                sstr << "    <rect x = \"" << c << "\" y = \"" << r
                    << "\" width = \"1\" height = \"1\" fill = \"none\" stroke = \"red\" stroke-width = \"0.02\"/>"
                    << std::endl;
            }
        }
    }

    if (DEBUG_SHOW_GRAPH) {
        for (int r = 0; r < height; r++) {
            for (int c = 0; c < width; c++) {
                const float graphStrokeWidth = 0.05f;

                if (graph->graph->at(r)->at(c)->top) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c + 0.5f) << "\" y2 = \"" << (r - 0.5f)
                        << "\" stroke = \"red\" stroke-width = \"" << graphStrokeWidth << "\"/>" << std::endl;
                }
                if (graph->graph->at(r)->at(c)->bottom) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c + 0.5f) << "\" y2 = \"" << (r + 1.5f)
                        << "\" stroke = \"green\" stroke-width = \"" << graphStrokeWidth << "\"/>" << std::endl;
                }

                if (graph->graph->at(r)->at(c)->left) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c - 0.5f) << "\" y2 = \"" << (r + 0.5f)
                        << "\" stroke = \"orange\" stroke-width = \"" << graphStrokeWidth << "\"/>" << std::endl;
                }
                if (graph->graph->at(r)->at(c)->right) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c + 1.5f) << "\" y2 = \"" << (r + 0.5f)
                        << "\" stroke = \"purple\" stroke-width = \"" << graphStrokeWidth << "\"/>" << std::endl;
                }

                if (graph->graph->at(r)->at(c)->topLeft) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c - 0.5f) << "\" y2 = \"" << (r - 0.5f)
                        << "\" stroke = \"blue\" stroke-width = \"" << graphStrokeWidth << "\"/>" << std::endl;
                }
                if (graph->graph->at(r)->at(c)->bottomRight) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c + 1.5f) << "\" y2 = \"" << (r + 1.5f)
                        << "\" stroke = \"cyan\" stroke-width = \"" << graphStrokeWidth << "\"/>" << std::endl;
                }

                if (graph->graph->at(r)->at(c)->topRight) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c + 1.5f) << "\" y2 = \"" << (r - 0.5f)
                        << "\" stroke = \"yellow\" stroke-width = \"" << graphStrokeWidth << "\"/>" << std::endl;
                }
                if (graph->graph->at(r)->at(c)->bottomLeft) {
                    sstr << "    <line x1 = \"" << (c + 0.5f) << "\" y1 = \"" << (r + 0.5f)
                        << "\" x2 = \"" << (c - 0.5f) << "\" y2 = \"" << (r + 1.5f)
                        << "\" stroke = \"magenta\" stroke-width = \"" << graphStrokeWidth << "\"/>" << std::endl;
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



int generateSplineSvg(int width, int height, const std::vector<Polygon>& polygons, const PixelGraph* graph,
    const char* outPath) {
    const float strokeWidth = 0.07f;

    std::stringstream sstr = std::stringstream();
    sstr << "<svg version=\"1.1\" baseProfile = \"full\" width = \"" << width << "\" height = \""
        << height << "\" xmlns = \"http://www.w3.org/2000/svg\">" << std::endl;

    for (const Polygon& poly : polygons) {
        cv::Vec3b bgr = poly.color;
        std::string colorStr = (std::stringstream() << "rgb(" << +bgr[2] << ", " << +bgr[1] << ", "
            << +bgr[0] << ")").str();

        sstr << "    <path d=\"M " << poly.contour[0].x << " " << poly.contour[0].y << " ";
        
        std::vector<Polygon::ControlAnchor> controlAnchors = Polygon::GenerateControlAnchors(poly.contour);

        for (int i = 0; i < poly.contour.size(); i++) {
            Polygon::ControlAnchor currentPoint = Polygon::GetPoint(i, controlAnchors);
            Polygon::ControlAnchor nextPoint = Polygon::GetPoint(i + 1, controlAnchors);

            cv::Point2f cp1 = currentPoint.nextControlPoint;
            cv::Point2f cp2 = nextPoint.previousControlPoint;

            sstr << "C " << cp1.x << " " << cp1.y << " " << cp2.x << " " << cp2.y << " ";
            sstr << nextPoint.anchorPoint.x << " " << nextPoint.anchorPoint.y << " ";
        }
        sstr << "\" fill = \"" << colorStr << "\" stroke = \"" << colorStr << "\" stroke-width = \"0\"/>" << std::endl;
    }

    sstr << "</svg>" << std::endl;

    std::ofstream outputFile;
    outputFile.open(outPath);
    outputFile << sstr.rdbuf();
    outputFile.close();

    return 0;
}