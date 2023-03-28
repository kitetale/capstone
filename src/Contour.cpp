//
//  Contour.cpp
//  capstone
//
//  Created by Ashley K on 3/22/23.
// ref: https://github.com/fabiaserra/crea/blob/master/src/Contour.cpp
//

#include "Contour.h"

using namespace ofxCv;
using namespace cv;


Contour::Contour(){
    isActive = true;
    
    isFadingIn = false; //opacity fading in
    isFadingOut = false; //opacity fading out
    startFadeIn = false; //fade in started?
    startFadeOut = false; //fade out started?
    elapsedFadeTime = 0.0;
    fadeTime = 1.2;
    
    red = 255.0;
    green = 255.0;
    blue = 255.0;
    
    opacity = 0.0; //current opacity of particles
    maxOpacity = 255.0; //max opacity value
    
    doOpticalFlow = true;
    
    //optical flow param
    flowStrength = 10.0; // 0 ~ 100
    flowOffset = 3.0; // 1 ~ 10
    flowLambda = 0.01; // 0 ~ 0.1
    flowThreshold = 0.04; // 0 ~ 0.2
    flowInverseX = false;
    flowInverseY = false;
    flowTimeBlurActive = true;
    flowTimeBlurDecay = 0.1; // 0 ~ 1
    flowTimeBlurRadius = 2.0; // 0 ~ 10
    
    //velocity mask
    vMaskBlurPasses = 1; // 0 ~ 10
    vMaskBlurRadius = 5.0; // 0 ~ 10
    vMaskRed = 255.0;
    vMaskGreen = 255.0;
    vMaskBlue = 255.0;
    vMaskOpacity = 255.0;
    vMaskColor = ofColor(vMaskRed, vMaskGreen, vMaskBlue);
    vMaskRandomColor = false;
    
    //contour settings
    smoothingSize = 0.0;
    lineWidth = 1.5;
    scaleContour = 1.0;
    
    //graphics debug
    drawBoundingRect = false;
    drawBoundingRectLine = false;
    drawConvexHull = false;
    drawConvexHullLine = false;
    drawSilhouette = false;
    drawSilhouetteLine = false;
    drawTangentLines = false;
    
    //debug
    drawFlow = false;
    drawFlowScalar = false;
    drawVelMask = false;
    drawVelMask = false;
    drawVelMaskContour = false;
    drawVelocities = false;
}

void Contour::setup(int w, int h, float scale){
    this->w = w;
    this->h = h;
    
    this->scale = scale; //for computing optical flow w lower res depth img
    
    this->flowWidth = w/scale;
    this->flowHeight = h/scale;
    
    opticalFlow.setup(flowWidth, flowHeight);

    opticalFlow.setStrength(100.0);
    velocityMask.setup(flowWidth, flowHeight);
    
    displayScalar.setup(flowWidth, flowHeight);
    velocityField.setup(flowWidth/4, flowHeight/4);
    
    rescaledRect.set(0,0,flowWidth,flowHeight);
    
    contourFinder.setSortBySize(true); // sort contours by size
    
    contourFinderVelMask.setMinAreaRadius(10);
    contourFinderVelMask.setMaxAreaRadius(500);
    contourFinderVelMask.setAutoThreshold(true); //auto threshold velocity mask
    
    // flow fbo
    flowFbo.allocate(flowWidth, flowHeight, GL_RGB32F);//, GL_RGB32F
    flowPixels.allocate(flowWidth, flowHeight, GL_RGB32F);

    // velocity mask
    velocityMaskPixels.allocate(flowWidth, flowHeight, 4);
    velocityMaskFbo.allocate(flowWidth, flowHeight, GL_RGB32F); //, GL_RGB32F
    
    // Fbo
    coloredDepthFbo.allocate(w, h, GL_RGBA32F);//, GL_RGBA32F
    
    coloredDepthFbo.begin();
    ofClear(255,255,255,0);
    ofSetColor(255, 255, 255);
    coloredDepthFbo.end();

}

