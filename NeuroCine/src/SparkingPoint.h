#pragma once
#include "WaveDrawer.h"

class Spark
{
public:
	Spark();
	Spark(ofColor _color, ofPoint _position, float _radius);
	void setup(ofColor _color, ofPoint _position, float _radius);
	void draw();
	void update();
	void inertialGo(ofPoint extVelocity, ofPoint extAcceleration);
	float getRadius() { return radius; };
protected:
	ofColor color;
	float radius;
	uint64_t birdtime;
	ofPoint position, velocity, acceleration;
	float  ageMillis();


};

class SparksSet :public Spark
{
public:
	
	SparksSet(ChannelsReceiver &channels, vector<ofColor> palette, ofPoint _position, float _radius);
	void update();
	void draw();
private:
	vector<Spark> sparks;
};

class SparkingPoint : public WaveDrawer
{
public:
	SparkingPoint();
	void setup();
	void update(ChannelsReceiver &channels);
	void draw();
	~SparkingPoint();
private:
	vector<SparksSet *> sparksSets;
	vector<ofColor> palette;
};
