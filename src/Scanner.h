#pragma once
#include <ofMain.h>
class Scanner : public ofNode {

public:
    void draw();
};


inline void Scanner::draw(){
    ofSetColor(ofColor::orange);
    ofFill();
    ofPushMatrix();
    ofTranslate(getPosition());
    ofDrawRectangle(-1,1, 2,2);
    ofPopMatrix();
}
