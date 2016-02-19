#include "Enemy.h"
#include <iostream>
#include "GameSession.h"

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

HealthFly::HealthFly( GameSession *owner, FlyType fType )
	:Enemy( owner, Enemy::HEALTHFLY ), flyType( fType )
{
	double radius = 20;
	
	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = radius;
	hurtBody.rh = radius;
	
	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = radius;
	hitBody.rh = radius;
}

void HealthFly::HandleEntrant( QuadTreeEntrant *qte )
{
	//empty
}

void HealthFly::UpdatePrePhysics()
{
	//empty
}

void HealthFly::UpdatePhysics()
{
	UpdateHitboxes();
	pair<bool,bool> hitMe = PlayerHitMe();
	bool ihit = IHitPlayer();
	if( ihit || hitMe.first )
	{
		//give player health right here!

		owner->RemoveEnemy( this );
		//get rid of me
	}

	
	//empty
}

void HealthFly::UpdatePostPhysics()
{
	//empty
}

void HealthFly::Draw( sf::RenderTarget *target)
{
	sf::CircleShape cs;
	cs.setRadius( 20 );
	cs.setFillColor( Color( 0, 255, 0, 100 ) );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	target->draw( cs );
}

void HealthFly::DrawMinimap( sf::RenderTarget *target )
{
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
	//owner->
	//cout << "resetting monitor" << endl;
}