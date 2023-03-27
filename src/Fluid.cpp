//
//  Fluid.cpp
//  capstone
//
//  Created by Ashley K on 3/22/23.
//

#include "Fluid.h"

Fluid::Fluid(){
    isActive = true;
    particlesActive = false;

    activeStarted = false;
    isFadingIn = false;
    isFadingOut = false;
    startFadeIn = false;
    startFadeOut = false;
    elapsedFadeTime = 0.0;
    fadeTime = 1.2;
    
    red = green = blue = 255.0;
    opacity = 0.0;
    maxOpacity = 255.0;

    //input
    contourInput = true;// Fluid input is the depth contour?
    
    // output
    drawVelocity = false;
    drawVelocityScalar = true;
    drawPressure = false;
    drawVorticity = false; //local spinning motion of a continuum near some point
    drawTemperature = false;
    drawContourFluid = false;
    
    // fluid parameters
    speed = 10.0; // 0 ~ 100
    cellSize = 1.25; // 0 ~ 2
    JacobianIterationNum = 40; // 1 ~ 100
    viscosity = 0.005; //  0 ~ 1
    vorticity = 0.1; // 0 ~ 1
    dissipation = 0.01; // 0 ~ 0.002
    dissipationVelocityOffset = 0.0; // -0.001 ~ 0.001
    dissipationDensityOffset = 0.0; // -0.001 ~ 0.001
    dissipationTemperatureOffset = 0.0; // -0.001 ~ 0.001
    smokeSigma = 0.05; // 0 ~ 1
    smokeWeight = 0.05; // 0 ~ 1
    ambientTemperature = 0.0; // 0 ~ 1
    gravity = ofPoint(0,0); // ofVec2f(-1,-1) ~ ofVec2f(1,1)
    clampForce = 0.0; // 0 ~ 0.1
    maxVelocity = 1.0; // 0 ~ 5
    maxDensity = 1.0; // 0 ~ 10
    maxTemperature = 1.0; // 0 ~ 5
    densityFromVorticity = 0.0; // -0.1 ~ 0.1
    densityFromPressure = 0.0; // -0.5 ~ 0.5
    
    // fluid particles parameters
    pBirthChance = 0.1;  // 0 ~ 1
    pBirthVelocityChance = 0.1; // 0 ~ 1
    pLifetime = 3.0;  // 0 ~ 10
    pLifetimeRnd = 0.0; // 0 ~ 1
    pMass = 1.0; // 0 ~ 2
    pMassRnd = 0.0; // 0 ~ 1
    pSize = 1.0; // 0 ~ 10
    pSizeRnd = 0.0; // 0 ~ 1
}

void Fluid::setup(int w, int h, float scale, bool internalFormat){
    this->w = w;
    this->h = h;

    this->scale = scale;

    this->flowWidth = w/scale;
    this->flowHeight = h/scale;

    fluid.setup(flowWidth,flowHeight, w, h); //fluid

    fluidParticles.setup(flowWidth, flowHeight, w, h); //particle

    rescaledRect.set(0,0,flowWidth,flowHeight); //create rect w flow size

    // Allocate visualisation classes
    //displayScalar.setup(flowWidth, flowHeight);
    densityDisplayScalar.setup(w, h);
    velocityField.setup(flowWidth/4, flowHeight/4);

    // allocate fluid FBO
    fluidFbo.allocate(flowWidth, flowWidth, GL_RGBA32F); //
    
    // Allocate fluid pixels
    fluidVelocities.allocate(flowWidth, flowHeight, 4);
}

