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

#include "WalkPathPlanner.h"

#include <cmath>
#include "messages/support/Configuration.h"
#include "messages/input/Sensors.h"
#include "messages/localisation/FieldObject.h"
#include "messages/vision/VisionObjects.h"
#include "messages/motion/WalkCommand.h"
#include "utility/nubugger/NUgraph.h"


namespace modules {
    namespace behaviour {
        namespace planning {

            using messages::support::Configuration;
            using messages::input::Sensors;
            using messages::motion::WalkCommand;
            using messages::behaviour::WalkTarget;
            using messages::behaviour::WalkApproach;
            using messages::motion::WalkStartCommand;
            using messages::motion::WalkStopCommand;
            using utility::nubugger::graph;
            //using namespace messages;

            //using messages::input::ServoID;
            //using messages::motion::ExecuteScriptByName;
            //using messages::behaviour::RegisterAction;
            //using messages::behaviour::ActionPriorites;
            //using messages::behaviour::LimbID;

            WalkPathPlanner::WalkPathPlanner(std::unique_ptr<NUClear::Environment> environment) : Reactor(std::move(environment)) {
                //we will initially stand still
                planType = messages::behaviour::WalkApproach::StandStill;

                //do a little configurating
                on<Trigger<Configuration<WalkPathPlanner>>>([this] (const Configuration<WalkPathPlanner>& file){

                    turnSpeed = file.config["turnSpeed"];
                    forwardSpeed = file.config["forwardSpeed"];
                    footSeparation = file.config["footSeparation"];

                    footSize = file.config["footSize"];

                    //timers for starting turning and walking
                    walkStartTime = file.config["walkStartTime"];
                    walkTurnTime = file.config["walkTurnTime"];

                    //walk accel/deccel controls
                    accelerationTime = file.config["accelerationTime"];
                    accelerationFraction = file.config["accelerationFraction"];

                    //approach speeds
                    closeApproachSpeed = file.config["closeApproachSpeed"];
                    closeApproachDistance = file.config["closeApproachDistance"];
                    midApproachSpeed = file.config["midApproachSpeed"];
                    midApproachDistance = file.config["midApproachDistance"];

                    //turning values
                    turnDeviation = file.config["turnDeviation"];

                    //hystereses
                    distanceHysteresis = file.config["distanceHysteresis"];
                    turningHysteresis = file.config["turningHysteresis"];
                    positionHysteresis = file.config["positionHysteresis"];

                    //ball lineup
                    //XXX: add these once we know what they do
                    //vector<float> ballApproachAngle = file.config["ballApproachAngle"];
                    //svector<int> ballKickFoot = file.config["ballKickFoot"];
                    ballLineupDistance = file.config["ballLineupDistance"];
                    ballLineupMinDistance = file.config["ballLineupMinDistance"];

                    //extra config options
                    useAvoidance = int(file.config["useAvoidance"]) != 0;
                    assumedObstacleWidth = file.config["assumedObstacleWidth"];
                    avoidDistance = file.config["avoidDistance"];

                    bearingSensitivity = file.config["bearingSensitivity"];
                    ApproachCurveFactor = file.config["ApproachCurveFactor"];
                });

                // add in fake walk localisation
                /*
                on<Trigger<Every<3, Per<std::chrono::seconds>>>, With<Optional<WalkCommand>>>
                    ([this](const time_t&, const std::shared_ptr<const WalkCommand>& w) {
                    static size_t ctr = 0;
                    
                    const double ballx = 0.0;
                    const double bally = 0.0;
                    const double targetx = 3.0;
                    const double targety = 0.0;
                    
                    static double x,y,h;
                    
                    if ( (ctr % 1000000000) == 0) {
                        //set position
                        x = -1.0;
                        y = -2.0;
                        h = -1.5;
                        
                        //emit ball pos
                        std::unique_ptr<messages::localisation::Ball> b = std::make_unique<messages::localisation::Ball>();
                        b->position = arma::vec({ballx,bally});
                        emit(std::move(b));
                        
                        //emit robots:
                        auto o = std::make_unique<std::vector<messages::vision::Obstacle>>();
                        emit(std::move(o));
                    }
                    
                    //update self pos
                    if (w != NULL) {
                        x += (w->velocity[0]*cos(h) - w->velocity[1]*sin(h)) * 0.015;
                        y += (w->velocity[0]*sin(h) + w->velocity[1]*cos(h)) * 0.015;
                        h += (w->rotationalSpeed) * 0.1;
                    }
                    
                    
                    
                    
                    auto robot_msg = std::make_unique<std::vector<messages::localisation::Self>>();

                    messages::localisation::Self robot_model;
                    robot_model.position = arma::vec({x,y});
                    robot_model.heading = arma::vec({cos(h),sin(h)});
                    robot_model.sr_xx = 0.1;
                    robot_model.sr_xy = 0.1;
                    robot_model.sr_yy = 0.1;
                    robot_msg->push_back(robot_model);
                    
                    emit(std::move(robot_msg));
                    
                    ++ctr;
                    });*/

                on<Trigger<Startup>>([this](const Startup&) {
                    emit(std::make_unique<WalkStartCommand>());
                });

                on<Trigger<Every<20, Per<std::chrono::seconds>>>,
                    With<messages::localisation::Ball>,
                    With<std::vector<messages::localisation::Self>>,
                    With<std::vector<messages::vision::Obstacle>>,
                    With<std::vector<messages::vision::Ball>>,
                    Options<Sync<WalkPathPlanner>>
                   >([this] (
                     const time_t& now,
                     const messages::localisation::Ball& ball,
                     const std::vector<messages::localisation::Self>& selfs,
                     const std::vector<messages::vision::Obstacle>& robots,
                     const std::vector<messages::vision::Ball>& visionBalls
                    ) {
                    if(visionBalls.size()>0){
                        arma::vec ballPosition = ball.position;

                        //Jake walk path planner:
                        auto self = selfs[0];

                        arma::vec goalPosition = arma::vec3({-3,0,1});

                        arma::vec normed_heading = arma::normalise(self.heading);
                        arma::mat worldToRobotTransform = arma::mat33{      normed_heading[0],  normed_heading[1],         0,
                                                                             -normed_heading[1],  normed_heading[0],         0,
                                                                                              0,                 0,         1};

                        worldToRobotTransform.submat(0,2,1,2) = -worldToRobotTransform.submat(0,0,1,1) * self.position;
                        
                        arma::vec homogeneousKickTarget = worldToRobotTransform * goalPosition;
                        arma::vec kickTarget_robot = homogeneousKickTarget.rows(0,1);    //In robot coords
                        arma::vec kickDirection = arma::normalise(kickTarget_robot-ballPosition);    //In robot coords
                        arma::vec kickDirectionNormal = arma::vec2({-kickDirection[1], kickDirection[0]});

                        //float kickTargetBearing_robot = std::atan2(kickTarget_robot[1],kickTarget_robot[0]);
                        float ballBearing = std::atan2(ballPosition[1],ballPosition[0]);
                        
                        //calc self in kick coords
                        arma::vec moveTarget = ballPosition - ballLineupDistance * kickDirection;

                        arma::mat robotToKickFrame = arma::mat33{      kickDirection[0],  kickDirection[1],         0,
                                                                        -kickDirection[1],  kickDirection[0],         0,
                                                                                              0,                 0,         1};
                        robotToKickFrame.submat(0,2,1,2) = -robotToKickFrame.submat(0,0,1,1) * moveTarget;
                        
                        arma::vec selfInKickFrame = robotToKickFrame * arma::vec3({0,0,1});

                        //Hyperboal x >a*sqrt(y^2/a^2 + 1)
                        if(selfInKickFrame[0] > ballLineupDistance * std::sqrt(selfInKickFrame[1]*selfInKickFrame[1]/(ApproachCurveFactor*ApproachCurveFactor) + 1)){   //Inside concave part
                            arma::vec moveTargetA = ballPosition + ballLineupDistance * kickDirectionNormal;
                            arma::vec moveTargetB = ballPosition - ballLineupDistance * kickDirectionNormal;
                            if(arma::norm(moveTargetA) < arma::norm(moveTargetB)){
                                moveTarget = moveTargetA;
                            } else {
                                moveTarget = moveTargetB;
                            }                        
                        }

                        if(arma::norm(moveTarget) < closeApproachDistance) {
                            moveTarget = moveTarget * closeApproachSpeed;
                        }

                        std::unique_ptr<WalkCommand> command = std::make_unique<WalkCommand>();

                        command->velocity = arma::normalise(arma::vec2{moveTarget[0],moveTarget[1]});
                        command->rotationalSpeed = ballBearing;  //vx,vy, alpha
                        emit(graph("Walk command:", command->velocity[0], command->velocity[1], command->rotationalSpeed));
                        emit(std::move(command));//XXX: emit here


                        //emit(std::move(std::make_unique<WalkStartCommand>()));

                    } else {
                        std::unique_ptr<WalkCommand> command = std::make_unique<WalkCommand>();
                        command->velocity = arma::vec({0,0});
                        command->rotationalSpeed = -turnSpeed;  //vx,vy, alpha
                        emit(graph("Walk command:", command->velocity[0], command->velocity[1], command->rotationalSpeed));
                        emit(std::move(command));//XXX: emit here
                    }
                    /*

                    //std::cout << "starting path planning" << std::endl;
                    arma::vec targetPos, targetHead;
                    //work out where we're going
                    if (targetPosition == messages::behaviour::WalkTarget::Robot) {
                        //XXX: check if robot is visible
                    } else if (targetPosition ==messages::behaviour::WalkTarget::Ball) {
                        targetPos = ball.position;
                    } else { //other types default to position/waypoint location
                        targetPos = currentTargetPosition;
                    }
                    //work out where to face when we get there
                    if (targetHeading == messages::behaviour::WalkTarget::Robot) {
                        //XXX: check if robot is visible
                    } else if (targetHeading == messages::behaviour::WalkTarget::Ball) {
                        targetHead = arma::normalise(arma::vec(ball.position)-targetPos);
                    } else { //other types default to position/waypoint bearings
                        targetHead = arma::normalise(arma::vec(currentTargetHeading)-targetPos);
                    }
                    //calculate the basic movement plan
                    arma::vec movePlan;

                    switch (planType) {
                        case messages::behaviour::WalkApproach::ApproachFromDirection:
                            movePlan = approachFromDirection(self.front(),targetPos,targetHead);
                            break;
                        case messages::behaviour::WalkApproach::WalkToPoint:
                            movePlan = goToPoint(self.front(),targetPos,targetHead);
                            break;
                        case messages::behaviour::WalkApproach::OmnidirectionalReposition:
                            movePlan = goToPoint(self.front(),targetPos,targetHead);
                            break;
                        case messages::behaviour::WalkApproach::StandStill:
                            emit(std::make_unique<WalkStopCommand>());
                            return;
                    }
                    //work out if we have to avoid something
                    if (useAvoidance) {
                        //this is a vision-based temporary for avoidance
                        movePlan = avoidObstacles(robots,movePlan);
                    }
                    //NUClear::log("Move Plan:", movePlan[0],movePlan[1],movePlan[2]);
                    // NUClear::log("Move Plan:", movePlan[0],movePlan[1],movePlan[2]);
                    //this applies acceleration/deceleration and hysteresis to movement
                    movePlan = generateWalk(movePlan,
                               planType == messages::behaviour::WalkApproach::OmnidirectionalReposition);

                    std::unique_ptr<WalkCommand> command = std::make_unique<WalkCommand>();
                    command->velocity = arma::vec({movePlan[0],movePlan[1]});
                    command->rotationalSpeed = movePlan[2];
                    // NUClear::log("Self Position:", self[0].position[0],self[0].position[1]);
                    // NUClear::log("Target Position:", targetPos[0],targetPos[1]);
                    emit(graph("Walk command:", command->velocity[0], command->velocity[1], command->rotationalSpeed));
                    // NUClear::log("Ball Position:", ball.position[0],ball.position[1]);
                    //std::cout << command->velocity << std::endl;
                    emit(std::move(command));//XXX: emit here


                    emit(std::move(std::make_unique<WalkStartCommand>()));*/
                });

                on<Trigger<messages::behaviour::WalkStrategy>, Options<Sync<WalkPathPlanner>>>([this] (const messages::behaviour::WalkStrategy& cmd) {
                    //reset hysteresis variables when a command changes
                    turning = 0;
                    distanceIncrement = 3;

                    //save the plan
                    planType = cmd.walkMovementType;
                    targetHeading = cmd.targetHeadingType;
                    targetPosition = cmd.targetPositionType;
                    currentTargetPosition = cmd.target;
                    currentTargetHeading = cmd.heading;
                });

                //Walk planning testing: Walk to ball face to goal
                // auto approach = std::make_unique<messages::behaviour::WalkStrategy>();
                // approach->targetPositionType = WalkTarget::Ball;
                // approach->targetHeadingType = WalkTarget::WayPoint;
                // approach->walkMovementType = WalkApproach::ApproachFromDirection;
                // approach->heading = arma::vec({-3,0});
                // approach->target = arma::vec({0,0});
                // emit(std::move(approach));
                

            }



