#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_DimensionExit.h"
#include "Actor.h"

#include "PauseMenu.h"

using namespace std;
using namespace sf;


DimensionExit::DimensionExit(ActorParams *ap)
	:Enemy(EnemyType::EN_DIMENSIONEXIT, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, 0, 0);


	actionLength[IDLE] = 1;
	animFactor[IDLE] = 1;


	ts = sess->GetSizedTileset("Bosses/GreySkeleton/dimensioneye_80x80.png");

	sprite.setColor(Color::Red);


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

	ResetEnemy();
}

void DimensionExit::ResetEnemy()
{
	facingRight = true;

	action = IDLE;
	frame = 0;
	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	frame = 0;

	UpdateSprite();
}

void DimensionExit::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case IDLE:
			frame = 0;
			break;
		}
	}
}

void DimensionExit::UpdateEnemyPhysics()
{
}

void DimensionExit::UpdateSprite()
{
	sprite.setTexture(*ts->texture);
	ts->SetSubRect(sprite, 0, !facingRight);

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void DimensionExit::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

void DimensionExit::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		GameSession *game = GameSession::GetSession();

		if (game != NULL)
		{
			game->ReturnFromBonus();
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

		receivedHit = NULL;
	}
}

void DimensionExit::HandleNoHealth()
{
	GameSession *game = GameSession::GetSession();
	if (game != NULL)
	{
		game->quit = true;
		game->returnVal = GameSession::GR_BONUS_RETURN;
		GameSession *parentGame = game->parentGame;
		if (parentGame != NULL)
		{
			parentGame->cam.offset = game->cam.offset;
			parentGame->cam.zoomFactor = game->cam.zoomFactor;
		}
	}
}
