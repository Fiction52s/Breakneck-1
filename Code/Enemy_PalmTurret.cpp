#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_PalmTurret.h"
#include "Shield.h"
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



PalmTurret::PalmTurret(ActorParams *ap)
	:Enemy(EnemyType::EN_PALMTURRET, ap)
{
	RegisterCollisionBody(laserBody);

	SetNumActions(Count);
	SetEditorActions(IDLE, 0, 0);

	actionLength[IDLE] = 6;
	actionLength[CHARGE] = 3;//60
	actionLength[FIRE] = 11;//30
	actionLength[RECOVER] = 14;//60

	animFactor[IDLE] = 4;
	animFactor[CHARGE] = 20;
	animFactor[FIRE] = 3;
	animFactor[RECOVER] = 5;

	SetLevel(ap->GetLevel());

	finalLaserWidth = 100;
	laserLength = 3000;

	ts = GetSizedTileset("Enemies/W4/palm_256x256.png");

	double width = ts->tileWidth;
	double height = ts->tileHeight;

	width *= scale;
	height *= scale;

	SetOffGroundHeight(height / 2.f - 56);

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	

	

	//shield = new Shield(Shield::ShieldType::T_BLOCK, 80 * scale, 3, this);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 3;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;
	hitboxInfo->hType = HitboxInfo::ORANGE;

	laserHitboxInfo = new HitboxInfo;
	laserHitboxInfo->damage = 180;//do more damage than hitting the turret itself
	laserHitboxInfo->drainX = 0;
	laserHitboxInfo->drainY = 0;
	laserHitboxInfo->hitlagFrames = 0;
	laserHitboxInfo->hitstunFrames = 40;
	laserHitboxInfo->knockback = 15;
	laserHitboxInfo->hType = HitboxInfo::ORANGE;


	BasicCircleHurtBodySetup(32);
	BasicCircleHitBodySetup(32);
	hitBody.hitboxInfo = hitboxInfo;

	laserBody.BasicRectSetup( laserLength/2, finalLaserWidth/2, 0, V2d());
	laserBody.hitboxInfo = laserHitboxInfo;

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(35);
	cutObject->SetSubRectBack(34);
	cutObject->SetScale(scale);
	cutObject->rotateAngle = sprite.getRotation();

	UpdateOnPlacement(ap);


	ResetEnemy();
}

PalmTurret::~PalmTurret()
{
	delete laserHitboxInfo;
}

void PalmTurret::ResetEnemy()
{
	action = IDLE;
	frame = 0;
	DefaultHurtboxesOn();
	DefaultHitboxesOn();

	UpdateHitboxes();
	UpdateSprite();
}

void PalmTurret::SetLevel(int lev)
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

void PalmTurret::StartCharge()
{
	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();
	action = CHARGE;
	frame = 0;

	data.currLaserWidth = 10;

	V2d laserAnchor = position + currPosInfo.GetEdge()->Normal() * 40.0;

	V2d laserDir = normalize(playerPos - laserAnchor);
	laserAngle = GetVectorAngleCW(laserDir);
	laserCenter = laserAnchor + laserDir * laserLength/2.0;
	
	laserBody.SetBasicPos(0, laserCenter, laserAngle);

	UpdateLaserWidth(data.currLaserWidth);

	SetRectColor(laserQuad, Color::White);
}

bool PalmTurret::CheckHitPlayer(int index)
{
	if (action == FIRE)
	{
		BasicCheckHitPlayer(&laserBody, index);
	}

	return BasicCheckHitPlayer(currHitboxes, index);
}

void PalmTurret::UpdateLaserWidth(double w)
{
	data.currLaserWidth = w;
	SetRectRotation(laserQuad, laserAngle, laserLength, w, Vector2f(laserCenter));
}

void PalmTurret::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case IDLE:
		{
			break;
		}
		case CHARGE:
		{
			action = FIRE;
			UpdateLaserWidth(finalLaserWidth);
			SetRectColor(laserQuad, Color::Red);
			break;
		}
		case FIRE:
		{
			action = RECOVER;
			break;
		}
		case RECOVER:
		{
			StartCharge();
			break;
		}
		}
	}
}

void PalmTurret::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();

	ActionEnded();
	
	//not implemented
	//ignoreRadius = 3000;

	switch (action)
	{
	case IDLE:
	{
		if (length(playerPos - position) < DEFAULT_DETECT_RADIUS + 1200)
		{
			StartCharge();
		}
		break;
	}
	case CHARGE:
	{
		break;
	}
	case FIRE:
	{
		break;
	}
	case RECOVER:
	{
		break;
	}

	}

	switch (action)
	{
	case IDLE:
	{
		
		break;
	}
	case CHARGE:
	{	
		data.currLaserWidth += 3.0;
		if (data.currLaserWidth > finalLaserWidth)
		{
			data.currLaserWidth = finalLaserWidth;
		}
		UpdateLaserWidth(data.currLaserWidth);
		break;
	}
	case FIRE:
	{
		
		break;
	}
	case RECOVER:
	{
		break;
	}
		
	}

}

void PalmTurret::EnemyDraw(sf::RenderTarget *target)
{
	if (action == CHARGE || action == FIRE)
	{
		target->draw(laserQuad, 4, sf::Quads);
	}

	DrawSprite(target, sprite);

	
}

void PalmTurret::UpdateSprite()
{
	/*IDLE,
		CHARGE,
		FIRE,
		RECOVER,*/

	int tile = 0;
	switch (action)
	{
	case IDLE:
	{
		tile = frame / animFactor[IDLE];
		break;
	}
	case CHARGE:
	{
		tile = frame / animFactor[CHARGE] + 6;
		break;
	}
	case FIRE:
	{
		tile = frame / animFactor[FIRE] + 9;
		break;
	}
	case RECOVER:
	{
		tile = frame / animFactor[RECOVER] + 20;
		break;
	}
	}

	sprite.setTextureRect(ts->GetSubRect(tile));

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
}

int PalmTurret::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void PalmTurret::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void PalmTurret::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}