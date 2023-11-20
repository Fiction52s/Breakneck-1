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

	actionLength[SUMMON_SCORPION] = 17;
	animFactor[SUMMON_SCORPION] = 3;

	actionLength[SCORPION_STAND] = 2;
	animFactor[SCORPION_STAND] = 1;

	actionLength[TURN] = 2;
	animFactor[TURN] = 3;

	actionLength[JUMPSQUAT] = 12;
	animFactor[JUMPSQUAT] = 1;

	actionLength[BOUNCE] = 2;
	animFactor[BOUNCE] = 2;

	actionLength[SLEEP] = 4;
	animFactor[SLEEP] = 1;

	actionLength[HOPSQUAT] = 3;
	animFactor[HOPSQUAT] = 1;

	actionLength[HOP_BACK] = 2;
	animFactor[HOP_BACK] = 2;

	actionLength[RUN] = 15;
	animFactor[RUN] = 2;

	ts_coy = GetSizedTileset("Enemies/Bosses/Coyote/coy_old_80x80.png");
	ts_scorp = GetSizedTileset("Enemies/Bosses/Coyote/coy_scorp_160x128.png");

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
		case RUN:
			frame = 0;
			break;
		case SUMMON_SCORPION:
			action = SCORPION_STAND;
			frame = 0;
			break;
		case SCORPION_STAND:
			frame = 0;
			break;
		case TURN:
			action = JUMPSQUAT;
			frame = 0;
			facingRight = !facingRight;
			break;
		case JUMPSQUAT:
			action = BOUNCE;
			frame = 0;
			enemyMover.SetModeNodeJump(bouncePos, 400);
			break;
		case SLEEP:
			frame = 0;
			break;
		case HOPSQUAT:
			action = HOP_BACK;
			frame = 0;
			enemyMover.SetModeNodeJump(hopPos, 20, 10);
			break;
		}
	}

	enemyMover.currPosInfo = currPosInfo;

	if ((action == WALK || action == BOUNCE || action == HOP_BACK || action == RUN ) && enemyMover.IsIdle())
	{
		action = IDLE;
		frame = 0;
	}
}

void SequenceCoyote::HopBack( V2d &pos )
{
	action = HOPSQUAT;
	frame = 0;

	if (pos.x < GetPosition().x)
	{
		facingRight = true;
	}
	else
	{
		facingRight = false;
	}

	hopPos = pos;
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

void SequenceCoyote::Run(V2d &pos)
{
	if (pos.x < GetPosition().x)
	{
		facingRight = false;
	}
	else
	{
		facingRight = true;
	}

	action = RUN;
	frame = 0;

	enemyMover.SetModeNodeLinearConstantSpeed(pos, CubicBezier(), 10);
	//enemyMover.SetModeNodeJump(pos, 400);
}

void SequenceCoyote::SummonScorpion()
{
	action = SUMMON_SCORPION;
	frame = 0;
}

void SequenceCoyote::Sleep()
{
	action = SLEEP;
	frame = 0;
}

void SequenceCoyote::Bounce(V2d &pos)
{
	/*if (pos.x >= GetPosition().x)
	{
		facingRight = true;
	}
	else
	{
		facingRight = false;
	}*/

	action = TURN;
	//action = JUMPSQUAT;//BOUNCE;
	frame = 0;
	bouncePos = pos;
	
	//enemyMover.SetModeNodeJump(pos, 400);
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
	sprite.setTexture(*ts_coy->texture);
	scorpSprite.setTexture(*ts_scorp->texture);

	int coyTile = 0;
	switch (action)
	{
	case IDLE:
		coyTile = 15;
		break;
	case WAIT:
		coyTile = 15;
		break;
	case WALK:
		coyTile = frame / animFactor[WALK];
		break;
	case RUN:
		coyTile = frame / animFactor[RUN];
		break;
	case SUMMON_SCORPION:
		coyTile = 15;
		break;
	case SCORPION_STAND:
		coyTile = 15;
		break;
	case TURN:
		coyTile = 17 + frame / animFactor[TURN];
		break;
	case JUMPSQUAT:
		coyTile = 19;
		break;
	case BOUNCE:
		coyTile = 20;
		break;
	case HOPSQUAT:
		coyTile = 19;
		break;
	case HOP_BACK:
		coyTile = 20;
		break;
	}

	ts_coy->SetSubRect(sprite, coyTile, !facingRight);

	int scorpTile = 0;
	switch (action)
	{
	case IDLE:
		break;
	case WAIT:
		break;
	case WALK:
		break;
	case SUMMON_SCORPION:
		scorpTile = frame / animFactor[SUMMON_SCORPION];
		break;
	case SCORPION_STAND:
		scorpTile = 17;
		break;
	case TURN:
		if (frame / animFactor[TURN] == 0)
		{
			scorpTile = 17;
		}
		else
		{
			scorpTile = 18;
		}
		break;
	case JUMPSQUAT:
		scorpTile = 15;
		break;
	case BOUNCE:
		scorpTile = 19;
		break;
	}

	ts_scorp->SetSubRect(scorpSprite, scorpTile, !facingRight);

	scorpSprite.setPosition(GetPositionF() + Vector2f( 0, -42 ));
	scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2, scorpSprite.getLocalBounds().height / 2);

	sprite.setPosition(GetPositionF() + Vector2f( 0, -ts_coy->tileHeight/2 + 8));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void SequenceCoyote::EnemyDraw(sf::RenderTarget *target)
{
	if (action == SUMMON_SCORPION || action == SCORPION_STAND || action == TURN 
		|| action == BOUNCE || action == JUMPSQUAT )
	{
		DrawSprite(target, scorpSprite);
	}
	
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