#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_PulseAttack.h"
#include "Actor.h"

using namespace std;
using namespace sf;

PulseAttackPool::PulseAttackPool( int p_numPulses)
{
	numPulses = p_numPulses;
	Session *sess = Session::GetSession();
	pulseVec.resize(numPulses);
	for (int i = 0; i < numPulses; ++i)
	{
		pulseVec[i] = new PulseAttack(this);
	}
}

PulseAttackPool::~PulseAttackPool()
{
	for (int i = 0; i < numPulses; ++i)
	{
		delete pulseVec[i];
	}
}

void PulseAttackPool::Reset()
{
	PulseAttack *pa = NULL;
	for (int i = 0; i < numPulses; ++i)
	{
		pa = pulseVec[i];
		if (pa->spawned)
		{
			pa->Die();
		}

		pa->Reset();
	}

	for (int i = 0; i < numPulses; ++i)
	{
		pulseVec[i]->Reset();
	}
}

bool PulseAttackPool::CanPulse()
{
	PulseAttack *pa = NULL;
	for (int i = 0; i < numPulses; ++i)
	{
		pa = pulseVec[i];
		if (!pa->spawned)
		{
			return true;
		}
	}
}

PulseAttack * PulseAttackPool::Pulse(
	int pulseType,
	V2d &pos, float pulseWidth,
	float pulseStartRadius,
	float pulseEndRadius,
	float pulseFrames,
	sf::Color startColor,
	sf::Color endColor, sf::Shader *sh )
{
	PulseAttack *pa = NULL;
	for (int i = 0; i < numPulses; ++i)
	{
		pa = pulseVec[i];
		if (!pa->spawned)
		{
			pa->Pulse(pulseType, pos, pulseWidth, pulseStartRadius, pulseEndRadius,
				pulseFrames, startColor, endColor, sh );
			return pa;
		}
	}

	return NULL;
}

PulseAttack::PulseAttack(PulseAttackPool *pool)
	:Enemy(EnemyType::EN_PULSE, NULL)
{
	SetNumActions(A_Count);
	SetEditorActions(A_PULSING, 0, 0);

	actionLength[A_PULSING] = 1;

	//startRadius = ts->tileWidth;
	//maxRadius = 200;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHitBodySetup(16);
	hitBody.hitboxInfo = hitboxInfo;

	myRing = new MovingRing(32, 20, 200, 10, 20, Vector2f(0, 0), Vector2f(0, 0),
		Color::Cyan, Color(0, 0, 100, 0), 60);
	geoGroup.AddGeo(myRing);
	geoGroup.Init();

	ResetEnemy();
}

void PulseAttack::ResetEnemy()
{
	facingRight = true;

	action = A_PULSING;
	frame = 0;

	geoGroup.Reset();

	DefaultHitboxesOn();

	UpdateHitboxes();
}

void PulseAttack::UpdateHitboxes()
{
	CollisionBox & cb = hitBody.GetCollisionBoxes(0)[0];
	cb.isRing = true;
	cb.rw = myRing->outerRadius;
	cb.rh = myRing->outerRadius;
	cb.innerRadius = myRing->innerRadius;

	BasicUpdateHitboxes();
	BasicUpdateHitboxInfo();
}

void PulseAttack::SetLevel(int lev)
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

void PulseAttack::Pulse(
	int pulseType,
	V2d &pos, float pulseWidth,
	float pulseStartRadius,
	float pulseEndRadius,
	float pulseFrames,
	sf::Color startColor,
	sf::Color endColor, sf::Shader *sh)
{
	Reset();
	sess->AddEnemy(this);
	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	action = A_PULSING;
	frame = 0;

	myRing->SetBase(Vector2f(pos));
	myRing->startWidth = pulseWidth;
	myRing->endWidth = pulseWidth;
	myRing->startInner = pulseStartRadius;
	myRing->endInner = pulseEndRadius;
	myRing->totalFrames = pulseFrames;

	myRing->startColor = startColor;
	myRing->endColor = endColor;
	myRing->SetShader(sh);

	myRing->Reset();

	geoGroup.Start();
}

