#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include <cstdlib>

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



PoisonFrog::PoisonFrog( GameSession *p_owner, bool p_hasMonitor, Edge *g, double q, int gFactor,
	sf::Vector2i &jStrength, int framesWait )
	:Enemy( p_owner, EnemyType::POISONFROG, p_hasMonitor, 2 ), 
	gravityFactor( gFactor ), jumpStrength( jStrength.x, jStrength.y ), 
	jumpFramesWait( framesWait )
{
	maxFallSpeed = 25;
	actionLength[STAND] = 10;
	actionLength[JUMPSQUAT] = 2;
	actionLength[JUMP] = 2;
	actionLength[LAND] = 2;
	actionLength[STEEPJUMP] = 2;
	actionLength[WALLCLING] = 30;

	animFactor[STAND] = 1;
	animFactor[JUMPSQUAT] = 4;
	animFactor[JUMP] = 1;
	animFactor[LAND] = 3;
	animFactor[STEEPJUMP] = 1;
	animFactor[WALLCLING] = 1;

	invincibleFrames = 0;
	double width = 80;
	double height = 80;
	ts_test = owner->GetTileset( "frog_80x80.png", width, height );

	//jumpStrength = 10;
	xSpeed = 8;

	mover = new GroundMover( p_owner, g, q, 30, true, this );
	mover->SetSpeed( 0 );

	initHealth = 60;
	health = initHealth;
	
	//cout << "creating the boss crawler" << endl;
	action = STAND;
	gravity = gravityFactor / 64.0;
	facingRight = false;
	receivedHit = NULL;

	
	dead = false;
	//sprite.setTexture( *ts_walk->texture );
	//sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
	//sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	V2d gPoint = g->GetPoint( q );

	sprite.setTexture( *ts_test->texture );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );



	sprite.setPosition( gPoint.x, gPoint.y );


	double size = max( width * 5, height * 5 );
	spawnRect = sf::Rect<double>( gPoint.x - size / 2, gPoint.y - size / 2, size, size);
	//spawnRect = sf::Rect<double>( gPoint.x - 96 / 2, gPoint.y - 96/ 2, 96, 96 );

	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 30;
	hurtBody.rh = 30;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = 30;
	hitBody.rh = 30;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 0;

	deathPartingSpeed = .4;

	/*physBody.isCircle = true;
	physBody.offset.x = 0;
	physBody.offset.y = 0;
	physBody.rw = 45;
	physBody.rh = 45;
	physBody.type = CollisionBox::BoxType::Physics;*/

	startGround = g;
	startQuant = q;
	frame = 0;
	position = mover->physBody.globalPosition;//gPoint + ground->Normal() * physBody.rh; //16.0;
}

void PoisonFrog::ResetEnemy()
{
	invincibleFrames = 0;
	health = initHealth;
	//ground = startGround;
	//edgeQuantity = startQuant;
	

	mover->ground = startGround;
	mover->edgeQuantity = startQuant;
	mover->roll = false;
	mover->UpdateGroundPos();
	mover->SetSpeed( 0 );

	position = mover->physBody.globalPosition;


	V2d gPoint = mover->ground->GetPoint( mover->edgeQuantity );

	sprite.setTexture( *ts_test->texture );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );

	sprite.setPosition( gPoint.x, gPoint.y );

	V2d gn = mover->ground->Normal();
	dead = false;

	//----update the sprite
	//double angle = 0;



	//position = gPoint + gn * physBody.rh;//16.0;
	angle = atan2( gn.x, -gn.y );
		
	//sprite.setTexture( *ts_walk->texture );
	//sprite.setTextureRect( ts_walk->GetSubRect( frame / crawlAnimationFactor ) );
	//V2d pp = ground->GetPoint( edgeQuantity );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	sprite.setRotation( angle / PI * 180 );
	sprite.setPosition( gPoint.x, gPoint.y );
	//----

	UpdateHitboxes();

	action = STAND;
	frame = 0;

	facingRight = false;
	//groundSpeed = 0;
}

