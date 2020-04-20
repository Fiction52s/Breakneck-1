#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_RaceFightTarget.h"

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

RaceFightTarget::RaceFightTarget( ActorParams *ap)//Vector2i &pos )
	:Enemy( EnemyType::EN_RACEFIGHTTARGET, ap)//., false, 1 )
{
	receivedHit = NULL;
	position = ap->GetPosition();
	//spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	frame = 0;

	gameTimeP1Hit = -1;
	gameTimeP2Hit = -1;

	animationFactor = 10;

	ts = sess->GetTileset( "Enemies/target_224x224.png", 224, 224 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition(position.x, position.y );
	//position.x = 0;
	//position.y = 0;

	BasicCircleHurtBodySetup(40, position);
	

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

void RaceFightTarget::ResetEnemy()
{
	gameTimeP1Hit = -1;
	gameTimeP2Hit = -1;

	dead = false;
	action = NEUTRAL;
	frame = 0;
	
	frame = 0;
	receivedHit = NULL;

	UpdateHitboxes();

	UpdateSprite();
	pPrev = NULL;
	pNext = NULL;
	p2Prev = NULL;
	p2Next = NULL;

	SetHurtboxes(&hurtBody, 0);
}



void RaceFightTarget::ProcessState()
{
	frame = 0;
}

void RaceFightTarget::HandleHitAndSurvive()
{

}

void RaceFightTarget::UpdateSprite()
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

void RaceFightTarget::EnemyDraw( sf::RenderTarget *target )
{
	target->draw( sprite );
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
	}
}