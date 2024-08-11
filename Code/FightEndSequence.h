#ifndef __FIGHT_END_SEQUENCE_H__
#define __FIGHT_END_SEQUENCE_H__


#include "Sequence.h"

struct ShapeEmitter;
struct Session;
struct MovingGeoGroup;

struct FightEndSequence : Sequence
{
	enum State
	{
		FREEZE,
		Count
	};

	FightEndSequence();
	~FightEndSequence();
	void UpdateState();
	void SetupStates();
	void ReturnToGame();
	void LayeredDraw(int layer, sf::RenderTarget *target);
	void Reset();

	Session *sess;
	Tileset *ts_game;
	sf::Sprite gameSprite;
};

#endif