#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_CurveTurret.h"
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



CurveTurret::CurveTurret( ActorParams *ap )
		:Enemy( EnemyType::EN_CURVETURRET, ap )
{	
	SetNumActions(Count);
	SetEditorActions(ATTACK, 0, 0);

	SetLevel(ap->GetLevel());

	framesWait = 60;
	bulletSpeed = 10;
	animationFactor = 3;
	assert( framesWait > 13 * animationFactor );

	realWait = framesWait - 13 * animationFactor;

	double width = 144; //112;
	double height = 96;

	//ts = owner->GetTileset( "basicturret_112x64.png", width, height );
	ts = sess->GetTileset( "Enemies/curveturret_144x96.png", width, height );

	width *= scale;
	height *= scale;

	SetOffGroundHeight(height / 2.f);// -30 * scale);

	sprite.setTexture( *ts->texture );
	
	sprite.setScale(scale, scale);	

	shield = new Shield(Shield::ShieldType::T_BLOCK, 80 * scale, 3, this);

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


	ts_bulletExplode = sess->GetTileset( "FX/bullet_explode2_64x64.png", 64, 64 );

	SetNumLaunchers(1);
	launchers[0] = new Launcher( this, BasicBullet::CURVE_TURRET, 32, 1, GetPosition(), V2d( 0,-1), 
		0, 90, false, 30, 50 );
	launchers[0]->SetBulletSpeed( bulletSpeed );
	launchers[0]->hitboxInfo->damage = 18;
	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(12);
	cutObject->SetSubRectBack(11);
	cutObject->SetScale(scale);
	cutObject->rotateAngle = sprite.getRotation();

	turnFactor = 500;

	ResetEnemy();
}

void CurveTurret::ResetEnemy()
{
	dead = false;
	action = WAIT;
	frame = 0;
	DefaultHurtboxesOn();
	DefaultHitboxesOn();
	currShield = shield;
	shield->Reset();

	UpdateHitboxes();
	UpdateSprite();
}

void CurveTurret::SetLevel(int lev)
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

void CurveTurret::Setup()
{
	Enemy::Setup();

	launchers[0]->position = GetPosition();
	launchers[0]->facingDir = currPosInfo.GetEdge()->Normal();
	//TurretSetup();
}

void CurveTurret::FireResponse(BasicBullet *b)
{
	double turn = turnFactor / 90.0;
	turn = turn / 180.0 * PI;
	/*if (rand() % 2 == 0)
	{
		b->spin = turn;
	}
	else
	{
		b->spin = -turn;
	}*/
	
}

void CurveTurret::UpdateBullet(BasicBullet *b)
{
	//V2d vel = b->velocity;
	int diff = b->launcher->maxFramesToLive - b->framesToLive;

	
	
	/*int ind = b->index - startBulletIndex;
	double turn = 0;
	if (diff == 1)
	{
		turn = turnFactor / 90.0;
		turn = turn / 180.0 * PI;
		if (ind % 2 == 0)
		{
			turn = -turn;
		}

		spins[ind] = turn;
	}
	else
	{
		turn = spins[ind];
	}*/

	//RotateCW(b->velocity, b->spin);
	//V2d playerPos = owner->GetPlayer(0)->position;

	//cross( normalize( playerPos - position ) )

	if ( diff < 30 )
	{
		//double rad = turnFactor / 60.0;
		//rad = rad / 180.0 * PI;
		//RotateCW(b->velocity, rad);
	}
	else if (diff < 60)
	{
		//b->velocity = V2d(0, 0);
	}
	else
	{
		//b->launcher->SetGravity(normalize(owner->GetPlayer(0)->position - b->position) * .6);
		//b->velocity = normalize(owner->GetPlayer(0)->position - b->position);// * b->launcher->bulletSpeed;
	}

}

void CurveTurret::BulletHitTerrain(BasicBullet *b, 
		Edge *edge, 
		sf::Vector2<double> &pos)
{
	V2d norm = edge->Normal();
	double angle = atan2( norm.y, -norm.x );
	sess->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true );
	b->launcher->DeactivateBullet( b );

	//if (b->launcher->def_e == NULL)
	//	b->launcher->SetDefaultCollision(max( b->framesToLive -4, 0 ), edge, pos);
}

void CurveTurret::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	V2d vel = b->velocity;
	double angle = atan2( vel.y, vel.x );
	sess->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );

	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}
	
	b->launcher->DeactivateBullet( b );
	//owner->GetPlayer( 0 )->ApplyHit( b->launcher->hitboxInfo );
}


void CurveTurret::ProcessState()
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
		else if (frame >= 4 * animationFactor && frame <= 4 * animationFactor + 10 && slowCounter == 1 )
		{
			//launchers[0]->facingDir = gn;//normalize(playerPos - position);
			launchers[0]->Fire();
		}
		break;
	}
	}
	
}

void CurveTurret::EnemyDraw(sf::RenderTarget *target )
{
	DrawSprite(target, sprite, auraSprite);
}


void CurveTurret::DirectKill()
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

void CurveTurret::UpdateSprite()
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

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height/2);
	sprite.setPosition(GetPositionF());
	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());

	SyncSpriteInfo(auraSprite, sprite);
}

void CurveTurret::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);
}