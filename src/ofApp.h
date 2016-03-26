#pragma once
#include "ofMain.h"
#include "ofxPd.h"
#include "ofxImGui.h"
#include "ofxKinect.h"
#include "ofxAnimatableFloat.h"
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
    int validPixelCount;
    unsigned char distanceThreshold;
    ofCamera camera;
    ofxPd pd;
    ofxKinect kinect;
    ofxImGui gui;
    ofxAnimatableFloat anim;

    std::vector<ofPoint> pointCloud;
    std::vector<ofFloatColor> colorCloud;
    std::vector<ofVec3f> spectrumContour;

    //VBO

    ofVbo spectrumContourVbo;
    ofVbo pointCloudVbo;
    Scanner scanner;

    std::vector<float> spectrum;
    int recordHead;
    void setupGL();
    void audioSetup();
    void storageSetup();
    void kinectSetup();

    void updatePointCloud();

};

