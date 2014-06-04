#pragma once
#include <vector>
#include <string>
#include <iostream>

#include "Train.h"

class NeuralNetwork
{
public:
	NeuralNetwork(int ID) : networkObjID(ID) {
		useBRDF = false;
		useNormal = ID < 2 ? true : false; // planes don't need normal.
		wasteXpID = -1;
		InputSize = 9; // brdf is wasted.
		if(ID >= 2){ // planes don't need one sub-position.
			if(ID == 2 || ID == 3){
				wasteXpID = 0;
			}
			else if(ID == 7 || ID == 6){
				wasteXpID = 2;
			}
			else{ // ID == 5, since ID=4 is light
				wasteXpID = 1;
			}
			InputSize = 5;
		}
		OutputSize = 3;
		FirstHiddenSize = 20;
		SecondHiddenSize = 10;
	}
	
	int	 networkObjID; // corresponding to scene object.
	bool useBRDF; // false
	bool useNormal; // ball & buddha true, others false
	int  wasteXpID; // ball & buddha -1, planes all have one waste from 0 to 2.
	
	int InputSize;	// decide by ID
	int OutputSize; // 3
	int FirstHiddenSize; // 20
	int SecondHiddenSize; // 10


public:
	float bias1[20], bias2[10], bias3[3]; // 1st hidden, 2nd hidden and output layer.
	float IW[20][9];	// for box we only use IW[20][5]
						// input x 1st hidden layer.
	float LW1[10][20];	// 1st x 2nd hidden layer.
	float LW2[3][10];   // 2nd x output layer;

	float inMin[9], inMax[9], outMin[3], outMax[3];

public:
	
	void loadWeights(const std::string &b1File, const std::string &b2File, const std::string &b3File,
		const std::string &IWFile, const std::string &LW1File, const std::string &LW2File, const std::string &MinMaxFile);
	void printWeights() const;

	vec3f predict(const Train::TrainData &oneData) const;

	__forceinline float tansig_func(float input) const{
		return (2 / (1 + exp(-2*input)) - 1);
	}

	typedef enum {APPLY, REVERSE} TYPE;  // map -> -1,1
	__forceinline void map_minmax(TYPE type, int len, float input[], const float minvec[], const float maxvec[]) const{
		bool all_the_same = true;
		for(int i = 0; i < len-1; i++){
			if(input[i] != input[i+1]){
				all_the_same = false;
				break;
			}
		}

		if(type == APPLY && !all_the_same){
			for(int i = 0; i < len; i++){
				input[i] = 2 * (input[i] - minvec[i]) / (maxvec[i] - minvec[i]) - 1;
			}
		}

		if(type == REVERSE && !all_the_same){
			for(int i = 0; i < len; i++){
				input[i] = (input[i] + 1) * (maxvec[i] - minvec[i]) / 2 + minvec[i];
			}
		}
		return ;
	}
};

