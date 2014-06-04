#pragma once
#include "NeuralNetwork.h"

class Test
{
	Train *train;
public:
	Test(void);
	~Test(void);

public:
	void setTrainPtr(Train *train){
		this->train = train;
	}

	void buildNetworks();

public:
	std::vector<NeuralNetwork> objNetworks;   // indirect illumination
	std::vector<NeuralNetwork> objNetworks2;  // direct illumination
	vec3f predict(const Train::TrainData &oneData, int objID);

	void debug();
};

