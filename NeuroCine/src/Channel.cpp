#include "Channel.h"

//Brain channels are populated with data from OpenVibe via OSC


// Channels: individual channel, just stored the last received sample and 
// some calculeted data


void Channel::setOSCSample(float _OSCSample)
{
	uint64_t actualTime= ofGetElapsedTimef();
	if (actualTime - timeStamp > temporalWindow) {
		prevOSCSample = OSCSample;
		timeStamp = actualTime;
	}
	OSCSample = _OSCSample;
	minim = min(minim, OSCSample);
	maxim = max(maxim, OSCSample);
	mean = (mean + abs(OSCSample)) / 2.0;
	historicalMax = max(historicalMax, maxim);
	
}

float Channel::getOSCSample() {	return OSCSample; }
float Channel::getMin() { return minim; }
float Channel::getMax() { return maxim;  }
float Channel::getMean() { return mean; }
float Channel::getHistoricalMax() { return historicalMax; }
void Channel::setHistoricalMax(float _historicalMax) {	historicalMax = _historicalMax;}


void ChannelsReceiver::logStatistics()
{
	cout << "\n";
	string outputString = "channel;min;max;mean;historicalMax";
	outputString += "\ndelta;" + to_string(delta.getMin()) + ";" + to_string(delta.getMax()) + ";" + to_string(delta.getMean()) + ";" + to_string(delta.getHistoricalMax());
	outputString += "\ntheta;" + to_string(theta.getMin()) + ";" + to_string(theta.getMax()) + ";" + to_string(theta.getMean()) + ";" + to_string(theta.getHistoricalMax());
	outputString += "\nalpha;" + to_string(alpha.getMin()) + ";" + to_string(alpha.getMax()) + ";" + to_string(alpha.getMean()) + ";" + to_string(alpha.getHistoricalMax());
	outputString += "\nbeta;" + to_string(beta.getMin()) + ";" + to_string(beta.getMax()) + ";" + to_string(beta.getMean()) + ";" + to_string(beta.getHistoricalMax());
	outputString += "\ngamma;" + to_string(gamma.getMin()) + ";" + to_string(gamma.getMax()) + ";" + to_string(gamma.getMean()) + ";" + to_string(gamma.getHistoricalMax());
	outputString += "\narousal;" + to_string(arousal.getMin()) + ";" + to_string(arousal.getMax()) + ";" + to_string(arousal.getMean()) + ";" + to_string(arousal.getHistoricalMax());
	outputString += "\nvalence;" + to_string(valence.getMin()) + ";" + to_string(valence.getMax()) + ";" + to_string(valence.getMean()) + ";" + to_string(valence.getHistoricalMax());
	outputString += "\n";
	cout << outputString;
	ofFile logFile(ofToDataPath("NeuroLog.txt"), ofFile::WriteOnly);
	logFile.writeFromBuffer(outputString);
	logFile.close();

}


// group of channels
void ChannelsReceiver::setup(int port)
{
	// initializes the OSC receiver on port
	receiver.setup(port);
	cout << "listening for OpenVibe OSC messages on port " << port << "\n";
	// setting a starting point pot atractionCenter. 
	attractionCenter.x = ofGetWidth() / 2;
	attractionCenter.y = ofGetHeight() / 2; 
	// TODO: set historical max from a file
	delta.setHistoricalMax(227.817093);
	theta.setHistoricalMax(74.674004);
	alpha.setHistoricalMax(11.707602);
	beta.setHistoricalMax(9.385706);
	gamma.setHistoricalMax(28.184204);
	arousal.setHistoricalMax(975.281494);
	valence.setHistoricalMax(411.030396);
}


void ChannelsReceiver::update()
{
	// receive all data channel from OpenVibe OSC and update channel data

		// check for waiting messages
		while (receiver.hasWaitingMessages()) {
			// get the next message
			ofxOscMessage m;
			receiver.getNextMessage(m);
			// check egg's messages. 
		 	if (m.getAddress() == "/eeg/delta/mean") {
				delta.setOSCSample(m.getArgAsFloat(0)); 
			} else if (m.getAddress() == "/eeg/theta/mean") {
				theta.setOSCSample(m.getArgAsFloat(0));
				attractionCenter.x = ceil(ofMap(valence.getOSCSample(), valence.getMin(), valence.getMax(), 0, ofGetWidth()));
			} else if (m.getAddress() == "/eeg/alpha/mean") {
				alpha.setOSCSample(m.getArgAsFloat(0));
				attractionCenter.y = ceil(ofMap(arousal.getOSCSample(), arousal.getMin(), arousal.getMax(), 0, ofGetHeight()));
			}else if (m.getAddress() == "/eeg/beta/mean") {
				beta.setOSCSample(m.getArgAsFloat(0));
				updateAttractionCenter(
					ceil(ofMap(valence.getOSCSample(), valence.getMin(), valence.getMax(), 0, ofGetWidth()))  , 
					ceil(ofMap(arousal.getOSCSample(), arousal.getMin(), arousal.getMax(), 0, ofGetHeight())) );
			} else if (m.getAddress() == "/eeg/gamma/mean") {
				gamma.setOSCSample(m.getArgAsFloat(0));
			} else if (m.getAddress() == "/eeg/arousal/mean") {
				arousal.setOSCSample(m.getArgAsFloat(0));
			} else if (m.getAddress() == "/eeg/valence/mean") {
				valence.setOSCSample(m.getArgAsFloat(0));
			} else {
				// unrecognized message
				ofLog() << "Unknown message ";
			} //format unknown message
			formatMessage(m);
			ofLog() << message << "\n";
		}//while has messages


		// now is time to compute the calculated variables
		// but we calculate them here for the moment
		
				
}//ChannelsReceiver::update

void ChannelsReceiver::updateAttractionCenter(float x, float y)
{
	attractionCenter.x = x;
	attractionCenter.y = y;
	// common atraccion center updates inmediatly.
	// the visuals atraccion centers will change smothly to go here
}






ChannelsReceiver::~ChannelsReceiver()
{
	logStatistics();
}

void ChannelsReceiver::formatMessage(ofxOscMessage &m)
{
	message = m.getAddress();
	message += ": ";
	for (int i = 0; i < m.getNumArgs(); i++) {
		// get the argument type
		message += m.getArgTypeName(i);
		message += ":";
		// display the argument - make sure we get the right type
		if (m.getArgType(i) == OFXOSC_TYPE_INT32) {
			message += ofToString(m.getArgAsInt32(i));
		}
		else if (m.getArgType(i) == OFXOSC_TYPE_FLOAT) {
			message += ofToString(m.getArgAsFloat(i));
		}
		else if (m.getArgType(i) == OFXOSC_TYPE_STRING) {
			message += m.getArgAsString(i);
		}
		else {
			message += "unknown";
		}
	}// for getNumArgs	
}



///////////////////HELPER
float  mapExp(float value, float inputMin, float inputMax, float outputMin, float outputMax, float exponent)
{
	// First, we map to value normalized between 0 and 1
	// the we calculate the power(exp), being sure that the result will be constrained between 0 and 1
	float middleValue = pow(ofMap(value, inputMin, inputMax, 0, 1), exponent);
	// finally we map the middleValue to the objetive range
	return ceil(ofMap(middleValue, 0, 1, outputMin, outputMax));

}

