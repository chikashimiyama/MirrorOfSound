#include "Drone.hpp"

CameraDrone::CameraDrone(){
    movingPos.setRepeatType(PLAY_ONCE);
    movingPos.setCurve(EASE_IN);
    setFarClip(10000);
    setNearClip(0.0001);
    lastCommandTime = std::chrono::steady_clock::now();
    hysteresis = -1;
}

CameraDrone::~CameraDrone(){}

LightDrone::LightDrone(){
    movingPos.setRepeatType(LOOP);
    movingPos.setCurve(LINEAR);

    setSpotlight();
    setOrientation(ofVec3f(0,-1,0));
    //setSpotlightCutOff(ofGetMouseX());
    setAttenuation(0.8, 0.1, 0);
    lookAt(ofPoint(0,-1,0));
    setAmbientColor(ofColor::gray);
    setDiffuseColor(ofColor::lightGray);
    setSpecularColor(ofColor::white);
}

LightDrone::~LightDrone(){}


