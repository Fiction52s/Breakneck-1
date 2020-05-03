#include "Enemy.h"
#include "Enemy_HealthFly.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>


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
	:HealthFly( Vector2i( hf.GetPosition() ), hf.level, hf.quad, hf.ts )
{

}

HealthFly::HealthFly(sf::Vector2i &pos, int p_level, sf::Vertex *p_quad, Tileset *p_ts )
	:Enemy(EnemyType::EN_HEALTHFLY, NULL)
{
	SetNumActions(Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	SetLevel(p_level);

	quad = p_quad;

	startPosInfo.position = V2d(pos);
	preTransformPos = startPosInfo.position;

	SetCurrPosInfo(startPosInfo);

	double radius = 40;
	BasicCircleHitBodySetup(radius);
	BasicCircleHurtBodySetup(radius);

	ts = p_ts;

	actionLength[NEUTRAL] = 5;
	actionLength[DEATH] = 8;

	animFactor[NEUTRAL] = 3;
	animFactor[DEATH] = 3;

	ResetEnemy();

	SetSpawnRect();
}

void HealthFly::SetPosition(V2d &pos)
{
	startPosInfo.position = pos;
	SetCurrPosInfo(startPosInfo);
	UpdateSprite();
}

sf::FloatRect HealthFly::GetAABB()
{
	return GetQuadAABB(quad);
}

//making it not heal when its dead!
void HealthFly::HandleQuery(QuadTreeCollider * qtc)
{
	if (!dead)
	{
		qtc->HandleEntrant(this);
	}
}

bool HealthFly::Collect()
{
	if (action == NEUTRAL)
	{
		action = DEATH;
		frame = 0;
		return true;
	}
	return false;
}

bool HealthFly::IsCollectible()
{
	return true;
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

