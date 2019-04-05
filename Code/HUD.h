#ifndef __HUD_H__
#define __HUD_H__

#include "ImageText.h"
#include "Tileset.h"
#include <SFML\Graphics.hpp>
#include "GameSession.h"



struct RaceFightHUD
{
	/*enum MaskState
	{
		NEUTRAL,
		RED,
		BLUE
	};*/

	enum PlayerColor
	{
		BLUE,
		RED,
		GREEN,
		PURPLE,
		NONE
	};

	RaceFightHUD(GameSession::RaceFight *rf );
	sf::Sprite mask;
	ImageText *scoreRed;
	ImageText *scoreBlue;
	GameSession *owner;
	Tileset *ts_mask;

	Tileset *ts_smallMask;

	Tileset *ts_fx_gainPointBlue;
	Tileset *ts_fx_gainPointRed;
	Tileset *ts_ready;

	sf::Sprite smallMask;

	void Draw(sf::RenderTarget *target);
	void UpdateScore( PlayerColor pc, int newScore );
	void UpdateScore();
	

	void SetRaceWinner( PlayerColor pc );
	void ScorePoint( PlayerColor pc );

	GameSession::RaceFight *raceFight;
};



struct Minimap;

struct AdventureHUD
{
	enum State
	{
		SHOWN,
		ENTERING,
		EXITING,
		HIDDEN
	};

	AdventureHUD(GameSession *owner);
	void Hide(int frames);
	void Show(int frames);
	void Draw(sf::RenderTarget *target);
	void Update();
	void Reset();
	bool IsHidden();

	int processFrames;
	bool show;
	int frame;
	State state;

	sf::Vector2f miniShowPos;
	sf::Vector2f miniHidePos;
	CubicBezier showBez;
	CubicBezier hideBez;
	
	Minimap *mini;

	GameSession *owner;
};

#endif