#ifndef __PAUSE_MENU_H__
#define __PAUSE_MENU_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "Input.h"

struct GameSession;
struct PauseMenu
{
	enum Tab
	{
		MAP,
		KIN,
		SHARDS,
		OPTIONS,
		PAUSE,
		Count
	};

	PauseMenu( GameSession *owner );
	void Draw( sf::RenderTarget *target );
	void SetTab( Tab t );
	void TabLeft();
	void TabRight();

	enum UpdateResponse
	{
		R_NONE,
		R_P_RESUME,
		R_P_RESPAWN,
		R_P_EXITLEVEL,
		R_P_EXITTITLE,
		R_P_EXITGAME
	};

	UpdateResponse Update( ControllerState &currInput,
		ControllerState &prevInput );
	Tileset *ts_background[Count];
	Tileset *ts_select;
	sf::Sprite bgSprite;
	sf::Sprite selectSprite;
	GameSession *owner;
	Tab currentTab;
	//bool show;
	//if you tab away it resets


	//map tab
	sf::Vector2f mapCenter;
	float mapZoomFactor;

	int pauseSelectIndex;

};

#endif