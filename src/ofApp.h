#pragma once
#include "ofMain.h"
#include "ofxPd.h"
#include "ofxKinect.h"
#include "Animation.hpp"
#include "const.hpp"

class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();

    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    // audio callbacks
    void audioReceived(float * input, int bufferSize, int nChannels);
    void audioRequested(float * output, int bufferSize, int nChannels);
    void exit();

protected:
    ofEasyCam cam;
    ofxPd pd;
    ofxKinect kinect;
    std::vector<ofPoint> pointCloud;
    std::vector<ofVec3f> eqLine3D;

    //VBO
    ofVbo eqLineVbo;
    ofVbo pointCloudVbo;

    std::vector<float> spectrum;
    int recordHead;
    void setupGL();
    void audioSetup();
    void storageSetup();
    void kinectSetup();

    void drawPointCloud();
    void updatePointCloud();

};

