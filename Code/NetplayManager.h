#ifndef __NETPLAY_MANAGER_H__
#define __NETPLAY_MANAGER_H__

#include <SFML/Graphics.hpp>
#include "LobbyManager.h"
#include "ConnectionManager.h"

struct MatchParams;
struct GameSession;

struct NetplayManager
{
	enum Action
	{
		A_IDLE,
		A_GATHERING_USERS,
		A_GET_CONNECTIONS,
		A_READY_TO_RUN,
		A_RUNNING_MATCH,
	};

	Action action;
	sf::Vertex quad[4];
	bool isSyncTest;

	LobbyManager *lobbyManager;
	ConnectionManager *connectionManager;
	GameSession *game;

	NetplayManager();
	~NetplayManager();

	bool IsConnected();
	bool IsReadyToRun();
	bool IsIdle();
	void LeaveLobby();
	void RunMatch( MatchParams *mp );
	HSteamNetConnection GetConnection();
	bool IsLobbyCreator();
	void Abort();
	void Update();
	void Draw(sf::RenderTarget *target);
	void FindMatch();
};

#endif