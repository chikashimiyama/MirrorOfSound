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

    void keyPressed(int key);
    void setupGLGainContour();

protected:
    bool guiEnabled;
    bool boxEnabled;

    int validPixelCount;
    unsigned char distanceThreshold;
    float sliceDist;
    float timeSpread;

    ofCamera camera;
    ofxPd pd;
    ofxKinect kinect;
    ofxImGui gui;
    ofxAnimatableFloat anim;

    //kinect
    std::vector<ofPoint> pointCloudVertices;
    std::vector<ofFloatColor> pointCloudColors;
    ofVbo pointCloudVbo;

    std::vector<ofPoint> gainContour;
    ofVbo gainContourVbo;

    //3D spectrogram based on Pd
    std::vector<float> pdSpectrumBuffer;
    std::vector<float> pdGainBuffer;
    std::vector<float> pdMaterialBuffer;


    std::vector<ofPoint> afterSpectrogram;
    ofVbo afterSpectrogramVbo;

    //VBO
    Scanner scanner;

    int recordHead;
    void setupGL();
    void audioSetup();
    void storageSetup();
    void kinectSetup();

    void updatePointCloud();

    void setupGLEnvironment();
    void setupGLCamera();
    void setupGLBuffer();
    void setupObject();
    void drawWorld();
    void drawGui();
    void drawSpectrogram();
    void updateSpectrogram();
    void setupPointCloud();
    void setupSpectrograms();
};

