//
//  Particle.cpp
//  capstone
//
//  Created by Ashley K on 3/22/23.
// ref: https://github.com/fabiaserra/crea/blob/master/src/Particle.cpp
//

#include "Particle.h"

Particle::Particle(){
    isAlive = true;
    isTouched = false;
    immortal = false;
    bounces = false;
    bounceTop = false;
    steers = false;
    wrapAround = false;
    
    sizeAge = false;
    opacityAge = false;
    colorAge = false;
    flickers = false;
    noFill = false;
    drawLine = false;
    drawStroke = false;
    strokeWidth = 1.2;
    
    limitSpeed = false;
    maxSpeed = 120.0;
    bounceDamping = true;
    damping = 0.6;
    
    age = 0;
    opacity = 0;
    maxOpacity = 255.0;
    startFadeIn = true;
    isFadeIn = true;
    
    w = ofGetWidth();
    h = ofGetHeight();
}

Shape chooseShape(){
    //Shape {DONUT, PIZZA, BUBBLE, SQUARE, TRIANGLE};
    int rand = (int)ofRandom(1,100);
    if (rand<=10){
        return SQUARE;
    }
    if (rand<=30){
        return TRIANGLE;
    }
    if (rand<=60){
        return DONUT;
    }
    if (rand<=80){
        return PIZZA;
    }
    return BUBBLE;
}

void Particle::setup(float id, ofPoint pos, ofPoint vel, ofColor color, float radius, float lifetime){
    this->id = id;
    this->pos = pos;
    this->vel = vel;
    this->color = color;
    this->initRadius = radius;
    this->lifetime = lifetime;
    this->maxLifetime = lifetime;
    
    this->radius = radius;
    this->mass = radius * radius * 0.005f;
    this->prevPos = pos;
    this->initPos = pos;
    this->initColor = color.getHue();
    
    this->stype = chooseShape();
}

void Particle::update(float dt){
    if(isAlive){
        //if opacity decreases over time
        if(age <= 1) opacity = ofMap(age, 0, 1, 0, 255);
        else if (opacityAge) opacity *= (1.0f - (age/lifetime)+(1.2/lifetime));
     
        
        accel = force/mass; // F=ma
        vel += accel * dt; // Euler's method
        vel *= friction; //decay velocity over time
        if (limitSpeed) limitVelocity();
        pos += vel * dt; // x=vt
        force.set(0,0); //reset force
        
        //update lifespan of particle
        age += dt;
        
        if (!immortal && age >= lifetime) isAlive = false;
        else if (immortal) age = fmodf(age, lifetime);
        
        //if radius decreases over time
        if (sizeAge) radius = initRadius * (1.0f - (age/lifetime));
        
        
        // if particle flickers right before death
        if (flickers){
            if ((age/lifetime) > 0.75 && ofRandomf() > (1.4-(age/lifetime))){
                opacity *= 0.5;
            }
        }
        
        // if particle changes color over time
        if (colorAge){
            color.setBrightness(ofMap(age, 0 ,lifetime, 255, 180));
            color.setHue(ofMap(age,0,lifetime, initColor, initColor-100));
        }
        
        // bouncing around window margin
        if (bounces) {marginBounce();}
        else if (steers) {marginSteer();}
        else if (wrapAround) {marginWrap();}
        
    }
}


 
void Particle::draw(){
    if (isAlive){
        ofPushStyle();
        
        ofSetColor(color,opacity);
        if (noFill){
            ofNoFill();
            ofSetLineWidth(2); // stroke width
        } else {
            ofFill();
        }
        
        if (!drawLine){ // just particle drawing
            int res = ofMap(fabs(radius), 0 ,10, 6, 22, true);
            ofSetCircleResolution(res);
            // define shape type in particle generation, then draw that shape based on type
            //Shape {DONUT, PIZZA, BUBBLE, SQUARE, TRIANGLE};
            ofDrawCircle(pos,radius);
            
            
            
            if(drawStroke){
                ofPushStyle();
                ofNoFill();
                ofSetLineWidth(strokeWidth);
                ofSetColor(0, opacity);
                ofDrawCircle(pos, radius);
                ofPopStyle();
            }
        } else { //draw line connecting last pos & pos instead
            ofSetLineWidth(ofMap(radius, 0, 15, 1, 5, true));
            ofDrawLine(pos, pos-vel.getNormalized()*radius);
        }
        
        ofPopStyle();
    }
}

