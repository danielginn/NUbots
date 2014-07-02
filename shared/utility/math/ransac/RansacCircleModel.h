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

#ifndef UTILITY_MATH_RANSAC_RANSACCIRCLEMODEL_H
#define UTILITY_MATH_RANSAC_RANSACCIRCLEMODEL_H

#include <vector>
#include <armadillo>

namespace utility {
namespace math {
namespace ransac {

    class RansacCircleModel {
    private:
        arma::vec2 centre;
        double radius;

    public:

        static constexpr size_t MINIMUM_POINTS = 3;
        using DataType = arma::vec2;

        RansacCircleModel() {};

        bool regenerate(const std::vector<DataType>& points);

        double calculateError(const DataType& p) const;

        double getRadius() const;

        arma::vec2 getCentre() const;

    private:
        bool constructFromPoints(const DataType& point1, const DataType& point2, const DataType& point3, double tolerance = 1.0e-6);
    };

}
}
}

#endif
