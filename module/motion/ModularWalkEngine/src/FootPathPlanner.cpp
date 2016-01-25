/*----------------------------------------------DOCUMENT HEADER----------------------------------------------*/
/*===========================================================================================================*/
/*
 * This file is part of ModularWalkEngine.
 *
 * ModularWalkEngine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ModularWalkEngine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ModularWalkEngine.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright 2013 NUBots <nubots@nubots.net>
 */
/*===========================================================================================================*/
/*----------------------------------------CONSTANTS AND DEFINITIONS------------------------------------------*/
/*===========================================================================================================*/
//      INCLUDE(S)
/*===========================================================================================================*/
#include "ModularWalkEngine.h"

#include "utility/motion/RobotModels.h"
#include "utility/nubugger/NUhelpers.h"
/*===========================================================================================================*/
//      NAMESPACE(S)
/*===========================================================================================================*/
namespace module 
{
namespace motion
{
    /*=======================================================================================================*/
    //      UTILIZATION REFERENCE(S)
    /*=======================================================================================================*/
    using message::input::LimbID;
    using utility::motion::kinematics::DarwinModel;
    using utility::math::matrix::Transform2D;
    using utility::nubugger::graph;
    /*=======================================================================================================*/
    //      NAME: footPhase
    /*=======================================================================================================*/
    /*
     *      @input  : <TODO: INSERT DESCRIPTION>
     *      @output : <TODO: INSERT DESCRIPTION>
     *      @pre-condition  : <TODO: INSERT DESCRIPTION>
     *      @post-condition : <TODO: INSERT DESCRIPTION>
    */
    arma::vec3 ModularWalkEngine::footPhase(double phase, double phase1Single, double phase2Single) 
    {
        // Computes relative x,z motion of foot during single support phase
        // phSingle = 0: x=0, z=0, phSingle = 1: x=1,z=0
        double phaseSingle = std::min(std::max(phase - phase1Single, 0.0) / (phase2Single - phase1Single), 1.0);
        double phaseSingleSkew = std::pow(phaseSingle, 0.8) - 0.17 * phaseSingle * (1 - phaseSingle);
        double xf = 0.5 * (1 - std::cos(M_PI * phaseSingleSkew));
        double zf = 0.5 * (1 - std::cos(2 * M_PI * phaseSingleSkew));

        return {xf, phaseSingle, zf};
    }
    /*=======================================================================================================*/
    //      NAME: updateFootPosition
    /*=======================================================================================================*/
    /*
     *      @input  : <TODO: INSERT DESCRIPTION>
     *      @output : <TODO: INSERT DESCRIPTION>
     *      @pre-condition  : <TODO: INSERT DESCRIPTION>
     *      @post-condition : <TODO: INSERT DESCRIPTION>
    */
    arma::vec2 ModularWalkEngine::updateFootPosition(double phase, const Sensors& sensors) 
    {
        //Instantiate unitless phases for x(=0), y(=1) and z(=2) foot motion...
        arma::vec3 footPhases = footPhase(phase, phase1Single, phase2Single);

        //Lift foot by amount depending on walk speed
        auto& limit = (velocityCurrent.x() > velocityHigh ? accelerationLimitsHigh : accelerationLimits); // TODO: use a function instead
        float speed = std::min(1.0, std::max(std::abs(velocityCurrent.x() / limit[0]), std::abs(velocityCurrent.y() / limit[1])));
        float scale = (step_height_fast_fraction - step_height_slow_fraction) * speed + step_height_slow_fraction;
        footPhases[2] *= scale;


        // don't lift foot at initial step, TODO: review
        if (initialStep > 0) 
        {
            footPhases[2] = 0;
        }

        //Interpolate Transform2D from start to destination - deals with flat resolved movement in (x,y) coordinates
        if (swingLeg == LimbID::RIGHT_LEG) 
        {
            //Vector field function??
            uRightFoot = uRightFootSource.interpolate(footPhases[0], uRightFootDestination);
        }
        else
        {
            //Vector field function??
            uLeftFoot  = uLeftFootSource.interpolate(footPhases[0], uLeftFootDestination);
        }
        
        //Translates foot motion into z dimension for stepping in three-dimensional space...
        Transform3D leftFootLocal = uLeftFoot;
        Transform3D rightFootLocal = uRightFoot;

        //Lift swing leg - manipulate(update) z component of foot position to action movement with a varying altitude locus...
        if (swingLeg == LimbID::RIGHT_LEG) 
        {
            rightFootLocal = rightFootLocal.translateZ(stepHeight * footPhases[2]);
        }
        else
        {
            leftFootLocal  = leftFootLocal.translateZ(stepHeight * footPhases[2]);
        }      

        return {leftFootLocal, rightFootLocal};
    }
    /*=======================================================================================================*/
    //      NAME: updateLowerBody
    /*=======================================================================================================*/
    /*
     *      @input  : <TODO: INSERT DESCRIPTION>
     *      @output : <TODO: INSERT DESCRIPTION>
     *      @pre-condition  : <TODO: INSERT DESCRIPTION>
     *      @post-condition : <TODO: INSERT DESCRIPTION>
    */
    std::unique_ptr<std::vector<ServoCommand>> ModularWalkEngine::updateLowerBody(double phase, const Sensors& sensors) 
    {
        //Interpret robot's zero point reference from torso for positional transformation into relative space...
        uTorsoLocal = zmpTorsoCompensation(phase, zmpCoefficients, zmpParams, stepTime, zmpTime, phase1Single, phase2Single, uSupport, uLeftFootDestination, uLeftFootSource, uRightFootDestination, uRightFootSource);

        //Interpret robot's world position from torso as local positional reference...
        Transform2D uTorsoWorld = uTorsoLocal.localToWorld({-DarwinModel::Leg::HIP_OFFSET_X, 0, 0});

        //Collect attributed metrics that describe the robot's spatial orientation in environmental space...
        Transform3D torsoWorldMetrics = arma::vec6({uTorsoWorld.x(), uTorsoWorld.y(), bodyHeight, 0, bodyTilt, uTorsoWorld.angle()});

        //DEBUGGING: Emit relative torsoWorldMetrics position with respect to world model... 
        if (emitLocalisation) 
        {
            localise(uTorsoWorld);
        }

        // Transform feet targets to be relative to the robot torso...
        Transform3D leftFootTorso = leftFoot.worldToLocal(torsoWorldMetrics);
        Transform3D rightFootTorso = rightFoot.worldToLocal(torsoWorldMetrics);

        //Compute compensation moment to apply hip roll and support foot balance...
        hipCompensation(footPhases, swingLeg, rightFootTorso, leftFootTorso);

        //DEBUGGING: Emit relative feet position with respect to robot torso model... 
        if (emitFootPosition)
        {
            emit(graph("Foot phase motion", phase));
            emit(graph("Right foot position", rightFootTorso.translation()));
            emit(graph("Left  foot position",  leftFootTorso.translation()));
        }

        auto joints = calculateLegJointsTeamDarwin<DarwinModel>(leftFootTorso, rightFootTorso);

        return (motionLegs(joints)); 
    }
}  // motion
}  // modules