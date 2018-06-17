#pragma once
#include "WaveDrawer.h"

class Semantic : public WaveDrawer
{
	// this visual is a white cincunference that will be asocieted 
	// with alerts level which are represente by ALPHA CHANNEL	
	void setup();
	void update(ChannelsReceiver &channels);
	void draw();
private:
	ofPoint attractionCenter; //represent a position un arousal / valence space
};
