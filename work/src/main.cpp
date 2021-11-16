//---------------------------------------------------------------------------
//
// Copyright (c) 2015 Taehyun Rhee, Joshua Scott, Ben Allen
//
// This software is provided 'as-is' for assignment of COMP308 in ECS,
// Victoria University of Wellington, without any express or implied warranty. 
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// The contents of this file may not be copied or duplicated in any form
// without the prior permission of its owner.
//
//----------------------------------------------------------------------------

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include "comp308.hpp"
#include "shaderLoader.hpp"
#include "geometry.hpp"
#include "physics.hpp"
#include "cut.hpp"
#include <map>
#include <random>
// #include <windows.h>
// #include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
using namespace std;
using namespace comp308;


// Global variables
// 
GLuint g_winWidth  = 640;
GLuint g_winHeight = 480;
GLuint g_mainWindow = 0;


// Projection values
// 
float g_fovy = 20.0;
float g_znear = 0.1;
float g_zfar = 1000;


// Mouse controlled Camera values
//
bool g_mouseDown = false;
vec2 g_mousePos;
bool g_drawMoved;
bool g_samurai;
float g_yRotation = 0;
float g_xRotation = 0;
float g_yPosition = -2;
float g_zoomFactor = 4;

// Mouse controlled drawing values
//
bool g_drawMouse = false;
vec3 cut_draw_1;
vec3 cut_draw_2;

// For cutting in three dimensions.
//
vec3 cut_proj_1;
vec3 cut_proj_2;
int g_start_time=0;
float g_delta=0;
vector<geometry> g_geometry;
vector<Rigidbody*> boxes;
cut *g_cut = nullptr;
Physics* physics;

// Utility variables
bool g_paused = false;
bool g_slow = false;
bool g_bounds = false;

geometry m_melon;
geometry m_log;
geometry m_cake;
int choice = 0;
int choiceShader = 0;

// SHADERS CODES.
GLuint melon_shader = 0;
GLuint cake_shader = 0;
GLuint wood_shader = 0;
//random number generator for spawning boxes
default_random_engine generator;
uniform_real_distribution<float> distribution(-1,1);
// Sets up where and what the light is
// Called once on start up
// 
void initLight() {
	float direction[]	  = {0.0f, 0.0f, 1.0f, 0.0f};
	float diffintensity[] = {0.7f, 0.7f, 0.7f, 1.0f};
	float ambient[]       = {0.2f, 0.2f, 0.2f, 1.0f};

	glLightfv(GL_LIGHT0, GL_POSITION, direction);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  diffintensity);
	glLightfv(GL_LIGHT0, GL_AMBIENT,  ambient);	
	
	
	glEnable(GL_LIGHT0);
}

// Sets up where the camera is in the scene
// Called every frame
//use glortho for putting a point in space for a clicked position
// 
void setUpCamera() {
	// Set up the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(g_fovy, float(g_winWidth) / float(g_winHeight), g_znear, g_zfar);

	// Set up the view part of the model view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Load camera transforms
	glTranslatef(0, g_yPosition, -5 * g_zoomFactor);
	glRotatef(g_xRotation, 1, 0, 0);
	glRotatef(g_yRotation, 0, 1, 0);
}

// Build shader program.
void initShader(string vert, string frag, GLuint* address) {
	*address = makeShaderProgram(vert, frag);
}

