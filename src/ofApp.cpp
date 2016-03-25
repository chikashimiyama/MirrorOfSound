#include "ofApp.h"

using namespace pd;

void ofApp::setupGL()
{
    ofSetBackgroundColor(0);
    ofEnableDepthTest();
    ofSetVerticalSync(true);
    ofSetCircleResolution(50);
    ofSetLineWidth(0.1);
    ofEnableAlphaBlending();
}

void ofApp::audioSetup()
{
    ofSoundStreamSetup(kNumOutput, kNumInput, this, kSampleRate, ofxPd::blockSize()*8, 3);
    pd.init(kNumOutput, kNumInput, kSampleRate);
    pd.openPatch("spectrum.pd");
    pd.start();
}

void ofApp::kinectSetup(){
    kinect.init();
    kinect.open();if(kinect.isConnected()) {
        ofLogNotice() << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
        ofLogNotice() << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
        ofLogNotice() << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
        ofLogNotice() << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
    }else{
        ofLog() << "kinect not found";
        ofExit(1);
    }
}

void ofApp::setup(){
    setupGL();
    audioSetup();
    storageSetup();
    kinectSetup();
}

void ofApp::storageSetup(){

    for(int v = 0; v < kKinectHeight; v++){
        for(int u = 0; u < kKinectWidth;u++){
            pointCloud.push_back(ofPoint(static_cast<float>(u)/ kHalfKinectWidth -1.0,
                                         static_cast<float>(v)/kHalfKinectHeight -1.0, 0.0 ));
        }
    }

    pointCloudVbo.setVertexData(&pointCloud[0], kNumVertices, GL_DYNAMIC_DRAW);
    eqLineVbo.setVertexData(&eqLine3D[0], kNumBins, GL_DYNAMIC_DRAW);
}

void ofApp::updatePointCloud()
{
    if(kinect.isFrameNew()){
        const ofPixels &depthPixels =  kinect.getDepthPixels();
        for(int v = 0; v < kKinectHeight; v++){
            int vOffset = v * kKinectWidth;
            for(int u = 0; u < kKinectWidth;u++){
                int uOffset = vOffset + u;
                pointCloud[uOffset].z = (static_cast<float>(depthPixels[uOffset]) -128.0) / 128.0;
                if(pointCloud[uOffset].z < -0.9) pointCloud[uOffset].z = -100.0;
            }
        }
    }
    pointCloudVbo.updateVertexData(&pointCloud[0], kNumVertices);
}

void ofApp::update(){

    kinect.update();
    recordHead++;
    recordHead %= kNumTimeSlices; // increment + wrap

    pd.readArray("spectrum", spectrum);
    updatePointCloud();
    eqLineVbo.updateVertexData(&eqLine3D[0], kNumBins);

}

void ofApp::draw(){
    ofSetColor(ofColor::white);
    cam.setNearClip(0.0001);
    cam.begin();
    drawPointCloud();
    cam.end();
    eqLineVbo.draw(GL_LINE_STRIP, 0, kNumBins);
}


void ofApp::drawPointCloud(){
    pointCloudVbo.draw(GL_POINTS, 0, kNumKinectPixels);
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}

void ofApp::exit(){
    kinect.setCameraTiltAngle(0); // zero the tilt on exit
    kinect.close();
}

void ofApp::audioReceived(float * input, int bufferSize, int nChannels) {
   pd.audioIn(input, bufferSize, nChannels);
}

//--------------------------------------------------------------
void ofApp::audioRequested(float * output, int bufferSize, int nChannels) {
    pd.audioOut(output, bufferSize, nChannels);
}
