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

FootTrap::FootTrap( GameSession *owner, Edge *g, double q )
		:Enemy( owner, EnemyType::FOOTTRAP ), ground( g ), edgeQuantity( q ), dead( false )
{
	initHealth = 40;
	health = initHealth;

	ts = owner->GetTileset( "foottrap.png", 160, 80 );
	sprite.setTexture( *ts->texture );
	
	V2d gPoint = g->GetPoint( edgeQuantity );
	//cout << "player " << owner->player.position.x << ", " << owner->player.position.y << endl;
	//cout << "gPoint: " << gPoint.x << ", " << gPoint.y << endl;
	position = gPoint;

	receivedHit = NULL;

	gn = g->Normal();
	angle = atan2( gn.x, -gn.y );

	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );
	sprite.setPosition( gPoint.x, gPoint.y );
	sprite.setRotation( angle / PI * 180 );

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
	
	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 100;
	hitboxInfo->drainX = .5;
	hitboxInfo->drainY = .5;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 0;

	frame = 0;
	deathFrame = 0;
	animationFactor = 7;
	slowCounter = 1;
	slowMultiple = 1;

	spawnRect = sf::Rect<double>( gPoint.x - 80, gPoint.y - 80, 80 * 2, 80 * 2 );

	ts_death = owner->GetTileset( "foottrapdeath.png", 160, 80 );
	//ts_top = owner->GetTileset( "patroldeathtop.png", 32, 32 );

	deathPartingSpeed = .3;
	deathVector = V2d( 1, -1 );

	ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	bloodSprite.setTexture( *ts_testBlood->texture );
}

void FootTrap::ResetEnemy()
{
	health = initHealth;
	frame = 0;
	deathFrame = 0;
	dead = false;
}

void FootTrap::HandleEntrant( QuadTreeEntrant *qte )
{
}

void FootTrap::UpdatePrePhysics()
{
	if( !dead && receivedHit != NULL )
	{	
		
		//gotta factor in getting hit by a clone
		health -= 20;
		cout << "damaging: " << health << endl;
		if( health <= 0 )
			dead = true;

		receivedHit = NULL;
	}

	if( frame == 0 )
	{
		
	}
}

void FootTrap::UpdatePhysics()
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

	if( !dead && receivedHit == NULL )
	{
		UpdateHitboxes();

		pair<bool, bool> result = PlayerHitMe();
		if( result.first )
		{
			//cout << "hit here!" << endl;
			//triggers multiple times per frame? bad?
			owner->player.test = true;
			owner->player.currAttackHit = true;
			owner->player.flashColor = COLOR_BLUE;
			owner->player.flashFrames = 5;
			owner->player.swordShader.setParameter( "energyColor", COLOR_BLUE );
			owner->powerBar.Charge( 2 * 6 * 1 );

			if( owner->player.ground == NULL && owner->player.velocity.y > 0 )
			{
				owner->player.velocity.y = 4;//.5;
			}
		}

		if( IHitPlayer() )
		{
		//	cout << "patroller just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}



void FootTrap::UpdatePostPhysics()
{
	if( receivedHit != NULL )
		owner->Pause( 5 );

	

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

	if( frame == 7 * animationFactor )
	{
		frame = 0;
	}

	if( deathFrame == 60 )
	{
		owner->RemoveEnemy( this );
	}

	UpdateSprite();
}

void FootTrap::Draw(sf::RenderTarget *target )
{
	if( !dead )
	{
		target->draw( sprite );
	}
	else
	{
		target->draw( botDeathSprite );

		if( deathFrame / 3 < 6 )
		{
			
			bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
			bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
			bloodSprite.setPosition( position.x, position.y );
			bloodSprite.setScale( 2, 2 );
			target->draw( bloodSprite );
		}
		
		target->draw( topDeathSprite );
	}
}

bool FootTrap::IHitPlayer()
{
	Actor &player = owner->player;
	
	if( hitBody.Intersects( player.hurtBody ) )
	{
		player.ApplyHit( hitboxInfo );
		return true;
	}
	
	return false;
}

pair<bool, bool> FootTrap::PlayerHitMe()
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

bool FootTrap::PlayerSlowingMe()
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

void FootTrap::UpdateSprite()
{
	sprite.setTextureRect( ts->GetSubRect( frame / animationFactor ) );
	sprite.setPosition( position.x, position.y );

	botDeathSprite.setTexture( *ts_death->texture );
	botDeathSprite.setTextureRect( ts_death->GetSubRect( 1 ) );
	botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height );
	botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
		position.y + deathVector.y * deathPartingSpeed * deathFrame );

	topDeathSprite.setTexture( *ts_death->texture );
	topDeathSprite.setTextureRect( ts_death->GetSubRect( 0 ) );
	topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height );
	topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
		position.y + -deathVector.y * deathPartingSpeed * deathFrame );
	//sprite.setTextureRect( ts->GetSubRect( frame / animationFactor ) );
}

void FootTrap::DebugDraw(sf::RenderTarget *target)
{
	if( !dead )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void FootTrap::UpdateHitboxes()
{
	hurtBody.globalPosition = position + gn * 10.0;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position + gn * 10.0;
	hitBody.globalAngle = 0;
}

bool FootTrap::ResolvePhysics( sf::Vector2<double> vel )
{
	return false;
}

void FootTrap::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.frame = frame;
//	stored.hitlagFrames = hitlagFrames;
//	stored.hitstunFrames = hitstunFrames;
}

void FootTrap::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	frame = stored.frame;
//	hitlagFrames = stored.hitlagFrames;
//	hitstunFrames = stored.hitstunFrames;
}

