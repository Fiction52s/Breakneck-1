#ifndef __QUICKPLAY_SEARCH_SCREEN_H__
#define __QUICKPLAY_SEARCH_SCREEN_H__

#include <SFML\Graphics.hpp>
#include "Tileset.h"

struct LoadingBackpack;
//#include "Actor.h"
//#include "PlayerSkinShader.h"

struct QuickplaySearchScreen : TilesetManager
{
	enum Action
	{
		A_SEARCHING,
		A_CANCELED,
		A_FOUND,
	};

	sf::Vertex bgQuad[4];
	sf::Vertex frameQuad[4];
	int frame;
	int action;
	Tileset *ts_bg;
	Tileset *ts_buttons;
	sf::Text searchingText;
	sf::Text cancelText;
	LoadingBackpack *backpack;
	sf::Vertex buttonQuad[4];
	

	QuickplaySearchScreen();
	~QuickplaySearchScreen();
	void UpdateButtonIconsWhenControllerIsChanged();
	void Reset();
	bool IsCanceled();
	bool IsMatchFound();
	void Update();
	void Draw(sf::RenderTarget *target);
};

#endif