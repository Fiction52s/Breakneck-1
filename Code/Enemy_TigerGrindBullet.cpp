#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_TigerGrindBullet.h"

using namespace std;
using namespace sf;


GrindFirePool::GrindFirePool()
{
	Session *sess = Session::GetSession();

	ts = NULL;
	numFires = 24;
	fireVec.resize(numFires);
	verts = new Vertex[numFires * 4];
	ts = sess->GetSizedTileset("Bosses/Coyote/coyotebullet_32x32.png");
	for (int i = 0; i < numFires; ++i)
	{
		fireVec[i] = new GrindFire(verts + 4 * i, this);
	}
}


GrindFirePool::~GrindFirePool()
{
	for (int i = 0; i < numFires; ++i)
	{
		delete fireVec[i];
	}

	delete[] verts;
}

void GrindFirePool::Reset()
{
	for (int i = 0; i < numFires; ++i)
	{
		fireVec[i]->Reset();
	}
}

GrindFire * GrindFirePool::Create( int type, V2d &pos, Edge *ground, double quant )
{
	GrindFire *bs = NULL;
	for (int i = 0; i < numFires; ++i)
	{
		bs = fireVec[i];
		if (!bs->active)
		{
			bs->Create( type, pos, ground, quant );
			break;
		}
	}
	return bs;
}

void GrindFirePool::Draw(sf::RenderTarget *target)
{
	target->draw(verts, fireVec.size() * 4, sf::Quads, ts->texture);
}

GrindFire::GrindFire(sf::Vertex *myQuad, GrindFirePool *pool)
	:Enemy(EnemyType::EN_TIGERGRINDFIRE, NULL)
{
	SetNumActions(A_Count);
	SetEditorActions(BURN, 0, 0);

	actionLength[BURN] = 4 * 20;
	animFactor[BURN] = 1;

	quad = myQuad;

	ts = pool->ts;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHitBodySetup(16);
	hitBody.hitboxInfo = hitboxInfo;

	ResetEnemy();
}

void GrindFire::ResetEnemy()
{
	/*switch (gbType)
	{
		case TigerGrindBullet::G
	}*/


	ClearRect(quad);

	facingRight = true;

	action = BURN;
	frame = 0;

	DefaultHitboxesOn();

	UpdateHitboxes();
}

void GrindFire::SetLevel(int lev)
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

V2d GrindFire::GetThrowDir(V2d &dir)
{
	double ang = GetVectorAngleCCW(dir);

	double degs = ang / PI * 180.0;
	double sec = 360.0 / 8.0;
	int mult = floor((degs / sec) + .5);

	double trueAngle = (PI / (8 / 2)) * mult;
	return V2d(cos(trueAngle), -sin(trueAngle));
}

void GrindFire::Create( int type, V2d &pos, Edge *ground, double quant )
{
	gbType = type;

	Reset();
	sess->AddEnemy(this);

	currPosInfo.position = pos;
	currPosInfo.ground = NULL;
}

void GrindFire::FrameIncrement()
{
}

void GrindFire::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case BURN:
			ClearRect(quad);
			sess->RemoveEnemy(this);
			spawned = false;
			break;
		}
		frame = 0;
	}
}

void GrindFire::IHitPlayer(int index)
{
}

void GrindFire::UpdateSprite()
{
	ts->SetQuadSubRect(quad, 0);
	SetRectCenter(quad, ts->tileWidth, ts->tileWidth, GetPositionF());
}

void GrindFire::EnemyDraw(sf::RenderTarget *target)
{
}

void GrindFire::HandleHitAndSurvive()
{
}

TigerGrindBulletPool::TigerGrindBulletPool()
{
	Session *sess = Session::GetSession();

	ts = NULL;
	numBullets = 10;
	bulletVec.resize(numBullets);
	verts = new Vertex[numBullets * 4];
	ts = sess->GetSizedTileset("Bosses/Coyote/coyotebullet_32x32.png");
	for (int i = 0; i < numBullets; ++i)
	{
		bulletVec[i] = new TigerGrindBullet(verts + 4 * i, this);
	}
}


TigerGrindBulletPool::~TigerGrindBulletPool()
{
	for (int i = 0; i < numBullets; ++i)
	{
		delete bulletVec[i];
	}

	delete[] verts;
}

void TigerGrindBulletPool::Reset()
{
	for (int i = 0; i < numBullets; ++i)
	{
		bulletVec[i]->Reset();
	}
}

TigerGrindBullet * TigerGrindBulletPool::Throw( int type, V2d &pos, V2d &dir)
{
	TigerGrindBullet *bs = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		bs = bulletVec[i];
		if (!bs->active)
		{
			bs->Throw(type, pos, dir);
			break;
		}
	}
	return bs;
}

