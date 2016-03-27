#include "ofApp.h"
using namespace pd;

void ofApp::setupGLEnvironment()
{
    ofSetBackgroundColor(0);
    ofEnableDepthTest();
    ofSetVerticalSync(true);
    ofSetCircleResolution(50);
    ofSetLineWidth(0.1);
    ofEnableAlphaBlending();
}

void ofApp::setupGLCamera()
{
    camera.setNearClip(0.0001);
    camera.setFarClip(120.0);
    camera.setPosition(0.0, 0.0, -5.0);
}


void ofApp::setupGLGainContour()
{
    for(int i = 0; i < kKinectWidth ;i++){
        gainContour.push_back(ofPoint( static_cast<float>(i) / kHalfKinectWidthFloat-1.0, -1,-1));
    }
    gainContourVbo.setVertexData(&gainContour[0], kKinectWidth, GL_DYNAMIC_DRAW);
}

void ofApp::setupPointCloud(){
    pointCloudVertices = std::vector<ofPoint>(kNumKinectPixels, ofPoint(0,0,0));
    pointCloudColors = std::vector<ofFloatColor>(kNumKinectPixels, ofColor::white);

    pointCloudVbo.setVertexData(&pointCloudVertices[0], kNumVertices, GL_DYNAMIC_DRAW);
    pointCloudVbo.setColorData(&pointCloudColors[0], kNumVertices, GL_DYNAMIC_DRAW);
}

void ofApp::setupSpectrograms(){
    for(int i = 0; i < kHalfNumTimeSlices; i++){
        for(int j = 0; j < kNumBins; j++){
            float x = static_cast<float>(j) / static_cast<float>(kNumBins) * 2.0 - 1.0;
            beforeSpectrogram.push_back(ofPoint(x,-1.0,-1.0));
            afterSpectrogram.push_back(ofPoint(x,-1.0,-1.0));

        }
    }
    beforeSpectrogramVbo.setVertexData(&beforeSpectrogram[0],kHalfNumVertices ,GL_DYNAMIC_DRAW);
    afterSpectrogramVbo.setVertexData(&afterSpectrogram[0],kHalfNumVertices ,GL_DYNAMIC_DRAW);
}

void ofApp::setupGLBuffer()
{
    distanceThreshold = 100;
    setupGLGainContour();
    setupPointCloud();
    setupSpectrograms();
    guiEnabled = false;
    boxEnabled = false;
}

void ofApp::setupObject()
{
    scanner.setPosition(0,0,-1);
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
                // valid
                float y = static_cast<float>(i / kKinectWidth);
                float x = static_cast<float>(i % kKinectWidth);
                pointCloudVertices[validPixelCount].x = (x - kHalfKinectWidthFloat) / kHalfKinectWidth;
                pointCloudVertices[validPixelCount].y = (y -kHalfKinectHeightFloat) / -kHalfKinectHeightFloat;
                float z = (static_cast<float>(distance) - 128.0) / -64.0;
                pointCloudVertices[validPixelCount].z = z;
                if(z < -1.0){
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
    beforeSpectrogramVbo.updateVertexData(&beforeSpectrogram[0], kHalfNumVertices );
    afterSpectrogramVbo.updateVertexData(&afterSpectrogram[0], kHalfNumVertices );

}

void ofApp::updateSpectrogram()
{
    int pixelOffset = recordHead * kNumBins;

    for(int i = 0; i < kNumBins ;i++){
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
        afterSpectrogram[pixelOffset+i].y = pdSpectrumBuffer[i] - 1.0;

        if(pdGainBuffer[i] > 1.0) pdGainBuffer[i] = 1.0;
        if(pdGainBuffer[i] < 0.0) pdGainBuffer[i] = 0.0;

    }
}

void ofApp::update(){
    kinect.update();
    updatePointCloud();
    pd.writeArray("gain", pdGainBuffer);
    pd.readArray("spectrum", pdSpectrumBuffer);
    updateSpectrogram();

    recordHead++;
    recordHead %= kHalfNumTimeSlices;

}

void ofApp::drawSpectrogram(){


    float sliceDist = 0.1;
    float maxDist = sliceDist * kHalfNumTimeSlices;
    int oneMinusRecHead = recordHead-1;
    if(oneMinusRecHead < 0) oneMinusRecHead += kHalfNumTimeSlices;

    ofSetColor(ofColor(125,125,255, 150));
    for(int i = 0; i < kHalfNumTimeSlices;i++){
        int offset = (oneMinusRecHead + i) % kHalfNumTimeSlices * kNumBins;
        ofPushMatrix();
        glTranslatef(0,0, sliceDist * -i + maxDist);
        afterSpectrogramVbo.draw(GL_LINE_STRIP, offset, kNumBins);
        ofPopMatrix();
    }
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


    drawSpectrogram();
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
    if(ImGui::SliderFloat("z", &z, -5.0, 5.0)) changed = true;
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

