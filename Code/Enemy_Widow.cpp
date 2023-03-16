#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Widow.h"
#include "EditorTerrain.h"
#include "Actor.h"

using namespace std;
using namespace sf;

Widow::Widow(ActorParams *ap)
	:Enemy(EnemyType::EN_WIDOW, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	SetLevel(ap->GetLevel());


	actionLength[IDLE] = 1;
	actionLength[MOVE] = 30;
	actionLength[WAIT] = 1;
	actionLength[JUMP] = 1;
	actionLength[LAND] = 2;

	gravity = .6;
	maxGroundSpeed = 20;
	attentionRadius = 1000;

	maxFallSpeed = 25;

	CreateSurfaceMover(startPosInfo, 32, this);
	surfaceMover->SetSpeed(0);

	ts = GetSizedTileset("Enemies/W1/crawler_160x160.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 10;
	hitboxInfo->kbDir = normalize(V2d(1, -.3));
	hitboxInfo->hType = HitboxInfo::ORANGE;

	BasicCircleHitBodySetup(32);
	BasicCircleHurtBodySetup(32);
	hitBody.hitboxInfo = hitboxInfo;

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(0);
	cutObject->SetSubRectBack(1);
	cutObject->SetScale(scale);

	cwCircle.setFillColor(Color::Red);
	cwCircle.setRadius(20);
	cwCircle.setOrigin(cwCircle.getLocalBounds().width / 2,
		cwCircle.getLocalBounds().height / 2);

	ccwCircle.setFillColor(Color::Cyan);
	ccwCircle.setRadius(20);
	ccwCircle.setOrigin(ccwCircle.getLocalBounds().width / 2,
		ccwCircle.getLocalBounds().height / 2);

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this, BasicBullet::TURTLE, 12, 1, GetPosition(), V2d(1, 0), 0, 90, false);
	launchers[0]->SetBulletSpeed(15);
	launchers[0]->hitboxInfo->hType = HitboxInfo::ORANGE;
	launchers[0]->Reset();

	distToCheck = 100;

	ResetEnemy();
}

Widow::~Widow()
{
}

void Widow::ResetEnemy()
{
	rayCastInfo.Reset();

	surfaceMover->Set(startPosInfo);
	surfaceMover->SetSpeed(0);
	surfaceMover->ClearAirForces();

	action = IDLE;
	frame = 0;


	data.fireCounter = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

void Widow::SetLevel(int lev)
{
	level = lev;

	switch (level)
	{
	case 1:
		scale = 1.0;
		break;
	case 2:
		scale = 2.0;
		maxHealth += 2;
		break;
	case 3:
		scale = 3.0;
		maxHealth += 5;
		break;
	}
}

void Widow::UpdateHitboxes()
{
	Edge *ground = surfaceMover->ground;
	if (ground != NULL)
	{
		V2d knockbackDir(1, -1);
		knockbackDir = normalize(knockbackDir);
		if (surfaceMover->groundSpeed > 0)
		{
			hitboxInfo->kbDir = knockbackDir;
			hitboxInfo->knockback = 15;
		}
		else
		{
			hitboxInfo->kbDir = V2d(-knockbackDir.x, knockbackDir.y);
			hitboxInfo->knockback = 15;
		}
	}

	BasicUpdateHitboxes();
}

void Widow::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case MOVE:
			if (!TryMove())
			{
				action = IDLE;
			}
			break;
		case WAIT:
			break;
		case JUMP:
			break;
		case LAND:
			action = MOVE;
			break;
		case ATTACK:
			break;
		}
	}
}

void Widow::CheckDist(double dist)
{
	Edge *g = surfaceMover->ground;

	assert(g != NULL);

	//double factor = slowMultiple * (double)numPhysSteps;
	double movement = dist;

	double quant = surfaceMover->edgeQuantity;

	while (!approxEquals(movement, 0))
	{
		double gLen = g->GetLength();

		if (movement > 0)
		{
			double extra = quant + movement - gLen;

			if (extra > 0)
			{
				movement -= gLen - quant;
				g = g->GetNextEdge();

				quant = 0;
			}
			else
			{
				quant += movement;
				movement = 0;
			}
		}
		else
		{
			double extra = quant + movement;

			if (extra < 0)
			{
				movement -= movement - extra;
				g = g->GetPrevEdge();
				quant = g->GetLength();
			}
			else
			{
				quant += movement;
				movement = 0;
			}
		}
	}

	V2d finalPos = g->GetPosition(quant);

	if (dist > 0)
	{
		cwPreview = finalPos;
		cwCircle.setPosition(Vector2f(finalPos));
	}
	else
	{
		ccwPreview = finalPos;
		ccwCircle.setPosition(Vector2f(finalPos));
	}

}

