#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_LaserWolf.h"
#include "Actor.h"
#include "Shield.h"

using namespace std;
using namespace sf;

LaserWolf::LaserWolf(ActorParams *ap)
	:Enemy(EnemyType::EN_LASERWOLF, ap),
	laserPool( 3 )
{
	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	SetEditorActions(IDLE, IDLE, 0);

	actionLength[IDLE] = 1;
	actionLength[ATTACK] = 8;
	actionLength[RECOVER] = 60;

	animFactor[IDLE] = 1;
	animFactor[ATTACK] = 3;
	animFactor[RECOVER] = 1;

	/*SetNumLaunchers(1);
	launchers[0] = new Launcher(this, BasicBullet::TURTLE, 16, 1, GetPosition(), V2d(1, 0), 0, 180, false);
	launchers[0]->SetBulletSpeed(10);
	launchers[0]->hitboxInfo->hType = HitboxInfo::ORANGE;
	launchers[0]->Reset();

	ts_bulletExplode = GetSizedTileset("FX/bullet_explode3_64x64.png");*/

	//shield = new Shield(Shield::ShieldType::T_BLOCK, 16 * scale, 3, this);
	//RegisterShield(shield);

	accel = 2.0;//3.0;//1.0;

	maxSpeed = 30;//20;

	ts = GetSizedTileset("Enemies/W6/chess_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(10);
	cutObject->SetSubRectBack(11);
	cutObject->SetScale(scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	hitboxInfo->hType = HitboxInfo::MAGENTA;

	BasicCircleHitBodySetup(32);
	BasicCircleHurtBodySetup(32);

	hitBody.hitboxInfo = hitboxInfo;

	ResetEnemy();
}

void LaserWolf::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
	//cutObject->SetCutRootPos(Vector2f(position));
}

LaserWolf::~LaserWolf()
{
	currShield = NULL;
	//delete shield;
}

void LaserWolf::SetLevel(int lev)
{
	level = lev;

	switch (level)
	{
	case 1:
		scale = 2.0;
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

void LaserWolf::ResetEnemy()
{
	action = IDLE;
	frame = 0;

	laserPool.Reset();

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

	data.fireCounter = 0;

	data.velocity = V2d();

	UpdateHitboxes();

	UpdateSprite();

	//shield->Reset();
	//shield->SetPosition(GetPosition());
	//currShield = shield;
}

void LaserWolf::ActionEnded()
{
	V2d playerPos = sess->GetPlayerPos();

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case IDLE:
			break;
		case ATTACK:
			action = RECOVER;
			frame = 0;
			break;
		case RECOVER:
			if (PlayerDist() > DEFAULT_IGNORE_RADIUS)
			{
				action = IDLE;
				frame = 0;
				//StartRush();
			}
			else
			{
				action = ATTACK;
				frame = 0;
			}
			break;
		}
	}
}

void LaserWolf::BulletHitTerrain(BasicBullet *b, Edge *edge, V2d &pos)
{
	b->launcher->DeactivateBullet(b);
}

void LaserWolf::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}
}

void LaserWolf::AddToGame()
{
	Enemy::AddToGame();

	laserPool.SetEnemyIDsAndAddToGame();
}

void LaserWolf::DirectKill()
{
	//deactivate lasers?
	Enemy::DirectKill();
}

void LaserWolf::ProcessState()
{
	ActionEnded();





	double dist = PlayerDist();
	V2d dir = PlayerDir();

	

	switch (action)
	{
	case IDLE:
	{
		if (dist < DEFAULT_DETECT_RADIUS)
		{
			action = ATTACK;
			frame = 0;
			//StartRush();
		}
		break;
	}
	
	
	}

	switch (action)
	{
	case IDLE:
		data.velocity = V2d(0, 0);
		break;
	case ATTACK:
	{
		if (frame == 15 && slowCounter == 1)
		{
			V2d offset = V2d(13, -32);
			if (!facingRight)
			{
				offset.x = -offset.x;
			}

			V2d shootDir = PlayerDir(offset, V2d() );
			laserPool.Throw(SkeletonLaser::LaserType::LT_ENEMY_WOLF, GetPosition() + offset, shootDir);
			//	shootDir);
		}

		if (dist < DEFAULT_DETECT_RADIUS)
		{
			data.velocity = PlayerDir() * 2.0;
		}
		else if (dist > DEFAULT_IGNORE_RADIUS)
		{
			data.velocity = V2d(0, 0);
		}

		if (dir.x < 0)
		{
			facingRight = false;
		}
		else
		{
			facingRight = true;
		}
		break;
	}
	case RECOVER:
	{
		if (dist < DEFAULT_DETECT_RADIUS)
		{
			data.velocity = PlayerDir() * 2.0;
		}
		else if (dist > DEFAULT_IGNORE_RADIUS)
		{
			data.velocity = V2d(0, 0);
		}

		if (dir.x < 0)
		{
			facingRight = false;
		}
		else
		{
			facingRight = true;
		}
		break;
	}
	}
}

void LaserWolf::FrameIncrement()
{
	/*if (action == CHASE && PlayerDist() < DEFAULT_DETECT_RADIUS)
	{
		++data.fireCounter;
		if (data.fireCounter == 60)
		{
			data.fireCounter = 0;
		}
	}*/
}

void LaserWolf::UpdateEnemyPhysics()
{
	//if (action == RUSH || action == CHASE)
	{
		V2d movementVec = data.velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
		//shield->SetPosition(GetPosition());
	}
}

void LaserWolf::UpdateSprite()
{
	int trueFrame;
	int tile = 0;
	switch (action)
	{
	case IDLE:
		tile = 0;
		//sprite.setColor(Color::White);
		break;
	case ATTACK:
		tile = frame / animFactor[ATTACK] + 1;
		break;
	case RECOVER:
		tile = 9;
		break;
	}

	ts->SetSubRect(sprite, tile, !facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void LaserWolf::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

int LaserWolf::GetNumStoredBytes()
{
	return sizeof(MyData);// +shield->GetNumStoredBytes();
}

void LaserWolf::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	//shield->StoreBytes(bytes);
	//bytes += shield->GetNumStoredBytes();
}

void LaserWolf::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	//shield->SetFromBytes(bytes);
	//bytes += shield->GetNumStoredBytes();
}