#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Pufferfish.h"
#include "Actor.h"

using namespace std;
using namespace sf;

Pufferfish::Pufferfish(ActorParams *ap)
	:Enemy(EnemyType::EN_PUFFERFISH, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	bulletSpeed = 5;

	actionLength[NEUTRAL] = 1;
	actionLength[PUFF] = 1;
	actionLength[HOLDPUFF] = 1;
	actionLength[UNPUFF] = 1;
	actionLength[BLAST] = 1;
	actionLength[RECOVER] = 1;

	animFactor[NEUTRAL] = 2;
	animFactor[PUFF] = 30;
	animFactor[HOLDPUFF] = 30;
	animFactor[UNPUFF] = 30;
	animFactor[BLAST] = 60;
	animFactor[RECOVER] = 17 * 5;

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this, BasicBullet::TURTLE, 24, 24, GetPosition(), V2d(1, 0), 2 * PI, 90, false);
	launchers[0]->SetBulletSpeed(bulletSpeed);
	launchers[0]->hitboxInfo->hType = HitboxInfo::ORANGE;
	launchers[0]->Reset();

	ts = GetSizedTileset("Enemies/W4/puffer_256x256.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	cutObject->SetTileset(ts);
	
	cutObject->SetScale(scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 3;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	hitboxInfo->hType = HitboxInfo::ORANGE;

	puffRadius = 800;
	unpuffRadius = 1000;

	BasicCircleHitBodySetup(16);
	BasicCircleHurtBodySetup(16);

	hitBody.hitboxInfo = hitboxInfo;

	ts_bulletExplode = GetSizedTileset("FX/bullet_explode3_64x64.png");

	ResetEnemy();
}

void Pufferfish::HandleNoHealth()
{
	if (action == HOLDPUFF)
	{
		Fire();
	}

	if (action == HOLDPUFF)
	{
		cutObject->SetSubRectFront(5);
		cutObject->SetSubRectBack(4);
	}
	else
	{
		cutObject->SetSubRectFront(7);
		cutObject->SetSubRectBack(6);
	}
	cutObject->SetFlipHoriz(facingRight);
	//cutObject->SetCutRootPos(Vector2f(position));
}

void Pufferfish::SetLevel(int lev)
{
	level = lev;

	switch (level)
	{
	case 1:
		scale = 1.0;
		break;
	case 2:
		scale = 2.0;
		maxHealth += 2;
		break;
	case 3:
		scale = 3.0;
		maxHealth += 5;
		break;
	}
}

void Pufferfish::DirectKill()
{
	BasicBullet *b = launchers[0]->activeBullets;
	while (b != NULL)
	{
		BasicBullet *next = b->next;
		double angle = atan2(b->velocity.y, -b->velocity.x);
		sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
		b->launcher->DeactivateBullet(b);

		b = next;
	}

	receivedHit = NULL;
}

void Pufferfish::BulletHitTerrain(BasicBullet *b, Edge *edge, V2d &pos)
{
	b->launcher->DeactivateBullet(b);
}

void Pufferfish::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}
}

void Pufferfish::ResetEnemy()
{
	if (GetPosition().x < sess->playerOrigPos[0].x)
		facingRight = false;
	else
		facingRight = true;

	action = NEUTRAL;
	frame = 0;



	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

void Pufferfish::ActionEnded()
{
	int blah = actionLength[action] * animFactor[action];
	V2d playerPos = sess->GetPlayerPos();

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case NEUTRAL:
			break;
		case PUFF:
			action = HOLDPUFF;
			break;
		case HOLDPUFF:
			break;
		case UNPUFF:
			action = NEUTRAL;
			break;
		case BLAST:
			action = RECOVER;
			break;
		case RECOVER:
			action = NEUTRAL;
			break;
		}
	}
}

void Pufferfish::ProcessState()
{
	ActionEnded();

	double dist = PlayerDist();
	switch (action)
	{
	case NEUTRAL:
		if (dist < puffRadius)
		{
			action = PUFF;
			frame = 0;
		}
		break;
	case PUFF:
		
		break;
	case HOLDPUFF:
	{
		int numEnemiesKilledLastFrame = sess->GetPlayerEnemiesKilledLastFrame(0);

		if (numEnemiesKilledLastFrame > 0)
		{
			action = BLAST;
			frame = 0;
		}
		else if (dist >= unpuffRadius)
		{
			action = UNPUFF;
			frame = 0;
		}
		break;
	}
	case UNPUFF:
		break;
	case BLAST:
		break;
	case RECOVER:
		break;
	}

	switch (action)
	{
	case NEUTRAL:
		break;
	case PUFF:
		break;
	case HOLDPUFF:
		break;
	case UNPUFF:
		break;
	case BLAST:
		break;
	case RECOVER:
		break;
	}

	if (action == BLAST && frame == 1 && slowCounter == 1)// frame == 0 && slowCounter == 1 )
	{
		Fire();
	}
}

void Pufferfish::Fire()
{
	launchers[0]->position = GetPosition();
	launchers[0]->facingDir = PlayerDir();
	launchers[0]->Reset();
	launchers[0]->Fire();
}

void Pufferfish::UpdateEnemyPhysics()
{
}

void Pufferfish::UpdateSprite()
{

	/*switch (action)
	{
	case NEUTRAL:
		sprite.setScale(scale, scale);
		break;
	case PUFF:
		break;
	case HOLDPUFF:
		sprite.setScale(scale * 2.0, scale * 2.0);
		break;
	case UNPUFF:
		break;
	case BLAST:
		break;
	case RECOVER:
		sprite.setScale(scale * .8, scale * .8);
		break;
	}*/

	int tile = 0;
	switch (action)
	{
	case NEUTRAL:
		tile = 0;
		break;
	case PUFF:
		tile = 1;
		break;
	case HOLDPUFF:
		tile = 2;
		break;
	case UNPUFF:
		tile = 3;
		break;
	case BLAST:
		tile = 2;
		break;
	case RECOVER:
		tile = 3;
		break;
	}

	ts->SetSubRect(sprite, tile, !facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void Pufferfish::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}