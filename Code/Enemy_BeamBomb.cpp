#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_BeamBomb.h"
#include "Actor.h"

using namespace std;
using namespace sf;


BeamBombPool::BeamBombPool()
{
	Session *sess = Session::GetSession();

	ts = NULL;
	numBombs = 10;//5;//10;
	bombVec.resize(numBombs);
	//verts = new Vertex[numLasers * 4];
	ts = sess->GetSizedTileset("Bosses/Coyote/coyotebullet_32x32.png");
	for (int i = 0; i < numBombs; ++i)
	{
		bombVec[i] = new BeamBomb(/*verts + 4 * i, */this);
	}
}


BeamBombPool::~BeamBombPool()
{
	for (int i = 0; i < numBombs; ++i)
	{
		delete bombVec[i];
	}

	//delete[] verts;
}

void BeamBombPool::Reset()
{
	for (int i = 0; i < numBombs; ++i)
	{
		bombVec[i]->Reset();
	}
}

BeamBomb * BeamBombPool::Throw(int thornType, V2d &pos, V2d &dir)
{
	BeamBomb *bb = NULL;
	for (int i = 0; i < numBombs; ++i)
	{
		bb = bombVec[i];
		if (!bb->active)
		{
			bb->Throw(thornType, pos, dir);
			break;
		}
	}
	return bb;
}

//BeamBomb * BeamBombPool::ThrowAt(int type, V2d &pos, PoiInfo *dest)
//{
//	BeamBomb *bb = NULL;
//	for (int i = 0; i < numBombs; ++i)
//	{
//		bb = bombVec[i];
//		if (!bb->active)
//		{
//			bb->ThrowAt(type, pos, dest);
//			break;
//		}
//	}
//	return bb;
//}

void BeamBombPool::DrawMinimap(sf::RenderTarget * target)
{
	for (auto it = bombVec.begin(); it != bombVec.end(); ++it)
	{
		if ((*it)->active)
		{
			(*it)->DrawMinimap(target);
		}
	}
}


BeamBomb::BeamBomb(/*sf::Vertex *myQuad, */BeamBombPool *pool)
	:Enemy(EnemyType::EN_BEAMBOMB, NULL)//, cg( 20, 40, Color::Green, 12 )
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, 0, 0);

	actionLength[IDLE] = 1;
	animFactor[IDLE] = 1;

	actionLength[BLAST_TEST] = 100000;
	animFactor[BLAST_TEST] = 1;

	maxNumRays = 40;

	quads = new Vertex[maxNumRays * 4];
	rayHitPoints.resize(maxNumRays);
	//quad = myQuad;

	ts = pool->ts;

	rayLengthLimit = 8000;

	hitBody.SetupNumFrames(1);
	hitBody.SetupNumBoxesOnFrame(0, maxNumRays);

	
	rayWidth = 10;

	accel = .3;//.5;
	maxSpeed = 8;

	CreateSurfaceMover(startPosInfo, 16, this);
	surfaceMover->collisionOn = false;

	//highResPhysics = true;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	//BasicCircleHurtBodySetup(16);
	//BasicCircleHitBodySetup(16);

	//BasicCircleHitBodySetup(16);
	hitBody.hitboxInfo = hitboxInfo;



	ResetEnemy();
}

BeamBomb::~BeamBomb()
{
	if (quads != NULL)
	{
		delete quads;
	}
	
}

void BeamBomb::ResetEnemy()
{
	ClearQuads();

	//currBounce = 0;

	//cg.ShowAll();

	numRays = 0;

	facingRight = true;

	currRotation = 0;

	rayCastInfo.tree = sess->terrainTree;

	//surfaceMover->collisionOn = true;

	action = BLAST_TEST;
	frame = 0;

	destPoi = NULL;


	DefaultHitboxesOn();
	//DefaultHurtboxesOn();
	//UpdateHitboxes();

}

void BeamBomb::ClearQuads()
{
	/*for (int i = 0; i < maxPastPositions; ++i)
	{
		ClearRect(quads + i * 4);
	}*/
}

void BeamBomb::SetLevel(int lev)
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

