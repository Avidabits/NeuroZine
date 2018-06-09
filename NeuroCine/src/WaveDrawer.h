#pragma once
#include "Channel.h"

// WaveDrawer defines the common interface to be used for 
// all the new WaveDrawer

class WaveDrawer
{
public:
	virtual void setup()=0;
	virtual void update(ChannelsReceiver &channels)=0;
	virtual void draw()=0;
	void conmuteActivation() { bActive = !bActive; };
	bool bActive = true;
};