//------------------------------------------------------------------------------
void Particle::follow(vector<ofPolyline> contours, float r){
    ofPoint predictVel = this->vel;
    predictVel.normalize();
    predictVel *= 50; //seeing where this goes in 50 frames, arbitrary num
    ofPoint predictPos = this->pos + predictVel;

    ofPoint normal, target;

    float shortest = 1000000;

     for(int i=0; i<contours.size(); i++){
        ofPolyline ct;
        ct = contours[i];

        auto pts = ct.getVertices();

        for (int j=0; j<pts.size()-1; j++){
            ofPoint a = pts[j];
            ofPoint b = pts[j+1];
            ofPoint normalpt = getNormalPoint(predictPos, a, b);

            if (normalpt.x < a.x || normalpt.x > b.x){
                // if off, set normal to be end of line
                normalpt = b;
            }

            float dist = predictPos.distance(normalpt);
            if (dist < shortest){
                shortest = dist;
                normal = normalpt;
                ofPoint dir = b-a;
                dir.normalize();
                dir *= 10; //TODO
                //ofPoint redir = this->vel.dot()
                target = normalpt + dir;
            }

            if (shortest > r){
                seek(target);
            }
        }
    }
}

ofPoint Particle::getNormalPoint(ofPoint p, ofPoint a, ofPoint b){
    ofPoint ap = p-a;
    ofPoint ab = b-a;
    ab.normalize();
    ab *= ap.dot(ab);
    ofPoint normalpt = a+ab;
    return normalpt;
}
/*
void Particle::update(){
    this->vel += this->accel;
    this->vel.limit(maxSpeed);
    this->pos += this->vel;
    this->accel *= 0;
}
*/
void Particle::applyForce(ofPoint force){
    this->accel += force;
}

void Particle::seek(ofPoint target){
    if (target.distance(this->pos)<0.1) return; //already there
    ofPoint dir = target - this->pos;

    dir.normalize();
    dir *= maxSpeed;

    ofPoint steer = dir - this->vel;
    steer.limit(maxForce);
    
    applyForce(steer);
}



//------------------------------------------------------------------------------

void Particle::addForce(ofPoint newForce){
    force += newForce;
}

void Particle::addNoise(float turbulence){
    float angle = ofSignedNoise(id*0.001f, pos.x*0.005f, pos.y*0.005f, ofGetElapsedTimef()*0.1f) * 20.0f;
    ofPoint noiseVector(cos(angle), sin(angle));
    if(!immortal) force += noiseVector * turbulence * age;
    else force += noiseVector * turbulence;
}

void Particle::addRepulsionForce(ofPoint posOfForce, float radiusSq, float scale){
    ofPoint dir = pos - posOfForce; //direction to force source
    float distSq = pos.squareDistance(posOfForce);
    
    if (distSq < radiusSq){ // add force if particles close enough
        float percent = 1-(distSq/radiusSq);
        dir.normalize();
        force += dir*scale*percent;
    }
}

void Particle::addRepulsionForce(Particle &p, float radiusSq, float scale){
    ofPoint posOfForce; //vector location of particle p
    posOfForce.set(p.pos.x, p.pos.y);
    
    ofPoint dir = pos - posOfForce; //direction to force source
    float distSq = pos.squareDistance(posOfForce);
    
    if (distSq < radiusSq){ // add force if particles close enough
        float percent = 1-(distSq/radiusSq);
        dir.normalize();
        force += dir*scale*percent;
        p.force -= dir*scale*percent; // counterforce of repulsion on p
    }
}

// wrapper function
void Particle::addRepulsionForce(Particle &p, float scale){
    float radius = this->radius + p.radius;
    float radiusSq = radius * radius;
    
    addRepulsionForce(p, radiusSq, scale);
}

void Particle::addAttractionForce(ofPoint posOfForce, float radiusSq, float scale){
    ofPoint dir = pos - posOfForce;
    float distSq = pos.squareDistance(posOfForce);
    
    if (distSq < radiusSq){
        float percent = 1-(distSq/radiusSq);
        dir.normalize();
        force -= dir*scale*percent;
    }
}

void Particle::addAttractionForce(Particle &p, float radiusSq, float scale){
    ofPoint posOfForce;
    posOfForce.set(p.pos.x, p.pos.y);
    
    ofPoint dir = pos - posOfForce;
    float distSq = pos.squareDistance(posOfForce);
    
    if (distSq < radiusSq){
        float percent = 1-(distSq/radiusSq);
        dir.normalize();
        force -= dir*scale*percent;
        p.force += dir*scale*percent; //counterforce of attraction on p
    }
}

void Particle::returnToOrigin(float radiusSq, float scale){
    ofPoint originDir = initPos - pos;
    float distSq = pos.squareDistance(initPos);
    
    float percent = 1;
    if(distSq < radiusSq){
        percent = distSq/radiusSq; // decrease force upon getting closer to the origin
    }
    
    originDir.normalize();
    force += originDir*scale*percent;
}

