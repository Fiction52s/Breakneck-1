#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_GrowingTree.h"

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

GrowingTree::GrowingTree( GameSession *owner, bool p_hasMonitor, Edge *g, double q, int p_level)
	:Enemy( owner, EnemyType::EN_GROWINGTREE, p_hasMonitor, 1 ), ground( g ), edgeQuantity( q )
	
{
	actionLength[RECOVER0] = 4;
	actionLength[RECOVER1] = 4;
	actionLength[RECOVER2] = 6;
	actionLength[LEVEL0] = 10;
	actionLength[LEVEL0TO1] = 10;
	actionLength[LEVEL1] = 10;
	actionLength[LEVEL1TO2] = 10;
	actionLength[LEVEL2] = 10;
	actionLength[EXPLODE] = 10;

	animFactor[RECOVER0] = 10;
	animFactor[RECOVER1] = 10;
	animFactor[RECOVER2] = 10;
	animFactor[LEVEL0] = 1;
	animFactor[LEVEL0TO1] = 1;
	animFactor[LEVEL1] = 1;
	animFactor[LEVEL1TO2] = 1;
	animFactor[LEVEL2] = 1;
	animFactor[EXPLODE] = 12;

	//32, 0, 1000

	totalBullets = 32;
	startPowerLevel = 0;
	pulseRadius = 1000;
	powerLevel = 0;

	double height = 48;
	ts = owner->GetTileset( "Enemies/sprout_160x160.png", 160, 160 );
	sprite.setTexture( *ts->texture );
	
	V2d gPoint = g->GetPoint(edgeQuantity);

	double bulletSpeed = 10;
	int framesToLive = ( pulseRadius * 2 ) / bulletSpeed + .5;

	V2d launchPos = gPoint + ground->Normal() * 20.0 * (double)scale;

	numLaunchers = 1;
	launchers = new Launcher*[numLaunchers];
	launchers[0] = new Launcher( this, BasicBullet::GROWING_TREE, owner, totalBullets, 1, position, V2d( 1, 0 ), 0, framesToLive, false );
	launchers[0]->SetBulletSpeed( bulletSpeed );	
	launchers[0]->hitboxInfo->damage = 18;

	gn = g->Normal();
	angle = atan2( gn.x, -gn.y );

	position = gPoint + gn * height / 2.0;

	sprite.setTexture(*ts->texture);
	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( gPoint.x, gPoint.y );
	sprite.setRotation( angle / PI * 180 );
	sprite.setScale(scale, scale);


	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = .5;
	hitboxInfo->drainY = .5;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 5;
	hitboxInfo->knockback = 0;

	SetupBodies(1, 1);
	AddBasicHurtCircle(32);
	AddBasicHitCircle(32);
	hitBody->hitboxInfo = hitboxInfo;

	
	cutObject->SetTileset(ts);
	//cutObject->SetSubRectFront(12);
	//cutObject->SetSubRectBack(11);
	cutObject->rotateAngle = sprite.getRotation();
	cutObject->SetScale(scale);
	//animationFactor = 7;
	
	spawnRect = sf::Rect<double>( gPoint.x - pulseRadius, gPoint.y - pulseRadius, pulseRadius * 2, pulseRadius * 2 );

	ts_bulletExplode = owner->GetTileset( "bullet_explode3_64x64.png", 64, 64 );

	ResetEnemy();

	//InitRangeMarkerVA();
}

GrowingTree::~GrowingTree()
{
	delete[] rangeMarkerVA;
}

void GrowingTree::ResetEnemy()
{

	switch (startPowerLevel)
	{
	case 0:
		action = RECOVER0;
		//frame = 0;
		break;
	case 1:
		action = RECOVER1;
		//frame = 0;
		break;
	case 2:
		action = RECOVER2;
		//frame = 0;
		break;
	}
	

	frame = 0;
	dead = false;
	receivedHit = NULL;
	numEnemiesKilledLastFrame = 0;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

	UpdateSprite();
}

void GrowingTree::ActionEnded()
{
	if( frame == actionLength[action] * animFactor[action] )
	{
		switch( action )
		{
		case RECOVER0:
			action = LEVEL0;
			break;
		case RECOVER1:
			action = LEVEL1;
			break;
		case RECOVER2:
			action = LEVEL2;
			break;
		case LEVEL0:
			break;
		case LEVEL0TO1:
			action = LEVEL1;
			break;
		case LEVEL1:
			break;
		case LEVEL1TO2:
			action = LEVEL2;
			break;
		case LEVEL2:
			break;
		case EXPLODE:
			{
				switch( startPowerLevel )
				{
				case 0:
					action = RECOVER0;
					break;
				case 1:
					action = RECOVER1;
					break;
				case 2:
					action = RECOVER2;
					break;
				}
		
			}
			break;
		}

		frame = 0;
	}
}


