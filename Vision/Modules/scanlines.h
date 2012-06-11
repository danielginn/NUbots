/**
*   @name   ScanLines
*   @file   scanlines.h
*   @brief  generate horizontal and vertical scanlines.
*   @author David Budden
*   @date   23/03/2012
*/

#ifndef SCANLINES_H
#define SCANLINES_H

#include <stdio.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Vision/visionblackboard.h"
//#include "../VisionTools/classificationcolours.h"

using namespace std;

class ScanLines
{
public:
    static const unsigned int HORIZONTAL_SCANLINES = 128;
    static const unsigned int VERTICAL_SCANLINE_SKIP = 2;
    static const int HORIZONTAL_SKIP = 1;
    static const int VERTICAL_SKIP = 1;

    static void generateScanLines();
    
    static void classifyHorizontalScanLines();
    static void classifyVerticalScanLines();
    
private:
    static vector<ColourSegment> classifyHorizontalScan(const VisionBlackboard& vbb, const NUImage& img, unsigned int y);
    static vector<ColourSegment> classifyVerticalScan(const VisionBlackboard& vbb, const NUImage& img, const PointType& start);
    
    
};

#endif // SCANLINES_H
