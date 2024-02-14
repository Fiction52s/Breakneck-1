#ifndef __CLOSED_BETA_SCREEN_H_
#define __CLOSED_BETA_SCREEN_H_

#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct ClosedBetaScreen : TilesetManager
{
	enum Action
	{
		A_IDLE,
		A_DONE,
	};

	int action;
	int frame;

	sf::Vertex quad[4];
	sf::Text text;
	Tileset *ts_bg;
	sf::Sprite bgSpr;

	ClosedBetaScreen();
	void Update();
	void Draw(sf::RenderTarget *target);
};

#endif