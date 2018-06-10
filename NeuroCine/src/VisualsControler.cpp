#pragma once

#include "VisualsControler.h"


void VisualsControler::conmuteSparkingPoint() { sparksEmitter.conmuteActivation(); }
void VisualsControler::conmuteCrazyLine() { crazyLine.conmuteActivation(); }
void VisualsControler::conmuteBigCircle() { circle.conmuteActivation(); }
void VisualsControler::conmuteCurtain() { curtain.conmuteActivation(); }
void VisualsControler::conmutePulsingCircunference() { circunference.conmuteActivation(); }

VisualsControler::VisualsControler()
{
	waveDrawers.push_back(&sparksEmitter);
	waveDrawers.push_back(&circle);
	waveDrawers.push_back(&crazyLine);
	waveDrawers.push_back(&curtain);
	waveDrawers.push_back(&circunference);
}
void VisualsControler::setup()
{
	// perform here the setup you would do in ofApp:setup()
	


	ofEnableSmoothing();
	ofSetFrameRate(30); // if vertical sync is off, we can go a bit fast... this caps the framerate at 60fps.
	ofSetCircleResolution(100);

	characterScenario(); //the initial scenario

	for (auto & wd : waveDrawers) {
		wd->setup();
	}
}

void VisualsControler::onChangeWindowSize()
{
	for (auto & wd : waveDrawers) {
		wd->setup();
	}
}

void VisualsControler::update(ChannelsReceiver &channels)
{
	for (auto & wd : waveDrawers) {
		wd->update(channels);
	}
}

void VisualsControler::draw()
{
	for (auto & wd : waveDrawers) {
		wd->draw();
	}
}


// Before drawing visuals, we need to perform all transformations
// on the backgroung from clearing to fading, rotation, etc, etc.

void VisualsControler::drawFirstTransformations()
{
	// Here transformations on background before Visuals
	// and Pop Matrix if needed, rotation, translation etc.
	if (bFadingBackground) fadeBackground();
}

// Once visuals are drawn, we can perform frame based transformations
// such as shaders, filters, Kaleidoscopes, rotations, etc, etc

void VisualsControler::drawLastTransformations()
{
	// Here Implement post frame transformations
	// for example:
	//     filter
	//     kaleidoscopic view
	//     rotations, translations
	//     shaders
	//     camera movements...

	// And finally, here Pushing matrix if needed
}
void VisualsControler::fadeBackground()
{
	ofSetColor(backgroundColor, 100.0); //color black for beamer and white for screen
	ofFill();
	ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());

}

void VisualsControler::characterScenario()
{
	bFadingBackground = false;
	ofBackground(backgroundColor);
	ofSetBackgroundAuto(true); 

	sparksEmitter.bActive=false;

	circle.bActive=circunference.bActive=crazyLine.bActive=curtain.bActive=true;

}

void VisualsControler::sparksScenario()
{
	bFadingBackground = true;
	ofBackground(backgroundColor);
	ofSetBackgroundAuto(false);
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);

	sparksEmitter.bActive = true;

	circle.bActive = circunference.bActive = crazyLine.bActive = curtain.bActive = false;

}