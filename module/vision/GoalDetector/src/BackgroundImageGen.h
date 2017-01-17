#include "GoalMatcherConstants.h"
#include "message/localisation/FieldObject.h"

std::vector<uint8_t> backgroundImageGen(uint8_t imageNum, std::unique_ptr<message::localisation::Self>& self) {
	std::vector<uint8_t> image(IMAGE_HEIGHT*IMAGE_WIDTH*2,0);
	if (imageNum == 1) {      // Center looking at away goal
		self->position[0] = 0; // x position
		self->position[1] = 0; // y position
		self->heading[0] = 1;  // x component heading
		self->heading[1] = 0;  // y component heading
		for (int i = 0;i < image.size();i++) {
			if (i < 10)      image[++i] = 30;  // shirt A
			else if (i < 20) image[i++] = 200; // shirt B
			else if (i < 30) image[i++] = 100; // shirt C
			else if (i < 40) image[i++] = 10;  // shirt D
			else             image[i++] = 230; // shirt E
		}
	}
	else if (imageNum == 2) { // center shifted right looking at away goal
		self->position[0] = 0; // x position
		self->position[1] = -500; // y position
		self->heading[0] = 1;  // x component heading
		self->heading[1] = 0;  // y component heading
		for (int i = 0; i < image.size();i++) {
			if (i < 5)       image[i++] = 30;  // shirt A
			else if (i < 15) image[i++] = 200; // shirt B
			else if (i < 25) image[i++] = 100; // shirt C
			else if (i < 35) image[i++] = 10;  // shirt D 
			else if (i < 45) image[i++] = 230; // shirt E
			else             image[i++] = 120; // shirt F
		}
	}
	else if (imageNum == 3) { // zoomed out with slightly noisy colours S.D. of about 10.
		self->position[0] = -500; // x position
		self->position[1] = 0; // y position
		self->heading[0] = 1;  // x component heading
		self->heading[1] = 0;  // y component heading
		for (int i = 0; i < image.size();i++) {
			if (i < 5)       image[i++] = 70;  // shirt Z
			else if (i < 12) image[i++] = 24;  // shirt A
			else if (i < 19) image[i++] = 217; // shirt B
			else if (i < 26) image[i++] = 96;  // shirt C
			else if (i < 33) image[i++] = 0;   // shirt D
			else if (i < 40) image[i++] = 216; // shirt E
			else if (i < 44) image[i++] = 133; // shirt F
			else		     image[i++] = 99;  // shirt G
		}
	}
	else if (imageNum == 4) { // 1 changed shirt colour and stands to the left of goal (angled)
		self->position[0] = 2000; // x position
		self->position[1] = 1000; // y position
		self->heading[0] = 0.707;  // x component heading
		self->heading[1] = -0.707;  // y component heading
		for (int i = 0; i < image.size();i++) {
			if (i < 5)       image[i++] = 187; // shirt B
			else if (i < 20) image[i++] = 40;  // shirt C (changed)
			else if (i < 32) image[i++] = 21;  // shirt D
			else if (i < 40) image[i++] = 222; // shirt E
			else if (i < 44) image[i++] = 117; // shirt F 
			else if (i < 47) image[i++] = 114; // shirt G
			else if (i < 49) image[i++] = 23;  // shirt H
			else			 image[i++] = 52;  // shirt I
		}
	}
	else if (imageNum == 5) { // up close to right
		self->position[0] = 3000; // x position
		self->position[1] = -2000; // y position
		self->heading[0] = 1;  // x component heading
		self->heading[1] = 0;  // y component heading
		for (int i = 0; i < image.size();i++) {
			if (i < 15)      image[i++] = 15;  // shirt D
			else if (i < 30) image[i++] = 222; // shirt E
			else if (i < 45) image[i++] = 112; // shirt F
			else			 image[i++] = 103; // shirt G
		}
	}


	else if (imageNum == 6) { // Center looking at own goal
		self->position[0] = 0; // x position
		self->position[1] = 0; // y position
		self->heading[0] = -1;  // x component heading
		self->heading[1] = 0;  // y component heading
		for (int i = 0; i < image.size();i++) {
			if (i < 10)      image[i++] = 140; // shirt A
			else if (i < 30) image[i++] = 60;  // shirt B
			else if (i < 35) image[i++] = 190; // shirt C
			else if (i < 45) image[i++] = 240; // shirt D
			else			 image[i++] = 100; // shirt E
		}
	}
	else if (imageNum == 7) { // From left of goal (angled)
		self->position[0] = -2000; // x position
		self->position[1] = -1000; // y position
		self->heading[0] = -0.707;  // x component heading
		self->heading[1] = 0.707;  // y component heading
		for (int i = 0; i < image.size();i++) {
			if (i < 5)       image[i++] = 145; // shirt A
			else if (i < 35) image[i++] = 66;  // shirt B
			else if (i < 38) image[i++] = 180; // shirt C
			else if (i < 43) image[i++] = 232; // shirt D
			else if (i < 48) image[i++] = 100; // shirt E
			else			 image[i++] = 150; // shirt F
		}
	}
	else if (imageNum == 8) { // zoomed out
		self->position[0] = -500; // x position
		self->position[1] = 0; // y position
		self->heading[0] = -1;  // x component heading
		self->heading[1] = 0;  // y component heading
		for (int i = 0; i < image.size();i++) {
			if (i < 7)       image[i++] = 33;  // shirt Z
			else if (i < 14) image[i++] = 132; // shirt A
			else if (i < 29) image[i++] = 65;  // shirt B
			else if (i < 32) image[i++] = 187; // shirt C
			else if (i < 39) image[i++] = 230; // shirt D
			else if (i < 46) image[i++] = 107; // shirt E
			else 			 image[i++] = 164; // shirt F
		}
	}
	else if (imageNum == 9) { // stands to right of goal zoomed a bit(straight)
		self->position[0] = -2000; // x position
		self->position[1] = 1000; // y position
		self->heading[0] = -1;  // x component heading
		self->heading[1] = 0;  // y component heading
		for (int i = 0; i < image.size();i++) {
			if (i < 5)       image[i++] = 172; // shirt C
			else if (i < 17) image[i++] = 238; // shirt D
			else if (i < 25) image[i++] = 95;  // shirt E
			else if (i < 35) image[i++] = 155; // shirt F
			else if (i < 42) image[i++] = 45;  // shirt G
			else			 image[i++] = 97;  // shirt H
		}
	}
	else if (imageNum == 10) { // center closer (2 swapped)
		self->position[0] = -1000; // x position
		self->position[1] = 0; // y position
		self->heading[0] = -1;  // x component heading
		self->heading[1] = 0;  // y component heading
		for (int i = 0; i < image.size();i++) {
			if (i < 8)       image[i++] = 133; // shirt A
			else if (i < 30) image[i++] = 54; // shirt B
			else if (i < 41) image[i++] = 244;  // shirt D (swapped)
			else if (i < 47) image[i++] = 187; // shirt C (swapped)
			else			 image[i++] = 115;  // shirt E
		}
	}


	return image;
}
