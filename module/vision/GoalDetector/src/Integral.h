
#pragma once

#include <algorithm>  // req'd for std::min/max
#include "message/vision/ClassifiedImage.h"
#include "GoalMatcherConstants.h"
#include <nuclear>

//! Computes the 1d integral image of the specified horizon line 
//! in image img.  Assumes source image to be a 32-bit floating point.  Returns IplImage in 32-bit float form.
void Integral(std::unique_ptr<std::vector<float>>& data, std::shared_ptr<const message::vision::ClassifiedImage<message::vision::ObjectClass>> frame_3, int left_horizon, int right_horizon);

// Convert horizon of image to single channel 32F
void getGrayHorizon(std::unique_ptr<std::vector<float>>& result, std::shared_ptr<const message::vision::ClassifiedImage<message::vision::ObjectClass>> frame_4, int left_horizon, int right_horizon);


//! Computes the sum of pixels within the row specified by the left start
//! co-ordinate and width
inline float BoxIntegral(std::unique_ptr<std::vector<float>>& data, int col, int cols) 
{

  // The subtraction by one for col because col is inclusive.
  int c1 = std::min(col,          IMAGE_WIDTH/SURF_SUBSAMPLE)  - 1;
  int c2 = std::min(col + cols,   IMAGE_WIDTH/SURF_SUBSAMPLE)  - 1;

  float A(0.0f), B(0.0f);
  if (c2 >= 0) A = data->at(c2);
  if (c1 >= 0) B = data->at(c1);

  return std::max(0.f, A - B);
}

