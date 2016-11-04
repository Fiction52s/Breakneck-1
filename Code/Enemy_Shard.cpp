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

Shard::Shard( GameSession *p_owner, Vector2i pos, int p_shardIndex )
	:Enemy( p_owner, EnemyType::SHARD, false, p_owner->envType ), deathFrame( 0 )
{
	shardIndex = p_shardIndex;
	assert( shardIndex < 3 );
	initHealth = 60;
	health = initHealth;

	//hopefully this doesnt cause deletion bugs
	radius = 400;

	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	deathFrame = 0;

	initHealth = 20;
	health = initHealth;

	caught = false;

	spawnRect = sf::Rect<double>( pos.x - 32, pos.y - 32, 32 * 2, 32 * 2 );
	
	frame = 0;

	animationFactor = 10;

	//cout << "world: " << world << endl;
	switch( p_owner->envType )
	{
	case 1:
		ts = owner->GetTileset( "shards_w1_64x64.png", 64, 64 );
		break;
	case 2:
		ts = owner->GetTileset( "shards_w2_64x64.png", 64, 64 );
		break;
	case 3:
		ts = owner->GetTileset( "shards_w3_64x64.png", 64, 64 );
		break;
	case 4:
		ts = owner->GetTileset( "shards_w4_64x64.png", 64, 64 );
		break;
	case 5:
		ts = owner->GetTileset( "shards_w5_64x64.png", 64, 64 );
		break;
	case 6:
		ts = owner->GetTileset( "shards_w6_64x64.png", 64, 64 );
		break;
	case 7:
		ts = owner->GetTileset( "shards_w7_64x64.png", 64, 64 );
		break;
	}
	//ts = owner->GetTileset( "Shard.png", 80, 80 );
	//ts = owner->GetTileset( "Shard_256x256.png", 256, 256 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( shardIndex ) );
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

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = 32;
	hitBody.rh = 32;

	/*hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;*/
	//hitboxInfo->kbDir;

	

	dead = false;

	//ts_bottom = owner->GetTileset( "patroldeathbot.png", 32, 32 );
	//ts_top = owner->GetTileset( "patroldeathtop.png", 32, 32 );
	//ts_death = owner->GetTileset( "patroldeath.png", 80, 80 );

	//deathPartingSpeed = .4;
	//deathVector = V2d( 1, -1 );

	//facingRight = true;
	 
	//ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	//bloodSprite.setTexture( *ts_testBlood->texture );

	UpdateHitboxes();

	//spawnRect = sf::Rect<double>( position.x - 200, position.y - 200,
	//	400, 400 );

	//cout << "finish init" << endl;
}

void Shard::HandleEntrant( QuadTreeEntrant *qte )
{
	//Shard 
}


void Shard::ResetEnemy()
{
//	fireCounter = 0;
//	testSeq.Reset();
//	launcher->Reset();
	//cout << "resetting enemy" << endl;
	//spawned = false;
	//targetNode = 1;
	//forward = true;
	dead = true;
	spawned = true;
//	dying = false;
	deathFrame = 0;
	frame = 0;
//	position.x = path[0].x;
//	position.y = path[0].y;
	receivedHit = NULL;
	

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;
	
}

void Shard::UpdatePrePhysics()
{
	if( frame == 11 * animationFactor )
	{
		frame = 0;
	}

	

	//if( !dead && receivedHit != NULL )
	//{	
	//	//gotta factor in getting hit by a clone
	//	health -= 20;

	//	//cout << "health now: " << health << endl;

	//	if( health <= 0 )
	//	{
	//		if( hasMonitor && !suppressMonitor )
	//			owner->keyMarker->CollectKey();
	//		dead = true;
	//	}

	//	receivedHit = NULL;
	//}
}

void Shard::UpdatePhysics()
{	
	/*if( PlayerSlowingMe() )
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
	}*/

	UpdateHitboxes();
	pair<bool,bool> hitMe = PlayerHitMe();
	bool ihit = IHitPlayer();
	if( ihit || hitMe.first )
	{
		if( !caught )
		{
			caught = true;
		}
	}

	PhysicsResponse();
}

