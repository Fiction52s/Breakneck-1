#ifndef __LOADING_POPUP_H__
#define __LOADING_POPUP_H__

#include "GUI.h"

struct LoadingPopup : GUIHandler
{
	Panel *panel;
	EditSession *edit;
	int action;

	enum Action
	{
		A_INACTIVE,
		A_ACTIVE,
	};

	int frame;
	std::string preDotsStr;
	std::string postDotsStr;
	sf::Text *messageText;

	LoadingPopup();
	~LoadingPopup();
	void UpdateFullMessage();
	void Update();
	void Pop(const std::string &prestr, const std::string &poststr);
	//void ButtonCallback(Button *b,
	//	const std::string &e);
	//void CancelCallback(Panel *p);
	//void ConfirmCallback(Panel *p);
	void Draw(sf::RenderTarget *target);
};

#endif