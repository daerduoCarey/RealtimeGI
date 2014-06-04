#include "stdafx.h"
#include "pathTracer.h"
#include "noself.h"
#include "renderEngine.h"
#include <limits>
#include "glossyObj.h"
#include "diffuseObj.h"
#include "emissiveObj.h"
#include "Train.h"



#include "3rdparty\opencv\include\opencv\cv.h"
#include "3rdparty\opencv\include\opencv\highgui.h"

namespace LFJ{
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

	void PathTracer::setTestPtr(Test *test){
		testPtr = test;
	}

	vec3f PathTracer::throughputByDirectIllumination(Ray &cameraRay, Ray &lightRay)
	{
		if(!cameraRay.intersectObj){
			return vec3f(0.f);
		}
		Ray eyeRay;
		eyeRay.contactObj = cameraRay.intersectObj;
		eyeRay.contactTriangleID = cameraRay.intersectTriangleID;
		eyeRay.insideObj = NULL;
		eyeRay.origin = cameraRay.origin + cameraRay.direction * cameraRay.intersectDist;
		eyeRay.direction = lightRay.origin - eyeRay.origin;	
		float dist = eyeRay.direction.length();
		eyeRay.direction.normalize();
		float cosTerm1 = eyeRay.cosineTerm();
		lightRay.direction = -eyeRay.direction;
		float cosTerm2 = lightRay.cosineTerm();

		bool visible = visibilityTest(eyeRay, lightRay);
		if(!visible){
			return vec3f(0.f);
		}

		Ray outRay = eyeRay;	outRay.direction = -cameraRay.direction;
		Ray inRay = eyeRay;		inRay.direction = -eyeRay.direction;
		vec3f alpha = vec3f(1,1,1);
		alpha *= cameraRay.cosineTerm() * inRay.evalBSDF(outRay) / lightRay.originProb * lightRay.radiance;
		alpha *= cosTerm1 * cosTerm2 / (dist * dist);
		return alpha;
	}

	std::vector<vec3f> PathTracer::renderPixels(){
		Camera &camera = getCamera();
		uint width = camera.mResolution.x, height = camera.mResolution.y;
		std::vector<vec3f> pixelColors(width * height, vec3f(0,0,0));
		std::vector<vec3f> pixelColorsDirect(width * height, vec3f(0,0,0));
		std::vector<vec3f> pixelColorsInDirect(width * height, vec3f(0,0,0));

		for(uint s = 0; s < spp; s++){
			std::cout << "Iter: " << s << std::endl;
#pragma omp parallel for
			for(int p = 0; p < pixelColors.size(); p++){
				Path eyePath;
				clock_t __start = clock();
				Ray eyeray = camera.generateRay(p);
//				std::cout << "Generate eyeray timeuse: " << clock() - __start << "ms. " << std::endl;
		//		samplePath(eyePath, eyeray);
				pixelColors[p] *= s / float(s+1);
				pixelColorsDirect[p] *= s / float(s+1);
				pixelColorsInDirect[p] *= s / float(s+1);

				vec3f color = vec3f(1,1,1), directIllumination, indirectIllumination;

				if(!eyeray.intersectObj){
					color = vec3f(0,0,0);
					directIllumination = indirectIllumination = vec3f(0.f);
				}
				else if(eyeray.intersectObj->isEmissive()){
					color = static_cast<EmissiveObject*>(eyeray.intersectObj)->color;
					directIllumination = color;
					indirectIllumination = vec3f(0.f);
				}
				else{
					Train::TrainData oneData;
					oneData.x_p = eyeray.origin + eyeray.intersectDist * eyeray.direction;
					oneData.v = eyeray.direction;
					oneData.n = eyeray.intersectObj->getWorldNormal(eyeray.intersectTriangleID, oneData.x_p);
					
					__start = clock();
					indirectIllumination = 
						testPtr->predict(oneData, eyeray.intesrsectObjID);

//					std::cout << "NN predict timeuse: " << clock() - __start << "ms. " << std::endl;

					__start = clock();
					directIllumination = 
						throughputByDirectIllumination(eyeray, genLightSample());
//					std::cout << "Direct illumination cal timeuse: " << clock() - __start << "ms. " << std::endl;

					color = directIllumination + indirectIllumination;
				}

		/*		{
					for(int i = 0; i < eyePath.size(); i++){
						if(i != eyePath.size() - 1){
							color *= i == 0 ? 1 : eyePath[i].cosineTerm();
							float dist = (eyePath[i+1].origin - eyePath[i].origin).length();
							color *= eyePath[i].radianceDecay(dist);
						}
						color *= eyePath[i].radiance / eyePath[i].originProb / eyePath[i].directionProb;
					}
				}
		*/
				pixelColors[p] += color / (s+1);
				pixelColorsDirect[p] += directIllumination / (s+1);
				pixelColorsInDirect[p] += indirectIllumination / (s+1);
			}

			camera.mFilm.setBuffer(pixelColors);
			std::string filename = engine->scene.name + "_NeuralNetwork_Global.pfm";//engine->renderer->name + engine->scene.name + ".pfm";
			camera.mFilm.savePFM(filename);

			camera.mFilm.setBuffer(pixelColorsDirect);
			filename = engine->scene.name + "_NeuralNetwork_Direct.pfm";//engine->renderer->name + engine->scene.name + ".pfm";
			camera.mFilm.savePFM(filename);

			camera.mFilm.setBuffer(pixelColorsInDirect);
			filename = engine->scene.name + "_NeuralNetwork_InDirect.pfm";//engine->renderer->name + engine->scene.name + ".pfm";
			camera.mFilm.savePFM(filename);

		}

		return pixelColors;
	}

	vec3f PathTracer::shadeOneSpp(Ray &eyeRay){
		Camera &camera = getCamera();
		vec3f sppColor = vec3f(0,0,0);

		int s = 0;
		while(s < spp){
			Path eyePath;
			samplePath(eyePath, eyeRay);

			vec3f color = vec3f(1,1,1);
			{
				for(int i = 0; i < eyePath.size(); i++){
					if(i != eyePath.size() - 1){
						color *= i == 0 ? 1 : eyePath[i].cosineTerm();
						float dist = (eyePath[i+1].origin - eyePath[i].origin).length();
						color *= eyePath[i].radianceDecay(dist);
					}
					color *= eyePath[i].radiance / eyePath[i].originProb / eyePath[i].directionProb;
				}
			}

			Ray lightRay = genLightSample();

			vec3f direct = throughputByDirectIllumination(eyeRay, lightRay);

			if(!isLegalColor(color-direct)){
				continue;
			}

			sppColor *= s / float(s+1);
			sppColor += (color-direct) / (s+1);

			s++;
		}
		return sppColor;
	}
}