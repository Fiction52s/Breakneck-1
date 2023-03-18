#include "Mover.h"
#include "GameSession.h"
#include <iostream>
#include "EditorTerrain.h"

#define TIMESTEP (1.0 / 60.0)

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

SurfaceMover::SurfaceMover()
{
	sess = Session::GetSession();

	surfaceHandler = NULL;
	surfaceMoverData.collisionOn = true;
	surfaceMoverData.physBody.isCircle = true;
	surfaceMoverData.physBody.rw = 0;
	surfaceMoverData.physBody.rh = 0;
	surfaceMoverData.physBody.offset = V2d(0, 0);
	surfaceMoverData.force = V2d(0, 0);
	ground = NULL;
	surfaceMoverData.edgeQuantity = 0;
	surfaceMoverData.roll = false;
	surfaceMoverData.groundSpeed = 0;
	UpdateGroundPos();
}

SurfaceMover::SurfaceMover(Edge *startGround, 
	double startQuantity, double radius )// double mSpeed )
	:ground( startGround )
{
	sess = Session::GetSession();

	surfaceMoverData.groundSpeed = 0;
	surfaceMoverData.edgeQuantity = startQuantity;
	surfaceMoverData.roll = false;

	surfaceHandler = NULL;
	surfaceMoverData.collisionOn = true;
	//maxSpeed = mSpeed;
	//gravity = V2d( 0, 0 );
	surfaceMoverData.physBody.isCircle = true;
	surfaceMoverData.physBody.rw = radius;
	surfaceMoverData.physBody.rh = radius;
	surfaceMoverData.physBody.offset = V2d( 0, 0 );
	surfaceMoverData.force = V2d(0, 0);
	UpdateGroundPos();
}

void SurfaceMover::Set(PositionInfo &pi)
{
	ground = pi.GetEdge();

	if (ground == NULL)
	{
		surfaceMoverData.physBody.globalPosition = pi.GetPosition();
		surfaceMoverData.edgeQuantity = 0;
	}
	else
	{
		surfaceMoverData.edgeQuantity = pi.GetQuant();
	}
	
	surfaceMoverData.roll = false;
	UpdateGroundPos();
}

void SurfaceMover::AddAirForce(V2d &p_force)
{
	surfaceMoverData.force += p_force;
}
void SurfaceMover::ClearAirForces()
{
	surfaceMoverData.force = V2d(0, 0);
}

V2d SurfaceMover::GetGroundPoint()
{
	//assert(ground != NULL);
	if (ground != NULL)
	{
		return ground->GetPosition(surfaceMoverData.edgeQuantity);
	}
	else
	{
		return V2d(0, 0);
	}
}

Vector2f SurfaceMover::GetGroundPointF()
{
	return Vector2f(GetGroundPoint());
}

double SurfaceMover::GetAngleRadians()
{
	double angle;
	if (ground != NULL)
	{
		if (!surfaceMoverData.roll)
		{
			angle = ground->GetNormalAngleRadians();
		}
		else
		{
			if (surfaceMoverData.edgeQuantity == 0)
			{
				V2d vec = normalize(surfaceMoverData.physBody.globalPosition - ground->v0);
				angle = atan2(vec.y, vec.x);
				angle += PI / 2.0;
			}
			else
			{
				V2d vec = normalize(surfaceMoverData.physBody.globalPosition - ground->v1);
				angle = atan2(vec.y, vec.x);
				angle += PI / 2.0;
			}
			
		}
	}
	else
	{
		angle = 0;
	}

	return angle;
}
double SurfaceMover::GetAngleDegrees()
{
	return GetAngleRadians() / PI * 180.0;
}


void SurfaceMover::UpdateGroundPos()
{
	if( ground == NULL )
	{
		surfaceMoverData.physBody.globalAngle = 0;
		return;
	}
	

	if(surfaceMoverData.roll )
	{
		V2d n;
		if(surfaceMoverData.edgeQuantity < .01 )
		{
			//basically 0
			n = normalize(surfaceMoverData.physBody.globalPosition - ground->v0 );
		}
		else
		{
			n = normalize(surfaceMoverData.physBody.globalPosition - ground->v1 );
		}
		surfaceMoverData.physBody.globalAngle = atan2( n.x, -n.y );
	}
	else
	{
		V2d gn = ground->Normal();
		surfaceMoverData.physBody.globalPosition = ground->GetPosition(surfaceMoverData.edgeQuantity )
			+ gn * surfaceMoverData.physBody.rw;
		surfaceMoverData.physBody.globalAngle = atan2( gn.x, -gn.y );
		//cout << "setting grounded position to: " << physBody.globalPosition.x 
		//	<< ", " << physBody.globalPosition.y << endl;
	}
	
}

void SurfaceMover::SetSpeed( double speed )
{
	if(surfaceMoverData.roll )
	{
		if(surfaceMoverData.groundSpeed > 0 && speed < 0 )
		{
			Edge *next = ground->edge1;
			ground = next;

			surfaceMoverData.edgeQuantity = 0;
		}
		else if(surfaceMoverData.groundSpeed < 0 && speed > 0 )
		{
			Edge *prev = ground->edge0;
			ground = prev;

			surfaceMoverData.edgeQuantity = length( ground->v1 - ground->v0 );
		}
	}

	surfaceMoverData.groundSpeed = speed;
}

