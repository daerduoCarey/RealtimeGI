// Roaming.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "renderEngine.h"
#include "Train.h"
#include "Test.h"
#include "pathTracer.h"
#include "Shader.h"
#include <GL/glut.h>
using namespace LFJ;

#define PI 3.1415926

int width = 512;
int height = 512;

float sd = 256;

float fovy = 90;

vec3f viewPos(0, 0, 3);
vec3f viewFront(0, 0, -1);
vec3f viewUp(0, 1, 0);

float speed = 0.04;//0.01;

float eye_phi = PI / 2;
float eye_theta =  -PI / 2;

bool keyDownBuffer[256];

//vector<float> testArr;


vector<float> normalArray[7];

vector<float> brdfArray[7];


inline matrix4<float> rotMat(const vec3f& axis, const float angle)
{
	float c = cos(angle);
	float s = sin(angle);
	float _c = 1 - c;
	float _s = 1 - s;
	float x = axis.x;
	float y = axis.y;
	float z = axis.z;
	return transpose(matrix4<float>(c+_c*x*x, _c*x*y-s*z, _c*x*z+s*y, 0,
		_c*x*y+s*z, c+_c*y*y, _c*y*z-s*x, 0,
		_c*x*z-s*y, _c*y*z+s*x, c+_c*z*z, 0,
		0, 0, 0, 1));
}

void display();

void reshape(GLsizei w,GLsizei h);

Shader shader;

void mouseMoveFunc(int x, int y);

void idleFunc();

void keyDown(unsigned char key, int x, int y);

void keyUp(unsigned char key, int x, int y);

void updateMotion();

void init();

void clear();

GLuint vertexBuffer, normalBuffer;

vector<vec3f> vertices;

vector<vec3f> normals;

vector<int> objsFirstVertexIDList;

Train trainer;

Test test;

int main(int argc,char ** argv)
{
	/*****************************************************************************/
	// load scene and networks.
	trainer.engine.runConfig("data/Config.xml");
	trainer.engine.scene.buildObjKDTrees();
	trainer.getBRDFParameters();

	test.setTrainPtr(&trainer);
	test.buildNetworks();
	std::cout << "[CPU]: objs loaded, networks built. " << std::endl;
	//static_cast<PathTracer*>(trainer.engine.renderer)->setTestPtr(&test);
	//trainer.engine.renderer->renderPixels();
	/*****************************************************************************/


	/*初始化*/
	fovy = atan(height / 2.0f / sd) * 180 / PI * 2;

	viewFront = vec3f(sin(eye_phi)*cos(eye_theta), cos(eye_phi), sin(eye_phi)*sin(eye_theta));
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(400, 400);

	/*创建窗口*/
	glutCreateWindow("Roaming");
	std::cout << "init running... " << std::endl;
	init();
	std::cout << "init done. " << std::endl;

	glutSetCursor(GLUT_CURSOR_NONE);
	glutWarpPointer(width/2, height/2);

	/*绘制与显示*/
	glClearColor(0.0,0.0,0.0,0.0);
	reshape(width, height);
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutPassiveMotionFunc(mouseMoveFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);
	glutMainLoop();
	clear();
	return 0;
}

void clear()
{
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &normalBuffer);
}




