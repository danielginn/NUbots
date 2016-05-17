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

#include "Odometry.h"

#include "messages/support/Configuration.h"
#include "messages/localisation/OdometryUpdate.h"
#include "messages/input/Sensors.h"
#include "messages/input/ServoID.h"
#include "utility/math/matrix/Transform3D.h"

namespace modules {
namespace localisation {

    using messages::support::Configuration;
    using messages::localisation::OdometryUpdate;
    using message::input::Sensors;
    using message::input::ServoID;
    using utility::math::matrix::Transform3D;

    Odometry::Odometry(std::unique_ptr<NUClear::Environment> environment)
    : Reactor(std::move(environment)) {

        on<Trigger<Configuration<Odometry>>>([this] (const Configuration<Odometry>& config) {
            // Use configuration here from file Odometry.yaml
        });

        on<Trigger<Sensors>>([this] (const Sensors& sensors)){
        	//check foot state and emit odometry update on step transition
        	StepState currentStepState;
        	if(sensors.leftFootDown && !sensors.rightFootDown){
				currentStepState = StepState::LEFT_FOOT_DOWN;
        	} else if(!sensors.leftFootDown && sensors.rightFootDown){
				currentStepState = StepState::RIGHT_FOOT_DOWN;
			} else {
				currentStepState = StepState::STAND_STILL;
			}

        	bool stepTransition = currentStepState != lastStepState;

        	//Compute odometry update

        	if(stepTransition){
        		
        		ServoID frontFoot;
        		ServoID backFoot;
        		
        	//TODO: this logic is incorrect
        		switch(lastStepState){
        			case(LEFT_FOOT_DOWN):
        				frontFoot = ServoID::L_ANKLE_ROLL;
        				backFoot = ServoID::R_ANKLE_ROLL;
        				break;
        			case(RIGHT_FOOT_DOWN):
        				frontFoot = ServoID::R_ANKLE_ROLL;
        				backFoot = ServoID::L_ANKLE_ROLL;
        				break;
        			case(STAND_STILL):
        				return;
        		}

        		Transform3D frontPose = sensors.forwardKinematics[frontFoot];
        		Transform3D backPose = sensors.forwardKinematics[backFoot];

        		//Diff maps points from front foot to back foot coordinate system
        		Transform3D diff = backPose.i() * frontPose;
        		emit(std::make_unique<OdometryUpdate>({diff});
        	}
        }

    }
}
}