void Contour::update(float dt, ofImage &depthImage){
    if(doOpticalFlow){
        // Compute optical flow
        //opticalFlow.setSource(depthImage.getTexture());
        opticalFlow.setInput(depthImage.getTexture()); // replace above
        opticalFlow.setStrength(flowStrength);
        opticalFlow.setOffset(flowOffset);
        //opticalFlow.setLambda(flowLambda);
        opticalFlow.setThreshold(flowThreshold);
        opticalFlow.setInverseY(flowInverseY);
        opticalFlow.setInverseX(flowInverseX);
        /*opticalFlow.setTimeBlurActive(flowTimeBlurActive);
        opticalFlow.setTimeBlurRadius(flowTimeBlurRadius);
        opticalFlow.setTimeBlurDecay(flowTimeBlurDecay);*/
        opticalFlow.update();
        
        // Save flow fbo and get its pixels to read velocities
        flowFbo.begin();
        ofPushStyle();
        ofClear(0); // clear buffer
        ofSetColor(255, 255, 255);
        ofFill();
        //opticalFlow.getOpticalFlowDecay().draw(0, 0, flowWidth, flowHeight);
        opticalFlow.drawInput(0,0,flowWidth,flowHeight); // replace above
        ofPopStyle();
        flowFbo.end();
        
        flowFbo.readToPixels(flowPixels);
        
        // tint binary depth image (silhouette)
       coloredDepthFbo.begin();
            ofPushStyle();
            ofClear(0); // clear buffer
            if(vMaskRandomColor){ // tint image with random colors
                vMaskColor.setHsb(ofMap(ofNoise(ofGetElapsedTimef()*0.3), 0.1, 0.9, 0, 255, true), 255, 255);
                vMaskRed = vMaskColor.r;
                vMaskGreen = vMaskColor.g;
                vMaskBlue = vMaskColor.b;
            }
            vMaskColor.set(vMaskRed, vMaskGreen, vMaskBlue, vMaskOpacity);
            ofSetColor(vMaskColor);
            depthImage.draw(0, 0, w, h); // draw binary image in the buffer
            ofPopStyle();
        coloredDepthFbo.end();

        
        velocityMask.setDensity(coloredDepthFbo.getTexture()); // to change mask color
        //velocityMask.setVelocity(opticalFlow.getOpticalFlow());
        velocityMask.setVelocity(opticalFlow.getVelocity()); // replace above
        //velocityMask.setBlurPasses(vMaskBlurPasses);
        velocityMask.setBlurRadius(vMaskBlurRadius);
        velocityMask.update(dt);
        
       // Save to velocityMask fbo and get its pixels to read velocities
        velocityMaskFbo.begin();
        ofPushStyle();
        ofClear(255, 255, 255); // clear buffer
        ofSetColor(255, 255, 255);
        velocityMask.getVelocity().draw(0, 0, flowWidth, flowHeight);
        ofPopStyle();
        velocityMaskFbo.end();
        
        velocityMaskFbo.readToPixels(velocityMaskPixels);

    }
    
    // Contour Finder in the depth Image
    contourFinder.findContours(depthImage);
    
    // Contour Finder in the velocity mask
    contourFinderVelMask.findContours(velocityMaskPixels);
    
    int n = contourFinder.size();
    int m = contourFinderVelMask.size();
    
    // Clear vectors
    boundingRects.clear();
    convexHulls.clear();
    contours.clear();
    vMaskContours.clear();

    // Initialize vectors
    boundingRects.resize(n);
    convexHulls.resize(n);
    contours.resize(n);
    vMaskContours.resize(m);
    
    for(int i = 0; i < n; i++){
        boundingRects[i] = toOf(contourFinder.getBoundingRect(i));

        ofPolyline convexHull;
        convexHull = toOf(contourFinder.getConvexHull(i));
        convexHulls[i] = convexHull;

        ofPolyline contour;
        contour = contourFinder.getPolyline(i);
        contour = contour.getSmoothed(smoothingSize, 0.5);
        contours[i] = contour;
    }
    
    for(int i = 0; i < m; i++){
        ofPolyline vMaskContour;
        vMaskContour = contourFinderVelMask.getPolyline(i);
        vMaskContours[i] = vMaskContour;
    }
    
    if(isActive || isFadingOut){
        // if it is the first frame where isActive is true and we are not fading out (trick to fix a bug)
        // start fadeIn and change activeStarted to true so we dont enter anymore
        if(!activeStarted && !isFadingOut){
            activeStarted = true;
            isFadingIn = true;
            isFadingOut = false;
            startFadeIn = true;
            startFadeOut = false;
            opacity = 0.0;
        }
        if(isFadingIn) fadeIn(dt);
        else if(isFadingOut && !isActive) fadeOut(dt);
        else opacity = maxOpacity;
    }
    else if(activeStarted){
        activeStarted = false;
        isFadingIn = false;
        isFadingOut = true;
        startFadeIn = false;
        startFadeOut = true;
    }
}

