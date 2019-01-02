#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Pulser.h"
#include "Shield.h"

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


Pulser::Pulser( GameSession *owner, bool p_hasMonitor, Vector2i &pos, 
	list<Vector2i> &pathParam, int p_framesBetweenNodes, bool p_loop )
	:Enemy( owner, EnemyType::EN_PULSER, p_hasMonitor, 2 )
{
	highResPhysics = true;

	pulseWait = 40;
	actionLength[IDLE] = 4;
	actionLength[SHIELDOFF] = 4;
	actionLength[SHIELDON] = 4;
	actionLength[NOSHIELD] = 4;

	animFactor[IDLE] = 5;
	animFactor[SHIELDOFF] = 5;
	animFactor[SHIELDON] = 5;
	animFactor[NOSHIELD] = 5;

	loop = p_loop;

	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;
	
	

	framesBetween = p_framesBetweenNodes;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	pathLength = pathParam.size() + 1;
	if( loop )
	{
		pathLength += 1;
		//cout << "looping bat" << endl;
		//assert( false );
		//tough cuz of set node distance from each other. for now don't use it.
	}
	else
	{
		//the road back
		//cout << "old: " << pathLength << endl;
		if( pathParam.size() > 0 )
		{
			pathLength += pathParam.size();
		}
		//cout << "new: " << pathLength << endl;
	}

	path = new Vector2i[pathLength];
	path[0] = pos;
	path[pathLength-1] = pos;

	int index = 1;
	for( list<Vector2i>::iterator it = pathParam.begin(); it != pathParam.end(); ++it )
	{
		path[index] = (*it) + pos;
		++index;
	}

	//make composite beziers
	if( pathLength == 1 )
	{

	}
	else if( !loop )
	{
		list<Vector2i>::reverse_iterator rit = pathParam.rbegin();
		++rit; //start at second item
		
		for(  ;rit != pathParam.rend(); ++rit )
		{
			path[index] = (*rit) + pos;
			++index;
		}
	}
	//cout << "path length: " << pathLength << ", " << index << endl;

	//basePos = position;
	V2d sqTest0 = position;
	V2d sqTest1 = position + V2d( 0, -150 );
	V2d sqTest2 = position + V2d( 150, -150 );
	V2d sqTest3 = position + V2d( 300, -150 );
	V2d sqTest4 = position + V2d( 300, 0 );

	for( int i = 0; i < pathLength - 1; ++i )
	{
		V2d A( path[i].x, path[i].y );
		V2d B( path[i+1].x, path[i+1].y );
		testSeq.AddLineMovement( A, B, CubicBezier( 0,0,1,1 ), framesBetween );
		testSeq.AddMovement( new WaitMovement( B, framesBetween ) );
	}

	if( pathLength == 1 )
	{
		V2d A( path[0].x, path[0].y );
		testSeq.AddLineMovement( A, A, CubicBezier( 0, 0, 1, 1 ), framesBetween );
		testSeq.AddMovement( new WaitMovement( A, framesBetween ) );
	}

	//testSeq.InitMovementDebug();

	frame = 0;

	//ts = owner->GetTileset( "Bat.png", 80, 80 );
	ts = owner->GetTileset( "Enemies/pulser_64x64.png", 64, 64 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );

	hurtBody = new CollisionBody(1);
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 26;
	hurtBox.rh = 26;
	hurtBody->AddCollisionBox(0, hurtBox);

	hitBody = new CollisionBody(2);
	CollisionBox smallHitbox;
	smallHitbox.type = CollisionBox::Hit;
	smallHitbox.isCircle = true;
	smallHitbox.globalAngle = 0;
	smallHitbox.offset.x = 0;
	smallHitbox.offset.y = 0;
	smallHitbox.rw = 26;
	smallHitbox.rh = 26;

	CollisionBox bigHitbox;
	bigHitbox.type = CollisionBox::Hit;
	bigHitbox.isCircle = true;
	bigHitbox.globalAngle = 0;
	bigHitbox.offset.x = 0;
	bigHitbox.offset.y = 0;
	bigHitbox.rw = 128;
	bigHitbox.rh = 128;

	hitBody->AddCollisionBox(0, smallHitbox);
	hitBody->AddCollisionBox(1, bigHitbox);

	

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	hitBody->hitboxInfo = hitboxInfo;


	shieldHitboxInfo = new HitboxInfo;
	shieldHitboxInfo->damage = 18;
	shieldHitboxInfo->drainX = 0;
	shieldHitboxInfo->drainY = 0;
	shieldHitboxInfo->hitlagFrames = 0;
	shieldHitboxInfo->hitstunFrames = 30;
	shieldHitboxInfo->knockback = 5;


	facingRight = true;

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(5);
	cutObject->SetSubRectBack(4);


	shield = new Shield(Shield::ShieldType::T_REFLECT, /*128*/150, 100, this, shieldHitboxInfo);
	//give it a unique hitboxinfo later
	shield->SetPosition(position);

	ResetEnemy();
	//cout << "finish init" << endl;
}

