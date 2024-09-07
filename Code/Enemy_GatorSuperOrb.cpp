#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_GatorSuperOrb.h"
#include "Actor.h"

using namespace std;
using namespace sf;

GatorSuperOrbPool::GatorSuperOrbPool()
{
	Session *sess = Session::GetSession();
	ts = NULL;
	numBullets = 5;
	bulletVec.resize(numBullets);
	verts = new Vertex[numBullets * 4];
	ts = sess->GetSizedTileset("Enemies/Bosses/Gator/orb_32x32.png");
	for (int i = 0; i < numBullets; ++i)
	{
		bulletVec[i] = new GatorSuperOrb(verts + 4 * i, this);
	}
}


GatorSuperOrbPool::~GatorSuperOrbPool()
{
	for (int i = 0; i < numBullets; ++i)
	{
		delete bulletVec[i];
	}

	delete[] verts;
}

void GatorSuperOrbPool::Reset()
{
	GatorSuperOrb *orb = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		orb = bulletVec[i];
		if (orb->spawned)
		{
			orb->Die();
		}

		orb->Reset();
	}

	for (int i = 0; i < numBullets; ++i)
	{
		bulletVec[i]->Reset();
	}
}

bool GatorSuperOrbPool::CanThrow()
{
	GatorSuperOrb *bs = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		bs = bulletVec[i];
		if (!bs->spawned)
		{
			return true;
		}
	}
}

void GatorSuperOrbPool::ReturnToGator( V2d &pos)
{
	GatorSuperOrb *bs = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		bs = bulletVec[i];
		if (bs->spawned)
		{
			bs->ReturnToGator( pos);
		}
	}
}

void GatorSuperOrbPool::SetEnemyIDAndAddToAllEnemiesVec()
{
	GatorSuperOrb *bs = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		bs = bulletVec[i];
		bs->SetEnemyIDAndAddToAllEnemiesVec();
	}
}

bool GatorSuperOrbPool::IsIdle()
{
	GatorSuperOrb *bs = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		bs = bulletVec[i];
		if (bs->spawned)
		{
			return bs->IsIdle();
		}
	}
}

GatorSuperOrb * GatorSuperOrbPool::Throw(V2d &pos, V2d &dir)
{
	GatorSuperOrb *bs = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		bs = bulletVec[i];
		if (!bs->spawned)
		{
			bs->Throw(pos, dir);
			break;
		}
	}
	return bs;
}

void GatorSuperOrbPool::Draw(sf::RenderTarget *target)
{
	target->draw(verts, bulletVec.size() * 4, sf::Quads, ts->texture);
}

GatorSuperOrb::GatorSuperOrb(sf::Vertex *myQuad, GatorSuperOrbPool *pool)
	:Enemy(EnemyType::EN_GATORSUPERORB, NULL)
{
	SetNumActions(A_Count);
	SetEditorActions(CHASING, 0, 0);


	actionLength[CHASING] = 1;
	animFactor[CHASING] = 1;

	actionLength[DISSIPATE] = 10;
	animFactor[DISSIPATE] = 1;

	quad = myQuad;

	ts = pool->ts;

	startRadius = ts->tileWidth;
	maxRadius = 100;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHitBodySetup(16);
	hitBody.hitboxInfo = hitboxInfo;

	maxFlySpeed = 60;
	accel = .05;

	ResetEnemy();
}

void GatorSuperOrb::ResetEnemy()
{
	ClearRect(quad);

	facingRight = true;

	action = CHASING;
	frame = 0;

	DefaultHitboxesOn();

	UpdateHitboxes();
}

void GatorSuperOrb::SetLevel(int lev)
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

void GatorSuperOrb::Throw(V2d &pos, V2d &dir)
{
	Reset();
	sess->AddEnemy(this);
	currPosInfo.position = pos;
	currPosInfo.ground = NULL;
	enemyMover.Reset();
	enemyMover.currPosInfo = currPosInfo;

	velocity = V2d(0, 0);

	DefaultHitboxesOn();

	action = CHASING;
	frame = 0;

	flySpeed = 10;

	enemyMover.SetModeCatch(&sess->GetPlayer(0)->position, 5, .1);

	currRadius = ts->tileWidth / 2;

}


void GatorSuperOrb::FrameIncrement()
{
	enemyMover.FrameIncrement();
	currPosInfo = enemyMover.currPosInfo;
}

