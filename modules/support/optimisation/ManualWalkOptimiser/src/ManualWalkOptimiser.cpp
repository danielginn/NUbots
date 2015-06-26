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
 * Copyright 2015 NUbots <nubots@nubots.net>
 */

#include <cstdio>

#include "messages/support/Configuration.h"
#include "messages/support/optimisation/WalkFitnessDelta.h"

#include "utility/math/optimisation/PGAoptimiser.h"

#include "ManualWalkOptimiser.h"


namespace modules {
namespace support {
namespace optimisation {

    using messages::support::Configuration;
    using messages::support::SaveConfiguration;
    using messages::support::optimisation::WalkFitnessDelta;

    struct WalkEngineConfig {
        static constexpr const char* CONFIGURATION_PATH = "WalkEngine.yaml";
    };
    

    ManualWalkOptimiser::ManualWalkOptimiser(std::unique_ptr<NUClear::Environment> environment)
    : Reactor(std::move(environment)), 
      numParameters(0), fitnessSum(0.0), numSamples(0), currentSample(-1), runCount(0), preserveOutputs(false) { 

        on<Trigger<Configuration<ManualWalkOptimiser>>>([this](const Configuration<ManualWalkOptimiser>& config) {
            preserveOutputs        = config["preserve_outputs"].as<bool>();
            numSamples             = config["number_of_samples"].as<int>();
            numParameters          = config["number_of_parameters"].as<int>();
            getUpCancelThreshold   = config["getup_cancel_trial_threshold"].as<uint>();
            configWaitMilliseconds = config["configuration_wait_milliseconds"].as<uint>();

            // Extract the sigmas from the config file.
            weights.set_size(numParameters);
            weights[0] = config["parameters_and_sigmas"]["stance"]["body_tilt"].as<double>();
            weights[1] = config["parameters_and_sigmas"]["walk_cycle"]["zmp_time"].as<double>();
            weights[2] = config["parameters_and_sigmas"]["walk_cycle"]["step_time"].as<double>();
            weights[3] = config["parameters_and_sigmas"]["walk_cycle"]["single_support_phase"]["start"].as<double>();
            weights[4] = config["parameters_and_sigmas"]["walk_cycle"]["single_support_phase"]["end"].as<double>();
            weights[5] = config["parameters_and_sigmas"]["walk_cycle"]["step"]["height"].as<double>();
        });

        on<Trigger<Every<20, std::chrono::seconds>>, With<Configuration<WalkEngineConfig>>>
            ([this](const time_t& , const Configuration<WalkEngineConfig>& config) {

            if (currentSample == -1) {
                // Get the initial parameters from the original WalkEngine config.
                samples.set_size(numParameters, numSamples);
                samples.zeros();
                samples(0, 0) = walkEngineConfig["stance"]["body_tilt"].as<double>();
                samples(0, 1) = walkEngineConfig["walk_cycle"]["zmp_time"].as<double>();
                samples(0, 2) = walkEngineConfig["walk_cycle"]["step_time"].as<double>();
                samples(0, 3) = walkEngineConfig["walk_cycle"]["single_support_phase"]["start"].as<double>();
                samples(0, 4) = walkEngineConfig["walk_cycle"]["single_support_phase"]["end"].as<double>();
                samples(0, 5) = walkEngineConfig["walk_cycle"]["step"]["height"].as<double>();

                currentSample = numSamples;
                fitnessScores.zeros(numSamples);

                // Make a copy of the WalkEngine config so we can modify it later.
                walkEngineConfig = YAML::Clone(config.config);
            }
            
            if (currentSample == numSamples) {
                // Get the current best estimate.
                auto bestEstimate = utility::math::optimisation::PGA::updateEstimate(samples, fitnessScores);

                // Get the next set of samples.
                samples = utility::math::optimisation::PGA::getSamples(bestEstimate, weights, numSamples);

                // Start at the first sample.
                currentSample = 0;
            } 

            else {
                // Pair fitness score with sample.
                fitnessScores[currentSample] = fitnessSum;

                // Progress to next sample.
                currentSample++;
            }

            // Clear fitness score for next run.
            fitnessSum = 0.0;

            // Generate config file with next sample.
            walkEngineConfig["stance"]["body_tilt"]                         = samples(currentSample, 0);
            walkEngineConfig["walk_cycle"]["zmp_time"]                      = samples(currentSample, 1);
            walkEngineConfig["walk_cycle"]["step_time"]                     = samples(currentSample, 2);
            walkEngineConfig["walk_cycle"]["single_support_phase"]["start"] = samples(currentSample, 3);
            walkEngineConfig["walk_cycle"]["single_support_phase"]["end"]   = samples(currentSample, 4);
            walkEngineConfig["walk_cycle"]["step"]["height"]                = samples(currentSample, 5);

            // Save config file.
            SaveConfiguration out;
            out.path   = "WalkEngine.yaml";
            out.config = walkEngineConfig;
            emit(std::move(std::make_unique<SaveConfiguration>(out)));

            // Preserve all outputs.
            if (preserveOutputs)
            {
                SaveConfiguration preservation;
                preservation.path   = "WalkEngine_run" + std::to_string(++runCount) + ".yaml";
                preservation.config = walkEngineConfig;
                emit(std::move(std::make_unique<SaveConfiguration>(preservation)));
            }
        });

        on<Trigger<WalkFitnessDelta>>([this](const WalkFitnessDelta& delta) {
           fitnessSum += delta.fitnessDelta;
        });
    }
}
}
}
