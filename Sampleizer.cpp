/*
 *  Sampleizer.cpp
 *  sampleizer
 *
 *  Created by Frederik Schroff on 1/3/12.
 *  Copyright 2012 voocoder. All rights reserved.
 *
 */
#include "Sampleizer.h"

using namespace std;
using namespace boost;

//~~ constructor ~~
Sampleizer::Sampleizer(Waveform inputWaveform)
: inputWaveform(inputWaveform) {
	
	waveformType = inputWaveform.getWaveformType();
	sampleRate = inputWaveform.getSampleRate();
	channels = inputWaveform.getNumberOfChannels();
}


//~~ methods ~~
//get specified amout of frames randomly from the input file
void Sampleizer::doRandom(int frames, int frameDuration) {
	if (frameDuration>inputWaveform.getDuration()) {
		frameDuration = inputWaveform.getDuration();
		cout << "Frame Duration too big, setting it to input waveform duration!" << endl;
	}
	frameVector.clear();
	
	Waveform tmp;
	for (int frame=0; frame<frames; frame++) {
		int start = randGen(inputWaveform.getDuration()-frameDuration);
		tmp = inputWaveform.getSection(start, frameDuration);
		frameVector.push_back(tmp);
	}
}

//get specified amount of frames linearly from the input file
void Sampleizer::doLinear(int frames, int frameDuration) {
	if (frameDuration>inputWaveform.getDuration()) {
		frameDuration = inputWaveform.getDuration();
		cout << "Frame Duration too big, setting it to input waveform duration!" << endl;
	}
	frameVector.clear();
	
	int step;
	if (frames==1) {
		step = floor((inputWaveform.getDuration() - (frames*frameDuration)) / frames);
	}
	else {
		step = floor((inputWaveform.getDuration() - (frames*frameDuration)) / (frames-1));
	}
	int start = 0;
	if (step<0) { //if frameDuration too big set to maximum possible duration
		cout << "frame duration is too big!\n";
		frameDuration = floor(inputWaveform.getDuration()/frames);
		cout << "... set frame duration to: " << frameDuration << endl;
		step=0;
	}
	Waveform tmp;
	for (int frame=0; frame<frames; frame++) {
		tmp = inputWaveform.getSection(start, frameDuration);
		frameVector.push_back(tmp);
		start += step + frameDuration;
	}
}

//get all sections provided in spz file (during initialization)
void Sampleizer::doSections(string inputSpzFilePath) {
	frameVector.clear();
	parseFile(inputSpzFilePath);
	
	for (int i=0; i<sectionTupleVector.size(); i++) {
		int startPoint = get<0>(sectionTupleVector[i]);
		int sectionDuration =  get<1>(sectionTupleVector[i]);
		double timeFactor = get<2>(sectionTupleVector[i]);

		Waveform tmp;
		tmp = inputWaveform.getSection(startPoint, sectionDuration);
		if (timeFactor!=1) {
			tmp.doTimeTransformation(timeFactor);
		}
		frameVector.push_back(tmp);
	}
}


//repeats all waveforms in object with specified permutation
void Sampleizer::doRepetitions(int rep, int permutation) {
//todo/to consider: different permutation for each repetition
	vector<Waveform> tmp = frameVector;
	for (int r=0; r<rep; r++) {
		switch (permutation) {
			case 0: //no permutation
				for (int i=0; i<tmp.size(); i++) {
					frameVector.push_back(tmp[i]);
				}
				break;
				
			case 1: //reverse
				for (int i=tmp.size()-1; i>=0; i--) {
					frameVector.push_back(tmp[i]);
				}
				break;
				
			case 2: //random
				while (!tmp.empty()) {
					int rand = randGen(tmp.size());
					frameVector.push_back(tmp[rand]);
					tmp.erase(tmp.begin()+rand);
				}
				break;

			default:
				for (int i=0; i<tmp.size(); i++) {
					frameVector.push_back(tmp[i]);
				}
				break;
		}
	}
}

//converts all waveforms in frameVector to a single one
Waveform Sampleizer::convertToWaveform() {
	int totalDuration = getDuration();
	Waveform tmpWF(totalDuration, sampleRate, channels, waveformType);
	
	for (int ch=0; ch<channels; ch++) {
		int currentSampleCount = 0;
		for (int frame=0; frame<frameVector.size(); frame++) {
			for (int sample=0; sample<frameVector[frame].getDuration(); sample++) {
		 		double val = frameVector[frame].getSample(ch, sample);
				tmpWF.writeSampleToWaveform(ch, currentSampleCount, val);
				currentSampleCount++;
			}
		}
	}
	return tmpWF;
}

Waveform Sampleizer::convertToWaveform(Envelope env) {
	for (int f=0; f<frameVector.size(); f++) {
		frameVector[f] = frameVector[f] * env;
	}
	return convertToWaveform();
}

//~~ get methods ~~
Waveform Sampleizer::getFrame(int pos) {
	return frameVector[pos];
}

//returns total duration of all waveforms in object
int Sampleizer::getDuration() {
	int dur = 0;
	for (int i=0; i<frameVector.size(); i++) {
		dur += frameVector[i].getDuration();
	}
	return dur;
}


//~~ utility methods ~~

//reads .spz file and converts content to sectionTupleVector
void Sampleizer::parseFile(string path) {
	//todo measures to check if provided file is correct
	ifstream spzFile;
	char row[1024];
	spzFile.open(path.c_str(), ios::in);
	if (spzFile.good()) {
		spzFile.seekg(0L, ios::beg);
        while (! spzFile.eof())
        {
            spzFile.getline(row, 1024);
			int sample;
			int duration;
			double timeFactor;
			
			int count = 0;
			char *ptr = strtok(row, " ");
			while (ptr!=NULL) {
				switch (count) {
					case 0:
						sample = atoi(ptr);
						break;
					case 1:
						duration = atoi(ptr);
						break;
					case 2:
						timeFactor = atof(ptr);
						break;
					default:
						cout << "Something went wrong in the parseFile method, most likely an spzFile error" << endl;
						break;
				}
				count++;
				ptr = strtok(NULL, " ");
			}
			tuple<int,int,double> tmp(sample, duration, timeFactor);
			sectionTupleVector.push_back(tmp);
        }
	}
	else {
		cout << "Datei konnte nicht geoeffnet werden!" << endl;
	}
}

//saves section data [startPoints, durations, time modification factor] to file
void Sampleizer::saveDataToFile(string path) {
	fstream file;
	file.open(path.c_str(), ios::out);
	for (int i=0; i<sectionTupleVector.size(); i++) {
		char out[1024];
		sprintf(out, "%d %d %f", get<0>(sectionTupleVector[i]), get<1>(sectionTupleVector[i]), get<2>(sectionTupleVector[i]));
		file << out << endl;
	}
	file.close();
}

//prints content of the sectionTupelVector(starting points, durations, time modification factors) to console
void Sampleizer::dumpSectionTupleVector() {
	for (int i=0; i<sectionTupleVector.size(); i++) {
		cout << "(" << get<0>(sectionTupleVector[i]) << " . " << get<1>(sectionTupleVector[i]) << " . " << get<2>(sectionTupleVector[i]) << ")\n";
	}
}

//generates random integer number
int Sampleizer::randGen(int maximum) {
	return rand() % maximum;
}

//generates random double number
double Sampleizer::doubleRandGen(double min, double max) {
	double r = (double)rand() / RAND_MAX;
	return min + r * (max - min);
}