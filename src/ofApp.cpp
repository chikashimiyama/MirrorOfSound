#include "ofApp.h"
using namespace pd;



void ofApp::setupGL(){
    ofSetBackgroundColor(0);
    ofEnableDepthTest();
    ofSetVerticalSync(true);
    ofSetCircleResolution(50);
    ofSetLineWidth(0.1);
    ofEnableAlphaBlending();

    camera.setNearClip(0.0001);
    camera.setFarClip(120.0);
    camera.setPosition(0.0, 0.0, -5.0);

    distanceThreshold = 100;
    for(int i = 0; i < kKinectWidth; i++){
        spectrumContour.push_back(ofPoint(static_cast<float>(i)/-kHalfKinectWidth +1.0,-1,-1));
    }

    for(int i = 0; i < kNumKinectPixels; i++){
        pointCloud.push_back(ofPoint(0,0,0));
        colorCloud.push_back(ofColor::white);
    }

    scanner.setPosition(0,0,-1);
}

void ofApp::audioSetup(){
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
    gui.setup();

}

void ofApp::storageSetup(){
    pointCloudVbo.setVertexData(&pointCloud[0], kNumVertices, GL_DYNAMIC_DRAW);
    pointCloudVbo.setColorData(&colorCloud[0], kNumVertices, GL_DYNAMIC_DRAW);
    spectrumContourVbo.setVertexData(&spectrumContour[0], kKinectWidth, GL_DYNAMIC_DRAW);
}

void ofApp::updatePointCloud(){
    if(kinect.isFrameNewDepth()){
        std::for_each(spectrumContour.begin(), spectrumContour.end(), [](ofPoint & point){
            point.y = -1.0;
        });
        validPixelCount = 0;
        const ofPixels &pixels = kinect.getDepthPixels();
        for(int i = 0; i < kNumKinectPixels; i++){
            const unsigned char distance = pixels[i];
            if( distance > distanceThreshold){
                // valid
                float y = static_cast<float>(i / kKinectWidth);
                float x = static_cast<float>(i % kKinectWidth);
                pointCloud[validPixelCount].x = (x - kHalfKinectWidthFloat) / -kHalfKinectWidth;
                pointCloud[validPixelCount].y = (y -kHalfKinectHeightFloat) / -kHalfKinectHeightFloat;
                float z = (static_cast<float>(distance) - 128.0) / -64.0;
                pointCloud[validPixelCount].z = z;
                if(z < -1.0){
                    colorCloud[validPixelCount] = ofColor::orange;
                    float max = spectrumContour[x].y;
                    if(max < y){
                        spectrumContour[x].y = pointCloud[validPixelCount].y;
                    }
                }else{
                    colorCloud[validPixelCount] = ofColor::white;
                }

                validPixelCount++;
           }
        }
    }
    pointCloudVbo.updateColorData(&colorCloud[0], validPixelCount);
    pointCloudVbo.updateVertexData(&pointCloud[0], validPixelCount);
    spectrumContourVbo.updateVertexData(&spectrumContour[0], kKinectWidth);
}

void ofApp::update(){
    kinect.update();
    updatePointCloud();
    pd.readArray("spectrum", spectrum);
}

void ofApp::draw(){
    ofSetColor(ofColor::white);

    camera.begin();
    camera.lookAt(ofVec3f(0,0,0));
    ofNoFill();
    ofDrawBox(2,2,2);
    pointCloudVbo.draw(GL_POINTS, 0, validPixelCount);
    ofSetColor(ofColor::red);
    spectrumContourVbo.draw(GL_LINE_STRIP, 0, kKinectWidth );

    scanner.draw();

    camera.end();

    gui.begin();
    auto pos = camera.getPosition();

    float x = pos.x;
    float y = pos.y;
    float z = pos.z;
    int thresh = distanceThreshold;
    if(ImGui::SliderInt("distThresh", &thresh, 0, 255)){
        distanceThreshold = thresh;
    }

    bool changed = false;
    if(ImGui::SliderFloat("x", &x, -3.0, 3.0)) changed = true;
    if(ImGui::SliderFloat("y", &y, -3.0, 3.0)) changed = true;
    if(ImGui::SliderFloat("z", &z, -5.0, 5.0)) changed = true;
    if(changed)camera.setPosition(ofVec3f(x,y,z));
    ImGui::Text(ofToString(validPixelCount).c_str());
    gui.end();
}

void ofApp::windowResized(int w, int h){
}

void ofApp::gotMessage(ofMessage msg){
}

void ofApp::exit(){
    kinect.setCameraTiltAngle(0); // zero the tilt on exit
    kinect.close();
}

void ofApp::audioReceived(float * input, int bufferSize, int nChannels) {
   pd.audioIn(input, bufferSize, nChannels);
}

void ofApp::audioRequested(float * output, int bufferSize, int nChannels) {
    pd.audioOut(output, bufferSize, nChannels);
}
