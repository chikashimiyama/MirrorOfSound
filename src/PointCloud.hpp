#pragma once
#include "ofMain.h"
#include "ofxKinect.h"
#include "const.hpp"

class PointCloud{

public:
    void setup();
    void update(const ofPixels &pixels, std::vector<ofPoint> &gainContour, const float &distanceThreshold);
    void draw();
protected:
    int validPixelCount;

    std::vector<ofPoint> pointCloudVertices;
    std::vector<ofFloatColor> pointCloudColors;
    ofVbo pointCloudVbo;
};


inline void PointCloud::setup(){


    pointCloudVertices = std::vector<ofPoint>(kNumKinectPixels, ofPoint(0,0,0));
    pointCloudColors = std::vector<ofFloatColor>(kNumKinectPixels, ofColor::white);
    pointCloudVbo.setVertexData(&pointCloudVertices[0],kNumVertices, GL_DYNAMIC_DRAW);
    pointCloudVbo.setColorData(&pointCloudColors[0], kNumVertices, GL_DYNAMIC_DRAW);
}

inline void PointCloud::update(const ofPixels &pixels, std::vector<ofPoint> &gainContour, const float &distanceThreshold){

    validPixelCount = 0;
    for(int i = 0; i < kNumKinectPixels; i++){
        const unsigned char distance = pixels[i];
        if( distance > distanceThreshold){
            float y = static_cast<float>(i / kKinectWidth);
            float x = static_cast<float>(i % kKinectWidth);
            pointCloudVertices[validPixelCount].x = (x - kHalfKinectWidthFloat) / kHalfKinectWidth;
            pointCloudVertices[validPixelCount].y = (y -kHalfKinectHeightFloat) / -kHalfKinectHeightFloat;
            float z = (static_cast<float>(distance) - 128.0) / -64.0 + 1.0;
            pointCloudVertices[validPixelCount].z = z;
            if(z < 0.0){ // enter negative side
                pointCloudColors[validPixelCount] = ofColor::white;
                float max = gainContour[x].y;
                if(max < y){
                   gainContour[x].y = pointCloudVertices[validPixelCount].y;
                }
            }else{
                 pointCloudColors[validPixelCount] = ofColor::gray;
            }
            validPixelCount++;
        }
    }
    pointCloudVbo.updateColorData(&pointCloudColors[0], validPixelCount);
    pointCloudVbo.updateVertexData(&pointCloudVertices[0], validPixelCount);
}

inline void PointCloud::draw(){
    ofDisableLighting();
    pointCloudVbo.draw(GL_POINTS, 0, validPixelCount);
    ofEnableLighting();

}