void init()
{
	/*****************************************************************************/
	// load objs: triangles and normals.
	std::cout << "init(): pushing objs into OpenGL... " << std::endl;
	for(int objID = 0; objID < trainer.engine.scene.objects.size(); objID++){
		AbstractObject *obj = trainer.engine.scene.objects[objID];
		matrix4<float> normalMat = transpose(inverse(obj->transform));
		
		objsFirstVertexIDList.push_back(vertices.size());

		for(int triID = 0; triID < obj->getTriangleNum(); triID++){
			vec3ui vertexIndexs = obj->faceVertexIndexList[triID];
			vec3f vertex0 = obj->transform * vec4f(obj->vertexList[vertexIndexs[0]], 1);
			vec3f vertex1 = obj->transform * vec4f(obj->vertexList[vertexIndexs[1]], 1);
			vec3f vertex2 = obj->transform * vec4f(obj->vertexList[vertexIndexs[2]], 1);
			
			vec3ui normalIndexs = obj->faceVertexNormalIndexList[triID];
			vec3f normal0 = normalMat * vec4f(obj->vertexNormalList[normalIndexs[0]], 0);
			vec3f normal1 = normalMat * vec4f(obj->vertexNormalList[normalIndexs[1]], 0);
			vec3f normal2 = normalMat * vec4f(obj->vertexNormalList[normalIndexs[2]], 0);
		
			vertices.push_back(vertex0);
			normals.push_back(normal0);
			vertices.push_back(vertex1);
			normals.push_back(normal1);
			vertices.push_back(vertex2);
			normals.push_back(normal2);
		}
	}
	std::cout << "init(): push done. " << std::endl;
	/*****************************************************************************/


	shader.init();
	shader.createProgram("nn", "Shader/nn.vert", "Shader/nn.frag");


	// transfer networks into Shader.
	std::cout << "init(): transfer networks into Shader... " << std::endl;

	// useNormal & wasteXpID
	for(int netID = 0; netID < test.objNetworks.size(); netID++){
		std::cout << "UseNormal&WasteXp(): netID = " << netID << std::endl;
		if(netID == 4)  continue;

		NeuralNetwork &net = test.objNetworks[netID];

		int arrIndex = netID > 4 ? netID - 1 : netID;
		// 0 -> use normal ?
		std::cout << "size: " << normalArray[arrIndex].size() << std::endl;
		normalArray[arrIndex].push_back(net.useNormal ? 1 : 0);
		// 1 -> waste Xp ID
		std::cout << "size: " << normalArray[arrIndex].size() << std::endl;
		normalArray[arrIndex].push_back(net.wasteXpID);
		
		// 2:21 -> bias1
		std::cout << "size: " << normalArray[arrIndex].size() << std::endl;
		for(int i = 0; i < 20; i++){
			normalArray[arrIndex].push_back(net.bias1[i]);
		}

		// 22:31 -> bias2
		std::cout << "size: " << normalArray[arrIndex].size() << std::endl;
		for(int i = 0; i < 10; i++){
			normalArray[arrIndex].push_back(net.bias2[i]);
		}

		// 32:34 -> bias3
		std::cout << "size: " << normalArray[arrIndex].size() << std::endl;
		for(int i = 0; i < 3; i++){
			normalArray[arrIndex].push_back(net.bias3[i]);
		}

		// 35:214 -> IW
		std::cout << "size: " << normalArray[arrIndex].size() << std::endl;
		for(int row = 0; row < 20; row++){
			for(int col = 0; col < 9; col++){
				normalArray[arrIndex].push_back(net.IW[row][col]);
			}
		}

		// 215:414 -> LW1
		std::cout << "size: " << normalArray[arrIndex].size() << std::endl;
		for(int row = 0; row < 10; row++){
			for(int col = 0; col < 20; col++){
				normalArray[arrIndex].push_back(net.LW1[row][col]);
			}
		}

		// 415:444 -> LW2
		std::cout << "size: " << normalArray[arrIndex].size() << std::endl;
		for(int row = 0; row < 3; row++){
			for(int col = 0; col < 10; col++){
				normalArray[arrIndex].push_back(net.LW2[row][col]);
			}
		}

		// 445:468 -> MinMax
		std::cout << "size: " << normalArray[arrIndex].size() << std::endl;
		for(int i = 0; i < 9; i++)
			normalArray[arrIndex].push_back(net.inMin[i]);
		for(int i = 0; i < 9; i++)
			normalArray[arrIndex].push_back(net.inMax[i]);
		for(int i = 0; i < 3; i++)
			normalArray[arrIndex].push_back(net.outMin[i]);
		for(int i = 0; i < 3; i++)
			normalArray[arrIndex].push_back(net.outMax[i]);




		NeuralNetwork &net2 = test.objNetworks2[netID];

		// 469:488 -> bias1
		std::cout << "size: " << normalArray[arrIndex].size() << std::endl;
		for(int i = 0; i < 20; i++){
			normalArray[arrIndex].push_back(net2.bias1[i]);
		}

		// 489:498 -> bias2
		std::cout << "size: " << normalArray[arrIndex].size() << std::endl;
		for(int i = 0; i < 10; i++){
			normalArray[arrIndex].push_back(net2.bias2[i]);
		}

		// 499:501 -> bias3
		std::cout << "size: " << normalArray[arrIndex].size() << std::endl;
		for(int i = 0; i < 3; i++){
			normalArray[arrIndex].push_back(net2.bias3[i]);
		}

		// 502:681 -> IW
		std::cout << "size: " << normalArray[arrIndex].size() << std::endl;
		for(int row = 0; row < 20; row++){
			for(int col = 0; col < 9; col++){
				normalArray[arrIndex].push_back(net2.IW[row][col]);
			}
		}

		// 682:881 -> LW1
		std::cout << "size: " << normalArray[arrIndex].size() << std::endl;
		for(int row = 0; row < 10; row++){
			for(int col = 0; col < 20; col++){
				normalArray[arrIndex].push_back(net2.LW1[row][col]);
			}
		}

		// 882:911 -> LW2
		std::cout << "size: " << normalArray[arrIndex].size() << std::endl;
		for(int row = 0; row < 3; row++){
			for(int col = 0; col < 10; col++){
				normalArray[arrIndex].push_back(net2.LW2[row][col]);
			}
		}

		// 912:935 -> MinMax
		std::cout << "size: " << normalArray[arrIndex].size() << std::endl;
		for(int i = 0; i < 9; i++)
			normalArray[arrIndex].push_back(net2.inMin[i]);
		for(int i = 0; i < 9; i++)
			normalArray[arrIndex].push_back(net2.inMax[i]);
		for(int i = 0; i < 3; i++)
			normalArray[arrIndex].push_back(net2.outMin[i]);
		for(int i = 0; i < 3; i++)
			normalArray[arrIndex].push_back(net2.outMax[i]);







		std::string name = "normalArr[" + std::to_string(arrIndex) + "]";
		shader.setTexArray1D(name.c_str(), normalArray[arrIndex]);
	
	}



	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3f)*vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3f)*normals.size(), normals.data(), GL_STATIC_DRAW);

}

