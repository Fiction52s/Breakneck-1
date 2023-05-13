#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_SpecialTarget.h"

using namespace std;
using namespace sf;

SpecialTarget::SpecialTarget(ActorParams *ap)
	:Enemy(EnemyType::EN_SPECIALTARGET, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(A_IDLE, A_IDLE, 0);

	SetLevel(ap->GetLevel());

	ts = GetSizedTileset("Enemies/healthfly_64x64.png");

	BasicCircleHitBodySetup(32);
	BasicCircleHurtBodySetup(32);

	const string &typeName = ap->GetTypeName();
	if (typeName == "glidetarget")
	{
		targetType = TARGET_GLIDE;
		sprite.setColor(Color::Green);
	}
	else if (typeName == "scorpiontarget")
	{
		targetType = TARGET_SCORPION;
		sprite.setColor(Color::Yellow);
	}
	else if (typeName == "freeflighttarget")
	{
		targetType = TARGET_FREEFLIGHT;
		sprite.setColor(Color::Magenta);
	}
	else if (typeName == "bluecomboertarget")
	{
		targetType = TARGET_COMBOER_BLUE;
	}
	else if (typeName == "greencomboertarget")
	{
		targetType = TARGET_COMBOER_GREEN;
	}

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	actionLength[A_IDLE] = 5;
	animFactor[A_IDLE] = 5;

	actionLength[A_DYING] = 30;
	animFactor[A_DYING] = 1;

	actionLength[A_WAIT_BEFORE_REGEN] = 60;
	animFactor[A_WAIT_BEFORE_REGEN] = 1;

	actionLength[A_REGENERATING] = 15;
	animFactor[A_REGENERATING] = 1;

	ResetEnemy();
}

void SpecialTarget::SetLevel(int lev)
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

void SpecialTarget::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

void SpecialTarget::ResetEnemy()
{
	action = A_IDLE;
	frame = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();
	UpdateSprite();
}

void SpecialTarget::Collect()
{
	if (numHealth > 0)
	{
		numHealth = 0;
		sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
		ConfirmKill();
	}
}

bool SpecialTarget::IsInteractible()
{
	return action == A_IDLE && !dead;
}

//void SpecialTarget::IHitPlayer(int index)
//{
//	Actor *p = sess->GetPlayer(index);
//
//	if (p->action == Actor::SPRINGSTUNGLIDE)
//	{
//		Collect();
//	}
//}

void SpecialTarget::ProcessState()
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
		}
		}
		
	}
}

void SpecialTarget::UpdateSprite()
{
	sprite.setTextureRect(ts->GetSubRect(frame / animFactor[A_IDLE]));

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void SpecialTarget::EnemyDraw(sf::RenderTarget *target)
{
	if (action != A_WAIT_BEFORE_REGEN)
	{
		DrawSprite(target, sprite);
	}
}

HitboxInfo * SpecialTarget::IsHit(int pIndex)
{
	//this if for comboer mode

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
			case BLUE:
				if (en->type == EN_COMBOER || en->type == EN_SPLITCOMBOER)
				{
					validHit = true;
				}
				break;
			case GREEN:
				if (en->type == EN_GRAVITYJUGGLER)
				{
					validHit = true;
				}
				break;
			case YELLOW:
				if (en->type == EN_BOUNCEJUGGLER)
				{
					validHit = true;
				}
				break;
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

	return NULL;
}


void SpecialTarget::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
		ConfirmHitNoKill();

		action = A_DYING;
		frame = 0;
		SetHitboxes(NULL, 0);
		SetHurtboxes(NULL, 0);

		numHealth = maxHealth;

		V2d dir;
		receivedHit.SetEmpty();

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

int SpecialTarget::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void SpecialTarget::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void SpecialTarget::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}