void Fluid::update(float dt, Contour &contour){
    if(isActive || isFadingOut){ 
        if (!activeStarted && !isFadingOut){
            activeStarted = true;
            isFadingIn = true;
            isFadingOut = false;
            startFadeIn = true;
            startFadeOut = false;
            opacity = 0.0;
        }
        if(isFadingIn) fadeIn(dt);
        else if(isFadingOut && !isActive) fadeOut(dt);
        else opacity = maxOpacity; // not fading 

        //set fluid parameters
        fluid.setSpeed(speed);
        //fluid.setCellSize(cellSize);
        //fluid.setNumJacobiIterations(JacobianIterationNum);
        fluid.setViscosityVel(viscosity);
        fluid.setVorticity(vorticity);
        //fluid.setDissipation(dissipation);
        fluid.setDissipationVel(dissipationVelocityOffset);
        fluid.setDissipationDen(dissipationDensityOffset);
        fluid.setDissipationTmp(dissipationTemperatureOffset);
        fluid.setBuoyancySigma(smokeSigma);
        fluid.setBuoyancyWeight(smokeWeight);
        fluid.setBuoyancyAmbientTemperature(ambientTemperature);
        //fluid.setGravity(gravity);
/*      fluid.setClampForce(clampForce);
        fluid.setMaxVelocity(maxVelocity);
        fluid.setMaxDensity(maxDensity);
        fluid.setMaxTemperature(maxTemperature);
        fluid.setDensityFromVorticity(densityFromVorticity);
        fluid.setDensityFromPressure(densityFromPressure);
*/
        if(contourInput){
            fluid.setVelocity(contour.getOpticalFlowDecay());
            fluid.setDensity(contour.getColorMask());
            fluid.setTemperature(contour.getLuminanceMask());
        }

        fluid.update(dt);

        // Get fluid texture and save to pixels so we can operate with them
        fluidFbo.begin();
        ofPushStyle();
        ofClear(255, 255, 255, 0); // clear buffer
        fluid.getVelocity().draw(0, 0, flowWidth, flowHeight);
        ofPopStyle();
        fluidFbo.end();

        fluidFbo.readToPixels(fluidVelocities);

         if(particlesActive){
            // set fluid particles parameters
            fluidParticles.setSpeed(fluid.getSpeed());
            //fluidParticles.setCellSize(fluid.getCellSize());
            fluidParticles.setBirthChance(pBirthChance);
            fluidParticles.setBirthVelocityChance(pBirthVelocityChance);
            fluidParticles.setLifeSpan(pLifetime);
            fluidParticles.setLifeSpanSpread(pLifetimeRnd);
            fluidParticles.setMass(pMass);
            fluidParticles.setMassSpread(pMassRnd);
            fluidParticles.setSize(pSize);
            fluidParticles.setSizeSpread(pSizeRnd);
            
            if(contourInput) fluidParticles.addFlowVelocity(contour.getOpticalFlowDecay());
            fluidParticles.addFluidVelocity(fluid.getVelocity());
        }
        fluidParticles.update(dt);

    }else if(activeStarted){
        activeStarted = false;
        isFadingIn = false;
        isFadingOut = true;
        startFadeIn = false;
        startFadeOut = true;
    } 
}

void Fluid::draw(){
    if(isActive || isFadingOut){
        if(drawVelocity){
            ofPushStyle();
            ofEnableBlendMode(OF_BLENDMODE_ADD);
            velocityField.setVelocity(fluid.getVelocity());
            velocityField.draw(0, 0, w, h);
            ofPopStyle();
        }
        if(drawVelocityScalar){ // --> extends
            ofPushStyle();
            ofEnableBlendMode(OF_BLENDMODE_DISABLED);
            /*displayScalar.setSource(fluid.getVelocity());
            displayScalar.draw(0, 0, w, h);*/
            fluid.drawVelocity(0,0,w,h);
            ofPopStyle();
        }
        if(drawPressure){ // |||||||
            ofPushStyle();
            ofEnableBlendMode(OF_BLENDMODE_DISABLED);
            /*displayScalar.setSource(fluid.getPressure());
            displayScalar.draw(0, 0, w, h);*/
            fluid.drawPressure(0, 0, w, h);
            ofPopStyle();
        }
        if(drawTemperature){ // .....
            ofPushStyle();
            ofEnableBlendMode(OF_BLENDMODE_DISABLED);
            /*displayScalar.setSource(fluid.getTemperature());
            displayScalar.draw(0, 0, w, h);*/
            fluid.drawTemperature(0, 0, w, h);
            ofPopStyle();
        }
        if(drawVorticity){ // --> gone
            ofPushStyle();
            ofEnableBlendMode(OF_BLENDMODE_DISABLED);
            /*displayScalar.setSource(fluid.getVorticity());
            displayScalar.draw(0, 0, w, h);*/
            fluid.drawVorticity(0, 0, w, h);
            ofPopStyle();
        }
        if(drawContourFluid){ // scary horror vibe
            ofPushStyle();
            ofEnableBlendMode(OF_BLENDMODE_ADD);
            ofSetColor(red, green, blue, opacity);
            fluid.draw(0, 0, w, h);
            ofPopStyle();
        }
        if(particlesActive){
            ofPushStyle();
            ofEnableBlendMode(OF_BLENDMODE_ADD);
            fluidParticles.draw(0, 0, w, h);
            ofPopStyle();
        }
    }
}

ofVec2f Fluid::getFluidOffset(ofPoint p){
    ofPoint pScaled = p/scale;
    ofVec2f offset(0,0);
    
    if(rescaledRect.inside(pScaled)){
        offset.x = fluidVelocities[((int)pScaled.y*flowWidth+(int)pScaled.x)*4 + 0]; // r
        offset.y = fluidVelocities[((int)pScaled.y*flowWidth+(int)pScaled.x)*4 + 1]; // g
    }
        
    return offset;
}

void Fluid::reset(){
    fluid.reset();
}

void Fluid::fadeIn(float dt){
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

void Fluid::fadeOut(float dt){
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
