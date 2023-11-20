#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_MultiplayerProgressTarget.h"
#include "Actor.h"
#include "GameMode.h"
#include "MapHeader.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

int MultiplayerProgressTarget::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void MultiplayerProgressTarget::StoreBytes(unsigned char *bytes)
{
	/*MyData md;
	memset(&md, 0, sizeof(MyData));
	md.dead = dead;
	md.receivedHit = receivedHit;
	md.pauseFrames = pauseFrames;
	md.numHealth = numHealth;
	md.prev = prev;
	md.next = next;
	md.receivedHitPlayer = receivedHitPlayer;
	memcpy(md.hitBy, hitBy, sizeof(hitBy));
	memcpy(bytes, &md, sizeof(MyData));*/
}

void MultiplayerProgressTarget::SetFromBytes(unsigned char *bytes)
{
	/*MyData md;
	memcpy(&md, bytes, sizeof(MyData));

	dead = md.dead;
	receivedHit = md.receivedHit;
	numHealth = md.numHealth;
	pauseFrames = md.pauseFrames;
	prev = md.prev;
	next = md.next;
	memcpy(hitBy, md.hitBy, sizeof(hitBy));

	receivedHitPlayer = md.receivedHitPlayer;*/
}

MultiplayerProgressTarget::MultiplayerProgressTarget(ActorParams *ap)
	:Enemy(EnemyType::EN_MULTIPLAYERPROGRESSTARGET, ap)//, false, 1, false)
{
	SetNumActions(S_Count);
	SetEditorActions(S_FLOAT, S_FLOAT, 0);

	actionLength[S_FLOAT] = 18;

	animFactor[S_FLOAT] = 2;

	SetCurrPosInfo(startPosInfo);

	facingRight = true;

	ts = GetSizedTileset("Enemies/Multiplayer/target_224x224.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);
	

	BasicCircleHurtBodySetup(48);

	ResetEnemy();
}

MultiplayerProgressTarget::~MultiplayerProgressTarget()
{
}

void MultiplayerProgressTarget::ResetEnemy()
{
	DefaultHurtboxesOn();
	action = S_FLOAT;
	frame = 0;

	UpdateHitboxes();

	UpdateSprite();

	memset(hitBy, 0, sizeof(hitBy));

	
}

void MultiplayerProgressTarget::ProcessState()
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

void MultiplayerProgressTarget::HandleNoHealth()
{
	//dead = true; //can never happen
}

HitboxInfo * MultiplayerProgressTarget::IsHit(int pIndex)
{
	if (!hitBy[pIndex])
	{
		return Enemy::IsHit(pIndex);
	}
	return NULL;
}

void MultiplayerProgressTarget::ProcessHit()
{
	int receivedHitIndex = GetReceivedHitPlayerIndex();
	if (!dead && HasReceivedHit() && numHealth > 0 && !hitBy[receivedHitIndex])
	{
		sess->PlayerConfirmEnemyNoKill(this, receivedHitIndex);
		ConfirmHitNoKill();
		hitBy[receivedHitIndex] = true;
		receivedHit.SetEmpty();

		if (sess->gameModeType == MatchParams::GAME_MODE_REACHENEMYBASE)
		{
			ReachEnemyBaseMode *rbm = (ReachEnemyBaseMode*)sess->gameMode;
			if (receivedHitIndex == 0)
				rbm->p0HitTargets++;
			else if (receivedHitIndex == 1)
			{
				rbm->p1HitTargets++;
			}
		}
	}
}

void MultiplayerProgressTarget::UpdateSprite()
{
	if (!hitBy[0] && !hitBy[1])
	{
		sprite.setColor(Color::Cyan);
	}
	else if (hitBy[0] && !hitBy[1])
	{
		sprite.setColor(Color::Blue);
	}
	else if (!hitBy[0] && hitBy[1])
	{
		sprite.setColor(Color::Red);
	}
	else
	{
		sprite.setColor(Color::Black);
	}
	

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