            arma::vec WalkPathPlanner::generateWalk(const arma::vec& move, bool omniPositioning) {
                //this uses hystereses to provide a stable, consistent positioning and movement
                double walk_speed;
                double walk_bearing;

                //check what distance increment we're in:
                if (distanceIncrement == 3 or move[0] > midApproachDistance+distanceHysteresis) {
                    distanceIncrement = 3;
                    walk_speed = forwardSpeed;
                } else if (distanceIncrement == 2 or move[0] > closeApproachDistance + distanceHysteresis and
                           move[0] < midApproachDistance) {
                    distanceIncrement = 2;
                    walk_speed = midApproachSpeed;
                } else if (distanceIncrement == 1 or move[0] > ballLineupDistance + distanceHysteresis and
                           move[0] < closeApproachDistance) {
                    distanceIncrement = 1;
                    walk_speed = closeApproachSpeed;
                } else {
                    distanceIncrement = 0;
                    walk_speed = 0.f;
                }
                //std::cout << "fwd: " << forwardSpeed << ", " << distanceIncrement << std::endl;
                //std::cout << midApproachDistance << ", " << closeApproachDistance << ", " << ballLineupDistance << ", " << distanceHysteresis << std::endl;
                //std::cout << move[0] << std::endl;

                //decide between heading and bearing
                if (distanceIncrement > 1) {
                    walk_bearing = move[1];
                } else {
                    walk_bearing = move[2];
                }

                //check turning hysteresis
                /*if (turning < 0 and walk_bearing < -turnDeviation) {
                    //walk_speed = std::min(walk_bearing,turnSpeed);
                } else if (m_turning > 0 and walk_bearing > turnDeviation) {
                    //walk_speed = std::min(walk_bearing,turnSpeed);
                } else {
                    walk_bearing = 0;
                }*/

                //This replaces turn hysteresis with a unicorn equation
                float g = 1./(1.+std::exp(-4.*walk_bearing*walk_bearing));

                /*arma::vec result;
                result[0] = walk_speed*(1.-g);
                result[1] = 0;
                result[2] = walk_bearing*g;*/
                //std::cout << walk_speed << ", " << g << std::endl;
                return arma::vec({walk_speed*(1.-g),0,walk_bearing*g});
            }