void Shard::PhysicsResponse()
{
	if( !dead && receivedHit == NULL )
	{
		UpdateHitboxes();

		pair<bool,bool> result = PlayerHitMe();
		if( result.first )
		{
			//cout << "color blue" << endl;
			//triggers multiple times per frame? bad?
			owner->player->ConfirmHit( COLOR_MAGENTA, 5, .8, 6 );


			if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
			{
				owner->player->velocity.y = 4;//.5;
			}

		//	cout << "frame: " << owner->player->frame << endl;

			//owner->player->frame--;
			//owner->ActivateEffect( EffectLayer::IN_FRONT, ts_testBlood, position, true, 0, 6, 3, true );
			
		//	cout << "Shard received damage of: " << receivedHit->damage << endl;
			/*if( !result.second )
			{
				owner->Pause( 8 );
			}
		
			health -= 20;

			if( health <= 0 )
				dead = true;

			receivedHit = NULL;*/
			//dead = true;
			//receivedHit = NULL;
		}

		if( IHitPlayer() )
		{
		//	cout << "Shard just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}

void Shard::UpdatePostPhysics()
{
	if( caught )
	{
		//should the one in patroller be in post or pre physics?
		owner->Pause( 5 );
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->player->position + position ) / 2.0, true, 0, 10, 2, true );
		owner->RemoveEnemy( this );
	}

	/*if( receivedHit != NULL )
	{
		owner->Pause( 5 );
	}*/

	
	UpdateSprite();

	++frame;
	//if( slowCounter == slowMultiple )
	//{
	//	//cout << "fireCounter: " << fireCounter << endl;
	//	++frame;
	//	slowCounter = 1;
	//
	//
	//	if( dead )
	//	{
	//		//cout << "deathFrame: " << deathFrame << endl;
	//		deathFrame++;
	//	}

	//}
	//else
	//{
	//	slowCounter++;
	//}

	

	//if( deathFrame == 60 )
	//{
	//	//cout << "switching dead" << endl;
	//	//dying = false;
	//	//dead = true;
	//	//cout << "REMOVING" << endl;
	//	//testLauncher->Reset();
	//	owner->RemoveEnemy( this );
	//	//return;
	//}

	
}

void Shard::UpdateSprite()
{
	if( !dead )
	{
		sprite.setTextureRect( ts->GetSubRect( shardIndex ) );
		sprite.setPosition( position.x, position.y );
	}
	else
	{

		/*botDeathSprite.setTexture( *ts->texture );
		botDeathSprite.setTextureRect( ts->GetSubRect( 13 ) );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2 );
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );

		topDeathSprite.setTexture( *ts->texture );
		topDeathSprite.setTextureRect( ts->GetSubRect( 12 ) );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );*/
	}
}

void Shard::Draw( sf::RenderTarget *target )
{
	//cout << "draw" << endl;
	if( !dead )
	{
		target->draw( sprite );
	}
	else
	{
		//target->draw( botDeathSprite );

		//if( deathFrame / 3 < 6 )
		//{
		//	
		//	/*bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
		//	bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
		//	bloodSprite.setPosition( position.x, position.y );
		//	bloodSprite.setScale( 2, 2 );
		//	target->draw( bloodSprite );*/
		//}
		//
		//target->draw( topDeathSprite );
	}



}

void Shard::DrawMinimap( sf::RenderTarget *target )
{
	if( !dead )
	{
		CircleShape enemyCircle;
		enemyCircle.setFillColor( COLOR_TEAL );
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

bool Shard::IHitPlayer()
{
	Actor *player = owner->player;
	
	if( hitBody.Intersects( player->hurtBody ) )
	{
		//player->ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void Shard::UpdateHitboxes()
{
	hurtBody.globalPosition = position;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position;
	hitBody.globalAngle = 0;

	/*if( owner->player->ground != NULL )
	{
		hitboxInfo->kbDir = normalize( -owner->player->groundSpeed * ( owner->player->ground->v1 - owner->player->ground->v0 ) );
	}
	else
	{
		hitboxInfo->kbDir = normalize( -owner->player->velocity );
	}*/
}

//return pair<bool,bool>( hitme, was it with a clone)
pair<bool,bool> Shard::PlayerHitMe()
{
	Actor *player = owner->player;
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

bool Shard::PlayerSlowingMe()
{
	/*Actor *player = owner->player;
	for( int i = 0; i < player->maxBubbles; ++i )
	{
		if( player->bubbleFramesToLive[i] > 0 )
		{
			if( length( position - player->bubblePos[i] ) <= player->bubbleRadius )
			{
				return true;
			}
		}
	}*/
	return false;
}

void Shard::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void Shard::SaveEnemyState()
{
	//stored.dead = dead;
	//stored.deathFrame = deathFrame;
	//stored.frame = frame;
	//stored.hitlagFrames = hitlagFrames;
	//stored.hitstunFrames = hitstunFrames;
	//stored.position = position;
}

void Shard::LoadEnemyState()
{
	//dead = stored.dead;
	//deathFrame = stored.deathFrame;
	//frame = stored.frame;
	//hitlagFrames = stored.hitlagFrames;
	//hitstunFrames = stored.hitstunFrames;
	//position = stored.position;
}