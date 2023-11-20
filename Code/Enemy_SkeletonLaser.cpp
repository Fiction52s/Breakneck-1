#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_SkeletonLaser.h"

using namespace std;
using namespace sf;


SkeletonLaserPool::SkeletonLaserPool( int maxLasers )
{
	Session *sess = Session::GetSession();

	ts = NULL;
	numLasers = maxLasers;//100;//5;//10;
	laserVec.resize(numLasers);
	//verts = new Vertex[numLasers * 4];
	ts = sess->GetSizedTileset("Enemies/Bosses/Coyote/coyotebullet_32x32.png");
	for (int i = 0; i < numLasers; ++i)
	{
		laserVec[i] = new SkeletonLaser(/*verts + 4 * i, */this);
	}
}


SkeletonLaserPool::~SkeletonLaserPool()
{
	for (int i = 0; i < numLasers; ++i)
	{
		delete laserVec[i];
	}

	//delete[] verts;
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
		if (!bs->active)
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
		if (!bs->active)
		{
			bs->ThrowAt(type, pos, dest);
			break;
		}
	}
	return bs;
}

void SkeletonLaserPool::RedirectAllTowards(V2d &pos)
{
	SkeletonLaser *bs = NULL;
	for (int i = 0; i < numLasers; ++i)
	{
		bs = laserVec[i];
		if (bs->active && bs->laserType != SkeletonLaser::LT_NO_COLLIDE
			&& bs->action != SkeletonLaser::REDIRECTED 
			&& bs->action != SkeletonLaser::DISSIPATE)
		{
			bs->RedirectTowards(pos);
		}
	}
}

void SkeletonLaserPool::SetAllSpeed(double speed)
{
	SkeletonLaser *bs = NULL;
	for (int i = 0; i < numLasers; ++i)
	{
		bs = laserVec[i];
		if (bs->active)
		{
			bs->SetSpeed(speed);
		}
	}
}

void SkeletonLaserPool::SetAllSpeedDefault()
{
	SkeletonLaser *bs = NULL;
	for (int i = 0; i < numLasers; ++i)
	{
		bs = laserVec[i];
		if (bs->active)
		{
			bs->SetSpeedDefault();
		}
	}
}

