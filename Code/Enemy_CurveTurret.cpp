#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_CurveTurret.h"
#include "Shield.h"

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

CurveTurret::CurveTurret( GameSession *owner, bool p_hasMonitor, Edge *g, double q, double speed,int wait,
	Vector2i &gravFactor, bool relative )
		:Enemy( owner, EnemyType::EN_CURVETURRET, p_hasMonitor, 2 ), framesWait( wait), bulletSpeed( speed ), ground( g ),
		edgeQuantity( q )
{	
	shield = new Shield(Shield::ShieldType::T_BLOCK, 80, 3, this);
	currShield = shield;
	shield->Reset();
	shield->SetPosition(position);

	

	animationFactor = 3;
	assert( framesWait > 13 * animationFactor );

	realWait = framesWait - 13 * animationFactor;

	initHealth = 60;
	health = initHealth;

	double width = 144; //112;
	double height = 96;

	//ts = owner->GetTileset( "basicturret_112x64.png", width, height );
	ts = owner->GetTileset( "Enemies/curveturret_144x96.png", width, height );
	sprite.setTexture( *ts->texture );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height /2 );
	V2d gPoint = g->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );

	gn = g->Normal();

	V2d gAlong = normalize( g->v1 - g->v0 );

	gravity = V2d( 0,0 );

	if( relative )
	{
		gravity += gAlong * ( gravFactor.x / 256.0);
		gravity += gn * ( -gravFactor.y / 256.0 );
	}
	else
	{
		gravity = V2d( gravFactor.x / 256.0, gravFactor.y / 256.0 );
	}
	

	position = gPoint + gn * height / 2.0;

	angle = atan2( gn.x, -gn.y );

	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );
	//V2d gPoint = ground->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	sprite.setRotation( angle / PI * 180 );
	cutObject->rotateAngle = sprite.getRotation();

	hurtBody = new CollisionBody(1);
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 32;
	hurtBox.rh = 32;
	hurtBody->AddCollisionBox(0, hurtBox);

	hitBody = new CollisionBody(1);
	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 32;
	hitBox.rh = 32;
	hitBody->AddCollisionBox(0, hitBox);
	
	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10; 

	hitBody->hitboxInfo = hitboxInfo;

	frame = 0;

	dead = false;

	bulletSpeed = 10;

	double size = 400;//max( width, height );


	ts_bulletExplode = owner->GetTileset( "FX/bullet_explode2_64x64.png", 64, 64 );

	numLaunchers = 1;
	launchers = new Launcher*[numLaunchers];
	V2d turretDir = gn;
	//RotateCCW(gn, PI / 3);
	V2d launchPos = gPoint + gn * 20.0;
	launchers[0] = new Launcher( this, BasicBullet::CURVE_TURRET, owner, 16, 1, position, gn, 0, 90 );
	launchers[0]->SetBulletSpeed( bulletSpeed );
	//launchers[0]->SetGravity( gravity );
	launchers[0]->hitboxInfo->damage = 18;
	//UpdateSprite();
	spawnRect = sf::Rect<double>( gPoint.x - size / 2, gPoint.y - size / 2, size, size );
	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(12);
	cutObject->SetSubRectBack(11);

	turnFactor = 500;

	ResetEnemy();
}

void CurveTurret::Setup()
{
	TurretSetup();
}

void CurveTurret::UpdateBullet(BasicBullet *b)
{
	//V2d vel = b->velocity;
	int diff = b->launcher->maxFramesToLive - b->framesToLive;
	if ( diff > 30 )
	{
		double rad = turnFactor / 60.0;
		rad = rad / 180.0 * PI;
		RotateCW(b->velocity, rad);
	}
}

void CurveTurret::BulletHitTerrain(BasicBullet *b, 
		Edge *edge, 
		sf::Vector2<double> &pos)
{
	V2d norm = edge->Normal();
	double angle = atan2( norm.y, -norm.x );
	owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true );
	b->launcher->DeactivateBullet( b );

	if (b->launcher->def_e == NULL)
		b->launcher->SetDefaultCollision(max( b->framesToLive -4, 0 ), edge, pos);
}

void CurveTurret::BulletHitPlayer(BasicBullet *b )
{
	V2d vel = b->velocity;
	double angle = atan2( vel.y, vel.x );
	owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
	owner->GetPlayer( 0 )->ApplyHit( b->launcher->hitboxInfo );
	b->launcher->DeactivateBullet( b );
	//owner->GetPlayer( 0 )->ApplyHit( b->launcher->hitboxInfo );
}

void CurveTurret::ResetEnemy()
{
	dead = false;
	action = WAIT;
	frame = 0;
	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);
	health = initHealth;
	currShield = shield;
	shield->Reset();
	shield->SetPosition(position);
}

void CurveTurret::ProcessState()
{
	V2d playerPos = owner->GetPlayer(0)->position;
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
			if (length(owner->GetPlayer(0)->position - position) >= 500)
			{
				action = WAIT;
				frame = 0;
			}
		}
		else if (frame == 4 * animationFactor && slowCounter == 1)
		{
			launchers[0]->facingDir = normalize(playerPos - position);
			launchers[0]->Fire();
		}
		break;
	}
	}
	
}

void CurveTurret::EnemyDraw(sf::RenderTarget *target )
{
	DrawSpriteIfExists(target, sprite);
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
			owner->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
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
}

void CurveTurret::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);
}

void CurveTurret::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hurtBox.globalPosition = position;// + gn * 8.0;
	hurtBox.globalAngle = 0;
	hitBox.globalPosition = position;// + gn * 8.0;
	hitBox.globalAngle = 0;
}
