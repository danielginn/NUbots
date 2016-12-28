#pragma once

#include <vector>
#include <string>
#include <stdio.h>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <eigen3/Eigen/Core>

#include "GoalMatcherConstants.h"
#include "Ipoint.h"

class Vocab {

	public:
		//! Constructor
	  	Vocab() {
	    	vec_length = 0;
	  	};

	  	int getSize(){
			return vec_length;
		}

		//! Loads a set of visual words for use
  		void loadVocabFile(std::string filename);

  		//! Map a set of interest points to a sparse term frequency vector while preserving 
  		// the pixel locations of the interest points
  		Eigen::VectorXf mapToVec(std::unique_ptr<std::vector<Ipoint>> &ipts, std::unique_ptr<std::vector<std::vector<float>>> &pixel_location);



	private:
  
		int vec_length;

  		std::vector<Ipoint> pos_words;
  		std::vector<Ipoint> neg_words;
	
		std::vector<Ipoint> pos_stop_words;
		std::vector<Ipoint> neg_stop_words;
};