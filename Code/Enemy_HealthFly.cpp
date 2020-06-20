#include "Enemy.h"
#include "Enemy_HealthFly.h"
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

FlyChain::FlyChain(ActorParams *ap)
	:EnemyChain( ap, EN_FLYCHAIN )
{
	FlyParams *fParams = (FlyParams*)ap;

	SetLevel(ap->GetLevel());

	SetSpawnRect();

	UpdateParams(ap);
}

void FlyChain::UpdateStartPosition(int ind, V2d &pos)
{
	((HealthFly*)enemies[ind])->SetStartPosition(pos);
}

Tileset *FlyChain::GetTileset(int variation)
{
	return sess->GetTileset("Enemies/healthfly_64x64.png", 64, 64);
}

Enemy *FlyChain::CreateEnemy(V2d &pos, int ind)
{
	return new HealthFly(this, ind, pos, level, va + ind * 4, ts);
}


void FlyChain::ReadParams(ActorParams *params)
{
	FlyParams *fParams = (FlyParams*)params;
	fill = true;
	paramsVariation = fParams->fType;
	paramsSpacing = fParams->spacing;
}

int HealthFly::GetCounterAmount()
{
	switch (level)
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
	}
}

int HealthFly::GetHealAmount()
{
	switch (level)
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
	}
}

void HealthFly::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

void HealthFly::SetLevel(int lev)
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

HealthFly::HealthFly(HealthFly &hf)
	:HealthFly( hf.chain, hf.index, hf.GetPosition(), hf.level, hf.quad, hf.ts )
{

}

HealthFly::HealthFly( FlyChain *fc, int p_index, V2d &pos, int p_level, sf::Vertex *p_quad, Tileset *p_ts )
	:Enemy(EnemyType::EN_HEALTHFLY, NULL), chain( fc ), index( p_index )
{
	SetNumActions(Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	SetLevel(p_level);

	quad = p_quad;

	startPosInfo.position = pos;
	preTransformPos = startPosInfo.position;

	SetCurrPosInfo(startPosInfo);

	double radius = 40;
	BasicCircleHitBodySetup(radius);
	BasicCircleHurtBodySetup(radius);

	ts = p_ts;

	actionLength[NEUTRAL] = 5;
	actionLength[DEATH] = 8;

	animFactor[NEUTRAL] = 5;
	animFactor[DEATH] = 3;

	ResetEnemy();

	SetSpawnRect();
}

void HealthFly::SetStartPosition(V2d &pos)
{
	startPosInfo.position = pos;
	SetCurrPosInfo(startPosInfo);
	UpdateSprite();
}

//bool HealthFly::IsTouchingBox(const sf::Rect<double> &r)
//{
//	return r.intersects(spawnRect);
//}

sf::FloatRect HealthFly::GetAABB()
{
	return GetQuadAABB(quad);
}

void HealthFly::ProcessHit()
{
	if (IsCollectible() && ReceivedHit())
	{
		Collect();
		receivedHitPlayer->CollectFly(this);
	}
}

//making it not heal when its dead!
//void HealthFly::HandleQuery(QuadTreeCollider * qtc)
//{
//	if (!dead)
//	{
//		if (qtc != NULL)
//		{
//			qtc->HandleEntrant(this);
//		}
//	}
//}

void HealthFly::IHitPlayer(int index)
{
	if (IsCollectible())
	{
		Actor *p = sess->GetPlayer(index);
		Collect();
		p->CollectFly(this);
	}
}

bool HealthFly::Collect()
{
	if (action == NEUTRAL)
	{
		action = DEATH;
		frame = 0;
		//SetHitboxes(NULL);
		//SetHurtboxes(NULL);
		return true;
	}
	return false;
}

bool HealthFly::IsCollectible()
{
	return action == NEUTRAL;
}

void HealthFly::ResetEnemy()
{
	action = NEUTRAL;
	dead = false;

	frame = 0;
	receivedHit = NULL;

	SetHitboxes(&hitBody);
	SetHurtboxes(&hurtBody);

	UpdateHitboxes();

	UpdateSprite();
}

void HealthFly::ProcessState()
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

void HealthFly::ClearSprite()
{
	ClearRect(quad);
}

void HealthFly::UpdateSprite()
{
	int tile = 0;
	IntRect ir;
	switch (action)
	{
	case NEUTRAL:
		tile = frame / animFactor[NEUTRAL];
		break;
	case DEATH:
		tile = 0;
		break;
	}

	ir = ts->GetSubRect(tile);

	ts->SetQuadSubRect(quad, tile);
	SetRectCenter(quad, ts->tileWidth * scale, ts->tileHeight* scale, GetPositionF());
}

void HealthFly::EnemyDraw(sf::RenderTarget *target)
{
}

void HealthFly::DrawMinimap(sf::RenderTarget *target)
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