void GatorSuperOrb::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();

	V2d diff = playerPos - position;
	V2d pDir = normalize(diff);

	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case CHASING:
			frame = 0;
			break;
		case DISSIPATE:
			ClearRect(quad);

			sess->RemoveEnemy(this);
			spawned = false;
			sess->GetPlayer(0)->SetAirPos(GetPosition(), true);
			break;
		}
		
	}

	/*if (action == CENTER && enemyMover.IsIdle())
	{
		action = GROW;
		frame = 0;
	}*/

	if (action == RETURN_TO_GATOR && enemyMover.IsIdle())
	{
		action = STASIS;
		frame = 0;
	}
	else if (action == LAUNCH && enemyMover.IsIdle())
	{
		action = DISSIPATE;
		frame = 0;
	}

	if (action == GROW)
	{
		if (currRadius < maxRadius)
		{
			currRadius += 1;
		}

		if (currRadius >= maxRadius)
		{
			currRadius = maxRadius;
			action = STASIS;
			frame = 0;
		}

		hitBody.GetCollisionBoxes(0).at(0).rw = currRadius;
	}
}

void GatorSuperOrb::IHitPlayer(int index)
{

}

void GatorSuperOrb::UpdateEnemyPhysics()
{
	if (!enemyMover.IsIdle())
	{
		enemyMover.UpdatePhysics(numPhysSteps, slowMultiple);
		currPosInfo = enemyMover.currPosInfo;

		if (action == RETURN_TO_GATOR || action == LAUNCH)
		{
			sess->GetPlayer(0)->position = currPosInfo.position;
		}
	}
	else
	{
		Enemy::UpdateEnemyPhysics();
	}
}

void GatorSuperOrb::UpdateSprite()
{
	int tile = 0;
	ts->SetQuadSubRect(quad, tile);
	SetRectCenter(quad, currRadius * 2, currRadius * 2, GetPositionF());
}



void GatorSuperOrb::EnemyDraw(sf::RenderTarget *target)
{
}

void GatorSuperOrb::HandleHitAndSurvive()
{
}

void GatorSuperOrb::Die()
{
	ClearRect(quad);
	sess->RemoveEnemy(this);
	spawned = false;
	dead = true;
}

bool GatorSuperOrb::IsIdle()
{
	return action == STASIS;
}

void GatorSuperOrb::Launch(V2d &pos, double extraHeight, double speed)
{
	action = LAUNCH;
	frame = 0;
	enemyMover.SetModeNodeJump(pos,extraHeight, speed);
}

void GatorSuperOrb::ReturnToGator(V2d &pos)
{
	action = RETURN_TO_GATOR;
	frame = 0;
	enemyMover.SetModeNodeLinearConstantSpeed(pos, CubicBezier(), 5);
}

void GatorSuperOrb::SetPos(V2d &pos)
{
	currPosInfo.position = pos;
	enemyMover.currPosInfo.position = pos;
	sess->GetPlayer(0)->position = currPosInfo.position;
}

bool GatorSuperOrb::CheckHitPlayer(int index)
{

	Actor *player = sess->GetPlayer(index);

	if (player == NULL)
		return false;

	if (currHitboxes != NULL && currHitboxes->hitboxInfo != NULL)
	{
		Actor::HitResult hitResult = player->CheckIfImHitByEnemy(this, currHitboxes, currHitboxFrame, currHitboxes->hitboxInfo->hitPosType,
			GetPosition(), facingRight,
			currHitboxes->hitboxInfo->canBeParried,
			currHitboxes->hitboxInfo->canBeBlocked);

		if (hitResult != Actor::HitResult::MISS)
		{
			//IHitPlayer(index);
			if (currHitboxes != NULL) //needs a second check in case ihitplayer changes the hitboxes
			{
				player->GatorStun();
				action = GROW;
				frame = 0;
				HitboxesOff();
				enemyMover.Reset();
				currPosInfo.position = player->position;
				enemyMover.currPosInfo = currPosInfo;
				//enemyMover.SetModeNodeLinearConstantSpeed(player->position, CubicBezier(), 10);
				

				//Die();
				//velocity = velocity *= .9;



				/*player->ApplyHit(currHitboxes->hitboxInfo,
				NULL, hitResult, GetPosition());*/
			}
		}
	}


	return false;
}