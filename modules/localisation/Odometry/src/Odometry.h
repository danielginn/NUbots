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

#ifndef MODULES_LOCALISATION_ODOMETRY_H
#define MODULES_LOCALISATION_ODOMETRY_H

#include <nuclear>

namespace modules {
namespace localisation {

    class Odometry : public NUClear::Reactor {
    private:
    	enum class StepState{
    		LEFT_FOOT_DOWN,
    		RIGHT_FOOT_DOWN,
    		STAND_STILL
    	};

    	StepState lastStepState;

    public:
        /// @brief Called by the powerplant to build and setup the Odometry reactor.
        explicit Odometry(std::unique_ptr<NUClear::Environment> environment);

        /// @brief the path to the configuration file for Odometry
        static constexpr const char* CONFIGURATION_PATH = "Odometry.yaml";
    };

}
}

#endif  // MODULES_LOCALISATION_ODOMETRY_H
