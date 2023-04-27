#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

// ref: Crea by Fabia Serra Arrizabalaga https://github.com/fabiaserra/crea
// ref: ofFlowTool example

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
    angle = 10;
    kinect.setCameraTiltAngle(angle);
    nearClipping = 0;
    farClipping = 5800;
    kinect.setDepthClipping(nearClipping,farClipping);
    nearThreshold = 600;
    farThreshold = 0;
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
    // TODO: Play around with bg color to lower contrast (eye comfort)
    
    
    
    bgGradient = false;
    
    // ALLOCATE IMAGES
    depthImage.allocate(w, h, OF_IMAGE_GRAYSCALE);
    bgImage.allocate(w, h); // to subtract background (Wall)
    grayDiff.allocate(w, h);
    grayDepthImage.allocate(w,h);
    depthOriginal.allocate(w, h, OF_IMAGE_GRAYSCALE);
    grayThreshNear.allocate(w, h, OF_IMAGE_GRAYSCALE);
    grayThreshFar.allocate(w, h, OF_IMAGE_GRAYSCALE);
    irImage.allocate(w, h, OF_IMAGE_GRAYSCALE);
    irOriginal.allocate(w, h, OF_IMAGE_GRAYSCALE);
    
    learnBg = false;
    //load bg img
    bgOriginal.load("background.png");
    bgOriginal.setImageType(OF_IMAGE_GRAYSCALE);
    ofPixels bgPix = bgOriginal.getPixels();
    bgImage.setFromPixels(bgPix);
    
    
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
    
    //============================= FLUID FIELD ==============
    densityWidth = 1280;
    densityHeight = 720;
    // process all but the density on 16th resolution
    simulationWidth = densityWidth / 2;
    simulationHeight = densityHeight / 2;
    
    opticalFlow.setup(simulationWidth, simulationHeight);
    velocityBridgeFlow.setup(simulationWidth, simulationHeight);
    densityBridgeFlow.setup(simulationWidth, simulationHeight, densityWidth, densityHeight);
    temperatureBridgeFlow.setup(simulationWidth, simulationHeight);
    fluidFlow.setup(simulationWidth, simulationHeight, densityWidth, densityHeight);
    
    opticalFlow.setVisualizationFieldSize(glm::vec2(simulationWidth / 2, simulationHeight / 2));
    velocityBridgeFlow.setVisualizationFieldSize(glm::vec2(simulationWidth / 2, simulationHeight / 2));
    densityBridgeFlow.setVisualizationFieldSize(glm::vec2(simulationWidth / 2, simulationHeight / 2));
    temperatureBridgeFlow.setVisualizationFieldSize(glm::vec2(simulationWidth / 2, simulationHeight / 2));
    fluidFlow.setVisualizationFieldSize(glm::vec2(simulationWidth / 2, simulationHeight / 2));
    fluidFlow.setBuoyancyWeight(0.8);
    fluidFlow.setBuoyancySigma(0.06);
    fluidFlow.setDissipationDen(0.25); //TODO: alter by installation distance
    //fluidFlow.setBuoyancyAmbientTemperature(0.2);
    
    sand = true;
    fbo.allocate(densityWidth, densityHeight);
    ftUtil::zero(fbo); //clear fbo
}

void ofApp::exit() {
    kinect.setCameraTiltAngle(0);
    kinect.close();
}

