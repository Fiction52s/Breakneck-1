#ifndef __POSTPRACTICEMATCHMENU_H__
#define __POSTPRACTICEMATCHMENU_H__

#include <SFML\Graphics.hpp>

struct BasicTextMenu;
struct SingleInputMenuButton;
struct GamePopup;

struct PostPracticeMatchMenu
{
	enum Action
	{
		A_NORMAL,
		A_ACCEPT_POPUP,
		A_WAIT_FOR_LOBBY,
	};

	int action;

	BasicTextMenu *textMenu;
	SingleInputMenuButton *readyButton;
	GamePopup *acceptPopup;

	sf::Text waitForLobbyText;

	sf::Vertex otherPlayerTestQuad[4];

	PostPracticeMatchMenu();
	~PostPracticeMatchMenu();
	void Reset();
	int Update();
	bool WantsToKeepPlaying();
	bool IsReadyToJoinCustomLobby();
	void Draw(sf::RenderTarget *target);
};

#endif