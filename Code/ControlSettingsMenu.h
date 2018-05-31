#ifndef __CONTROLLERSETTTINGSMENU_H__
#define __CONTROLLERSETTTINGSMENU_H__

#include "Input.h"

struct SingleAxisSelector;
struct MainMenu;
struct Tileset;
struct ProfileSelector;

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
	~ControlSettingsMenu();
	void InitAssocSymbols();
	XBoxButton CheckXBoxInput(ControllerState &currInput);
	void Update(ControllerState &currInput, 
		ControllerState &prevInput);
	void UpdateControlIcons();
	SingleAxisSelector *xSelector;
	SingleAxisSelector *ySelector;

	MainMenu *mainMenu;
	void SetActionTile(int actionIndex, int actionType);
	void Draw(sf::RenderTarget *target);
	void UpdateXboxButtonIcons();
	sf::Vertex *actionQuads;
	sf::Vertex *buttonQuads;
	sf::Text *actionText;
	Tileset *ts_xboxButtons;
	Tileset *ts_actionIcons;
	Tileset *ts_currentButtons;
	ProfileSelector *pSel;

};



#endif