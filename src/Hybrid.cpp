#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <limits>

#include "Hybrid.h"

namespace hybrid {

int hybridMain()
{
    std::cout << "Hybrid Image Generator" << std::endl;
    std::cout << "======================" << std::endl;

    // Parameters for image A
    std::cout << "Enter the image path for which to apply a low-pass filter: " << std::endl;
    cv::Mat srcA;
    while (true) {
        std::string path;
        std::getline(std::cin, path);

        // Attempt to load the image. Ask user to re-enter path on failure.
        srcA = cv::imread(path, 1);
        if (srcA.rows > 0) {
            break;
        } else {
            std::cout << "Image not found or not readable. Try again: " << std::endl;
        }
    }
    std::cout << "Enter the standard deviation for the Gaussian distribution of the low-pass filter: " << std::endl;
    std::string stdDevAStr;
    std::getline(std::cin, stdDevAStr);
    float stdDevA = std::stof(stdDevAStr);
    std::cout << "Low-pass filter kernel size is " << (2 * std::ceil(3 * stdDevA) + 1) << std::endl;

    // Parameters for image B
    std::cout << "Enter the image path for which to apply a high-pass filter: " << std::endl;
    cv::Mat srcB;
    while (true) {
        std::string path;
        std::getline(std::cin, path);

        // Attempt to load the image. Ask user to re-enter path on failure.
        srcB = cv::imread(path, 1);
        if (srcB.rows > 0) {
            break;
        } else {
            std::cout << "Image not found or not readable. Try again: " << std::endl;
        }
    }
    std::cout << "Enter the standard deviation for the Gaussian distribution of the high-pass filter: " << std::endl;
    std::string stdDevBStr;
    std::getline(std::cin, stdDevBStr);
    float stdDevB = std::stof(stdDevBStr);
    std::cout << "High-pass filter kernel size is " << (2 * std::ceil(3 * stdDevB) + 1) << std::endl;

    std::cout << "Enter the output image path (include a common image file extension such as .png): " << std::endl;
    std::string outPath;
    std::getline(std::cin, outPath);


    // Create hybrid image
    cv::Mat hybrid;
    hybridCombine3Channel(srcA, srcB, hybrid, stdDevA, stdDevB);

    // Create a window to display results
    const char* windowName = "Hybrid Images";
    cv::namedWindow(windowName, CV_WINDOW_AUTOSIZE);

    // Show the image
    cv::imshow(windowName, hybrid);

    // Save the image
    cv::imwrite(outPath, hybrid);

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

template <class inType, class outType>
void lowPassFilter2d(const cv::Mat& in, cv::Mat& out, float stdDev) {
    crossCorrelation2d<inType, outType>(in, out, gaussianBlurKernel2d(stdDev));
}

template <class inType, class outType>
void highPassFilter2d(const cv::Mat& in, cv::Mat& out, float stdDev) {
    lowPassFilter2d<inType, float>(in, out, stdDev);

    for (int r = 0; r < out.rows; r++) {
        for (int c = 0; c < out.cols; c++) {
            float inValue = (float)in.at<inType>(r, c);
            float lpfValue = out.at<float>(r, c);
            float hpfValue = inValue - lpfValue;

            out.at<outType>(r, c) = boundPixelValue<outType>(hpfValue);
        }
    }
}

void hybridCombine1Channel(const cv::Mat& imgA, const cv::Mat& imgB, cv::Mat& out, float stdDevA, float stdDevB) {
    out = cv::Mat_<uchar>(imgA.rows, imgA.cols);

    cv::Mat aLpf;
    lowPassFilter2d<uchar, float>(imgA, aLpf, stdDevA);

    cv::Mat bLpf;
    highPassFilter2d<uchar, float>(imgB, bLpf, stdDevB);

    for (int r = 0; r < imgA.rows; r++) {
        for (int c = 0; c < imgA.cols; c++) {
            float aValue = imgA.at<uchar>(r, c);
            float aLpfValue = aLpf.at<float>(r, c);

            float bValue = imgB.at<uchar>(r, c);
            float bHpfValue = bLpf.at<float>(r, c);

            out.at<uchar>(r, c) = boundPixelValue<uchar>(aLpfValue + bHpfValue);
        }
    }
}

void hybridCombine3Channel(const cv::Mat& imgA, const cv::Mat& imgB, cv::Mat& out, float stdDevA, float stdDevB) {
    // Split channels
    cv::Mat imgA_bgr[3];
    cv::split(imgA, imgA_bgr);

    cv::Mat imgB_bgr[3];
    cv::split(imgB, imgB_bgr);

    // Do the hybrid combine individually on each channel
    std::vector<cv::Mat> hybrid_bgr(3);
    for (int i = 0; i < 3; i++) {
        hybridCombine1Channel(imgA_bgr[i], imgB_bgr[i], hybrid_bgr[i], stdDevA, stdDevB);
    }

    // Merge the channels
    cv::merge(hybrid_bgr, out);
}

}