void PulseAttack::FrameIncrement()
{
	
}

void PulseAttack::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();

	V2d diff = playerPos - position;
	V2d pDir = normalize(diff);

	//if (frame == actionLength[action] * animFactor[action])
	//{
	//	switch (action)
	//	{
	//	case FLYING:
	//		break;
	//	}
	//	frame = 0;
	//}

	geoGroup.Update();

	if (!geoGroup.data.running)
	{
		sess->RemoveEnemy(this);
		spawned = false;
	}


	


	//if (action == FLYING && quadraticMoveSeq.currMovement == NULL)
	//{
	//	action = GROWING;
	//	frame = 0;
	//}

	//if (framesToLive == 0)
	//{
	//	ClearRect(quad);

	//	sess->RemoveEnemy(this);
	//	spawned = false;
	//}

	//if (growing)//action == FLYING || action == GROWING)
	//{
	//	if (currRadius < maxRadius)
	//	{
	//		currRadius += 1;
	//	}

	//	if (currRadius >= maxRadius)
	//	{
	//		currRadius = maxRadius;
	//		growing = false;
	//	}

	//	hitBody.GetCollisionBoxes(0).at(0).rw = currRadius;
	//}
}

void PulseAttack::IHitPlayer(int index)
{
	HitboxesOff();
}

void PulseAttack::UpdateEnemyPhysics()
{
	/*if (action == FLYING)
	{

		quadraticMoveSeq.Update(slowMultiple, NUM_MAX_STEPS / numPhysSteps);

		currPosInfo.position = quadraticMoveSeq.position;
	}
	else
	{
		V2d movementVec = velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
	}*/

}

void PulseAttack::UpdateSprite()
{
	/*int tile = 0;
	if (orbType == NODE_GROW_HIT)
	{
		tile = 0;
	}
	else if (orbType == NODE_GROW_SLOW)
	{
		tile = 1;
	}
	ts->SetQuadSubRect(quad, tile);
	SetRectCenter(quad, currRadius * 2, currRadius * 2, GetPositionF());*/
	
}

void PulseAttack::EnemyDraw(sf::RenderTarget *target)
{
	geoGroup.Draw(target);
}

void PulseAttack::HandleHitAndSurvive()
{
}

void PulseAttack::Die()
{
	geoGroup.Reset();
	sess->RemoveEnemy(this);
	spawned = false;
	dead = true;
}

//bool PulseAttack::CheckHitPlayer(int index)
//{
//
//	Actor *player = sess->GetPlayer(index);
//
//	if (player == NULL)
//		return false;
//
//	//if (player->specialSlow)
//	//{
//	//	return false;
//	//}
//
//
//	if (currHitboxes != NULL && currHitboxes->hitboxInfo != NULL)
//	{
//		Actor::HitResult hitResult = player->CheckIfImHitByEnemy(this, currHitboxes, currHitboxFrame, currHitboxes->hitboxInfo->hitPosType,
//			GetPosition(), facingRight,
//			currHitboxes->hitboxInfo->canBeParried,
//			currHitboxes->hitboxInfo->canBeBlocked);
//
//		if (hitResult != Actor::HitResult::MISS)
//		{
//			//IHitPlayer(index);
//			if (currHitboxes != NULL) //needs a second check in case ihitplayer changes the hitboxes
//			{
//				if (orbType == GatorWaterOrb::OrbType::NODE_GROW_HIT)
//				{
//					if (hitResult != Actor::HitResult::INVINCIBLEHIT) //needs a second check in case ihitplayer changes the hitboxes
//					{
//						player->ApplyHit(currHitboxes->hitboxInfo,
//							NULL, hitResult, GetPosition());
//						Die();
//					}
//					//Die(); might be better out here
//				}
//				else
//				{
//					player->RestoreAirOptions();
//					player->specialSlow = true;
//				}
//
//				//Die();
//				//velocity = velocity *= .9;
//
//
//
//				/*player->ApplyHit(currHitboxes->hitboxInfo,
//				NULL, hitResult, GetPosition());*/
//			}
//		}
//	}
//
//
//	return false;
//}