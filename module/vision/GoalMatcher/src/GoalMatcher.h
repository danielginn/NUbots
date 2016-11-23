

#ifndef MODULES_VISION_GOALMATCHER_H
#define MODULES_VISION_GOALMATCHER_H

#include <nuclear>
#include "GoalMatcherConstants.h"
#include "Ipoint.h"

namespace module {
namespace vision {

 	class GoalMatcher : public NUClear::Reactor {
    private:
    	uint VARIABLES_GO_HERE;
    	std::unique_ptr<std::vector<Ipoint>> landmarks;



	public:
		/// @brief Called by the powerplant to build and setup the GoalDetector reactor.
		explicit GoalMatcher(std::unique_ptr<NUClear::Environment> environment);
	};

}
}

#endif