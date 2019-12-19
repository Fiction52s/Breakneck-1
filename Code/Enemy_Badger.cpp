#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Badger.h"

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

Badger::Badger( GameSession *owner, bool hasMonitor, Edge *g, double q, int p_level )
	:Enemy( owner, EnemyType::EN_BADGER, hasMonitor, 2 ), facingRight( true ),
	moveBezTest( .22,.85,.3,.91 )
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

	maxGroundSpeed = 10;
	jumpStrength = 5;

	originalFacingRight = facingRight;
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
	

	startGround = g;
	startQuant = q;
	frame = 0;

	testMover = new GroundMover( owner, g, q, 32, true, this );
	testMover->AddAirForce(V2d(0, .6));
	//testMover->gravity = V2d( 0, .5 );
	testMover->SetSpeed( 0 );
	//testMover->groundSpeed = s;
	/*if( !facingRight )
	{
		testMover->groundSpeed = -testMover->groundSpeed;
	}*/

	ts = owner->GetTileset( "Enemies/badger_192x128.png", width, height );
	ts_aura = owner->GetTileset("Enemies/badger_aura_192x128.png", width, height);

	auraSprite.setTexture(*ts_aura->texture);

	sprite.setTexture( *ts->texture );
	sprite.setScale(scale, scale);

	auraSprite.setScale(scale, scale);
	//sprite.setTextureRect( ts->GetSubRect( 0 ) );
	//sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	V2d gPoint = g->GetPoint( q );
	//sprite.setPosition( testMover->physBody.globalPosition.x,
	//	testMover->physBody.globalPosition.y );


	position = testMover->physBody.globalPosition;
	//roll = false;
	//position = gPoint + ground->Normal() * height / 2.0;
	

	receivedHit = NULL;

	double size = max( width, height );
	spawnRect = sf::Rect<double>( gPoint.x - size, gPoint.y - size, size * 2, size * 2 );

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 0;

	SetupBodies(1, 1);
	AddBasicHurtCircle(32);
	AddBasicHitCircle(32);
	hitBody->hitboxInfo = hitboxInfo;

	crawlAnimationFactor = 5;
	rollAnimationFactor = 5;

	bezLength = 60 * NUM_STEPS;

	ResetEnemy();

	//highResPhysics = true;
}

void Badger::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
	cutObject->rotateAngle = sprite.getRotation();
}

void Badger::ResetEnemy()
{
	facingRight = originalFacingRight;
	testMover->ground = startGround;
	testMover->edgeQuantity = startQuant;
	testMover->roll = false;
	testMover->UpdateGroundPos();
	testMover->SetSpeed( 0 );

	position = testMover->physBody.globalPosition;


	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);

	bezFrame = 0;
	attackFrame = -1;
	
	frame = 0;


	dead = false;

	action = SHORTJUMP;
	frame = 0;

	UpdateSprite();
	UpdateHitboxes();
}

void Badger::UpdateHitboxes()
{
	Edge *ground = testMover->ground;
	if( ground != NULL )
	{
		V2d knockbackDir( 1, -1 );
		knockbackDir = normalize( knockbackDir );
		if( testMover->groundSpeed > 0 )
		{
			hitboxInfo->kbDir = knockbackDir;
			hitboxInfo->knockback = 15;
		}
		else
		{
			hitboxInfo->kbDir = V2d( -knockbackDir.x, knockbackDir.y );
			hitboxInfo->knockback = 15;
		}
	}
	else
	{
		//hitBody.globalAngle = 0;
		//hurtBody.globalAngle = 0;
	}

	BasicUpdateHitboxes();
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

	if( owner->GetPlayerPos( 0 ).x > position.x )
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
	assert( testMover->ground != NULL );

	landedAction = action;

	V2d gAlong = normalize( testMover->ground->v1 - testMover->ground->v0 );
	if( !facingRight )
		gAlong = -gAlong;


	if( false )//cross( normalize( V2d( strengthx, -strengthy )), gAlong ) < 0 )
	{
		gAlong = (gAlong + V2d( 0, -1 )) / 2.0;
		V2d jumpVec = gAlong * strengthy;
		testMover->Jump( jumpVec );
		cout << "jump blend: " << jumpVec.x << ", " << jumpVec.y << endl;
	}
	else
	{
		V2d jumpVec = V2d( strengthx, -strengthy );
		testMover->Jump( jumpVec );
		//cout << "jump: " << jumpVec.x << ", " << jumpVec.y << endl;
	}
}

