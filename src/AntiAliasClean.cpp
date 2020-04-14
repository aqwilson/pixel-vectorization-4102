#include "AntiAliasClean.h"

//INPUT
//  - inputBGR: a vec 3 with a BGR colour 
//OUTPUT 
//  - outLum: the calculated luminance of the input colour 
//  - outSat: the calculated saturaion of the input colour 
//  - outHue: the calculated hue of the input colour
void convertBGRtoHSL(cv::Vec3b& inputBGR, int& outLum, int& outSat, int& outHue)
{
    float B = (float)inputBGR[0] / 255;
    float G = (float)inputBGR[1] / 255;
    float R = (float)inputBGR[2] / 255;

    float maximum = std::max(B, G);
    float Cmax = std::max(maximum, R);

    float minimum = std::min(B, G);
    float Cmin = std::min(minimum, R);

    float delta = Cmax - Cmin;

    //Lum calc
    float lum = ((Cmax + Cmin) / 2);

    //Saturation calculation
    float sat;

    if (delta == 0)
    {
        sat = 0;
    }
    else
    {
        sat = (delta / (1 - abs(2 * lum - 1)));
    }

    //Hue Calc
    float hue;
    if (delta == 0)
    {
        hue = 0;
    }
    else if (Cmax == R)
    {
        hue = 60 * ((G - B) / delta);
    }
    else if (Cmax == G)
    {
        hue = 60 * (((B - R) / delta) + 2);
    }
    else if (Cmax == B)
    {
        hue = 60 * (((R - G) / delta) + 4);
    }

    hue = hue / 360;
    hue = hue * 239;

    lum = lum * 240;
    sat = sat * 240;

    outLum = round(lum);
    outSat = round(sat);
    outHue = round(hue);
}

//INPUT
//  - inputImg: the image for which the unique colorus will be found 
//  - printLine: a print line for tracing/debugging. 
//OUTPUT 
//  - colourList: where the unique colours will be stored 
void findAllColours(cv::Mat& inputImg, std::vector<colourBucket>& colourList, std::string printLine)
{
    std::cout << printLine << std::endl;

    //ITERATING OVER THE IMAGE MATRIX 
    for (int i = 0; i < inputImg.rows; i++)
    {
        for (int j = 0; j < inputImg.cols; j++)
        {
            //Current pixel of interest 
            cv::Vec3b pixel = inputImg.at<cv::Vec3b>(i, j);

            //If it's a magenta pixel, it's not one of interest. 
            if (!(pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 255))
            {
                //If our list is empty, we do want to keep this colour. 
                if (colourList.size() == 0)
                {
                    colourBucket temp;
                    temp.averageCol = pixel;
                    temp.blueSum = pixel[0];
                    temp.greenSum = pixel[1];
                    temp.redSum = pixel[2];
                    temp.colours.push_back(pixel);

                    colourList.push_back(temp);
                }
                else
                {
                    for (int colorListRow = 0; colorListRow < colourList.size(); colorListRow++)
                    {
                        uchar currAvgCol0 = colourList[colorListRow].averageCol[0];
                        uchar currAvgCol1 = colourList[colorListRow].averageCol[1];
                        uchar currAvgCol2 = colourList[colorListRow].averageCol[2];

                        int currAvgLum, currAvgSat, currAvgHue;
                        convertBGRtoHSL(colourList[colorListRow].averageCol, currAvgLum, currAvgSat, currAvgHue);

                        int currPixelLum, currPixelSat, currPixelHue;
                        convertBGRtoHSL(pixel, currPixelLum, currPixelSat, currPixelHue);

                        if (currAvgCol0 == pixel[0] && currAvgCol1 == pixel[1] && currAvgCol2 == pixel[2])
                        {
                            break;
                        }
                        else if (colorListRow == colourList.size() - 1)
                        {
                            colourBucket temp;
                            temp.averageCol = pixel;
                            temp.blueSum = pixel[0];
                            temp.greenSum = pixel[1];
                            temp.redSum = pixel[2];
                            temp.colours.push_back(pixel);
                            colourList.push_back(temp);
                        }
                    }
                }
            }
        }
    }
}

