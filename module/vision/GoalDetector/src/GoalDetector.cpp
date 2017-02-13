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
 * Copyright 2013 NUBots <nubots@nubots.net>
 */

#include "GoalDetector.h"

#include "RansacGoalModel.h"

#include "message/vision/ClassifiedImage.h"
#include "message/vision/VisionObjects.h"
#include "message/vision/LookUpTable.h"
#include "message/support/Configuration.h"
#include "message/support/FieldDescription.h"

#include "utility/math/geometry/Quad.h"
#include "utility/math/geometry/Line.h"
#include "utility/math/geometry/Plane.h"

#include "utility/math/ransac/NPartiteRansac.h"
#include "utility/math/vision.h"
#include "utility/math/coordinates.h"
#include "message/input/CameraParameters.h"

#include "GoalMatcherConstants.h"
#include "message/input/Image.h"
#include "SurfDetection.h"
#include "message/localisation/FieldObject.h"
#include "BackgroundImageGen.h"
#include "TestImageGen.h"

namespace module {
namespace vision {

    using message::input::CameraParameters;
    using message::input::Sensors;

    using utility::math::coordinates::cartesianToSpherical;

    using utility::math::geometry::Line;
    using Plane = utility::math::geometry::Plane<3>;
    using utility::math::geometry::Quad;

    using utility::math::ransac::NPartiteRansac;

    using utility::math::vision::widthBasedDistanceToCircle;
    using utility::math::vision::projectCamToPlane;
    using utility::math::vision::imageToScreen;
    using utility::math::vision::getCamFromScreen;
    using utility::math::vision::getParallaxAngle;
    using utility::math::vision::projectCamSpaceToScreen;
    using utility::math::vision::distanceToVerticalObject;

    using message::vision::ObjectClass;
    using message::vision::LookUpTable;
    using message::vision::ClassifiedImage;
    using message::vision::VisionObject;
    using message::vision::Goal;
    using message::input::Image;

    using message::support::Configuration;
    using message::support::FieldDescription;

    // TODO the system is too generous with adding segments above and below the goals and makes them too tall, stop it
    // TODO the system needs to throw out the kinematics and height based measurements when it cannot be sure it saw the tops and bottoms of the goals

