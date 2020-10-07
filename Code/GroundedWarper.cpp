#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "GroundedWarper.h"
#include "Actor.h"

#include "PauseMenu.h"

using namespace std;
using namespace sf;

void GroundedWarper::Setup()
{
	SetSpawnRect();

	GameSession *game = GameSession::GetSession();
	if (game != NULL )
	{
		myBonus = game->CreateBonus(bonusName);
	}
	else
	{
		myBonus = NULL;
	}
}

GroundedWarper::GroundedWarper(ActorParams *ap)
	:Enemy(EnemyType::EN_GROUNDEDWARPER, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(ACTIVE, 0, 0);

	actionLength[ACTIVE] = 1;
	animFactor[ACTIVE] = 1;

	actionLength[APPEAR] = 60;
	animFactor[APPEAR] = 1;

	actionLength[DISAPPEAR] = 60;
	animFactor[DISAPPEAR] = 1;

	actionLength[DEACTIVATED] = 1;
	animFactor[DEACTIVATED] = 1;

	GroundedWarperParams *gwParams = (GroundedWarperParams*)ap;

	startActivated = gwParams->startActivated;
	bonusName = gwParams->GetName();

	ts = sess->GetSizedTileset("Enemies/bouncefloater_128x128.png");
	sprite.setColor(Color::Red);
	/*hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 0;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 6;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 50;
	hitboxInfo->kbDir = normalize(V2d(1, -2));
	hitboxInfo->gravMultiplier = .5;
	hitboxInfo->invincibleFrames = 15;*/

	//BasicCircleHurtBodySetup(40);
	BasicCircleHitBodySetup(40);
	//hitBody.hitboxInfo = hitboxInfo;

	myBonus = NULL;

	ResetEnemy();
}

GroundedWarper::~GroundedWarper()
{
	GameSession *game = GameSession::GetSession();
	if (myBonus != NULL)
	{
		delete myBonus;
	}
	/*if (game != NULL)
	{
		if (game->bonusGame == myBonus)
		{
			game->bonusGame = NULL;
		}
		
	}*/
}

void GroundedWarper::ResetEnemy()
{
	/*if (myBonus != NULL)
	{
		myBonus->RestartLevel();
	}*/

	facingRight = true;

	if (startActivated)
	{
		Activate();
	}
	else
	{
		Deactivate();
	}
	

	UpdateSprite();
}

void GroundedWarper::Deactivate()
{
	action = DEACTIVATED;
	frame = 0;
	HitboxesOff();
}

void GroundedWarper::Activate()
{
	action = ACTIVE;
	frame = 0;
	DefaultHitboxesOn();
}

void GroundedWarper::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case ACTIVE:
			break;
		case APPEAR:
			action = ACTIVE;
			break;
		case DISAPPEAR:
			action = DEACTIVATED;
		}
	}
}

void GroundedWarper::UpdateEnemyPhysics()
{
}

void GroundedWarper::UpdateSprite()
{
	sprite.setTexture(*ts->texture);
	ts->SetSubRect(sprite, 0, !facingRight);
	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );
	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());

	if (action == ACTIVE)
	{
		sprite.setColor(Color::Green);
	}
	else
	{
		sprite.setColor(Color::Red);
	}
}

void GroundedWarper::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

void GroundedWarper::HandleHitAndSurvive()
{
	/*GameSession *game = GameSession::GetSession();

	if (game != NULL)
	{
	game->bonusGame = myBonus;
	game->activateBonus = true;
	}*/
}

bool GroundedWarper::CheckHitPlayer(int index)
{
	Actor *player = sess->GetPlayer(index);

	if (player == NULL)
		return false;

	if (currHitboxes != NULL )
	{
		Actor::HitResult hitResult = player->CheckIfImHit(currHitboxes, currHitboxFrame, HitboxInfo::HitPosType::GROUND,
			GetPosition(), IsHitFacingRight(),false,false);

		if (hitResult != Actor::HitResult::MISS)
		{
			GameSession *game = GameSession::GetSession();

			if (game != NULL)
			{
				game->SetBonus(myBonus, GetPosition());
				HitboxesOff();
			}
		}
	}


	return false;
}
