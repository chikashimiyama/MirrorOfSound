#include "ofApp.h"
using namespace pd;

const std::vector<std::string> ofApp::positionNames = {"Hand", "FarBack","Above","GroundFar","Below","Distant","Fish","Center", "Subjective"};

void ofApp::setupGLEnvironment(){
    ofSetBackgroundColor(0);
    ofEnableDepthTest();
    ofSetVerticalSync(true);
    ofSetCircleResolution(50);
    ofSetLineWidth(0.1);
    ofEnableAlphaBlending();
}

void ofApp::setupGLCamera(){
    staticCamera.storePosition("SideA", ofPoint(-10, 1, 10));
    staticCamera.storePosition("SideB", ofPoint(-9, 1, 11));
    targetObject.setPosition(ofPoint(0, 0, 5));
    staticCamera.setLookAt(targetObject);
    staticCamera.setCurve(EASE_IN_EASE_OUT);
    staticCamera.setRepeatType(LOOP_BACK_AND_FORTH);
    staticCamera.startMovement("SideA", "SideB", 30.0);
    
    camera.setPosition(-10, 1, -1);
    targetObject.setPosition(ofPoint(0, 1, 5));
    camera.lookAt(targetObject);
}

void ofApp::setupGLBuffer(){
    distanceThreshold = 100;
    sliceDist = 1.0;
    timeSpread = 1.0;

    pointCloud.setup();
    pastSpectrogram.setup();
    guiEnabled = false;
    boxEnabled = false;

    gainContour.reserve(kKinectWidth);
    for(int i = 0; i < kKinectWidth ;i++){
        gainContour.emplace_back(static_cast<float>(i) / kHalfKinectWidthFloat-1.0, -1, 0.0);
    }
    gainContourVbo.setVertexData(&gainContour[0], kKinectWidth, GL_DYNAMIC_DRAW);
}

void ofApp::setupGL(){
    setupGLEnvironment();
    setupGLCamera();
    setupGLBuffer();
}

void ofApp::audioSetup(){
    ofSoundStreamSetup(kNumOutput, kNumInput, this, kSampleRate, ofxPd::blockSize()*8, 3);
    pdGainBuffer = std::vector<float>(kNumBins, 0.0);
    pdFeedbackSpectrumBuffer = std::vector<float>(kNumBins, 0.0);
    pdPastSpectrumBuffer = std::vector<float>(kNumBins, 0.0);

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
    kinectSetup();
    setupGL();
    audioSetup();
    kinectSetup();
    pointCloud.setup();
    gui.setup();
    trigger = Trigger::Stay;
}

// update


void ofApp::updateGainContour(){
    static Trigger previousStat;
    float gainSum = 0;
    static float previousGainSum = 0;
    for(int i = 0; i < kNumBins;i++){
        float findex = static_cast<float>(i) * kWidthToBinRatio;
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
        gainSum += pdGainBuffer[i];
    }

    Trigger status = gainSum > kEnterThreshold ? Trigger::Enter : Trigger::Exit;
    trigger = (previousStat == status) ? Trigger::Stay : status;
    previousStat = status;
}

void ofApp::update(){
    updateGainContour();
    pd.writeArray("gain", pdGainBuffer);
    gainContourVbo.updateVertexData(&gainContour[0], kKinectWidth);

    if(trigger == Trigger::Enter){

    }else if(trigger == Trigger::Exit){
        
    }

    staticCamera.stepForward();
    timeSpread = 1.0;

    // read spectrum
    pd.readArray("pastSpectrum", pdPastSpectrumBuffer);
    pastSpectrogram.update(pdPastSpectrumBuffer);

    // read kinect data and sonificate
    kinect.update();
    if(kinect.isFrameNewDepth()){
        std::for_each(gainContour.begin(), gainContour.end(), [](ofPoint & point){
            point.y = -1.0;
        });

        pointCloud.update(kinect.getDepthPixels(),gainContour,distanceThreshold);
    }

}

void ofApp::drawWorld(){

    camera.begin();

    if(boxEnabled){ ofNoFill();ofDrawBox(2,2,2);}
    pointCloud.draw();
    gainContourVbo.draw(GL_LINE_STRIP, 0, kKinectWidth );
    pastSpectrogram.draw(sliceDist, timeSpread);

    ofPushMatrix();
    ofRotateY(180);
    ofPopMatrix();
    scanner.draw();
    camera.end();


}

void ofApp::drawGui(){
    gui.begin();
    int thresh = distanceThreshold;
    if(ImGui::SliderInt("distThresh", &thresh, 0, 255)){
        distanceThreshold = thresh;
    }
    ImGui::SliderFloat("slice dist", &sliceDist, 0.0, 1.0);
    ImGui::SliderFloat("time spread", &timeSpread, 0.0, 5.0);

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

