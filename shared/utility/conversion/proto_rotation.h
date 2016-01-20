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
 * Copyright 2015 NUbots <nubots@nubots.net>
 */

#ifndef UTILITY_CONVERSION_PROTO_ROTATION_H
#define UTILITY_CONVERSION_PROTO_ROTATION_H

#include "utility/conversion/proto_armadillo.h"

/**
 * @brief Functions to convert the rotation classes
 */
protobuf::message::Rotation2D& operator<< (protobuf::message::Rotation2D& proto, const utility::math::matrix::Rotation2D& transform) {
    *proto.mutable_rotation() << static_cast<arma::mat22>(transform);
    return proto;
}

utility::math::matrix::Rotation2D& operator<< (utility::math::matrix::Rotation2D& transform, const protobuf::message::Rotation2D& proto) {
    arma::mat22 t;
    t << proto.rotation();
    transform = t;
    return transform;
}

protobuf::message::Rotation3D& operator<< (protobuf::message::Rotation3D& proto, const utility::math::matrix::Rotation3D& transform) {
    *proto.mutable_rotation() << static_cast<arma::mat33>(transform);
    return proto;
}

utility::math::matrix::Rotation3D& operator<< (utility::math::matrix::Rotation3D& transform, const protobuf::message::Rotation3D& proto) {
    arma::mat33 t;
    t << proto.rotation();
    transform = t;
    return transform;
}

#endif  // UTILITY_CONVERSION_PROTO_ROTATION_H
