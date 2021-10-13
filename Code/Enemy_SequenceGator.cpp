#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_SequenceGator.h"
#include "Actor.h"

#include "PauseMenu.h"

using namespace std;
using namespace sf;


SequenceGator::SequenceGator(ActorParams *ap)
	:Enemy(EnemyType::EN_SEQUENCEGATOR, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, 0, 0);

	targetPlayerIndex = 0;

	actionLength[IDLE] = 2;
	actionLength[SUPER_ORB] = -1;
	actionLength[RETRACT_SUPER_ORB] = -1;
	actionLength[HOLD_SUPER_ORB] = -1;
	//actionLength[DIG_OUT] = 12;

	animFactor[IDLE] = 2;
	//animFactor[DIG_OUT] = 4;

	ts = GetSizedTileset("Bosses/Gator/dominance_384x384.png");

	//ts_walk = GetSizedTileset("Bosses/Coyote/coy_walk_80x80.png");

	ResetEnemy();
}

void SequenceGator::ResetEnemy()
{
	enemyMover.Reset();
	superOrbPool.Reset();
	facingRight = true;

	action = IDLE;
	frame = 0;

	waitFrames = 0;
	moveFrames = 0;

	currPosInfo.SetAerial();
	enemyMover.currPosInfo = currPosInfo;

	UpdateSprite();
}

void SequenceGator::DebugDraw(sf::RenderTarget *target)
{
	enemyMover.DebugDraw(target);
	Enemy::DebugDraw(target);
}

void SequenceGator::FrameIncrement()
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

void SequenceGator::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case IDLE:
			frame = 0;
			break;
		}
	}

	switch (action)
	{
		case SUPER_ORB:
		{
			if (frame == 30)
			{
				superOrbPool.Throw(GetPosition(), V2d(0, -1));
			}
			else if (frame > 30 && superOrbPool.IsIdle())
			{
				action = RETRACT_SUPER_ORB;
				frame = 0;
				superOrbPool.ReturnToGator(GetPosition() + V2d(-150, 0));
			}
			break;
		}
		case RETRACT_SUPER_ORB:
		{
			if (superOrbPool.IsIdle())
			{
				action = HOLD_SUPER_ORB;
				frame = 0;
			}
			break;
		}
	}

	enemyMover.currPosInfo = currPosInfo;

	/*if (action == WALK && enemyMover.IsIdle())
	{
	action = IDLE;
	frame = 0;
	}*/
}

void SequenceGator::ThrowSuperOrb()
{
	action = SUPER_ORB;
	frame = 0;
}

void SequenceGator::UpdateEnemyPhysics()
{
	if (!enemyMover.IsIdle())
	{
		enemyMover.UpdatePhysics(numPhysSteps, slowMultiple);
		currPosInfo = enemyMover.currPosInfo;
	}
}

void SequenceGator::UpdateSprite()
{
	sprite.setTexture(*ts->texture);

	ts->SetSubRect(sprite, 0, !facingRight);

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void SequenceGator::EnemyDraw(sf::RenderTarget *target)
{
	superOrbPool.Draw(target);
	DrawSprite(target, sprite);
}

void SequenceGator::Wait()
{
	action = IDLE;
	frame = 0;
	//snakePool.Reset();
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	enemyMover.Reset();
}