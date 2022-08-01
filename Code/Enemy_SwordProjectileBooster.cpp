#include "Enemy.h"
#include "Enemy_SwordProjectileBooster.h"
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


SwordProjectileBooster::SwordProjectileBooster(ActorParams *ap)//Vector2i &pos, int p_level)
	:Enemy(EnemyType::EN_SWORDPROJECTILEBOOSTER, ap)//, false, 1, false), strength( 20 )
{
	SetNumActions(Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	SetLevel(ap->GetLevel());

	SetCurrPosInfo(startPosInfo);

	const string &name = ap->GetTypeName();

	if (name == "swordprojectilebooster")
	{
		enemyProjectile = false;
		sprite.setColor(Color::Yellow);
	}
	else if (name == "enemyswordprojectilebooster")
	{
		enemyProjectile = true;
		sprite.setColor(Color::Red);
	}
	
	strength = 300;

	ts = GetSizedTileset("Enemies/Booster_512x512.png");
	ts_refresh = GetSizedTileset("Enemies/Booster_on_256x256.png");

	sprite.setScale(scale, scale);

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

void SwordProjectileBooster::ResetEnemy()
{
	action = NEUTRAL;
	frame = 0;

	SetHitboxes(&hitBody, 0);
	UpdateHitboxes();

	sprite.setTexture(*ts->texture);
	UpdateSprite();
}

void SwordProjectileBooster::SetLevel(int lev)
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

void SwordProjectileBooster::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

bool SwordProjectileBooster::Boost()
{
	if (action == NEUTRAL)
	{
		action = BOOST;
		frame = 0;
		return true;
	}
	return false;
}

bool SwordProjectileBooster::IsBoostable()
{
	return action == NEUTRAL;
}



void SwordProjectileBooster::ProcessState()
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

void SwordProjectileBooster::UpdateSprite()
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

void SwordProjectileBooster::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}

void SwordProjectileBooster::DrawMinimap(sf::RenderTarget *target)
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

int SwordProjectileBooster::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void SwordProjectileBooster::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void SwordProjectileBooster::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}
