#ifndef __GPPO_H__
#define __GPPO_H__

#include "VectorMath.h"

struct SaveGameState
{
	PState states[2];
};

struct PState
{
	V2d position;
	V2d velocity;
	int action;
	int frame;
	bool facingRight;
	double groundSpeed;
};

#endif