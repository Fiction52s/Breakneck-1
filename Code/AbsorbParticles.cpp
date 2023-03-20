#include "Actor.h"
#include "Session.h"
#include "AbsorbParticles.h"
#include "PowerOrbs.h"
#include "KeyMarker.h"

using namespace sf;
using namespace std;

AbsorbParticles::AbsorbParticles(Session *p_sess, AbsorbType p_abType )
	:va(NULL), maxSpeed(100), abType( p_abType ), sess( p_sess )
{
	activeList = NULL;
	inactiveList = NULL;
	data.directKilled = false;

	maxNumParticles = sess->GetNumTotalEnergyParticles(p_abType);

	allParticles.reserve(maxNumParticles);
	//switch (p_abType)
	//{
	//case ENERGY:
	//	maxNumParticles = sess->GetNumTotalEnergyParticles( p_abType); //256;
	//	break;
	//case DARK:
	//	maxNumParticles = sess->(p_abType);//64;
	//	break;
	//case SHARD:
	//	maxNumParticles = 64;
	//	break;
	//}

	va = new Vertex[maxNumParticles * 4];

	for (int i = 0; i < maxNumParticles; ++i)
	{
		AllocateParticle( i );
	}

	//ts_explodeCreate = NULL;
	ts_explodeDestroy = NULL;

	switch (p_abType)
	{
	case DARK:
		ts = sess->GetTileset("FX/key_128x128.png", 128, 128);
		ts_explodeDestroy = sess->GetSizedTileset("FX/keyexplode_128x128.png");
		animFactor = 2;
		break;
	case SHARD:
		ts = sess->GetTileset("HUD/shard_get_128x128.png", 128, 128);
		animFactor = 2;
		//ts_explodeCreate = GetTileset("FX/shard_explode_01_256x256.png", 256, 256);
		ts_explodeDestroy = sess->GetTileset("FX/shard_explode_02_256x256.png", 256, 256);
		break;
	default:
		ts = sess->GetTileset("FX/absorb_64x64.png", 64, 64);
		animFactor = 3;
		break;
	}
}

sf::Vector2f AbsorbParticles::SingleEnergyParticle::GetTargetPos(AbsorbType abType)
{
	switch (abType)
	{
	case ENERGY:
	{
		V2d playerPos = playerTarget->position;
		return Vector2f(playerPos);
		break;
	}
	case DARK:
	{
		V2d playerPos = playerTarget->position;
		return Vector2f(playerPos);
		//return Vector2f(1920 - 100, 100);
		break;
	}
	case SHARD:
	{
		V2d playerPos = playerTarget->position;
		return Vector2f(playerPos);//Vector2f(286, 202);
		break;
	}
	}
}

int AbsorbParticles::GetParticleID(AbsorbParticles::SingleEnergyParticle *sep)
{
	if (sep == NULL)
	{
		return -1;
	}
	else
	{
		return sep->particleIndex;
	}
}

AbsorbParticles::SingleEnergyParticle *AbsorbParticles::GetParticleFromID(int id)
{
	if (id < 0)
	{
		return NULL;
	}
	else
	{
		return allParticles[id];
	}
}

void AbsorbParticles::AllocateParticle( int tileIndex )
{
	SingleEnergyParticle *sp = new SingleEnergyParticle(this, tileIndex );
	if (inactiveList == NULL)
		inactiveList = sp;
	else
	{
		sp->next = inactiveList;
		inactiveList->prev = sp;
		inactiveList = sp;
	}

	allParticles.push_back(sp);
}

int AbsorbParticles::GetNumStoredBytes()
{
	return sizeof(MyData) + sizeof(SingleEnergyParticle::MyData) * maxNumParticles;
}

void AbsorbParticles::StoreBytes(unsigned char *bytes)
{
	data.activeListParticleID = GetParticleID(activeList);
	data.inactiveListParticleID = GetParticleID(inactiveList);

	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
	for (auto it = allParticles.begin(); it != allParticles.end(); ++it)
	{
		(*it)->StoreBytes(bytes);
		bytes += (*it)->GetNumStoredBytes();
	}
}

