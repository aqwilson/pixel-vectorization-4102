// This code get an image as input and threshold the pixels less than 50 to 0 values.
// You can apply multiple thesholding on image using several conditions on image pixels.

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <limits>

#include "EdgeDetection.h"

namespace edgeDetection {

int edgeDetectionMain()
{
    std::cout << "Edge Detection" << std::endl;
    std::cout << "==============" << std::endl;

    // Parameters for image A
    std::cout << "Enter the input image path: " << std::endl;
    cv::Mat src;
    while (true) {
        std::string path;
        std::getline(std::cin, path);

        // Attempt to load the image (as greyscale). Ask user to re-enter path on failure.
        src = cv::imread(path, 0);
        if (src.rows > 0) {
            break;
        } else {
            std::cout << "Image not found or not readable. Try again: " << std::endl;
        }
    }
    std::cout << "Enter the standard deviation for the Gaussian smoothing filter: " << std::endl;
    std::string stdDevStr;
    std::getline(std::cin, stdDevStr);
    float stdDev = std::stof(stdDevStr);
    std::cout << "Gaussian filter kernel size is " << (2 * std::ceil(3 * stdDev) + 1) << std::endl;

    std::cout << "Enter the gradient intensity threshold: " << std::endl;
    std::string intensityThresholdStr;
    std::getline(std::cin, intensityThresholdStr);
    float intensityThreshold = std::stof(intensityThresholdStr);

    std::cout << "Enter the output image path (include a common image file extension such as .png): " << std::endl;
    std::string outPath;
    std::getline(std::cin, outPath);


    // Detect edges
    cv::Mat dst;
    edgeDetection(src, dst, stdDev, intensityThreshold);


    // Create a window to display results
    const char* windowName = "Edge Detection";
    cv::namedWindow(windowName, CV_WINDOW_AUTOSIZE);

    // Show the image
    cv::imshow(windowName, dst);

    // Save the image
    cv::imwrite(outPath, dst);

    // Wait until user finishes program
    cv::waitKey(0);
    return 0;
}

template <class T>
T boundPixelValue(float floatValue) {
    T tMin = std::numeric_limits<T>().lowest();
    float floatMin = std::numeric_limits<float>().lowest();
    if (tMin > floatMin) {
        floatMin = tMin;
    }

    T tMax = std::numeric_limits<T>().max();
    float floatMax = std::numeric_limits<float>().max();
    if (tMax < floatMax) {
        floatMax = tMax;
    }

    return std::max(floatMin, std::min(floatMax, floatValue));
}

template <class inType, class outType>
void crossCorrelation2d(const cv::Mat& in, cv::Mat& out, const cv::Mat& kernel) {
    out = cv::Mat_<outType>(in.rows, in.cols);

    for (int ir = 0; ir < in.rows; ir++) {
        for (int ic = 0; ic < in.cols; ic++) {
            float pixelSum = 0;
            float weightSum = 0;
            float usedWeightSum = 0;

            for (int kr = 0; kr < kernel.rows; kr++) {
                for (int kc = 0; kc < kernel.cols; kc++) {
                    float kernelWeight = kernel.at<float>(kr, kc);
                    weightSum += std::abs(kernelWeight);

                    int sampleR = ir + kr - kernel.rows / 2;
                    int sampleC = ic + kc - kernel.cols / 2;

                    if (sampleR >= 0 && sampleC >= 0 && sampleR < in.rows && sampleC < in.cols) {
                        inType sampleValue = in.at<inType>(sampleR, sampleC);
                        pixelSum += sampleValue * kernelWeight;
                        usedWeightSum += std::abs(kernelWeight);
                    }
                }
            }

            pixelSum *= weightSum / usedWeightSum;
            out.at<outType>(ir, ic) = boundPixelValue<outType>(pixelSum);
        }
    }
}

template <class inType, class outType>
void convolve2d(const cv::Mat& in, cv::Mat& out, const cv::Mat& kernel) {
    cv::Mat reversedKernel = cv::Mat(kernel.rows, kernel.cols, kernel.type());

    // Reverse the kernel in both the x and y directions (equivalent to a half rotation)
    for (int r = 0; r < kernel.rows; r++) {
        for (int c = 0; c < kernel.cols; c++) {
            reversedKernel.at<float>(r, c) = kernel.at<float>(kernel.rows - 1 - r, kernel.cols - 1 - c);
        }
    }

    crossCorrelation2d<inType, outType>(in, out, reversedKernel);
}

cv::Mat sobelKernelX() {
    return cv::Mat_<float>(3, 3)
        << -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1;
}

cv::Mat sobelKernelY() {
    return cv::Mat_<float>(3, 3)
        << -1, -2, -1,
        0, 0, 0,
        1, 2, 1;
}

cv::Mat gaussianBlurKernel2d(float stdDev) {
    float var = stdDev * stdDev;
    int dim = 2 * std::ceil(3 * stdDev) + 1;

    cv::Mat kernel = cv::Mat::zeros(dim, dim, CV_32F) / ((double)dim * dim);
    for (int r = 0; r < kernel.rows; r++) {
        for (int c = 0; c < kernel.cols; c++) {
            kernel.at<float>(r, c) = gaussianSample2d(c - kernel.cols / 2, r - kernel.rows / 2, var);
        }
    }

    return kernel;
}

float gaussianSample2d(float x, float y, float var) {
    return 0.5f / PI / var * std::exp(-(x * x + y * y) * 0.5f / var);
}

void calcuateGradients(const cv::Mat& gradX, const cv::Mat& gradY, cv::Mat& outIntensities, cv::Mat& outAngles) {
    outIntensities = cv::Mat::zeros(gradX.rows, gradX.cols, CV_32F);
    outAngles = cv::Mat::zeros(gradX.rows, gradX.cols, CV_32F);
    for (int r = 0; r < gradX.rows; r++) {
        for (int c = 0; c < gradX.cols; c++) {
            float sobelXValue = gradX.at<float>(r, c);
            float sobelYValue = gradY.at<float>(r, c);
            outIntensities.at<float>(r, c) = std::sqrt(sobelXValue * sobelXValue + sobelYValue * sobelYValue);
            outAngles.at<float>(r, c) = std::atan2(sobelYValue, sobelXValue);
        }
    }
}

void thresholdAndNonMaximumSuppression(const cv::Mat& gradIntensities, const cv::Mat& gradAngles, cv::Mat& out, float threshold) {
    out = cv::Mat::ones(gradIntensities.rows, gradIntensities.cols, CV_8UC1) * 255;
    for (int r = 0; r < gradIntensities.rows; r++) {
        for (int c = 0; c < gradIntensities.cols; c++) {
            float gradIntensity = gradIntensities.at<float>(r, c);

            // Threshold the gradient intensity
            if (gradIntensity < threshold) {
                continue;
            }

            // Non-maximum suppression

            float gradAngle = gradAngles.at<float>(r, c);

            // Assign a section number between 0 and 7 based on the gradient angle
            int section = static_cast<int>((gradAngle + PI * 1.125f) / PI * 0.25f) % 8;

            // Recall that angles are clockwise because the y axis is inverted.

            // Set the back and forward pixels (with respect to the gradient direction) based on the section of the
            // gradient angle
            cv::Point back = cv::Point(c, r);
            cv::Point forward = cv::Point(c, r);

            if (section < 2 || section > 6) {
                // Right half
                back.x--;
                forward.x++;
            } else if (section > 2 && section < 6) {
                // Left half
                back.x++;
                forward.x--;
            }

            if (section > 0 && section < 4) {
                // Bottom half
                back.y--;
                forward.y++;
            } else if (section > 4) {
                // Top half
                back.y++;
                forward.y--;
            }

            if (back.x < 0 || back.y < 0
                || back.x >= gradIntensities.cols || back.y >= gradIntensities.rows
                || forward.x < 0 || forward.y < 0
                || forward.x >= gradIntensities.cols || forward.y >= gradIntensities.rows) {
                continue;
            }

            float backIntensity = gradIntensities.at<float>(back);
            float forwardIntensity = gradIntensities.at<float>(forward);

            // Only show this pixel if its intensity is greater than both the back and forward intensity.
            // In the case of a tie, the furthest pixel along the gradient is shown
            if (gradIntensity >= backIntensity && gradIntensity > forwardIntensity) {
                out.at<uchar>(r, c) = 0;
            }
        }
    }
}

void edgeDetection(const cv::Mat& src, cv::Mat& dst, float stdDev, float threshold) {
    // Apply Gaussian smoothing
    cv::Mat smoothed;
    crossCorrelation2d<uchar, uchar>(src, smoothed, gaussianBlurKernel2d(stdDev));

    // Get horizontal Sobel image
    cv::Mat sobelX;
    crossCorrelation2d<uchar, float>(smoothed, sobelX, sobelKernelX());

    // Get vertical Sobel image
    cv::Mat sobelY;
    crossCorrelation2d<uchar, float>(smoothed, sobelY, sobelKernelY());

    // Calculate gradients
    cv::Mat gradIntensities;
    cv::Mat gradAngles;
    calcuateGradients(sobelX, sobelY, gradIntensities, gradAngles);

    // Threshold gradient intensity and apply non-maximum suppression
    thresholdAndNonMaximumSuppression(gradIntensities, gradAngles, dst, threshold);
}

}