// Draw function
//
void draw() {
	g_delta = (glutGet(GLUT_ELAPSED_TIME) - g_start_time) /1000.0f;
	g_start_time = glutGet(GLUT_ELAPSED_TIME);
	if(g_paused || g_samurai){
		g_delta = 0;
	} if (g_slow) {
		g_delta *= 0.1;
	}
	// Set up camera every frame
	setUpCamera();

	// Black background
	glClearColor(0.2f,0.2f,0.9f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Enable flags for normal rendering
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);

	// Set the current material (for all objects) to red
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); 
	
	physics->update(g_delta, g_bounds);
	physics->checkCollisions(g_delta);
	
	
	// Render geometry
	// Enable Drawing texures
	glEnable(GL_TEXTURE_2D);
	// Set the location for binding the texture
	glActiveTexture(GL_TEXTURE0);
	// Bind the texture

	// Use the shader we made
	switch (choiceShader) {
		case 0:
			glUseProgram(melon_shader);
			glUniform1f(glGetUniformLocation(melon_shader, "radius"), 1.5);
			break;
		case 1:
			glUseProgram(cake_shader);
			glUniform1f(glGetUniformLocation(melon_shader, "radius"), 0.2);
			break;
		default:
			glUseProgram(wood_shader);
			glUniform1f(glGetUniformLocation(melon_shader, "radius"), 0.1);
			break;
	}
	for (unsigned int i=0;i<g_geometry.size(); i++) {

	    geometry Geometry = g_geometry[i];   
	    glPushMatrix();	
	    //get position from rigidbody corresponding to this geometry object
		Rigidbody* rigid = Geometry.getRigidbody();
		vec3 pos = rigid->position;
		glTranslatef(pos.x, pos.y, pos.z);
		glColor3f(1.0f, 0.0f, 0.0f);
		Geometry.render();
		glPopMatrix();
	}
	glUseProgram(0);
	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	for(Rigidbody * box : boxes){
	  glPushMatrix();
	   vec3 position = box->update(g_delta, g_bounds);
	   glTranslatef(position.x,position.y,position.z);
	   glColor3f(1,0,0);
	   glutSolidCube(1);
	  glPopMatrix();
	}
	glDisable(GL_LIGHTING);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1, 1, 1, 0.5);
	glBegin(GL_QUADS);
	glVertex3f(cut_draw_1.x, cut_draw_1.y, cut_draw_1.z);
	glVertex3f(cut_proj_1.x, cut_proj_1.y, cut_proj_1.z);
	glVertex3f(cut_proj_2.x, cut_proj_2.y, cut_proj_2.z);
	glVertex3f(cut_draw_2.x, cut_draw_2.y, cut_draw_2.z);
	glEnd();

	glBegin(GL_QUADS);
	glColor4f(1, 1, 1, 1);
	glVertex3f(10, 0, 10);
	glVertex3f(-10, 0, 10);
	glVertex3f(-10, 0, -10);
	glVertex3f(10, 0, -10);
	glEnd();

	glBegin(GL_QUADS);
	glColor4f(0.4, 1, 0.4, 1);
	glVertex3f(10000, -1, 10000);
	glVertex3f(-10000, -1, 10000);
	glVertex3f(-10000, -1, -10000);
	glVertex3f(10000, -1, -10000);
	glEnd();

	glDisable(GL_DEPTH_TEST);

	glColor4f(1, 0, 0, 1);
	glBegin(GL_LINES);
	glVertex3f(cut_draw_1.x, cut_draw_1.y, cut_draw_1.z);
	glVertex3f(cut_draw_2.x, cut_draw_2.y, cut_draw_2.z);
	glVertex3f(cut_proj_1.x, cut_proj_1.y, cut_proj_1.z);
	glVertex3f(cut_proj_2.x, cut_proj_2.y, cut_proj_2.z);
	glEnd();

	// Disable flags for cleanup (optional)
	glDisable(GL_NORMALIZE);
	glDisable(GL_COLOR_MATERIAL);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, g_winHeight, 0, g_winWidth);

	glDisable(GL_BLEND);
	// Feedback messages.

	glColor4f(0, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	stringstream mytext;
	mytext << "Current object count: " << physics->count() << ", Reset object set to: ";
	switch (choice) {
	case 0: mytext << "Melon."; break;
	case 1: mytext << "Cake."; break;
	case 2: mytext << "Log."; break;
	}
	if (g_slow) {
		mytext << " SLOWMO.";
	}
	if (g_samurai) {
		mytext << " SAMURAI MODE ACTIVE!!! Press x to resolve.";
	}
	else if (g_paused) {
		mytext << " PAUSED. Press space to resume.";
	}
	string text = mytext.str();
	glRasterPos2i(10, 30);  // move in 10 pixels from the left and bottom edges
	for (int i = 0; i < text.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, text[i]);
	}
	text = "Controls: 1, 2, 3 to switch reset models, r to reset, b to show bounds, [ to enter slowmo, x for samurai mode, 5 to add boxes";
	glRasterPos2i(10, 10);  // move in 10 pixels from the left and bottom edges
	for (int i = 0; i < text.size(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, text[i]);
	}
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glutSwapBuffers();

	// Queue the next frame to be drawn straight away
	glutPostRedisplay();
	g_delta =(float)((glutGet(GLUT_ELAPSED_TIME) - g_start_time)/1000);
}

