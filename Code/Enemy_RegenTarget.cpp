#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_RegenTarget.h"
#include "Actor.h"

using namespace std;
using namespace sf;

RegenTarget::RegenTarget(ActorParams *ap)
	:Enemy(EnemyType::EN_REGENTARGET, ap)
{
	enemyDrawLayer = ENEMYDRAWLAYER_TARGET;

	SetNumActions(A_Count);
	SetEditorActions(A_IDLE, A_IDLE, 0);

	SetLevel(ap->GetLevel());

	BasicCircleHitBodySetup(80);
	BasicCircleHurtBodySetup(80);

	ts = GetSizedTileset("Enemies/W1/respawner_192x192.png");

	actionLength[A_IDLE] = 9;
	animFactor[A_IDLE] = 5;

	actionLength[A_DYING] = 5;
	animFactor[A_DYING] = 3;

	actionLength[A_WAIT_BEFORE_REGEN] = 30;
	animFactor[A_WAIT_BEFORE_REGEN] = 1;

	actionLength[A_REGENERATING] = 5;
	animFactor[A_REGENERATING] = 3;

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	ResetEnemy();
}

void RegenTarget::SetLevel(int lev)
{
	level = lev;
	/*switch (level)
	{
	case 1:
	scale = 4.0;
	break;
	case 2:
	scale = 2.0;
	maxHealth += 2;
	break;
	case 3:
	scale = 4.0;
	maxHealth += 5;
	break;
	}*/
}

void RegenTarget::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

void RegenTarget::ResetEnemy()
{
	action = A_IDLE;
	frame = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();
	UpdateSprite();
}

void RegenTarget::Collect()
{
	if (numHealth > 0)
	{
		numHealth = 0;
		sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
		ConfirmKill();
	}
}

bool RegenTarget::IsInteractible()
{
	return action == A_IDLE && !dead;
}

bool RegenTarget::IsValidTrackEnemy()
{
	return IsInteractible();
}

void RegenTarget::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);

	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case A_IDLE:
		{
			frame = 0;
			break;
		}
		case A_DYING:
		{
			action = A_WAIT_BEFORE_REGEN;
			frame = 0;
			break;
		}
		case A_WAIT_BEFORE_REGEN:
		{
			action = A_REGENERATING;
			frame = 0;
			break;
		}
		case A_REGENERATING:
		{
			action = A_IDLE;
			frame = 0;
			DefaultHitboxesOn();
			DefaultHurtboxesOn();
		}
		}

	}
}

void RegenTarget::UpdateSprite()
{
	int tile = 0;
	switch (action)
	{
	case A_IDLE:
	{
		tile = frame / animFactor[A_IDLE];
		//sprite.setTextureRect(ts->GetSubRect(0));//ts->GetSubRect(frame / animFactor[A_IDLE]));
		break;
	}
	case A_DYING:
	{
		tile = frame / animFactor[A_DYING] + 10;
		//sprite.setTextureRect(ts->GetSubRect(0));
		break;
	}
	case A_WAIT_BEFORE_REGEN:
	{
		tile = 0;
		break;
	}
	case A_REGENERATING:
	{
		tile = (actionLength[A_REGENERATING]-1) - (frame / animFactor[A_REGENERATING]) + 10;
		break;
	}
	}


	ts->SetSubRect(sprite, tile);
	//sprite.setScale(3, 3);
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void RegenTarget::EnemyDraw(sf::RenderTarget *target)
{
	if (action == A_WAIT_BEFORE_REGEN)
		return;

	DrawSprite(target, sprite);
}


void RegenTarget::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
		ConfirmKill();
		if (hasMonitor)
		{
			suppressMonitor = true;
		}
		dead = false;

		action = A_DYING;
		frame = 0;
		HitboxesOff();
		HurtboxesOff();

		numHealth = maxHealth;

		receivedHit.SetEmpty();
	}
}

int RegenTarget::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void RegenTarget::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void RegenTarget::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}