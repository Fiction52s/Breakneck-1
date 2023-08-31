#include "Enemy.h"
#include "Enemy_TimeBooster.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "AbsorbParticles.h"

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

TimeBooster::TimeBooster(ActorParams *ap)//Vector2i &pos, int p_level)
	:Enemy(EnemyType::EN_TIMEBOOSTER, ap)//, false, 1, false), strength( 20 )
{
	SetNumActions(Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	SetLevel(ap->GetLevel());

	SetCurrPosInfo(startPosInfo);

	strength = 300;//180

	ts = GetSizedTileset("Enemies/boosters_384x384.png");

	sprite.setTexture(*ts->texture);
	//ts_refresh = sess->GetSizedTileset("Enemies/Booster_on_256x256.png");

	sprite.setScale(scale, scale);
	//sprite.setColor(Color::Magenta);

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

void TimeBooster::ResetEnemy()
{
	action = NEUTRAL;
	frame = 0;

	SetHitboxes(&hitBody, 0);
	UpdateHitboxes();

	
	UpdateSprite();
}

void TimeBooster::SetLevel(int lev)
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

void TimeBooster::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

bool TimeBooster::Boost()
{
	if (action == NEUTRAL)
	{
		action = BOOST;
		frame = 0;

		if (hasMonitor && !suppressMonitor )
		{	
			PlayKeyDeathSound();
			if (hasMonitor && !suppressMonitor)
			{
				sess->ActivateAbsorbParticles(AbsorbParticles::AbsorbType::DARK,
					sess->GetPlayer(receivedHitPlayerIndex), GetNumDarkAbsorbParticles(), GetPosition());
			}
			suppressMonitor = true;
		}
		return true;
	}
	return false;
}

bool TimeBooster::IsBoostable()
{
	return action == NEUTRAL;
}


void TimeBooster::ProcessState()
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

bool TimeBooster::IsSlowed(int index)
{
	return false;
}

void TimeBooster::UpdateSprite()
{
	int tile = 0;
	IntRect ir;
	switch (action)
	{
	case NEUTRAL:
		tile = frame / animFactor[NEUTRAL];
		sprite.setColor(Color::White);
		//ir = ts->GetSubRect(tile);
		break;
	case BOOST:
		tile = frame / animFactor[BOOST] + actionLength[NEUTRAL];
		sprite.setColor(Color::Blue);
		//ir = ts->GetSubRect(tile);
		break;
	case REFRESH:
		tile = frame / animFactor[REFRESH];
		sprite.setColor(Color::Green);
		//ir = ts_refresh->GetSubRect(tile);
		break;
	}

	sprite.setTextureRect(ts->GetSubRect( 8 ) );

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void TimeBooster::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
	//target->draw(sprite);
}

void TimeBooster::DrawMinimap(sf::RenderTarget *target)
{
	if (!dead)
	{
		if (hasMonitor && !suppressMonitor)
		{
			CircleShape cs;
			cs.setRadius(50);
			cs.setFillColor(Color::White);
			cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
			cs.setPosition(GetPositionF());
			target->draw(cs);
		}
		else
		{
			CircleShape cs;
			cs.setRadius(40);
			cs.setFillColor(COLOR_BLUE);
			cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
			cs.setPosition(GetPositionF());
			target->draw(cs);
		}
	}
}

int TimeBooster::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void TimeBooster::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void TimeBooster::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}
