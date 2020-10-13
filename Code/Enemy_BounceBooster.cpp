#include "Enemy.h"
#include "Enemy_BounceBooster.h"
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

BounceBooster::BounceBooster(ActorParams *ap)
	:Enemy(EnemyType::EN_BOUNCEBOOSTER, ap)
{
	SetNumActions(Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	SetLevel(ap->GetLevel());

	strength = 20;

	upOnly = false; //omni
	if (ap->GetTypeName() == "upbouncebooster")
	{
		upOnly = true;
	}

	ts = sess->GetSizedTileset("Enemies/Booster_512x512.png");
	ts_refresh = sess->GetSizedTileset("Enemies/Booster_on_256x256.png");
	sprite.setTexture(*ts->texture);

	BasicCircleHitBodySetup(90);
	//BasicCircleHitBodySetup(90);

	actionLength[NEUTRAL] = 6;
	actionLength[BOOST] = 8;
	actionLength[REFRESH] = 7;

	animFactor[NEUTRAL] = 3;
	animFactor[BOOST] = 2;
	animFactor[REFRESH] = 2;

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
			sprite.setColor(Color::Red);
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
	if (action == NEUTRAL)
	{
		action = BOOST;
		frame = 0;
		return true;
	}
	return false;
}

bool BounceBooster::IsBoostable()
{
	return action == NEUTRAL;
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
			action = REFRESH;
			frame = 0;
			sprite.setTexture(*ts_refresh->texture);
			break;
		}
		case REFRESH:
		{
			action = NEUTRAL;
			frame = 0;
			sprite.setTexture(*ts->texture);
			break;
		}
		}
	}
}

void BounceBooster::UpdateSprite()
{
	int tile = 0;
	IntRect ir;
	switch (action)
	{
	case NEUTRAL:
		tile = frame / animFactor[NEUTRAL];
		ir = ts->GetSubRect(tile);
		break;
	case BOOST:
		tile = frame / animFactor[BOOST] + actionLength[NEUTRAL];
		ir = ts->GetSubRect(tile);
		break;
	case REFRESH:
		tile = frame / animFactor[REFRESH];
		ir = ts_refresh->GetSubRect(tile);
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

