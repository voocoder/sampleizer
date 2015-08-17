/*
 *  Envelope.h
 *  sampleizer
 *
 *  Created by Frederik Schroff on 7/3/12.
 *  Copyright 2012 voocoder. All rights reserved.
 *
 */

#include <vector>
#include <utility>
#include <iostream>

using namespace std;

class Envelope{
private:
	double attackTime, decayTime, sustainLevel, releaseTime;
	vector<pair <int, double> > pointVector;
	int envType; //0=adsr, 1=pointVector
	
public:
	//constructors
	Envelope(double a, double d, double s, double r);
	Envelope(vector<pair <int, double> > pointVector);
	
	//get & set methods
	//adsr
	double getAttackTime();
	double getDecayTime();
	double getSustainLevel();
	double getReleaseTime();
	int getType();
	//point Vector
	int getTimeAt(int index);
	double getLevelAt(int index);
	vector<pair <int, double> > getPointVector();
	int getPointVectorSize();
	void pushBack(pair <int, double> entry);
	
	//utility
	bool between(double test);
	void dump();
	
};