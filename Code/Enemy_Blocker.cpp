#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Blocker.h"
#include "CircleGroup.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )




BlockerChain::BlockerChain(ActorParams *ap )//Vector2i &pos, list<Vector2i> &pathParam, int p_bType, bool p_armored,
	//int spacing, int p_level )
	:Enemy(EnemyType::EN_BLOCKERCHAIN, ap )//, false, 1, false)
{
	BlockerParams *bParams = (BlockerParams*)ap;

	level = ap->GetLevel();

	

	switch (level)
	{
	case 1:
		scale = 1.0;
		break;
	case 2:
		scale = 2.0;
		break;
	case 3:
		scale = 3.0;
		break;
	}

	bType = (BlockerType)(bParams->bType);
	armored = bParams->armored;
	//receivedHit = NULL;
	//position = startPosInfo.GetPosition();

	SetCurrPosInfo(startPosInfo);

	frame = 0;

	animationFactor = 2;

	
	switch (bType)
	{
	case BLUE:
		ts = sess->GetTileset("Enemies/blocker_w1_192x192.png", 192, 192);
		break;
	case GREEN:
		ts = sess->GetTileset("Enemies/blocker_w2_192x192.png", 192, 192);
		break;
	}
	

	double rad = 32;
	double minDistance = 60 * scale;

	dead = false;
	double totalLength = 0;
	double dist = minDistance + (double)bParams->spacing;

	int pathSize = ap->localPath.size();//pathParam.size();

	V2d position = GetPosition();

	if (pathSize == 0)
	{
		numBlockers = 1;
	}
	else
	{
		V2d prev = position;
		V2d temp;
		int i = 0;
		for (auto it = ap->localPath.begin(); it != ap->localPath.end(); ++it)
		{
			temp = V2d((*it).x + position.x, (*it).y + position.y);
			totalLength += length( temp - prev);
			prev = temp;
		}

		
		numBlockers = totalLength / dist; //round down
	}
	


	//numBlockers = pathParam.size();
	blockers = new Blocker*[numBlockers];
	va = new Vertex[numBlockers * 4];
	
	
	
	if (numBlockers > 1)
	{
		if (pathSize > 0)
		{

		}

		vector<Vector2i> path = ap->MakeGlobalPath();


		//
		auto currPoint = path.begin();
		++currPoint;
		auto nextPoint = currPoint;
		--currPoint;

		V2d currWalk = position;
		V2d nextD((*nextPoint).x, (*nextPoint).y);
		double pathWalk = 0;
		double travel = 0;
		double tempLen = 0;
		bool end = false;
		//while (pathWalk < totalLength)
		int ind = 0;

		while (true)
		{
			if (ind == numBlockers)
				break;
			assert(ind < numBlockers);
			blockers[ind] = new Blocker( ap, this, Vector2i(round(currWalk.x), round(currWalk.y)), ind);
			cout << blockers[ind]->GetPosition().x << ", " << blockers[ind]->GetPosition().y << endl;
			travel = dist;

			tempLen = length(nextD - currWalk);

			while (tempLen < travel)
			{
				travel -= tempLen;
				currPoint = nextPoint;
				currWalk = nextD;
				nextPoint++;
				if (nextPoint == path.end())
				{
					end = true;
					break;
				}

				nextD = V2d((*nextPoint).x, (*nextPoint).y);

				tempLen = length(nextD - currWalk);
			}

			if (end)
				break;

			if (travel > 0)
			{
				currWalk += travel * normalize(nextD - currWalk);
			}

			++ind;
		}

		V2d prevPos;
		V2d nextPos;
		V2d currPos;
		V2d bisector;
		V2d dir0;
		V2d dir1;
		
		for (int i = 0; i < numBlockers; ++i)
		{
			currPos = blockers[i]->GetPosition();
			if (i > 0)
			{
				prevPos = blockers[i - 1]->GetPosition();
			}
			if (i < numBlockers - 1)
			{
				nextPos = blockers[i + 1]->GetPosition();
			}

			if (i > 0 && i < numBlockers - 1)
			{
				dir0 = normalize(currPos - prevPos);
				dir0 = V2d(dir0.y, -dir0.x);
				dir1 = normalize(nextPos - currPos);
				dir1 = V2d(dir1.y, -dir1.x);
				bisector = normalize(dir0 + dir1);
			}
			else if (i == 0)
			{
				dir1 = normalize(nextPos - currPos);
				dir1 = V2d(dir1.y, -dir1.x);
				bisector = dir1;
			}
			else if (i == numBlockers - 1)
			{
				dir0 = normalize(currPos - prevPos);
				dir0 = V2d(dir0.y, -dir0.x);
				bisector = dir0;
			}

			blockers[i]->hitboxInfo->kbDir = bisector;
		}
	}
	else
	{
		blockers[0] = new Blocker( ap, this, Vector2i(round(GetPosition().x), round(GetPosition().y)), 0);
	}
	int minX = blockers[0]->spawnRect.left;
	int maxX = blockers[0]->spawnRect.left + blockers[0]->spawnRect.width;
	int minY = blockers[0]->spawnRect.top;
	int maxY = blockers[0]->spawnRect.top + blockers[0]->spawnRect.height;

	for (int i = 1; i < numBlockers; ++i)
	{
		minX = min((int)blockers[i]->spawnRect.left, minX);
		maxX = max((int)blockers[i]->spawnRect.left + (int)blockers[i]->spawnRect.width, maxX);
		minY = min((int)blockers[i]->spawnRect.top, minY);
		maxY = max((int)blockers[i]->spawnRect.top + (int)blockers[i]->spawnRect.height, maxY);
	}

	spawnRect = sf::Rect<double>(minX - 16, minY - 16, (maxX - minX) + 32, (maxY - minY) + 32);

	for (int i = 0; i < numBlockers; ++i)
	{
		blockers[i]->UpdateSprite();
	}

	circleGroup = new CircleGroup(numBlockers, 40, Color::Red, 20);
	for (int i = 0; i < numBlockers; ++i)
	{
		circleGroup->SetPosition( i, Vector2f(blockers[i]->GetPosition()));
	}
	circleGroup->ShowAll();
}

