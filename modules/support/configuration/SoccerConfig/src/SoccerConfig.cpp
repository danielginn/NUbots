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

#include "SoccerConfig.h"

#include <armadillo>

#include "extension/Configuration.h"
#include "messages/support/FieldDescription.h"

using extension::Configuration;
using messages::support::FieldDescription;

namespace modules {
    namespace support {
        namespace configuration {

            void setGoalpostPositions(FieldDescription& desc) {
                // Unused formulas remain as comments for completeness.

                FieldDescription::FieldDimensions& d = desc.dimensions;
                auto halfLength = d.fieldLength * 0.5;
                // auto goal_line_width = d.line_width * 0.5;
                auto goalPostRadius = d.goalpostDiameter * 0.5;
                // auto goal_x = (half_length - d.line_width * 0.5 + d.goal_depth + goal_line_width * 0.5);
                auto goalY = (d.goalWidth + d.goalpostDiameter) * 0.5;
                // auto goal_w = (d.goal_depth - d.line_width + goal_line_width * 0.5);
                // auto goal_h = (d.goal_width + d.goalpost_diameter);
                auto goalPostX = halfLength + goalPostRadius * 0.5;

                desc.goalpostOwnL = { -goalPostX, -goalY };
                desc.goalpostOwnR = { -goalPostX,  goalY };
                desc.goalpostOppL = {  goalPostX,  goalY };
                desc.goalpostOppR = {  goalPostX, -goalY };
            }

            FieldDescription loadFieldDescription(
                const Configuration& config) {
                FieldDescription desc;

                desc.ballRadius = config["ball_radius"].as<double>();

                FieldDescription::FieldDimensions& d = desc.dimensions;
                d.lineWidth = config["line_width"].as<double>();
                d.markWidth = config["mark_width"].as<double>();
                d.fieldLength = config["field_length"].as<double>();
                d.fieldWidth = config["field_width"].as<double>();
                d.goalDepth = config["goal_depth"].as<double>();
                d.goalWidth = config["goal_width"].as<double>();
                d.goalAreaLength = config["goal_area_length"].as<double>();
                d.goalAreaWidth = config["goal_area_width"].as<double>();
                d.goalCrossbarHeight = config["goal_crossbar_height"].as<double>();
                d.goalpostDiameter = config["goalpost_diameter"].as<double>();
                d.goalCrossbarDiameter = config["goal_crossbar_diameter"].as<double>();
                d.goalNetHeight = config["goal_net_height"].as<double>();
                d.penaltyMarkDistance = config["penalty_mark_distance"].as<double>();
                d.centerCircleDiameter = config["center_circle_diameter"].as<double>();
                d.borderStripMinWidth = config["border_strip_min_width"].as<double>();

                desc.penaltyRobotStart = config["penalty_robot_start"].as<double>();
                desc.goalpostTopHeight = d.goalCrossbarHeight + d.goalCrossbarDiameter;

                setGoalpostPositions(desc);

                return desc;
            }

            SoccerConfig::SoccerConfig(std::unique_ptr<NUClear::Environment> environment)
                : Reactor(std::move(environment)) {

                on<Configuration>("FieldDescription.yaml").then("FieldDescriptionConfig Update", [this](const Configuration& config) {
                    auto fd = std::make_unique<messages::support::FieldDescription>(loadFieldDescription(config));
                    emit(std::move(fd));
                });

            }

        }
    }
}

