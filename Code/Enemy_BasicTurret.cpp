#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_BasicTurret.h"

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

BasicTurret::BasicTurret( GameSession *owner, bool p_hasMonitor, Edge *g, double q, double speed,int wait )
		:Enemy( owner, EnemyType::EN_BASICTURRET, p_hasMonitor, 1 ), framesWait( wait), bulletSpeed( speed ), firingCounter( 0 ), ground( g ),
		edgeQuantity( q )
{
	receivedHit = NULL;
	//keyFrame = 0;
	//ts_key = owner->GetTileset( "key_w02_1_128x128.png", 128, 128 );

	//launcher = new Launcher( this, 

	initHealth = 60;
	health = initHealth;

	double width = 128;
	double height = 80;

	ts = owner->GetTileset( "basicturret_128x80.png", width, height );
	sprite.setTexture( *ts->texture );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height /2 );
	V2d gPoint = g->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	
	ts_bulletExplode = owner->GetTileset( "bullet_explode1_64x64.png", 64, 64 );

	gn = g->Normal();

	position = gPoint + gn * height / 2.0;

	angle = atan2( gn.x, -gn.y );

	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );
	//V2d gPoint = ground->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	sprite.setRotation( angle / PI * 180 );

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;

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
	hitBody->hitboxInfo = hitboxInfo;
	hitBody->AddCollisionBox(0, hitBox);


	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);

	frame = 0;
	animationFactor = 3;

	dead = false;

	double size = max( width, height );

	deathPartingSpeed = .4;

	Transform t;
	t.rotate( angle / PI * 180 );
	Vector2f newPoint = t.transformPoint( Vector2f( -1, -1 ) );
	deathVector = V2d( newPoint.x, newPoint.y );

	V2d along = normalize(ground->v1 - ground->v0);

	numLaunchers = 3;
	launchers = new Launcher*[numLaunchers];
	launchers[0] = new Launcher( this, BasicBullet::BASIC_TURRET, owner, 16, 1, position, gn, 0, 300 );
	launchers[0]->SetBulletSpeed( bulletSpeed );
	launchers[0]->hitboxInfo->damage = 18;

	launchers[1] = new Launcher(this, BasicBullet::BASIC_TURRET, owner, 16, 1, position, along, 0, 300);
	launchers[1]->SetBulletSpeed(bulletSpeed);
	launchers[1]->hitboxInfo->damage = 18;

	launchers[2] = new Launcher(this, BasicBullet::BASIC_TURRET, owner, 16, 1, position, -along, 0, 300);
	launchers[2]->SetBulletSpeed(bulletSpeed);
	launchers[2]->hitboxInfo->damage = 18;
	//launcher->Reset();
	
	
	//UpdateSprite();
	spawnRect = sf::Rect<double>( gPoint.x - size / 2, gPoint.y - size / 2, size, size );
}

void BasicTurret::ResetEnemy()
{
	health = initHealth;
	//launchers[0]->Reset();
	dead = false;
	frame = 0;

	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);
}

void BasicTurret::BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos )
{
	V2d norm = edge->Normal();
	double angle = atan2( norm.y, -norm.x );

	owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, -angle, 6, 2, true );
	b->launcher->DeactivateBullet( b );

	if( b->launcher->def_e == NULL )
		b->launcher->SetDefaultCollision(b->framesToLive, edge, pos);
}

void BasicTurret::BulletHitPlayer( BasicBullet *b )
{
	//if you dont deactivate the bullet it will hit constantly and make weird fx

	//cout << "hit player??" << endl;
	V2d vel = b->velocity;
	double angle = atan2( vel.y, vel.x );
	owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
	owner->GetPlayer( 0 )->ApplyHit( b->launcher->hitboxInfo );
	b->launcher->DeactivateBullet( b );
}

void BasicTurret::ProcessState()
{
	if (frame == 22 * animationFactor)
	{
		frame = 0;
	}

	if (frame == 0 && slowCounter == 1)
	{
		launchers[0]->Fire();
		launchers[1]->Fire();
		launchers[2]->Fire();
	}
}

void BasicTurret::UpdatePreLauncherPhysics()
{
	for (int i = 0; i < 3; ++i)
	{
		if (!prelimBox[i].Intersects(owner->players[0]->hurtBody))
		{
			launchers[i]->skipPlayerCollideForSubstep = true;
		}
	}
}

void BasicTurret::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);

	for (int i = 0; i < 3; ++i)
	{
		prelimBox[i].DebugDraw(target);
	}
	//
	//prelimBody->DebugDraw( 0, target);
}

void BasicTurret::DirectKill()
{
	for (int i = 0; i < 3; ++i)
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
	health = 0;
	receivedHit = NULL;
}

void BasicTurret::EnemyDraw(sf::RenderTarget *target )
{
	if( hasMonitor && !suppressMonitor )
	{
		if( owner->pauseFrames < 2 || receivedHit == NULL )
		{
			target->draw( sprite, keyShader );
		}
		else
		{
			target->draw( sprite, hurtShader );
		}
		target->draw( *keySprite );
	}
	else
	{
		if( owner->pauseFrames < 2 || receivedHit == NULL )
		{
			target->draw( sprite );
		}
		else
		{
			target->draw( sprite, hurtShader );
		}	
	}
}

void BasicTurret::UpdateSprite()
{
	sprite.setTextureRect(ts->GetSubRect(frame / animationFactor));
}

void BasicTurret::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hurtBox.globalPosition = position;// + gn * 8.0;
	hurtBox.globalAngle = 0;
	hitBox.globalPosition = position;// + gn * 8.0;
	hitBox.globalAngle = 0;
}

void BasicTurret::SetupPreCollision()
{

}

void BasicTurret::Setup()
{
	for (int li = 0; li < 3; ++li)
	{
		launchers[li]->Reset();
		launchers[li]->Fire();
		BasicBullet *bb = launchers[li]->activeBullets;
		V2d finalPos;
		bool collide = true;
		while (launchers[li]->GetActiveCount() > 0)
		{
			launchers[li]->UpdatePrePhysics();
			launchers[li]->UpdatePhysics(0, true);

			if (bb->framesToLive == 0)
			{
				finalPos = bb->position;
				collide = false;
			}

			launchers[li]->UpdatePostPhysics();
		}

		if (collide)
		{
			finalPos = launchers[li]->def_pos;
		}

		launchers[li]->interactWithTerrain = false;

		double rad = Launcher::GetRadius(launchers[li]->bulletType);
		double width = length(finalPos - launchers[li]->position) + rad * 2;

		prelimBox[li].type = CollisionBox::Hit;
		prelimBox[li].isCircle = false;
		prelimBox[li].rw = width / 2;
		prelimBox[li].rh = rad;

		V2d norm = ground->Normal();


		V2d along = normalize(ground->v1 - ground->v0);
		if (li == 0)
		{
			prelimBox[li].globalAngle = atan2(norm.y, norm.x);
		}
		else
		{
			prelimBox[li].globalAngle = atan2(along.y, along.x);
		}
		
		prelimBox[li].globalPosition = (finalPos + launchers[li]->position) / 2.0;
	}
}