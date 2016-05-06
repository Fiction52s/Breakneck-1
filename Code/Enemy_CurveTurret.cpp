#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

CurveTurret::CurveTurret( GameSession *owner, Edge *g, double q, double speed,int wait,
	Vector2i &gravFactor, bool relative )
		:Enemy( owner, EnemyType::CURVETURRET ), framesWait( wait), bulletSpeed( speed ), ground( g ),
		edgeQuantity( q )
{
	
	

	initHealth = 60;
	health = initHealth;

	double width = 64; //112;
	double height = 64;

	//ts = owner->GetTileset( "basicturret_112x64.png", width, height );
	ts = owner->GetTileset( "curveturret_64x64.png", width, height );
	sprite.setTexture( *ts->texture );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height /2 );
	V2d gPoint = g->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	
	

	gn = g->Normal();

	V2d gAlong = normalize( g->v1 - g->v0 );


	gravity = V2d( 0,0 );

	if( relative )
	{
		gravity += gAlong * ( gravFactor.x / 256.0);
		gravity += gn * ( -gravFactor.y / 256.0 );
	}
	else
	{
		gravity = V2d( gravFactor.x / 256.0, gravFactor.y / 256.0 );
	}
	

	position = gPoint + gn * height / 2.0;

	angle = atan2( gn.x, -gn.y );

	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );
	//V2d gPoint = ground->GetPoint( edgeQuantity );
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
	hitboxInfo->damage = 100;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;

	frame = 0;
	deathFrame = 0;
	animationFactor = 3;
	//slowCounter = 1;
	//slowMultiple = 1;

	//bulletSpeed = 5;

	dead = false;

	double size = max( width, height );

	deathPartingSpeed = .4;

	Transform t;
	t.rotate( angle / PI * 180 );
	Vector2f newPoint = t.transformPoint( Vector2f( -1, -1 ) );
	deathVector = V2d( newPoint.x, newPoint.y );

	ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	bloodSprite.setTexture( *ts_testBlood->texture );

	testLauncher = new Launcher( this, owner, 16, 1, position, gn, 0, 300 );
	testLauncher->SetBulletSpeed( bulletSpeed );
	testLauncher->SetGravity( gravity );
	//UpdateSprite();
	spawnRect = sf::Rect<double>( gPoint.x - size / 2, gPoint.y - size / 2, size, size );
	dying = false;
}

void CurveTurret::HandleEntrant( QuadTreeEntrant *qte )
{
}

void CurveTurret::BulletHitTerrain(BasicBullet *b, 
		Edge *edge, 
		sf::Vector2<double> &pos)
{
	b->launcher->DeactivateBullet( b );
}

void CurveTurret::BulletHitPlayer(BasicBullet *b )
{
	owner->player.ApplyHit( b->launcher->hitboxInfo );
}

void CurveTurret::ResetEnemy()
{
	dead = false;
	dying = false;
	frame = 0;
	deathFrame = 0;
	testLauncher->Reset();
	health = initHealth;
}

void CurveTurret::UpdatePrePhysics()
{
	testLauncher->UpdatePrePhysics();
	

	if( frame == 26 * animationFactor )
	{
		frame = 0;
	}

	if( !dead && !dying && receivedHit != NULL )
	{	
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			AttemptSpawnMonitor();
			dying = true;
		}

		receivedHit = NULL;
	}
	
	
	//if( frame == 12 * animationFactor && slowCounter == 1 )
	if( !dying && !dead && frame == 0 && slowCounter == 1 )
	{
		testLauncher->Fire();
	}

	
}

void CurveTurret::UpdatePhysics()
{
	testLauncher->UpdatePhysics();

	PhysicsResponse();
}

