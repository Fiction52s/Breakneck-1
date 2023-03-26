#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_ComboerTarget.h"
#include "Actor.h"

using namespace std;
using namespace sf;

ComboerTarget::ComboerTarget(ActorParams *ap)
	:Enemy(EnemyType::EN_COMBOERTARGET, ap)
{
	SetNumActions(S_Count);
	SetEditorActions(S_NEUTRAL, S_NEUTRAL, 0);

	SetLevel(ap->GetLevel());

	ts = GetSizedTileset("Enemies/healthfly_64x64.png");

	BasicCircleHurtBodySetup(32);

	string typeName = ap->GetTypeName();

	if (typeName == "bluecomboertarget")
	{
		targetType = BLUE;
	}
	else if (typeName == "greencomboertarget")
	{
		targetType = GREEN;
	}
	else if (typeName == "yellowcomboertarget")
	{
		targetType = YELLOW;
	}
	else
	{
		targetType = BLUE;
		assert(0);
	}

	sprite.setColor(Color::Red);
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	actionLength[S_NEUTRAL] = 5;
	animFactor[S_NEUTRAL] = 5;

	cutObject->Setup(ts, 0, 1, scale, 0, false, false);

	ResetEnemy();
}

void ComboerTarget::SetLevel(int lev)
{
	level = lev;
	switch (level)
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
	}
}

void ComboerTarget::AddToWorldTrees()
{
	sess->activeItemTree->Insert(this);
}

void ComboerTarget::ResetEnemy()
{
	action = S_NEUTRAL;
	frame = 0;

	DefaultHurtboxesOn();

	UpdateHitboxes();
	UpdateSprite();
}

void ComboerTarget::Collect()
{
	if (numHealth > 0)
	{
		numHealth = 0;
		sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
		ConfirmKill();
	}
}

HitboxInfo * ComboerTarget::IsHit(int pIndex)
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
				if (en->type == EN_BOUNCEJUGGLER )
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

void ComboerTarget::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
	}
}

void ComboerTarget::UpdateSprite()
{
	sprite.setTextureRect(ts->GetSubRect(frame / animFactor[S_NEUTRAL]));

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void ComboerTarget::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}