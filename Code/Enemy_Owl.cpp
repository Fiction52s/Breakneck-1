#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Owl.h"
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


Owl::Owl( GameSession *owner, bool p_hasMonitor, Vector2i &pos, int p_level )
	:Enemy( owner, EnemyType::EN_OWL, p_hasMonitor, 2 ), flyingBez( 0, 0, 1, 1 )
{
	level = p_level;

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

	ts_death = owner->GetTileset( "Enemies/owl_death_160x160.png", 160, 160 );
	ts_flap = owner->GetTileset( "Enemies/owl_flap_160x160.png", 160, 160 );
	ts_spin = owner->GetTileset( "Enemies/owl_spin_160x160.png", 160, 160 );
	ts_throw = owner->GetTileset( "Enemies/owl_throw_160x160.png", 160, 160 );

	cutObject->SetTileset(ts_death);
	cutObject->SetSubRectFront(0);
	cutObject->SetSubRectBack(1);
	
	retreatRadius = 400;
	chaseRadius = 600;
	shotRadius = 800;
	flySpeed = 5.0;
	velocity = V2d( 0, 0 );
	action = REST;
	frame = 0;
	//actionLength[NEUTRAL] = 30;
	actionLength[FIRE] = 10 * 6;
	//actionLength[RETREAT] = 30;
	//actionLength[CHASE] = 30;
	actionLength[GUARD] = 120;
	actionLength[REST] = 60;
	actionLength[SPIN] = 60;

	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	originalPos = pos;

	facingRight = true;

	bulletSpeed = 10;
	framesBetween = 60;
	
	numLaunchers = 2;
	launchers = new Launcher*[numLaunchers];
	launchers[0] = new Launcher( this, BasicBullet::OWL, owner, 16, 1, position, V2d( 1, 0 ), 0, 300 );
	launchers[0]->SetBulletSpeed( bulletSpeed );
	launchers[0]->hitboxInfo->damage = 18;
	launchers[0]->Reset();

	launchers[1] = new Launcher(this, BasicBullet::BIG_OWL, owner, 16, 6, position, V2d(0, -1), 2 * PI, 300);
	launchers[1]->SetBulletSpeed(bulletSpeed);
	launchers[1]->hitboxInfo->damage = 18;
	launchers[1]->Reset();
	//launchers[1]->facingDir = V2d(0, -1);

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	

	SetupBodies(1, 1);
	AddBasicHurtCircle(16);
	AddBasicHitCircle(16);
	hitBody->hitboxInfo = hitboxInfo;

	ts_bulletExplode = owner->GetTileset("FX/bullet_explode3_64x64.png", 64, 64);

	//hitboxInfo->kbDir;

	/*guardCircle.setRadius( 50 );
	guardCircle.setFillColor( Color::White );
	guardCircle.setOrigin( guardCircle.getLocalBounds().width / 2, guardCircle.getLocalBounds().height / 2 );
	guardCircle.setPosition( position.x, position.y );*/
	facingRight = true;

	shield = new Shield(Shield::ShieldType::T_BLOCK, 50, 4, this);

	ResetEnemy();

	sprite.setScale(scale, scale);
}

void Owl::BulletHitTerrain( BasicBullet *b, Edge *edge, V2d &pos )
{
	if( b->bounceCount == 2 )
	{
		V2d norm = edge->Normal();
		double angle = atan2( norm.y, -norm.x );
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true );
		b->launcher->DeactivateBullet( b );
	}
	else
	{
		V2d en = edge->Normal();
		if( pos == edge->v0 )
		{
			en = normalize( b->position - pos );
		}
		else if( pos == edge->v1 )
		{
			en = normalize( b->position - pos );
		}
		double d = dot( b->velocity, en );
		V2d ref = b->velocity - (2.0 * d * en);
		b->velocity = ref;
		//cout << "ref: " << ref.x << ", " << ref.y << endl;
		//b->velocity = -b->velocity;
		b->bounceCount++;
		b->framesToLive = b->launcher->maxFramesToLive;
	}
}

void Owl::BulletHitPlayer(BasicBullet *b )
{
	//owner->GetPlayer( 0 )->ApplyHit( b->launcher->hitboxInfo );
	V2d vel = b->velocity;
	double angle = atan2( vel.y, vel.x );
	owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
	owner->GetPlayer( 0 )->ApplyHit( b->launcher->hitboxInfo );
	b->launcher->DeactivateBullet( b );
}

void Owl::ResetEnemy()
{
	velocity = V2d( 0, 0 );
	
	dead = false;
	frame = 0;
	position.x = originalPos.x;
	position.y = originalPos.y;
	receivedHit = NULL;
	
	action = REST;
	frame = 0;

	currShield = shield;
	shield->Reset();

	UpdateHitboxes();

	UpdateSprite();
	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);
	
}

void Owl::DirectKill()
{
	BasicBullet *b = launchers[0]->activeBullets;
	while( b != NULL )
	{
		BasicBullet *next = b->next;
		double angle = atan2( b->velocity.y, -b->velocity.x );
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
		b->launcher->DeactivateBullet( b );

		b = next;
	}
	receivedHit = NULL;
}

