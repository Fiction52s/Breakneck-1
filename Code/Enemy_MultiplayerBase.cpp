#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_MultiplayerBase.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

int MultiplayerBase::GetNumStoredBytes()
{
	return sizeof(BaseData);
}

void MultiplayerBase::StoreBytes(unsigned char *bytes)
{
	BaseData bd;
	memset(&bd, 0, sizeof(BaseData));
	bd.dead = dead;
	bd.receivedHit = receivedHit;
	bd.pauseFrames = pauseFrames;
	bd.numHealth = numHealth;
	bd.prev = prev;
	bd.next = next;
	bd.receivedHitPlayer = receivedHitPlayer;
	//memset(&bd.pad, 0, 3 * sizeof(char));
	memcpy(bytes, &bd, sizeof( BaseData ));
}

void MultiplayerBase::SetFromBuffer(unsigned char *buf)
{
	BaseData bd;
	memcpy(&bd, buf, sizeof(BaseData));

	dead = bd.dead;
	receivedHit = bd.receivedHit;
	numHealth = bd.numHealth;
	pauseFrames = bd.pauseFrames;
	prev = bd.prev;
	next = bd.next;
	
	receivedHitPlayer = bd.receivedHitPlayer;
}

MultiplayerBase::MultiplayerBase(ActorParams *ap)
	:Enemy(EnemyType::EN_MULTIPLAYERBASE, ap)//, false, 1, false)
{
	SetNumActions(S_Count);
	SetEditorActions(S_FLOAT, S_FLOAT, 0);

	actionLength[S_FLOAT] = 18;

	animFactor[S_FLOAT] = 2;

	SetCurrPosInfo(startPosInfo);

	facingRight = true;

	ts = sess->GetTileset("Enemies/comboer_128x128.png", 128, 128);
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	BasicCircleHurtBodySetup(48);

	ResetEnemy();
}

MultiplayerBase::~MultiplayerBase()
{
}

void MultiplayerBase::ResetEnemy()
{
	DefaultHurtboxesOn();
	action = S_FLOAT;
	frame = 0;

	UpdateHitboxes();

	UpdateSprite();
}

void MultiplayerBase::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		/*switch (action)
		{
		case S_EXPLODE:
			numHealth = 0;
			dead = true;
			break;
		}*/
	}

	//V2d playerPos = owner->GetPlayer(0)->position;
}

void MultiplayerBase::HandleNoHealth()
{
	dead = true;
}

//void MultiplayerBase::ProcessHit()
//{
//	int receivedHitIndex = GetReceivedHitPlayerIndex();
//	if (!dead && ReceivedHit() && numHealth > 0 && receivedHitIndex != actorIndex )
//	{
//		numHealth -= 1;
//
//		if (numHealth <= 0)
//		{
//			if (hasMonitor && !suppressMonitor)
//			{
//				//sess->CollectKey();
//			}
//
//			sess->PlayerConfirmEnemyKill(this, receivedHitIndex);
//			ConfirmKill();
//		}
//		else
//		{
//			sess->PlayerConfirmEnemyNoKill(this, receivedHitIndex);
//			ConfirmHitNoKill();
//		}
//
//		receivedHit = NULL;
//	}
//}

void MultiplayerBase::UpdateSprite()
{
	sprite.setPosition(GetPositionF());
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setRotation(0);
	/*int tIndex = 0;
	switch (action)
	{
	case S_FLOAT:
	tIndex = 0;
	break;
	case S_SHOT:

	break;
	case S_EXPLODE:
	break;
	}
	sprite.setTextureRect(ts->GetSubRect(frame));*/
}

void MultiplayerBase::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}