#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Shark.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_RED Color( 0xff, 0x22, 0 )

Shark::Shark( GameSession *owner, bool p_hasMonitor, Vector2i pos, int p_level )
	:Enemy( owner, EnemyType::EN_SHARK, p_hasMonitor, 1 ), approachAccelBez( 1,.01,.86,.32 ) 
{
	level = p_level;

	switch (level)
	{
	case 1:
		scale = 1.0;
		break;
	case 2:
		scale = 2.0;
		maxHealth += 2;
		break;
	case 3:
		scale = 3.0;
		maxHealth += 5;
		break;
	}

	actionLength[WAKEUP] = 30;
	actionLength[APPROACH] = 2;
	actionLength[CIRCLE] = 2;
	actionLength[RUSH] = 7;
	actionLength[FINALCIRCLE] = 2;

	animFactor[WAKEUP] = 1;
	animFactor[FINALCIRCLE] = 1;
	animFactor[APPROACH] = 1;
	animFactor[CIRCLE] = 1;
	animFactor[RUSH] = 5;

	//offsetPlayer 
	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	origPosition = position;

	circleFrames = 120;
	wakeCap = 45;

	V2d dirFromPlayer = normalize( owner->GetPlayerPos( 0 ) - position );
	double fromPlayerAngle =  atan2( dirFromPlayer.y, dirFromPlayer.x ) + PI;

	circleSeq.AddRadialMovement(V2d(0, 0), 1, 0, 2 * PI,
		true, V2d(1, 1), 0, CubicBezier(.57, .3, .53, .97), circleFrames);
	
	circleSeq.InitMovementDebug();


	rushSeq.AddLineMovement( V2d( 0, 0 ), 
		V2d( 1, 0 ), CubicBezier( 0, 0, 1, 1 ), actionLength[RUSH] * animFactor[RUSH] );

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	speed = 20;

	ts_circle = owner->GetTileset( "Enemies/shark_circle_256x256.png", 256, 256 );
	ts_bite = owner->GetTileset( "Enemies/shark_bite_256x256.png", 256, 256 );
	ts_death = owner->GetTileset( "Enemies/shark_death_256x256.png", 256, 256 );
	sprite.setTexture( *ts_circle->texture );
	sprite.setTextureRect( ts_circle->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	SetupBodies(1, 1);
	AddBasicHurtCircle(16);
	AddBasicHitCircle(16);
	hitBody->hitboxInfo = hitboxInfo;

	facingRight = true;

	UpdateHitboxes();
}

void Shark::ResetEnemy()
{
	circleCounter = 0;
	wakeCounter = 0;
	action = WAKEUP;
	latchStartAngle = 0;
	latchedOn = false;
	circleSeq.Reset();
	dead = false;
	frame = 0;
	basePos = origPosition;
	position = basePos;

	receivedHit = NULL;
	
	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);

	UpdateHitboxes();
	UpdateSprite();
	
}

void Shark::ProcessState()
{
	double cs = cos( latchStartAngle );
	double sn = sin( latchStartAngle );

	V2d truePosOffset( circleSeq.position.x * cs - 
		circleSeq.position.y * sn, 
		circleSeq.position.x * sn + circleSeq.position.y * cs );
	truePosOffset *= length( offsetPlayer );

	V2d playerPos = owner->GetPlayerPos(0);

	if( action == WAKEUP )
	{
		if( WithinDistance( position, playerPos, 400))
		{
			wakeCounter++;
			if( wakeCounter == wakeCap )
			{
				circleCounter = 0;
				action = CIRCLE;
				frame = 0;
				latchedOn = true;
				offsetPlayer = basePos - playerPos;
				origOffset = offsetPlayer;
				V2d offsetDir = normalize( offsetPlayer );
				latchStartAngle = atan2( offsetDir.y, offsetDir.x );
				

				//testing

				circleSeq.Update( slowMultiple );
				basePos = playerPos;
			}
		}
		else
		{
			wakeCounter--;
			if( wakeCounter < 0 )
				wakeCounter = 0;
		}	
	}
	else if( action == CIRCLE )
	{	
		if( owner->GetPlayerHitstunFrames( 0 ) > 0  )
		{
			action = FINALCIRCLE;
			attackOffset = truePosOffset;

		}
		frame = 0;
	}
	else if( action == FINALCIRCLE && frame == circleFrames )
	{
		action = RUSH;
		frame = 0;
		rushSeq.Reset();
		rushSeq.currMovement->start = truePosOffset;
		rushSeq.currMovement->end = -truePosOffset;
		rushSeq.Update( slowMultiple );
	}
	else if( action == RUSH )
	{
		if( rushSeq.currMovement == NULL )
		{
			action = CIRCLE;
			circleCounter = 0;
			truePosOffset = -truePosOffset;
			offsetPlayer = truePosOffset;
			V2d offsetDir = normalize( truePosOffset );
			latchStartAngle = atan2( offsetDir.y, offsetDir.x );
			circleSeq.Reset();
			circleSeq.Update( slowMultiple );
		}
	}
		

	if( circleCounter == 10 )
	{
		wakeCounter = 0;
		action = WAKEUP;
		latchStartAngle = 0;
		latchedOn = false;
		circleSeq.Reset();
		basePos = position;
		circleCounter = 0;
		frame = 0;
	}
}

