#include "PulsingCircunference.h"

void PulsingCircunference::update(ChannelsReceiver &channels)
{
	if (!bActive) return;
	float nextRadiusAlert = ofMap(abs(channels.beta.getOSCSample()), 0, channels.beta.getMax(), 1, ofGetHeight()/4);
	radiusAlert = ofLerp(radiusAlert, nextRadiusAlert, 0.1);

	strokeWidth = ofMap(abs(channels.beta.getOSCSample()), 0, channels.beta.getHistoricalMax(), 0.5, ofGetHeight() / 70);

	// instead of assigning atraccion Center inmediatly, we can aproximate to it
	// using lerp, or pow,  but with a speed dependant on ALERT level
	float increment = ofMap(radiusAlert, 1, ofGetHeight() / 4, FLT_EPSILON, 0.1);
	attractionCenter.x = ofLerp(attractionCenter.x, channels.attractionCenter.x, increment);
	attractionCenter.y = ofLerp(attractionCenter.y, channels.attractionCenter.y, increment);
}

void PulsingCircunference::draw() 
{ 
	if (!bActive) return;
	// just an outline
	ofNoFill();
	ofSetColor(255); //TODO: must be related to colorBackground
	ofSetLineWidth(strokeWidth);
	ofDrawCircle(attractionCenter, radiusAlert);

}