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
    flip = false;
    nearClipping = 50;
    farClipping = 8000;
    nearThreshold = 255;
    farThreshold = 165;
    minContourSize = 20.0;
    maxContourSize = 250.0;
    irThreshold = 70;
    //kinect.setLed(ofxKinect::LED_OFF);
    int w = kinect.width;
    int h = kinect.height;
    sscale = 1;
    
    colorImage.allocate(w,h);
    
    ofSetFrameRate(60);
    
    time0 = ofGetElapsedTimef(); // init time
    red = 0; green = 0; blue = 0; // BACKGROUND COLOR
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
    // MARKER PARTICLES
    emitterParticles = new ParticleSystem();
    emitterParticles->setup(EMITTER, w, h);

    // GRID PARTICLES
    gridParticles = new ParticleSystem();
    gridParticles->setup(GRID, w, h);

    // BOIDS PARTICLES
    boidsParticles = new ParticleSystem();
    boidsParticles->setup(BOIDS, w, h);

    // ANIMATIONS PARTICLES
    animationsParticles = new ParticleSystem();
    animationsParticles->animation = RAIN;
    animationsParticles->setup(ANIMATIONS, w, h);
    
    // VECTOR OF PARTICLE SYSTEMS
    particleSystems.push_back(emitterParticles);
    particleSystems.push_back(gridParticles);
    particleSystems.push_back(boidsParticles);
    particleSystems.push_back(animationsParticles);
    currentParticleSystem = 0;
    

    // Transition ------------------------------------------------------
    // ALLOCATE FBO AND FILL WITH BG COLOR
    fbo.allocate(w, h, GL_RGBA); //GL_RGBA32F_ARB
    fbo.begin();
    ofClear(0);
    fbo.end();

    fadeAmount = 80;
    useFBO = false;
}

void ofApp::exit() {
    kinect.setCameraTiltAngle(0);
    kinect.close();
}

//--------------------------------------------------------------
void ofApp::update(){
    // Compute dt
    float time = ofGetElapsedTimef(); // get new time
    float dt = ofClamp(time - time0, 0, 0.1); // calculate time passed
    time0 = time;
    
    windowWidth = ofGetWindowWidth();
    windowHeight = ofGetWindowHeight();
    // Compute rescale value to scale kinect image
    sscale = (float)windowHeight / (float)kinect.height;
    
    //if(interpolatingWidgets) //interpolate values for smooth transition
    
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
    emitterParticles->update(dt, contour, fluid); // TODO: check markers param
    gridParticles->update(dt, contour, fluid);
    boidsParticles->update(dt, contour, fluid);
    animationsParticles->update(dt, contour, fluid);
 
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(255);
    
    shader.begin();
    ofDrawRectangle(0,0,ofGetWidth(),ofGetHeight());
    shader.end();
    
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    ofPushMatrix();
    ofPushStyle();
    
    ofColor contourBg(red, green, blue);
    ofColor centerBg(red, green, blue);
    if(bgGradient){
        if(centerBg.getBrightness() > 0) contourBg.setBrightness(ofMap(centerBg.getBrightness(), 0.0, 255.0, 20.0, 130.0));
        ofBackgroundGradient(centerBg, contourBg);
    }
    else ofBackground(centerBg);
    
    ofRectangle canvasRect(0, 0, windowWidth, windowHeight);
    ofRectangle kinectRect(0, 0, kinect.getWidth(), kinect.getHeight());
    kinectRect.scaleTo(canvasRect, OF_SCALEMODE_FIT);
    ofTranslate(kinectRect.x, kinectRect.y);
    ofScale(sscale, sscale);
    
    if(useFBO){
        fbo.clear();
        fbo.begin();
        
        // Draw semi-transparent white rectangle to slightly clear buffer (depends on the history value)
        ofFill();
        ofSetColor(red, green, blue, ofMap(fadeAmount, 0, 100, 250, 0));
        ofDrawRectangle(0, 0, kinect.width, kinect.height);

        // Graphics
        ofNoFill();
        ofSetColor(0xffffff);

        contour.draw();
/*
        emitterParticles->draw();
        gridParticles->draw();
        boidsParticles->draw();
        animationsParticles->draw();
*/
        particleSystems[currentParticleSystem]->draw();
        fbo.end();

        // Draw buffer (graphics) on the screen
        ofSetColor(255);
        fbo.draw(0,0);
    }else{
        ofSetColor(255, 255, 255);
        //ofClear(255, 255, 255);
        // Draw Graphics
       //contour.draw();

       fluid.draw();
/*
        emitterParticles->draw();
        gridParticles->draw();
        boidsParticles->draw();
        animationsParticles->draw();
 */
        emitterParticles->draw();
        particleSystems[currentParticleSystem]->draw();
  
    }
    
    ofPopStyle();
    ofPopMatrix();
    
//    colorImage.draw(0,0,windowWidth, windowHeight);
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
        case '1':
            currentParticleSystem = 1;
            break;
        case '2':
            currentParticleSystem = 2;
            break;
        case '3':
            currentParticleSystem = 3;
            break;
        default:
            currentParticleSystem = 0;
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
