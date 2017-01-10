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
 * Copyright 2013 NUBots <nubots@nubots.net>
 */

#ifndef MODULES_VISION_GOALDETECTOR_H
#define MODULES_VISION_GOALDETECTOR_H

#include <nuclear>
#include "Ipoint.h"
#include <string.h>
#include <eigen3/Eigen/Core>

namespace module {
namespace vision {

    class GoalDetector : public NUClear::Reactor {
    private:
        uint MINIMUM_POINTS_FOR_CONSENSUS;
        uint MAXIMUM_ITERATIONS_PER_FITTING;
        uint MAXIMUM_FITTED_MODELS;
        double CONSENSUS_ERROR_THRESHOLD;

        double MAXIMUM_ASPECT_RATIO;
        double MINIMUM_ASPECT_RATIO;
        double VISUAL_HORIZON_BUFFER;
        double MAXIMUM_GOAL_HORIZON_NORMAL_ANGLE;
        double MAXIMUM_ANGLE_BETWEEN_GOALS;
        double MAXIMUM_VERTICAL_GOAL_PERSPECTIVE_ANGLE;

        uint MEASUREMENT_LIMITS_LEFT;
        uint MEASUREMENT_LIMITS_RIGHT;
        uint MEASUREMENT_LIMITS_TOP;
        uint MEASUREMENT_LIMITS_BASE;

        std::unique_ptr<std::vector<Ipoint>>             landmarks;
        std::unique_ptr<Eigen::VectorXf>                 landmark_tf = std::make_unique<Eigen::VectorXf>(); // term frequency of landmarks
        std::unique_ptr<std::vector<std::vector<float>>> landmark_pixLoc = std::make_unique<std::vector<std::vector<float>>>(); // the pixel locations of terms in landmark_tf
        bool wasInitial;
        int awayMapSize;
        int homeMapSize;
        bool clearMap = false;
        std::string VocabFileName = "/home/vagrant/NUbots/module/vision/GoalDetector/data/words.vocab";
        std::string MapFileName = "/home/vagrant/NUbots/module/vision/GoalDetector/data/goals.map";

    public:
        /// @brief Called by the powerplant to build and setup the GoalDetector reactor.
        explicit GoalDetector(std::unique_ptr<NUClear::Environment> environment);
    };

}
}


#endif
