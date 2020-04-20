#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_BasicTurret.h"
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

BasicTurret::BasicTurret(ActorParams *ap )//bool p_hasMonitor, Edge *g, double q, int p_level )
		:Enemy( EnemyType::EN_BASICTURRET, ap )//, p_hasMonitor, 1 ), firingCounter( 0 ), ground( g ),
		//edgeQuantity( q )
{
	firingCounter = 0;
	ground = startPosInfo.GetEdge();
	edgeQuantity = startPosInfo.GetQuant();

	level = ap->GetLevel();

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

	framesWait = 60;
	bulletSpeed = 10;
	receivedHit = NULL;


	double width = 208;
	double height = 176;

	fireSound = sess->GetSound("Enemies/turret_shoot");

	ts = sess->GetTileset("Enemies/turret_208x176.png", width, height);//"basicturret_128x80.png", width, height );
	ts_aura = sess->GetTileset("Enemies/turret_aura_208x176.png", width, height);

	width *= scale;
	height *= scale;

	V2d gPoint = ground->GetPosition(edgeQuantity);
	gn = ground->Normal();
	angle = atan2(gn.x, -gn.y);

	position = gPoint + gn * (height / 2.f - 30 * scale);
	
	testShield = new Shield(Shield::ShieldType::T_BLOCK, 80 * scale, 3, this);
	testShield->SetPosition(position);

	auraSprite.setTexture(*ts_aura->texture);


	/*sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect(frame));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(pos.x, pos.y);
	sprite.setScale(scale, scale);*/

	sprite.setTexture( *ts->texture );
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setScale(scale, scale);
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height /2 );
	sprite.setPosition(position.x, position.y);
	sprite.setRotation(angle / PI * 180);

	
	//sprite.setPosition( gPoint.x, gPoint.y );
	
	ts_bulletExplode = sess->GetTileset( "FX/bullet_explode1_64x64.png", 64, 64 );

	
	//sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height/2 );
	//V2d gPoint = ground->GetPoint( edgeQuantity );
	
	

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3*60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;

	BasicCircleHurtBodySetup(64, position);
	BasicCircleHitBodySetup(64, position);
	hitBody.hitboxInfo = hitboxInfo;

	frame = 0;
	animationFactor = 4;

	dead = false;

	double size = max( width, height );

	V2d along = normalize(ground->v1 - ground->v0);

	V2d launchPos = gPoint + ground->Normal() * 20.0 * (double)scale;
	numLaunchers = 1;
	launchers = new Launcher*[numLaunchers];
	launchers[0] = new Launcher( this, BasicBullet::BASIC_TURRET, 16, 1, launchPos + ground->Normal() * 60.0, gn, 0, 300 );
	launchers[0]->SetBulletSpeed( bulletSpeed );
	launchers[0]->hitboxInfo->damage = 18;

	/*launchers[1] = new Launcher(this, BasicBullet::BASIC_TURRET, owner, 16, 1, launchPos + along * 20.0 , along, 0, 300);
	launchers[1]->SetBulletSpeed(bulletSpeed);
	launchers[1]->hitboxInfo->damage = 18;

	launchers[2] = new Launcher(this, BasicBullet::BASIC_TURRET, owner, 16, 1, launchPos - along * 20.0, -along, 0, 300);
	launchers[2]->SetBulletSpeed(bulletSpeed);
	launchers[2]->hitboxInfo->damage = 18;*/
	//launcher->Reset();
	
	
	//UpdateSprite();
	spawnRect = sf::Rect<double>( gPoint.x - size / 2, gPoint.y - size / 2, size, size );

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(12);
	cutObject->SetSubRectBack(11);
	cutObject->rotateAngle = sprite.getRotation();
	cutObject->SetScale(scale);

	testShield->SetPosition(position);

	ResetEnemy();
}

void BasicTurret::ResetEnemy()
{
	//launchers[0]->Reset();
	dead = false;
	frame = 0;

	action = WAIT;
	SetHurtboxes(&hurtBody, 0);
	SetHitboxes(&hitBody, 0);

	currShield = testShield;
	testShield->Reset();

	UpdateSprite();
}

void BasicTurret::BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos )
{
	V2d norm = edge->Normal();
	double angle = atan2( norm.y, -norm.x );

	sess->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, -angle, 6, 2, true );
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
	sess->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
	sess->PlayerApplyHit( b->launcher->hitboxInfo );
	b->launcher->DeactivateBullet( b );
}

void BasicTurret::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);
	switch (action)
	{
		case WAIT:
		{
			if (length(playerPos - position) < 700)
			{
				action = ATTACK;
				frame = 0;
			}
			break;
		}
		case ATTACK:
		{
			if (frame == 11 * animationFactor)
			{
				frame = 0;
				if (length(playerPos - position) >= 700)
				{
					action = WAIT;
					frame = 0;
				}
			}
			else if (frame == 3 * animationFactor && slowCounter == 1)
			{
				launchers[0]->Fire();
				sess->ActivateSoundAtPos( position, fireSound);
				//launchers[1]->Fire();
				//launchers[2]->Fire();
			}
			break;
		}
	}
}

void BasicTurret::UpdatePreLauncherPhysics()
{
	for (int i = 0; i < 1; ++i)
	{
		if (!prelimBox[i].Intersects(sess->GetPlayer(i)->hurtBody))
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
		prelimBox[i].DebugDraw( CollisionBox::Hit, target);
	}
}

void BasicTurret::DirectKill()
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

void BasicTurret::EnemyDraw(sf::RenderTarget *target )
{
	target->draw(auraSprite);
	DrawSpriteIfExists(target, sprite);
}

void BasicTurret::UpdateSprite()
{
	if (action == WAIT)
	{
		sprite.setTextureRect(ts->GetSubRect(0));
	}
	else
	{
		sprite.setTextureRect(ts->GetSubRect(frame / animationFactor));
	}
	
	SyncSpriteInfo(auraSprite, sprite);
}

void BasicTurret::SetupPreCollision()
{

}

void BasicTurret::Setup()
{
	for (int li = 0; li < 1; ++li)
	{
		/*launchers[li]->Reset();
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

		launchers[li]->interactWithTerrain = false;*/

		V2d finalPos = TurretSetup();

		double rad = Launcher::GetRadius(launchers[li]->bulletType);
		double width = length(finalPos - launchers[li]->position) + rad * 2;

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