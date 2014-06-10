//
//  Particles.cpp
//  HAndLGreenpeace
//
//  Created by Joel Gethin Lewis on 24/06/2013
//
//

#include "Particles.h"

void Particles::setup(float aWidth, float aHeight) {
    
    theFBO.allocate(aWidth, aHeight, GL_RGB);
    
    int num = 512;
	theParticles.assign(num, PointAttractParticle());
    
    for(int i = 0; i < theParticles.size(); i++){
		theParticles[i].setup(aWidth, aHeight);
	}
}

//--------------------------------------------------------------
void Particles::update(ofxCvContourFinder* aContourFinder){
    float FBOWidth = theFBO.getWidth();
    float FBOHeight = theFBO.getHeight();

    float contourWidth = aContourFinder->getWidth();
    float contourHeight = aContourFinder->getHeight();

    float widthRatio = FBOWidth/contourWidth;
    float heightRatio = FBOHeight/contourHeight;
    
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
        
        simplified.push_back(cur.getResampledByCount(10));
    }
    
    //these are the attraction points used in the forth demo
	attractPoints.clear();
    
    for(unsigned int i = 0; i < aContourFinder->blobs.size(); i++) {
        ofPoint centroid = aContourFinder->blobs[i].centroid;
        ofPoint translatedcentroid = ofPoint(centroid.x * widthRatio, centroid.y *heightRatio);
        attractPoints.push_back(translatedcentroid);
    }
    
    for(int i = 0; i < theParticles.size(); i++){
		theParticles[i].setAttractPoints(&attractPoints);
	}
    
    for(int i = 0; i < theParticles.size(); i++){
		theParticles[i].update();
	}
}

void Particles::drawPolyline(const ofPolyline& polyline){
    ofBeginShape();
	for( int i=0; i<polyline.size(); i++ ){
		ofVertex( polyline[ i ].x, polyline[ i ].y ); //to allow the fill
	}
	ofEndShape();
}

//--------------------------------------------------------------
void Particles::draw(ofColor theColour) {
    theFBO.begin();
	ofSetColor(ofColor::black);
    ofFill();
    ofRect(0,0,theFBO.getWidth(), theFBO.getHeight());
    
    ofSetColor(ofColor::white);
    
    for(unsigned int i = 0; i < simplified.size(); i++) {
        drawPolyline(simplified[i]);
	}
    
    for(int i = 0; i < theParticles.size(); i++){
		theParticles[i].draw();
	}
    
    theFBO.end();
}

