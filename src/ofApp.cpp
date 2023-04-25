#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

// ref: https://openframeworks.cc/ofBook/chapters/shaders.html
// ref: Crea by Fabia Serra Arrizabalaga https://github.com/fabiaserra/crea

//--------------------------------------------------------------
void ofApp::setup(){
    /*
    #ifdef TARGET_OPENGLES
        shader.load("shadersES2/shader");
    #else
        if(ofIsGLProgrammableRenderer()){
            shader.load("shadersGL3/shader");
        }else{
            shader.load("shadersGL2/shader");
        }
    #endif*/
    // loading vert & frag separately:
    //shader.load("myCrazyVertFile.vert", "myCrazyFragFile.frag");
    
    kinect.setRegistration(true);
    kinect.init(true); // shows infrared instead of RGB video Image
    kinect.open();
    flip = true;
    nearClipping = 20;
    farClipping = 4000;
    kinect.setDepthClipping(nearClipping,farClipping);
    nearThreshold = 255;
    farThreshold = 165;
    minContourSize = 20.0;
    maxContourSize = 250.0;
    irThreshold = 70;
    kinect.setLed(ofxKinect::LED_OFF);
    int w = kinect.width;
    int h = kinect.height;
    sscale = 4;
    
    colorImage.allocate(w,h);
    
    ofSetFrameRate(60);
    
    time0 = ofGetElapsedTimef(); // init time
    // beige (#FFF9EF)backgorund
    red = 255.0;
    green = 249.0;
    blue = 233.0;
    
    bgGradient = false;
    
    // ALLOCATE IMAGES
    depthImage.allocate(w, h, OF_IMAGE_GRAYSCALE);
    depthOriginal.allocate(w, h, OF_IMAGE_GRAYSCALE);
    grayThreshNear.allocate(w, h, OF_IMAGE_GRAYSCALE);
    grayThreshFar.allocate(w, h, OF_IMAGE_GRAYSCALE);
    irImage.allocate(w, h, OF_IMAGE_GRAYSCALE);
    irOriginal.allocate(w, h, OF_IMAGE_GRAYSCALE);
    
    // ALLOCATE CROPPING MASKS
    depthCroppingMask = Mat::ones(h, w, CV_8UC1); // row , col , type
    irCroppingMask = Mat::ones(h, w, CV_8UC1); // row , col , type
    depthLeftMask = irLeftMask = 0;
    depthRightMask = irRightMask = w;
    depthTopMask = irTopMask = 0;
    depthBottomMask = irBottomMask = h;
    
    // FILTER PARAM DEPTH
    depthDilate = 4;
    depthErode = 2;
    depthBlur = 7;
    
    // FILTER PARAM IR
    irDilates = 3;
    irErode = 1;
    irBlur = 21;

    float scaleFactor = 4.0; // for fluid & flow computation
    // Contour init ----------------------------------------------------
    // SILHOUETTE CONTOUR
    contour.setup(w, h, sscale);
    
    contour.setMinAreaRadius(minContourSize);
    contour.setMaxAreaRadius(maxContourSize);
    

    // Fluid init ------------------------------------------------------
    fluid.setup(w, h, scaleFactor);

    // Particle Init ---------------------------------------------------
    // BOIDS PARTICLES
    boidsParticles = new ParticleSystem();
    boidsParticles->setup(BOIDS, w, h);

    // Transition ------------------------------------------------------
    // ALLOCATE FBO AND FILL WITH BG COLOR
    fbo.allocate(w, h, GL_RGBA32F_ARB); //GL_RGBA32F_ARB
    fbo.begin();
    ofClear(255,255,255,0);
    fbo.end();

    fadeAmount = 80;
    useFBO = false;
    
    drawContour = false;
    drawFluid = false;
    
    ambientSound.load("equipoise_sound.mp3");
    s_c.load("do.mp3");
    xylophone.push_back(s_c);
    s_d.load("re.mp3");
    xylophone.push_back(s_d);
    s_e.load("mi.mp3");
    xylophone.push_back(s_e);
    s_g.load("sol.mp3");
    xylophone.push_back(s_g);
    s_a.load("la.mp3");
    xylophone.push_back(s_a);
    s_c2.load("do1.mp3");
    xylophone.push_back(s_c2);
    //s_d2.load("re2.mp3");
    //xylophone.push_back(s_d2);
    //s_e2.load("mi2.mp3");
    //xylophone.push_back(s_e2);
    //s_g2.load("sol2.mp3");
    //xylophone.push_back(s_g2);
    s_g0.load("sol0.mp3");
    xylophone.push_back(s_g0);
    s_f0.load("fa0.mp3");
    xylophone.push_back(s_f0);
    
    curNote = 0;
    donePlaying = true;
    lastNum = 0;
    
}

void ofApp::exit() {
    kinect.setCameraTiltAngle(0);
    kinect.close();
}

