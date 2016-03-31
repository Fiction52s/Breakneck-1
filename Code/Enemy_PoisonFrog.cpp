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


PoisonFrog::PoisonFrog( GameSession *owner, Edge *g, double q )
	:Enemy( owner, EnemyType::POISONFROG ), ground( g ), edgeQuantity( q )
{
	actionLength[STAND] = 10;
	actionLength[JUMPSQUAT] = 10;
	actionLength[JUMP] = 2;
	actionLength[LAND] = 5;

	animFactor[STAND] = 1;
	animFactor[JUMPSQUAT] = 1;
	animFactor[JUMP] = 1;
	animFactor[LAND] = 1;

	invincibleFrames = 0;
	double width = 64;
	double height = 64;
	ts_test = owner->GetTileset( "poisonfrog_64x64.png", width, height );


	initHealth = 60;
	health = initHealth;
	
	//cout << "creating the boss crawler" << endl;
	action = STAND;
	gravity = 1;
	facingRight = false;
	receivedHit = NULL;

	
	dead = false;
	//sprite.setTexture( *ts_walk->texture );
	//sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
	//sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	V2d gPoint = g->GetPoint( edgeQuantity );

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

	physBody.isCircle = true;
	physBody.offset.x = 0;
	physBody.offset.y = 0;
	physBody.rw = 64;
	physBody.rh = 64;
	physBody.type = CollisionBox::BoxType::Physics;

	startGround = ground;
	startQuant = edgeQuantity;
	frame = 0;
	position = gPoint + ground->Normal() * physBody.rh; //16.0;
}

void PoisonFrog::ResetEnemy()
{
	invincibleFrames = 0;
	health = initHealth;
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

	action = STAND;
	frame = 0;

	facingRight = false;
	groundSpeed = 0;
}

void PoisonFrog::HandleEntrant( QuadTreeEntrant *qte )
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
	++possibleEdgeCount;
}

void PoisonFrog::UpdateHitboxes()
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
		}
	}
}

void PoisonFrog::UpdatePrePhysics()
{
	ActionEnded();

	Actor &player = owner->player;

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
			AttemptSpawnMonitor();
			dead = true;
			owner->quit = true;
		}

		receivedHit = NULL;
	}

	switch( action )
	{
	case STAND:
		{
			if( player.position.x < position.x )
			{
				facingRight = false;
			}
			else if( player.position.x > position.x )
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
				ground = NULL;
				velocity = V2d( 0, -20 );
			}
		}
		break;
	case LAND:
		{

		}
		break;
	}
}

void PoisonFrog::UpdatePhysics()
{
	double maxMovement = min( physBody.rw, physBody.rh );

	if( ground != NULL )
	{
		//never moves on the ground
	}
	else
	{
		V2d movementVec = velocity;
		movementVec /= slowMultiple * NUM_STEPS;

		bool hit = ResolvePhysics( movementVec );
		if( hit && minContact.edge->Normal().y < 0 )
		{
			ground = minContact.edge;
			edgeQuantity = ground->GetQuantity( minContact.position + minContact.resolution );
			action = LAND;
			frame = 0;
		}	
	}

	PhysicsResponse();
}

bool PoisonFrog::ResolvePhysics( V2d vel )
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

	return col;
}

void PoisonFrog::PhysicsResponse()
{
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
			owner->player.test = true;
			owner->player.currAttackHit = true;
			owner->player.flashColor = COLOR_BLUE;
			owner->player.flashFrames = 5;
			//owner->player.desperationMode = false;
			owner->player.swordShader.setParameter( "energyColor", COLOR_BLUE );
			//owner->powerBar.Charge( 2 * 6 * 2 );
			//owner->powerBar.Charge( 6 );

			if( owner->player.ground == NULL && owner->player.velocity.y > 0 )
			{
				owner->player.velocity.y = 4;//.5;
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
	}
}

void PoisonFrog::UpdatePostPhysics()
{
	if( receivedHit != NULL )
		owner->Pause( 5 );

	sprite.setTexture( *ts_test->texture );
	sprite.setTextureRect( ts_test->GetSubRect( 0 ) );
	sprite.setScale( 1.3, 1.3 );
	switch( action )
	{
	case STAND:
		{
			V2d pp = ground->GetPoint( edgeQuantity );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		break;
	case JUMPSQUAT:
		{
			V2d pp = ground->GetPoint( edgeQuantity );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		break;
	case JUMP:
		{
			V2d pp = ground->GetPoint( edgeQuantity );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		break;
	case LAND:
		{
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
			sprite.setPosition( position.x, position.y );
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
}

bool PoisonFrog::PlayerSlowingMe()
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

void PoisonFrog::Draw(sf::RenderTarget *target )
{
	if( !dead )
	{
		target->draw( sprite );
	}
}

void PoisonFrog::DrawMinimap( sf::RenderTarget *target )
{
	CircleShape cs;
	cs.setRadius( 80 );
	cs.setFillColor( COLOR_BLUE );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	target->draw( cs );
}

bool PoisonFrog::IHitPlayer()
{
	Actor &player = owner->player;
	
	if( hitBody.Intersects( player.hurtBody ) )
	{
		hitboxInfo->kbDir = normalize( player.position - position );
		//knockback stuff?
		if( player.position.x < position.x )
		{
			//hitboxInfo->kbDir = V2d( -1, -1 ); //-abs( hitboxInfo->kbDir.x );
		}
		else if( player.position.x > position.x )
		{
			//hitboxInfo->kbDir = V2d( 1, -1 );//abs( hitboxInfo->kbDir.x );
		}


		player.ApplyHit( hitboxInfo );
		return true;
	}
	
	return false;
}

 pair<bool, bool> PoisonFrog::PlayerHitMe()
{
	if( invincibleFrames > 0 )
	{
		return pair<bool,bool>(false,false);
	}

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

void PoisonFrog::UpdateSprite()
{
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

		physBody.DebugDraw( target );
		hitBody.DebugDraw( target );
	}
}

void PoisonFrog::SaveEnemyState()
{
}

void PoisonFrog::LoadEnemyState()
{
}