void BeamBomb::SetBombTypeParams()
{
	switch (bombType)
	{
	case BOMB_NORMAL:
	{
		numRays = 4;
		currRotVel = .006 * PI;
		break;
	}
	case BOMB_SINGLE:
	{
		numRays = 1;
		currRotVel = .002 * PI;
		break;
	}
	}
	//switch (thornType)
	//{
	//case THORN_NORMAL:
	//{
	//	accel = .3;//.5;
	//	maxSpeed = 8;
	//	thornColor = Color::White;
	//	currMaxPastPositions = maxPastPositions;
	//	startWidth = 50;
	//	break;
	//}
	//case THORN_FAST:
	//{
	//	accel = 2.0;//.3;//.5;
	//	maxSpeed = 20;
	//	thornColor = Color::Green;
	//	currMaxPastPositions = 180;//120;
	//	startWidth = 50;
	//	break;
	//}
	//}
}

void BeamBomb::UpdateEnemyPhysics()
{
	Enemy::UpdateEnemyPhysics();
	/*if (thornType == THORN_PHYS)
	{
	if (action == GROW && frame > 0)
	{
	surfaceMover->velocity += PlayerDir() * accel;

	if (length(surfaceMover->velocity) > maxSpeed)
	{
	surfaceMover->velocity = normalize(surfaceMover->velocity) * maxSpeed;
	}
	}

	}
	else
	{
	V2d movementVec = velocity;
	movementVec /= slowMultiple * (double)numPhysSteps;

	currPosInfo.position += movementVec;

	if (action == GROW && frame > 0)
	{
	velocity += PlayerDir() * accel;

	if (length(velocity) > maxSpeed)
	{
	velocity = normalize(velocity) * maxSpeed;
	}
	}
	}*/
}

void BeamBomb::Throw(int p_bombType, V2d &pos, V2d &dir)
{
	bombType = (BeamBombType)p_bombType;

	//SetLaserTypeParams();

	Reset();
	sess->AddEnemy(this);

	SetBombTypeParams();

	/*if (type == LT_NO_COLLIDE)
	{
	surfaceMover->collisionOn = false;
	}*/

	currRotation = GetVectorAngleCCW(normalize(dir));

	surfaceMover->velocity = V2d(0, 0);//dir * min(10.0, maxSpeed);//maxSpeed;


	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	surfaceMover->Set(currPosInfo);


	DefaultHurtboxesOn();

	//pastPositions.push_back(pos);
	//anchorPositions.push_back(pos);

	UpdateHitboxes();
}

//void BeamBomb::ThrowAt(int type, V2d &pos, PoiInfo *pi)
//{
//	thornType = (ThornType)type;
//
//	//SetLaserTypeParams();
//
//	Reset();
//	sess->AddEnemy(this);
//
//	/*if (type == LT_NO_COLLIDE)
//	{
//	surfaceMover->collisionOn = false;
//	}*/
//
//	currPosInfo.position = pos;
//	currPosInfo.ground = NULL;
//
//	//surfaceMover->Set(currPosInfo);
//
//	//action = THROWN_AT;
//	frame = 0;
//	destPoi = pi;
//
//	V2d diff = pi->edge->GetPosition(pi->edgeQuantity) - GetPosition();
//	V2d dir = normalize(diff);
//
//	//surfaceMover->collisionOn = false;
//	//surfaceMover->velocity = dir * flySpeed;
//
//	framesToArriveToDestPoi = ceil(length(diff) / flySpeed);
//
//	//anchorPositions.push_back(pos);
//
//	UpdateHitboxes();
//}

void BeamBomb::FrameIncrement()
{
}

