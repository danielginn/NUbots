#include <iostream>
#include "Tfidf.h"
#include <stdio.h>
#include "RANSACLine.h"
#include "Ransac.h"

# define VALID_COSINE_SCORE 0.40f // 0.42f
# define VALID_INLIERS 40 // 50

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
			addDocumentToCorpus(tempMap.at(i));
		}
		printf("Loaded map with %d entries\n",map.size());
	}
}

// Saves map, including any new entries
void Tfidf::saveMap(std::string mapFile){
  
  std::ofstream ofs(mapFile.c_str());
  {
  boost::archive::text_oarchive oa(ofs);
  oa << map;
  }

}

//! Adds to the searchable collection, return true if successful
bool Tfidf::addDocumentToCorpus(MapEntry document){
   if(vocab.getSize() != 0){
      std::unique_ptr<std::vector< std::vector<float>>> pixLoc = std::make_unique<std::vector< std::vector<float>>>(); 
      std::unique_ptr<std::vector<Ipoint>> ipoints_ptr = std::make_unique<std::vector<Ipoint>>();
      *ipoints_ptr = document.ipoints;
      Eigen::VectorXf tf_doc = vocab.mapToVec(ipoints_ptr, pixLoc);
      return addDocumentToCorpus(document, tf_doc, *pixLoc);      
   } 
   return false;
}

//! Faster version if landmarks have already been mapped to words (with the same vocab file)
bool Tfidf::addDocumentToCorpus(MapEntry document, Eigen::VectorXf tf_doc, std::vector< std::vector<float> > pixLoc){

	bool result = false;

	if(vocab.getSize() != 0){

		if(tf_doc.sum() != 0){  // don't let an empty document be added;
	    tf.push_back(tf_doc);
			pixels.push_back(pixLoc);
	    ni = ni + tf_doc;	
	    N++;  
	    nd.push_back(tf_doc.sum());
	
	    map.push_back(document);
	    //! Recalculate the inverse document frequency (log(N/ni))
	    idf = (ni.array().inverse() * N ).log();

      // Remove Inf from idf (can occur with random initialised learned words)
      for(int i=0; i<T; i++){
      	if(idf[i] == std::numeric_limits<float>::infinity()){
       		idf[i] = 0.f;
       	}
      }
      result = true;
    }
  } 
  return result;
}

//! Faster version if landmarks have already been mapped to words
void Tfidf::searchDocument(Eigen::VectorXf tf_query, 
                      	   std::vector< std::vector<float> > query_pixLoc, // pixel locations of the words 
                      	   std::unique_ptr<std::priority_queue<MapEntry>>& matches, 
                      	   unsigned int *seed,
                      	   int n){

	NUClear::clock::time_point t = NUClear::clock::now();
  printf("tf_query.sum = %.1f, N = %d\n",tf_query.sum(),N);
	if(tf_query.sum() != 0 && N != 0){ // checked the document is not empty and corpus not empty
    printf("Running Tfidf::searchDocument now.\n");
		std::priority_queue<std::pair<MapEntry, std::vector<std::vector<float>>>> queue;
		Eigen::VectorXf tfidf_query = (tf_query / tf_query.sum() ).array() * idf.array();

    // Now compute the cosines against each document -- inverted index not used since our vectors are not sparse enough
    Eigen::VectorXf tfidf_doc;
    printf("Cosine scores: (for N=%d)\n", N);
    for (int i=0; i<N; i++){
      tfidf_doc = (tf.at(i) / nd.at(i) ).array() * idf.array(); // nd[i] can't be zero or it wouldn't have been added
      map.at(i).score = tfidf_query.dot(tfidf_doc) / ( tfidf_query.norm()*tfidf_doc.norm() );
      printf("%2d. map score: %.2f\n",i,map[i].score);
      if (map.at(i).score > VALID_COSINE_SCORE) {
        printf("Cos: %f, ",map.at(i).score);
        queue.push( std::make_pair(map.at(i), pixels.at(i)));
      }
    }
    printf("Complete.\n");

    // Now do geometric validation on the best until we have enough or the queue is empty
    while(!queue.empty() && matches->size() < (unsigned int)n){
      MapEntry mapEntry = queue.top().first;
      printf("Validating Cos: %f, ",mapEntry.score);
      std::vector< std::vector<float> > pixLoc = queue.top().second;
      queue.pop();

      // Do geometric validation - first build the points to run ransac
      std::vector<Point> matchpoints;
      for (int j=0; j<T; j++){
        for(uint32_t m=0; m<pixLoc[j].size(); m++){
          for(uint32_t n=0; n<query_pixLoc[j].size(); n++){
            matchpoints.push_back(Point(pixLoc[j][m], query_pixLoc[j][n]));
          }
        }
      }

      // ransac
      RANSACLine resultLine;
      uint16_t min_points = 2;
      std::vector<bool> *con, consBuf[2];
      consBuf[0].resize(matchpoints.size());
      consBuf[1].resize(matchpoints.size());
      float slopeConstraint = 2.f; // reject scale changes more than twice or half the distance
      bool ransacresult = RANSAC::findLineConstrained(matchpoints, &con, resultLine, MATCH_ITERATIONS, 
          PIXEL_ERROR_MARGIN, min_points, consBuf, seed, slopeConstraint);

      if(ransacresult && (resultLine.t2 != 0.f)){  // check t2 but should be fixed by slope constraint anyway
        // count the inliers
        int inliers = 0;
         for (int v = 0; v < (int)matchpoints.size(); v++) {
          if ( (*con)[v]) {
            inliers++;
          }
        }
        printf("found %d inliers, %d outliers, ",inliers, (int)matchpoints.size() - inliers);
        printf("at (x,y,theta) loc: (%.1f, %.1f, %.1f)\n", mapEntry.position.x(), mapEntry.position.y(), mapEntry.position.theta());

        if(inliers >= VALID_INLIERS){
          printf("Location is valid\n");
          matches->push(mapEntry);
        }
      } 
    }
  }
  //printf("\nCalculating cosines over " << map.size() << " images, RANSAC geo validation & position adjustments took ";
    //llog(DEBUG1) << t.elapsed_us() << " us" << std::endl;
}