void PoisonFrog::HandleEntrant( QuadTreeEntrant *qte )
{
	assert( queryMode != "" );

	//Edge *e = (Edge*)qte;


	//

	//if( e->edgeType == Edge::OPEN_GATE )
	//{
	//	return;
	//}

	//if( queryMode == "resolve" )
	//{

	//	if( ground == e )
	//		return;

	//		//so you can run on gates without transfer issues hopefully
	//	if( ground != NULL && ground->edgeType == Edge::CLOSED_GATE )
	//	{
	//		Gate *g = (Gate*)ground->info;
	//		Edge *edgeA = g->edgeA;
	//		Edge *edgeB = g->edgeB;
	//		if( ground == g->edgeA )
	//		{
	//			if( e == edgeB->edge0 
	//				|| e == edgeB->edge1
	//				|| e == edgeB )
	//			{
	//			//	cout << "RETURN A" << endl;
	//				return;
	//			}

	//			
	//		}
	//		else if( ground == g->edgeB )
	//		{
	//			if( e == edgeA->edge0 
	//				|| e == edgeA->edge1
	//				|| e == edgeA )
	//			{
	//				//cout << "RETURN B" << endl;
	//				return;
	//			}
	//		}
	//	}
	//	if( ground != NULL )
	//	{
	//		if( ground->edge0->edgeType == Edge::CLOSED_GATE )
	//		{
	//			Gate *g = (Gate*)ground->edge0->info;
	//			Edge *e0 = ground->edge0;
	//			if( e0 == g->edgeA )
	//			{
	//				Edge *edgeB = g->edgeB;
	//				if( e == edgeB->edge0 
	//					|| e == edgeB->edge1
	//					|| e == edgeB )
	//				{
	//					return;
	//				}
	//			}
	//			else if( e0 == g->edgeB )
	//			{
	//				Edge *edgeA = g->edgeA;
	//				if( e == edgeA->edge0 
	//					|| e == edgeA->edge1
	//					|| e == edgeA )
	//				{
	//					return;
	//				}
	//			}
	//		}
	//		
	//		
	//		if( ground->edge1->edgeType == Edge::CLOSED_GATE )
	//		{
	//			Gate *g = (Gate*)ground->edge1->info;
	//			Edge *e1 = ground->edge1;
	//			if( e1 == g->edgeA )
	//			{
	//				Edge *edgeB = g->edgeB;
	//				if( e == edgeB->edge0 
	//					|| e == edgeB->edge1
	//					|| e == edgeB )
	//				{
	//					return;
	//				}
	//			}
	//			else if( e1 == g->edgeB )
	//			{
	//				Edge *edgeA = g->edgeA;
	//				if( e == edgeA->edge0 
	//					|| e == edgeA->edge1
	//					|| e == edgeA )
	//				{
	//					return;
	//				}
	//			}
	//		}
	//	}


	//	Contact *c = owner->coll.collideEdge( position + physBody.offset, physBody, e, tempVel, V2d( 0, 0 ) );

	//	if( c != NULL )
	//	{
	//	double len0 = length( c->position - e->v0 );
	//	double len1 = length( c->position - e->v1 );

	//	if( e->edge0->edgeType == Edge::CLOSED_GATE && len0 < 1 )
	//	{
	//		V2d pVec = normalize( position - e->v0 );
	//		double pAngle = atan2( -pVec.y, pVec.x );

	//		if( pAngle < 0 )
	//		{
	//			pAngle += 2 * PI;
	//		}

	//		Edge *e0 = e->edge0;
	//		Gate *g = (Gate*)e0->info;

	//		V2d startVec = normalize( e0->v0 - e->v0 );
	//		V2d endVec = normalize( e->v1 - e->v0 );

	//		double startAngle = atan2( -startVec.y, startVec.x );
	//		if( startAngle < 0 )
	//		{
	//			startAngle += 2 * PI;
	//		}
	//		double endAngle = atan2( -endVec.y, endVec.x );
	//		if( endAngle < 0 )
	//		{
	//			endAngle += 2 * PI;
	//		}

	//		double temp = startAngle;
	//		startAngle = endAngle;
	//		endAngle = temp;

	//		if( endAngle < startAngle )
	//		{
	//			if( pAngle >= endAngle || pAngle <= startAngle )
	//			{
	//				
	//			}
	//			else
	//			{
	//				return;
	//			}
	//		}
	//		else
	//		{
	//			if( pAngle >= startAngle && pAngle <= endAngle )
	//			{
	//			}
	//			else
	//			{
	//				return;
	//			}
	//		}
	//		

	//	}
	//	else if( e->edge1->edgeType == Edge::CLOSED_GATE && len1 < 1 )
	//	{
	//		V2d pVec = normalize( position - e->v1 );
	//		double pAngle = atan2( -pVec.y, pVec.x );

	//		if( pAngle < 0 )
	//		{
	//			pAngle += 2 * PI;
	//		}

	//		Edge *e1 = e->edge1;
	//		Gate *g = (Gate*)e1->info;

	//		V2d startVec = normalize( e->v0 - e->v1 );
	//		V2d endVec = normalize( e1->v1 - e->v1 );

	//		double startAngle = atan2( -startVec.y, startVec.x );
	//		if( startAngle < 0 )
	//		{
	//			startAngle += 2 * PI;
	//		}
	//		double endAngle = atan2( -endVec.y, endVec.x );
	//		if( endAngle < 0 )
	//		{
	//			endAngle += 2 * PI;
	//		}
	//		
	//		double temp = startAngle;
	//		startAngle = endAngle;
	//		endAngle = temp;

	//		//double temp = startAngle;
	//		//startAngle = endAngle;
	//		//endAngle = temp;

	//		if( endAngle < startAngle )
	//		{
	//			/*if( pAngle > startAngle && pAngle < endAngle )
	//			{
	//				return;
	//			}*/


	//			if( pAngle >= endAngle || pAngle <= startAngle )
	//			{
	//			}
	//			else
	//			{
	//				return;
	//			}
	//		}
	//		else
	//		{
	//			/*if( pAngle < startAngle || pAngle > endAngle )
	//			{
	//				cout << "crawler edge: " << e->Normal().x << ", " << e->Normal().y << ", return b. start: " << startAngle << ", end: " << endAngle << ", p: " << pAngle << endl;
	//				return;
	//			}*/
	//			
	//			if( pAngle >= startAngle && pAngle <= endAngle )
	//			{
	//			}
	//			else
	//			{	
	//				return;
	//			}
	//		}
	//	}


	//	

	//	
	//		if( !col || (minContact.collisionPriority < 0 ) 
	//			|| (c->collisionPriority <= minContact.collisionPriority && c->collisionPriority >= 0 ) ) //(c->collisionPriority >= -.00001 && ( c->collisionPriority <= minContact.collisionPriority || minContact.collisionPriority < -.00001 ) ) )
	//		{	

	//			/*if( ground != NULL && e == ground->edge1 && ( c->normal.x == 0 && c->normal.y == 0 ) )
	//			{
	//				return;
	//			}*/

	//			if( c->collisionPriority == minContact.collisionPriority )
	//			{
	//				if(( c->normal.x == 0 && c->normal.y == 0 ) )
	//				{
	//					minContact.collisionPriority = c->collisionPriority;
	//					minContact.edge = e;
	//					minContact.resolution = c->resolution;
	//					minContact.position = c->position;
	//					minContact.normal = c->normal;
	//					minContact.movingPlat = NULL;
	//					col = true;
	//				}
	//			}
	//			else
	//			{
	//				minContact.collisionPriority = c->collisionPriority;
	//				minContact.edge = e;
	//				minContact.resolution = c->resolution;
	//				minContact.position = c->position;
	//				minContact.normal = c->normal;
	//				minContact.movingPlat = NULL;
	//				col = true;
	//				
	//			}
	//		}
	//	}
	//}
	//++possibleEdgeCount;
}

