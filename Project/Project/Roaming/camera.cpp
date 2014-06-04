#include "stdafx.h"
#include "camera.h"
#include "scene.h"

namespace LFJ{
	Ray Camera::generateRay(uint pixelID) const{
		vec3f front = mFocus - mPosition;
		front.normalize();
		vec3f right = front.cross(mUp);
		right.normalize();
		vec3f top = right.cross(front);

		Ray ray;
		uint width = mResolution.x, height = mResolution.y;
		uint x = pixelID % width;
		uint y = pixelID / width;

		float u = mRng->genFloat();
		float v = mRng->genFloat();

		ray.direction = front * mImagePlaneDist +
			right * (x + u - (width/2.f)) +
			top * ((height/2.f) - y - v);
		ray.direction.normalize();
		ray.origin = mPosition;
		ray.radiance = vec3f(1,1,1);
		ray.directionProb = 1.0;   //  1.0 / getPixelSolidAngle(pixelID); 

		Scene::ObjSourceInfo info;
		ray.intersectDist = scene->intersect(ray, info);
		ray.intersectObj = ray.intersectDist>=0 ? scene->objects[info.objID] : NULL;
		ray.intesrsectObjID = info.objID;
		ray.intersectTriangleID = info.triID;

		ray.contactObj = (AbstractObject*)this;
		ray.insideObj = scene->findInsideObject(ray, NULL);
		ray.isDeltaDirection = false;
		ray.pixelID = pixelID;


		return ray;
	}
	std::vector<vec3f> Camera::generateRays() const{
		uint width = mResolution.x, height = mResolution.y;
		std::vector<vec3f> rays(width * height);
		vec3f front = mFocus - mPosition;
		front.normalize();
		vec3f right = front.cross(mUp);
		right.normalize();
		vec3f top = right.cross(front);

		std::cout << "generateRays " << front << right << top << std::endl;

		for(uint y = 0; y < height; y++){
			for(uint x = 0; x < width; x++){
				rays[y*width + x] = front * mImagePlaneDist + 
					right * (x + 0.5 - (width/2.0)) +
					top * ((height/2.0) - 0.5 - y);
				rays[y*width + x].normalize();
			}
		}

		return rays;
	}
	vec3f Camera::fixVignetting(const vec3f &color, uint pixelID) const{
		// now only fix cosine term caused by eyeRay[0]
		uint width = mResolution.x, height = mResolution.y;
		vec3f front = mFocus - mPosition;
		front.normalize();
		vec3f right = front.cross(mUp);
		right.normalize();
		vec3f top = right.cross(front);
		uint x = pixelID % width, y = pixelID / width;
		vec3f dir =	front * mImagePlaneDist + 
					right * (x + 0.5 - (width/2.0)) +
					top * ((height/2.0) - 0.5 - y);
		dir.normalize();
		if(dir.dot(front) == 0)
			return 0.0;
		vec3f result = color / fabs(dir.dot(front));
		return isLegalColor(result) ? result : vec3f(0,0,0);
	}

	float Camera::getPixelSolidAngle(uint pixelID) const{
		uint width = mResolution.x, height = mResolution.y;
		uint x = pixelID % width, y = pixelID / width;
		float xc = x + 0.5 -(width/2.0), yc = (height/2.0) - 0.5 - y;
		float pixelDist = sqrt(xc*xc + yc*yc + mImagePlaneDist*mImagePlaneDist);
		return mImagePlaneDist / powf(pixelDist,3);
	}

	float Camera::evalDirectionProbability(const Ray &inRay, const Ray &outRay) const{
		return 1.0 / getPixelSolidAngle(outRay.pixelID);
	}

	vec2f Camera::convToRaster(const vec3f &p) const{
		vec2f pixel;
		vec3f v = p - mPosition;
		uint width = mResolution.x, height = mResolution.y;
		vec3f front = mFocus - mPosition;
		front.normalize();
		vec3f right = front.cross(mUp);
		right.normalize();
		vec3f top = right.cross(front);
		float vFrontDist = v.dot(front);
		float scale = mImagePlaneDist / vFrontDist;
		pixel.x = width/2.0 + v.dot(right) * scale;
		pixel.y = height/2.0 - v.dot(top) * scale;
		return pixel;
	}

	bool Camera::checkPixelBound(vec2f coord) const{
		int width = mResolution.x, height = mResolution.y;
		int pX = coord.x, pY = coord.y;
		if(pX >= 0 && pY >= 0 && pX < width && pY < height)
			return true;
		return false;
	}

	vec3f Camera::getWorldNormal(uint fi, const vec3f &pos, bool flat) const{
		vec3f front = mFocus - pos;
		front.normalize();
		return front;
	}

}