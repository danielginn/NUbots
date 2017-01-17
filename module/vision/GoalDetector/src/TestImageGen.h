
#include "GoalMatcherConstants.h"
#include "message/localisation/FieldObject.h"

std::vector<uint8_t> testImageGen(uint8_t imageNum, std::unique_ptr<message::localisation::Self>& self) {
	std::vector<uint8_t> image(IMAGE_HEIGHT*IMAGE_WIDTH*2,0);
	self->position[0] = 0; // x position
	self->position[1] = 0; // y position
	self->heading[0] = 1;  // x component heading
	self->heading[1] = 0;  // y component heading
	for (int i = 0;i < image.size();i++) { // shirt intensities modified
		if (i < 10)      image[++i] = 30;  // shirt A
		else if (i < 20) image[i++] = 200; // shirt B
		else if (i < 30) image[i++] = 100;  // shirt C
		else if (i < 40) image[i++] = 10;  // shirt D
		else             image[i++] = 229; // shirt E
	}

	return image;
}