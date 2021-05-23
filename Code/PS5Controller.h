#ifndef __PS5_CONTROLLER_H__
#define __PS5_CONTROLLER_H__

#include <Windows.h>
#include <ds5w.h>

struct GameController;
struct MainMenu;
struct PS5ControllerManager
{
	PS5ControllerManager();
	const static int MAX_CONTROLLERS = 4;
	unsigned int numControllers;
	DS5W::DeviceEnumInfo infos[4];
	bool CheckForControllers();
	void InitControllers( MainMenu *mm);
};

struct PS5Controller
{
	bool enabled;
	int index;

	DS5W::DeviceContext context;
	DS5W::DS5InputState inState;


	PS5Controller();
	~PS5Controller();
	void ClearState();
	bool InitContext(DS5W::DeviceEnumInfo &info);
	bool UpdateState();
	void CleanupContext();
};

#endif