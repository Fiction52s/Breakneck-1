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

	actionLength[NEUTRAL] = 4;
	actionLength[PUFF] = 1;
	actionLength[HOLDPUFF] = 1;
	actionLength[UNPUFF] = 1;
	actionLength[BLAST] = 1;
	actionLength[RECOVER] = 20;

	animFactor[NEUTRAL] = 16;//4;
	animFactor[PUFF] = 30;
	animFactor[HOLDPUFF] = 30;
	animFactor[UNPUFF] = 30;
	animFactor[BLAST] = 60;
	animFactor[RECOVER] = 1;//17 * 5;

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this, BasicBullet::TURTLE, 8, 8, GetPosition(), V2d(0, 1), 2 * PI, 90, false);
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

	BasicCircleHitBodySetup(32);
	BasicCircleHurtBodySetup(32);

	hitBody.hitboxInfo = hitboxInfo;

	ts_bulletExplode = GetSizedTileset("FX/bullet_explode3_64x64.png");

	ResetEnemy();
}

void Pufferfish::HandleNoHealth()
{
	/*if (action == HOLDPUFF)
	{
		Fire();
	}*/

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

	Enemy::DirectKill();
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
	action = NEUTRAL;
	frame = 0;

	data.velocity = V2d(0, 0);

	if (PlayerDir().x >= 0)
	{
		facingRight = true;
	}
	else
	{
		facingRight = false;
	}


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
			Fire();
			action = RECOVER;
			frame = 0;
			//action = BLAST;
			//frame = 0;
			//Fire();
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
		/*int numEnemiesKilledLastFrame = sess->GetPlayerEnemiesKilledLastFrame(0);

		if (numEnemiesKilledLastFrame > 0)
		{
			action = BLAST;
			frame = 0;
		}
		else */
	
		if(dist >= unpuffRadius)
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

	if (dist < DEFAULT_DETECT_RADIUS)
	{
		data.velocity = PlayerDir() * 1.0;
	}
	else if (dist > DEFAULT_IGNORE_RADIUS)
	{
		data.velocity = V2d(0, 0);
	}
	

	if (PlayerDir().x >= 0)
	{
		facingRight = true;
	}
	else
	{
		facingRight = false;
	}

	float rw = -1;
	switch (action)
	{
	case NEUTRAL:
		rw = 32;
		//hurtBody.GetCollisionBoxes(0).front().rw = scale * 
		break;
	case PUFF:
		rw = 48;
		break;
	case HOLDPUFF:
		rw = 64;
		break;
	case UNPUFF:
		rw = 48;
		break;
	case BLAST:
		rw = 64;
		break;
	case RECOVER:
		rw = 32;
		//if (frame < (actionLength[RECOVER] * animFactor[RECOVER]) / 2)
		//{
		//	tile = 0;
		//	//tile = 3;
		//}
		//else
		//{
		//	tile = 0;
		//}

		break;
	}
	assert(rw != -1);

	hurtBody.GetCollisionBoxes(0).front().rw = rw;
	hitBody.GetCollisionBoxes(0).front().rw = rw;
}

void Pufferfish::Fire()
{
	launchers[0]->position = GetPosition();
	launchers[0]->facingDir = V2d(1, 0);//PlayerDir();
	launchers[0]->Reset();
	launchers[0]->Fire();
}

void Pufferfish::UpdateEnemyPhysics()
{
	V2d movementVec = data.velocity;
	movementVec /= slowMultiple * (double)numPhysSteps;

	currPosInfo.position += movementVec;
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
		tile = frame / animFactor[NEUTRAL];
		break;
	case PUFF:
	{
		if (frame < 6)
		{
			tile = 4;
		}
		else if (frame < 15)
		{
			tile = 5;
		}
		else if (frame < 23)
		{
			tile = 6;
		}
		else
		{
			tile = 7;
		}
		break;
	}
	case HOLDPUFF:
		tile = 7;
		break;
	case UNPUFF:
		if (frame < 6)
		{
			tile = 7;
		}
		else if (frame < 15)
		{
			tile = 6;
		}
		else if (frame < 23)
		{
			tile = 5;
		}
		else
		{
			tile = 4;
		}
		//tile = 9;
		break;
	case BLAST:
		tile = 9;
		break;
	case RECOVER:
		if (frame < (actionLength[RECOVER] * animFactor[RECOVER]) / 2)
		{
			tile = 8;
			//tile = 3;
		}
		else
		{
			tile = 0;
		}
		
		break;
	}

	ts->SetSubRect(sprite, tile, facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void Pufferfish::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

int Pufferfish::GetNumStoredBytes()
{
	return sizeof(MyData) + launchers[0]->GetNumStoredBytes();
}

void Pufferfish::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	launchers[0]->StoreBytes(bytes);
	bytes += launchers[0]->GetNumStoredBytes();
}

void Pufferfish::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	launchers[0]->SetFromBytes(bytes);
	bytes += launchers[0]->GetNumStoredBytes();
}