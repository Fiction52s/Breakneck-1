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

Cactus::Cactus( GameSession *owner, bool p_hasMonitor, Edge *g, double q, int p_bulletSpeed,
	int p_rhythm, int p_amplitude )
		:Enemy( owner, EnemyType::CACTUS, p_hasMonitor, 3 ), ground( g ),
		edgeQuantity( q )
{

	bulletSpeed = p_bulletSpeed;
	//rhythm = p_rhythm;
	//amplitude = p_amplitude;
	

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
	hitboxInfo->damage = 18;
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

	//ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	//bloodSprite.setTexture( *ts_testBlood->texture );
	ts_bulletExplode = owner->GetTileset( "bullet_explode2_64x64.png", 64, 64 );

	testLauncher = new Launcher( this, BasicBullet::CACTUS_TURRET, owner, 16, 1, position, gn, 
		0, 90, false, 60, 60 );
	testLauncher->SetBulletSpeed( bulletSpeed );
	testLauncher->hitboxInfo->damage = 18;
	//testLauncher->wavelength = 60;
	//testLauncher->amplitude = 20;
	//testLauncher->SetGravity( gravity );
	//UpdateSprite();
	spawnRect = sf::Rect<double>( gPoint.x - size / 2, gPoint.y - size / 2, size, size );
	dying = false;
}

void Cactus::HandleEntrant( QuadTreeEntrant *qte )
{
	SpecterArea *sa = (SpecterArea*)qte;
	if( sa->barrier.Intersects( hurtBody ) )
	{
		specterProtected = true;
	}
}

void Cactus::BulletHitTerrain(BasicBullet *b, 
		Edge *edge, 
		sf::Vector2<double> &pos)
{
	//never hits terrain
}

void Cactus::BulletHitPlayer(BasicBullet *b )
{
	V2d vel = b->velocity;
	double angle = atan2( vel.y, vel.x );
	owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
	owner->GetPlayer( 0 )->ApplyHit( b->launcher->hitboxInfo );
	b->launcher->DeactivateBullet( b );
	//owner->GetPlayer( 0 )->ApplyHit( b->launcher->hitboxInfo );
}

void Cactus::ResetEnemy()
{
	dead = false;
	dying = false;
	frame = 0;
	deathFrame = 0;
	testLauncher->Reset();
	health = initHealth;
}

void Cactus::UpdatePrePhysics()
{
	specterProtected = false;
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
			if( hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			dying = true;
			owner->GetPlayer( 0 )->ConfirmEnemyKill( this );
		}
		else
		{
			owner->GetPlayer( 0 )->ConfirmEnemyNoKill( this );
		}

		receivedHit = NULL;
	}
	
	
	//if( frame == 12 * animationFactor && slowCounter == 1 )
	if( !dying && !dead && frame == 0 && slowCounter == 1 )
	{
		double pi8 = PI / 8.0;
		double pi4 = PI / 4.0;
		V2d playerPos = owner->GetPlayer( 0 )->position;
		V2d playerDir = normalize( playerPos - position );
		double angle = atan2( playerDir.x, -playerDir.y );
		if( angle < 0 )
			angle += 2.0 * PI;

		

		if( angle > 15.0 * pi8 || angle < pi8 )
		{
			angle = 0;
		}
		else if( angle < 3.0 * pi8 ) 
		{
			angle = pi4;
		}
		else if( angle < 5.0 * pi8 )
		{
			angle = 2.0 * pi4;
		}
		else if( angle < 7.0 * pi8 )
		{
			angle = 3.0 * pi4;
		}
		else if(  angle < 9.0 * pi8 )
		{
			angle = PI;
		}
		else if( angle < 11.0 * pi8 )
		{
			angle = 5.0 * pi4;
		}
		else if( angle < 13.0 * pi8 )
		{
			angle = 6.0 * pi4;
		}
		else if( angle < 15.0 * pi8 )
		{
			angle = 7.0 * pi4;
		}
		
		angle -= PI / 2.0;
		V2d newDir( cos( angle ), sin( angle ) );

		//cout << "angle: " << angle << endl;
		//cout << "launchDir: " << newDir.x << ", " << newDir.y << endl;

		testLauncher->facingDir = playerDir;
		testLauncher->Fire();
	}

	
}

void Cactus::UpdatePhysics()
{
	testLauncher->UpdatePhysics();

	PhysicsResponse();
}

