#include "stdafx.h"
// RealtimeGI.cpp : 定义控制台应用程序的入口点。
//
//
//#include "stdafx.h"
//#include "renderEngine.h"
//#include "Train.h"
//#include "Test.h"
//#include "pathTracer.h"
//
//using namespace LFJ;
//
//int _tmain(int argc, _TCHAR* argv[])
//{
//	Train trainer;
//	trainer.engine.runConfig("data/Config.xml");
//	trainer.engine.scene.buildObjKDTrees();
//	trainer.getBRDFParameters();
//
//	int flag = 0;
//	std::cout << "Input: Train-0 / Test-1 / CompPT-2 / ConvertToMatlab-3 / Realtime Render-4 ";
//	std::cin >> flag;
//
//	if(flag == 0){
//		int dataNum;
//		std::cout << "Input train data numbers required to generate: ";
//		std::cin >> dataNum;
//		trainer.genTrainData(dataNum);
//	}
//	
//	if(flag == 1){
//		std::string filename;
//		std::cin >> filename;
//		trainer.testData(filename);
//	}
//
//	if(flag == 2){
//		trainer.engine.renderer->renderPixels();
//	}
// 
//	if(flag == 3){
//		std::string filename;
//		int dataNum;
//		std::cout << "Convert to Matlab, input filename & data number:";
//		std::cin >> filename >> dataNum;
//		trainer.convertToMatlab(filename, dataNum);
//	}
//
//	if(flag == 4){
//		Test test;
//		test.setTrainPtr(&trainer);
//		test.buildNetworks();
//		//test.debug();
//		static_cast<PathTracer*>(trainer.engine.renderer)->setTestPtr(&test);
//		trainer.engine.renderer->renderPixels();
//	}
//
//	return 0;
//}