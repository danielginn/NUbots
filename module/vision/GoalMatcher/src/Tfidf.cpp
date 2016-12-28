#include <iostream>
#include "Tfidf.h"
#include <stdio.h>


// Loads vocab ready for use
void Tfidf::loadVocab(std::string vocabFile){

  // clear everything
  clearData();

  // load the vocab file
  vocab.loadVocabFile(vocabFile);
  T = vocab.getSize();
  ni = Eigen::VectorXf::Zero(T);	
  printf("Loaded vocab of %d words\n",T); 

}

int Tfidf::getSize(){
   return N;
}

void Tfidf::clearMap(){
   	clearData();
   	T = vocab.getSize();
   	ni = Eigen::VectorXf::Zero(T);	
}

// Loads map ready for use (needs a vocab first)
/*
void Tfidf::loadMap(std::string mapFile){

	if(T!=0){
		// clear previous map
		clearMap();

		// load the map file
		std::vector<MapEntry> tempMap;
		std::ifstream ifs(mapFile.c_str());
		if (ifs.is_open()){
			boost::archive::text_iarchive ia(ifs);
			ia >> tempMap;
		} else {
			throw std::runtime_error("error opening map file in tfidf");
		}
		for (int i=0; i<(int)tempMap.size(); i++){
			addDocumentToCorpus(tempMap[i]);
		}
		printf("Loaded map with %d entries\n",map.size());
	}
}
*/

//! Faster version if landmarks have already been mapped to words
void Tfidf::searchDocument(Eigen::VectorXf tf_query, 
                      	   std::vector< std::vector<float> > query_pixLoc, // pixel locations of the words 
                      	   std::unique_ptr<std::priority_queue<MapEntry>>& matches, 
                      	   unsigned int seed,
                      	   int n){

	NUClear::clock::time_point t = NUClear::clock::now();

	if(tf_query.sum() != 0 && N != 0){ // checked the document is not empty and corpus not empty
		//std::priority_queue< std::pair<MapEntry, std::vector <std::vector<float> > > > queue;
		//Eigen::VectorXf tfidf_query = (tf_query / tf_query.sum() ).cwise() * idf;










	}
}