//--------------------------------------------------------------
void ofApp::update(){
    if(!ambientSound.isPlaying()) ambientSound.play();
    
    int newNum = contour.boundingRects.size();
    if (lastNum > newNum && donePlaying){
        int noteNum = xylophone.size();
        int i1 = ofRandom(noteNum);
        int i2 = ofRandom(noteNum);
        int i3 = ofRandom(noteNum);
        if (i1==i2) i2 = ofRandom(noteNum);
        if (i2==i3) i3 = ofRandom(noteNum);
        if (i1==i3) i3 = ofRandom(noteNum);
        notei[0] = i1;
        notei[1] = i2;
        notei[2] = i3;
        sort(notei, notei+3);
        
        donePlaying = false;
    }
    if (!donePlaying){
        if (curNote >= 2){
            xylophone[notei[2]].play();
            curNote = 0;
            donePlaying = true;
            lastNum = contour.boundingRects.size();
        } else {
            xylophone[notei[curNote]].play();
            curNote++;
        }
    } else {
        lastNum = newNum;
    }
    
    
    
    // Compute dt
    float time = ofGetElapsedTimef(); // get new time
    float dt = ofClamp(time - time0, 0, 0.1); // calculate time passed
    time0 = time;
    
    windowWidth = ofGetWindowWidth();
    windowHeight = ofGetWindowHeight();
     
    // Compute rescale value to scale kinect image
    sscale = (float)windowHeight / (float)kinect.height;
    
    // update kinect
    kinect.update();
    if(kinect.isFrameNew()){
        colorImage.setFromPixels(kinect.getPixels());
        
        depthOriginal.setFromPixels(kinect.getDepthPixels());
        if(flip) depthOriginal.mirror(false, true);
        irOriginal.setFromPixels(kinect.getPixels());
        if(flip) irOriginal.mirror(false, true);
    }
    // update imgs
    copy(irOriginal, irImage);
    copy(depthOriginal, depthImage);
    copy(depthOriginal, grayThreshNear);
    copy(depthOriginal, grayThreshFar);
    
    // Filter and then threshold the IR image
    for(int i = 0; i < irErode; i++){
        erode(irImage); // delete small white dots
    }
    for(int i = 0; i < irDilates; i++){
        dilate(irImage);
    }
    blur(irImage, irBlur);
    threshold(irImage, irThreshold);

    // Treshold and filter depth image
    threshold(grayThreshNear, nearThreshold, true);
    threshold(grayThreshFar, farThreshold);
    bitwise_and(grayThreshNear, grayThreshFar, depthImage);
    
    grayThreshNear.update();
    grayThreshFar.update();
    
    // Filter and then threshold the Depth image
    for(int i = 0; i < depthErode; i++){
        erode(depthImage);
    }
    for(int i = 0; i < depthDilate; i++){
        dilate(depthImage);
    }
    blur(depthImage, depthBlur);

    // Crop depth image
    Mat depthMat = toCv(depthImage);
    Mat depthCropped = Mat::zeros(kinect.height, kinect.width, CV_8UC1);
    depthCropped = depthMat.mul(depthCroppingMask);
    copy(depthCropped, depthImage);
    
    // Crop IR image
    Mat irMat = toCv(irImage);
    Mat irCropped = Mat::zeros(kinect.height, kinect.width, CV_8UC1);
    irCropped = irMat.mul(irCroppingMask);
    copy(irCropped, irImage);

    // Update images
    irImage.update();
    depthImage.update();

    // Update contour
    contour.update(dt, depthImage);
    
    // Update fluid
    fluid.update(dt, contour);

    // Update particles
    boidsParticles->update(dt,contour,fluid);
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(255,255,255);
    
    shader.begin();
    ofDrawRectangle(0,0,ofGetWidth(),ofGetHeight());
    shader.end();
    
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    ofPushMatrix();
    
    ofColor contourBg(red, green, blue);
    ofColor centerBg(red, green, blue);
    if(bgGradient){
        if(centerBg.getBrightness() > 0) contourBg.setBrightness(ofMap(centerBg.getBrightness(), 0.0, 255.0, 20.0, 130.0));
        ofBackgroundGradient(centerBg, contourBg);
    }
    else ofBackground(centerBg);
    
    ofRectangle canvasRect(0, 0, windowWidth,windowHeight);
    ofRectangle kinectRect(0, 0, kinect.getWidth(),kinect.getHeight());
    kinectRect.scaleTo(canvasRect, OF_SCALEMODE_FIT);
    ofTranslate(kinectRect.x, kinectRect.y);
    ofScale(sscale, sscale);
    
    // Draw Graphics
    ofPushMatrix();
    ofTranslate(-50, -35); //TODO: Installation change dimension
    ofScale(1.1,1.1);
    if(drawContour) {contour.draw();}

    if(drawFluid) {fluid.draw();}
    else {boidsParticles->draw();}
    ofPopMatrix();
    /*
    float gapW = 0; //TODO: Installation change side bars
    ofPushStyle();
    ofSetColor(255);
    ofFill();
    ofDrawRectangle(0, 0, gapW, windowHeight);
    ofDrawRectangle(windowWidth-gapW, 0, gapW, windowHeight);
    ofPopStyle();
     */
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key){
        case OF_KEY_UP:
            if (angle < 30) {
                ++angle;
            }
            kinect.setCameraTiltAngle(angle);
            break;

        case OF_KEY_DOWN:
            if (angle > -30) {
                --angle;
            }
            kinect.setCameraTiltAngle(angle);
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    switch (key){
        case 'c':
            boidsParticles->drawConnections = !boidsParticles->drawConnections;
            break;
        case 'd':
            drawContour = !drawContour;
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

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