void Cactus::PhysicsResponse()
{
	PlayerSlowingMe();

	//UpdateBulletHitboxes();
	

	//pair<bool, bool> bulletResult = PlayerHitMyBullets(); //not needed for now

	if( !( dead || dying ) )
	{
		UpdateHitboxes();

		
		if( receivedHit == NULL )
		{
			pair<bool, bool> result = PlayerHitMe();
			if( result.first && !specterProtected )
			{
									//	cout << "patroller received damage of: " << receivedHit->damage << endl;
			/*if( !result.second )
			{
				owner->Pause( 6 );
				dead = true;
				receivedHit = NULL;
			}*/

				owner->GetPlayer( 0 )->ConfirmHit( 3, 5, .8, 6 );


				if( owner->GetPlayer( 0 )->ground == NULL && owner->GetPlayer( 0 )->velocity.y > 0 )
				{
					owner->GetPlayer( 0 )->velocity.y = 4;//.5;
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

	//if( IHitPlayerWithBullets() )
	//{
	//}
}

void Cactus::UpdatePostPhysics()
{
	testLauncher->UpdatePostPhysics();

	if( receivedHit != NULL )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->GetPlayer( 0 )->position + position ) / 2.0, true, 0, 10, 2, true );
		owner->Pause( 5 );
	}

	if( deathFrame == 0 && dying )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, position, true, 0, 15, 2, true );
	}

	//cout << "slowcounter: " << slowCounter << endl;

	if( deathFrame == 30 && dying )
	{
		dying = false;
		dead = true;
		//testLauncher->Reset();
		//owner->RemoveEnemy( this );
		//return;
	}

	UpdateSprite();
	testLauncher->UpdateSprites();

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
	
	if( dead && testLauncher->GetActiveCount() == 0 )
	{
		owner->RemoveEnemy( this );
	}
}

void Cactus::Draw(sf::RenderTarget *target )
{
	if( !(dead || dying ) )
	{
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
	else if( dying )
	{
		target->draw( botDeathSprite );

		/*if( deathFrame / 3 < 6 )
		{
			bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
			bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
			bloodSprite.setPosition( position.x, position.y );
			bloodSprite.setScale( 2, 2 );
			target->draw( bloodSprite );
		}*/
		
		target->draw( topDeathSprite );
	}
}

void Cactus::DrawMinimap( sf::RenderTarget *target )
{
	if( !dead && !dying )
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

void Cactus::DirectKill()
{
	BasicBullet *b = testLauncher->activeBullets;
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

bool Cactus::IHitPlayerWithBullets()
{
	return false;
}

bool Cactus::IHitPlayer( int index )
{
	Actor *player = owner->GetPlayer( 0 );
	if( hitBody.Intersects( player->hurtBody ) )
	{
		if( player->position.x < position.x )
		{
			hitboxInfo->kbDir = V2d( -1, -1 );
			//cout << "left" << endl;
		}
		else if( player->position.x > position.x )
		{
			//cout << "right" << endl;
			hitboxInfo->kbDir = V2d( 1, -1 );
		}
		else
		{
			//dont change it
		}

		player->ApplyHit( hitboxInfo );
		return true;
	}
}

 pair<bool, bool> Cactus::PlayerHitMe( int index )
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
			sf::Rect<double> qRect( position.x - hurtBody.rw,
			position.y - hurtBody.rw, hurtBody.rw * 2, 
			hurtBody.rw * 2 );
			owner->specterTree->Query( this, qRect );

			if( !specterProtected )
			{
				receivedHit = player->currHitboxInfo;
				return pair<bool, bool>(true,false);
			}
			else
			{
				return pair<bool, bool>(false,false);
			}
			
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

 pair<bool, bool> Cactus::PlayerHitMyBullets()
 {
	 	return pair<bool, bool>(false,false);
 }

bool Cactus::PlayerSlowingMe()
{
	Actor *player = owner->GetPlayer( 0 );

	bool found = false;
	for( int i = 0; i < player->maxBubbles; ++i )
	{
		if( player->bubbleFramesToLive[i] > 0 )
		{
			if( length( position - player->bubblePos[i] ) <= player->bubbleRadius )
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

	/*for( int i = 0; i < player->maxBubbles; ++i )
		{
			if( player->bubbleFramesToLive[i] > 0 )
			{
				if( length( currBullet->position - player->bubblePos[i] ) 
					<= player->bubbleRadius + currBullet->hurtBody.rw )
				{*/

	
	
	return false;
}

void Cactus::UpdateSprite()
{
	sprite.setTextureRect( ts->GetSubRect( 0 ) );//frame / animationFactor ) );

	//if( frame / animationFactor > 12 )
	//{
	//	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	//}
	//else
	//{
	//	sprite.setTextureRect( ts->GetSubRect( frame / animationFactor  ) );//frame / animationFactor ) );
	//}

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


	if( !dead && !dying && hasMonitor && !suppressMonitor )
	{
		//keySprite.setTexture( *ts_key->texture );
		keySprite->setTextureRect( ts_key->GetSubRect( owner->keyFrame / 5 ) );
		keySprite->setOrigin( keySprite->getLocalBounds().width / 2, 
			keySprite->getLocalBounds().height / 2 );
		keySprite->setPosition( position.x, position.y );
	}
	else if( dying )
	{
		botDeathSprite.setTexture( *ts->texture );
		botDeathSprite.setTextureRect( ts->GetSubRect( 7 ) );
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

void Cactus::DebugDraw(sf::RenderTarget *target)
{
	hitBody.DebugDraw( target );
	hurtBody.DebugDraw( target );
}

void Cactus::UpdateHitboxes()
{
	hurtBody.globalPosition = position;// + gn * 8.0;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position;// + gn * 8.0;
	hitBody.globalAngle = 0;
}


void Cactus::SaveEnemyState()
{
}

void Cactus::LoadEnemyState()
{
}
