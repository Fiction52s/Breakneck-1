#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_GatorWaterOrb.h"
#include "Actor.h"

using namespace std;
using namespace sf;

GatorWaterOrbPool::GatorWaterOrbPool()
{
	Session *sess = Session::GetSession();
	ts = NULL;
	numBullets = 5;
	bulletVec.resize(numBullets);
	verts = new Vertex[numBullets * 4];
	ts = sess->GetSizedTileset("Bosses/Gator/orb_32x32.png");
	for (int i = 0; i < numBullets; ++i)
	{
		bulletVec[i] = new GatorWaterOrb(verts + 4 * i, this);
	}
}


GatorWaterOrbPool::~GatorWaterOrbPool()
{
	for (int i = 0; i < numBullets; ++i)
	{
		delete bulletVec[i];
	}

	delete[] verts;
}

void GatorWaterOrbPool::Reset()
{
	GatorWaterOrb *orb = NULL;
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

bool GatorWaterOrbPool::CanThrow()
{
	GatorWaterOrb *bs = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		bs = bulletVec[i];
		if (!bs->spawned)
		{
			return true;
		}
	}
}

GatorWaterOrb * GatorWaterOrbPool::Throw(V2d &pos, V2d &dir, int orbType )
{
	GatorWaterOrb *bs = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		bs = bulletVec[i];
		if (!bs->spawned)
		{
			bs->Throw(pos, dir, orbType);
			break;
		}
	}
	return bs;
}

void GatorWaterOrbPool::Redirect(V2d &vel)
{
	GatorWaterOrb *bs = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		bs = bulletVec[i];
		if (bs->spawned )
		{
			bs->Redirect(vel);
		}
	}
}

GatorWaterOrb *GatorWaterOrbPool::GetOldest()
{
	GatorWaterOrb *bs = NULL;
	GatorWaterOrb *oldest = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		bs = bulletVec[i];
		if (bs->spawned && bs->action == GatorWaterOrb::GROWING)
		{
			if (oldest == NULL || bs->framesToLive
				< oldest->framesToLive)
			{
				oldest = bs;
			}
		}
	}

	return oldest;
}

bool GatorWaterOrbPool::RedirectOldestAtPlayer(Actor *player,double speed )
{
	GatorWaterOrb *oldest = NULL;
	oldest = GetOldest();

	if (oldest != NULL)
	{
		double bigger = oldest->currRadius - oldest->startRadius;
		speed -= .2/speed * bigger;
		V2d dir = normalize(V2d(player->position - oldest->GetPosition()));
		oldest->Redirect( dir * speed);
		return true;
	}
	return false;
}

bool GatorWaterOrbPool::RedirectOldest(V2d &vel)
{
	
	GatorWaterOrb *oldest = GetOldest();

	if (oldest != NULL)
	{
		oldest->Redirect(vel);
		return true;
	}
		
	return false;
}

int GatorWaterOrbPool::GetNumGrowingOrbs()
{
	GatorWaterOrb *bs = NULL;
	int numGrowing = 0;
	for (int i = 0; i < numBullets; ++i)
	{
		bs = bulletVec[i];
		if (bs->spawned && bs->action == GatorWaterOrb::GROWING)
		{
			++numGrowing;
		}
	}

	return numGrowing;
}

void GatorWaterOrbPool::Draw(sf::RenderTarget *target)
{
	target->draw(verts, bulletVec.size() * 4, sf::Quads, ts->texture);
}

GatorWaterOrb::GatorWaterOrb(sf::Vertex *myQuad, GatorWaterOrbPool *pool)
	:Enemy(EnemyType::EN_GATORORB, NULL)
{
	SetNumActions(A_Count);
	SetEditorActions(FLYING, 0, 0);

	actionLength[FLYING] = 1;
	animFactor[FLYING] = 1;

	quad = myQuad;

	ts = pool->ts;

	quadraticMove = quadraticMoveSeq.AddQuadraticMovement(
		V2d(), V2d(), V2d(), CubicBezier(), 0);

	startRadius = ts->tileWidth;
	maxRadius = 200;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHitBodySetup(16);
	hitBody.hitboxInfo = hitboxInfo;

	origFramesToLive = 600;
	maxFlySpeed = 60;
	accel = .05;

	ResetEnemy();
}

void GatorWaterOrb::ResetEnemy()
{
	ClearRect(quad);

	facingRight = true;

	action = FLYING;
	frame = 0;

	quadraticMoveSeq.Reset();

	DefaultHitboxesOn();

	UpdateHitboxes();
}

