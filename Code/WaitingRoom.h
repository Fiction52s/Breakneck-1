#ifndef __WAITING_ROOM_H__
#define __WAITING_ROOM_H__

#include <string>
#include <SFML/Graphics.hpp>
#include "GUI.h"

struct WaitingRoom : GUIHandler
{
	enum Action
	{
		A_WAITING_FOR_MEMBERS,
		A_READY_TO_START,
		A_STARTING,
	};

	Action action;
	int frame;

	Panel *panel;
	Button *startButton;
	Button *leaveButton;

	WaitingRoom();
	~WaitingRoom();
	void Update();
	void Draw(sf::RenderTarget *target);
	void OpenPopup();
	void ClosePopup();
	void SetAction(Action a);
	bool HandleEvent(sf::Event ev);

	void ChooseRectEvent(ChooseRect *cr, int eventType);
	void ButtonCallback(Button *b, const std::string & e);
	void PanelCallback(Panel *p, const std::string & e);
};

#endif