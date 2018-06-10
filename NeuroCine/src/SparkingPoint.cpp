#include "SparkingPoint.h"

//SPARKS 
Spark::Spark()
{
	setup(255, {0,0}, 1);
}
Spark::Spark(ofColor _color, ofPoint _position, float _radius)
{
	setup(_color, _position, _radius);
}

void Spark::setup(ofColor _color, ofPoint _position, float _radius)
{
	color = _color;
	radius = _radius;
	birdtime = ofGetElapsedTimeMillis();
	position = _position;
	velocity = { 0.5,0 };
	velocity.rotate(ofRandom(180), ofVec3f(0, 0, 1));
	acceleration = { 0, 0 }; // constante velocity, for the moment

}

float  Spark::ageMillis() { return ofGetElapsedTimeMillis() - birdtime; }

void Spark::draw()
{
	float alpha = ofMap(ageMillis(), 0, 10000, 255, 0);//al llegar a 10 segundos se vuelve trasparente
	ofSetColor(color, alpha); //255->opaco
	ofSetLineWidth(0);
	ofFill();
	ofDrawCircle(position, radius);
}

void Spark::update()
{
	if (radius>0) radius = radius - 0.01;
	if (radius<0) radius = 0;
	velocity+=acceleration;
	velocity.rotate(ofRandom(-45, 45), ofVec3f(0, 0, 1)); //random noise in direction
	position+=velocity;
}

void Spark::inertialGo(ofPoint extVelocity, ofPoint extAcceleration)
{
	// this method is for the spark move with a external force
	extVelocity+= extAcceleration;
	position+= extVelocity;

}

///////////////////////SPARK SETS 
SparksSet::SparksSet(ChannelsReceiver &channels, vector<ofColor> palette, ofPoint _position, float _radius) 
{
	assert(palette.size() >= 6); //fisrt color is backgroun
	Spark::Spark( palette[0],  _position,  _radius);
	// the maximun historical maximun used for radius computing, but not including arousal and valence
	float maxGlobal = max(channels.delta.getHistoricalMax(), channels.theta.getHistoricalMax());
	maxGlobal = max(maxGlobal, channels.beta.getHistoricalMax());
	maxGlobal = max(maxGlobal, channels.alpha.getHistoricalMax());
	maxGlobal = max(maxGlobal, channels.gamma.getHistoricalMax());

	// just one spark per channel, 5 sparks! 
	sparks.resize(5);
	float radiusChannel = ofMap(channels.delta.getMean(), 0, maxGlobal, 5, _radius);
	sparks[0].setup(palette[1], _position, radiusChannel);
	radiusChannel = ofMap(channels.theta.getMean(), 0, maxGlobal, 5, _radius);
	sparks[1].setup(palette[2], _position, radiusChannel);
	radiusChannel = ofMap(channels.alpha.getMean(), 0, maxGlobal, 5, _radius);
	sparks[2].setup(palette[3], _position, radiusChannel);
	radiusChannel = ofMap(channels.beta.getMean(), 0, maxGlobal, 5, _radius);
	sparks[3].setup(palette[4], _position, radiusChannel);
	radiusChannel = ofMap(channels.gamma.getMean(), 0, maxGlobal, 5, _radius);
	sparks[4].setup(palette[5], _position, radiusChannel);

}

void SparksSet::update() {

	Spark::update();
	for (auto &s : sparks)
	{
		s.update();
		s.inertialGo(velocity, position);
	}

}

void SparksSet::draw()
{
	for (auto &s : sparks) s.draw();
}


////////////////////// SPARKING POINT 
SparkingPoint::SparkingPoint()
{
	palette.push_back(ofColor(0, 0, 0));//background
	palette.push_back(ofColor(13, 95, 163)); //delta
	palette.push_back(ofColor(92, 9, 132)); //theta
	palette.push_back(ofColor(171, 9, 114)); //high alpha
	palette.push_back(ofColor(222, 9, 66)); //high beta
	palette.push_back(ofColor(237, 161, 23)); //high gamma
}
SparkingPoint::~SparkingPoint()
{

	for (auto it = sparksSets.begin(); it != sparksSets.end(); ) {
		SparksSet *pSet = *it;
		it = sparksSets.erase(it);
		delete pSet;
	}

}

void SparkingPoint::setup()
{
	bActive = false;
}


void SparkingPoint::update(ChannelsReceiver &channels) 
{
	if (!bActive) return;

	SparksSet *pSet = new SparksSet(channels, palette, channels.attractionCenter, min(ofGetWidth(), ofGetHeight())/10);
	sparksSets.push_back(pSet);

	for (auto &ss : sparksSets) ss->update();

	// cleaning of older sparks sets if radio reachs zero
	for (auto it = sparksSets.begin(); it != sparksSets.end(); ) {
		pSet = *it;
		if (pSet->getRadius()<= FLT_EPSILON) {
			it = sparksSets.erase(it);
			delete pSet;
		}
		else {
			++it;
		}
	}


}

void SparkingPoint::draw() 
{
	if (!bActive) return;

	for (auto &ss : sparksSets) ss->draw();


} //draw