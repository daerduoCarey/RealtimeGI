#include "stdafx.h"
#include "Train.h"
#include "uniformSphereSampler.h"
#include "noself.h"
#include "glossyObj.h"
#include "diffuseObj.h"
#include <cstdlib>
#include <time.h>

#include "pathTracer.h"

#include "3rdparty\opencv\include\opencv\cv.h"
#include "3rdparty\opencv\include\opencv\highgui.h"

Train::Train(void)
{
	// hard code 
	BBox.p_min = vec3f(-1,-1,-1);
	BBox.p_max = vec3f(1,1,1);
	Nv = DATA_NUM;
	Nl = DATA_NUM;
	Nr = SPP_RAYS;

	omp_init_lock(&dataPrintLock);
}


Train::~Train(void)
{
}

void Train::genTrainData(int dataNum){
	std::ofstream fout;
	std::string fName = "Train" + std::to_string(clock()) + ".dat";
	fout.open(fName.c_str());

#pragma omp parallel for
	for(int i = 0; i < dataNum; i++){
		genOneTrainData(fout);
		std::cout << "[Train Data ID = " << i << " done.] "<< std::endl;
	}
	fout.close();
	return ;
}

vec3f Train::genRandomPoint(){
	while(true){
		vec3f random3 = engine.rng->genVec3f();
		vec3f position;
		for(int i = 0; i < 3; i++){
			position[i] = -1 + 2 * random3[i];
		}
		Ray ray;
		ray.origin = position;
		ray.direction = vec3f(0,1,0);
		if(!engine.scene.findInsideObject(ray)){
			return position;
		}
	}
}

// one data contains 1-lightpos & 1-viewpos & 6000-viewdirs.
void Train::genOneTrainData(std::ofstream &fout){
	const vec3f viewPos = genRandomPoint();	 // 1-viewPos
	{
		for(int nr = 0; nr < Nr; nr++){		
			Train::TrainData oneDat;

			Ray ray;
			ray.origin = viewPos;

			UniformSphereSampler sampler;
			vec3f direction = sampler.sample(*engine.rng, vec3f());
			ray.direction = direction;
			oneDat.v = direction;


			NoSelfCondition condition(&engine.scene, ray);
			Scene::ObjSourceInfo info;
			float d = engine.scene.intersect(ray, info, &condition);
			if(d > 0){
				oneDat.x_p = ray.origin + ray.direction * d;
				oneDat.n = engine.scene.objects[info.objID]->getWorldNormal(info.triID, oneDat.x_p);
				if(engine.scene.objects[info.objID]->glossy()){
				//	std::cout << "Hit glossy objID = " << info.objID << std::endl;
					GlossyObject *obj = (GlossyObject*)engine.scene.objects[info.objID];
					IsotropicPhong *phong = (IsotropicPhong*)(obj->bsdf);
					oneDat.brdf_p = scaleBRDF(vec4f(phong->reflectanceAlbedo, phong->power));
				}
				else if(engine.scene.objects[info.objID]->isEmissive()){
				//	std::cout << "Hit emissive objID = " << info.objID << std::endl;
					continue;
				}
				else{
				//	std::cout << "Hit diffuse objID = " << info.objID << std::endl;
					DiffuseObject *obj = (DiffuseObject*)engine.scene.objects[info.objID];
					DiffuseBSDF *diffuse = (DiffuseBSDF*)(obj->bsdf);
					oneDat.brdf_p = scaleBRDF(vec4f(diffuse->reflectanceAlbedo, 0));
				}

				Camera &cam = engine.scene.getCamera();
				cam.reSetup(viewPos, direction);

				Ray eyeRay = ray;
				eyeRay.radiance = vec3f(1,1,1);
				eyeRay.directionProb = 1.f;
				eyeRay.originProb = 1.f;
				eyeRay.intersectDist = d;
				eyeRay.intersectTriangleID = info.triID;
				eyeRay.intersectObj = engine.scene.objects[info.objID];
				eyeRay.contactObj = &cam;
				eyeRay.insideObj = NULL;
				eyeRay.isDeltaDirection = false;

				PathTracer *pathTracer = (PathTracer*)engine.renderer;
				//BidirectionalPathTracer *BidirPathTracer = (BidirectionalPathTracer*)engine.renderer;
				vec3f shadedColor = /*BidirP*/pathTracer->shadeOneSpp(eyeRay);
				oneDat.rgb = shadedColor;

				//// runs here, one data sample is generated, we will write it into the file disk.
				//omp_set_lock(&dataPrintLock);
				//fout << "ObjID:" << info.objID << "\tLabel:" << oneDat.rgb << "\tAttr:" << oneDat.x_p << "\t" << oneDat.v << "\t" << oneDat.n << "\t" << viewPos << 
				//	"\t[" << oneDat.brdf_p.x << ',' << oneDat.brdf_p.y << ',' << oneDat.brdf_p.z << ',' << oneDat.brdf_p.w << "]" << std::endl;
				////std::cout << "Label: " << oneDat.rgb << " Attr: " << oneDat.x_p << " " << oneDat.v << " " << oneDat.n << 
				////	" [" << oneDat.brdf_p.x << ',' << oneDat.brdf_p.y << ',' << oneDat.brdf_p.z << ',' << oneDat.brdf_p.w << "]" << std::endl;
				//omp_unset_lock(&dataPrintLock);


				
			}
		}
	}
	return ;
}

