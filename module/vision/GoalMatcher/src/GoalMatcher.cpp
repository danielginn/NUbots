#include "message/input/Image.h"
#include "message/vision/ClassifiedImage.h"
#include "message/support/Configuration.h"
#include "message/input/CameraParameters.h"
#include "message/vision/LookUpTable.h"
#include "utility/math/geometry/Line.h"

#include "GoalMatcher.h"
#include "ClassifyGoalArea.h"
#include "SurfDetection.h"
#include <stdio.h>

namespace module {
namespace vision {

	//list of 'using' stuff here
	using message::input::CameraParameters;
	using message::support::Configuration;
    using message::input::Image;
	using message::vision::ClassifiedImage;
	using message::vision::ObjectClass;
	using message::vision::LookUpTable;
    using utility::math::geometry::Line;

	GoalMatcher::GoalMatcher(std::unique_ptr<NUClear::Environment> environment)
        : Reactor(std::move(environment))
        , VARIABLES_GO_HERE(0) {

        // Trigger the same function when either update
        on<Configuration, Trigger<CameraParameters>>("GoalMatcher.yaml")
        .then([this] (const Configuration& config, const CameraParameters& cam) {

        	VARIABLES_GO_HERE = config["variables_go_here"].as<uint>();

        });

        on<Every<1, std::chrono::seconds>>().then([this] {

            /* This is all stuff to make SurfDetection work in isolation. Will need to be deleted when integrated with GoalMatching.cpp */
            int pixValIt = -1;
            uint8_t Y = 0;
            int Yi = 0;
            NUClear::clock::time_point fake_timestamp = NUClear::clock::now();
            std::vector<uint8_t> test_image(IMAGE_HEIGHT*IMAGE_WIDTH*2,0);
            
            arma::vec2 n({ 0.0, 1.0 });
            double d = 3;
            int stripeType = 2; // Don't forget to change IMAGE_WIDTH as well

            // Creating a horizontally stripped grayscale image
            if (stripeType == 1) {   
                if ((IMAGE_HEIGHT == 10) && (IMAGE_WIDTH % 2 == 0))
                {
                    for (int ypix = 0; ypix < IMAGE_HEIGHT; ++ypix)
                    {
                        for (int xpix = 0; xpix < IMAGE_WIDTH; ++xpix)
                        {
                            test_image[Yi] = Y;
                            Yi = Yi + 2;
                        }
                        Y = Y+10;
                    }
                }
            }
            else if (stripeType == 2){
                uint8_t pixValues[] = {30,200,100,10,230};
                for (int ypix = 0; ypix < IMAGE_HEIGHT; ++ypix) {
                    for (int xpix = 0; xpix < IMAGE_WIDTH; ++xpix){
                        if (Yi%20 == 0){
                            ++pixValIt;
                        }
                        test_image.at(Yi) = pixValues[pixValIt];
                        Yi = Yi + 2;
                    }
                    pixValIt = -1;
                }
            }
            else {
                for (int ypix = 0; ypix < IMAGE_HEIGHT; ++ypix) {
                    for (int xpix = 0; xpix < IMAGE_WIDTH; ++xpix){
                        if ((xpix%IMAGE_WIDTH < (IMAGE_WIDTH/2))&&(ypix == 3)){
                            test_image.at(Yi) = 255;
                        }
                        Yi = Yi + 2;
                    }
                }
            }
            
            auto frame = std::make_unique<ClassifiedImage<ObjectClass>>();// Create an empty ClassifiedImage object
            frame->image = std::make_shared<const Image>((uint)IMAGE_WIDTH,(uint)IMAGE_HEIGHT,fake_timestamp,std::move(test_image));
            frame->horizon = Line(n,d);

            log("Hello world");
            emit(std::move(frame));
        });

        on<Trigger<ClassifiedImage<ObjectClass>>
         , Single>().then("Goal Matcher", [this] (const ClassifiedImage<ObjectClass>& frame) {

            int state = STATE_INITIAL; // During programming this variable can be modified here

            
            
            Image::Pixel tempPixel;
            uint w,h;
            w = (*frame.image).width;
            h = (*frame.image).height;
            int right_horizon = frame.horizon.y(w-1);
            printf("%dx%d\n\n",h,w);

            for (uint m = 0;m<h ;++m){
                for (uint n = 0;n<w;++n){
                    tempPixel = (*frame.image)(n,h-m-1);
                    printf("%3d ",tempPixel.y);
                }
                if ((h-m-1) == right_horizon){
                    printf("--");
                }
                printf("\n");
            }
            printf("\n");
            SurfDetection surf_obj(frame,filename);
         	surf_obj.findLandmarks(landmarks,landmark_tf,landmark_pixLoc);

            if (state == STATE_INITIAL) {
              wasInitial = true;
              clearMap = true;
            } else if (state != STATE_READY) {
              wasInitial = false;
            }        

            /****** TO DO ********/
            // Don't do anything if you are not really facing a field end 

            //if ((state == STATE_READY) && (wasInitial)){
                // saving landmarks mode
            /*
                if (clearMap) {
                    tfidf.clearMap();
                    clearMap = false;
                    awayMapSize = 0;
                    homeMapSize = 0;
                } */
                ClassifyGoalArea classifyGA;
                int num_matches = classifyGA.classifyGoalArea(frame, landmarks,landmark_tf,landmark_pixLoc);

         });

     }

}
}