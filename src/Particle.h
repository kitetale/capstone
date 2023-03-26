//
//  Particle.hpp
//  capstone
//
//  Created by Ashley K on 3/22/23.
// ref: https://github.com/fabiaserra/crea/blob/master/src/Particle.h
//

#pragma once
#include "ofMain.h"
#include <stdio.h>

class Particle {
    public:
        Particle();
        
        void setup(float id, ofPoint pos, ofPoint vel, ofColor color, float radius, float lifetime);
        void update(float dt);
        void draw();
        
        void addForce(ofPoint newForce);
        void addNoise(float turbulence);
        void addRepulsionForce(ofPoint posOfForce, float radiusSq, float scale);
        void addAttractionForce(ofPoint posOfForce, float radiusSq, float scale);
        void addRepulsionForce(Particle &p, float radiusSq, float scale);
        void addAttractionForce(Particle &p, float radiusSq, float scale);
        void addRepulsionForce(Particle &p, float scale);
        void returnToOrigin(float radiusSq, float scale);
        
        void addFlockingForces(Particle &p);
        void seek(ofPoint target, float radiusSq, float scale);
        void seek(ofPoint target, float scale);
        void pullToCenter();
        void limitVelocity();
        
        // particle interaction with window margin
        void marginBounce();
        void marginSteer();
        void marginWrap();
        
        void contourBounce(ofPolyline contour);
        
        void kill();
        
        float opacity;
        
        ofPoint pos;
        ofPoint prevPos;
        ofPoint initPos;
        ofPoint vel;
        ofPoint accel;
        ofPoint force;
        ofColor color;
        
        ofPolyline contour;
        
        float id;
        float age;
        float mass;
        float lifetime;
        float friction;
        float initRadius;
        float radius;
        float initColor;
        
        
        bool immortal; // can this particle die?
        bool isAlive; // is this particle alive?
        bool isTouched; // is this particle touched (have applied force)
        
        bool sizeAge; // particle changes size with age?
        bool opacityAge; //particle changes opacity with age?
        bool flickers; // particle flickers right before dying?
        bool colorAge; // particle changes color with age?
        bool noFill; // draw particle with no fill?
        bool drawLine; // draw particle as line from prevPos to Pos
        bool drawStroke; // draw outline around particle?
        float strokeWidth; // stroke line width
        
        bool limitSpeed; //limit the speed of the particle?
        bool bounceDamping; //damp when particle bounces?
        
        bool bounces; //should particle bounce on window edge?
        bool bounceTop; // should particle bounce on top window edge?
        bool steers; //particle steers direction before touching the walls?
        bool wrapAround; //particle wraps around the window edgs?
        
        
        float flockingRadiusSq;
        float lowThresh; //separate flock
        float highThresh; // align flock
        float separationStrength;
        float alignmentStrength;
        float attractionStrength;
        float maxSpeed;
        float damping; // damping amount when particle bounce off walls
        
        int w, h; //particle boundaries
};
