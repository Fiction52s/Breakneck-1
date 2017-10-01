#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

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

GrowingTree::GrowingTree( GameSession *owner, bool p_hasMonitor, Edge *g, double q,
	int numBullets, int p_startLevel, int p_pulseRadius )
	:Enemy( owner, EnemyType::OVERGROWTH, p_hasMonitor, 5 ), ground( g ), edgeQuantity( q ),
	totalBullets( numBullets ), rangeMarkerVA( sf::Quads, numBullets * 4 ) 
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

	

	startPowerLevel = p_startLevel;
	pulseRadius = p_pulseRadius;
	powerLevel = 0;
	initHealth = 40;
	health = initHealth;

	double height = 48;
	ts = owner->GetTileset( "sprout_160x160.png", 160, 160 );
	sprite.setTexture( *ts->texture );
	
	V2d gPoint = g->GetPoint( edgeQuantity );
	//cout << "player " << owner->GetPlayer( 0 )->position.x << ", " << owner->GetPlayer( 0 )->position.y << endl;
	//cout << "gPoint: " << gPoint.x << ", " << gPoint.y << endl;
	totalBullets = 32;

	double bulletSpeed = 10;
	int framesToLive = ( pulseRadius * 2 ) / bulletSpeed + .5;
	launcher = new Launcher( this, BasicBullet::GROWING_TREE, owner, totalBullets, 1, position, V2d( 1, 0 ), 0, framesToLive );
	launcher->SetBulletSpeed( bulletSpeed );	
	launcher->hitboxInfo->damage = 18;

	receivedHit = NULL;

	gn = g->Normal();
	angle = atan2( gn.x, -gn.y );

	position = gPoint + gn * height / 2.0;

	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( gPoint.x, gPoint.y );
	sprite.setRotation( angle / PI * 180 );

	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 32;
	hurtBody.rh = 32;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = 32;
	hitBody.rh = 32;
	
	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = .5;
	hitboxInfo->drainY = .5;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 5;
	hitboxInfo->knockback = 0;

	frame = 0;
	deathFrame = 0;
	animationFactor = 7;
	slowCounter = 1;
	slowMultiple = 1;
	sprite.setTexture( *ts->texture );
	spawnRect = sf::Rect<double>( gPoint.x - pulseRadius, gPoint.y - pulseRadius, pulseRadius * 2, pulseRadius * 2 );

	//ts_death = owner->GetTileset( "GrowingTreedeath.png", 160, 80 );
	//ts_top = owner->GetTileset( "patroldeathtop.png", 32, 32 );

	deathPartingSpeed = .4;

	Transform t;
	t.rotate( angle / PI * 180 );
	Vector2f newPoint = t.transformPoint( Vector2f( 1, -1 ) );
	deathVector = V2d( newPoint.x, newPoint.y );
	//deathVector = V2d( 1, -1 );

	ts_bulletExplode = owner->GetTileset( "bullet_explode3_64x64.png", 64, 64 );

	ResetEnemy();

	InitRangeMarkerVA();
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

void GrowingTree::ResetEnemy()
{
	
	switch( startPowerLevel )
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
	//cout << "reset" << endl;
	health = initHealth;
	dying = false;
	frame = 0;
	deathFrame = 0;
	dead = false;
	receivedHit = NULL;
	slowCounter = 1;
	launcher->Reset();
	slowMultiple = 1;

	UpdateSprite();
}

void GrowingTree::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void GrowingTree::UpdatePrePhysics()
{
	if( !dead && !dying )
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

		/*switch( action )
		{
		case RECOVER0:
			cout << "recover0: " << frame << endl;
			break;
		case RECOVER1:
			cout << "recover1: " << frame << endl;
			break;
		case RECOVER2:
			cout << "recover2: " << frame << endl;
			break;
		case LEVEL0:
			cout << "level0: " << frame << endl;
			break;
		case LEVEL0TO1:
			cout << "level0to1: " << frame << endl;
			break;
		case LEVEL1:
			cout << "level1: " << frame << endl;
			break;
		case LEVEL1TO2:
			cout << "level1to2: " << frame << endl;
			break;
		case LEVEL2:
			cout << "level2: " << frame << endl;
			break;
		case EXPLODE:
			cout << "explode: " << frame << endl;
			break;
		}*/
	}

	

	//cout << "dead: " << dead << endl;
	if( !dead && !dying && receivedHit != NULL )
	{	
		
		//gotta factor in getting hit by a clone
		health -= 20;
		//cout << "damaging: " << health << endl;
		if( health <= 0 )
		{
			//cout << "attempting. blue key is: " << owner->GetPlayer( 0 )->hasBlueKey << endl;
			//AttemptSpawnMonitor();
			if( hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			dead = true;
			owner->GetPlayer( 0 )->ConfirmEnemyKill( this );
		}
		else
		{
			owner->GetPlayer( 0 )->ConfirmEnemyNoKill( this );
		}

		

		receivedHit = NULL;
	}
}

