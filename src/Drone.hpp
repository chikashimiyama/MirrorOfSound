#pragma once

#include <chrono>
#include "ofMain.h"
#include "ofxAnimatableOfPoint.h"

class Drone{
public:
    bool isRunning();
    void setRepeatType(AnimRepeat repeat);
    void setCurve(AnimCurve curve);
    void storePosition(std::string name, ofPoint point);
    virtual void startMovement(std::string from, std::string to, float duration);
    virtual void stopMovement();
    virtual void stepForward();

protected:
    ofxAnimatableOfPoint movingPos;
    std::map<std::string, ofPoint> positionMap;
};

inline bool Drone::isRunning() {
    return movingPos.isAnimating();
}

inline void Drone::startMovement(std::string from, std::string to, float duration){
    ofPoint startPos = (*positionMap.find(std::move(from))).second;
    ofPoint endPos = (*positionMap.find(std::move(to))).second;
    ofLog() << startPos << "-->" <<endPos << " dur:" << duration;
    movingPos.setPosition(startPos);
    movingPos.setDuration(duration);
    movingPos.animateTo(endPos);
}

inline void Drone::stopMovement(){
    movingPos.reset();
}

inline void Drone::storePosition(std::string name, ofPoint point){
   positionMap.emplace(std::move(name), std::move(point));
}

inline void Drone::stepForward(){
    movingPos.update(1.0/30.0);
}

inline void Drone::setRepeatType(AnimRepeat repeat){
    movingPos.setRepeatType(std::move(repeat));
}

inline void Drone::setCurve(AnimCurve curve){
    movingPos.setCurve(std::move(curve));
}


// Camera Drone
class CameraDrone: public ofCamera, public Drone {
public:
    CameraDrone();
    ~CameraDrone();
    void setup();
    virtual void startMovement(std::string from, std::string to, float duration);
    virtual void stopMovement();
    void stepForward();
    void setHysteresis(int);
    void setLookAt(ofNode node);
protected:
    int hysteresis;
    int timeDifference();
    std::chrono::time_point<std::chrono::steady_clock> lastCommandTime;
    ofNode lookAtNode;
};

inline int CameraDrone::timeDifference(){
    auto now = std::chrono::steady_clock::now();
    return (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCommandTime)).count();
}

inline void CameraDrone::setHysteresis(int hys ){
    hysteresis = std::move(hys);
}

inline void CameraDrone::startMovement(std::string from, std::string to, float duration){
    if(timeDifference() >= hysteresis){
        Drone::startMovement(from, to, duration);
        lastCommandTime = std::chrono::steady_clock::now();
    }
}

inline void CameraDrone::stopMovement(){
    auto now = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCommandTime);
    if(timeDifference() >= hysteresis){
        Drone::stopMovement();
        lastCommandTime = std::chrono::steady_clock::now();
    }
}

inline void CameraDrone::setLookAt(ofNode node){
    lookAtNode = node;
}


inline void CameraDrone::stepForward(){
    Drone::stepForward();
    lookAt(lookAtNode);
    setPosition(movingPos.getCurrentPosition());
}


// Light Drone
class LightDrone: public ofLight, public Drone {
public:
    LightDrone();
    ~LightDrone();
    void setup();
    void stepForward();

    void draw();
};

inline void LightDrone::stepForward(){
    Drone::stepForward();
    setPosition(movingPos.getCurrentPosition());
}

inline void LightDrone::draw(){
    ofSetColor(ofColor::red);
    ofPushMatrix();
    ofTranslate(movingPos.getCurrentPosition());
    ofDrawSphere(1.0);
    ofPopMatrix();


}
