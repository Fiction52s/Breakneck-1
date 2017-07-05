#ifndef __HUD_H__
#define __HUD_H__

#include "ImageText.h"
#include "Tileset.h"
#include <SFML\Graphics.hpp>

struct GameSession;
struct RaceFightHUD
{
	enum MaskState
	{
		NEUTRAL,
		RED,
		BLUE
	};

	RaceFightHUD( GameSession *owner );
	sf::Sprite mask;
	ImageText *scoreRed;
	ImageText *scoreBlue;
	GameSession *owner;
	Tileset *ts_mask;

	void Draw(sf::RenderTarget *target);
	void UpdateScoreRed( int newScore );
	void UpdateScoreBlue(int newScore );
};

#endif