#pragma once
#include <ofMain.h>

class Scanner : public ofNode {
public:
    void update(unsigned char &fillAlpha, unsigned char &frameAlpha );
    void draw();

protected:
    unsigned char fill;
    unsigned char frame;
};

inline void Scanner::update(unsigned char &fillAlpha, unsigned char &frameAlpha){
    fill = fillAlpha;
    frame = frameAlpha;
}

inline void Scanner::draw(){

    ofPushMatrix();
    ofTranslate(getPosition());
    
    ofFill();
    ofSetColor(100,200,255,fill);
    ofDrawRectangle(-1,-1, 2,2);

    ofNoFill();
    ofSetColor(255,255,255,frame);
    ofDrawRectangle(-1,-1, 2,2);
    ofPopMatrix();
}


