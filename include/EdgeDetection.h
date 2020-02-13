#pragma once

#include "opencv2/imgproc/imgproc.hpp"

namespace edgeDetection {

const float PI = 3.14159265358979323846;

// Runs the user interface for the edge detection application.
int edgeDetectionMain();

// Bounds the given float value to the narrower value between the template type's min and max and the min and max of
// a 32-bit float.
template <class T>
T boundPixelValue(float floatValue);

// Applies cross-correlation with the given kernel to 'in' and stores the result in 'out'. in must be a 2D single-channel image.
template <class inType, class outType>
void crossCorrelation2d(const cv::Mat& in, cv::Mat& out, const cv::Mat& kernel);

// Applies convolution with the given kernel to 'in' and stores the result in 'out'. in must be a 2D single-channel image.
template <class inType, class outType>
void convolve2d(const cv::Mat& in, cv::Mat& out, const cv::Mat& kernel);

// Returns a (dim)x(dim) horizonal 2D Sobel kernel for cross-convolution / convolution.
cv::Mat sobelKernelX();

// Returns a (dim)x(dim) vertical 2D Sobel kernel for cross-convolution / convolution.
cv::Mat sobelKernelY();

// Returns a (dim)x(dim) 2D kernel for cross-convolution / convolution where cells follow a Gaussian distribution.
// The dimension of the kernel will determined by the formula dim = 2*ceil(3*stdDev)+1.
cv::Mat gaussianBlurKernel2d(float stdDev);

// Sample from a 2D Gaussian distribution with the given variance.
float gaussianSample2d(float x, float y, float var);

// Outputs a 2D single-channel float matrix of graident intensities, and another for gradient angles, given the 2D
// single-channel float matrices of gradient x and y compenents.
void calcuateGradients(const cv::Mat& gradX, const cv::Mat& gradY, cv::Mat& outIntensities, cv::Mat& outAngles);

// Outputs a 2D single-channel image of detected edges, given the 2D single-channel float matrices of
// gradient instensities and angles. Edges are detected using gradient intensity thresholding and non-maximum suppression.
void thresholdAndNonMaximumSuppression(const cv::Mat& gradIntensities, const cv::Mat& gradAngles, cv::Mat& out, float threshold);

// Outputs a 2D single-channel image of detected edges, given the 2D single-channel source image, a standard deviation
// for Gaussian smoothing, and a threshold for gradient intensities.
void edgeDetection(const cv::Mat& src, cv::Mat& dst, float stdDev, float threshold);

}