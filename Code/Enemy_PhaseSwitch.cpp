#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_PhaseSwitch.h"
#include "Actor.h"

using namespace std;
using namespace sf;

PhaseSwitch::PhaseSwitch(ActorParams *ap)
	:Enemy(EnemyType::EN_PHASESWITCH, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(A_PHASE_OFF, A_PHASE_OFF, 0);

	SetLevel(ap->GetLevel());

	scale = 1.0;

	ts = GetSizedTileset("Enemies/W4/phase_switch_192x192.png");

	BasicCircleHurtBodySetup(100);

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	actionLength[A_PHASE_OFF] = 2;
	animFactor[A_PHASE_OFF] = 1;

	actionLength[A_SWITCHING_ON] = 60;
	animFactor[A_SWITCHING_ON] = 1;

	actionLength[A_PHASE_ON] = 2;
	animFactor[A_PHASE_ON] = 1;

	actionLength[A_SWITCHING_OFF] = 60;
	animFactor[A_SWITCHING_OFF] = 1;

	ResetEnemy();
}

void PhaseSwitch::SetLevel(int lev)
{
	level = lev;
}

void PhaseSwitch::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

void PhaseSwitch::ResetEnemy()
{
	action = A_PHASE_OFF;
	frame = 0;
	DefaultHurtboxesOn();

	UpdateHitboxes();
	UpdateSprite();
}

void PhaseSwitch::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);

	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case A_PHASE_OFF:
		{
			frame = 0;
			break;
		}
		case A_SWITCHING_ON:
		{
			DefaultHurtboxesOn();
			action = A_PHASE_ON;
			frame = 0;
			break;
		}
		case A_PHASE_ON:
		{
			frame = 0;
			break;
		}
		case A_SWITCHING_OFF:
		{
			DefaultHurtboxesOn();
			action = A_PHASE_OFF;
			frame = 0;
			break;
		}
		}
	}

	if (action == A_PHASE_OFF)
	{
		if (sess->phaseOn)
		{
			action = A_PHASE_ON;
			frame = 0;
		}
	}
	else if (action == A_PHASE_ON)
	{
		if (!sess->phaseOn)
		{
			action = A_PHASE_OFF;
			frame = 0;
		}
	}
}

void PhaseSwitch::UpdateSprite()
{
	int f = 0;

	switch (action)
	{
	case A_PHASE_OFF:
	{
		f = 0;
		break;
	}
	case A_SWITCHING_ON:
	{
		f = 8;
		break;
	}
	case A_PHASE_ON:
	{
		f = 12;
		break;
	}
	case A_SWITCHING_OFF:
	{
		f = 21;
		break;
	}
	}

	sprite.setTextureRect(ts->GetSubRect(f));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void PhaseSwitch::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		//sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());

		HitboxInfo::HitboxType hType;

		if (!receivedHit.IsEmpty())
		{
			hType = receivedHit.hType;
		}
		else
		{
			hType = HitboxInfo::HitboxType::NORMAL;
		}

		if (receivedHit.comboer)
		{
			pauseFrames = 6;
			Enemy *ce = sess->GetEnemyFromID(comboHitEnemyID);
			ce->ComboKill(this);
		}
		else if (hType == HitboxInfo::WIREHITRED || hType == HitboxInfo::WIREHITBLUE)
		{
			pauseFrames = 6;
		}
		else
		{
			pauseFrames = 6;
		}
		pauseBeganThisFrame = true;

		pauseFramesFromAttacking = false;

		if (!receivedHit.comboer)
		{
			sess->cam.SetRumble(1.5, 1.5, 7);
		}

		if (hasMonitor && !suppressMonitor)
		{
			ActivateDarkAbsorbParticles();
			PlayKeyDeathSound();
		}

		if (hasMonitor)
		{
			suppressMonitor = true;
		}

		if (action == A_PHASE_OFF)
		{
			action = A_SWITCHING_ON;
			frame = 0;
		}
		else if (action == A_PHASE_ON)
		{
			action = A_SWITCHING_OFF;
			frame = 0;
		}
		else
		{
			assert(0);
		}

		HurtboxesOff();

		numHealth = maxHealth;

		receivedHit.SetEmpty();

		sess->phaseOn = !sess->phaseOn;
	}
}

int PhaseSwitch::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void PhaseSwitch::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void PhaseSwitch::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}