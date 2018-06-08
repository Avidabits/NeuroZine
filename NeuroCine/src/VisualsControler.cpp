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
	
	// I pick black background since visuals are intended for beamer projection
	// on performance space Neuro Cine
	backgroundColor = RGB(0, 0, 0);
	ofBackground(backgroundColor);

	ofEnableSmoothing();
	ofEnableAlphaBlending();
	ofSetFrameRate(30); // if vertical sync is off, we can go a bit fast... this caps the framerate at 60fps.
	ofSetCircleResolution(50);

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
	fadeBackground();
}

// Once visuals are drawn, we can perform frame based transformations
// such as shaders, filters, Kaleidoscopes, rotations, etc, etc

void VisualsControler::drawLastTransformations()
{
	// Here Implement post frame transformations

	// And finally, here Pushing matrix if needed
}
void VisualsControler::fadeBackground()
{
	ofEnableAlphaBlending();    // turn on alpha blending
	ofSetColor(backgroundColor, 127); //color black for beamer and white for screen
	ofFill();
	ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
	ofDisableAlphaBlending();   // turn it back off, if you don't need it



}
