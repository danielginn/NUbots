#include "GoalMatcherConstants.h"
#include "message/localisation/FieldObject.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>

using namespace cv;
using namespace std;

	std::vector<uint8_t> backgroundImageGen(uint8_t imageNum, std::unique_ptr<message::localisation::Self>& self) {
	std::vector<uint8_t> image(IMAGE_HEIGHT*IMAGE_WIDTH*2,0);
	Mat raw_image;
	if (imageNum == 1) {      // Center looking at away goal
		self->position[0] = 0; // x position
		self->position[1] = 0; // y position
		self->heading[0] = 1;  // x component heading
		self->heading[1] = 0;  // y component heading
		for (int m = 0; m < IMAGE_HEIGHT;m++){
			for (int i = 0;i < IMAGE_WIDTH;++i) {
				if (i < (410))       image[m*IMAGE_WIDTH*2+i*2] = 30; // shirt A
				else if (i < (820))  image[m*IMAGE_WIDTH*2+i*2] = 200; // shirt B
				else if (i < (1230)) image[m*IMAGE_WIDTH*2+i*2] = 100; // shirt C
				else if (i < (1640)) image[m*IMAGE_WIDTH*2+i*2] = 10;  // shirt D
				else           		 image[m*IMAGE_WIDTH*2+i*2] = 230; // shirt E
			}
		}
	}
	else if (imageNum == 2) { // center shifted right looking at away goal
		self->position[0] = 0; // x position
		self->position[1] = -500; // y position
		self->heading[0] = 1;  // x component heading
		self->heading[1] = 0;  // y component heading
		raw_image = imread("/home/vagrant/NUbots/module/vision/GoalDetector/data/20170123_140906.jpg",CV_LOAD_IMAGE_GRAYSCALE);
		if (!raw_image.data) {
			cout << "Could not open or find the image" << std::endl;
		}
		else {
			printf("raw_image rows: %d, columns: %d\n",raw_image.rows,raw_image.cols);
			for (int m=0;m < IMAGE_HEIGHT;m++){
				for (int n=0;n < IMAGE_WIDTH;n++){
					image[m*IMAGE_WIDTH*2+n*2] = raw_image.at<unsigned char>(m,n);
				}
			}
		//	namedWindow("Display window");
		//	imshow("Display window",raw_image);
		//	waitKey(0);	
		}	
	}
	else if (imageNum == 3) { // zoomed out with slightly noisy colours S.D. of about 10.
		self->position[0] = -500; // x position
		self->position[1] = 0; // y position
		self->heading[0] = 1;  // x component heading
		self->heading[1] = 0;  // y component heading
		for (int m = 0; m < IMAGE_HEIGHT;m++){
			for (int i = 0;i < IMAGE_WIDTH;++i) {
				if (i < 205)       image[m*IMAGE_WIDTH*2+i*2] = 70;  // shirt Z
				else if (i < 492)  image[m*IMAGE_WIDTH*2+i*2] = 24;  // shirt A
				else if (i < 779)  image[m*IMAGE_WIDTH*2+i*2] = 217; // shirt B
				else if (i < 1066) image[m*IMAGE_WIDTH*2+i*2] = 96;  // shirt C
				else if (i < 1353) image[m*IMAGE_WIDTH*2+i*2] = 0;   // shirt D
				else if (i < 1640) image[m*IMAGE_WIDTH*2+i*2] = 216; // shirt E
				else if (i < 1804) image[m*IMAGE_WIDTH*2+i*2] = 133; // shirt F
				else		       image[m*IMAGE_WIDTH*2+i*2] = 99;  // shirt G
			}
		}
	}
	else if (imageNum == 4) { // 1 changed shirt colour and stands to the left of goal (angled)
		self->position[0] = 2000; // x position
		self->position[1] = 1000; // y position
		self->heading[0] = 0.707;  // x component heading
		self->heading[1] = -0.707;  // y component heading
		for (int m = 0; m < IMAGE_HEIGHT;m++){
			for (int i = 0;i < IMAGE_WIDTH;++i) {
				if (i < 205)       image[m*IMAGE_WIDTH*2+i*2] = 187; // shirt B
				else if (i < 820)  image[m*IMAGE_WIDTH*2+i*2] = 40;  // shirt C (changed)
				else if (i < 1312) image[m*IMAGE_WIDTH*2+i*2] = 21;  // shirt D
				else if (i < 1640) image[m*IMAGE_WIDTH*2+i*2] = 222; // shirt E
				else if (i < 1804) image[m*IMAGE_WIDTH*2+i*2] = 117; // shirt F 
				else if (i < 1927) image[m*IMAGE_WIDTH*2+i*2] = 114; // shirt G
				else if (i < 2009) image[m*IMAGE_WIDTH*2+i*2] = 23;  // shirt H
				else			   image[m*IMAGE_WIDTH*2+i*2] = 52;  // shirt I
			}
		}
	}
	else if (imageNum == 5) { // up close to right
		self->position[0] = 3000; // x position
		self->position[1] = -2000; // y position
		self->heading[0] = 1;  // x component heading
		self->heading[1] = 0;  // y component heading
		for (int m = 0; m < IMAGE_HEIGHT;m++){
			for (int i = 0;i < IMAGE_WIDTH;++i) {
				if (i < 615)       image[m*IMAGE_WIDTH*2+i*2] = 15;  // shirt D
				else if (i < 1230) image[m*IMAGE_WIDTH*2+i*2] = 222; // shirt E
				else if (i < 1845) image[m*IMAGE_WIDTH*2+i*2] = 112; // shirt F
				else			   image[m*IMAGE_WIDTH*2+i*2] = 103; // shirt G
			}
		}
	}


	else if (imageNum == 6) { // Center looking at own goal
		self->position[0] = 0; // x position
		self->position[1] = 0; // y position
		self->heading[0] = -1;  // x component heading
		self->heading[1] = 0;  // y component heading
		for (int m = 0; m < IMAGE_HEIGHT;m++){
			for (int i = 0;i < IMAGE_WIDTH;++i) {
				if (i < 410)       image[m*IMAGE_WIDTH*2+i*2] = 140; // shirt A
				else if (i < 1230) image[m*IMAGE_WIDTH*2+i*2] = 60;  // shirt B
				else if (i < 1435) image[m*IMAGE_WIDTH*2+i*2] = 190; // shirt C
				else if (i < 1845) image[m*IMAGE_WIDTH*2+i*2] = 240; // shirt D
				else			   image[m*IMAGE_WIDTH*2+i*2] = 100; // shirt E
			}
		}
	}
	else if (imageNum == 7) { // From left of goal (angled)
		self->position[0] = -2000; // x position
		self->position[1] = -1000; // y position
		self->heading[0] = -0.707;  // x component heading
		self->heading[1] = 0.707;  // y component heading
		for (int m = 0; m < IMAGE_HEIGHT;m++){
			for (int i = 0;i < IMAGE_WIDTH;++i) {
				if (i < 205)       image[m*IMAGE_WIDTH*2+i*2] = 145; // shirt A
				else if (i < 1435) image[m*IMAGE_WIDTH*2+i*2] = 66;  // shirt B
				else if (i < 1558) image[m*IMAGE_WIDTH*2+i*2] = 180; // shirt C
				else if (i < 1763) image[m*IMAGE_WIDTH*2+i*2] = 232; // shirt D
				else if (i < 1968) image[m*IMAGE_WIDTH*2+i*2] = 100; // shirt E
				else			   image[m*IMAGE_WIDTH*2+i*2] = 150; // shirt F
			}
		}
	}
	else if (imageNum == 8) { // zoomed out
		self->position[0] = -500; // x position
		self->position[1] = 0; // y position
		self->heading[0] = -1;  // x component heading
		self->heading[1] = 0;  // y component heading
		for (int m = 0; m < IMAGE_HEIGHT;m++){
			for (int i = 0;i < IMAGE_WIDTH;++i) {
				if (i < 287)       image[m*IMAGE_WIDTH*2+i*2] = 33;  // shirt Z
				else if (i < 574)  image[m*IMAGE_WIDTH*2+i*2] = 132; // shirt A
				else if (i < 1189) image[m*IMAGE_WIDTH*2+i*2] = 65;  // shirt B
				else if (i < 1312) image[m*IMAGE_WIDTH*2+i*2] = 187; // shirt C
				else if (i < 1599) image[m*IMAGE_WIDTH*2+i*2] = 230; // shirt D
				else if (i < 1886) image[m*IMAGE_WIDTH*2+i*2] = 107; // shirt E
				else 			   image[m*IMAGE_WIDTH*2+i*2] = 164; // shirt F
			}
		}
	}
	else if (imageNum == 9) { // stands to right of goal zoomed a bit(straight)
		self->position[0] = -2000; // x position
		self->position[1] = 1000; // y position
		self->heading[0] = -1;  // x component heading
		self->heading[1] = 0;  // y component heading
		for (int m = 0; m < IMAGE_HEIGHT;m++){
			for (int i = 0;i < IMAGE_WIDTH;++i) {
				if (i < 205)       image[m*IMAGE_WIDTH*2+i*2] = 172; // shirt C
				else if (i < 697)  image[m*IMAGE_WIDTH*2+i*2] = 238; // shirt D
				else if (i < 1025) image[m*IMAGE_WIDTH*2+i*2] = 95;  // shirt E
				else if (i < 1435) image[m*IMAGE_WIDTH*2+i*2] = 155; // shirt F
				else if (i < 1722) image[m*IMAGE_WIDTH*2+i*2] = 45;  // shirt G
				else			   image[m*IMAGE_WIDTH*2+i*2] = 97;  // shirt H
			}
		}
	}
	else if (imageNum == 10) { // center closer (2 swapped)
		self->position[0] = -1000; // x position
		self->position[1] = 0; // y position
		self->heading[0] = -1;  // x component heading
		self->heading[1] = 0;  // y component heading
		for (int m = 0; m < IMAGE_HEIGHT;m++){
			for (int i = 0;i < IMAGE_WIDTH;++i) {
				if (i < 328)       image[m*IMAGE_WIDTH*2+i*2] = 133; // shirt A
				else if (i < 1230) image[m*IMAGE_WIDTH*2+i*2] = 54; // shirt B
				else if (i < 1681) image[m*IMAGE_WIDTH*2+i*2] = 244;  // shirt D (swapped)
				else if (i < 1927) image[m*IMAGE_WIDTH*2+i*2] = 187; // shirt C (swapped)
				else			   image[m*IMAGE_WIDTH*2+i*2] = 115;  // shirt E
			}
		}
	}


	return image;
}
