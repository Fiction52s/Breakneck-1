#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

Key::Key( GameSession *owner, Key::KeyType p_keyType, sf::Vector2i pos, std::list<sf::Vector2i> &pathParam, bool p_loop, float p_speed, int p_stayFrames, bool p_teleport )
	:Enemy( owner, EnemyType::KEY ), dead( false ), deathFrame( 0 )
{
	keyType = p_keyType;
	int keySize = 50;
	switch( keyType )
	{
	case RED:
		ts = owner->GetTileset( "key.png", keySize, keySize );
		break;
	case GREEN:
		ts = owner->GetTileset( "greenkey.png", keySize, keySize );
		break;
	case BLUE:
		ts = owner->GetTileset( "bluekey.png", keySize, keySize );
		break;
	default:
		assert( false );
	}
	
	
	position.x = pos.x;
	position.y = pos.y;

	spawnRect = sf::Rect<double>( pos.x - keySize/2, pos.y - keySize/2, keySize, keySize );
	
	pathLength = pathParam.size() + 1;
	cout << "pathLength: " << pathLength << endl;
	path = new Vector2i[pathLength];
	path[0] = pos;

	int index = 1;
	for( list<Vector2i>::iterator it = pathParam.begin(); it != pathParam.end(); ++it )
	{
		path[index] = (*it) + pos;
		++index;
	}

	teleport = p_teleport;

	loop = p_loop;
	
	stayFrames = p_stayFrames;

	speed = p_speed;

	frame = 0;

	animationFactor = 3;
	
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );
	
	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = keySize/2;
	hurtBody.rh = keySize/2;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = keySize/2;
	hitBody.rh = keySize/2;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 0;
	hitboxInfo->drain = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 0;
	hitboxInfo->knockback = 0;

	targetNode = 1;
	forward = true;

	dead = false;

	//ts_death = owner->GetTileset( "patroldeath.png", 80, 80 );

	//deathPartingSpeed = .3;
	//deathVector = V2d( 1, -1 );

	//ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	//bloodSprite.setTexture( *ts_testBlood->texture );

	UpdateHitboxes();
}

void Key::HandleEntrant( QuadTreeEntrant *qte )
{
}

void Key::ResetEnemy()
{
	targetNode = 1;
	forward = true;
	dead = false;
	deathFrame = 0;
	position.x = path[0].x;
	position.y = path[0].y;
}

void Key::UpdatePrePhysics()
{
	
}

void Key::UpdatePhysics()
{
	double movement = speed;
	
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

	if( dead )
		return;

	if( pathLength > 1 )
	{
		movement /= (double)slowMultiple;

		while( movement != 0 )
		{
			V2d targetPoint = V2d( path[targetNode].x, path[targetNode].y );
			V2d diff = targetPoint - position;
			double len = length( diff );
			if( len >= abs( movement ) )
			{
				position += normalize( diff ) * movement;
				movement = 0;
			}
			else
			{
				position += diff;
				movement -= length( diff );
				AdvanceTargetNode();	
			}
		}
	}

	PhysicsResponse();
}

void Key::AdvanceTargetNode()
{
	if( loop )
	{
		++targetNode;
		if( targetNode == pathLength )
			targetNode = 0;
	}
	else
	{
		if( forward )
		{
			++targetNode;
			if( targetNode == pathLength )
			{
				targetNode -= 2;
				forward = false;
			}
		}
		else
		{
			--targetNode;
			if( targetNode < 0 )
			{
				targetNode = 1;
				forward = true;
			}
		}
	}
}

