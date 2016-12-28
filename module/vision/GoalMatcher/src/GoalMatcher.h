

#ifndef MODULES_VISION_GOALMATCHER_H
#define MODULES_VISION_GOALMATCHER_H

#include <nuclear>
#include "GoalMatcherConstants.h"
#include "Ipoint.h"
#include <string.h>
#include <eigen3/Eigen/Core>

namespace module {
namespace vision {

 	class GoalMatcher : public NUClear::Reactor {
    private:
    	uint VARIABLES_GO_HERE;
        std::unique_ptr<std::vector<Ipoint>>             landmarks;
        std::unique_ptr<Eigen::VectorXf>                 landmark_tf = std::make_unique<Eigen::VectorXf>(); // term frequency of landmarks
        std::unique_ptr<std::vector<std::vector<float>>> landmark_pixLoc = std::make_unique<std::vector<std::vector<float>>>(); // the pixel locations of terms in landmark_tf
    	bool wasInitial;
        int awayMapSize;
        int homeMapSize;
    	bool clearMap = false;
    	std::string filename = "/home/vagrant/NUbots/module/vision/GoalMatcher/data/words.vocab";

	public:
		/// @brief Called by the powerplant to build and setup the GoalDetector reactor.
		explicit GoalMatcher(std::unique_ptr<NUClear::Environment> environment);
	};

}
}

#endif