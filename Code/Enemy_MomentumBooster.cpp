#include "Enemy.h"
#include "Enemy_MomentumBooster.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Actor.h"


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

MomentumBooster::MomentumBooster(ActorParams *ap)//Vector2i &pos, int p_level)
	:Enemy(EnemyType::EN_MOMENTUMBOOSTER, ap)//, false, 1, false), strength( 20 )
{
	SetNumActions(Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	SetLevel(ap->GetLevel());

	SetCurrPosInfo(startPosInfo);

	strength = 300;

	ts = GetSizedTileset("Enemies/W4/momentum_booster_192x256.png");

	//ts_refresh = sess->GetSizedTileset("Enemies/Booster_on_256x256.png");

	//sprite.setScale(scale, scale);

	flameAnimFactor = 3;

	double radius = 90;
	BasicCircleHitBodySetup(radius);

	actionLength[NEUTRAL] = 2;//6;
	actionLength[BOOST] = 6;
	actionLength[REFRESH] = 7;

	animFactor[NEUTRAL] = 1;
	animFactor[BOOST] = 3;
	animFactor[REFRESH] = 5;

	ResetEnemy();

	SetSpawnRect();
}

void MomentumBooster::ResetEnemy()
{
	action = NEUTRAL;
	frame = 0;

	SetHitboxes(&hitBody, 0);
	UpdateHitboxes();

	data.spinnerAngle = 0;

	data.flameFrame = 0;


	UpdateSprite();
}

void MomentumBooster::SetLevel(int lev)
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

void MomentumBooster::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

bool MomentumBooster::Boost()
{
	if (action == NEUTRAL)
	{
		action = BOOST;
		frame = 0;
		return true;
	}
	return false;
}

sf::FloatRect MomentumBooster::GetAABB()
{ 
	return GetQuadAABB(quads);
}

bool MomentumBooster::IsBoostable()
{
	return action == NEUTRAL;
}


void MomentumBooster::ProcessState()
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

	/*if (action == NEUTRAL)
	{
		data.spinnerAngle += .02 * PI;
	}*/

	if (sess->GetPlayer(0)->momentumBoostFrames > 0)
	{
		data.spinnerAngle += .1 * PI;
	}
	else
	{
		data.spinnerAngle += .02 * PI;
	}
	
}

void MomentumBooster::FrameIncrement()
{
	++data.flameFrame;

	if (data.flameFrame == 8 * flameAnimFactor)
	{
		data.flameFrame = 0;
	}
}

void MomentumBooster::UpdateSprite()
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
		tile = 0;//frame / animFactor[REFRESH];
		//ir = ts_refresh->GetSubRect(tile);
		break;
	}


	SetRectCenter(quads, ts->tileWidth * scale, ts->tileHeight * scale, GetPositionF());
	SetRectCenter(quads + 8, ts->tileWidth * scale, ts->tileHeight * scale, GetPositionF());

	ts->SetQuadSubRect(quads, 8 + data.flameFrame / flameAnimFactor); //flame

	
	if (action == REFRESH)
	{
		ts->SetQuadSubRect(quads + 4, 1); //spinner
	}
	else
	{
		ts->SetQuadSubRect(quads + 4, 0); //spinner
	}
	
	SetRectRotation(quads + 4, data.spinnerAngle - PI / 2, ts->tileWidth * scale, ts->tileHeight * scale, GetPositionF());



	if (action == BOOST)
	{
		int tile = frame / animFactor[BOOST] + 2;
		ts->SetQuadSubRect(quads + 8, tile); //shine	
	}
	else
	{
		ClearRect(quads + 8);
	}
	//sprite.setTextureRect(ts->GetSubRect(tile));

	//sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	//sprite.setPosition(GetPositionF());
}

void MomentumBooster::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(quads, 3 * 4, sf::Quads, ts->texture);
	//target->draw(sprite);
}

void MomentumBooster::DrawMinimap(sf::RenderTarget *target)
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

int MomentumBooster::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void MomentumBooster::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void MomentumBooster::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}

