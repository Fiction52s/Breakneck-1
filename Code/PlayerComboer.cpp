#include "Session.h"
#include "Actor.h"

#include "PlayerComboer.h"

using namespace sf;

PlayerComboer::PlayerComboer()
{
	sess = Session::GetSession();
	Reset();

	predictCircle.setFillColor(Color::Red);
	predictCircle.setRadius(20);
	predictCircle.setOrigin(predictCircle.getLocalBounds().width / 2,
		predictCircle.getLocalBounds().height / 2);
}

void PlayerComboer::Reset()
{
	predict = false;
	targetPos = V2d(0, 0);
	hasPredictedPos = false;
}

void PlayerComboer::CalcTargetAfterHit(int pIndex)
{
	sess->ForwardSimulatePlayer(pIndex, sess->GetPlayer(pIndex)->hitstunFrames);
	targetPos = sess->GetPlayerPos(pIndex);
	sess->RevertSimulatedPlayer(pIndex);
	predictCircle.setPosition(Vector2f(targetPos));
	hasPredictedPos = true;
}

const V2d & PlayerComboer::GetTargetPos()
{
	return targetPos;
}

void PlayerComboer::PredictNextFrame()
{
	predict = true;
}

bool PlayerComboer::CanPredict( int pIndex )
{
	Actor *targetPlayer = sess->GetPlayer(pIndex);
	//be careful, because if you hit the player early,
	//this will trigger the hitoutofhitstunlastframe
	//and cause you to not chase correctly
	return predict;// || targetPlayer->hitOutOfHitstunLastFrame;
}

void PlayerComboer::UpdatePreFrameCalculations(int pIndex)
{
	//if ( CanPredict(pIndex) )
	//{
		CalcTargetAfterHit(pIndex);
		predict = false;
	//}
}

void PlayerComboer::DebugDraw(sf::RenderTarget *target)
{
	if (hasPredictedPos)
	{
		target->draw(predictCircle);
	}
}