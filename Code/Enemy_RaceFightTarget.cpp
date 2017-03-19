#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

RaceFightTarget::RaceFightTarget( GameSession *owner, Vector2i &pos )
	:Enemy( owner, EnemyType::RACEFIGHTTARGET, false, 1 )
{
	initHealth = 60;
	health = initHealth;

	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	initHealth = 20;
	health = initHealth;

	//spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	frame = 0;

	animationFactor = 10;

	//ts = owner->GetTileset( "RaceFightTarget.png", 80, 80 );
	ts = owner->GetTileset( "racefighttarget_64x64.png", 64, 64 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );
	//position.x = 0;
	//position.y = 0;
	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 32;
	hurtBody.rh = 32;

	/*hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = 32;
	hitBody.rh = 32;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;*/
	//hitboxInfo->kbDir;

	pPrev = NULL;
	pNext = NULL;
	p2Prev = NULL;
	p2Next = NULL;
	

	dead = false;

	UpdateHitboxes();

	spawnRect = sf::Rect<double>( position.x - 32, position.y - 32,
		64, 64 );

	action = NEUTRAL;
	//cout << "finish init" << endl;
}

void RaceFightTarget::HandleEntrant( QuadTreeEntrant *qte )
{
	//RaceFightTarget 
}


void RaceFightTarget::ResetEnemy()
{
	dead = false;
	action = NEUTRAL;
	frame = 0;
	
	frame = 0;
	receivedHit = NULL;

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;

	pPrev = NULL;
	pNext = NULL;
	p2Prev = NULL;
	p2Next = NULL;
	
}

void RaceFightTarget::UpdatePrePhysics()
{
	frame = 0;
	/*if( action == NEUTRAL )
	{
		frame = 0;
	}*/
	//frame = 0;
	//if( frame == 11 * animationFactor )
	//{
	//	frame = 0;
	//}

	if( receivedHit != NULL )
		receivedHit = NULL;
	//if( !dead && receivedHit != NULL && action == NEUTRAL )
	//{	
	//	//gotta factor in getting hit by a clone
	//	//health -= 20;

	//	//cout << "health now: " << health << endl;

	//	if( health <= 0 )
	//	{
	//		if( hasMonitor && !suppressMonitor )
	//			owner->keyMarker->CollectKey();
	//		action = MALFUNCTION;
	//		frame = 0;
	//		//dead = true;
	//	}

	//	receivedHit = NULL;
	//}
}

void RaceFightTarget::UpdatePhysics()
{	
	if( PlayerSlowingMe() )
	{
		if( slowMultiple == 1 )
		{
			slowCounter = 1;
			slowMultiple = 5;
		}
	}
	else
	{
		slowMultiple = 1;
		slowCounter = 1;
	}

	PhysicsResponse();
}

void RaceFightTarget::PhysicsResponse()
{
	if( action == NEUTRAL && !dead && receivedHit == NULL )
	{
		UpdateHitboxes();

		//if( action == NEUTRAL )
		
		pair<bool,bool> result = PlayerHitMe( 0 );
		pair<bool,bool> result2 = PlayerHitMe( 1 );
		if( result.first || result2.first )
		{
			//triggers multiple times per frame? bad?
			//instead of 6
			owner->player->ConfirmHit( 1, 5, .8, 0 );

			if( result.first )
			{
				action = PLAYER1;
				assert( owner->raceFight != NULL );
				owner->raceFight->HitByPlayer( 0, this );
				//owner->raceFight->playerScore++;
			}
			else
			{
				action = PLAYER2;
				assert( owner->raceFight != NULL );
				owner->raceFight->HitByPlayer( 1, this );
			}

			if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
			{
				owner->player->velocity.y = 4;//.5;
			}
		}	
	}
}

void RaceFightTarget::UpdatePostPhysics()
{
	if( receivedHit != NULL )
	{
		owner->Pause( 5 );
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->player->position + position ) / 2.0, true, 0, 10, 2, true );
	}

	
	UpdateSprite();

	if( slowCounter == slowMultiple )
	{
		//cout << "fireCounter: " << fireCounter << endl;
		++frame;
		slowCounter = 1;
	}
	else
	{
		slowCounter++;
	}
}