//INPUT
//  - inputImg: the image for which the extreme colours (darkest and lightest) will be found 
//  - printLine: a print line for tracing/debugging. 
//OUTPUT 
//  - colourList: where the darkest and lightest colours will be put. for best use, should be empty to start 
void findExtremeColours(cv::Mat& inputImg, std::vector<colourBucket>& colourList, std::string printLine)
{
    std::cout << printLine << std::endl;

    colourBucket currDarkest;
    currDarkest.averageCol = cv::Vec3b(255, 255, 255);
    currDarkest.blueSum = 255;
    currDarkest.greenSum = 255;
    currDarkest.redSum = 255;
    currDarkest.colours.push_back(cv::Vec3b(255, 255, 255));
    int currDarkestLum = 240;

    colourBucket currLightest;
    currLightest.averageCol = cv::Vec3b(0, 0, 0);
    currLightest.blueSum = 0;
    currLightest.greenSum = 0;
    currLightest.redSum = 0;
    currLightest.colours.push_back(cv::Vec3b(0, 0, 0));
    int currLightestLum = 0;

    //ITERATING OVER THE IMAGE MATRIX 
    for (int i = 0; i < inputImg.rows; i++)
    {
        for (int j = 0; j < inputImg.cols; j++)
        {
            //Current pixel of interest 
            cv::Vec3b pixel = inputImg.at<cv::Vec3b>(i, j);

            //If it's a magenta pixel, it's not one of interest. 
            if (pixel[0] != 255 && pixel[1] != 0 && pixel[2] != 255)
            {
                int pixelLum, pixelSat, pixelHue;
                convertBGRtoHSL(pixel, pixelLum, pixelSat, pixelHue);

                //If the current colour has a luminance lower than the current darkest, update it
                if (pixelLum < currDarkestLum)
                {
                    currDarkest.averageCol = pixel;
                    currDarkest.blueSum = pixel[0];
                    currDarkest.greenSum = pixel[1];
                    currDarkest.redSum = pixel[2];
                    currDarkest.colours.clear();
                    currDarkest.colours.push_back(pixel);
                    currDarkestLum = pixelLum;
                }
                //If the current colour has a luminance higher than current lightest, update it 
                if (pixelLum > currLightestLum)
                {
                    currLightest.averageCol = pixel;
                    currLightest.blueSum = pixel[0];
                    currLightest.greenSum = pixel[1];
                    currLightest.redSum = pixel[2];
                    currLightest.colours.clear();
                    currLightest.colours.push_back(pixel);
                    currLightestLum = pixelLum;
                }
            }
        }
    }

    //Now with lightest and darkest elements, we add them to the colourlist. 
    colourList.push_back(currDarkest);
    //So long as they're not exactly the same colour, let's add lightest too. 
    if (!(currDarkest.blueSum == currLightest.blueSum
        && currDarkest.greenSum == currLightest.greenSum
        && currDarkest.redSum == currLightest.redSum))
    {
        colourList.push_back(currLightest);
    }

    std::cout << "CURRENT STATUS AFTER EXTREMES: " << std::endl;
    for (int k = 0; k < colourList.size(); k++)
    {
        std::cout << "Bucket " << k << std::endl;
        std::cout << "Avg col (RGB): " << (int)colourList[k].averageCol[2]
            << ", " << (int)colourList[k].averageCol[1] << ", "
            << (int)colourList[k].averageCol[0] << std::endl;
        int hue, lum, sat;
        convertBGRtoHSL(colourList[k].averageCol, lum, sat, hue);
        std::cout << "Avg col (HSL): " << hue << ", " << sat << ", " << lum << std::endl;
        for (int colorListRow = 0; colorListRow < colourList[k].colours.size(); colorListRow++)
        {
            std::cout << "\tR: " << (int)(colourList[k].colours[colorListRow][2]) << ", G: "
                << (int)colourList[k].colours[colorListRow][1] << ", B: "
                << (int)colourList[k].colours[colorListRow][0] << std::endl;
        }
    }
    std::cout << "-------------------------------" << std::endl;
}