//--------------------------------------------------------------
void ofApp::update(){
    if(!ambientSound.isPlaying()) ambientSound.play();
    if (!sand){
        unsigned long newNum = contour.boundingRects.size();
        if (lastNum > newNum && donePlaying){
            unsigned long noteNum = xylophone.size();
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
    }
    //---------------------------------------------------- ^ SOUND ----
    
    // Compute dt
    float time = ofGetElapsedTimef(); // get new time
    //float dt = ofClamp(time - time0, 0, 0.1); // calculate time passed
    float dt = 1.0 / max(ofGetFrameRate(), 1.f);
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
        
        if (learnBg) {
            ofPixels pix = depthOriginal.getPixels();
            bgImage.setFromPixels(pix);
            // save bg image for next load
            ofSaveImage(pix, "background.png");
            learnBg = false;
        }
        grayDepthImage.setFromPixels(depthOriginal.getPixels());
        grayDiff.absDiff(bgImage, grayDepthImage);
        depthOriginal.setFromPixels(grayDiff.getPixels());
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
    
    if (sand){
        if (kinect.isFrameNew()){
            fbo.begin();
            kinect.draw(fbo.getWidth(), 0, -fbo.getWidth(), fbo.getHeight());  // draw flipped
            fbo.end();
            
            opticalFlow.setInput(fbo.getTexture());
        }
        opticalFlow.update();
        
        velocityBridgeFlow.setVelocity(opticalFlow.getVelocity());
        velocityBridgeFlow.update(dt);
        densityBridgeFlow.setDensity(fbo.getTexture());
        densityBridgeFlow.setVelocity(opticalFlow.getVelocity());
        densityBridgeFlow.update(dt);
        temperatureBridgeFlow.setDensity(fbo.getTexture());
        temperatureBridgeFlow.setVelocity(opticalFlow.getVelocity());
        temperatureBridgeFlow.update(dt);
        
        fluidFlow.addVelocity(velocityBridgeFlow.getVelocity());
        fluidFlow.addDensity(densityBridgeFlow.getDensity());
        fluidFlow.addTemperature(temperatureBridgeFlow.getTemperature());
        fluidFlow.update(dt);
    } else {
        // Update particles
        boidsParticles->update(dt,contour,fluid);
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(255,255,255);
    
    shader.begin();
    ofDrawRectangle(0,0,ofGetWidth(),ofGetHeight());
    shader.end();
    
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (sand){
        ofClear(0);
        ofPushStyle();
        //ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        //fbo.draw(0,0,windowWidth,windowHeight);
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        ofSetColor(ofColor(250,110,50, 200));
        fluidFlow.draw(0, 0, windowWidth, windowHeight);
        ofPopStyle();
    } else {
        ofPushMatrix();
        
        ofColor contourBg(red, green, blue);
        ofColor centerBg(red, green, blue);
        if(bgGradient){
            if(centerBg.getBrightness() > 0) contourBg.setBrightness(ofMap(centerBg.getBrightness(), 0.0, 255.0, 20.0, 130.0));
            ofBackgroundGradient(centerBg, contourBg);
        }
        else {
            ofBackground(centerBg);
            //ofBackground(180);
        }
        
        ofRectangle canvasRect(0, 0, ofGetWindowWidth(),ofGetWindowHeight());
        ofRectangle kinectRect(0, 0, kinect.getWidth(),kinect.getHeight());
        kinectRect.scaleTo(canvasRect, OF_ASPECT_RATIO_KEEP_BY_EXPANDING, OF_ALIGN_HORZ_CENTER);
        //ofTranslate(kinectRect.x, kinectRect.y);
        ofScale(sscale, sscale);
        
        // Draw Graphics
        ofPushMatrix();
        ofTranslate(-25, -85); //TODO: Installation change dimension
        ofScale(1.42,1.42);
        if(drawContour) {contour.draw();}

        if(drawFluid) {fluid.draw();}
        else {boidsParticles->draw();}
        ofPopMatrix();
    }
    
    if (drawContour){ // DEBUG
        ofPushStyle();
        if (sand) ofSetColor(0);
        else ofSetColor(0);
        stringstream reportStream;
        reportStream << "set near clipping " << nearClipping << " (press: k l)" << endl
            << "set far clipping " << farClipping << " (press: < >)" << endl
            << "set near thresh " << nearThreshold << " (press: h j)" << endl
            << "set far thresh " << farThreshold << " (press: n m)" << endl
            << "set IR thresh " << irThreshold << " (press: z x)" <<endl
            << "press UP and DOWN to change the tilt angle: " << angle << " degrees" << endl;
            
            ofDrawBitmapString(reportStream.str(), 20, 100);
        ofPopStyle();
    }
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
        case 'k':
            if (drawContour) nearClipping-=10;
            break;
        case 'l':
            if (drawContour) nearClipping+=10;
            break;
        case ',':
        case '<':
            if (drawContour) farClipping-=10;
            break;
        case '.':
        case '>':
            if (drawContour) farClipping+=10;
            break;
        case 'h':
            if (drawContour) nearThreshold-=5;
            break;
        case 'j':
            if (drawContour) nearThreshold+=5;
            break;
        case 'n':
            if (drawContour) farThreshold-=5;
            break;
        case 'm':
            if (drawContour) farThreshold+=5;
            break;
        case 'z':
            if (drawContour) irThreshold-=5;
            break;
        case 'x':
            if (drawContour) irThreshold+=5;
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
        case 'k':
        case 'l':
        case ',':
        case '.':
        case '<':
        case '>':
            if (drawContour) kinect.setDepthClipping(nearClipping,farClipping);
            break;
        case ' ':
            learnBg = true;
            break;
        case 's':
            sand = !sand;
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
