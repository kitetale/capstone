//
//  ParticleSystem.hpp
//  capstone
//
//  Created by Ashley K on 3/22/23.
// ref: https://github.com/fabiaserra/crea/blob/master/src/ParticleSystem.h

#pragma once
#include "ofMain.h"
#include "Particle.h"
#include "Contour.h"
#include "Fluid.h"

enum ParticleMode {EMITTER, BOIDS, GRID, RANDOM, ANIMATIONS, FALL};
enum InputSource {CONTOUR};
enum Animation {SNOW, RAIN, EXPLOSION};

class ParticleSystem
{
    public:
        ParticleSystem();
        ~ParticleSystem();

        void setup(ParticleMode particleMode, int width, int height);
        void update(float dt, Contour& contour, Fluid& fluid, vector<ofPolyline> c);
        void draw();
        void setup(float w, float h, ofPoint initPos, float maxForce, float maxSpeed);
        void update(vector<ofPolyline> c);
        
        void addParticle(ofPoint pos, ofPoint vel, ofColor color, float radius, float lifetime);
        void addParticles(int n);
        void addParticles(int n, const ofPolyline& contour, Contour& flow);

        void createParticleGrid(int width, int height);

        void removeParticles(int n);
        void killParticles();
        void bornParticles();
        void resetTouchedParticles();

        void setAnimation(Animation animation);
    
    ofPoint initPos;
    float maxForce;
    
        
        bool isActive;          // is Particle System active
        bool activeStarted;     // Active has started?
        bool isFadingIn;        // Opacity fading in?
        bool isFadingOut;       // Opacity fading out?
        bool startFadeIn;       // Fade in has started?
        bool startFadeOut;      // Fade out has started?
        float elapsedFadeTime;  // Elapsed time of fade
        float fadeTime;         // Transition time of fade
    
        int w, h; // particle system boundary
        
        float opacity;
        float maxOpacity;
        
        vector<Particle *> particles; //particle vector
        
        int numParticles;
        int totalParticlesCreated;
        
        ParticleMode particleMode;
        
        Animation animation;
        
        bool immortal;              // Can the particles die?
        float velocity;             // Initial velocity magnitude of newborn particles
        float radius;               // Radius of the particles
        float lifetime;             // Lifetime of particles
        float red, green, blue;     // Color of the particles
        
        int nParticles; //init number of particles born at start
        float bornRate; //number of particles born per frame
        
        // Emitter
        float emitterSize;          // Size of the emitter area
        float velocityRnd;          // Magnitude randomness % of the initial velocity
        float velocityMotion;       // Marker motion contribution to the initial velocity
        float lifetimeRnd;          // Randomness of lifetime
        float radiusRnd;            // Randomness of radius
        
        // Grid
        int gridRes;                // Resolution of the grid
        
        // Flocking
        float lowThresh;            // If dist. ratio lower than lowThresh separate
        float highThresh;           // Else if dist. ratio higher than highThresh attract. Else just align
        float separationStrength;   // Separation force
        float attractionStrength;   // Attraction force
        float alignmentStrength;    // Alignment force
        float maxSpeed;             // Max speed particles
        float flockingRadius;       // Radius of flocking
        
        // Graphic output
        bool sizeAge; // particle changes size with age?
        bool opacityAge; //particle changes opacity with age?
        bool flickers; // particle flickers right before dying?
        bool colorAge; // particle changes color with age?
        bool noFill; // draw particle with no fill?
        bool drawLine; // draw particle as line from prevPos to Pos
        bool drawStroke; // draw outline around particle?
        float strokeWidth; // stroke line width
        // \.__./ , _._._
        bool drawConnections;       // Draw a connecting line between close particles?
        float connectDist;          // Maximum distance to connect particles with line
        float connectWidth;         // Connected line width
        
        // Physics
        bool bounces; //should particle bounce on window edge?
        bool bounceTop; // should particle bounce on top window edge?
        bool bounceDamping; //damp when particle bounces?
        bool steers; //particle steers direction before touching the walls?
        bool wrapAround; //particle wraps around the window edgs?
        float friction; //friction to velocity 0~100
        ofPoint gravity; //gravity vector for particles
        float turbulence; //perlin noise turbulence
        bool repulse; //particles repulse in close proximity?
        float repulseDist; //max dist particles should repulse in
        float returnToOriginForce; //how fast particle returns to its position
        
        // Behavior
        bool interact; //can we interact with particles?
        bool emit; //should we emit new particles in each frame?
        bool flock; //flocking behavior?
        bool flowInteraction; //interact w particles using optical flow?
        bool fluidInteraction; //interact w the fluid velocities?
        bool repulseInteraction; //repulse particles from user?
        bool attractInteraction; //attract particles to user?
        bool seekInteraction; // TODO: is seek/target function necessary?? this is to seek tracer/target(marker)
        bool gravityInteraction; //apply gravity force to particles touched by user?
        bool bounceInteraction; //bounce particles w depth contour?
        bool returnToOrigin; //make particles return to original pos?
        
        //Input
        bool contourInput;
        float interactionForce;
        float interactionRadius;
        bool emitInMovement; // Emit particles only in regions that there has been some movement?
        bool emitAllTimeInside; // Emit particles every frame inside all the defined area?
        bool emitAllTimeContour; // Emit particles every frame only on the contour of the defined area?
        bool useFlow; //use optical flow to get motion velocity
        bool useFlowRegion; //use optical flow region to get motion velocity?
        bool useContourArea; //use contour area to interact w particles?
        bool useContourVel; //use contour velocity to interact w particles?
    
        float contourRadius;
    
    protected:
        //helper functions
        ofPoint randomVector();
        float randomRange(float percentage, float value);
        ofPoint getClosestPointInContour(const Particle& particle, const Contour& contour, bool onlyInside = true, unsigned int* contourIdx = NULL);
    
        void fadeIn(float dt);
        void fadeOut(float dt);
    
        void repulseParticles();
        void flockParticles();
};
