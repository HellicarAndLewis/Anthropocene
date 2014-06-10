//
//  Shatter.cpp
//  HAndLGreenpeace
//
//  Created by Joel Gethin Lewis on 23/06/2013.
//
//

#include "Shatter.h"

void Shatter::setup(float aWidth, float aHeight) {
    
    theFBO.allocate(aWidth, aHeight, GL_RGB);
    
    // Box2d
	box2d.init();
	box2d.setGravity(0, -20); //rise up!
	box2d.createGround();
	box2d.setFPS(30.0);
}

//--------------------------------------------------------------
void Shatter::update(ofxCvContourFinder* aContourFinder){
    float shatterWidth = theFBO.getWidth();
    float shatterHeight = theFBO.getHeight();

    float contourWidth = aContourFinder->getWidth();
    float contourHeight = aContourFinder->getHeight();

    float widthRatio = shatterWidth/contourWidth;
    float heightRatio = shatterHeight/contourHeight;
    
    // now just stick some particles on the contour and emit them randomly
    for(int i = 0; i < aContourFinder->nBlobs; i++) {
        int step = 33;
        for(int j = 0; j < aContourFinder->blobs[i].pts.size(); j+=step) {
            
            ofVec2f startPoint = ofVec2f(aContourFinder->blobs[i].pts[j].x*widthRatio,aContourFinder->blobs[i].pts[j].y*heightRatio);
            
            float r = ofRandom(4, 12);		// a random radius 4px - 20px
            ofxBox2dCircle circle;
            circle.setPhysics(3.0, 0.53, 0.1);
            circle.setup(box2d.getWorld(), startPoint.x, startPoint.y, r);
            circles.push_back(circle);
        }
    }
    
    box2d.update();
}

//--------------------------------------------------------------
void Shatter::draw(ofColor theColour) {
    theFBO.begin();
	ofSetColor(ofColor::black);
    ofRect(0,0,theFBO.getWidth(), theFBO.getHeight());
    
    ofSetColor(theColour);
    
    for (int i=0; i<circles.size(); i++){
        ofCircle(circles[i].getPosition(), circles[i].getRadius());
    }
    
    theFBO.end();
}