void GrowingTree::UpdatePhysics()
{
	launcher->UpdatePhysics();
	specterProtected = false;
	if( PlayerSlowingMe() )
	{
		if( slowMultiple == 1 )
		{
			slowCounter = 1;
			slowMultiple = 5;
		}
	}
	else
	{
		slowMultiple = 1;
		slowCounter = 1;
	}

	if( !dead && receivedHit == NULL )
	{
		UpdateHitboxes();

		pair<bool, bool> result = PlayerHitMe();
		if( result.first )
		{
			//cout << "hit here!" << endl;
			//triggers multiple times per frame? bad?
			owner->GetPlayer( 0 )->ConfirmHit( 5, 5, .8, 6 );
			/*owner->GetPlayer( 0 )->test = true;
			owner->GetPlayer( 0 )->currAttackHit = true;
			owner->GetPlayer( 0 )->flashColor = COLOR_BLUE;
			owner->GetPlayer( 0 )->flashFrames = 5;
			owner->GetPlayer( 0 )->currentSpeedBar += .8;
			owner->GetPlayer( 0 )->swordShaders[owner->GetPlayer( 0 )->speedLevel]setUniform( "energyColor", COLOR_BLUE );
			owner->GetPlayer( 0 )->desperationMode = false;
			owner->powerBar.Charge( 2 * 6 * 3 );*/

			if( owner->GetPlayer( 0 )->ground == NULL && owner->GetPlayer( 0 )->velocity.y > 0 )
			{
				owner->GetPlayer( 0 )->velocity.y = 4;//.5;
			}
		}

		if( IHitPlayer() )
		{
		//	cout << "patroller just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}
	}
}

void GrowingTree::Fire()
{
	powerLevel = startPowerLevel;
	launcher->Reset();
	Vector2f start( 0, -pulseRadius );

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

void GrowingTree::UpdatePostPhysics()
{


	launcher->UpdatePostPhysics();

	if( receivedHit != NULL )
	{
		owner->Pause( 5 );
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->GetPlayer( 0 )->position + position ) / 2.0, true, 0, 10, 2, true );
	}

	if( owner->GetPlayer( 0 )->enemiesKilledThisFrame > 0 && powerLevel < 3 )
	{
		double lenPlayer = length( owner->GetPlayer( 0 )->position - position );
		if( lenPlayer < pulseRadius )
		{
			switch( action )
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
			if( powerLevel == 3 )
			{
				pulseFrame = 0;
			}
		}
		
	}

	if( deathFrame == 0 && dying )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, position, true, 0, 15, 2, true );
	}

	if( deathFrame == 60 && dying )
	{
		dying = false;
		dead = true;
		//owner->RemoveEnemy( this );
		//return;
	}

	if( dead && launcher->GetActiveCount() == 0 )
	{
		//cout << "REMOVING" << endl;
		owner->RemoveEnemy( this );
	}

	
	UpdateSprite();
	

	if( slowCounter == slowMultiple )
	{
		++frame;
		slowCounter = 1;
	
		if( dying )
		{
			deathFrame++;
		}

	}
	else
	{
		slowCounter++;
	}

	

	//cout << "dead post: " << dead << endl;
	
}

void GrowingTree::Draw(sf::RenderTarget *target )
{
	if( !dying && !dead )
	{
		target->draw( rangeMarkerVA );
		if( hasMonitor && !suppressMonitor )
		{
			if( owner->pauseFrames < 2 || receivedHit == NULL )
			{
				target->draw( sprite, keyShader );
			}
			else
			{
				target->draw( sprite, hurtShader );
			}
			target->draw( *keySprite );
		}
		else
		{
			if( owner->pauseFrames < 2 || receivedHit == NULL )
			{
				target->draw( sprite );
			}
			else
			{
				target->draw( sprite, hurtShader );
			}
			
		}
	}
	else
	{
		target->draw( botDeathSprite );

		if( deathFrame / 3 < 15 )
		{
			
			//bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
			//bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
			//bloodSprite.setPosition( position.x, position.y );
			////bloodSprite.setScale( 2, 2 );
			//target->draw( bloodSprite );
		}
		
		target->draw( topDeathSprite );
	}
}

