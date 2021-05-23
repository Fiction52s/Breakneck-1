#ifndef __PS5_CONTROLLER_H__
#define __PS5_CONTROLLER_H__

#include <Windows.h>
#include <ds5w.h>

struct PS5Controller
{
	static unsigned int numControllers;
	static DS5W::DeviceEnumInfo infos[4];
	static bool CheckForControllers();

	bool enabled;

	
	DS5W::DeviceContext context;
	DS5W::DS5InputState inState;

	PS5Controller();
	void ClearState();
	bool InitContext();
	bool Update();
	void CleanupContext();
};

#endif