void BeamBomb::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case IDLE:
			break;
		case BLAST_TEST:
			break;
		/*case SHRINK:
		{
			sess->RemoveEnemy(this);
			dead = true;
			numHealth = 0;
			break;
		}*/

		}
		frame = 0;
	}

	if (action == BLAST_TEST )//&& frame == 0 )
	{
		V2d offset(rayLengthLimit, 0);
		RotateCW(offset, currRotation);
		V2d endPoint;
		V2d myPos = GetPosition();
		for (int i = 0; i < numRays; ++i)
		{
			endPoint = myPos + offset;
			if (ExecuteRayCast(myPos, myPos + offset ) )
			{
				V2d rayPos = rayCastInfo.GetRayHitPos();
				rayHitPoints[i] = rayPos;
			}
			else
			{
				rayHitPoints[i] = endPoint;
			}

			//if(IsEdgeTouchingQuad(myPos, rayHitPoints[i],)

			RotateCW(offset, 2 * PI / numRays);
		}

		currRotation += currRotVel;
	}
}

void BeamBomb::IHitPlayer(int index)
{
}

void BeamBomb::UpdateSprite()
{
	if (action != IDLE)
	{
		ClearQuads();

		V2d diff;
		for (int i = 0; i < numRays; ++i)
		{
			diff = rayHitPoints[i] - GetPosition();
			SetRectRotation(quads + i * 4, 
				GetVectorAngleCW(normalize(diff)), 
				length(diff), 
				rayWidth, 
				Vector2f((rayHitPoints[i] + GetPosition()) / 2.0));

			SetRectColor(quads + i * 4, Color::Red);

			/*cg.SetPosition(i, Vector2f(rayHitPoints[i]));

			if (i % 2 == 0)
			{
				cg.SetColor(i, Color::Magenta);
			}
			else
			{
				cg.SetColor(i, Color::Green);
			}*/
		}
		
	}
}

void BeamBomb::UpdateHitboxes()
{
	V2d myPos = GetPosition();
	V2d rayDest;
	V2d diff;
	V2d center;
	double angle;

	hitBody.ResetFrames();

	for (int i = 0; i < numRays; ++i)
	{
		rayDest = rayHitPoints[i];
		diff = rayDest - myPos;
		center = (myPos + rayDest) / 2.0;
		angle = GetVectorAngleCW(normalize(diff));
		hitBody.AddBasicRect(0, length(diff) / 2, rayWidth / 2, angle, center);
	}

	if (numRays > 0)
	{
		hitBody.SetBasicPos(V2d(0,0));
	}
	//Enemy::UpdateHitboxes();
	//if (pastPosit.empty())
	//{
	//	return;
	//}

	//hitBody.ResetFrames();
	//hurtBody.ResetFrames();


	//double totalLength = 0;

	//V2d currPos, tailPos, laserDir, laserCenter;
	//double laserAngle, laserLength;

	//currPos = GetPosition();
	//tailPos = anchorPositions[currBounce];
	//laserLength = length(currPos - tailPos);
	//laserDir = normalize(currPos - tailPos);

	//if (laserLength > lengthLimit)
	//{
	//	tailPos = currPos - laserDir * lengthLimit;
	//	totalLength = lengthLimit;
	//	laserLength = lengthLimit;
	//}
	//else
	//{
	//	totalLength += laserLength;
	//}

	//laserAngle = GetVectorAngleCW(laserDir);
	//laserCenter = (currPos + tailPos) / 2.0;

	//bool laserOn = false;
	//if (laserLength != 0)
	//{
	//	hitBody.AddBasicRect(0, laserLength / 2, laserWidth / 2, laserAngle, laserCenter);
	//	hurtBody.AddBasicRect(0, laserLength / 2, laserWidth / 2, laserAngle, laserCenter);
	//	laserOn = true;
	//}


	//for (int i = currBounce; i > 0; --i)
	//{
	//	if (totalLength >= lengthLimit)
	//	{
	//		break;
	//	}

	//	currPos = anchorPositions[i];
	//	tailPos = anchorPositions[i - 1];
	//	laserLength = length(currPos - tailPos);
	//	laserDir = normalize(currPos - tailPos);

	//	if (totalLength + laserLength > lengthLimit)
	//	{
	//		tailPos = currPos - laserDir * (lengthLimit - totalLength);
	//		laserLength = lengthLimit - totalLength;
	//		totalLength = lengthLimit;
	//	}
	//	else
	//	{
	//		totalLength += laserLength;
	//	}

	//	laserAngle = GetVectorAngleCW(laserDir);
	//	laserCenter = (currPos + tailPos) / 2.0;

	//	if (laserLength != 0)
	//	{
	//		hitBody.AddBasicRect(0, laserLength / 2, laserWidth / 2, laserAngle, laserCenter);
	//		hurtBody.AddBasicRect(0, laserLength / 2, laserWidth / 2, laserAngle, laserCenter);
	//		laserOn = true;
	//	}

	//	//SetRectRotation(quads + (currBounce - (i - 1)) * 4, laserAngle, laserLength, laserWidth, Vector2f(laserCenter));
	//}

	//if (laserOn)
	//{
	//	hitBody.SetBasicPos(V2d(0, 0));
	//	hurtBody.SetBasicPos(V2d(0, 0));
	//}
}

