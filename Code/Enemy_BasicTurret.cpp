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

BasicTurret::BasicTurret( GameSession *owner, bool p_hasMonitor, Edge *g, double q, double speed,int wait )
		:Enemy( owner, EnemyType::BASICTURRET, p_hasMonitor, 1 ), framesWait( wait), bulletSpeed( speed ), firingCounter( 0 ), ground( g ),
		edgeQuantity( q ), bulletVA( sf::Quads, maxBullets * 4 )
{
	receivedHit = NULL;
	//keyFrame = 0;
	//ts_key = owner->GetTileset( "key_w02_1_128x128.png", 128, 128 );

	//launcher = new Launcher( this, 

	initHealth = 60;
	health = initHealth;

	double width = 112;
	double height = 64;

	ts = owner->GetTileset( "basicturret_112x64.png", width, height );
	sprite.setTexture( *ts->texture );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height /2 );
	V2d gPoint = g->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	
	ts_bulletExplode = owner->GetTileset( "bullet_explode1_64x64.png", 64, 64 );

	gn = g->Normal();

	position = gPoint + gn * height / 2.0;

	angle = atan2( gn.x, -gn.y );

	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );
	//V2d gPoint = ground->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	sprite.setRotation( angle / PI * 180 );

	
	ts_bullet = owner->GetTileset( "basicbullet_32x32.png", 32, 32 );


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

	activeBullets = NULL;
	inactiveBullets = NULL;


	for( int i = 0; i < maxBullets; ++i )
	{
		AddBullet();
	}

	
	bulletHitboxInfo = new HitboxInfo;
	bulletHitboxInfo->damage = 40;
	bulletHitboxInfo->drainX = 0;
	bulletHitboxInfo->drainY = 0;
	bulletHitboxInfo->hitlagFrames = 0;
	bulletHitboxInfo->hitstunFrames = 10;
	bulletHitboxInfo->knockback = 0;

	frame = 0;
	deathFrame = 0;
	animationFactor = 3;
	//slowCounter = 1;
	//slowMultiple = 1;

	//bulletSpeed = 5;

	dead = false;
	dying = false;

	double size = max( width, height );

	deathPartingSpeed = .4;

	Transform t;
	t.rotate( angle / PI * 180 );
	Vector2f newPoint = t.transformPoint( Vector2f( -1, -1 ) );
	deathVector = V2d( newPoint.x, newPoint.y );

	launcher = new Launcher( this, BasicBullet::BASIC_TURRET, owner, 16, 1, position, gn, 0, 300 );
	launcher->SetBulletSpeed( bulletSpeed );

	//UpdateSprite();
	spawnRect = sf::Rect<double>( gPoint.x - size / 2, gPoint.y - size / 2, size, size );
}

void BasicTurret::ResetEnemy()
{
	health = initHealth;
	launcher->Reset();
	dying = false;
	dead = false;
	frame = 0;
	deathFrame = 0;
	while( activeBullets != NULL )
	{
		DeactivateBullet( activeBullets );
	}
}

void BasicTurret::HandleEntrant( QuadTreeEntrant *qte )
{
	//basic turret needs to be redone so add in the specter stuff then
	//cout << "handling entrant" << endl;
	Edge *e = (Edge*)qte;

	if( e == ground )
		return;

	Contact *c = owner->coll.collideEdge( queryBullet->position + tempVel, queryBullet->physBody, e, tempVel, V2d( 0, 0 ) );
	

	if( c != NULL )
	{
		//cout << "touched something at all" << endl;
		if( !col )
		{
			minContact = *c;
			col = true;
		}
		else if( c->collisionPriority < minContact.collisionPriority )
		{
			minContact = *c;
		}
	}
	//Contact *c = owner->coll.collideEdge( queryBullet->position, queryBullet->physBody, e, tempVel, owner->window );

}

void BasicTurret::BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos )
{
	V2d norm = edge->Normal();
	double angle = atan2( norm.y, -norm.x );

	owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true );
	b->launcher->DeactivateBullet( b );
}

void BasicTurret::BulletHitPlayer( BasicBullet *b )
{
	//if you dont deactivate the bullet it will hit constantly and make weird fx

	//cout << "hit player??" << endl;
	V2d vel = b->velocity;
	double angle = atan2( vel.y, vel.x );
	owner->ActivateEffect( EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true );
	owner->player->ApplyHit( b->launcher->hitboxInfo );
	b->launcher->DeactivateBullet( b );
}

