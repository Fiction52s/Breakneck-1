#include "Enemy.h"
#include "Enemy_TouchKey.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "CircleGroup.h"
#include "Actor.h"
#include "Enemy_Blocker.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

TouchKeyChain::TouchKeyChain(ActorParams *ap)
	:EnemyChain(ap, EN_TOUCHKEYCHAIN)
{
	TouchKeyParams *cParams = (TouchKeyParams*)ap;

	SetLevel(ap->GetLevel());

	SetKey();

	SetSpawnRect();

	UpdateParams(ap);
}

void TouchKeyChain::UpdateStartPosition(int ind, V2d &pos)
{
	((TouchKey*)enemies[ind])->SetStartPosition(pos);
}

Tileset *TouchKeyChain::GetTileset(int variation)
{
	//return GetSizedTileset("Enemies/General/healthfly_64x64.png");
	return GetSizedTileset("Enemies/General/currency_test_160x160.png");
}

Enemy *TouchKeyChain::CreateEnemy(V2d &pos, int ind)
{
	return new TouchKey(this, ind, pos, level, va + ind * 4, ts);
}


void TouchKeyChain::ReadParams(ActorParams *params)
{
	TouchKeyParams *tkParams = (TouchKeyParams*)params;
	fill = true;
	paramsVariation = tkParams->touchKeyType;
	paramsSpacing = 128;
	fill = true;
}

int TouchKey::GetCounterAmount()
{
	/*switch (level)
	{
	case 1:
	return 1;
	break;
	case 2:
	return 10;
	break;
	case 3:
	return 20;
	break;
	}*/

	return 1;
}

int TouchKey::GetHealAmount()
{
	return 20;
	/*switch (level)
	{
	case 1:
	return 20;
	break;
	case 2:
	return 40;
	break;
	case 3:
	return 80;
	break;
	}*/
}

void TouchKey::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

void TouchKey::SetLevel(int lev)
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

TouchKey::TouchKey(TouchKey &hf)
	:TouchKey(hf.chain, hf.index, hf.GetPosition(), hf.level, hf.quad, hf.ts)
{
	hasMonitor = true;
	SetKey();
}

TouchKey::TouchKey(TouchKeyChain *fc, int p_index, V2d &pos, int p_level, sf::Vertex *p_quad, Tileset *p_ts)
	: Enemy(EnemyType::EN_TOUCHKEY, NULL), chain(fc), index(p_index)
{
	
	SetNumActions(Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	SetLevel(p_level);

	SetKey();

	quad = p_quad;

	startPosInfo.position = pos;
	preTransformPos = startPosInfo.position;

	SetCurrPosInfo(startPosInfo);

	double radius = 80;
	//BasicCircleHitBodySetup(radius);
	BasicCircleHitBodySetup(radius);

	hasPhysics = false;

	ts = p_ts;

	actionLength[NEUTRAL] = 5;
	actionLength[DEATH] = 6;

	animFactor[NEUTRAL] = 5;
	animFactor[DEATH] = 3;

	ResetEnemy();

	SetSpawnRect();
}

void TouchKey::SetStartPosition(V2d &pos)
{
	startPosInfo.position = pos;
	SetCurrPosInfo(startPosInfo);
	UpdateSprite();
}

//bool TouchKey::IsTouchingBox(const sf::Rect<double> &r)
//{
//	return r.intersects(spawnRect);
//}

sf::FloatRect TouchKey::GetAABB()
{
	return GetQuadAABB(quad);
}

bool TouchKey::IsCollectable()
{
	return action == NEUTRAL;
}

void TouchKey::ProcessHit()
{
	if (IsCollectible() && HasReceivedHit())
	{
		//Collect();
		//sess->GetPlayer(receivedHitPlayerIndex)->CollectCurrency(this);
	}
}

//making it not heal when its dead!
void TouchKey::HandleQuery(QuadTreeCollider * qtc)
{
	if (action == NEUTRAL)
	{
		if (qtc != NULL)
		{
			qtc->HandleEntrant(this);
		}
	}
}

void TouchKey::IHitPlayer(int index)
{
	if (IsCollectible())
	{
		Actor *p = sess->GetPlayer(index);
		Collect();
		//p->CollectCurrency(this);
	}
}

bool TouchKey::Collect()
{
	if (action == NEUTRAL)
	{
		action = DEATH;
		frame = 0;
		sess->CollectKey();
		//SetHitboxes(NULL);
		//SetHurtboxes(NULL);
		return true;
	}
	return false;
}

bool TouchKey::IsCollectible()
{
	return action == NEUTRAL;
}

void TouchKey::ResetEnemy()
{
	action = NEUTRAL;
	dead = false;

	frame = 0;
	receivedHit.SetEmpty();

	HurtboxesOff();
	HitboxesOff();

	hasMonitor = true;
	suppressMonitor = false;

	//SetHitboxes(&hitBody);
	//SetHurtboxes(&hurtBody);

	UpdateHitboxes();

	UpdateSprite();
}

void TouchKey::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case NEUTRAL:
		{
			frame = 0;
			break;
		}
		case DEATH:
		{
			numHealth = 0;
			ClearSprite();
			dead = true;
			break;
		}
		}
	}
}

void TouchKey::ClearSprite()
{
	ClearRect(quad);
}

void TouchKey::UpdateSprite()
{
	int tile = 0;
	IntRect ir;
	switch (action)
	{
	case NEUTRAL:
		//tile = frame / animFactor[NEUTRAL];
		tile = 0;
		break;
	case DEATH:
		tile = frame / animFactor[DEATH] + 1;
		break;
	}

	ir = ts->GetSubRect(tile);

	ts->SetQuadSubRect(quad, tile);
	SetRectCenter(quad, ts->tileWidth * scale, ts->tileHeight* scale, GetPositionF());
}

void TouchKey::EnemyDraw(sf::RenderTarget *target)
{
}

void TouchKey::DrawMinimap(sf::RenderTarget *target)
{
	/*if (!dead)
	{
	CircleShape enemyCircle;
	enemyCircle.setFillColor(COLOR_BLUE);
	enemyCircle.setRadius(50);
	enemyCircle.setOrigin(enemyCircle.getLocalBounds().width / 2, enemyCircle.getLocalBounds().height / 2);
	enemyCircle.setPosition(position.x, position.y);
	target->draw(enemyCircle);
	}*/
}

int TouchKey::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void TouchKey::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void TouchKey::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}
