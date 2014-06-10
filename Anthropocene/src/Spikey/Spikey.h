//
//  Spikey.h
//  HAndLGreenpeace
//
//  Created by Joel Gethin Lewis on 24/06/2013.
//
//

#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"

class Spikey{
    
public:
    
	void setup(float aWidth, float aHeight);
	void update(ofxCvContourFinder* aContourFinder);
    void drawPolyline(const ofPolyline& polyline);
	void draw(ofColor theColour);
    
    ofFbo theFBO;
    
    vector<ofPolyline> /*smoothed, resampled,*/ simplified;
};
