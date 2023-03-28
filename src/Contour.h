//
//  Contour.h
//  capstone
//
//  Created by Ashley K on 3/22/23.
// ref: https://github.com/fabiaserra/crea/blob/master/src/Contour.h
//
#pragma once

#include <stdio.h>
#include "ofMain.h"
#include "ofxCv.h"
#include "ofxFlowTools.h"

using namespace flowTools;

class Contour {
    public:
        Contour();
    
        void setup(int w, int h, float scale = 4.0);
        void update(float dt, ofImage &depthImage);
        void draw();
        ofVec2f getFlowOffset(ofPoint p);
        ofPoint getVelocityInPoint(ofPoint curPoint);

        ofTexture& getOpticalFlowVelocity() {return opticalFlow.getVelocity();}
        ofTexture& getDensityMask() {return velocityMask.getDensity();}
        ofTexture& getTemperatureMask() {return velocityMask.getTemperature();}

        float getFlowWidth() {return flowWidth;}
        float getFlowHeight() {return flowHeight;}

        void computeVelocities();
        void setMinAreaRadius(float minContourSize) {contourFinder.setMinAreaRadius(minContourSize);}
        void setMaxAreaRadius(float maxContourSize) {contourFinder.setMaxAreaRadius(maxContourSize);}
    
        ofTexture& getTexture(){return flowFbo.getTexture();}
        
        bool isActive;
        bool doFading;          // Do opacity fading?
        bool activeStarted;     // Active has started?
        bool isFadingIn;        // Opacity fading in?
        bool isFadingOut;       // Opacity fading out?
        bool startFadeIn;       // Fade in has started?
        bool startFadeOut;      // Fade out has started?
        float elapsedFadeTime;  // Elapsed time of fade
        float fadeTime;         // Transition time of fade
        
        int w, h;
        int flowWidth,flowHeight;
        
        float opacity;
        float maxOpacity;
        
        float red, green, blue;
        
        bool doOpticalFlow;
        float scale;
        
        float flowStrength;
        int flowOffset;
        float flowLambda;
        float flowThreshold;
        bool  flowInverseX;
        bool  flowInverseY;
        bool  flowTimeBlurActive;
        float flowTimeBlurDecay;
        float flowTimeBlurRadius;
        
        int vMaskBlurPasses;
        float vMaskBlurRadius;
        float vMaskRed, vMaskGreen, vMaskBlue;
        float vMaskOpacity;
        ofColor vMaskColor;
        bool vMaskRandomColor;
        
        float smoothingSize;
        float lineWidth;
        float scaleContour;
        
        vector<ofRectangle> boundingRects;
        vector<ofPolyline> convexHulls;
        vector<ofPolyline> contours;    // silhouettes
        vector<ofPolyline> prevContours;
        vector<ofPolyline> diffContours;
        vector<ofPolyline> vMaskContours;
        vector< vector<ofPoint> > velocities;
        
        bool drawBoundingRect;
        bool drawBoundingRectLine;
        bool drawConvexHull;
        bool drawConvexHullLine;
        bool drawSilhouette;
        bool drawSilhouetteLine;
        bool drawTangentLines;
        
        bool drawFlow;
        bool drawFlowScalar;
        bool drawVelocities;
        bool drawVelMask;
        bool drawVelMaskContour;
    
    
    protected:
        ftOpticalFlow opticalFlow;
        ftCombinedBridgeFlow velocityMask;
        //ftVisualizationField velocityMask;
        //--------------------------------------------------------------
        ofxCv::ContourFinder contourFinder;
        ofxCv::ContourFinder contourFinderVelMask;
        //--------------------------------------------------------------
        //ftDisplayScalar displayScalar;
        ftVisualization displayScalar;
        //ftVelocityField velocityField;
        ftFluidFlow velocityField;
        //--------------------------------------------------------------
        ofRectangle rescaledRect;
        ofFbo flowFbo;
        ofFloatPixels flowPixels;
        ofFbo velocityMaskFbo;
        ofPixels velocityMaskPixels;
        ofFbo coloredDepthFbo;
        //--------------------------------------------------------------
        void fadeIn(float dt);
        void fadeOut(float dt);
};