void Owl::ActionEnded()
{
	double dist = 800;
	Actor *player = owner->GetPlayer( 0 );
	double len = length( player->position - position );
	if( frame == actionLength[action] )
	{
		switch( action )
		{
		case REST:
			break;
		case SPIN:
			{
				action = FIRE;

				//V2d dir = normalize( parent->position - position );
				//double angle = atan2( dir.x, -dir.y );
				launchers[0]->position = position + fireDir * 40.0;
				fireDir = normalize( owner->GetPlayer( 0 )->position - position );
				ang = atan2( fireDir.x, -fireDir.y );
				//cout << "true ang: " << (ang / PI * 180.0) << endl;
			}
			break;
		case GUARD:
			action = REST;
			frame = 0;
			break;
		case FIRE:
			if( length( player->position - position ) >= dist )
			{
				action = REST;
				frame = 0;
			}

			action = SPIN;
			//action = FIRE;
			break;
		}
		//if( action == FIRE )
		//{
		//	if( len > shotRadius )
		//	{
		//		action = REST;
		//	}
		//	else if( len > chaseRadius )
		//	{
		//		action = CHASE;
		//		velocity = normalize( player->position - position ) * 2.5;
		//	}
		//	else if( len < retreatRadius )
		//	{
		//		action = RETREAT;
		//		velocity = normalize( player->position - position ) * -2.5;
		//	}
		//	else
		//	{
		//		action = NEUTRAL;
		//	}	
		//}
		//else if( action == REST )
		//{
		//	if( len > shotRadius )
		//	{
		//		//stay the same
		//	}
		//	else if( len > chaseRadius )
		//	{
		//		action = CHASE;
		//		velocity = normalize( player->position - position ) * 2.5;
		//	}
		//	else if( len < retreatRadius )
		//	{
		//		action = RETREAT;
		//		velocity = normalize( player->position - position ) * -2.5;
		//	}
		//	else
		//	{
		//		action = NEUTRAL;
		//		velocity = V2d( 0, 0 );
		//	}	
		//}
		//else
		//{
		//	action = FIRE;
		//	velocity = V2d( 0, 0 );
		//}
		frame = 0;
	}
}

void Owl::ShieldDestroyed(Shield *shield)
{
	launchers[1]->position = position;
	//launchers[1]->facingDir = fireDir;//normalize( owner->GetPlayer( 0 )->position - position );
	launchers[1]->Fire();
}

void Owl::ProcessState()
{
	ActionEnded();

	Actor *player = owner->GetPlayer( 0 );
	double dist = 600;
	bool lessThanSize = length( player->position - position ) < dist;
	
	switch( action )
	{
	case REST:
		{
			if( lessThanSize )
			{
				action = SPIN;
				frame = 0;
			}
		}
		break;
	case GUARD:
		{
			//action = REST;
		}
		break;
	case SPIN:
		{
			if( !lessThanSize )
			{
				action = REST;
				frame = 0;
			}
		}
		break;
	case FIRE:
		{
			
		}
		break;
	}


	//switch( action )
	//{
	//case NEUTRAL:
	//	cout << "neutral: " << frame << endl;
	//	break;
	//case FIRE:
	//	cout << "fire: " << frame << endl;
	//	break;
	//case RETREAT:
	//	cout << "retreat: " << frame << endl;
	//	
	//	break;
	//case CHASE:
	//	cout << "chase" << endl;
	//	break;
	//case REST:
	//	cout << "rest" << endl;
	//	break;
	//default:
	//	cout << "what" << endl;
	//}


	/*if( action == RETREAT )
	{
		velocity = normalize( player->position - position ) * -2.5;
	}
	else if( action == NEUTRAL )
	{
		velocity = normalize( player->position - position ) * 2.5;
	}*/

	


	

	if( action == FIRE && frame == 3 * 6 - 1  )// frame == 0 && slowCounter == 1 )
	{
		launchers[0]->position = position;
		launchers[0]->facingDir = fireDir;//normalize( owner->GetPlayer( 0 )->position - position );
		launchers[0]->Fire();
	}
}

void Owl::UpdateEnemyPhysics()
{	
	shield->SetPosition(position);
}

void Owl::UpdateSprite()
{
	switch( action )
	{
	case REST:
		{
			sprite.setRotation( 0 );
			sprite.setTexture( *ts_flap->texture );
			sprite.setTextureRect( ts_flap->GetSubRect( (frame / 5) % 7 ) );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
		}
		break;
	case SPIN:
		{
			sprite.setRotation( 0 );
			sprite.setTexture( *ts_spin->texture );
			sprite.setTextureRect( ts_spin->GetSubRect( (frame / 5) % 8 ) );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
		}
		break;
	case GUARD:
		{
			sprite.setRotation( 0 );
			sprite.setTexture( *ts_spin->texture );
			sprite.setTextureRect( ts_spin->GetSubRect( 0 ) );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
		}
		break;
	case FIRE:
		{
			sprite.setRotation( ang / PI * 180.f + 90 );
			sprite.setTexture( *ts_throw->texture );
			sprite.setTextureRect( ts_throw->GetSubRect( frame / 6 ) );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
		}
		break;
	}
	//sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setPosition( position.x, position.y );

	if( hasMonitor && !suppressMonitor )
	{
		//keySprite.setTexture( *ts_key->texture );
		keySprite->setTextureRect( ts_key->GetSubRect( owner->keyFrame / 5 ) );
		keySprite->setOrigin( keySprite->getLocalBounds().width / 2, 
			keySprite->getLocalBounds().height / 2 );
		keySprite->setPosition( position.x, position.y );

	}
}

void Owl::EnemyDraw( sf::RenderTarget *target )
{
	DrawSpriteIfExists(target, sprite);
}

void Owl::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();

	hurtBox.globalPosition = position;
	hurtBox.globalAngle = 0;
	hitBox.globalPosition = position;
	hitBox.globalAngle = 0;

	if( owner->GetPlayer( 0 )->ground != NULL )
	{
		hitboxInfo->kbDir = normalize( -owner->GetPlayer( 0 )->groundSpeed * ( owner->GetPlayer( 0 )->ground->v1 - owner->GetPlayer( 0 )->ground->v0 ) );
	}
	else
	{
		hitboxInfo->kbDir = normalize( -owner->GetPlayer( 0 )->velocity );
	}
}