#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>


Crawler::Crawler( GameSession *owner, Edge *g, double q, bool cw, double s )
	:Enemy( owner, EnemyType::CRAWLER ), ground( g ), edgeQuantity( q ), clockwise( cw ), groundSpeed( s )
{
	lastReverser = false;
	dead = false;
	ts_walk = owner->GetTileset( "crawlerwalk.png", 96, 64 );
	ts_roll = owner->GetTileset( "crawlerroll.png", 96, 64 );
	sprite.setTexture( *ts_walk->texture );
	sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	V2d gPoint = g->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	roll = false;

	if( !clockwise )
	{
		groundSpeed = -groundSpeed;
	}


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
	hitboxInfo->drain = 0;
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

void Crawler::ResetEnemy()
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
	double angle = 0;
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

void Crawler::HandleEntrant( QuadTreeEntrant *qte )
{
	assert( queryMode != "" );

	if( queryMode == "resolve" )
	{
		Edge *e = (Edge*)qte;


		if( ground == e )
			return;

		Contact *c = owner->coll.collideEdge( position + physBody.offset, physBody, e, tempVel, V2d( 0, 0 ) );

		if( c != NULL )
		{
			if( !col || (minContact.collisionPriority < 0 ) || (c->collisionPriority <= minContact.collisionPriority && c->collisionPriority >= 0 ) ) //(c->collisionPriority >= -.00001 && ( c->collisionPriority <= minContact.collisionPriority || minContact.collisionPriority < -.00001 ) ) )
			{	

				if( e == ground->edge1 && ( c->normal.x == 0 && c->normal.y == 0 ) )
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
	else if( queryMode == "reverse" )
	{	
		CrawlerReverser *cr = (CrawlerReverser*)qte;
		if( cr != lastReverser )
		{
			//cout << "PLEASE" << endl;
			if( cr->hurtBody.Intersects( physBody ) )
			{
				//cout << "reversed!" << endl;
				lastReverser = cr;
				groundSpeed = -groundSpeed;
				clockwise = !clockwise;
			}
		}
	}
	++possibleEdgeCount;
}

void Crawler::UpdateHitboxes()
{
	if( ground != NULL )
	{
		V2d gn = ground->Normal();
		double angle = 0;
		if( !approxEquals( abs(offset.x), physBody.rw ) )
		{
			//this should never happen
		}
		else
		{
			angle = atan2( gn.x, -gn.y );
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

void Crawler::UpdatePrePhysics()
{
	if( ( !roll && frame == 17 * crawlAnimationFactor )
		|| ( roll && frame == 7 * rollAnimationFactor ) )
	{
		frame = 0;
	}
}

void Crawler::UpdatePhysics()
{
	double movement = 0;
	double maxMovement = min( physBody.rw, physBody.rh );
	movement = groundSpeed;

	movement /= slowMultiple * NUM_STEPS;

	while( movement != 0 )
	{
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

		if( movement > 0 && q == groundLength )
		{
			double c = cross( e1n, gNormal );
			double d = dot( e1n, gNormal );
			//cout << "c: " << c << ", d: " << d << endl;
			//if( c >= -.5 && d > 0 )
			//if( d > . )
			if( gNormal == e1n )
			{
				q = 0;
				ground = e1;
			}
			else if( !roll )
			{
				roll = true;
				rollFactor = 0;
				frame = 0;
			}
			else
			{
				double angle = m / 3.0 /  physBody.rw;
				V2d currVec = position - ground->v1;
				V2d newPos;
				newPos.x = currVec.x * cos( angle ) - 
					currVec.y * sin( angle ) + ground->v1.x;
				newPos.y = currVec.x * sin( angle ) + 
					currVec.y * cos( angle ) + ground->v1.y;
				V2d newVec = newPos - ground->v1;
				double rollNew = atan2( newVec.y, newVec.x );
				if( rollNew < 0 )
				{
					rollNew += 2 * PI;
				}

				double oldRollFactor = rollFactor;
				double rollStart = atan2( gNormal.y, gNormal.x );
				V2d startVec = V2d( cos( rollStart ), sin( rollStart ) );
				double rollEnd = atan2( e1n.y, e1n.x );

				if( rollStart < 0 )
					rollStart += 2 * PI;
				if( rollEnd < 0 )
					rollEnd += 2 * PI;

				//cout << "totalAngleDist: " << totalAngleDist << endl;
				//cout << "angleDist: " << angleDist << endl;
				//cout << "rollEnd: " << rollEnd << endl;
				//cout << "rollNew: " << rollNew << endl;

				bool changed = false;
				if( rollEnd > rollStart && rollNew > rollEnd )
				{
					changed = true;
					newPos = ground->v1 + e1n * physBody.rw;
				}
				else if( rollEnd < rollStart && rollNew < rollEnd )
				{
					changed = true;
					newPos = ground->v1 + e1n * physBody.rw;
				}

				//V2d newVec = newPos - ground->v1;


				bool hit = ResolvePhysics( newPos - position );
				if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
				{
					V2d eNorm = minContact.edge->Normal();
					ground = minContact.edge;
					q = ground->GetQuantity( minContact.position + minContact.resolution );
					edgeQuantity = q;
					V2d gn = ground->Normal();
					roll = false;
					//cout << "hitting" << endl;
					break;
				}

				if( changed )
				{
					ground = e1;
					q = 0;
					roll = false;
				}
				//if no hit


				//if( rollFactor < 1.0 )
				//{ 
				//	double oldRollFactor = rollFactor;
				//	double rollStart = atan2( gNormal.y, gNormal.x );
				//	V2d startVec = V2d( cos( rollStart ), sin( rollStart ) );
				//	double rollEnd = atan2( e1n.y, e1n.x );

				//	if( rollStart < 0 )
				//		rollStart += 2 * PI;
				//	if( rollEnd < 0 )
				//		rollEnd += 2 * PI;

				//	V2d currentVec = position - ground->v1;
				//	currentVec = normalize( currentVec );
				//	double rollCurrent = atan2( currentVec.y, currentVec.x );
				//	if( rollCurrent < 0 )
				//		rollCurrent += 2 * PI;


				//	double totalAngleDist = rollEnd - rollStart;
				//	if( rollEnd < rollStart )
				//	{
				//		totalAngleDist = ( 2 * PI - rollStart ) + rollEnd;
				//	}


				//	double angleDist = rollEnd - rollCurrent;

				//	if( rollEnd < rollCurrent )
				//	{
				//		angleDist = ( 2 * PI - rollCurrent ) + rollEnd;
				//	}

				//	

				//	double arcDist = angleDist * physBody.rw;
				//	//arcDist *= 100;
				//	double oldArcDist = arcDist;
				//	//m /= 10;
				//	movement -= m;
				//	if( movement < 0 )
				//	{
				//		assert( false );
				//		movement = 0;
				//	}
				//	if( m > arcDist )
				//	{
				//		//cout << "m: " << m << ", arcDist: " << arcDist << endl;
				//		//double realMove = ;
				//		m -= arcDist;
				//		if( approxEquals( m, 0 ) )
				//		{
				//			m = 0;
				//		}
				//		rollFactor = 1;
				//		movement += m;

				//		V2d oldPos = position;
				//		V2d rollEndVec = V2d( cos( rollEnd ), sin ( rollEnd ) );
				//		V2d newPos = ground->v1 + rollEndVec * physBody.rw;

				//		bool hit = ResolvePhysics( newPos - oldPos );
				//		if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
				//		{
				//			V2d eNorm = minContact.edge->Normal();
				//			ground = minContact.edge;
				//			q = ground->GetQuantity( minContact.position + minContact.resolution );
				//			edgeQuantity = q;
				//			V2d gn = ground->Normal();
				//			roll = false;
				//			break;
				//		}			
				//	}
				//	else
				//	{
				//		//m = 0;
				//		arcDist -= m;
				//		rollFactor = ( totalAngleDist - arcDist / physBody.rw ) / totalAngleDist;

				//		V2d oldPos = position;
				//		double trueAngle = rollStart + angleDist * rollFactor;
				//		if( trueAngle > PI * 2 )
				//		{
				//			trueAngle -= PI * 2;
				//		}

				//		V2d trueVec = V2d( cos( trueAngle ), sin( trueAngle ) );
				//		
				//		V2d newPos = ground->v1 + trueVec * physBody.rw;

				//		//cout << "current: " << rollCurrent << ", new: " << rollFactor << "total: " << totalAngleDist << ", arcdist: " << arcDist << endl;
				//		//cout << "other vel: " << (newPos-oldPos).x << ", " << (newPos-oldPos).y << endl;
				//		bool hit = ResolvePhysics( newPos - oldPos );
				//		if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
				//		{
				//			V2d eNorm = minContact.edge->Normal();
				//			ground = minContact.edge;
				//			q = ground->GetQuantity( minContact.position + minContact.resolution );
				//			edgeQuantity = q;
				//			V2d gn = ground->Normal();
				//			roll = false;
				//			break;
				//		}			
				//		//rollFactor = 
				//	}




				//	
			

				//	//movement += m;
				//	//rollFactor += .01;

				//	//double diff = abs( rollStart - rollEnd );

				//	//if( rollFactor > 1.0 )
				//	//	rollFactor = 1.0;
				//}
				//else
				//{
				//	ground = e1;
				//	q = 0;
				//	roll = false;
				//}
			}
		}
		else if( movement < 0 && q == 0 )
		{
			double d = dot( e1n, gNormal );

			if( gNormal == e0n )
			{
			//	cout << "what" << endl;
				q = length( e0->v1 - e0->v0 );
				ground = e0;
			}
			else if( !roll )
			{
				roll = true;
				rollFactor = 0;
				frame = 0;
			}
			else
			{
				double angle = m / 3.0 / physBody.rw;
				V2d currVec = position - ground->v0;
				V2d newPos;
				newPos.x = currVec.x * cos( angle ) - 
					currVec.y * sin( angle ) + ground->v0.x;
				newPos.y = currVec.x * sin( angle ) + 
					currVec.y * cos( angle ) + ground->v0.y;
				V2d newVec = newPos - ground->v0;
				double rollNew = atan2( newVec.y, newVec.x );
				if( rollNew < 0 )
				{
					rollNew += 2 * PI;
				}

				double oldRollFactor = rollFactor;
				double rollStart = atan2( gNormal.y, gNormal.x );
				V2d startVec = V2d( cos( rollStart ), sin( rollStart ) );
				double rollEnd = atan2( e0n.y, e0n.x );

				if( rollStart < 0 )
					rollStart += 2 * PI;
				if( rollEnd < 0 )
					rollEnd += 2 * PI;

				bool changed = false;
				if( rollEnd < rollStart && rollNew < rollEnd )
				{
					changed = true;
					newPos = ground->v0 + e0n * physBody.rw;
				}
				else if( rollEnd > rollStart && rollNew > rollEnd )
				{
					changed = true;
					newPos = ground->v0 + e0n * physBody.rw;
				}

				bool hit = ResolvePhysics( newPos - position );
				if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
				{
					V2d eNorm = minContact.edge->Normal();
					ground = minContact.edge;
					q = ground->GetQuantity( minContact.position + minContact.resolution );
					edgeQuantity = q;
					V2d gn = ground->Normal();
					roll = false;
					//cout << "hitting" << endl;
					break;
				}	

				if( changed )
				{
					ground = e0;
					q = length( e0->v1 - e0->v0 );
					roll = false;
				}

				//}
				//else
				//{
				//	ground = e0;
				//	q = length( e0->v1 - e0->v0 );
				//	roll = false;
					//cout << "roll now false" << endl;
				//}
				//if( rollFactor < 1.0 )
				//{ 
				//	double oldRollFactor = rollFactor;
				//	double rollStart = atan2( gNormal.y, gNormal.x );
				//	V2d startVec = V2d( cos( rollStart ), sin( rollStart ) );
				//	double rollEnd = atan2( e0n.y, e0n.x );

				//	if( rollStart < 0 )
				//		rollStart += 2 * PI;
				//	if( rollEnd < 0 )
				//		rollEnd += 2 * PI;

				//	V2d currentVec = position - ground->v0;
				//	cout << "pos: " << position.x << ", " << position.y << ", currentVec: " << currentVec.x << ", " << currentVec.y << endl;
				//	currentVec = normalize( currentVec );
				//	double rollCurrent = atan2( currentVec.y, currentVec.x );
				//	if( rollCurrent < 0 )
				//		rollCurrent += 2 * PI;

				//	//everything before here is right for sure		2 

				//	double totalAngleDist = rollStart - rollEnd;//rollEnd - rollStart;
				//	if( rollStart < rollEnd )
				//	{
				//		totalAngleDist = ( 2 * PI - rollEnd ) + rollStart;
				//	}

				//	cout << "rollCurrent: " << rollCurrent << ", rollEnd: " << rollEnd << endl;
				//	double angleDist = rollCurrent - rollEnd;//rollEnd - rollCurrent;

				//	if( rollCurrent < rollEnd )
				//	{
				//		cout << "adjustment" << endl;
				//		angleDist = ( 2 * PI - rollEnd ) + rollCurrent;
				//	}

				//	/*if( rollEnd < rollCurrent )
				//	{
				//		cout << "adjustment" << endl;
				//		angleDist = ( 2 * PI - rollCurrent ) + rollEnd;
				//	}*/
				//	cout << "totalAngleDist: " << totalAngleDist << endl;
				//	cout << "angleDist: " << angleDist << endl;
				//	
				//	double arcDist = angleDist * physBody.rw;
				//	//arcDist *= 100;
				//	double oldArcDist = arcDist;
				//	//m /= 10;
				//	//movement -= m;
				//	movement = 0;

				//	sf::Transform t;
				//}
				
				//if( rollFactor < 1.0 )
				//{ 
				//	double oldRollFactor = rollFactor;
				//	double rollStart = atan2( gNormal.y, gNormal.x );
				//	V2d startVec = V2d( cos( rollStart ), sin( rollStart ) );
				//	double rollEnd = atan2( e0n.y, e0n.x );

				//	if( rollStart < 0 )
				//		rollStart += 2 * PI;
				//	if( rollEnd < 0 )
				//		rollEnd += 2 * PI;

				//	V2d currentVec = position - ground->v0;
				//	cout << "pos: " << position.x << ", " << position.y << ", currentVec: " << currentVec.x << ", " << currentVec.y << endl;
				//	currentVec = normalize( currentVec );
				//	double rollCurrent = atan2( currentVec.y, currentVec.x );
				//	if( rollCurrent < 0 )
				//		rollCurrent += 2 * PI;

				//	//everything before here is right for sure		2 

				//	double totalAngleDist = rollStart - rollEnd;//rollEnd - rollStart;
				//	if( rollStart < rollEnd )
				//	{
				//		totalAngleDist = ( 2 * PI - rollEnd ) + rollStart;
				//	}

				//	cout << "rollCurrent: " << rollCurrent << ", rollEnd: " << rollEnd << endl;
				//	double angleDist = rollCurrent - rollEnd;//rollEnd - rollCurrent;

				//	if( rollCurrent < rollEnd )
				//	{
				//		cout << "adjustment" << endl;
				//		angleDist = ( 2 * PI - rollEnd ) + rollCurrent;
				//	}

				//	/*if( rollEnd < rollCurrent )
				//	{
				//		cout << "adjustment" << endl;
				//		angleDist = ( 2 * PI - rollCurrent ) + rollEnd;
				//	}*/
				//	cout << "totalAngleDist: " << totalAngleDist << endl;
				//	cout << "angleDist: " << angleDist << endl;
				//	
				//	double arcDist = angleDist * physBody.rw;
				//	//arcDist *= 100;
				//	double oldArcDist = arcDist;
				//	//m /= 10;
				//	movement -= m;
				//	if( movement > 0 )
				//	{
				//		assert( false );
				//		movement = 0;
				//	}

				//	if( -m > arcDist )
				//	{
				//		//cout << "m: " << m << ", arcDist: " << arcDist << endl;
				//		//double realMove = ;
				//		m += arcDist;
				//		if( approxEquals( m, 0 ) )
				//		{
				//			m = 0;
				//		}
				//		rollFactor = 1;
				//		movement += m;

				//		V2d oldPos = position;
				//		V2d rollEndVec = V2d( cos( rollEnd ), sin ( rollEnd ) );
				//		V2d newPos = ground->v0 + rollEndVec * physBody.rw;

				//		bool hit = ResolvePhysics( newPos - oldPos );
				//		if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
				//		{
				//			V2d eNorm = minContact.edge->Normal();
				//			ground = minContact.edge;
				//			q = ground->GetQuantity( minContact.position + minContact.resolution );
				//			edgeQuantity = q;
				//			V2d gn = ground->Normal();
				//			roll = false;
				//			break;
				//		}			
				//	}
				//	else
				//	{
				//		//m = 0;
				//		cout << "arcdist: " << arcDist << ", m: " << m << endl;
				//		arcDist += m;
				//		rollFactor = ( totalAngleDist - arcDist / physBody.rw ) / totalAngleDist;
				//		cout << "rollFActor: "<< rollFactor << ", current arcdist: " << arcDist << endl;
				//		V2d oldPos = position;
				//		cout << "rollStart: " << rollStart << ", rollEnd: " << rollEnd << ", angleDist: " << angleDist << endl;
				//		//double trueAngle = rollStart + angleDist * rollFactor;
				//		double trueAngle = rollStart - angleDist * rollFactor;
				//		
				//		if( trueAngle > PI * 2 )
				//		{
				//			trueAngle -= PI * 2;
				//		}
				//	//	if( trueAngle < 0 )
				//	//	{
				//	//		trueAngle += PI * 2;
				//	//	}

				//		

				//		V2d trueVec = V2d( cos( trueAngle ), sin( trueAngle ) );
				//		//cout << "trueVec: " << trueVec.x << ", " << trueVec.y << endl;
				//		V2d newPos = ground->v0 + trueVec * physBody.rw;
				//		cout << "trueangle: " << trueAngle << ", newPos: " << newPos.x << ", " << newPos.y << endl;
				//		//cout << "current: " << rollCurrent << ", new: " << rollFactor << "total: " << totalAngleDist << ", arcdist: " << arcDist << endl;
				//		//cout << "other vel: " << (newPos-oldPos).x << ", " << (newPos-oldPos).y << endl;
				//		cout << "movement: " << (newPos-oldPos).x << ", " << (newPos.y - oldPos.y) << endl;
				//		bool hit = ResolvePhysics( newPos - oldPos );
				//		if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
				//		{
				//			V2d eNorm = minContact.edge->Normal();
				//			ground = minContact.edge;
				//			q = ground->GetQuantity( minContact.position + minContact.resolution );
				//			edgeQuantity = q;
				//			V2d gn = ground->Normal();
				//			roll = false;
				//			break;
				//		}			
				//		//rollFactor = 
				//	}




				//	
			

				//	//movement += m;
				//	//rollFactor += .01;

				//	//double diff = abs( rollStart - rollEnd );

				//	//if( rollFactor > 1.0 )
				//	//	rollFactor = 1.0;
				//}
				//else
				//{
				//	ground = e0;
				//	q = length( e0->v1 - e0->v0 );
				//	roll = false;
				//}
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
					cout << "hit" << endl;
					break;
				}			
			}
			else
			{
				if( clockwise )
				{
					ground = e1;
					q = 0;
				}
				else
				{
					//cout << "here>? " << endl;
					ground = e0;
					q = length( e0->v1 - e0->v0 );
				}
				
			}
		}

		if( movement == extra )
			movement += steal;
		else
			movement = steal;

		edgeQuantity = q;
	}

	PhysicsResponse();
}

bool Crawler::ResolvePhysics( V2d vel )
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

void Crawler::PhysicsResponse()
{
	if( !dead )
	{
		//cout << "response" << endl;
		double spaceNeeded = 0;
		V2d gn = ground->Normal();
		V2d gPoint = ground->GetPoint( edgeQuantity );
	

		double angle = 0;
	
		if( !roll )
		{
			position = gPoint + gn * 16.0;
			angle = atan2( gn.x, -gn.y );
		
			sprite.setTexture( *ts_walk->texture );
			IntRect r = ts_walk->GetSubRect( frame / crawlAnimationFactor );
			if( !clockwise )
			{
				sprite.setTextureRect( sf::IntRect( r.left + r.width, r.top, -r.width, r.height ) );
			}
			else
			{
				sprite.setTextureRect( r );
			}
			
			V2d pp = ground->GetPoint( edgeQuantity );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		else
		{
			
			if( clockwise )
			{
				/*V2d e1n = ground->edge1->Normal();
				double rollStart = atan2( gn.y, gn.x );
				double rollEnd = atan2( e1n.y, e1n.x );
				double adjRollStart = rollStart;
				double adjRollEnd = rollEnd;

				if( rollStart < 0 )
					adjRollStart += 2 * PI;
				if( rollEnd < 0 )
					adjRollEnd += 2 * PI;
		
				if( adjRollEnd > adjRollStart )
				{
					angle  = adjRollStart * ( 1.0 - rollFactor ) + adjRollEnd  * rollFactor ;
				}
				else
				{
			
					angle = rollStart * ( 1.0 - rollFactor ) + rollEnd  * rollFactor;

					if( rollStart < 0 )
						rollStart += 2 * PI;
					if( rollEnd < 0 )
						rollEnd += 2 * PI;
				}

				if( angle < 0 )
					angle += PI * 2;

		

				V2d angleVec = V2d( cos( angle ), sin( angle ) );
				angleVec = normalize( angleVec );

				position = gPoint + angleVec * 16.0;

				angle += PI / 2.0;*/

				V2d vec = normalize( position - ground->v1 );
				angle = atan2( vec.y, vec.x );
				angle += PI / 2.0;
	

				sprite.setTexture( *ts_roll->texture );
				IntRect r = ts_roll->GetSubRect( frame / rollAnimationFactor );
				if( clockwise )
				{
					sprite.setTextureRect( r );
				}
				else
				{
					sprite.setTextureRect( sf::IntRect( r.left + r.width, r.top, -r.width, r.height ) );
				}
			
				sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
				sprite.setRotation( angle / PI * 180 );
				V2d pp = ground->GetPoint( edgeQuantity );
				sprite.setPosition( pp.x, pp.y );
			}
			else
			{
				//angle = 
				/*V2d e0n = ground->edge0->Normal();
				double rollStart = atan2( gn.y, gn.x );
				double rollEnd = atan2( e0n.y, e0n.x );
				double adjRollStart = rollStart;
				double adjRollEnd = rollEnd;

				if( rollStart < 0 )
					adjRollStart += 2 * PI;
				if( rollEnd < 0 )
					adjRollEnd += 2 * PI;
		
				if( adjRollEnd > adjRollStart )
				{
					angle  = adjRollStart * ( 1.0 - rollFactor ) + adjRollEnd  * rollFactor ;
				}
				else
				{
			
					angle = rollStart * ( 1.0 - rollFactor ) + rollEnd  * rollFactor;

					if( rollStart < 0 )
						rollStart += 2 * PI;
					if( rollEnd < 0 )
						rollEnd += 2 * PI;
				}

				if( angle < 0 )
					angle += PI * 2;*/

			

			//	V2d angleVec = V2d( cos( angle ), sin( angle ) );
			//	angleVec = normalize( angleVec );

			//	position = gPoint + angleVec * 16.0;
				V2d vec = normalize( position - ground->v0 );
				angle = atan2( vec.y, vec.x );
				angle += PI / 2.0;
	

				sprite.setTexture( *ts_roll->texture );
				IntRect r = ts_roll->GetSubRect( frame / rollAnimationFactor );
				if( clockwise )
				{
					sprite.setTextureRect( r );
				}
				else
				{
					sprite.setTextureRect( sf::IntRect( r.left + r.width, r.top, -r.width, r.height ) );
				}
			
				sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
				sprite.setRotation( angle / PI * 180 );
				V2d pp = ground->GetPoint( edgeQuantity );
				sprite.setPosition( pp.x, pp.y );
			}

			
		}
	

		UpdateHitboxes();

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

		pair<bool, bool> result = PlayerHitMe();
		if( result.first )
		{
		//	cout << "patroller received damage of: " << receivedHit->damage << endl;
			if( !result.second )
			{
				owner->Pause( 6 );
			}
			
			dead = true;
			receivedHit = NULL;
		}

		if( IHitPlayer() )
		{
		//	cout << "patroller just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}

		queryMode = "reverse";

		//physbody is a circle
		Rect<double> r( position.x - physBody.rw, position.y - physBody.rw, physBody.rw * 2, physBody.rw * 2 );
		owner->crawlerReverserTree->Query( this, r );
	}
}

void Crawler::UpdatePostPhysics()
{
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

bool Crawler::PlayerSlowingMe()
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

void Crawler::Draw(sf::RenderTarget *target )
{
	if( !dead )
	{
		target->draw( sprite );
	}
}

bool Crawler::IHitPlayer()
{
	Actor &player = owner->player;
	
	if( hitBody.Intersects( player.hurtBody ) )
	{
		player.ApplyHit( hitboxInfo );
		return true;
	}
	
	return false;
}

 pair<bool, bool> Crawler::PlayerHitMe()
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

void Crawler::UpdateSprite()
{
}

void Crawler::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{

		CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 10 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		V2d g = ground->GetPoint( edgeQuantity );
		cs.setPosition( g.x, g.y );

		//owner->window->draw( cs );
		//UpdateHitboxes();
		physBody.DebugDraw( target );
	}
//	hurtBody.DebugDraw( target );
//	hitBody.DebugDraw( target );
}

void Crawler::SaveEnemyState()
{
}

void Crawler::LoadEnemyState()
{
}

CrawlerReverser::CrawlerReverser( GameSession *owner, Edge *edge, double q )
	:ground( edge ), quantity( q )
{

	V2d gNorm = edge->Normal();
	ts = owner->GetTileset( "crawlerreverser.png", 32, 32 );
	position = edge->GetPoint( quantity ) + 16.0 * gNorm;
	sprite.setTexture( *ts->texture );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );

	drawNext = NULL;
	double angle = atan2( gNorm.y, gNorm.x );
	sprite.setRotation( angle );
	sprite.setPosition( position.x, position.y );

	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 32;
	hurtBody.rh = 32;

	hurtBody.globalAngle = 0;
	hurtBody.globalPosition = position;
}

void CrawlerReverser::HandleQuery( QuadTreeCollider *qtc )
{
	qtc->HandleEntrant( this );
}

bool CrawlerReverser::IsTouchingBox( const sf::Rect<double> &r )
{
	V2d rA( r.left, r.top );
	V2d rB( r.left + r.width, r.top );
	V2d rC( r.left + r.width, r.top + r.height );
	V2d rD( r.left, r.top + r.height );

	V2d gPos = ground->GetPoint( quantity );
	V2d along = normalize( ground->v1 - ground->v0 );
	V2d other( along.y, -along.x );

	double width = 16;
	double height = 16;
	V2d A = gPos - along * width;
	V2d B = A + other * height;
	V2d C = B + along * width;
	V2d D = C - other * height;

	if( isQuadTouchingQuad( rA, rB, rC, rD, A, B, C, D ) )
	{
		return true;
	}

	return false;
}

void CrawlerReverser::Draw( sf::RenderTarget *target )
{
	target->draw( sprite );
}