//INPUT
//  - pixel: a single pixel colour 
//INTPUT/OUTPUT 
//  - colourList: the vec of colour buckets which will be searched, in order to place pixel in an appropriate place 
void placePixelInBucket(cv::Vec3b& pixel, std::vector<colourBucket>& colourList)
{
    bool newBuck = false;
    //OUTPUT DATA 
    //cout << "new colour RGB: " << (int)pixel[2] << ", " << (int)pixel[1] << ", " << (int)pixel[0] << endl;

    for (int colorListRow = 0; colorListRow < colourList.size(); colorListRow++)
    {
        uchar currAvgCol0 = colourList[colorListRow].averageCol[0];
        uchar currAvgCol1 = colourList[colorListRow].averageCol[1];
        uchar currAvgCol2 = colourList[colorListRow].averageCol[2];

        int currAvgLum, currAvgSat, currAvgHue;
        convertBGRtoHSL(colourList[colorListRow].averageCol, currAvgLum, currAvgSat, currAvgHue);

        int currPixelLum, currPixelSat, currPixelHue;
        convertBGRtoHSL(pixel, currPixelLum, currPixelSat, currPixelHue);

        //OUTPUT DATA 
        /** /
        cout << "\tcurrent bucket index: " << colorListRow << endl;
        cout << "\t Curr bucket col RGB: " << (int)currAvgCol2
            << ", " << (int)currAvgCol1 << ", "
            << (int)currAvgCol0 << endl;
        cout << "\tabs(currAvgHue - currPixelHue): " << (int)abs(currAvgHue - currPixelHue) << endl;
        cout << "\tabs(pixel[0] - currAvgCol0) + abs(pixel[1] - currAvgCol1) + abs(pixel[2] - currAvgCol2): "
            << abs(pixel[0] - currAvgCol0) + abs(pixel[1] - currAvgCol1) + abs(pixel[2] - currAvgCol2) << endl;
        // */

        //FINE TUNING STILL REQUIRED!!!!!!!!
        if ((currAvgCol0 + currAvgCol1 + currAvgCol2 <= 75 || currAvgLum <= 45 || (currAvgLum <= 80 && currAvgSat <= 60))
            && (pixel[0] + pixel[1] + pixel[2] <= 75 || currPixelLum <= 45 || (currPixelLum <= 80 && currPixelSat <= 60)))
        {
            //cout << "ADDING TO BLACK" << endl;  //OUTPUT DATA 
            //For now this is the same mixing, but kept it different in case we want to do special case with black 
            //Add current colour to bucket 
            colourList[colorListRow].colours.push_back(pixel);

            //Update sums 
            colourList[colorListRow].blueSum += pixel[0];
            colourList[colorListRow].greenSum += pixel[1];
            colourList[colorListRow].redSum += pixel[2];

            //Update average colour 
            colourList[colorListRow].averageCol[0] = int(colourList[colorListRow].blueSum / colourList[colorListRow].colours.size());
            colourList[colorListRow].averageCol[1] = int(colourList[colorListRow].greenSum / colourList[colorListRow].colours.size());
            colourList[colorListRow].averageCol[2] = int(colourList[colorListRow].redSum / colourList[colorListRow].colours.size());

            break;
        }
        //FINE TUNING STILL REQUIRED!!!!!!!!
        else if ((currAvgCol0 + currAvgCol1 + currAvgCol2 >= 710 || currAvgLum >= 215 || (currAvgLum >= 195 && currAvgSat <= 50))
            && (pixel[0] + pixel[1] + pixel[2] >= 710 || currPixelLum >= 215 || (currAvgLum >= 195 && currAvgSat <= 50)))
        {
            //cout << "ADDING TO WHITE" << endl;//OUTPUT DATA 
            //For now this is the same mixing, but kept it different in case we want to do special case with white  
            //Add current colour to bucket 
            colourList[colorListRow].colours.push_back(pixel);

            //Update sums 
            colourList[colorListRow].blueSum += pixel[0];
            colourList[colorListRow].greenSum += pixel[1];
            colourList[colorListRow].redSum += pixel[2];

            //Update average colour 
            colourList[colorListRow].averageCol[0] = int(colourList[colorListRow].blueSum / colourList[colorListRow].colours.size());
            colourList[colorListRow].averageCol[1] = int(colourList[colorListRow].greenSum / colourList[colorListRow].colours.size());
            colourList[colorListRow].averageCol[2] = int(colourList[colorListRow].redSum / colourList[colorListRow].colours.size());

            break;
        }
        //FINE TUNING STILL REQUIRED!!!!!!!!
        //If their RGB diff is < 100 & hue diff < 20, OR,
        //If (lum > 185 || lum < 70) && both sats < 50
        else if (abs(pixel[0] - currAvgCol0) + abs(pixel[1] - currAvgCol1) + abs(pixel[2] - currAvgCol2) <= 100 && abs(currAvgHue - currPixelHue) < 20
            || (currAvgSat < 50 && currPixelSat < 50 && ((currPixelLum < 70 && currAvgLum < 70) || (currPixelLum > 185 && currAvgLum > 185))))
        {
            //cout << "ADDING TO COLOUR" << endl; //OUTPUT DATA 
            //Add current colour to bucket 
            colourList[colorListRow].colours.push_back(pixel);

            //Update sums 
            colourList[colorListRow].blueSum += pixel[0];
            colourList[colorListRow].greenSum += pixel[1];
            colourList[colorListRow].redSum += pixel[2];

            //Update average colour 
            colourList[colorListRow].averageCol[0] = int(colourList[colorListRow].blueSum / colourList[colorListRow].colours.size());
            colourList[colorListRow].averageCol[1] = int(colourList[colorListRow].greenSum / colourList[colorListRow].colours.size());
            colourList[colorListRow].averageCol[2] = int(colourList[colorListRow].redSum / colourList[colorListRow].colours.size());
            break;
        }
        //We're in the last row- need to make a new colour bucket. 
        else if (colorListRow == colourList.size() - 1)
        {
            newBuck = true;
            //cout << "ADDING TO NEW BUCKET" << endl; //OUTPUT DATA 
            colourBucket temp;
            temp.averageCol = pixel;
            temp.blueSum = pixel[0];
            temp.greenSum = pixel[1];
            temp.redSum = pixel[2];
            temp.colours.push_back(pixel);
            colourList.push_back(temp);
            break;
        }
    }

    //OUTPUT DATA 
    /*
    if (newBuck)
    {
        cout << "NEW COLOUR: " << (int)pixel[2] << ", " << (int)pixel[1] << ", " << (int)pixel[0] << endl;
        cout << "CURRENT STATUS: " << endl;
        for (int k = 0; k < colourList.size(); k++)
        {
            cout << "Bucket " << k << endl;
            cout << "Avg col (RGB): " << (int)colourList[k].averageCol[2]
                << ", " << (int)colourList[k].averageCol[1] << ", "
                << (int)colourList[k].averageCol[0] << endl;
            int hue, lum, sat;
            convertBGRtoHSL(colourList[k].averageCol, lum, sat, hue);
            cout << "Avg col (HSL): " << hue << ", " << sat << ", " << lum << endl;
            for (int colorListRow = 0; colorListRow < colourList[k].colours.size(); colorListRow++)
            {
                cout << "\tR: " << (int)(colourList[k].colours[colorListRow][2]) << ", G: "
                    << (int)colourList[k].colours[colorListRow][1] << ", B: "
                    << (int)colourList[k].colours[colorListRow][0] << endl;
            }
        }
        cout << "-------------------------------" << endl;
        newBuck = false;
    }
    //*/
}

