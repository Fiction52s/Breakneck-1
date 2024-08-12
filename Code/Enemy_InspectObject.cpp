#include "Enemy_InspectObject.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "InspectSequence.h"
#include "MainMenu.h"


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
	SetEditorActions(A_NEUTRAL, A_NEUTRAL, 0);


	currControllerType = -1;
	ClearRect(buttonQuad);

	SetLevel(ap->GetLevel());

	SetCurrPosInfo(startPosInfo);


	SetLevel(ap->GetLevel());

	const string &typeName = ap->GetTypeName();

	ts_inspect = GetSizedTileset("Story/kin_family_1109x1060.png");

	if (typeName == "familypicture")
	{
	}

	ts = GetSizedTileset("Enemies/General/tutorial_flower_256x256.png");

	inspectSeq = new InspectSequence;

	inspectSeq->ts_inspect = ts_inspect;
	inspectSeq->myInspectObject = this;

	inspectSeq->Init();

	sprite.setTexture(*ts_inspect->texture);

	sprite.setScale(.1, .1);
	//sprite.setColor(Color::Red);

	entranceRadius = 100;
	exitRadius = 100;
	//double radius = 500;
	//BasicCircleHitBodySetup(radius);

	actionLength[A_NEUTRAL] = 6;
	actionLength[A_SHOW_ICON] = 8;
	actionLength[A_SHOW_INSPECTABLE] = 8;
	actionLength[A_RECOVERY] = 30;

	animFactor[A_NEUTRAL] = 3;
	animFactor[A_SHOW_ICON] = 3;
	animFactor[A_SHOW_INSPECTABLE] = 1;
	animFactor[A_RECOVERY] = 1;

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
	action = A_NEUTRAL;
	frame = 0;

	float buttonSize = 48;

	ClearRect(buttonQuad);
	SetRectCenter(buttonQuad, buttonSize, buttonSize,
		GetPositionF() + Vector2f( 0, -100 ));

	//SetHitboxes(&hitBody, 0);
	UpdateHitboxes();

	//sprite.setTexture(*ts->texture);
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

void InspectObject::ShowInspectable()
{
	assert(action == A_SHOW_ICON);

	action = A_SHOW_INSPECTABLE;
	frame = 0;
	//sess->cam.Ease(GetPositionF() + Vector2f( 0, -350 ), 2.0, 60);
	inspectSeq->Reset();
	sess->SetActiveSequence(inspectSeq);
}

void InspectObject::HideInspectable()
{
	assert(action == A_SHOW_INSPECTABLE);
	//sess->cam.EaseOutOfManual(60);
	action = A_RECOVERY;
	frame = 0;
	//sess->SetActiveSequence(NULL);
}

void InspectObject::ShowIcon()
{
	assert(action == A_NEUTRAL);
	action = A_SHOW_ICON;
	frame = 0;
}

void InspectObject::HideIcon()
{
	//if( action == A_RECOVERY )
	assert(action == A_SHOW_ICON);
	action = A_NEUTRAL;
	frame = 0;
}


bool InspectObject::IsShowingIcon()
{
	return action == A_SHOW_ICON;
}

bool InspectObject::IsShowingInspectable()
{
	return action == A_SHOW_INSPECTABLE;
}

void InspectObject::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case A_NEUTRAL:
		{
			frame = 0;
			break;
		}
		case A_SHOW_ICON:
		{
			frame = 0;
			break;
		}
		case A_SHOW_INSPECTABLE:
		{
			frame = 0;
			break;
		}
		case A_RECOVERY:
		{
			action = A_NEUTRAL;
			frame = 0;
			break;
		}
		}
	}

	UpdateButtonIconsWhenControllerIsChanged();
	/*double dist = PlayerDist();
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
	}*/
	
}

bool InspectObject::TryActivate()
{
	if (action == A_NEUTRAL && PlayerDist() < entranceRadius)
	{
		ShowIcon();
		return true;
	}

	return false;
}

bool InspectObject::TryDeactivate()
{
	if (action == A_SHOW_ICON && PlayerDist() > exitRadius)
	{
		HideIcon();
		return true;
	}

	return false;
}

bool InspectObject::IsUsed()
{
	return action == A_RECOVERY || action == A_NEUTRAL;
}

void InspectObject::UpdateButtonIconsWhenControllerIsChanged()
{
	if (sess == NULL)
		return;

	int sessControllerType = sess->controllerStates[0]->GetControllerType();

	currControllerType = sessControllerType;


	auto button = XBOX_A;
	IntRect ir = sess->mainMenu->GetButtonIconTileForMenu(currControllerType, button);
	SetRectSubRect(buttonQuad, ir);

	//ControllerSettings::ButtonType bType = ControllerSettings::ButtonType::BUTTONTYPE_JUMP;
	//SetRectSubRect(buttonQuad, sess->GetButtonIconTile(0, bType));
}

void InspectObject::UpdateSprite()
{
	int tile = 0;
	IntRect ir;

	//sprite.setColor(Color::White);

	
	//switch (action)
	//{
	//case A_NEUTRAL:
	//	tile = 0;//frame / animFactor[NEUTRAL];
	//	ir = ts->GetSubRect(tile);
	//	
	//	break;
	//case A_SHOW_ICON:
	//	tile = 1;
	//	ir = ts->GetSubRect(tile);
	//	sprite.setColor(Color::Green);
	//	break;
	//case A_SHOW_INSPECTABLE:

	//	tile = 1;
	//	ir = ts->GetSubRect(tile);
	//	sprite.setColor(Color::Blue);
	//	break;
	//}

	//sprite.setTextureRect(ir);
	sprite.setTextureRect(ts_inspect->GetSubRect(0));

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void InspectObject::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);

	if (action == A_SHOW_ICON)
	{
		target->draw(buttonQuad, 4, sf::Quads, sess->GetButtonIconTileset(0)->texture);
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

