#include "BigCircle.h"

void BigCircle::update(ChannelsReceiver &channels)
{
	if (!bActive) return;
	float nextRadiusRelax = ofMap(channels.alpha.getOSCSample(), channels.alpha.getMin(), channels.alpha.getMax(), 10, ofGetWidth()/2);
	radiusRelax = ofLerp(radiusRelax, nextRadiusRelax, 0.1);
	// instead of assigning atraccion Center inmediatly, we can aproximate to it
	// using lerp, or pow,  but with a speed dependant on relaxing level

	float increment = ofMap(radiusRelax, 1, ofGetWidth()/2, FLT_EPSILON, 0.1);
	attractionCenter.x = ofLerp(attractionCenter.x, channels.attractionCenter.x, increment);
	attractionCenter.y = ofLerp(attractionCenter.y, channels.attractionCenter.y, increment);
	ofLog() << "updating circle " << attractionCenter<<" radio "<<radiusRelax;
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
	// una bola azul palpita segun los niveles de relajacion (canales 5 y 6) 
	ofSetColor(96,16,227, 50); //100->opaco
	ofSetLineWidth(0);
	ofFill();
	ofDrawCircle(attractionCenter, radiusRelax);

}