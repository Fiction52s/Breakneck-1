#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include <cstdlib>

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>
#define COLOR_BLUE Color( 0, 0x66, 0xcc )


BossCrawler::BossCrawler( GameSession *owner, Edge *g, double q )
	:Enemy( owner, EnemyType::CRAWLER, false, 1 ), ground( g ), edgeQuantity( q ), numBullets( 6 ), 
		bulletVA( sf::Quads, numBullets * 4 )
{
	invincibleFrames = 0;
	double width = 128;
	double height = 144;
	ts_test = owner->GetTileset( "bosscrawler_128x144.png", width, height );
	
	ts_bullet = owner->GetTileset( "basicbullet_32x32.png", 32, 32 );

	initHealth = 60 * 10;
	health = initHealth;

	numBullets = 6;
	bullets = new Bullet[numBullets];
	bulletRadius = 16;
	
	
	//cout << "creating the boss crawler" << endl;
	action = STAND;
	gravity = 1;
	facingRight = false;
	receivedHit = NULL;

	groundSpeed = 0;
	dead = false;
	ts_walk = owner->GetTileset( "crawlerwalk.png", 96, 64 );
	ts_roll = owner->GetTileset( "crawlerroll.png", 96, 64 );
	//sprite.setTexture( *ts_walk->texture );
	//sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
	//sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	V2d gPoint = g->GetPoint( edgeQuantity );

	sprite.setTexture( *ts_test->texture );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );



	sprite.setPosition( gPoint.x, gPoint.y );
	roll = false;


	double size = max( width * 5, height * 5 );
	spawnRect = sf::Rect<double>( gPoint.x - size / 2, gPoint.y - size / 2, size, size);
	//spawnRect = sf::Rect<double>( gPoint.x - 96 / 2, gPoint.y - 96/ 2, 96, 96 );

	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 64;
	hurtBody.rh = 64;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = 64;
	hitBody.rh = 64;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 100;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 0;

	bulletHitboxInfo = new HitboxInfo;
	bulletHitboxInfo->damage = 40;
	bulletHitboxInfo->drainX = 0;
	bulletHitboxInfo->drainY = 0;
	bulletHitboxInfo->hitlagFrames = 0;
	bulletHitboxInfo->hitstunFrames = 10;
	bulletHitboxInfo->knockback = 0;

	crawlAnimationFactor = 2;
	rollAnimationFactor = 2;
	physBody.isCircle = true;
	physBody.offset.x = 0;
	physBody.offset.y = 0;
	physBody.rw = 64;
	physBody.rh = 64;
	physBody.type = CollisionBox::BoxType::Physics;

	for( int i = 0; i < numBullets; ++i )
	{
		Bullet &b = bullets[i];
		b.hurtBody.isCircle = true;
		b.hurtBody.globalAngle = 0;
		b.hurtBody.offset.x = 0;
		b.hurtBody.offset.y = 0;
		b.hurtBody.rw = bulletRadius;
		b.hurtBody.rh = bulletRadius;

		b.hitBody.type = CollisionBox::Hit;
		b.hitBody.isCircle = true;
		b.hitBody.globalAngle = 0;
		b.hitBody.offset.x = 0;
		b.hitBody.offset.y = 0;
		b.hitBody.rw = bulletRadius;
		b.hitBody.rh = bulletRadius;

		b.physBody.type = CollisionBox::Physics;
		b.physBody.isCircle = true;
		b.physBody.globalAngle = 0;
		b.physBody.offset.x = 0;
		b.physBody.offset.y = 0;
		b.physBody.rw = bulletRadius;
		b.physBody.rh = bulletRadius;
	}

	startGround = ground;
	startQuant = edgeQuantity;
	frame = 0;
	position = gPoint + ground->Normal() * physBody.rh; //16.0;

	bulletGrav = .5;

	hitsBeforeHurt = 4;
	hitsCounter = 0;
}

void BossCrawler::ResetEnemy()
{
	hitsCounter = 0;
	owner->cam.bossCrawler = false;
	invincibleFrames = 0;
	health = initHealth;
	roll = false;
	ground = startGround;
	edgeQuantity = startQuant;
	V2d gPoint = ground->GetPoint( edgeQuantity );

	sprite.setTexture( *ts_test->texture );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );

	sprite.setPosition( gPoint.x, gPoint.y );

	V2d gn = ground->Normal();
	dead = false;

	//----update the sprite
	//double angle = 0;



	position = gPoint + gn * physBody.rh;//16.0;
	angle = atan2( gn.x, -gn.y );
		
	//sprite.setTexture( *ts_walk->texture );
	//sprite.setTextureRect( ts_walk->GetSubRect( frame / crawlAnimationFactor ) );
	V2d pp = ground->GetPoint( edgeQuantity );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	sprite.setRotation( angle / PI * 180 );
	sprite.setPosition( pp.x, pp.y );
	//----

	UpdateHitboxes();

	for( int i = 0; i < numBullets; ++i )
	{
		bullets[i].active = false;
	}

	action = STAND;
	frame = 0;

	//action = LUNGE
	//action = STAND;
	facingRight = false;
	groundSpeed = 0;
}

