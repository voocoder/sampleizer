/*
 *  Sampleizer.h
 *  sampleizer
 *
 *  Created by Frederik Schroff on 1/3/12.
 *  Copyright 2012 voocoder. All rights reserved.
 *
 */

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sndfile.h>
#include <math.h>
#include <stdlib.h>
#include "Waveform.h"
#include "boost/tuple/tuple.hpp"

using namespace std;

class Sampleizer {
private:
	vector< Waveform > frameVector; //new sampleVector, instead of [frame][channel][sample] = [waveform]
	int duration; //duration of new file [in samples]
	int sampleRate; //samplerate of new file
	int waveformType; //waveform format of new file
	int channels; //number of channels of new file
	int frames; //number of frames
	int frameDuration; //length of one frame of new file
	Waveform inputWaveform; //input Waveform
	vector< boost::tuple<int,int,double> > sectionTupleVector; //vector of tuple with (startPoint, duration, timeModificationParameter)
	
public:
	//constructors
	Sampleizer(Waveform inputWaveform);
	
	//methods
	void doSections(string inputSpzFilePath);
	void doRandom(int frames, int frameDuration);
	void doLinear(int frames, int frameDuration);
	void doRepetitions(int repetitions, int permutation);
	
	Waveform convertToWaveform();
	Waveform convertToWaveform(Envelope env);
	
	//get methods
	int getDuration();
	Waveform getFrame(int index);
	
	//utility
	int randGen(int maximum);
	double doubleRandGen(double min, double max);
	void parseFile(string fileInputPath);
	void saveDataToFile(string fileOutputPath);	
	void dumpSectionTupleVector();
};