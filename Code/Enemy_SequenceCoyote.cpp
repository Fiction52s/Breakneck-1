#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_SequenceCoyote.h"
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


SequenceCoyote::SequenceCoyote(ActorParams *ap)
	:Enemy(EnemyType::EN_SEQUENCECOYOTE, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(WALK, 0, 0);

	targetPlayerIndex = 0;

	actionLength[WALK] = 15;
	animFactor[WALK] = 5;

	actionLength[SUMMON_SCORPION] = 30;
	animFactor[SUMMON_SCORPION] = 2;

	actionLength[BOUNCE] = 2;
	animFactor[BOUNCE] = 2;

	ts_walk = GetSizedTileset("Bosses/Coyote/coy_walk_80x80.png");

	ResetEnemy();
}

void SequenceCoyote::ResetEnemy()
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

void SequenceCoyote::DebugDraw(sf::RenderTarget *target)
{
	enemyMover.DebugDraw(target);
	Enemy::DebugDraw(target);
}

void SequenceCoyote::FrameIncrement()
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

void SequenceCoyote::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case WALK:
			frame = 0;
			break;
		case SUMMON_SCORPION:
			action = IDLE;
			frame = 0;
			break;
		}
	}

	enemyMover.currPosInfo = currPosInfo;

	if (action == WALK && enemyMover.IsIdle())
	{
		action = IDLE;
		frame = 0;
	}
}

void SequenceCoyote::Walk( V2d &pos )
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

void SequenceCoyote::SummonScorpion()
{
	action = SUMMON_SCORPION;
	frame = 0;
}

void SequenceCoyote::Bounce(V2d &pos)
{
	action = BOUNCE;
	frame = 0;
	enemyMover.SetModeNodeJump(pos, 400);
}

void SequenceCoyote::UpdateEnemyPhysics()
{
	if (!enemyMover.IsIdle())
	{
		enemyMover.UpdatePhysics(numPhysSteps, slowMultiple);
		currPosInfo = enemyMover.currPosInfo;	
	}
}

void SequenceCoyote::UpdateSprite()
{
	sprite.setTexture(*ts_walk->texture);

	if (action == WALK)
	{
		ts_walk->SetSubRect(sprite, frame / animFactor[WALK], !facingRight);
	}
	else
	{
		ts_walk->SetSubRect(sprite, 0, !facingRight);
	}
	
	sprite.setPosition(GetPositionF() + Vector2f( 0, -ts_walk->tileHeight/2 ));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void SequenceCoyote::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

void SequenceCoyote::Wait()
{
	action = WAIT;
	frame = 0;
	//snakePool.Reset();
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	enemyMover.Reset();
}