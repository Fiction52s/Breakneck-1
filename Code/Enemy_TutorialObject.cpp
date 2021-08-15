#include "Enemy_TutorialObject.h"
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


TutorialObject::TutorialObject(ActorParams *ap)
	:Enemy(EnemyType::EN_TUTORIALOBJECT, ap)
{
	SetNumActions(Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	SetLevel(ap->GetLevel());

	SetCurrPosInfo(startPosInfo);

	ts = GetSizedTileset("Enemies/booster_512x512.png");
	ts_tutorial = GetTileset("Menu/tut_dash.png", 1220, 320 );

	sprite.setScale(scale, scale);
	sprite.setColor(Color::Red);

	tutorialSpr.setTexture(*ts_tutorial->texture);
	tutorialSpr.setOrigin(tutorialSpr.getLocalBounds().width / 2, 
		tutorialSpr.getLocalBounds().height / 2);

	double radius = 90;
	BasicCircleHitBodySetup(radius);

	actionLength[NEUTRAL] = 6;
	actionLength[SHOW] = 8;

	animFactor[NEUTRAL] = 3;
	animFactor[SHOW] = 3;

	ResetEnemy();

	SetSpawnRect();
}

void TutorialObject::ResetEnemy()
{
	action = NEUTRAL;
	frame = 0;

	SetHitboxes(&hitBody, 0);
	UpdateHitboxes();

	sprite.setTexture(*ts->texture);
	UpdateSprite();
}

void TutorialObject::SetLevel(int lev)
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

void TutorialObject::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

bool TutorialObject::ShowTutorial()
{
	if (action == NEUTRAL)
	{
		action = SHOW;
		frame = 0;
		sess->cam.Ease(GetPositionF(), 2.0, 60);
		return true;
	}
	return false;
}

bool TutorialObject::IsTutorialShowable()
{
	return action == NEUTRAL;
}



void TutorialObject::ProcessState()
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
		case SHOW:
		{
			frame = 0;	
			break;
		}
		}
	}
}

void TutorialObject::UpdateSprite()
{
	int tile = 0;
	IntRect ir;
	switch (action)
	{
	case NEUTRAL:
		tile = frame / animFactor[NEUTRAL];
		ir = ts->GetSubRect(tile);
		break;
	case SHOW:
		tile = 0;
		ir = ts->GetSubRect(tile);
		break;
	}

	sprite.setTextureRect(ir);

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());

	tutorialSpr.setPosition(GetPositionF() + Vector2f( 0, - 400 ));
}

void TutorialObject::EnemyDraw(sf::RenderTarget *target)
{
	
	target->draw(sprite);

	if (action == SHOW)
	{
		target->draw(tutorialSpr);
	}
}

void TutorialObject::DrawMinimap(sf::RenderTarget *target)
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

