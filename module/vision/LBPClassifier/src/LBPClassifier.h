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

namespace module {
namespace vision {

    class LBPClassifier : public NUClear::Reactor {
        
    private:
        //this enumerates the LBP subtypes so we can turn parts on and off at will
        enum LBPAlgorithmTypes {
            Discriminative = 1,
            Robust = 2,
            Uniform = 4,
            Ternary = 8
        };
        uint64_t LBPAlgorithm = 0;
        
        uint samplingPts = 8;
        std::string typeLBP;
        int noiseLim;
        int numChannels;

        float divisorLBP; //7000.0
        float divisorDRLBP; //25000.0
        
        bool draw;
        int width;
        bool output;
        
        int numImages = 0;
        float tests = 0;
        float correct = 0;
        std::string trainingStage;

        //int histLBP[256][CHANNELS];
        arma::mat histLBP; 

    public:
        /// @brief Called by the powerplant to build and setup the LBPClassifier reactor.
        explicit LBPClassifier(std::unique_ptr<NUClear::Environment> environment);
        void toFile(arma::mat histLBP, int polarity);
        void drawHist(arma::mat histLBP, const uint imgW, const uint imgH, bool found);
	};
}
}

#endif  // MODULES_VISION_LBPCLASSIFIER_H
