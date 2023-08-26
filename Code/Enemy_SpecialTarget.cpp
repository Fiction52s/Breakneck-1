#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_SpecialTarget.h"
#include "Actor.h"

using namespace std;
using namespace sf;

SpecialTarget::SpecialTarget(ActorParams *ap)
	:Enemy(EnemyType::EN_SPECIALTARGET, ap)
{
	enemyDrawLayer = ENEMYDRAWLAYER_TARGET;

	SetNumActions(A_Count);
	SetEditorActions(A_IDLE, A_IDLE, 0);

	SetLevel(ap->GetLevel());

	regenOn = false;
	//scale = 4.0;

	targetType = -1;

	//ts = GetSizedTileset("Enemies/target_256x256.png");
	//ts = GetSizedTileset("Enemies/healthfly_64x64.png");

	BasicCircleHitBodySetup(80);
	BasicCircleHurtBodySetup(80);

	const string &typeName = ap->GetTypeName();
	if (typeName == "regentarget")
	{
		regenOn = true;
		targetType = TARGET_REGEN;
		//sprite.setColor(Color::Blue);
	}
	else if (typeName == "glidetarget")
	{
		targetType = TARGET_GLIDE;
		//sprite.setColor(Color::Green);
	}
	else if (typeName == "bouncetarget")
	{
		targetType = TARGET_BOUNCE;
		//sprite.setColor(Color::Red);
	}
	else if (typeName == "grindtarget")
	{
		targetType = TARGET_GRIND;
		//sprite.setColor(Color::Red);
	}
	else if (typeName == "swingtarget")
	{
		targetType = TARGET_SWING;
	}
	else if (typeName == "scorpiontarget")
	{
		targetType = TARGET_SCORPION;
	}
	else if (typeName == "homingtarget")
	{
		targetType = TARGET_HOMING;
	}
	else if (typeName == "freeflighttarget")
	{
		targetType = TARGET_FREEFLIGHT;
		sprite.setColor(Color::Magenta);
	}
	

	ts = GetSizedTileset("Enemies/target_256x256.png");

	actionLength[A_IDLE] = 5;
	animFactor[A_IDLE] = 5;

	actionLength[A_DYING] = 30;
	animFactor[A_DYING] = 1;

	assert(targetType != -1);

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	

	

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

bool SpecialTarget::IsValidTrackEnemy()
{
	return IsInteractible();
}

bool SpecialTarget::IsHomingTarget()
{
	if (targetType == TARGET_HOMING || targetType == TARGET_REGEN)
	{
		return true;
	}

	return false;

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

void SpecialTarget::UpdateSprite()
{
	switch (action)
	{
	case A_IDLE:
	{
		sprite.setTextureRect(ts->GetSubRect(0));//ts->GetSubRect(frame / animFactor[A_IDLE]));
		break;
	}
	case A_DYING:
	{
		if (targetType == TARGET_COMBOER_BLUE
			|| targetType == TARGET_COMBOER_GREEN
			|| targetType == TARGET_COMBOER_YELLOW
			|| targetType == TARGET_COMBOER_ORANGE
			|| targetType == TARGET_COMBOER_RED
			|| targetType == TARGET_COMBOER_MAGENTA)
		{
			sprite.setTextureRect(ts->GetSubRect(frame / animFactor[A_DYING] + 1));
		}
		else
		{
			sprite.setTextureRect(ts->GetSubRect(0));
		}
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

void SpecialTarget::EnemyDraw(sf::RenderTarget *target)
{
	if (action == A_WAIT_BEFORE_REGEN)
		return;

	DrawSprite(target, sprite);
}

HitboxInfo * SpecialTarget::IsHit(int pIndex)
{
	//this if for comboer mode

	/*TARGET_COMBOER_ORANGE, 
		TARGET_COMBOER_RED,
		TARGET_COMBOER_MAGENTA,
		TARGET_COMBOER_GREY,*/

	if (targetType == TARGET_COMBOER_BLUE || targetType == TARGET_COMBOER_GREEN || targetType == TARGET_COMBOER_YELLOW 
		|| targetType == TARGET_COMBOER_ORANGE || targetType == TARGET_COMBOER_RED || targetType == TARGET_COMBOER_MAGENTA
		|| targetType == TARGET_COMBOER_GREY )
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
					if (en->type == EN_BOUNCEJUGGLER || en->type == EN_BALL || en->type == EN_EXPLODINGBARREL)
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
					if (en->type == EN_RELATIVECOMBOER || en->type == EN_HUNGRYCOMBOER )
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
	else if (targetType == TARGET_BOUNCE
		|| targetType == TARGET_GLIDE
		|| targetType == TARGET_SCORPION
		|| targetType == TARGET_GRIND
		|| targetType == TARGET_SWING)
	{
		return NULL;
	}
	else if (targetType == TARGET_HOMING)
	{
		Actor *player = sess->GetPlayer(0);

		if (player->IsHomingAttackAction(player->action))
		{
			return Enemy::IsHit(pIndex);
		}
		else
		{
			return NULL;
		}

	}
	else
	{
		return Enemy::IsHit(pIndex);
	}

	return NULL;
}


void SpecialTarget::ProcessHit()
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