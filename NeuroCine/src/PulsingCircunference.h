#pragma once
#include "WaveDrawer.h"

class PulsingCircunference : public WaveDrawer
{
    // this visual is a white cincunference that will be asocieted 
	// with alerts level which are represente by BETA CHANNEL
	
	void setup() {};
	void update(ChannelsReceiver &channels);

	void draw();
	
private:
	float radiusAlert = 0;
	ofPoint attractionCenter; //REPRESENT A POSICION IN THE VALENCE/AROUSAL SPACE

};
