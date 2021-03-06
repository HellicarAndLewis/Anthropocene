//
//  SparkleCloud.cpp
//  HAndLGreenpeace
//
//  Created by Joel Gethin Lewis on 19/06/2013.
//
//



#include "SparkleCloud.h"

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

SparkleCloud::SparkleCloud(){
    
	pos = 0;
    maxAge = 50;
    numParticles = 500;
    positions = new ofVec2f[numParticles];
    velocities = new ofVec2f[numParticles];
    ages = new int[numParticles];
    sizes = new float[numParticles];
    
    for(int i = 0; i < numParticles; i++) {
        positions[i] = ofVec2f(-10, -10);
        velocities[i] = ofVec2f(0, 0);
        ages[i] = maxAge;
		sizes[i] = ofRandom(5, 26);
    }
}

void SparkleCloud::setup() {
	vector<string> imgs;
	imgs.push_back("particles/blob.png");
	imgs.push_back("particles/glitter.png");
	imgs.push_back("particles/sparkle.png");
	imgs.push_back("particles/star.png");
	for(int i = 0; i < imgs.size(); i++) {
		images.push_back(ofImage());
		images.back().loadImage(imgs[i]);
		images.back().setAnchorPercent(0.5, 0.5);
	}
}


SparkleCloud::~SparkleCloud(){
    delete [] positions;
    delete [] velocities;
    delete [] ages;
	delete [] sizes;
}


void SparkleCloud::update(){
    for(int i = 0; i < numParticles; i++) {
        ages[i]++;
        positions[i] += velocities[i];
    }
}

void SparkleCloud::draw(ofColor theColour){
    
	ofColor c;

    //set the colour properly here!
    c = theColour;
    c = ofColor(ofColor::white);
    
    for(int i = 0; i < numParticles; i++) {
        if(ages[i]<maxAge) {
            float size = ofMap(ages[i], 0, maxAge, sizes[i], 0);
    
            ofSetColor(c.r, c.g, c.b, ofMap(ages[i], 0, maxAge, 255, 0));
            
			if(ofRandom(0.f, 1.f)>0.96f) {
				int r = 1+rand()%(images.size()-1);
				images[r].draw(positions[i].x, positions[i].y, size, size);
//                cout << "I'm trying to draw at:" << positions[i].x << "," << positions[i].y << ", at size:" << size << endl;
			} else {
				images[0].draw(positions[i].x, positions[i].y, size, size);
			}
        }
    }
    
//	for(int i = 0; i < images.size(); i++) {
//		images[i].draw(i*100, i*100);
//	}
    
}

void SparkleCloud::spawn(float x, float y, float dx, float dy){
    ages[pos] = 0;
    positions[pos] = ofVec2f(x, y);
    velocities[pos] = ofVec2f(dx, dy);
    
    pos++;
    pos %= numParticles;
}
