#ifndef __POST_MATCH_OPTIONS_POPUP_H__
#define __POST_MATCH_OPTIONS_POPUP_H__

#include "GUI.h"
#include <vector>
#include <string>

struct MapHeader;
struct LobbyParams;
struct MapNode;

struct PostMatchOptionsPopup : GUIHandler
{
	enum Action
	{
		A_IDLE,
		A_REMATCH,
		A_CHOOSE_MAP,
		A_LEAVE,
		Count,
	};

	Action action;

	sf::Vertex bgQuad[4];
	Panel *panel;

	PostMatchOptionsPopup();
	~PostMatchOptionsPopup();

	void ChooseRectEvent(ChooseRect *cr, int eventType);
	void ButtonCallback(Button *b, const std::string & e);

	bool HandleEvent(sf::Event ev);
	void Start();

	void Update();
	void Draw(sf::RenderTarget *target);
};

#endif