void Contour::draw(){
    if(isActive || isFadingOut){
        ofPushMatrix();
        ofTranslate(w/2.0, h/2.0);
        ofScale(scaleContour, scaleContour);
        
        ofPushMatrix();
        ofTranslate(-w/2.0, -h/2.0);
        
        if(drawBoundingRect||drawBoundingRectLine){
            ofPushStyle();
            ofFill();
            ofSetColor(ofColor(red, green, blue), opacity);
            ofSetLineWidth(lineWidth);
            if(drawBoundingRectLine) ofNoFill();
            for(int i = 0; i < boundingRects.size(); i++)
                ofDrawRectangle(boundingRects[i]);
            ofPopStyle();
        }
        if(drawConvexHull ){
            ofPushStyle();
            ofFill();
            ofSetColor(ofColor(red, green, blue), opacity);
            for(int i = 0; i < convexHulls.size(); i++){
                ofBeginShape();
                for(int j = 0; j < convexHulls[i].getVertices().size(); j++){
                    ofVertex(convexHulls[i].getVertices().at(j).x, convexHulls[i].getVertices().at(j).y);
                }
                ofEndShape();
            }
            ofPopStyle();
        }

        if(drawConvexHullLine ){
            ofPushStyle();
            ofSetColor(ofColor(red, green, blue), opacity);
            ofSetLineWidth(lineWidth);
            for(int i = 0; i < convexHulls.size(); i++)
                convexHulls[i].draw(); //if we only want the contour
            ofPopStyle();
        }
        
        if(drawSilhouette){
            ofPushStyle();
            ofSetColor(ofColor(red, green, blue), opacity);
            for(int i = 0; i < contours.size(); i++){
                ofBeginShape();
                for(int j = 0; j < contours[i].getVertices().size(); j++){
                    ofVertex(contours[i].getVertices().at(j).x, contours[i].getVertices().at(j).y);
                }
                ofEndShape();
            }
            ofPopStyle();
        }

        if(drawSilhouetteLine || true){
            ofPushStyle();
            ofSetColor(ofColor(red, green, blue), opacity);
            ofSetLineWidth(lineWidth);
            for(int i = 0; i < contours.size(); i++){
                contours[i].draw();
            }
            ofPopStyle();
        }
        
        if(drawTangentLines){
            ofPushStyle();
            ofSetLineWidth(1);
            ofSetColor(255, 255,255);
            for(int i = 0; i < contours.size(); i++){
                float numPoints = contours[i].size();
                float tangentLength = 100;
                for(int p = 0; p < 1000; p++){
                    ofPoint point = contours[i].getPointAtPercent(p/1000.0);
                    float floatIndex = p/1000.0 * (numPoints-1);
                    ofVec3f tangent = contours[i].getTangentAtIndexInterpolated(floatIndex) * tangentLength;
                    ofDrawLine(point-tangent/2, point+tangent/2);
                }
            }
            ofPopStyle();
        }
        
        ofPopMatrix();
        ofPopMatrix();
    }
    
    // DEBUGGING DRAWINGS
    if(drawFlow){
        ofPushStyle();
        
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        //velocityField.setVelocity(opticalFlow.getOpticalFlowDecay());
        /*velocityField.setVelocity(opticalFlow.getVelocity());
        velocityField.draw(0, 0, w, h);*/
        velocityField.drawVelocity(0,0,w,h);
        ofPopStyle();
    }
    if(drawFlowScalar){
        ofPushStyle();
        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        //displayScalar.setSource(opticalFlow.getOpticalFlowDecay());
        /*displayScalar.setSource(opticalFlow.getVelocity());
        displayScalar.draw(0, 0, w, h);*/
        ofPopStyle();
    }
    if(drawVelMask){
        ofPushStyle();
        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        velocityMask.draw(0, 0, w, h);
        ofPopStyle();
    }
    if(drawVelMaskContour){
        ofPushStyle();
        ofSetColor(255);
        ofSetLineWidth(1.5);
        for(int i = 0; i < vMaskContours.size(); i++){
            vMaskContours[i].draw();
        }
        ofPopStyle();
    }
    if(drawVelocities){
        ofPushStyle();
        //ofSetColor(255, 0, 0);
        ofSetColor(255, 255, 255, 255);
        ofSetLineWidth(1.5);
        for(int i = 0; i < contours.size(); i+=10){
            for(int p = 0; p < contours[i].size(); p++){
                ofDrawLine(contours[i][p], contours[i][p] - getVelocityInPoint(contours[i][p]));
            }
        }
        ofPopStyle();
    }
    prevContours = contours; // Save actual contour to do difference with next one
}


