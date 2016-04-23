#include "Mover.h"
#include "GameSession.h"
#include <iostream>

#define TIMESTEP 1.0 / 60.0
#define V2d sf::Vector2<double>

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

using namespace std;
using namespace sf;

SurfaceMover::SurfaceMover( GameSession *p_owner, Edge *startGround, double startQuantity, double radius )
	:ground( startGround ), edgeQuantity( startQuantity ), owner( p_owner ),
	groundSpeed( 0 ), roll( false )
{
	gravity = V2d( 0, 0 );
	physBody.isCircle = true;
	physBody.rw = radius;
	physBody.rh = radius;
	physBody.offset = V2d( 0, 0 );

	UpdateGroundPos();
}

void SurfaceMover::UpdateGroundPos()
{
	if( roll )
	{
		V2d n;
		if( edgeQuantity < .01 )
		{
			//basically 0
			n = normalize( physBody.globalPosition - ground->v0 );
		}
		else
		{
			n = normalize( physBody.globalPosition - ground->v1 );
		}
		physBody.globalAngle = atan2( n.x, -n.y );
	}
	else
	{
		V2d gn = ground->Normal();
		physBody.globalPosition = ground->GetPoint( edgeQuantity )
			+ gn * physBody.rw;
		physBody.globalAngle = atan2( gn.x, -gn.y );
	}
	
}

void SurfaceMover::SetSpeed( double speed )
{
	if( roll )
	{
		if( groundSpeed > 0 && speed < 0 )
		{
			Edge *next = ground->edge1;
			ground = next;

			edgeQuantity = 0;
		}
		else if( groundSpeed < 0 && speed > 0 )
		{
			Edge *prev = ground->edge0;
			ground = prev;

			edgeQuantity = length( ground->v1 - ground->v0 );
		}
	}

	groundSpeed = speed;
}

bool SurfaceMover::ResolvePhysics( V2d &vel )
{
	//possibleEdgeCount = 0;
	
	Rect<double> oldR( physBody.globalPosition.x + physBody.offset.x - physBody.rw, 
		physBody.globalPosition.y + physBody.offset.y - physBody.rh, 2 * physBody.rw, 2 * physBody.rh );

	physBody.globalPosition += vel;
	
	Rect<double> newR( physBody.globalPosition.x + physBody.offset.x - physBody.rw, 
		physBody.globalPosition.y + physBody.offset.y - physBody.rh, 2 * physBody.rw, 2 * physBody.rh );
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

	//queryMode = "resolve";
	owner->terrainTree->Query( this, r );
	//Query( this, owner->testTree, r );

	return col;
}

//return true if you should break out of the loop
bool SurfaceMover::MoveAlongEdge( double &movement, double &groundLength, double &q, double &m )
{
	//cout << "moving along edge" << endl;
	double extra = 0;
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
			HitTerrain( q );	
			return true;
		}			
	}
	else
	{
		if( groundSpeed > 0 )
		{
			//cout << "t33" << endl;
			ground = ground->edge1;
			q = 0;
		}
		else
		{
			//cout << "here>? " << endl;
			ground = ground->edge0;
			q = length( ground->edge0->v1 - ground->edge0->v0 );
		}
				
	}

	return false;
}

