#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Shark.h"
#include "Actor.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_RED Color( 0xff, 0x22, 0 )

Shark::Shark( ActorParams *ap )
	:Enemy( EnemyType::EN_SHARK, ap), approachAccelBez( 1,.01,.86,.32 ) 
{
	SetNumActions(A_Count);
	SetEditorActions(APPROACH, APPROACH, 0);

	circleFrames = 120;
	wakeCap = 45;

	SetLevel(ap->GetLevel());
	actionLength[NEUTRAL] = 2;
	actionLength[WAKEUP] = 30;
	actionLength[APPROACH] = 2;
	actionLength[CIRCLE] = circleFrames;
	actionLength[RUSH] = 7;
	actionLength[FINALCIRCLE] = circleFrames;

	animFactor[WAKEUP] = 1;
	animFactor[FINALCIRCLE] = 1;
	animFactor[APPROACH] = 1;
	animFactor[CIRCLE] = 1;
	animFactor[RUSH] = 5;

	

	//V2d dirFromPlayer = normalize( owner->GetPlayerPos( 0 ) - position );
	//double fromPlayerAngle =  atan2( dirFromPlayer.y, dirFromPlayer.x ) + PI;

	circleMovement = circleSeq.AddRadialMovement(V2d(0, 0), V2d(1, 0), 2 * PI, true,
		CubicBezier(.57, .3, .53, .97), circleFrames);

	//circleMovement->Set(V2d(), V2d(1, 0), 2 * PI, true, CubicBezier(), circleFrames);
	circleSeq.InitMovementDebug();


	rushSeq.AddLineMovement( V2d( 0, 0 ), 
		V2d( 1, 0 ), CubicBezier( 0, 0, 1, 1 ), actionLength[RUSH] * animFactor[RUSH] );
	
	speed = 20;

	ts_circle = sess->GetSizedTileset( "Enemies/W5/shark_circle_256x256.png");
	ts_bite = sess->GetSizedTileset( "Enemies/W5/shark_bite_256x256.png");
	ts_death = sess->GetSizedTileset( "Enemies/W5/shark_death_256x256.png");

	sprite.setTexture( *ts_circle->texture );
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 5;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 4;
	hitboxInfo->kbDir = V2d(1, 0);
	hitboxInfo->hType = HitboxInfo::RED;

	BasicCircleHurtBodySetup(48);
	BasicCircleHitBodySetup(48);

	hitBody.hitboxInfo = hitboxInfo;

	UpdateHitboxes();

	ResetEnemy();
}

void Shark::SetLevel(int lev)
{
	level = lev;

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
}

