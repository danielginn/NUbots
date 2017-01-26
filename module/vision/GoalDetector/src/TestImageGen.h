
#include "GoalMatcherConstants.h"
#include "message/localisation/FieldObject.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>

using namespace cv;
using namespace std;

std::vector<uint8_t> testImageGen(uint8_t imageNum, std::unique_ptr<message::localisation::Self>& self) {
	Mat raw_image;
	std::vector<uint8_t> image(IMAGE_HEIGHT*IMAGE_WIDTH*2,0);
	self->position[0] = 0; // x position
	self->position[1] = 0; // y position
	self->heading[0] = 1;  // x component heading
	self->heading[1] = 0;  // y component heading
	raw_image = imread("/home/vagrant/NUbots/module/vision/GoalDetector/data/20170123_140914.jpg",CV_LOAD_IMAGE_GRAYSCALE);
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

	return image;
}