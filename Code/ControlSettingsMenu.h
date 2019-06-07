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
		S_EDITING,
		S_WAITING,
	};

	enum UpdateState
	{
		CONFIRM,
		NORMAL
	};

	int numActions;
	ButtonState currButtonState;
	ControlSettingsMenu(MainMenu *p_mm);
	XBoxButton GetFilteredButton( ControllerType cType,
		ControllerSettings::ButtonType b );
	~ControlSettingsMenu();
	void InitAssocSymbols();
	XBoxButton CheckXBoxInput(ControllerState &currInput);
	UpdateState Update(ControllerState &currInput, 
		ControllerState &prevInput);
	void UpdateControlIcons();
	SingleAxisSelector *xSelector;
	SingleAxisSelector *ySelector;
	void SetGreyActionTiles( bool greyOn );
	void SetButtonAssoc();
	bool IsEditingButtons();

	MainMenu *mainMenu;
	void SetActionTile(int actionIndex, int actionType);
	void Draw(sf::RenderTarget *target);
	void UpdateXboxButtonIcons();
	sf::Vertex *actionQuads;
	sf::Vertex *buttonQuads;
	sf::Vertex *selectQuads;
	sf::Vertex *labelQuads;
	void UpdateSelectedQuad();
	sf::Text *actionText;
	Tileset *ts_xboxButtons;
	Tileset *ts_actionIcons;
	Tileset *ts_currentButtons;
	ProfileSelector *pSel;
	bool editMode;
	ControllerType currCType;
};



#endif