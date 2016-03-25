#pragma once
#include "ofMain.h"
#include "ofxPd.h"
#include "ofxKinect.h"
#include "Animation.hpp"
#include "const.hpp"
#include "Scanner.h"

class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();

    void windowResized(int w, int h);
    void gotMessage(ofMessage msg);
    // audio callbacks
    void audioReceived(float * input, int bufferSize, int nChannels);
    void audioRequested(float * output, int bufferSize, int nChannels);
    void exit();

protected:
    ofCamera camera;
    ofxPd pd;
    ofxKinect kinect;
    AnimationManager aManager;

    std::vector<ofPoint> pointCloud;
    std::vector<ofVec3f> eqLine3D;

    //VBO
    ofVbo eqLineVbo;
    ofVbo pointCloudVbo;

    Scanner scanner;

    std::vector<float> spectrum;
    int recordHead;
    void setupGL();
    void audioSetup();
    void storageSetup();
    void kinectSetup();

    void drawPointCloud();
    void updatePointCloud();

};

