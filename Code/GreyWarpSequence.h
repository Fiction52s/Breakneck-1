#ifndef __GREYWARPSEQUENCE_H__
#define __GREYWARPSEQUENCE_H__

#include "Sequence.h"

struct ShapeEmitter;
struct Session;
struct MovingGeoGroup;

struct GreyWarpSequence : Sequence
{
	enum State
	{
		FADEOUT,
		Count
	};

	GreyWarpSequence();
	~GreyWarpSequence();
	void UpdateState();
	void SetupStates();
	void ReturnToGame();
	void LayeredDraw(int layer, sf::RenderTarget *target);
	void Reset();
	GameSession *bonus;
	Session *sess;
};

#endif
