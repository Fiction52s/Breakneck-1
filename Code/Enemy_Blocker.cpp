#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Blocker.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )


BlockerChain::BlockerChain(GameSession *owner, Vector2i &pos, list<Vector2i> &pathParam, int p_bType, bool p_armored,
	int spacing )
	:Enemy(owner, EnemyType::EN_BLOCKERCHAIN, false, 1, false)
{
	bType = (BlockerType)p_bType;
	armored = p_armored;
	//receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	initHealth = 40;
	health = initHealth;

	frame = 0;

	animationFactor = 2;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	ts = owner->GetTileset("blocker.png", 64, 64);

	double rad = 32;
	double minDistance = 60;

	dead = false;
	double totalLength = 0;
	double dist = minDistance + (double)spacing;

	int pathSize = pathParam.size();

	if (pathSize == 0)
	{
		numBlockers = 1;
	}
	else
	{
		V2d prev = position;;
		V2d temp;
		int i = 0;
		for (auto it = pathParam.begin(); it != pathParam.end(); ++it)
		{
			temp = V2d((*it).x + pos.x, (*it).y + pos.y);
			totalLength += length( temp - prev);
			prev = temp;
		}

		
		numBlockers = totalLength / dist; //round down
	}
	


	//numBlockers = pathParam.size();
	blockers = new Blocker*[numBlockers];
	va = new Vertex[numBlockers * 4];
	
	
	
	if (pathSize > 0)
	{
		
	}

	pathParam.push_front(Vector2i( 0, 0 ) );

	for (auto it = pathParam.begin(); it != pathParam.end(); ++it)
	{
		(*it) += pos;
	}
	//
	auto currPoint = pathParam.begin();
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

	while( true )
	{
		if (ind == numBlockers)
			break;
		assert(ind < numBlockers);
		blockers[ind] = new Blocker(this, Vector2i(round(currWalk.x), round(currWalk.y)), ind);
		cout << blockers[ind]->position.x << ", " << blockers[ind]->position.y << endl;
		travel = dist;

		tempLen = length(nextD - currWalk);
		
		while (tempLen < travel)
		{
			travel -= tempLen;
			currPoint = nextPoint;
			currWalk = nextD;
			nextPoint++;
			if (nextPoint == pathParam.end())
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
}

void BlockerChain::ResetEnemy()
{
	for (int i = 0; i < numBlockers; ++i)
	{
		blockers[i]->ResetEnemy();
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
		owner->RemoveEnemy(this);
	}
}

void BlockerChain::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(va, numBlockers * 4, sf::Quads, ts->texture);
}

void BlockerChain::DrawMinimap(sf::RenderTarget *target)
{

}

void BlockerChain::DebugDraw(RenderTarget *target)
{
	for (int i = 0; i < numBlockers; ++i)
	{
		blockers[i]->DebugDraw(target);
	}
}

Blocker::Blocker(BlockerChain *p_bc, Vector2i &pos, int index)
	:Enemy(p_bc->owner, EnemyType::EN_BLOCKER, false, 1, false), bc(p_bc), vaIndex(index)
{
	initHealth = 40;
	health = initHealth;

	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	frame = 0;

	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 32;
	hurtBox.rh = 32;
	hurtBox.globalPosition = position;
	hurtBody = new CollisionBody(1);
	hurtBody->AddCollisionBox(0, hurtBox);

	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.globalPosition = position;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 32;
	hitBox.rh = 32;
	hitBody = new CollisionBody(1);
	hitBody->AddCollisionBox(0, hitBox);

	hitBody->hitboxInfo = p_bc->hitboxInfo;
	
	//SetHurtboxes(hurtBody, 0);

	spawnRect = sf::Rect<double>(position.x - 32, position.y - 32,
		64, 64);

	owner->staticItemTree->Insert(this);

	ResetEnemy();

	actionLength[WAIT] = 1;
	actionLength[MALFUNCTION] = 60;
	actionLength[FASTDEATH] = 10;
	
	animFactor[WAIT] = 1;
	animFactor[MALFUNCTION] = 1;
	animFactor[FASTDEATH] = 1;
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
		case MALFUNCTION:
		case FASTDEATH:
			numHealth = 0;
			dead = true;
			ClearSprite();
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
	case FASTDEATH:
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
	case FASTDEATH:
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
		owner->GetPlayer(0)->ConfirmEnemyNoKill(this);
		ConfirmHitNoKill();
		if (IsFastDying())
		{
			action = FASTDEATH;
			frame = 0;
			SetHitboxes(NULL, 0);
			SetHurtboxes(NULL, 0);
		}
		else
		{
			action = MALFUNCTION;
			frame = 0;
			SetHitboxes(hitBody, 0);
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
	action = WAIT;
	dead = false;
	frame = 0;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

	//UpdateHitboxes();

	UpdateSprite();
	health = initHealth;
}

bool Blocker::IsFastDying()
{
	switch (bc->bType)
	{
	case BlockerChain::NORMAL:
		break;
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
		Vertex *va = bc->va;
		IntRect subRect = bc->ts->GetSubRect(0);
		if (owner->GetPlayer(0)->position.x < position.x)
		{
			subRect.left += subRect.width;
			subRect.width = -subRect.width;
		}
		va[vaIndex * 4 + 0].texCoords = Vector2f(subRect.left,
			subRect.top);
		va[vaIndex * 4 + 1].texCoords = Vector2f(subRect.left
			+ subRect.width,
			subRect.top);
		va[vaIndex * 4 + 2].texCoords = Vector2f(subRect.left
			+ subRect.width,
			subRect.top + subRect.height);
		va[vaIndex * 4 + 3].texCoords = Vector2f(subRect.left,
			subRect.top + subRect.height);

		Vector2f p(position.x, position.y);

		Vector2f spriteSize(bc->ts->tileWidth, bc->ts->tileHeight);//parent->spriteSize;
		va[vaIndex * 4 + 0].position = p + Vector2f(-spriteSize.x, -spriteSize.y);
		va[vaIndex * 4 + 1].position = p + Vector2f(spriteSize.x, -spriteSize.y);
		va[vaIndex * 4 + 2].position = p + Vector2f(spriteSize.x, spriteSize.y);
		va[vaIndex * 4 + 3].position = p + Vector2f(-spriteSize.x, spriteSize.y);		
	}
}

void Blocker::DrawMinimap(sf::RenderTarget *target)
{
}