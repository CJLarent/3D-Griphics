//***************************************************************************
// GAME2012_FinalProject_MiguelPenalver_ChristopherLarent_Carlson.cpp (C) 2020 All Rights Reserved.
//
// Assignment - FinalProject CASTLE submission.
//
// Description:
// Click run to see the results, W S A D Q E MOVEMENT.
//*****************************************************************************

using namespace std;

#include "stdlib.h"
#include "time.h"
#include <GL/glew.h>
#include <GL/freeglut.h> 
#include "prepShader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <iostream>
#include "Shape.h"
#include "Light.h"

#define BUFFER_OFFSET(x)  ((const void*) (x))
#define FPS 60
#define MOVESPEED 0.1f
#define TURNSPEED 0.05f
#define X_AXIS glm::vec3(1,0,0)
#define Y_AXIS glm::vec3(0,1,0)
#define Z_AXIS glm::vec3(0,0,1)
#define XY_AXIS	    glm::vec3(1,1,0)
#define YZ_AXIS	    glm::vec3(0,1,1)
#define XZ_AXIS		glm::vec3(1,0,1)
#define XYZ_AXIS	glm::vec3(1,1,1)
#define SPEED 0.25f
//MUST BE TOGETHER
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
//*******************************
enum keyMasks {
	KEY_FORWARD = 0b00000001,		// 0x01 or   1	or   01
	KEY_BACKWARD = 0b00000010,		// 0x02 or   2	or   02
	KEY_LEFT = 0b00000100,
	KEY_RIGHT = 0b00001000,
	KEY_UP = 0b00010000,
	KEY_DOWN = 0b00100000,
	KEY_MOUSECLICKED = 0b01000000

	// Any other keys you want to add.
};

static unsigned int
program,
vertexShaderId,
fragmentShaderId;
// IDs.
GLuint modelID, viewID, projID;
// Matrices.
glm::mat4 View, Projection;
// Our bitflag variable. 1 byte for up to 8 key states.
unsigned char keys = 0; // Initialized to 0 or 0b00000000.

// Texture variables.
GLuint blankID, firstID, secondID, thirdID;
GLint width, height, bitDepth;

// Light objects. Now OOP.
AmbientLight aLight(
	glm::vec3(1.0f, 1.0f, 0.0f),	// Diffuse colour.
	0.2f);// Diffuse strength.
//Directional light -  not used
DirectionalLight dLight(
	glm::vec3(1.0f, 0.0f, 0.0f),	// Origin.
	glm::vec3(1.0f, 0.0f, 0.0f),	// Diffuse colour.
	0.0f);							// Diffuse strength.
//Point light - not used
PointLight pLights(
	glm::vec3(5.0f, 1.0f, -2.5f),	// Position.
	10.0f,							// Range.
	1.0f, 4.5f, 75.0f,				// Constant, Linear, Quadratic.   
	glm::vec3(0.1f, 0.2f, 1.0f),	// Diffuse colour.
	0.0f);					// Diffuse strength.
					// Diffuse strength.
//Spot Light- Used
SpotLight sLight(
	glm::vec3(5.25f, 4.0f, -5.0f),	// Position.
	glm::vec3(1.0f, 0.6f, 0.3f),	// Diffuse colour.
	1.0f,							// Diffuse strength.
	glm::vec3(0.0f, -1.0f, 0.0f),   // Direction. Normally opposite because it's used in dot product. See constructor.
	30.0f);
// Camera and transform variables.
float scale = 1.0f, angle = 0.0f;
glm::vec3 position, frontVec, worldUp, upVec, rightVec; // Set by function
GLfloat pitch, yaw;
int lastX, lastY;

// Geometry data.
Grid g_grid(25);
Cube g_cube(1.0f, 1.0f, 1.0f); // Setting a scale for the cube.
Prism g_prism(7);
Cone g_cone(2);

void timer(int); // Prototype.

void resetView()
{
	position = glm::vec3(10.0f, 5.0f, 25.0f); // Super pulled back because of grid size.
	frontVec = glm::vec3(0.0f, 0.0f, -1.0f);
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	pitch = 0.0f;
	yaw = -90.0f;
	// View will now get set only in transformObject
}

