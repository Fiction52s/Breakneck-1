#ifndef __WAITING_ROOM_H__
#define __WAITING_ROOM_H__

#include <string>
#include <SFML/Graphics.hpp>
#include "GUI.h"
#include "steam/steam_api.h"
#include "Tileset.h"

struct WaitingRoom;
struct PlayerBox
{
	HyperLink *playerName;
	int index;
	sf::Vector2i topLeft;
	sf::Vertex bgQuad[4];
	WaitingRoom *waitingRoom;

	void Init(WaitingRoom *wr, int index );
	void Draw(sf::RenderTarget *target);
	void SetName(const std::string &name);
	void Show();
	void Hide();
	void SetTopLeft(sf::Vector2i &pos);
private:
	bool show;
	std::string playerNameStr;
};

struct WaitingRoom : GUIHandler, TilesetManager
{
	enum Action
	{
		A_WAITING_FOR_MEMBERS,
		A_READY_TO_START,
		A_STARTING,
		A_LEAVE_ROOM,
	};

	Action action;
	int frame;

	Panel *panel;
	Button *startButton;
	Button *leaveButton;

	int playerBoxWidth;
	int playerBoxHeight;
	int playerBoxSpacing;
	
	//PlayerBox players[4];
	PlayerBox playerBoxes[4];
	//TextChooseRect *memberNameRects[4];

	CSteamID ownerID;

	Tileset *ts_preview;
	sf::Vertex previewQuad[4];
	sf::Vector2f previewBottomLeft;

	int maxPlayers;


	WaitingRoom();
	~WaitingRoom();
	void Update();
	void Draw(sf::RenderTarget *target);
	void OpenPopup();
	void ClosePopup();
	void SetPreview(const std::string &previewPath);
	void SetMaxPlayers(int n);
	void SetAction(Action a);
	bool HandleEvent(sf::Event ev);
	void UpdateMemberList();
	void ClearPreview();

	void ChooseRectEvent(ChooseRect *cr, int eventType);
	void ButtonCallback(Button *b, const std::string & e);
	void PanelCallback(Panel *p, const std::string & e);
	void CancelCallback(Panel *p);
	void OnLobbyChatUpdateCallback(LobbyChatUpdate_t *pCallback);
};

#endif