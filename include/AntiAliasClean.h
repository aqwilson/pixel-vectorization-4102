#pragma once
#include <opencv2/core.hpp>

struct colourBucket
{
    cv::Vec3b averageCol;
    int redSum, greenSum, blueSum;
    std::vector<cv::Vec3b> colours; //the current colours in this bucket
};

//Given an RGB colour as Vec3, returns ints for lum, sat and hue. 
void convertBGRtoHSL(cv::Vec3b& inputBGR, int& outLum, int& outSat, int& outHue);

//Given an img, will return a vec of colour buckets that are all the unique colours in the img. Printline is used for debugging & clarity 
void findAllColours(cv::Mat& inputImg, std::vector<colourBucket>& colourList, std::string printLine);

//Given an img, will find the lightest and darkest colours in the image, and store them in colour buckets
void findExtremeColours(cv::Mat& inputImg, std::vector<colourBucket>& colourList, std::string printLine);

//Return the sum of a colour's RGB values
int sumRGB(cv::Vec3b& colour);

//Return the total difference between two colour's RGBs 
int sumRGBDiffs(cv::Vec3b& colourA, cv::Vec3b& colourB);

//Compares two colours to see if they're close by specific hue classifications
bool closeByHue(cv::Vec3b& colourA, cv::Vec3b& colourB);

//Compare two colours to see if they're close by specific sat classifications
bool closeBySat(cv::Vec3b& colourA, cv::Vec3b& colourB);

//Compare two colours to see if they're close by specific lum classifications
bool closeByLum(cv::Vec3b& colourA, cv::Vec3b& colourB);

//Given an RGB colour, will find which colour bucket it should go in 
void placePixelInBucket(cv::Vec3b& pixel, std::vector<colourBucket>& colourList);

//Given an img, will find the main averaged colours in the img by combining similar colours 
void findCoreColours(cv::Mat& inputImg, std::vector<colourBucket>& colourList, std::string printLine);

//Given a vec of colour buckets, will compress it further if possible 
void compressCoreColours(std::vector<colourBucket>& origColList, std::vector<colourBucket>& compColList, std::string printLine);

//Given an image to draw on (canvas), will draw out the colours in a vec of colour buckets. 
//int will be the size of each colour. If set to 1, the colours will not be drawn (used if there are too many colours to fit on the canvas) 
void printBuckets(cv::Mat& canvas, std::vector<colourBucket>& colourList, cv::String currBucketString, int colourStripeSize);

//Given an img and a vec of colour buckets, will find in which colour bucket each individual colour is 
//and paint over with the new corresponding colour. 
void paintImgWithBuckets(cv::Mat& antiAlias, cv::Mat& paintedImg, std::vector<colourBucket>& colours);

//Function for testing and outputting cleanup 
void testingAntiAliasCleanup();

//Run the cleanup
void antiAliasCleanup(cv::Mat& antiAliasImage, cv::Mat& cleanedImage);