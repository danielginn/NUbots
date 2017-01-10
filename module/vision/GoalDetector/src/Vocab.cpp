#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <stdio.h>

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
    //printf("Mapping to Vec:\n");
    for(unsigned int i=0; i<ipts->size(); i++){
        //printf("For word %d:\n",i);
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
                    //printf("found a better positive laplacian match\n");
                }
            }   
            // now search stop list
            for(int j=0; j<(int)pos_stop_words.size(); j++){
                dist = pos_stop_words[j] - ipt;
                if(dist < best){
                    stop = true;
                    //printf("This word was on the stoplist\n");
                    break;
                }
            }            
        } else {
            for(int j=0; j<(int)neg_words.size(); j++){
                dist = neg_words[j] - ipt;
                if(dist < best){
                    best = dist;
                    word = j+pos_words.size();
                    //printf("found a better negative laplacian match\n");
                } 
            }    
            // now search stop list
            for(int j=0; j<(int)neg_stop_words.size(); j++){
                dist = neg_stop_words[j] - ipt;
                if(dist < best){
                    stop = true;
                    //printf("This word was on the stoplist\n");
                    break;
                } 
            }           
        }
        if (!stop) {
            vec[word]=vec[word]+1.f;
            pixel_location->at(word).push_back(ipt.x); 
        }  
    }
    /*
    printf("\nfrequency Histogram\n");
    int blank_count = 0;
    for (int j=0; j < vec_length; j++){
        if (vec[j] < 0.5) {
            ++blank_count;
        }
        else {
            if (blank_count > 0) {
                printf(".\n.\n.\n(%d)\n.\n.\n.\n",blank_count);
                blank_count = 0;
            }
            printf("------ (%0.0f)\n",vec[j]);
        }
    }
    if (blank_count > 0) {
        printf(".\n.\n.\n(%d)\n.\n.\n.\n",blank_count);
    }
    */
    return vec;
}