void BossCrawler::HandleEntrant( QuadTreeEntrant *qte )
{
	assert( queryMode != "" );

	Edge *e = (Edge*)qte;


	

	if( e->edgeType == Edge::OPEN_GATE )
	{
		return;
	}

	if( queryMode == "resolve" )
	{

		if( ground == e )
			return;

			//so you can run on gates without transfer issues hopefully
		if( ground != NULL && ground->edgeType == Edge::CLOSED_GATE )
		{
			Gate *g = (Gate*)ground->info;
			Edge *edgeA = g->edgeA;
			Edge *edgeB = g->edgeB;
			if( ground == g->edgeA )
			{
				if( e == edgeB->edge0 
					|| e == edgeB->edge1
					|| e == edgeB )
				{
				//	cout << "RETURN A" << endl;
					return;
				}

				
			}
			else if( ground == g->edgeB )
			{
				if( e == edgeA->edge0 
					|| e == edgeA->edge1
					|| e == edgeA )
				{
					//cout << "RETURN B" << endl;
					return;
				}
			}
		}
		if( ground != NULL )
		{
			if( ground->edge0->edgeType == Edge::CLOSED_GATE )
			{
				Gate *g = (Gate*)ground->edge0->info;
				Edge *e0 = ground->edge0;
				if( e0 == g->edgeA )
				{
					Edge *edgeB = g->edgeB;
					if( e == edgeB->edge0 
						|| e == edgeB->edge1
						|| e == edgeB )
					{
						return;
					}
				}
				else if( e0 == g->edgeB )
				{
					Edge *edgeA = g->edgeA;
					if( e == edgeA->edge0 
						|| e == edgeA->edge1
						|| e == edgeA )
					{
						return;
					}
				}
			}
			
			
			if( ground->edge1->edgeType == Edge::CLOSED_GATE )
			{
				Gate *g = (Gate*)ground->edge1->info;
				Edge *e1 = ground->edge1;
				if( e1 == g->edgeA )
				{
					Edge *edgeB = g->edgeB;
					if( e == edgeB->edge0 
						|| e == edgeB->edge1
						|| e == edgeB )
					{
						return;
					}
				}
				else if( e1 == g->edgeB )
				{
					Edge *edgeA = g->edgeA;
					if( e == edgeA->edge0 
						|| e == edgeA->edge1
						|| e == edgeA )
					{
						return;
					}
				}
			}
		}


		Contact *c = owner->coll.collideEdge( position + physBody.offset, physBody, e, tempVel, V2d( 0, 0 ) );

		if( c != NULL )
		{
		double len0 = length( c->position - e->v0 );
		double len1 = length( c->position - e->v1 );

		if( e->edge0->edgeType == Edge::CLOSED_GATE && len0 < 1 )
		{
			V2d pVec = normalize( position - e->v0 );
			double pAngle = atan2( -pVec.y, pVec.x );

			if( pAngle < 0 )
			{
				pAngle += 2 * PI;
			}

			Edge *e0 = e->edge0;
			Gate *g = (Gate*)e0->info;

			V2d startVec = normalize( e0->v0 - e->v0 );
			V2d endVec = normalize( e->v1 - e->v0 );

			double startAngle = atan2( -startVec.y, startVec.x );
			if( startAngle < 0 )
			{
				startAngle += 2 * PI;
			}
			double endAngle = atan2( -endVec.y, endVec.x );
			if( endAngle < 0 )
			{
				endAngle += 2 * PI;
			}

			double temp = startAngle;
			startAngle = endAngle;
			endAngle = temp;

			if( endAngle < startAngle )
			{
				if( pAngle >= endAngle || pAngle <= startAngle )
				{
					
				}
				else
				{
					return;
				}
			}
			else
			{
				if( pAngle >= startAngle && pAngle <= endAngle )
				{
				}
				else
				{
					return;
				}
			}
			

		}
		else if( e->edge1->edgeType == Edge::CLOSED_GATE && len1 < 1 )
		{
			V2d pVec = normalize( position - e->v1 );
			double pAngle = atan2( -pVec.y, pVec.x );

			if( pAngle < 0 )
			{
				pAngle += 2 * PI;
			}

			Edge *e1 = e->edge1;
			Gate *g = (Gate*)e1->info;

			V2d startVec = normalize( e->v0 - e->v1 );
			V2d endVec = normalize( e1->v1 - e->v1 );

			double startAngle = atan2( -startVec.y, startVec.x );
			if( startAngle < 0 )
			{
				startAngle += 2 * PI;
			}
			double endAngle = atan2( -endVec.y, endVec.x );
			if( endAngle < 0 )
			{
				endAngle += 2 * PI;
			}
			
			double temp = startAngle;
			startAngle = endAngle;
			endAngle = temp;

			//double temp = startAngle;
			//startAngle = endAngle;
			//endAngle = temp;

			if( endAngle < startAngle )
			{
				/*if( pAngle > startAngle && pAngle < endAngle )
				{
					return;
				}*/


				if( pAngle >= endAngle || pAngle <= startAngle )
				{
				}
				else
				{
					return;
				}
			}
			else
			{
				/*if( pAngle < startAngle || pAngle > endAngle )
				{
					cout << "crawler edge: " << e->Normal().x << ", " << e->Normal().y << ", return b. start: " << startAngle << ", end: " << endAngle << ", p: " << pAngle << endl;
					return;
				}*/
				
				if( pAngle >= startAngle && pAngle <= endAngle )
				{
				}
				else
				{	
					return;
				}
			}
		}


		

		
			if( !col || (minContact.collisionPriority < 0 ) 
				|| (c->collisionPriority <= minContact.collisionPriority && c->collisionPriority >= 0 ) ) //(c->collisionPriority >= -.00001 && ( c->collisionPriority <= minContact.collisionPriority || minContact.collisionPriority < -.00001 ) ) )
			{	

				/*if( ground != NULL && e == ground->edge1 && ( c->normal.x == 0 && c->normal.y == 0 ) )
				{
					return;
				}*/

				if( c->collisionPriority == minContact.collisionPriority )
				{
					if(( c->normal.x == 0 && c->normal.y == 0 ) )
					{
						minContact.collisionPriority = c->collisionPriority;
						minContact.edge = e;
						minContact.resolution = c->resolution;
						minContact.position = c->position;
						minContact.normal = c->normal;
						minContact.movingPlat = NULL;
						col = true;
					}
				}
				else
				{
					minContact.collisionPriority = c->collisionPriority;
					minContact.edge = e;
					minContact.resolution = c->resolution;
					minContact.position = c->position;
					minContact.normal = c->normal;
					minContact.movingPlat = NULL;
					col = true;
					
				}
			}
		}
	}
	else if( queryMode == "bullet" )
	{
		Edge *e = (Edge*)qte;

		Bullet &b = bullets[queryIndex];
		Contact *c = owner->coll.collideEdge( b.position + tempVel, b.physBody, e, tempVel, V2d( 0, 0 ) );
	

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
	}
	++possibleEdgeCount;
}

