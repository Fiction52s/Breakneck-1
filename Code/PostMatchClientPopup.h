#ifndef __POST_MATCH_CLIENT_POPUP_H__
#define __POST_MATCH_CLIENT_POPUP_H__

#include "GUI.h"
#include <vector>
#include <string>

struct MapHeader;
struct MapNode;

struct PostMatchClientPopup : GUIHandler
{
	enum Action
	{
		A_IDLE,
		A_LEAVE,
		Count,
	};

	Action action;
	int framesWaiting;

	sf::Vertex bgQuad[4];
	Panel *panel;

	sf::Text *waitingText;

	int secondsToWait;

	PostMatchClientPopup();
	~PostMatchClientPopup();

	int GetSecondsRemaining();
	void ChooseRectEvent(ChooseRect *cr, int eventType);
	void ButtonCallback(Button *b, const std::string & e);
	void UpdateMsg();

	bool HandleEvent(sf::Event ev);
	void Start();

	void Update();
	void Draw(sf::RenderTarget *target);
};

#endif