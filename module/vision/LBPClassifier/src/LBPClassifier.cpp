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

#include "LBPClassifier.h"
#include <iostream>
#include <iomanip>
#include <fstream> 
#include "message/support/Configuration.h"
#include "message/input/Image.h"
#include "utility/nubugger/NUhelpers.h"
#include "message/platform/darwin/DarwinSensors.h"
#include "utility/time/time.h"
#include "svm.h"

#define CHANNELS 1

namespace module {
namespace vision {

    using message::support::Configuration;
    using message::input::Image;
    using message::platform::darwin::ButtonLeftDown;
    using message::platform::darwin::ButtonMiddleDown;    

    void LBPClassifier::toFile(int histLBP[][CHANNELS], int polarity){
        std::ofstream output;
        output.open(LBPClassifier::typeLBP, std::ofstream::app);
        output << std::fixed << std::setprecision(3);
        output << polarity << " ";
        if(LBPAlgorithm & LBPAlgorithmTypes::Uniform){
            for(auto j=0; j<CHANNELS; j++){
                for(auto i=0; i<256; i++){
                    output << j*256+1+i << ":" << (double)histLBP[i][j]/divisorLBP << " ";
                }
            }
        }
        else if(LBPAlgorithm & LBPAlgorithmTypes::Robust){  /*NOT IMPLEMENTED*/ 
            for(auto i=0; i<128; i++){
                output << j*128+1+i << ":" << (double)histLBP[i][j]/divisorLBP << " ";
            }
        }
        else{ //XXX: this isn't how polarity works.... polarity refers to the direction of the >=/<= when creating hte LBP codes.
            for(auto j=0; j<CHANNELS; j++){
                for(auto i=0; i<128; i++){
                    output << j*256+1+i << ":" << (double)(histLBP[i][j] + histLBP[255-i][j])/divisorDRLBP << " ";
                }
                for(auto i=128; i<256; i++){
                    output << j*256+1+i << ":" << (double)(fabs(histLBP[i][j] - histLBP[255-i][j]))/divisorDRLBP << " ";
                }
            }            
        } 
        output << "\n";
        output.close();
        if(polarity == 1){
            log("Positive image histogram saved.");
        }
        else{
            log("Negative image histogram saved.");
        }
    }

    void LBPClassifier::drawHist(int histLBP[][CHANNELS], const uint imgW, const uint imgH, bool found){
        int width = 50;
        int x0 = imgW/2.0-width, x1 = imgW/2.0+width, y0 = imgH/2.0-width, y1 = imgH/2.0+width;
        std::vector<std::pair<arma::ivec2, arma::ivec2>> hist;
        hist.reserve(256*CHANNELS);
        double tempHist, max=0;
        int x_init;
        //TODO: Properly adjust output for Ternary Types
        //Not printing histogram for ternary type
        if(LBPAlgorithm & LBPAlgorithmTypes::Uniform){
            for(auto j=0; j<CHANNELS; j++){
                for(auto i=0; i<256; i++){
                    tempHist = (double)histLBP[i][j]/divisorLBP;
                    hist.push_back({arma::ivec2({x_init,imgH}),arma::ivec2({x_init+1,(int)(imgH-(double)(imgH)*tempHist/2.0)})});
                }
            }
        }
        else if(LBPAlgorithm & LBPAlgorithmTypes::Robust){  /*NOT IMPLEMENTED*/ 
            for(auto j=0; j<CHANNELS; j++){
                for(auto i=0; i<128; i++){
                    tempHist = (double)histLBP[i][j]/divisorLBP;
                    hist.push_back({arma::ivec2({x_init,imgH}),arma::ivec2({x_init+1,(int)(imgH-(double)(imgH)*tempHist/2.0)})});
                }
            }
        }
        else{ //XXX: this isn't how polarity works.... polarity refers to the direction of the >=/<= when creating hte LBP codes.
            for(auto j=0; j<CHANNELS; j++){
                for(auto i=0; i<128; i++){
                    tempHist = (double)(histLBP[i][j] + histLBP[255-i][j])/divisorDRLBP;
                    x_init = ((imgW-CHANNELS*128)/2.0+CHANNELS*i+j);
                    hist.push_back({arma::ivec2({x_init,0}),arma::ivec2({x_init+1,(int)((double)(imgH)*tempHist/2.0)})});
                }
                for(auto i=128; i<256; i++){
                    tempHist = (double)(fabs(histLBP[i][j] - histLBP[255-i][j]))/divisorDRLBP;
                    x_init = (imgW-CHANNELS*128)/2.0+CHANNELS*(i-128)+j;
                    hist.push_back({arma::ivec2({x_init,imgH}),arma::ivec2({x_init+1,(int)(imgH-(double)(imgH)*tempHist/2.0)})});
                }
            }            
        }
        //log("Max:",max);
        arma::ivec2 tl = {x0,y0};
        arma::ivec2 tr = {x1,y0};
        arma::ivec2 bl = {x0,y1};
        arma::ivec2 br = {x1,y1};
        hist.push_back({tl, tr});
        hist.push_back({tr, br});
        hist.push_back({br, bl});
        hist.push_back({bl, tl});
        if(found == true){
            hist.push_back({tl,br});
            hist.push_back({tr,bl});
        }
        emit(utility::nubugger::drawVisionLines(hist));   
    }