void PoisonFrog::UpdateHitboxes()
{
	Edge *ground = mover->ground;
	if( ground != NULL )
	{
		V2d gn = ground->Normal();
		
		V2d knockbackDir( 1, -1 );
		knockbackDir = normalize( knockbackDir );
		if( mover->groundSpeed > 0 )
		{
			hitboxInfo->kbDir = knockbackDir;
			hitboxInfo->knockback = 15;
		}
		else
		{
			hitboxInfo->kbDir = V2d( -knockbackDir.x, knockbackDir.y );
			hitboxInfo->knockback = 15;
		}
		//hitBody.globalAngle = angle;
		//hurtBody.globalAngle = angle;
	}
	else
	{
		//hitBody.globalAngle = 0;
		//hurtBody.globalAngle = 0;
	}

	//hitBody.globalPosition = position + V2d( hitBody.offset.x * cos( hitBody.globalAngle ) + hitBody.offset.y * sin( hitBody.globalAngle ), hitBody.offset.x * -sin( hitBody.globalAngle ) + hitBody.offset.y * cos( hitBody.globalAngle ) );
	//hurtBody.globalPosition = position + V2d( hurtBody.offset.x * cos( hurtBody.globalAngle ) + hurtBody.offset.y * sin( hurtBody.globalAngle ), hurtBody.offset.x * -sin( hurtBody.globalAngle ) + hurtBody.offset.y * cos( hurtBody.globalAngle ) );
	hitBody.globalPosition = position;
	hurtBody.globalPosition = position;
	//physBody.globalPosition = position;//+ V2d( -16, 0 );// + //physBody.offset + offset;
	
	//test knockback
	//V2d knockbackDir( 1, -1 );
	//knockbackDir = normalize( knockbackDir );
	//hitboxInfo->knockback = 8;
}

