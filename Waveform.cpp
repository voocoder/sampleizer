/*
 *  Waveform.cpp
 *  sampleizer
 *
 *  Created by Frederik Schroff on 1/3/12.
 *  Copyright 2012 voocoder. All rights reserved.
 *
 */

#include <iostream>
#include <sndfile.h>
#include "Waveform.h"

//~~ constructors ~~
Waveform::Waveform() {
}

Waveform::Waveform(int dur, int sr, int numberOfChannels, int wfType) {
	duration = dur;
	channels = numberOfChannels;
	sampleRate = sr;
	waveformType = wfType;
	
	sfInfo.samplerate = sampleRate;
	sfInfo.channels = channels;
	sfInfo.format = waveformType;
	
	this->waveformVector = vector<vector<double> > (channels, vector<double>(duration));
}

//constructor to load file given by inputPath
Waveform::Waveform(string inputPath) {
	SNDFILE* inFile;

	if(!(inFile = sf_open(inputPath.c_str(), SFM_READ, &sfInfo)))
	{
		cout << "Not able to open " << inputPath << endl;
		sf_perror(NULL);
	}
	else {
		duration = sfInfo.frames;
		channels = sfInfo.channels;
		sampleRate = sfInfo.samplerate;
		waveformType = sfInfo.format;
		waveformVector = splitFile(inFile);
	}
	sf_close(inFile);
}

//splits one dimensional array from libsndfile into two dimensional vector, split by channel
vector< vector<double> > Waveform::splitFile(SNDFILE* inputFile){
	double* data = new double[duration * channels];

	vector< vector<double> > splitByChannel(channels);
	
	sf_readf_double(inputFile, data, duration);
	
	int count = 0;
	for (int sample=0; sample<duration; sample++) {
		for (int ch=0; ch<channels; ch++) {
			splitByChannel[ch].push_back(data[count]);
			count++;
		}
	}
	delete [] data;
	
	return splitByChannel;	
}

void Waveform::writeWaveformToFile(string outputPath){
	double* mergeArray = new double[duration*channels];
		
	//open output file for writing
	SNDFILE* outFile = sf_open(outputPath.c_str(), SFM_WRITE, &sfInfo);

	int mergeCount = 0;
	//write two dimensional vector back to one dimensional array for libsndfile
	for (int sample=0; sample<duration; sample++) {
		for (int ch=0; ch<channels; ch++) {
			mergeArray[mergeCount] = waveformVector[ch][sample];
			mergeCount++;
		}
	}
	
	//actually write it to the output file
	sf_writef_double(outFile, mergeArray, duration);

	delete [] mergeArray;

	sf_close(outFile);
}

//do constant time transformation on waveform with given factor
Waveform Waveform::doTimeTransformation(double parameter) {
	Waveform tmp = (*this);
	if (parameter<0) {
		//reverse
		for (int ch=0; ch<channels; ch++) {
			tmp.clearChannel(ch);
			double count = duration;
			
			while (count>0) {
				int step = floor(count);
				double remainder = count - step;
				double leftPart = waveformVector[ch][count] * (1-remainder);
				double rightPart = waveformVector[ch][count+1] * remainder;
				tmp.pushBackSample(ch, leftPart + rightPart);
				count += parameter;
			}
		}
		(*this)=tmp;
	}
	else if (parameter>0) {
		for (int ch=0; ch<channels; ch++) {
			tmp.clearChannel(ch);
			double count = 0;
			while (count<duration) {
				int step = floor(count);
				double remainder = count - step;
				double leftPart = waveformVector[ch][count] * (1-remainder);
				double rightPart = waveformVector[ch][count+1] * remainder;
				tmp.pushBackSample(ch, leftPart + rightPart);
				count += parameter;
			}
		}
		(*this)=tmp;
	}
	//parameter = 0
	else {
		for (int ch=0; ch<channels; ch++) {
			tmp.clearChannel(ch);
			tmp.pushBackSample(ch, 0);
		}
	}
	return tmp;
}

//do continuous time transformation to points set in the envelope
Waveform Waveform::doTimeTransformation(Envelope env) {
	Waveform tmp = (*this);
	
	//checks if last given start point exceeds duration
	if(env.getTimeAt(env.getPointVectorSize()-1) > duration) {
		//last point out of range
		cout << "Point Vector out of range, please check your Envelope. Nothing happend\n";
	}
	else {
		double lastLevel = env.getLevelAt(env.getPointVectorSize()-1);
		pair<int, double> lastPair (duration, lastLevel);
		env.pushBack(lastPair);
		//debug output
		env.dump();
		for (int ch=0; ch<channels; ch++) {
			tmp.clearChannel(ch);
			for (int i=1; i<env.getPointVectorSize(); i++) {
				double startLevel = env.getLevelAt(i-1);
				double endLevel = env.getLevelAt(i);
				int startSample	= env.getTimeAt(i-1);
				int endSample = env.getTimeAt(i);
				
				int deltaSample = endSample - startSample;
				double deltaLevel = (endLevel - startLevel);
				double step = deltaLevel/(deltaSample/startLevel);
				double currentLevel = startLevel;
				
				if (startLevel>0) {
					double sample = startSample;
					while (sample<endSample) {
						int sampleStep = floor(sample);
						double remainder = sample - sampleStep;
						double leftPart = waveformVector[ch][sampleStep] * (1-remainder);
						double rightPart = waveformVector[ch][sampleStep+1] * remainder;
						tmp.pushBackSample(ch, (leftPart+rightPart));
						
						sample += currentLevel;
						currentLevel += step;
						step = deltaLevel/(deltaSample/currentLevel);
					}
				}
				else {
					double sample = endSample;
					while (sample>startSample) {
						int sampleStep = floor(sample);
						double remainder = sample - sampleStep;
						double leftPart = waveformVector[ch][sampleStep] * (1-remainder);
						double rightPart = waveformVector[ch][sampleStep+1] * remainder;
						tmp.pushBackSample(ch, (leftPart+rightPart));
						
						sample += currentLevel;
						currentLevel -= step;
						step = deltaLevel/(deltaSample/currentLevel);
					}
				}

			}
		}
	}
	return tmp;
}

