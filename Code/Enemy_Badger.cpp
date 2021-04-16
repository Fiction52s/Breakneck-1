#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Badger.h"

using namespace std;
using namespace sf;

Badger::Badger(ActorParams *ap)
	:Enemy( EnemyType::EN_BADGER, ap ), moveBezTest( .22,.85,.3,.91 )
{
	SetNumActions(Count);
	SetEditorActions(RUN, RUN, 0);

	SetLevel(ap->GetLevel());

	maxGroundSpeed = 10;
	jumpStrength = 5;

	actionLength[RUN] = 7 * 2;
	actionLength[LEDGEJUMP] = 2;
	actionLength[SHORTJUMP] = 2;
	actionLength[SHORTJUMPSQUAT] = 3;
	actionLength[TALLJUMP] = 2;
	actionLength[TALLJUMPSQUAT] = 3;
	actionLength[LAND] = 3;
	
	//runReps = 10;
	animFactor[RUN] = 5;
	animFactor[LEDGEJUMP] = 1;
	animFactor[SHORTJUMP] = 1;
	animFactor[SHORTJUMPSQUAT] = 1;
	animFactor[TALLJUMP] = 1;
	animFactor[TALLJUMPSQUAT] = 1;
	animFactor[LAND] = 1;

	//jumpStrength = p_jumpStrength;
	gravity = V2d( 0, .6 );
	
	action = RUN;

	dead = false;
	nextAction = SHORTJUMP;
	maxFallSpeed = 25;

	attackFrame = -1;
	attackMult = 10;

	double width = 192;
	double height = 128;

	CreateGroundMover(startPosInfo, 32, true, this);
	groundMover->SetSpeed(0);
	groundMover->AddAirForce(gravity);

	ts = sess->GetSizedTileset("Enemies/W3/badger_192x128.png");

	sprite.setTexture( *ts->texture );
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 20;
	hitboxInfo->knockback = 15;

	BasicCircleHurtBodySetup(32);
	BasicCircleHitBodySetup(32);

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(33);
	cutObject->SetSubRectBack(34);
	cutObject->SetScale(scale);

	hitBody.hitboxInfo = hitboxInfo;

	bezLength = 60 * NUM_STEPS;



	ResetEnemy();
}

void Badger::SetLevel(int lev)
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

void Badger::HandleNoHealth()
{
	cutObject->SetFlipHoriz(!facingRight);
	cutObject->rotateAngle = sprite.getRotation();
}

void Badger::ResetEnemy()
{
	DefaultHurtboxesOn();
	DefaultHitboxesOn();

	facingRight = true;
	
	groundMover->Set(startPosInfo);
	groundMover->SetSpeed(0);

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	bezFrame = 0;
	attackFrame = -1;
	
	frame = 0;

	action = RUN;
	frame = 0;

	UpdateSprite();
	UpdateHitboxes();
}

void Badger::UpdateNextAction()
{
	switch( nextAction )
	{
	case RUN:
		nextAction = SHORTJUMP;
		break;
	case SHORTJUMP:
		nextAction = TALLJUMP;
		break;
	case TALLJUMP:
		nextAction = RUN;
		break;
	case TALLJUMPSQUAT:
		{
		}
		break;
	case SHORTJUMPSQUAT:
		{
		}
		break;
	}

	if( sess->GetPlayerPos( 0 ).x > GetPosition().x )
	{
		//cout << "facing right" << endl;
		facingRight = true;
	}
	else
	{
		//cout << "facing left" << endl;
		facingRight = false;
	}
}

void Badger::ActionEnded()
{
	if( frame == animFactor[action] * actionLength[action] )
	{
		switch( action )
		{
		case RUN:
			action = nextAction;
			UpdateNextAction();
			frame = 0;
			break;
		case LEDGEJUMP:
			frame = 1;
			break;
		case SHORTJUMP:
			frame = 1;
			break;
		case SHORTJUMPSQUAT:
			{
				action = SHORTJUMP;
				frame = 0;
			}
			break;
		case TALLJUMP:
			frame = 1;
			break;
		case TALLJUMPSQUAT:
			{
				action = TALLJUMP;
				frame = 0;
			}
			break;
		
		case LAND:
			action = nextAction;
			UpdateNextAction();
			frame = 0;
			break;
		case ATTACK:
			action = RUN;
			frame = 0;
			break;
		}
	}
}