void Pulser::ResetEnemy()
{
	testSeq.Reset();
	dead = false;
	frame = 0;
	position.x = path[0].x;
	position.y = path[0].y;
	receivedHit = NULL;

	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);

	action = IDLE;

	UpdateHitboxes();
	UpdateSprite();
	testSeq.Reset();

	currShield = NULL;
	//currShield = shield;
	shield->Reset();
	pulseFrame = 0;
}

void Pulser::FrameIncrement()
{
	++pulseFrame;
}

void Pulser::HandleHitAndSurvive()
{
	--pulseFrame;
	if (pulseFrame < 0)
		pulseFrame = 0;
}

void Pulser::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
	}

	Actor *player = owner->GetPlayer(0);
	if( length( player->position - position ) )
	
	if (pulseFrame == pulseWait) 
	{
		pulseFrame = 0;
		if (action == SHIELDOFF)
		{
			action = SHIELDON;
			frame = 0;
			currShield = shield;
			
			
		}
		else if (action == SHIELDON)
		{
			action = SHIELDOFF;
			frame = 0;
			currShield = NULL;
			SetHitboxes(hitBody, 0);
		}
	}


	switch (action)
	{
	case SHIELDOFF:
		break;
	case SHIELDON:
		break;
	case NOSHIELD:
		break;
	}

	switch (action)
	{
	case SHIELDOFF:
		break;
	case SHIELDON:
		break;
	case NOSHIELD:
		break;
	}
	
}

void Pulser::UpdateEnemyPhysics()
{
	if (numPhysSteps == 1)
	{
		for (int i = 0; i < 10; ++i)
			testSeq.Update(slowMultiple);
	}
	else
	{
		testSeq.Update(slowMultiple);
	}

	position = testSeq.position;
}

void Pulser::UpdateSprite()
{
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setPosition(position.x, position.y);
}

void Pulser::EnemyDraw( sf::RenderTarget *target )
{
	/*assert(testSeq.currMovement != NULL);
	if (testSeq.currMovement->moveType == Movement::WAIT)
	{
		CircleShape cs;
		cs.setRadius(128);
		cs.setFillColor(COLOR_YELLOW);
		cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
		cs.setPosition(position.x, position.y);
		target->draw(cs);
	}*/

	DrawSpriteIfExists(target, sprite);
}

void Pulser::UpdateHitboxes()
{
	int hitboxIndex = 0;
	if (action == SHIELDON && pulseFrame == pulseWait - 1)
	{
		SetHitboxes(hitBody, 1);
		hitboxIndex = 1;
	}
	else
	{
		SetHitboxes(hitBody, 0);
	}

	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(hitboxIndex)->front();

	hurtBox.globalPosition = position;
	hurtBox.globalAngle = 0;
	hitBox.globalPosition = position;
	hitBox.globalAngle = 0;

	if( owner->GetPlayer( 0 )->ground != NULL )
	{
		hitboxInfo->kbDir = normalize( -owner->GetPlayer( 0 )->groundSpeed * ( owner->GetPlayer( 0 )->ground->v1 - owner->GetPlayer( 0 )->ground->v0 ) );
	}
	else
	{
		hitboxInfo->kbDir = normalize( -owner->GetPlayer( 0 )->velocity );
	}
}



//void Pulser::DebugDraw( RenderTarget *target )
//{
//	if( !dead )
//	{
//		assert( testSeq.currMovement != NULL );
//		if( testSeq.currMovement != NULL )
//		{
//			if( testSeq.currMovement->vertices != NULL )
//			{
//				testSeq.currMovement->DebugDraw( target );
//			}
//		}
//		hurtBody.DebugDraw( target );
//		hitBody.DebugDraw( target );
//	}
//}