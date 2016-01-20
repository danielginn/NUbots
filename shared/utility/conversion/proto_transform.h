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

#ifndef UTILITY_CONVERSION_PROTO_TRANSFORM_H
#define UTILITY_CONVERSION_PROTO_TRANSFORM_H

#include "utility/conversion/proto_armadillo.h"

/**
 * @brief Functions to convert the transform classes
 */
protobuf::message::Transform2D& operator<< (protobuf::message::Transform2D& proto, const utility::math::matrix::Transform2D& transform) {
    *proto.mutable_transform() << static_cast<arma::vec3>(transform);
    return proto;
}

utility::math::matrix::Transform2D& operator<< (utility::math::matrix::Transform2D& transform, const protobuf::message::Transform2D& proto) {
    arma::vec3 t;
    t << proto.transform();
    transform = t;
    return transform;
}

protobuf::message::Transform3D& operator<< (protobuf::message::Transform3D& proto, const utility::math::matrix::Transform3D& transform) {
    *proto.mutable_transform() << static_cast<arma::mat44>(transform);
    return proto;
}

utility::math::matrix::Transform3D& operator<< (utility::math::matrix::Transform3D& transform, const protobuf::message::Transform3D& proto) {
    arma::mat44 t;
    t << proto.transform();
    transform = t;
    return transform;
}

#endif  // UTILITY_CONVERSION_PROTO_TRANSFORM_H