void Key::PhysicsResponse()
{
	Actor &player = owner->player;

	bool checkHit = true;


	//the player already has this key
	if( keyType == Key::RED && player.hasRedKey 
		|| keyType == Key::GREEN && player.hasGreenKey 
		|| keyType == Key::BLUE && player.hasBlueKey )
	{
		checkHit = false;
	}

	if( !dead && checkHit )
	{
		UpdateHitboxes();

		pair<bool,bool> result = PlayerHitMe();
		if( result.first )
		{
		 //no hitlag ever
			if( result.second )
			{
				dead = true;
				receivedHit = NULL;
				if( keyType == Key::RED )
					player.hasRedKey = true;
				else if( keyType == Key::GREEN )
					player.hasGreenKey = true;
				else if( keyType == Key::BLUE )
					player.hasBlueKey = true;
			}	
		}

		if( IHitPlayer() )
		{
			dead = true;
			receivedHit = NULL;
			if( keyType == Key::RED )
				player.hasRedKey = true;
			else if( keyType == Key::GREEN )
				player.hasGreenKey = true;
			else if( keyType == Key::BLUE )
				player.hasBlueKey = true;
		//	cout << "patroller just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}

void Key::UpdatePostPhysics()
{
	

	UpdateSprite();

	if( slowCounter == slowMultiple )
	{
		++frame;
		slowCounter = 1;
	
		if( dead )
		{
			deathFrame++;
		}

	}
	else
	{
		slowCounter++;
	}

	if( frame == 16 * animationFactor )
	{
		frame = 0;
	}

	if( deathFrame == 60 )
	{
		owner->RemoveEnemy( this );
	}
}

void Key::UpdateSprite()
{
	sprite.setTextureRect( ts->GetSubRect( frame / animationFactor ) );
	sprite.setPosition( position.x, position.y );

	/*botDeathSprite.setTexture( *ts_death->texture );
	botDeathSprite.setTextureRect( ts_death->GetSubRect( 1 ) );
	botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2 );
	botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
		position.y + deathVector.y * deathPartingSpeed * deathFrame );

	topDeathSprite.setTexture( *ts_death->texture );
	topDeathSprite.setTextureRect( ts_death->GetSubRect( 0 ) );
	topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
	topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
		position.y + -deathVector.y * deathPartingSpeed * deathFrame );*/
}

void Key::Draw( sf::RenderTarget *target )
{
	if( !dead )
	{
		target->draw( sprite );
	}
	else
	{
		/*target->draw( botDeathSprite );

		if( deathFrame / 3 < 6 )
		{
			
			bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
			bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
			bloodSprite.setPosition( position.x, position.y );
			bloodSprite.setScale( 2, 2 );
			target->draw( bloodSprite );
		}
		
		target->draw( topDeathSprite );*/
	}



}

bool Key::IHitPlayer()
{
	Actor &player = owner->player;
	
	if( hitBody.Intersects( player.hurtBody ) )
	{
		//player.ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void Key::UpdateHitboxes()
{
	hurtBody.globalPosition = position;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position;
	hitBody.globalAngle = 0;
}

//return pair<bool,bool>( hitme, was it with a clone)
pair<bool,bool> Key::PlayerHitMe()
{
	
	Actor &player = owner->player;

	
		


	if( player.currHitboxes != NULL )
	{
		bool hit = false;

		for( list<CollisionBox>::iterator it = player.currHitboxes->begin(); it != player.currHitboxes->end(); ++it )
		{
			if( hurtBody.Intersects( (*it) ) )
			{
				hit = true;
				break;
			}
		}
		

		if( hit )
		{
			receivedHit = player.currHitboxInfo;
			return pair<bool, bool>(true,false);
		}
		
	}

	for( int i = 0; i < player.recordedGhosts; ++i )
	{
		if( player.ghostFrame < player.ghosts[i]->totalRecorded )
		{
			if( player.ghosts[i]->currHitboxes != NULL )
			{
				bool hit = false;
				
				for( list<CollisionBox>::iterator it = player.ghosts[i]->currHitboxes->begin(); it != player.ghosts[i]->currHitboxes->end(); ++it )
				{
					if( hurtBody.Intersects( (*it) ) )
					{
						hit = true;
						break;
					}
				}
		

				if( hit )
				{
					receivedHit = player.currHitboxInfo;
					return pair<bool, bool>(true,true);
				}
			}
			//player.ghosts[i]->curhi
		}
	}

	return pair<bool, bool>(false,false);
}

bool Key::PlayerSlowingMe()
{
	Actor &player = owner->player;
	for( int i = 0; i < player.maxBubbles; ++i )
	{
		if( player.bubbleFramesToLive[i] > 0 )
		{
			if( length( position - player.bubblePos[i] ) <= player.bubbleRadius )
			{
				return true;
			}
		}
	}
	return false;
}

void Key::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void Key::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.forward = forward;
	stored.frame = frame;
	stored.hitlagFrames = hitlagFrames;
	stored.hitstunFrames = hitstunFrames;
	stored.position = position;
	stored.targetNode = targetNode;
}

void Key::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	forward = stored.forward;
	frame = stored.frame;
	hitlagFrames = stored.hitlagFrames;
	hitstunFrames = stored.hitstunFrames;
	position = stored.position;
	targetNode = stored.targetNode;
}