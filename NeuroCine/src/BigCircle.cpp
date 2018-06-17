#include "BigCircle.h"

void BigCircle::update(ChannelsReceiver &channels)
{
	if (!bActive) return;
	float nextRadiusRelax = mapExp(abs(channels.alpha.getOSCSample()), channels.alpha.getMin(), channels.alpha.getMax(), 10, ofGetHeight()/4, 2);
	radiusRelax = ofLerp(radiusRelax, nextRadiusRelax, 0.5);
	// instead of assigning atraccion Center inmediatly, we can aproximate to it
	// using lerp, or pow,  but with a speed dependant on relaxing level

	float increment = ofMap(radiusRelax, 1, ofGetHeight()/4, FLT_EPSILON, 0.1);
	attractionCenter.x = ofLerp(attractionCenter.x, channels.attractionCenter.x, increment);
	attractionCenter.y = ofLerp(attractionCenter.y, channels.attractionCenter.y, increment);

	// now we need to constrain Circle to window region
	if ((attractionCenter.y - radiusRelax) < 0) attractionCenter.y = radiusRelax;
	//if ((attractionCenter.y + radiusRelax > ofGetHeigt())) attractionCenter.y -= radiusRelax;
	if ((attractionCenter.x - radiusRelax) < 0) attractionCenter.x = radiusRelax;
	if ((attractionCenter.x+radiusRelax)>ofGetWidth()) attractionCenter.x -= radiusRelax;
	// TODO: 
	// In order to decide precisely how to update radio, we must take into account 
	// the time stamps between updates ad the update time windows 
	// framerate is fixed, but udp por events are not. 
	// Several udp messages can arrive on a frame time slot

}

void BigCircle::draw()
{ 
	if (!bActive) return;
	// just a point
	// A big circle changes regarding levels of relax (alpha)
	ofSetColor(96,16,227); 
	ofSetLineWidth(0);
	ofFill();
	ofDrawCircle(attractionCenter, radiusRelax);

}