bool SurfaceMover::ResolvePhysics( V2d &vel )
{
	//possibleEdgeCount = 0;
	
	Rect<double> oldR(surfaceMoverData.physBody.globalPosition.x + surfaceMoverData.physBody.offset.x - surfaceMoverData.physBody.rw,
		surfaceMoverData.physBody.globalPosition.y + surfaceMoverData.physBody.offset.y - surfaceMoverData.physBody.rh, 2 * surfaceMoverData.physBody.rw, 2 * surfaceMoverData.physBody.rh );

	surfaceMoverData.physBody.globalPosition += vel;
	
	Rect<double> newR(surfaceMoverData.physBody.globalPosition.x + surfaceMoverData.physBody.offset.x - surfaceMoverData.physBody.rw,
		surfaceMoverData.physBody.globalPosition.y + surfaceMoverData.physBody.offset.y - surfaceMoverData.physBody.rh, 2 * surfaceMoverData.physBody.rw, 2 * surfaceMoverData.physBody.rh );
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

	if (surfaceHandler != NULL)
		surfaceHandler->ExtraQueries(r);

	//queryMode = "resolve";
	if (surfaceMoverData.collisionOn)
	{
		sess->terrainTree->Query(this, r);
	}
	

	
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
			//if (surfaceHandler != NULL)
			//	surfaceHandler->TransferEdge(ground);
			return true;
			//return false;
		}			
	}
	else
	{
		if(surfaceMoverData.groundSpeed > 0 )
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

		if( surfaceHandler != NULL )
			surfaceHandler->TransferEdge( ground );
				
	}

	return false;
}

void SurfaceMover::SetVelocity(const V2d &vel)
{
	surfaceMoverData.velocity = vel;
}

void SurfaceMover::HandleEntrant( QuadTreeEntrant *qte )
{
	//if( queryMode == "resolve" )
	{
		Edge *e = (Edge*)qte;

		if (e->edgeType == Edge::OPEN_GATE)
		{
			Gate *g = (Gate*)e->info;
			if (g->category != Gate::SECRET)
			{
				return;
			}
		}
			


		//cout << "edge: " << e->Normal().x << ", " << e->Normal().y << endl;

		if (ground == e || e->IsInvisibleWall()  )
			return;



		if( ground != NULL && ground->IsClosedGateEdge())
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
			else
			{
				assert(0);
			}
		}
		if( ground != NULL )
		{
			if( ground->edge0->IsClosedGateEdge())
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
				else
				{
					assert(0);
				}
			}
			
			
			if( ground->edge1->IsClosedGateEdge())
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
				else
				{
					assert(0);
				}
			}
		}

		Contact *c = sess->collider.collideEdge( surfaceMoverData.physBody.globalPosition + surfaceMoverData.physBody.offset, surfaceMoverData.physBody, e, tempVel, V2d( 0, 0 ) );


		if( c != NULL )
		{
			//cout << "c is not null!: " << e->Normal().x << ", " << e->Normal().y << endl;
			double len0 = length( c->position - e->v0 );
			double len1 = length( c->position - e->v1 );

			if( e->edge0->edgeType == Edge::CLOSED_GATE && len0 < 1 )
			{
				V2d pVec = normalize(surfaceMoverData.physBody.globalPosition - e->v0 );
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
				V2d pVec = normalize(surfaceMoverData.physBody.globalPosition - e->v1 );
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
						/*cout << "replacing collision: " << c->collisionPriority <<
						", " << c->edge->Normal().x << ", " << c->edge->Normal().y
						<< ", " << endl;*/
						minContact.collisionPriority = c->collisionPriority;
						minContact.edge = e;
						minContact.resolution = c->resolution;
						minContact.position = c->position;
						minContact.normal = c->normal;
						//cout << "replacing res: " << c->resolution.x << ", " << c->resolution.y << endl;
						col = true;
					}
				}
				else
				{
					/*cout << "setting collision: " << c->collisionPriority <<
						", " << c->edge->Normal().x << ", " << c->edge->Normal().y
						<< ", " << c->position.x << ", " << c->position.y << endl;*/
						
					//why is res negative???

					//cout << "res: " << c->resolution.x << ", " << c->resolution.y << endl;
					minContact.collisionPriority = c->collisionPriority;
					minContact.edge = e;
					minContact.resolution = c->resolution;
					minContact.position = c->position;
					minContact.normal = c->normal;
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

	double angle = m / surfaceMoverData.physBody.rw;
	V2d currVec = surfaceMoverData.physBody.globalPosition - ground->v1;
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

	//cout << "rollStart: " << rollStart << ", rollend: " << rollEnd << endl;

	bool changed = false;
	if( rollEnd > rollStart && ( rollNew > rollEnd || rollNew < rollStart ) )
	{
		changed = true;
		newPos = ground->v1 + e1n * surfaceMoverData.physBody.rw;
		if (rollNew > rollEnd)
		{
			double diff = (rollNew - rollEnd) * surfaceMoverData.physBody.rw;
			steal = diff;
		}
		else if (rollNew < rollStart)
		{
			//need to implement this!!!!

			int xxxxx = 6;
		}
	}
	else if( rollEnd < rollStart && ( rollNew > rollEnd && rollNew < rollStart ) )
	{
		changed = true;
		newPos = ground->v1 + e1n * surfaceMoverData.physBody.rw;

		//implement this when it comes up too
		int xxxxxx = 6;
	}

	bool hit = ResolvePhysics( newPos - surfaceMoverData.physBody.globalPosition );
	if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
	{
		HitTerrain(q);
		return true;
	}

	if( changed )
	{
		//cout << "changed" << endl;
		ground = ground->edge1;
		q = 0;
		roll = false;
		FinishedRoll();
		if( surfaceHandler != NULL )
				surfaceHandler->TransferEdge( ground );
		//return true;
	}

	return false;
}

