#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Eye.h"
#include "KeyMarker.h"
#include "Enemy_BounceFloater.h"
#include "Actor.h"

using namespace std;
using namespace sf;

void BounceFloater::SetLevel(int lev)
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

BounceFloater::BounceFloater(ActorParams *ap)
	:Enemy( EnemyType::EN_BOUNCEFLOATER, ap )
{
	SetNumActions(S_Count);
	SetEditorActions(S_FLOAT, 0, 0);

	SetLevel(ap->GetLevel());

	ts = GetSizedTileset("Enemies/boosters_384x384.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);
	//sprite.setColor(Color::Red);

	actionLength[S_FLOAT] = 18;
	actionLength[S_BOUNCE] = 10;
	actionLength[S_RECOVER] = 10;

	animFactor[S_FLOAT] = 2;
	animFactor[S_BOUNCE] = 1;
	animFactor[S_RECOVER] = 1;


	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHurtBodySetup(48);
	BasicCircleHitBodySetup(48);

	hitBody.hitboxInfo = hitboxInfo;

	ResetEnemy();
}

void BounceFloater::ResetEnemy()
{
	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	action = S_FLOAT;
	frame = 0;

	UpdateHitboxes();

	UpdateSprite();
}

void BounceFloater::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0 && action == S_FLOAT )
	{
		Actor *player = sess->GetPlayer(0);

		SetHitboxes(NULL, 0);
		SetHurtboxes(NULL, 0);

		action = S_BOUNCE;
		frame = 0;

		V2d dir = receivedHit.hDir;

		player->BounceFloaterBoost(dir);

		player->ConfirmEnemyNoKill(this);
		ConfirmHitNoKill();
	}
}

void BounceFloater::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_FLOAT:
			break;
		case S_BOUNCE:
			action = S_RECOVER;
			break;
		case S_RECOVER:
			action = S_FLOAT;
			DefaultHitboxesOn();
			DefaultHurtboxesOn();
			break;
		}
	}
}


void BounceFloater::HandleNoHealth()
{

}

void BounceFloater::UpdateEnemyPhysics()
{
}

void BounceFloater::FrameIncrement()
{
}

void BounceFloater::UpdateSprite()
{

	int tile = 5;
	sprite.setTextureRect(ts->GetSubRect(tile));

	switch (action)
	{
	case S_FLOAT:
		break;
	}

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setRotation(0);
}

void BounceFloater::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

int BounceFloater::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void BounceFloater::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void BounceFloater::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}