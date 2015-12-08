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

#ifndef UTILITY_CONVERSION_PROTO_YAML_H
#define UTILITY_CONVERSION_PROTO_YAML_H

#include <yaml-cpp/yaml.h>
#include <google/protobuf/struct.pb.h>

google::protobuf::Struct& operator<< (google::protobuf::Struct& proto, const YAML::Node& node) {
    return proto;
}

YAML::Node& operator<< (YAML::Node& node, const google::protobuf::Struct& proto) {
    return node;
}

#endif  // UTILITY_CONVERSION_PROTO_YAML_H
