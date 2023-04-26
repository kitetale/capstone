#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxKinect.h"
#include "ofxFlowTools.h"
#include "ParticleSystem.h"
#include "Fluid.h"
#include "Contour.h"

#define GESTURE_FOLLOWER
#ifdef GESTURE_FOLLOWER
//#include "vmo.h"
//#include "helper.h"
#endif

#define KINECT_CONNECTED


#define SCREEN_W 640
#define SCREEN_H 480

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void exit();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    //------------------------ INIT VARIABLES ----------------------------
    int windowWidth, windowHeight;
    
    ofxKinect kinect;
    bool resetKinect;
    int angle; //kinect cam tilt
    float sscale; // scale ratio for screensize
    bool flip; // mirror kinect?
    float nearClipping, farClipping;
    float nearThreshold, farThreshold;
    float minContourSize, maxContourSize;
    float irThreshold;
    
    float time0; //current time
    
    float red, green, blue; //background color
    bool bgGradient;
    
    //----------------------- KINECT DEPTH IMG ---------------------------
    int depthDilate; // num of dilates applied to depth img
    int depthErode; // num of erode applied to depth img
    int depthBlur; // amount of blur applied to depth img
    
    // mocap tracker IR img
    int irDilates; // num of dilates applied to IR img
    int irErode; // num of erode applied to IR img
    int irBlur; // amount of blur applied to IR img
    
    ofImage irImage, irOriginal;
    ofImage depthImage, depthOriginal;
    ofxCvGrayscaleImage grayDiff, grayDepthImage, bgImage;
    ofImage grayThreshNear;
    ofImage grayThreshFar;
    
    bool learnBg;
    
    // for adjusting cam view area
    cv::Mat depthCroppingMask;
    float depthLeftMask, depthRightMask;
    float depthTopMask, depthBottomMask;
    
    cv::Mat irCroppingMask;
    float irLeftMask, irRightMask;
    float irTopMask, irBottomMask;
    
    // for mocap trackers
    //ofxCv::ContourFinder irMarkerFinder;
    //ofxCv::RectTrackerFollower<irMarker> tracker;
    
   // ----------------------------- CONTOUR -----------------------------
    Contour contour;
    unsigned long lastNum; // last number of blobs detected
    bool donePlaying; //done playing four notes
    int curNote;
    int notei[4] = {};
    
    // ----------------------------- FLUID -------------------------------
    Fluid fluid;
 
    // ------------------------ PARTICLE SYSTEM --------------------------
    ParticleSystem *boidsParticles;
  
    // ---------------------------- SOUND --------------------------------
    ofSoundPlayer ambientSound;
    ofSoundPlayer s_c, s_d, s_e, s_g, s_a, s_c2, s_d2,s_e2, s_g2, s_g0, s_f0; // pentatonic xylophone
    vector<ofSoundPlayer> xylophone;
    
    // -------------------------- TRANSITION -----------------------------
    ofFbo fbo;
    int fadeAmount;
    bool useFBO;
    int maxTransitionFrames;
    int nInterpolatedFrames;
    
    ofShader shader;
    ofTexture tex;
    
    ofxCvColorImage colorImage;
    
    bool drawContour;
    bool drawFluid;
    
    ftFluidFlow fluidFlow;
    ftOpticalFlow opticalFlow;
    ftVelocityBridgeFlow velocityBridgeFlow;
    ftDensityBridgeFlow densityBridgeFlow;
    ftTemperatureBridgeFlow temperatureBridgeFlow;
    int densityWidth, densityHeight, simulationWidth, simulationHeight;
    
    bool sand;
};