    LBPClassifier::LBPClassifier(std::unique_ptr<NUClear::Environment> environment)
    : Reactor(std::move(environment)) {
        on<Configuration>("LBPClassifier.yaml").then([this] (const Configuration& config) {
            // Use configuration here from file LBPClassifier.yaml
            //samplingPts = config["samplingPts"].as<const uint>();
            typeLBP = config["typeLBP"].as<std::string>();
            //XXX: typeLBP needs to be cleaned up later, this just makes things easier
            if (typeLBP[0] == 'R' or typeLBP[1] == 'R' or typeLBP[2] == 'R') {
                LBPAlgorithm |= LBPAlgorithmTypes::Robust;
            }
            if (typeLBP[0] == 'D' or typeLBP[1] == 'D' or typeLBP[2] == 'D') {
                LBPAlgorithm |= LBPAlgorithmTypes::Discriminative;
            }
            if (typeLBP[0] == 'U' or typeLBP[1] == 'U' or typeLBP[2] == 'U') {
                LBPAlgorithm |= LBPAlgorithmTypes::Uniform;
            }
            if (typeLBP[typeLBP.size()-2] == 'T') {
                LBPAlgorithm |= LBPAlgorithmTypes::Ternary;
            }
            
            noiseLim = config["noiseLim"].as<int>();
            divisorLBP = config["divisorLBP"].as<float>();
            divisorDRLBP = config["divisorDRLBP"].as<float>();
            trainingStage = config["trainingStage"].as<std::string>();
            draw = config["draw"].as<bool>();
            output = config["output"].as<bool>();
        });

        log("Vision Channels:",CHANNELS);
        log("LBP Type       :",typeLBP);
        log("Noise Limit    :",noiseLim);
        log("Training Stage :",trainingStage);
        log("Histogram Draw :",draw);
        log("Output         :",output);

        if(output == true){
            std::ofstream oFile;
            oFile.open(LBPClassifier::typeLBP, std::ofstream::trunc); //Clears the file each time program is run
        }

        on<Trigger<Image>, Single>().then([this](const Image& image){
            numImages++;
            std::memset(&histLBP, 0, sizeof(histLBP));
            constexpr const int shift[8][2] = {{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,1},{-1,-1},{-1,0}};
            uint64_t LBP[] = {0,0,0};
            uint64_t LLBP[] = {0,0,0};

            //NEEDS TO BE REMOVED AND CONFIG TO BE WORKING
            if (typeLBP[0] == 'R' or typeLBP[1] == 'R' or typeLBP[2] == 'R') {
                LBPAlgorithm |= LBPAlgorithmTypes::Robust;
            }
            if (typeLBP[0] == 'D' or typeLBP[1] == 'D' or typeLBP[2] == 'D') {
                LBPAlgorithm |= LBPAlgorithmTypes::Discriminative;
            }
            if (typeLBP[0] == 'U' or typeLBP[1] == 'U' or typeLBP[2] == 'U') {
                LBPAlgorithm |= LBPAlgorithmTypes::Uniform;
            }
            if (typeLBP[typeLBP.size()-2] == 'T') {
                LBPAlgorithm |= LBPAlgorithmTypes::Ternary;
            }

            arma::vec2 gradPix;
            int width = 75;
            int x0 = image.width/2.0-width, x1 = image.width/2.0+width, y0 = image.height/2.0-width, y1 = image.height/2.0+width;
            Image::Pixel currPix;
            bool found = false;
            divisorDRLBP = 0.;
            divisorRLBP = 0.;
            divisorLBP = 0.;
            for(auto x = x0; x < x1; x++){
                for(auto y = y0; y < y1; y++){
                    //clear this first so we don't need to think about implicit changes.
                    LBP[0] = 0;
                    LBP[1] = 0;
                    LBP[2] = 0;
                    LLBP[0] = 0;
                    LLBP[1] = 0;
                    LLBP[2] = 0;
                    
                    currPix = image(x,y);
                    for(auto i=0;i<8;i++){
                        int32_t tmpval;
                        switch(CHANNELS){
                            case 3:
                                tmpval = currPix.cr-image(x+shift[i][0],y+shift[i][1]).cr;
                                if(tmpval >= noiseLim){ //TODO: implement a polarity switch with <= instead of >=
                                    LBP[2] += (1ull << i);
                                } else if (tmpval <= -noiseLim) {
                                    LLBP[2] += (1ull << i);
                                }
                            case 2:
                                tmpval = currPix.cb-image(x+shift[i][0],y+shift[i][1]).cb;
                                if(tmpval >= noiseLim){
                                    LBP[1] += (1ull << i); //any time you bitshift a constant, cast it to ULL so that it's 64-bit. Just in case you need it.
                                } else if (tmpval <= -noiseLim) {
                                    LLBP[1] += (1ull << i);
                                }
                            case 1:
                                tmpval = currPix.y-image(x+shift[i][0],y+shift[i][1]).y;
                                if(tmpval >= noiseLim){
                                    LBP[0] += (1ull << i);
                                } else if (tmpval <= -noiseLim) {
                                    LLBP[0] += (1ull << i);
                                }
                            break;
                        }                        
                    }
                    
                    //this is the inverted lighting condition from the paper (the "R" part of "RLBP")
                    if (LBPAlgorithm & LBPAlgorithmTypes::Robust) {
                        for(auto j = 0; j < CHANNELS; j++) {
                            //XXX: shift by 8 as it's the size of the shift array. THIS IS BAD, USE arma::Mat<int32_t>(8,2) for shift.
                            //Better yet, give a distance and number of bits in the config and calculate shift at init time.
                            LBP[j] = std::min(LBP[j], (1ull << 8) - 1 - LBP[j]);
                            
                            if (LBPAlgorithm & LBPAlgorithmTypes::Ternary) {
                                LLBP[j] = std::max(LLBP[j], (1ull << 8) - 1 - LLBP[j]);
                            }
                        }
                    } 
                    
                    //TODO: uniform pattern mapping - should be precalculated on init to let us map down to a MUCH smaller vector than we currently use
                    
                    
                    //do the "D" - discriminative part of LBP
                    if (LBPAlgorithm & LBPAlgorithmTypes::Discriminative) {
                        double result;
                        if (LBPAlgorithm & LBPAlgorithmTypes::Ternary) {
                            switch(CHANNELS){
                                case 3:
                                    gradPix[0] = (image(x+1,y).cr-image(x-1,y).cr);
                                    gradPix[1] = (image(x,y+1).cr-image(x,y-1).cr);
                                    result = arma::norm(gradPix*0.5);
                                    histLBP[LBP[2]][2] += result;
                                    histLBP[LLBP[2]][2] += result;
                                    divisorDRLBP += 2*result;
                                case 2:
                                    gradPix[0] = (image(x+1,y).cb-image(x-1,y).cb);
                                    gradPix[1] = (image(x,y+1).cb-image(x,y-1).cb);
                                    result = arma::norm(gradPix*0.5);
                                    histLBP[LBP[1]][1] += result;
                                    histLBP[LLBP[1]][1] += result;
                                    divisorDRLBP += 2*result;
                                case 1:
                                    gradPix[0] = (image(x+1,y).y-image(x-1,y).y);
                                    gradPix[1] = (image(x,y+1).y-image(x,y-1).y);
                                    result = arma::norm(gradPix*0.5);
                                    histLBP[LBP[0]][0] += result;
                                    histLBP[LLBP[0]][0] += result;
                                    divisorDRLBP += 2*result;
                                    break;
                            }
                        } else {
                            switch(CHANNELS){
                                case 3:
                                    gradPix[0] = (image(x+1,y).cr-image(x-1,y).cr);
                                    gradPix[1] = (image(x,y+1).cr-image(x,y-1).cr);
                                    result = arma::norm(gradPix*0.5);
                                    histLBP[LBP[2]][2] += result;
                                    divisorDRLBP += result;
                                case 2:
                                    gradPix[0] = (image(x+1,y).cb-image(x-1,y).cb);
                                    gradPix[1] = (image(x,y+1).cb-image(x,y-1).cb);
                                    result = arma::norm(gradPix*0.5);
                                    histLBP[LBP[1]][1] += result;
                                    divisorDRLBP += result;
                                case 1:
                                    gradPix[0] = (image(x+1,y).y-image(x-1,y).y);
                                    gradPix[1] = (image(x,y+1).y-image(x,y-1).y);
                                    result = arma::norm(gradPix*0.5);
                                    histLBP[LBP[0]][0] += result;
                                    divisorDRLBP += result;
                                    break;
                            }
                        }
                        
                    //if we're not being discriminative, just use plain LBP
                    } else {
                        for(auto j=0; j<CHANNELS; j++){
                            histLBP[LBP[j]][j]++;
                            if (LBPAlgorithm & LBPAlgorithmTypes::Ternary) {
                                histLBP[LLBP[j]][j]++;
                                divisorLBP++;
                            }
                        }
                    }
                }
            }

            if(trainingStage == "TESTING"){
                svm_node node[256*CHANNELS];
                if(typeLBP == "LBP"){
                    for(int j=0;j<CHANNELS;j++){
                        for(int i=0;i<256;i++){
                            svm_node tempNode;
                            tempNode.index = j*256+1+i;
                            tempNode.value = ((double)(histLBP[i][j]))/divisorLBP;
                            node[j*256+i] = tempNode;
                        }
                    }
                }
                else{
                    svm_node tempNode;
                    for(int j=0;j<CHANNELS;j++){
                        for(int i=0;i<128;i++){
                            tempNode.index = j*256+1+i;
                            tempNode.value = (double)(histLBP[i][j] + histLBP[255-i][j])/divisorDRLBP;
                            node[j*256+i] = tempNode;
                        }
                        for(int i=128;i<256;i++){
                            tempNode.index = j*256+1+i;
                            tempNode.value = ((double)( fabs(histLBP[i][j] - histLBP[255-i][j]) ))/divisorDRLBP;
                            node[j*256+i] = tempNode;
                        }
                    }
                }
                node[256*CHANNELS].index = -1;
                svm_model* model = svm_load_model((std::string(typeLBP)+std::string(".model")).c_str()); 
                double result = svm_predict(model,node);
                log("Prediction:",result);
                if(result == 1){
                    found = true;
                }
                else{
                    found = false;
                }
            }

            if(draw == true){
                drawHist(histLBP, image.width, image.height, found);
            }
            
        });
        on<Trigger<ButtonLeftDown>, Single>().then([this]{
            if(output == true){
                toFile(histLBP,1);
            }
        });
        on<Trigger<ButtonMiddleDown>, Single>().then([this]{
            if(output == true){
                toFile(histLBP,-1);
            }
        });
    }
}
}
