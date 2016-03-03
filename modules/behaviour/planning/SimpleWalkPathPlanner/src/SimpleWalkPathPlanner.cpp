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

#include "SimpleWalkPathPlanner.h"

#include <cmath>
#include "messages/behaviour/KickPlan.h"
#include "messages/support/Configuration.h"
#include "messages/input/Sensors.h"
#include "messages/localisation/FieldObject.h"
#include "messages/vision/VisionObjects.h"
#include "messages/motion/WalkCommand.h"
#include "messages/motion/KickCommand.h"
#include "utility/nubugger/NUhelpers.h"
#include "utility/localisation/transform.h"
#include "utility/math/matrix/Transform2D.h"


namespace modules {
    namespace behaviour {
        namespace planning {

            using messages::support::Configuration;
            using messages::input::Sensors;
            using messages::motion::WalkCommand;
            using messages::behaviour::WalkTarget;
            using messages::behaviour::WalkApproach;
            using messages::behaviour::KickPlan;
            using messages::behaviour::MotionCommand;
            using messages::motion::WalkStartCommand;
            using messages::motion::WalkStopCommand;
            using messages::motion::KickFinished;
            using utility::localisation::transform::RobotToWorldTransform;
            using utility::math::matrix::Transform2D;
            using utility::nubugger::graph;
            using utility::nubugger::drawSphere;

            using LocalisationBall = messages::localisation::Ball;
            using Self = messages::localisation::Self;
            using VisionBall = messages::vision::Ball;
            using VisionObstacle = messages::vision::Obstacle;

            SimpleWalkPathPlanner::SimpleWalkPathPlanner(std::unique_ptr<NUClear::Environment> environment) : Reactor(std::move(environment)) {
                //we will initially stand still
                planType = messages::behaviour::WalkApproach::StandStill;

                //do a little configurating
                on<Configuration>("SimpleWalkPathPlanner.yaml").then([this] (const Configuration& file){

                    turnSpeed = file.config["turnSpeed"].as<float>();
                    forwardSpeed = file.config["forwardSpeed"].as<float>();
                    a = file.config["a"].as<float>();
                    b = file.config["b"].as<float>();

                });


                on<Trigger<KickFinished>>().then([this] (const KickFinished&) {
                    emit(std::move(std::make_unique<WalkStartCommand>()));
                });

                on<Every<20, Per<std::chrono::seconds>>
                 , With<messages::localisation::Ball>
                 , With<std::vector<messages::localisation::Self>>
                 , With<Optional<std::vector<messages::vision::Obstacle>>>
                 , Sync<SimpleWalkPathPlanner>>().then([this] (
                     const LocalisationBall& ball,
                     const std::vector<Self>& selfs,
                     std::shared_ptr<const std::vector<VisionObstacle>> robots) {

                    if (planType == messages::behaviour::WalkApproach::StandStill) {

                        emit(std::make_unique<WalkStopCommand>());
                        return;

                    }
                    else if (planType == messages::behaviour::WalkApproach::DirectCommand) {

                        std::unique_ptr<WalkCommand> command = std::make_unique<WalkCommand>();
                        command->command.xy()    = currentTargetPosition;
                        command->command.angle() = currentTargetHeading[0];
                        emit(std::move(command));
                        emit(std::move(std::make_unique<WalkStartCommand>()));
                        return;

                    }

                    // TODO: support non-ball targets

                    float angle = std::atan2(ball.position[1], ball.position[0]);
                    angle = std::min(turnSpeed, std::max(angle, -turnSpeed));
                    // emit(graph("angle", angle));
                    // emit(graph("ball position", ball.position));
                    // emit(graph("robot position", selfs.front().position));
                    // emit(graph("robot heading", selfs.front().heading));

                    float distanceToBall = arma::norm(ball.position);
                    float scale = 2.0 / (1.0 + std::exp(-a * distanceToBall + b)) - 1.0;
                    float scale2 = angle / M_PI;
                    float finalForwardSpeed = forwardSpeed * scale * (1.0 - scale2);
                    // emit(graph("forwardSpeed1", forwardSpeed));
                    // emit(graph("scale", scale));
                    // emit(graph("distanceToBall", distanceToBall));
                    // emit(graph("forwardSpeed2", finalForwardSpeed));

                    std::unique_ptr<WalkCommand> command = std::make_unique<WalkCommand>();
                    command->command = Transform2D({finalForwardSpeed, 0, angle});

                    arma::vec2 ball_world_position = RobotToWorldTransform(selfs.front().position, selfs.front().heading, ball.position);
                    arma::vec2 kick_target = 2 * ball_world_position - selfs.front().position;
                    emit(drawSphere("kick_target", arma::vec3({kick_target[0], kick_target[1], 0.0}), 0.1, arma::vec3({1, 0, 0}), 0));

                    emit(std::make_unique<KickPlan>(KickPlan{kick_target}));
                    emit(std::move(std::make_unique<WalkStartCommand>()));
                    emit(std::move(command));

                });

                on<Trigger<MotionCommand>, Sync<SimpleWalkPathPlanner>>().then([this] (const MotionCommand& cmd) {
                    //save the plan
                    planType = cmd.walkMovementType;
                    targetHeading = cmd.targetHeadingType;
                    targetPosition = cmd.targetPositionType;
                    currentTargetPosition = cmd.target;
                    currentTargetHeading = cmd.heading;
                });

            }

        }  // planning
    }  // behaviours
}  // modules