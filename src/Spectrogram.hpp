#pragma once

#include "const.hpp"
#include "ofMain.h"

class Spectrogram{
public:
    void setup();
    void update(const std::vector<float> &pdSpectrumBuffer);
    void draw(const float& sliceDist, const float& timeSpread);

protected:
    int recordHead;

    std::vector<ofPoint> spectrogramVertices;
    ofVbo spectrogramVbo;

};

inline void Spectrogram::setup(){
    spectrogramVertices.reserve(kNumVertices);
    for(int i = 0; i < kNumTimeSlices; i++){
        for(int j = 0; j < kNumBins; j++){
            float phase = static_cast<float>(j) / static_cast<float>(kNumBins);
            spectrogramVertices.emplace_back(2.0 * phase - 1.0,-1.0,0);
        }
    }
    spectrogramVbo.setVertexData(&spectrogramVertices[0],kNumVertices ,GL_DYNAMIC_DRAW);
}

inline void Spectrogram::update(const std::vector<float> &pdSpectrumBuffer){
    int pixelOffset = recordHead * kNumBins;

    for(int i = 0; i < kNumBins ;i++){
        float findex = static_cast<float>(i) * kWidthToBinRatio;
        float floor = std::floor(findex);
        float weight = findex - floor;
        int index = static_cast<int>(floor);
        spectrogramVertices[pixelOffset+i].y = pdSpectrumBuffer[i] - 1.0;
    }
    spectrogramVbo.updateVertexData(&spectrogramVertices[0], kNumVertices );
    recordHead++;
    recordHead %= kNumTimeSlices;

}

inline void Spectrogram::draw(const float& sliceDist, const float& timeSpread){
    ofSetLineWidth(1);
    ofColor fadeColor = ofColor::white;
    fadeColor.a = 255.0;
    float fadeStep = 255.0/static_cast<float>(kNumTimeSlices);
    
    for(int i = 0; i < kNumTimeSlices;i++){

        ofSetColor(fadeColor);
        float distance = sliceDist * i;
        float scale = 1 + timeSpread  * i;

        ofPushMatrix();
        glTranslatef(0,0,distance );

        glScalef(scale, 1, scale);

        // offset target row for rendering
        int readHead;
        readHead = recordHead -i;
        while(readHead < 0) readHead += kNumTimeSlices;
        int offset = readHead * kNumBins;
        spectrogramVbo.draw(GL_LINE_STRIP, offset, kNumBins);
        ofPopMatrix();
        
        fadeColor.a -= fadeStep;
    }

}
