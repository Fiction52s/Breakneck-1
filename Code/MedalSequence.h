#ifndef __MEDAL_SEQUENCE_H__
#define __MEDAL_SEQUENCE_H__

#include "Sequence.h"

struct Medal;

struct MedalSequence : Sequence
{
	enum State
	{
		SHOW_MEDAL,
		Count
	};

	/*int enterTime;
	int exitTime;
	sf::Vector2<double> abovePlayer;
	sf::Vector2<double> origPlayer;
	sf::Vector2<double> attachPoint;
	MovementSequence shipMovement;
	MovementSequence center;
	Tileset *ts_ship;
	sf::Sprite shipSprite;*/

	Medal *shownMedal;
	int medalType;

	MedalSequence();
	~MedalSequence();
	void StartRunning();
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddFlashes();
	void AddEnemies();
	void UpdateState();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
};

#endif