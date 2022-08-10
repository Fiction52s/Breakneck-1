#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_CoyoteBullet.h"
#include "Actor.h"

using namespace std;
using namespace sf;

CoyoteBulletPool::CoyoteBulletPool()
{
	Session *sess = Session::GetSession();

	ts = NULL;
	numBullets = 30;
	bulletVec.resize(numBullets);
	verts = new Vertex[numBullets * 4];
	ts = sess->GetSizedTileset("Bosses/Coyote/coyotebullet_32x32.png");
	for (int i = 0; i < numBullets; ++i)
	{
		bulletVec[i] = new CoyoteBullet(verts + 4 * i, this);
	}
}
	

CoyoteBulletPool::~CoyoteBulletPool()
{
	for (int i = 0; i < numBullets; ++i)
	{
		delete bulletVec[i];
	}

	delete[] verts;
}

void CoyoteBulletPool::Reset()
{
	CoyoteBullet *cb = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		cb = bulletVec[i];
		if (cb->spawned)
		{
			cb->Die();
		}

		cb->Reset();
	}
}

CoyoteBullet * CoyoteBulletPool::Throw(V2d &pos, V2d &dir)
{
	CoyoteBullet *bs = NULL;
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

void CoyoteBulletPool::Draw(sf::RenderTarget *target)
{
	target->draw(verts, bulletVec.size() * 4, sf::Quads, ts->texture);
}

CoyoteBullet::CoyoteBullet(sf::Vertex *myQuad, CoyoteBulletPool *pool)
	:Enemy(EnemyType::EN_COYOTEBULLET, NULL)
{
	SetNumActions(A_Count);
	SetEditorActions(DASHING, 0, 0);

	rhythm = 30;

	actionLength[DASHING] = rhythm;
	animFactor[DASHING] = 1;

	actionLength[WAITING] = rhythm;
	animFactor[WAITING] = 1;

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

	dashSpeed = 10;

	ResetEnemy();
}

void CoyoteBullet::ResetEnemy()
{
	ClearRect(quad);

	facingRight = true;

	action = DASHING;
	frame = 0;

	timeToLive = 140;

	DefaultHitboxesOn();

	UpdateHitboxes();
}

void CoyoteBullet::Die()
{
	ClearRect(quad);
	sess->RemoveEnemy(this);
	spawned = false;
	dead = true;
}

void CoyoteBullet::SetLevel(int lev)
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

V2d CoyoteBullet::GetThrowDir(V2d &dir)
{
	double ang = GetVectorAngleCCW(dir);

	double degs = ang / PI * 180.0;
	double sec = 360.0 / 8.0;
	int mult = floor((degs / sec) + .5);

	double trueAngle = (PI / ( 8 / 2 )) * mult;
	return V2d( cos(trueAngle), -sin(trueAngle));
}

void CoyoteBullet::Throw(V2d &pos, V2d &dir)
{
	Reset();
	sess->AddEnemy(this);
	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	int changeFrames = actionLength[DASHING];
	int f = sess->totalGameFrames % (changeFrames * 2);
	int f2 = sess->totalGameFrames % changeFrames;
	if (f < changeFrames)
	{
		action = DASHING;
		frame = f2;
		velocity = GetThrowDir( dir ) * dashSpeed;
	}
	else
	{
		action = WAITING;
		frame = f2;
		velocity = V2d(0, 0);
	}
}

void CoyoteBullet::FrameIncrement()
{
	--timeToLive;
	if (timeToLive == 0)
	{
		Die();
	}
}

void CoyoteBullet::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();

	V2d diff = playerPos - position;
	V2d pDir = normalize(diff);

	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case DASHING:
			action = WAITING;
			velocity = V2d();
			break;
		case WAITING:
			action = DASHING;
			velocity = GetThrowDir(pDir) * dashSpeed;
			break;
		}
		frame = 0;
	}
}

void CoyoteBullet::IHitPlayer(int index)
{
}

bool CoyoteBullet::CheckHitPlayer(int index)
{

	Actor *player = sess->GetPlayer(index);

	if (player == NULL)
		return false;

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
				/*player->SetAction(Actor::BOOSTERBOUNCE);
				player->frame = 0;
				Die();*/

				//Die();
				//velocity = velocity *= .9;



				player->ApplyHit(currHitboxes->hitboxInfo,
				NULL, hitResult, GetPosition());
			}
		}
	}


	return false;
}

void CoyoteBullet::UpdateEnemyPhysics()
{
	V2d movementVec = velocity;
	movementVec /= slowMultiple * (double)numPhysSteps;

	currPosInfo.position += movementVec;
}

void CoyoteBullet::UpdateSprite()
{
	ts->SetQuadSubRect(quad, 0);
	SetRectCenter(quad, ts->tileWidth, ts->tileWidth, GetPositionF());
}

void CoyoteBullet::EnemyDraw(sf::RenderTarget *target)
{
}

void CoyoteBullet::HandleHitAndSurvive()
{
}