void BossCrawler::UpdateHitboxes()
{
	if( ground != NULL )
	{
		V2d gn = ground->Normal();
		//angle = 0;
		//double angle = 0;
		if( !approxEquals( abs(offset.x), physBody.rw ) )
		{
			//this should never happen
		}
		else
		{
			//angle = atan2( gn.x, -gn.y );
		}
		hitBody.globalAngle = angle;
		hurtBody.globalAngle = angle;
	}
	else
	{
		hitBody.globalAngle = 0;
		hurtBody.globalAngle = 0;
	}

	//hitBody.globalPosition = position + V2d( hitBody.offset.x * cos( hitBody.globalAngle ) + hitBody.offset.y * sin( hitBody.globalAngle ), hitBody.offset.x * -sin( hitBody.globalAngle ) + hitBody.offset.y * cos( hitBody.globalAngle ) );
	//hurtBody.globalPosition = position + V2d( hurtBody.offset.x * cos( hurtBody.globalAngle ) + hurtBody.offset.y * sin( hurtBody.globalAngle ), hurtBody.offset.x * -sin( hurtBody.globalAngle ) + hurtBody.offset.y * cos( hurtBody.globalAngle ) );
	hitBody.globalPosition = position;
	hurtBody.globalPosition = position;
	physBody.globalPosition = position;//+ V2d( -16, 0 );// + //physBody.offset + offset;
	
	//test knockback
	V2d knockbackDir( 1, -1 );
	knockbackDir = normalize( knockbackDir );
	hitboxInfo->knockback = 8;
	


	if( action == ROLL )
	{
		//hurtBody.rw = 64;
		//hurtBody.rh = 64;
		hitBody.rw = 128;
		hitBody.rh = 128;
	}
	else
	{
		hitBody.rw = 64;
		hitBody.rh = 64;
		//hurtBody.rw = 128;
		//hurtBody.rh = 128;
		
	}
}

