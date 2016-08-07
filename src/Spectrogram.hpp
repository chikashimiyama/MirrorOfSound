#pragma once

#include "const.hpp"
#include "ofMain.h"

class Spectrogram{
public:
    void setup(bool rev = false, ofColor col = ofColor::white);
    void update(const std::vector<float> &pdSpectrumBuffer);
    void draw();

protected:
    int recordHead;
    std::vector<ofPoint> spectrogramVertices;
    ofVbo spectrogramVbo;
    bool reverse;
    ofFloatColor color;
};

inline void Spectrogram::setup(bool rev, ofColor col){
    reverse = rev;
    color = col;
    spectrogramVertices.reserve(kNumVertices);
    
    for(int i = 0; i < kNumTimeSlices; i++){
        float alpha = rev ? kNumTimeSlices * i :(1.0 - kRNumTimeSlices * i);
        
        for(int j = 0; j < kNumBins; j++){
            float phase = static_cast<float>(j) / static_cast<float>(kNumBins);
            spectrogramVertices.emplace_back(2.0 * phase - 1.0,-1.0,0);
        }
    }
    spectrogramVbo.setVertexData(&spectrogramVertices[0],kNumVertices ,GL_DYNAMIC_DRAW);
}

inline void Spectrogram::update(const std::vector<float> &pdSpectrumBuffer){
    
    recordHead++;
    recordHead %= kNumTimeSlices;

    int pixelOffset = recordHead * kNumBins;
    int index = reverse ? kNumBins-1 : 0;
    int incr = reverse? -1 : 1;

    for(int i = 0; i < kNumBins ;i++){
        spectrogramVertices[pixelOffset+i].y = pdSpectrumBuffer[index] - 1.0;
        index += incr;
    }
    spectrogramVbo.updateVertexData(&spectrogramVertices[0], kNumVertices );
}

inline void Spectrogram::draw(){
    ofSetLineWidth(2);
    float step = 0.0;
    for(int i = 0; i < kNumTimeSlices;i++){

        float distance = reverse?  kMaxDistance - kDistanceBetweenLines * i : kDistanceBetweenLines * i;
        float scale = 1 + (reverse? kMaxSpread - kLineSpread * i: kLineSpread  * i);
        float alpha = reverse ? step : 1.0 - step;
        
        ofPushMatrix();
        ofSetColor(ofFloatColor(color.r,color.g,color.b, alpha));
        glTranslatef(0,0,distance );
        glScalef(scale, 1, scale);
        
        int readHead = recordHead -i;
        if(readHead < 0) readHead += kNumTimeSlices;

        int offset = readHead * kNumBins;
        spectrogramVbo.draw(GL_LINE_STRIP, offset, kNumBins);
        ofPopMatrix();
        step += kRNumTimeSlices/2;
    }

}
