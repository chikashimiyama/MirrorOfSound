#include "ofApp.h"
using namespace pd;

#pragma mark setups

void ofApp::setupGLEnvironment(){
    ofSetBackgroundColor(0);
    ofEnableDepthTest();
    ofSetVerticalSync(true);
    ofSetCircleResolution(50);
    ofSetLineWidth(0.1);
    ofEnableAlphaBlending();
}

void ofApp::setupGLCamera(){
    camera.setFarClip(500);
    camera.setNearClip(0.01);
    
    lookAtAnimation.setPosition(ofPoint(-0.2, 0, 2.35));
    lookAtAnimation.setRepeatType(LOOP_BACK_AND_FORTH);
    lookAtAnimation.setCurve(EASE_IN_EASE_OUT);
    lookAtAnimation.animateTo(ofPoint(-0.3, 0.6, 4.1));

    cameraAnimation.setPosition(ofPoint(-3., 0.1, -0.7));
    cameraAnimation.setRepeatType(LOOP_BACK_AND_FORTH);
    cameraAnimation.setCurve(EASE_IN_EASE_OUT);
    cameraAnimation.animateTo(ofPoint(-5.1, -2.14, -0.816));
}

void ofApp::setupGLBuffer(){
    pointCloud.setup();
    pastSpectrogram.setup();
    futureSpectrogram.setup(true);
    guiEnabled = false;
    boxEnabled = false;
    gainContour.reserve(kKinectWidth);
    for(int i = 0; i < kKinectWidth ;i++){
        gainContour.emplace_back(static_cast<float>(i) / kHalfKinectWidthFloat-1.0, -1, 0.0);
    }
    gainContourVbo.setVertexData(&gainContour[0], kKinectWidth, GL_DYNAMIC_DRAW);
}

void ofApp::audioSetup(){
    ofSoundStreamSetup(kNumOutput, kNumInput, this, kSampleRate, ofxPd::blockSize()*8, 3);
    pdGainBuffer = std::vector<float>(kNumBins, 0.0);
    pdFeedbackSpectrumBuffer = std::vector<float>(kNumBins, 0.0);
    pdPastSpectrumBuffer = std::vector<float>(kNumBins, 0.0);
    pd.init(kNumOutput, kNumInput, kSampleRate);
    pd.openPatch(patchname);
    pd.start();
}

void ofApp::guiSetup(){
    gui.setup();
    gui.add(lookAtSlider.setup("lookat", ofVec3f(-0.2, 0, 2.35), ofVec3f(-10,-10,-10), ofVec3f(10,10,10)));
    gui.add(cameraPosSlider.setup("cameraPos", ofVec3f(-3., 0.1, -0.7), ofVec3f(-10,-10,-10), ofVec3f(10,10,10)));
    gui.add(distThresholdSlider.setup("dist thresh", 100, 0, 500));
}

void ofApp::kinectSetup(){
    kinect.init(true,false,false); // we need only infrared
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
    ofEnableSmoothing();
    ofEnableAntiAliasing();
    kinectSetup();
    ofSetFrameRate(kTargetFPS);
    setupGLEnvironment();
    setupGLCamera();
    setupGLBuffer();
    audioSetup();
    guiSetup();
    pointCloud.setup();
    trigger = Trigger::Stay;
}

#pragma mark update;


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
    
    // camera
    cameraAnimation.update(kCameraSpeed);
    lookAtAnimation.update(kCameraSpeed);
    ofPoint currentCamera, currentLookAt;
    if(guiEnabled){
        currentCamera = cameraPosSlider;
        currentLookAt = lookAtSlider;
    }else{
        currentCamera= cameraAnimation.getCurrentPosition();
        currentLookAt = lookAtAnimation.getCurrentPosition();
    }
    
    camera.setPosition(currentCamera);
    camera.lookAt(currentLookAt);
    
    // read spectrum
    pd.readArray("pastSpectrum", pdPastSpectrumBuffer);
    pd.readArray("futureSpectrum",pdFutureSpectrumBuffer);
    pastSpectrogram.update(pdPastSpectrumBuffer);
    futureSpectrogram.update(pdFutureSpectrumBuffer);

    // read kinect data and sonificate
    kinect.update();
    if(kinect.isFrameNewDepth()){
        std::for_each(gainContour.begin(), gainContour.end(), [](ofPoint & point){
            point.y = -1.0;
        });
        pointCloud.update(kinect.getDepthPixels(),gainContour,kExitThreshold);
    }
}

void ofApp::drawWorld(){

    camera.begin();

    if(boxEnabled){ ofNoFill();ofDrawBox(2,2,2);}
    pointCloud.draw();
    
    ofSetLineWidth(2);
    ofSetColor(ofColor::orange);
    gainContourVbo.draw(GL_LINE_STRIP, 0, kKinectWidth );

    
    pastSpectrogram.draw();

    ofPushMatrix();
    ofTranslate(0,-2,0);
    ofRotateZ(180);
    futureSpectrogram.draw();
    ofPopMatrix();
    
    ofPushMatrix();
    ofRotateY(180);
    ofPopMatrix();
    scanner.draw();
    
    camera.end();
}

void ofApp::drawGui(){
    ofSetColor(255, 255, 255, 255);
    ofDisableDepthTest();
    gui.draw();
    ofEnableDepthTest();
}

void ofApp::draw(){
    ofSetColor(ofColor::white);
    drawWorld();
    if(guiEnabled)drawGui();
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