void PoisonFrog::ActionEnded()
{
	if( frame == actionLength[action] * animFactor[action] )
	{
		switch( action )
		{
		case STAND:
			{
				action = JUMPSQUAT;
				frame = 0;
			}
			break;
		case JUMPSQUAT:
			{
				action = JUMP;
				frame = 0;
			}
			break;
		case JUMP:
			{
				frame = 1;
			}
			break;
		case LAND:
			{
				action = STAND;
				frame = 0;
			}
			break;
		case WALLCLING:
			{
				action = JUMP;
				frame = 1;

				if( facingRight )
				{
					mover->Jump( V2d( jumpStrength.x, -jumpStrength.y ) );
				}
				else
				{
					mover->Jump( V2d( -jumpStrength.x, -jumpStrength.y ) );
				}
				

				//frame = 0;
			}
			break;
		}
	}
}

void PoisonFrog::UpdatePrePhysics()
{
	ActionEnded();

	Actor *player = owner->GetPlayer( 0 );


	V2d jumpVel;
	V2d gAlong;
	V2d gn;
	if( mover->ground != NULL )
	{
		gAlong = normalize( mover->ground->v1 - mover->ground->v0 );
		gn = mover->ground->Normal();
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
			//AttemptSpawnMonitor();
			dead = true;
			owner->GetPlayer( 0 )->ConfirmEnemyKill( this );
		}
		else
		{
			owner->GetPlayer( 0 )->ConfirmEnemyNoKill( this );
		}

		receivedHit = NULL;
	}

	switch( action )
	{
	case STAND:
		{
			//cout << "frame: " << frame << endl;
			if( player->position.x < position.x )
			{
				facingRight = false;
			}
			else if( player->position.x > position.x )
			{
				facingRight = true;
			}
		}
		break;
	case JUMPSQUAT:
		{
		}
		break;
	case JUMP:
		{
			if( frame == 0 )
			{
				//jumpVel = 
				//cout << "jumping" << endl;
				if( facingRight )
				{
					//if( gn.x < 0 )
					if( cross( normalize( V2d( jumpStrength.x, -jumpStrength.y )), gAlong ) < 0 )
					{
						gAlong = (gAlong + V2d( 0, -1 )) / 2.0;
						mover->Jump( gAlong * jumpStrength.y );
					}
					else
					{
						mover->Jump( V2d( jumpStrength.x, -jumpStrength.y ) );
					}
					
				}
				else
				{
					if( cross( normalize( V2d( -jumpStrength.x, -jumpStrength.y )), gAlong ) < 0 )
					//if( gn.x > 0 )
					{
						gAlong = (-gAlong + V2d( 0, -1 )) / 2.0;
						mover->Jump( gAlong * jumpStrength.y );
					}
					else
					{
						mover->Jump( V2d( -jumpStrength.x, -jumpStrength.y ) );
					}
				}
				
				/*if( owner->IsSteepGround( ground->Normal() ) >= 0 )
				{
					steepJump = true;
				}
				else
				{
					steepJump = false;
				}*/
				//cout << "frog jumping " << (int)steepJump << endl;
				//ground = NULL;

				
				/*if( facingRight )
				{
					velocity = V2d( jumpStrength.x * (int)(!steepJump), -jumpStrength.y );
				}
				else
				{
					velocity = V2d( -jumpStrength.x * (int)(!steepJump), -jumpStrength.y );
				}*/
				
			}
			else
			{
				//if( steepJump && velocity.y >= 0 )
				//{
				//	if( facingRight )
				//	{
				//		velocity.x += .3; //some number
				//		if( velocity.x > jumpStrength.x )
				//			velocity.x = jumpStrength.x;
				//	}
				//	else
				//	{
				//		velocity.x -= .3;
				//		if( velocity.x < -jumpStrength.x )
				//			velocity.x = -jumpStrength.x;
				//	}
				//}
				//else if( !steepJump )
				//{
				//	if( facingRight )
				//	{
				//		velocity.x = jumpStrength.x;
				//	}
				//	else
				//	{
				//		velocity.x = -jumpStrength.x;
				//	}
				//}


				
			}
		}
		break;
	case STEEPJUMP:
		{
			if( player->position.x < position.x )
			{
				facingRight = false;
			}
			else if( player->position.x > position.x )
			{
				facingRight = true;
			}

			if( facingRight )
			{
				mover->SetSpeed( 5 );
			}
			else
			{
				mover->SetSpeed( -5 );
			}
		}
		break;
	case LAND:
		{

		}
		break;
	case WALLCLING:
		{
		}
		break;
	}

	//if( ground == NULL )
	//{
	//	velocity += V2d( 0, gravity );
	//}

	//cout << "velocity: " << velocity.x << ", " << velocity.y << endl;
}

