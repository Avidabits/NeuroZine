#pragma once
#include "WaveDrawer.h"

// CrazyLine is related to the levels of gamma waves
// whitch comes with hight levels of cognitive activity

class CrazyLine : public WaveDrawer
{
public:
	CrazyLine(); //default constructor, to allocate the crazy points
	void setup();
	void update(ChannelsReceiver &channels);
	void draw();
private:
	vector<ofPoint> crazyPoints;
	float strokeWidth=1;
	ofColor strokeColor=ofColor(227, 157, 16);

};