void AbsorbParticles::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));

	activeList = GetParticleFromID(data.activeListParticleID);
	inactiveList = GetParticleFromID(data.inactiveListParticleID);

	bytes += sizeof(MyData);
	for (auto it = allParticles.begin(); it != allParticles.end(); ++it)
	{
		(*it)->SetFromBytes(bytes);
		bytes += (*it)->GetNumStoredBytes();
	}
}

//void AbsorbParticles::SingleEnergyParticle::StoreBytes(unsigned char *bytes)
//{
//	memcpy(bytes, &data, sizeof(MyData));
//
//	bytes += sizeof(MyData);
//}
//
//void AbsorbParticles::SingleEnergyParticle::SetFromBytes(unsigned char *bytes)
//{
//	memcpy(&data, bytes, sizeof(MyData));
//
//	bytes += sizeof(MyData);
//}

AbsorbParticles::~AbsorbParticles()
{
	delete[] va;

	for (auto it = allParticles.begin(); it != allParticles.end(); ++it)
	{
		delete (*it);
	}

	/*SingleEnergyParticle *sp = data.activeList;
	SingleEnergyParticle *tNext;
	while (sp != NULL)
	{
		tNext = sp->data.next;
		delete sp;
		sp = tNext;
	}

	sp = data.inactiveList;
	while (sp != NULL)
	{
		tNext = sp->data.next;
		delete sp;
		sp = tNext;
	}*/
}

void AbsorbParticles::Activate(Actor *p_playerTarget, int storedHits, V2d &p_pos,
	float p_startAngle )
{
	//cout << "activate particles" << endl;
	float startSpeed = 4;

	int numProjectiles = storedHits;

	Transform t;
	t.rotate(p_startAngle / PI * 180.f );

	Vector2f vel(0, -startSpeed);
	Vector2f startPos;
	Vector2f targetPos;
	V2d startVel;

	targetPos = Vector2f(p_playerTarget->position);

	switch( abType )
	{
	case ENERGY:
	{
		startPos = Vector2f(p_pos);//Vector2f(round(p_pos.x), round(p_pos.y));
		//targetPos = Vector2f(playerTarget->position);
		break;
	}
	case DARK:
	{
		startPos = Vector2f(p_pos);
		vel = normalize(Vector2f(startPos) - targetPos) * startSpeed; //away from player
		/*startPos = Vector2f(playerTarget->owner->preScreenTex->mapCoordsToPixel(Vector2f(p_pos)));
		targetPos = GetTargetPos(DARK);
		t = Transform::Identity;
		vel = normalize(Vector2f(startPos) - targetPos ) * startSpeed;*/
		break;
	}
	case SHARD:
	{
		//owner->ActivateEffect()

		startPos = Vector2f(p_pos);//Vector2f(playerTarget->owner->preScreenTex->mapCoordsToPixel(Vector2f(p_pos)));
		//pos = Vector2f(playerTarget->owner->preScreenTex->mapCoordsToPixel(Vector2f(p_pos)));
		//startPos = Vector2f(400, 200);
		t = Transform::Identity;
		vel = normalize(Vector2f(startPos) - targetPos) * startSpeed;
		//vel = normalize(Vector2f(targetPos) - pos) * startSpeed;
		break;
	}
	}

	switch (abType)
	{
	case ENERGY:
		break;
	case DARK:
		break;
	case SHARD:
		break;
	}

	SingleEnergyParticle *sp = NULL;
	for (int i = 0; i < numProjectiles; ++i)
	{
		sp = GetInactiveParticle();
		assert(sp != NULL);

		sp->Activate( p_playerTarget, startPos, t.transformPoint(vel));

		if (activeList == NULL)
		{
			activeList = sp;
		}
		else
		{
			sp->next = activeList;
			activeList->prev = sp;
			activeList = sp;
		}
		
		t.rotate(360.f / numProjectiles);
	}
}