void PoisonFrog::UpdatePhysics()
{
	specterProtected = false;
	if( dead )
		return;

	if( mover->ground != NULL )
	{
	}
	else
	{
		double grav = gravity;
		if( action == WALLCLING )
		{
			grav = 0;//.1 * grav;
		}
		mover->velocity.y += grav / (NUM_STEPS * slowMultiple);

		if( mover->velocity.y >= maxFallSpeed )
		{
			mover->velocity.y = maxFallSpeed;
		}
	}


	mover->Move( slowMultiple );

	position = mover->physBody.globalPosition;
	//cout << "setting position to: " << position.x << ", " << position.y << endl;
	//double maxMovement = min( physBody.rw, physBody.rh );

	//if( ground != NULL )
	//{
	//	//never moves on the ground
	//}
	//else
	//{
	//	V2d movementVec = velocity;
	//	movementVec /= slowMultiple * NUM_STEPS;

	//	bool hit = ResolvePhysics( movementVec );
	//	if( hit )//&&  )
	//	{
	//		if( minContact.edge->Normal().y < 0 )
	//		{
	//			cout << "landing" << endl;
	//			ground = minContact.edge;
	//			edgeQuantity = ground->GetQuantity( minContact.position + minContact.resolution );
	//			position += minContact.resolution;
	//			action = LAND;
	//			frame = 0;
	//		}
	//		else
	//		{
	//			cout << "reso: " << minContact.resolution.x << ", " << minContact.resolution.y << endl;
	//			position += minContact.resolution;
	//			//velocity = V2d( 0, 0 );
	//		}
	//		
	//	}
	//	
	//	
	//}

	PhysicsResponse();
}

bool PoisonFrog::ResolvePhysics( V2d vel )
{
	possibleEdgeCount = 0;

	double rw = mover->physBody.rw;
	double rh = mover->physBody.rh;

	Rect<double> oldR( position.x - rw, 
		position.y - rh, 2 * rw, 2 * rh );
	
	position += vel;
	
	Rect<double> newR( position.x - rw, 
		position.y - rh, 2 * rw, 2 * rh );
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

	return col;
}

