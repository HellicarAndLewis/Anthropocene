#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup() {
    ofSetLogLevel(OF_LOG_SILENT);
    ofSetVerticalSync(true);
    ofSetFrameRate(30);
	//ofEnableSmoothing();
    ofBackground(255, 255, 255);
	
    //kinect
	kinect.init();
    // enable depth->video image calibration
    //    bool infraRed = true;
    //    bool colourVideo = false;
    //    bool texture = true;
    //	kinect.init(infraRed, colourVideo, texture);
    kinect.setRegistration(true);
	kinect.enableDepthNearValueWhite(true);
	kinect.open();
    
    // depth image stuff
    depthPreCrop.allocate(kinect.width, kinect.height);
    maskImage.allocate(kinect.width, kinect.height);
    
    //from memo
    depthOrig.allocate(kinect.width, kinect.height);
    depthProcessed.allocate(kinect.width, kinect.height);
    
    // RGB image stuff
    colorImageRGB.allocate(kinect.width, kinect.height);
    //greyIRSingleChannel.allocate(kinect.width, kinect.height);
    
    ofSetVerticalSync(true);
	ofSetFrameRate(30);
    
    ofSetVerticalSync(true);
	
	nonTimelineGUI.setup();
    //crop
    nonTimelineGUI.add(leftCrop.setup("Crop Left", 0.f, 0.f, 1.f));
    nonTimelineGUI.add(rightCrop.setup("Crop Right", 1.f, 0.f, 1.f));
    nonTimelineGUI.add(topCrop.setup("Crop Top", 0.f, 0.f, 1.f));
    nonTimelineGUI.add(bottomCrop.setup("Crop Bottom", 1.f, 0.f, 1.f));
    // depth
    nonTimelineGUI.add(invert.setup("Depth invert", false));
    nonTimelineGUI.add(mirror.setup("Depth mirror", true));
    nonTimelineGUI.add(preBlur.setup("Depth Pre blur", 1,0,20));
    nonTimelineGUI.add(bottomThreshold.setup("Depth Bottom threshold", 0,0,1));
    nonTimelineGUI.add(topThreshold.setup("Depth Top threshold", 1,0,1));
	nonTimelineGUI.add(erodeAmount.setup("Depth Erode amout",1,0,10));
	nonTimelineGUI.add(dilateAmount.setup("Depth Dilate amount", 1,0,10));
    nonTimelineGUI.add(dilateBeforeErode.setup("Depth Dilate before erode", false));
    // contours
    nonTimelineGUI.add(maxNumBlobs.setup("Contours max number of blobs", 1,1,11));
    nonTimelineGUI.add(findHoles.setup("Contours find holes", false));
    nonTimelineGUI.add(useApproximation.setup("Contours use approximation", true));
    nonTimelineGUI.add(minBlobSize.setup("Contours min blob size", 0,0,1));
    nonTimelineGUI.add(maxBlobSize.setup("Contours max blob size", 1,0,1));
    // flow
    nonTimelineGUI.add(pyramidScale.setup("Pyramid scale", 0.5f, 0.f, 0.75f));
    nonTimelineGUI.add(pyramidLevels.setup("Pyramid levels", 3, 1, 100));
    nonTimelineGUI.add(windowSize.setup("Window size", 10, 1, 100));
    nonTimelineGUI.add(expansionAreaDoubleMe.setup("Half expansion area", 3,1,10));
    nonTimelineGUI.add(expansionSigma.setup("Expansion Sigma", 1.5f, 0.f, 10.f));
    nonTimelineGUI.add(flowFeedback.setup("Flow feedback", false));
    nonTimelineGUI.add(gaussianFiltering.setup("Gaussian filtering", false));
    
    nonTimelineGUI.loadFromFile("settings.xml");
    
    //showing
    bShowNonTimelineGUI = true;
    showMouse = true; //show the mouse by default
    showTimeline = true; //and timeline
    
    //scan for slitscans, so can populate gui correctly...
    //now scan the slit scan directory for all the files you can
	slitScanDirectory.listDir("bigMaps/");
	slitScanDirectory.sort(); // in linux the file system doesn't return file lists ordered in alphabetical order
    
	//allocate the vector to have as many ofImages as files
	if( slitScanDirectory.size() ){
		slitScans.assign(slitScanDirectory.size(), ofImage());
	}
    
    //ofxTimeLine
    ofxTimeline::removeCocoaMenusFromGlut("HAndLGreenpeace");
	timeline.setup();
    
    timeline.addVideoTrack("video", "movies/Greenpeace_Final.mov");
    timeline.addAudioTrack("audio", "audio/Greenpeace_Final.aiff");
    timeline.setDurationInSeconds(timeline.getAudioTrack("audio")->getDuration());
    timeline.addBangs("bangs");
	timeline.addFlags("scene");
    timeline.addCurves("slitscan", ofRange(0, slitScanDirectory.size()-1));
    timeline.addCurves("star", ofRange(2, 12));
    timeline.addCurves("offset", ofRange(0.f, 1.f));
    
	timeline.setPageName("Page 1");
	timeline.setCurrentPage(0);
    
	timeline.enableSnapToOtherKeyframes(true);
	timeline.setLoopType(OF_LOOP_NORMAL);
	
	ofAddListener(timeline.events().bangFired, this, &ofApp::bangFired);
    
    //slitscan
    
    ofImage distortionMap;
    distortionMap.loadImage("distortionBig.png");
    distortionMap.resize(timeline.getVideoPlayer("video")->getWidth(), timeline.getVideoPlayer("video")->getHeight());
    
    //set up a slit scan with a maximum capacity of frames
    // in the distortion buffer
    slitScan.setup(timeline.getVideoPlayer("video")->getWidth(),timeline.getVideoPlayer("video")->getHeight(), 30);
    
    slitScan.setDelayMap(distortionMap);
    //blending means the edges between the scans are feathered
    slitScan.setBlending(true);
    //time delay is the deepest in history the delay can go
    //and width is the number of frames the distortion will encompass
    //note that the delay cannot be more than the total capacity
    slitScan.setTimeDelayAndWidth(30, 30);
    
	// you can now iterate through the files and load them into the ofImage vector
	for(int i = 0; i < (int)slitScanDirectory.size(); i++){
		slitScans[i].loadImage(slitScanDirectory.getPath(i));
	}
    
    //gui bits...
    slitScanSliderSlid(); // call it once to initialise
    prevSlitScan = 0;
    
    //sparkles
    someSparkles.setup(timeline.getVideoPlayer("video")->getWidth(),timeline.getVideoPlayer("video")->getHeight());
    
    //paint
    paintCanvas.allocate(timeline.getVideoPlayer("video")->getWidth(), timeline.getVideoPlayer("video")->getHeight());
    paintCanvasAsOfImage.allocate(timeline.getVideoPlayer("video")->getWidth(), timeline.getVideoPlayer("video")->getHeight(), OF_IMAGE_COLOR);
    
    paintCanvas.set(0); //black! so much black...
    
    //opptimising mirrors
    verticalMirrorImage.allocate(timeline.getVideoPlayer("video")->getWidth(), timeline.getVideoPlayer("video")->getHeight());
    horizontalMirrorImage.allocate(timeline.getVideoPlayer("video")->getWidth(), timeline.getVideoPlayer("video")->getHeight());
    kaleidoscopeMirrorImage.allocate(timeline.getVideoPlayer("video")->getWidth(), timeline.getVideoPlayer("video")->getHeight());
    
    ofBackground(ofColor::black);
    
    //fur
    
    int iterationsPerLevel = 1;
    
    flowSolver.setup(kinect.getWidth()/2.f, kinect.getHeight()/2.f,
                     pyramidScale,
                     pyramidLevels,
                     windowSize,
                     iterationsPerLevel,
                     expansionAreaDoubleMe*2,
                     expansionSigma,
                     flowFeedback,
                     gaussianFiltering);
    
    //shatter
    theShatter.setup(timeline.getVideoPlayer("video")->getWidth(),timeline.getVideoPlayer("video")->getHeight());
    
    //spikey!
    theSpikey.setup(timeline.getVideoPlayer("video")->getWidth(),timeline.getVideoPlayer("video")->getHeight());
    
    //flocking!
    theParticles.setup(timeline.getVideoPlayer("video")->getWidth(),timeline.getVideoPlayer("video")->getHeight());
}