void CurveTurret::PhysicsResponse()
{
	PlayerSlowingMe();

	//UpdateBulletHitboxes();
	

	pair<bool, bool> bulletResult = PlayerHitMyBullets(); //not needed for now

	if( !( dead || dying ) )
	{
		UpdateHitboxes();

		
		if( receivedHit == NULL )
		{
			pair<bool, bool> result = PlayerHitMe();
			if( result.first )
			{
									//	cout << "patroller received damage of: " << receivedHit->damage << endl;
			/*if( !result.second )
			{
				owner->Pause( 6 );
				dead = true;
				receivedHit = NULL;
			}*/

				owner->player.test = true;
				owner->player.currAttackHit = true;
				owner->player.flashColor = COLOR_BLUE;
				owner->player.flashFrames = 5;
				owner->player.currentSpeedBar += .8;
				owner->player.swordShader.setParameter( "energyColor", COLOR_BLUE );
				owner->player.desperationMode = false;
				owner->powerBar.Charge( 2 * 6 * 3 );


				if( owner->player.ground == NULL && owner->player.velocity.y > 0 )
				{
					owner->player.velocity.y = 4;//.5;
				}
				//	dead = true;
		//	receivedHit = NULL;
			}
		}

		

		if( IHitPlayer() )
		{
		//	cout << "patroller just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}

		
	}

	if( IHitPlayerWithBullets() )
	{
	}
}

void CurveTurret::UpdatePostPhysics()
{
	testLauncher->UpdatePostPhysics();

	if( receivedHit != NULL )
	{
		owner->Pause( 5 );
	}

	

	
	//cout << "slowcounter: " << slowCounter << endl;
	if( slowCounter == slowMultiple )
	{
		
		++frame;		
	//	cout << "frame" << endl;
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
	

	if( deathFrame == 30 && dying )
	{
		dying = false;
		dead = true;
		//testLauncher->Reset();
		//owner->RemoveEnemy( this );
		//return;
	}

	if( dead && testLauncher->GetActiveCount() == 0 )
	{
		owner->RemoveEnemy( this );
	}

	
	UpdateSprite();
	testLauncher->UpdateSprites();
}

void CurveTurret::Draw(sf::RenderTarget *target )
{
	if( !(dead || dying ) )
	{
		if( monitor != NULL )
		{
			//owner->AddEnemy( monitor );
			CircleShape cs;
			cs.setRadius( 40 );
			cs.setFillColor( COLOR_BLUE );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
		target->draw( sprite );
	}
	else if( dying )
	{
		target->draw( botDeathSprite );

		if( deathFrame / 3 < 6 )
		{
			bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
			bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
			bloodSprite.setPosition( position.x, position.y );
			bloodSprite.setScale( 2, 2 );
			target->draw( bloodSprite );
		}
		
		target->draw( topDeathSprite );
	}
	

	/*if( activeBullets != NULL )
	{
		target->draw( bulletVA, ts_bullet->texture );
	}*/
	
}

void CurveTurret::DrawMinimap( sf::RenderTarget *target )
{
	if( !(dead || dying) )
	{
		CircleShape cs;
		cs.setRadius( 50 );
		cs.setFillColor( COLOR_BLUE );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( position.x, position.y );
		target->draw( cs );

		if( monitor != NULL )
		{
			monitor->miniSprite.setPosition( position.x, position.y );
			target->draw( monitor->miniSprite );
		}
	}
}

bool CurveTurret::IHitPlayerWithBullets()
{
	/*Actor &player = owner->player;
	
	Bullet *currBullet = activeBullets;
	while( currBullet != NULL )
	{
		if( currBullet->hitBody.Intersects( player.hurtBody ) )
		{
			player.ApplyHit( bulletHitboxInfo );
			return true;
		}
		currBullet = currBullet->next;
	}*/

	
	return false;
}

bool CurveTurret::IHitPlayer()
{
	Actor &player = owner->player;
	if( hitBody.Intersects( player.hurtBody ) )
	{
		if( player.position.x < position.x )
		{
			hitboxInfo->kbDir = V2d( -1, -1 );
			//cout << "left" << endl;
		}
		else if( player.position.x > position.x )
		{
			//cout << "right" << endl;
			hitboxInfo->kbDir = V2d( 1, -1 );
		}
		else
		{
			//dont change it
		}

		player.ApplyHit( hitboxInfo );
		return true;
	}
}

 pair<bool, bool> CurveTurret::PlayerHitMe()
{
	Actor &player = owner->player;

	if( player.currHitboxes != NULL )
	{
		bool hit = false;

		for( list<CollisionBox>::iterator it = player.currHitboxes->begin(); it != player.currHitboxes->end(); ++it )
		{
			if( hurtBody.Intersects( (*it) ) )
			{
				hit = true;
				break;
			}
		}
		

		if( hit )
		{
			receivedHit = player.currHitboxInfo;
			return pair<bool, bool>(true,false);
		}
		
	}

	for( int i = 0; i < player.recordedGhosts; ++i )
	{
		if( player.ghostFrame < player.ghosts[i]->totalRecorded )
		{
			if( player.ghosts[i]->currHitboxes != NULL )
			{
				bool hit = false;
				
				for( list<CollisionBox>::iterator it = player.ghosts[i]->currHitboxes->begin(); it != player.ghosts[i]->currHitboxes->end(); ++it )
				{
					if( hurtBody.Intersects( (*it) ) )
					{
						hit = true;
						break;
					}
				}
		

				if( hit )
				{
					receivedHit = player.currHitboxInfo;
					return pair<bool, bool>(true,true);
				}
			}
			//player.ghosts[i]->curhi
		}
	}
	return pair<bool, bool>(false,false);
}

 pair<bool, bool> CurveTurret::PlayerHitMyBullets()
 {
	 	return pair<bool, bool>(false,false);
 }

bool CurveTurret::PlayerSlowingMe()
{
	Actor &player = owner->player;

	/*Bullet *currBullet = activeBullets;
	while( currBullet != NULL )
	{
		bool slowed = false;
		for( int i = 0; i < player.maxBubbles; ++i )
		{
			if( player.bubbleFramesToLive[i] > 0 )
			{
				if( length( currBullet->position - player.bubblePos[i] ) 
					<= player.bubbleRadius + currBullet->hurtBody.rw )
				{
					if( currBullet->slowMultiple == 1 )
					{
						currBullet->slowCounter = 1;
						currBullet->slowMultiple = 5;
					}

					slowed = true;
					break;
				}
			}
		}
		if( !slowed )
		{
			currBullet->slowCounter = 1;
			currBullet->slowMultiple = 1;
		}
		currBullet = currBullet->next;
	}*/

	//Actor &player = owner->player;
	bool found = false;
	for( int i = 0; i < player.maxBubbles; ++i )
	{
		if( player.bubbleFramesToLive[i] > 0 )
		{
			if( length( position - player.bubblePos[i] ) <= player.bubbleRadius )
			{
				found = true;
				if( slowMultiple == 1 )
				{
					slowCounter = 1;
					slowMultiple = 5;
				}
				break;
			}
		}
	}

	if( !found )
	{
		slowCounter = 1;
		slowMultiple = 1;
	}

	/*for( int i = 0; i < player.maxBubbles; ++i )
		{
			if( player.bubbleFramesToLive[i] > 0 )
			{
				if( length( currBullet->position - player.bubblePos[i] ) 
					<= player.bubbleRadius + currBullet->hurtBody.rw )
				{*/

	
	
	return false;
}

void CurveTurret::UpdateSprite()
{
	sprite.setTextureRect( ts->GetSubRect( 0 ) );//frame / animationFactor ) );

	//int i = 0;
	//Bullet *currBullet = activeBullets;
	//int rad = 16;
	//while( currBullet != NULL )
	//{	
	//	bulletVA[i*4].position = Vector2f( currBullet->position.x - rad, currBullet->position.y - rad );
	//	bulletVA[i*4+1].position = Vector2f( currBullet->position.x + rad, currBullet->position.y - rad );
	//	bulletVA[i*4+2].position = Vector2f( currBullet->position.x + rad, currBullet->position.y + rad );
	//	bulletVA[i*4+3].position = Vector2f( currBullet->position.x - rad, currBullet->position.y + rad );

	//	sf::IntRect rect = ts_bullet->GetSubRect( currBullet->frame );

	//	bulletVA[i*4].texCoords = Vector2f( rect.left, rect.top );
	//	bulletVA[i*4+1].texCoords = Vector2f( rect.left + rect.width, rect.top );
	//	bulletVA[i*4+2].texCoords = Vector2f( rect.left + rect.width, rect.top + rect.height );
	//	bulletVA[i*4+3].texCoords = Vector2f( rect.left, rect.top + rect.height );
	//	
	//	currBullet = currBullet->next;
	//	++i;
	//}

	//Bullet *notBullet = inactiveBullets;
	////i = 0;
	//while( notBullet != NULL )
	//{
	//	bulletVA[i*4].position = Vector2f( 0,0 );
	//	bulletVA[i*4+1].position = Vector2f( 0,0 );
	//	bulletVA[i*4+2].position = Vector2f( 0,0 );
	//	bulletVA[i*4+3].position = Vector2f( 0,0 );

	//	bulletVA[i*4].texCoords = Vector2f( 0,0 );
	//	bulletVA[i*4+1].texCoords = Vector2f( 0,0 );
	//	bulletVA[i*4+2].texCoords = Vector2f( 0,0 );
	//	bulletVA[i*4+3].texCoords = Vector2f( 0,0 );

	//	++i;
	//	notBullet = notBullet->next;
	//}

	if( dead )
	{
		botDeathSprite.setTexture( *ts->texture );
		botDeathSprite.setTextureRect( ts->GetSubRect( 0 ) );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, 
			botDeathSprite.getLocalBounds().height / 2  );
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );
		botDeathSprite.setRotation( sprite.getRotation() );

		topDeathSprite.setTexture( *ts->texture );
		topDeathSprite.setTextureRect( ts->GetSubRect( 0 ) );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, 
			topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );
		topDeathSprite.setRotation( sprite.getRotation() );
	}
}

void CurveTurret::DebugDraw(sf::RenderTarget *target)
{
	//Bullet *currBullet = activeBullets;
	//while( currBullet != NULL )
	//{
	//	currBullet->hitBody.DebugDraw( target );
	//	//currBullet->hurtBody.DebugDraw( target );

	//	currBullet = currBullet->next;
	//}

	/*sf::CircleShape cs;
	cs.setFillColor( Color( 0, 255, 0, 100 ) );
	cs.setRadius( 15 );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );*/
	
	//target->draw( cs );

	hitBody.DebugDraw( target );
	hurtBody.DebugDraw( target );
}

void CurveTurret::UpdateHitboxes()
{
	hurtBody.globalPosition = position;// + gn * 8.0;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position;// + gn * 8.0;
	hitBody.globalAngle = 0;
}


void CurveTurret::SaveEnemyState()
{
}

void CurveTurret::LoadEnemyState()
{
}