void PoisonFrog::PhysicsResponse()
{
	//doesn't get called if dead.
	assert( !dead );


	bool roll = mover->roll;
	//double angle = 0;
	Edge *ground = mover->ground;
	double edgeQuantity = mover->edgeQuantity;
	
	
	if( ground != NULL )
	{
		if( roll )
		{
			V2d gPoint = ground->GetPoint( edgeQuantity );
			V2d gn = normalize( mover->physBody.globalPosition - gPoint );
			angle = atan2( gn.x, -gn.y );
			
		}
		else
		{
			V2d gn = ground->Normal();
			angle = atan2( gn.x, -gn.y );
		}
		//ground->Normal();
		//position = gPoint + gn * mover->physBody.rh;
		
	}
	else
	{
		angle = 0;
	}

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
		slowCounter = 1;
		slowMultiple = 1;
	}
		
	if( receivedHit == NULL )
	{
		pair<bool,bool> result = PlayerHitMe();
		if( result.first )
		{
			//cout << "color blue" << endl;
			//triggers multiple times per frame? bad?
			owner->GetPlayer( 0 )->ConfirmHit( 2, 5, .8, 6 );
			//owner->powerBar.Charge( 2 * 6 * 2 );
			//owner->powerBar.Charge( 6 );

			if( owner->GetPlayer( 0 )->ground == NULL && owner->GetPlayer( 0 )->velocity.y > 0 )
			{
				owner->GetPlayer( 0 )->velocity.y = 4;//.5;
			}
		
			//owner->ActivateEffect( ts_testBlood, position, true, 0, 6, 3, facingRight );

		}
	}

	UpdateHitboxes();

	
		//can hit back on the same frame because im a boss? maybe everyone should be able to hit back on the same frame
	if( IHitPlayer() )
	{
	}

	Transform t;
	t.rotate( angle / PI * 180 );
	Vector2f newPoint = t.transformPoint( Vector2f( 1, -1 ) );
	deathVector = V2d( newPoint.x, newPoint.y );

}

void PoisonFrog::UpdatePostPhysics()
{
	//cout << "position: " << position.x << ", " << position.y << endl;

	if( receivedHit != NULL )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->GetPlayer( 0 )->position + position ) / 2.0, true, 0, 10, 2, true );
		owner->Pause( 5 );
	}

	if( deathFrame == 30 )
	{
		owner->RemoveEnemy( this );
		return;
	}

	if( deathFrame == 0 && dead )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, position, true, 0, 15, 2, true );
	}

	UpdateSprite();

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
}

bool PoisonFrog::PlayerSlowingMe()
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

void PoisonFrog::Draw(sf::RenderTarget *target )
{
	if( !dead )
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
	else
	{
		target->draw( botDeathSprite );

	/*	if( deathFrame / 3 < 6 )
		{
			
			bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
			bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
			bloodSprite.setPosition( position.x, position.y );
			bloodSprite.setScale( 2, 2 );
			target->draw( bloodSprite );
		}*/
		
		target->draw( topDeathSprite );
	}


	/*if( !dead )
	{
		target->draw( sprite );
	}*/
}

void PoisonFrog::DrawMinimap( sf::RenderTarget *target )
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
	/*CircleShape cs;
	cs.setRadius( 80 );
	cs.setFillColor( COLOR_BLUE );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	target->draw( cs );*/
}