void setupLights()
{
	// Setting ambient light.
	glUniform3f(glGetUniformLocation(program, "aLight.base.diffuseColour"), aLight.diffuseColour.x, aLight.diffuseColour.y, aLight.diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "aLight.base.diffuseStrength"), aLight.diffuseStrength);


	// Setting directional light.
	glUniform3f(glGetUniformLocation(program, "dLight.base.diffuseColour"), dLight.diffuseColour.x, dLight.diffuseColour.y, dLight.diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "dLight.base.diffuseStrength"), dLight.diffuseStrength);
	glUniform3f(glGetUniformLocation(program, "dLight.origin"), dLight.origin.x, dLight.origin.y, dLight.origin.z);

	// Setting point lights.
	glUniform3f(glGetUniformLocation(program, "pLight.base.diffuseColour"), pLights.diffuseColour.x, pLights.diffuseColour.y, pLights.diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "pLight.base.diffuseStrength"), pLights.diffuseStrength);
	glUniform3f(glGetUniformLocation(program, "pLight.position"), pLights.position.x, pLights.position.y, pLights.position.z);
	glUniform1f(glGetUniformLocation(program, "pLight.constant"), pLights.constant);
	glUniform1f(glGetUniformLocation(program, "pLight.linear"), pLights.linear);
	glUniform1f(glGetUniformLocation(program, "pLight.quadratic"), pLights.quadratic);

	// Setting spot light.
	glUniform3f(glGetUniformLocation(program, "sLight.base.diffuseColour"), sLight.diffuseColour.x, sLight.diffuseColour.y, sLight.diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "sLight.base.diffuseStrength"), sLight.diffuseStrength);

	glUniform3f(glGetUniformLocation(program, "sLight.position"), sLight.position.x, sLight.position.y, sLight.position.z);
	glUniform3f(glGetUniformLocation(program, "sLight.direction"), sLight.direction.x, sLight.direction.y, sLight.direction.z);
	glUniform1f(glGetUniformLocation(program, "sLight.edge"), sLight.edgeRad);

}
void setupVAOs()
{
	g_grid.BufferShape();
	g_cube.BufferShape();
	g_prism.BufferShape();
}
void setupShaders()
{
	srand((unsigned)time(NULL));
	// Create shader program executable.
	vertexShaderId = setShader((char*)"vertex", (char*)"cube.vert");
	fragmentShaderId = setShader((char*)"fragment", (char*)"cube_mig.frag");
	program = glCreateProgram();
	glAttachShader(program, vertexShaderId);
	glAttachShader(program, fragmentShaderId);
	glLinkProgram(program);
	glUseProgram(program);
	modelID = glGetUniformLocation(program, "model");
	viewID = glGetUniformLocation(program, "view");
	projID = glGetUniformLocation(program, "projection");
}
void setupTextures()
{
	// Image loading.
	stbi_set_flip_vertically_on_load(true);

	// Load first image.
	unsigned char* image = stbi_load("stone.jpg", &width, &height, &bitDepth, 0);
	if (!image) { cout << "Unable to load file!" << endl; }
	glGenTextures(1, &blankID);
	glBindTexture(GL_TEXTURE_2D, blankID);
	// Note: image types with native transparency will need to be GL_RGBA instead of GL_RGB.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(image);
	// End first image.

	// Load second image.
	image = stbi_load("grass.png", &width, &height, &bitDepth, 0);
	if (!image) { cout << "Unable to load file!" << endl; }
	glGenTextures(1, &firstID);
	glBindTexture(GL_TEXTURE_2D, firstID);
	// Note: image types with native transparency will need to be GL_RGBA instead of GL_RGB.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(image);
	// End second image.

	// Load third image.
	image = stbi_load("glass.png", &width, &height, &bitDepth, 0);
	if (!image) { cout << "Unable to load file!" << endl; }
	glGenTextures(1, &secondID);
	glBindTexture(GL_TEXTURE_2D, secondID);
	// Note: image types with native transparency will need to be GL_RGBA instead of GL_RGB.
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(image);
	// End third image.

	// Load fouth image.
	image = stbi_load("brick.jpg", &width, &height, &bitDepth, 0);
	if (!image) { cout << "Unable to load file!" << endl; }
	glGenTextures(1, &thirdID);
	glBindTexture(GL_TEXTURE_2D, thirdID);
	// Note: image types with native transparency will need to be GL_RGBA instead of GL_RGB.
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(image);
	// End third image.

	glUniform1i(glGetUniformLocation(program, "texture0"), 0);

}
void glEnables()
{

	// Enable depth test and blend.
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	// Enable smoothing.
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
}
void init(void)
{
	srand((unsigned)time(NULL));
	setupShaders();
	Projection = glm::perspective(glm::radians(45.0f), 1.0f / 1.0f, 0.1f, 100.0f);
	resetView();
	setupTextures();
	setupLights();
	setupVAOs();
	glEnables();
	timer(0); // Setup my recursive 'fixed' timestep/framerate.
}