//INPUT
//  - inputImg: the image for which the main colours will be found 
//  - printLine: a print line for tracing/debugging. 
//INPUT/OUTPUT 
//  - colourList: the resulting average colours 
void findCoreColours(cv::Mat& inputImg, std::vector<colourBucket>& colourList, std::string printLine)
{
    std::cout << printLine << std::endl;

    //ITERATING OVER THE IMAGE MATRIX 
    for (int i = 0; i < inputImg.rows; i++)
    {
        for (int j = 0; j < inputImg.cols; j++)
        {
            //Current pixel of interest 
            cv::Vec3b pixel = inputImg.at<cv::Vec3b>(i, j);

            //If it's a magenta pixel, it's not one of interest. 
            if (pixel[0] != 255 && pixel[1] != 0 && pixel[2] != 255)
            {
                //If our list is empty, we do want to keep this colour. 
                if (colourList.size() == 0)
                {
                    colourBucket temp;
                    temp.averageCol = pixel;
                    temp.blueSum = pixel[0];
                    temp.greenSum = pixel[1];
                    temp.redSum = pixel[2];
                    temp.colours.push_back(pixel);

                    colourList.push_back(temp);
                }
                //Need to see which bucket this colour should go in 
                else
                {
                    //cout << "CALL i: " << i << ", j: " << j << endl;  //OUTPUT DATA 
                    placePixelInBucket(pixel, colourList);
                }
            }
        }
    }
}

