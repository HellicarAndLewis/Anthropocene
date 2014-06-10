#pragma once
#include "ofMain.h"

class PointAttractParticle{

	public:
		PointAttractParticle();
		
		void setAttractPoints( vector <ofPoint> * attract );

		void setup(int aWidth, int aHeight);
		void update();
		void draw();		
		
		ofPoint pos;
		ofPoint vel;
		ofPoint frc;
		
		float drag; 
		float uniqueVal;
		float scale;
		
		vector <ofPoint> * attractPoints;
    
    int maxWidth;
    int maxHeight;
};