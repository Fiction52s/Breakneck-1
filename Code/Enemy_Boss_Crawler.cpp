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
	:Enemy( owner, EnemyType::CRAWLER ), ground( g ), edgeQuantity( q )
{
	//cout << "creating the boss crawler" << endl;
	action = STAND;
	gravity = 1;
	facingRight = false;
	receivedHit = NULL;

	groundSpeed = 0;
	dead = false;
	ts_walk = owner->GetTileset( "crawlerwalk.png", 96, 64 );
	ts_roll = owner->GetTileset( "crawlerroll.png", 96, 64 );
	sprite.setTexture( *ts_walk->texture );
	sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	V2d gPoint = g->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	roll = false;

	spawnRect = sf::Rect<double>( gPoint.x - 96 / 2, gPoint.y - 96/ 2, 96, 96 );

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
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 0;

	crawlAnimationFactor = 2;
	rollAnimationFactor = 2;
	physBody.isCircle = true;
	physBody.offset.x = 0;
	physBody.offset.y = 0;
	physBody.rw = 16;
	physBody.rh = 16;
	physBody.type = CollisionBox::BoxType::Physics;

	startGround = ground;
	startQuant = edgeQuantity;
	frame = 0;
	position = gPoint + ground->Normal() * 16.0;
}

void BossCrawler::ResetEnemy()
{
	roll = false;
	ground = startGround;
	edgeQuantity = startQuant;
	V2d gPoint = ground->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );

	V2d gn = ground->Normal();
	if( gn.x > 0 )
		offset.x = physBody.rw;
	else if( gn.x < 0 )
		offset.x = -physBody.rw;
	if( gn.y > 0 )
		offset.y = physBody.rh;
	else if( gn.y < 0 )
		offset.y = -physBody.rh;

	position = gPoint + offset;

	dead = false;

	//----update the sprite
	//double angle = 0;
	position = gPoint + gn * 16.0;
	angle = atan2( gn.x, -gn.y );
		
	sprite.setTexture( *ts_walk->texture );
	sprite.setTextureRect( ts_walk->GetSubRect( frame / crawlAnimationFactor ) );
	V2d pp = ground->GetPoint( edgeQuantity );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	sprite.setRotation( angle / PI * 180 );
	sprite.setPosition( pp.x, pp.y );
	//----

	UpdateHitboxes();

}

void BossCrawler::HandleEntrant( QuadTreeEntrant *qte )
{
	assert( queryMode != "" );

	Edge *e = (Edge*)qte;


	if( ground == e )
			return;

	if( queryMode == "resolve" )
	{
		Contact *c = owner->coll.collideEdge( position + physBody.offset, physBody, e, tempVel, V2d( 0, 0 ) );

		if( c != NULL )
		{
			if( !col || (minContact.collisionPriority < 0 ) || (c->collisionPriority <= minContact.collisionPriority && c->collisionPriority >= 0 ) ) //(c->collisionPriority >= -.00001 && ( c->collisionPriority <= minContact.collisionPriority || minContact.collisionPriority < -.00001 ) ) )
			{	

				if( ground != NULL && e == ground->edge1 && ( c->normal.x == 0 && c->normal.y == 0 ) )
				{
					return;
				}

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
}

void BossCrawler::UpdatePrePhysics()
{
	int standLength = 60;
	int shootLength = 60;
	int lungeLength = 20;
	int lungeLandLength = 20;
	int rollLength = 60;
	int stunLength = 60;
	bool doneRunning = false;
	int runFrames = 60;

	double runSpeed = 10;
	double rollSpeed = 15;
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
				ground = NULL;
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
			cout << "stand" << endl;
		}
		break;
	case SHOOT:
		{
			cout << "shoot" << endl;
		}
		break;
	case LUNGE:
		{
			groundSpeed = 0;
			cout << "lunge" << endl;
		}
		break;
	case LUNGEAIR:
		{
			cout << "lunge air" << endl;
			velocity += V2d( 0, gravity / slowMultiple );
		}
		break;
	case LUNGELAND:
		{
			groundSpeed = 0;
			cout << "lunge land" << endl;
		}
		break;
	case RUN:
		{
			cout << "run" << endl;
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
			cout << "roll" << endl;
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
			cout << "stunned" << endl;
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
}

void BossCrawler::UpdatePhysics()
{
	double maxMovement = min( physBody.rw, physBody.rh );

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
				q = 0;
									ground = e1;
			if( gNormal == e1n )
			{
				
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
				q = e0Length;
				ground = e0;
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
			action = LUNGELAND;
			frame = 0;
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

void BossCrawler::PhysicsResponse()
{
	if( !dead && receivedHit == NULL )
	{
		if( ground != NULL )
		{
			V2d gn = ground->Normal();
			V2d gPoint = ground->GetPoint( edgeQuantity );
			position = gPoint + gn * 16.0;
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
			owner->player.test = true;
			owner->player.currAttackHit = true;
			owner->player.flashColor = COLOR_BLUE;
			owner->player.flashFrames = 5;
			owner->player.swordShader.setParameter( "energyColor", COLOR_BLUE );

			if( owner->player.ground == NULL && owner->player.velocity.y > 0 )
			{
				owner->player.velocity.y = 4;//.5;
			}
		
			//owner->ActivateEffect( ts_testBlood, position, true, 0, 6, 3, facingRight );

		}

		if( IHitPlayer() )
		{
		}
	}

	if( !dead )
	{
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
	switch( action )
	{
	case STAND:
		{
			sprite.setTexture( *ts_walk->texture );
			sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
			V2d pp = ground->GetPoint( edgeQuantity );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		break;
	case SHOOT:
		{
			sprite.setTexture( *ts_walk->texture );
			sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
			V2d pp = ground->GetPoint( edgeQuantity );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		break;
	case LUNGE:
		{
			sprite.setTexture( *ts_walk->texture );
			sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
			V2d pp = ground->GetPoint( edgeQuantity );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		break;
	case LUNGEAIR:
		{
			sprite.setTexture( *ts_walk->texture );
			sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );

			
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
			//sprite.setRotation( 0 );
			sprite.setPosition( position.x, position.y );
		}
		break;
	case LUNGELAND:
		{
			sprite.setTexture( *ts_walk->texture );
			sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
			V2d pp = ground->GetPoint( edgeQuantity );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		break;
	case RUN:
		{
		//	cout << "putting sprite at the correct angle: " << angle << endl;
			sprite.setTexture( *ts_walk->texture );
			sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
			V2d pp = ground->GetPoint( edgeQuantity );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		break;
	case ROLL:
		{
			sprite.setTexture( *ts_walk->texture );
			sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
			V2d pp = ground->GetPoint( edgeQuantity );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
	case STUNNED:
		{
			sprite.setTexture( *ts_walk->texture );
			sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
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



	//sprite.setPosition( position );
	//UpdateHitboxes();
}

bool BossCrawler::PlayerSlowingMe()
{
	Actor &player = owner->player;
	for( int i = 0; i < player.maxBubbles; ++i )
	{
		if( player.bubbleFramesToLive[i] > 0 )
		{
			if( length( position - player.bubblePos[i] ) <= player.bubbleRadius )
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
		target->draw( sprite );
	}
}

bool BossCrawler::IHitPlayer()
{
	Actor &player = owner->player;
	
	if( hitBody.Intersects( player.hurtBody ) )
	{
		player.ApplyHit( hitboxInfo );
		return true;
	}
	
	return false;
}

 pair<bool, bool> BossCrawler::PlayerHitMe()
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
