#include "Enemy.h"
#include "Enemy_GravityModifier.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>


using namespace std;
using namespace sf;

GravityModifier::GravityModifier(ActorParams *ap )
	:Enemy(EnemyType::EN_GRAVITYMODIFIER, ap), gravFactor(1.0), duration( 300 )
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
		ts = GetSizedTileset("Enemies/W2/grav_increase_256x256.png");
		gravFactor = 2.0;
	}
	else
	{
		ts = GetSizedTileset("Enemies/W2/grav_decrease_256x256.png");
		gravFactor = .5;
	}

	sprite.setTexture(*ts->texture);

	//BasicCircleHurtBodySetup(90);
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

void GravityModifier::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

bool GravityModifier::IsModifiable()
{
	return action == NEUTRAL;
}

void GravityModifier::ResetEnemy()
{
	action = NEUTRAL;
	frame = 0;

	//HitboxesOff();

	//this is inefficient because it does the normal check for hitting a player,
	//but has no effect. need to adjust this for all the enemies that are
	//used in world trees. just wastes processor time
	DefaultHitboxesOn();
	//DefaultHurtboxesOn();
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

int GravityModifier::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void GravityModifier::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void GravityModifier::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
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

