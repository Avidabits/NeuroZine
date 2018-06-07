#pragma once
// programming instructions for adding a new visual
// 1. Create a new visual inheriting from Wavedrawer class
// 2. Add in this file the include file of the new class
// 3. Create a specific function in VisualsControler for commuting activation (not mandatory)
// 4. Include in OfApp.cpp the calling of commuting activation (not mandatory)
// 5. Create in VisualsControler a private member of the new class
// 6. In VisualsControler constructor add the new member to the waverDrawers vector . 

#include "Channel.h"
#include "WaveDrawer.h"
#include "PulsingCircunference.h" // related to alertness, beta channels levels
#include "Sparks.h"
#include "BigCircle.h"
#include "CrazyLine.h"
#include "Curtain.h"


class VisualsControler
{
public:	
	VisualsControler(); // default contructor to populate wavedrawers
	void setup();
	void update(ChannelsReceiver &channels);
	void draw();
	void drawFirstTransformations();
	void drawLastTransformations();
	void onChangeWindowSize();
	void fadeBackground();
	void conmuteSparks();
	void conmuteCrazyLine();
	void conmuteBigCircle();
	void conmuteCurtain();	 
	void conmutePulsingCircunference();
private: 
	ofColor backgroundColor;
	Sparks sparks;
	BigCircle circle;
	PulsingCircunference circunference;
	CrazyLine crazyLine;
	Curtain curtain;
	// just a helper for common calling interface
	vector<WaveDrawer *> waveDrawers;

};
