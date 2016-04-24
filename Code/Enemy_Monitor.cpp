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

Monitor::Monitor( GameSession *owner, MonitorType mType, Enemy *e_host )
	:Enemy( owner, Enemy::GATEMONITOR ), monitorType( mType )
{
	ts = owner->GetTileset( "monitor.png", 64, 64 );
	ts_mini = owner->GetTileset( "monitor_minimap_64x64.png", 64, 64 );
	
	IntRect subRect = ts->GetSubRect( 0 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( subRect );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );

	miniSprite.setTexture( *ts_mini->texture );
	miniSprite.setScale( 8, 8 );
	miniSprite.setOrigin( miniSprite.getLocalBounds().width / 2, miniSprite.getLocalBounds().height / 2 );
	
	
	
	host = e_host;
	double radius = 40;
	respawnSpecial = true; //doesnt matter. set in frame
	
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

	frame = 0;
	animationFactor = 3;
	
}

void Monitor::HandleEntrant( QuadTreeEntrant *qte )
{
	//empty
}

void Monitor::UpdatePrePhysics()
{
	if( frame == 22 * animationFactor )
	{
		frame = 0;
	}
	//empty
}

void Monitor::UpdatePhysics()
{
	UpdateHitboxes();

	int gateType = (int)monitorType + 1;

	if( owner->player.hasKey[ gateType])
	{
		return;
	}

	pair<bool,bool> hitMe = PlayerHitMe();
	bool ihit = IHitPlayer();
	if( ihit || hitMe.first )
	{
		//cout << "got the monitor!" << endl;
		owner->player.hasKey[gateType] = true;
		/*switch( monitorType )
		{
		case BLUE:
			owner->player.hasBlueKey = true;
			break;
		case GREEN:
			break;
		case YELLOW:
			break;
		case ORANGE:
			break;
		case RED:
			break;
		case MAGENTA:
			break;
		case WHITE:
			break;
		default:
			assert( false );
		}*/
		owner->RemoveEnemy( this );
		//get rid of me
	}

	
	//empty
}

void Monitor::UpdatePostPhysics()
{
	IntRect subRect = ts->GetSubRect( frame / animationFactor );
	sprite.setTextureRect( subRect );
	sprite.setPosition( position.x, position.y );

	miniSprite.setPosition( position.x, position.y );
	//no slowing for now
	++frame;
	//empty
}

void Monitor::Draw( sf::RenderTarget *target)
{
	/*sf::CircleShape cs;
	cs.setRadius( 40 );
	cs.setFillColor( Color( 0, 255, 0, 100 ) );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	target->draw( cs );*/

	target->draw( sprite );
}

void Monitor::DrawMinimap( sf::RenderTarget *target )
{
	
	target->draw( miniSprite );
	/*CircleShape cs;
	cs.setRadius( 80 );
	cs.setFillColor( COLOR_BLUE );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	target->draw( cs );*/
}

bool Monitor::IHitPlayer()
{
	Actor &player = owner->player;
	
	if( hitBody.Intersects( player.hurtBody ) )
	{
		//player.ApplyHit( hitboxInfo );
		return true;
	}
	return false;
}

void Monitor::UpdateHitboxes()
{
	hurtBody.globalPosition = position;
	hitBody.globalPosition = position;
}

std::pair<bool,bool> Monitor::PlayerHitMe()
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

bool Monitor::PlayerSlowingMe()
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

void Monitor::DebugDraw(sf::RenderTarget *target)
{
	hurtBody.DebugDraw( target );
	hitBody.DebugDraw( target );
}

void Monitor::SaveEnemyState()
{
}

void Monitor::LoadEnemyState()
{
}

void Monitor::ResetEnemy()
{
	//owner->
	cout << "resetting monitor" << endl;
}