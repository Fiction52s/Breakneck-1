#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_SequenceSkeleton.h"
#include "Actor.h"

#include "PauseMenu.h"

using namespace std;
using namespace sf;


SequenceSkeleton::SequenceSkeleton(ActorParams *ap)
	:Enemy(EnemyType::EN_SEQUENCESKELETON, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, 0, 0);

	targetPlayerIndex = 0;

	actionLength[IDLE] = 2;
	actionLength[WALK] = 2;
	actionLength[LASER] = 30;
	actionLength[WIRETHROW] = 30;
	actionLength[WIRE_IDLE] = 2;
	actionLength[WIREPULL] = 5;
	//actionLength[DIG_OUT] = 12;

	animFactor[IDLE] = 2;
	animFactor[WALK] = 1;
	animFactor[LASER] = 1;
	animFactor[WIRETHROW] = 1;
	animFactor[WIRE_IDLE] = 1;
	animFactor[WIREPULL] = 1;
	//animFactor[DIG_OUT] = 4;

	extraHeight = 64;
	wireThrowSpeed = 30;

	ts = GetSizedTileset("Bosses/Skeleton/skele_128x128.png");

	SetRectColor(wireQuad, Color::Red);
	//ts_walk = GetSizedTileset("Bosses/Coyote/coy_walk_80x80.png");

	ResetEnemy();
}

void SequenceSkeleton::ResetEnemy()
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

void SequenceSkeleton::DebugDraw(sf::RenderTarget *target)
{
	enemyMover.DebugDraw(target);
	Enemy::DebugDraw(target);
}

void SequenceSkeleton::FrameIncrement()
{
	if (moveFrames > 0)
	{
		--moveFrames;
	}

	if (waitFrames > 0)
	{
		--waitFrames;
	}

	if (action == WIRETHROW)
	{
		framesThrowingWire++;
	}
	

	enemyMover.FrameIncrement();
	currPosInfo = enemyMover.currPosInfo;
}

void SequenceSkeleton::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case IDLE:
			frame = 0;
			break;
		case WALK:
			frame = 0;
			break;
		case LASER:
			action = IDLE;
			frame = 0;
			break;
		case WIRETHROW:
			frame = actionLength[WIREPULL] * animFactor[WIREPULL] - 1;
			break;
		case WIRE_IDLE:
			frame = 0;
			break;
		case WIREPULL:
			frame = actionLength[WIREPULL] * animFactor[WIREPULL] - 1;
			break;
		}
	}

	enemyMover.currPosInfo = currPosInfo;

	if ((action == WALK || action == WIREPULL ) && enemyMover.IsIdle())
	{
		action = IDLE;
		frame = 0;
	}

	UpdateWire();

	UpdateWireQuad();
}

void SequenceSkeleton::UpdateWire()
{
	if (action == WIRETHROW)
	{
		double len = length(wireAnchor - GetPosition());
		V2d along = normalize(wireAnchor - GetPosition());

		if (framesThrowingWire * wireThrowSpeed >= len)
		{
			currWirePos = wireAnchor;
			action = WIRE_IDLE;
		}
		else
		{
			currWirePos = GetPosition() + along * wireThrowSpeed * (double)framesThrowingWire;
		}
	}
}

void SequenceSkeleton::Walk(V2d &pos)
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

void SequenceSkeleton::Laser()
{
	action = LASER;
	frame = 0;
}

void SequenceSkeleton::UpdateEnemyPhysics()
{
	if (!enemyMover.IsIdle())
	{
		enemyMover.UpdatePhysics(numPhysSteps, slowMultiple);
		currPosInfo = enemyMover.currPosInfo;
	}
}

void SequenceSkeleton::WireThrow(V2d &pos)
{
	action = WIRETHROW;
	frame = 0;
	framesThrowingWire = 0;

	if (GetPosition().x > pos.x)
	{
		facingRight = true;
	}
	else if (GetPosition().x < pos.x)
	{
		facingRight = false;
	}

	wireAnchor = pos;
	currWirePos = GetPosition();
}

void SequenceSkeleton::UpdateWireQuad()
{
	V2d myPos = GetPosition();
	V2d along = normalize(currWirePos - myPos);
	V2d other(along.y, -along.x);

	double width = 5;
	wireQuad[0].position = Vector2f(myPos + other * width);
	wireQuad[1].position = Vector2f(myPos - other * width);
	wireQuad[2].position = Vector2f(currWirePos - other * width);
	wireQuad[3].position = Vector2f(currWirePos + other * width);
}

void SequenceSkeleton::WirePull()
{
	action = WIREPULL;
	frame = 0;

	enemyMover.SetModeNodeLinearConstantSpeed(wireAnchor, CubicBezier(), 30);
}

void SequenceSkeleton::UpdateSprite()
{
	sprite.setTexture(*ts->texture);

	ts->SetSubRect(sprite, 0, !facingRight);

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void SequenceSkeleton::DrawWire(sf::RenderTarget *target)
{
	target->draw(wireQuad, 4, sf::Quads);
}

void SequenceSkeleton::EnemyDraw(sf::RenderTarget *target)
{
	if (action == WIRETHROW || action == WIREPULL|| action == WIRE_IDLE)
	{
		DrawWire(target);
	}
	
	DrawSprite(target, sprite);
}

void SequenceSkeleton::Wait()
{
	action = IDLE;
	frame = 0;
	//snakePool.Reset();
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	enemyMover.Reset();
}