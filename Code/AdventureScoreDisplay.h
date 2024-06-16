#ifndef __ADVENTURE_SCORE_DISPLAY_H__
#define __ADVENTURE_SCORE_DISPLAY_H__

#include <SFML\Graphics.hpp>
#include "ScoreDisplay.h"
#include "Tileset.h"

struct KinExperienceBar;

struct AdventureScoreDisplay : ScoreDisplay
{
	enum Action
	{
		A_IDLE,
		A_EXPBAR,
		A_SHOW,
		A_GIVE_GOLD,
		A_GIVE_SILVER,
		A_GIVE_BRONZE,
		A_WAIT,
	};

	Tileset *ts_test;
	sf::Sprite testSpr;
	KinExperienceBar *expBar;

	AdventureScoreDisplay(TilesetManager *p_tm, KinExperienceBar *p_expBar, sf::Font &f );
	void Reset();
	void Activate();
	void Deactivate();
	void Confirm();
	void Update();
	bool IsActive();
	bool IsConfirmable();
	bool IsWaiting();
	bool IsIncludingExtraOptions();

	void Draw(sf::RenderTarget *target);
	
};

#endif