void BossCrawler::UpdatePrePhysics()
{
	if( !owner->cam.bossCrawler )
	{
		owner->cam.bossCrawler = true;
		owner->cam.pos.x = position.x;
		owner->cam.pos.y = position.y - 50;
		owner->cam.zoomFactor = 1.5;
	}

	if( !dead && receivedHit != NULL )
	{	
		//gotta factor in getting hit by a clone
		health -= 20;

		hitsCounter++;
		if( hitsCounter == hitsBeforeHurt )
		{
			hitsCounter = 0;
			invincibleFrames = 30;
		}
		//invincibleFrames = 10;
		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			if( hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			dead = true;
			owner->quit = true;
		}

		receivedHit = NULL;
	}


	int standLength = 40;
	int shootLength = 30;
	int lungeLength = 20;
	int lungeLandLength = 20;
	int rollLength = 200;
	int stunLength = 60;
	bool doneRunning = false;
	int runFrames = 60;

	double runSpeed = 10;
	double rollSpeed = 17;//15;
	double lungeSpeed = 10;

	V2d gNormal;
	if( ground != NULL )
	{
		gNormal = ground->Normal();
	}

	switch( action )
	{
	case STAND:
		{
			if( frame == standLength - 1 )
			{
				int r = rand() % 4;
				int r1 = rand() % 2;
				switch( r )
				{
				case 0:
					action = RUN;
					
					if( r1 == 0 )
					{
						facingRight = true;
					}
					else
					{
						facingRight = false;
					}

					break;
				case 1:
					action = LUNGE;
					break;
				case 2:
					action = ROLL;
					break;
				case 3:
					action = SHOOT;
					break;
				}
				frame = 0;
				//cout << "stand -> run" << endl;
				
				//randomly choose to LUNGE or RUN or SHOOT
			}
		}
		break;
	case SHOOT:
		{
			if( frame == shootLength - 1 )
			{
				action = STAND;
				frame = 0;
				//randomly choose LUNGE or RUN
			}
		}
		break;
	case LUNGE:
		{
			if( frame == lungeLength - 1 )
			{
				assert( ground != NULL );

				V2d normal = ground->Normal();
				V2d along = normalize( ground->v1 - ground->v0 );
				V2d other( along.y, -along.x );


				ground = NULL;

				int lr = rand() % 3;
				if( lr == 0 )
				{
					normal = normalize( normal + along * .5 );
				}
				else if( lr == 1 )
				{
					normal = normalize( normal - along * .5 );
				}
				else
				{
				}
				velocity = normal * 20.0;

				

				action = LUNGEAIR;
				frame = 0;
			}
		}
		break;
	case LUNGEAIR:
		{
		}
		break;
	case LUNGELAND:
		{
			if( frame == lungeLandLength - 1 )
			{
				action = STAND;
				frame = 0;
			}
		}
		break;
	case RUN:
		{
			if( frame == runFrames - 1 || doneRunning )
			{
				int r = rand() % 3;
				switch( r )
				{
				case 0:
					action = ROLL;
					break;
				case 1:
					action = LUNGE;
					break;
				case 2:
					action = SHOOT;
					break;
				}
				frame = 0;
				//could shoot between LUNGE or SHOOT or ROLL
			}
		}
		break;
	case ROLL:
		{
			if( frame == rollLength - 1 )
			{
				action = STAND;
				frame = 0;
			}
		}
	case STUNNED:
		{
			if( frame == stunLength - 1 )
			{
				if( ground != NULL ) //ground
				{
					action = STAND;
					frame = 0;
				}
				else //air
				{
					//stall it out on its last frame until you land
					--frame;
				}
			}
		}
		break;
	}

	switch( action )
	{
	case STAND:
		{
			groundSpeed = 0;
			//cout << "stand" << endl;
		}
		break;
	case SHOOT:
		{
			if( frame == 20 )
			{
				FireBullets();
			}
			//cout << "shoot" << endl;
		}
		break;
	case LUNGE:
		{
			groundSpeed = 0;
			//cout << "lunge" << endl;
		}
		break;
	case LUNGEAIR:
		{
			//cout << "lunge air" << endl;
			velocity += V2d( 0, gravity / slowMultiple );
		}
		break;
	case LUNGELAND:
		{
			groundSpeed = 0;
			//cout << "lunge land" << endl;
		}
		break;
	case RUN:
		{
			//cout << "run" << endl;
			if( facingRight )
			{
				groundSpeed = runSpeed;
			}
			else
			{
				groundSpeed = -runSpeed;
			}
		}
		break;
	case ROLL:
		{
			//cout << "roll" << endl;
			if( facingRight )
			{
				groundSpeed = rollSpeed;
			}
			else
			{
				groundSpeed = -rollSpeed;
			}
		}
		break;
	case STUNNED:
		{
			//cout << "stunned" << endl;
			if( ground != NULL && gNormal.y > 0 )
			{
				ground = NULL;
				velocity = V2d( 0, 0 );
			}
			else if( ground == NULL )
			{
				velocity += V2d( 0, gravity / slowMultiple );
			}
			else
			{
				groundSpeed = 0; //maybe while stunned he should roll using gravity
			}
		}
		break;
	}

	for( int i = 0; i < numBullets; ++i )
	{
		Bullet &b = bullets[i];
		if( b.active )
		{
			b.velocity += V2d( 0, bulletGrav );
		}
	}
}

