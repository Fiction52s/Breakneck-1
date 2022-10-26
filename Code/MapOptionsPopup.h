#ifndef __MAP_OPTIONS_POPUP_H__
#define __MAP_OPTIONS_POPUP_H__

#include "GUI.h"
#include <vector>
#include <string>

struct MapHeader;
struct LobbyParams;
struct MapNode;

struct MapOptionsPopup : GUIHandler
{
	enum Action
	{
		A_INACTIVE,
		A_ACTIVE,
		A_CONFIRMED,
		A_CANCELLED,
	};

	Panel *panel;
	bool active;
	Action action;

	MapHeader *currMapHeader;
	LobbyParams *currLobbyParams;
	int chosenGameModeType;
	std::string mapPath;
	Dropdown *modeDropdown;
	Dropdown *numPlayersDropdown;

	std::vector<int> gameModeDropdownModes;
	std::vector<std::string> gameModeOptions;

	std::vector<int> playerNumOptions;

	MapOptionsPopup();
	~MapOptionsPopup();
	bool Activate(MapNode *mp);
	void Update();
	void HandleEvent(sf::Event ev);
	void Draw(sf::RenderTarget *target);
	void UpdateNumPlayerOptions();

	void ButtonCallback(Button *b, const std::string & e);
	void DropdownCallback(Dropdown *dropdown, const std::string & e);
};

#endif