// one data contains 1-lightpos & 1-viewpos & 6000-viewdirs.
Train::TrainData Train::genOneTrainData(int &objID){
	const vec3f viewPos = genRandomPoint();	 // 1-viewPos
	{
		for(int nr = 0; nr < Nr; nr++){		
			Train::TrainData oneDat;

			Ray ray;
			ray.origin = viewPos;

			UniformSphereSampler sampler;
			vec3f direction = sampler.sample(*engine.rng, vec3f());
			ray.direction = direction;
			oneDat.v = direction;


			NoSelfCondition condition(&engine.scene, ray);
			Scene::ObjSourceInfo info;
			float d = engine.scene.intersect(ray, info, &condition);
			if(d > 0){
				oneDat.x_p = ray.origin + ray.direction * d;
				oneDat.n = engine.scene.objects[info.objID]->getWorldNormal(info.triID, oneDat.x_p);
				if(engine.scene.objects[info.objID]->glossy()){
				//	std::cout << "Hit glossy objID = " << info.objID << std::endl;
					GlossyObject *obj = (GlossyObject*)engine.scene.objects[info.objID];
					IsotropicPhong *phong = (IsotropicPhong*)(obj->bsdf);
					oneDat.brdf_p = scaleBRDF(vec4f(phong->reflectanceAlbedo, phong->power));
				}
				else if(engine.scene.objects[info.objID]->isEmissive()){
				//	std::cout << "Hit emissive objID = " << info.objID << std::endl;
					continue;
				}
				else{
				//	std::cout << "Hit diffuse objID = " << info.objID << std::endl;
					DiffuseObject *obj = (DiffuseObject*)engine.scene.objects[info.objID];
					DiffuseBSDF *diffuse = (DiffuseBSDF*)(obj->bsdf);
					oneDat.brdf_p = scaleBRDF(vec4f(diffuse->reflectanceAlbedo, 0));
				}

				Camera &cam = engine.scene.getCamera();
				cam.reSetup(viewPos, direction);

				Ray eyeRay = ray;
				eyeRay.radiance = vec3f(1,1,1);
				eyeRay.directionProb = 1.f;
				eyeRay.originProb = 1.f;
				eyeRay.intersectDist = d;
				eyeRay.intersectTriangleID = info.triID;
				eyeRay.intersectObj = engine.scene.objects[info.objID];
				eyeRay.contactObj = &cam;
				eyeRay.insideObj = NULL;
				eyeRay.isDeltaDirection = false;

				PathTracer *pathTracer = (PathTracer*)engine.renderer;
				//BidirectionalPathTracer *BidirPathTracer = (BidirectionalPathTracer*)engine.renderer;
				vec3f shadedColor = /*BidirP*/pathTracer->shadeOneSpp(eyeRay);
				oneDat.rgb = shadedColor;

				objID = info.objID;

				return oneDat;
			}
		}
	}
}



