//
//  ParticleSystem.cpp
//  capstone
//
//  Created by Ashley K on 3/22/23.
// ref: https://github.com/fabiaserra/crea/blob/master/src/ParticleSystem.cpp

#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(){
    isActive = true;
    activeStarted = false;
    contourInput = true;
    
    isFadingIn = false;
    isFadingOut = false;
    startFadeIn = false;
    startFadeOut = false;
    elapsedFadeTime = 0.0;
    fadeTime = 1.2; // transition time of fade
    
    opacity = 0.0;
    maxOpacity = 255.0;
    
    immortal = false;
    velocity = 0.0;
    radius = 2.0;
    lifetime = 5.0;
    // first main color: red
    red = 255.0;
    green = 72.0;
    blue = 82.0;
    colRed = ofColor(255.0,72.0,82.0,1);
    colBlue = ofColor(64.0,88.0,188.0,1);
    colNavy = ofColor(17.0,46.0,78.0,1);
    colOrange = ofColor(255.0,163.0,0.0,1);
    colBeige = ofColor(255.0,72.0,82.0,1);
    
    nParticles = 200;
    bornRate = 5.0;
    
    // Emitter
    emitterSize = 2.0;
    velocityRnd = 20.0;
    velocityMotion = 50.0;
    lifetimeRnd = 20.0;
    radiusRnd = 50.0;
    
    // Grid
    gridRes = 10;
    
    //Flocking
    lowThresh = 0.1333;
    highThresh = 0.6867;
    separationStrength = 0.01f;
    attractionStrength = 0.002f;
    alignmentStrength = 0.01f;
    maxSpeed = 80.0;
    flockingRadius = 200.0;
    
    // Graphic output
    sizeAge = false;
    opacityAge = false;
    colorAge = false;
    flickers = false;
    noFill = false;
    drawLine = false;
    drawStroke = false;
    strokeWidth = 1.2;
    drawConnections = true;
    connectDist = 20.0;
    connectWidth = 1.0;
    
    //Physics
    friction = 5.0;
    gravity = ofPoint(0,0);
    turbulence = 0.0f;
    repulse = false;
    bounces = false;
    steers = false;
    wrapAround = false;
    bounceDamping = true;
    repulseDist = 5.0*radius;
    returnToOriginForce = 10.0;
    
    //Behavior
    emit = false;
    interact = true;
    flock = false;
    flowInteraction = true;
    fluidInteraction = false;
    repulseInteraction = false;
    attractInteraction = false;
    seekInteraction = true;
    gravityInteraction = false;
    bounceInteraction = false;
    returnToOrigin = true;
    
    //Input
    interactionForce = 10.0; // HOW STRONG 80
    interactionRadius = 40.0; // HOW CLOSE 20
    
    //Emitter
    emitInMovement = false;
    emitAllTimeInside = false;
    emitAllTimeContour = false;
    
    useFlow = false;
    useFlowRegion = false;
    useContourArea = false;
    useContourVel = false;
    
    contourRadius = 15;
    
    particles.clear();
}

ParticleSystem::~ParticleSystem(){
    // Delete all the particles
    for (int i = 0; i < particles.size(); i++) {
        delete particles.at(i);
        particles.at(i) = NULL;
    }
    particles.clear();
}

void ParticleSystem::setup(float w, float h, ofPoint initPos, float maxForce, float maxSpeed){
    this->initPos = initPos;
    this->w = w;
    this->h = h;
    this->maxForce = maxForce;
    this->maxSpeed = maxSpeed;

    addParticles(this->nParticles);
}

