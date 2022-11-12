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
		A_HOST,
		A_CANCELLED,
		A_EDIT,
		A_PLAY,
		A_SAVE,
	};

	enum Mode
	{
		MODE_WORKSHOP_BROWSE,
		MODE_CREATE_LOBBY,
	};

	int mode;

	Panel *panel;
	bool active;
	Action action;

	sf::Sprite previewSpr;
	Tileset *ts_preview;


	MapHeader *currMapHeader;
	LobbyParams *currLobbyParams;

	int chosenGameModeType;
	std::string mapPath;
	Dropdown *modeDropdown;
	Dropdown *numPlayersDropdown;

	std::vector<int> gameModeDropdownModes;
	std::vector<std::string> gameModeOptions;

	std::vector<int> playerNumOptions;

	sf::Vector2i previewPos;
	HyperLink *creatorLink;

	sf::Text *creatorLabel;
	sf::Text *creatorByLabel;
	HyperLink *nameLink;
	sf::Text *nameLabel;
	sf::Text *descriptionText;

	HyperLink *fileLink;

	//create lobby mode
	Button *createLobbyHostButton;
	Button *createLobbyCancelButton;

	MapOptionsPopup();
	~MapOptionsPopup();
	bool Activate(MapNode *mp);
	void Update();
	void HandleEvent(sf::Event ev);
	void Draw(sf::RenderTarget *target);
	void UpdateNumPlayerOptions();

	void ButtonCallback(Button *b, const std::string & e);
	void DropdownCallback(Dropdown *dropdown, const std::string & e);
	void CancelCallback();
};

#endif