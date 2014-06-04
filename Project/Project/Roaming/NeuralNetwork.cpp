#include "stdafx.h"
#include "NeuralNetwork.h"


extern std::vector<std::string> split(const std::string &s, char delim);

void NeuralNetwork::loadWeights(const std::string &b1File, const std::string &b2File, const std::string &b3File,
		const std::string &IWFile, const std::string &LW1File, const std::string &LW2File, const std::string &MinMaxFile)
{
//	std::cout << "start loading weights." << std::endl;
	std::ifstream FinB1(b1File.c_str()), FinB2(b2File.c_str()), FinB3(b3File.c_str()),
		FinIW(IWFile.c_str()),FinLW1(LW1File.c_str()),FinLW2(LW2File.c_str()), FinMinMax(MinMaxFile.c_str());
	
	std::string data;
	
	getline(FinB1, data);
	std::vector<std::string> splitedB1 = split(data, ',');
	//std::cout << "parse bias1... " << std::endl;
	for(int i = 0; i < FirstHiddenSize; i++){
		bias1[i] = atof(splitedB1[i].c_str());
		//std::cout << "bias1[" << i << "] = " << bias1[i] << std::endl;
	}

	getline(FinB2, data);
	//std::cout << "parse bias2... " << std::endl;
	std::vector<std::string> splitedB2 = split(data, ',');
	for(int i = 0; i < SecondHiddenSize; i++){
		bias2[i] = atof(splitedB2[i].c_str());
		//std::cout << "bias2[" << i << "] = " << bias2[i] << std::endl;
	}

	getline(FinB3, data);
	//std::cout << "parse bias3... " << std::endl;
	std::vector<std::string> splitedB3 = split(data, ',');
	for(int i = 0; i < OutputSize; i++){
		bias3[i] = atof(splitedB3[i].c_str());
		//std::cout << "bias3[" << i << "] = " << bias3[i] << std::endl;
	}

	//std::cout << "parse IW... " << std::endl;
	for(int line = 0; line < 20; line++){
		getline(FinIW, data);
		std::vector<std::string> splitedIW = split(data, ',');
		for(int i = 0; i < splitedIW.size(); i++){
			IW[line][i] = atof(splitedIW[i].c_str());
			//std::cout << IW[line][i] << ' ';
		}
		//std::cout << std::endl;
	}

	//std::cout << "parse LW1... " << std::endl;
	for(int line = 0; line < 10; line++){
		getline(FinLW1, data);
		std::vector<std::string> splitedLW1 = split(data, ',');
		for(int i = 0; i < splitedLW1.size(); i++){
			LW1[line][i] = atof(splitedLW1[i].c_str());
		//	std::cout << LW1[line][i] << ' ';
		}
		//std::cout << std::endl;
	}

	//std::cout << "parse LW2... " << std::endl;
	for(int line = 0; line < 3; line++){
		getline(FinLW2, data);
		std::vector<std::string> splitedLW2 = split(data, ',');
		for(int i = 0; i < splitedLW2.size(); i++){
			LW2[line][i] = atof(splitedLW2[i].c_str());
		//	std::cout << LW2[line][i] << ' ';
		}
		//std::cout << std::endl;
	}

	//std::cout << "Loading min max.. " << std::endl;
	for(int i = 0; i < InputSize; i++){
		getline(FinMinMax, data);
		inMin[i] = atof(data.c_str());
	//	std::cout << "inMin["<<i<<"]" << inMin[i] << ' ' ;
	}
//	std::cout << std::endl;

	for(int i = 0; i < InputSize; i++){
		getline(FinMinMax, data);
		inMax[i] = atof(data.c_str());
	//	std::cout << "inMax["<<i<<"]" << inMax[i] << ' ' ;
	}
//	std::cout << std::endl;

	for(int i = 0; i < OutputSize; i++){
		getline(FinMinMax, data);
		outMin[i] = atof(data.c_str());
	//	std::cout << "outMin["<<i<<"]"<< outMin[i] << ' ' ;
	}
	//std::cout << std::endl;

	for(int i = 0; i < OutputSize; i++){
		getline(FinMinMax, data);
		outMax[i] = atof(data.c_str());
	//	std::cout << "outMax["<<i<<"]" << outMax[i] << ' ';
	}
	//std::cout << std::endl;
	
	//std::cout << "weights loaded. " << std::endl;

	return ;
}

