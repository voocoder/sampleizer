#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "Sampleizer.h"
#include <string>
#include "boost/tuple/tuple.hpp"

using namespace std;
using namespace boost;

int main (int argc, char * const argv[]) {
	srand(time(0)); //resets the randomgen/ only once per runtime!
	
	//some default values
	int numberOfFrames = 10; 
	int frameDuration = 10000;
	string inputFilePath = "/Users/voo/Desktop/Beethoven.wav";
	string inputSpzFilePath = "/Users/voo/Desktop/input.spz";
	string outputFilePath = "/Users/voo/Desktop/output.wav";
	
	int numberOfRepetitions = 0;
	int permutation = 0;
	int algorithm = 0;
	
	//if invoked via terminal
	for (int i=1; i<argc; i++) {
		if (i+1!=argc) {
			if (string(argv[i]) == "-f") {
				numberOfFrames = atoi(argv[i+1]);
			}
			else if (string(argv[i]) == "-d") {
				frameDuration = atoi(argv[i+1]);
			}
			else if (string(argv[i]) == "-i") {
				inputFilePath = argv[i+1];
				cout << inputFilePath;
			}
			else if (string(argv[i]) == "-s") {
				inputSpzFilePath = argv[i+1];
			}
			else if (string(argv[i]) == "-o") {
				outputFilePath = argv[i+1];
			}
			else if (string(argv[i]) == "-r") {
				numberOfRepetitions = atoi(argv[i+1]);
			}
			else if (string(argv[i]) == "-p") {
				permutation = atoi(argv[i+1]);
			}
			else if (string(argv[i]) == "-a") {
				algorithm = atoi(argv[i+1]);
			}
		}
	}
	
	//minimal setup
	Waveform inputWaveform(inputFilePath);
	Envelope env(0.01, 0, 1, 0.01);
	Sampleizer s(inputWaveform);
	
	if (algorithm==0) { //linear
		s.doLinear(numberOfFrames, frameDuration);
		s.convertToWaveform(env).writeWaveformToFile(outputFilePath);
	}
	else if (algorithm==1) { //random
		s.doRandom(numberOfFrames, frameDuration);
		s.convertToWaveform(env).writeWaveformToFile(outputFilePath);
	}
	else if (algorithm==2) { //section
		s.doSections(inputSpzFilePath);
		s.convertToWaveform(env).writeWaveformToFile(outputFilePath);
	}
	
	//time transformation examples
	vector<pair<int, double> > pointVector;
	pair<int, double> p1 = make_pair(0, 0.2);
	pair<int, double> p2 = make_pair(1000, 0.5);
	pair<int, double> p3 = make_pair(2000, 3);
	pointVector.push_back(p1);
	pointVector.push_back(p2);
	pointVector.push_back(p3);
	
	Envelope envTime(pointVector);
	Waveform timeTransWaveform = inputWaveform.doTimeTransformation(envTime);
	timeTransWaveform.writeWaveformToFile("/Users/voo/Desktop/timeTransWF.wav");
	
	//operation example
	Waveform summand = inputWaveform + timeTransWaveform + inputWaveform + timeTransWaveform;
	summand.writeWaveformToFile("/Users/voo/Desktop/summand.wav");
	return 0;
}
