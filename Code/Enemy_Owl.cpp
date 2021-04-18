#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Owl.h"
#include "Shield.h"
#include "Actor.h"

using namespace std;
using namespace sf;

Owl::Owl(ActorParams *ap)
	:Enemy( EnemyType::EN_OWL, ap ), flyingBez( 0, 0, 1, 1 )
{
	SetNumActions(A_Count);
	SetEditorActions(REST, REST, 0);

	SetLevel(ap->GetLevel());

	ts_death = sess->GetSizedTileset("Enemies/W3/owl_death_160x160.png");
	ts_flap = sess->GetSizedTileset( "Enemies/W3/owl_flap_160x160.png");
	ts_spin = sess->GetSizedTileset( "Enemies/W3/owl_spin_160x160.png");
	ts_throw = sess->GetSizedTileset( "Enemies/W3/owl_throw_160x160.png");

	cutObject->SetTileset(ts_death);
	cutObject->SetSubRectFront(0);
	cutObject->SetSubRectBack(1);
	cutObject->SetScale(scale);
	
	retreatRadius = 400;
	chaseRadius = 600;
	shotRadius = 800;
	flySpeed = 5.0;

	actionLength[FIRE] = 10 * 6;
	actionLength[GUARD] = 120;
	actionLength[REST] = 60;
	actionLength[SPIN] = 40;

	bulletSpeed = 10;
	framesBetween = 60;
	
	SetNumLaunchers(2);

	launchers[0] = new Launcher( this, BasicBullet::OWL, 16, 3, GetPosition(), V2d( 1, 0 ), PI / 12, 300 );
	launchers[0]->SetBulletSpeed( bulletSpeed );
	launchers[0]->hitboxInfo->damage = 18;
	launchers[0]->Reset();

	launchers[1] = new Launcher(this, BasicBullet::BIG_OWL, 16, 6, GetPosition(), V2d(0, -1), 2 * PI, 300);
	launchers[1]->SetBulletSpeed(bulletSpeed);
	launchers[1]->hitboxInfo->damage = 18;
	launchers[1]->Reset();

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;
	hitboxInfo->kbDir = V2d(1, 0);
	
	BasicCircleHurtBodySetup(32);
	BasicCircleHitBodySetup(32);

	hitBody.hitboxInfo = hitboxInfo;

	sprite.setScale(scale, scale);

	ts_bulletExplode = sess->GetSizedTileset("FX/bullet_explode3_64x64.png");

	if (level == 2)
	{
		shield = new Shield(Shield::ShieldType::T_BLOCK, 70, 4, this);
	}
	else
	{
		shield = NULL;
	}
	

	ResetEnemy();
}

Owl::~Owl()
{
	currShield = NULL;
	delete shield;
}

void Owl::SetLevel(int lev)
{
	level = lev;

	switch (level)
	{
	case 1:
		scale = 1.8;
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

void Owl::BulletHitTerrain( BasicBullet *b, Edge *edge, V2d &pos )
{
	if( b->bounceCount == 2 )
	{
		V2d norm = edge->Normal();
		double angle = atan2( norm.y, -norm.x );
		sess->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true );
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


void Owl::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	V2d vel = b->velocity;
	double angle = atan2( vel.y, vel.x );
	sess->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );

	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}
	
	b->launcher->DeactivateBullet( b );
}

void Owl::ResetEnemy()
{
	velocity = V2d( 0, 0 );

	SetCurrPosInfo(startPosInfo);
	
	action = REST;
	frame = 0;

	if (shield != NULL)
	{
		shield->SetPosition(GetPosition());
		currShield = shield;
		shield->Reset();
	}
	


	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

void Owl::DirectKill()
{
	BasicBullet *b = launchers[0]->activeBullets;
	while( b != NULL )
	{
		BasicBullet *next = b->next;
		double angle = atan2( b->velocity.y, -b->velocity.x );
		sess->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
		b->launcher->DeactivateBullet( b );

		b = next;
	}
	receivedHit = NULL;
}

void Owl::ActionEnded()
{
	double dist = 800;
	V2d playerPos = sess->GetPlayerPos(0);
	V2d pos = GetPosition();
	double len = length( playerPos - pos);
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
				launchers[0]->position = pos + fireDir * 40.0;
				fireDir = normalize(playerPos - pos);
				ang = atan2( fireDir.x, -fireDir.y );
				//cout << "true ang: " << (ang / PI * 180.0) << endl;
			}
			break;
		case GUARD:
			action = REST;
			frame = 0;
			break;
		case FIRE:
			if( length(playerPos - pos) >= dist )
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
	launchers[1]->position = GetPosition();
	//launchers[1]->facingDir = fireDir;//normalize( owner->GetPlayer( 0 )->position - position );
	launchers[1]->Fire();
}

void Owl::ProcessState()
{
	ActionEnded();

	
	V2d playerPos = sess->GetPlayerPos(0);
	V2d pos = GetPosition();

	double dist = 600;
	bool lessThanSize = length(playerPos - pos) < dist;
	
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
		launchers[0]->position = pos;
		launchers[0]->facingDir = fireDir;//normalize( owner->GetPlayer( 0 )->position - position );
		launchers[0]->Fire();
	}
}

void Owl::UpdateEnemyPhysics()
{	
	
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

	sprite.setPosition( GetPositionF() );
}

void Owl::EnemyDraw( sf::RenderTarget *target )
{
	DrawSprite(target, sprite);
}