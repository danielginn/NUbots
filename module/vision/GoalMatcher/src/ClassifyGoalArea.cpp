

#include "ClassifyGoalArea.h"
#include "message/input/Image.h"
#include <stdio.h>
#define SEARCH_POSITIONS 8 // how many of the top responses to look at

ClassifyGoalArea::ClassifyGoalArea(){

}



int ClassifyGoalArea::classifyGoalArea(const message::vision::ClassifiedImage<message::vision::ObjectClass>& frame, 
                                   std::unique_ptr<std::vector<Ipoint>>& landmarks,
                                   std::unique_ptr<Eigen::VectorXf>& landmark_tf,
                                   std::unique_ptr<std::vector<std::vector<float>>>& landmark_pixLoc){

	//if ((!vocabLoaded) || (!frame.wordMapped)) return 0; // Check valid data

	  int num = 0;
	  std::unique_ptr<std::priority_queue<MapEntry>> matches;
  	Eigen::VectorXf query; 
  	std::vector< std::vector<float> > query_pixLoc;
    unsigned int seed = 0; // Not sure what this seed does, but it is supposed to come from the figure.

  	// Augment landmarks with those from a previous matched frame if possible
  	
  	//if(frame.validSurf){
    	  //query = frame.landmark_tf_aug;
    	  //query_pixLoc = frame.landmark_pixLoc_aug;
  	//} else { 
    	  query = *landmark_tf; 
    	  query_pixLoc = *landmark_pixLoc;
    	
  	//}
  	
    tfidf.searchDocument(query, query_pixLoc, matches, seed, SEARCH_POSITIONS);

    return num;
}
