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
	SetEditorActions(A_IDLE, A_IDLE, 0);

	SetLevel(ap->GetLevel());

	scale = 1.0;

	ts = GetSizedTileset("Enemies/boosters_384x384.png");

	BasicCircleHitBodySetup(32);
	BasicCircleHurtBodySetup(32);

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	actionLength[A_IDLE] = 5;
	animFactor[A_IDLE] = 5;

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
	action = A_IDLE;
	frame = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();
	UpdateSprite();
}

void PhaseSwitch::Collect()
{
	if (numHealth > 0)
	{
		numHealth = 0;
		sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
		ConfirmKill();
	}
}

bool PhaseSwitch::IsInteractible()
{
	return action == A_IDLE && !dead;
}

//void PhaseSwitch::IHitPlayer(int index)
//{
//	Actor *p = sess->GetPlayer(index);
//
//	if (p->action == Actor::SPRINGSTUNGLIDE)
//	{
//		Collect();
//	}
//}

void PhaseSwitch::ProcessState()
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
			if (regenOn)
			{
				action = A_WAIT_BEFORE_REGEN;
				frame = 0;
			}
			else
			{
				dead = true;
				numHealth = 0;
			}
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

void PhaseSwitch::UpdateSprite()
{
	switch (action)
	{
	case A_IDLE:
	{
		sprite.setTextureRect(ts->GetSubRect(frame / animFactor[A_IDLE]));
		break;
	}
	case A_DYING:
	{
		sprite.setTextureRect(ts->GetSubRect(0));
		break;
	}
	case A_WAIT_BEFORE_REGEN:
	{
		break;
	}
	case A_REGENERATING:
	{
		sprite.setTextureRect(ts->GetSubRect(0));
		break;
	}
	}


	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void PhaseSwitch::EnemyDraw(sf::RenderTarget *target)
{
	if (action == A_WAIT_BEFORE_REGEN)
		return;

	DrawSprite(target, sprite);
}

HitboxInfo * PhaseSwitch::IsHit(int pIndex)
{
	//this if for comboer mode

	/*TARGET_COMBOER_ORANGE,
	TARGET_COMBOER_RED,
	TARGET_COMBOER_MAGENTA,
	TARGET_COMBOER_GREY,*/

	if (targetType == TARGET_COMBOER_BLUE || targetType == TARGET_COMBOER_GREEN || targetType == TARGET_COMBOER_YELLOW
		|| targetType == TARGET_COMBOER_ORANGE || targetType == TARGET_COMBOER_RED || targetType == TARGET_COMBOER_MAGENTA
		|| targetType == TARGET_COMBOER_GREY)
	{
		if (currHurtboxes == NULL)
			return NULL;

		Actor *player = sess->GetPlayer(pIndex);

		if (CanBeHitByComboer())
		{
			ComboObject *co = player->IntersectMyComboHitboxes(this, currHurtboxes, currHurtboxFrame);
			if (co != NULL)
			{
				HitboxInfo *hi = co->enemyHitboxInfo;

				Enemy *en = co->enemy;

				bool validHit = false;
				switch (targetType)
				{
				case TARGET_COMBOER_BLUE:
					if (en->type == EN_COMBOER || en->type == EN_SPLITCOMBOER)
					{
						validHit = true;
					}
					break;
				case TARGET_COMBOER_GREEN:
					if (en->type == EN_GRAVITYJUGGLER)
					{
						validHit = true;
					}
					break;
				case TARGET_COMBOER_YELLOW:
					if (en->type == EN_BOUNCEJUGGLER || en->type == EN_BALL)
					{
						validHit = true;
					}
					break;
				case TARGET_COMBOER_ORANGE:
					if (en->type == EN_GRINDJUGGLER || en->type == EN_GROUNDEDGRINDJUGGLER)
					{
						validHit = true;
					}
					break;
				case TARGET_COMBOER_RED:
					if (en->type == EN_RELATIVECOMBOER)
					{
						validHit = true;
					}
					break;
					//case TARGET_COMBOER_MAGENTA:
					//	if( en->type == EN_)
				}

				if (validHit)
				{
					co->enemy->ComboHit();
					comboHitEnemyID = co->enemy->enemyIndex;

					return hi;
				}
				else
				{
					return NULL;
				}

			}
		}
	}
	else if (targetType == TARGET_SCORPION)
	{
		return NULL;
	}
	else
	{
		return Enemy::IsHit(pIndex);
	}

	return NULL;
}


void PhaseSwitch::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		//sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
		//ConfirmHitNoKill();
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

		/*if (hasMonitor && !suppressMonitor)
		{
		sess->ActivateAbsorbParticles(AbsorbParticles::AbsorbType::DARK,
		sess->GetPlayer(0), 1, GetPosition());
		suppressMonitor = true;
		}*/

		/*switch (receivedHit.hDir)
		{
		case HitboxInfo::LEFT:
		dir = V2d(-1, 0);
		break;
		case HitboxInfo::RIGHT:
		dir = V2d(1, 0);
		break;
		case HitboxInfo::UP:
		dir = V2d(0, -1);
		break;
		case HitboxInfo::DOWN:
		dir = V2d(0, 1);
		break;
		case HitboxInfo::UPLEFT:
		dir = V2d(-1, -1);
		break;
		case HitboxInfo::UPRIGHT:
		dir = V2d(1, -1);
		break;
		case HitboxInfo::DOWNLEFT:
		dir = V2d(-1, 1);
		break;
		case HitboxInfo::DOWNRIGHT:
		dir = V2d(1, 1);
		break;
		default:
		assert(0);

		}*/
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