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

#include "SoccerStrategy.h"

#include "messages/behaviour/LookStrategy.h"
#include "messages/behaviour/WalkStrategy.h"
#include "messages/input/Sensors.h"
#include "messages/platform/darwin/DarwinSensors.h"
#include "messages/support/Configuration.h"

#include "utility/math/geometry/Plane.h"
#include "utility/math/geometry/ParametricLine.h"
#include "utility/support/armayamlconversions.h"

namespace modules {
    namespace behaviour {
        namespace strategy {

		using messages::localisation::Ball;
		using messages::localisation::Self;
		using messages::behaviour::LookAtAngle;
		using messages::behaviour::LookAtPoint;
		using messages::behaviour::LookAtPosition;
		using messages::behaviour::HeadBehaviourConfig;
		using messages::input::Sensors;
		using messages::platform::darwin::DarwinSensors;
		using messages::support::Configuration;
		using messages::support::FieldDescription;
		using messages::motion::KickCommand;
		using messages::motion::WalkCommand;
		using messages::behaviour::WalkStrategy;
		using messages::behaviour::WalkTarget;
		using messages::behaviour::WalkApproach;

		using utility::math::geometry::Plane;
		using utility::math::geometry::ParametricLine;

		SoccerStrategy::SoccerStrategy(std::unique_ptr<NUClear::Environment> environment) : Reactor(std::move(environment)) {
			penalisedButtonStatus = false;
			feetOnGround = true;
			isKicking = false;
			isWalking = false;

			on<Trigger<Configuration<SoccerStrategyConfig>>>([this](const Configuration<SoccerStrategyConfig>& config) {
				std::vector<arma::vec2> zone;

				MAX_BALL_DISTANCE = config["MAX_BALL_DISTANCE"].as<float>();
				KICK_DISTANCE_THRESHOLD = config["KICK_DISTANCE_THRESHOLD"].as<float>();
				BALL_CERTAINTY_THRESHOLD = config["BALL_CERTAINTY_THRESHOLD"].as<float>();
				BALL_SELF_INTERSECTION_REGION = config["BALL_SELF_INTERSECTION_REGION"].as<float>();
				BALL_LOOK_ERROR = config["BALL_LOOK_ERROR"].as<arma::vec2>();
				BALL_MOVEMENT_THRESHOLD = config["BALL_MOVEMENT_THRESHOLD"].as<float>();
				IS_GOALIE = (config["GOALIE"].as<int>() == 1);
				START_POSITION = config["START_POSITION"].as<arma::vec2>();
				MY_ZONE = config["MY_ZONE"].as<int>();

				ZONE_DEFAULTS.push_back(config["ZONE_1_DEFAULT"].as<arma::vec2>());
				zone = config["ZONE_1"].as<std::vector<arma::vec2>>();
				ZONES.push_back(zone);

				ZONE_DEFAULTS.push_back(config["ZONE_2_DEFAULT"].as<arma::vec2>());
				zone = config["ZONE_2"].as<std::vector<arma::vec2>>();
				ZONES.push_back(zone);

				ZONE_DEFAULTS.push_back(config["ZONE_3_DEFAULT"].as<arma::vec2>());
				zone = config["ZONE_3"].as<std::vector<arma::vec2>>();
				ZONES.push_back(zone);

				ZONE_DEFAULTS.push_back(config["ZONE_4_DEFAULT"].as<arma::vec2>());
				zone = config["ZONE_4"].as<std::vector<arma::vec2>>();
				ZONES.push_back(zone);
			});

			// Last 10 to do some button debouncing.
			on<Trigger<Last<10, messages::platform::darwin::DarwinSensors>>>([this](const std::vector<std::shared_ptr<const messages::platform::darwin::DarwinSensors>>& sensors) {
				int leftCount = 0, middleCount = 0;

				for (size_t i = 0; i < sensors.size(); i++) {
					if (sensors.at(i)->buttons.left) {
						leftCount++;
					}
					if (sensors.at(i)->buttons.middle) {
						middleCount++;
					}
				}

				penalisedButtonStatus = ((leftCount / 10) >= 0.75) ? !penalisedButtonStatus : penalisedButtonStatus;
				penalisedButtonStatus = ((middleCount / 10) >= 0.75) ? !penalisedButtonStatus : penalisedButtonStatus;
			});

			// Check to see if both feet are on the ground.
			on<Trigger<messages::input::Sensors>>([this](const messages::input::Sensors& sensors) {
				feetOnGround = (sensors.leftFootDown && sensors.rightFootDown);
			});

			// Check to see if we are about to kick.
			on<Trigger<messages::motion::KickCommand>>([this](const messages::motion::KickCommand& kick) {
				isKicking = true;
				kickData = kick;
			});

			// Check to see if the kick has finished.
			on<Trigger<messages::motion::KickFinished>>([this](const messages::motion::KickFinished& kick) {
				isKicking = false;
			});

			// Check to see if we are walking.
			on<Trigger<messages::motion::WalkStartCommand>, With<messages::motion::WalkCommand>>
				([this](const messages::motion::WalkStartCommand& walkStart,
					const messages::motion::WalkCommand& walk) {
				isWalking = true;
				walkData = walk;
			});

			// Check to see if we are no longer walking.
			on<Trigger<messages::motion::WalkStopCommand>>([this](const messages::motion::WalkStopCommand& walkStop) {
				isWalking = false;
			});

			// Get the field description.
			on<Trigger<messages::support::FieldDescription>>([this](const messages::support::FieldDescription& field) {
				FIELD_DESCRIPTION = field;
			});

			on<Trigger<Every<30, Per<std::chrono::seconds>>>, Options<Single>, 
				With<messages::localisation::Ball>,
				With<messages::localisation::Self>
				/* Need to add in game controller triggers. */
				/* Need to add team localisation triggers. */>([this](
											const time_t&,
											const messages::localisation::Ball& ball,
											const messages::localisation::Self& self
											/* Need to add in game controller parameters. */
											/* Need to add in team localisation  parameters. */) {

					// Make a copy of the previous state.					
					memcpy(&previousState, &currentState, sizeof(State));

					// Store current position and heading.
					currentState.transform = self.robot_to_world_rotation;
					currentState.position = self.position;
					currentState.heading = atan2(currentState.transform(1, 0), currentState.transform(0, 0));

					// What state is the game in?
					// Initial?
					/* currentState.gameState.Initial = gameController[STATE_INITIAL] */;
					// Set?
					/* currentState.gameState.Set = gameController[STATE_SET] */;
					// Ready?
					/* currentState.gameState.Ready = gameController[STATE_READY] */;
					// Finish?
					/* currentState.gameState.Finish = gameController[STATE_FINISH] */;
					// Playing?
					/* currentState.gameState.Playing = gameController[STATE_PLAYING] */;
					// Penalty kick?
					/* currentState.gameState.penaltyKick = gameController[STATE_PENALTY_KICK] */;
					// Free kick?
					/* currentState.gameState.freeKick = gameController[STATE_FREE_KICK] */;
					// Goal kick?
					/* currentState.gameState.goalKick = gameController[STATE_GOAL_KICK] */;
					// Corner kick?
					/* currentState.gameState.cornerKick = gameController[STATE_CORNER_KICK] */;
					// Throw-In?
					/* currentState.gameState.throwIn = gameController[STATE_THROW_IN] */;
					// Paused?
					/* currentState.gameState.paused = gameController[STATE_PAUSED] */;
					
					// Are we kicking off?
					/* currentState.kickOff = gameController[KICK_OFF] */;

					// Am I the kicker?
					// Is my start position inside the centre circle? 
					currentState.kicker = ((arma::norm(START_POSITION, 2) < (FIELD_DESCRIPTION.dimensions.center_circle_diameter / 2)) && (currentState.gameState.ready || currentState.gameState.set || currentState.gameState.playing)) || 
								((currentState.gameState.penaltyKick || currentState.gameState.freeKick || currentState.gameState.goalKick || currentState.gameState.cornerKick) /* && currentState.??? */);

					// Have I been picked up?
					currentState.pickedUp = !feetOnGround;

					// Am I penalised?
					currentState.penalised = penalisedButtonStatus /* || gameController[PENALISED] */;

					// Was I just put down?
					currentState.putDown = feetOnGround && previousState.pickedUp;
					
					// Did I just become unpenalised?
					currentState.unPenalised = !currentState.penalised && previousState.penalised;

					// Am I in my zone?
					/* currentState.selfInZone = pointInPolygon(MY_ZONE, self.position); */
							
					// Can I see the ball?
					currentState.ballSeen = ((ball.sr_xx < BALL_CERTAINTY_THRESHOLD) && (ball.sr_xy < BALL_CERTAINTY_THRESHOLD) && (ball.sr_yy < BALL_CERTAINTY_THRESHOLD));

					// Can anyone else see the ball?
					/* currentState.teamBallSeen = ((teamBall.sr_xx < BALL_CERTAINTY_THRESHOLD) && (teamBall.sr_xy < BALL_CERTAINTY_THRESHOLD) && (teamBall.sr_yy < BALL_CERTAINTY_THREHOLD)); */

					// Is the ball lost?
					currentState.ballLost = !currentState.ballSeen && !currentState.teamBallSeen;

					// Select the best ball to use (our ball or the teams ball).
					// We could be a little more sophisticated here and consider how certain we are about the 
					// balls location.
					if (!currentState.ballLost && currentState.ballSeen) {
						memcpy(&currentState.ball, &ball, sizeof(Ball));
					}

					else if (!currentState.ballLost && currentState.teamBallSeen)  { 
						/* memcpy(&currentState.ball, &teamBall, sizeof(Ball)) */;
					}

					// Has the ball moved?
					currentState.ballHasMoved = arma::norm(currentState.ball.position - previousState.ball.position, 2) > BALL_MOVEMENT_THRESHOLD;

					// Is the ball in my zone?
					currentState.ballInZone = (!currentState.ballLost /* && pointInPolygon(MY_ZONE, currentState.ball.position) */);
							
					// Are the goals in range?
					// x = position[0]?
					// x = 0 = centre field.
					// Assumption: We could potentially kick a goal if we are in the other half of the field.
					// Assumption: goal.position is the x, y coordinates of the goals relative to us.
					currentState.goalInRange = (!currentState.ballLost && (arma::norm(currentState.ball.position, 2) < MAX_BALL_DISTANCE) && ((transformPoint(currentState.ball.position) + currentState.position)[0] > 0));

					// Am I in position to kick the ball?
					bool kickThreshold = arma::norm(currentState.ball.position, 2) < KICK_DISTANCE_THRESHOLD;
					bool inPosition = (currentState.position[0] == previousState.targetPosition[0]) && (currentState.position[1] == previousState.targetPosition[1]);
					bool correctHeading = (currentState.heading[0] == previousState.targetHeading[0]) && (currentState.heading[1] == previousState.targetHeading[1]);
					currentState.kickPosition = (inPosition && correctHeading && kickThreshold);
				
					// Make preparations to calculate whether the ball is approaching our own goals or ourselves.
					Plane<2> planeGoal, planeSelf;
					ParametricLine<2> line;
					arma::vec2 xaxis = {1, 0};
					arma::vec2 fieldWidth = {-FIELD_DESCRIPTION.dimensions.field_length / 2, 0};

					planeGoal.setFromNormal(xaxis, fieldWidth);
					planeSelf.setFromNormal(transformPoint(currentState.ball.position), currentState.position);

					line.setFromDirection(transformPoint(currentState.ball.velocity), (transformPoint(currentState.ball.position) + currentState.position));

					// Is the ball approaching our goals?
					try {
						// Throws std::domain_error if there is no intersection.
						currentState.ballGoalIntersection = planeGoal.intersect(line);

						currentState.ballApproachingGoal = arma::norm(fieldWidth - currentState.ballGoalIntersection, 2) <= (FIELD_DESCRIPTION.dimensions.goal_area_width / 2);
						/* currentState.ballApproachingGoal = ((currentState.ballGoalIntersection[1] <= (FIELD_DESCRIPTION.dimensions.goal_area_width / 2)) &&
											 (currentState.ballGoalIntersection[1] >= -(FIELD_DESCRIPTION.dimensions.goal_area_width / 2))); */
					}

					catch (std::domain_error e) {
						currentState.ballApproachingGoal = false;
					}

					
					// Is the ball heading in my direction?
					try {
						// Throws std::domain_error if there is no intersection.
						currentState.ballSelfIntersection = planeSelf.intersect(line);

						currentState.ballApproaching = arma::norm(currentState.position - currentState.ballSelfIntersection, 2) <= (BALL_SELF_INTERSECTION_REGION / 2);
					}

					catch (std::domain_error e) {
						currentState.ballApproaching = false;
					}






					// Create a list of hints in case we need to search for the ball.
					std::vector<messages::localisation::Ball> hints = {ball/*, teamBall*/};

					// Calculate the optimal zone position.
					arma::vec2 optimalPosition = findOptimalPosition();

					// Determine current state and appropriate action(s).
					if (currentState.gameState.initial || currentState.gameState.set || currentState.gameState.finished || currentState.penalised || currentState.pickedUp) {
						stopMoving();
		
						NUClear::log<NUClear::INFO>("Standing still.");
					}
	
					else if (currentState.gameState.ready) {
						goToPoint(START_POSITION);

						NUClear::log<NUClear::INFO>("Game is about to start. I should be in my starting position.");
					}

					else if (!currentState.selfInZone) {
						goToPoint(optimalPosition);

						NUClear::log<NUClear::INFO>("I am not where I should be. Going there now.");
					}

					else if(currentState.penalised && currentState.putDown) {
						findSelf();
						findBall(hints);

						NUClear::log<NUClear::INFO>("I am penalised and have been put down. I must be on the side line somewhere. Where am I?");
					}

					else if (currentState.unPenalised) {
						goToPoint(optimalPosition);

						NUClear::log<NUClear::INFO>("I am unpenalised, I should already know where I am and where the ball is. So find the most optimal location in my zone to go to.");
					}

					else if (currentState.gameState.penaltyKick && IS_GOALIE && currentState.ballLost) {
						findBall(hints);

						NUClear::log<NUClear::INFO>("Penalty kick in progress. Locating ball.");
					}

					else if (currentState.gameState.penaltyKick && IS_GOALIE && !currentState.ballLost && currentState.ballHasMoved && !currentState.ballApproachingGoal) {
						arma::vec2 blockPosition = {currentState.position[0], (transformPoint(currentState.ball.position) + currentState.position)[1]};
						goToPoint(blockPosition);
						watchBall();

						NUClear::log<NUClear::INFO>("Penalty kick in progress. Locating ball.");
					}

					else if (currentState.gameState.penaltyKick && currentState.ballLost && currentState.kicker) {
						findBall(hints);

						NUClear::log<NUClear::INFO>("Penalty kick in progress. Locating ball.");
					}

					else if (currentState.gameState.penaltyKick && !currentState.ballLost && currentState.kicker) {
						approachBall();

						NUClear::log<NUClear::INFO>("Penalty kick in progress. Approaching ball.");
					}

					else if (previousState.gameState.set && currentState.gameState.playing && currentState.kickOff && currentState.kicker) {
						kickBall();

						NUClear::log<NUClear::INFO>("Game just started. Time to kick off.");
					}

					else if (isKicking) {
						watchBall();

						NUClear::log<NUClear::INFO>("I be looking at what I be kicking.");
					}


					else if (currentState.ballLost) {
						findBall(hints);

						NUClear::log<NUClear::INFO>("Don't know where the ball is. Looking for it.");
					}

					else if (currentState.ballInZone || currentState.ballApproaching) {
						approachBall();

						NUClear::log<NUClear::INFO>("Walking to ball.");
					}

					else if (currentState.kickPosition && !isKicking) {
						kickBall();
	
						NUClear::log<NUClear::INFO>("In kicking position. Kicking ball.");
					}

					else if (currentState.goalInRange) {
						approachBall();
						kickBall();

						NUClear::log<NUClear::INFO>("Kick for goal.");
					}

					else if (IS_GOALIE && currentState.ballApproachingGoal) {
						goToPoint(currentState.ballGoalIntersection);

						NUClear::log<NUClear::INFO>("Ball is approaching goal. Goalie moving to block it.");
					}

					else {
						findSelf();
						findBall(hints);
						goToPoint(ZONE_DEFAULTS.at(MY_ZONE));
	
						NUClear::log<NUClear::INFO>("Unknown behavioural state. Finding self, finding ball, moving to default position.");
					}
				});
			}