ofVec2f Contour::getFlowOffset(ofPoint p){
    ofPoint p_ = p/scale; // scale point to match flow texture
    ofVec2f offset(0,0);
    
    if(rescaledRect.inside(p_)){ // if point is inside flow texture size
        offset.x = flowPixels[((int)p_.y*flowWidth+(int)p_.x)*3 + 0]; // r
        offset.y = flowPixels[((int)p_.y*flowWidth+(int)p_.x)*3 + 1]; // g
    }
    
    return offset;
}


void Contour::computeVelocities(){
    // Get velocities from prev frame to current frame
    velocities.clear();
    for(int i = 0; i < contours.size(); i++){
        vector<ofPoint> contourVelocities;
        for(int p = 0; p < contours[i].size(); p++){
            ofPoint curPoint = contours[i][p];
            ofPoint prevPoint = prevContours[i].getClosestPoint(curPoint);
            ofPoint vel(0, 0);
            if(prevPoint.squareDistance(curPoint) < 200){
                vel = curPoint - prevPoint;
            }
            contourVelocities.push_back(vel);
        }
        velocities.push_back(contourVelocities);
    }
}


ofPoint Contour::getVelocityInPoint(ofPoint curPoint){
    // Get velocity in point from closest point in prev frame
    float minDistSqrd = 600;
    ofPoint vel(0, 0);
    for(int i = 0; i < prevContours.size(); i++){
        ofPoint prevPoint = prevContours[i].getClosestPoint(curPoint);
        float distSqrd = prevPoint.squareDistance(curPoint);
        if(distSqrd < minDistSqrd){
            minDistSqrd = distSqrd;
            vel = curPoint - prevPoint;
        }
    }
    return vel;
}


void Contour::fadeIn(float dt){
    if(startFadeIn){
        startFadeIn = false;
        elapsedFadeTime = 0.0;
        opacity = 0.0;
    }
    else{
        opacity = ofMap(elapsedFadeTime, 0.0, fadeTime, 0.0, maxOpacity, true);
        elapsedFadeTime += dt;
        if(elapsedFadeTime > fadeTime){
            isFadingIn = false;
            opacity = maxOpacity;
        }
    }
}

void Contour::fadeOut(float dt){
    if(startFadeOut){
        startFadeOut = false;
        elapsedFadeTime = 0.0;
        opacity = maxOpacity;
    }
    else{
        opacity = ofMap(elapsedFadeTime, 0.0, fadeTime, maxOpacity, 0.0, true);
        elapsedFadeTime += dt;
        if(elapsedFadeTime > fadeTime){
            isFadingOut = false;
            opacity = 0.0;
        }
    }
}