//--------------------------------------------------------------
void ofApp::exit(){
	kinect.close();
}

//--------------------------------------------------------------
void ofApp::slitScanSliderSlid(){
    ofImage distortionMap;
    int theCurrentSlitScan = timeline.getValue("slitscan");
    distortionMap.setFromPixels(slitScans[theCurrentSlitScan]);
    distortionMap.resize(timeline.getVideoPlayer("video")->getWidth(), timeline.getVideoPlayer("video")->getHeight());
    slitScan.setDelayMap(distortionMap);
}

//--------------------------------------------------------------
void ofApp::update() {
    //kinect
    kinect.update();
    
    // there is a new frame and we are connected
    if(kinect.isFrameNew()) {
        
        // load grayscale depth image from the kinect source
        depthPreCrop.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
        
        if(mirror){
            depthPreCrop.mirror(false, true);
        }
        
        maskGrayImage();
        
        depthPreCrop.flagImageChanged();
        
        // save original depth, and do some preprocessing
        
        depthOrig = depthPreCrop; //coopy cropped image into orig
        depthProcessed = depthOrig; //copy orig into processd
        colorImageRGB = kinect.getPixels(); //getting colour pixels
        //greyIRSingleChannel = kinect.getPixels(); //getting IR single channel pixels
        
        if(invert) depthProcessed.invert();
        if(mirror) {
            colorImageRGB.mirror(false, true);
            //greyIRSingleChannel.mirror(false, true);
        }
        
        depthOrig.flagImageChanged();
        depthProcessed.flagImageChanged();
        colorImageRGB.flagImageChanged();
        //greyIRSingleChannel.flagImageChanged();
        
        if(preBlur) cvSmooth(depthProcessed.getCvImage(), depthProcessed.getCvImage(), CV_BLUR , preBlur*2+1);
        if(topThreshold) cvThreshold(depthProcessed.getCvImage(), depthProcessed.getCvImage(), topThreshold * 255, 255, CV_THRESH_TRUNC);
        if(bottomThreshold) cvThreshold(depthProcessed.getCvImage(), depthProcessed.getCvImage(), bottomThreshold * 255, 255, CV_THRESH_TOZERO);
        if(dilateBeforeErode) {
            if(dilateAmount) cvDilate(depthProcessed.getCvImage(), depthProcessed.getCvImage(), 0, dilateAmount);
            if(erodeAmount) cvErode(depthProcessed.getCvImage(), depthProcessed.getCvImage(), 0, erodeAmount);
        } else {
            if(erodeAmount) cvErode(depthProcessed.getCvImage(), depthProcessed.getCvImage(), 0, erodeAmount);
            if(dilateAmount) cvDilate(depthProcessed.getCvImage(), depthProcessed.getCvImage(), 0, dilateAmount);
        }
        depthProcessed.flagImageChanged();
        
        // do adaptive threshold
        //	if(adaptiveThreshold) {
        //		cvAdaptiveThreshold(depthProcessed.getCvImage(), depthAdaptive.getCvImage(), 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 2*adaptiveBlockSize+1, adaptiveThreshold);
        //		depthAdaptive.invert();
        //		depthAdaptive.flagImageChanged();
        //	}
        
        // accumulate older depths to smooth out
        //	if(accumOldWeight) {
        //		cvAddWeighted(depthAccum.getCvImage(), accumOldWeight, depthProcessed.getCvImage(), accumNewWeight, 0, depthAccum.getCvImage());
        //		if(accumBlur) cvSmooth(depthAccum.getCvImage(), depthAccum.getCvImage(), CV_BLUR , accumBlur * 2 + 1);
        //		if(accumThreshold) cvThreshold(depthAccum.getCvImage(), depthAccum.getCvImage(), accumThreshold * 255, 255, CV_THRESH_TOZERO);
        //
        //		depthAccum.flagImageChanged();
        //
        //	} else {
        //		depthAccum = depthProcessed;
        //	}
        //
        
        // find contours
        depthContours.findContours(depthProcessed,
                                   minBlobSize * minBlobSize * depthProcessed.getWidth() * depthProcessed.getHeight(),
                                   maxBlobSize * maxBlobSize * depthProcessed.getWidth() * depthProcessed.getHeight(),
                                   maxNumBlobs, findHoles, useApproximation);
        
        //blobTracker.update(grayImage, -1, minArea, maxArea, nConsidered, 20, findHoles, true);
        
        //now do the diff bits for the PAINT mode
        ofxCvGrayscaleImage thresholdedDepthImageForPaint;
        thresholdedDepthImageForPaint.setFromPixels(depthProcessed.getPixelsRef());
        
        thresholdedDepthImageForPaint.resize(paintCanvas.getWidth(), paintCanvas.getHeight());
        thresholdedDepthImageForPaint.flagImageChanged();
        
        // loop through pixels
        //  - add new colour pixels into canvas
        unsigned char *canvasPixels = paintCanvas.getPixels();
        unsigned char *diffPixels = thresholdedDepthImageForPaint.getPixels();
        //        int r = colours[colourIndex].r;
        //        int g = colours[colourIndex].g;
        //        int b = colours[colourIndex].b;
        
        
        //int r = (int)((0.5f + ofNoise(ofGetElapsedTimef())*0.5f)*255.f);
        int r = 255;
        //int matchCount = 0;
        for(int i = 0; i < paintCanvas.width*paintCanvas.height; i++) {
            if(diffPixels[i]) {
                //paint in the new colour if
                canvasPixels[i*3] = r;
                canvasPixels[i*3+1] = r;
                canvasPixels[i*3+2] = r;
            }else{
                int greyScale = (int)(canvasPixels[i*3]*0.9f);
                
                canvasPixels[i*3] = greyScale;
                canvasPixels[i*3+1] = greyScale;
                canvasPixels[i*3+2] = greyScale;
            }
            //            if(canvasPixels[i*3]==r && canvasPixels[i*3+1]==g && canvasPixels[i*3+2]==b) {
            //                matchCount++;
            //            }
        }
        
        //        float totalCount = paintCanvas.width*paintCanvas.height;
        //        float proportionOfMatchedPixels = (float)matchCount/totalCount;
        
        paintCanvas.blur();
        paintCanvas.flagImageChanged();
        
        paintCanvasAsOfImage.setFromPixels(paintCanvas.getPixelsRef());
        paintCanvasAsOfImage.update();
        
        flowSolver.setPyramidScale(pyramidScale);
        flowSolver.setPyramidLevels(pyramidLevels);
        flowSolver.setWindowSize(windowSize);
        flowSolver.setExpansionArea(expansionAreaDoubleMe*2);
        flowSolver.setExpansionSigma(expansionSigma);
        flowSolver.setFlowFeedback(flowFeedback);
        flowSolver.setGaussianFiltering(gaussianFiltering);
        
        flowSolver.update(depthProcessed);
    }
    
    
    //Dirty filthy hack
    if(currentMode != SLITSCANBASIC){
        prevSlitScan = -1;
    }
    
    switch(currentMode){
        case BLANK: //image drawing mode
            break;
        case GUI: //GUI MODE
            break;
        case VIDEO:
            break;
        case VIDEOCIRCLES: //the film as circles
            break;
        case KINECTPOINTCLOUD: //draw the kinect camera depth cloud
            break;
        case SLITSCANBASIC: //slit scan the movie on the grey from the kinect depth grey
        {
            //check slit scan...
            int theCurrentSlitScan = timeline.getValue("slitscan");
            if(prevSlitScan != theCurrentSlitScan){
                slitScanSliderSlid(); //only update when you have to...
                prevSlitScan = theCurrentSlitScan;
            }
            
            if(timeline.getVideoPlayer("video")->isFrameNew()){
                slitScan.addImage(timeline.getVideoPlayer("video")->getPixelsRef());
            }
        }
            break;
        case SLITSCANKINECTDEPTHGREY: //slit scan the movie on the grey from the kinect depth grey
        {
            if(timeline.getVideoPlayer("video")->isFrameNew()){
                //kinect slitscan
                //depthPixels.setFromPixels(kinect.getDepthPixelsRef());
                depthPixels.setFromPixels(depthProcessed.getPixelsRef());
                depthPixels.resize(timeline.getVideoPlayer("video")->getWidth(), timeline.getVideoPlayer("video")->getHeight());
                //            slitScanDepthGrey.setDelayMap(depthPixels);
                //            slitScanDepthGrey.addImage(timeline.getVideoPlayer("video")->getPixelsRef());
                slitScan.setDelayMap(depthPixels);
                slitScan.addImage(timeline.getVideoPlayer("video")->getPixelsRef());
            }
        }
            break;
        case SPARKLE: //sparkles on the slitscan
        {
            //update the sparkles come what may...
            someSparkles.update(&depthContours);
            someSparkles.draw(ofColor::white);
            //someSparkles.draw(timeline.getColor("colour"));
            
            ofImage distortionMap;
            distortionMap.allocate(someSparkles.theFBO.getWidth(), someSparkles.theFBO.getHeight(), OF_IMAGE_COLOR);
            
            someSparkles.theFBO.readToPixels(distortionMap.getPixelsRef());
            
            distortionMap.resize(timeline.getVideoPlayer("video")->getWidth(), timeline.getVideoPlayer("video")->getHeight());
            slitScan.setDelayMap(distortionMap);
            
            if(timeline.getVideoPlayer("video")->isFrameNew()){
                slitScan.addImage(timeline.getVideoPlayer("video")->getPixelsRef());
            }
        }
            break;
        case VERTICALMIRROR: //vertical mirror
        {
            if(timeline.getVideoPlayer("video")->isFrameNew()){
                verticalMirrorImage.setFromPixels(timeline.getVideoPlayer("video")->getPixels(), verticalMirrorImage.getWidth(), verticalMirrorImage.getHeight());
                
                verticalMirrorImage.updateTexture();
            }
        }
            break;
        case PAINT: //body painting diff
        {
            slitScan.setDelayMap(paintCanvasAsOfImage);
            if(timeline.getVideoPlayer("video")->isFrameNew()){
                slitScan.addImage(timeline.getVideoPlayer("video")->getPixelsRef());
            }
        }
            break;
        case HORIZONTALMIRROR: //HORIZONTALMIRROR mirror
        {
            if(timeline.getVideoPlayer("video")->isFrameNew()){
                horizontalMirrorImage.setFromPixels(timeline.getVideoPlayer("video")->getPixels(), horizontalMirrorImage.getWidth(), horizontalMirrorImage.getHeight());
                
                horizontalMirrorImage.updateTexture();
            }
        }
            break;
        case KALEIDOSCOPE: //kaleidsocope
        {
            if(timeline.getVideoPlayer("video")->isFrameNew()){
                kaleidoscopeMirrorImage.setFromPixels(timeline.getVideoPlayer("video")->getPixels(), kaleidoscopeMirrorImage.getWidth(), kaleidoscopeMirrorImage.getHeight());
                
                kaleidoscopeMirrorImage.updateTexture();
            }
        }
            break;
        case COLOURFUR: //COLOURFUR
        {
        }
            break;
        case DEPTH: //DEPTH
        {
        }
            break;
        case SHATTER:
        {
            //update the shatter
            theShatter.update(&depthContours);
            theShatter.draw(ofColor::white);
            
            ofImage distortionMap;
            distortionMap.allocate(theShatter.theFBO.getWidth(), theShatter.theFBO.getHeight(), OF_IMAGE_COLOR);
            
            theShatter.theFBO.readToPixels(distortionMap.getPixelsRef());
            
            distortionMap.resize(timeline.getVideoPlayer("video")->getWidth(), timeline.getVideoPlayer("video")->getHeight());
            slitScan.setDelayMap(distortionMap);
            
            if(timeline.getVideoPlayer("video")->isFrameNew()){
                slitScan.addImage(timeline.getVideoPlayer("video")->getPixelsRef());
            }
        }
            break;
        case SELFSLITSCAN:
        {
            if(timeline.getVideoPlayer("video")->isFrameNew()){
                //self slitscan
                //                ofImage selfSlitScanDelayMap;
                //                selfSlitScanDelayMap.allocate(timeline.getVideoPlayer("video")->getWidth(), timeline.getVideoPlayer("video")->getHeight(), OF_IMAGE_COLOR);
                //                selfSlitScanDelayMap.setFromPixels(timeline.getVideoPlayer("video")->getPixelsRef());
                
                slitScan.setDelayMap(timeline.getVideoPlayer("video")->getPixelsRef());
                slitScan.addImage(timeline.getVideoPlayer("video")->getPixelsRef());
            }
        }
            break;
        case SPIKYBLOBSLITSCAN:
        {
            //SPIKYBLOBSLITSCAN
            //update the spikes come what may...
            theSpikey.update(&depthContours);
            theSpikey.draw(ofColor::white);
            
            ofImage distortionMap;
            distortionMap.allocate(theSpikey.theFBO.getWidth(), theSpikey.theFBO.getHeight(), OF_IMAGE_COLOR);
            
            theSpikey.theFBO.readToPixels(distortionMap.getPixelsRef());
            
            distortionMap.resize(timeline.getVideoPlayer("video")->getWidth(), timeline.getVideoPlayer("video")->getHeight());
            slitScan.setDelayMap(distortionMap);
            
            if(timeline.getVideoPlayer("video")->isFrameNew()){
                slitScan.addImage(timeline.getVideoPlayer("video")->getPixelsRef());
            }
        }
            break;
        case MIRRORKALEIDOSCOPE: //MIRRORKALEIDOSCOPE mirror
        {
            if(timeline.getVideoPlayer("video")->isFrameNew()){
                verticalMirrorImage.setFromPixels(timeline.getVideoPlayer("video")->getPixels(), verticalMirrorImage.getWidth(), verticalMirrorImage.getHeight());
                
                verticalMirrorImage.updateTexture();
                
                kaleidoscopeMirrorImage.setFromPixels(timeline.getVideoPlayer("video")->getPixels(), kaleidoscopeMirrorImage.getWidth(), kaleidoscopeMirrorImage.getHeight());
                
                kaleidoscopeMirrorImage.updateTexture();
            }
        }
            break;
        case PARTICLES:
        {
            //PARTICLES
            theParticles.update(&depthContours);
            theParticles.draw(ofColor::white);
            
            ofImage distortionMap;
            distortionMap.allocate(theParticles.theFBO.getWidth(), theParticles.theFBO.getHeight(), OF_IMAGE_COLOR);
            
            theParticles.theFBO.readToPixels(distortionMap.getPixelsRef());
            
            distortionMap.resize(timeline.getVideoPlayer("video")->getWidth(), timeline.getVideoPlayer("video")->getHeight());
            slitScan.setDelayMap(distortionMap);
            
            if(timeline.getVideoPlayer("video")->isFrameNew()){
                slitScan.addImage(timeline.getVideoPlayer("video")->getPixelsRef());
            }
        }
            break;
        case WHITEFUR: //WHITEFUR, nowt
        {
        }
            break;
        default:
            break;
    }
}

