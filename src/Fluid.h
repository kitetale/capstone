//
//  Fluid.hpp
//  capstone
//
//  Created by Ashley K on 3/22/23.
// ref: https://github.com/fabiaserra/crea/blob/master/src/Fluid.h
//

#pragma once
#include "ofMain.h"
#include "ofxFlowTools.h"
#include "Contour.h"

class Fluid {
    public:
        Fluid();
        
        void setup(int w, int h, float scale = 4.0, bool internalFormat = false);
        void update(float dt, Contour &contour);
        void draw();
        
        ofVec2f getFluidOffset(ofPoint p);
        
        void reset();
        
        bool isActive;
        bool particlesActive;
        
        bool activeStarted;
        bool isFadingIn;
        bool isFadingOut;
        bool startFadeIn;
        bool startFadeOut;
        bool elapsedFadeTime;
        bool fadeTime;
        
        int w, h;
        
        int flowWidth;
        int flowHeight;
        
        float red, green, blue;
        
        float opacity;
        float maxOpacity;
        
        float scale;
        
        bool contourInput;// Fluid input is the depth contour?
        
        bool drawVelocity;
        bool drawVelocityScalar;
        bool drawPressure;
        bool drawVorticity; //local spinning motion of a continuum near some point
        bool drawTemperature;
        bool drawContourFluid;
        
        // fluid parameters
        float speed;
        float cellSize;
        int JacobianIterationNum;
        float viscosity;
        float vorticity;
        float dissipation;
        float dissipationVelocityOffset;
        float dissipationDensityOffset;
        float dissipationTemperatureOffset;
        float smokeSigma;
        float smokeWeight;
        float ambientTemperature;
        ofPoint gravity;
        float clampForce;
        float maxVelocity;
        float maxDensity;
        float maxTemperature;
        float densityFromVorticity;
        float densityFromPressure;
        
        // fluid particles parameters
        float pVelocity;
        float pCellSize;
        float pBirthChance;
        float pBirthVelocityChance;
        float pLifetime;
        float pLifetimeRnd;
        float pMass;
        float pMassRnd;
        float pSize;
        float pSizeRnd;
        
    protected:
        ftFluidFlow fluid;
        ftParticleFlow fluidParticles;
        
        //ftOpticalFlow displayScalar;
        //ftSvFromVelocity displayScalar;
        ftDensityBridgeFlow densityDisplayScalar; // ftMouseFlow
        ftVelocityBridgeFlow velocityField;
        
        ofTexture fluidFlow;
        ofFbo fluidFbo;
        ofFloatPixels fluidVelocities;
        
        ofRectangle rescaledRect;
        
        void fadeIn(float dt);
        void fadeOut(float dt);
};
