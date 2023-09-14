#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_MultiplayerBase.h"
#include "MapHeader.h"
#include "Actor.h"
#include "GameMode.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

int MultiplayerBase::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void MultiplayerBase::StoreBytes(unsigned char *bytes)
{
	//MyData md;
	//memset(&md, 0, sizeof(MyData));
	//md.dead = dead;
	//md.receivedHit = receivedHit;
	//md.pauseFrames = pauseFrames;
	//md.numHealth = numHealth;
	//md.prev = prev;
	//md.next = next;
	//md.receivedHitPlayer = receivedHitPlayer;
	////memset(&bd.pad, 0, 3 * sizeof(char));
	//memcpy(bytes, &md, sizeof(MyData));
}

void MultiplayerBase::SetFromBytes(unsigned char *bytes )
{
	/*MyData md;
	memcpy(&md, bytes, sizeof(MyData));

	dead = md.dead;
	receivedHit = md.receivedHit;
	numHealth = md.numHealth;
	pauseFrames = md.pauseFrames;
	prev = md.prev;
	next = md.next;
	
	receivedHitPlayer = md.receivedHitPlayer;*/
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

	ts = GetSizedTileset("Enemies/target_224x224.png");
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

HitboxInfo *MultiplayerBase::IsHit(int pIndex)
{
	if (sess->gameModeType == MatchParams::GAME_MODE_REACHENEMYBASE)
	{
		ReachEnemyBaseMode *rbm = (ReachEnemyBaseMode*)sess->gameMode;
		if (pIndex == 0)
		{
			if (rbm->p0HitTargets == rbm->totalProgressTargets)
			{
				return Enemy::IsHit(pIndex);
			}
		}
		else if (pIndex == 1)
		{
			if (rbm->p1HitTargets == rbm->totalProgressTargets)
			{
				return Enemy::IsHit(pIndex);
			}
		}
	}

	return NULL;
}

void MultiplayerBase::HandleNoHealth()
{
	dead = true;
}

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