#include "Enemy.h"
#include "Enemy_PhaseBooster.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>


using namespace std;
using namespace sf;

PhaseBooster::PhaseBooster(ActorParams *ap)//Vector2i &pos, int p_level)
	:Enemy(EnemyType::EN_PHASEBOOSTER, ap)//, false, 1, false), strength( 20 )
{
	SetNumActions(Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	SetLevel(ap->GetLevel());

	SetCurrPosInfo(startPosInfo);

	strength = 300;

	ts = sess->GetSizedTileset("Enemies/boosters_384x384.png");
	//ts_refresh = sess->GetSizedTileset("Enemies/Booster_on_256x256.png");

	sprite.setScale(scale, scale);
	//sprite.setColor(Color::Magenta);

	sprite.setTexture(*ts->texture);

	double radius = 90;
	BasicCircleHitBodySetup(radius);

	actionLength[NEUTRAL] = 6;
	actionLength[BOOST] = 8;
	actionLength[REFRESH] = 7;

	animFactor[NEUTRAL] = 3;
	animFactor[BOOST] = 3;
	animFactor[REFRESH] = 5;

	ResetEnemy();

	SetSpawnRect();
}

void PhaseBooster::ResetEnemy()
{
	action = NEUTRAL;
	frame = 0;

	SetHitboxes(&hitBody, 0);
	UpdateHitboxes();

	//sprite.setTexture(*ts->texture);
	UpdateSprite();
}

void PhaseBooster::SetLevel(int lev)
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

void PhaseBooster::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

bool PhaseBooster::Boost()
{
	if (action == NEUTRAL)
	{
		action = BOOST;
		frame = 0;
		return true;
	}
	return false;
}

bool PhaseBooster::IsBoostable()
{
	return action == NEUTRAL;
}


void PhaseBooster::ProcessState()
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
		case BOOST:
		{
			action = REFRESH;
			frame = 0;
			//sprite.setTexture(*ts_refresh->texture);
			break;
		}
		case REFRESH:
		{
			action = NEUTRAL;
			frame = 0;
			//sprite.setTexture(*ts->texture);
			break;
		}
		}
	}
}

void PhaseBooster::UpdateSprite()
{
	int tile = 0;
	IntRect ir;
	switch (action)
	{
	case NEUTRAL:
		tile = frame / animFactor[NEUTRAL];
		//ir = ts->GetSubRect(tile);
		break;
	case BOOST:
		tile = frame / animFactor[BOOST] + actionLength[NEUTRAL];
		//ir = ts->GetSubRect(tile);
		break;
	case REFRESH:
		tile = frame / animFactor[REFRESH];
		//ir = ts_refresh->GetSubRect(tile);
		break;
	}

	sprite.setTextureRect(ts->GetSubRect(7));

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void PhaseBooster::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}

void PhaseBooster::DrawMinimap(sf::RenderTarget *target)
{
	if (!dead)
	{
		CircleShape enemyCircle;
		enemyCircle.setFillColor(Color::Blue);
		enemyCircle.setRadius(50);
		enemyCircle.setOrigin(enemyCircle.getLocalBounds().width / 2, enemyCircle.getLocalBounds().height / 2);
		enemyCircle.setPosition(GetPositionF());
		target->draw(enemyCircle);
	}
}