void BossCrawler::UpdatePhysics()
{
	double maxMovement = min( physBody.rw, physBody.rh );

	//bullet movement

	for( int i = 0; i < numBullets; ++i )
	{
		
		Bullet &b = bullets[i];

		if( b.active )
		{

			V2d movementVec = b.velocity / (double)b.slowMultiple / NUM_STEPS;

			if( ResolveBulletPhysics( i, movementVec ) )
			{
				b.active = false;
			}
		}
	}


	if( ground != NULL )
	{
		double movement = 0;
		movement = groundSpeed;
		movement /= slowMultiple * NUM_STEPS;

		while( movement != 0 )
		{
			//cout << "stuck here? " << endl;
		//ground is always some value

		double steal = 0;
		if( movement > 0 )
		{
			if( movement > maxMovement )
			{
				steal = movement - maxMovement;
				movement = maxMovement;
			}
		}
		else 
		{
			if( movement < -maxMovement )
			{
				steal = movement + maxMovement;
				movement = -maxMovement;
			}
		}

		double extra = 0;
		bool leaveGround = false;
		double q = edgeQuantity;

		V2d gNormal = ground->Normal();


		double m = movement;
		double groundLength = length( ground->v1 - ground->v0 ); 

		if( approxEquals( q, 0 ) )
			q = 0;
		else if( approxEquals( q, groundLength ) )
			q = groundLength;

		Edge *e0 = ground->edge0;
		Edge *e1 = ground->edge1;
		V2d e0n = e0->Normal();
		V2d e1n = e1->Normal();

		bool transferLeft = false;
		bool transferRight = false;

		if( facingRight )
		{
			if( q == groundLength )
			{
				if( e1n.y == -1 && gNormal.x < 0 && action == ROLL )
				{
					velocity = normalize( ground->v1 - ground->v0 ) * groundSpeed;
					ground = NULL;
					frame = 0;
					return;
				}
				else
				{
					q = 0;
					ground = e1;
					if( gNormal == e1n )
					{
				
					}
				}
			}
			else
			{
				if( movement > 0 )
				{	
					extra = (q + movement) - groundLength;
				}
				else 
				{
					extra = (q + movement);
				}
					
				if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
				{
					if( movement > 0 )
					{
						q = groundLength;
					}
					else
					{
						q = 0;
					}
					movement = extra;
					m -= extra;
						
				}
				else
				{
					movement = 0;
					q += m;
				}
				
				if( !approxEquals( m, 0 ) )//	if(m != 0 )
				{	
					bool down = true;
					bool hit = ResolvePhysics( normalize( ground->v1 - ground->v0 ) * m);
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
						V2d eNorm = minContact.edge->Normal();
						ground = minContact.edge;
						q = ground->GetQuantity( minContact.position + minContact.resolution );
						edgeQuantity = q;
						V2d gn = ground->Normal();
						break;
					}			
				}
				else
				{
					ground = e1;
					q = 0;
				}
			}
		}
		else //counter clockwise
		{
			double e0Length = length( e0->v1 - e0->v0 );
			if( q == 0 )
			{
				if( e0n.y == -1 && gNormal.x > 0 && action == ROLL )
				{
					velocity = normalize( ground->v1 - ground->v0 ) * groundSpeed;
					ground = NULL;
					frame = 0;
					return;
				}
				else
				{
					q = e0Length;
					ground = e0;
				}
			}
			else
			{
				if( movement > 0 )
				{	
					extra = (q + movement) - groundLength;
				}
				else 
				{
					extra = (q + movement);
				}
					
				if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
				{
					if( movement > 0 )
					{
						q = groundLength;
					}
					else
					{
						q = 0;
					}
					movement = extra;
					m -= extra;
						
				}
				else
				{
					movement = 0;
					q += m;
				}
				
				if( !approxEquals( m, 0 ) )//	if(m != 0 )
				{	
					bool down = true;
					bool hit = ResolvePhysics( normalize( ground->v1 - ground->v0 ) * m);
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
						V2d eNorm = minContact.edge->Normal();
						ground = minContact.edge;
						q = ground->GetQuantity( minContact.position + minContact.resolution );
						edgeQuantity = q;
						V2d gn = ground->Normal();
						break;
					}			
				}
				else
				{
					ground = e0;
					q = e0Length;
				}
			}
		}
		if( movement == extra )
			movement += steal;
		else
			movement = steal;

		edgeQuantity = q;
		}
	
	}
	else
	{
		V2d movementVec = velocity;
		movementVec /= slowMultiple * NUM_STEPS;

		bool hit = ResolvePhysics( movementVec );
		if( hit )
		{
			//V2d eNorm = minContact.edge->Normal();
			ground = minContact.edge;
			edgeQuantity  = ground->GetQuantity( minContact.position + minContact.resolution );
			if( action == ROLL )
			{
				facingRight = !facingRight;
				
				frame = ( 150 + 13 ) - (rand() % 30);
			}
			else
			{
				action = LUNGELAND;
				frame = 0;
			}
			
			//= q;
			//V2d gn = ground->Normal();
			//break;
		}			
	}

	PhysicsResponse();
}

