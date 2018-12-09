#pragma once
#include "ofMain.h"
#include "ofxOsc.h"

using namespace std; 

class Channel
{
public:
	void setOSCSample(float _OSCSample);
	float getOSCSample();
	float getMin();
	float getMax();
	float getMean();
	float getHistoricalMax();
	void setHistoricalMax(float _historicalMax);

private:
	// received data
	float prevOSCSample = 0.0;
	float OSCSample=0.0; //it's and instant sample
	uint64_t timeStamp=0.0; // timestamp milliseconds at whitch prevOSCSample has been set
	uint64_t temporalWindow=50; // elapsed this time, prevOSCSample will be update
	// computed data from all samples received
	float minim = 0; 
	float maxim = 0; 
	float mean = 0;  // to be computed after absolute value
	float historicalMax=0;

};// class Channel

class ChannelsReceiver
{

public:
	void setup(int port); // initializes the OSC receiver on port
	void update(); // get new data from OSC receiver
	void logStatistics();
	Channel delta, theta, alpha, beta, gamma;// channels to be received from OpenVibe
	Channel arousal, valence; // channels computed on OpenVibe 
	string message;
// some calculated variables, shared between visuals
	ofPoint attractionCenter; 
	void updateAttractionCenter();
	~ChannelsReceiver(); // log historicals on exit
private:
	ofxOscReceiver receiver;
	void formatMessage(ofxOscMessage &m);


};

//helper funcion por exponential interpolations, that generate more organic results
float  mapExp(float value, float inputMin, float inputMax, float outputMin, float outputMax, float exponent);

// helper funtionfor drwing

