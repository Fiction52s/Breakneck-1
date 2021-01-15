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
	numBullets = 10;
	bulletVec.resize(numBullets);
	verts = new Vertex[numBullets * 4];
	ts = sess->GetSizedTileset("Bosses/Coyote/coyotebullet_32x32.png");
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
	

	action = FLYING;
	frame = 0;
	framesToLive = origFramesToLive;

	orbType = p_orbType;

	flySpeed = 10;

	if (orbType == UNDODGEABLE_REFRESH)
	{
		distToTarget = length(pos - sess->GetPlayerPos(0));
	}
	else if (orbType == NODE_GROW)
	{
		quadraticMove->SetFrameDuration(60);
		quadraticMove->A = pos;
		quadraticMove->B = sess->GetPlayerPos(0);
		quadraticMove->C = dir;
		quadraticMove->start = pos;
		quadraticMove->end = dir;
		quadraticMoveSeq.Reset();
	}

	currRadius = ts->tileWidth;

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

	if (framesToLive == 0)
	{
		ClearRect(quad);
		
		sess->RemoveEnemy(this);
		spawned = false;
	}

	if (action == FLYING)
	{
		if (orbType == UNDODGEABLE_REFRESH)
		{
			velocity = pDir * flySpeed;
			flySpeed += accel;
			if (flySpeed > maxFlySpeed)
				flySpeed = maxFlySpeed;
		}
		else if (orbType == NODE_GROW)
		{
			currRadius += 1;

			if (length(GetPosition() - targetPos) < flySpeed)
			{
				velocity = V2d(0, 0);
				currPosInfo.position = targetPos;
			}
		}
	}
	
}

void GatorWaterOrb::IHitPlayer(int index)
{
}

void GatorWaterOrb::UpdateEnemyPhysics()
{
	if (orbType == NODE_GROW)
	{
		if (numPhysSteps == 1)
		{
			quadraticMoveSeq.Update(slowMultiple, 10);
		}
		else
		{
			quadraticMoveSeq.Update(slowMultiple, 1);
		}

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
	ts->SetQuadSubRect(quad, 0);
	SetRectCenter(quad, currRadius, currRadius, GetPositionF());
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


	if (currHitboxes != NULL && currHitboxes->hitboxInfo != NULL)
	{
		Actor::HitResult hitResult = player->CheckIfImHit(currHitboxes, currHitboxFrame, currHitboxes->hitboxInfo->hitPosType,
			GetPosition(), facingRight,
			currHitboxes->hitboxInfo->canBeParried,
			currHitboxes->hitboxInfo->canBeBlocked);

		if (hitResult != Actor::HitResult::MISS)
		{
			IHitPlayer(index);
			if (currHitboxes != NULL) //needs a second check in case ihitplayer changes the hitboxes
			{
				player->RestoreAirOptions();
				ClearRect(quad);
				spawned = false;
				sess->RemoveEnemy(this);
				/*player->ApplyHit(currHitboxes->hitboxInfo,
					NULL, hitResult, GetPosition());*/
			}
		}
	}


	return false;
}