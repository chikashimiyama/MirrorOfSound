#pragma once
#include <ofMain.h>
class Scanner : public ofNode {

public:
    void draw();
};


inline void Scanner::draw(){
    ofColor green = ofColor(ofColor::green);
    green.a = 50;
    ofSetColor(green);

    ofFill();
    ofPushMatrix();
    ofTranslate(getPosition());
    ofDrawRectangle(-1,-1, 2,2);
    ofPopMatrix();


}
