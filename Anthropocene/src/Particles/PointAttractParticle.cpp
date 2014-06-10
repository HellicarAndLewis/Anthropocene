#include "PointAttractParticle.h"

//------------------------------------------------------------------
PointAttractParticle::PointAttractParticle(){
	attractPoints = NULL;
}

//------------------------------------------------------------------
void PointAttractParticle::setAttractPoints( vector <ofPoint> * attract ){
	attractPoints = attract;
}

//------------------------------------------------------------------
void PointAttractParticle::setup(int aWidth, int aHeight){
	//the unique val allows us to set properties slightly differently for each particle
	uniqueVal = ofRandom(-10000, 10000);
    
    maxWidth = aWidth;
    maxHeight = aHeight;
	
	pos.x = ofRandom(maxWidth);
	pos.y = ofRandom(maxHeight);
	
	vel.x = ofRandom(-3.9, 3.9);
	vel.y = ofRandom(-3.9, 3.9);
	
	frc   = ofPoint(0,0,0);
	
	scale = ofRandom(1.0, 2.0);
	
    drag  = ofRandom(0.95, 0.998);	
}

//------------------------------------------------------------------
void PointAttractParticle::update(){
		
    if( attractPoints ){

        //1 - find closest attractPoint 
        ofPoint closestPt;
        int closest = -1; 
        float closestDist = 9999999;
        
        for(unsigned int i = 0; i < attractPoints->size(); i++){
            float lenSq = ( attractPoints->at(i)-pos ).lengthSquared();
            if( lenSq < closestDist ){
                closestDist = lenSq;
                closest = i;
            }
        }
        
        //2 - if we have a closest point - lets calcuate the force towards it
        if( closest != -1 ){
            closestPt = attractPoints->at(closest);				
            float dist = sqrt(closestDist);
            
            //in this case we don't normalize as we want to have the force proportional to distance 
            frc = closestPt - pos;
    
            vel *= drag;
             
            //lets also limit our attraction to a certain distance and don't apply if 'f' key is pressed
            if( dist < 300 && dist > 40 ){
                vel += frc * 0.003;
            }else{
                //if the particles are not close to us, lets add a little bit of random movement using noise. this is where uniqueVal comes in handy. 			
                frc.x = ofSignedNoise(uniqueVal, pos.y * 0.01, ofGetElapsedTimef()*0.2);
                frc.y = ofSignedNoise(uniqueVal, pos.x * 0.01, ofGetElapsedTimef()*0.2);
                vel += frc * 0.4;
            }
            
        }
    
    }
	
	//2 - UPDATE OUR POSITION
	
	pos += vel; 
	
	
	//3 - (optional) LIMIT THE PARTICLES TO STAY ON SCREEN 
	//we could also pass in bounds to check - or alternatively do this at the testApp level
	if( pos.x > maxWidth ){
		pos.x = maxWidth;
		vel.x *= -1.0;
	}else if( pos.x < 0 ){
		pos.x = 0;
		vel.x *= -1.0;
	}
	if( pos.y > maxHeight){
		pos.y = maxHeight;
		vel.y *= -1.0;
	}
	else if( pos.y < 0 ){
		pos.y = 0;
		vel.y *= -1.0;
	}	
		
}

//------------------------------------------------------------------
void PointAttractParticle::draw(){
    ofCircle(pos.x, pos.y, scale * 4.0);
}

