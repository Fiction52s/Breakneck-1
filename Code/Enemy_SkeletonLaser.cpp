#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_SkeletonLaser.h"

using namespace std;
using namespace sf;


SkeletonLaserPool::SkeletonLaserPool()
{
	Session *sess = Session::GetSession();

	ts = NULL;
	numLasers = 10;
	laserVec.resize(numLasers);
	verts = new Vertex[numLasers * 4];
	ts = sess->GetSizedTileset("Bosses/Coyote/coyotebullet_32x32.png");
	for (int i = 0; i < numLasers; ++i)
	{
		laserVec[i] = new SkeletonLaser(verts + 4 * i, this);
	}
}


SkeletonLaserPool::~SkeletonLaserPool()
{
	for (int i = 0; i < numLasers; ++i)
	{
		delete laserVec[i];
	}

	delete[] verts;
}

void SkeletonLaserPool::Reset()
{
	for (int i = 0; i < numLasers; ++i)
	{
		laserVec[i]->Reset();
	}
}

SkeletonLaser * SkeletonLaserPool::Throw(int type, V2d &pos, V2d &dir)
{
	SkeletonLaser *bs = NULL;
	for (int i = 0; i < numLasers; ++i)
	{
		bs = laserVec[i];
		if (!bs->spawned)
		{
			bs->Throw(type, pos, dir);
			break;
		}
	}
	return bs;
}

SkeletonLaser * SkeletonLaserPool::ThrowAt(int type, V2d &pos, PoiInfo *dest)
{
	SkeletonLaser *bs = NULL;
	for (int i = 0; i < numLasers; ++i)
	{
		bs = laserVec[i];
		if (!bs->spawned)
		{
			bs->ThrowAt(type, pos, dest);
			break;
		}
	}
	return bs;
}

void SkeletonLaserPool::Draw(sf::RenderTarget *target)
{
	target->draw(verts, laserVec.size() * 4, sf::Quads, ts->texture);
}

void SkeletonLaserPool::DrawMinimap(sf::RenderTarget * target)
{
	for (auto it = laserVec.begin(); it != laserVec.end(); ++it)
	{
		if ((*it)->active)
		{
			(*it)->DrawMinimap(target);
		}
	}
}

SkeletonLaser::SkeletonLaser(sf::Vertex *myQuad, SkeletonLaserPool *pool)
	:Enemy(EnemyType::EN_SKELETONLASER, NULL)
{
	SetNumActions(A_Count);
	SetEditorActions(THROWN, 0, 0);

	actionLength[THROWN] = 1;
	animFactor[THROWN] = 1;

	actionLength[THROWN_AT] = 1;
	animFactor[THROWN] = 1;

	actionLength[GRIND] = 4;
	animFactor[GRIND] = 1;

	actionLength[DISSIPATE] = 10;
	animFactor[DISSIPATE] = 1;

	quad = myQuad;

	ts = pool->ts;

	CreateSurfaceMover(startPosInfo, 12, this);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHitBodySetup(16);
	hitBody.hitboxInfo = hitboxInfo;

	flySpeed = 10;

	ResetEnemy();
}

void SkeletonLaser::ResetEnemy()
{
	ClearRect(quad);

	facingRight = true;

	action = THROWN;
	frame = 0;

	destPoi = NULL;

	DefaultHitboxesOn();

	UpdateHitboxes();

}

void SkeletonLaser::HitTerrainAerial(Edge *e, double quant)
{
	//StartGrind();
}

void SkeletonLaser::SetLevel(int lev)
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

V2d SkeletonLaser::GetThrowDir(V2d &dir)
{
	double ang = GetVectorAngleCCW(dir);

	double degs = ang / PI * 180.0;
	double sec = 360.0 / 8.0;
	int mult = floor((degs / sec) + .5);

	double trueAngle = (PI / (8 / 2)) * mult;
	return V2d(cos(trueAngle), -sin(trueAngle));
}

void SkeletonLaser::Throw(int type, V2d &pos, V2d &dir)
{
	laserType = (LaserType)type;

	Reset();
	sess->AddEnemy(this);

	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	surfaceMover->Set(currPosInfo);

	surfaceMover->velocity = dir * flySpeed;
}

void SkeletonLaser::ThrowAt(int type, V2d &pos, PoiInfo *pi)
{
	laserType = (LaserType)type;

	Reset();
	sess->AddEnemy(this);

	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	surfaceMover->Set(currPosInfo);

	action = THROWN_AT;
	frame = 0;
	destPoi = pi;

	V2d diff = pi->edge->GetPosition(pi->edgeQuantity) - GetPosition();
	V2d dir = normalize(diff);

	surfaceMover->collisionOn = false;
	surfaceMover->velocity = dir * flySpeed;

	framesToArriveToDestPoi = ceil(length(diff) / flySpeed);
}

void SkeletonLaser::FrameIncrement()
{
	if (action == THROWN_AT)
	{
		--framesToArriveToDestPoi;
		if (framesToArriveToDestPoi <= 0)
		{
			PositionInfo posInfo;
			posInfo.SetGround(destPoi->poly, destPoi->edgeIndex, destPoi->edgeQuantity);
			surfaceMover->Set(posInfo);
			//StartGrind();
			destPoi = NULL;
		}
	}
}

void SkeletonLaser::ProcessState()
{

	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case THROWN:
			break;
		case THROWN_AT:
			break;
		case GRIND:
			break;
		case DISSIPATE:
			break;
		}
		frame = 0;
	}

	if (action == DISSIPATE)
	{
		/*bool anyActive = false;
		for (auto it = firePool.fireVec.begin(); it != firePool.fireVec.end(); ++it)
		{
			if ((*it)->active)
			{
				anyActive = true;
				break;
			}
		}

		if (!anyActive)
		{
			dead = true;
			numHealth = 0;
			sess->RemoveEnemy(this);
			return;
		}*/
	}



	/*if (action == GRIND && frame == 1 && currBurnFrame < maxBurnFrames)
	{
		firePool.Create(gbType, GetPosition(), surfaceMover->ground, surfaceMover->edgeQuantity);
	}
	else if (action == GRIND && currBurnFrame == maxBurnFrames)
	{
		action = DISSIPATE;
		frame = 0;
		HitboxesOff();
	}*/

}

void SkeletonLaser::IHitPlayer(int index)
{
}

void SkeletonLaser::UpdateSprite()
{
	if (action != DISSIPATE)
	{
		ts->SetQuadSubRect(quad, 0);
		SetRectCenter(quad, ts->tileWidth, ts->tileWidth, GetPositionF());
	}
	else
	{
		ClearRect(quad);
	}
}

void SkeletonLaser::EnemyDraw(sf::RenderTarget *target)
{
	//firePool.Draw(target);
}

void SkeletonLaser::HandleHitAndSurvive()
{
}