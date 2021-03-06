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

#ifndef MODULES_PLATFORM_DARWIN_KINEMATICSMODEL_H
#define MODULES_PLATFORM_DARWIN_KINEMATICSMODEL_H

#include <nuclear>
#include <armadillo>
#include <yaml-cpp/yaml.h>

#include "message/support/Configuration.h"
#include "message/motion/KinematicsModels.h"

namespace module {
namespace motion {

    class KinematicsConfiguration : public NUClear::Reactor {

    public:
        /// @brief Called by the powerplant to build and setup the KinematicsConfiguration reactor.
        explicit KinematicsConfiguration(std::unique_ptr<NUClear::Environment> environment);

    private:
    	void configure (message::motion::kinematics::KinematicsModel& model, const message::support::Configuration& objDarwinModel);
        void configureLeg (message::motion::kinematics::KinematicsModel& model, const YAML::Node& objLeg);
        void configureHead (message::motion::kinematics::KinematicsModel& model, const YAML::Node& objHead);
        void configureArm (message::motion::kinematics::KinematicsModel& model, const YAML::Node& objArm);
    	void configureMassModel (message::motion::kinematics::KinematicsModel& model, const YAML::Node& objMassModel);
    };

}
}

#endif  // MODULES_PLATFORM_DARWIN_KINEMATICSMODEL_H
