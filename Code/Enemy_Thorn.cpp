#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Thorn.h"

using namespace std;
using namespace sf;


ThornPool::ThornPool()
{
	Session *sess = Session::GetSession();

	ts = NULL;
	numThorns = 10;//5;//10;
	thornVec.resize(numThorns);
	//verts = new Vertex[numLasers * 4];
	ts = sess->GetSizedTileset("Bosses/Coyote/coyotebullet_32x32.png");
	for (int i = 0; i < numThorns; ++i)
	{
		thornVec[i] = new Thorn(/*verts + 4 * i, */this);
	}
}


ThornPool::~ThornPool()
{
	for (int i = 0; i < numThorns; ++i)
	{
		delete thornVec[i];
	}

	//delete[] verts;
}

void ThornPool::Reset()
{
	for (int i = 0; i < numThorns; ++i)
	{
		thornVec[i]->Reset();
	}
}

Thorn * ThornPool::Throw(int thornType, V2d &pos, V2d &dir)
{
	Thorn *thorn = NULL;
	for (int i = 0; i < numThorns; ++i)
	{
		thorn = thornVec[i];
		if (!thorn->active)
		{
			thorn->Throw(thornType, pos, dir);
			break;
		}
	}
	return thorn;
}

Thorn * ThornPool::ThrowAt(int type, V2d &pos, PoiInfo *dest)
{
	Thorn *thorn = NULL;
	for (int i = 0; i < numThorns; ++i)
	{
		thorn = thornVec[i];
		if (!thorn->active)
		{
			thorn->ThrowAt(type, pos, dest);
			break;
		}
	}
	return thorn;
}

void ThornPool::DrawMinimap(sf::RenderTarget * target)
{
	for (auto it = thornVec.begin(); it != thornVec.end(); ++it)
	{
		if ((*it)->active)
		{
			(*it)->DrawMinimap(target);
		}
	}
}


Thorn::Thorn(/*sf::Vertex *myQuad, */ThornPool *pool)
	:Enemy(EnemyType::EN_THORN, NULL)
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, 0, 0);

	actionLength[IDLE] = 1;
	animFactor[IDLE] = 1;

	actionLength[GROW] = 100000;
	animFactor[GROW] = 1;

	actionLength[HOLD] = 10;
	animFactor[HOLD] = 1;

	actionLength[SHRINK] = 100000;
	animFactor[SHRINK] = 1;

	maxPastPositions = 300;//180; //300


	pastPositions.reserve(maxPastPositions);
	normals.resize(maxPastPositions);
	quads = new Vertex[maxPastPositions * 4];
	//quad = myQuad;

	ts = pool->ts;

	//origTimeToLive = 300;
	laserWidth = 10;
	lengthLimit = 1000;//500;
	flySpeed = 30;


	//hitBody.SetupNumFrames(1);
	//hitBody.SetupNumBoxesOnFrame(0, maxBounces);

	//hurtBody.SetupNumFrames(1);
	//hurtBody.SetupNumBoxesOnFrame(0, maxBounces);
	accel = .3;//.5;
	maxSpeed = 8;

	startWidth = 50;

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

	BasicCircleHurtBodySetup(16);
	//BasicCircleHitBodySetup(16);

	//BasicCircleHitBodySetup(16);
	hitBody.hitboxInfo = hitboxInfo;



	ResetEnemy();
}

Thorn::~Thorn()
{
	delete quads;
}

void Thorn::ResetEnemy()
{
	ClearQuads();

	numActivePositions = 0;
	//currBounce = 0;

	facingRight = true;

	//surfaceMover->collisionOn = true;

	action = GROW;
	frame = 0;

	//timeToLive = origTimeToLive;

	//anchorPositions.clear();
	pastPositions.clear();


	destPoi = NULL;

	//DefaultHitboxesOn();
	//DefaultHurtboxesOn();
	//UpdateHitboxes();

}

void Thorn::ClearQuads()
{
	for (int i = 0; i < maxPastPositions; ++i)
	{
		ClearRect(quads + i * 4);
	}
}

void Thorn::SetLevel(int lev)
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

void Thorn::SetThornTypeParams()
{
	switch (thornType)
	{
	case THORN_NORMAL:
	{
		accel = .3;//.5;
		maxSpeed = 8;
		thornColor = Color::White;
		currMaxPastPositions = maxPastPositions;
		startWidth = 50;
		break;
	}
	case THORN_FAST:
	{
		accel = 2.0;//.3;//.5;
		maxSpeed = 20;
		thornColor = Color::Green;
		currMaxPastPositions = 180;//120;
		startWidth = 50;
		break;
	}
	}
}

void Thorn::UpdateEnemyPhysics()
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

void Thorn::Throw(int p_thornType, V2d &pos, V2d &dir)
{
	thornType = (ThornType)p_thornType;

	SetThornTypeParams();

	//SetLaserTypeParams();

	Reset();
	sess->AddEnemy(this);

	/*if (type == LT_NO_COLLIDE)
	{
		surfaceMover->collisionOn = false;
	}*/

	surfaceMover->velocity = dir * min(10.0, maxSpeed);//maxSpeed;
	

	velocity = dir * maxSpeed;//5.0;

	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	surfaceMover->Set(currPosInfo);

	//numActivePositions = 1;
	numActivePositions = 0;


	DefaultHurtboxesOn();
	
	//pastPositions.push_back(pos);
	//anchorPositions.push_back(pos);

	UpdateHitboxes();
}