void display()
{
	shader.useProgram("nn");

	shader.setUniform("width", width);
	shader.setUniform("height", height);
	shader.setUniform("fovy", fovy);
	shader.setUniform("pos", viewPos);
	shader.setUniform("front", viewFront);
	shader.setUniform("up", viewUp);

	glMatrixMode(GL_MODELVIEW);//指明当前矩阵为GL_PROJECTION
	glLoadIdentity();//将当前矩阵置换为单位阵

	updateMotion();

	vec3f center = viewPos + viewFront;
	gluLookAt(viewPos.x, viewPos.y, viewPos.z, center.x, center.y, center.z, viewUp.x, viewUp.y, viewUp.z);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//buffer设置为颜色可写

	glEnable(GL_DEPTH_TEST);

	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	glEnableClientState(GL_VERTEX_ARRAY);

	glEnableClientState(GL_NORMAL_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glNormalPointer(GL_FLOAT, 0, NULL);

	//glDrawArrays(GL_TRIANGLES, 0, vertices.size());
 	for(int objID = 0; objID < trainer.engine.scene.objects.size(); objID++){
		shader.setUniform("objID", objID);
		int lastVertexIndex = objID == trainer.engine.scene.objects.size()-1 ?
			vertices.size() : objsFirstVertexIDList[objID+1];
		glDrawArrays(GL_TRIANGLES, objsFirstVertexIDList[objID], lastVertexIndex - objsFirstVertexIDList[objID]);
	}
	glPopClientAttrib();

	glFlush();//强制OpenGL函数在有限时间内运行

	glutPostRedisplay();
}

void reshape(GLsizei w,GLsizei h)
{
	glViewport(0,0,w,h);//设置视口

	width = w;
	height = h;

	glMatrixMode(GL_PROJECTION);//指明当前矩阵为GL_PROJECTION
	glLoadIdentity();//将当前矩阵置换为单位阵

	gluPerspective(fovy, width / float(height), 0.01, 1000);
}

void mouseMoveFunc(int x, int y)
{
	glutWarpPointer(width/2, height/2);

	vec2i delta(x - width/2, y - height/2);
	eye_theta += delta.x * 0.01;
	eye_phi += delta.y * 0.01;

	if(eye_theta > 2*PI)
		eye_theta -= 2*PI;
	if(eye_theta < 0)
		eye_theta += 2*PI;
	if(eye_phi < PI/12)
		eye_phi = PI/12;
	if(eye_phi > PI*11/12)
		eye_phi = PI*11/12;

	viewFront = vec3f(sin(eye_phi)*cos(eye_theta), cos(eye_phi), sin(eye_phi)*sin(eye_theta));
	vec3f axis = vec3f(0, 1, 0).cross(viewUp);
	axis.normalize();
	float angle = acos(viewUp.dot(vec3f(0, 1, 0)));
	viewFront = vec3f(rotMat(axis, angle)*vec4f(viewFront, 0));

	display();
}

void idleFunc()
{
	glutPostRedisplay();
}

void keyDown(unsigned char key, int x, int y)
{
//	std::cout << "Key = " << key << " viewPos = " << viewPos << std::endl;
	keyDownBuffer[key] = true;
	switch(key)
	{
	case 'q':
		exit(0);
		break;
	case 'r':
		shader.deleteProgram("nn");
		shader.createProgram("nn", "Shader/nn.vert", "Shader/nn.frag");
		break;
	}
}

void keyUp(unsigned char key, int x, int y)
{
	keyDownBuffer[key] = false;
}

void updateMotion()
{
	vec3f velocityDir(0, 0, 0);
	vec3f right = viewFront.cross(viewUp);
	right.normalize();

	if(keyDownBuffer['w'])
		velocityDir += viewFront;
	if(keyDownBuffer['s'])
		velocityDir -= viewFront;
	if(keyDownBuffer['a'])
		velocityDir -= right;
	if(keyDownBuffer['d'])
		velocityDir += right;

	if(keyDownBuffer['k']){
		viewPos -= vec3f(0,0.01,0);		return ;
	}
	if(keyDownBuffer['i']){
		viewPos += vec3f(0,0.01,0);		return ;
	}


	if(velocityDir.length() < 0.5)
		return;

	velocityDir.normalize();

	viewPos += velocityDir * speed;
}

