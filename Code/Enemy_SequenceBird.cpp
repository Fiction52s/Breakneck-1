#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_SequenceBird.h"
#include "Actor.h"
#include "Enemy_SequenceTiger.h"
#include "PauseMenu.h"

using namespace std;
using namespace sf;


SequenceBird::SequenceBird(ActorParams *ap)
	:Enemy(EnemyType::EN_SEQUENCEBIRD, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, 0, 0);

	targetPlayerIndex = 0;

	actionLength[IDLE] = 2;
	actionLength[BREATHE] = 2;
	actionLength[WALK] = 4;
	actionLength[FLY] = 10;
	actionLength[FLY_IDLE] = 10;
	actionLength[PICKUP_TIGER] = 10;
	actionLength[FLY_HOLDING_TIGER] = 10;
	actionLength[FLY_WITH_SKELETON] = 10;
	actionLength[HIT_BY_MIND_CONTROL] = 10;
	actionLength[PUT_ON_TIGER] = 10;
	actionLength[RIDE_TIGER] = 2;

	actionLength[BREAK_BUBBLE] = 30;
	actionLength[BUBBLE_BREAK_IDLE] = 2;
	actionLength[SUPER_KICK] = 2;
	actionLength[POST_SUPER_KICK_LIE] = 2;

	//actionLength[DIG_OUT] = 12;

	animFactor[IDLE] = 2;
	animFactor[BREATHE] = 1;
	animFactor[WALK] = 8;
	animFactor[FLY] = 1;
	animFactor[FLY_IDLE] = 1;
	animFactor[PICKUP_TIGER] = 1;
	animFactor[FLY_HOLDING_TIGER] = 1;
	animFactor[FLY_WITH_SKELETON] = 1;
	animFactor[HIT_BY_MIND_CONTROL] = 1;
	animFactor[PUT_ON_TIGER] = 1;
	animFactor[RIDE_TIGER] = 1;

	animFactor[BREAK_BUBBLE] = 1;
	animFactor[BUBBLE_BREAK_IDLE] = 1;
	animFactor[SUPER_KICK] = 1;
	animFactor[POST_SUPER_KICK_LIE] = 1;
	//animFactor[DIG_OUT] = 4;

	extraHeight = 76;

	ts = GetSizedTileset("Bosses/Bird/bird_walk_160x160.png");

	ResetEnemy();
}

void SequenceBird::ResetEnemy()
{
	enemyMover.Reset();
	facingRight = true;

	action = IDLE;
	frame = 0;

	waitFrames = 0;
	moveFrames = 0;

	currPosInfo.SetAerial();
	currPosInfo.position += V2d(0, -extraHeight);
	enemyMover.currPosInfo = currPosInfo;

	UpdateSprite();
}

void SequenceBird::Breathe()
{
	action = BREATHE;
	frame = 0;
	//facingRight = false;
}

void SequenceBird::HitByMindControl()
{
	action = HIT_BY_MIND_CONTROL;
	frame = 0;
}

void SequenceBird::Walk(V2d &pos)
{
	pos += V2d(0, -extraHeight);

	if (pos.x < GetPosition().x)
	{
		facingRight = false;
	}
	else
	{
		facingRight = true;
	}

	action = WALK;
	frame = 0;

	enemyMover.SetModeNodeLinearConstantSpeed(pos, CubicBezier(), 3);
	//enemyMover.SetModeNodeJump(pos, 400);
}

void SequenceBird::DebugDraw(sf::RenderTarget *target)
{
	enemyMover.DebugDraw(target);
	Enemy::DebugDraw(target);
}

void SequenceBird::FrameIncrement()
{
	if (moveFrames > 0)
	{
		--moveFrames;
	}

	if (waitFrames > 0)
	{
		--waitFrames;
	}

	enemyMover.FrameIncrement();
	currPosInfo = enemyMover.currPosInfo;
}

