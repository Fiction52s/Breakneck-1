#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )


BlockerChain::BlockerChain(GameSession *owner, Vector2i &pos, list<Vector2i> &pathParam, int p_bType, bool p_armored,
	int spacing )
	:Enemy(owner, EnemyType::BLOCKERCHAIN, false, 1)
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

	//eventually use a quadtree for all static collision types
	
	//facingRight = true;
}

void BlockerChain::HandleEntrant(QuadTreeEntrant *qte)
{
	/*SpecterArea *sa = (SpecterArea*)qte;
	if (sa->barrier.Intersects(hurtBody))
	{
		specterProtected = true;
	}*/
}

void BlockerChain::ResetEnemy()
{
	//cout << "resetting enemy" << endl;
	//spawned = false;
	/*targetNode = 1;
	forward = true;
	dead = false;
	deathFrame = 0;*/
	//frame = 0;
	//position.x = path[0].x;
	//position.y = path[0].y;
	//receivedHit = NULL;


	//UpdateHitboxes();

	//UpdateSprite();
	//health = initHealth;
	for (int i = 0; i < numBlockers; ++i)
	{
		blockers[i]->ResetEnemy();
	}
}

void BlockerChain::UpdatePrePhysics()
{
	checkCol = false;
	for (int i = 0; i < numBlockers; ++i)
	{
		blockers[i]->UpdatePrePhysics();
	}
}

void BlockerChain::UpdatePhysics()
{
	if (checkCol)
	{
		for (int i = 0; i < numBlockers; ++i)
		{
			blockers[i]->UpdatePhysics();
		}
	}
}

void BlockerChain::PhysicsResponse()
{
	
}


void BlockerChain::UpdatePostPhysics()
{
	int activeNum = 0;
	for (int i = 0; i < numBlockers; ++i)
	{
		if (!blockers[i]->dead)
		{
			activeNum++;
			blockers[i]->UpdatePostPhysics();
		}
	}
/*
	if (slowCounter == slowMultiple)
	{
		++frame;

		slowCounter = 1;

		if (dying)
		{
			deathFrame++;
		}

	}
	else
	{
		slowCounter++;
	}*/

	if (activeNum == 0)
	{
		owner->RemoveEnemy(this);
	}
}

void BlockerChain::UpdateSprite()
{
	//sprite.setPosition(position.x, position.y);

	//if (dead)
	//{
	//	botDeathSprite.setTexture(*ts->texture);
	//	botDeathSprite.setTextureRect(ts->GetSubRect(13));
	//	botDeathSprite.setOrigin(botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2);
	//	botDeathSprite.setPosition(position.x + deathVector.x * deathPartingSpeed * deathFrame,
	//		position.y + deathVector.y * deathPartingSpeed * deathFrame);

	//	topDeathSprite.setTexture(*ts->texture);
	//	topDeathSprite.setTextureRect(ts->GetSubRect(12));
	//	topDeathSprite.setOrigin(topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2);
	//	topDeathSprite.setPosition(position.x + -deathVector.x * deathPartingSpeed * deathFrame,
	//		position.y + -deathVector.y * deathPartingSpeed * deathFrame);
	//}
	//else
	//{
	//	sprite.setTextureRect(ts->GetSubRect(frame / animationFactor));
	//	if (hasMonitor && !suppressMonitor)
	//	{
	//		//keySprite.setTexture( *ts_key->texture );
	//		keySprite->setTextureRect(ts_key->GetSubRect(owner->keyFrame / 5));
	//		keySprite->setOrigin(keySprite->getLocalBounds().width / 2,
	//			keySprite->getLocalBounds().height / 2);
	//		keySprite->setPosition(position.x, position.y);

	//	}


	//}
}

void BlockerChain::Draw(sf::RenderTarget *target)
{
	target->draw(va, numBlockers * 4, sf::Quads, ts->texture);
}