void ofApp::maskGrayImage(){
    //make sure its black
    cvSet(maskImage.getCvImage(), cvScalar(0));
    
    float leftCropInPixels = leftCrop * kinect.getWidth();
    float rightCropInPixels = rightCrop * kinect.getWidth();
    float topCropInPixels = topCrop * kinect.getHeight();
    float bottomCropInPixels = bottomCrop * kinect.getHeight();
    
    CvPoint topLeft = cvPoint(leftCropInPixels, topCropInPixels);
    CvPoint bottomRight = cvPoint(rightCropInPixels,bottomCropInPixels);
    
    cvRectangle(maskImage.getCvImage(), topLeft, bottomRight, cvScalar(255), CV_FILLED);
    maskImage.flagImageChanged();
    //and it with the grayImage from the kinect, to only get the pixels that you want to look at, and then do blobs on those
    cvAnd(maskImage.getCvImage(), depthPreCrop.getCvImage(), depthPreCrop.getCvImage()); //can this be done "in place"?
    depthPreCrop.flagImageChanged();
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofBackground(0, 0, 0);
	ofSetColor(255, 255, 255);
    
    switch (currentMode) {
        case BLANK: //nothing
            ofFill();
            ofSetColor(0);
            ofRect(0,0,ofGetWidth(),ofGetHeight()); //draw a black rectangle
            break;
        case GUI: //image drawing mode
        {
            ofFill();
            ofSetColor(0);
            ofRect(0,0,ofGetWidth(),ofGetHeight()); //draw a black rectangle
            
            int imageOffSet = 10;
            int imageWidth = 320;
            int imageHeight = 240;
            int imageX = imageOffSet;
            
            // draw everything
            ofSetColor(ofColor::white);
            ofEnableAlphaBlending();
            flowSolver.drawColored(imageWidth, imageHeight, 10, 3);
            ofDisableAlphaBlending();
            ofSetColor(ofColor::royalBlue);
            ofDrawBitmapString("Flow", imageX, imageOffSet);
            ofSetColor(ofColor::white);
            colorImageRGB.draw(imageX, imageHeight+imageOffSet, imageWidth, imageHeight);
            //greyIRSingleChannel.draw(imageX, imageHeight+imageOffSet, imageWidth, imageHeight);
            ofSetColor(ofColor::royalBlue);
            ofDrawBitmapString("Kinect Video", imageX, imageHeight+imageOffSet);
            imageX += imageOffSet+imageWidth;
            ofSetColor(ofColor::white);
            kinect.drawDepth(imageX, imageHeight+imageOffSet, imageWidth, imageHeight);
            ofSetColor(ofColor::royalBlue);
            ofDrawBitmapString("Kinect", imageX, imageHeight+imageOffSet);
            imageX += imageOffSet+imageWidth;
            ofSetColor(ofColor::white);
            maskImage.draw(imageX,imageHeight+imageOffSet, imageWidth, imageHeight);
            ofSetColor(ofColor::royalBlue);
            ofDrawBitmapString("Mask", imageX, imageHeight+imageOffSet);
            imageX = imageOffSet;
            ofSetColor(ofColor::white);
            depthOrig.draw(imageX,imageHeight+imageOffSet+imageHeight+imageOffSet, imageWidth, imageHeight);
            ofSetColor(ofColor::royalBlue);
            ofDrawBitmapString("Original Depth", imageX, imageHeight+imageOffSet+imageHeight+imageOffSet);
            imageX += imageOffSet+imageWidth;
            ofSetColor(ofColor::white);
            depthProcessed.draw(imageX,imageHeight+imageOffSet+imageHeight+imageOffSet, imageWidth, imageHeight);
            ofSetColor(ofColor::royalBlue);
            ofDrawBitmapString("Depth Processed", imageX, imageHeight+imageOffSet+imageHeight+imageOffSet);
            imageX += imageOffSet+imageWidth;
            ofSetColor(ofColor::white);
            depthContours.draw(imageX, imageHeight+imageOffSet+imageHeight+imageOffSet, imageWidth, imageHeight);
            ofSetColor(ofColor::royalBlue);
            ofDrawBitmapString("Depth Contours", imageX, imageHeight+imageOffSet+imageHeight+imageOffSet);
            ofSetColor(ofColor::skyBlue);
            // draw instructions
            stringstream reportStream;
            reportStream
            << "f to fullscreen, g to show/hide timeline, m to show/hide mouse" << endl
            << "a/s to cycle through scenes" << endl
            << "Function	                                      Shortcut" << endl
            << "Cut Selection	                                  command+x" << endl
            << "Copy Selection	                                  command+c" << endl
            << "Paste Selection	                                  command+v" << endl
            << "Undo	                                          command+z" << endl
            << "Redo	                                          shift+command+z" << endl
            << "Select all keyframes in Focused track	          command+a" << endl
            << "Add all keyframes in Focused track to selection   command+shift+a" << endl
            << "Delete all selected keyframes	                  delete or backspace" << endl
            << "Nudge keyframes a little	                      arrow keys" << endl
            << "Nudge keyframes a little more	                  shift+arrow keys" << endl
            << "Expand Focused track	                          alt+e" << endl
            << "Collapse all tracks	                              alt+c" << endl
            << "Evenly distribute track sizes	                  alt+shift+c" << endl
            //                << "accel is: " << ofToString(kinect.getMksAccel().x, 2) << " / "
            //                << ofToString(kinect.getMksAccel().y, 2) << " / "
            //                << ofToString(kinect.getMksAccel().z, 2) << endl
            << ", fps: " << ofGetFrameRate() << endl
            << "press shift squerty 1-5 & 0 to change the led mode" << endl;
            ofDrawBitmapString(reportStream.str(),20,ofGetHeight()/2.f);
            
            stringstream m;
            m << "fps " << ofGetFrameRate() << endl
            << "pyramid scale: " << flowSolver.getPyramidScale() << " p/P" << endl
            << "pyramid levels: " << flowSolver.getPyramidLevels() << " l/L" << endl
            << "averaging window size: " << flowSolver.getWindowSize() << " w/W" << endl
            << "iterations per level: " << flowSolver.getIterationsPerLevel() << " i/I" << endl
            << "expansion area: " << flowSolver.getExpansionArea() << " a/A" << endl
            << "expansion sigma: " << flowSolver.getExpansionSigma() << " s/S" << endl
            << "flow feedback: " << flowSolver.getFlowFeedback() << " f/F" << endl
            << "gaussian filtering: " << flowSolver.getGaussianFiltering() << " g/G";
            
            ofDrawBitmapString(m.str(), 20+320, 20);
            
        }
            break;
        case VIDEO: //the film
            ofFill();
            ofSetColor(255);
            timeline.getVideoPlayer("video")->draw(0, 0, ofGetWidth(),ofGetHeight());
            break;
        case VIDEOCIRCLES: //the film as circles
        {
            ofFill();
            ofSetColor(0);
            ofRect(0,0,ofGetWidth(),ofGetHeight()); //draw a black rectangle
            if (timeline.getVideoPlayer("video")->isLoaded()) {
                unsigned char * pixels = timeline.getVideoPlayer("video")->getPixels();
                ofPixelsRef pixelsRef = timeline.getVideoPlayer("video")->getPixelsRef();
                
                // let's move through the "RGB(A)" char array
                // using the red pixel to control the size of a circle.
                //ofSetColor(timeline.getColor("colour"));
                ofSetColor(ofColor::lightBlue);
                
                float circleSpacing = 10.f;
                
                float widthRatio = ofGetWidth()/timeline.getVideoPlayer("video")->getWidth();
                float heightRatio = ofGetHeight()/timeline.getVideoPlayer("video")->getHeight();
                
                for(int i = 0; i < timeline.getVideoPlayer("video")->getWidth(); i+= 8){
                    for(int j = 0; j < timeline.getVideoPlayer("video")->getHeight(); j+= 8){
                        ofColor pixelColor = timeline.getVideoPlayer("video")->getPixelsRef().getColor(i, j);
                        int b = pixelColor.b;
                        float val = 1 - ((float)b / 255.0f); //more blue in the arctic!
                        ofCircle(i*widthRatio, j*heightRatio, circleSpacing * val);
                    }
                }
            }
        }
            break;
        case KINECTPOINTCLOUD: //draw the kinect camera depth cloud
            easyCam.begin();
            drawPointCloud();
            easyCam.end();
            break;
        case SLITSCANBASIC: //slit scan the movie on depth png
            slitScan.getOutputImage().draw(0, 0, ofGetWidth(), ofGetHeight());
            
            //white fur
            ofEnableAlphaBlending();
            flowSolver.drawGrey(ofGetWidth(),ofGetHeight(), 10, 3);
            ofDisableAlphaBlending();
            
            break;
        case SLITSCANKINECTDEPTHGREY: //slit scan the movie on the grey from the kinect depth grey
            slitScan.getOutputImage().draw(0, 0, ofGetWidth(), ofGetHeight());
            //slitScanDepthGrey.getOutputImage().draw(0, 0, ofGetWidth(), ofGetHeight());
            break;
        case SPARKLE:
            //do some sparkles - used the slit scan to hold it....
            slitScan.getOutputImage().draw(0, 0, ofGetWidth(), ofGetHeight());
            //ofSetColor(255,255,255);
            //someSparkles.theFBO.draw(0, 0, ofGetWidth(), ofGetHeight());
            break;
        case VERTICALMIRROR:
        {
            bool usingNormTexCoords = ofGetUsingNormalizedTexCoords();
            
            if(!usingNormTexCoords) {
                ofEnableNormalizedTexCoords();
            }
            
            verticalMirrorImage.getTextureReference().bind();
            
            ofMesh mesh;
            mesh.clear();
            mesh.addVertex(ofVec3f(0, 0));
            mesh.addVertex(ofVec3f(0, ofGetHeight()));
            mesh.addVertex(ofVec3f(ofGetWidth()/2, 0));
            mesh.addVertex(ofVec3f(ofGetWidth()/2, ofGetHeight()));
            mesh.addVertex(ofVec3f(ofGetWidth(), 0));
            mesh.addVertex(ofVec3f(ofGetWidth(), ofGetHeight()));
            
            
            mesh.addTexCoord(ofVec2f(0.25, 0.0));
            mesh.addTexCoord(ofVec2f(0.25, 1.0));
            mesh.addTexCoord(ofVec2f(0.75, 0.0));
            mesh.addTexCoord(ofVec2f(0.75, 1.0));
            mesh.addTexCoord(ofVec2f(0.25, 0.0));
            mesh.addTexCoord(ofVec2f(0.25, 1.0));
            
            mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
            ofSetColor(ofColor::white);
            mesh.draw();
            
            verticalMirrorImage.getTextureReference().unbind();
            
            // pop normalized tex coords
            if(!usingNormTexCoords) {
                ofDisableNormalizedTexCoords();
            }
            
            //white fur
            ofEnableAlphaBlending();
            flowSolver.drawGrey(ofGetWidth(),ofGetHeight(), 10, 3);
            ofDisableAlphaBlending();
        }
            break;
        case HORIZONTALMIRROR:
        {
            bool usingNormTexCoords = ofGetUsingNormalizedTexCoords();
            
            if(!usingNormTexCoords) {
                ofEnableNormalizedTexCoords();
            }
            
            horizontalMirrorImage.getTextureReference().bind();
            
            ofMesh mesh;
            mesh.clear();
            mesh.addVertex(ofVec3f(ofGetWidth(), 0));
            mesh.addVertex(ofVec3f(0, 0));
            mesh.addVertex(ofVec3f(ofGetWidth(), ofGetHeight()/2));
            mesh.addVertex(ofVec3f(0, ofGetHeight()/2));
            mesh.addVertex(ofVec3f(ofGetWidth(), ofGetHeight()));
            mesh.addVertex(ofVec3f(0,ofGetHeight()));
            
            mesh.addTexCoord(ofVec2f(1.0, 0.25));
            mesh.addTexCoord(ofVec2f(0.0, 0.25));
            mesh.addTexCoord(ofVec2f(1.0, 0.75));
            mesh.addTexCoord(ofVec2f(0.0, 0.75));
            mesh.addTexCoord(ofVec2f(1.0, 0.25));
            mesh.addTexCoord(ofVec2f(0.0, 0.25));
            
            mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
            ofSetColor(ofColor::white);
            mesh.draw();
            
            horizontalMirrorImage.getTextureReference().unbind();
            
            // pop normalized tex coords
            if(!usingNormTexCoords) {
                ofDisableNormalizedTexCoords();
            }
            
            //white fur
            ofEnableAlphaBlending();
            flowSolver.drawGrey(ofGetWidth(),ofGetHeight(), 10, 3);
            ofDisableAlphaBlending();
        }
            break;
        case KALEIDOSCOPE:
        {
            bool usingNormTexCoords = ofGetUsingNormalizedTexCoords();
            
            if(!usingNormTexCoords) {
                ofEnableNormalizedTexCoords();
            }
            
            kaleidoscopeMirrorImage.getTextureReference().bind();
            
            int star = ((int)timeline.getValue("star")*2);//8; //get star from the timeline gui, but multiply by 2 to get to always even
            float offset = timeline.getValue("offset");//0.5f; // get offset from the timeline gui
            float angle = 360.f/star; //8 sides to start
            
            
            
			ofMesh mesh;
            
			ofVec3f vec(0,0,0);
			mesh.addVertex(vec);
			vec.x += ofGetHeight()/2;
            
			for(int i = 0; i < star; i++) {
				mesh.addVertex(vec);
				vec.rotate(angle, ofVec3f(0,0,1));
			}
            
			// close the loop
			mesh.addVertex(vec);
            
            
            
			// now work out the texcoords
			/*
			 __________________
			 |   \        /   |
			 |    \      /    |
			 |     \    /     |
			 |      \  /      |
			 |       \/       |
			 +----------------+
             
			 A v shape out of the centre of the camera texture
			 */
            
            
            
			float realOffset = 0.5;
			// normalized distance from the centre (half the width of the above 'V')
			float dist = ABS((float)kaleidoscopeMirrorImage.getHeight()*tan(ofDegToRad(angle)*0.5))/(float)kaleidoscopeMirrorImage.getHeight();
            
            
			// the realOffset is where the (normalized) middle of the 'V' is on the x-axis
			realOffset = ofMap(offset, 0, 1, dist, 1-dist);
            
            
			// this is the point at the bottom of the triangle - our centre for the triangle fan
			mesh.addTexCoord(ofVec2f(realOffset, 1));
            
            
			ofVec2f ta(realOffset-dist, 0);
			ofVec2f tb(realOffset+dist, 0);
			for(int i = 0; i <= star; i++) {
				if(i%2==0) {
					mesh.addTexCoord(ta);
				} else {
					mesh.addTexCoord(tb);
				}
			}
            
            
			glPushMatrix();
			glTranslatef(ofGetWidth()/2, ofGetHeight()/2, 0);
			mesh.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
			mesh.draw();
			glPopMatrix();
            
            kaleidoscopeMirrorImage.getTextureReference().unbind();
            
            // pop normalized tex coords
            if(!usingNormTexCoords) {
                ofDisableNormalizedTexCoords();
            }
            
            //white fur
            ofEnableAlphaBlending();
            flowSolver.drawGrey(ofGetWidth(),ofGetHeight(), 10, 3);
            ofDisableAlphaBlending();
        }
            break;
        case PAINT:
        {
            //do some paint - used the slit scan to hold it....
            slitScan.getOutputImage().draw(0, 0, ofGetWidth(), ofGetHeight());
            
            //paintCanvasAsOfImage.draw(0, 0, ofGetWidth(), ofGetHeight());
        }
            break;
        case COLOURFUR:
        {
            ofSetColor(ofColor::white);
            timeline.getVideoPlayer("video")->draw(0, 0, ofGetWidth(),ofGetHeight());
            ofEnableAlphaBlending();
            flowSolver.drawColored(ofGetWidth(),ofGetHeight(), 10, 3);
            ofDisableAlphaBlending();
        }
            break;
        case DEPTH:
        {
            depthProcessed.draw(0,0,ofGetWidth(), ofGetHeight());
        }
            break;
        case SHATTER:
        {
            //do some shattering - used the slit scan to hold it....
            slitScan.getOutputImage().draw(0, 0, ofGetWidth(), ofGetHeight());
            //ofSetColor(255,255,255);
            //theShatter.theFBO.draw(0, 0, ofGetWidth(), ofGetHeight());
        }
            break;
        case SELFSLITSCAN:
        {
            //do some SELFSLITSCAN - used the slit scan to hold it....
            ofSetColor(255,255,255);
            slitScan.getOutputImage().draw(0, 0, ofGetWidth(), ofGetHeight());
        }
            break;
        case SPIKYBLOBSLITSCAN:
        {
            //do some SPIKYBLOBSLITSCAN - used the slit scan to hold it....
            ofSetColor(255,255,255);
            slitScan.getOutputImage().draw(0, 0, ofGetWidth(), ofGetHeight());
            //theSpikey.theFBO.draw(0,0,ofGetWidth(), ofGetHeight());
        }
            break;
        case MIRRORKALEIDOSCOPE:
        {
            bool usingNormTexCoords = ofGetUsingNormalizedTexCoords();
            
            if(!usingNormTexCoords) {
                ofEnableNormalizedTexCoords();
            }
            
            verticalMirrorImage.getTextureReference().bind();
            
            ofMesh mirrorMesh;
            mirrorMesh.clear();
            mirrorMesh.addVertex(ofVec3f(0, 0));
            mirrorMesh.addVertex(ofVec3f(0, ofGetHeight()));
            mirrorMesh.addVertex(ofVec3f(ofGetWidth()/2, 0));
            mirrorMesh.addVertex(ofVec3f(ofGetWidth()/2, ofGetHeight()));
            mirrorMesh.addVertex(ofVec3f(ofGetWidth(), 0));
            mirrorMesh.addVertex(ofVec3f(ofGetWidth(), ofGetHeight()));
            
            
            mirrorMesh.addTexCoord(ofVec2f(0.25, 0.0));
            mirrorMesh.addTexCoord(ofVec2f(0.25, 1.0));
            mirrorMesh.addTexCoord(ofVec2f(0.75, 0.0));
            mirrorMesh.addTexCoord(ofVec2f(0.75, 1.0));
            mirrorMesh.addTexCoord(ofVec2f(0.25, 0.0));
            mirrorMesh.addTexCoord(ofVec2f(0.25, 1.0));
            
            mirrorMesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
            ofSetColor(ofColor::white);
            mirrorMesh.draw();
            
            verticalMirrorImage.getTextureReference().unbind();
            
            kaleidoscopeMirrorImage.getTextureReference().bind();
            
            int star = ((int)timeline.getValue("star")*2);//8; //get star from the timeline gui, but multiply by 2 to get to always even
            float offset = timeline.getValue("offset");//0.5f; // get offset from the timeline gui
            float angle = 360.f/star; //8 sides to start
            
			ofMesh mesh;
            
			ofVec3f vec(0,0,0);
			mesh.addVertex(vec);
			vec.x += ofGetHeight()/2;
            
			for(int i = 0; i < star; i++) {
				mesh.addVertex(vec);
				vec.rotate(angle, ofVec3f(0,0,1));
			}
            
			// close the loop
			mesh.addVertex(vec);
            
            
            
			// now work out the texcoords
			/*
			 __________________
			 |   \        /   |
			 |    \      /    |
			 |     \    /     |
			 |      \  /      |
			 |       \/       |
			 +----------------+
             
			 A v shape out of the centre of the camera texture
			 */
            
            
            
			float realOffset = 0.5;
			// normalized distance from the centre (half the width of the above 'V')
			float dist = ABS((float)kaleidoscopeMirrorImage.getHeight()*tan(ofDegToRad(angle)*0.5))/(float)kaleidoscopeMirrorImage.getHeight();
            
            
			// the realOffset is where the (normalized) middle of the 'V' is on the x-axis
			realOffset = ofMap(offset, 0, 1, dist, 1-dist);
            
            
			// this is the point at the bottom of the triangle - our centre for the triangle fan
			mesh.addTexCoord(ofVec2f(realOffset, 1));
            
            
			ofVec2f ta(realOffset-dist, 0);
			ofVec2f tb(realOffset+dist, 0);
			for(int i = 0; i <= star; i++) {
				if(i%2==0) {
					mesh.addTexCoord(ta);
				} else {
					mesh.addTexCoord(tb);
				}
			}
            
            
			glPushMatrix();
			glTranslatef(ofGetWidth()/2, ofGetHeight()/2, 0);
			mesh.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
			mesh.draw();
			glPopMatrix();
            
            kaleidoscopeMirrorImage.getTextureReference().unbind();
            
            // pop normalized tex coords
            if(!usingNormTexCoords) {
                ofDisableNormalizedTexCoords();
            }
            
            //white fur
            ofEnableAlphaBlending();
            flowSolver.drawGrey(ofGetWidth(),ofGetHeight(), 10, 3);
            ofDisableAlphaBlending();
        }
            break;
        case PARTICLES:
        {
            //do some PARTICLES - used the slit scan to hold it....
            ofSetColor(255,255,255);
            slitScan.getOutputImage().draw(0, 0, ofGetWidth(), ofGetHeight());
            //theParticles.theFBO.draw(0,0,ofGetWidth(), ofGetHeight());
        }
            break;
        case WHITEFUR:
        {
            ofSetColor(ofColor::white);
            timeline.getVideoPlayer("video")->draw(0, 0, ofGetWidth(),ofGetHeight());
            ofEnableAlphaBlending();
            flowSolver.drawGrey(ofGetWidth(),ofGetHeight(), 10, 3);
            ofDisableAlphaBlending();
        }
            break;
    }
    
    if( bShowNonTimelineGUI ){
        nonTimelineGUI.draw();
    }
    
    
	if( timeline.getIsShowing() ){
        ofSetColor(255, 255, 255);
        
        //timeline
        timeline.draw();
        
        string modeString;
        modeString = "Mode is ";
        
        switch (currentMode) {
            case BLANK: //blank mode
                modeString += "BLANK";
                break;
            case GUI: //GUI drawing mode
                modeString += "GUI";
                break;
            case VIDEO: //the film
                modeString += "VIDEO";
                break;
            case VIDEOCIRCLES: //the film as circles
                modeString += "VIDEOCIRCLES";
                break;
            case KINECTPOINTCLOUD: //draw the kinect camera depth cloud
                modeString += "POINTCLOUD";
                break;
            case SLITSCANBASIC: //slit scan the movie on the grey from the kinect depth grey
                modeString += "SLITSCAN BASIC";
                break;
            case SLITSCANKINECTDEPTHGREY: //slit scan the movie on the grey from the kinect depth grey
                modeString += "SLITSCAN KINECT DEPTH GREY";
                break;
            case SPARKLE: //sparkles on the slitscan
                modeString += "SLITSCAN SPARKLES";
                break;
            case VERTICALMIRROR: //vertical mirror
                modeString += "VERTICAL MIRROR";
                break;
            case PAINT: //body painting diff
                modeString += "PAINT";
                break;
            case HORIZONTALMIRROR: //HORIZONTALMIRROR mirror
                modeString += "HORIZONTALMIRROR";
                break;
            case KALEIDOSCOPE: //KALEIDOSCOPE mirror
                modeString += "KALEIDOSCOPE";
                break;
            case COLOURFUR: //fur!
                modeString +="COLOURFUR";
                break;
            case DEPTH: // depth image
                modeString +="DEPTH";
                break;
            case SHATTER: //shattering blobs
                modeString +="SHATTER";
                break;
            case SELFSLITSCAN: //feeding video frame back into itself as the slitscan
                modeString += "SELFSLITSCAN";
                break;
            case SPIKYBLOBSLITSCAN: //making spikey blobs and putting them in the slitscan
                modeString += "SPIKYBLOBSLITSCAN";
                break;
            case MIRRORKALEIDOSCOPE: //making a kaleidescope with a mirrored background
                modeString += "MIRRORKALEIDOSCOPE";
                break;
            case PARTICLES: //making a partickles that seeks to centroids of the blobs
                modeString += "PARTICLES";
                break;
            case WHITEFUR: //making a white fur coat for people..
                modeString += "WHITEFUR";
                break;
        }
        
        ofSetColor(ofColor::red);
        ofDrawBitmapString(modeString,20,100);
	}
}

