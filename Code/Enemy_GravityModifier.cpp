#include "Enemy.h"
#include "Enemy_GravityModifier.h"
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

GravityModifier::GravityModifier(ActorParams *ap )
	:Enemy(EnemyType::EN_GRAVITYMODIFIER ), gravFactor(1.0), duration( 300 )
{
	SetNumActions(Count);
	SetEditorActions(NEUTRAL, 0, 0);

	actionLength[NEUTRAL] = 15;
	actionLength[MODIFY] = 1;
	actionLength[REFRESH] = 2;

	animFactor[NEUTRAL] = 5;
	animFactor[MODIFY] = 45;
	animFactor[REFRESH] = 30;

	const string &typeName = ap->GetTypeName();

	if (typeName == "gravityincreaser")
	{
		increaser = true;
	}
	else if (typeName == "gravitydecreaser")
	{
		increaser = false;
	}
	else
	{
		assert(0);
	}
	
	if (increaser)
	{
		ts = sess->GetSizedTileset("Enemies/grav_increase_256x256.png");
	}
	else
	{
		ts = sess->GetSizedTileset("Enemies/grav_decrease_256x256.png");
	}


	sprite.setTexture(*ts->texture);

	BasicCircleHurtBodySetup(90);
	BasicCircleHitBodySetup(90);

	ResetEnemy();
}

bool GravityModifier::Modify()
{
	if (action == NEUTRAL)
	{
		action = MODIFY;
		frame = 0;
		return true;
	}
	return false;
}

bool GravityModifier::IsModifiable()
{
	return action == NEUTRAL;
}

void GravityModifier::ResetEnemy()
{
	action = NEUTRAL;
	frame = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();
	UpdateHitboxes();

	sprite.setTexture(*ts->texture);

	UpdateSprite();
}

void GravityModifier::ProcessState()
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
		case MODIFY:
		{
			action = REFRESH;
			frame = 0;
			break;
		}
		case REFRESH:
		{
			action = NEUTRAL;
			frame = 0;
			
			break;
		}
		}
	}
}

void GravityModifier::UpdateSprite()
{
	int tile = 0;
	IntRect ir;
	switch (action)
	{
	case NEUTRAL:
		tile = frame / animFactor[NEUTRAL];
		ir = ts->GetSubRect(tile);
		break;
	case MODIFY:
		tile = frame / animFactor[MODIFY] + actionLength[NEUTRAL];
		ir = ts->GetSubRect(tile);
		break;
	case REFRESH:
		tile = frame / animFactor[REFRESH] + 16;
		ir = ts->GetSubRect(tile);
		break;
	}

	sprite.setTextureRect(ir);

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void GravityModifier::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}

//void GravityModifier::DrawMinimap(sf::RenderTarget *target)
//{
//	if (!dead)
//	{
//		CircleShape enemyCircle;
//		enemyCircle.setFillColor(COLOR_BLUE);
//		enemyCircle.setRadius(50);
//		enemyCircle.setOrigin(enemyCircle.getLocalBounds().width / 2, enemyCircle.getLocalBounds().height / 2);
//		enemyCircle.setPosition(position.x, position.y);
//		target->draw(enemyCircle);
//	}
//}

