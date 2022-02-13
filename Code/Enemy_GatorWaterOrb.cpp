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
	
	EndCircle();

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

int GatorWaterOrbPool::GetNumActive()
{
	int numActive = 0;
	for (int i = 0; i < numBullets; ++i)
	{
		if (bulletVec[i]->active)
		{
			++numActive;
		}
	}

	return numActive;
}

void GatorWaterOrbPool::RotateCircle(double rotSpeed, double rotAccel,
	double maxRotSpeed)
{
	circleRotateSpeed = rotSpeed;
	circleRotateAccel = rotAccel;
	circleRotateMaxSpeed = maxRotSpeed;
}

void GatorWaterOrbPool::ExpandCircle(double expandSpeed,
	double accel, double maxExpandSpeed)
{
	useCircleGoalRadius = false;
	circleExpandSpeed = expandSpeed;
	circleExpandAccel = accel;
	circleExpandMaxSpeed = maxExpandSpeed;
	if (circleExpandMaxSpeed == 0)
	{
		circleExpandMaxSpeed = circleExpandSpeed;
	}
}

void GatorWaterOrbPool::ExpandCircleToRadius( double endRadius, double expandSpeed,
	double accel, double maxExpandSpeed)
{
	useCircleGoalRadius = true;
	circleGoalRadius = endRadius;
	circleExpandSpeed = expandSpeed;
	circleExpandAccel = accel;
	circleExpandMaxSpeed = maxExpandSpeed;
	if (circleExpandMaxSpeed == 0)
	{
		circleExpandMaxSpeed = circleExpandSpeed;
	}
}

bool GatorWaterOrbPool::IsChangingSize()
{
	return circleExpandSpeed == 0 && circleExpandAccel == 0;
}

void GatorWaterOrbPool::ChangeAllCircleOrbsRadiusOverTime(double p_orbGrowSpeed, double goalRadius)
{
	GatorWaterOrb *orb = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		orb = bulletVec[i];
		if (orb->active)
		{
			orb->ChangeRadiusOverTime(p_orbGrowSpeed, goalRadius);
		}
	}
}

void GatorWaterOrbPool::SetCircleFollowPos(V2d *p_followTarget)
{
	followTarget = p_followTarget;

	if (followTarget != NULL)
	{
		followOffset = circleCenter - *followTarget;
	}
}

void GatorWaterOrbPool::StopCircleFollow()
{
	//circleCenter

	followTarget = NULL;
	followOffset = V2d();
	

}

void GatorWaterOrbPool::CreateCircle(V2d &pos, int numOrbs,
	double radius, double orbRadius, double startAngle,
	int p_orbType )
{
	EndCircle();

	action = CIRCLE;

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
			orb->CreateForCircle(centerPos + offset, orbRadius,
				p_orbType);
			RotateCW(offset, angleChange);
			++numOrbsMade;

			if (numOrbsMade == numOrbs)
			{
				break;
			}
		}
	}
}

void GatorWaterOrbPool::EndCircle()
{
	circleRotateSpeed = 0;
	circleRotateAccel = 0;
	circleRotateMaxSpeed = 0;

	circleExpandSpeed = 0;
	circleExpandAccel = 0;
	circleExpandMaxSpeed = 0;

	followTarget = NULL;
	followOffset = V2d();

	circleVel = V2d(0, 0);

	action = NORMAL;
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
		if (bs->active )
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
		if (bs->active && bs->growthFactor != 0 )
		{
			++numGrowing;
		}
	}

	return numGrowing;
}

void GatorWaterOrbPool::GroupChase(V2d *target, double p_chaseAccel, double p_chaseMaxSpeed)
{
	chaseTarget = target;
	GatorWaterOrb *bs = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		bs = bulletVec[i];
		if (bs->active )// bs->action == GatorWaterOrb::GROWING)
		{
			bs->GroupChase( p_chaseAccel, p_chaseMaxSpeed);
		}
	}
}

void GatorWaterOrbPool::Chase(V2d *target,
	double p_chaseAccel, double p_maxChaseSpeed )
{
	chaseTarget = target;
	GatorWaterOrb *bs = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		bs = bulletVec[i];
		if (bs->active)// && bs->action == GatorWaterOrb::GROWING)
		{
			bs->Chase( p_chaseAccel, p_maxChaseSpeed);
		}
	}
}

