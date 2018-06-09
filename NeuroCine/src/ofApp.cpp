#include "ofApp.h"

//--------------------------------------------------------------

void ofApp::setup()
{
	ofSetLogLevel(OF_LOG_SILENT);
	
	ShowKeyboardInstructions();
	// listen on the given port
	channelsReceiver.setup(9998);

	visualsControler.setup();

}

//--------------------------------------------------------------
void ofApp::update(){

	channelsReceiver.update();
	visualsControler.update(channelsReceiver);
}

//--------------------------------------------------------------
void ofApp::draw(){

	// Before drawing visual, we need to perform all transformations
	// on the backgroung from clearing to fading, rotation, etc, etc.
	// Here transformations on background before Visuals
	visualsControler.drawFirstTransformations();

	// The WaveDrawer classes draw differents kinds of Visual concepts
	// all concepts or none can be active at a time. 
	// Activation of a WaveDrawer concept could be made algortimically
	// or by keyboard commands or even other commands (midi, OSC, DMX, etc)
	// The VisualsControler  maintains the list of WaveDrawers, controls
	// the activation from the various interfaces, and the Z-Order of visuals
	visualsControler.draw();
	
	// Once visuals are drawn, we can perform frame based transformations
    // such as shaders, filters, Kaleidoscopes, rotations, etc, etc
	// call post frame transformations
	visualsControler.drawLastTransformations();

	// Once drawing is fully finish we can save the frame
	if (bSaveFrames) ofSaveFrame();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	switch (key)
	{
	case '1':
		visualsControler.characterScenario();
		break;
	case '2':
		visualsControler.sparksScenario();
		break;
	case 'S':
	case 's': //Brain Sparks
		visualsControler.conmuteSparkingPoint();
		break;
	case 'c':
	case 'C': // Crazy Line
		visualsControler.conmuteCrazyLine();
		break;
	case 'B':
	case 'b': // Blue point 
		visualsControler.conmuteBigCircle();
		break;
	case 'd':
	case 'D': // dreamer curtain
		visualsControler.conmuteCurtain();
		break;
	case 'p':
	case 'P': // dreamer curtain
		visualsControler.conmutePulsingCircunference();
		break;
	case 'F':
	case 'f':
		bSaveFrames = !bSaveFrames;
		if (bSaveFrames) cout<<"\nframe saving activated";
		else cout << "\nframe saving deactivated";
		break;
	case 'I':
	case 'i': // show keyboard instrucction
		ShowKeyboardInstructions();
	break;
	case 'V':
	case 'v':
		if (ofGetLogLevel() == OF_LOG_VERBOSE) {
			// verbose variables to be removed
			ofSetLogLevel(OF_LOG_SILENT);
			cout<<"\nverbose mode deactivated";
		}
		else {
			ofSetLogLevel(OF_LOG_VERBOSE);
			cout << "\nverbose mode deactivated";
		}
		break;
	case 'l':
	case 'L':
		channelsReceiver.logStatistics();
		break;
	case ' ':
		ofGetWindowPtr()->toggleFullscreen();
		visualsControler.onChangeWindowSize();
		channelsReceiver.updateAttractionCenter();
	break;
	default:
		break;

	}// switch
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}


void ofApp::ShowKeyboardInstructions()
{
	cout << "\nThis is Neuro Cine, by AvidaBits";
	cout << "\nOlimex device must be connected";
	cout << "\nOpenVibe Server must be connected and playing";
	cout << "\nOpenVibe Designer must have the OCSMeans.xml scenary open and playing";
	cout << "\nThe keyboards available commands are as follows:";
	cout << "\n I, i makes this instruccions appear again";
	cout << "\n V, v set the verbose mode";
	cout << "\n L, l log statistics on screen and on file NeuroLog.txt";
	cout << "\n SPACEBAR toggles fullscreen mode";
	cout << "\n 1, Abstract character scenario";
	cout << "\n 2, Sparks scenario";
	cout << "\n F, f activated/deactivated the saving frames mode";
	cout << "\n S, s activated/deactivated the visual: brain Sparks";
	cout << "\n C, c activated/deactivated the visual: Crazy line";
	cout << "\n B, b activated/deactivated the visual: Blue point";
	cout << "\n D, d activated/deactivated the visual: Drowsiness curtain";
	cout << "\n P, p activated/deactivated the visual: Pulsing circunference";
	cout << "\n";
}


