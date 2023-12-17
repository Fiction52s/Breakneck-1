#include "Enemy_TutorialObject.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "TutorialSequence.h"


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


void TutorialObject::UpdateParamsSettings()
{
	TutorialObjectParams *top = (TutorialObjectParams*)editParams;
	
	tutorialSeq->SetText(top->tutStr);
}

TutorialObject::TutorialObject(ActorParams *ap)
	:Enemy(EnemyType::EN_TUTORIALOBJECT, ap)
{
	SetNumActions(Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	SetLevel(ap->GetLevel());

	SetCurrPosInfo(startPosInfo);

	ts = GetSizedTileset("Enemies/General/tutorial_flower_256x256.png");

	TutorialObjectParams *top = (TutorialObjectParams*)ap;
	
	tutorialSeq = new TutorialSequence;
	tutorialSeq->Init();

	sprite.setScale(scale, scale);
	//sprite.setColor(Color::Red);

	entranceRadius = 300;
	exitRadius = 600;
	//double radius = 500;
	//BasicCircleHitBodySetup(radius);

	actionLength[NEUTRAL] = 6;
	actionLength[SHOW] = 8;

	animFactor[NEUTRAL] = 3;
	animFactor[SHOW] = 3;

	UpdateParamsSettings();

	ResetEnemy();

	//spawnRect = Rect<double>(GetPosition(), V2d(entranceRadius * 2, entranceRadius * 2));
	SetSpawnRect();
}

TutorialObject::~TutorialObject()
{
	delete tutorialSeq;
}

sf::FloatRect TutorialObject::GetAABB()
{
	Vector2f posF = GetPositionF();
	return FloatRect(posF.x - entranceRadius, posF.y - entranceRadius, entranceRadius * 2,
		entranceRadius * 2);
}

void TutorialObject::ResetEnemy()
{
	action = NEUTRAL;
	frame = 0;

	//SetHitboxes(&hitBody, 0);
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

void TutorialObject::SetExtraIDsAndAddToVectors()
{
	tutorialSeq->SetIDs();
}

bool TutorialObject::ShowTutorial()
{
	if (action == NEUTRAL)
	{
		action = SHOW;
		frame = 0;
		//sess->cam.Ease(GetPositionF() + Vector2f( 0, -350 ), 2.0, 60);
		tutorialSeq->Reset();
		sess->SetActiveSequence(tutorialSeq);
		return true;
	}
	return false;
}

bool TutorialObject::IsTutorialShowable()
{
	return action == NEUTRAL;
}

bool TutorialObject::IsShowing()
{
	return action == SHOW;
}

void TutorialObject::HideTutorial()
{
	//sess->cam.EaseOutOfManual(60);
	action = NEUTRAL;
	frame = 0;
	sess->SetActiveSequence(NULL);
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

bool TutorialObject::TryActivate()
{
	if (action == NEUTRAL && PlayerDist() < entranceRadius)
	{
		ShowTutorial();
		return true;
	}

	return false;
}

bool TutorialObject::TryDeactivate()
{
	if (action == SHOW && PlayerDist() > exitRadius)
	{
		HideTutorial();
		return true;
	}

	return false;
}

void TutorialObject::UpdateSprite()
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

void TutorialObject::EnemyDraw(sf::RenderTarget *target)
{
	
	target->draw(sprite);

	if (action == SHOW)
	{
	//	target->draw(tutorialSpr);
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

int TutorialObject::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void TutorialObject::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void TutorialObject::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}