    GoalDetector::GoalDetector(std::unique_ptr<NUClear::Environment> environment)
        : Reactor(std::move(environment))
        , MINIMUM_POINTS_FOR_CONSENSUS(0)
        , MAXIMUM_ITERATIONS_PER_FITTING(0)
        , MAXIMUM_FITTED_MODELS(0)
        , CONSENSUS_ERROR_THRESHOLD(0.0)
        , MAXIMUM_ASPECT_RATIO(0.0)
        , MINIMUM_ASPECT_RATIO(0.0)
        , VISUAL_HORIZON_BUFFER(0.0)
        , MAXIMUM_GOAL_HORIZON_NORMAL_ANGLE(0.0)
        , MAXIMUM_ANGLE_BETWEEN_GOALS(0.0)
        , MAXIMUM_VERTICAL_GOAL_PERSPECTIVE_ANGLE(0.0)
        , MEASUREMENT_LIMITS_LEFT(10)
        , MEASUREMENT_LIMITS_RIGHT(10)
        , MEASUREMENT_LIMITS_TOP(10)
        , MEASUREMENT_LIMITS_BASE(10) {


        // Trigger the same function when either update
        on<Configuration, Trigger<CameraParameters>>("GoalDetector.yaml")
        .then([this] (const Configuration& config, const CameraParameters& cam) {

            MINIMUM_POINTS_FOR_CONSENSUS = config["ransac"]["minimum_points_for_consensus"].as<uint>();
            CONSENSUS_ERROR_THRESHOLD = config["ransac"]["consensus_error_threshold"].as<double>();
            MAXIMUM_ITERATIONS_PER_FITTING = config["ransac"]["maximum_iterations_per_fitting"].as<uint>();
            MAXIMUM_FITTED_MODELS = config["ransac"]["maximum_fitted_models"].as<uint>();

            MINIMUM_ASPECT_RATIO = config["aspect_ratio_range"][0].as<double>();
            MAXIMUM_ASPECT_RATIO = config["aspect_ratio_range"][1].as<double>();
            VISUAL_HORIZON_BUFFER = std::max(1, int(cam.focalLengthPixels * tan(config["visual_horizon_buffer"].as<double>())));
            MAXIMUM_GOAL_HORIZON_NORMAL_ANGLE = std::cos(config["minimum_goal_horizon_angle"].as<double>() - M_PI_2);
            MAXIMUM_ANGLE_BETWEEN_GOALS = std::cos(config["maximum_angle_between_goals"].as<double>());
            MAXIMUM_VERTICAL_GOAL_PERSPECTIVE_ANGLE = std::sin(-config["maximum_vertical_goal_perspective_angle"].as<double>());

            MEASUREMENT_LIMITS_LEFT  = config["measurement_limits"]["left"].as<uint>();
            MEASUREMENT_LIMITS_RIGHT = config["measurement_limits"]["right"].as<uint>();
            MEASUREMENT_LIMITS_TOP   = config["measurement_limits"]["top"].as<uint>();
            MEASUREMENT_LIMITS_BASE  = config["measurement_limits"]["base"].as<uint>();

            goalMatcher.loadVocab(VocabFileName);
            printf("\n\n\n");
            //goalMatcher.loadMap(MapFileName);
            //myfile.open("/home/vagrant/NUbots/module/vision/GoalDetector/data/resultTable.txt"); //,std::ios_base::app); This last bit adds text to end of file
            myfile2.open("/home/vagrant/NUbots/module/vision/GoalDetector/data/CutoffTable.txt");
        });

        on<Every<1, std::chrono::seconds>>().then([this] {

            /* This is all stuff to make SurfDetection work in isolation. Will need to be deleted when integrated with GoalMatching.cpp */
            int pixValIt = -1;
            uint8_t Y = 0;
            int Yi = 0;
            NUClear::clock::time_point fake_timestamp = NUClear::clock::now();
            
            
            
            if (imageNum == 1){
                myfile2 << ",,Cosine Score\n" << ",0.2,,0.3,,0.4,,0.5,,0.6,\n" << "Min Inlier,AWAY,HOME,AWAY,HOME,AWAY,HOME,AWAY,HOME,AWAY,HOME\n" << goalMatcher.getValidInliers() << ",";
            }
            
            if (imageNum <= 21) {
                std::vector<uint8_t> test_image(IMAGE_HEIGHT*IMAGE_WIDTH*2,0);
                std::unique_ptr<message::localisation::Self> self = std::make_unique<message::localisation::Self>();
            
                arma::vec2 n({ 0.0, 1.0 });
                double d;

                test_image = backgroundImageGen(imageNum, self, &d);
                auto frame = std::make_unique<ClassifiedImage<ObjectClass>>();// Create an empty ClassifiedImage object
                frame->image = std::make_shared<const Image>((uint)IMAGE_WIDTH,(uint)IMAGE_HEIGHT,fake_timestamp,std::move(test_image));
                frame->horizon = Line(n,d);
                printf("Producing background image #%d\n",imageNum);
                imageNum++;
                emit(std::move(self));
                emit(std::move(frame));

            }
            else if (imageNum <= 54) {
                goalMatcher.setWasInitial(false);
                std::vector<uint8_t> test_image(IMAGE_HEIGHT*IMAGE_WIDTH*2,0);
                std::unique_ptr<message::localisation::Self> self = std::make_unique<message::localisation::Self>();
            
                arma::vec2 n({ 0.0, 1.0 });
                double d;

                test_image = testImageGen(imageNum-21, self, &d,awayImages);
                auto frame = std::make_unique<ClassifiedImage<ObjectClass>>();// Create an empty ClassifiedImage object
                frame->image = std::make_shared<const Image>((uint)IMAGE_WIDTH,(uint)IMAGE_HEIGHT,fake_timestamp,std::move(test_image));
                frame->horizon = Line(n,d);
                printf("Producing Test image #%d\n",imageNum-21);
                imageNum++;
                emit(std::move(self));
                emit(std::move(frame));
            }
            else if (imageNum == 55) {
                printf("\t\tBest\n");
                printf("Image\tMatch Scores\t AWAY/HOME\tInliers   Outliers\n");
                printf("=====================================================================================\n");
                for (int m = 0;m < 33;m++){
                    printf("#%2d\t\t\t",m+1);
                    for (int mi = 0;mi < 8;mi++) {
                        if ((resultTable(m*8+mi,1) < 0.1) && (resultTable(m*8+mi,1) > -0.1)){
                            printf("----\t\t\t -\t\t   --\t\t--\n");
                        }
                        else {
                            printf("%.2f\t\t\t%2.0f\t\t   %3.0f\t\t%3.0f\n",resultTable(m*8+mi,0),resultTable(m*8+mi,1),resultTable(m*8+mi,2),resultTable(m*8+mi,3));
                        }
                        if (mi < 7) printf("\t\t\t");
                    }
                    printf("--------------------------------------------------------------------------------------\n");
                }

                
                //ofstream myfile;
                //myfile.open("/home/Desktop/resultTable.txt");
                /*
                myfile << ",Best,,,\n";
                myfile << "Image,Match Scores,AWAY/HOME,Inliers,Outliers,Ratio,Correct\n";
                for (int m = 0;m < 33;m++){
                    myfile << "#" << m+1 << ",";
                    for (int mi = 0;mi < 8;mi++) {
                        if ((resultTable(m*8+mi,1) < 0.1) && (resultTable(m*8+mi,1) > -0.1)){
                            myfile << "-,-,-,-,,\n";
                        }
                        else {
                            myfile << resultTable(m*8+mi,0) << ",";
                            if (resultTable(m*8+mi,1) > 0.5) myfile << "AWAY,";
                            else myfile << "HOME,";
                            myfile << resultTable(m*8+mi,2) << "," << resultTable(m*8+mi,3);
                            if ((mi == 0) && ((resultTable(m*8+3,1) > 0.1) || (resultTable(m*8+3,1) < -0.1))){
                                myfile << "," << resultTable(m*8,4) << "/" << resultTable(m*8,5) << ",";
                                float awayGoalVotes = resultTable(m*8,4) - resultTable(m*8,5);
                                if ((awayImages == 1)&&(awayGoalVotes < -1.9)) myfile << "WRONG\n"; // WRONG for AWAY dataset
                                else if ((awayImages == 1)&&(awayGoalVotes > 1.9)) myfile << "CORRECT\n";
                                else if ((awayImages == 0)&&(awayGoalVotes < -1.9)) myfile << "CORRECT\n";
                                else if ((awayImages == 0)&&(awayGoalVotes > 1.9)) myfile << "WRONG\n";
                                else myfile << "-\n";
                            }
                            else myfile << ",,\n";
                        }
                        if (mi < 7) myfile << ",";
                    }
                }
                //myfile.close();
                */
                /******* CORRECT/WRONG COUNTER **********/
                int correctCount = 0;
                int wrongCount = 0;
                for (int m = 0;m < 33;m++){
                    float awayGoalVotes = resultTable(m*8,4) - resultTable(m*8,5);
                    printf("%.3f\n",awayGoalVotes);
                    if ((resultTable(m*8,4) > 1.99) || (resultTable(m*8,5) > 1.99)){ //Checks if enough matches are available
                        if(((awayImages == 1)&&(awayGoalVotes > 1.99)) || ((awayImages == 0) && (awayGoalVotes < -1.99))) {
                            correctCount++;
                        }
                        else if(((awayImages == 1)&&(awayGoalVotes < -1.99)) || ((awayImages == 0) && (awayGoalVotes > 1.99))) {
                            wrongCount++;
                        }
                    }
                }

                printf("\nCorrect Count = %d",correctCount);
                int CorrectPercent = (int)(correctCount/imageSetSize*100 + 0.5);
                int WrongPercent = (int)(wrongCount/imageSetSize*100 + 0.5);
                int UnsurePercent = (int)((imageSetSize-correctCount-wrongCount)/imageSetSize*100 + 0.5);
                printf(" (%d)\n\n",CorrectPercent);
                //myfile << correctCount;
                myfile2 << CorrectPercent << "/" << WrongPercent << "/" << UnsurePercent;
                if (CutoffTableColCounter < (CutoffTableColMax-1)){ // Not yet the end of a row
                    myfile2 << ",";
                    CutoffTableColCounter++;
                    imageNum = 22; //Reset imageNum
                    if (awayImages == 0){ //Transition to next cosine cutoff
                        awayImages = 1; //Toggling awayImages
                        goalMatcher.setValidCosineScore(goalMatcher.getValidCosineScore() + 0.1); // Increasing Valid Cosine Score by 0.1
                        printf("ValidCosineScore is now: %.2f\n",goalMatcher.getValidCosineScore());
                    }
                    else {
                        awayImages = 0;
                        printf("ValidCosineScore stays at: %.2f\n",goalMatcher.getValidCosineScore());
                    }
                }
                else if (CutoffTableRowCounter < (CutoffTableRowMax-1)) { //End of row, but not of table
                    imageNum = 22;
                    CutoffTableColCounter = 0;
                    CutoffTableRowCounter++;
                    goalMatcher.setValidInliers(goalMatcher.getValidInliers() + 10);
                    goalMatcher.setValidCosineScore(0.2);
                    myfile2 << endl << goalMatcher.getValidInliers() << ",";
                    awayImages = 1;
                    printf("ValidCosineScore is now: %.2f\n",goalMatcher.getValidCosineScore());
                }
                else { //End of row and table
                    imageNum++;
                }
                resultTable = Eigen::MatrixXd::Zero(33*8,6);
            }
            else if (imageNum == 56){
                printf("\nFINISHED...PRESS CTRL + C\n");
                imageNum++;
                //myfile.close();
                myfile2.close();
            }

            
            
            

            
        });

        on<Trigger<ClassifiedImage<ObjectClass>>
         , With<CameraParameters>
         , With<LookUpTable>
         , With<message::localisation::Self>
         , Single>().then("Goal Detector", [this] (std::shared_ptr<const ClassifiedImage<ObjectClass>> rawImage
                          , const CameraParameters& cam
                          , const LookUpTable& lut
                          , const message::localisation::Self& self) {

            const auto& image = *rawImage;
            // Our segments that may be a part of a goal
            std::vector<RansacGoalModel::GoalSegment> segments;
            auto goals = std::make_unique<std::vector<Goal>>();

            // Get our goal segments
            auto hSegments = image.horizontalSegments.equal_range(ObjectClass::GOAL);
            for(auto it = hSegments.first; it != hSegments.second; ++it) {

                // We throw out points if they are:
                // Less the full quality (subsampled)
                // Do not have a transition on the other side
                if(it->second.subsample == 1 && it->second.previous && it->second.next) {
                    segments.push_back({ { double(it->second.start[0]), double(it->second.start[1]) }, { double(it->second.end[0]), double(it->second.end[1]) } });
                }
            }

            // Partition our segments so that they are split between above and below the horizon
            auto split = std::partition(std::begin(segments), std::end(segments), [image] (const RansacGoalModel::GoalSegment& segment) {
                // Is the midpoint above or below the horizon?
                return image.horizon.distanceToPoint(segment.left + segment.right / 2) > 0;
            });

            // Make an array of our partitions
            std::array<std::vector<RansacGoalModel::GoalSegment>::iterator, RansacGoalModel::REQUIRED_POINTS + 1> points = {
                segments.begin(),
                split,
                segments.end()
            };

            // Ransac for goals
            auto models = NPartiteRansac<RansacGoalModel>::fitModels(points
                                                                   , MINIMUM_POINTS_FOR_CONSENSUS
                                                                   , MAXIMUM_ITERATIONS_PER_FITTING
                                                                   , MAXIMUM_FITTED_MODELS
                                                                   , CONSENSUS_ERROR_THRESHOLD);

            // Look at our results
            for (auto& result : models) {

                // Get our left, right and midlines
                Line& left = result.model.left;
                Line& right = result.model.right;
                Line mid;

                // Normals in same direction
                if(arma::dot(left.normal, right.normal) > 0) {
                    mid.normal = arma::normalise(right.normal + left.normal);
                    mid.distance = ((right.distance / arma::dot(right.normal, mid.normal)) + (left.distance / arma::dot(left.normal, mid.normal))) * 0.5;
                }
                // Normals opposed
                else {
                    mid.normal = arma::normalise(right.normal - left.normal);
                    mid.distance = ((right.distance / arma::dot(right.normal, mid.normal)) - (left.distance / arma::dot(left.normal, mid.normal))) * 0.5;
                }

                // Find a point that should work to start searching down
                arma::vec2 midpoint({0, 0});
                int i = 0;
                for(auto& m : result) {
                    midpoint += m.left;
                    midpoint += m.right;
                    i += 2;
                }
                midpoint /= i;

                // Work out which direction to go
                arma::vec2 direction = mid.tangent();
                direction *= direction[1] > 0 ? 1 : -1;
                double theta = std::acos(direction[0]);
                if (std::abs(theta) < M_PI_4) {
                    direction[0] = 1;
                    direction[1] = -std::tan(theta);
                }
                else {
                    direction[0] = std::tan(M_PI_2 - std::abs(theta));
                    direction[1] = 1;
                }

                // Classify until we reach green
                arma::vec2 basePoint({0, 0});
                int notWhiteLen = 0;
                for(arma::vec2 point = mid.orthogonalProjection(midpoint);
                    (point[0] < image.dimensions[0]) && (point[0] > 0) && (point[1] < image.dimensions[1]);
                    point += direction) {

                    char c = lut(image.image->operator()(int(point[0]), int(point[1])));

                    if(c != 'y') {
                        ++notWhiteLen;
                        if(notWhiteLen > 4) {
                            basePoint = point;
                            break;
                        }
                    }
                    else if(c == 'g') {
                        basePoint = point;
                        break;
                    }
                    else if(c == 'y') {
                        notWhiteLen = 0;
                    }
                }

                arma::running_stat<double> stat;

                // Look through our segments to find endpoints
                for(auto& point : result) {
                    // Project left and right onto midpoint keep top and bottom
                    stat(mid.tangentialDistanceToPoint(point.left));
                    stat(mid.tangentialDistanceToPoint(point.right));
                }

                // Get our endpoints from the min and max points on the line
                arma::vec2 midP1 = mid.pointFromTangentialDistance(stat.min());
                arma::vec2 midP2 = mid.orthogonalProjection(basePoint);

                // Project those points outward onto the quad
                arma::vec2 p1 = midP1 - left.distanceToPoint(midP1) * arma::dot(left.normal, mid.normal) * mid.normal;
                arma::vec2 p2 = midP2 - left.distanceToPoint(midP2) * arma::dot(left.normal, mid.normal) * mid.normal;
                arma::vec2 p3 = midP1 - right.distanceToPoint(midP1) * arma::dot(right.normal, mid.normal) * mid.normal;
                arma::vec2 p4 = midP2 - right.distanceToPoint(midP2) * arma::dot(right.normal, mid.normal) * mid.normal;

                // Make a quad
                Goal g;
                g.side = Goal::Side::UNKNOWN;

                // Seperate tl and bl
                arma::vec2 tl = p1[1] > p2[1] ? p2 : p1;
                arma::vec2 bl = p1[1] > p2[1] ? p1 : p2;
                arma::vec2 tr = p3[1] > p4[1] ? p4 : p3;
                arma::vec2 br = p3[1] > p4[1] ? p3 : p4;

                g.quad.set(bl, tl, tr, br);

                g.sensors = image.sensors;
                g.classifiedImage = rawImage;
                goals->push_back(std::move(g));
            }

            // Throwout invalid quads
            for(auto it = goals->begin(); it != goals->end();) {

                auto& quad = it->quad;
                arma::vec2 lhs = arma::normalise(quad.getTopLeft() - quad.getBottomLeft());
                arma::vec2 rhs = arma::normalise(quad.getTopRight() - quad.getBottomRight());

                // Check if we are within the aspect ratio range
                bool valid = quad.aspectRatio() > MINIMUM_ASPECT_RATIO
                          && quad.aspectRatio() < MAXIMUM_ASPECT_RATIO
                // Check if we are close enough to the visual horizon
                          && (image.visualHorizonAtPoint(quad.getBottomLeft()[0]) < quad.getBottomLeft()[1] + VISUAL_HORIZON_BUFFER
                              || image.visualHorizonAtPoint(quad.getBottomRight()[0]) < quad.getBottomRight()[1] + VISUAL_HORIZON_BUFFER)
                // Check we finish above the kinematics horizon or or kinematics horizon is off the screen
                          && (image.horizon.y(quad.getTopLeft()[0]) > quad.getTopLeft()[1] || image.horizon.y(quad.getTopLeft()[0]) < 0)
                          && (image.horizon.y(quad.getTopRight()[0]) > quad.getTopRight()[1] || image.horizon.y(quad.getTopRight()[0]) < 0)
                // Check that our two goal lines are perpendicular with the horizon must use greater than rather then less than because of the cos
                          && std::abs(arma::dot(lhs, image.horizon.normal)) > MAXIMUM_GOAL_HORIZON_NORMAL_ANGLE
                          && std::abs(arma::dot(rhs, image.horizon.normal)) > MAXIMUM_GOAL_HORIZON_NORMAL_ANGLE
                // Check that our two goal lines are approximatly parallel
                          && std::abs(arma::dot(lhs, rhs)) > MAXIMUM_ANGLE_BETWEEN_GOALS;
                // Check that our goals don't form too much of an upward cup (not really possible for us)
                          //&& lhs.at(0) * rhs.at(1) - lhs.at(1) * rhs.at(0) > MAXIMUM_VERTICAL_GOAL_PERSPECTIVE_ANGLE;


                if(!valid) {
                    it = goals->erase(it);
                }
                else {
                    ++it;
                }
            }

            // Merge close goals
            for (auto a = goals->begin(); a != goals->end(); ++a) {
                for (auto b = std::next(a); b != goals->end();) {

                    if (a->quad.overlapsHorizontally(b->quad)) {
                        // Get outer lines.
                        arma::vec2 tl;
                        arma::vec2 tr;
                        arma::vec2 bl;
                        arma::vec2 br;

                        tl = { std::min(a->quad.getTopLeft()[0],     b->quad.getTopLeft()[0]),     std::min(a->quad.getTopLeft()[1],     b->quad.getTopLeft()[1]) };
                        tr = { std::max(a->quad.getTopRight()[0],    b->quad.getTopRight()[0]),    std::min(a->quad.getTopRight()[1],    b->quad.getTopRight()[1]) };
                        bl = { std::min(a->quad.getBottomLeft()[0],  b->quad.getBottomLeft()[0]),  std::max(a->quad.getBottomLeft()[1],  b->quad.getBottomLeft()[1]) };
                        br = { std::max(a->quad.getBottomRight()[0], b->quad.getBottomRight()[0]), std::max(a->quad.getBottomRight()[1], b->quad.getBottomRight()[1]) };

                        // Replace original two quads with the new one.
                        a->quad.set(bl, tl, tr, br);
                        b = goals->erase(b);
                    }
                    else {
                        b++;
                    }
                }
            }

            // Store our measurements
            for(auto it = goals->begin(); it != goals->end(); ++it) {

                // Get the quad points in screen coords
                arma::vec2 tl = imageToScreen(it->quad.getTopLeft(), image.dimensions);
                arma::vec2 tr = imageToScreen(it->quad.getTopRight(), image.dimensions);
                arma::vec2 bl = imageToScreen(it->quad.getBottomLeft(), image.dimensions);
                arma::vec2 br = imageToScreen(it->quad.getBottomRight(), image.dimensions);
                arma::vec2 screenGoalCentre = (tl + tr + bl + br) * 0.25;

                // Get vectors for TL TR BL BR;
                arma::vec3 ctl = getCamFromScreen(tl, cam.focalLengthPixels);
                arma::vec3 ctr = getCamFromScreen(tr, cam.focalLengthPixels);;
                arma::vec3 cbl = getCamFromScreen(bl, cam.focalLengthPixels);;
                arma::vec3 cbr = getCamFromScreen(br, cam.focalLengthPixels);;

                // Get our four normals for each edge
                // BL TL cross product gives left side
                arma::vec3 left   = arma::normalise(arma::cross(cbl, ctl));
                it->measurements.push_back(std::make_pair(Goal::MeasurementType::LEFT_NORMAL, left));

                // TR BL cross product gives right side
                arma::vec3 right  = arma::normalise(arma::cross(ctr, cbl));
                it->measurements.push_back(std::make_pair(Goal::MeasurementType::RIGHT_NORMAL, right));

                // Check that the points are not too close to the edges of the screen
                if(                         std::min(cbr[0], cbl[0]) > MEASUREMENT_LIMITS_LEFT
                &&                          std::min(cbr[1], cbl[1]) > MEASUREMENT_LIMITS_TOP
                && cam.imageSizePixels[0] - std::max(cbr[0], cbl[0]) < MEASUREMENT_LIMITS_TOP
                && cam.imageSizePixels[1] - std::max(cbr[1], cbl[1]) < MEASUREMENT_LIMITS_BASE) {

                    // BR BL cross product gives the bottom side
                    arma::vec3 bottom = arma::normalise(arma::cross(cbr, cbl));
                    it->measurements.push_back(std::make_pair(Goal::MeasurementType::BASE_NORMAL, right));
                }

                // Check that the points are not too close to the edges of the screen
                if(                         std::min(ctr[0], ctl[0]) > MEASUREMENT_LIMITS_LEFT
                &&                          std::min(ctr[1], ctl[1]) > MEASUREMENT_LIMITS_TOP
                && cam.imageSizePixels[0] - std::max(ctr[0], ctl[0]) < MEASUREMENT_LIMITS_TOP
                && cam.imageSizePixels[1] - std::max(ctr[1], ctl[1]) < MEASUREMENT_LIMITS_BASE) {

                    // TL TR cross product gives the top side
                    arma::vec3 top    = arma::normalise(arma::cross(ctl, ctr));
                    it->measurements.push_back(std::make_pair(Goal::MeasurementType::TOP_NORMAL, right));
                }

                // Attach our sensors
                it->sensors = image.sensors;

                // Angular positions from the camera
                it->screenAngular = arma::atan(cam.pixelsToTanThetaFactor % screenGoalCentre);
                arma::vec2 brAngular = arma::atan(cam.pixelsToTanThetaFactor % br);
                arma::vec2 trAngular = arma::atan(cam.pixelsToTanThetaFactor % tr);
                arma::vec2 blAngular = arma::atan(cam.pixelsToTanThetaFactor % bl);
                arma::vec2 tlAngular = arma::atan(cam.pixelsToTanThetaFactor % tl);
                Quad angularQuad(blAngular,tlAngular,trAngular,brAngular);
                it->angularSize = angularQuad.getSize();
            }


            // Assign leftness and rightness to goals
            if (goals->size() == 2) {
                if (goals->at(0).quad.getCentre()(0) < goals->at(1).quad.getCentre()(0)) {
                    goals->at(0).side = Goal::Side::LEFT;
                    goals->at(1).side = Goal::Side::RIGHT;
                } else {
                    goals->at(0).side = Goal::Side::RIGHT;
                    goals->at(1).side = Goal::Side::LEFT;
                }
            }

            Image::Pixel tempPixel;
            uint w,h;
            w = rawImage->image->width;
            h = rawImage->image->height;
            int right_horizon = rawImage->horizon.y(w-1);
            //printf("%dx%d\n\n",h,w);
            /*
            for (uint m = 0;m<h ;++m){
                for (uint n = 0;n<w;++n){
                    tempPixel = (*rawImage->image)(n,m);
                    printf("%3d ",tempPixel.y);
                }
                if (m == right_horizon){
                    printf("--");
                }
                printf("\n");
            }
            printf("\n");
            */
            /********************************************************************
             * SURF Landmark Extraction - needs Robot Detection                *
             *******************************************************************/
            SurfDetection surf_obj(rawImage);
            surf_obj.loadVocab(VocabFileName);
            surf_obj.findLandmarks(landmarks,landmark_tf,landmark_pixLoc);

            /********************************************************************
             * SURF Landmarks used to classify the goal area                   *
             *******************************************************************/
            
            float awayGoalProb = 0.5;
            if (imageNum <= 22){
                goalMatcher.process(rawImage, landmarks, landmark_tf, landmark_pixLoc, self,awayGoalProb,MapFileName,&resultTable);
            }
            else{
                printf("resultTable(%d:%d,:)\n",(imageNum-23)*8,(imageNum-23)*8 + 7);
                Eigen::MatrixXd temp = resultTable.middleRows((imageNum-23)*8,8);
                goalMatcher.process(rawImage, landmarks, landmark_tf, landmark_pixLoc, self,awayGoalProb,MapFileName,&temp);
                resultTable.middleRows((imageNum-23)*8,8) = temp;
            }
            printf("awayGoalProb = %.2f\n",awayGoalProb);
            printf("imageNum = %d\n",imageNum);
            printf("goal size = %d\n\n\n",goals->size());
            for (int j=0;j<goals->size();j++){
                goals->at(j).awayGoalProb = awayGoalProb;
            }

            emit(std::move(goals));
            
        });
    }

}
}