            arma::vec WalkPathPlanner::approachFromDirection(const messages::localisation::Self& self,
                                                             const arma::vec& target,
                                                             const arma::vec& direction) {


                //this method calculates the possible ball approach commands for the robot
                //and then chooses the lowest cost action
                std::vector<arma::vec> waypoints(3);

                //calculate the values we need to set waypoints
                const double ballDistance = arma::norm(target-self.position);
                const double selfHeading = atan2(self.heading[1],self.heading[0]);

                //create our waypoints
                waypoints[0] = -direction*ballDistance*ApproachCurveFactor;
                waypoints[1] = arma::vec({waypoints[0][1],-waypoints[0][0]});
                waypoints[2] = arma::vec({-waypoints[0][1],waypoints[0][0]});


                //fill target headings and distances, and movement bearings and costs
                std::vector<double> headings(3);
                std::vector<double> distances(3);
                std::vector<double> bearings(3);
                std::vector<double> costs(3);
                for (size_t i = 0; i < 3; ++i) {
                    //calculate the heading the robot wants to achieve at its destination
                    const double waypointHeading = atan2(-waypoints[i][1],-waypoints[i][0])-selfHeading;

                    //std::cout << selfHeading << ", " << waypointHeading << ", " << waypoints[i][0] << ", " << waypoints[i][1] << std::endl;
                    headings[i] = atan2(sin(waypointHeading),cos(waypointHeading));

                    //calculate the distance to destination
                    arma::vec waypointPos = waypoints[i]+target-arma::vec(self.position);
                    distances[i] = arma::norm(waypointPos);

                    //calculate the angle between the current direction and the destination
                    const double waypointBearing = atan2(waypointPos[1],waypointPos[0])-selfHeading;
                    bearings[i] = atan2(sin(waypointBearing),cos(waypointBearing));

                    //std::cout << selfHeading << ", " << waypointBearing << ", " << waypointPos[0] << ", " << waypointPos[1] << ", " << bearings[i] << ", " << headings[i] << std::endl << std::endl;
                    //costs defines which move plan is the most appropriate
                    costs[i] = bearings[i]*bearings[i]*bearingSensitivity+distances[i]*distances[i];

                }


                //decide which approach to the ball is cheapest
                size_t selected;
                if (costs[0] < costs[1] and costs[0] < costs[2]) {
                    selected = 0;
                } else if (costs[1] < costs[2]) {
                    selected = 1;
                } else {
                    selected = 2;
                }

                //return the values for the selected approach
                /*arma::vec result;
                result[0] = ;
                result[1] = ;
                result[2] = ;*/
                return arma::vec({distances[selected],bearings[selected],headings[selected]});
            }