void ofApp::drawPointCloud(){
	int w = 640;
	int h = 480;
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_POINTS);
	int step = 2;
	for(int y = 0; y < h; y += step) {
		for(int x = 0; x < w; x += step) {
			if(kinect.getDistanceAt(x, y) > 0) {
				mesh.addColor(kinect.getColorAt(x,y));
				mesh.addVertex(kinect.getWorldCoordinateAt(x, y));
			}
		}
	}
	glPointSize(3);
	ofPushMatrix();
	// the projected points are 'upside down' and 'backwards'
	ofScale(1, -1, -1);
	ofTranslate(0, 0, -1000); // center the points a bit
	glEnable(GL_DEPTH_TEST);
	mesh.drawVertices();
	glDisable(GL_DEPTH_TEST);
	ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::keyPressed (int key) {
    
    switch (key) {
        case 'b':
            break;
        case '0':
            currentMode = BLANK;
            break;
        case '1':
            currentMode = GUI;
            break;
        case '2':
            currentMode = VIDEO;
            break;
        case '3':
            currentMode = VIDEOCIRCLES;
            break;
        case '4':
            currentMode = KINECTPOINTCLOUD;
            break;
        case '5':
            currentMode = VERTICALMIRROR;
            break;
        case '6':
            currentMode = HORIZONTALMIRROR;
            break;
        case '7':
            currentMode = SPARKLE;
            break;
        case '8':
            currentMode = SLITSCANBASIC;
            break;
        case '9':
            currentMode = SLITSCANKINECTDEPTHGREY;
            break;
        case '-':
            currentMode = KALEIDOSCOPE;
            break;
        case '=':
            currentMode = PAINT;
            break;
        case 'q':
            currentMode = COLOURFUR;
            break;
        case 'w':
            currentMode = DEPTH;
            break;
        case 'e':
            currentMode = SHATTER;
            break;
        case 'r':
            currentMode = SELFSLITSCAN;
            break;
        case 'y':
            currentMode = SPIKYBLOBSLITSCAN;
            break;
        case 'u':
            currentMode = MIRRORKALEIDOSCOPE;
            break;
        case 'i':
            currentMode = PARTICLES;
            break;
        case 'o':
            currentMode = WHITEFUR;
            break;
        case 'p':
            bShowNonTimelineGUI = !bShowNonTimelineGUI;
            break;
        case 'f':
            ofToggleFullscreen();
            break;
        case 'a': //used to be key left, but it interferes with ofxtimeline
        {
            currentMode = (GreenpeaceModes)((int)currentMode - 1);
            if(currentMode < 0){
                currentMode = GreenpeaceModes_MAX;//see .h file for stackoverflow justification
            }
            break;
        }
        case 's': //used to be key right, but it interferes with ofxtimeline
        {
            currentMode = (GreenpeaceModes)((int)currentMode + 1);
            if(currentMode > GreenpeaceModes_MAX){
                currentMode = (GreenpeaceModes)0;//see .h file for stackoverflow justification
            }
        }
            break;
            //		case OF_KEY_UP:
            //			angle++;
            //			if(angle>30) angle=30;
            //			kinect.setCameraTiltAngle(angle);
            //			break;
            //		case OF_KEY_DOWN:
            //			angle--;
            //			if(angle<-30) angle=-30;
            //			kinect.setCameraTiltAngle(angle);
            //			break;
            //		case 'p':
            //			bThreshWithOpenCV = !bThreshWithOpenCV;
            //			break;
            //		case'p':
            //			bDrawPointCloud = !bDrawPointCloud;
            //			break;
		case '!':
			kinect.setLed(ofxKinect::LED_GREEN);
			break;
		case '@':
			kinect.setLed(ofxKinect::LED_YELLOW);
			break;
		case '$':
			kinect.setLed(ofxKinect::LED_RED);
			break;
		case '%':
			kinect.setLed(ofxKinect::LED_BLINK_GREEN);
			break;
		case '^':
			kinect.setLed(ofxKinect::LED_BLINK_YELLOW_RED);
			break;
		case '&':
			kinect.setLed(ofxKinect::LED_OFF);
			break;
        case 't': //t for timeline
            if(showTimeline){
                showTimeline = false;
                timeline.hide();
            }else{
                showTimeline = true;
                timeline.show();
            }
            break;
        case 'm': //m for mouse
            if(showMouse){
                showMouse = false;
                ofHideCursor();
            }else{
                showMouse = true;
                ofShowCursor();
            }
            break;
        default:
            //nothing to see here, move along
            break;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
}

//--------------------------------------------------------------
void ofApp::bangFired(ofxTLBangEventArgs& args){
	//cout << "bang fired!" << args.flag << endl;
    
    if(args.flag == "VIDEO"){
        currentMode = VIDEO;
    }
    
    if(args.flag == "SLITSCANBASIC"){
        currentMode = SLITSCANBASIC;
    }
    
    if (args.flag == "SPARKLE") {
        currentMode = SPARKLE;
    }
    
    if (args.flag == "VERTICALMIRROR"){
        currentMode = VERTICALMIRROR;
    }
    
    if (args.flag == "HORIZONTALMIRROR"){
        currentMode = HORIZONTALMIRROR;
    }
    
    if (args.flag == "PAINT"){
        currentMode = PAINT;
    }
    
    if (args.flag == "KALEIDOSCOPE"){
        currentMode = KALEIDOSCOPE;
    }
    
    if (args.flag == "COLOURFUR"){
        currentMode = COLOURFUR;
    }
    
    if(args.flag == "DEPTH"){
        currentMode = DEPTH;
    }
    
    if(args.flag == "SHATTER"){
        currentMode = SHATTER;
    }
    
    if(args.flag == "SELFSLITSCAN"){
        currentMode = SELFSLITSCAN;
    }
    
    if(args.flag == "SPIKYBLOBSLITSCAN"){
        currentMode = SPIKYBLOBSLITSCAN;
    }
    
    if(args.flag == "MIRRORKALEIDOSCOPE"){
        currentMode = MIRRORKALEIDOSCOPE;
    }
    
    if(args.flag == "PARTICLES"){
        currentMode = PARTICLES;
    }
    
    if(args.flag == "WHITEFUR"){
        currentMode = WHITEFUR;
    }
}
