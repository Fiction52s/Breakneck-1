#ifndef __ADVENTURE_SCORE_DISPLAY_H__
#define __ADVENTURE_SCORE_DISPLAY_H__

#include <SFML\Graphics.hpp>
#include "ScoreDisplay.h"
#include "Tileset.h"

struct AdventureScoreDisplay : ScoreDisplay
{
	enum Action
	{
		A_IDLE,
		A_SHOW,
		A_GIVE_GOLD,
		A_GIVE_SILVER,
		A_GIVE_BRONZE,
		A_WAIT,
	};

	Tileset *ts_test;
	sf::Sprite testSpr;

	AdventureScoreDisplay(TilesetManager *p_tm, sf::Font &f );
	void Reset();
	void Activate();
	void Deactivate();
	void Update();
	bool IsActive();
	bool IsWaiting();
	bool IsIncludingExtraOptions();

	void Draw(sf::RenderTarget *target);
	
};

#endif