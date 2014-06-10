//
//  Spikey.cpp
//  HAndLGreenpeace
//
//  Created by Joel Gethin Lewis on 24/06/2013
//
//

#include "Spikey.h"

void Spikey::setup(float aWidth, float aHeight) {
    
    theFBO.allocate(aWidth, aHeight, GL_RGB);
}

//--------------------------------------------------------------
void Spikey::update(ofxCvContourFinder* aContourFinder){
    float FBOWidth = theFBO.getWidth();
    float FBOHeight = theFBO.getHeight();

    float contourWidth = aContourFinder->getWidth();
    float contourHeight = aContourFinder->getHeight();

    float widthRatio = FBOWidth/contourWidth;
    float heightRatio = FBOHeight/contourHeight;
    
//    smoothed.clear();
//    resampled.clear();
    simplified.clear();
    
    for(unsigned int i = 0; i < aContourFinder->blobs.size(); i++) {
        ofPolyline cur;
        // add all the current vertices to cur polyline
        //cur.addVertices(aContourFinder->blobs[i].pts);
        
        for(int j=0; j< aContourFinder->blobs[i].nPts; j++){
            //add them scaling to the size of the FBO
            cur.addVertex(aContourFinder->blobs[i].pts[j].x * widthRatio, aContourFinder->blobs[i].pts[j].y * heightRatio);
        }
        
        cur.setClosed(true);
        
        // add the cur polyline to all these vector<ofPolyline>

//        smoothed.push_back(cur.getSmoothed(8));
//        resampled.push_back(cur.getResampledByCount(100).getSmoothed(8));
        simplified.push_back(cur.getResampledByCount(10));
    }
}

void Spikey::drawPolyline(const ofPolyline& polyline){
    ofBeginShape();
	for( int i=0; i<polyline.size(); i++ ){
		ofVertex( polyline[ i ].x, polyline[ i ].y ); //to allow the fill
	}
	ofEndShape();
}

//--------------------------------------------------------------
void Spikey::draw(ofColor theColour) {
    theFBO.begin();
	ofSetColor(ofColor::black);
    ofFill();
    ofRect(0,0,theFBO.getWidth(), theFBO.getHeight());
    
    ofSetColor(ofColor::white);
    
    for(unsigned int i = 0; i < simplified.size(); i++) {
        
//		ofSetColor(ofColor::red);
//		drawPolyline(smoothed[i]);
//		
//		ofSetColor(ofColor::green);
//        drawPolyline(resampled[i]);
//		
//		ofSetColor(ofColor::blue);
//        drawPolyline(simplified[i]);
        drawPolyline(simplified[i]);
	}
    
    theFBO.end();
}

