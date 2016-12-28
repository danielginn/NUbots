
#pragma once

#include <vector>
#include <queue>
#include <math.h>


#include "GoalMatcherConstants.h"
#include "message/vision/ClassifiedImage.h"
#include "Ipoint.h"
#include "Tfidf.h"

class ClassifyGoalArea {
	public:
		ClassifyGoalArea();



		int classifyGoalArea(const message::vision::ClassifiedImage<message::vision::ObjectClass>& frame, 
							 std::unique_ptr<std::vector<Ipoint>>& landmarks,
							 std::unique_ptr<Eigen::VectorXf>& landmark_tf,
							 std::unique_ptr<std::vector<std::vector<float>>>& landmark_pixLoc);

	private:
		Tfidf tfidf;
};