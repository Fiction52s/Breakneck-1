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

BasicTurret::BasicTurret( GameSession *owner, Edge *g, double q, double speed,int wait )
		:Enemy( owner, EnemyType::BASICTURRET ), framesWait( wait), bulletSpeed( speed ), firingCounter( 0 ), ground( g ),
		edgeQuantity( q ), bulletVA( sf::Quads, maxBullets * 4 )
{
	initHealth = 80;
	health = initHealth;

	double height = 32;

	ts = owner->GetTileset( "basicturret.png", 64, height );
	sprite.setTexture( *ts->texture );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height /2 );
	V2d gPoint = g->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	
	

	gn = g->Normal();

	position = gPoint + gn * height / 2.0;

	angle = atan2( gn.x, -gn.y );

	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );
	//V2d gPoint = ground->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	sprite.setRotation( angle / PI * 180 );

	
	ts_bullet = owner->GetTileset( "basicbullet.png", 16, 16 );


	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 16;
	hurtBody.rh = 16;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = 16;
	hitBody.rh = 16;
	
	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 100;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 0;

	activeBullets = NULL;
	inactiveBullets = NULL;


	for( int i = 0; i < maxBullets; ++i )
	{
		AddBullet();
	}

	
	bulletHitboxInfo = new HitboxInfo;
	bulletHitboxInfo->damage = 100;
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

	spawnRect = sf::Rect<double>( gPoint.x - 24, gPoint.y - 24, 24 * 2, 24 * 2 );
}

void BasicTurret::ResetEnemy()
{
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
	//cout << "handling entrant" << endl;
	Edge *e = (Edge*)qte;

	if( e == ground )
		return;

	/*V2d v0 = e->v0;
	V2d v1 = e->v1;

	double result = cross( queryBullet->position - v0, normalize( v1 - v0 ) );
	double d = dot( queryBullet->position - v0, normalize( v1 - v0 ) );

	bool a = d >= -queryBullet->physBody.rw && d <= length( v1 - v0 ) + queryBullet->physBody.rw;
	bool b = result >= 0 && result <= queryBullet->physBody.rw;
	//cout << "rw: " << queryBullet->physBody.rw << endl;

	//cout << "D: " << d << ", " << "result: "  << result << ", length: " << length( v1 - v0 ) << " : a,b: " <<
	//	a << ", " << b << endl;
	

	if( a && b ) 
	{
		//cout << "bullet hit edge" << endl;
		col = true;
	}*/

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

void BasicTurret::UpdatePrePhysics()
{
	if( !dead && receivedHit != NULL )
	{	
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			AttemptSpawnMonitor();
			dead = true;
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
			DeactivateBullet( currBullet );
		}
		currBullet = next;
	}
	

	if( frame == 12 * animationFactor && slowCounter == 1 )
	{
		//cout << "firing" << endl;
		Bullet *b = ActivateBullet();
		if( b != NULL )
		{
			//cout << "firing bullet" << endl;
			b->position = position ;//+ ground->Normal() * 16.0;
			b->slowCounter = 1;
			b->slowMultiple = 1;
			b->framesToLive = b->maxFramesToLive;
		}
		else
		{
			//cout << "unable to make bullet" << endl;
		}
	}


}

void BasicTurret::UpdatePhysics()
{
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
	Bullet *currBullet = activeBullets;
	while( currBullet != NULL )
	{
		if( currBullet->slowCounter == currBullet->slowMultiple )
		{
			currBullet->frame++;
			currBullet->slowCounter = 1;
		}
		else
		{
			currBullet->slowCounter++;
		}

			
		//++frame;
		if( currBullet->frame == 11 )
			currBullet->frame = 0;

		currBullet = currBullet->next;
	}

	pair<bool, bool> bulletResult = PlayerHitMyBullets(); //not needed for now

	if( !dead )
	{
		UpdateHitboxes();

		

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

		

		if( IHitPlayer() )
		{
		//	cout << "patroller just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}

		if( IHitPlayerWithBullets() )
		{
		}

		if( frame == 26 * animationFactor )
		{
			frame = 0;
		}
	}
}

void BasicTurret::UpdatePostPhysics()
{
	if( receivedHit != NULL )
	{
		owner->Pause( 5 );
	}

	
	//cout << "slowcounter: " << slowCounter << endl;
	if( slowCounter == slowMultiple )
	{
		
		++frame;
		Bullet *currBullet = activeBullets;
		while( currBullet != NULL )
		{
			currBullet->framesToLive--;
			currBullet = currBullet->next;
		}
	//	cout << "frame" << endl;
		slowCounter = 1;
	
		if( dead )
		{
			deathFrame++;
		}

	}
	else
	{
		slowCounter++;
	}

	

	if( deathFrame == 60 )
	{
		owner->RemoveEnemy( this );
	}

	UpdateSprite();
}

void BasicTurret::Draw(sf::RenderTarget *target )
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

	target->draw( bulletVA, ts_bullet->texture );
	target->draw( sprite );
}