void RaceFightTarget::UpdateSprite()
{
	if( !dead )
	{
		IntRect ir;
		switch( action )
		{
		case NEUTRAL:
			ir = ts->GetSubRect( 0 );
			break;
		case PLAYER1:
			ir = ts->GetSubRect( 1 );
			break;
		case PLAYER2:
			ir = ts->GetSubRect( 2 );
			break;
		}
		sprite.setTextureRect( ir );
		sprite.setOrigin( sprite.getLocalBounds().width / 2, 
			sprite.getLocalBounds().height / 2 );
		sprite.setPosition( position.x, position.y );
	}
	else
	{
	}
}

void RaceFightTarget::Draw( sf::RenderTarget *target )
{
	target->draw( sprite );

	//cout << "draw" << endl;
	/*if( !dead )
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
	else
	{
	}*/
}

void RaceFightTarget::DrawMinimap( sf::RenderTarget *target )
{
	if( !dead )
	{
		CircleShape enemyCircle;
		enemyCircle.setFillColor( COLOR_BLUE );
		enemyCircle.setRadius( 50 );
		enemyCircle.setOrigin( enemyCircle.getLocalBounds().width / 2, enemyCircle.getLocalBounds().height / 2 );
		enemyCircle.setPosition( position.x, position.y );
		target->draw( enemyCircle );

		/*if( hasMonitor && !suppressMonitor )
		{
			monitor->miniSprite.setPosition( position.x, position.y );
			target->draw( monitor->miniSprite );
		}*/
	}
}

bool RaceFightTarget::IHitPlayer( int index )
{
	return false;
}

void RaceFightTarget::UpdateHitboxes()
{
	hurtBody.globalPosition = position;
	hurtBody.globalAngle = 0;
}

//return pair<bool,bool>( hitme, was it with a clone)
pair<bool,bool> RaceFightTarget::PlayerHitMe( int index )
{
	Actor *player = owner->player;
	if( index == 1 )
	{
		player = owner->player2;
		assert( player != NULL );
	}

	if( player->currHitboxes != NULL )
	{
		bool hit = false;

		for( list<CollisionBox>::iterator it = player->currHitboxes->begin(); it != player->currHitboxes->end(); ++it )
		{
			if( hurtBody.Intersects( (*it) ) )
			{
				hit = true;
				break;
			}
		}
		
		

		if( hit )
		{
			receivedHit = player->currHitboxInfo;
			return pair<bool, bool>(true,false);
		}
		
	}

	for( int i = 0; i < player->recordedGhosts; ++i )
	{
		if( player->ghostFrame < player->ghosts[i]->totalRecorded )
		{
			if( player->ghosts[i]->currHitboxes != NULL )
			{
				bool hit = false;
				
				for( list<CollisionBox>::iterator it = player->ghosts[i]->currHitboxes->begin(); it != player->ghosts[i]->currHitboxes->end(); ++it )
				{
					if( hurtBody.Intersects( (*it) ) )
					{
						hit = true;
						break;
					}
				}
		

				if( hit )
				{
					receivedHit = player->currHitboxInfo;
					return pair<bool, bool>(true,true);
				}
			}
			//player->ghosts[i]->curhi
		}
	}

	return pair<bool, bool>(false,false);
}

bool RaceFightTarget::PlayerSlowingMe()
{
	Actor *player = owner->player;
	for( int i = 0; i < player->maxBubbles; ++i )
	{
		if( player->bubbleFramesToLive[i] > 0 )
		{
			if( length( position - player->bubblePos[i] ) <= player->bubbleRadius )
			{
				return true;
			}
		}
	}
	return false;
}

void RaceFightTarget::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		hurtBody.DebugDraw( target );
	}
}

void RaceFightTarget::SaveEnemyState()
{

}

void RaceFightTarget::LoadEnemyState()
{

}