void GrowingTree::InitRangeMarkerVA()
{
	//rangeMarkerVA( sf::Quads, numBullets * 4 ) 
	rangeMarkerVA = new Vertex[totalBullets * 4];
	int markerSize = 8;
	Vector2f start( 0, -pulseRadius );
	Transform rot;
	for( int i = 0; i < totalBullets; ++i )
	{
		Vector2f trans = rot.transformPoint( start ) + Vector2f( position.x, position.y );

		rangeMarkerVA[i*4+0].position = trans + Vector2f( -markerSize, -markerSize );
		rangeMarkerVA[i*4+1].position = trans + Vector2f( markerSize, -markerSize );
		rangeMarkerVA[i*4+2].position = trans + Vector2f( markerSize, markerSize );
		rangeMarkerVA[i*4+3].position = trans + Vector2f( -markerSize, markerSize );

		rangeMarkerVA[i*4+0].color = Color::Red;
		rangeMarkerVA[i*4+1].color = Color::Red;
		rangeMarkerVA[i*4+2].color = Color::Red;
		rangeMarkerVA[i*4+3].color = Color::Red;


		rot.rotate( 360.f / totalBullets );
	}
}


void GrowingTree::ProcessState()
{
	ActionEnded();

	if( action == EXPLODE )
	{
		//frame 0 doesnt work cuz its set in post physics
		if( frame == 3 && slowCounter == 1 )
		{
			Fire();
		}
	}

	if ( numEnemiesKilledLastFrame > 0 && powerLevel < 3)
	{
		if (WithinDistance(owner->GetPlayer(0)->position, position, pulseRadius))
		{
			switch (action)
			{
			case LEVEL0:
				action = LEVEL0TO1;
				frame = 0;
				break;
			case LEVEL1:
				action = LEVEL1TO2;
				frame = 0;
				break;
			case LEVEL2:
				action = EXPLODE;
				frame = 0;
				break;
			}

			powerLevel++;
			if (powerLevel == 3)
			{
				pulseFrame = 0;
			}
		}

	}
}

void GrowingTree::Fire()
{
	powerLevel = startPowerLevel;
	//launcher->Reset();
	Vector2f start( 0, -pulseRadius );

	Launcher *launcher = launchers[0];

	Transform t;
	for( int i = 0; i < totalBullets; ++i )
	{
		Vector2f trans = t.transformPoint( start );
		launcher->position = position + V2d( trans.x, trans.y );
		launcher->facingDir = normalize( position - launcher->position );
		launcher->Fire();
		
		t.rotate( 360.f / totalBullets );
	}
}

void GrowingTree::EnemyDraw(sf::RenderTarget *target )
{
	DrawSpriteIfExists(target, sprite);
	//target->draw(rangeMarkerVA, totalBullets * 4, sf::Quads);
}

void GrowingTree::UpdateSprite()
{
	int tileIndex = 0;
	switch( action )
	{
	case RECOVER0:
		tileIndex = frame / animFactor[action] + 4;
		
		break;
	case RECOVER1:
		tileIndex = frame / animFactor[action] + 8;
		break;
	case RECOVER2:
		tileIndex = frame / animFactor[action] + 8;
		break;
	case LEVEL0:
		tileIndex = 0;
		break;
	case LEVEL0TO1:
		tileIndex = 1;
		break;
	case LEVEL1:
		tileIndex = 1;
		break;
	case LEVEL1TO2:
		tileIndex = 2;
		break;
	case LEVEL2:
		tileIndex = 2;
		break;
	case EXPLODE:
		tileIndex = 3;
		break;
	}

	sprite.setTextureRect(ts->GetSubRect(tileIndex));
	V2d gPoint = ground->GetPoint( edgeQuantity );

	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );
	sprite.setPosition( gPoint.x, gPoint.y );
	

	switch( powerLevel )
	{
	case 0:
	//	sprite.setColor( Color::White );
		break;
	case 1:
	//	sprite.setColor( Color::Blue );
		break;
	case 2:
	//	sprite.setColor( Color::Green );
		break;
	case 3:
	//	sprite.setColor( Color::Red );
		break;
	}
}

void GrowingTree::DirectKill()
{
	Enemy::DirectKill();

	for (int i = 0; i < numLaunchers; ++i)
	{
		BasicBullet *b = launchers[i]->activeBullets;
		while (b != NULL)
		{
			BasicBullet *next = b->next;
			double angle = atan2(b->velocity.y, -b->velocity.x);
			owner->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
			b->launcher->DeactivateBullet(b);

			b = next;
		}
	}
	receivedHit = NULL;
}

void GrowingTree::BulletHitTerrain(BasicBullet *b, 
	Edge *edge, V2d &pos)
{
	/*V2d norm = edge->Normal();
	double angle = atan2(norm.y, -norm.x);

	owner->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, -angle, 6, 2, true);
	b->launcher->DeactivateBullet(b);*/
}

void GrowingTree::BulletHitPlayer( BasicBullet *b )
{
	V2d vel = b->velocity;
	double angle = atan2( vel.y, vel.x );
	owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
	owner->GetPlayer( 0 )->ApplyHit( b->launcher->hitboxInfo );
	b->launcher->DeactivateBullet( b );
}

void GrowingTree::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hurtBox.globalPosition = position - gn * 10.0;;// + gn * 8.0;
	hurtBox.globalAngle = 0;
	hitBox.globalPosition = position - gn * 10.0;;// + gn * 8.0;
	hitBox.globalAngle = 0;
}