void BasicTurret::DrawMinimap( sf::RenderTarget *target )
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

bool BasicTurret::IHitPlayerWithBullets()
{
	Actor &player = owner->player;
	
	Bullet *currBullet = activeBullets;
	while( currBullet != NULL )
	{
		if( currBullet->hitBody.Intersects( player.hurtBody ) )
		{
			player.ApplyHit( bulletHitboxInfo );
			return true;
		}
		currBullet = currBullet->next;
	}

	
	return false;
}

bool BasicTurret::IHitPlayer()
{
	Actor &player = owner->player;
	if( hitBody.Intersects( player.hurtBody ) )
	{
		player.ApplyHit( hitboxInfo );
		return true;
	}
}

 pair<bool, bool> BasicTurret::PlayerHitMe()
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

 pair<bool, bool> BasicTurret::PlayerHitMyBullets()
 {
	 	return pair<bool, bool>(false,false);
 }

bool BasicTurret::PlayerSlowingMe()
{
	Actor &player = owner->player;

	Bullet *currBullet = activeBullets;
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
	}

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

void BasicTurret::UpdateSprite()
{
	sprite.setTextureRect( ts->GetSubRect( frame / animationFactor ) );

	int i = 0;
	Bullet *currBullet = activeBullets;
	while( currBullet != NULL )
	{	
		bulletVA[i*4].position = Vector2f( currBullet->position.x - 8, currBullet->position.y - 8 );
		bulletVA[i*4+1].position = Vector2f( currBullet->position.x + 8, currBullet->position.y - 8 );
		bulletVA[i*4+2].position = Vector2f( currBullet->position.x + 8, currBullet->position.y + 8 );
		bulletVA[i*4+3].position = Vector2f( currBullet->position.x - 8, currBullet->position.y + 8 );

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
}

void BasicTurret::DebugDraw(sf::RenderTarget *target)
{
	Bullet *currBullet = activeBullets;
	while( currBullet != NULL )
	{
		currBullet->hitBody.DebugDraw( target );
		currBullet->hurtBody.DebugDraw( target );

		currBullet = currBullet->next;
	}

	sf::CircleShape cs;
	cs.setFillColor( Color( 0, 255, 0, 100 ) );
	cs.setRadius( 15 );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	
	target->draw( cs );
}

void BasicTurret::UpdateHitboxes()
{
	hurtBody.globalPosition = position + gn * 32.0;
	hurtBody.globalAngle = 0;
	hitBody.globalPosition = position + gn * 32.0;
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
	
	Rect<double> r( bullet->position.x - 8, bullet->position.y - 8, 
		2 * 8, 2 * 8 );
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


	inactiveBullets->hurtBody.isCircle = true;
	inactiveBullets->hurtBody.globalAngle = 0;
	inactiveBullets->hurtBody.offset.x = 0;
	inactiveBullets->hurtBody.offset.y = 0;
	inactiveBullets->hurtBody.rw = 8;
	inactiveBullets->hurtBody.rh = 8;

	inactiveBullets->hitBody.type = CollisionBox::Hit;
	inactiveBullets->hitBody.isCircle = true;
	inactiveBullets->hitBody.globalAngle = 0;
	inactiveBullets->hitBody.offset.x = 0;
	inactiveBullets->hitBody.offset.y = 0;
	inactiveBullets->hitBody.rw = 8;
	inactiveBullets->hitBody.rh = 8;

	inactiveBullets->physBody.type = CollisionBox::Physics;
	inactiveBullets->physBody.isCircle = true;
	inactiveBullets->physBody.globalAngle = 0;
	inactiveBullets->physBody.offset.x = 0;
	inactiveBullets->physBody.offset.y = 0;
	inactiveBullets->physBody.rw = 8;
	inactiveBullets->physBody.rh = 8;
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