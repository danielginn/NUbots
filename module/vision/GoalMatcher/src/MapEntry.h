#pragma once

#include <vector>
#include "Ipoint.h"

struct MapEntry {

	MapEntry(){
    	score = 0.f;
  	};

  	// The position of the MapEntry
  	//AbsCoord position;

  	// A RR coordinate if a MapEntry represents a view of another robot
  	//RRCoord object;

  	// Landmarks visible from that position
 	std::vector<Ipoint> ipoints;

 	// Matching score against a query using cosine tfidf or other method
	float score;

	// Pixel ransac line between the two images
  	//RANSACLine ransacLine;

  	template<class Archive>
   	void serialize(Archive &ar, const unsigned int file_version)
   	{
      	//ar & position;
      	ar & ipoints;      
      	ar & score;
      	//ar & object;
      	//ar & ransacLine;
   	}
};