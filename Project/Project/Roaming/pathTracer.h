#include "mcRenderer.h"
#include <omp.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "Test.h"

namespace LFJ{
	class PathTracer : public MCRenderer{
		bool useNextEventEstimation;
	public:
		PathTracer(RenderEngine *engine, uint spp, uint maxLen) :
			MCRenderer(engine, spp, maxLen)
		{
			name = "PathTracer";
			useNextEventEstimation = false;
			prepareForLightSampling();
		}

		Test *testPtr;
		void setTestPtr(Test *test);
		
		std::vector<vec3f> renderPixels();
		vec3f shadeOneSpp(Ray &eyeRay);
		vec3f throughputByDirectIllumination(Ray &cameraRay, Ray &lightRay);
	};
}
