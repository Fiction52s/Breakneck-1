#ifndef __FREEPLAY_MAP_OPTIONS_POPUP_H__
#define ___FREEPLAY_MAP_OPTIONS_POPUP_H__

#include "GUI.h"
#include <vector>
#include <string>

struct MapHeader;
struct LobbyData;
struct MapNode;

struct FreeplayMapOptionsPopup : GUIHandler
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
	LobbyData *currLobbyData;

	int chosenGameModeType;
	std::string mapPath;
	Dropdown *modeDropdown;

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

	FreeplayMapOptionsPopup();
	~FreeplayMapOptionsPopup();
	bool Activate(MapNode *mp);
	void Update();
	bool HandleEvent(sf::Event ev);
	void Draw(sf::RenderTarget *target);
	void UpdateNumPlayerOptions();

	void ButtonCallback(Button *b, const std::string & e);
	void DropdownCallback(Dropdown *dropdown, const std::string & e);
	void CancelCallback(Panel *p);
};

#endif