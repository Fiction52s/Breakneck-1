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
	chaseTarget = NULL;
	circleRotateSpeed = 0;
	circleRotateAccel = 0;
	circleRotateMaxSpeed = 0;

	circleExpandSpeed = 0;
	circleExpandAccel = 0;
	circleExpandMaxSpeed = 0;

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

void GatorWaterOrbPool::RotateCircle(double rotSpeed, double rotAccel,
	double maxRotSpeed)
{
	circleRotateSpeed = rotSpeed;
	circleRotateAccel = rotAccel;
	circleRotateMaxSpeed = maxRotSpeed;
	if (circleRotateMaxSpeed == 0)
	{
		circleRotateMaxSpeed = circleRotateSpeed;
	}
}

void GatorWaterOrbPool::ExpandCircle(double expandSpeed,
	double accel, double maxExpandSpeed)
{
	circleExpandSpeed = expandSpeed;
	circleExpandAccel = accel;
	circleExpandMaxSpeed = maxExpandSpeed;
	if (circleExpandMaxSpeed == 0)
	{
		circleExpandMaxSpeed = circleExpandSpeed;
	}
}

void GatorWaterOrbPool::CreateCircle(V2d &pos, int numOrbs,
	double radius, double orbRadius, double startAngle)
{
	GatorWaterOrb *orb = NULL;

	circleCenter = pos;
	circleRadius = radius;

	V2d centerPos = pos;
	V2d offset(radius, 0);

	RotateCW(offset, startAngle);

	double angleChange = (2 * PI) / numOrbs;

	int numOrbsMade = 0;
	for (int i = 0; i < numBullets; ++i)
	{
		orb = bulletVec[i];
		if (!orb->active)
		{
			orb->CreateForCircle(centerPos + offset, orbRadius);
			RotateCW(offset, angleChange);
			++numOrbsMade;

			if (numOrbsMade == numOrbs)
			{
				break;
			}
		}
	}
}

V2d GatorWaterOrbPool::GetActiveCenter()
{
	GatorWaterOrb *orb = NULL;
	V2d center;
	V2d total;
	int numActive = 0;
	for (int i = 0; i < numBullets; ++i)
	{
		orb = bulletVec[i];
		if (orb->active)
		{
			total += orb->GetPosition();
			++numActive;
		}
	}

	center = total / (double)numActive;

	return center;
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

void GatorWaterOrbPool::GroupChase(V2d *target)
{
	chaseTarget = target;
	GatorWaterOrb *bs = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		bs = bulletVec[i];
		if (bs->active && bs->action == GatorWaterOrb::GROWING)
		{
			bs->GroupChase();
		}
	}
}

void GatorWaterOrbPool::Update()
{
	GatorWaterOrb *orb = NULL;

	V2d centerPos = circleCenter;
	//V2d offset(circleRadius, 0);
	V2d orbCenter;

	V2d orbOffset;

	V2d dir;

	circleRadius += circleExpandSpeed;

	circleRotateSpeed += circleRotateAccel;

	if (circleRotateSpeed > 0 && circleRotateSpeed > circleRotateMaxSpeed)
	{
		circleRotateSpeed = circleRotateMaxSpeed;
	}
	else if (circleRotateSpeed < 0 && circleRotateSpeed < -circleRotateMaxSpeed)
	{
		circleRotateSpeed = -circleRotateMaxSpeed;
	}

	circleExpandSpeed += circleExpandAccel;
	
	if (circleExpandSpeed > 0 && circleExpandSpeed > circleExpandMaxSpeed)
	{
		circleExpandSpeed = circleExpandMaxSpeed;
	}
	else if (circleExpandSpeed < 0 && circleExpandSpeed < -circleExpandMaxSpeed)
	{
		circleExpandSpeed = -circleExpandMaxSpeed;
	}

	for (int i = 0; i < numBullets; ++i)
	{
		orb = bulletVec[i];
		if (orb->active)
		{
			orbCenter = orb->GetPosition();
			orbOffset = orbCenter - circleCenter;

			if (circleExpandSpeed != 0)
			{
				dir = normalize(orbOffset);
				orbOffset = dir * circleRadius;
			}

			RotateCW(orbOffset, circleRotateSpeed);
			orb->currPosInfo.SetPosition(circleCenter + orbOffset);
		}
	}
}

void GatorWaterOrbPool::StopChase()
{
	chaseTarget = NULL;
	GatorWaterOrb *bs = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		bs = bulletVec[i];
		if (bs->active && bs->action == GatorWaterOrb::GROUP_CHASE)
		{
			bs->Die();
		}
	}	
}


void GatorWaterOrbPool::Draw(sf::RenderTarget *target)
{
	target->draw(verts, bulletVec.size() * 4, sf::Quads, ts->texture);
}

GatorWaterOrb::GatorWaterOrb(sf::Vertex *myQuad, GatorWaterOrbPool *p_pool)
	:Enemy(EnemyType::EN_GATORORB, NULL)
{
	SetNumActions(A_Count);
	SetEditorActions(FLYING, 0, 0);

	actionLength[FLYING] = 1;
	animFactor[FLYING] = 1;

	quad = myQuad;

	pool = p_pool;

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

	chaseAccel = 1.0;
	maxChaseVel = 15;

	ResetEnemy();
}

void GatorWaterOrb::ResetEnemy()
{
	ClearRect(quad);

	facingRight = true;

	action = FLYING;
	frame = 0;

	growing = false;

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

void GatorWaterOrb::GroupChase()
{
	action = GROUP_CHASE;
	frame = 0;
	velocity = V2d(0, 0);
	startChasingPos = GetPosition();
}

void GatorWaterOrb::CreateForCircle(V2d &pos, double orbRadius )
{
	Reset();

	sess->AddEnemy(this);

	action = CIRCLE_APPEAR;
	frame = 0;
	velocity = V2d(0, 0);

	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	growing = false;
	velocity = V2d(0, 0);
	framesToLive = -1;//origFramesToLive;

	orbType = NODE_GROW_HIT;

	flySpeed = 10;

	currRadius = orbRadius;
}


void GatorWaterOrb::Throw(V2d &pos, V2d &dir, int p_orbType )
{
	Reset();
	sess->AddEnemy(this);
	currPosInfo.position = pos;
	currPosInfo.ground = NULL;
	
	growing = true;
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
	growing = false;
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

	if (growing )//action == FLYING || action == GROWING)
	{
		if (currRadius < maxRadius)
		{
			currRadius += 1;
		}
		
		if (currRadius >= maxRadius)
		{
			currRadius = maxRadius;
			growing = false;
		}

		hitBody.GetCollisionBoxes(0).at(0).rw = currRadius;
	}

	if (action == GROUP_CHASE)
	{
		V2d activeCenter = pool->GetActiveCenter();//GetPosition();//
		V2d chasePos = *pool->chaseTarget;

		V2d diff = chasePos - activeCenter;
		V2d dir = normalize(diff);

		double sizeFactor = (currRadius-startRadius) / (maxRadius - startRadius);
		double currMaxChaseVel = maxChaseVel - sizeFactor * 10;

		currMaxChaseVel = maxChaseVel;

		velocity += dir * chaseAccel;

		if (length(velocity) > currMaxChaseVel)
		{
			velocity = normalize(velocity) * currMaxChaseVel;
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