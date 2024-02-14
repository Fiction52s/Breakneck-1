#include "Enemy.h"
#include "Enemy_AntiTimeSlowBooster.h"
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


AntiTimeSlowBooster::AntiTimeSlowBooster(ActorParams *ap)
	:Enemy(EnemyType::EN_ANTITIMESLOWBOOSTER, ap)
{
	SetNumActions(Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	SetLevel(ap->GetLevel());

	SetCurrPosInfo(startPosInfo);

	strength = 360;

	ts = GetSizedTileset("Enemies/W5/anti_time_384x384.png");
	//ts_refresh = sess->GetSizedTileset("Enemies/booster_on_256x256.png");

	sprite.setScale(scale, scale);

	boostSound = GetSound("Enemies/Boosters/booster_anti_time_slow");
	//sprite.setColor(Color::Red);

	double radius = 90;
	BasicCircleHitBodySetup(radius);

	actionLength[NEUTRAL] = 1;
	actionLength[BOOST] = 10;
	actionLength[REFRESH] = 7;

	animFactor[NEUTRAL] = 3;
	animFactor[BOOST] = 5;
	animFactor[REFRESH] = 5;

	ResetEnemy();

	SetSpawnRect();
}

void AntiTimeSlowBooster::ResetEnemy()
{
	action = NEUTRAL;
	frame = 0;

	SetHitboxes(&hitBody, 0);
	UpdateHitboxes();

	sprite.setTexture(*ts->texture);
	UpdateSprite();
}

void AntiTimeSlowBooster::SetLevel(int lev)
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

void AntiTimeSlowBooster::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

bool AntiTimeSlowBooster::Boost()
{
	if (action == NEUTRAL)
	{
		sess->ActivateSoundAtPos(GetPosition(), boostSound);
		action = BOOST;
		frame = 0;
		return true;
	}
	return false;
}

bool AntiTimeSlowBooster::IsBoostable()
{
	return action == NEUTRAL;
}

bool AntiTimeSlowBooster::IsSlowed(int index)
{
	return false;
}

void AntiTimeSlowBooster::ProcessState()
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

void AntiTimeSlowBooster::UpdateSprite()
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
		tile = frame / animFactor[BOOST] + 1;
		//ir = ts->GetSubRect(tile);
		break;
	case REFRESH:
		tile = frame / animFactor[REFRESH] + 12;
		//ir = ts_refresh->GetSubRect(tile);
		break;
	}



	sprite.setTextureRect(ts->GetSubRect(tile) );

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void AntiTimeSlowBooster::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}

void AntiTimeSlowBooster::DrawMinimap(sf::RenderTarget *target)
{
	if (!dead)
	{
		CircleShape enemyCircle;
		enemyCircle.setFillColor(COLOR_BLUE);
		enemyCircle.setRadius(50);
		enemyCircle.setOrigin(enemyCircle.getLocalBounds().width / 2, enemyCircle.getLocalBounds().height / 2);
		enemyCircle.setPosition(GetPositionF());
		target->draw(enemyCircle);
	}
}

int AntiTimeSlowBooster::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void AntiTimeSlowBooster::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void AntiTimeSlowBooster::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}

