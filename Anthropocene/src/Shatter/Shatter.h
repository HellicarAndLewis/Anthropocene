//
//  Shatter.h
//  HAndLGreenpeace
//
//  Created by Joel Gethin Lewis on 23/06/2013.
//
//

#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxBox2D.h"

class Shatter{
    
public:
    
	void setup(float aWidth, float aHeight);
	void update(ofxCvContourFinder* aContourFinder);
	void draw(ofColor theColour);
    
    ofFbo theFBO;
	ofxBox2d					box2d;
    vector <ofxBox2dCircle> circles;
};