bool BossCrawler::ResolvePhysics( V2d vel )
{
	possibleEdgeCount = 0;

	Rect<double> oldR( position.x + physBody.offset.x - physBody.rw, 
		position.y + physBody.offset.y - physBody.rh, 2 * physBody.rw, 2 * physBody.rh );
	
	position += vel;
	
	Rect<double> newR( position.x + physBody.offset.x - physBody.rw, 
		position.y + physBody.offset.y - physBody.rh, 2 * physBody.rw, 2 * physBody.rh );
	//minContact.collisionPriority = 1000000;
	
	double oldRight = oldR.left + oldR.width;
	double right = newR.left + newR.width;

	double oldBottom = oldR.top + oldR.height;
	double bottom = newR.top + newR.height;

	double maxRight = max( right, oldRight );
	double maxBottom = max( oldBottom, bottom );
	double minLeft = min( oldR.left, newR.left );
	double minTop = min( oldR.top, newR.top );
	//Rect<double> r( minLeft - 5 , minTop - 5, maxRight - minLeft + 5, maxBottom - minTop + 5 );
	Rect<double> r( minLeft , minTop, maxRight - minLeft, maxBottom - minTop );

	
	minContact.collisionPriority = 1000000;

	

	tempVel = vel;

	col = false;
	minContact.edge = NULL;

	queryMode = "resolve";
	owner->terrainTree->Query( this, r );
	//Query( this, owner->testTree, r );

	return col;
}

bool BossCrawler::ResolveBulletPhysics( int i, V2d vel )
{
	possibleEdgeCount = 0;

	Bullet &b = bullets[i];
	
	b.position += vel;
	
	Rect<double> r( b.position.x - bulletRadius, b.position.y - bulletRadius, 
		2 * bulletRadius, 2 * bulletRadius );
	minContact.collisionPriority = 1000000;

	col = false;

	tempVel = vel;
	minContact.edge = NULL;

	queryIndex = i;
	//queryBullet = bullet;
	queryMode = "bullet";
	owner->terrainTree->Query( this, r );

	return col;
}

void BossCrawler::UpdateBulletSprites()
{
	for( int i = 0; i < numBullets; ++i )
	{
		Bullet &b = bullets[i];
		if( b.active )
		{
			bulletVA[i*4+0].position = Vector2f( b.position.x - bulletRadius, b.position.y - bulletRadius );
			bulletVA[i*4+1].position = Vector2f( b.position.x + bulletRadius, b.position.y - bulletRadius );
			bulletVA[i*4+2].position = Vector2f( b.position.x + bulletRadius, b.position.y + bulletRadius );
			bulletVA[i*4+3].position = Vector2f( b.position.x - bulletRadius, b.position.y + bulletRadius );
			
			//cout << "b.frame: " << b.frame << endl;
			IntRect sub = ts_bullet->GetSubRect( b.frame );
			bulletVA[i*4+0].texCoords = Vector2f( sub.left, sub.top );
			bulletVA[i*4+1].texCoords = Vector2f( sub.left + sub.width, sub.top );
			bulletVA[i*4+2].texCoords = Vector2f( sub.left + sub.width, sub.top + sub.height );
			bulletVA[i*4+3].texCoords = Vector2f( sub.left, sub.top + sub.height );
		}
		else
		{
			bulletVA[i*4+0].position = Vector2f( 0, 0 );
			bulletVA[i*4+1].position = Vector2f( 0, 0 );
			bulletVA[i*4+2].position = Vector2f( 0, 0 );
			bulletVA[i*4+3].position = Vector2f( 0, 0 );
		}
	}
}

void BossCrawler::UpdateBulletHitboxes()
{
	for( int i = 0; i < numBullets; ++i )
	{
		Bullet &b = bullets[i];
		b.hurtBody.globalPosition = b.position;
		b.hurtBody.globalAngle = 0;
		b.hitBody.globalPosition = b.position;
		b.hitBody.globalAngle = 0;
	}
}

