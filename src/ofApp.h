#pragma once
#include "ofMain.h"
#include "ofxGui.h"
#include "ofxPd.h"
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

    Trigger trigger; // touch detection
    bool guiEnabled; // gui of debugging
    bool boxEnabled; // virtual kinect sensing field
    int validPixelCount; // how many infrared pixels detect something

    // kinect and point cloud
    ofxKinect kinect;
    PointCloud pointCloud;
    Scanner scanner;
    Spectrogram pastSpectrogram, futureSpectrogram;
    
    std::vector<ofPoint> gainContour; // passed to Pd table
    ofVbo gainContourVbo;
    
    //camera in OpenGL
    ofNode lookAtObject;
    ofxAnimatableOfPoint lookAtAnimation;

    ofCamera camera;
    ofxAnimatableOfPoint cameraAnimation;
    
    // pure data
    ofxPd pd;
    std::vector<float> pdPastSpectrumBuffer, pdFutureSpectrumBuffer;
    std::vector<float> pdFeedbackSpectrumBuffer;
    std::vector<float> pdGainBuffer;
    std::vector<float> pdMaterialBuffer;

    //GUI
    ofxPanel gui;
    ofxVec3Slider lookAtSlider;
    ofxVec3Slider cameraPosSlider;
    ofxIntSlider distThresholdSlider;
    

#pragma mark protectedFunction
    void audioSetup();
    void storageSetup();
    void kinectSetup();
    void guiSetup();

    void setupGLEnvironment();
    void setupGLCamera();
    void setupGLBuffer();
    void setupObject();
    void drawWorld();
    void drawGui();
    void updateGainContour();

};

