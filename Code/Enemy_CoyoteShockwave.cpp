#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_CoyoteShockwave.h"
#include "Actor.h"

using namespace std;
using namespace sf;

CoyoteShockwavePool::CoyoteShockwavePool()
{
	Session *sess = Session::GetSession();

	ts = NULL;
	numShockwaves = 1;
	shockwaveVec.resize(numShockwaves);
	verts = new Vertex[numShockwaves * 4];
	ts = sess->GetSizedTileset("Enemies/Bosses/Coyote/coyotebullet_32x32.png");
	for (int i = 0; i < numShockwaves; ++i)
	{
		shockwaveVec[i] = new CoyoteShockwave(verts + 4 * i, this);
	}
}


CoyoteShockwavePool::~CoyoteShockwavePool()
{
	for (int i = 0; i < numShockwaves; ++i)
	{
		delete shockwaveVec[i];
	}

	delete[] verts;
}

void CoyoteShockwavePool::Reset()
{
	for (int i = 0; i < numShockwaves; ++i)
	{
		shockwaveVec[i]->Reset();
	}
}

CoyoteShockwave * CoyoteShockwavePool::Throw(V2d &pos, double startRadius,
	double endRadius, double waveThickness, int expandFrames)
{
	CoyoteShockwave *bs = NULL;
	for (int i = 0; i < numShockwaves; ++i)
	{
		bs = shockwaveVec[i];
		if (!bs->spawned)
		{
			bs->Throw(pos, startRadius, endRadius, waveThickness, expandFrames );
			break;
		}
	}
	return bs;
}

void CoyoteShockwavePool::Draw(sf::RenderTarget *target)
{
	target->draw(verts, numShockwaves * 4, sf::Quads, ts->texture);
}

CoyoteShockwave::CoyoteShockwave(sf::Vertex *myQuad, CoyoteShockwavePool *pool)
	:Enemy(EnemyType::EN_COYOTESHOCKWAVE, NULL)
{
	SetNumActions(A_Count);
	SetEditorActions(STARTUP, 0, 0);

	actionLength[STARTUP] = 30;
	animFactor[STARTUP] = 1;

	actionLength[EXPANDING] = 60;
	animFactor[EXPANDING] = 1;

	actionLength[DISSIPATE] = 30;
	animFactor[DISSIPATE] = 1;

	quad = myQuad;

	ts = pool->ts;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	hitboxInfo->knockbackOnBlock = false;

	BasicCircleHitBodySetup(16);
	CollisionBox & cb = hitBody.GetCollisionBoxes(0)[0];
	
	//cb.isRing = true;

	hitBody.hitboxInfo = hitboxInfo;

	waveThickness = 50;
	startRadius = 100;
	endRadius = 500;

	ResetEnemy();
}

void CoyoteShockwave::ResetEnemy()
{
	ClearRect(quad);

	facingRight = true;

	action = STARTUP;
	frame = 0;

	DefaultHitboxesOn();

	UpdateHitboxes();
}

void CoyoteShockwave::SetLevel(int lev)
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

void CoyoteShockwave::Throw(V2d &pos, double p_startRadius,
	double p_endRadius,
	double p_waveThickness,
	int p_expandFrames)
{
	Reset();

	startRadius = p_startRadius;
	endRadius = p_endRadius;
	waveThickness = p_waveThickness;
	actionLength[EXPANDING] = p_expandFrames;

	sess->AddEnemy(this);
	currPosInfo.position = pos;
	currPosInfo.ground = NULL;
	distToTarget = length(pos - sess->GetPlayerPos(0));

	action = STARTUP;
	frame = 0;

	CollisionBox & cb = hitBody.GetCollisionBoxes(0)[0];
	cb.isRing = true;
	cb.rw = startRadius;
	cb.rh = startRadius;
	cb.innerRadius = startRadius - waveThickness;
}

void CoyoteShockwave::FrameIncrement()
{
}

void CoyoteShockwave::Die()
{
	dead = true;
	spawned = false;
	sess->RemoveEnemy(this);
	ClearRect(quad);
}

void CoyoteShockwave::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();

	V2d diff = playerPos - position;
	V2d pDir = normalize(diff);

	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case STARTUP:
			action = EXPANDING;
			break;
		case EXPANDING:
			action = DISSIPATE;
			break;
		case DISSIPATE:
			Die();
			break;
		}
		frame = 0;
	}

	if (action == EXPANDING)
	{
		double numFrames = (actionLength[EXPANDING] * animFactor[EXPANDING])-1;
		double fac = frame/ numFrames;
		double currRadius = fac * endRadius + (1.0 - fac) * startRadius;
		CollisionBox & cb = hitBody.GetCollisionBoxes(0)[0];
		cb.rw = currRadius;
		cb.rh = currRadius;
		cb.innerRadius = currRadius - waveThickness;
	}
}

void CoyoteShockwave::IHitPlayer(int index)
{
}

void CoyoteShockwave::UpdateEnemyPhysics()
{
}

void CoyoteShockwave::UpdateSprite()
{
	ts->SetQuadSubRect(quad, 0);
	SetRectCenter(quad, ts->tileWidth, ts->tileWidth, GetPositionF());
}

void CoyoteShockwave::EnemyDraw(sf::RenderTarget *target)
{
}

void CoyoteShockwave::HandleHitAndSurvive()
{
}

bool CoyoteShockwave::CheckHitPlayer(int index)
{
	Actor *player = sess->GetPlayer(index);

	if (player == NULL)
		return false;

	if (currHitboxes != NULL && currHitboxes->hitboxInfo != NULL)
	{
		V2d playerPos = player->position;


		bool hitboxFacingRight;
		if (playerPos.x - GetPosition().x > 0)
		{
			hitboxFacingRight = true;
		}
		else
		{
			hitboxFacingRight = false;
		}


		V2d pDir = normalize(playerPos - GetPosition());

		Actor::HitResult hitResult = player->CheckIfImHitByEnemy( this, currHitboxes, currHitboxFrame, HitboxInfo::GetAirType(pDir),
			GetPosition(), hitboxFacingRight,
			currHitboxes->hitboxInfo->canBeParried,
			currHitboxes->hitboxInfo->canBeBlocked);

		if (hitResult != Actor::HitResult::MISS)
		{
			IHitPlayer(index);
			if (currHitboxes != NULL) //needs a second check in case ihitplayer changes the hitboxes
			{
				player->ApplyHit(currHitboxes->hitboxInfo,
				NULL, hitResult, GetPosition());
			}
		}
	}


	return false;
}