void Badger::Jump( double strengthx, double strengthy )
{
	//cout << "jump: " << strengthx << ", " << strengthy << endl;
	assert(groundMover->ground != NULL );

	landedAction = action;

	V2d gAlong = normalize(groundMover->ground->v1 - groundMover->ground->v0 );
	if( !facingRight )
		gAlong = -gAlong;


	if( false )//cross( normalize( V2d( strengthx, -strengthy )), gAlong ) < 0 )
	{
		gAlong = (gAlong + V2d( 0, -1 )) / 2.0;
		V2d jumpVec = gAlong * strengthy;
		groundMover->Jump( jumpVec );
		cout << "jump blend: " << jumpVec.x << ", " << jumpVec.y << endl;
	}
	else
	{
		V2d jumpVec = V2d( strengthx, -strengthy );
		groundMover->Jump( jumpVec );
		//cout << "jump: " << jumpVec.x << ", " << jumpVec.y << endl;
	}
}

void Badger::ProcessState()
{
	//cout << "vel: " << groundMover->velocity.x << ", " << groundMover->velocity.y << endl;
	//cout << "action: " << (int)action << endl;
	//testLaunch->UpdatePrePhysics();
	Actor *player = sess->GetPlayer( 0 );

	if( dead )
		return;

	ActionEnded();

	switch( action )
	{
	case RUN:
	//	cout << "RUN: " << frame << endl;
		break;
	case LEDGEJUMP:
	//	cout << "LEDGEJUMP: " << frame << endl;
		break;
	case SHORTJUMP:
	//	cout << "SHORTJUMP: " << frame << endl;
		break;
	case SHORTJUMPSQUAT:
		{
	//		cout << "SHORTJUMPSQUAT: " << frame << endl;
		}
		break;
	case TALLJUMP:
	//	cout << "TALLJUMP: " << frame << endl;
		break;
	case TALLJUMPSQUAT:
		{
	//		cout << "TALLJUMPSQUAT: " << frame << endl;
		}
		break;
	case ATTACK:

		break;
	case LAND:
	//	cout << "LAND: " << frame << endl;
		break;
	}

	if(groundMover->ground == NULL )
	{
		double airAccel = .5;
		if( facingRight )
		{
			groundMover->velocity.x += airAccel;
			if(groundMover->velocity.x > 10 )
			{
				groundMover->velocity.x = 10;
			}
			
		}
		else
		{
			groundMover->velocity.x -= airAccel;
			if(groundMover->velocity.x < -10 )
				groundMover->velocity.x = -10;
			
		}
	}

	int runSpeed = 10;
	switch( action )
	{
	case RUN:
		if( facingRight )
		{
			groundMover->SetSpeed( 10 );//testMover->groundSpeed + .3 );
		}
		else
		{
			groundMover->SetSpeed( -10 );//testMover->groundSpeed - .3 );
		}

		if(groundMover->groundSpeed > maxGroundSpeed )
			groundMover->SetSpeed( maxGroundSpeed );
		else if(groundMover->groundSpeed < -maxGroundSpeed )
			groundMover->SetSpeed( -maxGroundSpeed );
		break;
	case LEDGEJUMP:
		break;
	case SHORTJUMP:
		if( frame == 0 )
		{
			if( facingRight )
				Jump( 10, 10 );
			else
				Jump( -10, 10 );
		}
		break;
	case SHORTJUMPSQUAT:
		{
		}
		break;
	case TALLJUMP:
		if( frame == 0 )
		{
			if( facingRight )
				Jump( 10, 20 );
			else
				Jump( -10, 20 );
		}
		break;
	case TALLJUMPSQUAT:
		{
		}
		break;
	case ATTACK:
		{
		groundMover->SetSpeed( 0 );
		}
		break;
	case LAND:
		{
		groundMover->SetSpeed( 0 );
		}
		break;
	}

	
	if( attackFrame == 2 * attackMult )
	{
		attackFrame = -1;
	}

}

void Badger::UpdateEnemyPhysics()
{	
	if (numHealth > 0)
	{
		groundMover->Move(slowMultiple, numPhysSteps);

		if (groundMover->ground == NULL)
		{
			if (groundMover->velocity.y > maxFallSpeed)
			{
				groundMover->velocity.y = maxFallSpeed;
			}
		}

		//Enemy::UpdateEnemyPhysics();
	}
}

void Badger::EnemyDraw(sf::RenderTarget *target )
{
	DrawSprite(target, sprite);
}