void Badger::ProcessState()
{
	//cout << "vel: " << testMover->velocity.x << ", " << testMover->velocity.y << endl;
	//cout << "action: " << (int)action << endl;
	//testLaunch->UpdatePrePhysics();
	Actor *player = owner->GetPlayer( 0 );

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

	if( testMover->ground == NULL )
	{
		double airAccel = .5;
		if( facingRight )
		{
			testMover->velocity.x += airAccel;
			if( testMover->velocity.x > 10 )
			{
				testMover->velocity.x = 10;
			}
			
		}
		else
		{
			testMover->velocity.x -= airAccel;
			if( testMover->velocity.x < -10 )
				testMover->velocity.x = -10;
			
		}
	}

	int runSpeed = 10;
	switch( action )
	{
	case RUN:
		if( facingRight )
		{
			testMover->SetSpeed( 10 );//testMover->groundSpeed + .3 );
		}
		else
		{
			testMover->SetSpeed( -10 );//testMover->groundSpeed - .3 );
		}

		if( testMover->groundSpeed > maxGroundSpeed )
			testMover->SetSpeed( maxGroundSpeed );
		else if( testMover->groundSpeed < -maxGroundSpeed )
			testMover->SetSpeed( -maxGroundSpeed );
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
			testMover->SetSpeed( 0 );
		}
		break;
	case LAND:
		{
			testMover->SetSpeed( 0 );
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
	testMover->Move(slowMultiple, numPhysSteps);

	if (testMover->ground == NULL)
	{
		if (testMover->velocity.y > maxFallSpeed)
		{
			testMover->velocity.y = maxFallSpeed;
		}
	}

	position = testMover->physBody.globalPosition;
}

void Badger::EnemyDraw(sf::RenderTarget *target )
{
	target->draw(auraSprite);
	DrawSpriteIfExists(target, sprite);
}


void Badger::UpdateSprite()
{
	double angle = 0;

	V2d p = testMover->physBody.globalPosition;
	V2d vel = testMover->velocity;
	
	double groundSpeed = testMover->groundSpeed;
	
	float extraVert = 0;

	if( testMover->ground != NULL )
	{
		V2d gPoint = testMover->ground->GetPoint( testMover->edgeQuantity );
		V2d gn = testMover->ground->Normal();

		if (!testMover->roll)
		{
			angle = atan2(gn.x, -gn.y);
			
		}
		else
		{
			if (facingRight)
			{
				V2d vec = normalize(position - testMover->ground->v1);
				angle = atan2(vec.y, vec.x);
				angle += PI / 2.0;

				sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
				sprite.setRotation(angle / PI * 180);
			}
			else
			{
				V2d vec = normalize(position - testMover->ground->v0);
				angle = atan2(vec.y, vec.x);
				angle += PI / 2.0;

				sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
				sprite.setRotation(angle / PI * 180);
			}
		}

		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
		sprite.setRotation(angle / PI * 180);
		sprite.setPosition(gPoint.x, gPoint.y);
	}
	else
	{
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
		sprite.setPosition(p.x, p.y);
		sprite.setRotation(0);
	}


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

	SyncSpriteInfo(auraSprite, sprite);
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
	if( facingRight && testMover->groundSpeed < 0 
		|| !facingRight && testMover->groundSpeed > 0 )
	{
		testMover->SetSpeed( 0 );
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

	//testMover->Jump( v );
	//testMover->groundSpeed = -testMover->groundSpeed;
	//facingRight = !facingRight;
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