			arma::vec2 SoccerStrategy::findOptimalPosition() {
				arma::vec2 optimalPosition = {0, 0};

				return(optimalPosition);
			}

			void SoccerStrategy::stopMoving() {
				auto approach = std::make_unique<messages::behaviour::WalkStrategy>();
				approach->targetPositionType = WalkTarget::Robot;
				approach->targetHeadingType = WalkTarget::Robot;
				approach->walkMovementType = WalkApproach::StandStill;
				approach->heading = currentState.heading; 
				approach->target = currentState.position; 
				emit(std::move(approach));
			}

			void SoccerStrategy::findSelf() {
				/* Try to locate both goals. */
				/* Look at closest goal for short period to reset localisation. */
			}

			void SoccerStrategy::findBall(const std::vector<Ball>& hints) {
				/* Look for the ball. Use the provided hints as the last known locations to attempt to speed up the search. */
			}

			void SoccerStrategy::goToPoint(const arma::vec2& point) {
				auto approach = std::make_unique<messages::behaviour::WalkStrategy>();
				approach->targetPositionType = WalkTarget::WayPoint;
				approach->targetHeadingType = WalkTarget::WayPoint;
				approach->walkMovementType = WalkApproach::WalkToPoint;
				approach->heading = arma::zeros<arma::vec>(2); 			// TODO: fix me! 
				approach->target = point; 
				emit(std::move(approach));
			}

			void SoccerStrategy::watchBall() {
				arma::vec2 ballPosition = transformPoint(currentState.ball.position) + currentState.position;

				auto look = std::make_unique<messages::behaviour::LookAtPoint>();
				look->x = ballPosition[0];
				look->y = ballPosition[1];
				look->xError = BALL_LOOK_ERROR[0];
				look->yError = BALL_LOOK_ERROR[1];
				emit(std::move(look));
			}

			void SoccerStrategy::approachBall() {
				auto approach = std::make_unique<messages::behaviour::WalkStrategy>();
				approach->targetPositionType = WalkTarget::Ball;
				approach->targetHeadingType = WalkTarget::WayPoint;
				approach->walkMovementType = WalkApproach::ApproachFromDirection;
				approach->heading = arma::vec({-3,0});
				approach->target = transformPoint(currentState.ball.position) + currentState.position;
				emit(std::move(approach));
			}

			void SoccerStrategy::kickBall() {
			}

			arma::vec2 SoccerStrategy::transformPoint(const arma::vec2& point) {
				return (currentState.transform * point);
			}
		}  // strategy
	}  // behaviours
}  // modules