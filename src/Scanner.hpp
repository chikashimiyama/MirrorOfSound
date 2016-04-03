#pragma once
#include <ofMain.h>

class Scanner : public ofNode {
public:
    void draw();
protected:
    ofMaterial material;
};



inline void Scanner::draw(){


    ofFill();
    ofPushMatrix();
    ofTranslate(getPosition());
    ofDisableLighting();
    ofSetColor(100,200,255,100);
    ofDrawRectangle(-1,-1, 2,2);
    ofEnableLighting();
    ofPopMatrix();
}
