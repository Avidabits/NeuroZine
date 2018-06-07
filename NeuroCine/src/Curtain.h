#pragma once
#include "WaveDrawer.h"
class Curtain : public WaveDrawer
{
public:
	void setup() {};
	void update(ChannelsReceiver &channels);
	void draw();
private:
	float drowsiness; // sum of delta and theta values
	float maxDrowsiness; // sum of historical maximun
	vector<ofPoint> crazyPoints;
	float strokeWidth = 1;
	ofColor strokeColor = ofColor(244, 173, 252);
	//parameters for a curtain
	float strands=300; //number of strans/threads: a map of drowsiness inside a range
	float dStrands = 1; // distance between strands: an inverse map of drowsiness in a range 

};
