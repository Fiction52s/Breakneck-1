#ifndef __CONTROLLERSETTTINGSMENU_H__
#define __CONTROLLERSETTTINGSMENU_H__

#include "Input.h"

struct SingleAxisSelector;
struct MainMenu;
struct Tileset;


struct ControlSettingsMenu
{
	enum ButtonState
	{
		S_NEUTRAL,
		S_PRESSED,
		S_UNPRESSED,
		S_SELECTED,
	};

	ButtonState currButtonState;
	ControlSettingsMenu(MainMenu *p_mm);
	void InitAssocSymbols();
	XBoxButton CheckXBoxInput(ControllerState &currInput);
	void Update();
	void UpdateControlIcons();
	SingleAxisSelector *xSelector;
	SingleAxisSelector *ySelector;
	MainMenu *mainMenu;

	Tileset *ts_xboxButtons;
	Tileset *ts_actionIcons;

};

#endif