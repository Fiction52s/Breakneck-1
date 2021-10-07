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
	actionLength[WALK] = 18;
	actionLength[JUMPSQUAT] = 2;
	actionLength[HOP] = 2;
	actionLength[LAND] = 2;
	actionLength[CHARGELASER] = 7;
	actionLength[LASER] = 12;
	actionLength[LASER_IDLE] = 2;
	actionLength[WIRETHROW] = 30;
	actionLength[WIRE_IDLE] = 2;
	actionLength[WIREPULL] = 5;
	//actionLength[DIG_OUT] = 12;

	animFactor[IDLE] = 2;
	animFactor[WALK] = 3;
	animFactor[JUMPSQUAT] = 3;//1
	animFactor[HOP] = 1;
	animFactor[LAND] = 3;
	animFactor[CHARGELASER] = 3;
	animFactor[LASER] = 4;
	animFactor[WIRETHROW] = 1;
	animFactor[WIRE_IDLE] = 1;
	animFactor[WIREPULL] = 1;
	animFactor[LASER_IDLE] = 1;
	//animFactor[DIG_OUT] = 4;

	extraHeight = 64;
	wireThrowSpeed = 30;
	laserSpeed = 6;

	laserAnimFrames = 20;
	laserAnimFactor = 2;

	ts_bullet = GetSizedTileset("Bosses/Skeleton/skele_bullet_128x128.png");

	ts_laser = GetSizedTileset("Bosses/Skeleton/skele_laser_160x128.png");

	ts = GetSizedTileset("Bosses/Skeleton/skele_128x128.png");

	ts_walk = GetSizedTileset("Bosses/Skeleton/skele_walk_128x128.png");

	ts_hop = GetSizedTileset("Bosses/Skeleton/skele_hop_128x128.png");

	SetRectColor(wireQuad, Color::Red);

	//SetRectColor(laserQuad, Color::Magenta);
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

	/*if (action == LASER)
	{

	}*/
	
	if (action == LASER)
	{
		laserFrame++;
		if (laserFrame == laserAnimFrames * laserAnimFactor)
		{
			laserFrame = 0;
		}
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
		case JUMPSQUAT:
		{
			action = HOP;
			frame = 0;
			int hopFrames = enemyMover.SetModeNodeJump(hopTarget, 16, 5);
			animFactor[HOP] = hopFrames / 2;
		}
			
			break;
		case HOP:
			frame = 0;
			break;
		case LAND:
			action = IDLE;
			frame = 0;
			break;
		case CHARGELASER:
			frame = 0;
			break;
		case LASER:
			action = LASER_IDLE;
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
	else if (action == HOP && enemyMover.IsIdle())
	{
		action = LAND;
		frame = 0;
	}

	if (action == LASER || action == LASER_IDLE)
	{
		laserPos += laserVel;
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

void SequenceSkeleton::Laser( V2d &pos)
{
	action = LASER;
	frame = 0;
	laserFrame = 0;
	laserPos = GetPosition();

	V2d dir = normalize(pos - laserPos);
	laserVel = dir * laserSpeed;
}

void SequenceSkeleton::ChargeLaser()
{
	action = CHARGELASER;
	frame = 0;
}

void SequenceSkeleton::HopDown(V2d &pos)
{
	action = JUMPSQUAT;
	frame = 0;

	if (pos.x < GetPosition().x)
	{
		facingRight = false;
	}
	else if (pos.x > GetPosition().x)
	{
		facingRight = true;
	}

	hopTarget = pos + V2d(0, -extraHeight);
	
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
	if (action == LASER || action == CHARGELASER || action == LASER_IDLE)
	{
		sprite.setTexture(*ts_laser->texture);

		int tile = 0;
		if (action == CHARGELASER)
		{
			tile = frame / animFactor[CHARGELASER] + 1;
		}
		else if (action == LASER)
		{
			tile = frame / animFactor[LASER] + 8;
		}
		else if (action == LASER_IDLE)
		{
			tile = 0;
		}

		ts_laser->SetSubRect(sprite, tile, !facingRight);
	}
	else if (action == WALK)
	{
		sprite.setTexture(*ts_walk->texture);

		ts_walk->SetSubRect(sprite, frame / animFactor[WALK], !facingRight);
	}
	else if (action == JUMPSQUAT || action == HOP || action == LAND)
	{
		int extra = 0;
		switch (action)
		{
		case JUMPSQUAT:
			extra = 0;
			break;
		case HOP:
			extra = 2;
			break;
		case LAND:
			extra = 4;
			break;
		}
		sprite.setTexture(*ts_hop->texture);
		ts_hop->SetSubRect(sprite, frame / animFactor[action] + extra, !facingRight);
	}
	else
	{
		sprite.setTexture(*ts_laser->texture);
		ts_laser->SetSubRect(sprite, 0, !facingRight);
	}

	

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);

	if (action == LASER || action == LASER_IDLE)
	{
		ts_bullet->SetQuadSubRect(laserQuad, laserFrame / laserAnimFactor);
		SetRectCenter(laserQuad, 128, 128, Vector2f(laserPos));
	}
}

void SequenceSkeleton::Idle()
{
	action = IDLE;
	frame = 0;
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
	
	if (action == LASER || action == LASER_IDLE)
	{
		target->draw(laserQuad, 4, sf::Quads, ts_bullet->texture);
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