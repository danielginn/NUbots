/*
 * This file is part of LUTClassifier.
 *
 * LUTClassifier is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LUTClassifier is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LUTClassifier.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright 2013 NUBots <nubots@nubots.net>
 */

#include "LUTClassifier.h"
#include "ScanLines.h"
#include "GreenHorizon.h"

namespace modules {
    namespace vision {

        using messages::input::Image;
        using messages::support::Configuration;
        
        LUTClassifier::LUTClassifier(std::unique_ptr<NUClear::Environment> environment) : Reactor(std::move(environment)),greenHorizon(),scanLines() { 
			current_LUT_index = 0;
			

            on<Trigger<Configuration<VisionConstants>>>([this](const Configuration<VisionConstants>& constants) {
           		//HACK FOR RC2013
				// WHITE_SIDE_IS_BLUE = constants.config["WHITE_SIDE_IS_BLUE"];
				// NON_WHITE_SIDE_CHECK = constants.config["NON_WHITE_SIDE_CHECK"];
				// UPPER_WHITE_THRESHOLD = constants.config["UPPER_WHITE_THRESHOLD"];
				// LOWER_WHITE_THRESHOLD = constants.config["LOWER_WHITE_THRESHOLD"];
				// //! Distortion Correction
				// DO_RADIAL_CORRECTION = constants.config["DO_RADIAL_CORRECTION"];
				// RADIAL_CORRECTION_COEFFICIENT = constants.config["RADIAL_CORRECTION_COEFFICIENT"];
				// //! Goal filtering constants
				// THROWOUT_ON_ABOVE_KIN_HOR_GOALS = constants.config["THROWOUT_ON_ABOVE_KIN_HOR_GOALS"];
				// THROWOUT_ON_DISTANCE_METHOD_DISCREPENCY_GOALS = constants.config["THROWOUT_ON_DISTANCE_METHOD_DISCREPENCY_GOALS"];
				// MAX_DISTANCE_METHOD_DISCREPENCY_GOALS = constants.config["MAX_DISTANCE_METHOD_DISCREPENCY_GOALS"];
				// THROWOUT_DISTANT_GOALS = constants.config["THROWOUT_DISTANT_GOALS"];
				// MAX_GOAL_DISTANCE = constants.config["MAX_GOAL_DISTANCE"];
				// THROWOUT_INSIGNIFICANT_GOALS = constants.config["THROWOUT_INSIGNIFICANT_GOALS"];
				// MIN_TRANSITIONS_FOR_SIGNIFICANCE_GOALS = constants.config["MIN_TRANSITIONS_FOR_SIGNIFICANCE_GOALS"];
				// THROWOUT_NARROW_GOALS = constants.config["THROWOUT_NARROW_GOALS"];
				// MIN_GOAL_WIDTH = constants.config["MIN_GOAL_WIDTH"];
				// THROWOUT_SHORT_GOALS = constants.config["THROWOUT_SHORT_GOALS"];
				// MIN_GOAL_HEIGHT = constants.config["MIN_GOAL_HEIGHT"];
				// GOAL_HEIGHT_TO_WIDTH_RATIO_MIN = constants.config["GOAL_HEIGHT_TO_WIDTH_RATIO_MIN"];
				// GOAL_MAX_OBJECTS = constants.config["GOAL_MAX_OBJECTS"];
				// GOAL_BINS = constants.config["GOAL_BINS"];
				// GOAL_MIN_THRESHOLD = constants.config["GOAL_MIN_THRESHOLD"];
				// GOAL_SDEV_THRESHOLD = constants.config["GOAL_SDEV_THRESHOLD"];
				// GOAL_RANSAC_MATCHING_TOLERANCE = constants.config["GOAL_RANSAC_MATCHING_TOLERANCE"];
				// THROWOUT_ON_ABOVE_KIN_HOR_BALL = constants.config["THROWOUT_ON_ABOVE_KIN_HOR_BALL"];
				// THROWOUT_ON_DISTANCE_METHOD_DISCREPENCY_BALL = constants.config["THROWOUT_ON_DISTANCE_METHOD_DISCREPENCY_BALL"];
				// MAX_DISTANCE_METHOD_DISCREPENCY_BALL = constants.config["MAX_DISTANCE_METHOD_DISCREPENCY_BALL"];
				// THROWOUT_SMALL_BALLS = constants.config["THROWOUT_SMALL_BALLS"];
				// MIN_BALL_DIAMETER_PIXELS = constants.config["MIN_BALL_DIAMETER_PIXELS"];
				// THROWOUT_INSIGNIFICANT_BALLS = constants.config["THROWOUT_INSIGNIFICANT_BALLS"];
				// MIN_TRANSITIONS_FOR_SIGNIFICANCE_BALL = constants.config["MIN_TRANSITIONS_FOR_SIGNIFICANCE_BALL"];
				// THROWOUT_DISTANT_BALLS = constants.config["THROWOUT_DISTANT_BALLS"];
				// MAX_BALL_DISTANCE = constants.config["MAX_BALL_DISTANCE"];
				// //! Distance calculation options
				// D2P_INCLUDE_BODY_PITCH = constants.config["D2P_INCLUDE_BODY_PITCH"];
				// BALL_DISTANCE_POSITION_BOTTOM = constants.config["BALL_DISTANCE_POSITION_BOTTOM"];
				// //! Distance method options
				// BALL_DISTANCE_METHOD = constants.config["BALL_DISTANCE_METHOD"];
				// GOAL_DISTANCE_METHOD = constants.config["GOAL_DISTANCE_METHOD"];
				// //DistanceMethod VisionConstants::BEACON_DISTANCE_METHOD;
				// LINE_METHOD = constants.config["LINE_METHOD"];
				// //! Field-object detection constants
				// BALL_EDGE_THRESHOLD = constants.config["BALL_EDGE_THRESHOLD"];
				// BALL_ORANGE_TOLERANCE = constants.config["BALL_ORANGE_TOLERANCE"];
				// BALL_MIN_PERCENT_ORANGE = constants.config["BALL_MIN_PERCENT_ORANGE"];
				// GOAL_MIN_PERCENT_YELLOW = constants.config["GOAL_MIN_PERCENT_YELLOW"];
				// GOAL_MIN_PERCENT_BLUE = constants.config["GOAL_MIN_PERCENT_BLUE"];
				// //float VisionConstants::BEACON_MIN_PERCENT_YELLOW;
				// //float VisionConstants::BEACON_MIN_PERCENT_BLUE;
				// MIN_GOAL_SEPARATION = constants.config["MIN_GOAL_SEPARATION"];
				// //! Obstacle detection constants
				// MIN_DISTANCE_FROM_HORIZON = constants.config["MIN_DISTANCE_FROM_HORIZON"];
				// MIN_CONSECUTIVE_POINTS = constants.config["MIN_CONSECUTIVE_POINTS"];
				// //! Field dimension constants
				// GOAL_WIDTH = constants.config["GOAL_WIDTH"];
				// GOAL_HEIGHT = constants.config["GOAL_HEIGHT"];
				// DISTANCE_BETWEEN_POSTS = constants.config["DISTANCE_BETWEEN_POSTS"];
				// BALL_WIDTH = constants.config["BALL_WIDTH"];
				// CENTRE_CIRCLE_RADIUS = constants.config["CENTRE_CIRCLE_RADIUS"];
				// //float VisionConstants::BEACON_WIDTH;
				// //! ScanLine options
				// //! Split and Merge constants
				// SAM_MAX_LINES = constants.config["SAM_MAX_LINES"];
				// SAM_SPLIT_DISTANCE = constants.config["SAM_SPLIT_DISTANCE"];
				// SAM_MIN_POINTS_OVER = constants.config["SAM_MIN_POINTS_OVER"];
				// SAM_MIN_POINTS_TO_LINE = constants.config["SAM_MIN_POINTS_TO_LINE"];
				// SAM_MAX_ANGLE_DIFF_TO_MERGE = constants.config["SAM_MAX_ANGLE_DIFF_TO_MERGE"];
				// SAM_MAX_DISTANCE_TO_MERGE = constants.config["SAM_MAX_DISTANCE_TO_MERGE"];
				// SAM_MIN_POINTS_TO_LINE_FINAL = constants.config["SAM_MIN_POINTS_TO_LINE_FINAL"];
				// SAM_MIN_LINE_R2_FIT = constants.config["SAM_MIN_LINE_R2_FIT"];
				// SAM_MAX_LINE_MSD = constants.config["SAM_MAX_LINE_MSD"];
				// SAM_CLEAR_SMALL = constants.config["SAM_CLEAR_SMALL"];
				// SAM_CLEAR_DIRTY = constants.config["SAM_CLEAR_DIRTY"];
				// //! RANSAC constants
				// RANSAC_MAX_ANGLE_DIFF_TO_MERGE = constants.config["RANSAC_MAX_ANGLE_DIFF_TO_MERGE"]; 
				// RANSAC_MAX_DISTANCE_TO_MERGE = constants.config["RANSAC_MAX_DISTANCE_TO_MERGE"]; 
            });

			//Load LUTs
			on<Trigger<Configuration<LUTLocations>>>([this](const Configuration<LUTLocations>& locations) {
				for(std::string& location : locations.config) {
					LookUpTable LUT;
					bool loaded = LUT.loadLUTFromFile(location);

					if(loaded) {
						LUTs.push_back(LUT);
					}

					else {
						log<NUClear::ERROR>("LUT ", location, " has not loaded successfully." );
					}
				}
			}

			//Load in greenhorizon parameters
			on<Trigger<Configuration<GreenHorizonConfig>>>([this](const Configuration<GreenHorizonConfig>& constants) {
				greenHorizon.setParameters(constants.config["GREEN_HORIZON_SCAN_SPACING"],
											constants.config["GREEN_HORIZON_MIN_GREEN_PIXELS"],
											constants.config["GREEN_HORIZON_UPPER_THRESHOLD_MULT"]);
			}

			//Load in scanline parameters
			on<Trigger<Configuration<ScanLinesConfig>>>([this](const Configuration<GreenHorizonConfig>& constants) {
				scanLines.setParameters(constants.config["HORIZONTAL_SCANLINE_SPACING"],
										 constants.config["VERTICAL_SCANLINE_SPACING"]);
			}

            on<Trigger<Image>>([this](const Image& image) {
            	std::vector<arma::vec> green_horizon_points = greenHorizon.calculateGreenHorizon(image, LUTs[current_LUT_index]);
            	std::vector<int> scan_lines = scanLines.generateScanLines(image, green_horizon_points, LUTs[current_LUT_index]);
            });
        }

    }  // vision
}  // modules