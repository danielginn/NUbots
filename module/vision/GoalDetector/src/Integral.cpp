

#include "Integral.h"
#include "message/input/Image.h"
#include <stdio.h>
// Convert horizon of image to single channel 32F
void getGrayHorizon(std::unique_ptr<std::vector<float>>& result, std::shared_ptr<const message::vision::ClassifiedImage<message::vision::ObjectClass>> frame_4, int left_horizon, int right_horizon)
{

	// This is Bresenham's line algorithm
  int x0 = 0;
  int y0 = left_horizon;
	int x1 = IMAGE_WIDTH-1;
  int y1 = right_horizon;

  int dx = abs(x1-x0);
  int dy = abs(y1-y0); 

  int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;
  int err = dx-dy; 

  int vsum;
  while (1){
		// Get pixel sum
    if (x0 % SURF_SUBSAMPLE == 0){
      result->at(x0/SURF_SUBSAMPLE) = 0.0;
      vsum = 0; 

      if (x0 == 0){ // Only going to do this printout on the first time through the loop
        printf("********** CALCULATING INTEGRAL **************\n");
      }
		  for (int j=-(SURF_HORIZON_WIDTH/2); j<=SURF_HORIZON_WIDTH/2; j+=2){		
			  if( (y0 + j) >=0 && (y0 + j) < IMAGE_HEIGHT){
          vsum += result->at(x0/SURF_SUBSAMPLE) += (*frame_4->image)(x0,y0+j).y;
          if (x0 == 0){
            printf("using row %d, based on y0= %d and j= %d, then the cell is: %d\n",(y0+j),y0,j,(*frame_4->image)(x0,y0+j).y);
            printf("vsum = %d, and result[] = %0.1f\n",vsum,result->at(x0/SURF_SUBSAMPLE));	
          }
			  }
		  }
      result->at(x0/SURF_SUBSAMPLE) = static_cast<float>(vsum);
      if (x0 == 0){
        printf("Finally, result[] is set to %0.1f\n",result->at(x0/SURF_SUBSAMPLE));
      }
      
    }

    if (x0 == x1 && y0 == y1) return; // Exit loop

    int e2 = 2*err;
    if (e2 > -dy){ 
      err -= dy;
      x0 += sx;
		}
    if (e2 < dx){ 
    	err += dx;
       y0 += sy; 
    }
  }
  
}



//! Computes the 1d integral image of the specified horizon line in 32-bit grey float
void Integral(std::unique_ptr<std::vector<float>>& data, std::shared_ptr<const message::vision::ClassifiedImage<message::vision::ObjectClass>> frame_3, int left_horizon, int right_horizon)
{

  // convert the image to single channel 32f
  getGrayHorizon(data, frame_3, left_horizon, right_horizon);

  
  // one row only
  float rs = 0.0f;
  for(int j=0; j<IMAGE_WIDTH / SURF_SUBSAMPLE; j++) 
  {
    rs += data->at(j); 
    data->at(j) = rs;
  }
  
}




