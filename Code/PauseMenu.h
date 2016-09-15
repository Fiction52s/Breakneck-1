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

	void Update( ControllerState &currInput,
		ControllerState &prevInput );
	Tileset *ts_background[Count];
	sf::Sprite bgSprite;
	GameSession *owner;
	Tab currentTab;
	//bool show;
	//if you tab away it resets


	//map tab
	sf::Vector2f mapCenter;
	float mapZoomFactor;

};

#endif