void GrowingTree::DrawMinimap( sf::RenderTarget *target )
{
	

	if( !dead )
	{
		if( hasMonitor && !suppressMonitor )
		{
			CircleShape cs;
			cs.setRadius( 50 );
			cs.setFillColor( Color::White );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
		else
		{
			CircleShape cs;
			cs.setRadius( 40 );
			cs.setFillColor( Color::Red );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
	}
}

bool GrowingTree::IHitPlayer( int index )
{
	Actor *player = owner->GetPlayer( 0 );
	
	if( hitBody.Intersects( player->hurtBody ) )
	{
		player->ApplyHit( hitboxInfo );
		return true;
	}
	
	return false;
}

pair<bool, bool> GrowingTree::PlayerHitMe( int index )
{
	Actor *player = owner->GetPlayer( 0 );

	if( player->currHitboxes != NULL )
	{
		bool hit = false;

		for( list<CollisionBox>::iterator it = player->currHitboxes->begin(); it != player->currHitboxes->end(); ++it )
		{
			if( hurtBody.Intersects( (*it) ) )
			{
				hit = true;
				break;
			}
		}
		

		if( hit )
		{
			receivedHit = player->currHitboxInfo;
			return pair<bool, bool>(true,false);
		}
		
	}

	for( int i = 0; i < player->recordedGhosts; ++i )
	{
		if( player->ghostFrame < player->ghosts[i]->totalRecorded )
		{
			if( player->ghosts[i]->currHitboxes != NULL )
			{
				bool hit = false;
				
				for( list<CollisionBox>::iterator it = player->ghosts[i]->currHitboxes->begin(); it != player->ghosts[i]->currHitboxes->end(); ++it )
				{
					if( hurtBody.Intersects( (*it) ) )
					{
						hit = true;
						break;
					}
				}
		

				if( hit )
				{
					receivedHit = player->currHitboxInfo;
					return pair<bool, bool>(true,true);
				}
			}
			//player->ghosts[i]->curhi
		}
	}
	return pair<bool, bool>(false,false);
}

bool GrowingTree::PlayerSlowingMe()
{
	Actor *player = owner->GetPlayer( 0 );
	for( int i = 0; i < player->maxBubbles; ++i )
	{
		if( player->bubbleFramesToLive[i] > 0 )
		{
			if( length( position - player->bubblePos[i] ) <= player->bubbleRadius )
			{
				return true;
			}
		}
	}
	return false;
}

void GrowingTree::UpdateSprite()
{
	launcher->UpdateSprites();
	
	
	switch( action )
	{
	case RECOVER0:
		sprite.setTextureRect( ts->GetSubRect( frame / animFactor[action] + 4 ) );
		break;
	case RECOVER1:
		sprite.setTextureRect( ts->GetSubRect( frame / animFactor[action] + 8 ) );
		break;
	case RECOVER2:
		sprite.setTextureRect( ts->GetSubRect( frame / animFactor[action] + 8 ) );
		break;
	case LEVEL0:
		sprite.setTextureRect( ts->GetSubRect( 0 ) );
		break;
	case LEVEL0TO1:
		sprite.setTextureRect( ts->GetSubRect( 1 ) );
		break;
	case LEVEL1:
		sprite.setTextureRect( ts->GetSubRect( 1 ) );
		break;
	case LEVEL1TO2:
		sprite.setTextureRect( ts->GetSubRect( 2 ) );
		break;
	case LEVEL2:
		sprite.setTextureRect( ts->GetSubRect( 2 ) );
		break;
	case EXPLODE:
		sprite.setTextureRect( ts->GetSubRect( 3 ) );
		break;
	}
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

	if( dead )
	{
		botDeathSprite.setTexture( *ts->texture );
		botDeathSprite.setTextureRect( ts->GetSubRect( 9 ) );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2  );
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );
		botDeathSprite.setRotation( sprite.getRotation() );

		topDeathSprite.setTexture( *ts->texture );
		topDeathSprite.setTextureRect( ts->GetSubRect( 8 ) );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );
		topDeathSprite.setRotation( sprite.getRotation() );
	}
	else
	{
		if( hasMonitor && !suppressMonitor )
		{
			//keySprite.setTexture( *ts_key->texture );
			keySprite->setTextureRect( ts_key->GetSubRect( owner->keyFrame / 5 ) );
			keySprite->setOrigin( keySprite->getLocalBounds().width / 2, 
				keySprite->getLocalBounds().height / 2 );
			keySprite->setPosition( position.x, position.y );

		}
	}
	//sprite.setTextureRect( ts->GetSubRect( frame / animationFactor ) );
}

void GrowingTree::DebugDraw(sf::RenderTarget *target)
{
	if( !dead )
	{
		hurtBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void GrowingTree::DirectKill()
{
	BasicBullet *b = launcher->activeBullets;
	while( b != NULL )
	{
		BasicBullet *next = b->next;
		double angle = atan2( b->velocity.y, -b->velocity.x );
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
		b->launcher->DeactivateBullet( b );

		b = next;
	}

	dying = true;
	health = 0;
	receivedHit = NULL;
}

void GrowingTree::BulletHitTerrain(BasicBullet *b, 
	Edge *edge, V2d &pos)
{
	/*V2d norm = edge->Normal();
	double angle = atan2( norm.y, -norm.x );

	owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true );
	b->launcher->DeactivateBullet( b );*/
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
	hurtBody.globalPosition = position - gn * 10.0;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position - gn * 10.0;
	hitBody.globalAngle = 0;
}

bool GrowingTree::ResolvePhysics( sf::Vector2<double> vel )
{
	return false;
}

void GrowingTree::SaveEnemyState()
{
	stored.dead = dead;
	stored.deathFrame = deathFrame;
	stored.frame = frame;
//	stored.hitlagFrames = hitlagFrames;
//	stored.hitstunFrames = hitstunFrames;
}

void GrowingTree::LoadEnemyState()
{
	dead = stored.dead;
	deathFrame = stored.deathFrame;
	frame = stored.frame;
//	hitlagFrames = stored.hitlagFrames;
//	hitstunFrames = stored.hitstunFrames;
}

