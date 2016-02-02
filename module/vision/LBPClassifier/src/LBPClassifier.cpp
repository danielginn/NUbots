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
#include "message/support/Configuration.h"
#include "message/input/Image.h"
#include "utility/nubugger/NUhelpers.h"
#include "utility/time/time.h"

#define DIFFERENCE 0

namespace module {
namespace vision {

    using message::support::Configuration;
    using message::input::Image;

    LBPClassifier::LBPClassifier(std::unique_ptr<NUClear::Environment> environment)
    : Reactor(std::move(environment)) {

        /*on<Configuration>("LBPClassifier.yaml").then([this] (const Configuration& config) {
            // Use configuration here from file LBPClassifier.yaml
        });*/
        

        on<Trigger<Image>, Single>().then([this](const Image& image) {
            //NUClear::clock::time_point start;
            int histLBP[256][3];
            int histDRLBP[256][3];

            std::memset(&histLBP, 0, sizeof(histLBP));
            std::memset(&histDRLBP, 0, sizeof(histDRLBP));

            constexpr const int shift[8][2] = {{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,1},{-1,-1},{-1,0}};
            int LBP[] = {0,0,0};
            double gradPix[] = {0,0};

            int x0 = image.width/2-50, x1 = image.width/2+50, y0 = image.height/2-50, y1 = image.height/2+50;

            Image::Pixel currPix;
            for(auto x = x0; x < x1; x++){
                for(auto y = y0; y < y1; y++){
                    currPix = image(x,y);
                    for(auto i=0;i<8;i++){
                        if(currPix.y-image(x+shift[i][0],y+shift[i][1]).y >= DIFFERENCE){
                            LBP[0] += (1 << i);
                        }
                        if(currPix.cb-image(x+shift[i][0],y+shift[i][1]).cb >= DIFFERENCE){
                            LBP[1] += (1 << i);
                        }
                        if(currPix.cr-image(x+shift[i][0],y+shift[i][1]).cr >= DIFFERENCE){
                            LBP[2] += (1 << i);
                        }
                    }
                    
                    gradPix[0] = (image(x+1,y).y-image(x-1,y).y)/2;
                    gradPix[1] = (image(x,y+1).y-image(x,y-1).y)/2;
                    histLBP[LBP[0]][0] += sqrt(gradPix[0]*gradPix[0] + gradPix[1]*gradPix[1]);

                    gradPix[0] = (image(x+1,y).cb-image(x-1,y).cb)/2;
                    gradPix[1] = (image(x,y+1).cb-image(x,y-1).cb)/2;                           //DRLBP
                    histLBP[LBP[1]][1] += sqrt(gradPix[0]*gradPix[0] + gradPix[1]*gradPix[1]);

                    gradPix[0] = (image(x+1,y).cr-image(x-1,y).cr)/2;
                    gradPix[1] = (image(x,y+1).cr-image(x,y-1).cr)/2;
                    histLBP[LBP[2]][2] += sqrt(gradPix[0]*gradPix[0] + gradPix[1]*gradPix[1]);

                    /*
                    histLBP[LBP[0]][0]++;
                    histLBP[LBP[1]][1]++;                                                       //LBP
                    histLBP[LBP[2]][2]++;
                    */

                    LBP[0] = 0;
                    LBP[1] = 0;
                    LBP[2] = 0;
                }
            }

            std::vector<std::pair<arma::ivec2, arma::ivec2>> hist;
            hist.reserve(768);

            double tempHist;
            for(auto k=0;k<128;k++){
                for(auto l=0;l<3;l++){
                    histDRLBP[k][l] = histLBP[k][l] + histLBP[255-k][l];
                    //tempHist = (double)(2*histLBP[k][l])/(double)((x1-x0)*(y1-y0));   //LBP
                    tempHist = (double)histDRLBP[k][l]/((x1-x0)*(y1-y0)*2);             //DRLBP
                    hist.push_back({arma::ivec2({3*k+l,0}),arma::ivec2({3*k+l+1,(double)(image.height)*tempHist})});
                }
            }

            for(auto k=128;k<256;k++){
                for(auto l=0;l<3;l++){
                    histDRLBP[k][l] = fabs(histLBP[k][l] - histLBP[255-k][l]);
                    //tempHist = (double)(2*histLBP[k][l])/(double)((x1-x0)*(y1-y0));   //LBP
                    tempHist = (double)histDRLBP[k][l]/((x1-x0)*(y1-y0)*2);             //DRLBP
                    hist.push_back({arma::ivec2({3*(k-128)+l,image.height}),arma::ivec2({3*(k-128)+l+1,image.height-(double)(image.height)*tempHist})});
                }
            }

            arma::ivec2 tl = {x0,y0};
            arma::ivec2 tr = {x1,y0};
            arma::ivec2 bl = {x0,y1};
            arma::ivec2 br = {x1,y1};

            hist.push_back({tl, tr});
            hist.push_back({tr, br});
            hist.push_back({br, bl});
            hist.push_back({bl, tl});
            /*
            
            /*if(abs(max[0][1]-1.13)<=epsilon && abs(max[0][2]-1.02)<=epsilon && abs(max[0][3]-0.79)<=epsilon && abs(max[0][4]-0.78)<=epsilon){

            }
            std::cout << std::fixed << std::setprecision(3);


            emit(utility::nubugger::drawVisionLines(hist));  */  
            //NUClear::clock::time_point end;  
            //auto time_diff = end - start;
            //log("Elapsed(ns):",std::chrono::duration_cast<std::chrono::nanoseconds>(time_diff).count());    
        });
        
    }
}
}