BlockerChain::~BlockerChain()
{
	delete circleGroup;
	for (int i = 0; i < numBlockers; ++i)
	{
		delete blockers[i];
	}
	delete[] blockers;
	delete[] va;
}

void BlockerChain::ResetEnemy()
{
	for (int i = 0; i < numBlockers; ++i)
	{
		blockers[i]->Reset();
	}
	circleGroup->ShowAll();
}

void BlockerChain::SetZone(Zone *p_zone)
{
	zone = p_zone;
	for (int i = 0; i < numBlockers; ++i)
	{
		blockers[i]->SetZone(p_zone);
	}
}

void BlockerChain::UpdatePhysics( int substep )
{
	for (int i = 0; i < numBlockers; ++i)
	{
		blockers[i]->UpdatePhysics( substep );
	}
}

void BlockerChain::UpdatePrePhysics()
{
	for (int i = 0; i < numBlockers; ++i)
	{
		blockers[i]->UpdatePrePhysics();
	}
}

void BlockerChain::ProcessState()
{
	checkCol = false;
}

void BlockerChain::UpdateEnemyPhysics()
{
	if (checkCol)
	{
		for (int i = 0; i < numBlockers; ++i)
		{
			blockers[i]->UpdateEnemyPhysics();
		}
	}
}


void BlockerChain::UpdatePostPhysics()
{
	int activeNum = 0;
	for (int i = 0; i < numBlockers; ++i)
	{
		if (!blockers[i]->dead)
		{
			blockers[i]->UpdatePostPhysics();
			activeNum++;
		}
	}

	if (activeNum == 0)
	{
		sess->RemoveEnemy(this);
	}
}

void BlockerChain::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(va, numBlockers * 4, sf::Quads, ts->texture);
}

void BlockerChain::DrawMinimap(sf::RenderTarget *target)
{
	circleGroup->Draw(target);
}

void BlockerChain::DebugDraw(RenderTarget *target)
{
	for (int i = 0; i < numBlockers; ++i)
	{
		blockers[i]->DebugDraw(target);
	}
}

int BlockerChain::GetNumCamPoints()
{
	return numBlockers;
}


V2d BlockerChain::GetCamPoint(int index)
{
	return blockers[index]->GetPosition();
}

Blocker::Blocker( ActorParams *ap, BlockerChain *p_bc, Vector2i &pos, int index)
	:Enemy( EnemyType::EN_BLOCKER, ap ),//false, 1, false), 
	bc(p_bc), vaIndex(index)
{
	level = bc->level;

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

	minimapCirclePoints = 20;
	
	receivedHit = NULL;


	currPosInfo.position.x = pos.x;
	currPosInfo.position.y = pos.y;

	frame = 0;

	
	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 10;

	BasicCircleHurtBodySetup(32, GetPosition());
	BasicCircleHitBodySetup(32, GetPosition());

	hitBody.hitboxInfo = hitboxInfo;


	

	//SetHurtboxes(hurtBody, 0);

	//spawnRect = sf::Rect<double>(GetPosition().x - 32, position.y - 32,
	//	64, 64);

	sess->staticItemTree->Insert(this);

	actionLength[WAIT] = 15;
	actionLength[MALFUNCTION] = 13;
	actionLength[HITTING] = 2;
	actionLength[EXPLODE] = 10;

	animFactor[WAIT] = 3;
	animFactor[HITTING] = 20;
	animFactor[MALFUNCTION] = 4;
	animFactor[EXPLODE] = 2;

	ResetEnemy();
}