//~~ set methods ~~
void Waveform::writeSampleToWaveform(int ch, int sample, double val){
	waveformVector[ch][sample] = val;
}

//inserts sample value at end of the waveformVector to a given channel, increasing vector size by one
void Waveform::pushBackSample(int ch, double val) {
	waveformVector[ch].push_back(val);
	duration++;
}

void Waveform::clearChannel(int ch) {
	waveformVector[ch].clear();
	duration = 0;
}


//~~ overloaded operators ~~
//function to employ envelopes on waveforms in an intuitive way
Waveform Waveform::operator*(Envelope env) {
	Waveform tmp(duration, sampleRate, channels, waveformType);
	double a = env.getAttackTime();
	double d = env.getDecayTime();
	double r = env.getReleaseTime();
	double sLevel = env.getSustainLevel();
	double val;
	//duration of the different values in samples
	int aDur = duration*a;
	int dDur = duration*d;
	int rDur = duration*r;
	
	aDur = (aDur>0) ? aDur : 1;
	dDur = (dDur>0) ? dDur : 1;
	rDur = (rDur>0) ? rDur : 1;
	//envelope processing
	int sustainStart = aDur + dDur;
	double decayStep = (1.0 - sLevel)/dDur;
	int releaseStart = duration - rDur;
	double releaseStep = sLevel/rDur;
	
	for (int ch=0; ch<channels; ch++) {
		//attack
		for (int sample=0; sample<aDur; sample++) {
			val = waveformVector[ch][sample] * (double) sample/aDur;
			tmp.writeSampleToWaveform(ch, sample, val);
		}
		//decay
		double currentStep = 1.0;
		for (int sample=aDur; sample<sustainStart; sample++) {
			val = waveformVector[ch][sample] * currentStep;
			tmp.writeSampleToWaveform(ch, sample, val);
			currentStep -= decayStep;
		}
		//sustain
		for (int sample=sustainStart; sample<releaseStart; sample++) {
			val = waveformVector[ch][sample] * sLevel;	
			tmp.writeSampleToWaveform(ch, sample, val);
		}
		//release
		currentStep = sLevel;
		for (int sample=releaseStart; sample<duration; sample++) {
			currentStep -= releaseStep;
			val = waveformVector[ch][sample] * currentStep;
			tmp.writeSampleToWaveform(ch, sample, val);
		}
	}
	return tmp;
}

//function to add one waveform to another. very slow for many consecutive additions 
Waveform Waveform::operator+(Waveform wf) {
	int totalDuration = duration+wf.getDuration();
	
	//create new temporary waveform and copy all samples from left and right waveform to it
	Waveform tmpWF(totalDuration, sampleRate, channels, waveformType);

	for (int ch=0; ch<channels; ch++) {
		//left waveform
		for (int sample=0; sample<duration; sample++) {
			tmpWF.writeSampleToWaveform(ch, sample, getSample(ch, sample));
		}
		//right waveform
		int currentSample = 0;
		for (int sample=duration; sample<totalDuration; sample++) {
			tmpWF.writeSampleToWaveform(ch, sample, wf.getSample(ch, currentSample));
			currentSample++;
		}
	}
	return tmpWF;
}

//~~ get methods ~~
double Waveform::getSample(int ch, int sample) {
	return waveformVector[ch][sample];
}

//returns a waveform which contains a section of this waveform (all channels)
Waveform Waveform::getSection(int startingPoint, int durationInSamples) {
	Waveform returnWaveform(durationInSamples, sampleRate, channels, waveformType);
	for (int ch=0; ch<channels; ch++) {
		returnWaveform.waveformVector[ch].assign(waveformVector[ch].begin()+startingPoint, waveformVector[ch].begin()+startingPoint+durationInSamples);
	}
	return returnWaveform;
}

int Waveform::getNumberOfChannels() {
	return channels;
}

int Waveform::getDuration() {
	return duration;
}

int Waveform::getSampleRate(){
	return sampleRate;
}

int Waveform::getWaveformType(){
	return waveformType;
}