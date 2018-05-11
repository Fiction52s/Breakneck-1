#include "ShipPickup.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

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

ShipPickup::ShipPickup( GameSession *owner, Edge *g, double q, bool p_facingRight )
		:Enemy( owner, EnemyType::EN_SHIPPICKUP, false, 1 ), ground( g ), edgeQuantity( q ),
	facingRight( p_facingRight )
{
	
	initHealth = 40;
	health = initHealth;

	double height = 48;
	ts = owner->GetTileset( "Ship/shippickup_128x128.png", 128, height );
	sprite.setTexture( *ts->texture );
	
	V2d gPoint = g->GetPoint(edgeQuantity);
	

	receivedHit = NULL;

	V2d gn = g->Normal();
	float angle = atan2( gn.x, -gn.y );

	position = gPoint;// +gn * height / 2.0;

	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);// / 2 );
	sprite.setPosition( gPoint.x, gPoint.y );
	sprite.setRotation( angle / PI * 180 );
	

	frame = 0;
	slowCounter = 1;
	slowMultiple = 1;

	spawnRect = sf::Rect<double>( gPoint.x - 64, gPoint.y - 64, 64 * 2, 64 * 2 );
}

void ShipPickup::ResetEnemy()
{
	health = initHealth;
	frame = 0;
	dead = false;
	receivedHit = NULL;
	slowCounter = 1;
	slowMultiple = 1;
}

void ShipPickup::ProcessState()
{
}

void ShipPickup::UpdateEnemyPhysics()
{
	Actor *player = owner->GetPlayer( 0 );

	if( player->ground == ground )
	{
		if( ground->Normal().y == -1 )
		{
			if( abs( ( player->edgeQuantity + player->offsetX ) - edgeQuantity ) < 5 )
			{
				player->ShipPickupPoint( edgeQuantity, facingRight );
			}
		}
		else
		{
			if( abs( player->edgeQuantity - edgeQuantity ) < 5 )
			{
				player->ShipPickupPoint( edgeQuantity, facingRight );
			}
		}
	}
}


void ShipPickup::EnemyDraw(sf::RenderTarget *target )
{
	target->draw(sprite);
}

void ShipPickup::DrawMinimap( sf::RenderTarget *target )
{
}

void ShipPickup::UpdateSprite()
{
	sprite.setTextureRect( ts->GetSubRect( 0 ) );//frame / animationFactor ) );
	sprite.setPosition( position.x, position.y );
}

void ShipPickup::DebugDraw(sf::RenderTarget *target)
{
}