// Unprojection system for creating cutting planes.
vec3 myUnProject(int x, int y, int z) {
	glLoadIdentity();

	GLint viewport[4];
	GLdouble modelView[16];
	GLdouble projection[16];
	GLfloat winX, winY, winZ;
	GLdouble resX, resY, resZ;
	setUpCamera();

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);


	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	winZ = z;

	gluUnProject(winX, winY, winZ, modelView, projection, viewport, &resX, &resY, &resZ);

	return vec3(resX, resY, resZ);
}


// Reshape function
// 
void reshape(int w, int h) {
    if (h == 0) h = 1;

	g_winWidth = w;
	g_winHeight = h;
    
    // Sets the openGL rendering window to match the window size
    glViewport(0, 0, g_winWidth, g_winHeight);  
}

void reset() {
	g_geometry.clear();
	boxes.clear();
	physics->clear();
	geometry new_g;
	switch (choice) {
	case 0:
		new_g = m_melon; break;
	case 1:
		new_g = m_cake; break;
	default:
		new_g = m_log; break;
	}
	new_g.setRigidBody();
	g_geometry.push_back(new_g);
	choiceShader = choice;
}

// Keyboard callback
// Called once per button state change
//
void keyboardCallback(unsigned char key, int x, int y) {
	if (key == GLUT_KEY_RIGHT || key == 'a') {
		g_yRotation += 5;
	} else if (key == GLUT_KEY_LEFT || key == 'd') {
		g_yRotation -= 5;
	}else if (key == GLUT_KEY_UP || key == 'w') {
		g_yPosition += 0.03;
	}else if (key == GLUT_KEY_DOWN || key == 's') {
		g_yPosition -= 0.03;
	}
	if(key == '5'){
	  float offset = distribution(generator);
	  float offset2 = distribution(generator);
	  vector<vec3> vertex;
	vertex.push_back(vec3(-.5, .5, -.5));
	vertex.push_back(vec3(-.5, -.5, -.5));
	vertex.push_back(vec3(.5, .5, .5));
	vertex.push_back(vec3(.5, -.5, .5));
	Rigidbody * box = new Rigidbody(vec3(0, 9.99, 0), vertex, 1, vertex.size(), vec3(offset, 0, offset2));
	boxes.push_back(box);
	  physics->addRigidbody(box);
	}
	
	cout << key << " " << g_yRotation << endl;
	if(key=='j'){
             boxes[0]->addForce(vec3(-1,0,0));
	}
	if(key=='l'){
               boxes[0]->addForce(vec3(1,0,0));
        }
	if(key=='n'){
              boxes[0]->addForce(vec3(0,1,0));
        }
	if(key=='m'){
               boxes[0]->addForce(vec3(0,-1,0));
        }
	if(key=='i'){
               boxes[0]->addForce(vec3(0,0,-1));
        }
	if(key=='k'){
               boxes[0]->addForce(vec3(0,0,1));
	}

     switch(key){
		 case ' ':
			 g_paused = !g_paused;
			 g_samurai = false;
			 break;
	   case '[':
		   g_slow = !g_slow;
		   break;
	   case 'b':
		   g_bounds = !g_bounds;
		   break;
	   case 'x':
		   g_samurai = !g_samurai;
		   g_paused = false;
		   break;
	   case 'r':
		   reset();
		   break;
	   case '1':
		   choice = 0;
		   break;
	   case '2':
		   choice = 1;
		   break;
	   case '3':
		   choice = 2;
		   break;
	 }

}


// Special Keyboard callback
// Called once per button state change
//
void specialCallback(int key, int x, int y) {

}