void SurfaceMover::HandleEntrant( QuadTreeEntrant *qte )
{
	//if( queryMode == "resolve" )
	{
		Edge *e = (Edge*)qte;


		if( ground == e )
			return;

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
					return;
				}

				
			}
			else if( ground == g->edgeB )
			{
				if( e == edgeA->edge0 
					|| e == edgeA->edge1
					|| e == edgeA )
				{
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

		Contact *c = owner->coll.collideEdge( physBody.globalPosition + physBody.offset, physBody, e, tempVel, V2d( 0, 0 ) );

		if( c != NULL )
		{
			double len0 = length( c->position - e->v0 );
			double len1 = length( c->position - e->v1 );

			if( e->edge0->edgeType == Edge::CLOSED_GATE && len0 < 1 )
			{
				V2d pVec = normalize( physBody.globalPosition - e->v0 );
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
				V2d pVec = normalize( physBody.globalPosition - e->v1 );
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

			if( !col || (minContact.collisionPriority < 0 ) || (c->collisionPriority <= minContact.collisionPriority && c->collisionPriority >= 0 ) ) //(c->collisionPriority >= -.00001 && ( c->collisionPriority <= minContact.collisionPriority || minContact.collisionPriority < -.00001 ) ) )
			{	
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
}

bool SurfaceMover::RollClockwise( double &q, double &m )
{
	V2d gNormal = ground->Normal();
	V2d e1n = ground->edge1->Normal();

	double angle = m /  physBody.rw;
	V2d currVec = physBody.globalPosition - ground->v1;
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

	double rollStart = atan2( gNormal.y, gNormal.x );
	V2d startVec = V2d( cos( rollStart ), sin( rollStart ) );
	double rollEnd = atan2( e1n.y, e1n.x );

	if( rollStart < 0 )
		rollStart += 2 * PI;
	if( rollEnd < 0 )
		rollEnd += 2 * PI;

	bool changed = false;
	if( rollEnd > rollStart && ( rollNew > rollEnd || rollNew < rollStart ) )
	{
		changed = true;
		newPos = ground->v1 + e1n * physBody.rw;
	}
	else if( rollEnd < rollStart && ( rollNew > rollEnd && rollNew < rollStart ) )
	{
		changed = true;
		newPos = ground->v1 + e1n * physBody.rw;
	}

	bool hit = ResolvePhysics( newPos - physBody.globalPosition );
	if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
	{
		HitTerrain(q);
		return true;
	}

	if( changed )
	{
		ground = ground->edge1;
		q = 0;
		roll = false;
		FinishedRoll();
	}

	return false;
}

bool SurfaceMover::RollCounterClockwise( double &q, double &m )
{
	V2d gNormal = ground->Normal();
	V2d e0n = ground->edge0->Normal();

	double angle = m / physBody.rw;
	V2d currVec = physBody.globalPosition - ground->v0;
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

	double rollStart = atan2( gNormal.y, gNormal.x );
	V2d startVec = V2d( cos( rollStart ), sin( rollStart ) );
	double rollEnd = atan2( e0n.y, e0n.x );

	if( rollStart < 0 )
		rollStart += 2 * PI;
	if( rollEnd < 0 )
		rollEnd += 2 * PI;

	bool changed = false;
	if( rollEnd < rollStart && ( rollNew < rollEnd || rollNew > rollStart ) )
	{
		//cout << "first" << endl;
		changed = true;
		newPos = ground->v0 + e0n * physBody.rw;
	}
	else if( rollEnd > rollStart && ( rollNew < rollEnd && rollNew > rollStart ) )
	{
		//cout << "second: " << rollStart << ", end: " << rollEnd << ", new: " << rollNew << endl;
		changed = true;
		newPos = ground->v0 + e0n * physBody.rw;
	}

	bool hit = ResolvePhysics( newPos - physBody.globalPosition );
	if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
	{
		HitTerrain(q);
		return true;
	}	

	if( changed )
	{
		ground = ground->edge0;
		q = length( ground->v1 - ground->v0 );
		roll = false;

		FinishedRoll();
	}

	return false;
}

void SurfaceMover::Move( int slowMultiple )
{
	if( ground != NULL )
	{
		double movement = 0;
		double maxMovement = min( physBody.rw, physBody.rh ); //circle so this might be unnecessary
		movement = groundSpeed;

		movement /= slowMultiple * NUM_STEPS;

		if( abs( movement ) < .0001 )
		{
			movement = 0;
			return;
		}

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
			
				if( gNormal == e1n )
				{
					//cout << "t1" << endl;
					q = 0;
					ground = e1;
				}
				else if( !roll )
				{
					bool br = StartRoll();
					if( br )
						break;
					//callback for starting to roll
				}
				else
				{
					bool br = RollClockwise( q, m );
					if( br )
					{
						edgeQuantity = q;
						break;
					}
				}
			}
			else if( movement < 0 && q == 0 )
			{
				double d = dot( e1n, gNormal );

				if( gNormal == e0n )
				{
					q = length( e0->v1 - e0->v0 );
					ground = e0;
				}
				else if( !roll )
				{
					bool br = StartRoll();
					if( br )
						break;
				}
				else
				{
					bool br = RollCounterClockwise( q, m );
					if( br )
					{
						edgeQuantity = q;
						break;
					}
				}
			}
			else
			{
				bool br = MoveAlongEdge( movement, groundLength, q, m );
				if( br )
				{
					edgeQuantity = q;
					break;
				}
			}

			if( movement == extra )
				movement += steal;
			else
				movement = steal;

			edgeQuantity = q;

			if( abs( movement ) < .0001 )
			{
				movement = 0;
			}
		}

		if( ground != NULL )
		{
			UpdateGroundPos();
		}
	}
	else 
	{
		velocity += gravity / NUM_STEPS / (double)slowMultiple;
		//cout << "move through the air" << endl;

		V2d movementVec = velocity;
		movementVec /= slowMultiple * NUM_STEPS;

		bool hit = ResolvePhysics( movementVec );
		if( hit )
		{
			//cout << "landing aerial" << endl;
			HitTerrainAerial();

			
		}
		//else
		//{
		//	testMover->physBody.globalPosition = position;
		//}
	}
	


	

	//PhysicsResponse();
}

void SurfaceMover::HitTerrainAerial()
{
	bool corner = false;
	V2d en = minContact.normal;
	if( en.x == 0 && en.y == 0 )
	{
		corner = true;
		en = normalize( physBody.globalPosition - minContact.position );
	}

	if( corner )
	{
		roll = true;
		physBody.globalPosition += minContact.resolution;	
		ground = minContact.edge;

		if( minContact.position == ground->v0 )
		{
			edgeQuantity = 0;
		}
		else
		{
			edgeQuantity = length( ground->v1 - ground->v0 );
		}
	}
	else
	{
		ground = minContact.edge;
		edgeQuantity = ground->GetQuantity( minContact.position );
		UpdateGroundPos();
	}
}

void SurfaceMover::HitTerrain( double &q )
{
	//V2d eNorm = minContact.edge->Normal();
	if( roll )
	{
		ground = minContact.edge;
		if( minContact.normal.x == 0 && minContact.normal.y == 0 )
		{
			q = ground->GetQuantity( minContact.position );
			physBody.globalPosition += minContact.resolution;
		}
		else
		{
			roll = false;
			q = ground->GetQuantity( minContact.position + minContact.resolution );
		}
		
	}
	else
	{
		ground = minContact.edge;
		if( minContact.normal.x == 0 && minContact.normal.y == 0 )
		{
			roll = true;
			q = ground->GetQuantity( minContact.position );
			physBody.globalPosition += minContact.resolution;
		}
		else
		{
			q = ground->GetQuantity( minContact.position + minContact.resolution );
		}
	}
}

void SurfaceMover::Jump( V2d &vel )
{
	velocity = vel;
	ground = NULL;
}

bool SurfaceMover::StartRoll()
{
	roll = true;
	return false;
}

void SurfaceMover::FinishedRoll()
{
	//nothing in the default
}

GroundMover::GroundMover( GameSession *owner, Edge *startGround, double startQuantity, 
	double radius, bool p_steeps, GroundMoverHandler *p_handler )
	:SurfaceMover( owner, startGround, startQuantity, radius ), steeps( p_steeps )
	,handler( p_handler )
{

}

void GroundMover::HitTerrain( double &q )
{
	V2d en = minContact.normal;
	bool corner = false;
	if( en.x == 0 && en.y == 0 )
	{
		corner = true;
		en = normalize( physBody.globalPosition - minContact.position );
	}


	if( en.y < 0 && (owner->IsFlatGround( en ) >= 0 || owner->IsSlopedGround( en ) >= 0 
		|| ( steeps && owner->IsSteepGround( en ) >= 0 ) ) )
	{
		ground = minContact.edge;
		if( corner )
		{
			roll = true;
			q = ground->GetQuantity( minContact.position );
			physBody.globalPosition += minContact.resolution;
		}
		else
		{
			//cout << "this transition" << endl;
			roll = false;
			q = ground->GetQuantity( minContact.position + minContact.resolution );
		}
	}
	else
	{
		//might need some extra code for nonsteeps that run into steeps
		//cout << "this OTHER transition" << endl;
		physBody.globalPosition += minContact.resolution;
		q = ground->GetQuantity( physBody.globalPosition );
		if( handler != NULL )
			handler->HitOther();
	}
}

void GroundMover::HitTerrainAerial()
{
	bool corner = false;
	V2d en = minContact.normal;
	if( en.x == 0 && en.y == 0 )
	{
		corner = true;
		en = normalize( physBody.globalPosition - minContact.position );
	}

	if( en.y < 0 && (owner->IsFlatGround( en ) >= 0 || owner->IsSlopedGround( en ) >= 0 
		|| ( steeps && owner->IsSteepGround( en ) >= 0 ) ) )
	{
		ground = minContact.edge;
		if( corner )
		{
			roll = true;
			edgeQuantity = ground->GetQuantity( minContact.position );
			physBody.globalPosition += minContact.resolution;
		}
		else
		{
			roll = false;
			edgeQuantity = ground->GetQuantity( minContact.position + minContact.resolution );
		}
		if( handler != NULL )
			handler->Land();
	}
	else
	{
		//cout << "collision vel: " << velocity.x << ", " << velocity.y << endl;
		physBody.globalPosition += minContact.resolution;
		velocity = dot( velocity, V2d( -en.y, en.x ) ) * V2d( -en.y, en.x );
		if( handler != NULL )
			handler->HitOtherAerial();
		//cout << "vel: " << velocity.x << ", " << velocity.y << endl;
		//q = ground->GetQuantity( physBody.globalPosition );
		//if( handler != NULL )
		//	handler->HitOther();
	}
}

bool GroundMover::StartRoll()
{
	V2d en;
	if( groundSpeed > 0 )
	{
		en = ground->edge1->Normal();
	}
	else
	{
		en = ground->edge0->Normal();
	}

	if( en.y < 0 && (owner->IsFlatGround( en ) >= 0 || owner->IsSlopedGround( en ) >= 0 
		|| ( steeps && owner->IsSteepGround( en ) >= 0 ) ) )
	{
		roll = true;
		return false;
	}
	else
	{
		if( handler != NULL )
		{
			handler->ReachCliff();
		}
		return true;
	}
}

void GroundMover::FinishedRoll()
{
}