TigerGrindBullet * TigerGrindBulletPool::ThrowAt(int type, V2d &pos, PoiInfo *dest)
{
	TigerGrindBullet *bs = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		bs = bulletVec[i];
		if (!bs->active)
		{
			bs->ThrowAt(type, pos, dest);
			break;
		}
	}
	return bs;
}

void TigerGrindBulletPool::Draw(sf::RenderTarget *target)
{
	target->draw(verts, bulletVec.size() * 4, sf::Quads, ts->texture);
}

void TigerGrindBulletPool::DrawMinimap(sf::RenderTarget * target)
{
	for (auto it = bulletVec.begin(); it != bulletVec.end(); ++it)
	{
		if ((*it)->active)
		{
			(*it)->DrawMinimap(target);
		}
	}
}

TigerGrindBullet::TigerGrindBullet(sf::Vertex *myQuad, TigerGrindBulletPool *pool)
	:Enemy(EnemyType::EN_TIGERGRINDBULLET, NULL)
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

	maxBurnFrames = 120;
	

	quad = myQuad;

	ts = pool->ts;

	CreateSurfaceMover(startPosInfo, 12, this);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 60;
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

void TigerGrindBullet::ResetEnemy()
{
	ClearRect(quad);

	facingRight = true;

	action = THROWN;
	frame = 0;

	destPoi = NULL;

	firePool.Reset();

	currBurnFrame = 0;

	DefaultHitboxesOn();

	UpdateHitboxes();
	
}

void TigerGrindBullet::HitTerrainAerial(Edge *e, double quant)
{
	StartGrind();
}

void TigerGrindBullet::SetLevel(int lev)
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

V2d TigerGrindBullet::GetThrowDir(V2d &dir)
{
	double ang = GetVectorAngleCCW(dir);

	double degs = ang / PI * 180.0;
	double sec = 360.0 / 8.0;
	int mult = floor((degs / sec) + .5);

	double trueAngle = (PI / (8 / 2)) * mult;
	return V2d(cos(trueAngle), -sin(trueAngle));
}

void TigerGrindBullet::Throw( int type, V2d &pos, V2d &dir)
{
	gbType = (GrindBulletType)type;

	Reset();
	sess->AddEnemy(this);

	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	surfaceMover->Set(currPosInfo);

	surfaceMover->SetVelocity(dir * flySpeed);
}

void TigerGrindBullet::ThrowAt(int type, V2d &pos, PoiInfo *pi)
{
	gbType = (GrindBulletType)type;

	Reset();
	sess->AddEnemy(this);

	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	surfaceMover->Set(currPosInfo);

	action = THROWN_AT;
	frame = 0;
	destPoi = pi;

	V2d diff = pi->edge->GetPosition(pi->edgeQuantity) - GetPosition();
	V2d dir = normalize( diff );

	surfaceMover->SetCollisionOn(false);
	surfaceMover->SetVelocity(dir * flySpeed);

	framesToArriveToDestPoi = ceil(length(diff) / flySpeed);
}

void TigerGrindBullet::StartGrind()
{
	action = GRIND;
	frame = 0;
	currBurnFrame = 0;

	double speed = 10;

	switch (gbType)
	{
	case GB_REGULAR_CW:
		speed = 10;
		break;
	case GB_REGULAR_CCW:
		speed = -10;
		break;
	case GB_FAST_CW:
		speed = 15;
		break;
	case GB_FAST_CCW:
		speed = -15;
		break;
	}
	surfaceMover->SetSpeed(speed);
}

void TigerGrindBullet::FrameIncrement()
{
	if (action == THROWN_AT)
	{
		--framesToArriveToDestPoi;
		if (framesToArriveToDestPoi <= 0)
		{
			PositionInfo posInfo;
			posInfo.SetGround(destPoi->poly, destPoi->edgeIndex, destPoi->edgeQuantity);
			surfaceMover->Set(posInfo);
			StartGrind();
			destPoi = NULL;
		}
	}
	else if (action == GRIND)
	{
		if (currBurnFrame < maxBurnFrames)
		{
			++currBurnFrame;
			//action = DISSIPATE;
			//frame = 0;
		}
	}
}

void TigerGrindBullet::ProcessState()
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
		bool anyActive = false;
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
		}
	}
	
	

	if (action == GRIND && frame == 1 && currBurnFrame < maxBurnFrames)
	{
		firePool.Create( gbType, GetPosition(), surfaceMover->ground, surfaceMover->GetEdgeQuantity());
	}
	else if (action == GRIND && currBurnFrame == maxBurnFrames)
	{
		action = DISSIPATE;
		frame = 0;
		HitboxesOff();
	}
	
}

void TigerGrindBullet::IHitPlayer(int index)
{
}

void TigerGrindBullet::UpdateSprite()
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

void TigerGrindBullet::EnemyDraw(sf::RenderTarget *target)
{
	firePool.Draw(target);
}

void TigerGrindBullet::HandleHitAndSurvive()
{
}