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

#ifndef MODULES_MOTION_HEAD6DOFCONTROLLER_H
#define MODULES_MOTION_HEAD6DOFCONTROLLER_H

#include <nuclear>
#include "utility/math/matrix/Transform3D.h"

namespace module {
namespace motion {

    class Head6DoFController : public NUClear::Reactor {
    	float foot_separation = 0.10;
    	float body_angle = 0.0;

        arma::vec3 l_arm,r_arm;

        utility::math::matrix::Transform3D camera_to_robot;

    	utility::math::matrix::Transform3D robot_to_head;
        float robot_to_head_scale;

        struct {
            struct Range {
                float max = 0;
                float min = 0;
            };
            Range roll;
            Range pitch;
            Range yaw;
        } eulerLimits;
        
        float distance_limit = 0.1;
        
        //State:
        utility::math::matrix::Transform3D robotCamPose;

        //Subsumption:
    	size_t id;

    	void updatePriority(const float& priority);

        void limitPose(utility::math::matrix::Transform3D& pose);

    public:
        /// @brief Called by the powerplant to build and setup the Head6DoFController reactor.
        explicit Head6DoFController(std::unique_ptr<NUClear::Environment> environment);
    };

}
}

#endif  // MODULES_MOTION_HEAD6DOFCONTROLLER_H