#include "Enemy.h"
#include "Enemy_BounceBooster.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

using namespace std;
using namespace sf;

BounceBooster::BounceBooster(ActorParams *ap)
	:Enemy(EnemyType::EN_BOUNCEBOOSTER, ap)
{
	SetNumActions(Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	SetLevel(ap->GetLevel());

	strength = 40;

	upOnly = false; //omni
	if (ap->GetTypeName() == "upbouncebooster")
	{
		upOnly = true;
		strength = 40;
		BasicCircleHitBodySetup(80, 0,V2d( 0, 50 ), V2d());
	}
	else
	{
		BasicCircleHitBodySetup(128);
	}

	ts = sess->GetSizedTileset("Enemies/boosters_384x384.png");
	ts_refresh = ts;//sess->GetSizedTileset("Enemies/Booster_on_256x256.png");
	sprite.setTexture(*ts->texture);

	
	//BasicCircleHitBodySetup(90);

	actionLength[NEUTRAL] = 6;
	actionLength[BOOST] = 8;

	animFactor[NEUTRAL] = 3;
	animFactor[BOOST] = 2;

	ResetEnemy();
}

void BounceBooster::SetLevel(int lev)
{
	level = lev;

	switch (level)
	{
	case 1:
		scale = 1.0;
		if (upOnly)
		{
			strength = 30;
			//sprite.setColor(Color::Red);
		}
		else
		{
			strength = 40;
		}

		break;
	case 2:
		scale = 2.0;
		maxHealth += 2;
		strength = 40;
		break;
	case 3:
		scale = 3.0;
		maxHealth += 5;
		strength = 40;
		break;
	}
}

void BounceBooster::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

bool BounceBooster::Boost()
{
	//no refresh
	action = BOOST;
	frame = 0;
	return true;
}

bool BounceBooster::IsBoostable()
{
	return true;
}

void BounceBooster::ResetEnemy()
{
	action = NEUTRAL;
	dead = false;

	frame = 0;
	receivedHit = NULL;

	DefaultHitboxesOn();

	UpdateHitboxes();

	sprite.setTexture(*ts->texture);

	UpdateSprite();
}

void BounceBooster::ProcessState()
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
			action = NEUTRAL;
			frame = 0;
			//sprite.setTexture(*ts_refresh->texture);
			break;
		}
		}
	}
}

void BounceBooster::UpdateSprite()
{
	int tile = 0;

	if (upOnly)
	{
		tile = 3;
	}
	else
	{
		tile = 4;
	}

	IntRect ir;
	switch (action)
	{
	case NEUTRAL:
		//tile = frame / animFactor[NEUTRAL];
		ir = ts->GetSubRect(tile);
		break;
	case BOOST:
		//tile = frame / animFactor[BOOST] + actionLength[NEUTRAL];
		ir = ts->GetSubRect(tile);
		break;
	}

	sprite.setTextureRect(ir);

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void BounceBooster::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}