void BossCrawler::PhysicsResponse()
{
	UpdateBulletHitboxes();

	if( !dead && receivedHit == NULL )
	{
		if( ground != NULL )
		{
			V2d gn = ground->Normal();
			V2d gPoint = ground->GetPoint( edgeQuantity );
			position = gPoint + gn * physBody.rh;
			angle = atan2( gn.x, -gn.y );
		}
		else
		{
			angle = 0;
		}

		UpdateHitboxes();

		pair<bool,bool> result = PlayerHitMe();
		if( result.first )
		{
			//cout << "color blue" << endl;
			//triggers multiple times per frame? bad?
			//owner->player->test = true;
			//owner->player->currAttackHit = true;
			//owner->player->flashColor = COLOR_BLUE;
			//owner->player->flashFrames = 5;
			//owner->player->desperationMode = false;
			//owner->player->swordShader.setParameter( "energyColor", COLOR_BLUE );
			//owner->powerBar.Charge( 2 * 6 * 2 );
			//owner->powerBar.Charge( 6 );

			if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
			{
				owner->player->velocity.y = 4;//.5;
			}
		
			//owner->ActivateEffect( ts_testBlood, position, true, 0, 6, 3, facingRight );

		}

		
	}

	if( !dead )
	{
		//can hit back on the same frame because im a boss? maybe everyone should be able to hit back on the same frame
		if( IHitPlayer() )
		{
		}

		if( IHitPlayerWithBullets() )
		{
		}

		if( PlayerSlowingMe() )
		{
			if( slowMultiple == 1 )
			{
				slowCounter = 1;
				slowMultiple = 5;
			//	cout << "yes slow" << endl;
			}
		}
		else
		{
			slowCounter = 1;
			slowMultiple = 1;
		//	cout << "no slow" << endl;
		}
	}
}

