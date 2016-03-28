#include "ofApp.h"
using namespace pd;

void ofApp::setupGLEnvironment(){
    ofSetBackgroundColor(0);
    ofEnableDepthTest();
    ofSetVerticalSync(true);
    ofSetCircleResolution(50);
    ofSetLineWidth(0.1);
    ofEnableAlphaBlending();
}

void ofApp::setupGLCamera(){
    camera.setNearClip(0.0001);
    camera.setFarClip(1000.0);
    camera.setPosition(0.0, 0.0, -5.0);
}

void ofApp::setupGLGainContour(){
    gainContour.reserve(kKinectWidth);
    for(int i = 0; i < kKinectWidth ;i++){
        gainContour.emplace_back(static_cast<float>(i) / kHalfKinectWidthFloat-1.0, -1, 0.0);
    }
    gainContourVbo.setVertexData(&gainContour[0], kKinectWidth, GL_DYNAMIC_DRAW);
}

void ofApp::setupPointCloud(){
    pointCloudVertices = std::vector<ofPoint>(kNumKinectPixels, ofPoint(0,0,0));
    pointCloudColors = std::vector<ofFloatColor>(kNumKinectPixels, ofColor::white);

    pointCloudVbo.setVertexData(&pointCloudVertices[0],kNumVertices, GL_DYNAMIC_DRAW);
    pointCloudVbo.setColorData(&pointCloudColors[0], kNumVertices, GL_DYNAMIC_DRAW);
}



void ofApp::setupGLBuffer(){
    distanceThreshold = 100;
    sliceDist = 0.1;
    timeSpread = 0.5;

    setupGLGainContour();
    setupPointCloud();
    spectrogram.setup();
    guiEnabled = false;
    boxEnabled = false;
}

void ofApp::setupObject(){
    scanner.setPosition(0,0,0);
    scaleAnimation.reset(0.2);
    scaleAnimation.setCurve(EASE_IN);
    scaleAnimation.setRepeatType(LOOP_BACK_AND_FORTH);
    scaleAnimation.setDuration(15);
    scaleAnimation.animateTo( 2.8 );


    lightAnimation.setPosition(ofPoint(0.0, 35.0, 25.0));
    lightAnimation.setCurve(EASE_IN_EASE_OUT);
    lightAnimation.setRepeatType(LOOP_BACK_AND_FORTH);
    lightAnimation.setDuration(10);
    lightAnimation.animateTo(ofPoint(0.0, 35.0, 150.0));

    pointLight.setSpotlight();
    pointLight.setAttenuation(1.5,0.,0.);
    pointLight.setDiffuseColor(ofColor::lightGray);
    pointLight.setSpecularColor(ofColor::silver);

}

void ofApp::setupGL(){
    setupGLEnvironment();
    setupGLCamera();
    setupGLBuffer();
    setupObject();
}

void ofApp::audioSetup(){
    ofSoundStreamSetup(kNumOutput, kNumInput, this, kSampleRate, ofxPd::blockSize()*8, 3);
    pdGainBuffer = std::vector<float>(kNumBins, 0.0);
    pdSpectrumBuffer = std::vector<float>(kNumBins, 0.0);

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
    kinectSetup();
    gui.setup();
}

// update
void ofApp::updatePointCloud(){
    if(kinect.isFrameNewDepth()){
        std::for_each(gainContour.begin(), gainContour.end(), [](ofPoint & point){
            point.y = -1.0;
        });
        validPixelCount = 0;
        const ofPixels &pixels = kinect.getDepthPixels();
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
    }

    pointCloudVbo.updateColorData(&pointCloudColors[0], validPixelCount);
    pointCloudVbo.updateVertexData(&pointCloudVertices[0], validPixelCount);
    gainContourVbo.updateVertexData(&gainContour[0], kKinectWidth);

}

void ofApp::update(){
    scaleAnimation.update(1/30.0);
    lightAnimation.update(1/30.0);

    pointLight.setPosition(lightAnimation.getCurrentPosition());
    timeSpread = scaleAnimation.val();
    // read audio data and visualize
    pd.readArray("spectrum", pdSpectrumBuffer);

    for(int i = 0; i < kNumBins;i++){
        float findex = static_cast<float>(i) * widthToBinRatio;
        float floor = std::floor(findex);
        float weight = findex - floor;
        int index = static_cast<int>(floor);
        if(index >= kKinectWidth-1){
            pdGainBuffer[i] = ofMap(gainContour[kKinectWidth-1].y, -1.0, 1.0, 0.0, 1.0, true);
        }else{
            float gainLeft = gainContour[index].y;
            float gainRight = gainContour[index+1].y;
            float gainVal = (gainRight-gainLeft) * weight + gainLeft;
            pdGainBuffer[i] = ofMap(gainVal, -1.0,1.0,0.0,1.0, true);
        }
    }
    spectrogram.update(pdSpectrumBuffer);

    // read kinect data and sonificate
    kinect.update();
    updatePointCloud();
    pd.writeArray("gain", pdGainBuffer);

}

void ofApp::drawWorld(){

    camera.begin();
    camera.lookAt(ofVec3f(0,0,0));

    if(boxEnabled){
        ofNoFill();
        ofDrawBox(2,2,2);
    }

    pointCloudVbo.draw(GL_POINTS, 0, validPixelCount);

    ofSetColor(ofColor(255,255,255,125));
    gainContourVbo.draw(GL_LINE_STRIP, 0, kKinectWidth );

    ofEnableLighting();
    pointLight.enable();
    spectrogram.draw(sliceDist, timeSpread);
    pointLight.disable();

    ofDisableLighting();

    ofPushMatrix();
    ofTranslate(lightAnimation.getCurrentPosition());
    ofDrawSphere(1);
    ofPopMatrix();
    scanner.draw();
    camera.end();
}

void ofApp::drawGui(){
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
    if(ImGui::SliderFloat("z", &z, -15.0, 5.0)) changed = true;

    ImGui::SliderFloat("slice dist", &sliceDist, 0.0, 1.0);
    ImGui::SliderFloat("time spread", &timeSpread, 0.0, 5.0);

    if(changed)camera.setPosition(ofVec3f(x,y,z));
    ImGui::Text(ofToString(validPixelCount).c_str());
    gui.end();
}

void ofApp::draw(){
    ofSetColor(ofColor::white);
    drawWorld();
    if(guiEnabled)drawGui();
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

void ofApp::keyPressed(int key){
    switch (key){
    case 'b':
        boxEnabled = !boxEnabled;
        break;
    case 'g':
        guiEnabled = !guiEnabled;
        break;
    case 't':
        pd.sendBang("testTone");
        break;
    }
}

