#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>

#include "Vocab.h"


void Vocab::loadVocabFile(std::string filename){
  
   	pos_words.clear();
   	neg_words.clear();
	  pos_stop_words.clear();
	  neg_stop_words.clear();

    std::ifstream ifs(filename.c_str());
   	if (ifs.is_open()){  
    	boost::archive::text_iarchive ia(ifs);
      ia >> pos_words;
	   	ia >> neg_words;
	   	ia >> pos_stop_words;
	   	ia >> neg_stop_words;
	   	
    } else {
      	throw std::runtime_error("error opening vocab file");
   	}

   	vec_length = pos_words.size() + neg_words.size();
   	
}


//! Map a set of interest points to a sparse term frequency vector while preserving 
// the pixel locations of the interest points

Eigen::VectorXf Vocab::mapToVec(std::unique_ptr<std::vector<Ipoint>> &ipts, std::unique_ptr<std::vector<std::vector<float>>> &pixel_location){

	  pixel_location->clear();
   	pixel_location->resize(vec_length); // will call default constructor of std::vector<float>
   	Eigen::VectorXf vec;
   	vec = Eigen::VectorXf::Zero(vec_length);
    for(unsigned int i=0; i<ipts->size(); i++){
        bool stop = false;
        Ipoint ipt = ipts->at(i);
        float best = std::numeric_limits<float>::max();
        float dist;
        int word = 0;

        // Search either pos or neg laplacians, not both
        if(ipt.laplacian == 1){
            for(int j=0; j<(int)pos_words.size(); j++){
                dist = pos_words[j] - ipt;
                if(dist < best){
                    best = dist;
                    word = j;
                }
            }   
            // now search stop list
            for(int j=0; j<(int)pos_stop_words.size(); j++){
                dist = pos_stop_words[j] - ipt;
                if(dist < best){
                    stop = true;
                    break;
                }
            }            
        } else {
            for(int j=0; j<(int)neg_words.size(); j++){
                dist = neg_words[j] - ipt;
                if(dist < best){
                    best = dist;
                    word = j+pos_words.size();
                } 
            }    
            // now search stop list
            for(int j=0; j<(int)neg_stop_words.size(); j++){
                dist = neg_stop_words[j] - ipt;
                if(dist < best){
                    stop = true;
                    break;
                } 
            }           
        }
        if (!stop) {
            vec[word]=vec[word]+1.f;
            pixel_location->at(word).push_back(ipt.x); 
        }  
    }
    return vec;
}