void SequenceBird::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case IDLE:
			frame = 0;
			break;
		case BREATHE:
			frame = 0;
			break;
		case WALK:
			frame = 0;
			break;
		case FLY:
			frame = 0;
			break;
		case FLY_IDLE:
			frame = 0;
			break;
		case PICKUP_TIGER:
			frame = 0;
			break;
		case FLY_HOLDING_TIGER:
			frame = 0;
			break;
		case FLY_WITH_SKELETON:
			frame = 0;
			break;
		case HIT_BY_MIND_CONTROL:
			frame = 0;
			break;
		case PUT_ON_TIGER:
			action = RIDE_TIGER;
			frame = 0;
			break;
		case RIDE_TIGER:
			frame = 0;
			break;
		case BREAK_BUBBLE:
			action = BUBBLE_BREAK_IDLE;
			frame = 0;
			break;
		case BUBBLE_BREAK_IDLE:
			frame = 0;
			break;
		case SUPER_KICK:
			frame = 0;
			break;
		case POST_SUPER_KICK_LIE:
			frame = 0;
			break;
		}
	}

	enemyMover.currPosInfo = currPosInfo;

	if (enemyMover.IsIdle())
	{
		if (action == WALK)
		{
			action = IDLE;
			frame = 0;
		}
		else if (action == FLY || action == FLY_HOLDING_TIGER || action == FLY_WITH_SKELETON)
		{
			action = FLY_IDLE;
			frame = 0;
		}
		else if (action == SUPER_KICK)
		{
			action = POST_SUPER_KICK_LIE;
			frame = 0;
		}
	}
}

void SequenceBird::UpdateEnemyPhysics()
{
	if (!enemyMover.IsIdle())
	{
		enemyMover.UpdatePhysics(numPhysSteps, slowMultiple);
		currPosInfo = enemyMover.currPosInfo;
	}
}

void SequenceBird::UpdateSprite()
{
	if (action == RIDE_TIGER) //update post physics
	{
		currPosInfo.position = seqTiger->GetPosition() + offsetFromTiger;
		enemyMover.currPosInfo = currPosInfo;
	}

	sprite.setTexture(*ts->texture);

	int tile = 0;
	switch (action)
	{
	case WALK:
	{
		tile = frame / animFactor[WALK] + 1;
		break;
	}
	}

	ts->SetSubRect(sprite, tile, !facingRight);

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void SequenceBird::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

void SequenceBird::Wait()
{
	action = IDLE;
	frame = 0;
	//snakePool.Reset();
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	enemyMover.Reset();
}

void SequenceBird::SuperKick(V2d &pos)
{
	action = SUPER_KICK;
	frame = 0;

	if (pos.x > GetPosition().x)
	{
		facingRight = true;
	}
	else if (pos.x < GetPosition().x)
	{
		facingRight = false;
	}

	enemyMover.SetModeNodeLinearConstantSpeed(pos, CubicBezier(), 15);
}

void SequenceBird::RideTiger(SequenceTiger *p_seqTiger)
{
	seqTiger = p_seqTiger;
	action = RIDE_TIGER;
	frame = 0;

	offsetFromTiger = GetPosition() - seqTiger->GetPosition();
}

void SequenceBird::BreakFreeFromBubble()
{
	action = BREAK_BUBBLE;
	frame = 0;
}

void SequenceBird::Fly(V2d &pos)
{
	if (pos.x < GetPosition().x)
	{
		facingRight = false;
	}
	else
	{
		facingRight = true;
	}

	action = FLY;
	frame = 0;

	enemyMover.SetModeNodeLinearConstantSpeed(pos, CubicBezier(), 10);
}

void SequenceBird::PickupTiger()
{
	action = PICKUP_TIGER;
	frame = 0;
}

void SequenceBird::FlyAwayWithSkeleton(V2d &pos, double speed)
{
	if (pos.x < GetPosition().x)
	{
		facingRight = false;
	}
	else
	{
		facingRight = true;
	}

	action = FLY_WITH_SKELETON;
	frame = 0;

	enemyMover.SetModeNodeLinearConstantSpeed(pos, CubicBezier(), speed);
}

void SequenceBird::FlyAwayWithTiger(V2d &pos)
{
	if (pos.x < GetPosition().x)
	{
		facingRight = false;
	}
	else
	{
		facingRight = true;
	}

	action = FLY_HOLDING_TIGER;
	frame = 0;

	enemyMover.SetModeNodeLinearConstantSpeed(pos, CubicBezier(), 10);
}