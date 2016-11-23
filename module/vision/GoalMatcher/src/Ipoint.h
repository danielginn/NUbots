#pragma once

#include <vector>
#include <math.h>
#include <limits>
#include "GoalMatcherConstants.h"

struct Ipoint {

	  //! Constructor
 	  Ipoint() {  };

  	//! Destructor
  	//virtual ~Ipoint() {};

	  //! Coordinates of the detected interest point
  	float x,y;

  	//! Detected scale
  	float scale;

  	//! Sign of laplacian for fast matching purposes
  	int laplacian;

  	//! Vector of descriptor components
  	float descriptor[SURF_DESCRIPTOR_LENGTH];

};