void BeamBomb::EnemyDraw(sf::RenderTarget *target)
{
	//laserBody.DebugDraw(0, target);
	target->draw(quads, numRays * 4, sf::Quads);
	//cg.Draw(target);
	//firePool.Draw(target);
}

//void SkeletonLaser::DebugDraw(sf::RenderTarget *target)
//{
//	hitBody.DebugDraw( 0, target);
//}

void BeamBomb::HandleHitAndSurvive()
{
}

bool BeamBomb::CanBeHitByPlayer()
{
	return false;
}

bool BeamBomb::CanBeHitByComboer()
{
	return false;
}

void BeamBomb::HitTerrainAerial(Edge * edge, double quant)
{
	surfaceMover->ground = NULL;
}

//void BeamBomb::HandleRayCollision(Edge *edge, double edgeQuantity,
//	double rayPortion)
//{
//	if (edge->edgeType == Edge::BORDER)
//	{
//		return;
//	}
//
//	V2d dir = normalize(rayCastInfo.rayEnd - rayCastInfo.rayStart);
//	V2d pos = edge->GetPosition(edgeQuantity);
//
//	//if (!sess->IsWithinBounds(pos))
//	//{
//	//	return; //prevents it from hitting the birdtransform area currently
//	//}
//
//	double along = dot(dir, edge->Normal());
//
//	double posDist = length(pos - GetPosition());
//
//	if (along < 0 && (rayCastInfo.rcEdge == NULL || length(edge->GetPosition(edgeQuantity) - rayCastInfo.rayStart) <
//		length(rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcPortion) - rayCastInfo.rayStart)))
//	{
//		//if (!ignorePointsCloserThanPlayer || (ignorePointsCloserThanPlayer && posDist > playerDist))
//		{
//			rayCastInfo.rcEdge = edge;
//			rayCastInfo.rcQuant = edgeQuantity;
//		}
//	}
//}

//bool BeamBomb::CheckHitPlayer(int index)
//{
//
//	Actor *player = sess->GetPlayer(index);
//
//	if (player == NULL)
//		return false;
//
//
//	if (currHitboxes != NULL && currHitboxes->hitboxInfo != NULL)
//	{
//		Actor::HitResult hitResult = player->CheckIfImHitByEnemy(this, currHitboxes, currHitboxFrame, 
//			currHitboxes->hitboxInfo->hitPosType, GetPosition(), facingRight, 
//			currHitboxes->hitboxInfo->canBeParried, currHitboxes->hitboxInfo->canBeBlocked);
//
//		if (hitResult != Actor::HitResult::MISS)
//		{
//			if (hitResult != Actor::HitResult::INVINCIBLEHIT) //needs a second check in case ihitplayer changes the hitboxes
//			{
//				if (hitResult == Actor::HitResult::FULLBLOCK)
//				{
//					IHitPlayerShield(index);
//				}
//				else
//				{
//					IHitPlayer(index);
//				}
//
//				if (currHitboxes->hitboxInfo != NULL)
//				{
//					pauseFrames = currHitboxes->hitboxInfo->hitlagFrames;
//					pauseBeganThisFrame = true;
//					pauseFramesFromAttacking = true;
//				}
//				player->ApplyHit(currHitboxes->hitboxInfo,
//					NULL, hitResult, GetPosition());
//			}
//		}
//	}
//
//
//	return false;
//}