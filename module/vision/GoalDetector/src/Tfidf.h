#include <eigen3/Eigen/Dense>


#include <nuclear>
#include "Vocab.h"
#include <vector>
#include <queue>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include "MapEntry.h"
#include <string>
#include <fstream>
#include <sstream>

// Term frequency - Inverse document frequency calculator (for fast searching over a lot of images)
class Tfidf {
	public:
		//! Constructor
  		Tfidf() {
  			clearData();
		}

		//! find out how many entries in map
		int getSize();

		void loadVocab(std::string vocabFile);
		void loadMap(std::string mapFile);
		void clearMap();

		//! Saves the map, including any new entries
  		void saveMap(std::string mapFile);

  		//! Adds to the searchable collection, returns true if successful (won't work without a vocab loaded)
		bool addDocumentToCorpus(MapEntry document);

		//! Faster version if landmarks have already been mapped to words (with the same vocab file)
  		bool addDocumentToCorpus(MapEntry document, Eigen::VectorXf tf_doc, std::vector< std::vector<float> > pixLoc);

		//! Faster version if landmarks have already been mapped to words (with the same vocab file)
  		void searchDocument(Eigen::VectorXf tf_query, 
                      		std::vector< std::vector<float> > query_pixLoc, // pixel locations of the words 
                      		std::unique_ptr<std::priority_queue<MapEntry>>& matches, 
                      		unsigned int *seed, 
                      		int n);



	private:
		void clearData(){
			T = 0;
			N = 0;
			tf.clear();
			nd.clear();
			map.clear();
			pixels.clear();
		}

		Vocab vocab;
		int N; 	// number of documents
		int T;	// number of terms
		std::vector<Eigen::VectorXf> tf;	// term frequency for each document
		std::vector< std::vector< std::vector<float> > > pixels;	// the pixel locations associated with each word
		std::vector<int> nd;	// term count by document
		Eigen::VectorXf	 ni;	// term count by word;
		std::vector<MapEntry> 	map;
		Eigen::VectorXf idf;	// corpus inverse document frequency
};