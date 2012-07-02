#ifndef VISIONDATAWRAPPERNUVIEW_H
#define VISIONDATAWRAPPERNUVIEW_H


#include <iostream>
#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Kinematics/Horizon.h"
#include "Infrastructure/NUSensorsData/NUSensorsData.h"
#include "Infrastructure/FieldObjects/FieldObjects.h"
//#include "Infrastructure/Jobs/JobList.h"
//#include "Infrastructure/Jobs/VisionJobs/SaveImagesJob.h"

#include "Vision/basicvisiontypes.h"
#include "Vision/VisionTypes/segmentedregion.h"
#include "Vision/VisionTools/pccamera.h"
#include "Vision/VisionTools/lookuptable.h"
#include "Vision/VisionTypes/VisionFieldObjects/ball.h"
#include "Vision/VisionTypes/VisionFieldObjects/beacon.h"
#include "Vision/VisionTypes/VisionFieldObjects/goal.h"
#include "Vision/VisionTypes/VisionFieldObjects/obstacle.h"
#include "Infrastructure/NUImage/ClassifiedImage.h"

//for virtualNUbot/Qt
#include "GLDisplay.h"

using namespace std;
//using namespace cv;
using cv::Mat;
using cv::VideoCapture;
using cv::Scalar;
using cv::namedWindow;
using cv::Vec3b;

class virtualNUbot;

class DataWrapper
{
    friend class VisionController;
    friend class VisionControlWrapper;
    friend class virtualNUbot;

public:
    
    enum DATA_ID {
        DID_IMAGE,
        DID_CLASSED_IMAGE
    };
    
    enum DEBUG_ID {
        DBID_IMAGE=0,
        DBID_H_SCANS=1,
        DBID_V_SCANS=2,
        DBID_SEGMENTS=3,
        DBID_TRANSITIONS=4,
        DBID_HORIZON=5,
        DBID_GREENHORIZON_SCANS=6,
        DBID_GREENHORIZON_FINAL=7,
        DBID_OBJECT_POINTS=8,
        DBID_FILTERED_SEGMENTS=9,
        DBID_GOALS=10,
        DBID_BEACONS=11,
        DBID_BALLS=12,
        DBID_OBSTACLES=13,
        NUMBER_OF_IDS=14
    };

    static string getIDName(DEBUG_ID id);
    static string getIDName(DATA_ID id);

    static DataWrapper* getInstance();

    //! RETRIEVAL METHODS
    const NUImage* getFrame();

    bool getCTGVector(vector<float>& ctgvector);    //for transforms
    bool getCTVector(vector<float>& ctvector);    //for transforms
    bool getCameraHeight(float& height);            //for transforms
    bool getCameraPitch(float& pitch);              //for transforms
    bool getBodyPitch(float& pitch);
    
    //! @brief Generates spoofed horizon line.
    const Horizon& getKinematicsHorizon();

    CameraSettings getCameraSettings();

    const LookUpTable& getLUT() const;
        
    //! PUBLISH METHODS
    void publish(DATA_ID id, const Mat& img);
    void publish(const vector<const VisionFieldObject*> &visual_objects);
    void publish(const VisionFieldObject* visual_object);

    void debugRefresh();
    bool debugPublish(vector<Ball> data);
    bool debugPublish(vector<Beacon> data);
    bool debugPublish(vector<Goal> data);
    bool debugPublish(vector<Obstacle> data);
    bool debugPublish(DEBUG_ID id, const vector<PointType>& data_points);
    bool debugPublish(DEBUG_ID id, const SegmentedRegion& region);
    bool debugPublish(DEBUG_ID id, const Mat& img);
    
    
private:
    DataWrapper();
    ~DataWrapper();
    //void startImageFileGroup(string filename);
    bool updateFrame();
    void postProcess();
    bool loadLUTFromFile(const string& fileName);
    int getNumFramesDropped() const {return numFramesDropped;}      //! @brief Returns the number of dropped frames since start.
    int getNumFramesProcessed() const {return numFramesProcessed;}  //! @brief Returns the number of processed frames since start.
    void saveAnImage();
    
    //for virtualnubot
    void setRawImage(const NUImage *image);
    void setSensorData(NUSensorsData* sensors);
    void setFieldObjects(FieldObjects* fieldObjects);
    void setLUT(unsigned char* vals);
    void classifyImage(ClassifiedImage &target) const;
    void classifyPreviewImage(ClassifiedImage &target,unsigned char* temp_vals) const;
    
    
private:

    static DataWrapper* instance;

    void (*display_callback)(vector< Vector2<int> >, GLDisplay::display);

    const NUImage* m_current_image;

    string LUTname;
    LookUpTable LUT;

    vector<float> m_horizon_coefficients;
    Horizon m_kinematics_horizon;
    
    //! Frame info
    double m_timestamp;
    int numFramesDropped;
    int numFramesProcessed;

    //! SavingImages:
    bool isSavingImages;
    bool isSavingImagesWithVaryingSettings;
    int numSavedImages;
    ofstream imagefile;
    ofstream sensorfile;
    CameraSettings currentSettings;

    //! Shared data objects
    NUSensorsData* sensor_data;             //! pointer to shared sensor data
    NUActionatorsData* actions;             //! pointer to shared actionators data
    FieldObjects* field_objects;            //! pointer to shared fieldobject data

};

#endif // VISIONDATAWRAPPERNUVIEW_H