void Badger::UpdateSprite()
{
	V2d vel = groundMover->velocity;
	
	float extraVert = 0;

	


	int airRange = 3;
	int fallRange = 15;
	sf::IntRect ir;
	switch( action )
	{
	case RUN:
		{
			ir = ts->GetSubRect( ( frame / animFactor[RUN] ) % 7 );
		}
		break;
	case LEDGEJUMP:
	case SHORTJUMP:
		{
			if( vel.y > fallRange )
			{
				ir = ts->GetSubRect( 14 ); //fall 2
			}
			else if( vel.y > airRange ) 
			{
				ir = ts->GetSubRect( 13 ); //fall 1
			}
			else if( vel.y < -airRange )
			{
				ir = ts->GetSubRect( 10 ); //rising
			}
			else
			{
				ir = ts->GetSubRect( 11 ); //neutral
			}
		}
		break;
	case SHORTJUMPSQUAT:
		{
			ir = ts->GetSubRect( 8 );
		}
		break;
	case TALLJUMP:
		{
			int div = 2;
			int orig = 16;
			if( vel.y > orig - div ) 
			{
				ir = ts->GetSubRect( 29 );
			}
			else if( vel.y > orig - div * 2 )
			{
				ir = ts->GetSubRect( 28 );
			}
			else if( vel.y > orig - div * 3 )
			{
				ir = ts->GetSubRect( 27 );
			}
			else if( vel.y > orig - div * 4 )
			{
				ir = ts->GetSubRect( 26 );
			}
			else if( vel.y > orig - div * 5 )
			{
				ir = ts->GetSubRect( 25 );
			}
			else if( vel.y > orig - div * 6 )
			{
				ir = ts->GetSubRect( 24 );
			}
			else if( vel.y > orig - div * 7 )
			{
				ir = ts->GetSubRect( 23 );
			}
			else if( vel.y < -4 )
			{
				ir = ts->GetSubRect( 21 );
			}
			else
			{
				ir = ts->GetSubRect( 22 );
			}
		}
		break;
	case TALLJUMPSQUAT:
		{
			ir = ts->GetSubRect( (frame / animFactor[TALLJUMPSQUAT] ) + 16 );
		}
		break;
	//case ATTACK:
	//	break;
	case LAND:
		if( landedAction == LEDGEJUMP || landedAction == SHORTJUMP )
		{
			ir = ts->GetSubRect( frame / animFactor[LAND] + 15 );
		}
		else if( landedAction == TALLJUMP )
		{
			ir = ts->GetSubRect( frame / animFactor[LAND] + 30 );
		}
		break;
	}

	if( attackFrame >= 0 )
	{
		ir = ts->GetSubRect( 28 + attackFrame / attackMult );
	}

	if( !facingRight )
	{
		ir = sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height );
	}
	sprite.setTextureRect( ir );

	if (groundMover->ground != NULL)
	{
		sprite.setRotation(groundMover->GetAngleDegrees());
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
		Vector2f spritePos = groundMover->GetGroundPointF();
		sprite.setPosition(spritePos);
	}
	else
	{
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
		sprite.setPosition(GetPositionF());
		sprite.setRotation(0);
	}
}
//
//void Badger::HitTerrain( double &q )
//{
//	
//}

bool Badger::StartRoll()
{
	return false;
}

void Badger::FinishedRoll()
{

}

void Badger::HitOther()
{
	/*V2d v;
	if( facingRight && testMover->groundSpeed > 0 )
	{
		v = V2d( 10, -10 );
		testMover->Jump( v );
	}
	else if( !facingRight && testMover->groundSpeed < 0 )
	{
		v = V2d( -10, -10 );
		testMover->Jump( v );
	}*/
	//cout << "hit other!" << endl;
	//testMover->SetSpeed( 0 );
	//facingRight = !facingRight;
}

void Badger::ReachCliff()
{
	if( facingRight && groundMover->groundSpeed < 0 
		|| !facingRight && groundMover->groundSpeed > 0 )
	{
		groundMover->SetSpeed( 0 );
		return;
	}

	//cout << "reach cliff!" << endl;
	//ground = NULL;
	V2d v;
	if( facingRight )
	{
		v = V2d( 10, -10 );
	}
	else
	{
		v = V2d( -10, -10 );
	}

	action = LEDGEJUMP;
	frame = 0;

	Jump( v.x, v.y );
}

void Badger::HitOtherAerial( Edge *e )
{
	//cout << "hit edge" << endl;
}

void Badger::Land()
{
	action = LAND;
	frame = 0;
	
	//cout << "land" << endl;
}