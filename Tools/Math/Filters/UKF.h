/*! @file UKF.h
 @brief Declaration of general UKF class

 @class UKF
 @brief This class is a template to create a custom UKF specific for an application.

 The processEquation and measurementEquation virtual functions must be defined for each application,
 the remainder of the algorithm is otherwise identical.

 @author Steven Nicklin

 Copyright (c) 2012 Steven Nicklin

 This file is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This file is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with NUbot.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include "Tools/Math/Moment.h"
#include "Tools/Math/Matrix.h"
#include "UnscentedTransform.h"

class UKF: public UnscentedTransform, public Moment
{
public:
    UKF();
    UKF(unsigned int numStates);
    UKF(const UKF& source);
    virtual ~UKF();

    // Public functions, these are the only two functions that should need to be called externally.
    // They are virtual in case the need to be redefined, but the default functions should work in most cases.
    virtual bool timeUpdate(double deltaT, const Matrix& measurment, const Matrix& linearProcessNoise, const Matrix& measurementNoise);
    virtual bool measurementUpdate(const Matrix& measurement, const Matrix& measurementNoise, const Matrix& measurementArgs = Matrix());

protected:
   Matrix m_mean_weights;
   Matrix m_covariance_weights;
   Matrix m_sigma_points;
   Matrix m_sigma_mean;

   // Functions for performing steps of the UKF algorithm.
   void CalculateWeights();
   Matrix GenerateSigmaPoints() const;
   Matrix CalculateMeanFromSigmas(const Matrix& sigmaPoints) const;
   Matrix CalculateCovarianceFromSigmas(const Matrix& sigmaPoints, const Matrix& mean) const;

   // These functions need to be defined for each implementation of a filter, as they are dependent on the specific application.
   virtual Matrix processEquation(const Matrix& sigma_point, double deltaT, const Matrix& measurement) = 0;
   virtual Matrix measurementEquation(const Matrix& sigma_point, const Matrix& measurementArgs) = 0;

};
