#pragma once
#include <ofMain.h>
class Scanner : public ofNode {

public:
    void draw();
};


inline void Scanner::draw(){
    ofColor blue = ofColor(ofColor::lightBlue);
    blue.a = 50;
    ofSetColor(blue);

    ofFill();
    ofPushMatrix();
    ofTranslate(getPosition());
    ofDrawRectangle(-1,-1, 2,2);
    ofPopMatrix();


}
