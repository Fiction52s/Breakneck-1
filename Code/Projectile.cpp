#include "Projectile.h"

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

TigerGrindBullet * TigerGrindBulletPool::Throw(V2d &pos, V2d &dir)
{
	TigerGrindBullet *bs = NULL;
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

void TigerGrindBulletPool::Draw(sf::RenderTarget *target)
{
	target->draw(verts, bulletVec.size() * 4, sf::Quads, ts->texture);
}

TigerGrindBullet::TigerGrindBullet(sf::Vertex *myQuad, TigerGrindBulletPool *pool)
	:Enemy(EnemyType::EN_TIGERGRINDBULLET, NULL)
{
	SetNumActions(A_Count);
	SetEditorActions(THROWN, 0, 0);

	actionLength[THROWN] = 1;
	animFactor[THROWN] = 1;

	actionLength[GRIND] = 10;
	animFactor[GRIND] = 1;

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

void TigerGrindBullet::ResetEnemy()
{
	ClearRect(quad);

	facingRight = true;

	action = THROWN;
	frame = 0;

	firePool.Reset();

	DefaultHitboxesOn();

	UpdateHitboxes();
}

void TigerGrindBullet::HitTerrainAerial(Edge *e, double quant)
{
	action = GRIND;
	frame = 0;
	surfaceMover->SetSpeed(10);
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

void TigerGrindBullet::Throw(V2d &pos, V2d &dir)
{
	Reset();
	sess->AddEnemy(this);

	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	surfaceMover->Set(currPosInfo);

	surfaceMover->velocity = dir * flySpeed;
}

void TigerGrindBullet::FrameIncrement()
{
}

void TigerGrindBullet::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case THROWN:
			break;
		case GRIND:
			break;
		}
		frame = 0;
	}

	if (action == GRIND)
	{
		firePool.Create(GetPosition(), surfaceMover->ground, surfaceMover->edgeQuantity);
	}
}

void TigerGrindBullet::IHitPlayer(int index)
{
}

void TigerGrindBullet::UpdateSprite()
{
	ts->SetQuadSubRect(quad, 0);
	SetRectCenter(quad, ts->tileWidth, ts->tileWidth, GetPositionF());
}

void TigerGrindBullet::EnemyDraw(sf::RenderTarget *target)
{
	firePool.Draw(target);
}

void TigerGrindBullet::HandleHitAndSurvive()
{
}