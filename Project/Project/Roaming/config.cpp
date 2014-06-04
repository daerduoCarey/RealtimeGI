#include "stdafx.h"
#include "config.h"
#include "util.h"
#include "renderEngine.h"
#include "diffuseObj.h"
#include "emissiveObj.h"
#include "glossyObj.h"
#include "mcRenderer.h"

#include "pathTracer.h"

namespace LFJ{
	AbstractObject* Config::generateSceneObject(const std::string &path, xml_node<>* nodeObj, xml_node<>* nodeMat, Mesh* shape){
		if(strcmp(nodeMat->first_node("type")->value(), "Diffuse") == 0)
		{
			DiffuseObject *obj = new DiffuseObject(&engine->scene);
			obj->setRng(engine->rng);
			if(nodeObj->first_node("transform"))
				obj->transform = (readMatrix(nodeObj->first_node("transform")->value()));
			if(shape){
				obj->copyMesh(*shape);
			}
			obj->setColor(readVec(nodeMat->first_node("color")->value()));
			std::cout << "Diffuse Obj " << std::endl;

			if(!shape)
				obj->loadShape(path);
			std::cout << "BBox = " << (vec3f(obj->transform * vec4f(obj->minCoord, 1))) << " " << (vec3f(obj->transform * vec4f(obj->maxCoord, 1))) << std::endl;
			return obj;
		}
		if(strcmp(nodeMat->first_node("type")->value(), "Emissive") == 0)
		{
			EmissiveObject *obj = new EmissiveObject(&engine->scene);
			obj->setRng(engine->rng);
			if(nodeObj->first_node("transform"))
				obj->transform = (readMatrix(nodeObj->first_node("transform")->value()));
			obj->setColor(readVec(nodeMat->first_node("color")->value()));
			if(shape){
				obj->copyMesh(*shape);
			}
			std::cout << "Emissive Obj " << std::endl;
			if(!shape)
				obj->loadShape(path);
			std::cout << "BBox = " << (vec3f(obj->transform * vec4f(obj->minCoord, 1))) << " " << (vec3f(obj->transform * vec4f(obj->maxCoord, 1))) << std::endl;
			return obj;
		}
		if(strcmp(nodeMat->first_node("type")->value(), "Glossy") == 0)
		{
			GlossyObject *obj = new GlossyObject(&engine->scene);
			obj->setRng(engine->rng);
			if(nodeObj->first_node("transform"))
				obj->transform = (readMatrix(nodeObj->first_node("transform")->value()));
			if(shape)
				obj->copyMesh(*shape);
			obj->setColor(readVec(nodeMat->first_node("color")->value()));
			obj->setPower(atof(nodeMat->first_node("power")->value()));
			std::cout << "Glossy Obj " << std::endl;
			if(!shape)
				obj->loadShape(path);
			std::cout << "BBox = " << (vec3f(obj->transform * vec4f(obj->minCoord, 1))) << " " << (vec3f(obj->transform * vec4f(obj->maxCoord, 1))) << std::endl;
			return obj;
		}
		std::cerr << "Unknown type obj" << std::endl;
		return NULL;
	}


	void Config::load(const string &configFilePath){
		clear();

		pair<string, string> path_name;

		unsigned pos1 = configFilePath.rfind('/');
		unsigned pos2 = configFilePath.rfind('\\');
		unsigned pos = pos1 < pos2 ? pos1 : pos2;
		rootPath = configFilePath.substr(0, pos);
		xml_node<> *nodeConfig = findNode(configFilePath, "Config", "");

		xml_node<> *nodeSceneConfig = nodeConfig->first_node("Scene");

		path_name = getPathAndName(nodeSceneConfig);
		xml_node<> *nodeScene = findNode(path_name.first, "Scene", path_name.second);
		if(!nodeScene)
			nodeScene = findNode(configFilePath, "Scene", path_name.second);
		engine->scene.name = path_name.second;
		currentPath = path_name.first;

		for(xml_node<>* nodeGroupObj = nodeScene->first_node("GroupObject"); nodeGroupObj; nodeGroupObj = nodeGroupObj->next_sibling("GroupObject"))
		{
			std::cout << "reading group object..." << std::endl;
			string path = nodeGroupObj->first_node("filePath")->value();

			if(_access(path.c_str(), 0))
			{
				path = rootPath + '/' + path;
			}

			Mesh ss;
			if(nodeGroupObj->first_node("transform")){
				ss.setTransform(readMatrix(nodeGroupObj->first_node("transform")->value()));
			}
			vector<Mesh*> shapes;
			ss.loadShape(path, true, &shapes);
			unordered_map<string, Mesh*> name_shape;

			for(unsigned i=0; i<shapes.size(); i++){
				std::cout << "group obj name: " << shapes[i]->name << std::endl;
				name_shape[shapes[i]->name] = shapes[i];
			}

			for(xml_node<>* nodeObj = nodeGroupObj->first_node("Object"); nodeObj; nodeObj = nodeObj->next_sibling("Object"))
			{
				path_name = getPathAndName(nodeObj->first_node("Material"));
				xml_node<>* nodeMat = findNode(path_name.first, "Material", path_name.second);
				Mesh *shape = name_shape[nodeObj->first_attribute("Name")->value()];
				AbstractObject *obj = generateSceneObject(path, nodeObj, nodeMat, shape);
				engine->scene.objects.push_back(obj);
			}
			for(unsigned i=0; i<shapes.size(); i++)
				delete shapes[i];
		}

		for(xml_node<>* nodeObj = nodeScene->first_node("Object"); nodeObj; nodeObj = nodeObj->next_sibling("Object"))
		{
			path_name = getPathAndName(nodeObj->first_node("Material"));
			xml_node<>* nodeMat = findNode(path_name.first, "Material", path_name.second);
			string path = nodeObj->first_node("filePath")->value();
			if(_access(path.c_str(), 0))
			{
				path = rootPath + '/' + path;
			}

			AbstractObject *obj = generateSceneObject(path, nodeObj, nodeMat);
			/*obj->loadShape(path);*/
			engine->scene.objects.push_back(obj);
		}

		xml_node<> *nodeCam = findNode(nodeScene, "Camera", nodeSceneConfig->first_node("camera")->value());
		std::cout << "config camera:" << std::endl;
		engine->scene.mCamera.setup(
			readVec(nodeCam->first_node("position")->value()),
			readVec(nodeCam->first_node("focus")->value()),
			readVec(nodeCam->first_node("up")->value()),
			vec2f(atof(nodeCam->first_node("width")->value()), atof(nodeCam->first_node("height")->value())),
			atof(nodeCam->first_node("sightDist")->value()),
			engine->rng
			);

		engine->scene.buildKDTree();
		std::cout << "build Scene KDTree done " << std::endl;
		if(engine->renderer)
		{
			delete engine->renderer;
			engine->renderer = NULL;
		}
		

		if(nodeConfig->first_node("renderer"))
		{
			string typeName = nodeConfig->first_node("renderer")->value();
			
			if(typeName == "PT")
			{
				std::cout << "PathTracer " << std::endl;
				int spp = 10; // TODO
				if(nodeConfig->first_node("iter"))
					spp = atof(nodeConfig->first_node("iter")->value());
				int maxlen = 7; // TODO
				engine->renderer = new PathTracer(engine, spp, maxlen);
				engine->renderer->name = "PathTracer";
			}
			
		}
		
	}