void ParticleSystem::setup(ParticleMode particleMode, int width , int height){
    this->particleMode = particleMode;
    this->w = width;
    this->h = height;
    
    if(particleMode == EMITTER){
        emit = true;
        sizeAge = true;
        opacityAge = true;
        colorAge = true;
        velocity = 20;
        lifetime = 1;
    } else if(particleMode == GRID){
        interact = true;
        flowInteraction = true;
        fluidInteraction = true;
        connectDist = 10.0;
        radiusRnd = 0.0;
        returnToOrigin = true;
        immortal = true;
        velocity = 0.0;
        velocityRnd = 0.0;
        gravity = ofPoint(0,0);
        friction = 100;
        createParticleGrid(width, height); 
    } else if(particleMode == BOIDS){
        emitAllTimeContour = false;
        emitInMovement = true;
        opacityAge = true;
        flock = true;
        repulse = true;
        repulseDist = 50.0*radius;
        bounceDamping = false;
        wrapAround = true;
        immortal = false;
        addParticles(nParticles);
        radiusRnd = 7.0;
        lifetimeRnd = 20;
    } else if(particleMode == ANIMATIONS){
        immortal = false;
        friction = 0.0;
        turbulence = 0.0;
        bounces = false;
        opacityAge = true;
        sizeAge = false;
        flickers = false;

        if (animation == RAIN){
            radius = 0.65;
            radiusRnd = 10.0;
            velocity = 80.0;
            velocityRnd = 20.0;
            lifetime = 1.3;
            lifetimeRnd = 50.0;
            gravity = ofPoint(0,80);
            bounces = true;
        } else if (animation == SNOW){
            radius = 2.0;
            radiusRnd = 30.0;
            velocity = 40.0;
            velocityRnd = 20.0;
            lifetime = 2.8;
            lifetimeRnd = 30.0;
            gravity = ofPoint(0,15);
            bounces = true;
        } else if (animation == EXPLOSION){
            radius = 7.0;
            radiusRnd = 80.0;
            velocity = 700.0;
            velocityRnd = 70.0;
            lifetime = 1.6;
            lifetimeRnd = 40.0;
            gravity = ofPoint(0,100);
            friction = 90.0;
            turbulence = 15.0;
            flickers = true;
            sizeAge = true;
            opacityAge = false;
            addParticles(nParticles);
        }
    } else if (particleMode == FALL){
        interact = true;
        flowInteraction = true;
        fluidInteraction = false;
        seekInteraction = false;
        connectDist = 20.0;
        radiusRnd = 0.0;
        returnToOrigin = false;
        immortal = true;
        velocity = 0.0;
        velocityRnd = 0.0;
        gravity = ofPoint(0,0);
        friction = 100;
        createParticleGrid(width, height);
    }
}

bool comparisonFunction(Particle * a, Particle * b) {
    return a->pos.x < b->pos.x;
}
/*
void ParticleSystem::update(vector<ofPolyline> c){ //for boid
    //sort particles for more effective particle/particle interactions
    //sort(particles.begin(), particles.end(), comparisonFunction);

     for (int i=0; i < particles.size(); i++){
        particles[i]->follow(c,contourRadius);
        particles[i]->update();
     }
}

void ParticleSystem::draw(){
    for(int i = 0; i < particles.size(); i++){
        particles[i]->draw();
    }
}
*/

