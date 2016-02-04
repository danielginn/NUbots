/*
 * This file is part of NUbots Codebase.
 *
 * The NUbots Codebase is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The NUbots Codebase is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the NUbots Codebase.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright 2015 NUbots <nubots@nubots.net>
 */

#ifndef MODULES_VISION_LBPCLASSIFIER_H
#define MODULES_VISION_LBPCLASSIFIER_H

#include <nuclear>
#include "utility/nubugger/NUhelpers.h"
#define CHANNELS 1

namespace module {
namespace vision {

    class LBPClassifier : public NUClear::Reactor {

    public:
        /// @brief Called by the powerplant to build and setup the LBPClassifier reactor.
        explicit LBPClassifier(std::unique_ptr<NUClear::Environment> environment);
        void toFile(int histLBP[][CHANNELS], int polarity);
        void drawHist(int histLBP[][CHANNELS], const uint imgW, const uint imgH);
	private:
		int histLBP[256][CHANNELS];
		uint samplingPts = 8;
		std::string typeLBP = "DRLBP";
		int noiseLim = 2;
		
		float divisorLBP = 7000.0;
		float divisorRLBP = 15000.0;
		float divisorDRLBP = 25000.0;
		
		bool draw = true;
		bool output = true;
		std::string trainingStage = "TESTING";
	};
}
}

#endif  // MODULES_VISION_LBPCLASSIFIER_H
