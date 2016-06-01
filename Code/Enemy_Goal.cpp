#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

Goal::Goal( GameSession *owner, Edge *g, double q )
		:Enemy( owner, EnemyType::GOAL ), ground( g ), edgeQuantity( q ), dead( false )
{
	double height = 160;
	double width = 128;
	ts = owner->GetTileset( "goal_128x160.png", width, height );
	ts_mini = owner->GetTileset( "goal_minimap_32x40.png", 32, 40 );
	sprite.setTexture( *ts->texture );
	
	miniSprite.setTexture( *ts_mini->texture );
	miniSprite.setScale( 10, 10 );
	miniSprite.setOrigin( miniSprite.getLocalBounds().width / 2, miniSprite.getLocalBounds().height / 2 );
	

	V2d gPoint = g->GetPoint( edgeQuantity );


	gn = g->Normal();
	angle = atan2( gn.x, -gn.y );

	position = gPoint + gn * height / 2.0;

	miniSprite.setPosition( position.x, position.y );

	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );
	sprite.setPosition( gPoint.x, gPoint.y );
	sprite.setRotation( angle / PI * 180 );

	
	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 100;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 0;

	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 40;
	hurtBody.rh = 40;

	double angle = 0;
		
	angle = atan2( gn.x, -gn.y );
		
	hurtBody.globalAngle = angle;

	hurtBody.globalPosition = position + 
		V2d( hitBody.offset.x * cos( hurtBody.globalAngle ) + hurtBody.offset.y * sin( hurtBody.globalAngle ), 
		hitBody.offset.x * -sin( hurtBody.globalAngle ) + hurtBody.offset.y * cos( hurtBody.globalAngle ) );

	frame = 0;
	deathFrame = 0;
	animationFactor = 7;
	slowCounter = 1;
	slowMultiple = 1;

	spawnRect = sf::Rect<double>( gPoint.x - 160 / 2, gPoint.y - 160 / 2, 160, 160 );
}

void Goal::ResetEnemy()
{
	frame = 0;
	deathFrame = 0;
}

void Goal::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void Goal::UpdatePrePhysics()
{
	if( frame == 0 )
	{
		
	}
}

void Goal::UpdatePhysics()
{
	specterProtected = false;
	if( !dead )
	{
		UpdateHitboxes();

		pair<bool, bool> result = PlayerHitMe();
		if( result.first )
		{
			if( !result.second )
			{
				owner->Pause( 6 );
			}
			dead = true;
			receivedHit = NULL;

			
		}

		if( IHitPlayer() )
		{
		//	cout << "patroller just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}

}

void Goal::UpdatePostPhysics()
{
	
	

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

	//if( frame == 4 * animationFactor )
	//{
	frame = 0;
	//}

	if( deathFrame == 60 )
	{
		if( owner->player.record == 0 )
		{
			cout << "GAME OVER" << endl;
			owner->goalDestroyed = true;
		}
		owner->RemoveEnemy( this );
	}

	UpdateSprite();
}

void Goal::Draw(sf::RenderTarget *target )
{
	target->draw( sprite );
}

void Goal::DrawMinimap( sf::RenderTarget *target )
{
	/*CircleShape cs;
	cs.setRadius( 80 );
	cs.setFillColor(  );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );*/
	target->draw( miniSprite );
}

bool Goal::IHitPlayer()
{
	Actor &player = owner->player;
	

	/*if( currBullet->hitBody.Intersects( player.hurtBody ) )
		{
			player.ApplyHit( bulletHitboxInfo );
			return true;
		}
	*/
	
	return false;
}

pair<bool, bool> Goal::PlayerHitMe()
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

bool Goal::PlayerSlowingMe()
{
	return false;
}

void Goal::UpdateSprite()
{
	sprite.setTextureRect( ts->GetSubRect( frame / animationFactor ) );
}

void Goal::DebugDraw(sf::RenderTarget *target)
{
}

void Goal::UpdateHitboxes()
{
}

bool Goal::ResolvePhysics( sf::Vector2<double> vel )
{
	return false;
}

void Goal::SaveEnemyState()
{
}

void Goal::LoadEnemyState()
{
}

