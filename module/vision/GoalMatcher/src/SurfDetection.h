
#ifndef MODULES_VISION_SURFDECTION_H
#define MODULES_VISION_SURFDECTION_H

#include <nuclear>

#include "message/vision/ClassifiedImage.h"
#include "GoalMatcherConstants.h"
#include "Ipoint.h"

namespace module {
namespace vision {

	
	class SurfDetection {
	   public:



	   	SurfDetection(const message::vision::ClassifiedImage<message::vision::ObjectClass>& frame_2);
	   				  

		void findLandmarks(std::unique_ptr<std::vector<Ipoint>>& landmarks);
	
	    //! Loads vocab ready for use, returns the size of the vocab
	    //int loadVocab(std::string vocabFile);

	    // is a vocab loaded
	    //bool vocabLoaded();

		// find interest points


	   private:

	 	//---------------- Private Functions -----------------//
	   
		//! Get all descriptors
	    void getHorizonDescriptors();		

		//! Our modified descriptor
	    void getHorizonDescriptor();

	    //! Calculate Haar wavelet response
		inline float haar(int column, int size);

		//! Round float to nearest integer
			
		inline int fRound(float flt)
		{
			return (int) floor(flt+0.5f);
		}
			

	    //---------------- Private Variables -----------------//

	    // the Vocab used to map features to visual words 
	    //Vocab vocab;

		// Landmarks pointer
		std::unique_ptr<std::vector<Ipoint>> landmarks = std::make_unique<std::vector<Ipoint>>();
				
		// the Classified image structure
		const message::vision::ClassifiedImage<message::vision::ObjectClass>& frame_p;		
			
		// the integral horizon image
		std::unique_ptr<std::vector<float>> int_img = std::make_unique<std::vector<float>>(IMAGE_WIDTH / SURF_SUBSAMPLE,0.0);

	    //! Index of current Ipoint in the vector
	    int index;
	      
	};
}
}
#endif