//INPUT
//  - origColList: the uncompressed list of averaged colours 
//  - printLine: a print line for tracing/debugging. 
//OUTPUT 
//  - compColList: a resulting list that can from compressing the averaged colours 
void compressCoreColours(std::vector<colourBucket>& origColList, std::vector<colourBucket>& compColList, std::string printLine)
{
    for (int i = 0; i < origColList.size(); i++)
        compColList.push_back(origColList[i]);

    std::cout << printLine << std::endl;
    //Go over all buckets to see if there's any combinations needed
    for (int colListCheck = 0; colListCheck < compColList.size() - 1; colListCheck++)
    {
        uchar currAvgCol0 = compColList[colListCheck].averageCol[0];
        uchar currAvgCol1 = compColList[colListCheck].averageCol[1];
        uchar currAvgCol2 = compColList[colListCheck].averageCol[2];

        int currAvgLum, currAvgSat, currAvgHue;
        convertBGRtoHSL(compColList[colListCheck].averageCol, currAvgLum, currAvgSat, currAvgHue);

        for (int nextBucket = colListCheck + 1; nextBucket < compColList.size(); nextBucket++)
        {
            uchar nextBucketAvgCol0 = compColList[nextBucket].averageCol[0];
            uchar nextBucketAvgCol1 = compColList[nextBucket].averageCol[1];
            uchar nextBucketAvgCol2 = compColList[nextBucket].averageCol[2];

            int nextBuckAvgLum, nextBuckAvgSat, nextBuckAvgHue;
            convertBGRtoHSL(compColList[nextBucket].averageCol, nextBuckAvgLum, nextBuckAvgSat, nextBuckAvgHue);

            //CSC
            if ((nextBucketAvgCol0 + nextBucketAvgCol1 + nextBucketAvgCol2 <= 75 || nextBuckAvgLum <= 45 || (nextBuckAvgLum <= 80 && nextBuckAvgSat <= 45))//(nextBuckAvgLum <= 120 && nextBuckAvgSat <= 60)) //
                && (currAvgCol0 + currAvgCol1 + currAvgCol2 <= 75 || currAvgLum <= 45 || (currAvgLum <= 80 && currAvgSat <= 45)))// (currAvgLum <= 120 && currAvgSat <= 60))) //
            {
                //For now this is the same mixing, but kept it different in case we want to do special case with black (also for sanity's & clarity's sake) 
                //Add all the colours from colListChick's bucket into nextBucket's
                for (int i = 0; i < compColList[colListCheck].colours.size(); i++)
                {
                    compColList[nextBucket].colours.push_back(compColList[colListCheck].colours[i]);

                    //Update sums with individual colour values 
                    compColList[nextBucket].blueSum += compColList[colListCheck].colours[i][0];
                    compColList[nextBucket].greenSum += compColList[colListCheck].colours[i][1];
                    compColList[nextBucket].redSum += compColList[colListCheck].colours[i][2];

                    compColList[nextBucket].averageCol[0] = int(compColList[nextBucket].blueSum / compColList[nextBucket].colours.size());
                    compColList[nextBucket].averageCol[1] = int(compColList[nextBucket].greenSum / compColList[nextBucket].colours.size());
                    compColList[nextBucket].averageCol[2] = int(compColList[nextBucket].redSum / compColList[nextBucket].colours.size());
                }

                compColList.erase(compColList.begin() + colListCheck);
                colListCheck--;
                break;
            }
            //CSC
            else if ((nextBucketAvgCol0 + nextBucketAvgCol1 + nextBucketAvgCol2 >= 710 || (nextBuckAvgLum >= 195 && nextBuckAvgSat <= 50))// nextBuckAvgLum >= 215 || (nextBuckAvgLum > 120 && nextBuckAvgSat <= 60))//
                && (currAvgCol0 + currAvgCol1 + currAvgCol2 >= 710 || currAvgLum >= 215 || (currAvgLum >= 195 && currAvgSat <= 50)))//(currAvgLum > 120 && currAvgSat <= 60 )))//
            {
                //For now this is the same mixing, but kept it different in case we want to do special case with white (also for sanity's & clarity's sake) 
                //Add all the colours from colListChick's bucket into nextBucket's
                for (int i = 0; i < compColList[colListCheck].colours.size(); i++)
                {
                    compColList[nextBucket].colours.push_back(compColList[colListCheck].colours[i]);

                    //Update sums with individual colour values 
                    compColList[nextBucket].blueSum += compColList[colListCheck].colours[i][0];
                    compColList[nextBucket].greenSum += compColList[colListCheck].colours[i][1];
                    compColList[nextBucket].redSum += compColList[colListCheck].colours[i][2];

                    compColList[nextBucket].averageCol[0] = int(compColList[nextBucket].blueSum / compColList[nextBucket].colours.size());
                    compColList[nextBucket].averageCol[1] = int(compColList[nextBucket].greenSum / compColList[nextBucket].colours.size());
                    compColList[nextBucket].averageCol[2] = int(compColList[nextBucket].redSum / compColList[nextBucket].colours.size());
                }
                compColList.erase(compColList.begin() + colListCheck);
                colListCheck--;
                break;
            }
            //100                                     //20
            else if (abs(currAvgCol0 - nextBucketAvgCol0) + abs(currAvgCol1 - nextBucketAvgCol1) + abs(currAvgCol2 - nextBucketAvgCol2) <= 120 && abs(currAvgHue - nextBuckAvgHue) < 30
                || (currAvgSat < 50 && nextBuckAvgSat < 50 && ((nextBuckAvgLum < 70 && currAvgLum < 70) || (nextBuckAvgLum > 150 && currAvgLum > 150))) //Lum is relatively high or low, and sats are low 
                || (abs(currAvgHue - nextBuckAvgHue) < 60 && currAvgSat < 110 && nextBuckAvgSat < 110 && (nextBuckAvgLum > 190 && currAvgLum > 190)) //Lum really high, 
                || (abs(currAvgCol0 - nextBucketAvgCol0) + abs(currAvgCol1 - nextBucketAvgCol1) + abs(currAvgCol2 - nextBucketAvgCol2) < 85 && (nextBuckAvgLum > 170 && currAvgLum > 170)))
            {
                //Add all the colours from colListChick's bucket into nextBucket's
                for (int i = 0; i < compColList[colListCheck].colours.size(); i++)
                {
                    compColList[nextBucket].colours.push_back(compColList[colListCheck].colours[i]);

                    //Update sums with individual colour values 
                    compColList[nextBucket].blueSum += compColList[colListCheck].colours[i][0];
                    compColList[nextBucket].greenSum += compColList[colListCheck].colours[i][1];
                    compColList[nextBucket].redSum += compColList[colListCheck].colours[i][2];

                    compColList[nextBucket].averageCol[0] = int(compColList[nextBucket].blueSum / compColList[nextBucket].colours.size());
                    compColList[nextBucket].averageCol[1] = int(compColList[nextBucket].greenSum / compColList[nextBucket].colours.size());
                    compColList[nextBucket].averageCol[2] = int(compColList[nextBucket].redSum / compColList[nextBucket].colours.size());
                }

                compColList.erase(compColList.begin() + colListCheck);
                colListCheck--;
                break;
            }
        }
    }
}

