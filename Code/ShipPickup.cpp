#include "ShipPickup.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Actor.h"

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

void ShipPickup::UpdateSpriteFromParams(ActorParams *ap)
{
	if (ap->posInfo.IsAerial())
	{
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
		sprite.setPosition(editParams->GetFloatPos());
		sprite.setRotation(0);
	}
}

ShipPickup::ShipPickup(ActorParams * ap)
		:Enemy( EnemyType::EN_SHIPPICKUP, ap)
{
	ShipPickupParams *sParams = (ShipPickupParams*)ap;
	facingRight = sParams->facingRight;

	ts = sess->GetSizedTileset( "Ship/ship_pickup_256x180.png");
	sprite.setTexture( *ts->texture );
	
	SetOffGroundHeight(67);

	//SetCurrPosInfo(startPosInfo);
	
	SetNumActions(Count);
	SetEditorActions(IDLE, IDLE, 0);


	actionLength[IDLE] = 16;
	actionLength[FOUND] = 16;

	animFactor[IDLE] = 4;
	animFactor[FOUND] = 4;

	ResetEnemy();

	SetSpawnRect();
}

void ShipPickup::ResetEnemy()
{
	SetCurrPosInfo(startPosInfo);

	frame = 0;
	dead = false;
	action = IDLE;

	UpdateSprite();
}

void ShipPickup::ProcessState()
{
	if (action == DONE)
	{
		return;
	}

	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case IDLE:
			frame = 0;
			break;
		case FOUND:
			//action = DONE;
			frame = 0;
			break;
		}
	}
}

void ShipPickup::UpdateEnemyPhysics()
{
	Actor *player = sess->GetPlayer( 0 );

	Edge *ground = currPosInfo.GetEdge();
	double edgeQuantity = currPosInfo.GetQuant();
	if( player->ground == currPosInfo.GetEdge() && action == IDLE)
	{
		if( ground->Normal().y == -1 )
		{
			if( abs( ( player->edgeQuantity + player->offsetX ) - edgeQuantity ) < 5 )
			{
				player->ShipPickupPoint( edgeQuantity, facingRight );
				action = FOUND;
				//frame = 0; //keeps the same value since its just looping the same anim atm
			}
		}
		else
		{
			if( abs( player->edgeQuantity - edgeQuantity ) < 5 )
			{
				player->ShipPickupPoint( edgeQuantity, facingRight );
				action = FOUND;
				//frame = 0;
			}
		}
	}
}


void ShipPickup::EnemyDraw(sf::RenderTarget *target )
{
	//if( action != DONE )
	target->draw(sprite);
}

void ShipPickup::DrawMinimap( sf::RenderTarget *target )
{
}

void ShipPickup::UpdateSprite()
{
	if (action == DONE)
		return;

	int f;
	switch (action)
	{
	case IDLE:
		f = frame / animFactor[IDLE];
		break;
	case FOUND:
		f = frame / animFactor[FOUND];//( frame / animFactor[FOUND] ) + actionLength[IDLE];
		break;
	}
	sprite.setTextureRect( ts->GetSubRect( f ) );
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height/2);// / 2 );
	sprite.setPosition(currPosInfo.GetPositionF());
	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
	//sprite.setPosition( position.x, position.y );
}

void ShipPickup::DebugDraw(sf::RenderTarget *target)
{
}

int ShipPickup::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void ShipPickup::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void ShipPickup::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}