void BasicTurret::UpdatePrePhysics()
{
	launcher->UpdatePrePhysics();

	if( !dead && !dying )
	{

	if( frame == 26 * animationFactor )
	{
		frame = 0;
	}

	if( receivedHit != NULL )
	{	
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			if( hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			//AttemptSpawnMonitor();
			dying = true;
			owner->player->ConfirmEnemyKill( this );
		}
		else
		{
			owner->player->ConfirmEnemyNoKill( this );
		}

		receivedHit = NULL;
	}


//	DeactivateBullet( currBullet );
	Bullet *currBullet = activeBullets;
	while( currBullet != NULL )
	{
		Bullet *next = currBullet->next;
		if( currBullet->framesToLive == 0 )
		{
			//put this in post physics too?
			DeactivateBullet( currBullet );
		}
		else
		{
			//if( currBullet->frame == 12 )
			//	currBullet->frame = 0;
		}
		currBullet = next;
	}
	

	//if( frame == 12 * animationFactor && slowCounter == 1 )
	if( frame == 0 && slowCounter == 1 )
	{
		launcher->Fire();
	}

	}
}

void BasicTurret::UpdatePhysics()
{
	launcher->UpdatePhysics();
	specterProtected = false;
	Bullet *currBullet = activeBullets;
	int i = 0;
	while( currBullet != NULL )
	{

		Bullet *next = currBullet->next;
		//cout << "moving bullet" << endl;

		double movement = bulletSpeed / (double)currBullet->slowMultiple / NUM_STEPS;
		//cout << "movement at bullet " << i << ": "  << movement << endl;
		double speed;
		while( movement > 0 )
		{
			if( movement > 8 )
			{
				movement -= 8;
				speed = 8;
			}
			else
			{
				speed = movement;
				movement = 0;
			}

			if( ResolvePhysics( currBullet, gn * speed ) )
			{
				DeactivateBullet( currBullet );
				break;
			}
		}

		//currBullet->position += gn * bulletSpeed;

		currBullet = next;
		++i;
	}

	PhysicsResponse();
}

void BasicTurret::PhysicsResponse()
{
	PlayerSlowingMe();

	UpdateBulletHitboxes();
	

	pair<bool, bool> bulletResult = PlayerHitMyBullets(); //not needed for now

	if( !dead && !dying )
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

				owner->player->ConfirmHit( COLOR_BLUE, 5, .8, 2 * 6 * 3 );


				if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
				{
					owner->player->velocity.y = 4;//.5;
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

void BasicTurret::UpdatePostPhysics()
{
	launcher->UpdatePostPhysics();
	if( receivedHit != NULL )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->player->position + position ) / 2.0, true, 0, 10, 2, true );
		owner->Pause( 5 );
	}

	if( deathFrame == 0 && dying )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, position, true, 0, 15, 2, true );
	}

	if( deathFrame == 30 && dying )
	{
		dying = false;
		dead = true;
	}

	Bullet *currBullet = activeBullets;
	while( currBullet != NULL )
	{
		if( currBullet->slowCounter == currBullet->slowMultiple )
		{
			currBullet->frame++;
			if( currBullet->frame == 12 )
			{
				currBullet->frame = 0;
			}
			currBullet->framesToLive--;
			currBullet->slowCounter = 1;
		}
		else
		{
			currBullet->slowCounter++;
		}

			
		//++frame;
		

		currBullet = currBullet->next;
	}
	
	UpdateSprite();
	launcher->UpdateSprites();

	//cout << "slowcounter: " << slowCounter << endl;
	if( slowCounter == slowMultiple )
	{
		++keyFrame;
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

	if( dead && launcher->GetActiveCount() == 0 )
	{
		owner->RemoveEnemy( this );
	}
}


void BasicTurret::DirectKill()
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

void BasicTurret::Draw(sf::RenderTarget *target )
{
	if( !dead && !dying )
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
	else if( !dead )
	{
		target->draw( botDeathSprite );

		if( deathFrame / 3 < 6 )
		{
			/*bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
			bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
			bloodSprite.setPosition( position.x, position.y );
			bloodSprite.setScale( 2, 2 );
			target->draw( bloodSprite );*/
		}
		
		target->draw( topDeathSprite );
	}
	

	if( activeBullets != NULL )
	{
		target->draw( bulletVA, ts_bullet->texture );
	}
	
}

void BasicTurret::DrawMinimap( sf::RenderTarget *target )
{
	if( !dead && !dying && hasMonitor && !suppressMonitor )
	{
		CircleShape cs;
		cs.setRadius( 50 );
		cs.setFillColor( Color::White );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( position.x, position.y );
		target->draw( cs );
	}
}

bool BasicTurret::IHitPlayerWithBullets()
{
	Actor *player = owner->player;
	
	Bullet *currBullet = activeBullets;
	while( currBullet != NULL )
	{
		if( currBullet->hitBody.Intersects( player->hurtBody ) )
		{
			player->ApplyHit( bulletHitboxInfo );
			return true;
		}
		currBullet = currBullet->next;
	}

	
	return false;
}

