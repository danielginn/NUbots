/*
 * This file is part of the NUbots Codebase.
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

#ifndef MESSAGE_BEHAVIOUR_FIXED_WALK_COMMAND_H
#define MESSAGE_BEHAVIOUR_FIXED_WALK_COMMAND_H

#include <nuclear>
#include <armadillo>
#include <yaml-cpp/yaml.h>

namespace message {
    namespace behaviour {

        struct FixedWalkFinished{};
        struct WalkOptimiserCommand{
            YAML::Node walkConfig;
        };
        struct WalkConfigSaved{};
        struct CancelFixedWalk{};
        class FixedWalkCommand{
        public:
            FixedWalkCommand() : segments() {}

            struct WalkSegment
            {
                WalkSegment()
                    : direction(arma::fill::zeros)
                    , curvePeriod(0.0)
                    , normalisedVelocity(0.0)
                    , normalisedAngularVelocity(0.0)
                    , duration() {}

                arma::vec2 direction;
                double curvePeriod;

                double normalisedVelocity;
                double normalisedAngularVelocity;
                NUClear::clock::duration duration;
            };

            std::vector<WalkSegment> segments;
        };
    }
}

#endif
