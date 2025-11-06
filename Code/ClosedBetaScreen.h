#ifndef __CLOSED_BETA_SCREEN_H_
#define __CLOSED_BETA_SCREEN_H_

#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct ClosedBetaScreen : TilesetManager
{
	enum Action
	{
		A_IDLE,
		A_SPLASH_TRANSITION,
		A_SPLASH,
		A_DONE,
	};

	int action;
	int frame;

	sf::Vertex quad[4];
	sf::Text closedBetaText;
	Tileset *ts_closedBeta;
	sf::Sprite closedBetaSpr;

	sf::Text splashText;
	sf::Sprite splashSpr;
	Tileset *ts_splash;

	ClosedBetaScreen();
	void Update();
	void Draw(sf::RenderTarget *target);
};

#endif