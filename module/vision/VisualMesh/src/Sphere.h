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
 * Copyright 2016 NUbots <nubots@nubots.net>
 */

#ifndef MODULE_VISION_SPHERE_H
#define MODULE_VISION_SPHERE_H

#include <cmath>
#include <limits>

#include "message/vision/MeshObjectRequest.h"

namespace module {
namespace vision {

    class Sphere {
    public:
        static inline double deltaPhi(const message::vision::MeshObjectRequest& request, double phi, double cameraHeight) {

            // If we are looking at a sphere above our camera move our ground plane to the other side
            if (request.height > cameraHeight) {
                phi = M_PI - phi;
                cameraHeight = request.height - cameraHeight;
            }

            // If we are looking on the correct side of the horizon
            if (phi < M_PI_2) {

                // If we are within our distance range
                if (phi < angleToMaxDistance(cameraHeight, request.maxDistance)) {
                    return angularWidth(phi, cameraHeight, request.radius) / request.intersections;
                }

                // If we have passed our max distance range but are soft limiting
                else if (!request.hardLimit) {
                    return angularWidth(angleToMaxDistance(cameraHeight, request.maxDistance), cameraHeight, request.radius) / request.intersections;
                }
            }

            // The sphere is currently out of our visible range
            return std::numeric_limits<double>::max();
        }

        static inline double deltaTheta(const message::vision::MeshObjectRequest& request, double phi, double cameraHeight) {
            // If we are looking at a sphere above our camera move our ground plane to the other side
            if (request.height > cameraHeight) {
                phi = M_PI - phi;
                cameraHeight = request.height - cameraHeight;
            }

            // If we are looking on the correct side of the horizon
            if (phi < M_PI_2) {
                return 2 * std::asin(request.radius / distanceFromCameraToObjectCentre(phi, cameraHeight, request.radius));
            }

            // The sphere is currently out of our visible range
            return std::numeric_limits<double>::max();
        }

    private:
        static inline double angularWidth(double phi, double cameraHeight, double radius) {
            return 2 * (std::atan((cameraHeight / (std::cos(phi) * (cameraHeight - radius))) + std::tan(phi)) - phi);
        }

        static inline double distanceFromCameraToObjectCentre(double phi, double cameraHeight, double radius) {
            return cameraHeight * std::tan(phi) + radius / std::tan(M_PI_4 + phi / 2);
        }

        static inline double angleToMaxDistance(double cameraHeight, double maxDistance) {
            return std::atan(maxDistance / cameraHeight);
        }

    };

}
}

#endif  // MODULE_VISION_SPHERE_H