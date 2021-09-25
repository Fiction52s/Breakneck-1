#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_SequenceBird.h"
#include "Actor.h"

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
	//actionLength[DIG_OUT] = 12;

	animFactor[IDLE] = 2;
	animFactor[BREATHE] = 1;
	//animFactor[DIG_OUT] = 4;

	ts = GetSizedTileset("Bosses/Bird/intro_256x256.png");

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

	UpdateSprite();
}

void SequenceBird::Breathe()
{
	action = BREATHE;
	frame = 0;
	//facingRight = false;
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
		}
	}

	enemyMover.currPosInfo = currPosInfo;

	/*if (action == WALK && enemyMover.IsIdle())
	{
	action = IDLE;
	frame = 0;
	}*/
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
	sprite.setTexture(*ts->texture);

	ts->SetSubRect(sprite, 0, !facingRight);

	sprite.setPosition(GetPositionF());// +Vector2f(0, -128));
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