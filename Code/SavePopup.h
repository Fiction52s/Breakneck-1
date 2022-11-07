#ifndef __SAVE_POPUP_H__
#define __SAVE_POPUP_H__

#include "GUI.h"
#include <vector>
#include <string>

struct MapHeader;
struct LobbyParams;
struct MapNode;

struct MapBrowserHandler;
struct ConfirmPopup;

struct SavePopup
{
	enum Action
	{
		A_INACTIVE,
		A_ACTIVE,
		A_CONFIRMED,
		A_CHECK_OVERWRITE,
		A_CANCELLED,
	};

	int action;
	MapBrowserHandler *browserHandler;
	ConfirmPopup *overwritePopup;

	SavePopup();
	~SavePopup();

	bool Activate(const std::string &activationPath, const std::string &defaultFileName );
	void Deactivate();
	void Update();
	bool HandleEvent(sf::Event ev);
	void Draw(sf::RenderTarget *target);
};

#endif