void Blocker::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case WAIT:
			frame = 0;
			break;
		case HITTING:
			frame = 0;
			action = WAIT;
			break;
		case MALFUNCTION:
			action = EXPLODE;
			frame = 0;
			SetHitboxes(NULL, 0);
			break;
		case EXPLODE:
			numHealth = 0;
			dead = true;
			ClearSprite();
			bc->circleGroup->SetVisible(vaIndex, false);
			break;
		}
	}

	switch (action)
	{
	case WAIT:

		break;
	case MALFUNCTION:
		//cout << "mal: " << frame << endl;
		break;
	case EXPLODE:
		break;
	}

	switch (action)
	{
	case WAIT:
		//SetHitboxes(hitBody, 0);
		//SetHurtboxes(hurtBody, 0);
		break;
	case MALFUNCTION:
		//SetHitboxes(hitBody, 0);
		//SetHurtboxes(NULL, 0);
		break;
	case EXPLODE:
		//SetHitboxes(NULL, 0);
		//SetHurtboxes(NULL, 0);
		//SetHitboxes(NULL, 0);
		break;
	}
}

void Blocker::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		sess->PlayerConfirmEnemyNoKill(this);
		ConfirmHitNoKill();

		if (receivedHit->hType == HitboxInfo::COMBO)
		{
			comboHitEnemy->ComboKill(this);
		}

		if (IsFastDying())
		{
			action = EXPLODE;
			frame = 0;
			SetHitboxes(NULL, 0);
			SetHurtboxes(NULL, 0);
		}
		else
		{
			action = MALFUNCTION;
			frame = 0;
			SetHitboxes(&hitBody, 0);
			SetHurtboxes(NULL, 0);
		}
	}
}

void Blocker::HandleQuery(QuadTreeCollider * qtc)
{
	if (!dead)
	{
		if (qtc != NULL)
			qtc->HandleEntrant(this);
		else
		{
			checkCol = true;
			if (!bc->checkCol)
				bc->checkCol = true;
		}
	}
}

bool Blocker::IsTouchingBox(const sf::Rect<double> &r)
{
	return r.intersects(spawnRect);
}

void Blocker::ClearSprite()
{
	Vertex *va = bc->va;
	va[vaIndex * 4 + 0].position = Vector2f(0, 0);
	va[vaIndex * 4 + 1].position = Vector2f(0, 0);
	va[vaIndex * 4 + 2].position = Vector2f(0, 0);
	va[vaIndex * 4 + 3].position = Vector2f(0, 0);
}

void Blocker::ResetEnemy()
{
	Vector2f p = GetPositionF();

	Vector2f spriteSize(bc->ts->tileWidth * scale, bc->ts->tileHeight * scale);
	SetRectCenter(bc->va + vaIndex * 4, spriteSize.x, spriteSize.y, p);

	action = WAIT;
	dead = false;

	//random start frame so they are all desynced
	int r = rand() % (animFactor[WAIT] * actionLength[WAIT]);
	frame = r;

	SetHitboxes(&hitBody, 0);
	SetHurtboxes(&hurtBody, 0);

	//UpdateHitboxes();

	UpdateSprite();
}

void Blocker::IHitPlayer(int index)
{
	V2d playerPos = sess->GetPlayerPos(index);
	if (dot(normalize(playerPos - GetPosition()), hitboxInfo->kbDir) < 0)
	{
		hitboxInfo->kbDir = -hitboxInfo->kbDir;
	}
	if (action == WAIT)
	{
		action = HITTING;
		frame = 0;
	}
}

bool Blocker::IsFastDying()
{
	switch (bc->bType)
	{
	case BlockerChain::BLUE:
		if (receivedHit->hType == HitboxInfo::BLUE)
			return true;
		break;
	case BlockerChain::GREEN:
		if (receivedHit->hType == HitboxInfo::GREEN)
			return true;
		break;
	case BlockerChain::YELLOW:
		if (receivedHit->hType == HitboxInfo::YELLOW)
			return true;
		break;
	case BlockerChain::ORANGE:
		if (receivedHit->hType == HitboxInfo::ORANGE)
			return true;
		break;
	case BlockerChain::RED:
		if (receivedHit->hType == HitboxInfo::RED)
			return true;
		break;
	case BlockerChain::MAGENTA:
		if (receivedHit->hType == HitboxInfo::MAGENTA)
			return true;
		break;
	}

	return false;
}

void Blocker::UpdateSprite()
{
	if (dead)
	{
		ClearSprite();
	}
	else
	{
		int f = 0;
		switch (action)
		{
		case WAIT:
			f = frame / animFactor[WAIT];
			break;
		case MALFUNCTION:
			f = frame / animFactor[MALFUNCTION] + 17;
			break;
		case HITTING:
			f = frame / animFactor[HITTING] + 15;
			break;
		case EXPLODE:
			f = frame / animFactor[EXPLODE] + 31;
			break;
		}
		IntRect subRect = bc->ts->GetSubRect(f);
		SetRectSubRect(bc->va + vaIndex * 4, subRect);
	}
}

void Blocker::DrawMinimap(sf::RenderTarget *target)
{
}