AbsorbParticles::SingleEnergyParticle::SingleEnergyParticle(AbsorbParticles *p_parent,
	int p_particleIndex)
	:parent(p_parent), particleIndex(p_particleIndex)
{
	next = NULL;
	prev = NULL;
	playerTarget = NULL;
	Clear();
}

void AbsorbParticles::SingleEnergyParticle::Clear()
{
	sf::Vertex *va = parent->va;
	va[particleIndex * 4].position = Vector2f(0, 0);
	va[particleIndex * 4 + 1].position = Vector2f(0, 0);
	va[particleIndex * 4 + 2].position = Vector2f(0, 0);
	va[particleIndex * 4 + 3].position = Vector2f(0, 0);
}

void AbsorbParticles::SingleEnergyParticle::UpdateSprite()
{
	IntRect sub;
	

	sf::Vertex *va = parent->va;
	
	switch (parent->abType)
	{
	case ENERGY:
	{
		sub.width = 64;//12;
		sub.height = 64;// 12;
		/*va[tileIndex * 4].color = Color::Red;
		va[tileIndex * 4 + 1].color = Color::Blue;
		va[tileIndex * 4 + 2].color = Color::Green;
		va[tileIndex * 4 + 3].color = Color::Cyan;*/
		SetRectSubRect(va + particleIndex * 4, parent->ts->GetSubRect(
			(data.frame % (9 * parent->animFactor)) / parent->animFactor));
		break;
	}
	case DARK:
	{
		sub.width = 128;
		sub.height = 128;
		//SetRectColor(va + tileIndex * 4, Color(Color::White));
		SetRectSubRect(va + particleIndex * 4, parent->ts->GetSubRect(
			(data.frame % (16 * parent->animFactor)) / parent->animFactor));
		/*va[tileIndex * 4 + 0].color = Color::Black;
		va[tileIndex * 4 + 1].color = Color::Black;
		va[tileIndex * 4 + 2].color = Color::Black;
		va[tileIndex * 4 + 3].color = Color::Black;*/
		break;
	}	
	case SHARD:
	{
		sub.width = 128;
		sub.height = 128;
		SetRectSubRect(va + particleIndex * 4, parent->ts->GetSubRect(0));
		break;
	}
	
	}

	//cout << "pos: " << pos.x << ", " << pos.y << "   targetPos" << targetPos.x << ", " << targetPos.y << endl;
	SetRectCenter(va + particleIndex * 4, sub.width, sub.height, data.pos);
}

void AbsorbParticles::SingleEnergyParticle::Activate( Actor *p_playerTarget, Vector2f &p_pos, Vector2f &vel )
{
	data.frame = 0;
	data.velocity = vel;
	data.pos = p_pos;
	
	next = NULL;
	prev = NULL;
	data.lockFrame = -1;

	playerTarget = p_playerTarget;
}

bool AbsorbParticles::SingleEnergyParticle::Update()
{
	assert( playerTarget != NULL );

	float accel = 1;
	Vector2f targetPos = GetTargetPos(parent->abType);
	
	if (parent->data.directKilled)
	{
		switch (parent->abType)
		{
		case DARK:
		{
			Tileset *tss = parent->sess->GetTileset("FX/keyexplode_128x128.png", 128, 128);
			parent->sess->ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES,
				tss, V2d(data.pos), true, 0, 6, 3, true);
			break;
		}
		}
		return false;
	}


	float len = length(targetPos - data.pos);
	if (data.lockFrame != -1 || (len < 60 && data.frame > 30) )
	{
		if (data.lockFrame == -1)
		{
			data.lockFrame = data.frame;
			data.lockDist = len;
		}
		int diffFrame = data.frame - data.lockFrame;
		float distPortion = (float)diffFrame / 10;
		data.pos = targetPos + normalize(data.pos - targetPos ) * data.lockDist * ( 1.f - distPortion );
	}
	else
	{
		data.pos += data.velocity;
	}

	float blahFactor = 0;

	if (data.frame < 20)
	{
		accel = .01;
	}
	else 
	{
		if (parent->abType == DARK)
		{
			accel = 1.f;
		}
		else
		{
			accel = 1.f;
		}
	}
	data.velocity += normalize(targetPos - data.pos) * accel;

	if (data.frame > 30)
	{
		data.velocity = (length(data.velocity) * normalize(targetPos - data.pos));
	}


	if (length(data.velocity) > parent->maxSpeed)
	{
		data.velocity = normalize(data.velocity) * (float)parent->maxSpeed;
	}

	if ( length(targetPos - data.pos) < 1.f && data.frame > 30 )
	{
		switch (parent->abType)
		{
		case ENERGY:
		{
			//need to tune this to heal the right amount
			//int healFrames = 1 * 60;
			//parent->sess->GetPlayer(0)->HealTimer(healFrames); 
			break;
		}
		case DARK:
		{
			parent->sess->ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES,
				parent->ts_explodeDestroy, V2d(targetPos), true, 0, 6, 3, true);
			//parent->sess->CollectKey();
			break;
		}
		case SHARD:
		{
			parent->sess->ActivateEffect(EffectLayer::IN_FRONT,
				parent->ts_explodeDestroy, V2d(targetPos), true, 0, 9, 3, true);
		}
		}
		return false;
	}


	++data.frame;
	return true;
}