//INPUT
//  - colourList: a list of averaged colours that have been found 
//  - currBucketString: a print line for tracing/debugging. 
//  - colourStripeSize: the size each colour stripe will be. if set to 1, the canvas will be empty. 
//INPUT/OUTPUT 
//  - canvas: an image that is very tall and is magenta. The colours in colourList will be painted onto it 
void printBuckets(cv::Mat& canvas, std::vector<colourBucket>& colourList, cv::String currBucketString, int colourStripeSize)
{
    std::cout << currBucketString << std::endl;

    //Current row that the current colour at i will be painted at on the canvas 
    int currCanvasRow = 0;

    //i: the current colour bucket we're looking at 
    for (int i = 0; i < colourList.size(); i++)
    {
        std::cout << "R: " << (int)(colourList[i].averageCol[2]) << ", G: " << (int)colourList[i].averageCol[1] << ", B: " << (int)colourList[i].averageCol[0] << std::endl;

        int lum, sat, hue;
        convertBGRtoHSL(colourList[i].averageCol, lum, sat, hue);

        std::cout << "\tHue: " << hue << ", Lum: " << lum << ", Sat: " << sat << std::endl;

        //j: drawing that many rows of the current colour at i 
        for (int j = 1; j < colourStripeSize; j++)
        {
            //painting horizontally on the canvas with colour i for j rows 
            for (int canvasLength = 0; canvasLength < canvas.cols; canvasLength++)
            {
                cv::Vec3b& pixel = canvas.at<cv::Vec3b>(i + j + currCanvasRow, canvasLength);

                pixel[0] = colourList[i].averageCol[0];
                pixel[1] = colourList[i].averageCol[1];
                pixel[2] = colourList[i].averageCol[2];
            }
        }
        currCanvasRow += colourStripeSize;
    }
    std::cout << std::endl;
}

