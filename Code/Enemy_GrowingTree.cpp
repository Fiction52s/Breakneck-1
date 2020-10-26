#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_GrowingTree.h"
#include "Actor.h"

using namespace std;
using namespace sf;

GrowingTree::GrowingTree( ActorParams *ap )
	:Enemy( EnemyType::EN_GROWINGTREE, ap )
	
{
	SetNumActions(A_Count);
	SetEditorActions(LEVEL1, 0, 0);

	SetLevel(ap->GetLevel());

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

	ts = sess->GetSizedTileset("Enemies/sprout_160x160.png");
	sprite.setTexture( *ts->texture );

	double bulletSpeed = 10;
	int framesToLive = ( pulseRadius * 2 ) / bulletSpeed + .5;

	SetNumLaunchers(1);
	launchers[0] = new Launcher( this, BasicBullet::GROWING_TREE, totalBullets, 1, GetPosition(), V2d( 1, 0 ), 0, framesToLive, false );
	launchers[0]->SetBulletSpeed( bulletSpeed );	
	launchers[0]->hitboxInfo->damage = 18;

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	SetOffGroundHeight(ts->tileHeight / 2.f - 0 * scale);


	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = .5;
	hitboxInfo->drainY = .5;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 5;
	hitboxInfo->knockback = 0;

	BasicCircleHitBodySetup(32);
	BasicCircleHurtBodySetup(32);

	hitBody.hitboxInfo = hitboxInfo;

	
	cutObject->SetTileset(ts);
	cutObject->SetScale(scale);

	ts_bulletExplode = sess->GetSizedTileset( "FX/bullet_explode3_64x64.png");

	rangeMarkerVA = NULL;

	ResetEnemy();

	InitRangeMarkerVA();
}

GrowingTree::~GrowingTree()
{
	delete[] rangeMarkerVA;
}

void GrowingTree::UpdateOnPlacement(ActorParams *ap)
{
	Enemy::UpdateOnPlacement(ap);

	InitRangeMarkerVA();

	//testShield->SetPosition(GetPosition());
}

void GrowingTree::HandleNoHealth()
{
	cutObject->rotateAngle = sprite.getRotation();
}

void GrowingTree::SetLevel( int lev)
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

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();
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

	if (rangeMarkerVA == NULL)
	{
		rangeMarkerVA = new Vertex[totalBullets * 4];
	}
	
	int markerSize = 8;
	Vector2f start( 0, -pulseRadius );
	Transform rot;
	for( int i = 0; i < totalBullets; ++i )
	{
		Vector2f trans = rot.transformPoint( start ) + GetPositionF();

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

	int numEnemiesKilledLastFrame = sess->GetPlayerEnemiesKilledLastFrame(0);

	if ( numEnemiesKilledLastFrame > 0 && powerLevel < 3)
	{
		if (WithinDistance(sess->GetPlayerPos(0), GetPosition(), pulseRadius))
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
		launcher->position = GetPosition() + V2d( trans.x, trans.y );
		launcher->facingDir = normalize(GetPosition() - launcher->position );
		launcher->Fire();
		
		t.rotate( 360.f / totalBullets );
	}
}

void GrowingTree::EnemyDraw(sf::RenderTarget *target )
{
	DrawSprite(target, sprite);
	target->draw(rangeMarkerVA, totalBullets * 4, sf::Quads);
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
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
	

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
			sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
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

void GrowingTree::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	V2d vel = b->velocity;
	double angle = atan2(vel.y, vel.x);
	sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}

	b->launcher->DeactivateBullet(b);
}