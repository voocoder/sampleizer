/*
 *  Envelope.cpp
 *  sampleizer
 *
 *  Created by Frederik Schroff on 7/3/12.
 *  Copyright 2012 voocoder. All rights reserved.
 *
 */

#include "Envelope.h"

//~~ constructors ~~
Envelope::Envelope(double aTime, double dTime, double sLevel, double rTime) 
: attackTime(aTime), decayTime(dTime), sustainLevel(sLevel), releaseTime(rTime), envType(0) {
	if (!between(aTime) || !between(dTime) || !between(sLevel) || !between(rTime) || (aTime+dTime+rTime > 1.0)) {
		cout << "An error occured. At least one parameter isn't between 0.0 and 1.0 or the overall percentage of a,d,r is above 100%\n";
		cout << "I took the liberty to set a, d, r values to 0.1 each and the sustain level to 1\n";
		attackTime = 0.1;
		decayTime = 0.1;
		sustainLevel = 1;
		releaseTime = 0.1;
	}
}

//constructor that takes vector of pairs, check if in right form
Envelope::Envelope(vector< pair <int, double> > pV) : pointVector(pV), envType(1) {
	for (int i=1; i<pointVector.size(); i++) {
		//check if x-axis points are consecutive
		int prePoint = pointVector[i-1].first;
		int curPoint = pointVector[i].first;
		if (curPoint-prePoint<1) {
			cout << "Wrong initialisation! At least one Point is in wrong order. Changed pointVector to (0, 1)\n";
			pointVector.clear();
			pair<int,double> tmp(0, 1);
			pointVector.push_back(tmp);
		}
		//check if y-axis values are signed identical
		double preLevel = pointVector[i-1].second;
		double curLevel = pointVector[i].second;
		if ((preLevel>0 && curLevel<0) || (preLevel<0 && curLevel>0)) {
			cout << "Wrong initialisation! At least one Level has a different sign than the others. Changed pointVector to (0, 1)\n";
			pointVector.clear();
			pair<int,double> tmp(0,1);
			pointVector.push_back(tmp);
		}
	}
}


//~~ get & set methods ~~
//adsr
double Envelope::getAttackTime() {
	return attackTime;
}

double Envelope::getDecayTime() {
	return decayTime;
}

double Envelope::getSustainLevel() {
	return sustainLevel;
}

double Envelope::getReleaseTime() {
	return releaseTime;
}

//point Vector
int Envelope::getTimeAt(int index) {
	return pointVector[index].first;
}

double Envelope::getLevelAt(int index) {
	return pointVector[index].second;
}

int Envelope::getPointVectorSize() {
	int ret = -1;
	if (envType==1) {
		ret = pointVector.size();
	}
	return ret;
}

//for both
int Envelope::getType() {
	return envType;
}

//adds new entry behind last pointVector entry
void Envelope::pushBack(pair<int, double> entry) {
	pointVector.push_back(entry);
}


//~~ utility ~~
//test if value lies between [0,1]
bool Envelope::between(double test) {
	bool ret = false;
	if (test>=0.0 && test<=1.0) {
		ret = true;
	}
	return ret;
}

//prints contents of Envelope object to console, dependent on Envelope type
void Envelope::dump() {
	if (envType==0) { //adsr
		cout << "Attack Time: " << attackTime << endl;
		cout << "Decay Time: " << decayTime << endl;
		cout << "Sustain Level: " << sustainLevel << endl;
		cout << "Release Time: " << releaseTime << endl;
	}
	else if (envType==1){ //pointVector
		for (int i=0; i<pointVector.size(); i++) {
			cout << "(" << pointVector[i].first << " . " << pointVector[i].second << ")\n";
		}
	}
}