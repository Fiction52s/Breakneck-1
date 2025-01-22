#include "Enemy.h"
#include "Enemy_CurrencyItem.h"
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

CurrencyItemChain::CurrencyItemChain(ActorParams *ap)
	:EnemyChain(ap, EN_CURRENCYCHAIN)
{
	CurrencyItemParams *cParams = (CurrencyItemParams*)ap;

	SetLevel(ap->GetLevel());

	SetSpawnRect();

	UpdateParams(ap);
}

void CurrencyItemChain::UpdateStartPosition(int ind, V2d &pos)
{
	((CurrencyItem*)enemies[ind])->SetStartPosition(pos);
}

Tileset *CurrencyItemChain::GetTileset(int variation)
{
	return GetSizedTileset("Enemies/General/currency_64x64.png");
	//return GetSizedTileset("Enemies/General/currency_test_160x160.png"); 
}

Enemy *CurrencyItemChain::CreateEnemy(V2d &pos, int ind)
{
	return new CurrencyItem(this, ind, pos, level, va + ind * 4, ts);
}


void CurrencyItemChain::ReadParams(ActorParams *params)
{
	CurrencyItemParams *cParams = (CurrencyItemParams*)params;
	fill = true;
	paramsVariation = cParams->currencyItemType;
	paramsSpacing = cParams->spacing;
	fill = cParams->fill;
}

int CurrencyItem::GetCounterAmount()
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

int CurrencyItem::GetHealAmount()
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

void CurrencyItem::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

void CurrencyItem::SetLevel(int lev)
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

CurrencyItem::CurrencyItem(CurrencyItem &hf)
	:CurrencyItem(hf.chain, hf.index, hf.GetPosition(), hf.level, hf.quad, hf.ts)
{

}

CurrencyItem::CurrencyItem(CurrencyItemChain *fc, int p_index, V2d &pos, int p_level, sf::Vertex *p_quad, Tileset *p_ts)
	: Enemy(EnemyType::EN_CURRENCYITEM, NULL), chain(fc), index(p_index)
{
	SetNumActions(Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	SetLevel(p_level);

	quad = p_quad;

	startPosInfo.position = pos;
	preTransformPos = startPosInfo.position;

	SetCurrPosInfo(startPosInfo);

	radius = 128;
	//BasicCircleHitBodySetup(radius);
	BasicCircleHitBodySetup(radius);

	hasPhysics = false;

	ts = p_ts;

	actionLength[NEUTRAL] = 5;
	actionLength[DEATH] = 1;
	actionLength[PLAYER_COLLECT] = 20;

	animFactor[NEUTRAL] = 5;
	animFactor[DEATH] = 1;
	animFactor[PLAYER_COLLECT] = 1;

	ResetEnemy();

	SetSpawnRect();
}

void CurrencyItem::SetStartPosition(V2d &pos)
{
	startPosInfo.position = pos;
	SetCurrPosInfo(startPosInfo);
	UpdateSprite();
}

//bool CurrencyItem::IsTouchingBox(const sf::Rect<double> &r)
//{
//	return r.intersects(spawnRect);
//}

sf::FloatRect CurrencyItem::GetAABB()
{
	return FloatRect(currPosInfo.GetPositionF() + Vector2f(-radius, -radius), Vector2f(radius * 2, radius * 2));//GetQuadAABB(quad);
}

bool CurrencyItem::IsCollectable()
{
	return action == NEUTRAL;
}

void CurrencyItem::ProcessHit()
{
	if (IsCollectible() && HasReceivedHit())
	{
		//Collect();
		sess->GetPlayer(receivedHitPlayerIndex)->CollectCurrency(this);
	}
}

//making it not heal when its dead!
void CurrencyItem::HandleQuery(QuadTreeCollider * qtc)
{
	if (action == NEUTRAL)
	{
		if (qtc != NULL)
		{
			qtc->HandleEntrant(this);
		}
	}
}

void CurrencyItem::IHitPlayer(int index)
{
	/*if (IsCollectible())
	{
		Actor *p = sess->GetPlayer(index);
		Collect();
		p->CollectCurrency(this);
	}*/
}

bool CurrencyItem::Collect( Actor *p )
{
	if (action == NEUTRAL)
	{
		action = PLAYER_COLLECT;//DEATH;
		frame = 0;
		collectedPlayer = p;
		//SetHitboxes(NULL);
		//SetHurtboxes(NULL);
		return true;
	}
	return false;
}

bool CurrencyItem::IsCollectible()
{
	return action == NEUTRAL;
}

void CurrencyItem::ResetEnemy()
{
	action = NEUTRAL;
	dead = false;

	collectedPlayer = NULL;

	frame = 0;
	receivedHit.SetEmpty();

	SetCurrPosInfo(startPosInfo);

	HurtboxesOff();
	HitboxesOff();

	//SetHitboxes(&hitBody);
	//SetHurtboxes(&hurtBody);

	UpdateHitboxes();

	UpdateSprite();
}

void CurrencyItem::ProcessState()
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
		case PLAYER_COLLECT:
		{
			numHealth = 0;
			ClearSprite();
			dead = true;

			//action = DEATH;
			//frame = 0;
			break;
		}
		case DEATH:
		{
			
			break;
		}
		}
	}

	if (action == PLAYER_COLLECT)
	{
		double df = frame + 1;
		double f = df / actionLength[PLAYER_COLLECT];
		V2d currPos = startPosInfo.GetPosition() * (1.0 - f) + collectedPlayer->position * f;
		currPosInfo.position = currPos;
	}
}

void CurrencyItem::ClearSprite()
{
	ClearRect(quad);
}

void CurrencyItem::UpdateSprite()
{
	int tile = 0;
	IntRect ir;
	switch (action)
	{
	case NEUTRAL:
		tile = frame / animFactor[NEUTRAL];
		//tile = 0;
		break;
	case DEATH:
		tile = 1;//frame / animFactor[DEATH] + 1;
		break;
	}

	tile = 0;

	float tempScale = 1.f;
	if (action == PLAYER_COLLECT)
	{
		float ff = frame + 1;
		float f = ff / actionLength[PLAYER_COLLECT];
		tempScale = 1.f - f;
	}

	ir = ts->GetSubRect(tile);

	ts->SetQuadSubRect(quad, tile);
	SetRectCenter(quad, ts->tileWidth * scale * tempScale, ts->tileHeight* scale * tempScale, GetPositionF());
}

void CurrencyItem::EnemyDraw(sf::RenderTarget *target)
{
}

void CurrencyItem::DrawMinimap(sf::RenderTarget *target)
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

int CurrencyItem::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void CurrencyItem::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void CurrencyItem::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}
