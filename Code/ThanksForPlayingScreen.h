#ifndef __THANKS_FOR_PLAYING_SCREEN_H__
#define __THANKS_FOR_PLAYING_SCREEN_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct ThanksForPlayingScreen : TilesetManager
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

	//Tileset *ts_thanksForPlaying;
	//sf::Vertex thanksQuad[4];

	ThanksForPlayingScreen();
	void Update();
	void Draw(sf::RenderTarget *target);
};

#endif