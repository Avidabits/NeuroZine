#include "CrazyLine.h"

CrazyLine::CrazyLine(void)
{
	// default constructor allocate the crazyPoints
	for (int i = 0; i < 15; i++) crazyPoints.push_back({ 0.0f ,0.0f  });

}
void CrazyLine::setup()
{
	// Here we set the number of points ad inicial position 
	float h = ofGetHeight() / 2;
	float dx = ofGetWidth() / 15;
	for (int i = 0; i < crazyPoints.size(); i++) crazyPoints[i]={ i*dx, h };
	// we have created fifthteen points horizontaly and along the width of screen

}


void CrazyLine::update(ChannelsReceiver &channels)
{
	if (!bActive) return;
    // Here we ar going to modify the crazy point positions and the strokeWidth
	// based on the gamma channel values, which are waves generated on multiattention processes
	// vector<ofPoint> crazyPoints;
	// float strokeWidth = 1;

	// TODO: don't use 1 as minimal but zero. 
	// TODO: don't user 12 as maximal but a number related to screen resolution
	strokeWidth = round(ofMap(channels.gamma.getOSCSample(), 0, channels.gamma.getHistoricalMax(), 1, 12));
	
	// TODO: don't use 1 as minimal but zero. 
	// TODO: don't user 50 as maximal but a number related to screen resolution
	float modulus = ceil(ofMap(channels.gamma.getOSCSample(), 0, channels.gamma.getHistoricalMax(), 1, 50));
   
	// adding some noisy variation on crazyPoints related to gamma levels
	ofPoint noise(1, 0, 0); //starting with a normalize vector
	noise *= modulus;

	for (auto &p: crazyPoints) {
		noise.rotate(ofRandom(360), ofVec3f(0, 0, 1)); //rotating in XY plane
		p += noise; // adding the moisy vector to the crazy point
		// constrain to window region
        if (p.x > ofGetWidth() - 2 * strokeWidth) p.x = ofGetWidth() - 2 * strokeWidth;
		if (p.x < 2 * strokeWidth) p.x = 2 * strokeWidth;
		if (p.y>ofGetHeight() - 2 * strokeWidth) p.y = ofGetHeight() - 2 * strokeWidth;
		if (p.y<2 * strokeWidth) p.y = 2 * strokeWidth;
	}

}

void CrazyLine::draw()
{ 
	if (!bActive) return;
	ofNoFill();
	ofSetColor(strokeColor);
	ofSetLineWidth(strokeWidth);

	ofBeginShape();
	for (auto & p : crazyPoints) {
		ofCurveVertex(p);
	}
	//picking the middle point as clousure
	ofCurveVertex(crazyPoints[crazyPoints.size()/2]);
	ofNextContour(true); //close countour
	ofEndShape();
}