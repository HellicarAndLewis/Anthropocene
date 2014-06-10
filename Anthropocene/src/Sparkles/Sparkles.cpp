//
//  Sparkles.cpp
//  HAndLGreenpeace
//
//  Created by Joel Gethin Lewis on 19/06/2013.
//
//

#include "Sparkles.h"

/**
 * == Simplified BSD *** MODIFIED FOR NON-COMMERCIAL USE ONLY!!! *** ==
 * Copyright (c) 2011, Cariad Interactive LTD
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice, this list of
 *     conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 *   * Any redistribution, use, or modification is done solely for personal benefit and not for any
 *     commercial purpose or for monetary gain
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//
//  Sparkles.cpp
//  SomanticsMac
//
//  Created by Marek Bereza on 11/07/2011.
//  Copyright 2011 MAZBOX LTD. All rights reserved.
//

void Sparkles::setup(float aWidth, float aHeight) {
	
    cloud.setup();
    
    
	threshold = 40;
	amount = 0.3;
    
    theFBO.allocate(aWidth, aHeight, GL_RGB);
}

//--------------------------------------------------------------
void Sparkles::update(ofxCvContourFinder* aContourFinder){
    
    float cloudWidth = theFBO.getWidth();
    float cloudHeight = theFBO.getHeight();
    
    float contourWidth = aContourFinder->getWidth();
    float contourHeight = aContourFinder->getHeight();
    
    float widthRatio = cloudWidth/contourWidth;
    float heightRatio = cloudHeight/contourHeight;
    
    // now just stick some particles on the contour and emit them randomly
    for(int i = 0; i < aContourFinder->nBlobs; i++) {
        int step = 10;
        for(int j = 0; j < aContourFinder->blobs[i].pts.size(); j+=step) {
            cloud.spawn(
                    (aContourFinder->blobs[i].pts[j].x)*widthRatio,
                    (aContourFinder->blobs[i].pts[j].y)*heightRatio,
                    ofRandom(-5, 5), ofRandom(-5, 5));
        }
    }

    cloud.update();
    
}

//--------------------------------------------------------------
void Sparkles::draw(ofColor theColour) {
    theFBO.begin();
	ofSetColor(ofColor::black);
    ofRect(0,0,theFBO.getWidth(), theFBO.getHeight());
    ofSetColor(ofColor::white);
    ofEnableAlphaBlending();
    cloud.draw(theColour);
    ofDisableAlphaBlending();
    
//    ofSetColor(ofColor::white);
//    float circleX = theFBO.getWidth()/2.f;
//    float circleY = theFBO.getHeight()/2.f;
//    float circleRadius = min(circleX, circleY);
//    ofCircle(circleX,circleY, circleRadius);
    
    theFBO.end();
}