V2d Shark::GetCircleOffset()
{
	double cs = cos(latchStartAngle);
	double sn = sin(latchStartAngle);

	V2d truePosOffset(circleSeq.position.x * cs -
		circleSeq.position.y * sn,
		circleSeq.position.x * sn + circleSeq.position.y * cs);
	return truePosOffset;
}

void Shark::UpdateEnemyPhysics()
{
	double cs = cos( latchStartAngle );
	double sn = sin( latchStartAngle );

	V2d truePosOffset( circleSeq.position.x * cs - 
		circleSeq.position.y * sn, 
		circleSeq.position.x * sn + circleSeq.position.y * cs );


	if( (action == CIRCLE || action == FINALCIRCLE) && latchedOn )
	{
		position = basePos + truePosOffset * length( offsetPlayer );

		circleSeq.Update( slowMultiple, NUM_MAX_STEPS / numPhysSteps );
		if( circleSeq.currMovement == NULL )
		{
			circleSeq.Reset();
			circleSeq.Update( slowMultiple, NUM_MAX_STEPS / numPhysSteps);
			++circleCounter;
		}
	}
	else if( action == RUSH )
	{
		position = basePos + rushSeq.position;
		rushSeq.Update( slowMultiple, NUM_MAX_STEPS / numPhysSteps);
	}
}

void Shark::UpdateSprite()
{
	if (latchedOn)
	{
		V2d playerPos = owner->GetPlayerPos(0);
		basePos = playerPos;
		if ((action == CIRCLE || action == FINALCIRCLE))
		{
			position = basePos + GetCircleOffset() * length(offsetPlayer);
		}
		else if (action == RUSH)
		{
			position = basePos + rushSeq.position;
		}
	}

	switch( action )
	{
	case WAKEUP:
		{
		sprite.setTexture( *ts_circle->texture );
		IntRect ir = ts_circle->GetSubRect( 0 );
		//if( 
		sprite.setTextureRect( ir );
		sprite.setOrigin( sprite.getLocalBounds().width / 2, 
				sprite.getLocalBounds().height / 2 );
		sprite.setRotation( 0 );
			
			//testColor = Color::White;
		}
		break;
	case APPROACH:
		{
		sprite.setTexture( *ts_circle->texture );
		IntRect ir = ts_circle->GetSubRect( 0 );
		//if( 
		sprite.setTextureRect( ir );
		sprite.setOrigin( sprite.getLocalBounds().width / 2, 
				sprite.getLocalBounds().height / 2 );
		sprite.setRotation( 0 );
		//testColor = Color::Green;
		}
		break;
	case FINALCIRCLE:
		//sprite.setColor( Color::Blue );
		//break;
	case CIRCLE:
		{
		//sprite.setColor( Color::Blue );
		sprite.setTexture( *ts_circle->texture );
		int trueFrame = 0;
			
			
			

		double div = 2 * PI / 12.0;

			
		double cs = cos( latchStartAngle );
		double sn = sin( latchStartAngle );
		V2d truePosOffset( circleSeq.position.x * cs - 
			circleSeq.position.y * sn, 
			circleSeq.position.x * sn + circleSeq.position.y * cs );
		V2d normOffset = normalize( truePosOffset );
		double angle = atan2( normOffset.x, -normOffset.y );
		//angle -= PI / 4;
		if( angle < 0 )
			angle += PI * 2;
			
		int mults = angle / div;
		//cout << "mults: " << mults << ", angle: " << angle << endl;
			
		IntRect ir = ts_circle->GetSubRect( mults );
		sprite.setTextureRect( ir );
		sprite.setOrigin( sprite.getLocalBounds().width / 2, 
				sprite.getLocalBounds().height / 2 );
		sprite.setRotation( 0 );
		}
		break;
	case RUSH:
		{
			sprite.setTexture( *ts_bite->texture );

			//int maxFrame = actionLength[RUSH] - 1;
			//int f = frame / animFactor[RUSH];
			IntRect ir = ts_bite->GetSubRect( frame / animFactor[RUSH] );
			
			if( attackOffset.x <= 0 )
			{
			}
			else
			{
				ir.top += ir.height;
				ir.height = -ir.height;
				//ir.left += ir.width;
				//ir.width = -ir.width;
					
			}
			sprite.setTextureRect( ir );

			V2d normOffset = normalize( -attackOffset );
			double angle = atan2( normOffset.y, normOffset.x );//atan2( normOffset.x, -normOffset.y );
			if( angle < 0 )
				angle += PI * 2;
			sprite.setOrigin( sprite.getLocalBounds().width / 2, 
				sprite.getLocalBounds().height / 2 );
			sprite.setRotation( angle / PI * 180.0 );


		}
		//testColor = Color::Red;
		break;
	}

	sprite.setPosition( position.x, position.y );
}

void Shark::EnemyDraw( sf::RenderTarget *target )
{
	DrawSpriteIfExists(target, sprite);
}