int AbsorbParticles::SingleEnergyParticle::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void AbsorbParticles::SingleEnergyParticle::StoreBytes(unsigned char *bytes)
{
	data.nextParticleID = parent->GetParticleID(next);
	data.prevParticleID = parent->GetParticleID(prev);
	data.playerTargetIndex = parent->sess->GetPlayerIndex(playerTarget);

	memcpy(bytes, &data, sizeof(MyData));

	bytes += sizeof(MyData);
}

void AbsorbParticles::SingleEnergyParticle::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));

	next = parent->GetParticleFromID(data.nextParticleID);
	prev = parent->GetParticleFromID(data.prevParticleID);
	playerTarget = parent->sess->GetPlayer(data.playerTargetIndex);

	bytes += sizeof(MyData);
}

AbsorbParticles::SingleEnergyParticle *AbsorbParticles::GetInactiveParticle()
{
	if (inactiveList == NULL)
		return NULL;
	
	SingleEnergyParticle *sp = inactiveList;

	if (inactiveList->next != NULL)
		inactiveList->next->prev = NULL;

	inactiveList = inactiveList->next;

	return sp;
}

void AbsorbParticles::DeactivateParticle(AbsorbParticles::SingleEnergyParticle *sp)
{
	sp->Clear();

	if (activeList == NULL)
		assert(0);

	if (sp->prev == NULL)
	{
		if (sp->next != NULL)
			sp->next->prev = NULL;
		activeList = sp->next;
	}
	else
	{
		sp->prev->next = sp->next;
		if (sp->next != NULL)
			sp->next->prev = sp->prev;
	}

	sp->prev = NULL;
	sp->next = NULL;

	if (inactiveList != NULL)
	{
		sp->next = inactiveList;
		inactiveList->prev = sp;
	}
	inactiveList = sp;
}

void AbsorbParticles::Update()
{
	SingleEnergyParticle *sp = activeList;
	SingleEnergyParticle *tNext = NULL;
	while (sp != NULL)
	{
		tNext = sp->next;
		if (sp->Update())
		{
			sp->UpdateSprite();
		}
		else
		{
			sp->Clear();
			DeactivateParticle(sp);
		}
		sp = tNext;
	}

	data.directKilled = false;
}

void AbsorbParticles::Draw(sf::RenderTarget *target)
{
	//target->draw(va, maxNumParticles * 4, sf::Quads, ts->texture);
	switch (abType)
	{
	case ENERGY:
	case DARK:
		target->draw(va, maxNumParticles * 4, sf::Quads, ts->texture);
		break;
	case SHARD:
		target->draw(va, maxNumParticles * 4, sf::Quads, ts->texture);
		break;
	}
	
}

void AbsorbParticles::KillAllActive()
{
	data.directKilled = true;
}

void AbsorbParticles::Reset()
{
	data.directKilled = false;
	while (activeList != NULL)
	{
		DeactivateParticle(activeList);
	}
}