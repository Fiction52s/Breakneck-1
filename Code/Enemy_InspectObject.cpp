#include "Enemy_InspectObject.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "InspectSequence.h"


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


InspectObject::InspectObject(ActorParams *ap)
	:Enemy(EnemyType::EN_INSPECTOBJECT, ap)
{
	SetNumActions(Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	SetLevel(ap->GetLevel());

	SetCurrPosInfo(startPosInfo);


	SetLevel(ap->GetLevel());

	const string &typeName = ap->GetTypeName();

	if (typeName == "familypicture")
	{
	}

	ts = GetSizedTileset("Enemies/General/tutorial_flower_256x256.png");

	inspectSeq = new InspectSequence;
	inspectSeq->Init();

	sprite.setScale(scale, scale);
	//sprite.setColor(Color::Red);

	entranceRadius = 200;
	exitRadius = 200;
	//double radius = 500;
	//BasicCircleHitBodySetup(radius);

	actionLength[NEUTRAL] = 6;
	actionLength[SHOW] = 8;
	actionLength[READY_TO_SHOW] = 8;
	actionLength[RECOVERY] = 30;

	animFactor[NEUTRAL] = 3;
	animFactor[SHOW] = 3;
	animFactor[READY_TO_SHOW] = 1;
	animFactor[RECOVERY] = 1;

	UpdateParamsSettings();

	ResetEnemy();

	//spawnRect = Rect<double>(GetPosition(), V2d(entranceRadius * 2, entranceRadius * 2));
	SetSpawnRect();
}

InspectObject::~InspectObject()
{
	delete inspectSeq;
}

sf::FloatRect InspectObject::GetAABB()
{
	Vector2f posF = GetPositionF();
	return FloatRect(posF.x - entranceRadius, posF.y - entranceRadius, entranceRadius * 2,
		entranceRadius * 2);
}

void InspectObject::ResetEnemy()
{
	action = NEUTRAL;
	frame = 0;

	//SetHitboxes(&hitBody, 0);
	UpdateHitboxes();

	sprite.setTexture(*ts->texture);
	UpdateSprite();
}

void InspectObject::SetLevel(int lev)
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

void InspectObject::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

void InspectObject::SetExtraIDsAndAddToVectors()
{
	inspectSeq->SetIDs();
}

bool InspectObject::ShowTutorial()
{
	if (action == NEUTRAL)
	{
		action = SHOW;
		frame = 0;
		//sess->cam.Ease(GetPositionF() + Vector2f( 0, -350 ), 2.0, 60);
		inspectSeq->Reset();
		sess->SetActiveSequence(inspectSeq);
		return true;
	}
	return false;
}

bool InspectObject::IsReadyToShow()
{
	return action == READY_TO_SHOW;
}

bool InspectObject::IsShowing()
{
	return action == SHOW;
}

void InspectObject::HideTutorial()
{
	//sess->cam.EaseOutOfManual(60);
	action = NEUTRAL;
	frame = 0;
	//sess->SetActiveSequence(NULL);
}


void InspectObject::ProcessState()
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
		case READY_TO_SHOW:
		{
			frame = 0;
			break;
		}
		case SHOW:
		{
			frame = 0;
			break;
		}
		case RECOVERY:
		{
			action = NEUTRAL;
			frame = 0;
			break;
		}
		
		}
	}

	double dist = PlayerDist();
	switch (action)
	{
	case NEUTRAL:
		if (dist < entranceRadius)
		{
			action = READY_TO_SHOW;
			frame = 0;
		}
		break;
	case READY_TO_SHOW:
		if (dist > exitRadius)
		{
			action = NEUTRAL;
			frame = 0;
		}
		break;
	case SHOW:
		break;
	case RECOVERY:
		break;
	}
	
}

bool InspectObject::TryActivate()
{
	if (action == READY_TO_SHOW )//&& PlayerDist() < entranceRadius)
	{
		ShowTutorial();
		return true;
	}

	return false;
}

bool InspectObject::TryDeactivate()
{
	/*if (action == SHOW && PlayerDist() > exitRadius)
	{
		HideTutorial();
		return true;
	}*/

	return false;
}

void InspectObject::UpdateSprite()
{
	int tile = 0;
	IntRect ir;
	switch (action)
	{
	case NEUTRAL:
		tile = 0;//frame / animFactor[NEUTRAL];
		ir = ts->GetSubRect(tile);
		break;
	case SHOW:
		tile = 1;
		ir = ts->GetSubRect(tile);
		break;
	}

	sprite.setTextureRect(ir);

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void InspectObject::EnemyDraw(sf::RenderTarget *target)
{

	target->draw(sprite);

	if (action == SHOW)
	{
		//	target->draw(tutorialSpr);
	}
}

void InspectObject::DrawMinimap(sf::RenderTarget *target)
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

int InspectObject::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void InspectObject::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void InspectObject::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}

