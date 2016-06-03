/*
 * Should produce world to robot coordinate transform
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

#include "MotionModel.h"

#include "utility/math/geometry/UnitQuaternion.h"

namespace module {
    namespace platform {
        namespace darwin {

            using utility::math::geometry::UnitQuaternion;

            arma::vec::fixed<MotionModel::size> MotionModel::limitState(const arma::vec::fixed<size>& state) {
                arma::vec::fixed<size> newState = state;
                newState.rows(QW, QZ) = arma::normalise(newState.rows(QW, QZ));
                return newState;
            }

            // @brief The process equation is used to update the systems state using the process euquations of the system.
            // @param sigma_point The sigma point representing a system state.
            // @param deltaT The amount of time that has passed since the previous update, in seconds.
            // @param measurement The reading from the rate gyroscope in rad/s used to update the orientation.
            // @return The new estimated system state.
            arma::vec::fixed<MotionModel::size> MotionModel::timeUpdate(const arma::vec::fixed<size>& state, double deltaT) {

                // Prepare our new state
                arma::vec::fixed<MotionModel::size> newState = state;

                // Extract our unit quaternion rotation
                UnitQuaternion rotation(state.rows(QW, QZ));

                // Add our global velocity to our position (rotate our local velocity)
                newState.rows(PX, PZ) += state.rows(VX, VZ)*deltaT;

                // Robot rotational velocity delta
                const double omega = arma::norm(state.rows(WX, WZ)) + 0.00000000001;
                //Negate to compensate for some later mistake.
                //deltaT has been negative for a while and has masked an incorrect hack below
                const double theta = -omega*deltaT*0.5;
                const double sinTheta = sin(theta);
                const double cosTheta = cos(theta);
                arma::vec vq({cosTheta,state(WX)*sinTheta/omega,state(WY)*sinTheta/omega,state(WZ)*sinTheta/omega});

                // Update our rotation
                newState.rows(QW, QZ) = rotation * UnitQuaternion(vq);
                
                //add velocity decay
                newState.rows(VX, VZ) = newState.rows(VX, VZ) % timeUpdateVelocityDecay;

                return newState;
            }

            // Accelerometer
            arma::vec3 MotionModel::predictedObservation(const arma::vec::fixed<size>& state, const MeasurementType::ACCELEROMETER&) {

                // Extract our rotation quaternion
                UnitQuaternion rotation(state.rows(QW, QZ));

                // Make a gravity vector and return it
                return rotation.rotateVector(arma::vec3({0, 0, G}));
            }

            // Gyroscope
            arma::vec3 MotionModel::predictedObservation(const arma::vec::fixed<size>& state, const MeasurementType::GYROSCOPE&) {
                return state.rows(WX, WZ);
            }

            // Foot up with z
            arma::vec4 MotionModel::predictedObservation(const arma::vec::fixed<size>& state, const MeasurementType::FOOT_UP_WITH_Z&) {

                arma::vec4 prediction;

                // Extract our rotation quaternion
                UnitQuaternion rotation(state.rows(QW, QZ));

                // First 3 is the up vector in torso space
                prediction.rows(0,2) = rotation.rotateVector(arma::vec3({0,0,1}));

                // 4th component is our z height
                prediction[3] = state[PZ];

                return prediction;
            }

            // Flat foot odometry measurement
            arma::vec2 MotionModel::predictedObservation(const arma::vec::fixed<size>& state, const arma::vec2& originalXY, const MeasurementType::FLAT_FOOT_ODOMETRY&)  {
                // Predict our delta from our original position to our current position
                return state.rows(PX, PY) - originalXY;
            }

            arma::vec MotionModel::observationDifference(const arma::vec& a, const arma::vec& b) {
                return a - b;
            }

            const arma::mat::fixed<MotionModel::size, MotionModel::size>& MotionModel::processNoise() {
                // Return our process noise matrix
                return processNoiseMatrix;
            }

        }
    }
}
