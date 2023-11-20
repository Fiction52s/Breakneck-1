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
	actionLength[BEAT_UP_KIN] = 60;
	actionLength[SMASH_WITH_ORB] = -1;
	actionLength[KICKED_BY_BIRD] = 30;
	actionLength[DEAD_BODY] = 10;
	//actionLength[DIG_OUT] = 12;

	animFactor[IDLE] = 2;
	//animFactor[DIG_OUT] = 4;

	ts = GetSizedTileset("Enemies/Bosses/Gator/dominance_384x384.png");

	superOrbOffset = V2d(-150, 0);

	//ts_walk = GetSizedTileset("Enemies/Bosses/Coyote/coy_walk_80x80.png");

	ResetEnemy();
}

void SequenceGator::ResetEnemy()
{
	enemyMover.Reset();
	superOrbPool.Reset();
	facingRight = true;

	superOrb = NULL;
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
		case BEAT_UP_KIN:
			frame = 0;
			//action = HOLD_SUPER_ORB;
			//frame = 0;
			break;
		case KICKED_BY_BIRD:
			action = DEAD_BODY;
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
				superOrb = superOrbPool.Throw(GetPosition(), V2d(0, -1));
			}
			else if (frame > 30 && superOrb->IsIdle())
			{
				action = RETRACT_SUPER_ORB;
				frame = 0;
				superOrb->ReturnToGator(GetPosition() + superOrbOffset);
			}
			break;
		}
		case RETRACT_SUPER_ORB:
		{
			if (superOrb->IsIdle())
			{
				action = HOLD_SUPER_ORB;
				frame = 0;
			}
			break;
		}
		case FLOAT_WITH_ORB:
		{
			if (enemyMover.IsIdle())
			{
				action = HOLD_SUPER_ORB;
				frame = 0;
			}

			superOrb->SetPos(GetPosition() + superOrbOffset);
			break;
		}
		case SMASH_WITH_ORB:
		{
			if (enemyMover.IsIdle())
			{
				action = HOLD_SUPER_ORB;
				frame = 0;
			}

			superOrb->SetPos(GetPosition() + superOrbOffset);
			break;
		}
		case BEAT_UP_KIN:
		{
			if (frame == 0)
			{
				sess->cam.SetRumble(5, 5, 10);
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

void SequenceGator::KickedByBird()
{
	action = KICKED_BY_BIRD;
	frame = 0;
}

void SequenceGator::LaunchSuperOrb(V2d &pos, double extraHeight, double speed)
{
	superOrb->Launch(pos, extraHeight, speed);
}

void SequenceGator::BeatUpKin()
{
	action = BEAT_UP_KIN;
	frame = 0;
}

void SequenceGator::FloatWithOrb( V2d &pos, double speed  )
{
	action = FLOAT_WITH_ORB;
	frame = 0;
	enemyMover.SetModeNodeLinearConstantSpeed(pos, CubicBezier(), speed);
}

void SequenceGator::SmashWithOrb(V2d &pos, double speed)
{
	action = SMASH_WITH_ORB;
	frame = 0;
	enemyMover.SetModeNodeLinearConstantSpeed(pos, CubicBezier(), speed);
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