//INPUT
//  - antiAlias: the anti-aliased image that we are trying to clean 
//  - colours: the colours that we found to be the anti-aliased img's palette 
//OUTPUT 
//  - paintedImg: the resulting image that has had the average colours painted on
void paintImgWithBuckets(cv::Mat& antiAlias, cv::Mat& paintedImg, std::vector<colourBucket>& colours)
{
    //ITERATING OVER THE IMAGE  MATRIX 
    for (int i = 0; i < antiAlias.rows; i++)
    {
        for (int j = 0; j < antiAlias.cols; j++)
        {
            //Current pixel of interest 
            cv::Vec3b pixel = antiAlias.at<cv::Vec3b>(i, j);

            //If it's a magenta pixel, it's not one of interest. 
            if (!(pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 255))
            {
                //Current pixel is not magenta
                //Find the actual pixel colour in colourBucket
                bool foundBucket = false;

                //Search through each bucket 
                for (int bucket = 0; bucket < colours.size(); bucket++)
                {
                    //Look through all the colours each bucket has 
                    for (int colourInBucket = 0; colourInBucket < colours[bucket].colours.size(); colourInBucket++)
                    {
                        //If we find a colour that matches the current pixel, this is the bucket of interest 
                        if (pixel[0] == colours[bucket].colours[colourInBucket][0]
                            && pixel[1] == colours[bucket].colours[colourInBucket][1]
                            && pixel[2] == colours[bucket].colours[colourInBucket][2])
                        {
                            foundBucket = true;
                            break;
                        }
                    }

                    //If we have located the bucket, no need to search anymore 
                    if (foundBucket)
                    {
                        //Apply current bucket's average colour to the canvas and break 
                        cv::Vec3b& newPixel = paintedImg.at<cv::Vec3b>(i, j);

                        newPixel[0] = colours[bucket].averageCol[0];
                        newPixel[1] = colours[bucket].averageCol[1];
                        newPixel[2] = colours[bucket].averageCol[2];

                        break;
                    }
                    /* !! Can use for debugging. If we reach the last bucket, and we haven't found our colour, that's BAD.
                          Fill this pixel in as Magenta to indicate that it doesn't exist in colour bucket !!
                    else if (bucket == colours.size() - 1)
                    {
                        Vec3b& newPixel = paintedImg.at<Vec3b>(i, j);
                        newPixel[0] = 255;
                        newPixel[1] = 0;
                        newPixel[2] = 255;
                    }
                    */
                }
            }
        }
    }
}

