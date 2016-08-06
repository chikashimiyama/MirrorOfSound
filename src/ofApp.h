#pragma once
#include "ofMain.h"
#include "ofxPd.h"
#include "ofxImGui.h"
#include "ofxKinect.h"
#include "ofxAnimatableFloat.h"
#include "ofxAnimatableOfPoint.h"
#include "PointCloud.hpp"
#include "Spectrogram.hpp"
#include "const.hpp"
#include "Drone.hpp"
#include "Scanner.hpp"

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

    void gainContourFromPd();

protected:
    enum class Trigger{
        Enter =0,
        Exit = 1,
        Stay = 2
    };

    Trigger trigger;
    bool guiEnabled;
    bool boxEnabled;

    int validPixelCount;
    unsigned char distanceThreshold;
    float sliceDist;
    float timeSpread;

    std::array<LightDrone, 3> lightDrones;

    CameraDrone staticCamera;
    CameraDrone insertionCamera;
    ofEasyCam camera;
    bool manualCamera;

    ofxPd pd;
    ofxImGui gui;
    ofxAnimatableFloat anim;
    ofxKinect kinect;

    std::vector<ofPoint> gainContour;
    ofVbo gainContourVbo;
    PointCloud pointCloud;

    //3D spectrogram based on Pd
    std::vector<float> pdPastSpectrumBuffer;
    std::vector<float> pdFutureSpectrumBuffer;
    std::vector<float> pdGainBuffer;
    std::vector<float> pdMaterialBuffer;

    ofxAnimatableFloat scaleAnimation;

    Scanner scanner;
    Spectrogram futureSpectrogram, pastSpectrogram;

    void setupGL();
    void audioSetup();
    void storageSetup();
    void kinectSetup();


    void setupGLEnvironment();
    void setupGLCamera();
    void setupGLBuffer();
    void setupObject();
    void drawWorld();
    void drawGui();
    void updateGainContour();

    static const std::vector<std::string> positionNames;
    void triggerInsertionCamera();
};

