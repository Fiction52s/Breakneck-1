#ifndef __MAP_LOBBY_INVITE_POPUP_H__
#define __MAP_LOBBY_INVITE_POPUP_H__

#include "GUI.h"
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "steam/isteamremotestorage.h"
#include "steam/steam_api.h"

struct LobbyInvitePopup : GUIHandler, TilesetManager, PanelUpdater
{
	enum Action
	{
		A_INACTIVE,
		A_ACTIVE,
		A_CONFIRMED,
		A_CANCELLED,
	};

	Panel *panel;
	Action action;
	
	HyperLink *inviteSenderName;
	sf::Text *inviteMessage;

	Button *confirmButton;
	Button *cancelButton;

	CSteamID lobbyId;
	CSteamID senderId;

	LobbyInvitePopup();
	~LobbyInvitePopup();
	void OpenPopup(CSteamID p_lobbyId, CSteamID p_senderId );
	void Update();
	void HandleEvent(sf::Event ev);
	void Draw(sf::RenderTarget *target);
	void ButtonCallback(Button *b, const std::string & e);
};


#endif
