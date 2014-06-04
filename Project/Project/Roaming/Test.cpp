#include "stdafx.h"
#include "Test.h"
#include "Train.h"

Test::Test(void)
{
}


Test::~Test(void)
{
}



void Test::buildNetworks(){

	std::cout << "build neural networks from weight files... " << std::endl;

	// total objs: 6 (box planes) + 1 (ball) + 1 (buddha).
	for(int objID = 0; objID < 8; objID ++){
		if(objID == 4){
			objNetworks.push_back(NeuralNetwork(4)); // an empty nn.
			objNetworks2.push_back(NeuralNetwork(4)); // an empty nn.
			continue;		// 4 is light.
		}

		std::string biasFile1 = "NeuralNetworkWeightFiles\\" + std::to_string(objID) + "B1.txt";
		std::string biasFile2 = "NeuralNetworkWeightFiles\\" + std::to_string(objID) + "B2.txt";
		std::string biasFile3 = "NeuralNetworkWeightFiles\\" + std::to_string(objID) + "B3.txt";
		std::string IWFile = "NeuralNetworkWeightFiles\\" + std::to_string(objID) + "IW.txt";
		std::string LWFile1 = "NeuralNetworkWeightFiles\\" + std::to_string(objID) + "LW1.txt";
		std::string LWFile2 = "NeuralNetworkWeightFiles\\" + std::to_string(objID) + "LW2.txt";
		std::string MinMaxFile = "NeuralNetworkWeightFiles\\" + std::to_string(objID) + "_minmax.txt";


		NeuralNetwork network(objID);
		network.loadWeights(biasFile1, biasFile2, biasFile3, IWFile, LWFile1, LWFile2, MinMaxFile);
		objNetworks.push_back(network);



		std::string biasFile1_2 = "NeuralNetworkWeightFiles2\\" + std::to_string(objID) + "B1.txt";
		std::string biasFile2_2 = "NeuralNetworkWeightFiles2\\" + std::to_string(objID) + "B2.txt";
		std::string biasFile3_2 = "NeuralNetworkWeightFiles2\\" + std::to_string(objID) + "B3.txt";
		std::string IWFile_2 = "NeuralNetworkWeightFiles2\\" + std::to_string(objID) + "IW.txt";
		std::string LWFile1_2 = "NeuralNetworkWeightFiles2\\" + std::to_string(objID) + "LW1.txt";
		std::string LWFile2_2 = "NeuralNetworkWeightFiles2\\" + std::to_string(objID) + "LW2.txt";
		std::string MinMaxFile_2 = "NeuralNetworkWeightFiles2\\" + std::to_string(objID) + "_minmax.txt";
		NeuralNetwork network2(objID);
		network2.loadWeights(biasFile1_2, biasFile2_2, biasFile3_2, IWFile_2, LWFile1_2, LWFile2_2, MinMaxFile_2);
		objNetworks2.push_back(network2);

	}

	std::cout << "build nerual networks from weight files done. " << std::endl;
	return ;
}

vec3f Test::predict(const Train::TrainData &oneData, int objID){
	return objNetworks[objID].predict(oneData);
}

void Test::debug(){
	for(;;){
		int objID;
		Train::TrainData oneDat;// = train->genOneTrainData(objID);
		//0.476318 0.68689 -0.567737 -0.320607 -0.61593 -0.719612 -0.0363887 0.438878 0.89781 1 1 1 1 0.438401 0.362312 0.346491
		oneDat.x_p = vec3f(0.476318, 0.68689, -0.567737);
		oneDat.v = vec3f(-0.320607, -0.61593, -0.719612);
		oneDat.n = vec3f(-0.0363887, 0.438878, 0.89781);
		oneDat.rgb = vec3f(0.438401, 0.362312, 0.346491);
		objID = 0;
		vec3f rgb_predict = predict(oneDat, objID);
	}
	return ;
}