void ParticleSystem::update(float dt, Contour& contour, Fluid& fluid){
    // if is active or we are fading out, update particles
   if(isActive || isFadingOut){
        if (!activeStarted && !isFadingOut){
            activeStarted = true;
            isFadingIn = true;
            isFadingOut = false;
            startFadeIn = true;
            startFadeOut = false;
            opacity = 0.0;

            bornParticles();
        }

        //if(isFadingIn) fadeIn(dt);
        if(isFadingOut && !isActive) fadeOut(dt);
        else opacity = maxOpacity; // not fading 

        //sort particles for more effective particle/particle interactions
        sort(particles.begin(), particles.end(), comparisonFunction); 

        float interactionRadiusSq = interactionRadius*interactionRadius;
        float flockingRadiusSq = flockingRadius * flockingRadius;

        // 1. delete inactive particles
        int i = 0;
        while(i < particles.size()){
            if (!particles[i]->isAlive){
                delete particles.at(i);
                particles.erase(particles.begin() + i);
                numParticles--;
            } else {
                i++;
            }
        }

        // 2. calculate behavior of particle
        for (int i=0; i < particles.size(); i++){
            if(interact){
                if (contourInput){
                    unsigned int contourIdx = -1;
                    bool isOutside = true;
                    if (particleMode == BOIDS && seekInteraction){// get closest point to particle
                        isOutside = false;
                    }
                    // else get closest point to particle only if particle is inside contour
                    ofPoint closestPointInContour = getClosestPointInContour(*particles[i], contour, isOutside, &contourIdx);

                    if(flowInteraction){
                        ofPoint frc = contour.getFlowOffset(particles[i]->pos);
                        if (frc != ofPoint(0,0)){
                            particles[i]->isTouched = true;
                        }
                        if (particleMode == FALL){
                            if(particles[i]->isTouched){
                                particles[i]->force.set(ofPoint(0.0, 500.0)*particles[i]->mass);
                            }
                        }else{
                            particles[i]->addForce(frc*interactionForce);
                        }
                    }

                    if(closestPointInContour != ofPoint(-1, -1)){
                        if(repulseInteraction){ // it is an attractForce but result is more logical saying repulse
                            particles[i]->addAttractionForce(closestPointInContour, interactionRadiusSq, interactionForce);
                        } else if(attractInteraction){
                            particles[i]->addRepulsionForce(closestPointInContour, interactionRadiusSq, interactionForce);
                        } else if(seekInteraction){
                            particles[i]->seek(closestPointInContour, interactionRadiusSq, interactionForce*10.0);
                        } else if(gravityInteraction){
                            particles[i]->addForce(ofPoint(ofRandom(-100, 100), 500.0)*particles[i]->mass);
                            particles[i]->isTouched = true;
                        } else if(bounceInteraction){
                            if(contourIdx != -1) particles[i]->contourBounce(contour.contours[contourIdx]);
                        }

                    } else if(gravityInteraction && particles[i]->isTouched){
                        particles[i]->addForce(ofPoint(0.0, 500.0)*particles[i]->mass);
                    }
                } // if contour input
                if(fluidInteraction){
                    ofPoint frc = fluid.getFluidOffset(particles[i]->pos);
                    particles[i]->addForce(frc*interactionForce);
                }      
            } // if interact particles w input

            if(flock){ // Flocking behavior
                particles[i]->flockingRadiusSq    =   flockingRadiusSq;

                particles[i]->separationStrength    =   separationStrength;
                particles[i]->alignmentStrength     =   alignmentStrength;
                particles[i]->attractionStrength    =   attractionStrength;

                particles[i]->lowThresh             =   lowThresh;
                particles[i]->highThresh            =   highThresh;
                particles[i]->maxSpeed              =   maxSpeed;
            } 

            if(returnToOrigin && (particleMode == GRID || particleMode==FALL) && !gravityInteraction) {particles[i]->returnToOrigin(100, returnToOriginForce);}

            if(particleMode == ANIMATIONS && animation == SNOW){
                float windX = ofSignedNoise(particles[i]->pos.x * 0.003, particles[i]->pos.y * 0.006, ofGetElapsedTimef() * 0.1) * 3.0;
                ofPoint frc;
                frc.x = windX + ofSignedNoise(particles[i]->id, particles[i]->pos.y * 0.04) * 8.0;
                frc.y = ofSignedNoise(particles[i]->id, particles[i]->pos.x * 0.006, ofGetElapsedTimef() * 0.2) * 3.0;
                particles[i]->addForce(frc*particles[i]->mass);
            }
        } // end of for loop for particles update

        if(emit){ // Born new particles
            if(contourInput){
                if(emitInMovement){
                    for(unsigned int i = 0; i < contour.vMaskContours.size(); i++){
                        // born more particles if bigger area
                        float bornNum = bornRate * abs(contour.vMaskContours[i].getArea())/1500.0;
                        addParticles(bornNum, contour.vMaskContours[i], contour);
                    }
                }
                else{
                    for(unsigned int i = 0; i < contour.contours.size(); i++){
                        float range = ofMap(bornRate, 0, 150, 0, 30);
                        addParticles(ofRandom(bornRate-range, bornRate+range), contour.contours[i], contour);
                    }
                }
            }
        }

        // Keep adding particles if it is an animation (unless it is an explosion)
        if(particleMode == ANIMATIONS && animation != EXPLOSION){
            float range = ofMap(bornRate, 0, 60, 0, 15);
            addParticles(ofRandom(bornRate-range, bornRate+range));
        }

        if(flock) flockParticles();
        if(repulse) repulseParticles();

        // 3. add general behaviors like damping/gravity to particles
        for(int i = 0; i < particles.size(); i++){
            particles[i]->addForce(gravity*particles[i]->mass);
            particles[i]->addNoise(turbulence);
            
            particles[i]->opacity = opacity;
            particles[i]->friction = 1-friction/1000;
            particles[i]->bounces = bounces;
            particles[i]->bounceDamping = bounceDamping;
            particles[i]->steers = steers;
            particles[i]->wrapAround = wrapAround;

            if (immortal){ // for GRID & BOID systems
                if(particleMode != BOIDS){ particles[i]->radius = radius; }
                particles[i]->color = ofColor(red, green, blue);
                particles[i]->noFill = noFill;
                particles[i]->drawLine = drawLine;
                particles[i]->drawStroke = drawStroke;
                particles[i]->strokeWidth = strokeWidth;
            }

            if(gravityInteraction) particles[i]->bounces = true;
            
            // BOID FOLLOW PATH
            particles[i]->follow(contour.contours,contourRadius);
           //particles[i]->update(dt);
            

            particles[i]->update(dt);
        }
   } else if (activeStarted) {
        activeStarted = false;
        isFadingIn = false;
        isFadingOut = true;
        startFadeIn = false;
        startFadeOut = true;
        killParticles();
   }
}

