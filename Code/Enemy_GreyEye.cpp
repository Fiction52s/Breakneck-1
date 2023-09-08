#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_GreyEye.h"
#include "Actor.h"
#include "Enemy_GreySkeleton.h"

#include "PauseMenu.h"

using namespace std;
using namespace sf;

void GreyEye::Setup()
{
	SetSpawnRect();

	if (!isBonusEye)
	{
		GameSession *game = GameSession::GetSession();
		if (game != NULL)
		{
			//cout << "load bonus" << endl;
			myBonus = game->CreateBonus("BossTest/greyskeleton_blue", BONUSTYPE_GREY_SKELETON);
		}
		else
		{
			myBonus = NULL;
		}
	}
}

GreyEye::GreyEye( int p_eyeType, GreySkeleton *gs )
	:Enemy(EnemyType::EN_GREYEYE, NULL)
{
	greySkel = gs;
	eyeType = (EyeType)p_eyeType;

	SetNumActions(A_Count);
	SetEditorActions(IDLE, 0, 0);


	actionLength[IDLE] = 1;
	animFactor[IDLE] = 1;

	actionLength[DIE] = 60;
	animFactor[DIE] = 1;

	actionLength[WARP_RETURN] = 60;
	animFactor[WARP_RETURN] = 1;

	isBonusEye = gs->bonusType != BONUSTYPE_NONE;

	if (isBonusEye)
	{
		sprite.setColor(Color::Red);
	}

	ts = GetSizedTileset("Bosses/GreySkeleton/dimensioneye_80x80.png");

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 0;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 6;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 10;
	hitboxInfo->kbDir = normalize(V2d(1, -2));
	hitboxInfo->gravMultiplier = .5;
	hitboxInfo->invincibleFrames = 15;

	BasicCircleHurtBodySetup(40);
	BasicCircleHitBodySetup(40);
	hitBody.hitboxInfo = hitboxInfo;

	myBonus = NULL;

	ResetEnemy();
}

GreyEye::~GreyEye()
{
	if (myBonus != NULL)
		delete myBonus;
}

void GreyEye::ResetEnemy()
{
	if (myBonus != NULL)
	{
		myBonus->RestartLevel();
	}

	facingRight = true;

	action = IDLE;
	frame = 0;
	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	frame = 0;

	UpdateSprite();
}

void GreyEye::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case IDLE:
			frame = 0;
			break;
		case DIE:
			dead = true;
			numHealth = 0;
			break;
		case WARP_RETURN:
		{
			action = BASE_WORLD_IDLE;
			frame = 0;
			DefaultHitboxesOn();
			DefaultHurtboxesOn();
			break;
		}
		}
	}
}

void GreyEye::UpdateEnemyPhysics()
{
}

void GreyEye::UpdateSprite()
{
	sprite.setTexture(*ts->texture);
	ts->SetSubRect(sprite, 0, !facingRight);

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void GreyEye::HandleHitAndSurvive()
{
	/*GameSession *game = GameSession::GetSession();

	if (game != NULL)
	{
	game->bonusGame = myBonus;
	game->activateBonus = true;
	}*/
}

void GreyEye::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		GameSession *game = GameSession::GetSession();

		if (game != NULL)
		{
			if (action == BASE_WORLD_IDLE)
			{
				game->SetBonus(myBonus, GetPosition(), greySkel);
				greySkel->currWarpEye = this;
				//game->CrossFade(10, 10, 10, Color::White, true);
				//action = DIE;
				//frame = 0;

				//HitboxesOff();
				//HurtboxesOff();
			}
			else if (action == DIMENSION_WORLD_IDLE)
			{
				if (game != NULL)
				{
					game->ReturnFromBonus();
				}

				receivedHit.SetEmpty();
			}
		}
		//numHealth -= 1;

		//if (numHealth <= 0)
		//{
		//	if (hasMonitor && !suppressMonitor)
		//	{
		//		//sess->CollectKey();
		//	}

		//	sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
		//	ConfirmKill();
		//}
		//else
		//{
		//	sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
		//	ConfirmHitNoKill();
		//}

		receivedHit.SetEmpty();
	}
}


void GreyEye::Appear(V2d &pos)
{
	Reset();
	sess->AddEnemy(this);

	currPosInfo.position = pos;
	currPosInfo.ground = NULL;

	DefaultHurtboxesOn();

	if (isBonusEye)
	{
		action = DIMENSION_WORLD_IDLE;
		frame = 0;
	}
	else
	{
		action = BASE_WORLD_IDLE;
		frame = 0;
	}




	UpdateHitboxes();
}


void GreyEye::WarpReturn()
{
	action = WARP_RETURN;
	frame = 0;
	HitboxesOff();
	HurtboxesOff();
}