void Shark::ResetEnemy()
{
	circleCounter = 0;
	wakeCounter = 0;
	action = WAKEUP;
	latchStartAngle = 0;
	latchedOn = false;
	circleSeq.Reset();

	basePos = GetPosition();

	frame = 0;
	
	DefaultHitboxesOn();
	DefaultHurtboxesOn();

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
	V2d playerPos = sess->GetPlayerPos();

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case NEUTRAL:
			
			break;
		case WAKEUP:
		{
			circleCounter = 0;
			action = CIRCLE;
			frame = 0;
			latchedOn = true;
			offsetPlayer = basePos - playerPos;
			origOffset = offsetPlayer;
			V2d offsetDir = normalize(offsetPlayer);
			latchStartAngle = GetVectorAngleCW(offsetDir);//atan2(offsetDir.y, offsetDir.x);

			circleMovement->Set(V2d(), V2d( 1, 0 ), 2 * PI, true, CubicBezier(), circleFrames );
			circleMovement->InitDebugDraw();
			circleSeq.Reset();

			//circleMovement->radius = 1;//length(origOffset);
			//circleMovement->start = offsetDir;
			//testing

			circleSeq.Update(slowMultiple);
			basePos = playerPos;
			break;
		}
		case APPROACH:
			break;
		case CIRCLE:
			circleSeq.Reset();
			circleSeq.Update(slowMultiple, NUM_MAX_STEPS / numPhysSteps);
			++circleCounter;
			break;
		case FINALCIRCLE:
		{
			action = RUSH;
			frame = 0;
			rushSeq.Reset();
			rushSeq.currMovement->start = truePosOffset;
			rushSeq.currMovement->end = -truePosOffset;
			rushSeq.Update(slowMultiple);
			break;
		}
		case RUSH:
		{
			assert(rushSeq.currMovement == NULL);
			action = CIRCLE;
			circleCounter = 0;
			truePosOffset = -truePosOffset;
			offsetPlayer = truePosOffset;
			V2d offsetDir = normalize(truePosOffset);
			latchStartAngle = atan2(offsetDir.y, offsetDir.x);
			circleSeq.Reset();
			circleSeq.Update(slowMultiple);
		}
			break;
		}
	}

	switch (action)
	{
	case NEUTRAL:
		if (PlayerDist() < 1000)
		{
			action = WAKEUP;
			frame = 0;
		}
		break;
	case WAKEUP:
		break;
	case APPROACH:
		break;
	case CIRCLE:
		if (sess->GetPlayer(0)->hitstunFrames > 0)
		{
			action = FINALCIRCLE;
			frame = 0;
			attackOffset = truePosOffset;
		}
		break;
	case FINALCIRCLE:
		break;
	case RUSH:
		break;
	}

	/*if( circleCounter == 10 )
	{
		wakeCounter = 0;
		action = WAKEUP;
		latchStartAngle = 0;
		latchedOn = false;
		circleSeq.Reset();
		basePos = position;
		circleCounter = 0;
		frame = 0;
	}*/
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
		currPosInfo.position = basePos + truePosOffset * length( offsetPlayer );

		circleSeq.Update(slowMultiple, NUM_MAX_STEPS / numPhysSteps);

		//if (numPhysSteps == 1)
		//{
		//	circleSeq.Update(slowMultiple, 10);
		//}
		//else
		//{
		//	circleSeq.Update(slowMultiple);
		//}

		////circleSeq.Update( slowMultiple, NUM_MAX_STEPS / numPhysSteps );
		//if( circleSeq.currMovement == NULL )
		//{
		//	circleSeq.Reset();
		//	circleSeq.Update( slowMultiple, NUM_MAX_STEPS / numPhysSteps);
		//	++circleCounter;
		//}
	}
	else if( action == RUSH )
	{
		currPosInfo.position = basePos + rushSeq.position;
		rushSeq.Update( slowMultiple, NUM_MAX_STEPS / numPhysSteps);
	}
}

void Shark::UpdateSprite()
{
	if (latchedOn)
	{
		V2d playerPos = sess->GetPlayerPos(0);
		basePos = playerPos;
		if ((action == CIRCLE || action == FINALCIRCLE))
		{
			currPosInfo.position = basePos + GetCircleOffset() * length(offsetPlayer);
		}
		else if (action == RUSH)
		{
			currPosInfo.position = basePos + rushSeq.position;
		}
	}

	switch( action )
	{
	case NEUTRAL:
		sprite.setRotation(0);
		sprite.setTextureRect(ts_circle->GetSubRect(0));
		sprite.setOrigin(sprite.getLocalBounds().width / 2,
			sprite.getLocalBounds().height / 2);
		break;
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


			//GetVectorAngleCW(-attackOffset);
			//V2d normOffset = normalize( -attackOffset );
			//double angle = atan2( normOffset.y, normOffset.x );//atan2( normOffset.x, -normOffset.y );
			//if( angle < 0 )
		//		angle += PI * 2;

			double angle = GetVectorAngleCW(attackOffset);
			sprite.setOrigin( sprite.getLocalBounds().width / 2, 
				sprite.getLocalBounds().height / 2 );
			sprite.setRotation( angle / PI * 180.0 );


		}
		//testColor = Color::Red;
		break;
	}

	sprite.setPosition( GetPositionF() );
}

void Shark::EnemyDraw( sf::RenderTarget *target )
{
	DrawSprite(target, sprite);
}