void ParticleSystem::draw(){
    if(isActive || isFadingOut || true){
        ofPushStyle();

        //Draw lines between near points
        if(drawConnections){
            ofPushStyle();
            float connectDistSq = connectDist*connectDist;
            ofSetColor(ofColor(red, green, blue), 255);//opacity
            ofSetLineWidth(connectWidth);
            for(int i = 0; i < particles.size(); i++){
                for(int j = i-1; j >= 0; j--){
                    if(particles[i]->pos.squareDistance(particles[j]->pos) < connectDistSq){
                        ofDrawLine(particles[i]->pos, particles[j]->pos);
                    }
                }
            }
            ofPopStyle();
        }

        // Draw particles
        for(int i = 0; i < particles.size(); i++){
            particles[i]->draw();
        }

        ofPopStyle();
    }
}

void ParticleSystem::addParticle(ofPoint pos, ofPoint vel, ofColor color, float radius, float lifetime){
    Particle * newParticle = new Particle();
    float id = totalParticlesCreated;

    newParticle->sizeAge = sizeAge;
    newParticle->opacityAge = opacityAge;
    newParticle->flickers = flickers;
    newParticle->colorAge = colorAge;
    newParticle->noFill = noFill;
    newParticle->drawLine = drawLine;
    newParticle->drawStroke = drawStroke;
    newParticle->strokeWidth = strokeWidth;

    newParticle->friction = 1 - friction/1000;

    newParticle->w = w;
    newParticle->h = h;

    if (particleMode == GRID || particleMode == BOIDS || particleMode == FALL){
        newParticle->immortal = true;

        if (particleMode == BOIDS){
            newParticle->limitSpeed = true;
        }
    } else if (particleMode == ANIMATIONS){
        if(animation == SNOW) {
            newParticle->damping = 0.05;
        } else {
            newParticle->damping = 0.2;
            newParticle->bounceTop = false;
        }
    }

    newParticle->setup(id,pos,vel,color,radius,lifetime);
    particles.push_back(newParticle);

    numParticles++;
    totalParticlesCreated++;
}

void ParticleSystem::addParticles(int n){
    for(int i = 0; i < n; i++){
        ofPoint pos = ofPoint(ofRandom(w), ofRandom(h));
        ofPoint vel = randomVector()*(velocity+randomRange(velocityRnd, velocity));

        if(particleMode == ANIMATIONS && (animation == RAIN || animation == SNOW)){
            pos = ofPoint(ofRandom(w), ofRandom(-5*radius, -10*radius));
            vel.x = 0;
            vel.y = velocity+randomRange(velocityRnd, velocity); // make particles all be going down when born
        }
        else if(particleMode == ANIMATIONS && animation == EXPLOSION){
            pos = ofPoint(ofRandom(w), ofRandom(h, h+radius*15));
            vel.x = 0;
            vel.y = -velocity-randomRange(velocityRnd, velocity); // make particles all be going up when born
        }

        float initRadius = radius + ofRandom(1,radiusRnd);
        float lifetime = this->lifetime + ofRandom(1,lifetimeRnd);

        ofColor col;
        int rand = (int)ofRandom(1,10);
        if (rand <= 2){
            col = colBlue;
        } else if (rand <= 4){
            col = colOrange;
        } else if (rand <= 7){
            col = colNavy;
        } else{
            col = colRed;
        }
        
        addParticle(pos, vel, col, initRadius, lifetime);
    }
}

