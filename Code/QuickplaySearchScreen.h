#ifndef __QUICKPLAY_SEARCH_SCREEN_H__
#define __QUICKPLAY_SEARCH_SCREEN_H__

#include <SFML\Graphics.hpp>
#include "Tileset.h"
//#include "Actor.h"
//#include "PlayerSkinShader.h"

struct QuickplaySearchScreen
{
	enum Action
	{
		A_SEARCHING,
		A_CANCELED,
		A_FOUND,
	};

	sf::Vertex bgQuad[4];
	int frame;
	int action;

	QuickplaySearchScreen();
	void Reset();
	bool IsCanceled();
	bool IsMatchFound();
	void Update();
	void Draw(sf::RenderTarget *target);
};

#endif