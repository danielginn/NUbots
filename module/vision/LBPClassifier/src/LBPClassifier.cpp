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
        if(typeLBP == "LBP"){
            for(auto j=0; j<CHANNELS; j++){
                for(auto i=0; i<256; i++){
                    output << j*256+1+i << ":" << (float)histLBP[i][j]/divisorLBP << " ";
                }
            }
        }
        else{
            for(auto j=0; j<CHANNELS; j++){
                for(auto i=0; i<128; i++){
                    output << j*256+1+i << ":" << (float)(histLBP[i][j] + histLBP[255-i][j])/divisorDRLBP << " ";
                }
                for(auto i=128; i<256; i++){
                    output << j*256+1+i << ":" << (float)(fabs(histLBP[i][j] - histLBP[255-i][j]))/divisorDRLBP << " ";
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

    void LBPClassifier::drawHist(int histLBP[][CHANNELS], const uint imgW, const uint imgH){
        int width = 50;
        int x0 = imgW/2.0-width, x1 = imgW/2.0+width, y0 = imgH/2.0-width, y1 = imgH/2.0+width;
        std::vector<std::pair<arma::ivec2, arma::ivec2>> hist;
        hist.reserve(768);
        double tempHist, max=0;
        int x_init;
        for(auto k=0;k<128;k++){
            for(auto l=0;l<CHANNELS;l++){
                if(typeLBP == "LBP"){
                    tempHist = ((double)(histLBP[k][l]/divisorLBP));
                }
                else if(typeLBP == "DRLBP"){
                    tempHist = ( (double)(histLBP[k][l] + histLBP[255-k][l])/divisorDRLBP );
                }
                (tempHist > max) ? (max = tempHist) : (max = max) ;
                x_init = ((imgW-CHANNELS*128)/2.0+CHANNELS*k+l);
                hist.push_back({arma::ivec2({x_init,0}),arma::ivec2({x_init+1,(int)((double)(imgH)*tempHist/2.0)})});
            }
        }
        for(auto k=128;k<256;k++){
            for(auto l=0;l<CHANNELS;l++){
                if(typeLBP == "LBP"){
                    tempHist = ((double)(histLBP[k][l]/divisorLBP));
                }
                else if(typeLBP == "DRLBP"){
                    tempHist = ((double)( fabs(histLBP[k][l] - histLBP[255-k][l])/divisorDRLBP ));
                }
                (tempHist > max) ? (max = tempHist) : (max = max) ;
                x_init = (imgW-CHANNELS*128)/2.0+CHANNELS*(k-128)+l;
                hist.push_back({arma::ivec2({x_init,imgH}),arma::ivec2({x_init+1,(int)(imgH-(double)(imgH)*tempHist/2.0)})});
            }
        }
        log("Max:",max);
        arma::ivec2 tl = {x0,y0};
        arma::ivec2 tr = {x1,y0};
        arma::ivec2 bl = {x0,y1};
        arma::ivec2 br = {x1,y1};
        hist.push_back({tl, tr});
        hist.push_back({tr, br});
        hist.push_back({br, bl});
        hist.push_back({bl, tl});
        emit(utility::nubugger::drawVisionLines(hist));   
    }

    LBPClassifier::LBPClassifier(std::unique_ptr<NUClear::Environment> environment)
    : Reactor(std::move(environment)) {
        on<Configuration>("LBPClassifier.yaml").then([this] (const Configuration& config) {
            // Use configuration here from file LBPClassifier.yaml
            //samplingPts = config["samplingPts"].as<const uint>();
            typeLBP = config["typeLBP"].as<std::string>();
            noiseLim = config["noiseLim"].as<int>();
            divisorLBP = config["divisorLBP"].as<float>();
            divisorRLBP = config["divisorRLBP"].as<float>();
            divisorDRLBP = config["divisorDRLBP"].as<float>();
            trainingStage = config["trainingStage"].as<std::string>();
        });

        log("Vision Channels:",CHANNELS);
        log("LBP Type       :",typeLBP);
        log("Training Stage :",trainingStage);
        log("Output         :",output);

        if(output == true){
            std::ofstream oFile;
            oFile.open(LBPClassifier::typeLBP, std::ofstream::trunc); //Clears the file each time program is run
        }

        on<Trigger<Image>, Single>().then([this](const Image& image){
            //NUClear::clock::time_point start;
            std::memset(&histLBP, 0, sizeof(histLBP));
            constexpr const int shift[8][2] = {{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,1},{-1,-1},{-1,0}};
            int LBP[] = {0,0,0};
            double gradPix[] = {0,0};
            int width = 50;
            int x0 = image.width/2.0-width, x1 = image.width/2.0+width, y0 = image.height/2.0-width, y1 = image.height/2.0+width;
            Image::Pixel currPix;
            svm_node node[256*CHANNELS];
            for(auto x = x0; x < x1; x++){
                for(auto y = y0; y < y1; y++){
                    currPix = image(x,y);
                    for(auto i=0;i<8;i++){
                        switch(CHANNELS){
                            case 3:
                                if(currPix.cr-image(x+shift[i][0],y+shift[i][1]).cr >= noiseLim){
                                    LBP[2] += (1 << i);
                                }
                            case 2:
                                if(currPix.cb-image(x+shift[i][0],y+shift[i][1]).cb >= noiseLim){
                                    LBP[1] += (1 << i);
                                }
                            case 1:
                                if(currPix.y-image(x+shift[i][0],y+shift[i][1]).y >= noiseLim){
                                    LBP[0] += (1 << i);
                                }
                            break;
                        }                        
                    }
                    if(typeLBP == "LBP"){
                        for(auto j=0; j<CHANNELS; j++){
                            histLBP[LBP[j]][j]++;                                                       //LBP
                        }
                    }
                    else{
                        gradPix[0] = (image(x+1,y).y-image(x-1,y).y)/2.0;
                        gradPix[1] = (image(x,y+1).y-image(x,y-1).y)/2.0;
                        histLBP[LBP[0]][0] += sqrt(gradPix[0]*gradPix[0] + gradPix[1]*gradPix[1]);
                        if(CHANNELS > 1){
                            gradPix[0] = (image(x+1,y).cb-image(x-1,y).cb)/2.0;
                            gradPix[1] = (image(x,y+1).cb-image(x,y-1).cb)/2.0;                           //DRLBP
                            histLBP[LBP[1]][1] += sqrt(gradPix[0]*gradPix[0] + gradPix[1]*gradPix[1]);
                            if(CHANNELS == 3){
                                gradPix[0] = (image(x+1,y).cr-image(x-1,y).cr)/2.0;
                                gradPix[1] = (image(x,y+1).cr-image(x,y-1).cr)/2.0;
                                histLBP[LBP[2]][2] += sqrt(gradPix[0]*gradPix[0] + gradPix[1]*gradPix[1]);
                            }
                        } 
                    }
                    LBP[0] = 0;
                    LBP[1] = 0;
                    LBP[2] = 0;
                }
            }
            if(draw == true){
                drawHist(histLBP, image.width, image.height);
            }
            if(trainingStage == "TESTING"){
                for(int i=0;i<CHANNELS;i++){
                    for(int j=0;j<256;j++){
                        node[j+CHANNELS*256] = histLBP[j][i];
                    }
                }
                svm_model* model = svm_load_model("ballModel"); 
                log("Prediction:",svm_predict(model,node));
            }
            /*NUClear::clock::time_point end;  
            auto time_diff = end - start;
            log("Elapsed(ns):",std::chrono::duration_cast<std::chrono::nanoseconds>(time_diff).count());*/
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