void ParticleSystem::addParticles(int n, const ofPolyline& contour, Contour& flow){
    for(int i = 0; i < n; i++){
        ofPoint pos, randomVel, motionVel, vel;

        // Create random particles inside contour polyline
        if(emitAllTimeInside || emitInMovement){
            ofRectangle box = contour.getBoundingBox(); // so it is easier that the particles are born inside contour
            ofPoint center = box.getCenter();
            pos.x = center.x + (ofRandom(1.0f) - 0.5f) * box.getWidth();
            pos.y = center.y + (ofRandom(1.0f) - 0.5f) * box.getHeight();

            while(!contour.inside(pos)){
                pos.x = center.x + (ofRandom(1.0f) - 0.5f) * box.getWidth();
                pos.y = center.y + (ofRandom(1.0f) - 0.5f) * box.getHeight();
            }

            // set velocity to random vector direction with 'velocity' as magnitude
            randomVel = randomVector()*(velocity+randomRange(velocityRnd, velocity));
        } else if (emitAllTimeContour){
            float indexInterpolated = ofRandom(0, contour.size());
            pos = contour.getPointAtIndexInterpolated(indexInterpolated);

            // Use normal vector in surface as vel. direction so particle moves out of the contour
            randomVel = -contour.getNormalAtIndexInterpolated(indexInterpolated)*(velocity+randomRange(velocityRnd, velocity));
        }

        // get velocity vector in particle pos
        motionVel = flow.getFlowOffset(pos)*(velocity*5.0+randomRange(velocityRnd, velocity*5.0));
        if(useContourVel){ // slower and poorer result
            motionVel = flow.getVelocityInPoint(pos)*(velocity*5.0+randomRange(velocityRnd, velocity*5.0));
        }
        vel = randomVel*(velocityRnd/100.0) + motionVel*(velocityMotion/100.0);
        pos += randomVector()*emitterSize; // randomize position within a range of emitter size

        float initRadius = radius + randomRange(radiusRnd, radius);
        float lifetime = this->lifetime + randomRange(lifetimeRnd, this->lifetime);

        addParticle(pos, vel, ofColor(red, green, blue), initRadius, lifetime);
    }
}

void ParticleSystem::createParticleGrid(int width, int height){
    for(int y = 0; y < height/gridRes; y++){
        for(int x = 0; x < width/gridRes; x++){
            int xi = (x + 0.5f) * gridRes;
            int yi = (y + 0.5f) * gridRes;

            ofPoint vel = ofPoint(0, 0);
            addParticle(ofPoint(xi, yi), vel, ofColor(red, green, blue), radius, lifetime);
        }
    }
}

void ParticleSystem::removeParticles(int n){
    n = MIN(particles.size(), n);
    for(int i = 0; i < n; i++){
        particles[i]->immortal = false;
    }
}

void ParticleSystem::killParticles(){
    for(int i = 0; i < particles.size(); i++){
        particles[i]->immortal = false;
    }
}

void ParticleSystem::resetTouchedParticles(){
    for(int i = 0; i < particles.size(); i++){
        particles[i]->isTouched = false;
    }
}

void ParticleSystem::bornParticles(){
    // Kill all the remaining particles before creating new ones
    if(!particles.empty()){
        for(int i = 0; i < particles.size(); i++){
            particles[i]->isAlive = false;
        }
    }

    setup(particleMode, w, h); // resets the settings to default
}

void ParticleSystem::setAnimation(Animation animation){
    this->animation = animation;
}

void ParticleSystem::repulseParticles(){
    float repulseDistSq = repulseDist*repulseDist;
    for(int i = 1; i < particles.size(); i++){
        for(int j = i-1; j >= 0; j--){
            if (fabs(particles[i]->pos.x - particles[j]->pos.x) > repulseDist) break; // to speed the loop
            particles[i]->addRepulsionForce( *particles[j], repulseDistSq, 8.0);
        }
    }
}

void ParticleSystem::flockParticles(){
    for(int i = 0; i < particles.size(); i++){
        for(int j = i-1; j >= 0; j--){
            if (fabs(particles[i]->pos.x - particles[j]->pos.x) > flockingRadius) break;
            particles[i]->addFlockingForces(*particles[j]);
        }
    }
}

ofPoint ParticleSystem::randomVector(){
    float angle = ofRandom((float)M_PI * 2.0f);
    return ofPoint(cos(angle), sin(angle));
}

float ParticleSystem::randomRange(float percentage, float value){
    return ofRandom(value-(percentage/100)*value, value+(percentage/100)*value);
}

ofPoint ParticleSystem::getClosestPointInContour(const Particle& particle, const Contour& contour, bool onlyInside, unsigned int* contourIdx){
    ofPoint closestPoint(-1, -1);
    float minDistSqrd = 999999999;

    // Get closest point to particle from the different contours
    for(unsigned int i = 0; i < contour.contours.size(); i++){
        if(!onlyInside || contour.contours[i].inside(particle.pos)){
            ofPoint candidatePoint = contour.contours[i].getClosestPoint(particle.pos);
            float pointDistSqrd = particle.pos.squareDistance(candidatePoint);
            if(pointDistSqrd < minDistSqrd){
                minDistSqrd = pointDistSqrd;
                closestPoint = candidatePoint;
                if(contourIdx != NULL) *contourIdx = i; // save contour index
            }
        }
    }
    return closestPoint;
}

void ParticleSystem::fadeIn(float dt){
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

void ParticleSystem::fadeOut(float dt){
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
