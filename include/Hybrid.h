#pragma once

#include "opencv2/imgproc/imgproc.hpp"

namespace hybrid {

const float PI = 3.14159265358979323846;

// Runs the user interface for the hybrid images application.
int hybridMain();

// Bounds the given float value to the narrower value between the template type's min and max and the min and max of
// a 32-bit float.
template <class T>
T boundPixelValue(float floatValue);

// Applies cross-correlation with the given kernel to 'in' and stores the result in 'out'. in and out must both
// be 2D images with the same dimensions.
template <class inType, class outType>
void crossCorrelation2d(const cv::Mat& in, cv::Mat& out, const cv::Mat& kernel);

// Applies convolution with the given kernel to 'in' and stores the result in 'out'. in and out must both
// be 2D images with the same dimensions.
template <class inType, class outType>
void convolve2d(const cv::Mat& in, cv::Mat& out, const cv::Mat& kernel);

// Returns a (dim)x(dim) 2D kernel for cross-convolution / convolution where cells follow a Gaussian distribution.
// The dimension of the kernel will determined by the formula dim = 2*ceil(3*stdDev)+1.
cv::Mat gaussianBlurKernel2d(float stdDev);

// Sample from a 2D Gaussian distribution with the given variance.
float gaussianSample2d(float x, float y, float var);

// Apply cross-correlation with a Gaussian blur kernel. The Gaussian distribution will have the given standard
// deviation. The dimension of the kernel will determined by the formula dim = 2*ceil(3*stdDev)+1.
template <class inType, class outType>
void lowPassFilter2d(const cv::Mat& in, cv::Mat& out, float stdDev);

// Subtract a low-pass filter (Gaussian blur) from the source image, then add a base grey value to avoid flattening
// the negative values. The Gaussian distribution will have the given standard deviation.
// The dimension of the kernel will determined by the formula dim = 2*ceil(3*stdDev)+1.
template <class inType, class outType>
void highPassFilter2d(const cv::Mat& in, cv::Mat& out, float stdDev);

// Applies a low-pass filter to imgA and a high-pass filter to imgB, then sums the results.
// The low-pass and high-pass filters each use a Gaussian blur with standard deviations of stdDevA and stdDevB,
// respectively. The dimension of each kernel will be determined by the formula dim = 2*ceil(3*stdDev)+1.
// imgA and imgB and out should all be single-channel 2D images with the same dimensions.
void hybridCombine1Channel(const cv::Mat& imgA, const cv::Mat& imgB, cv::Mat& out, float stdDevA, float stdDevB);

// Applies a low-pass filter to imgA and a high-pass filter to imgB, then sums the results.
// The low-pass and high-pass filters each use a Gaussian blur with standard deviations of stdDevA and stdDevB,
// respectively. The dimension of each kernel will be determined by the formula dim = 2*ceil(3*stdDev)+1.
// imgA and imgB and out should all be three-channel 2D images with the same dimensions.
void hybridCombine3Channel(const cv::Mat& imgA, const cv::Mat& imgB, cv::Mat& out, float stdDevA, float stdDevB);

}