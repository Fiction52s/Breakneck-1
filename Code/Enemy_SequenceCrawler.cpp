#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_SequenceCrawler.h"
#include "Actor.h"

#include "PauseMenu.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )


SequenceCrawler::SequenceCrawler(ActorParams *ap)
	:Enemy(EnemyType::EN_SEQUENCECRAWLER, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, 0, 0);

	targetPlayerIndex = 0;

	actionLength[IDLE] = 2;
	actionLength[DIG_OUT] = 12;
	
	animFactor[IDLE] = 2;
	animFactor[DIG_OUT] = 4;

	actionLength[DIG_IN] = 21;
	animFactor[DIG_IN] = 4;

	actionLength[TRIGGER_BOMBS] = 30;
	animFactor[TRIGGER_BOMBS] = 1;

	actionLength[HIT_BY_TIGER] = 30;
	animFactor[HIT_BY_TIGER] = 1;

	actionLength[DYING_BREATH] = 30;
	animFactor[DYING_BREATH] = 1;

	actionLength[DIE_BY_TIGER] = 30;
	animFactor[DIE_BY_TIGER] = 1;

	ts_dig_in = GetSizedTileset("Enemies/Bosses/Crawler/crawler_queen_dig_in_320x320.png");
	ts_dig_out = GetSizedTileset("Enemies/Bosses/Crawler/crawler_queen_dig_out_320x320.png");

	//ts_walk = GetSizedTileset("Enemies/Bosses/Coyote/coy_walk_80x80.png");

	ResetEnemy();
}

void SequenceCrawler::ResetEnemy()
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

void SequenceCrawler::DebugDraw(sf::RenderTarget *target)
{
	enemyMover.DebugDraw(target);
	Enemy::DebugDraw(target);
}

void SequenceCrawler::TriggerBombs()
{
	action = TRIGGER_BOMBS;
	frame = 0;
}


void SequenceCrawler::FrameIncrement()
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

void SequenceCrawler::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case UNDERGROUND:
			frame = 0;
			break;
		case DIG_IN:
		{
			action = UNDERGROUND;
			frame = 0;
			break;
		}
		case DIG_OUT:
			action = IDLE;
			frame = 0;
			break;
		case IDLE:
			frame = 0;
			break;
		case TRIGGER_BOMBS:
		{
			action = IDLE;
			frame = 0;
			break;
		}
		case HIT_BY_TIGER:
		{
			action = DYING_BREATH;
			frame = 0;
			break;
		}
		case DIE_BY_TIGER:
		{
			action = DEAD;
			frame = 0;
			break;
		}

		}
	}

	enemyMover.currPosInfo = currPosInfo;

	/*if (action == WALK && enemyMover.IsIdle())
	{
		action = IDLE;
		frame = 0;
	}*/
}

void SequenceCrawler::Underground()
{
	action = UNDERGROUND;
	frame = 0;
}

void SequenceCrawler::DigIn()
{
	action = DIG_IN;
	frame = 0;
}

void SequenceCrawler::DigOut()
{
	/*if (pos.x < GetPosition().x)
	{
		facingRight = false;
	}
	else
	{
		facingRight = true;
	}*/

	action = DIG_OUT;
	frame = 0;

	//enemyMover.SetModeNodeLinearConstantSpeed(pos, CubicBezier(), 3);
	//enemyMover.SetModeNodeJump(pos, 400);
}
void SequenceCrawler::UpdateEnemyPhysics()
{
	if (!enemyMover.IsIdle())
	{
		enemyMover.UpdatePhysics(numPhysSteps, slowMultiple);
		currPosInfo = enemyMover.currPosInfo;
	}
}

void SequenceCrawler::UpdateSprite()
{
	if (action == DIG_IN)
	{
		sprite.setTexture(*ts_dig_in->texture);
		ts_dig_in->SetSubRect(sprite, frame / animFactor[DIG_IN], !facingRight);
	}
	else if (action == DIG_OUT)
	{
		sprite.setTexture(*ts_dig_out->texture);
		ts_dig_out->SetSubRect(sprite, frame / animFactor[DIG_OUT], !facingRight);
	}
	else
	{
		sprite.setTexture(*ts_dig_out->texture);
		ts_dig_out->SetSubRect(sprite,11, !facingRight);
	}

	sprite.setPosition(GetPositionF() + Vector2f(0, -128));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void SequenceCrawler::EnemyDraw(sf::RenderTarget *target)
{
	if (action == UNDERGROUND)
	{
		return;
	}

	if (action == DEAD)
	{
		return;
	}

	DrawSprite(target, sprite);
}

void SequenceCrawler::HitByTiger()
{
	action = HIT_BY_TIGER;
	frame = 0;
}

void SequenceCrawler::DieByTiger()
{
	action = DIE_BY_TIGER;
	frame = 0;
}

void SequenceCrawler::Wait()
{
	action = IDLE;
	frame = 0;
	//snakePool.Reset();
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	enemyMover.Reset();
}