#ifndef __QUICKPLAYER_H__
#define __QUICKPLAYER_H__

#include <SFML/Graphics.hpp>

struct LobbyManager;
struct ConnectionManager;

struct Quickplayer
{
	enum Action
	{
		A_IDLE,
		A_GATHERING_USERS,
		A_GET_CONNECTIONS,
		RUNNING_MATCH,
	};

	Action action;
	sf::Vertex quad[4];

	LobbyManager *lobbyManager;
	ConnectionManager *connectionManager;

	Quickplayer();
	~Quickplayer();
	void Update();
	void Draw(sf::RenderTarget *target);
	void FindMatch();
};

#endif