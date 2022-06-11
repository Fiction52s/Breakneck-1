#include <iostream>
//#include "PlayerChar.h"
#include <sstream>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <assert.h>
#include <fstream>
#include <list> 
#include <stdlib.h>
//#include "EditSession.h"
#include "VectorMath.h"
#include "Input.h"
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
#include <vld.h>
#include "steam/steam_api.h"
//#include "WorkshopManager.h"


using namespace std;
using namespace sf;

using namespace boost::filesystem;



void perspectiveGL(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
	const GLdouble pi = 3.1415926535897932384626433832795;
	GLdouble fW, fH;

	//fH = tan( (fovY / 2) / 180 * pi ) * zNear;
	fH = tan(fovY / 360 * pi) * zNear;
	fW = fH * aspect;

	glFrustum(-fW, fW, -fH, fH, zNear, zFar);
}


void collideShapes(Actor &a, const CollisionBox &b, Actor &a1, const CollisionBox &b1)
{
	if (b.isCircle && b1.isCircle)
	{
		//circle circle
	}
	else if (b.isCircle)
	{
		//circle rect
	}
	else if (b1.isCircle)
	{
		//circle rect
	}
	else
	{
		//rect rect
	}
}

// Purpose: callback hook for debug text emitted from the Steam API
//-----------------------------------------------------------------------------
extern "C" void __cdecl SteamAPIDebugTextHook(int nSeverity, const char *pchDebugText)
{
	// if you're running in the debugger, only warnings (nSeverity >= 1) will be sent
	// if you add -debug_steamapi to the command-line, a lot of extra informational messages will also be sent
	//OutputDebugStringA(pchDebugText);
	printf(pchDebugText);

	if (nSeverity >= 1)
	{
		// place to set a breakpoint for catching API errors
		int x = 3;
		(void)x;
	}
}

void OutputDebugString(const std::string &str)
{
	cout << str;
}

void Alert(const std::string &str, const std::string &detail)
{
	cout << "Message: " << str << ", Detail: " << detail;
}

int SteamStartup()
{
	if (SteamAPI_RestartAppIfNecessary(k_uAppIdInvalid))
	{
		// if Steam is not running or the game wasn't started through Steam, SteamAPI_RestartAppIfNecessary starts the 
		// local Steam client and also launches this game again.

		// Once you get a public Steam AppID assigned for this game, you need to replace k_uAppIdInvalid with it and
		// removed steam_appid.txt from the game depot.

		return EXIT_FAILURE;
	}

	if (!SteamAPI_Init())
	{
		OutputDebugString("SteamAPI_Init() failed\n");
		Alert("Fatal Error", "Steam must be running to play this game (SteamAPI_Init() failed).\n");
		return EXIT_FAILURE;
	}

	SteamClient()->SetWarningMessageHook(&SteamAPIDebugTextHook);

	if (!SteamUser()->BLoggedOn())
	{
		OutputDebugString("Steam user is not logged in\n");
		Alert("Fatal Error", "Steam user must be logged in to play this game (SteamUser()->BLoggedOn() returned false).\n");
		return EXIT_FAILURE;
	}

	return 0;
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

	//return 0;
	//GCC::USBDriver driver;
	//GCC::VJoyGCControllers joys(driver);

	/*while (true)
	{
	auto controllers = driver.getState();

	if (controllers[0].buttons.a)
	{
	cout << (int)(controllers[0].axis.left_x) << endl;
	break;
	}
	}*/

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

	int result = SteamStartup();
	if (result != 0)
		return result;

	MainMenu *mm = new MainMenu();
	mm->Run();
	delete mm;

	//_CrtDumpMemoryLeaks();
}

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR lpCmdLine, INT nCmdShow)
{
	int result = SteamStartup();
	if (result != 0)
		return result;

	MainMenu *mm = new MainMenu();
	mm->Run();
	delete mm;

	return 0;
}
