#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_KeyFly.h"

using namespace std;
using namespace sf;

KeyFly::KeyFly(ActorParams *ap)
	:Enemy(EnemyType::EN_KEYFLY, ap)
{
	enemyDrawLayer = ENEMYDRAWLAYER_KEY;

	SetNumActions(S_Count);
	SetEditorActions(S_NEUTRAL, S_NEUTRAL, 0);

	SetLevel(ap->GetLevel());

	hasMonitor = true;
	SetKey();

	ts = sess->ts_key;
	//ts = GetSizedTileset("Enemies/General/healthfly_64x64.png");
	//ts = GetSizedTileset("FX/key_128x128.png");
	ts_container = GetSizedTileset("Enemies/General/key_container_256x256.png");

	BasicCircleHurtBodySetup(80, 0, V2d( 0, 10 ), V2d());

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	containerSpr.setTexture(*ts_container->texture);
	containerSpr.setScale(scale, scale);

	//actionLength[S_NEUTRAL] = 5;
	//animFactor[S_NEUTRAL] = 5;
	actionLength[S_NEUTRAL] = 16;
	animFactor[S_NEUTRAL] = 3;

	

	//cutObject->Setup(ts, 0, 1, scale, 0, false, false);

	ResetEnemy();
}

void KeyFly::SetLevel(int lev)
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
		scale = 4.0;
		maxHealth += 5;
		break;
	}
}

void KeyFly::ResetEnemy()
{
	action = S_NEUTRAL;
	frame = 0;

	if (sess->currWorldDependentTilesetWorldIndex == 5)
	{
		animFactor[S_NEUTRAL] = 6;
	}
	else
	{
		animFactor[S_NEUTRAL] = 3;
	}
	

	hasMonitor = true;
	suppressMonitor = false;

	SetHurtboxes(&hurtBody, 0);

	UpdateHitboxes();
	UpdateSprite();
}



void KeyFly::ProcessState()
{
	//cout << "process state" << endl;
	V2d playerPos = sess->GetPlayerPos(0);

	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case S_NEUTRAL:
			frame = 0;
			break;
		}
		
	}
}

//void KeyFly::ProcessHit()
//{
//	if (!dead && ReceivedHit() && numHealth > 0)
//	{
//		--numHealth;
//
//		if (numHealth == 0)
//		{
//			action = S_EXPLODING;
//			frame = 0;
//			SetHurtboxes(NULL, 0);
//			ConfirmKill();
//			dead = false;
//
//			if (receivedHit->hType == HitboxInfo::COMBO)
//			{
//				comboHitEnemy->ComboKill(this);
//			}
//		}
//		else
//		{
//			sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
//			ConfirmHitNoKill();
//		}
//	}
//}

void KeyFly::HandleNoHealth()
{
	sess->ActivateEffect(DrawLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_container, GetPosition(), false, 0, 10, 3, true);
}

void KeyFly::PlayDeathSound()
{
	PlayKeyDeathSound();
}

void KeyFly::UpdateSprite()
{
	ts = sess->ts_key;

	sess->ts_key->SetSpriteTexture(sprite);

	//cout << "update sprite " << frame << endl;
	sprite.setTextureRect(ts->GetSubRect(frame / animFactor[S_NEUTRAL]));

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());

	int containerTile = 0;
	
	containerSpr.setTextureRect(ts_container->GetSubRect(containerTile));

	containerSpr.setOrigin(containerSpr.getLocalBounds().width / 2, containerSpr.getLocalBounds().height / 2);
	containerSpr.setPosition(GetPositionF());
}

void KeyFly::EnemyDraw(sf::RenderTarget *target)
{
	sess->ts_key->SetSpriteTexture(sprite);

	//DrawSprite(target, sprite);

	//DrawSprite(target, containerSpr);

	bool drawHurtShader = (pauseFrames >= 2 && !pauseFramesFromAttacking) && currShield == NULL;
	if (hasMonitor && !suppressMonitor)
	{
		if (drawHurtShader)
		{
			target->draw(containerSpr, &hurtShader);
		}
		else
		{
			target->draw(containerSpr);
		}
		target->draw(keySprite);
	}
	else
	{
		if (drawHurtShader)
		{
			target->draw(containerSpr, &hurtShader);
		}
		else
		{
			target->draw(containerSpr);
		}
	}

	DrawSprite(target, sprite);

	//target->draw(containerSpr);
}

int KeyFly::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void KeyFly::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void KeyFly::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}