//The process for actually cleaning up an antialiased img 
void antiAliasCleanup()
{
    //Loading in the original (for comparison) and the anti alias imgs 
    cv::String origPic = "Ocarina_Original";
    cv::String antiPic = "Ocarina_Anti";

    cv::Mat imgOriginal = imread("./input/" + origPic + ".png");
    cv::Mat img_toTestImage = imread("./input/" + antiPic + ".png");

    //Canvas used for seeing what colours we're finding
    cv::Mat drawOnCanvas = cv::imread("./input/Canvas.png");
    cv::Mat origAllColoursCanvas = drawOnCanvas.clone(); //seeing all colours in original img 
    cv::Mat antiAliasAllColoursCanvas = drawOnCanvas.clone(); //seeing all colours in anti alias img (there's basically TOO many) 
    cv::Mat sharpAllColoursCanvas = drawOnCanvas.clone(); //seeing all colours in anti+sharpened img 
    cv::Mat sharpCoreColoursCanvas = drawOnCanvas.clone(); //seeing the core colours found in anti+sharpened img
    cv::Mat sharpCompressedCoreColoursCanvas = drawOnCanvas.clone(); //seeing the core colours after compression on anti+sharpened img

    //Imgs for filtering purposes 
    cv::Mat img_blurred = img_toTestImage.clone(); //blurred img for sharpening 
    cv::Mat img_Gauss = img_toTestImage.clone(); //gaussian img for sharpening 
    cv::Mat img_Sharp = img_toTestImage.clone(); //resulting sharpened img 

    //Finding the sharpened version of the anti-alias img 
    blur(img_toTestImage, img_blurred, cv::Size(3, 3), cv::Point(-1, -1));
    GaussianBlur(img_toTestImage, img_Gauss, cv::Size(3, 3), 3);
    addWeighted(img_toTestImage, 1.5, img_Gauss, -0.5, 0, img_Sharp);

    //Our resulting cleaned image will be based off of sharpened 
    cv::Mat cleanedImage = img_Sharp.clone();

    //Finding bucket data 
    std::vector<colourBucket> origImgcolourBuckets;
    findAllColours(imgOriginal, origImgcolourBuckets, "Original Img colours:");
    std::vector<colourBucket> sharpImgColourBuckets;
    findAllColours(img_Sharp, sharpImgColourBuckets, "Sharp colours");
    std::vector<colourBucket> sharpImgCoreColourBuckets;
    findExtremeColours(img_Sharp, sharpImgCoreColourBuckets, "Sharp Extreme Colours being found...");
    std::vector<colourBucket> sharpImgCompressedCoreColourBuckets;
    compressCoreColours(sharpImgCoreColourBuckets, sharpImgCompressedCoreColourBuckets, "Sharp CORE colours");

    //Drawing colour data onto canvases 
    printBuckets(origAllColoursCanvas, origImgcolourBuckets, "Original colours:", 10);
    //printBuckets(antiAliasAllColoursCanvas, antiImgColourBuckets, "Anti-Aliased colors:", 3); //too many colours here tbh, no point 
    printBuckets(sharpAllColoursCanvas, sharpImgColourBuckets, "Sharp colours", 1);
    printBuckets(sharpCoreColoursCanvas, sharpImgCoreColourBuckets, "Sharp CORE colours", 10);
    printBuckets(sharpCompressedCoreColoursCanvas, sharpImgCompressedCoreColourBuckets, "Sharp COMPRESSED core colours", 10);

    //Paint our resulting img 
    paintImgWithBuckets(img_Sharp, cleanedImage, sharpImgCompressedCoreColourBuckets);

    //Make windows for Results 
    cv::String windowName = "AntiAliased"; //Name of the window
    namedWindow(windowName); // Create a window
    cv::String SharpWindow = "Sharpened";
    namedWindow(SharpWindow);
    cv::String origCanvasWindow = "Original Img: All Cols";
    namedWindow(origCanvasWindow);
    cv::String sharpCanvasWindow = "Sharp Img: All Cols";
    namedWindow(sharpCanvasWindow);
    cv::String sharpCoreColsCanvasWindow = "Sharp Img: Core Cols";
    namedWindow(sharpCoreColsCanvasWindow);
    cv::String sharpCompCoreColsCanvasWindow = "Sharp Img: COMPRESSED Core Cols";
    namedWindow(sharpCompCoreColsCanvasWindow);
    cv::String paintedImgCanvasWindow = "Painted Over Image";
    namedWindow(paintedImgCanvasWindow);

    //Displaying imgs 
    imshow(windowName, img_toTestImage); //Anti-aliased image 
    imshow(SharpWindow, img_Sharp); //Sharpened Image 
    imshow(sharpCanvasWindow, sharpAllColoursCanvas); //All unique colours in sharpened image 
    imshow(sharpCoreColsCanvasWindow, sharpCoreColoursCanvas); //All core colours in sharpened image 
    imshow(sharpCompCoreColsCanvasWindow, sharpCompressedCoreColoursCanvas); //All COMPRESSED core colours in sharpened image 
    imshow(paintedImgCanvasWindow, cleanedImage);
}