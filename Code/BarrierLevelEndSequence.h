#ifndef __BARRIER_LEVEL_END_SEQUENCE_H__
#define __BARRIER_LEVEL_END_SEQUENCE_H__


#include "Sequence.h"

struct Session;

struct BarrierLevelEndSequence : Sequence
{
	enum State
	{
		FADE,
		Count
	};

	BarrierLevelEndSequence();
	~BarrierLevelEndSequence();
	void UpdateState();
	void SetupStates();
	void ReturnToGame();
	void LayeredDraw(int p_drawLayer, sf::RenderTarget *target);
	void Reset();

	Session *sess;
};

#endif