void BlockerChain::DrawMinimap(sf::RenderTarget *target)
{
	CircleShape enemyCircle;
	enemyCircle.setFillColor( COLOR_BLUE );
	enemyCircle.setRadius( 50 );
	enemyCircle.setOrigin( enemyCircle.getLocalBounds().width / 2, enemyCircle.getLocalBounds().height / 2 );
	enemyCircle.setPosition( position.x, position.y );
	target->draw( enemyCircle );

	/*if( hasMonitor && !suppressMonitor )
	{
	monitor->miniSprite.setPosition( position.x, position.y );
	target->draw( monitor->miniSprite );
	}*/

	/*if (!dead)
	{
		if (hasMonitor && !suppressMonitor)
		{
			CircleShape cs;
			cs.setRadius(50);
			cs.setFillColor(Color::White);
			cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
			cs.setPosition(position.x, position.y);
			target->draw(cs);
		}
		else
		{
			CircleShape cs;
			cs.setRadius(40);
			cs.setFillColor(Color::Red);
			cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
			cs.setPosition(position.x, position.y);
			target->draw(cs);
		}
	}*/
}

bool BlockerChain::IHitPlayer(int index)
{
	return false;
}

void BlockerChain::UpdateHitboxes()
{
	/*hurtBody.globalPosition = position;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position;
	hitBody.globalAngle = 0;

	if (owner->GetPlayer(0)->ground != NULL)
	{
		hitboxInfo->kbDir = normalize(-owner->GetPlayer(0)->groundSpeed * (owner->GetPlayer(0)->ground->v1 - owner->GetPlayer(0)->ground->v0));
	}
	else
	{
		hitboxInfo->kbDir = normalize(-owner->GetPlayer(0)->velocity);
	}*/
}

//return pair<bool,bool>( hitme, was it with a clone)
pair<bool, bool> BlockerChain::PlayerHitMe(int index)
{
	//Actor *player = owner->GetPlayer(0);
	//if (player->currHitboxes != NULL)
	//{
	//	bool hit = false;

	//	for (list<CollisionBox>::iterator it = player->currHitboxes->begin(); it != player->currHitboxes->end(); ++it)
	//	{
	//		if (hurtBody.Intersects((*it)))
	//		{
	//			hit = true;
	//			break;
	//		}
	//	}


	//	if (hit)
	//	{
	//		receivedHit = player->currHitboxInfo;
	//		return pair<bool, bool>(true, false);
	//	}

	//}

	//for (int i = 0; i < player->recordedGhosts; ++i)
	//{
	//	if (player->ghostFrame < player->ghosts[i]->totalRecorded)
	//	{
	//		if (player->ghosts[i]->currHitboxes != NULL)
	//		{
	//			bool hit = false;

	//			for (list<CollisionBox>::iterator it = player->ghosts[i]->currHitboxes->begin(); it != player->ghosts[i]->currHitboxes->end(); ++it)
	//			{
	//				if (hurtBody.Intersects((*it)))
	//				{
	//					hit = true;
	//					break;
	//				}
	//			}


	//			if (hit)
	//			{
	//				receivedHit = player->currHitboxInfo;
	//				return pair<bool, bool>(true, true);
	//			}
	//		}
	//		//player->ghosts[i]->curhi
	//	}
	//}

	return pair<bool, bool>(false, false);
}

bool BlockerChain::PlayerSlowingMe()
{
	/*Actor *player = owner->GetPlayer(0);
	for (int i = 0; i < player->maxBubbles; ++i)
	{
		if (player->bubbleFramesToLive[i] > 0)
		{
			if (length(position - player->bubblePos[i]) <= player->bubbleRadius)
			{
				return true;
			}
		}
	}*/
	return false;
}

void BlockerChain::DebugDraw(RenderTarget *target)
{
	for (int i = 0; i < numBlockers; ++i)
	{
		blockers[i]->DebugDraw(target);
	}
}