void NeuralNetwork::printWeights() const{
	int inputVecSize = networkObjID < 2 ? 9 : 5;

	std::cout << "parse bias1... " << std::endl;
	for(int i = 0; i < FirstHiddenSize; i++){
		std::cout << "bias1[" << i << "] = " << bias1[i] << std::endl;
	}
	std::cout << "parse bias2... " << std::endl;
	for(int i = 0; i < SecondHiddenSize; i++){
		std::cout << "bias2[" << i << "] = " << bias2[i] << std::endl;
	}
	std::cout << "parse bias3... " << std::endl;
	for(int i = 0; i < OutputSize; i++){
		std::cout << "bias3[" << i << "] = " << bias3[i] << std::endl;
	}
	std::cout << "parse IW... " << std::endl;
	for(int line = 0; line < FirstHiddenSize; line++){
		for(int i = 0; i < InputSize; i++){
			std::cout << IW[line][i] << ' ';
		}
		std::cout << std::endl;
	}
	std::cout << "parse LW1... " << std::endl;
	for(int line = 0; line < SecondHiddenSize; line++){
		for(int i = 0; i < FirstHiddenSize; i++){
			std::cout << LW1[line][i] << ' ';
		}
		std::cout << std::endl;
	}
	std::cout << "parse LW2... " << std::endl;
	for(int line = 0; line < 3; line++){
		for(int i = 0; i < SecondHiddenSize; i++){
		    std::cout << LW2[line][i] << ' ';
		}
		std::cout << std::endl;
	}
	std::cout << "Loading min max.. " << std::endl;
	for(int i = 0; i < InputSize; i++){
		std::cout << "inMin["<<i<<"]" << inMin[i] << ' ' ;
	}
	std::cout << std::endl;

	for(int i = 0; i < InputSize; i++){
		std::cout << "inMax["<<i<<"]" << inMax[i] << ' ' ;
	}
	std::cout << std::endl;

	for(int i = 0; i < OutputSize; i++){
		std::cout << "outMin["<<i<<"]"<< outMin[i] << ' ' ;
	}
	std::cout << std::endl;

	for(int i = 0; i < OutputSize; i++){
		std::cout << "outMax["<<i<<"]" << outMax[i] << ' ';
	}
	std::cout << std::endl;
}

vec3f NeuralNetwork::predict(const Train::TrainData &oneData) const{
	vec3f rgb(0.f);
	//printWeights();

	std::vector<float> inputVec;
	int inputVecSize = networkObjID < 2 ? 9 : 5;
	for(int i = 0; i < 3; i++){
		if(i == wasteXpID)	continue;
		inputVec.push_back(oneData.x_p[i]);
	}
	for(int i = 0; i < 3; i++){
		inputVec.push_back(oneData.v[i]);
	}
	if(useNormal){
		for(int i = 0; i < 3; i++){
			inputVec.push_back(oneData.n[i]);
		}
	}
	//std::cout << "inputVecSize = " << inputVecSize << ' ' << inputVec.size() << std::endl;


	// mapminmax 'apply'
	map_minmax(APPLY, inputVecSize, &inputVec[0], inMin, inMax);

	/* calcualte neural network outputs.*/
	// input -> 1st hidden.
	float firstHiddenOutputs[20] = {};
	//std::cout << "First hidden layer outputs: " << std::endl;
	for(int neuronID = 0; neuronID < FirstHiddenSize; neuronID++){
		// calculate neurons in the 1st layer.
		float neuronInput = 0;
		for(int inputID = 0; inputID < inputVecSize; inputID++){
			neuronInput += inputVec[inputID] * IW[neuronID][inputID];
		}
		neuronInput += bias1[neuronID];
		firstHiddenOutputs[neuronID] = tansig_func(neuronInput);
		//std::cout << firstHiddenOutputs[neuronID] << ' ' ;
	}
	//std::cout << std::endl;

	// 1st hidden -> 2nd hidden.
	float secondHiddenOutputs[10] = {};
	//std::cout << "Second hidden layer outputs: " << std::endl;
	for(int neuronID = 0; neuronID < SecondHiddenSize; neuronID++){
		// calculate neurons in the 2nd layer.
		float neuronInput = 0;
		for(int firstID = 0; firstID < FirstHiddenSize; firstID++){
			neuronInput += firstHiddenOutputs[firstID] * LW1[neuronID][firstID];
		}
		neuronInput += bias2[neuronID];
		secondHiddenOutputs[neuronID] = tansig_func(neuronInput);
		//std::cout << secondHiddenOutputs[neuronID] << ' ' ;
	}
	//std::cout << std::endl;

	// 2nd hidden -> output.
	for(int neuronID = 0; neuronID < OutputSize; neuronID++){
		// calculate neurons in the output layer.
		float neuronInput = 0;
		for(int secondID = 0; secondID < SecondHiddenSize; secondID++){
			neuronInput += secondHiddenOutputs[secondID] * LW2[neuronID][secondID];
		}
		neuronInput += bias3[neuronID];
		rgb[neuronID] = neuronInput;
	}


	// mapminmax 'reverse'
	map_minmax(REVERSE, OutputSize, &rgb[0], outMin, outMax);

	//std::cout << "Eval_rgb= " << rgb << "  Real_rgb=" << oneData.rgb << std::endl;
	
	return rgb;
}