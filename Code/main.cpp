#include <iostream>
//#include "PlayerChar.h"
#include <sstream>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <assert.h>
#include <fstream>
#include <list> 
#include <stdlib.h>
#include "EditSession.h"
#include "VectorMath.h"
#include "Input.h"
#include "poly2tri/poly2tri.h"
#include "Physics.h"
#include "Actor.h"
#include "Tileset.h"
#include "GameSession.h"
#include "LevelSelector.h"
#include <boost/filesystem.hpp>
#include "Primitive3D.h"
#include <SFML/OpenGL.hpp>
#include "WorldMap.h"
#include "SaveFile.h"
#include "MainMenu.h"
//#include <crtdbg.h>
#include "Enemy.h"
//#include <vld.h>

using namespace std;
using namespace sf;

using namespace boost::filesystem;



void perspectiveGL( GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar )
{
    const GLdouble pi = 3.1415926535897932384626433832795;
    GLdouble fW, fH;

    //fH = tan( (fovY / 2) / 180 * pi ) * zNear;
    fH = tan( fovY / 360 * pi ) * zNear;
    fW = fH * aspect;

    glFrustum( -fW, fW, -fH, fH, zNear, zFar );
}


void collideShapes( Actor &a, const CollisionBox &b, Actor &a1, const CollisionBox &b1 )
{
	if( b.isCircle && b1.isCircle )
	{
		//circle circle
	}
	else if( b.isCircle )
	{
		//circle rect
	}
	else if( b1.isCircle )
	{
		//circle rect
	}
	else
	{
		//rect rect
	}
}

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

int main()
{
	//_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	// _CrtSetDbgFlag (
 //     _CRTDBG_ALLOC_MEM_DF |
 //     _CRTDBG_LEAK_CHECK_DF
	//  | _CRTDBG_CHECK_ALWAYS_DF
	//	);

	// _CrtMemState state;
	// _CrtMemCheckpoint(&state);
	// //state.

	//_CrtSetReportMode ( _CRT_ERROR,
 //     _CRTDBG_MODE_DEBUG);

	std::cout << "starting program" << endl;

	/*Text mainMenu[5];
	int fontSize = 32;

	int h = 100;
	int yoffset = 200;
	int xoffset = 200;
	int index = 0;*/
	
	/*Primitive prim;

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	glViewport(0,0,1920,1080);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	perspectiveGL(45.0f,(GLfloat)1920/(GLfloat)1080,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();	*/

	MainMenu *mm = new MainMenu();
	mm->Run();
	delete mm;

	//_CrtDumpMemoryLeaks();
}