void BlockerChain::SaveEnemyState()
{
	/*stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.forward = forward;
	stored.frame = frame;
	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.position = position;
	stored.targetNode = targetNode;*/
}

void BlockerChain::LoadEnemyState()
{
	//dead = stored.dead;
	//deathFrame = stored.deathFrame;
	//forward = stored.forward;
	//frame = stored.frame;
	//hitlagFrames = stored.hitlagFrames;
	//hitstunFrames = stored.hitstunFrames;
	//position = stored.position;
	//targetNode = stored.targetNode;
}


Blocker::Blocker(BlockerChain *p_bc, Vector2i &pos, int index)
	:Enemy(p_bc->owner, EnemyType::BLOCKER, false, 1), bc(p_bc), vaIndex(index)
{

	initHealth = 40;
	health = initHealth;

	//hopefully this doesnt cause deletion bugs
	
	//radius = 400;

	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	
	//spawnRect = sf::Rect<double>(pos.x - 16, pos.y - 16, 16 * 2, 16 * 2);

	frame = 0;


	//animationFactor = 10;

	//ts = owner->GetTileset( "Specter.png", 80, 80 );
	/*ts = owner->GetTileset("specter_256x256.png", 256, 256);
	sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect(frame));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(pos.x, pos.y);*/
	//position.x = 0;
	//position.y = 0;
	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 32;
	hurtBody.rh = 32;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = 16;
	hitBody.rh = 16;

	/*hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;*/
	//hitboxInfo->kbDir;



	dead = false;

	//ts_bottom = owner->GetTileset( "patroldeathbot.png", 32, 32 );
	//ts_top = owner->GetTileset( "patroldeathtop.png", 32, 32 );
	//ts_death = owner->GetTileset( "patroldeath.png", 80, 80 );

	//deathPartingSpeed = .4;
	//deathVector = V2d(1, -1);

	//facingRight = true;

	//ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	//bloodSprite.setTexture( *ts_testBlood->texture );

	UpdateHitboxes();

	spawnRect = sf::Rect<double>(position.x - 32, position.y - 32,
		64, 64);

	owner->staticItemTree->Insert(this);
	//cout << "finish init" << endl;
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

void Blocker::HandleEntrant(QuadTreeEntrant *qte)
{
	//Specter 
}


void Blocker::ResetEnemy()
{
	//	fireCounter = 0;
	//	testSeq.Reset();
	//	launcher->Reset();
	//cout << "resetting enemy" << endl;
	//spawned = false;
	//targetNode = 1;
	//forward = true;
	action = WAIT;
	dead = false;
	//	dying = false;
	frame = 0;
	//	position.x = path[0].x;
	//	position.y = path[0].y;
	//receivedHit = NULL;

	//ClearSprite();

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;
}

void Blocker::UpdatePrePhysics()
{
	if (dead) return;

	checkCol = false;

	if (!dead && receivedHit != NULL)
	{
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if (health <= 0)
		{
			action = MALFUNCTION;
			frame = 0;
		}

		receivedHit = NULL;
	}
}

void Blocker::UpdatePhysics()
{
	if (dead) return;
	/*if (PlayerSlowingMe())
	{
		if (slowMultiple == 1)
		{
			slowCounter = 1;
			slowMultiple = 5;
		}
	}
	else
	{
		slowMultiple = 1;
		slowCounter = 1;
	}*/

	PhysicsResponse();
}

void Blocker::PhysicsResponse()
{
	if (!checkCol)
		return;

	if (!dead && receivedHit == NULL)
	{
		UpdateHitboxes();

		pair<bool, bool> result = PlayerHitMe(0);
		if (result.first)
		{
			//cout << "color blue" << endl;
			//triggers multiple times per frame? bad?
			bc->owner->GetPlayer(0)->ConfirmHit(6, 5, .8, 6);


			if (bc->owner->GetPlayer(0)->ground == NULL && owner->GetPlayer(0)->velocity.y > 0)
			{
				bc->owner->GetPlayer(0)->velocity.y = 4;//.5;
			}

			//	cout << "frame: " << owner->GetPlayer( 0 )->frame << endl;

			//owner->GetPlayer( 0 )->frame--;
			//owner->ActivateEffect( EffectLayer::IN_FRONT, ts_testBlood, position, true, 0, 6, 3, true );

			//	cout << "Specter received damage of: " << receivedHit->damage << endl;
			/*if( !result.second )
			{
			owner->Pause( 8 );
			}

			health -= 20;

			if( health <= 0 )
			dead = true;

			receivedHit = NULL;*/
			//dead = true;
			//receivedHit = NULL;
		}


		if (IHitPlayer(0))
		{
			//	cout << "Specter just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}

void Blocker::UpdatePostPhysics()
{
	if (dead) return;

	if (receivedHit != NULL)
	{
		owner->Pause(5);
	}

	if (action == MALFUNCTION && frame == 60)
	{
		dead = true;
		ClearSprite();
		return;
	}
	

	UpdateSprite();



	++frame;

	//if (slowCounter == slowMultiple)
	//{
	//	//cout << "fireCounter: " << fireCounter << endl;
	//	++frame;
	//	slowCounter = 1;


	//	if (dead)
	//	{
	//		//cout << "deathFrame: " << deathFrame << endl;
	//		deathFrame++;
	//	}

	//}
	//else
	//{
	//	slowCounter++;
	//}
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

		/*va[vaIndex * 4 + 0].color = Color::Red;
		va[vaIndex * 4 + 1].color = Color::Red;
		va[vaIndex * 4 + 2].color = Color::Red;
		va[vaIndex * 4 + 3].color = Color::Red;*/
	}

	/*if (!dead)
	{
		sprite.setTextureRect(ts->GetSubRect(frame / animationFactor));
		sprite.setPosition(position.x, position.y);
	}
	else
	{

		botDeathSprite.setTexture(*ts->texture);
		botDeathSprite.setTextureRect(ts->GetSubRect(13));
		botDeathSprite.setOrigin(botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2);
		botDeathSprite.setPosition(position.x + deathVector.x * deathPartingSpeed * deathFrame,
			position.y + deathVector.y * deathPartingSpeed * deathFrame);

		topDeathSprite.setTexture(*ts->texture);
		topDeathSprite.setTextureRect(ts->GetSubRect(12));
		topDeathSprite.setOrigin(topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2);
		topDeathSprite.setPosition(position.x + -deathVector.x * deathPartingSpeed * deathFrame,
			position.y + -deathVector.y * deathPartingSpeed * deathFrame);
	}*/
}

void Blocker::Draw(sf::RenderTarget *target)
{
	//cout << "draw" << endl;
	//if (!dead)
	//{
	//	if (hasMonitor && !suppressMonitor)
	//	{
	//		//owner->AddEnemy( monitor );
	//		CircleShape cs;
	//		cs.setRadius(40);

	//		cs.setFillColor(Color::Black);

	//		//cs.setFillColor( monitor-> );
	//		cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
	//		cs.setPosition(position.x, position.y);
	//		target->draw(cs);
	//	}
	//	CircleShape cs;
	//	cs.setFillColor(Color(255, 0, 0, 100));
	//	cs.setRadius(400);
	//	cs.setOrigin(cs.getLocalBounds().width / 2,
	//		cs.getLocalBounds().height / 2);
	//	cs.setPosition(position.x, position.y);
	//	target->draw(cs);
	//	target->draw(sprite);

	//}
	//else
	//{
	//	target->draw(botDeathSprite);

	//	if (deathFrame / 3 < 6)
	//	{

	//		/*bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
	//		bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
	//		bloodSprite.setPosition( position.x, position.y );
	//		bloodSprite.setScale( 2, 2 );
	//		target->draw( bloodSprite );*/
	//	}

	//	target->draw(topDeathSprite);
	//}



}

void Blocker::DrawMinimap(sf::RenderTarget *target)
{
	//if (!dead)
	//{
	//	CircleShape enemyCircle;
	//	enemyCircle.setFillColor(COLOR_BLUE);
	//	enemyCircle.setRadius(50);
	//	enemyCircle.setOrigin(enemyCircle.getLocalBounds().width / 2, enemyCircle.getLocalBounds().height / 2);
	//	enemyCircle.setPosition(position.x, position.y);
	//	target->draw(enemyCircle);

	//	/*if( hasMonitor && !suppressMonitor )
	//	{
	//	monitor->miniSprite.setPosition( position.x, position.y );
	//	target->draw( monitor->miniSprite );
	//	}*/
	//}
}

bool Blocker::IHitPlayer(int index)
{
	Actor *player = owner->GetPlayer(index);

	if (hitBody.Intersects(player->hurtBody))
	{
		player->ApplyHit(bc->hitboxInfo);
		return true;
	}
	return false;
}

void Blocker::UpdateHitboxes()
{
	hurtBody.globalPosition = position;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position;
	hitBody.globalAngle = 0;

	/*if (owner->GetPlayer(0)->ground != NULL)
	{
		hitboxInfo->kbDir = normalize(-owner->GetPlayer(0)->groundSpeed * (owner->GetPlayer(0)->ground->v1 - owner->GetPlayer(0)->ground->v0));
	}
	else
	{
		hitboxInfo->kbDir = normalize(-owner->GetPlayer(0)->velocity);
	}*/
}

//return pair<bool,bool>( hitme, was it with a clone)
pair<bool, bool> Blocker::PlayerHitMe(int index)
{
	if (action != WAIT)
		return pair<bool, bool>(false, false);

	Actor *player = owner->GetPlayer(0);
	if (player->currHitboxes != NULL)
	{
		bool hit = false;

		for (list<CollisionBox>::iterator it = player->currHitboxes->begin(); it != player->currHitboxes->end(); ++it)
		{
			if (hurtBody.Intersects((*it)))
			{
				hit = true;
				break;
			}
		}



		if (hit)
		{
			receivedHit = player->currHitboxInfo;
			return pair<bool, bool>(true, false);
		}

	}

	for (int i = 0; i < player->recordedGhosts; ++i)
	{
		if (player->ghostFrame < player->ghosts[i]->totalRecorded)
		{
			if (player->ghosts[i]->currHitboxes != NULL)
			{
				bool hit = false;

				for (list<CollisionBox>::iterator it = player->ghosts[i]->currHitboxes->begin(); it != player->ghosts[i]->currHitboxes->end(); ++it)
				{
					if (hurtBody.Intersects((*it)))
					{
						hit = true;
						break;
					}
				}


				if (hit)
				{
					receivedHit = player->currHitboxInfo;
					return pair<bool, bool>(true, true);
				}
			}
			//player->ghosts[i]->curhi
		}
	}

	return pair<bool, bool>(false, false);
}

bool Blocker::PlayerSlowingMe()
{
	/*Actor *player = owner->GetPlayer(0);
	for (int i = 0; i < player->maxBubbles; ++i)
	{
		if (player->bubbleFramesToLive[i] > 0)
		{
			if (length(position - player->bubblePos[i]) <= player->bubbleRadius)
			{
				return true;
			}
		}
	}*/
	return false;
}

void Blocker::DebugDraw(RenderTarget *target)
{
	if (!dead)
	{
		hurtBody.DebugDraw(target);
		hitBody.DebugDraw(target);
	}
}

void Blocker::SaveEnemyState()
{
	/*stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.frame = frame;
	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.position = position;*/
}

void Blocker::LoadEnemyState()
{
	//dead = stored.dead;
	//deathFrame = stored.deathFrame;
	//frame = stored.frame;
	//hitlagFrames = stored.hitlagFrames;
	//hitstunFrames = stored.hitstunFrames;
	//position = stored.position;
}