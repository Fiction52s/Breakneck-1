#ifndef __PLAYERCOMBOER_H__
#define __PLAYERCOMBOER_H__

#include "VectorMath.h"

struct Session;

struct PlayerComboer
{
	Session *sess;
	V2d targetPos;
	sf::CircleShape predictCircle;
	bool predict;
	bool hasPredictedPos;

	PlayerComboer();
	void Reset();
	const V2d & GetTargetPos();
	void PredictNextFrame();
	void CalcTargetAfterHit(int pIndex);
	void UpdatePreFrameCalculations(int pIndex);
	void DebugDraw(sf::RenderTarget *target);
	bool CanPredict( int pIndex );
};

#endif