bool Widow::TryMove()
{
	if (PlayerDist() < 500)
	{
		action = MOVE;
		frame = 0;

		V2d along = surfaceMover->ground->Along();
		double alongD = dot(PlayerDir(), along);
		double moveSpeed = 10;
		if (alongD >= 0)
		{
			moveSpeed = -moveSpeed;
		}
		surfaceMover->SetSpeed(moveSpeed);
		return true;
	}
	return false;
}

void Widow::ProcessState()
{
	Actor *player = sess->GetPlayer(0);

	if (dead)
		return;

	ActionEnded();

	
	

	
	V2d playerPos = sess->GetPlayerPos(0);

	switch (action)
	{
	case IDLE:
		if (PlayerDist() < attentionRadius)
		{
			action = WAIT;
			frame = 0;
		}
		break;
	case WAIT:
		if (!TryMove())
		{
			surfaceMover->SetSpeed(0);
		}
		break;
	case MOVE:
		break;
	case JUMP:
		break;
	case ATTACK:
		break;
	case LAND:
		break;
	}

	switch (action)
	{
	case IDLE:
		break;
	case WAIT:
		break;
	case MOVE:
		break;
	case JUMP:
		break;
	case ATTACK:
	{
	}
	break;
	case LAND:
	{
	}
	break;
	}

	if (action != IDLE)
	{
		if (data.fireCounter == 30)
		{
			launchers[0]->position = GetPosition();
			launchers[0]->facingDir = PlayerDir();
			launchers[0]->Fire();
			data.fireCounter = 0;
		}
	}
}

void Widow::FrameIncrement()
{
	++data.fireCounter;
}

void Widow::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);

	//target->draw(cwCircle);
	//target->draw(ccwCircle);
}


void Widow::UpdateSprite()
{
	IntRect ir = ts->GetSubRect(0);

	ts->SetSubRect(sprite, 0, !facingRight, false);

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
}

bool Widow::StartRoll()
{
	return false;
}

void Widow::FinishedRoll()
{
}

void Widow::HitTerrain(double &q)
{

}

void Widow::HitOther()
{
	V2d v;
	if (facingRight && surfaceMover->groundSpeed > 0)
	{
		v = V2d(10, -10);
		surfaceMover->Jump(v);
	}
	else if (!facingRight && surfaceMover->groundSpeed < 0)
	{
		v = V2d(-10, -10);
		surfaceMover->Jump(v);
	}
	//cout << "hit other!" << endl;
	//mover->SetSpeed( 0 );
	//facingRight = !facingRight;
}

void Widow::ReachCliff()
{
	if (facingRight && surfaceMover->groundSpeed < 0
		|| !facingRight && surfaceMover->groundSpeed > 0)
	{
		surfaceMover->SetSpeed(0);
		return;
	}

	//cout << "reach cliff!" << endl;
	//ground = NULL;
	V2d v;
	if (facingRight)
	{
		v = V2d(10, -10);
	}
	else
	{
		v = V2d(-10, -10);
	}

	surfaceMover->Jump(v);
	//mover->groundSpeed = -mover->groundSpeed;
	//facingRight = !facingRight;
}

void Widow::HitOtherAerial(Edge *e)
{
	//cout << "hit edge" << endl;
}

void Widow::Land()
{
	action = LAND;
	frame = 0;
	surfaceMover->SetSpeed(0);
	//cout << "land" << endl;
}

void Widow::HandleRayCollision(Edge *edge, double equant, double rayPortion)
{
	if (edge->edgeType == Edge::OPEN_GATE)
	{
		return;
	}

	RayCastHandler::HandleRayCollision(edge, equant, rayPortion);
}

void Widow::BulletHitTerrain(BasicBullet *b, Edge *edge, V2d &pos)
{
	b->launcher->DeactivateBullet(b);
}

void Widow::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}
}

void Widow::DirectKill()
{
	BasicBullet *b = launchers[0]->activeBullets;
	while (b != NULL)
	{
		BasicBullet *next = b->next;
		double angle = atan2(b->velocity.y, -b->velocity.x);
		//sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
		b->launcher->DeactivateBullet(b);

		b = next;
	}

	receivedHit.SetEmpty();
}

int Widow::GetNumStoredBytes()
{
	return sizeof(MyData) + launchers[0]->GetNumStoredBytes();
}

void Widow::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	launchers[0]->StoreBytes(bytes);
	bytes += launchers[0]->GetNumStoredBytes();
}

void Widow::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	launchers[0]->SetFromBytes(bytes);
	bytes += launchers[0]->GetNumStoredBytes();
}