	char* Config::textFileRead(const char *fn) {
		FILE *fp;
		char *content = NULL;
		int count=0;
		if (fn != NULL) {
			fopen_s(&fp, fn, "rt");
			if (fp != NULL) {
				fseek(fp, 0, SEEK_END);
				count = ftell(fp);
				rewind(fp);
				if (count > 0) {
					content = (char *)malloc(sizeof(char) * (count+1));
					count = fread(content,sizeof(char),count,fp);
					content[count] = '\0';
				}
				fclose(fp);
			}
		}
		return content;
	}
	int Config::textFileWrite(const char *fn, const char *s) {
		FILE *fp;
		int status = 0;
		if (fn != NULL) {
			fopen_s(&fp, fn,"w");
			if (fp != NULL) {
				if (fwrite(s,sizeof(char),strlen(s),fp) == strlen(s))
					status = 1;
				fclose(fp);
			}
		}
		return(status);
	}
	mat4f Config::readMatrix(const char *value) const{
		float v[16];
		sscanf_s(value, "[%f, %f, %f, %f][%f, %f, %f, %f][%f, %f, %f, %f][%f, %f, %f, %f]",
			v, v+1, v+2, v+3,
			v+4,v+5, v+6, v+7,
			v+8, v+9, v+10, v+11,
			v+12, v+13, v+14, v+15);
		mat4f mat;
		mat.set_value(v);
		return transpose(mat);
	}
	vec3f Config::readVec(const char *value) const{
		vec3f vec;
		sscanf_s(value, "(%f, %f, %f)", &vec.x, &vec.y, &vec.z);
		return vec;
	}
	xml_node<>* Config::findNode(const string& filePath, const string& nodeTag, const string& nodeName){
		if(filePath == "")
			return NULL;
		string fullFilePath = filePath;
		if(_access(filePath.c_str(), 0))
			fullFilePath = rootPath + string("/") + filePath;
		xml_document<> *doc;
		if(path_doc.find(fullFilePath) != path_doc.end())
		{
			doc = path_doc[fullFilePath].first;
		}
		else
		{
			doc = new xml_document<>;
			char* text = textFileRead(fullFilePath.c_str());
			path_doc[fullFilePath] = make_pair(doc, text);
			doc->parse<0>(text);
		}
		for(xml_node<> *node=doc->first_node(nodeTag.c_str()); node; node = node->next_sibling(nodeTag.c_str()))
		{
			if(node->first_attribute("Name") == NULL || nodeName == "")
				return node;
			if(node->first_attribute("Name") && node->first_attribute("Name")->value() == nodeName)
				return node;
		}
		return NULL;
	}
	xml_node<>* Config::findNode(xml_node<>* root, const string& nodeTag, const string& nodeName){
		for(xml_node<> *node=root->first_node(nodeTag.c_str()); node; node = node->next_sibling(nodeTag.c_str())){
			if(node->first_attribute("Name") == NULL || nodeName == "")
				return node;
			if(node->first_attribute("Name") && node->first_attribute("Name")->value() == nodeName)
				return node;
		}
		return NULL;
	}
	pair<string, string> Config::getPathAndName(xml_node<>* node){
		pair<string, string> path_name;
		path_name.first = node->first_node("path") ? node->first_node("path")->value() : currentPath;
		path_name.second = node->first_node("name") ? node->first_node("name")->value() : "";
		return path_name;
	}
	void Config::clear(){
		for(unordered_map<string, pair<xml_document<>*, char*>>::iterator it=path_doc.begin(); it!=path_doc.end(); it++){
			if(it->second.first)
				delete it->second.first;
			if(it->second.second)
				free(it->second.second);
		}
		path_doc.clear();
	}
}