//---------------------------------------------------------------------
// calculateView
void calculateView()
{
	frontVec.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	frontVec.y = sin(glm::radians(pitch));
	frontVec.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	frontVec = glm::normalize(frontVec);
	rightVec = glm::normalize(glm::cross(frontVec, worldUp));
	upVec = glm::normalize(glm::cross(rightVec, frontVec));

	View = glm::lookAt(
		position, // Camera position
		position + frontVec, // Look target
		upVec); // Up vector
}

//---------------------------------------------------------------------
// transformModel
void transformObject(glm::vec3 scale, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 translation) {
	glm::mat4 Model;
	Model = glm::mat4(1.0f);
	Model = glm::translate(Model, translation);
	Model = glm::rotate(Model, glm::radians(rotationAngle), rotationAxis);
	Model = glm::scale(Model, scale);
	
	// We must now update the View.
	calculateView();

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(projID, 1, GL_FALSE, &Projection[0][0]);
}

//---------------------------------------------------------------------
// display
void display(void)
{
	//glClearColor(0.01, 0.1, 0.1, 1.0); // green background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, firstID); // Use this texture for all shapes.
	//glClearColor(0.6, 0.6, 0.6,1.0);

	// Grid. Note: I rendered it solid!
	transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, -90.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	g_grid.DrawShape(GL_TRIANGLES);

	glBindTexture(GL_TEXTURE_2D, thirdID); // Use this texture for all shapes.


	// walls
	//back 
	transformObject(glm::vec3(13.0f, 5.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(0.0f, 0.0f, -25.0f));
	g_cube.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(12.5f, 5.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(13.0f, 0.0f, -25.0f));
	g_cube.DrawShape(GL_TRIANGLES);
	// left 
	transformObject(glm::vec3(8.5f, 5.0f, 1.0f), Y_AXIS, 90.0f, glm::vec3(-0.5f, 0.0f, -16.0f));
	g_cube.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(16.0f, 5.0f, 1.0f), Y_AXIS, 90.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	g_cube.DrawShape(GL_TRIANGLES);

	// right
	transformObject(glm::vec3(12.5f, 5.0f, 1.0f), Y_AXIS, 90.0f, glm::vec3(25.0f, 0.0f, 0.0f));
	g_cube.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(12.5f, 5.0f, 1.0f), Y_AXIS, -90.0f, glm::vec3(25.0f, 0.0f, -25.0f));
	g_cube.DrawShape(GL_TRIANGLES);

	//pilers


	transformObject(glm::vec3(1.0f, 8.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	g_prism.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(1.0f, 8.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(25.0f, 0.0f, 0.0f));
	g_prism.DrawShape(GL_TRIANGLES);

	// open building 

	transformObject(glm::vec3(3.0f, 3.0f, 1.0f), Y_AXIS, 90.0f, glm::vec3(15.0f, 0.0f, -8.0f));
	g_cube.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(3.0f, 3.0f, 1.0f), Y_AXIS, 90.0f, glm::vec3(10.0f, 0.0f, -8.0f));
	g_cube.DrawShape(GL_TRIANGLES);


	// roof 
	transformObject(glm::vec3(2.0f, 4.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(9.5f, 3.0f, -10.0f));
	g_cone.DrawShape(GL_TRIANGLES);

	transformObject(glm::vec3(2.0f, 4.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(15.0f, 3.0f, -10.0f));
	g_cone.DrawShape(GL_TRIANGLES);


	glBindTexture(GL_TEXTURE_2D, 0);

	glutSwapBuffers(); // Now for a potentially smoother render.
}

void idle() // Not even called.
{
	glutPostRedisplay();
}

void parseKeys()
{
	if (keys & KEY_FORWARD)
		position += frontVec * MOVESPEED;
	if (keys & KEY_BACKWARD)
		position -= frontVec * MOVESPEED;
	if (keys & KEY_LEFT)
		position -= rightVec * MOVESPEED;
	if (keys & KEY_RIGHT)
		position += rightVec * MOVESPEED;
	if (keys & KEY_UP)
		position += upVec * MOVESPEED;
	if (keys & KEY_DOWN)
		position -= upVec * MOVESPEED;
}

void timer(int) { // Tick of the frame.
	// Get first timestamp
	int start = glutGet(GLUT_ELAPSED_TIME);
	// Update call
	parseKeys();
	// Display call
	glutPostRedisplay();
	// Calling next tick
	int end = glutGet(GLUT_ELAPSED_TIME);
	glutTimerFunc((1000 / FPS) - (end-start), timer, 0);
}
// Keyboard input processing routine.
void keyDown(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	case 'w':
		if (!(keys & KEY_FORWARD))
			keys |= KEY_FORWARD; // keys = keys | KEY_FORWARD
		break;
	case 's':
		if (!(keys & KEY_BACKWARD))
			keys |= KEY_BACKWARD;
		break;
	case 'a':
		if (!(keys & KEY_LEFT))
			keys |= KEY_LEFT;
		break;
	case 'd':
		if (!(keys & KEY_RIGHT))
			keys |= KEY_RIGHT;
		break;
	case 'q':
		if (!(keys & KEY_UP))
			keys |= KEY_UP;
		break;
	case 'e':
		if (!(keys & KEY_DOWN))
			keys |= KEY_DOWN;
		break;
	default:
		break;
	}
}

void keyDownSpec(int key, int x, int y) // x and y is mouse location upon key press.
{
	switch (key)
	{
	case GLUT_KEY_UP: // Up arrow.
		if (!(keys & KEY_FORWARD))
			keys |= KEY_FORWARD; // keys = keys | KEY_FORWARD
		break;
	case GLUT_KEY_DOWN: // Down arrow.
		if (!(keys & KEY_BACKWARD))
			keys |= KEY_BACKWARD;
		break;
	default:
		break;
	}
}

void keyUp(unsigned char key, int x, int y) // x and y is mouse location upon key press.
{
	switch (key)
	{
	case 'w':
		keys &= ~KEY_FORWARD; // keys = keys & ~KEY_FORWARD. ~ is bitwise NOT.
		break;
	case 's':
		keys &= ~KEY_BACKWARD;
		break;
	case 'a':
		keys &= ~KEY_LEFT;
		break;
	case 'd':
		keys &= ~KEY_RIGHT;
		break;
	case 'q':
		keys &= ~KEY_UP;
		break;
	case 'e':
		keys &= ~KEY_DOWN;
		break;
	case ' ':
		resetView();
		break;
	default:
		break;
	}
}

void keyUpSpec(int key, int x, int y) // x and y is mouse location upon key press.
{
	switch (key)
	{
	case GLUT_KEY_UP:
		keys &= ~KEY_FORWARD; // keys = keys & ~KEY_FORWARD
		break;
	case GLUT_KEY_DOWN:
		keys &= ~KEY_BACKWARD;
		break;
	default:
		break;
	}
}

void mouseMove(int x, int y)
{
	if (keys & KEY_MOUSECLICKED)
	{
		pitch -= (GLfloat)((y - lastY) * TURNSPEED);
		yaw += (GLfloat)((x - lastX) * TURNSPEED);
		lastY = y;
		lastX = x;
	}
}

void mouseClick(int btn, int state, int x, int y)
{
	if (state == 0)
	{
		lastX = x;
		lastY = y;
		keys |= KEY_MOUSECLICKED; // Flip flag to true
		glutSetCursor(GLUT_CURSOR_NONE);
		//cout << "Mouse clicked." << endl;
	}
	else
	{
		keys &= ~KEY_MOUSECLICKED; // Reset flag to false
		glutSetCursor(GLUT_CURSOR_INHERIT);
		//cout << "Mouse released." << endl;
	}
}

//---------------------------------------------------------------------
// clean
void clean()
{
	cout << "Cleaning up!" << endl;
	glDeleteTextures(1, &firstID);
	glDeleteTextures(1, &secondID);
	glDeleteTextures(1, &blankID);
	glDeleteTextures(1, &thirdID);
}

//---------------------------------------------------------------------
int main(int argc, char** argv)
{
	//Before we can open a window, theremust be interaction between the windowing systemand OpenGL.In GLUT, this interaction is initiated by the following function call :
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
	glutSetOption(GLUT_MULTISAMPLE, 8);

	//if you comment out this line, a window is created with a default size
	glutInitWindowSize(1024, 1024);

	//the top-left corner of the display
	glutInitWindowPosition(0, 0);

	glutCreateWindow("GAME2012_A5_MiguelPenalver.cpp");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.

	init(); // Our own custom function.

	glutDisplayFunc(display);
	glutKeyboardFunc(keyDown);
	glutSpecialFunc(keyDownSpec);
	glutKeyboardUpFunc(keyUp);
	glutSpecialUpFunc(keyUpSpec);

	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseMove); // Requires click to register.

	atexit(clean); // This useful GLUT function calls specified function before exiting program. 
	glutMainLoop();

	return 0;
}