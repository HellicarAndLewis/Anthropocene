#pragma once
#include "ofxOpenCv.h"
#include "ofMain.h"
#include "ofxKinect.h"

enum GreenpeaceModes {BLANK, GUI, VIDEO, VIDEOCIRCLES, KINECTPOINTCLOUD, SLITSCANBASIC, SLITSCANKINECTDEPTHGREY, SPARKLE, VERTICALMIRROR, HORIZONTALMIRROR, KALEIDOSCOPE, COLOURFUR, DEPTH, SHATTER, SELFSLITSCAN, SPIKYBLOBSLITSCAN, MIRRORKALEIDOSCOPE, PARTICLES, WHITEFUR, PAINT, GreenpeaceModes_MAX = PAINT}; //best to use ALL CAPS for enumerated types and constants so you can tell them from ClassNames and variableNames. Use camelCase for variableNames - http://en.wikipedia.org/wiki/CamelCase
 
/* http://stackoverflow.com/questions/2102582/how-can-i-count-the-items-in-an-enum
 For C++, there are various type-safe enum techniques available, and some of those (such as the proposed-but-never-submitted Boost.Enum) include support for getting the size of a enum.
 
 The simplest approach, which works in C as well as C++, is to adopt a convention of declaring a ...MAX value for each of your enum types:
 
 enum Folders { FA, FB, FC, Folders_MAX = FC };
 ContainerClass *m_containers[Folders_MAX + 1];
 ....
 m_containers[FA] = ...; // etc.
 Edit: Regarding { FA, FB, FC, Folders_MAX = FC} versus {FA, FB, FC, Folders_MAX]: I prefer setting the ...MAX value to the last legal value of the enum for a few reasons:
 
 The constant's name is technically more accurate (since Folders_MAX gives the maximum possible enum value).
 Personally, I feel like Folders_MAX = FC stands out from other entries out a bit more (making it a bit harder to accidentally add enum values without updating the max value, a problem Martin York referenced).
 GCC includes helpful warnings like "enumeration value not included in switch" for code such as the following. Letting Folders_MAX == FC + 1 breaks those warnings, since you end up with a bunch of ...MAX enumeration values that should never be included in switch.
 switch (folder)
 {
 case FA: ...;
 case FB: ...;
 // Oops, forgot FC!
 }
*/
    
//slitscan
#include "ofxSlitScan.h"

//timeline intergration
#include "ofxTimeline.h"

//sparkles
#include "Sparkles.h"

//new gui
#include "ofxGui.h"

//bits for the fur
#include "ofxOpticalFlowFarneback.h"

//bits for shattering
#include "Shatter.h"

//bits for SPIKYBLOBSLITSCAN
#include "Spikey.h"

//bits for PARTICLES
#include "Particles.h"

class testApp : public ofBaseApp {
public:
	
	void setup();
	void update();
	void draw();
	void exit();
	
	void drawPointCloud();
	
	void keyPressed(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
    
    //kinect
	ofxKinect kinect;
	
    //computer vision
    // depth image stuff
    ofxCvGrayscaleImage     depthPreCrop;
    ofxCvGrayscaleImage     maskImage;
    
    //from memo
    ofxCvGrayscaleImage		depthOrig;
    ofxCvGrayscaleImage		depthProcessed;
    ofxCvContourFinder		depthContours;
    
    // RGB image stuff
    ofxCvColorImage			colorImageRGB;
    
    // depth
    ofxToggle	invert;
    ofxToggle	mirror;
    ofxIntSlider		preBlur;
    ofxFloatSlider	bottomThreshold;
    ofxFloatSlider	topThreshold;
    //	int		adaptiveBlockSize;
    //	float	adaptiveThreshold;
    ofxIntSlider		erodeAmount;
    ofxIntSlider		dilateAmount;
    ofxToggle	dilateBeforeErode;
    
    // contours
    ofxToggle	findHoles;
    ofxToggle	useApproximation;
    ofxFloatSlider	minBlobSize;
    ofxFloatSlider	maxBlobSize;
    ofxIntSlider		maxNumBlobs;
    
    //jgl one
    ofxFloatSlider leftCrop;
    ofxFloatSlider rightCrop;
    ofxFloatSlider topCrop;
    ofxFloatSlider bottomCrop;
    void maskGrayImage();
    
    //blobs no blob tracker right now....
    //    ofxBlobTracker blobTracker;
    //    void blobAdded(ofxBlob &_blob);
    //    void blobMoved(ofxBlob &_blob);
    //    void blobDeleted(ofxBlob &_blob);
    
    //gui
    bool bShowNonTimelineGUI;
    
	ofxPanel nonTimelineGUI;
	
	// used for viewing the point cloud
	ofEasyCam easyCam;
    
    //cleaning up GUI and basic pragmatic modes to get going
    GreenpeaceModes currentMode;
    bool showMouse;
    bool showTimeline;
    
    //slitscan
    ofxSlitScan slitScan;
    //ofxSlitScan slitScanDepthGrey;
    ofImage depthPixels;
    //directory listing for slitscan
    ofDirectory slitScanDirectory;
    vector<ofImage> slitScans;
    
    void slitScanSliderSlid();
    int prevSlitScan;
    
    //ofxTimeline
    ofxTimeline timeline;
    void bangFired(ofxTLBangEventArgs& args);
    
    //sparkles
    Sparkles someSparkles;
    
    //for painting
    ofxCvColorImage paintCanvas;
    ofImage paintCanvasAsOfImage;
    
    //mirrors and kaleidoscopes
    ofxCvColorImage verticalMirrorImage;
    ofxCvColorImage horizontalMirrorImage;
    ofxCvColorImage kaleidoscopeMirrorImage;
    
    //fur
    ofxOpticalFlowLK flowSolver;
    
    ofxFloatSlider pyramidScale;
    ofxIntSlider pyramidLevels;
    ofxIntSlider windowSize;
    ofxIntSlider expansionAreaDoubleMe;
    ofxFloatSlider expansionSigma;
    ofxToggle flowFeedback;
    ofxToggle gaussianFiltering;
    
    //shatter
    Shatter theShatter;
    
    //spikey
    Spikey theSpikey;
    
    //Particles
    Particles theParticles;
};
