#pragma once
#include <ofMain.h>
class Scanner : public ofNode {

public:
    void draw();
};


inline void Scanner::draw(){
    ofColor orange = ofColor(ofColor::orange);
    orange.a = 25;
    ofSetColor(orange);

    ofFill();
    ofPushMatrix();
    ofTranslate(getPosition());
    ofDrawRectangle(-1,-1, 2,2);
    ofPopMatrix();


}