void Thorn::ThrowAt(int type, V2d &pos, PoiInfo *pi)
{
	thornType = (ThornType)type;

	//SetLaserTypeParams();

	Reset();
	sess->AddEnemy(this);

	/*if (type == LT_NO_COLLIDE)
	{
		surfaceMover->collisionOn = false;
	}*/

	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	//surfaceMover->Set(currPosInfo);

	//action = THROWN_AT;
	frame = 0;
	destPoi = pi;

	V2d diff = pi->edge->GetPosition(pi->edgeQuantity) - GetPosition();
	V2d dir = normalize(diff);

	//surfaceMover->collisionOn = false;
	//surfaceMover->velocity = dir * flySpeed;

	framesToArriveToDestPoi = ceil(length(diff) / flySpeed);

	//anchorPositions.push_back(pos);

	UpdateHitboxes();
}

void Thorn::FrameIncrement()
{
}

void Thorn::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case IDLE:
			break;
		case GROW:
			break;
		case HOLD:
			action = SHRINK;
			frame = 0;
			break;
		case SHRINK:
		{
			sess->RemoveEnemy(this);
			dead = true;
			numHealth = 0;
			break;
		}
		
		}
		frame = 0;
	}

	if (action == GROW)
	{
		if (numActivePositions == currMaxPastPositions)
		{
			action = HOLD;
			frame = 0;
		}
		else
		{
			//if (frame % 10 == 0)
			{
				pastPositions.push_back(GetPosition());
				++numActivePositions;
			}
		}

		surfaceMover->velocity += PlayerDir() * accel;
		if (length(surfaceMover->velocity) > maxSpeed)
		{
			surfaceMover->velocity = normalize(surfaceMover->velocity) * maxSpeed;
		}
	}
	else if (action == SHRINK)
	{
		if (numActivePositions == 0)
		{
			sess->RemoveEnemy(this);
			dead = true;
			numHealth = 0;
		}
		else
		{
			int shrinkMultiplier = 2;
			for( int i = 0; i < shrinkMultiplier; ++i )
			{
				pastPositions.pop_back();
				--numActivePositions;
				if (pastPositions.size() == 0)
				{
					break;
				}
			}
		}
	}
}

void Thorn::IHitPlayer(int index)
{
}

void Thorn::UpdateSprite()
{
	if (action != IDLE)
	{
		ClearQuads();

		for (int i = 0; i < maxPastPositions; ++i)
		{
			/*quads[i * 4 + 0].color = tailColor;
			quads[i * 4 + 1].color = headColor;
			quads[i * 4 + 2].color = headColor;
			quads[i * 4 + 3].color = tailColor;*/
			SetRectColor(quads + i * 4, thornColor);
			
		}

		V2d oldPos;
		V2d currPos;
		V2d nextPos;
		V2d along;
		V2d other;

		V2d currBisector;

		V2d along1;
		V2d along2;

		if (numActivePositions > 1)
		{
			normals[0] = normalize(pastPositions[1] - pastPositions[0]);
			normals[0] = V2d(normals[0].y, -normals[0].x);

			for (int i = 1; i < numActivePositions; ++i)
			{
				oldPos = pastPositions[i - 1];

				currPos = pastPositions[i];

				if (i < numActivePositions - 1)
				{
					nextPos = pastPositions[i + 1];

					along1 = normalize(nextPos - currPos);
					along2 = normalize(currPos - oldPos);
					normals[i] = normalize(along1 + along2);
				}
				else
				{
					normals[i] = normalize(currPos - oldPos);
				}

				
				normals[i] = V2d(normals[i].y, -normals[i].x);
			}


			
			double prevDistFromRoot = 0;
			double currDistFromRoot = 0;

			double prevWidthRatio = 0;
			double currWidthRatio = 0;


			double fullDist = 0;
			for (int i = 1; i < numActivePositions; ++i)
			{
				oldPos = pastPositions[i - 1];
				currPos = pastPositions[i];
				fullDist += length(currPos - oldPos);
			}

			for (int i = 1; i < numActivePositions; ++i)
			{
				oldPos = pastPositions[i - 1];
				currPos = pastPositions[i];

				prevDistFromRoot = currDistFromRoot;
				currDistFromRoot += length(pastPositions[i] - pastPositions[i - 1]);

				prevWidthRatio = ( 1.0 - (prevDistFromRoot / fullDist)) * startWidth;
				currWidthRatio = (1.0 - (currDistFromRoot / fullDist)) * startWidth;

				along = normalize(currPos - oldPos);
				other = V2d(along.y, -along.x);

				quads[(i - 1) * 4 + 0].position = Vector2f(oldPos - normals[i-1] * prevWidthRatio);
				quads[(i - 1) * 4 + 1].position = Vector2f(currPos - normals[i] * currWidthRatio);
				quads[(i - 1) * 4 + 2].position = Vector2f(currPos + normals[i] * currWidthRatio);
				quads[(i - 1) * 4 + 3].position = Vector2f(oldPos + normals[i-1] * prevWidthRatio);
			}

		}
		
	}
}

void Thorn::UpdateHitboxes()
{
	Enemy::UpdateHitboxes();
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

void Thorn::EnemyDraw(sf::RenderTarget *target)
{
	//laserBody.DebugDraw(0, target);
	target->draw(quads, currMaxPastPositions * 4, sf::Quads);
	//firePool.Draw(target);
}

//void SkeletonLaser::DebugDraw(sf::RenderTarget *target)
//{
//	hitBody.DebugDraw( 0, target);
//}

void Thorn::HandleHitAndSurvive()
{
}

bool Thorn::CanBeHitByPlayer()
{
	return false;
}

bool Thorn::CanBeHitByComboer()
{
	return false;
}

void Thorn::HitTerrainAerial(Edge * edge, double quant)
{
	surfaceMover->ground = NULL;
}