// Mouse Button Callback function
// (x,y) :: (0,0) is top left and (g_winWidth, g_winHeight) is bottom right
// state :: 0 is down, 1 is up
// 
// Called once per button state change
// 
void mouseCallback(int button, int state, int x, int y) {
	//cout << "Mouse Callback :: button=" << button << ", state=" << state << ", (" << x << "," << y << ")" << endl;
	switch(button){

		case 0: //left mouse button
			g_drawMouse = (state==0);
			if (g_drawMouse) {
				cut_draw_1 = myUnProject(x, y, 0);
				cut_proj_1 = myUnProject(x, y, 1);
				cut_draw_2 = myUnProject(x, y, 0);
				cut_proj_2 = myUnProject(x, y, 1);
				g_drawMoved = false;
			}
			if (!g_drawMouse && g_drawMoved) {
				cut_draw_2 = myUnProject(x, y, 0);
				cut_proj_2 = myUnProject(x, y, 1);

				//Create cut upon mouse release.
				vector<vec3> plane;
				plane.push_back(cut_proj_1);
				plane.push_back(cut_proj_2);
				plane.push_back(cut_draw_2);
				vector<geometry> allGeometry;
				allGeometry = g_cut->createCut(plane, g_geometry, physics, g_samurai);
				g_geometry = allGeometry;
				//for all geometry objects
				//preserve momentum
				//clear rigid body list
				//add all the new rigidbodies post cut
				// PlaySound("../work/res/sounds/kama1.wav", NULL, SND_FILENAME | SND_ASYNC);
			}
			break;

		case 1:
			g_zoomFactor = 1;
			g_yPosition = 0;
			g_yRotation = 0;
			g_xRotation = 0;
			break;

		case 2:
			g_mouseDown = (state == 0);
			g_mousePos = vec2(x, y);
			break;

		case 3: //scroll foward/up
			g_zoomFactor /= 1.1;
			break;

		case 4: //scroll back/down
			g_zoomFactor *= 1.1;
			break;
	}
}


// Mouse Motion Callback function
// Called once per frame if the mouse has moved and
// at least one mouse button has an active state
void mouseMotionCallback(int x, int y) {
	//cout << "Mouse Motion Callback :: (" << x << "," << y << ")" << endl;
	if (g_drawMouse) {
		cut_draw_2 = myUnProject(x, y, 0);
		cut_proj_2 = myUnProject(x, y, 1);
		g_drawMoved = true;
	}
	if (g_mouseDown) {
		vec2 dif = vec2(x, y) - g_mousePos;
		g_mousePos = vec2(x, y);
		g_yRotation += 0.3 * dif.x;
		g_xRotation += 0.3 * dif.y;
		if (g_xRotation < 0) { g_xRotation = 0; }
		if (g_xRotation > 90) { g_xRotation = 90; }
	}
}



//Main program
// 
int main(int argc, char **argv) {

	// Initialise GL, GLU and GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);

	// Initialise window size and create window
	glutInitWindowSize(g_winWidth, g_winHeight);
	g_mainWindow = glutCreateWindow("COMP308 Final Project");


	// Initilise GLEW
	// must be done after creating GL context (glutCreateWindow in this case)
	// GLenum err = glewInit();
	// if (GLEW_OK != err) { // Problem: glewInit failed, something is seriously wrong.
		// cerr << "Error: " << glewGetErrorString(err) << endl;
		// abort(); // Unrecoverable error
	// }

	cout << "Using OpenGL " << glGetString(GL_VERSION) << endl;
	// cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;

	//Create our physics and geometry
	physics = new Physics();
	// Finally create our geometry
	m_melon = geometry("../work/res/assets/sphere.obj", physics);
	m_log = geometry("../work/res/assets/TestLog.obj", physics);
	m_cake = geometry("../work/res/assets/TestCake.obj", physics);
	physics->clear();
	boxes.clear();
	geometry starter = m_melon;
	starter.setRigidBody();
	g_geometry.push_back(starter);

	g_cut = new cut();
	
	initShader("../work/res/shaders/melon_shader.vert", "../work/res/shaders/melon_shader.frag", &melon_shader);
	initShader("../work/res/shaders/cake_shader.vert", "../work/res/shaders/cake_shader.frag", &cake_shader);
	initShader("../work/res/shaders/wood_shader.vert", "../work/res/shaders/wood_shader.frag", &wood_shader);
	cout << melon_shader << " " << cake_shader << " " << wood_shader << endl;

	// Register functions for callback
	glutDisplayFunc(draw);
	glutReshapeFunc(reshape);
	
	glutKeyboardFunc(keyboardCallback);
	glutSpecialFunc(specialCallback);

	glutMouseFunc(mouseCallback);
	glutMotionFunc(mouseMotionCallback);

	cut_draw_1 = vec3(-1, -1, 0);
	cut_draw_2 = vec3(-1, -1, 0);


	// Create a light on the camera
	initLight();
	// Loop required by OpenGL
	// This will not return until we tell OpenGL to finish
	glutMainLoop();

	// Don't forget to delete all pointers that we made
	delete physics;
	delete g_cut;
	for(Rigidbody* box :boxes){
	  delete box;
	}
	return 0;
}
