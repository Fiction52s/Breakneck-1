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
	for (int i = 0; i < numBullets; ++i)
	{
		bulletVec[i]->Reset();
	}
}

GatorWaterOrb * GatorWaterOrbPool::Throw(V2d &pos, V2d &dir)
{
	GatorWaterOrb *bs = NULL;
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

void GatorWaterOrb::Throw(V2d &pos, V2d &dir)
{
	Reset();
	sess->AddEnemy(this);
	currPosInfo.position = pos;
	currPosInfo.ground = NULL;
	distToTarget = length(pos - sess->GetPlayerPos(0));

	action = UNDODGEABLE;
	frame = 0;
	framesToLive = origFramesToLive;

	flySpeed = 10;

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

	if (action == UNDODGEABLE)
	{
		velocity = pDir * flySpeed;
		flySpeed += accel;
		if (flySpeed > maxFlySpeed)
			flySpeed = maxFlySpeed;
	}
	
}

void GatorWaterOrb::IHitPlayer(int index)
{
}

void GatorWaterOrb::UpdateEnemyPhysics()
{
	V2d movementVec = velocity;
	movementVec /= slowMultiple * (double)numPhysSteps;

	currPosInfo.position += movementVec;
}

void GatorWaterOrb::UpdateSprite()
{
	ts->SetQuadSubRect(quad, 0);
	SetRectCenter(quad, ts->tileWidth, ts->tileWidth, GetPositionF());
}

void GatorWaterOrb::EnemyDraw(sf::RenderTarget *target)
{
}

void GatorWaterOrb::HandleHitAndSurvive()
{
}

bool GatorWaterOrb::CheckHitPlayer(int index)
{
	Actor *player = sess->GetPlayer(index);

	if (player == NULL)
		return false;


	if (currHitboxes != NULL && currHitboxes->hitboxInfo != NULL)
	{
		Actor::HitResult hitResult = player->CheckIfImHit(currHitboxes, currHitboxFrame, currHitboxes->hitboxInfo->hitPosType,
			GetPosition(), facingRight);

		if (hitResult != Actor::HitResult::MISS)
		{
			IHitPlayer(index);
			if (currHitboxes != NULL) //needs a second check in case ihitplayer changes the hitboxes
			{
				player->RestoreAirDash();
				player->RestoreDoubleJump();
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