void GatorWaterOrb::SetLevel(int lev)
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

void GatorWaterOrb::Throw(V2d &pos, V2d &dir, int p_orbType )
{
	Reset();
	sess->AddEnemy(this);
	currPosInfo.position = pos;
	currPosInfo.ground = NULL;
	
	velocity = V2d(0, 0);

	action = FLYING;
	frame = 0;
	framesToLive = origFramesToLive;

	orbType = p_orbType;

	flySpeed = 10;

	quadraticMove->SetFrameDuration(60);
	quadraticMove->A = pos;
	quadraticMove->B = sess->GetPlayerPos(0);
	quadraticMove->C = dir;
	quadraticMove->start = pos;
	quadraticMove->end = dir;
	quadraticMoveSeq.Reset();

	currRadius = ts->tileWidth / 2;

}

void GatorWaterOrb::Redirect(V2d &vel)
{
	velocity = vel;
	action = REDIRECT;
	frame = 0;
	framesToLive = 180;
}

void GatorWaterOrb::FrameIncrement()
{
	--framesToLive;
}

void GatorWaterOrb::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();

	V2d diff = playerPos - position;
	V2d pDir = normalize(diff);

	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case FLYING:
			break;
		}
		frame = 0;
	}

	if (action == FLYING && quadraticMoveSeq.currMovement == NULL)
	{
		action = GROWING;
		frame = 0;
	}

	if (framesToLive == 0)
	{
		ClearRect(quad);

		sess->RemoveEnemy(this);
		spawned = false;
	}

	if (action == FLYING || action == GROWING)
	{
		if (currRadius < maxRadius)
		{
			currRadius += 1;
			hitBody.GetCollisionBoxes(0).at(0).rw = currRadius;
		}
	}
}

void GatorWaterOrb::IHitPlayer(int index)
{
	
}

void GatorWaterOrb::UpdateEnemyPhysics()
{
	if (action == FLYING )
	{

		quadraticMoveSeq.Update(slowMultiple, NUM_MAX_STEPS / numPhysSteps);

		currPosInfo.position = quadraticMoveSeq.position;
	}
	else
	{
		V2d movementVec = velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
	}
	
}

void GatorWaterOrb::UpdateSprite()
{
	int tile = 0;
	if (orbType == NODE_GROW_HIT)
	{
		tile = 0;
	}
	else if (orbType == NODE_GROW_SLOW)
	{
		tile = 1;
	}
	ts->SetQuadSubRect(quad, tile);
	SetRectCenter(quad, currRadius * 2, currRadius * 2, GetPositionF());
}

void GatorWaterOrb::EnemyDraw(sf::RenderTarget *target)
{
}

void GatorWaterOrb::HandleHitAndSurvive()
{
}

void GatorWaterOrb::Die()
{
	ClearRect(quad);
	sess->RemoveEnemy(this);
	spawned = false;
	dead = true;
}

bool GatorWaterOrb::CheckHitPlayer(int index)
{

	Actor *player = sess->GetPlayer(index);

	if (player == NULL)
		return false;

	//if (player->specialSlow)
	//{
	//	return false;
	//}


	if (currHitboxes != NULL && currHitboxes->hitboxInfo != NULL)
	{
		Actor::HitResult hitResult = player->CheckIfImHitByEnemy( this, currHitboxes, currHitboxFrame, currHitboxes->hitboxInfo->hitPosType,
			GetPosition(), facingRight,
			currHitboxes->hitboxInfo->canBeParried,
			currHitboxes->hitboxInfo->canBeBlocked);

		if (hitResult != Actor::HitResult::MISS)
		{
			//IHitPlayer(index);
			if (currHitboxes != NULL) //needs a second check in case ihitplayer changes the hitboxes
			{
				if (orbType == GatorWaterOrb::OrbType::NODE_GROW_HIT)
				{
					if (hitResult != Actor::HitResult::INVINCIBLEHIT) //needs a second check in case ihitplayer changes the hitboxes
					{
						player->ApplyHit(currHitboxes->hitboxInfo,
							NULL, hitResult, GetPosition());
						Die();
					}
					//Die(); might be better out here
				}
				else
				{
					player->RestoreAirOptions();
					player->specialSlow = true;
				}
				
				//Die();
				//velocity = velocity *= .9;

				

				/*player->ApplyHit(currHitboxes->hitboxInfo,
					NULL, hitResult, GetPosition());*/
			}
		}
	}


	return false;
}