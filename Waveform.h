/*
 *  Waveform.h
 *  sampleizer
 *
 *  Created by Frederik Schroff on 1/3/12.
 *  Copyright 2012 voocoder. All rights reserved.
 *
 */

// to handle the input and output of waveforms / analysis?
#include <iostream>
#include <sndfile.h>
#include "Envelope.h"
#include <vector>
#include <math.h>


using namespace std;

class Waveform {

private:
	vector<vector<double> > waveformVector; //[channel][sample]
	int duration; //duration of input sound file in samples
	int channels;
	int sampleRate;
	int waveformType;
	
	SF_INFO sfInfo;
	
	vector<vector<double> > splitFile(SNDFILE* inputFile);
	
public:
	//constructors
	Waveform();
	Waveform(string inputPath); //fill waveformArray with sound file
	Waveform(int dur, int sr, int numberOfChannels, int wftype); //create new empty waveform with fixed length(dur), and set sfinfo
	
	//methods
	void writeWaveformToFile(string outputPath); //write soundfile
	Waveform doTimeTransformation(double parameter);
	Waveform doTimeTransformation(Envelope env);
	
	//set methods
	void writeSampleToWaveform(int channel, int sample, double val);
	void pushBackSample(int channel, double val);
	void clearChannel(int channel);
	
	//operators
	Waveform operator*(Envelope env);
	Waveform operator+(Waveform wf);
	
	//get methods
	int getNumberOfChannels();
	int getDuration();
	int getSampleRate();
	int getWaveformType();
	double getSample(int ch, int sample);
	Waveform getSection(int startingPoint, int durationInSamples);
};