void GatorWaterOrbPool::CircleChase(V2d *target, double p_chaseAccel,
	double p_chaseMaxSpeed)
{
	assert(followTarget == NULL);
	/*if (followTarget != NULL)
	{
		StopCircleFollow();
	}*/
	chaseTarget = target;
	chaseAccel = p_chaseAccel;
	chaseMaxSpeed = p_chaseMaxSpeed;
}

void GatorWaterOrbPool::Update()
{
	if (action == CIRCLE)
	{
		GatorWaterOrb *orb = NULL;

		V2d oldCircleCenter = circleCenter;
		V2d circleTestOffset;
		if (followTarget != NULL)
		{
			circleCenter = *followTarget + followOffset;
			circleTestOffset = circleCenter - oldCircleCenter;
			followOffset += circleVel;
		}
		else
		{
			circleCenter += circleVel;
		}

		if (chaseTarget != NULL)
		{
			V2d accelDir = normalize(*chaseTarget - circleCenter);
			circleVel += accelDir * chaseAccel;
			if (length(circleVel) > chaseMaxSpeed)
			{
				circleVel = normalize(circleVel) * chaseMaxSpeed;
			}
		}
		

		V2d centerPos = circleCenter;
		//V2d offset(circleRadius, 0);
		V2d orbCenter;

		V2d orbOffset;

		V2d dir;

		circleRadius += circleExpandSpeed;

		if (circleExpandSpeed != 0)
		{
			if (useCircleGoalRadius)
			{
				if ((circleExpandSpeed > 0 && circleRadius > circleGoalRadius)
					|| (circleExpandSpeed < 0 && circleRadius < circleGoalRadius))
				{
					circleRadius = circleGoalRadius;
					circleExpandSpeed = 0;
					circleExpandAccel = 0;
					circleExpandMaxSpeed = 0;
				}
				else
				{

				}
			}
		}

		circleRotateSpeed += circleRotateAccel;

		if (circleRotateAccel > 0 && circleRotateSpeed > circleRotateMaxSpeed)
		{
			circleRotateSpeed = circleRotateMaxSpeed;
		}
		else if (circleRotateAccel < 0 && circleRotateSpeed < circleRotateMaxSpeed)
		{
			circleRotateSpeed = circleRotateMaxSpeed;
		}

		circleExpandSpeed += circleExpandAccel;

		if ( circleExpandAccel > 0 && circleExpandSpeed > 0 && circleExpandSpeed > circleExpandMaxSpeed)
		{
			circleExpandSpeed = circleExpandMaxSpeed;
		}
		else if ( circleExpandAccel < 0 && circleExpandSpeed < 0 && circleExpandSpeed < -circleExpandMaxSpeed)
		{
			circleExpandSpeed = circleExpandMaxSpeed;
		}

		V2d oldOrbPos;
		for (int i = 0; i < numBullets; ++i)
		{
			orb = bulletVec[i];
			if (orb->active)
			{
				oldOrbPos = orb->GetPosition();

				if (followTarget != NULL)
				{
					orbCenter = orb->GetPosition() + circleTestOffset;
				}
				else
				{
					orbCenter = orb->GetPosition() + circleVel;
				}
				orbOffset = orbCenter - circleCenter;

				dir = normalize(orbOffset);
				orbOffset = dir * circleRadius;

				RotateCW(orbOffset, circleRotateSpeed);
				orb->currPosInfo.SetPosition(circleCenter + orbOffset);

				//doesn't make the orb move, but useful for when
				//you switch to chase mode
				orb->velocity = orb->currPosInfo.position - oldOrbPos;

				
			}
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

void GatorWaterOrbPool::StopCircleChase()
{
	chaseTarget = NULL;
}

void GatorWaterOrbPool::SetCircleTimeToLive(int frames)
{
	GatorWaterOrb *bs = NULL;
	for (int i = 0; i < numBullets; ++i)
	{
		bs = bulletVec[i];
		if (bs->active && bs->action == GatorWaterOrb::CIRCLE_APPEAR)
		{
			bs->framesToLive = frames;
		}
	}
}

void GatorWaterOrbPool::SetCircleVelocity(V2d &vel)
{
	circleVel = vel;
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
	maxChaseSpeed = 15;

	ResetEnemy();
}

void GatorWaterOrb::ResetEnemy()
{
	ClearRect(quad);

	facingRight = true;

	action = FLYING;
	frame = 0;

	growthFactor = 0;

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

void GatorWaterOrb::GroupChase(double p_chaseAccel, double p_chaseMaxSpeed)
{
	action = GROUP_CHASE;
	frame = 0;
	velocity = V2d(0, 0);
	startChasingPos = GetPosition();

	chaseAccel = p_chaseAccel;
	maxChaseSpeed = p_chaseMaxSpeed;
}

void GatorWaterOrb::Chase( double p_chaseAccel, double p_chaseMaxSpeed )
{
	action = CHASE;
	frame = 0;
	//velocity = V2d(0, 0); //maintain velocity
	startChasingPos = GetPosition();

	chaseAccel = p_chaseAccel;
	maxChaseSpeed = p_chaseMaxSpeed;

}

void GatorWaterOrb::CreateForCircle(V2d &pos, double orbRadius,
	int p_orbType)
{
	Reset();

	sess->AddEnemy(this);

	action = CIRCLE_APPEAR;
	frame = 0;
	velocity = V2d(0, 0);

	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	growthFactor = 0;
	velocity = V2d(0, 0);
	framesToLive = -1;//origFramesToLive;

	orbType = p_orbType;

	flySpeed = 10;

	currRadius = orbRadius;
}


void GatorWaterOrb::Throw(V2d &pos, V2d &dir, int p_orbType )
{
	Reset();
	sess->AddEnemy(this);
	currPosInfo.position = pos;
	currPosInfo.ground = NULL;
	
	growthFactor = 0;
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
	growthFactor = 0;
}

void GatorWaterOrb::FrameIncrement()
{
	--framesToLive;
}

void GatorWaterOrb::SetRadius(double rad)
{
	currRadius = rad;
	hitBody.GetCollisionBoxes(0).at(0).rw = currRadius;
}

void GatorWaterOrb::ChangeRadiusOverTime(double p_growthFactor,
	double p_goalRadius)
{
	goalRadius = p_goalRadius;
	growthFactor = p_growthFactor;
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
		action = FLOATING;
		frame = 0;
	}

	if (framesToLive == 0)
	{
		ClearRect(quad);

		sess->RemoveEnemy(this);
		spawned = false;
	}

	if (growthFactor > 0)
	{
		if (currRadius < goalRadius)
		{
			SetRadius(min(currRadius + growthFactor, goalRadius));

			if (currRadius == goalRadius)
			{
				growthFactor = 0;
			}
		}
	}
	else if (growthFactor < 0)
	{
		if (currRadius > goalRadius)
		{
			SetRadius(max(currRadius + growthFactor, goalRadius));

			if (currRadius == goalRadius)
			{
				growthFactor = 0;
			}
		}
	}
		/*if (currRadius < maxRadius)
		{
			SetRadius(currRadius + 1);
		}
		
		if (currRadius >= maxRadius)
		{
			SetRadius(maxRadius);
			growing = false;
		}*/

	if (action == GROUP_CHASE)
	{
		V2d activeCenter = pool->GetActiveCenter();//GetPosition();//
		V2d chasePos = *pool->chaseTarget;

		V2d diff = chasePos - activeCenter;
		V2d dir = normalize(diff);

		//double sizeFactor = (currRadius-startRadius) / (maxRadius - startRadius);
		//double currMaxChaseSpeed = maxChaseSpeed - sizeFactor * 10;

		double currMaxChaseSpeed = maxChaseSpeed;

		velocity += dir * chaseAccel;

		if (length(velocity) > currMaxChaseSpeed)
		{
			velocity = normalize(velocity) * currMaxChaseSpeed;
		}
	}
	else if (action == CHASE)
	{
		V2d myPos =GetPosition();
		V2d chasePos = *pool->chaseTarget;

		V2d diff = chasePos - myPos;
		V2d dir = normalize(diff);

		//double sizeFactor = (currRadius - startRadius) / (maxRadius - startRadius);
		//double currMaxChaseVel = maxChaseVel - sizeFactor * 10;

		double currMaxChaseSpeed = maxChaseSpeed;

		velocity += dir * chaseAccel;

		if (length(velocity) > currMaxChaseSpeed)
		{
			velocity = normalize(velocity) * currMaxChaseSpeed;
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
	else if (action == CIRCLE_APPEAR)
	{

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