#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_LobTurret.h"
#include "Shield.h"
#include "Actor.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )



LobTurret::LobTurret(ActorParams *ap)
	:Enemy(EnemyType::EN_LOBTURRET, ap)
{
	SetNumActions(Count);
	SetEditorActions(ATTACK, 0, 0);

	SetLevel(ap->GetLevel());

	framesWait = 60;
	bulletSpeed = 10;
	animationFactor = 3;
	assert(framesWait > 13 * animationFactor);

	ts = sess->GetSizedTileset("Enemies/curveturret_144x96.png");

	double width = ts->tileWidth;
	double height = ts->tileHeight;

	width *= scale;
	height *= scale;

	SetOffGroundHeight(height / 2.f /*- 30 * scale*/);

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	//shield = new Shield(Shield::ShieldType::T_BLOCK, 80 * scale, 3, this);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;

	BasicCircleHurtBodySetup(32);
	BasicCircleHitBodySetup(32);
	hitBody.hitboxInfo = hitboxInfo;

	bulletSpeed = 7;

	double size = 400;//max( width, height );


	ts_bulletExplode = sess->GetTileset("FX/bullet_explode2_64x64.png", 64, 64);

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this,
		BasicBullet::LOB_TURRET, 32, 1, GetPosition(), V2d(0, -1),
		0, 180, true);
	launchers[0]->SetBulletSpeed(bulletSpeed);
	launchers[0]->hitboxInfo->damage = 18;

	V2d gravity(0, .3);
	const string &typeName = ap->GetTypeName();
	if (typeName == "reverselobturret")
	{
		gravity.y = -gravity.y;
		sprite.setColor(Color::Blue);
	}


	launchers[0]->SetGravity(gravity);
	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(12);
	cutObject->SetSubRectBack(11);
	cutObject->SetScale(scale);
	cutObject->rotateAngle = sprite.getRotation();

	UpdateOnPlacement(ap);

	turnFactor = 500;

	lobDirs[0] = normalize(V2d(2, -1));
	lobDirs[1] = normalize(V2d(1.7, -3));
	lobDirs[2] = normalize(V2d(.3, -6));

	lobSpeeds[0] = 7;
	lobSpeeds[1] = 11;
	lobSpeeds[2] = 13;


	ResetEnemy();
}

void LobTurret::UpdateOnPlacement(ActorParams *ap)
{
	Enemy::UpdateOnPlacement(ap);

	if (startPosInfo.ground != NULL)
	{
		launchers[0]->position = startPosInfo.GetEdge()->GetRaisedPosition(startPosInfo.GetQuant(), 80.0 * (double)scale);
		launchers[0]->facingDir = startPosInfo.GetEdge()->Normal();
	}

	//testShield->SetPosition(GetPosition());
}

void LobTurret::ResetEnemy()
{
	lobTypeCounter = 0;
	action = WAIT;
	frame = 0;
	DefaultHurtboxesOn();
	DefaultHitboxesOn();
	//currShield = shield;
	//shield->Reset();

	UpdateHitboxes();
	UpdateSprite();
}

void LobTurret::SetLevel(int lev)
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

void LobTurret::Setup()
{
	Enemy::Setup();

	//launchers[0]->position = startPosInfo.GetEdge()->GetRaisedPosition(startPosInfo.GetQuant(), 80.0 * (double)scale);
	//launchers[0]->position = GetPosition();

	
	//TurretSetup();
}

void LobTurret::FireResponse(BasicBullet *b)
{
}

void LobTurret::UpdateBullet(BasicBullet *b)
{
}

void LobTurret::BulletHitTerrain(BasicBullet *b,
	Edge *edge,
	sf::Vector2<double> &pos)
{
	V2d norm = edge->Normal();
	double angle = atan2(norm.y, -norm.x);
	sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true);
	b->launcher->DeactivateBullet(b);

	//if (b->launcher->def_e == NULL)
	//	b->launcher->SetDefaultCollision(max( b->framesToLive -4, 0 ), edge, pos);
}

void LobTurret::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	V2d vel = b->velocity;
	double angle = atan2(vel.y, vel.x);
	sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);

	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}

	b->launcher->DeactivateBullet(b);
	//owner->GetPlayer( 0 )->ApplyHit( b->launcher->hitboxInfo );
}


void LobTurret::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();
	switch (action)
	{
	case WAIT:
	{
		if (length(playerPos - position) < 1000)
		{
			action = ATTACK;
			frame = 0;
		}
		break;
	}
	case ATTACK:
	{
		if (frame == 13 * animationFactor)
		{
			frame = 0;
			if (length(playerPos - position) >= 500)
			{
				action = WAIT;
				frame = 0;
			}
		}
		//else if (frame >= 4 * animationFactor && frame <= 4 * animationFactor + 10 && slowCounter == 1)
		else if (frame == 4 * animationFactor && slowCounter == 1)
		{
			V2d currLobDir = lobDirs[lobTypeCounter];
			double currLobSpeed = lobSpeeds[lobTypeCounter];
			//launchers[0]

			launchers[0]->bulletSpeed = currLobSpeed;
			if (DistFromPlayerX() < 0)
			{
				currLobDir.x = -currLobDir.x;
			}
			launchers[0]->facingDir = currLobDir;//normalize(playerPos - position);
			launchers[0]->Fire();

			++lobTypeCounter;
			if (lobTypeCounter == NUM_LOB_TYPES)
			{
				lobTypeCounter = 0;
			}
		}
		break;
	}
	}

}

void LobTurret::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite, auraSprite);
}


void LobTurret::DirectKill()
{
	for (int i = 0; i < numLaunchers; ++i)
	{
		BasicBullet *b = launchers[i]->activeBullets;
		while (b != NULL)
		{
			BasicBullet *next = b->next;
			double angle = atan2(b->velocity.y, -b->velocity.x);
			sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
			b->launcher->DeactivateBullet(b);

			b = next;
		}
	}
	receivedHit = NULL;
}

void LobTurret::UpdateSprite()
{
	if (action == WAIT)
	{
		sprite.setTextureRect(ts->GetSubRect(0));
	}
	else
	{
		if (frame / animationFactor > 12)
		{
			sprite.setTextureRect(ts->GetSubRect(0));
		}
		else
		{
			sprite.setTextureRect(ts->GetSubRect(frame / animationFactor));//frame / animationFactor ) );
		}
	}

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());

	SyncSpriteInfo(auraSprite, sprite);
}

void LobTurret::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);
}