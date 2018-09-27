#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Bat.h"

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


Bat::Bat( GameSession *owner, bool p_hasMonitor, Vector2i pos, 
	list<Vector2i> &pathParam, int p_bulletSpeed,
	//int p_nodeDistance, 
	int p_framesBetweenNodes, bool p_loop )
	:Enemy( owner, EnemyType::EN_BAT, p_hasMonitor, 2 )
{
	loop = p_loop;
	//loop = false; //no looping on bat for now

	fireCounter = 0;
	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	bulletSpeed = p_bulletSpeed;
	//nodeDistance = p_nodeDistance;
	framesBetween = p_framesBetweenNodes;
	
	//ts_hitSpack = owner->GetTileset( "hit_spack_2_128x128.png", 128, 128 );


	numLaunchers = 1;
	launchers = new Launcher*[numLaunchers];
	launchers[0] = new Launcher( this, BasicBullet::BAT, owner, 16, 1, position, V2d( 1, 0 ), 0, 300 );
	launchers[0]->SetBulletSpeed( bulletSpeed );	
	launchers[0]->hitboxInfo->damage = 18;
	launchers[0]->SetGravity( V2d( 0, .5 ));

	initHealth = 40;
	health = initHealth;

	spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );
	
	pathLength = pathParam.size() + 1;
	if( loop )
	{
		
		cout << "looping bat" << endl;
		assert( false );
		//tough cuz of set node distance from each other. for now don't use it.
	}
	else
	{
		//the road back
		if( pathParam.size() > 0 )
		{
			pathLength += pathParam.size();
		}
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

	if( pathLength == 1 )
	{

	}
	else
	{
		list<Vector2i>::reverse_iterator rit = pathParam.rbegin();
		++rit; //start at second item
		
		for(  ;rit != pathParam.rend(); ++rit )
		{
			path[index] = (*rit) + pos;
			++index;
		}
	}

	V2d sqTest0 = position;
	V2d sqTest1 = position + V2d( 0, -150 );
	V2d sqTest2 = position + V2d( 150, -150 );
	V2d sqTest3 = position + V2d( 300, -150 );
	V2d sqTest4 = position + V2d( 300, 0 );

	for( int i = 0; i < pathLength - 1; ++i )
	{
		V2d A( path[i].x, path[i].y );
		V2d B( path[i+1].x, path[i+1].y );
		//A += position;
		//B += position;
		testSeq.AddLineMovement( A, B, CubicBezier( .42,0,.58,1 ), 60 * 10 );
	}
	if( pathLength == 1 )
	{
		testSeq.AddMovement( new WaitMovement( position, 60 * 10 ) );
	}
	testSeq.InitMovementDebug();
	testSeq.Reset();
	
	frame = 0;

	animationFactor = 5;

	//ts = owner->GetTileset( "Bat.png", 80, 80 );
	ts = owner->GetTileset( "Bat_144x176.png", 144, 176 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );
	//position.x = 0;
	//position.y = 0;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	hurtBody = new CollisionBody(1);
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 16;
	hurtBox.rh = 16;
	hurtBody->AddCollisionBox(0, hurtBox);

	hitBody = new CollisionBody(1);
	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 16;
	hitBox.rh = 16;
	hitBody->hitboxInfo = hitboxInfo;
	hitBody->AddCollisionBox(0, hitBox);

	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);

	facingRight = true;
	
	ts_bulletExplode = owner->GetTileset( "bullet_explode3_64x64.png", 64, 64 );

	UpdateHitboxes();

	spawnRect = sf::Rect<double>( position.x - 200, position.y - 200,
		400, 400 );

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(7);
	cutObject->SetSubRectBack(6);
}


void Bat::BulletHitTerrain( BasicBullet *b, Edge *edge, V2d &pos )
{
	//V2d vel = b->velocity;
	//double angle = atan2( vel.y, vel.x );
	V2d norm = edge->Normal();
	double angle = atan2( norm.y, -norm.x );

	owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true );
	b->launcher->DeactivateBullet( b );
}

void Bat::BulletHitPlayer(BasicBullet *b )
{
	//if you dont deactivate the bullet it will hit constantly and make weird fx

	//cout << "hit player??" << endl;
	V2d vel = b->velocity;
	double angle = atan2( vel.y, vel.x );
	owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
	owner->GetPlayer( 0 )->ApplyHit( b->launcher->hitboxInfo );
	b->launcher->DeactivateBullet( b );
}


void Bat::ResetEnemy()
{
	//keyFrame = 0;
	fireCounter = 0;
	testSeq.Reset();
	dead = false;
	frame = 0;
	position.x = path[0].x;
	position.y = path[0].y;
	receivedHit = NULL;

	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;

	
}

void Bat::DirectKill()
{
	BasicBullet *b = launchers[0]->activeBullets;
	while( b != NULL )
	{
		BasicBullet *next = b->next;
		double angle = atan2( b->velocity.y, -b->velocity.x );
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
		b->launcher->DeactivateBullet( b );

		b = next;
	}
	health = 0;
	receivedHit = NULL;
}


void Bat::FrameIncrement()
{
	++fireCounter;
}

void Bat::ProcessState()
{
	if( frame == 5 * animationFactor )
	{
		frame = 0;
	}

	if( testSeq.currMovement == NULL )
	{
		//cout << "resetting" << endl;
		testSeq.Reset();
		//testSeq.currMovement = testSeq.movementList;
		//testSeq.currMovementStartTime = 0;
	}

	if( (fireCounter == 0 || fireCounter == 10 || fireCounter == 20/*framesBetween - 1*/) && slowCounter == 1 )// frame == 0 && slowCounter == 1 )
	{
		launchers[0]->position = position;
		launchers[0]->facingDir = normalize( owner->GetPlayer( 0 )->position - position );
		launchers[0]->Fire();
		//cout << "shoot:" << position.x << ", " << position.y << endl;
		
	}
	if (fireCounter == framesBetween - 1 && slowCounter == 1)
	{
		fireCounter = -1;
	}
}

void Bat::UpdateEnemyPhysics()
{	
	if (numPhysSteps == 1)
	{
		for( int i = 0; i < 10; ++i )
			testSeq.Update(slowMultiple);
	}
	else
	{
		testSeq.Update(slowMultiple);
	}
	
	position = testSeq.position;
}

void Bat::UpdateSprite()
{
	sprite.setTextureRect( ts->GetSubRect( frame / animationFactor ) );
	sprite.setPosition( position.x, position.y );

	if( hasMonitor && !suppressMonitor )
	{
		//keySprite.setTexture( *ts_key->texture );
		keySprite->setTextureRect( ts_key->GetSubRect( owner->keyFrame / 5 ) );
		keySprite->setOrigin( keySprite->getLocalBounds().width / 2, 
			keySprite->getLocalBounds().height / 2 );
		keySprite->setPosition( position.x, position.y );
	}
}

void Bat::EnemyDraw( sf::RenderTarget *target )
{
	DrawSpriteIfExists(target, sprite);
}

void Bat::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
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