            arma::vec WalkPathPlanner::goToPoint(const messages::localisation::Self& self,
                                                  const arma::vec2& target,
                                                  const arma::vec2& direction) {
                //quick and dirty go to point calculator
                //gets position and heading difference and returns walk params for it
                const arma::vec2 posdiff = target-self.position;
                const double targetDistance = arma::norm(posdiff);
                const double selfHeading = atan2(self.heading[1],self.heading[0]);
                const double targetHeading = atan2(posdiff[1],posdiff[0])-selfHeading;
                const double targetBearing = atan2(direction[1],direction[0])-selfHeading;

                arma::vec result;
                result[0] = targetDistance;
                result[1] = atan2(sin(targetBearing),cos(targetBearing));
                result[2] = atan2(sin(targetHeading),cos(targetHeading));
                return result;
            }

            arma::vec WalkPathPlanner::avoidObstacles(const std::vector<messages::vision::Obstacle>& robotPosition,
                                                  const arma::vec3& movePlan) {
                return movePlan; //XXX:unimplemented
                //double heading = 0.0;
                /*float new_bearing = relative_bearing;
                float avoid_distance = min(m_avoid_distance,distance);
                vector<Object> obstacles;



                //use either localised or visual avoidance
                if (m_use_localisation_avoidance) {
                    //XXX: localisation based avoidance not implemented


                } else {
                    obstacles = NavigationLogic::getVisibleObstacles();
                    for(unsigned int i=0; i < obstacles.size(); i++) { //for each object
                        if (obstacles[i].measuredDistance() < avoid_distance) { //if we are an obstacle
                            if (obstacles[i].measuredBearing() > relative_bearing and obstacles[i].measuredBearing()-obstacles[i].arc_width < relative_bearing) {
                                //if we are on the right and occluding
                                new_bearing = mathGeneral::normaliseAngle(obstacles[i].measuredBearing()-obstacles[i].arc_width);
                            } else if (obstacles[i].measuredBearing() < relative_bearing and obstacles[i].measuredBearing()+obstacles[i].arc_width > relative_bearing) {
                                //if we are on the left and occluding
                                new_bearing = mathGeneral::normaliseAngle(obstacles[i].measuredBearing()+obstacles[i].arc_width);
                            }
                        }
                    }
                }*/
            }



        }  // planning
    }  // behaviours
}  // modules