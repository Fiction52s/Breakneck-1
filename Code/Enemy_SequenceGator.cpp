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
	//actionLength[DIG_OUT] = 12;

	animFactor[IDLE] = 2;
	//animFactor[DIG_OUT] = 4;

	ts = GetSizedTileset("Bosses/Crawler/crawler_queen_dig_out_320x320.png");

	//ts_walk = GetSizedTileset("Bosses/Coyote/coy_walk_80x80.png");

	ResetEnemy();
}

void SequenceGator::ResetEnemy()
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

	enemyMover.currPosInfo = currPosInfo;

	/*if (action == WALK && enemyMover.IsIdle())
	{
	action = IDLE;
	frame = 0;
	}*/
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

	ts->SetSubRect(sprite, 11, !facingRight);

	sprite.setPosition(GetPositionF() + Vector2f(0, -128));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void SequenceGator::EnemyDraw(sf::RenderTarget *target)
{
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