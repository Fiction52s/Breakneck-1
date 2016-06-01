#include "Enemy.h"
#include <iostream>
#include "GameSession.h"

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

HealthFly::HealthFly( GameSession *owner, Vector2i &pos, FlyType fType )
	:Enemy( owner, Enemy::HEALTHFLY ), flyType( fType )
{
	frame = 0;
	animationFactor = 4;
	ts = owner->GetTileset( "healthfly_64x64.png", 64, 64 );

	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	
	position.x = pos.x;
	position.y = pos.y;

	sprite.setPosition( position.x, position.y );

	initHealth = 60;
	health = initHealth;

	//frame = 0;
	//dead = false;

	//animationFactor = 3;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );

	double radius = 20;
	position = V2d( pos.x, pos.y );
	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = radius;
	hurtBody.rh = radius;
	
	caught = false;
	
	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = radius;
	hitBody.rh = radius;

	hurtBody.globalPosition = position;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position;
	hitBody.globalAngle = 0;
}

void HealthFly::HandleEntrant( QuadTreeEntrant *qte )
{
	//empty
}

void HealthFly::UpdatePrePhysics()
{
	

	if( frame == 5 * animationFactor ) //5 frames of animation
	{
		frame = 0;
	}
	//empty
}

void HealthFly::UpdatePhysics()
{
	UpdateHitboxes();
	pair<bool,bool> hitMe = PlayerHitMe();
	bool ihit = IHitPlayer();
	if( ihit || hitMe.first )
	{
		if( !caught )
		{
			caught = true;
			//give player health right here!
			int rows = 10;
			int rowCap = 2 * 5;
			owner->powerBar.Charge( rowCap * rows ); 
			owner->player.desperationMode = false;
		}
		//owner->RemoveEnemy( this );
		//get rid of me
	}

	
	//empty
}

void HealthFly::UpdatePostPhysics()
{
	if( caught )
	{
		//should the one in patroller be in post or pre physics?
		AttemptSpawnMonitor();
		owner->RemoveEnemy( this );
	}

	sprite.setTextureRect( ts->GetSubRect( frame / animationFactor ) );


	if( slowCounter == slowMultiple )
	{
		
		++frame;
		slowCounter = 1;
	
		//if( dead )
		//{
		//	deathFrame++;
		//}

	}
	else
	{
		slowCounter++;
	}

	
	//empty
}

void HealthFly::Draw( sf::RenderTarget *target)
{
	if( monitor != NULL && !suppressMonitor )
	{
		//owner->AddEnemy( monitor );
		CircleShape cs;
		cs.setRadius( 30 );
		cs.setFillColor( COLOR_BLUE );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( position.x, position.y );
		target->draw( cs );
	}
	//cout << "drawing health fly" << endl;
	target->draw( sprite );
	/*sf::CircleShape cs;
	cs.setRadius( 20 );
	cs.setFillColor( Color( 0, 255, 0, 100 ) );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	target->draw( cs );*/
}

void HealthFly::DrawMinimap( sf::RenderTarget *target )
{
	CircleShape cs;
	cs.setFillColor( COLOR_TEAL );
	cs.setRadius( 50 );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	target->draw( cs );

	if( monitor != NULL && !suppressMonitor )
	{
		monitor->miniSprite.setPosition( position.x, position.y );
		target->draw( monitor->miniSprite );
	}
}

bool HealthFly::IHitPlayer()
{
	Actor &player = owner->player;
	
	if( hitBody.Intersects( player.hurtBody ) )
	{
		//player.ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void HealthFly::UpdateHitboxes()
{
	hurtBody.globalPosition = position;
	hitBody.globalPosition = position;
}

std::pair<bool,bool> HealthFly::PlayerHitMe()
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
			sf::Rect<double> qRect( position.x - hurtBody.rw,
			position.y - hurtBody.rw, hurtBody.rw * 2, 
			hurtBody.rw * 2 );
			owner->specterTree->Query( this, qRect );

			if( !specterProtected )
			{
				receivedHit = player.currHitboxInfo;
				return pair<bool, bool>(true,false);
			}
			else
			{
				return pair<bool, bool>(false,false);
			}
			
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

bool HealthFly::PlayerSlowingMe()
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

void HealthFly::DebugDraw(sf::RenderTarget *target)
{
	hurtBody.DebugDraw( target );
	hitBody.DebugDraw( target );
}

void HealthFly::SaveEnemyState()
{
}

void HealthFly::LoadEnemyState()
{
}

void HealthFly::ResetEnemy()
{
	caught = false;
	frame = 0;
	//owner->
	//cout << "resetting monitor" << endl;
}