void SkeletonLaserPool::SetEnemyIDsAndAddToGame()
{
	SkeletonLaser *bs = NULL;
	for (int i = 0; i < numLasers; ++i)
	{
		bs = laserVec[i];
		bs->SetEnemyIDAndAddToAllEnemiesVec();
	}
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

//void SkeletonLaserPool::DebugDraw(sf::RenderTarget *target)
//{
//	target->
//}

SkeletonLaser::SkeletonLaser(/*sf::Vertex *myQuad, */SkeletonLaserPool *pool)
	:Enemy(EnemyType::EN_SKELETONLASER, NULL)
{
	SetNumActions(A_Count);
	SetEditorActions(THROWN, 0, 0);

	actionLength[THROWN] = 1;
	animFactor[THROWN] = 1;

	actionLength[THROWN_AT] = 1;
	animFactor[THROWN] = 1;

	actionLength[DISSIPATE] = 10;
	animFactor[DISSIPATE] = 1;

	maxBounces = 100;
	

	anchorPositions.reserve(maxBounces + 1);
	quads = new Vertex[maxBounces * 4];
	//quad = myQuad;

	ts = pool->ts;

	origTimeToLive = 300;
	laserWidth = 10;
	lengthLimit = 1000;//500;
	flySpeed = 30;


	hitBody.SetupNumFrames(1);
	hitBody.SetupNumBoxesOnFrame(0, maxBounces);

	hurtBody.SetupNumFrames(1);
	hurtBody.SetupNumBoxesOnFrame(0, maxBounces);
	//hitBody.boxType = CollisionBox::BoxType::Hit;
	//CollisionBox cb;
	//cb.AddBasicRect(0, w, h, angle, offset);
	//laserBody.AddCollisionBox()
	
	

	CreateSurfaceMover(startPosInfo, laserWidth, this);

	highResPhysics = true;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	hitboxInfo->knockbackOnBlock = false;

	

	//BasicCircleHitBodySetup(16);
	hitBody.hitboxInfo = hitboxInfo;

	

	ResetEnemy();
}

SkeletonLaser::~SkeletonLaser()
{
	delete quads;
}

void SkeletonLaser::ResetEnemy()
{
	ClearQuads();
	
	currBounce = 0;

	facingRight = true;

	surfaceMover->SetCollisionOn(true);

	action = THROWN;
	frame = 0;

	timeToLive = origTimeToLive;

	anchorPositions.clear();

	destPoi = NULL;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();
	//UpdateHitboxes();

}

void SkeletonLaser::ClearQuads()
{
	for (int i = 0; i < maxBounces; ++i)
	{
		ClearRect(quads + i * 4);
	}
}

void SkeletonLaser::RedirectTowards(V2d &pos)
{
	V2d newVel = normalize((pos - GetPosition())) 
		* length( surfaceMover->GetVel() );
	TryBounce(newVel);
	action = REDIRECTED;
	frame = 0;
	timeToLive = 180;
	//laserWidth = 30;
	surfaceMover->SetCollisionOn(false);
}

void SkeletonLaser::Dissipate()
{
	action = DISSIPATE;
	ClearQuads();
	frame = 0;
	surfaceMover->SetVelocity(V2d(0, 0));
	surfaceMover->SetCollisionOn(false);
	HitboxesOff();
	HurtboxesOff();
}

void SkeletonLaser::TryBounce(V2d &newVel)
{
	if (currBounce == maxBounces)
	{
		Dissipate();
	}
	else
	{
		anchorPositions.push_back(GetPosition());

		surfaceMover->Jump(newVel);

		++currBounce;
	}
}

void SkeletonLaser::HitTerrainAerial(Edge *e, double quant)
{
	V2d newVel;
	
	V2d en = e->Normal();
	V2d pos = e->GetPosition(quant);

	if (pos == e->v0)
	{
		en = normalize(GetPosition() - pos);
	}
	else if (pos == e->v1)
	{
		en = normalize(GetPosition() - pos);
	}
	double d = dot(surfaceMover->GetVel(), en);
	V2d ref = surfaceMover->GetVel() - (2.0 * d * en);
	
	V2d playerDir = PlayerDir();

	if (laserType == LT_HOME && dot(en, playerDir) > 0)
	{
		newVel = playerDir * length(ref);
	}
	else
	{
		newVel = ref;
	}
	

	TryBounce(newVel);
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

void SkeletonLaser::SetSpeed(double speed)
{
	surfaceMover->SetVelocity(normalize(surfaceMover->GetVel()) * speed);
}

void SkeletonLaser::SetSpeedDefault()
{
	surfaceMover->SetVelocity(normalize(surfaceMover->GetVel()) * flySpeed);
}

void SkeletonLaser::SetLaserTypeParams()
{
	switch (laserType)
	{
	case LT_REGULAR:
	{
		origTimeToLive = 60 * 30;
		laserWidth = 10;
		lengthLimit = 1000;//1500;//500;
		flySpeed = 30;
		headColor = Color::Magenta;
		tailColor = Color::Magenta;
		tailColor.a = 50;
		break;
	}
	case LT_LONG:
	{
		origTimeToLive = 60 * 30;
		laserWidth = 20;
		lengthLimit = 1500;
		flySpeed = 20;
		headColor = Color::Red;
		tailColor = Color::Red;
		tailColor.a = 50;
		break;
	}
	case LT_HOME:
	{
		origTimeToLive = 60 * 15;//30;
		laserWidth = 20;
		lengthLimit = 700;//2000;
		flySpeed = 40;
		headColor = Color::Green;
		tailColor = Color::Green;
		tailColor.a = 50;
		break;
	}
	case LT_NO_COLLIDE:
	{
		origTimeToLive = 60 * 3;//60 * 30;
		laserWidth = 6;
		lengthLimit = 50;
		flySpeed = 30;
		headColor = Color::Cyan;
		tailColor = Color::Cyan;
		tailColor.a = 50;
		break;
	}
	case LT_ENEMY_WOLF:
	{
		origTimeToLive = 180;//60 * 30;
		laserWidth = 10;
		lengthLimit = 400;//1000;//1500;//500;
		flySpeed = 30;
		headColor = Color::Magenta;
		tailColor = Color::Magenta;
		tailColor.a = 50;
		break;
	}
	}
}

void SkeletonLaser::Throw(int type, V2d &pos, V2d &dir)
{
	laserType = (LaserType)type;

	SetLaserTypeParams();

	Reset();
	sess->AddEnemy(this);

	if (type == LT_NO_COLLIDE)
	{
		surfaceMover->SetCollisionOn(false);
	}

	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	surfaceMover->Set(currPosInfo);

	surfaceMover->SetVelocity(dir * flySpeed);

	anchorPositions.push_back(pos);

	UpdateHitboxes();
}

void SkeletonLaser::ThrowAt(int type, V2d &pos, PoiInfo *pi)
{
	laserType = (LaserType)type;

	SetLaserTypeParams();

	Reset();
	sess->AddEnemy(this);

	if (type == LT_NO_COLLIDE)
	{
		surfaceMover->SetCollisionOn(false);
	}

	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	surfaceMover->Set(currPosInfo);

	action = THROWN_AT;
	frame = 0;
	destPoi = pi;

	V2d diff = pi->edge->GetPosition(pi->edgeQuantity) - GetPosition();
	V2d dir = normalize(diff);

	surfaceMover->SetCollisionOn(false);
	surfaceMover->SetVelocity(dir * flySpeed);

	framesToArriveToDestPoi = ceil(length(diff) / flySpeed);

	anchorPositions.push_back(pos);

	UpdateHitboxes();
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

	if (action != DISSIPATE)
	{
		--timeToLive;
		if (timeToLive == 0)
		{
			Dissipate();
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
		case DISSIPATE:
			sess->RemoveEnemy(this);
			dead = true;
			numHealth = 0;
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

	/*if (laserType == LT_NO_COLLIDE)
	{

	}*/



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
		ClearQuads();

		if (action != REDIRECTED)
		{
			for (int i = 0; i < maxBounces; ++i)
			{
				/*quads[i * 4 + 0].color = tailColor;
				quads[i * 4 + 1].color = headColor;
				quads[i * 4 + 2].color = headColor;
				quads[i * 4 + 3].color = tailColor;*/
				SetRectColor(quads + i * 4, headColor);
			}
		}
		else
		{
			for (int i = 0; i < maxBounces; ++i)
			{
				/*quads[i * 4 + 0].color = tailColor;
				quads[i * 4 + 1].color = headColor;
				quads[i * 4 + 2].color = headColor;
				quads[i * 4 + 3].color = tailColor;*/
				SetRectColor(quads + i * 4, Color::Yellow);
			}
		}

		double totalLength = 0;

		V2d currPos,tailPos,laserDir,laserCenter;
		double laserAngle,laserLength;

		currPos = GetPosition();
		tailPos = anchorPositions[currBounce];
		laserLength = length(currPos - tailPos);
		laserDir = normalize(currPos - tailPos);

		if (laserLength > lengthLimit)
		{
			tailPos = currPos - laserDir * lengthLimit;
			totalLength = lengthLimit;
			laserLength = lengthLimit;
		}
		else
		{
			totalLength += laserLength;
		}
		
		laserAngle = GetVectorAngleCW(laserDir);
		laserCenter = (currPos + tailPos) / 2.0;
		

		SetRectRotation(quads, laserAngle, laserLength, laserWidth, Vector2f(laserCenter));
	
		for (int i = currBounce; i > 0; --i)
		{
			if (totalLength >= lengthLimit)
			{
				break;
			}

			currPos = anchorPositions[i]; 
			tailPos = anchorPositions[i - 1];
			laserLength = length(currPos - tailPos);
			laserDir = normalize(currPos - tailPos);

			if (totalLength + laserLength > lengthLimit)
			{
				tailPos = currPos - laserDir * (lengthLimit - totalLength);
				laserLength = lengthLimit - totalLength;
				totalLength = lengthLimit;
			}
			else
			{
				totalLength += laserLength;
			}

			laserAngle = GetVectorAngleCW(laserDir);
			laserCenter = (currPos + tailPos) / 2.0;

			SetRectRotation(quads + (currBounce - ( i - 1 )) * 4, laserAngle, laserLength, laserWidth, Vector2f(laserCenter));
		}
		

		
		//ts->SetQuadSubRect(quad, 0);
		//SetRectCenter(quad, ts->tileWidth, ts->tileWidth, GetPositionF());
	}
}

void SkeletonLaser::UpdateHitboxes()
{
	//BasicUpdateHitboxes();
	//BasicUpdateHitboxInfo();

	if (anchorPositions.empty())
	{
		return;
	}
	
	hitBody.ResetFrames();
	hurtBody.ResetFrames();


	double totalLength = 0;

	V2d currPos, tailPos, laserDir, laserCenter;
	double laserAngle, laserLength;

	currPos = GetPosition();
	tailPos = anchorPositions[currBounce];
	laserLength = length(currPos - tailPos);
	laserDir = normalize(currPos - tailPos);

	if (laserLength > lengthLimit)
	{
		tailPos = currPos - laserDir * lengthLimit;
		totalLength = lengthLimit;
		laserLength = lengthLimit;
	}
	else
	{
		totalLength += laserLength;
	}

	laserAngle = GetVectorAngleCW(laserDir);
	laserCenter = (currPos + tailPos) / 2.0;

	bool laserOn = false;
	if (laserLength != 0)
	{
		hitBody.AddBasicRect(0, laserLength / 2, laserWidth / 2, laserAngle, laserCenter);
		hurtBody.AddBasicRect(0, laserLength / 2, laserWidth / 2, laserAngle, laserCenter);
		laserOn = true;
	}
	

	for (int i = currBounce; i > 0; --i)
	{
		if (totalLength >= lengthLimit)
		{
			break;
		}

		currPos = anchorPositions[i];
		tailPos = anchorPositions[i - 1];
		laserLength = length(currPos - tailPos);
		laserDir = normalize(currPos - tailPos);

		if (totalLength + laserLength > lengthLimit)
		{
			tailPos = currPos - laserDir * (lengthLimit - totalLength);
			laserLength = lengthLimit - totalLength;
			totalLength = lengthLimit;
		}
		else
		{
			totalLength += laserLength;
		}

		laserAngle = GetVectorAngleCW(laserDir);
		laserCenter = (currPos + tailPos) / 2.0;

		if (laserLength != 0)
		{
			hitBody.AddBasicRect(0, laserLength / 2, laserWidth / 2, laserAngle, laserCenter);
			hurtBody.AddBasicRect(0, laserLength / 2, laserWidth / 2, laserAngle, laserCenter);
			laserOn = true;
		}
		
		//SetRectRotation(quads + (currBounce - (i - 1)) * 4, laserAngle, laserLength, laserWidth, Vector2f(laserCenter));
	}

	if (laserOn)
	{
		hitBody.SetBasicPos(V2d(0, 0));
		hurtBody.SetBasicPos(V2d(0, 0));
	}
}

void SkeletonLaser::EnemyDraw(sf::RenderTarget *target)
{
	//laserBody.DebugDraw(0, target);
	target->draw(quads, maxBounces * 4, sf::Quads);
	//firePool.Draw(target);
}

//void SkeletonLaser::DebugDraw(sf::RenderTarget *target)
//{
//	hitBody.DebugDraw( 0, target);
//}

void SkeletonLaser::HandleHitAndSurvive()
{
}

bool SkeletonLaser::CanBeHitByPlayer()
{
	return false;
}

bool SkeletonLaser::CanBeHitByComboer()
{
	return false;
}