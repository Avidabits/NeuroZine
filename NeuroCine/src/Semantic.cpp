#include "Semantic.h"

void Semantic::setup()
{
	bActive = false;
}
void Semantic::update(ChannelsReceiver &channels)
{
	if (!bActive) return;
	attractionCenter.x = ofLerp(attractionCenter.x, channels.attractionCenter.x, 0.1);
	attractionCenter.y = ofLerp(attractionCenter.y, channels.attractionCenter.y, 0.1);
	// TODO: 

}

void Semantic::draw()
{ 
	if (!bActive) return;
	// draw text accordingly to attractionCenter
}