bool SurfaceMover::RollCounterClockwise( double &q, double &m )
{
	V2d gNormal = ground->Normal();
	V2d e0n = ground->edge0->Normal();

	double angle = m / surfaceMoverData.physBody.rw;
	V2d currVec = surfaceMoverData.physBody.globalPosition - ground->v0;
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
		newPos = ground->v0 + e0n * surfaceMoverData.physBody.rw;

		if (rollNew < rollEnd)
		{
			double diff = (rollNew - rollEnd) *  surfaceMoverData.physBody.rw;
			steal = diff;
		}
		else
		{
			//implemenet
			int xxxxx = 6;
		}
	}
	else if( rollEnd > rollStart && ( rollNew < rollEnd && rollNew > rollStart ) )
	{
		//cout << "second: " << rollStart << ", end: " << rollEnd << ", new: " << rollNew << endl;
		changed = true;
		newPos = ground->v0 + e0n * surfaceMoverData.physBody.rw;

		//implement
		int xxxx = 6;
	}

	bool hit = ResolvePhysics( newPos - surfaceMoverData.physBody.globalPosition );
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
		if( surfaceHandler != NULL )
				surfaceHandler->TransferEdge( ground );
	}

	return false;
}

void SurfaceMover::Move( int slowMultiple, int numPhysSteps )
{
	if( ground != NULL )
	{
		double movement = 0;
		double maxMovement = min(surfaceMoverData.physBody.rw, surfaceMoverData.physBody.rh ); //circle so this might be unnecessary
		movement = surfaceMoverData.groundSpeed;

		movement /= slowMultiple * numPhysSteps;

		if( abs( movement ) < .0001 )
		{
			movement = 0;
			return;
		}

		while( movement != 0 )
		{

			//ground is always some value
			steal = 0;
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

			if( movement > 0 && roll && q == 0 )
			{
				ground = ground->edge0;
				groundLength = length( ground->v1 - ground->v0 );
				edgeQuantity = groundLength;
				q = edgeQuantity;
				gNormal = ground->Normal();
				//cout << "Stuff" << endl;
			}
			else if( movement < 0 && roll && q == groundLength )
			{
				//cout << "A" << endl;
				ground = ground->edge1;
				groundLength = length( ground->v1 - ground->v0 );
				edgeQuantity = 0;
				q = edgeQuantity;
				gNormal = ground->Normal();
			}
			

			Edge *e0 = ground->edge0;
			Edge *e1 = ground->edge1;
			V2d e0n = e0->Normal();
			V2d e1n = e1->Normal();

			bool transferLeft = false;
			bool transferRight = false;

			
			
			if( movement > 0 && q == groundLength )
			{
				//cout << "transfer right" << endl;
				double c = cross( e1n, gNormal );
				double d = dot( e1n, gNormal );
			
				if( gNormal == e1n )
				{
					//cout << "transfer clockwise" << endl;
					//cout << "t1" << endl;
					q = 0;
					ground = e1;

					if( surfaceHandler != NULL )
						surfaceHandler->TransferEdge( ground );
				}
				else
				{
					if (!roll)
					{
						bool br = StartRoll();
						if (br)
							break;
					}

					//cout << "roll clockwise" << endl;
					bool br = RollClockwise( q, m );
					if( br )
					{
						//cout << "blah" << endl;
						edgeQuantity = q;
						break;
					}
					else
					{
						//cout << "keep going" << endl;
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

					if( surfaceHandler != NULL )
						surfaceHandler->TransferEdge( ground );
				}
				else
				{
					if (!roll)
					{
						//cout << "what start roll" << endl;
						bool br = StartRoll();
						if (br)
							break;
					}
					
					bool br = RollCounterClockwise(q, m);
					if (br)
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
		double nSteps = numPhysSteps;
		surfaceMoverData.velocity += surfaceMoverData.force / nSteps / (double)slowMultiple;
		//cout << "move through the air" << endl;

		V2d movementVec = surfaceMoverData.velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		//cout << "before moving air position: " << physBody.globalPosition.x 
		//	<< ", " << physBody.globalPosition.y << endl;


		bool hit = ResolvePhysics( movementVec );
		if( hit )
		{
			//cout << "landing aerial" << endl;
			HitTerrainAerial();
		}
	}
	


	
	surfaceMoverData.framesInAir++;
	//PhysicsResponse();
}

void SurfaceMover::HitTerrainAerial()
{
	bool corner = false;
	V2d en = minContact.normal;
	if( en.x == 0 && en.y == 0 )
	{
		corner = true;
		en = normalize(surfaceMoverData.physBody.globalPosition - minContact.position );
	}

	if( corner )
	{
		//cout << "cornering" << endl;
		surfaceMoverData.roll = true;
		surfaceMoverData.physBody.globalPosition += minContact.resolution;
		ground = minContact.edge;

		if( minContact.position == ground->v0 )
		{
			//ground = ground->edge0;
			surfaceMoverData.edgeQuantity = 0;//length( ground->v1 - ground->v0 );
		}
		else
		{
			surfaceMoverData.edgeQuantity = length( ground->v1 - ground->v0 );
		}
	}
	else
	{
		ground = minContact.edge;
		surfaceMoverData.edgeQuantity = ground->GetQuantity( minContact.position );
		UpdateGroundPos();
	}

	if( surfaceHandler != NULL )
	{
		surfaceHandler->HitTerrainAerial( ground, surfaceMoverData.edgeQuantity );
	}
		//surfaceHandler->TransferEdge( ground );
}

void SurfaceMover::HitTerrain( double &q )
{
	//V2d eNorm = minContact.edge->Normal();
	if(surfaceMoverData.roll )
	{
		ground = minContact.edge;
		if( minContact.normal.x == 0 && minContact.normal.y == 0 )
		{
			q = ground->GetQuantity( minContact.position );
			surfaceMoverData.physBody.globalPosition += minContact.resolution;
		}
		else
		{
			surfaceMoverData.roll = false;
			q = ground->GetQuantity( minContact.position + minContact.resolution );
		}
		
	}
	else
	{
		ground = minContact.edge;
		if( minContact.normal.x == 0 && minContact.normal.y == 0 )
		{
			surfaceMoverData.roll = true;
			q = ground->GetQuantity( minContact.position );
			surfaceMoverData.physBody.globalPosition += minContact.resolution;
		}
		else
		{
			q = ground->GetQuantity( minContact.position + minContact.resolution );
		}
	}

	if( surfaceHandler != NULL )
		surfaceHandler->TransferEdge( ground );
}

void SurfaceMover::Jump( V2d &vel )
{
	//cout << "jumping vel is: " << vel.x << ", " <<
		//vel.y << endl;
	surfaceMoverData.framesInAir = 0;
	//cout << "jumping surface mover: " << vel.x << ", " << vel.y << endl;
	surfaceMoverData.velocity = vel;
	ground = NULL;
}

bool SurfaceMover::StartRoll()
{
	surfaceMoverData.roll = true;
	return false;
}

void SurfaceMover::FinishedRoll()
{
	//nothing in the default
}

void SurfaceMover::SetHandler(SurfaceMoverHandler *h)
{
	surfaceHandler = h;
}

void SurfaceMover::PopulateFromData(const SurfaceMoverData &sfm)
{
	surfaceMoverData = sfm;
	ground = sess->GetEdge(&surfaceMoverData.groundInfo);
}

void SurfaceMover::PopulateData(SurfaceMoverData &sfm)
{
	surfaceMoverData.groundInfo.SetFromEdge(ground);
	sfm = surfaceMoverData;
}

int SurfaceMover::GetNumStoredBytes()
{
	return sizeof(surfaceMoverData);
}

void SurfaceMover::StoreBytes(unsigned char *bytes)
{
	surfaceMoverData.groundInfo.SetFromEdge(ground);
	memcpy(bytes, &surfaceMoverData, sizeof(surfaceMoverData));
}

void SurfaceMover::SetFromBytes(unsigned char *bytes)
{
	memcpy(&surfaceMoverData, bytes, sizeof(surfaceMoverData));
	ground = sess->GetEdge(&surfaceMoverData.groundInfo);
}

void SurfaceMover::DebugDraw(RenderTarget *target)
{
	surfaceMoverData.physBody.DebugDraw(CollisionBox::Physics, target);
}


GroundMover::GroundMover()
	:SurfaceMover( NULL, 0, 0 )
{
	groundMoverData.steeps = false;
	handler = NULL;
	groundMoverData.reverse = false;
}

GroundMover::GroundMover( Edge *startGround, double startQuantity, 
	double radius, bool p_steeps, GroundMoverHandler *p_handler )
	:SurfaceMover( startGround, startQuantity, radius )
	,handler( p_handler )
{
	groundMoverData.reverse = false;
	groundMoverData.steeps = p_steeps;
}

bool GroundMover::IsEdgeViableGround(V2d &en)
{
	if (groundMoverData.reverse)
	{
		return en.y > 0 && !TerrainPolygon::IsWall(en) && (groundMoverData.steeps || !TerrainPolygon::IsSteepGround(en));
	}
	else
	{
		return en.y < 0 && !TerrainPolygon::IsWall(en) && (groundMoverData.steeps || !TerrainPolygon::IsSteepGround(en));
	}
}

void GroundMover::HitTerrain( double &q )
{
	V2d en = minContact.normal;
	bool corner = false;
	if( en.x == 0 && en.y == 0 )
	{
		corner = true;
		en = normalize(surfaceMoverData.physBody.globalPosition - minContact.position );
	}

	

	//if( en.y < 0 && (TerrainPolygon::IsFlatGround( en ) >= 0 || TerrainPolygon::IsSlopedGround( en ) >= 0
		//|| ( steeps && TerrainPolygon::IsSteepGround( en ) >= 0 ) ) )
	if(IsEdgeViableGround( en ))
	{
		ground = minContact.edge;
		if( corner )
		{
			surfaceMoverData.roll = true;
			q = ground->GetQuantity( minContact.position );
			surfaceMoverData.physBody.globalPosition += minContact.resolution;
		}
		else
		{
			//cout << "this transition" << endl;
			surfaceMoverData.roll = false;
			q = ground->GetQuantity( minContact.position + minContact.resolution );
		}
	}
	else
	{
		//might need some extra code for nonsteeps that run into steeps
		//cout << "this OTHER transition" << endl;
		surfaceMoverData.physBody.globalPosition += minContact.resolution;
		q = ground->GetQuantity(surfaceMoverData.physBody.globalPosition );
		if( handler != NULL )
			handler->HitOther();
	}
}

void GroundMover::HitTerrainAerial()
{
	//cout << "hit terrain aerial" << endl;
	bool corner = false;
	V2d en = minContact.normal;
	if( en.x == 0 && en.y == 0 )
	{
		//cout << "whaaat " << endl;
		corner = true;
		en = normalize(surfaceMoverData.physBody.globalPosition - minContact.position );
	}

	
	//I had this as framesInAir > 100 before. why?
	if(surfaceMoverData.framesInAir > 10 && IsEdgeViableGround( en ))
	{
		ground = minContact.edge;
		if( corner )
		{

			surfaceMoverData.roll = true;
			surfaceMoverData.edgeQuantity = ground->GetQuantity( minContact.position );
			surfaceMoverData.physBody.globalPosition += minContact.resolution;

			if (handler != NULL)
				handler->Land();

			Edge *prev = ground->GetPrevEdge();
			Edge *next = ground->GetNextEdge();
			if ( approxEquals(surfaceMoverData.edgeQuantity, 0 ) && !IsEdgeViableGround(prev->Normal()))
			{
				handler->ReachCliff();
			}
			else if (approxEquals(surfaceMoverData.edgeQuantity, ground->GetLength()) && !IsEdgeViableGround(next->Normal()))
			{
				handler->ReachCliff();
			}

			
			//cout << "corner: " << minContact.resolution.x << ", " <<
			//	", " << minContact.resolution.y << endl;
			//cout << framesInAir << " land corner: " << ground->Normal().x << ", " << ground->Normal().y << endl;
		}
		else
		{
			surfaceMoverData.roll = false;
			surfaceMoverData.edgeQuantity = ground->GetQuantity( minContact.position + minContact.resolution );
			//cout << framesInAir << " land non corner: " << ground->Normal().x << ", " << ground->Normal().y << endl;
			UpdateGroundPos();

			if (handler != NULL)
				handler->Land();
		}
		
	}
	else
	{
		//cout << "collision vel: " << velocity.x << ", " << velocity.y << ", res: " 
		//	<< minContact.resolution.x << ", " << minContact.resolution.y << endl;
		surfaceMoverData.physBody.globalPosition += minContact.resolution;
		if( corner )
		{
			if( corner )
			{
			//otherwise it swipes by the corner when it moves again
				surfaceMoverData.physBody.globalPosition += normalize( minContact.resolution ) * .1;
			}
			en = normalize(surfaceMoverData.physBody.globalPosition - minContact.position );
		}

		
		//cout << "old vel: " << velocity.x << ", " << velocity.y << endl;
		
		V2d along = V2d( -en.y, en.x );
		//cout << "along: " << along.x << ", " << along.y << endl;
		if( corner )
		{
			surfaceMoverData.velocity = dot(surfaceMoverData.velocity, along ) * along;
			//velocity = V2d( 0, 0 );
		}
		else
		{
			surfaceMoverData.velocity = dot(surfaceMoverData.velocity, along ) * along;
		}

		
		//cout << "new vel: " << velocity.x << ", " << velocity.y << endl;
		if( handler != NULL )
			handler->HitOtherAerial( minContact.edge );
		//cout << "vel: " << velocity.x << ", " << velocity.y << endl;
		//q = ground->GetQuantity( physBody.globalPosition );
		//if( handler != NULL )
		//	handler->HitOther();
	}
}

bool GroundMover::StartRoll()
{
	V2d en;
	if(surfaceMoverData.groundSpeed > 0 )
	{
		en = ground->edge1->Normal();
	}
	else
	{
		en = ground->edge0->Normal();
	}

	if( IsEdgeViableGround( en ) )
	{
		surfaceMoverData.roll = true;
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


int GroundMover::GetNumStoredBytes()
{
	return sizeof(surfaceMoverData) + sizeof(groundMoverData);
}

void GroundMover::StoreBytes(unsigned char *bytes)
{
	surfaceMoverData.groundInfo.SetFromEdge(ground);
	memcpy(bytes, &surfaceMoverData, sizeof(surfaceMoverData));

	bytes += sizeof(surfaceMoverData);

	memcpy(bytes, &groundMoverData, sizeof(groundMoverData));
}

void GroundMover::SetFromBytes(unsigned char *bytes)
{
	memcpy(&surfaceMoverData, bytes, sizeof(surfaceMoverData));
	ground = sess->GetEdge(&surfaceMoverData.groundInfo);

	bytes += sizeof(surfaceMoverData);

	memcpy(&groundMoverData, bytes, sizeof(groundMoverData));
}

void GroundMover::PopulateFromData(const SurfaceMoverData &sfm, const GroundMoverData &gmd)
{
	SurfaceMover::PopulateFromData(sfm);
	groundMoverData = gmd;
}

void GroundMover::PopulateData(SurfaceMoverData &sfm, GroundMoverData &gmd)
{
	SurfaceMover::PopulateData(sfm);
	gmd = groundMoverData;
}

SurfaceRailMover::SurfaceRailMover(Edge *startGround,
	double startQuantity,
	double radius)
	:SurfaceMover( startGround, startQuantity, radius )
{
	railMoverData.railCollisionOn = true;
	surfaceRailHandler = NULL;
}

void SurfaceRailMover::SetHandler(SurfaceRailMoverHandler *h)
{
	surfaceHandler = h;
	surfaceRailHandler = h;
}

bool SurfaceRailMover::ResolvePhysics(V2d &vel)
{
	//possibleEdgeCount = 0;

	Rect<double> oldR(surfaceMoverData.physBody.globalPosition.x + surfaceMoverData.physBody.offset.x - surfaceMoverData.physBody.rw,
		surfaceMoverData.physBody.globalPosition.y + surfaceMoverData.physBody.offset.y - surfaceMoverData.physBody.rh, 2 * surfaceMoverData.physBody.rw, 2 * surfaceMoverData.physBody.rh);

	surfaceMoverData.physBody.globalPosition += vel;

	Rect<double> newR(surfaceMoverData.physBody.globalPosition.x + surfaceMoverData.physBody.offset.x - surfaceMoverData.physBody.rw,
		surfaceMoverData.physBody.globalPosition.y + surfaceMoverData.physBody.offset.y - surfaceMoverData.physBody.rh, 2 * surfaceMoverData.physBody.rw, 2 * surfaceMoverData.physBody.rh);
	//minContact.collisionPriority = 1000000;

	double oldRight = oldR.left + oldR.width;
	double right = newR.left + newR.width;

	double oldBottom = oldR.top + oldR.height;
	double bottom = newR.top + newR.height;

	double maxRight = max(right, oldRight);
	double maxBottom = max(oldBottom, bottom);
	double minLeft = min(oldR.left, newR.left);
	double minTop = min(oldR.top, newR.top);
	//Rect<double> r( minLeft - 5 , minTop - 5, maxRight - minLeft + 5, maxBottom - minTop + 5 );

	Rect<double> r(minLeft, minTop, maxRight - minLeft, maxBottom - minTop);


	minContact.collisionPriority = 1000000;

	tempVel = vel;

	col = false;
	minContact.edge = NULL;

	if (railMoverData.railCollisionOn)
	{
		queryMode = "rail";
		sess->railEdgeTree->Query(this, r);
	}
	

	if (surfaceMoverData.collisionOn)
	{
		queryMode = "terrain";
		sess->terrainTree->Query(this, r);
	}



	//Query( this, owner->testTree, r );

	return col;
}


void SurfaceRailMover::HandleEntrant(QuadTreeEntrant *qte)
{
	if (queryMode == "terrain")
	{
		SurfaceMover::HandleEntrant(qte);
	}
	else
	{
		Edge *e = (Edge*)qte;
		TerrainRail *rail = e->rail;

		V2d pos = surfaceMoverData.physBody.globalPosition;

		//if (IsEdgeTouchingCircle(e->v0, e->v1, mover->physBody.globalPosition, mover->physBody.rw))

		V2d r;
		V2d eFocus;
		bool ceiling = false;
		V2d en = e->Normal();
		if (en.y > 0)
		{
			r = e->v0 - e->v1;
			eFocus = e->v1;
			ceiling = true;
		}
		else
		{
			r = e->v1 - e->v0;
			eFocus = e->v0;
		}

		V2d along = normalize(r);
		double lenR = length(r);
		double q = dot(pos - eFocus, along);

		double c = cross(pos - e->v0, along);
		double preC = cross((pos - tempVel) - e->v0, along);

		double alongQuantVel = dot(surfaceMoverData.velocity, along);

		bool cStuff = (c >= 0 && preC <= 0) || (c <= 0 && preC >= 0);

		if (cStuff && q >= 0 && q <= lenR)//&& alongQuantVel != 0)
		{
			V2d rn(along.y, -along.x);

			//railEdge = e;
			ground = e;

			currRail = rail;
			//prevRail = (Rail*)grindEdge->info;


			LineIntersection li;
			lineIntersection( li, pos, pos - tempVel, ground->v0, ground->v1);
			if (!li.parallel)
			{
				V2d p = li.position;
				surfaceMoverData.edgeQuantity = ground->GetQuantity(p);
				tempQuant = surfaceMoverData.edgeQuantity;

				surfaceMoverData.physBody.globalPosition = p;
			}
			else
			{
				assert(0);

				//probably just start at the beginning or end of the rail based on which is closer?
			}

			

			//if (ceiling) //ceiling rail
			//{
			//	groundSpeed = -10;
			//}
			//else
			//{
			//	groundSpeed = 10;
			//}
			//groundSpeed = -groundSpeed;
			surfaceMoverData.groundSpeed = 0;

			surfaceMoverData.collisionOn = false;
			railMoverData.railCollisionOn = false;

			if (surfaceRailHandler != NULL)
			{
				surfaceRailHandler->BoardRail();
			}

			//physBody.globalPosition = position;
			//mover->ground = railEdge;
			//mover->edgeQuantity = railQuant;
			//SetSpeed(ground);

			//action = S_RAILGRIND;
			//frame = 0;
			//velocity = along * 10.0;//V2d(0, 0);
		}
	}
}

void SurfaceRailMover::SetRailSpeed(double s)
{
	assert(ground != NULL);
	V2d gn = ground->Normal();
	bool ceiling = gn.y > 0 || ( gn.y == 0 && gn.x < 0 );
	if (currRail != NULL)
	{
		if (ceiling)
		{
			surfaceMoverData.groundSpeed = -s;
		}
		else
		{
			surfaceMoverData.groundSpeed = s;
		}
	}
}

void SurfaceRailMover::Move(int slowMultiple, int numPhysSteps)
{
	if (ground != NULL)
	{
		double movement = 0;
		double maxMovement = min(surfaceMoverData.physBody.rw, surfaceMoverData.physBody.rh); //circle so this might be unnecessary
		movement = surfaceMoverData.groundSpeed;

		movement /= slowMultiple * numPhysSteps;

		if (abs(movement) < .0001)
		{
			movement = 0;
			return;
		}

		while (movement != 0)
		{
			//ground is always some value
			steal = 0;
			if (movement > 0)
			{
				if (movement > maxMovement)
				{
					steal = movement - maxMovement;
					movement = maxMovement;
				}
			}
			else
			{
				if (movement < -maxMovement)
				{
					steal = movement + maxMovement;
					movement = -maxMovement;
				}
			}

			double extra = 0;
			bool leaveGround = false;
			tempQuant = surfaceMoverData.edgeQuantity;

			V2d gNormal = ground->Normal();


			double m = movement;

			double groundLength = length(ground->v1 - ground->v0);

			if (approxEquals(tempQuant, 0))
				tempQuant = 0;
			else if (approxEquals(tempQuant, groundLength))
				tempQuant = groundLength;

			if (movement > 0 && surfaceMoverData.roll && tempQuant == 0)
			{
				ground = ground->edge0;
				groundLength = length(ground->v1 - ground->v0);
				surfaceMoverData.edgeQuantity = groundLength;
				tempQuant = surfaceMoverData.edgeQuantity;
				gNormal = ground->Normal();
			}
			else if (movement < 0 && surfaceMoverData.roll && tempQuant == groundLength)
			{
				ground = ground->edge1;
				groundLength = length(ground->v1 - ground->v0);
				surfaceMoverData.edgeQuantity = 0;
				tempQuant = surfaceMoverData.edgeQuantity;
				gNormal = ground->Normal();
			}


			Edge *e0 = ground->edge0;
			Edge *e1 = ground->edge1;
			V2d e0n;
			V2d e1n;
			if( e0 != NULL )
				e0n = e0->Normal();
			if( e1 != NULL )
				e1n = e1->Normal();

			bool transferLeft = false;
			bool transferRight = false;



			if (movement > 0 && tempQuant == groundLength)
			{
				//cout << "transfer right" << endl;
				double c = cross(e1n, gNormal);
				double d = dot(e1n, gNormal);

				if (e1 == NULL)
				{
					assert(currRail != NULL);

					if (surfaceRailHandler == NULL || (surfaceRailHandler->CanLeaveRail()))
					{
						if (surfaceRailHandler != NULL)
						{
							surfaceRailHandler->LeaveRail();
						}

						surfaceMoverData.velocity = ground->Along() * surfaceMoverData.groundSpeed;
						ground = NULL;
						currRail = NULL;
						railMoverData.railCollisionOn = true;
						surfaceMoverData.collisionOn = true;
						surfaceMoverData.roll = false;
					}
					else
					{
						SetSpeed(0);
						movement = 0;
					}

					
					
				}
				else if (gNormal == e1n || currRail != NULL )
				{
					//cout << "transfer clockwise" << endl;
					//cout << "t1" << endl;
					tempQuant = 0;
					ground = e1;

					if (surfaceHandler != NULL)
						surfaceHandler->TransferEdge(ground);
				}
				else
				{
					if (!surfaceMoverData.roll)
					{
						bool br = StartRoll();
						if (br)
							break;
					}

					//cout << "roll clockwise" << endl;
					bool br = RollClockwise(tempQuant, m);
					if (br)
					{
						//cout << "blah" << endl;
						surfaceMoverData.edgeQuantity = tempQuant;
						break;
					}
					else
					{
						//cout << "keep going" << endl;
					}
					
				}
			}
			else if (movement < 0 && tempQuant == 0)
			{

				double d = dot(e1n, gNormal);

				if (e0 == NULL)
				{
					assert(currRail != NULL);

					if (surfaceRailHandler == NULL || (surfaceRailHandler->CanLeaveRail()))
					{
						surfaceMoverData.velocity = ground->Along() * surfaceMoverData.groundSpeed;
						ground = NULL;
						currRail = NULL;
						railMoverData.railCollisionOn = true;
						surfaceMoverData.collisionOn = true;
						surfaceMoverData.roll = false;
					}
					else
					{
						SetSpeed(0);
						movement = 0;
					}
					
					
				}
				else if (gNormal == e0n || currRail != NULL )
				{
					tempQuant = length(e0->v1 - e0->v0);
					ground = e0;

					if (surfaceHandler != NULL)
						surfaceHandler->TransferEdge(ground);
				}
				else
				{
					if (!surfaceMoverData.roll)
					{
						bool br = StartRoll();
						if (br)
							break;
					}
					
					bool br = RollCounterClockwise(tempQuant, m);
					if (br)
					{
						surfaceMoverData.edgeQuantity = tempQuant;
						break;
					}
				}
			}
			else
			{
				bool br = MoveAlongEdge(movement, groundLength, tempQuant, m);
				if (br)
				{
					surfaceMoverData.edgeQuantity = tempQuant;
					break;
				}
			}

			if (movement == extra)
				movement += steal;
			else
				movement = steal;

			surfaceMoverData.edgeQuantity = tempQuant;

			if (abs(movement) < .0001)
			{
				movement = 0;
			}
		}

		if (ground != NULL)
		{
			UpdateGroundPos();
		}
	}
	else
	{
		double nSteps = numPhysSteps;
		surfaceMoverData.velocity += surfaceMoverData.force / nSteps / (double)slowMultiple;
		//cout << "move through the air" << endl;

		V2d movementVec = surfaceMoverData.velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		bool hit = ResolvePhysics(movementVec);
		if (hit)
		{
			HitTerrainAerial();
		}
	}

	surfaceMoverData.framesInAir++;
}


int SurfaceRailMover::GetNumStoredBytes()
{
	return sizeof(surfaceMoverData) + sizeof( railMoverData );
}

void SurfaceRailMover::StoreBytes(unsigned char *bytes)
{
	surfaceMoverData.groundInfo.SetFromEdge(ground);
	memcpy(bytes, &surfaceMoverData, sizeof(surfaceMoverData));

	bytes += sizeof(surfaceMoverData);

	railMoverData.currRailID = sess->GetRailID(currRail);
	memcpy(bytes, &railMoverData, sizeof(railMoverData));
}

void SurfaceRailMover::SetFromBytes(unsigned char *bytes)
{
	memcpy(&surfaceMoverData, bytes, sizeof(surfaceMoverData));
	ground = sess->GetEdge(&surfaceMoverData.groundInfo);

	bytes += sizeof(surfaceMoverData);

	memcpy(&railMoverData, bytes, sizeof(railMoverData));

	currRail = sess->GetRailFromID(railMoverData.currRailID);
}

void SpaceMover::Reset()
{
	SetState(S_WAIT);
	currStateLength = -1;
}

void SpaceMover::SetHover(double dipPixels, int loopFrames)
{
	SetState(S_HOVER);
	hoverDipPixels = dipPixels;
	currStateLength = loopFrames;
	frame = 0;
	startPos = position;
}

void SpaceMover::StateEnded()
{
	if (frame == currStateLength || stateOver )
	{
		switch (state)
		{
		case S_HOVER:
		{
			frame = 0;
			break;
		}
		case S_STRAIGHTMOVE:
		{
			SetState(S_WAIT);
			break;
		}
		case S_CURVEDMOVE:
		{
			SetState(S_WAIT);
			velocity = targetVelocity;
			break;
		}
			
		}
	}
}

void SpaceMover::ApplyHover()
{
	int t = frame % currStateLength;
	double tf = t;
	tf /= (currStateLength - 1);
	double f = sin(2 * PI * tf);

	currentHoverOffset = f * hoverDipPixels;
	//position = startPos;
	//position.y += f * hoverDipPixels;
}

void SpaceMover::ApplyLinearMove()
{
	double len = length(targetPos - position);
	double velLen = length(velocity);
	if (len < velLen)
	{
		position = targetPos;
		velocity = V2d(0, 0);
		stateOver = true;
	}
	else
	{
		position += velocity;
		double newSpeed;
		if (accel == 0)
		{
			newSpeed = maxVel;
		}
		else
		{
			newSpeed = min(maxVel, velLen + accel);
		}

		velocity = normalize(targetPos - position) * newSpeed;
	}
}

SpaceMover::SpaceMover()
{

}

void SpaceMover::SetCurvedMove(V2d startVelocity, V2d target,
	V2d targetVel)
{
	velocity = startVelocity;
	SetCurvedMoveContinue(target, targetVel);
}

void SpaceMover::SetCurvedMoveContinue(V2d target, V2d targetVel)
{
	double tuning = 40.0;
	double tuning1 = 40.0;

	if (targetVel.x == 0 && targetVel.y == 0)
	{
		if (target.x == position.x && target.y == position.y)
		{
			SetState(S_WAIT);
			currStateLength = -1;
			frame = 0;
			return;
		}

		targetVel = normalize(target - position);
		targetVel *= curvedSpeed;
		//control0 = position + dir * tuning;
	}

	if (velocity.x == 0 && velocity.y == 0)
	{
		velocity = normalize(target - position) * 5.0;
	}

	V2d control0 = position + velocity * tuning;
	//V2d control1 = target - targetVel * tuning1;
	curvedSpeed = 10.0;
	targetVelocity = targetVel;


	curve.A = position;
	curve.B = control0;
	curve.C = target;//control1;
	//curve.D = target;

	SetState(S_CURVEDMOVE);
	currentT = 0;
	frame = 0;
	currStateLength = -1;
	
}

bool SpaceMover::Update()
{
	StateEnded();

	if (state == S_WAIT)
	{
		return false;
	}

	switch (state)
	{
	case S_HOVER:
	{
		//if( frame > 0)
		ApplyHover();
		position.y += currentHoverOffset;
		break;
	}
	case S_STRAIGHTMOVE:
	{
		ApplyLinearMove();

		ApplyHover();
		position.y += currentHoverOffset;
		break;
	}
	case S_HOVERMOVE:
	{
		break;
	}
	case S_CURVEDMOVE:
	{
		//double t;// = ((double)frame) / currStateLength;
		//v1 = -3A + 9B -  9C + 3D
		//v2 = 6A - 12B + 6C
		//v3 = -3A + 3B
		//V2d v1 = curve.A * -3.0 + curve.B * 9.0 - curve.C * 9.0 + curve.D * 3.0;
		//V2d v2 = curve.A * 6.0 - curve.B * 12.0 + curve.C * 6.0;
		//V2d v3 = curve.A * -3.0 + curve.B * 3.0;

		V2d v1 = 2.0 * curve.A - 4.0 * curve.B + 2.0 * curve.C;
		V2d v2 = -2.0 * curve.A + 2.0 * curve.B;

		double denom = length(currentT * v1 + v2);
		double step = curvedSpeed / denom;

		currentT = currentT + step;

		if (currentT >= 1.0)
		{
			currentT = 1.0;
			stateOver = true;
		}
		V2d pos = curve.GetPosition(currentT);

		cout << "currentT: " << currentT << ", pos: " << pos.x << ", " << pos.y << "\n";
		position = pos;

		if (frame == 0)
		{
			//cout << "denom:" << denom << ", step: " << step << endl;
			//if (denom == 0)
			//{
			//	int xxxx = 5;
			//}
		}
		//cout << "t: " << t << ", pos: " << position.x << ", " << position.y << endl;
	}
		
	}
	++frame;
	
	return (state != S_WAIT); //all movement has ceased
}

bool SpaceMover::IsIdle()
{
	return state == S_WAIT;
}

void SpaceMover::SetState(SpaceMovementState newState)
{
	state = newState;
	stateOver = false;

	if (state == S_WAIT)
	{
		currStateLength = -1;
	}
}

void SpaceMover::SetMove(V2d &target, double maxSpeed,
	double p_accel, int dFrames, double decelRad)
{
	targetPos = target;
	velocity = V2d(0, 0);
	SetState(S_STRAIGHTMOVE);
	accel = p_accel;
	decelRadius = decelRad;
	decelFrames = dFrames;
	maxVel = maxSpeed;
	currStateLength = -1;
}

void SpaceMover::DebugDraw(sf::RenderTarget *target)
{
	if (state == S_CURVEDMOVE)
	{
		sf::CircleShape cs;
		cs.setFillColor(Color::Red);
		cs.setRadius(20);
		cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);

		int divs = 10;
		for (int i = 0; i < divs; i++)
		{
			cs.setPosition(Vector2f(curve.GetPosition((double)i / divs)));
			target->draw(cs);
		}

		cs.setFillColor(Color::White);
		cs.setPosition(Vector2f(curve.A));
		target->draw(cs);
		cs.setPosition(Vector2f(curve.B));
		target->draw(cs);
		cs.setPosition(Vector2f(curve.C));
		target->draw(cs);
		//cs.setPosition(Vector2f(curve.D));
		//target->draw(cs);
	}
}

CubicCurve::CubicCurve() {}


CubicCurve::CubicCurve(sf::Vector2<double> &a,
	sf::Vector2<double> &b,
	sf::Vector2<double> &c,
	sf::Vector2<double> &d,
	CubicBezier &bez)
	:A(a), B(b), C(c), D(d)
{
}

V2d CubicCurve::GetPosition(double t)
{
	//double v = t / 
	//double v = bez.GetValue(t / (double)duration);
	double v = t;
	double rv = (1 - v);
	return pow(rv, 3) * A
		+ 3 * rv * rv * v * B
		+ 3 * rv * v * v * C
		+ pow(v, 3) * D;
}

QuadraticCurve::QuadraticCurve(sf::Vector2<double> &a,
	sf::Vector2<double> &b,
	sf::Vector2<double> &c)
	:A(a), B(b), C(c)
{
}

QuadraticCurve::QuadraticCurve() {}

V2d QuadraticCurve::GetPosition(double t)
{
	//double v = t / 
	//double v = bez.GetValue(t / (double)duration);
	double v = t;
	double rv = (1 - v);
	return pow(rv, 2) * A + 2 * rv * v * B + pow(v, 2)*C;
}