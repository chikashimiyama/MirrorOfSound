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
    staticCamera.storePosition("Back", ofPoint(5, 3, 10));
    staticCamera.storePosition("Front", ofPoint(-5, 1, 6));
    staticCamera.setCurve(EASE_IN_EASE_OUT);
    staticCamera.setRepeatType(LOOP_BACK_AND_FORTH);
    staticCamera.startMovement("Back", "Front", 30.0);

    insertionCamera.storePosition("Hand", ofPoint(5,2,0.5));
    insertionCamera.storePosition("FarBack", ofPoint(5,5,150));
    insertionCamera.storePosition("Above", ofPoint(-1,50,1));
    insertionCamera.storePosition("GroundFar", ofPoint(0,0, 10));
    insertionCamera.storePosition("Below", ofPoint(1,-10,10));
    insertionCamera.storePosition("Distant", ofPoint(-25,-15,50));
    insertionCamera.storePosition("Fish", ofPoint(100,-5,10));
    insertionCamera.storePosition("Center", ofPoint(10,10,10));
    insertionCamera.storePosition("Subjective", ofPoint(1,2,5));
    insertionCamera.setHysteresis(5000);

    lightDrones[0].storePosition("FarAbove", ofPoint(0.0, 52.0, -30.0));
    lightDrones[0].storePosition("NearAbove", ofPoint(0.0, 52.0, 30.0));
    lightDrones[0].startMovement("FarAbove", "NearAbove", 30);

    lightDrones[1].storePosition("FarAbove", ofPoint(30.0, 82.0, 0.0));
    lightDrones[1].storePosition("NearAbove", ofPoint(-30.0, 82.0, 00.0));
    lightDrones[1].startMovement("FarAbove", "NearAbove", 20);

    lightDrones[2].storePosition("FarAbove", ofPoint(0.0, 0.0, 0.0));
    lightDrones[2].storePosition("NearAbove", ofPoint(0.0, 100.0, 0.0));
    lightDrones[2].startMovement("FarAbove", "NearAbove", 50);

}

void ofApp::setupGLBuffer(){
    distanceThreshold = 100;
    sliceDist = 0.1;
    timeSpread = 0.5;

    pointCloud.setup();
    futureSpectrogram.setup(true);
    pastSpectrogram.setup(false);
    guiEnabled = false;
    boxEnabled = false;

    gainContour.reserve(kKinectWidth);
    for(int i = 0; i < kKinectWidth ;i++){
        gainContour.emplace_back(static_cast<float>(i) / kHalfKinectWidthFloat-1.0, -1, 0.0);
    }
    gainContourVbo.setVertexData(&gainContour[0], kKinectWidth, GL_DYNAMIC_DRAW);
}

void ofApp::setupObject(){
    scanner.setPosition(0,0,0);
    scaleAnimation.reset(0.2);
    scaleAnimation.setCurve(EASE_IN);
    scaleAnimation.setRepeatType(LOOP_BACK_AND_FORTH);
    scaleAnimation.setDuration(15);
    scaleAnimation.animateTo( 2.8 );
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
    pdFutureSpectrumBuffer = std::vector<float>(kNumBins, 0.0);
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
    manualCamera = false;
    trigger = Trigger::Stay;
}

// update


void ofApp::updateGainContour(){
    static Trigger previousStat;
    float gainSum = 0;
    static float previousGainSum = 0;
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

    scaleAnimation.update(1/30.0);
    if(trigger == Trigger::Enter){
        if(!insertionCamera.isRunning()){
            int startPos = ofRandom(8.99);
            int endPos = ofRandom(8.99);
            float duration = ofRandom(5.0)+5.0;
            insertionCamera.startMovement(positionNames[startPos], positionNames[endPos], duration);
        }
    }else if(trigger == Trigger::Exit){
        insertionCamera.stopMovement();
    }

    if(insertionCamera.isRunning()){
        insertionCamera.stepForward();
    }else{
        staticCamera.stepForward();
    }
    for(auto &lightDrone : lightDrones){lightDrone.stepForward();}
    timeSpread = scaleAnimation.val();

    // read spectrum
    pd.readArray("pastSpectrum", pdPastSpectrumBuffer);
    pastSpectrogram.update(pdPastSpectrumBuffer);
    pd.readArray("futureSpectrum" , pdFutureSpectrumBuffer);
    futureSpectrogram.update(pdFutureSpectrumBuffer);

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
    ofEnableLighting();
    for(auto &lightDrone: lightDrones){lightDrone.enable();}

    if(manualCamera){
        camera.begin();
    }else{
        if(insertionCamera.isRunning()){
            insertionCamera.begin();
            insertionCamera.lookAt(ofVec3f(0,0,0));
        }else{
            staticCamera.begin();
            staticCamera.lookAt(ofVec3f(0,0,0));
           // ofLog() << staticCamera.getPosition();

        }
    }

    if(boxEnabled){
        ofNoFill();
        ofDrawBox(2,2,2);
    }

    pointCloud.draw();
    gainContourVbo.draw(GL_LINE_STRIP, 0, kKinectWidth );
    pastSpectrogram.draw(sliceDist, timeSpread);

    ofPushMatrix();
    ofRotateY(180);
    futureSpectrogram.draw(sliceDist, timeSpread);
    ofPopMatrix();
    scanner.draw();
    //for(auto &lightDrone: lightDrones){lightDrone.draw();}

    if(manualCamera){camera.end();}
    else{
       insertionCamera.isRunning() ? insertionCamera.end() : staticCamera.end();
    }

    for(auto &lightDrone: lightDrones){lightDrone.disable();}
    ofDisableLighting();

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
    case 'c':
        manualCamera = !manualCamera;
        break;
    }
}