void Train::getBRDFParameters(){

	max4 = vec4f(std::numeric_limits<float>::min()), 
		min4 = vec4f(std::numeric_limits<float>::max());

	// obtain brdf scale factor.
	for(int i = 0; i < engine.scene.objects.size(); i++){
		if(engine.scene.objects[i]->isEmissive()){
			continue;
		}
		if(engine.scene.objects[i]->glossy()){
			GlossyObject *obj = (GlossyObject*)engine.scene.objects[i];
			IsotropicPhong *phong = (IsotropicPhong*)(obj->bsdf);
			vec4f brdfP = vec4f(phong->reflectanceAlbedo, phong->power);
			for(int j = 0; j < 4; j++){
				max4[j] = std::max(max4[j], brdfP[j]);
				min4[j] = std::min(min4[j], brdfP[j]);
			}
		}
		else{
			DiffuseObject *obj = (DiffuseObject*)engine.scene.objects[i];
			DiffuseBSDF *diffuse = (DiffuseBSDF*)(obj->bsdf);
			vec4f brdfP = vec4f(diffuse->reflectanceAlbedo, 0);
			for(int j = 0; j< 4; j++){
				max4[j] = std::max(max4[j], brdfP[j]);
				min4[j] = std::min(min4[j], brdfP[j]);
			}
		}
	}

	for(int i = 0; i < 4; i++){
		std::cout << "max4 " << max4[i] << " min4 " << min4[i] << std::endl;
	}
}


std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}


Train::TrainData Train::parseOneLineData(const std::string &data){
	Train::TrainData oneData;
	std::vector<std::string> splited = split(data, '\t');
	
	// rgb
	std::vector<std::string> rgbStr = split(split(split(splited[1], '(')[1], ')')[0], ',');
	for(int i = 0; i < 3; i++){
		oneData.rgb[i] = atof(rgbStr[i].c_str());
	}
	
	// x_p
	std::vector<std::string> xpStr = split(split(split(splited[2], '(')[1], ')')[0], ',');
	for(int i = 0; i < 3; i++){
		oneData.x_p[i] = atof(xpStr[i].c_str());
	}

	// v
	std::vector<std::string> vStr = split(split(split(splited[3], '(')[1],')')[0], ',');
	for(int i = 0; i < 3; i++){
		oneData.v[i] = atof(vStr[i].c_str());
	}

	// v
	std::vector<std::string> nStr = split(split(split(splited[4], '(')[1],')')[0], ',');
	for(int i = 0; i < 3; i++){
		oneData.n[i] = atof(nStr[i].c_str());
	}

	// viewPos
	vec3f _viewPos;
	std::vector<std::string> vpStr = split(split(split(splited[5], '(')[1],')')[0], ',');
	for(int i = 0; i < 3; i++){
		 _viewPos[i] = atof(vpStr[i].c_str());
	}

	// brdf_p
	std::vector<std::string> brdfStr = split(split(split(splited[6], '[')[1],']')[0], ',');
	for(int i = 0; i < 4; i++){
		oneData.brdf_p[i] = atof(brdfStr[i].c_str());
	}

	return oneData;
}

void Train::testData(const std::string filename){
	std::ifstream fin(filename.c_str());
	std::string data;

	while(getline(fin, data)){
		parseOneLineData(data);
	}

	return ;
}

void Train::convertToMatlab(const std::string &fName, int dataNum){
	std::ifstream fin(fName.c_str());
	std::string fName2 = fName + "_Matlab.dat";
	
	std::ofstream fout(fName2.c_str());

	std::string data;
	
	const int InputDim = 13, OutputDim = 3;

	fout << dataNum << ' ' << InputDim << ' ' << OutputDim << std::endl;

	for(int num = 0; num < dataNum; num++){
		if(!getline(fin, data)){
			break;
		}
		Train::TrainData oneData = parseOneLineData(data);
		fout << oneData.x_p.x << ' ' << oneData.x_p.y << ' ' << oneData.x_p.z << ' ' 
			<< oneData.v.x << ' ' << oneData.v.y << ' ' << oneData.v.z << ' ' 
			<< oneData.n.x << ' ' << oneData.n.y << ' ' << oneData.n.z << ' ' 
			<< oneData.brdf_p.x << ' ' << oneData.brdf_p.y << ' ' << oneData.brdf_p.z << ' ' << oneData.brdf_p.w << ' ' 
			<< oneData.rgb.x << ' ' << oneData.rgb.y << ' ' << oneData.rgb.z << std::endl;
	}

	fin.close();	
	fout.close();
	return ;
}