bool BasicTurret::IHitPlayer()
{
	Actor *player = owner->player;
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

 pair<bool, bool> BasicTurret::PlayerHitMe()
{
	Actor *player = owner->player;

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

 pair<bool, bool> BasicTurret::PlayerHitMyBullets()
 {
	 	return pair<bool, bool>(false,false);
 }

bool BasicTurret::PlayerSlowingMe()
{
	Actor *player = owner->player;

	Bullet *currBullet = activeBullets;
	while( currBullet != NULL )
	{
		bool slowed = false;
		for( int i = 0; i < player->maxBubbles; ++i )
		{
			if( player->bubbleFramesToLive[i] > 0 )
			{
				if( length( currBullet->position - player->bubblePos[i] ) 
					<= player->bubbleRadius + currBullet->hurtBody.rw )
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
	}

	//Actor *player = owner->player;
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

void BasicTurret::UpdateSprite()
{
	sprite.setTextureRect( ts->GetSubRect( frame / animationFactor / 14 ) );//frame / animationFactor ) );

	int i = 0;
	Bullet *currBullet = activeBullets;
	int rad = 16;
	while( currBullet != NULL )
	{	
		bulletVA[i*4].position = Vector2f( currBullet->position.x - rad, currBullet->position.y - rad );
		bulletVA[i*4+1].position = Vector2f( currBullet->position.x + rad, currBullet->position.y - rad );
		bulletVA[i*4+2].position = Vector2f( currBullet->position.x + rad, currBullet->position.y + rad );
		bulletVA[i*4+3].position = Vector2f( currBullet->position.x - rad, currBullet->position.y + rad );

		sf::IntRect rect = ts_bullet->GetSubRect( currBullet->frame );

		bulletVA[i*4].texCoords = Vector2f( rect.left, rect.top );
		bulletVA[i*4+1].texCoords = Vector2f( rect.left + rect.width, rect.top );
		bulletVA[i*4+2].texCoords = Vector2f( rect.left + rect.width, rect.top + rect.height );
		bulletVA[i*4+3].texCoords = Vector2f( rect.left, rect.top + rect.height );
		
		currBullet = currBullet->next;
		++i;
	}

	Bullet *notBullet = inactiveBullets;
	//i = 0;
	while( notBullet != NULL )
	{
		bulletVA[i*4].position = Vector2f( 0,0 );
		bulletVA[i*4+1].position = Vector2f( 0,0 );
		bulletVA[i*4+2].position = Vector2f( 0,0 );
		bulletVA[i*4+3].position = Vector2f( 0,0 );

		bulletVA[i*4].texCoords = Vector2f( 0,0 );
		bulletVA[i*4+1].texCoords = Vector2f( 0,0 );
		bulletVA[i*4+2].texCoords = Vector2f( 0,0 );
		bulletVA[i*4+3].texCoords = Vector2f( 0,0 );

		++i;
		notBullet = notBullet->next;
	}

	if( dying && !dead )
	{
		botDeathSprite.setTexture( *ts->texture );
		botDeathSprite.setTextureRect( ts->GetSubRect( 2 ) );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, 
			botDeathSprite.getLocalBounds().height / 2  );
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );
		botDeathSprite.setRotation( sprite.getRotation() );

		topDeathSprite.setTexture( *ts->texture );
		topDeathSprite.setTextureRect( ts->GetSubRect( 3 ) );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, 
			topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );
		topDeathSprite.setRotation( sprite.getRotation() );
	}
	else
	{
		if( hasMonitor && !suppressMonitor )
		{
			//keySprite.setTexture( *ts_key->texture );
			keySprite->setTextureRect( ts_key->GetSubRect( keyFrame / 2 ) );
			keySprite->setOrigin( keySprite->getLocalBounds().width / 2, 
				keySprite->getLocalBounds().height / 2 );
			keySprite->setPosition( position.x, position.y );

		}
	}
}

void BasicTurret::DebugDraw(sf::RenderTarget *target)
{
	Bullet *currBullet = activeBullets;
	while( currBullet != NULL )
	{
		currBullet->hitBody.DebugDraw( target );
		//currBullet->hurtBody.DebugDraw( target );

		currBullet = currBullet->next;
	}

	/*sf::CircleShape cs;
	cs.setFillColor( Color( 0, 255, 0, 100 ) );
	cs.setRadius( 15 );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );*/
	
	//target->draw( cs );

	hitBody.DebugDraw( target );
	hurtBody.DebugDraw( target );
}

void BasicTurret::UpdateHitboxes()
{
	hurtBody.globalPosition = position;// + gn * 8.0;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position;// + gn * 8.0;
	hitBody.globalAngle = 0;
}

void BasicTurret::UpdateBulletHitboxes()
{
	Bullet *currBullet = activeBullets;
	while( currBullet != NULL )
	{
		currBullet->hurtBody.globalPosition = currBullet->position;
		currBullet->hurtBody.globalAngle = 0;
		currBullet->hitBody.globalPosition = currBullet->position;
		currBullet->hitBody.globalAngle = 0;

		currBullet = currBullet->next;
	}
}

bool BasicTurret::ResolvePhysics( BasicTurret::Bullet * bullet, sf::Vector2<double> vel )
{
	possibleEdgeCount = 0;
	bullet->position += vel;
	
	Rect<double> r( bullet->position.x - 16, bullet->position.y - 16, 
		2 * 16, 2 * 16 );
	minContact.collisionPriority = 1000000;

	col = false;

	tempVel = vel;
	minContact.edge = NULL;

	//queryMode = "resolve";
//	Query( this, owner->testTree, r );
	queryBullet = bullet;
	owner->terrainTree->Query( this, r );

	return col;
}

void BasicTurret::SaveEnemyState()
{
}

void BasicTurret::LoadEnemyState()
{
}

void BasicTurret::AddBullet()
{
	if( inactiveBullets == NULL )
	{
		inactiveBullets = new Bullet;
		inactiveBullets->prev = NULL;
		inactiveBullets->next = NULL;
	}
	else
	{
		Bullet *b = new Bullet;
		b->next = inactiveBullets;
		inactiveBullets->prev = b;
		//b = inactiveBullets;
		inactiveBullets = b;
	}

	double rad = 12;
	inactiveBullets->hurtBody.isCircle = true;
	inactiveBullets->hurtBody.globalAngle = 0;
	inactiveBullets->hurtBody.offset.x = 0;
	inactiveBullets->hurtBody.offset.y = 0;
	inactiveBullets->hurtBody.rw = rad;
	inactiveBullets->hurtBody.rh = rad;

	inactiveBullets->hitBody.type = CollisionBox::Hit;
	inactiveBullets->hitBody.isCircle = true;
	inactiveBullets->hitBody.globalAngle = 0;
	inactiveBullets->hitBody.offset.x = 0;
	inactiveBullets->hitBody.offset.y = 0;
	inactiveBullets->hitBody.rw = rad;
	inactiveBullets->hitBody.rh = rad;

	inactiveBullets->physBody.type = CollisionBox::Physics;
	inactiveBullets->physBody.isCircle = true;
	inactiveBullets->physBody.globalAngle = 0;
	inactiveBullets->physBody.offset.x = 0;
	inactiveBullets->physBody.offset.y = 0;
	inactiveBullets->physBody.rw = rad;
	inactiveBullets->physBody.rh = rad;
}

void BasicTurret::DeactivateBullet( Bullet *b )
{
	//cout << "deactivating" << endl;
	Bullet *prev = b->prev;
	Bullet *next = b->next;

	if( prev == NULL && next == NULL )
	{
		activeBullets = NULL;
	}
	else
	{
		if( b == activeBullets )
		{
			if( next != NULL )
			{
				next->prev = NULL;
			}
			
			activeBullets = next;
		}
		else
		{
			if( prev != NULL )
			{
				prev->next = next;
			}

			if( next != NULL )
			{
				next->prev = prev;
			}
		}
		
	}


	if( inactiveBullets == NULL )
	{
		b->next = NULL;
		b->prev = NULL;
		inactiveBullets = b;
	}
	else
	{
		b->prev = NULL;
		b->next = inactiveBullets;
		inactiveBullets->prev = b;
		inactiveBullets = b;
	}
}

BasicTurret::Bullet * BasicTurret::ActivateBullet()
{
	if( inactiveBullets == NULL )
	{
		return NULL;
	}
	else
	{
		Bullet *oldStart = inactiveBullets;
		Bullet *newStart = inactiveBullets->next;

		if( newStart != NULL )
		{
			newStart->prev = NULL;	
		}
		inactiveBullets = newStart;

		

		if( activeBullets == NULL )
		{
			activeBullets = oldStart;
			//oldStart->prev = NULL;
			oldStart->next = NULL;
		}
		else
		{
			//oldStart->prev = NULL;
			oldStart->next = activeBullets;
			activeBullets->prev = oldStart;
			activeBullets = oldStart;
		}

		
		return oldStart;
	}
}

BasicTurret::Bullet::Bullet()
	:prev( NULL ), next( NULL ), frame( 0 ), slowCounter( 1 ), slowMultiple( 1 ), maxFramesToLive( 120 )
{
	//framesToLive = maxFramesToLive;
}