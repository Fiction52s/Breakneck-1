#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_SequenceTiger.h"
#include "Actor.h"

#include "PauseMenu.h"

using namespace std;
using namespace sf;


SequenceTiger::SequenceTiger(ActorParams *ap)
	:Enemy(EnemyType::EN_SEQUENCETIGER, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, 0, 0);

	targetPlayerIndex = 0;

	actionLength[IDLE] = 2;
	actionLength[WALK] = 2;
	actionLength[BREATHE] = 10;
	actionLength[LOOK_UP] = 10;
	actionLength[CARRIED_BY_BIRD] = -1;
	actionLength[HIT_BY_MIND_CONTROL] = 2;
	actionLength[INJURED_ROAR] = 40;
	actionLength[FALL] = 1;
	actionLength[FALL_LAND_IDLE] = 1;
	actionLength[PUT_BIRD_ON_BACK] = 10;
	actionLength[CARRY_BIRD] = 2;
	//actionLength[DIG_OUT] = 12;

	animFactor[IDLE] = 2;
	animFactor[WALK] = 1;
	animFactor[BREATHE] = 1;
	animFactor[LOOK_UP] = 1;
	animFactor[CARRIED_BY_BIRD] = 1;
	animFactor[HIT_BY_MIND_CONTROL] = 1;
	animFactor[INJURED_ROAR] = 1;
	animFactor[FALL] = 1;
	animFactor[FALL_LAND_IDLE] = 1;
	animFactor[PUT_BIRD_ON_BACK] = 1;
	animFactor[CARRY_BIRD] = 1;

	//animFactor[DIG_OUT] = 4;

	ts = GetSizedTileset("Bosses/Crawler/crawler_queen_dig_out_320x320.png");

	//ts_walk = GetSizedTileset("Bosses/Coyote/coy_walk_80x80.png");

	ResetEnemy();
}

void SequenceTiger::ResetEnemy()
{
	enemyMover.Reset();
	facingRight = true;

	action = IDLE;
	frame = 0;

	waitFrames = 0;
	moveFrames = 0;

	currPosInfo.SetAerial();
	enemyMover.currPosInfo = currPosInfo;

	UpdateSprite();
}

void SequenceTiger::DebugDraw(sf::RenderTarget *target)
{
	enemyMover.DebugDraw(target);
	Enemy::DebugDraw(target);
}

void SequenceTiger::Walk(V2d &pos)
{
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

void SequenceTiger::FrameIncrement()
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

void SequenceTiger::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case IDLE:
			frame = 0;
			break;
		case LOOK_UP:
			frame = 0;
			break;
		case BREATHE:
			frame = 0;
			break;
		case HIT_BY_MIND_CONTROL:
			frame = 0;
			break;
		case INJURED_ROAR:
			action = BREATHE;
			frame = 0;
			break;
		case FALL:
			frame = 0;
			break;
		case FALL_LAND_IDLE:
			frame = 0;
			break;
		case PUT_BIRD_ON_BACK:
			action = CARRY_BIRD;
			frame = 0;
			break;
		case CARRY_BIRD:
			frame = 0;
			break;
		}
	}

	enemyMover.currPosInfo = currPosInfo;

	if ((action == WALK || action == CARRIED_BY_BIRD ) && enemyMover.IsIdle())
	{
		action = IDLE;
		frame = 0;
	}
	else if (action == FALL && enemyMover.IsIdle())
	{
		action = FALL_LAND_IDLE;
		frame = 0;
	}
}

void SequenceTiger::UpdateEnemyPhysics()
{
	if (!enemyMover.IsIdle())
	{
		enemyMover.UpdatePhysics(numPhysSteps, slowMultiple);
		currPosInfo = enemyMover.currPosInfo;
	}
}

void SequenceTiger::UpdateSprite()
{
	sprite.setTexture(*ts->texture);

	ts->SetSubRect(sprite, 11, !facingRight);

	sprite.setPosition(GetPositionF() + Vector2f(0, -128));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void SequenceTiger::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

void SequenceTiger::Wait()
{
	action = IDLE;
	frame = 0;
	//snakePool.Reset();
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	enemyMover.Reset();
}

void SequenceTiger::Fall(double y)
{
	assert(y > GetPosition().y);

	action = FALL;
	frame = 0;

	V2d nodePos(GetPosition().x, y);
	enemyMover.SetModeNodeProjectile(nodePos, V2d(0, .5), 0);
}

void SequenceTiger::LookUp()
{
	action = LOOK_UP;
	frame = 0;
}

void SequenceTiger::PutBirdOnBack()
{
	action = PUT_BIRD_ON_BACK;
	frame = 0;
}

void SequenceTiger::CarryBirdAway(V2d &pos)
{
	action = CARRY_BIRD;
	frame = 0;

	if (pos.x > GetPosition().x)
	{
		facingRight = true;
	}
	else if (pos.x < GetPosition().x)
	{
		facingRight = false;
	}

	enemyMover.SetModeNodeLinearConstantSpeed(pos, CubicBezier(), 10);
}

void SequenceTiger::InjuredRoar()
{
	action = INJURED_ROAR;
	frame = 0;
}

void SequenceTiger::Carried(V2d &pos)
{
	action = CARRIED_BY_BIRD;
	frame = 0;
	enemyMover.SetModeNodeLinearConstantSpeed(pos, CubicBezier(), 10);
}

void SequenceTiger::Breathe()
{
	action = BREATHE;
	frame = 0;
}

void SequenceTiger::HitByMindControl()
{
	action = HIT_BY_MIND_CONTROL;
	frame = 0;
}