//separate -> align -> attract & repeat
void Particle::addFlockingForces(Particle &p){
    ofPoint dir = pos - p.pos;
    float distSq = pos.squareDistance(p.pos);
    
    if(0.01f < distSq && distSq < flockingRadiusSq){
        float percent = distSq/flockingRadiusSq;
        
        if(percent < lowThresh){ //separate when below low thresh
            float F = (lowThresh/percent - 1.0f)*separationStrength;
            dir = dir.getNormalized()*F;
            force += dir;
            p.force -= dir;
        } else if (percent <highThresh){ //align when above high thresh
            float threshDiff = highThresh - lowThresh;
            float newPercent = (percent - lowThresh)/highThresh;
            float F = (0.5f - cos(newPercent*M_PI*2.0f)*0.5f+0.5f)*alignmentStrength;
            force += p.vel.getNormalized() * F;
            p.force += vel.getNormalized() * F;
        } else { // attract until high thresh
            float threshDiff = 1.0f - highThresh;
            float newPercent = (percent - highThresh)/threshDiff;
            float F = (0.5f - cos(newPercent*M_PI*2.0f)*0.5f+0.5f)*attractionStrength;
            dir = dir.getNormalized() * F;
            force -= dir;
            p.force += dir;
        }
    }
}

void Particle::pullToCenter(){
    ofPoint center(w/2, h/2);
    ofPoint centerDir = pos - center; //vector pointing to center
    float centerDistSq = centerDir.lengthSquared();
    float distThresh = 900.0f;
    
    if (centerDistSq > distThresh){
        centerDir.normalize();
        float pullStrength = 0.000015f;
        force -= centerDir * ((centerDistSq - distThresh)*pullStrength);
    }
}

void Particle::seek(ofPoint target, float radiusSq, float scale){
    //dir to target
    ofPoint targetDir = target - pos;
    float distSq = pos.squareDistance(target);
    
    //scale force based on distance
    float percent = 1;
    if(distSq < radiusSq){
        percent = distSq / radiusSq;
    }
    
    targetDir.normalize();
    force += targetDir*scale*percent;
}

// seek in general regardless of distance
void Particle::seek(ofPoint target, float scale){
    ofPoint targetDir = target - pos;
    
    float percent = ofRandom(0, 1); // random selection of force
    
    targetDir.normalize();
    vel += targetDir*scale*percent;
}


void Particle::marginBounce(){
    bool isBouncing = false;
    
    if(pos.x > w-radius){
        pos.x = w-radius;
        vel.x *= -1.0;
    }
    else if(pos.x < radius){
        pos.x = radius;
        vel.x *= -1.0;
    }
    if(pos.y > h-radius){
        pos.y = h-radius;
        vel.y *= -1.0;
        isBouncing = true;
    }
    else if(bounceTop && pos.y < radius){
        pos.y = radius;
        vel.y *= -1.0;
    }

    if (isBouncing && bounceDamping){
        vel *= damping;
    }
}

void Particle::marginSteer(){
    float margin = radius*10;

    if(pos.x > w-margin){
        vel.x -= ofMap(pos.x, w-margin, w, maxSpeed/1000.0, maxSpeed/10.0);
    }
    else if(pos.x < margin){
        vel.x += ofMap(pos.x, 0, margin, maxSpeed/10.0, maxSpeed/1000.0);
    }

    if(pos.y > h-margin){
        vel.y -= ofMap(pos.y, h-margin, h, maxSpeed/1000.0, maxSpeed/10.0);
    }
    else if(pos.y < margin){
        vel.y += ofMap(pos.y, 0, margin, maxSpeed/10.0, maxSpeed/10.0);
    }
}

void Particle::marginWrap(){
    if(pos.x-radius > (float)w){
        pos.x = -radius;
    }
    else if(pos.x+radius < 0.0){
        pos.x = w;
    }

    if(pos.y-radius > (float)h){
        pos.y = -radius;
    }
    else if(pos.y+radius < 0.0){
        pos.y = h;
    }
}

void Particle::contourBounce(ofPolyline contour){
    unsigned int index;
    ofPoint contactPoint = contour.getClosestPoint(pos, &index);
    ofVec2f normal = contour.getNormalAtIndex(index);
    vel = vel - 2*vel.dot(normal)*normal; //reflection vector
    vel *= 0.35; //damping
    age += 0.5;
}

void Particle::kill(){
    isAlive = false;
}

void Particle::limitVelocity(){
    if(vel.lengthSquared() > (maxSpeed*maxSpeed)){
        vel.normalize();
        vel *= maxSpeed;
    }
}