void BossCrawler::UpdatePostPhysics()
{
	if( receivedHit != NULL )
		owner->Pause( 5 );

	UpdateBulletSprites();

	sprite.setTexture( *ts_test->texture );
	sprite.setTextureRect( ts_test->GetSubRect( 0 ) );
	sprite.setScale( 1.3, 1.3 );
	switch( action )
	{
	case STAND:
		{
			//sprite.setTexture( *ts_walk->texture );
			//sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
			V2d pp = ground->GetPoint( edgeQuantity );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		break;
	case SHOOT:
		{
			//sprite.setTexture( *ts_walk->texture );
			//sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
			V2d pp = ground->GetPoint( edgeQuantity );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		break;
	case LUNGE:
		{
			//sprite.setTexture( *ts_walk->texture );
			//sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
			V2d pp = ground->GetPoint( edgeQuantity );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		break;
	case LUNGEAIR:
		{
			//sprite.setTexture( *ts_walk->texture );
			//sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );

			
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
			//sprite.setRotation( 0 );
			sprite.setPosition( position.x, position.y );
		}
		break;
	case LUNGELAND:
		{
			//sprite.setTexture( *ts_walk->texture );
			//sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
			V2d pp = ground->GetPoint( edgeQuantity );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		break;
	case RUN:
		{
		//	cout << "putting sprite at the correct angle: " << angle << endl;
			//sprite.setTexture( *ts_walk->texture );
			//sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
			V2d pp = ground->GetPoint( edgeQuantity );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		break;
	case ROLL:
		{
			//sprite.setTexture( *ts_walk->texture );
			//sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
			if( ground != NULL )
			{
				V2d pp = ground->GetPoint( edgeQuantity );
				sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
				sprite.setRotation( angle / PI * 180 );
				sprite.setPosition( pp.x, pp.y );
			}
			else
			{
				sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);		
				sprite.setPosition( position.x, position.y );
			}
			
			
		}
		break;
	case STUNNED:
		{
		//	sprite.setTexture( *ts_walk->texture );
		//	sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
			V2d pp = ground->GetPoint( edgeQuantity );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		break;
	}

	if( !facingRight)
	{
		sf::IntRect r = sprite.getTextureRect();
		sprite.setTextureRect( sf::IntRect( r.left + r.width, r.top, -r.width, r.height ) );
	}

	if( slowCounter == slowMultiple )
	{
		++frame;
		if( invincibleFrames > 0 )
			--invincibleFrames;
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

	for( int i = 0; i < numBullets; ++i )
	{
		Bullet &b = bullets[i];
		if( b.active )
		{
			if( b.slowCounter == b.slowMultiple )
			{
				b.frame++;
				if( b.frame == 12 )
				{
					b.frame = 0;
				}
				b.slowCounter = 1;
			}
			else
			{
				b.slowCounter++;
			}
		}
	}



	//sprite.setPosition( position );
	//UpdateHitboxes();
}

bool BossCrawler::PlayerSlowingMe()
{
	Actor *player = owner->player;
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

void BossCrawler::Draw(sf::RenderTarget *target )
{
	if( !dead )
	{
		if( action == ROLL )
		{
			CircleShape cs;
			cs.setRadius( 128 );
			cs.setFillColor( COLOR_BLUE );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
		target->draw( sprite );

		sf::RenderStates states;
		states.texture = ts_bullet->texture;

		target->draw( bulletVA, states );
	}
}

void BossCrawler::DrawMinimap( sf::RenderTarget *target )
{
	CircleShape cs;
	cs.setRadius( 80 );
	cs.setFillColor( COLOR_BLUE );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	target->draw( cs );
}

bool BossCrawler::IHitPlayer()
{
	Actor *player = owner->player;
	
	if( hitBody.Intersects( player->hurtBody ) )
	{
		hitboxInfo->kbDir = normalize( player->position - position );
		//knockback stuff?
		if( player->position.x < position.x )
		{
			//hitboxInfo->kbDir = V2d( -1, -1 ); //-abs( hitboxInfo->kbDir.x );
		}
		else if( player->position.x > position.x )
		{
			//hitboxInfo->kbDir = V2d( 1, -1 );//abs( hitboxInfo->kbDir.x );
		}


		player->ApplyHit( hitboxInfo );
		return true;
	}
	
	return false;
}

bool BossCrawler::IHitPlayerWithBullets()
{
	Actor *player = owner->player;
	
	for( int i = 0; i < numBullets; ++i )
	{
		Bullet &b = bullets[i];
		if( b.active )
		{
			if( b.hitBody.Intersects( player->hurtBody ) )
			{
				player->ApplyHit( bulletHitboxInfo );
				return true;
			}
		}
	}
	return false;
}

 pair<bool, bool> BossCrawler::PlayerHitMe()
{
	if( invincibleFrames > 0 )
	{
		return pair<bool,bool>(false,false);
	}

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

void BossCrawler::UpdateSprite()
{
}

void BossCrawler::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{

		CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 10 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		//V2d g = ground->GetPoint( edgeQuantity );
		//cs.setPosition( g.x, g.y );
		cs.setPosition( position.x, position.y );

		//owner->window->draw( cs );
		//UpdateHitboxes();
		physBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
//	hurtBody.DebugDraw( target );
//	hitBody.DebugDraw( target );
}

void BossCrawler::SaveEnemyState()
{
}

void BossCrawler::LoadEnemyState()
{
}

void BossCrawler::FireBullets()
{
	double launchSpeed = 10;
	//double grav = 1;
	V2d norm = ground->Normal();
	for( int i = 0; i < numBullets; ++i )
	{
		
		V2d vel = norm * launchSpeed + V2d( 0, -3 ) * (double)i;
		if( owner->player->position.x < position.x - 200 && vel.x > 0 
			|| owner->player->position.x > position.x + 200 && vel.x < 0 )
		{
			vel.x = -vel.x;
		}
		bullets[i].active = true;
		bullets[i].position = position;
		bullets[i].frame = 0;
		bullets[i].slowCounter = 1;
		bullets[i].slowMultiple = 1;
		bullets[i].velocity = vel;
	}

	if( norm.y == -1 )
	{
		//bullets[0].velocity = V2d( 0, -launchSpeed * 2 );
		bullets[0].velocity = normalize( V2d( launchSpeed, -launchSpeed ) ) * launchSpeed * 1.5;
		bullets[1].velocity = normalize( V2d( -launchSpeed, -launchSpeed ) ) * launchSpeed * 1.5;

		bullets[2].velocity = normalize( V2d( launchSpeed * 2, -launchSpeed ) ) * launchSpeed;
		bullets[3].velocity = normalize( V2d( -launchSpeed * 2, -launchSpeed ) ) * launchSpeed;

		bullets[4].velocity = normalize( V2d( -launchSpeed, -launchSpeed ) ) * launchSpeed * 3.0;
		bullets[5].velocity = normalize( V2d( -launchSpeed, -launchSpeed ) ) * launchSpeed  * 3.0;
	}
	else if( norm.y == 1 )
	{
		//bullets[0].velocity = V2d( 0, -launchSpeed * 2 );
		bullets[0].velocity = normalize( V2d( launchSpeed, launchSpeed ) ) * launchSpeed * 1.5;
		bullets[1].velocity = normalize( V2d( -launchSpeed, launchSpeed ) ) * launchSpeed * 1.5;

		bullets[2].velocity = normalize( V2d( launchSpeed * 2, launchSpeed ) ) * launchSpeed;
		bullets[3].velocity = normalize( V2d( -launchSpeed * 2, launchSpeed ) ) * launchSpeed;

		bullets[4].velocity = normalize( V2d( -launchSpeed, launchSpeed ) ) * launchSpeed * 3.0;
		bullets[5].velocity = normalize( V2d( -launchSpeed, launchSpeed ) ) * launchSpeed  * 3.0;
	}
}

BossCrawler::Bullet::Bullet()
	:frame( 0 ), slowCounter( 1 ), slowMultiple( 1 ), active( false )
{

}