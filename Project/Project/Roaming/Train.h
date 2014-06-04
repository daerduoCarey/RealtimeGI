#pragma once
#include "renderEngine.h"
using namespace LFJ;
#define DATA_NUM 200
#define SPP_RAYS 6000

class Train
{
public:
	Train(void);
	~Train(void);

	struct TrainData{
	// label:
		vec3f rgb;  // RGB color channels
	// attributes:
		vec3f x_p;  // hit surface point position
		vec3f v;	// view direction
		vec3f n;	// hit surface point normal	
	//	vec3f l;	position-located light for simplify
		vec4f brdf_p; // for diffuse->vec3f,  for glossy->vec3f + phong power 
	};

	vec4f min4, max4; // brdf parameters scale factor.
	vec4f scaleBRDF(vec4f brdfP){
		vec4f brdfRst;
		for(int i = 0; i < 4; i++){
			brdfRst[i] = -1 + 2 * (brdfP[i] - min4[i]) / (max4[i] - min4[i]);
		}
		return brdfRst;
	}

	struct SceneBBox{
		vec3f p_min, p_max;
	}BBox;

	omp_lock_t dataPrintLock;
	int Nr, Nv, Nl;
public:
	RenderEngine engine;
	
public:
	void genTrainData(int dataNum = DATA_NUM);
	
	void genOneTrainData(std::ofstream &fout);
	Train::TrainData genOneTrainData(int &objID);
	
	void testData(const std::string filename);
	vec3f genRandomPoint();
	void getBRDFParameters();
	TrainData parseOneLineData(const std::string &data);

	void convertToMatlab(const std::string &fName, int dataNum);
};