bool PoisonFrog::IHitPlayer( int index )
{
	Actor *player = owner->GetPlayer( 0 );
	
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

 pair<bool, bool> PoisonFrog::PlayerHitMe( int index )
{
	if( invincibleFrames > 0 )
	{
		return pair<bool,bool>(false,false);
	}

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

void PoisonFrog::UpdateSprite()
{
	Edge *ground = mover->ground;
	double edgeQuantity = mover->edgeQuantity;
	V2d pp;

	//cout << "edgeQuantity: " << edgeQuantity << endl;
	
	if( ground != NULL )
	{
	//	cout << "grounded" << endl;
		pp = ground->GetPoint( edgeQuantity );
	}
	else
	{
	//	cout << "not grounded" << endl;
	}

	if( dead )
	{
		//cout << "deathVector: " << deathVector.x << ", " << deathVector.y << endl;
		/*botDeathSprite.setTexture( *ts->texture );
		botDeathSprite.setTextureRect( ts->GetSubRect( 31 ) );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2);
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );
		botDeathSprite.setRotation( sprite.getRotation() );

		topDeathSprite.setTexture( *ts->texture );
		topDeathSprite.setTextureRect( ts->GetSubRect( 30 ) );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );
		topDeathSprite.setRotation( sprite.getRotation() );*/
	}
	else
	{
		
		if( hasMonitor && !suppressMonitor )
		{
			keySprite->setTextureRect( ts_key->GetSubRect( owner->keyFrame / 2 ) );
			keySprite->setOrigin( keySprite->getLocalBounds().width / 2, 
				keySprite->getLocalBounds().height / 2 );
			keySprite->setPosition( position.x, position.y );
		}

		sprite.setTexture( *ts_test->texture );
		
		switch( action )
		{
		case STAND:
			{
				sprite.setTextureRect( ts_test->GetSubRect( 0 ) );
				sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
				sprite.setRotation( angle / PI * 180 );
				sprite.setPosition( pp.x, pp.y );
			}
			break;
		case JUMPSQUAT:
			{
				sprite.setTextureRect( ts_test->GetSubRect( ( frame / animFactor[JUMPSQUAT] ) + 1 ) );
				sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
				sprite.setRotation( angle / PI * 180 );
				sprite.setPosition( pp.x, pp.y );
			}
			break;
		case JUMP:
			{
				int window = 6;
				if( mover->velocity.y < -window )
				{
					sprite.setTextureRect( ts_test->GetSubRect( 3 ) );
				}
				else if( mover->velocity.y > window )
				{
					sprite.setTextureRect( ts_test->GetSubRect( 5 ) );
				}
				else
				{
					sprite.setTextureRect( ts_test->GetSubRect( 4 ) );
				}
				//sprite.setTextureRect( ts_test->GetSubRect( frame ) );
				
				sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
				sprite.setPosition( position.x, position.y );
				sprite.setRotation( 0 );
			}
			break;
		case LAND:
			{
				sprite.setTextureRect( ts_test->GetSubRect( (frame / animFactor[LAND]) + 6 ) );
				sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
				sprite.setRotation( angle / PI * 180 );
				sprite.setPosition( pp.x, pp.y );
			}
			break;
		case WALLCLING:
			{
				sprite.setTextureRect( ts_test->GetSubRect( 8 ) );
				sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
				sprite.setRotation( 0 );//angle / PI * 180 );
				sprite.setPosition( position.x, position.y );
			}
			break;
		}

		if( !facingRight)
		{
			sf::IntRect r = sprite.getTextureRect();
			sprite.setTextureRect( sf::IntRect( r.left + r.width, r.top, -r.width, r.height ) );
		}
		/*if( attackFrame >= 0 )
		{
			IntRect r = ts->GetSubRect( 28 + attackFrame / attackMult );
			if( !facingRight )
			{
				r = sf::IntRect( r.left + r.width, r.top, -r.width, r.height );
			}
			sprite.setTextureRect( r );
		}*/
	}
}

void PoisonFrog::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{

		CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 10 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( position.x, position.y );

		mover->physBody.DebugDraw( target );
//		physBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void PoisonFrog::SaveEnemyState()
{
}

void PoisonFrog::LoadEnemyState()
{
}

void PoisonFrog::HitTerrain( double &q )
{

}

bool PoisonFrog::StartRoll()
{
	return false;
}

void PoisonFrog::FinishedRoll()
{
	if( owner->IsSteepGround( mover->ground->Normal() ) == -1 )
	{
		action = JUMPSQUAT;
		mover->SetSpeed( 0 );
		frame = 0;
	}
}

void PoisonFrog::HitOther()
{
}

void PoisonFrog::ReachCliff()
{
	//if( owner->IsSteepGround( mover->ground->Normal() ) == -1 )
	{
		action = JUMPSQUAT;
		mover->SetSpeed( 0 );
		frame = 0;
	}
}

void PoisonFrog::HitOtherAerial( Edge *e )
{
	V2d norm = e->Normal();
	if( owner->IsWall( norm ) > 0 )
	{
		//if( action != WALLCLING )
		//{
			action = WALLCLING;
			frame = 0;
			//wallTouchCounter = 1;
		
			//if( norm.x > 0 ) //left wall, facing right
			//{
			//	mover->velocity = V2d( -.1, 0 );
			//}
			//else //right wall, facing left
			//{
			//	mover->velocity = V2d( .1, 0 );
			//}
			mover->velocity = V2d( 0, 0 );

		
			facingRight = !facingRight;
		//}
		//else
	//	{
		//	wallTouchCounter++;
	//	}
	}
}


	//}
	//if( 
void PoisonFrog::Land()
{
	//cout << "LANDING" << endl;
	action = LAND;
	frame = 0;
}