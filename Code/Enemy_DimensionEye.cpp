#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_DimensionEye.h"
#include "Actor.h"

#include "PauseMenu.h"

using namespace std;
using namespace sf;

void DimensionEye::Setup()
{
	SetSpawnRect();

	GameSession *game = GameSession::GetSession();
	if (game != NULL )
	{
		assert(myBonus == NULL);
		myBonus = game->CreateBonus("Enemies/Bosses/greyw1");
	}
	else
	{
		myBonus = NULL;
	}
}

DimensionEye::DimensionEye(ActorParams *ap)
	:Enemy(EnemyType::EN_DIMENSIONEYE, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, 0, 0);


	actionLength[IDLE] = 1;
	animFactor[IDLE] = 1;

	actionLength[DIE] = 60;
	animFactor[DIE] = 1;
	

	ts = GetSizedTileset("Enemies/Bosses/GreySkeleton/dimensioneye_80x80.png");

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 0;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 6;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 50;
	hitboxInfo->kbDir = normalize(V2d(1, -2));
	hitboxInfo->gravMultiplier = .5;
	hitboxInfo->invincibleFrames = 15;

	BasicCircleHurtBodySetup(40);
	BasicCircleHitBodySetup(40);
	hitBody.hitboxInfo = hitboxInfo;

	myBonus = NULL;

	ResetEnemy();
}

DimensionEye::~DimensionEye()
{
	if (myBonus != NULL)
		delete myBonus;
}

void DimensionEye::ResetEnemy()
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

void DimensionEye::ProcessState()
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
		}
	}
}

void DimensionEye::UpdateEnemyPhysics()
{
}

void DimensionEye::UpdateSprite()
{
	sprite.setTexture(*ts->texture);
	ts->SetSubRect(sprite, 0, !facingRight);

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void DimensionEye::HandleHitAndSurvive()
{
	/*GameSession *game = GameSession::GetSession();

	if (game != NULL)
	{
		game->bonusGame = myBonus;
		game->activateBonus = true;
	}*/
}

void DimensionEye::ProcessHit()
{
	if (!dead && HasReceivedHit() && numHealth > 0)
	{
		GameSession *game = GameSession::GetSession();

		if (game != NULL)
		{
			game->SetBonus(myBonus, GetPosition());

			action = DIE;
			frame = 0;

			HitboxesOff();
			HurtboxesOff();
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
