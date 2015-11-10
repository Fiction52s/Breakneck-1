#include "Physics.h"
#include "VectorMath.h"
#include <iostream>
#include <assert.h>
#include "GameSession.h"
#include "poly2tri/poly2tri.h"

using namespace sf;
using namespace std;

//EDGE FUNCTIONS
Edge::Edge()
{
	edge0 = NULL;
	edge1 = NULL;
}

Vector2f Edge::Normal()
{
	Vector2f v = v1 - v0;
	Vector2f temp = normalize( v );
	return Vector2f( temp.y, -temp.x );
}

Vector2f Edge::GetPoint( float quantity )
{
	//gets the point on a line w/ length quantity in the direction of the edge vector
	Vector2f e( v1 - v0 );
	e = normalize( e );
	return v0 + quantity * e;
}

float Edge::GetQuantity( Vector2f p )
{
	//projects the origin of the line to p onto the edge. if the point is on the edge it will just be 
	//normal to use dot product to get cos(0) =1
	Vector2f vv = p - v0;
	Vector2f e = normalize(v1 - v0);
	float result = dot( vv, e );
	float len = length( v1 - v0 );
	if( approxEquals( result, 0 ) )
		return 0;
	else if( approxEquals( result, length( v1 - v0 ) ) )
		return len;
	else
		return result;
}

float Edge::GetQuantityGivenX( float x )
{

	Vector2f e = normalize(v1 - v0);
	float deltax = x - v0.x;
	float factor = deltax / e.y;
}

//pathparam is local. pointsParam is local
MovingTerrain::MovingTerrain( GameSession *own, Vector2i pos, list<Vector2i> &pathParam, list<Vector2i> &pointsParam,
	bool loopP, float pspeed )
	:quadTree( NULL ), numEdges( 0 ), edgeArray( NULL ), loop( loopP ), speed( pspeed ), 
	position( pos.x, pos.y ), targetNode( 1 ), slowCounter( 1 ), slowMultiple( 1 ), owner( own )
{
	//cout << "pos is: " << pos.x << ", " << pos.y << endl;
	//set up path
	pathLength = pathParam.size() + 1;
	path = new Vector2i[pathLength];
	path[0] = pos;

	int index = 1;
	for( list<Vector2i>::iterator it = pathParam.begin(); it != pathParam.end(); ++it )
	{
		path[index] = (*it) + pos;
		++index;
	}


	//finalize edges and stuff
	list<Vector2i>::iterator it = pointsParam.begin();
	left = (*it).x;
	right = (*it).x;
	top = (*it).y;
	bottom = (*it).y;
	++it;

	for( ; it != pointsParam.end(); ++it )
	{
		if( (*it).x < left )
			left = (*it).x;
		if( (*it).x > right )
			right = (*it).x;
		if( (*it).y < top )
			top = (*it).y;
		if( (*it).y > bottom )
			bottom = (*it).y;
	}
	
	
	
	list<Edge*> edges;

	//could be smaller/more optimized
	quadTree = new QuadTree( right - left, bottom - top );

	list<Vector2i>::iterator last = pointsParam.end();
	--last;

	for( it = pointsParam.begin(); it != pointsParam.end(); ++it )
	{
		Edge *e = new Edge;
		e->v0 = Vector2f( (float)(*last).x, (float)(*last).y );
		e->v1 = Vector2f( (float)(*it).x, (float)(*it).y );
		edges.push_back( e );
		last = it;
	}

	//tempPoints.clear();

	//set up the quadtree and array
	numEdges = edges.size();
	edgeArray = new Edge*[numEdges];

	int i = 0;
	for( list<Edge*>::iterator eit = edges.begin(); eit != edges.end(); ++eit )
	{
		quadTree->Insert( (*eit) );
		edgeArray[i] = (*eit);
		++i;
	}

	//give the edges their links
	for( i = 0; i < numEdges; ++i )
	{
		if( i == 0 )
		{
			edgeArray[i]->edge0 = edgeArray[numEdges-1];
		}
		else
		{
			edgeArray[i]->edge0 = edgeArray[i-1];
		}

		if( i == numEdges - 1 )
		{
			edgeArray[i]->edge1 = edgeArray[0];
		}
		else
		{
			edgeArray[i]->edge1 = edgeArray[i+1];
		}
	}
	cout << "creating moving terrain with position: " << position.x << ", " << position.y << endl;

	vector<p2t::Point*> polyline;
	for( it = pointsParam.begin(); it != pointsParam.end(); ++it )
	{
		polyline.push_back( new p2t::Point( (*it).x + position.x, (*it).y + position.y ) );
	}

	p2t::CDT * cdt = new p2t::CDT( polyline );
	
	cdt->Triangulate();
	vector<p2t::Triangle*> tris;
	tris = cdt->GetTriangles();
	numTris = tris.size();
	polygonVA = new VertexArray( sf::Triangles , tris.size() * 3 );
	VertexArray & v = *polygonVA;
	Color testColor( 0x75, 0x70, 0x90 );
	for( int i = 0; i < tris.size(); ++i )
	{	
		p2t::Point *p = tris[i]->GetPoint( 0 );	
		p2t::Point *p1 = tris[i]->GetPoint( 1 );	
		p2t::Point *p2 = tris[i]->GetPoint( 2 );	
		v[i*3] = Vertex( Vector2f( p->x, p->y ), testColor );
		v[i*3 + 1] = Vertex( Vector2f( p1->x, p1->y ), testColor );
		v[i*3 + 2] = Vertex( Vector2f( p2->x, p2->y ), testColor );
	}
}

MovingTerrain::~MovingTerrain()
{
	delete quadTree;
	delete [] edgeArray;
}

void MovingTerrain::AddPoint( Vector2i p )
{
//	tempPoints.push_back( p );
}

void MovingTerrain::Finalize()
{
	
}

void MovingTerrain::Query( QuadTreeCollider *qtc, const sf::Rect<float> &r )
{
	sf::Rect<float> realR = r;
	realR.left -= position.x;
	realR.top -= position.y;
	quadTree->Query( qtc, realR );
}

void MovingTerrain::UpdatePhysics()
{
	//return;
	oldPosition = position;

	float movement = speed;
	
	/*if( PlayerSlowingMe() )
	{
		if( slowMultiple == 1 )
		{
			slowCounter = 1;
			slowMultiple = 5;
		}
	}
	else
	{
		slowMultiple = 1;
		slowCounter = 1;
	}*/

	//if( dead )
	//	return;


	movement /= (float)slowMultiple;

	while( movement != 0 )
	{
		Vector2f targetPoint = Vector2f( path[targetNode].x, path[targetNode].y );
		Vector2f diff = targetPoint - position;
		float len = length( diff );
		if( len >= abs( movement ) )
		{
			position += normalize( diff ) * movement;
			movement = 0;
		}
		else
		{
			position += diff;
			movement -= length( diff );
			AdvanceTargetNode();	
		}
	}

	vel = position - oldPosition;

	VertexArray &v = *polygonVA;
	for( int i = 0; i < numTris; ++i )
	{
		v[i*3].position += Vector2f( vel.x, vel.y );
		v[i*3+1].position += Vector2f( vel.x, vel.y );
		v[i*3+2].position += Vector2f( vel.x, vel.y );
	}
}

void MovingTerrain::AdvanceTargetNode()
{
	if( loop )
	{
		++targetNode;
		if( targetNode == pathLength )
			targetNode = 0;
	}
	else
	{
		if( forward )
		{
			++targetNode;
			if( targetNode == pathLength )
			{
				targetNode -= 2;
				forward = false;
			}
		}
		else
		{
			--targetNode;
			if( targetNode < 0 )
			{
				targetNode = 1;
				forward = true;
			}
		}
	}

//	cout << "new targetNode: " << targetNode << endl;
}

void MovingTerrain::DebugDraw( sf::RenderTarget *target )
{
	for( int i = 0; i < numEdges; ++i )
	{
		sf::CircleShape cs;
		cs.setFillColor( Color::Green );
		cs.setRadius( 20 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		Vector2f realPos = position + edgeArray[i]->v0;
		Vector2f realv1 = position + edgeArray[i]->v1;
		cs.setPosition( realPos.x, realPos.y );
		//cout << i << ": " << realPos.x << ", " << realPos.y << endl;
		target->draw( cs );

		sf::Vertex line[] =
		{
			sf::Vertex(sf::Vector2f(realPos.x, realPos.y)),
			sf::Vertex(sf::Vector2f(realv1.x, realv1.y))
		};

		target->draw(line, 2, sf::Lines);
	}


}

void MovingTerrain::Draw( RenderTarget *target )
{
	sf::Rect<float> realRect( left + position.x, top + position.y, right - left, bottom - top );
	owner->UpdateTerrainShader( realRect );
	owner->polyShader.setParameter( "topLeft", owner->view.getCenter().x - owner->view.getSize().x / 2 - ( position.x - path[0].x ),
			owner->view.getCenter().y - owner->view.getSize().y / 2 - ( position.y - path[0].y ) );
	target->draw( *polygonVA, &owner->polyShader );
}

bool CollisionBox::Intersects( CollisionBox &c )
{
	//first, box with box aabb. can adjust it later
	if( c.isCircle && this->isCircle )
	{
		float dist = length( this->globalPosition - c.globalPosition );
		//cout << "dist: " << dist << endl;
		if( dist <= this->rw + c.rw )
			return true;
	}
	else if( c.isCircle && !this->isCircle )
	{
		Vector2f pA = globalPosition + Vector2f( -rw * cos( globalAngle ) + -rh * sin( globalAngle ), -rw * -sin( globalAngle ) + -rh * cos( globalAngle ) );
		Vector2f pB = globalPosition + Vector2f( rw * cos( globalAngle ) + -rh * sin( globalAngle ), rw * -sin( globalAngle ) + -rh * cos( globalAngle ) );
		Vector2f pC = globalPosition + Vector2f( rw * cos( globalAngle ) + rh * sin( globalAngle ), rw * -sin( globalAngle ) + rh * cos( globalAngle ) );
		Vector2f pD = globalPosition + Vector2f( -rw * cos( globalAngle ) + rh * sin( globalAngle ), -rw * -sin( globalAngle ) + rh * cos( globalAngle ) );
		
		float A = cross( c.globalPosition - pA, normalize(pB - pA) );
		float B = cross( c.globalPosition - pB, normalize(pC - pB) );
		float C = cross( c.globalPosition - pC, normalize(pD - pC) );
		float D = cross( c.globalPosition - pD, normalize(pA - pD) );

		if( A <= c.rw && B <= c.rw && C <= c.rw && D <= c.rw )
		{
			return true;
		}


		return false;
	}
	else if( !c.isCircle && this->isCircle )
	{
		Vector2f pA = c.globalPosition + Vector2f( -c.rw * cos( c.globalAngle ) + -c.rh * sin( c.globalAngle ), -c.rw * -sin( c.globalAngle ) + -c.rh * cos( c.globalAngle ) );
		Vector2f pB = c.globalPosition + Vector2f( c.rw * cos( c.globalAngle ) + -c.rh * sin( c.globalAngle ), c.rw * -sin( c.globalAngle ) + -c.rh * cos( c.globalAngle ) );
		Vector2f pC = c.globalPosition + Vector2f( c.rw * cos( c.globalAngle ) + c.rh * sin( c.globalAngle ), c.rw * -sin( c.globalAngle ) + c.rh * cos( c.globalAngle ) );
		Vector2f pD = c.globalPosition + Vector2f( -c.rw * cos( c.globalAngle ) + c.rh * sin( c.globalAngle ), -c.rw * -sin( c.globalAngle ) + c.rh * cos( c.globalAngle ) );
		
		float A = cross( globalPosition - pA, normalize(pB - pA) );
		float B = cross( globalPosition - pB, normalize(pC - pB) );
		float C = cross( globalPosition - pC, normalize(pD - pC) );
		float D = cross( globalPosition - pD, normalize(pA - pD) );

		//cout << "a: " << a << ", b: " << b << ", c: " << c << ", d: " << d << ", rw: " << rw << endl;

		if( A <= rw && B <= rw && C <= rw && D <= rw )
		{
			return true;
		}


		return false;
	}
	else //both are boxes
	{
		Vector2f pA0 = globalPosition + Vector2f( -rw * cos( globalAngle ) + -rh * sin( globalAngle ), -rw * -sin( globalAngle ) + -rh * cos( globalAngle ) );
		Vector2f pB0 = globalPosition + Vector2f( rw * cos( globalAngle ) + -rh * sin( globalAngle ), rw * -sin( globalAngle ) + -rh * cos( globalAngle ) );
		Vector2f pC0 = globalPosition + Vector2f( rw * cos( globalAngle ) + rh * sin( globalAngle ), rw * -sin( globalAngle ) + rh * cos( globalAngle ) );
		Vector2f pD0 = globalPosition + Vector2f( -rw * cos( globalAngle ) + rh * sin( globalAngle ), -rw * -sin( globalAngle ) + rh * cos( globalAngle ) );

		Vector2f pA1 = c.globalPosition + Vector2f( -c.rw * cos( c.globalAngle ) + -c.rh * sin( c.globalAngle ), -c.rw * -sin( c.globalAngle ) + -c.rh * cos( c.globalAngle ) );
		Vector2f pB1 = c.globalPosition + Vector2f( c.rw * cos( c.globalAngle ) + -c.rh * sin( c.globalAngle ), c.rw * -sin( c.globalAngle ) + -c.rh * cos( c.globalAngle ) );
		Vector2f pC1 = c.globalPosition + Vector2f( c.rw * cos( c.globalAngle ) + c.rh * sin( c.globalAngle ), c.rw * -sin( c.globalAngle ) + c.rh * cos( c.globalAngle ) );
		Vector2f pD1 = c.globalPosition + Vector2f( -c.rw * cos( c.globalAngle ) + c.rh * sin( c.globalAngle ), -c.rw * -sin( c.globalAngle ) + c.rh * cos( c.globalAngle ) );

		//finish this up!

	}
	return false;
}

void CollisionBox::DebugDraw( sf::RenderTarget *target )
{
	if( isCircle )
	{
		CircleShape cs;
		//cs.setFillColor( Color( 255, 0, 0, 255 ) );

		if( type == Physics )
		{
			cs.setFillColor( Color( 255, 0, 0, 100 ) );
		}
		else if( type == Hit )
		{
			cs.setFillColor( Color( 0, 255, 0, 100 ) );
		}
		else if( type == Hurt )
		{
			cs.setFillColor( Color( 0, 0, 255, 100 ) );
		}

		cs.setRadius( rw );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( globalPosition.x, globalPosition.y );

		target->draw( cs );

	}
	else
	{
		Vector2f pos = globalPosition;
		float angle = globalAngle;
		sf::RectangleShape r;
		if( type == Physics )
		{
			r.setFillColor( Color( 255, 0, 0, 100 ) );
		}
		else if( type == Hit )
		{
			r.setFillColor( Color( 0, 255, 0, 100 ) );
		}
		else if( type == Hurt )
		{
			r.setFillColor( Color( 0, 0, 255, 100 ) );
		}
		
		r.setSize( sf::Vector2f( rw * 2, rh * 2 ) );
		r.setOrigin( r.getLocalBounds().width / 2, r.getLocalBounds().height / 2 );
		r.setRotation( angle / PI * 180 );
		
		r.setPosition( globalPosition.x, globalPosition.y );


		target->draw( r );

	}
}

//CONTACT FUNCTIONS
Contact::Contact()
	:edge( NULL ), movingPlat( NULL )
{
	collisionPriority = 0;
}

Collider::Collider()
	:currentContact(NULL)
{
	currentContact = new Contact;
}

Collider::~Collider()
{
	delete currentContact;
}

Contact *Collider::collideEdge( Vector2f position, const CollisionBox &b, Edge *e, const Vector2f &vel, const Vector2f &tVel )
{
	if( b.isCircle )
	{
		float pointMinTime = 100;

		Vector2f oldPosition = position - vel;

		Vector2f v0 = e->v0;
		Vector2f v1 = e->v1;

		float edgeLength = length( v1 - v0 );
		float radius = b.rw;
		Vector2f edgeNormal = e->Normal();

		float lineQuantity = dot( position - v0, normalize( v1 - v0 ) );
		float dist = cross( position - v0, normalize( v1 - v0 ) );
		
		if( length( v0 - position ) <= radius )
		{
			Vector2f pointDir = normalize( v0 - position );
			Vector2f velDir = normalize( vel );
			bool hit = dot( vel, pointDir ) <= length( v0 - position );
			if( hit )
			{
				currentContact->position = v0;
				currentContact->edge = e;
				currentContact->normal = Vector2f( 0, 0 );
				currentContact->collisionPriority = 0;

				CircleShape *cs = new CircleShape;
				cs->setFillColor( Color::Cyan );
				cs->setRadius( 10 );
				cs->setOrigin( cs->getLocalBounds().width / 2, cs->getLocalBounds().height / 2 );
				cs->setPosition( v0.x, v0.y );

				progressDraw.push_back( cs );

				return currentContact;
			}
		}
		if( dot( -vel, edgeNormal ) > 0 && dist >= 0 && dist <= radius )
		{
			if( lineQuantity >= 0 && lineQuantity <= edgeLength ) //point is on the circle in the dir of the ege normal
			{
				LineIntersection li = lineIntersection( oldPosition + radius * -edgeNormal, position
					+ radius * -edgeNormal, e->v0, e->v1 );


				//float testing = dot( normalize( (corner-vel) - corner), normalize( e->v1 - e->v0 ));
				if( li.parallel )//|| abs( testing ) == 1 )
				{
					cout << "returning circle null1" << endl;
					return NULL;
				}


				Vector2<float> intersect = li.position;


				//float intersectQuantity = e->GetQuantity( intersect );


				Vector2f newPosition = intersect + radius * edgeNormal;

				currentContact->resolution = newPosition - position;
				currentContact->edge = e;
				currentContact->normal= edgeNormal;
				currentContact->position = e->GetPoint( lineQuantity );
				currentContact->collisionPriority = length( intersect - ( oldPosition + radius * -edgeNormal ) );

				CircleShape *cs = new CircleShape;
				cs->setFillColor( Color::Cyan );
				cs->setRadius( 10 );
				cs->setOrigin( cs->getLocalBounds().width / 2, cs->getLocalBounds().height / 2 );
				cs->setPosition( intersect.x, intersect.y );

				progressDraw.push_back( cs );
				//cout << "success dot: " << ( dot( -vel, edgeNormal ) > 0 ) << ", dist: " << dist << ", radius: " << radius << endl;
				return currentContact;
			}
			else //special side/hit case for colliding with points
			{
			//	cout << "blah" << endl;
				//use right triangeles from the vertex to the circle point and cross product to figure out the y. then use
				//radius and the y to find the x value which is the value along the velocity that you should go until you
				//collide. thats how u get resolution here and other stuff. don't need it for this build so do it later
				//if( 
				return NULL;
			}
		}
		//cout << "dot: " << ( dot( -vel, edgeNormal ) > 0 ) << ", dist: " << dist << ", radius: " << radius << endl;
		return NULL;
	}
	else
	{
		Vector2<float> oldPosition = position - vel;
		float left = position.x - b.rw;
		float right = position.x + b.rw;
		float top = position.y - b.rh;
		float bottom = position.y + b.rh;

	

		float oldLeft = oldPosition.x - b.rw;
		float oldRight = oldPosition.x + b.rw;
		float oldTop = oldPosition.y - b.rh;
		float oldBottom = oldPosition.y + b.rh;


		float edgeLeft = min( e->v0.x, e->v1.x );
		float edgeRight = max( e->v0.x, e->v1.x ); 
		float edgeTop = min( e->v0.y, e->v1.y ); 
		float edgeBottom = max( e->v0.y, e->v1.y ); 

		Vector2f en = e->Normal();
		Vector2f prevEn = e->edge0->Normal();
		Vector2f point = e->v0;
		//Vector2f v1 = e->v1;
		//check for point collisions first

		//bool pointInRect = point.x >= left && point.x <= right && point.y >= top && point.y <= bottom;		

		//hopefully will catch any rounding errors
		float ex = .001f;
		bool pointInRect = point.x >= min( left, oldLeft ) - ex  && point.x <= max( right, oldRight ) + ex && point.y >= min( top, oldTop ) - ex && point.y <= max( bottom, oldBottom ) + ex;		


		float unPoint = cross( normalize(e->v1 - e->v0), normalize( e->edge0->v0 - e->v0 ) );
		float leftTime = 1, rightTime = 1, bottomTime = 1, topTime = 1; // one whole timestep
		
		Vector2f intersect;

		float pointMinTime = 100;
		int type = 0;
		Vector2f pointNormal(0,0);
		
		if( pointInRect && unPoint > -.0001f )
		{
			//cout << "uinpoint: " << unPoint << endl;
			bool rightCond0 = (prevEn.x < 0 && prevEn.y >= 0 && en.x < 0 && en.y <= 0);
			bool rightCond1 = ( prevEn.x >= 0 && prevEn.y > 0 && en.x <= 0 && en.y < 0 );
			bool rightCond2 = prevEn.y > 0 && en.x >= 0 && en.y < 0;//( prevEn.x < 0 && prevEn.y > 0 && en.x >= 0 && en.y < 0 );
			
			bool rightCond3 = prevEn.x < 0 || en.x < 0;
			//bool rightCond4 = prevEn.x <= 0 && prevEn.y <= 0 && en.x >= 0 && en.y <= 0;
			//bool rightCond5 = prevEn.x >= 0 && prevEn.y >= 0 && en.x <= 0 && en.y >= 0;
			//bool rightCond4 = prevEn.x < 0 && prevEn.y <= 0 && en.x >= 0;
			//bool rightCond5 = en.x < 0 && en.y >= 0 && prevEn.x >= 0;
			bool rightCond4 = prevEn.x >= 0 && en.y < 0;
			bool rightCond5 = en.x >= 0 && prevEn.y > 0;
			bool rightCond6 = prevEn.x < 0 && en.x < 0;
			


			rightCond0 = rightCond3 && ( rightCond4 || rightCond5 || rightCond6 );
			rightCond2 = rightCond1 = rightCond0;
			
			//rightCond2 = prevEn.x < 0 && abs( prevEn.y ) > -steepThresh

			//rightCond0 = rightCond3 && !rightCond4 && !rightCond5;
			//rightCond1 = rightCond0;
			//rightCond2 = rightCond1;
			//bool rightCond5 = en.x 
			//bool rightCond3 = ( prevEn.x == 0 && prevEn.y > 0 && en.x < 0 && en.y < 0 );
			
			bool leftCond0 = (prevEn.x > 0 && prevEn.y <= 0 && en.x > 0 && en.y >= 0);
			bool leftCond1 = ( prevEn.x <= 0 && prevEn.y < 0 && en.x > 0 && en.y > 0 );
			bool leftCond2 = prevEn.y < 0 && en.x <= 0 && en.y > 0;//( prevEn.x >= 0 && prevEn.y < 0 && en.x <= 0 && en.y > 0 );
			//cout << "blah: " << (prevEn.x > 0) << ", " << (prevEn.y <= 0) << ", " << (en.x < 0 )<< ", " << (en.y > 0) << endl;

			bool leftCond3 = prevEn.x > 0 || en.x > 0;
			bool leftCond4 = prevEn.x <= 0 && en.y > 0;
			bool leftCond5 = en.x <= 0 && prevEn.y < 0;
			bool leftCond6 = prevEn.x > 0 && en.x > 0;

			leftCond0 = leftCond3 && (leftCond4 || leftCond5 || leftCond6 );
			leftCond1 = leftCond2 = leftCond0;
		//	bool leftCond3 = prevEn.x > 0 || en.x > 0;
		//	bool leftCond4 = 

			bool topCond0 = (prevEn.y > 0 && prevEn.x >= 0 && en.y >= 0 && en.x <= 0);
			//bool topCond0 = (prevEn.y > 0 && en.y >= 0 && en.x <= 0);
			bool topCond1 = ( prevEn.y <= 0 && prevEn.x > 0 && en.y > 0 && en.x < 0 );
			bool topCond2 = prevEn.y > 0 && prevEn.x > 0 && en.x < 0;

			bool topCond3 = prevEn.y > 0 || en.y > 0;
			bool topCond4 = prevEn.y <= 0 && en.x < 0;
			bool topCond5 = en.y <= 0 && prevEn.x > 0;
			bool topCond6 = prevEn.y > 0 && en.y > 0;
			topCond0 = topCond3 && ( topCond4 || topCond5 || topCond6 );
			topCond1 = topCond2 = topCond0;

			//bool bottomCond0 = (prevEn.y <= 0 && prevEn.x <= 0 && en.y < 0 && en.x >= 0);
			bool bottomCond0 = (prevEn.y <= 0 && prevEn.x <= 0 && en.y < 0 && en.x >= 0);
			bool bottomCond1 = ( prevEn.y > 0 && prevEn.x < 0 && en.y <= 0 && en.x > 0 );
			//bool bottomCond2 = ( prevEn.y < 0 && prevEn.x < 0 && en.y >= 0 && en.x > 0 );
			bool bottomCond2 = ( prevEn.y < 0 && prevEn.x < 0 &&  en.x > 0 && en.y > 0 );

			bool bottomCond3 = prevEn.y < 0 || en.y < 0;
			//bool bottomCond4 = prevEn.y <= 0 && prevEn.x >= 0 && en.y >= 0 && en.x >= 0; //4 is done. finish 5 then move onto right
			//bool bottomCond5 = prevEn.y >= 0 && prevEn.x <= 0 && en.y <= 0 && en.x <= 0;

			//bool bottomCond4 = prevEn.y < 0 && prevEn.x >= 0 && en.y >= 0;
			//bool bottomCond5 = en.y < 0 && en.x <= 0 && prevEn.y >= 0;
			//bool bottomCond6 = prevEn.x == -1 && en.y == -1;
			//bool bottomCond7 = prevEn.y == -1 && en.x == 1;

			bool bottomCond4 = prevEn.y >=0 && en.x > 0;
			bool bottomCond5 = en.y >=0 && prevEn.x < 0;
			bool bottomCond6 =  ( prevEn.y <= 0 && en.y <= 0 );

			
			bottomCond0 = bottomCond3 && (bottomCond4 || bottomCond5 || bottomCond6 );
			bottomCond1 = bottomCond2 = bottomCond0;

			//temporary
			bottomCond0 = bottomCond1 = bottomCond2 = false;
			rightCond0 = rightCond1 = rightCond2 = false;
			leftCond0 =leftCond1 = leftCond2 = false;
			topCond0 = topCond1 = topCond2 = false;

			bool topPos = false, botPos = false, leftPos = false, rightPos = false;
			float aaaa =  dot (e->edge0->v0 - e->v0, normalize( e->v1 - e->v0 ) );
			if( aaaa > 0 )
			{
				bool topRight = prevEn.x < 0 && prevEn.y < 0 && en.x > 0 && en.y > 0;
				bool topLeft = prevEn.x < 0 && prevEn.y > 0 && en.x > 0 && en.y < 0;
				bool botLeft = prevEn.x > 0 && prevEn.y > 0 && en.x < 0 && en.y < 0;
				bool botRight = prevEn.x > 0 && prevEn.y < 0 && en.x < 0 && en.y > 0;

				bool topFlat = (prevEn.x == 1 && en.x < 0 && en.y > 0 ) || ( prevEn.x > 0 && prevEn.y > 0 && en.x == -1 );
				bool botFlat = ( prevEn.x < 0 && prevEn.y < 0 && en.x == 1 ) || ( prevEn.x == -1 && en.x > 0 && en.y < 0 );
				bool leftFlat = (prevEn.y == -1 && en.x > 0 && en.y > 0 ) || (prevEn.x > 0 && prevEn.y < 0 && en.y == 1 );
				bool rightFlat = ( prevEn.x < 0 && prevEn.y > 0 && en.y == -1 ) || ( prevEn.y == 1 && en.x < 0 && en.y < 0 );

				bool up = prevEn.x < 0 && prevEn.y < 0 && en.x > 0 && en.y < 0;
				bool r = prevEn.x > 0 && prevEn.y < 0 && en.x > 0 && en.y > 0;
				bool down = prevEn.x > 0 && prevEn.y > 0 && en.x < 0 && en.y > 0;
				bool l = prevEn.x < 0 && prevEn.y > 0 && en.x < 0 && en.y < 0;

				//cout << "up: " << up << ", r: " << r << ", down: " << down << ", l: "<< l << endl;
				//cout << "topleft: " << topLeft << endl;
				if( botLeft || botRight || down || topFlat )
					topCond0 = true;
				if( topLeft || l || botLeft || rightFlat )
					rightCond0 = true;
				if( topRight || up || topLeft || botFlat )
					bottomCond0 = true;
				if( botRight || r || topRight || leftFlat )
					leftCond0 = true;
			}
			else
			{
				//float bbbb = dot (e->edge0->v0 - e->v0, normalize( e->v1 - e->v0 ) );
				//the equals signs are for straight edges connected to slopes. not sure why i need to test those points but i guess it makes sense
				bool up = prevEn.x <= 0 && prevEn.y < 0 && en.x >= 0 && en.y < 0;
				bool r = prevEn.x > 0 && prevEn.y <= 0 && en.x > 0 && en.y >= 0;
				bool down = prevEn.x >= 0 && prevEn.y > 0 && en.x <= 0 && en.y > 0;
				bool l = prevEn.x < 0 && prevEn.y >= 0 && en.x < 0 && en.y <= 0;

				if( down )
					topCond0 = true;
				if( l )
					rightCond0 = true;
				if( r )
					leftCond0 = true;
				if( up )
					bottomCond0 = true;
			}

			
			//cout << "prev: " << prevEn.x << ", " << prevEn.y << ", en: " << en.x << ", " << en.y << "bottom: " << bottomCond0 << ", top: " << topCond0 << ", left: " << leftCond0 << ", right: " << rightCond0 << endl;
		//	cout << "fullbottom: " << bottomCond3 << ", " << bottomCond4 << ", " << bottomCond5 << ", " << bottomCond6 << endl;
			
			//cout << "CMON " << unPoint << ", normal: " << en.x << ", " << en.y << ", prev: " << prevEn.x << ", " << prevEn.y << endl;
			//cout << "3 : " << bottomCond0 << ", 4: " << bottomCond4 << ",5: " << bottomCond5 << ", 6: " << bottomCond6 << endl;
			//bool bottomCond3 = prevEn.y < 0 && prevEn.x > 0 && en.y < 0 && en.x > 0 );
			//bool bottomCond3 = ( prevEn.y < 0 && prevEn.x < 0 && en.y < 0 && en.x > 0 );
			
			
			//cout << "six: " << bottomCond6 << ", pre: " << prevEn.x << ", " << prevEn.y << ", en: " << en.x << ", " << en.y << endl;
			//bottomCond0 = bottomCond3 && !bottomCond4 && !bottomCond5 && !bottomCond6 && !bottomCond7;
			//bottomCond1 = bottomCond0;
			//bottomCond2 = bottomCond0;
			
			//cout << "oldLeft: " << oldLeft << ", px: " << point.x << ", left: " << left << endl;
			//cout << "leftconds: " << leftCond0 << ", " << leftCond1 << ", " << leftCond2 << endl;
			
			//cout << "vel: " << vel.x << ", " << vel.y << ", bottomconds: " << bottomCond0 <<" , " << bottomCond1 <<", " << bottomCond2 << endl;
			//cout << "prev: " << prevEn.x << ", " << prevEn.y << " n: " << en.x << ", " << en.y << endl;
			//cout << "rightcond3: " << prevEn.x << ", " << prevEn.y << ", en: " << en.x << ", " << en.y << ", cond: " << rightCond3  << endl;
			//cout << "oldright: " << oldRight << ", " << point.x << endl;
			//.f01 into 1 here
			//cout << "point: " << point.x << ", " << point.y << ", oldBottom: " << oldBottom << ", " << bottom << endl;
			if( (rightCond0 || rightCond1 || rightCond2 ) && vel.x > 0 && oldRight <= point.x + .001f && right >= point.x  )
			{
				rightTime = ( point.x - oldRight ) / abs(vel.x);
				Vector2f testRes =-vel * ( 1 - rightTime );
				if( top + testRes.y <= point.y && bottom + testRes.y >= point.y )
				{
					pointMinTime = rightTime;
					pointNormal.x = -1;
				}

			//	cout << "righttime: " << pointMinTime << endl;
				
			}
			else if( ( leftCond0 || leftCond1 || leftCond2 ) && vel.x < 0 && oldLeft >= point.x - .001f && left <= point.x  )
			{
				leftTime = ( oldLeft - point.x ) / abs( vel.x );
				Vector2f testRes =-vel * ( 1 - leftTime );
				if( top + testRes.y <= point.y && bottom + testRes.y >= point.y )
				{
					pointMinTime = leftTime;
					pointNormal.x = 1;
				}
			}
			
			if( (bottomCond0 || bottomCond1 || bottomCond2 ) && vel.y > 0 && oldBottom <= point.y + .001f && bottom >= point.y )
			{
				bool okay = true;
				//not sure what this is for. find out the purpose.
				/*if( vel.x > 0 )
				{
					if( oldLeft < edgeRight )
					{
						okay = true;
					}
				}
				else if( vel.x < 0 )
				{
					if( oldRight > edgeLeft )
					{
						okay = true;
					}
				}
				else
				{
					okay = true;
				}*/


				

				
				//cout << "right: " << right << ", px: " << point.x << ", pos: " << position.x << endl;
				
			//cout << "bottom cond okay: " << okay << endl;

				bottomTime = ( point.y - oldBottom ) / abs( vel.y );

				

				if( okay && bottomTime < pointMinTime )
				{
					Vector2f testRes =-vel * ( 1 - bottomTime );
					//doesnt include the equals stuff because left/right is preferred

					if( right + testRes.x >= point.x && left + testRes.x <= point.x )
					{
						//cout << "bottom vel: " << vel.x << ", " << vel.y << endl;
						pointMinTime = bottomTime;
						pointNormal.x = 0;
						pointNormal.y = -1;
					}
					/*if( pointMinTime == rightTime || pointMinTime == leftTime )
					{
						if( right + testRes.x > point.x && left + testRes.x < point.x )
						{
							//cout << "bottom" << endl;
							pointMinTime = bottomTime;
							pointNormal.x = 0;
							pointNormal.y = -1;
						}
					}
					else
					{
						if( right + testRes.x >= point.x && left + testRes.x <= point.x )
						{
							//cout << "bottom" << endl;
							pointMinTime = bottomTime;
							pointNormal.x = 0;
							pointNormal.y = -1;
						}
					}*/
					
				//	cout << "bottom: " << bottomCond0 << ", " << bottomCond1 << ", " << bottomCond2 << endl;
					//cout << "bottomtime: " << bottomTime << endl;
					
				}
				
				//pointMinTime = min( bottomTime, pointMinTime );
			}
			else if( (topCond0 || topCond1 || topCond2 ) && vel.y < 0 && oldTop >= point.y - .001f && top <= point.y )
			{
			//	cout << "top" << endl;
				topTime = ( oldTop - point.y ) / abs( vel.y );
				
				if( topTime < pointMinTime )
				{
					Vector2f testRes =-vel * ( 1 - topTime );
					if( right + testRes.x >= point.x && left + testRes.x <= point.x )
					{
						pointMinTime = topTime;
						pointNormal.x = 0;
						pointNormal.y = 1;
					}
				}
			}

		}

		float time = 100;
		if( en.x == 0 )
		{
			float edgeYPos = edgeTop;
			if( en.y > 0 ) //down
			{
				//cout << "vel.y: " << vel.y << ", oldtop: " << oldTop << ", edgeypos: " << edgeYPos << ", top: " << top << endl;
				if( vel.y < 0 && oldTop >= edgeYPos - .001f && top <= edgeYPos )
				{
					bool hit = true;

					bool a = left >= edgeLeft && left <= edgeRight;
					bool b = right >= edgeLeft && right <= edgeRight;
					//cout << "edge l/r: " << edgeLeft << ", " << edgeRight << ", l/r: " << left << ", " << right << endl;
					

					if( a && b )
					{
						intersect.x = (right + left ) / 2.f;
					}
					else if(a  )
					{
						intersect.x = left;
					}
					else if( b )
					{
						intersect.x = right;
					}
					else if( left <= edgeLeft && right >= edgeRight )
					{
						//cout << "blahhhhh:" << endl;
						intersect.x = (edgeLeft + edgeRight ) / 2.f;
					}
					else
					{
						hit = false;
						
					} 

					if( hit )
					{
						time = ( oldTop - edgeYPos ) / abs( vel.y );

						intersect.y = edgeYPos;
					}
					
				}
			}
			else //up
			{
				if( vel.y > 0 && oldBottom <= edgeYPos + .001f && bottom >= edgeYPos )
				{
					//cout << "this one: " << oldBottom << ", bottom: " << bottom << ", eyp: " << edgeYPos << endl;
					
					bool a = left >= edgeLeft && left <= edgeRight;
					bool b = right >= edgeLeft && right <= edgeRight;
					//cout << "edge l/r: " << edgeLeft << ", " << edgeRight << ", l/r: " << left << ", " << right << endl;
					bool hit = true;

					if( a && b )
					{
						intersect.x = (right + left ) / 2.f;
					}
					else if(a  )
					{
						intersect.x = left;
					}
					else if( b )
					{
						
						intersect.x = right;
						//cout << "only this!!: " << intersect.x << ", pos: " << position.x << endl;
					}
					else if( left <= edgeLeft && right >= edgeRight )
					{
						//cout << "blahhhhh:" << endl;
						intersect.x = (edgeLeft + edgeRight ) / 2.f;
					}
					else
					{
						hit = false;
						
					} 
					if( hit )
					{
						
						time = ( edgeYPos - oldBottom ) / abs( vel.y );

						intersect.y = edgeYPos;
					}
				}
			}

		}
		else if( en.y == 0 )
		{
			float edgeXPos = edgeLeft;
			if( en.x > 0 ) //right
			{
				//cout << "trying!: oldLeft: " << oldLeft << ", edgeXPos: " << edgeXPos <<", left: " << left << ", vel: " << vel.x << ", " << vel.y << endl;
				//cout << "blah: " << (vel.x < 0 ) << ", " << (oldLeft >= edgeXPos ) << ", " << (left <= edgeXPos ) << endl;
				if( vel.x < 0 && oldLeft >= edgeXPos - .001f && left <= edgeXPos )
				{
					bool a = top >= edgeTop && top <= edgeBottom;
					bool b = bottom >= edgeTop && bottom <= edgeBottom;
					//cout << "edge l/r: " << edgetop << ", " << edgebottom << ", l/r: " << top << ", " << bottom << endl;
					//cout << "in here: " << a << ", " << b << endl;
					bool hit = true;

					if( a && b )
					{
						intersect.y = (bottom + top ) / 2.f;
					}
					else if(a  )
					{
						intersect.y = top;
					}
					else if( b )
					{
						intersect.y = bottom;
					}
					else if( top <= edgeTop && bottom >= edgeBottom )
					{
						//cout << "blahhhhh:" << endl;
						intersect.y = (edgeTop + edgeBottom) / 2.f;
					}
					else
					{
						cout << "miss: 1 0: " << edgeTop << ", " << edgeBottom << ", l/r: " << top << ", " << bottom << endl;
						hit = false;
					} 

					if( hit )
					{
						time = ( oldLeft - edgeXPos ) / abs( vel.x);
						intersect.x = edgeXPos;
					}

				}
			}
			else //left
			{
				//cout << "attempting right: " << oldRight << ", " << edgeXPos << ", " << right << endl;
				if( vel.x > 0 && oldRight <= edgeXPos + .001f && right >= edgeXPos )
				{
					bool a = top >= edgeTop && top <= edgeBottom;
					bool b = bottom >= edgeTop && bottom <= edgeBottom;
					//cout << "edge l/r: " << edgetop << ", " << edgebottom << ", l/r: " << top << ", " << bottom << endl;
					bool hit = true;

					if( a && b )
					{
						intersect.y = (bottom + top ) / 2.f;
					}
					else if(a  )
					{
						intersect.y = top;
					}
					else if( b )
					{
						intersect.y = bottom;
					}
					else if( top <= edgeTop && bottom >= edgeBottom )
					{
						//cout << "blahhhhh:" << endl;
						intersect.y = (edgeTop + edgeBottom) / 2.f;
					}
					else
					{
						
						hit = false;
					} 

					if( hit )
					{
						time = ( edgeXPos - oldRight ) / abs( vel.x );
						intersect.x = edgeXPos;
					}
				}
				
				

			}

		
			
			
			//return NULL;
				
		}
		else
		{
			Vector2<float> corner(0,0);
			Vector2f opp;
			if( en.x > 0 )
			{
				corner.x = left;
				opp.x = right;
			}
			else if( en.x < 0 )
			{
				corner.x = right;
				opp.x = left;
			}
			
			if( en.y > 0 )
			{
				corner.y = top;
				opp.y = bottom;
			}
			else if( en.y < 0 )
			{
				corner.y = bottom;
				opp.y = top;
			}

			float res = cross( corner - e->v0, e->v1 - e->v0 );
			float oldRes = cross( (corner - vel ) - e->v0, e->v1 - e->v0 );
			float resOpp = cross( opp - e->v0, e->v1 - e->v0 );
			//might remove the opp thing soon

			float measureNormal = dot( en, normalize(-vel) );
			//cout << "oldRes : " << oldRes << endl;
			bool test = res < -.001f && resOpp > 0 && measureNormal > 0 && ( vel.x != 0 || vel.y != 0 ) ;
			//cout << "res: " << res << endl;
			//oldRes >= -.f01
			
			//if( res < -.f01 && oldRes >= -.f01 && resOpp > 0 && measureNormal > -.f01 && ( vel.x != 0 || vel.y != 0 )  )	
			if( res < -.001f && oldRes >= -.001f && resOpp > 0 && measureNormal > -.001f && ( vel.x != 0 || vel.y != 0 )  )	
			
			{
				//cout << "normal: " << e->Normal().x << ", " << e->Normal().y << endl;
				LineIntersection li = lineIntersection( corner, corner - (vel), e->v0, e->v1 );
				float testing = dot( normalize( (corner-vel) - corner), normalize( e->v1 - e->v0 ));
				if( li.parallel || abs( testing ) == 1 )
				{
					//cout << "returning null1" << endl;
					return NULL;
				}
				intersect = li.position;

				float intersectQuantity = e->GetQuantity( intersect );

				//cout << "test: " << test << " normal: " << en.x << ", " << en.y << " q: " << intersectQuantity << "len: " << length( e->v1 - e->v0 ) << endl;
				//if( intersectQuantity < 0 )
				//	intersectQuantity = 0;
				//if( intersectQuantity >length( e->v1 - e->v0 ) )
				//	intersectQuantity = length( e->v1 - e->v0 );
				float len = length( e->v1 - e->v0 );
				if( intersectQuantity < -.0001f || intersectQuantity > len + .0001f )
				{
					
					//cout << "bad: " << en.x << ", " << en.y << "  " << intersectQuantity << ", len: " << length( e->v1 - e->v0 ) << endl;
					if( intersectQuantity <= 0 )
					{
					//	point = e->v0;
					//	cout << "adjusting" << endl;
					}
					else
					{
					//	point = e->v1;
					}
				}
				else
				{
					bool okay = true;
					bool a = approxEquals( intersectQuantity, 0 );

					bool b = approxEquals( intersectQuantity, len );
					if( a || b  )
					{
						okay = false;
						float t;
						if( a )
						{
							t = cross( e->v1 - e->v0, normalize(e->edge0->v0 - e->v0) );
							intersect = e->v0;
						}
						else
						{
							t = -cross( e->edge1->v1 - e->v1, normalize(e->v1 - e->v0) ); 
							intersect = e->v1;
						}
						
						okay = true;
						if( t < -.001f )
						{	
							//okay = true;
							currentContact->weirdPoint = false;
							
						}
						else
						{
							currentContact->weirdPoint = true;
						//	cout << "weird point" << endl;
						}
					}

					if( okay )
					{
				//	cout << "using: " << intersectQuantity << ", length: " << length( e->v1 - e->v0 ) << endl;
					//this is prob wrong
					float tempTime = dot( intersect - ( corner - vel ), normalize( vel ) );
					tempTime /= length( vel );
					//cout << "tempTime: " << tempTime << endl;
					//if( tempTime >= -4 )
					{

						//if( tempTime < 0 )
						//	tempTime = 0;
						time = tempTime;
					}
						
					//		cout << "time: " << time << " normal: " << en.x << ", " << en.y << 
			//" vel: " << vel.x << ", " << vel.y << ", q: " << intersectQuantity << ", len: " << length( e->v1 - e->v0 ) << endl;
					}
						
					
						
					
				}
			}
			else
			{
				//cout << "res: " << res << ", " << oldRes << ", " << resOpp << ", " << measureNormal << endl;
			
				//cout << "baz: " << (res < 0 ) <<", " << ( oldRes >= -.f01 ) << ", " << (resOpp > 0 ) << ", " << (measureNormal > -.f01 ) << endl;
			}


		}
		//aabb's already collide

		if( pointMinTime <= time )
		{
			time = pointMinTime;
			currentContact->position = point;
			currentContact->normal = pointNormal;
			currentContact->weirdPoint = false;

			if( time == 100 )
			return NULL;

			CircleShape *cs = new CircleShape;
			cs->setFillColor( Color::Yellow );
			cs->setRadius( 5 );
			cs->setOrigin( cs->getLocalBounds().width / 2, cs->getLocalBounds().height / 2 );
			cs->setPosition( point.x, point.y );

			progressDraw.push_back( cs );	

			//cout << "point " << endl;
		}
		else
		{
			currentContact->position = intersect;
			currentContact->normal = Vector2f( 0, 0 );

			CircleShape *cs = new CircleShape;
			cs->setFillColor( Color::Yellow );
			cs->setRadius( 5 );
			cs->setOrigin( cs->getLocalBounds().width / 2, cs->getLocalBounds().height / 2 );
			cs->setPosition( intersect.x, intersect.y );

			progressDraw.push_back( cs );
		}

		if( time == 100 )
			return NULL;
		
		if( time < 0 )
			time = 0;
		if( approxEquals( time, 0 ) )
			time = 0;
		currentContact->collisionPriority = time;
		currentContact->edge = e;
		currentContact->movingPlat = NULL;
		
		currentContact->resolution = -vel * ( 1 - time );

	//	cout << "pri: " << currentContact->collisionPriority << " normal: " << en.x << ", " << en.y << 
	//		" res: " << currentContact->resolution.x << ", " << currentContact->resolution.y <<
	//		" vel: " << vel.x << ", " << vel.y << ", pos: " << currentContact->position.x << ", " << currentContact->position.y
	//		<< "old: " << oldPosition.x << ", " << oldPosition.y << endl;

//		cout << "pri: " << currentContact->collisionPriority << " normal: " << en.x << ", " << en.y << endl;
		return currentContact;

		
		
	}
}

void Collider::DebugDraw( RenderTarget *target )
{
	for( list<Drawable*>::iterator it = progressDraw.begin(); it != progressDraw.end(); ++it )
	{
		target->draw( *(*it) );
	}
	//progressDraw.clear();
}

void Collider::ClearDebug()
{
	for( list<Drawable*>::iterator it = progressDraw.begin(); it != progressDraw.end(); ++it )
	{
		delete (*it);
	}
	progressDraw.clear();
}

EdgeParentNode::EdgeParentNode( const Vector2f &poss, float rww, float rhh )
{
	pos = poss;
	rw = rww;
	rh = rhh;
	leaf = false;
	children[0] = new EdgeLeafNode( Vector2f(pos.x - rw / 2.f, pos.y - rh / 2.f), rw / 2.f, rh / 2.f );
	children[1] = new EdgeLeafNode( Vector2f(pos.x + rw / 2.f, pos.y - rh / 2.f), rw / 2.f, rh / 2.f );
	children[2] = new EdgeLeafNode( Vector2f(pos.x - rw / 2.f, pos.y + rh / 2.f), rw / 2.f, rh / 2.f );
	children[3] = new EdgeLeafNode( Vector2f(pos.x + rw / 2.f, pos.y + rh / 2.f), rw / 2.f, rh / 2.f );

	
}

EdgeLeafNode::EdgeLeafNode( const Vector2f &poss, float rww, float rhh )
	:objCount(0)
{
	pos = poss;
	rw = rww;
	rh = rhh;

	leaf = true;
	for( int i = 0; i < 4; ++i )
	{
		edges[i] = NULL;
	}
}

sf::Rect<float> GetEdgeBox( Edge *e )
{
	float left = min( e->v0.x, e->v1.x );
	float right = max( e->v0.x, e->v1.x );
	float top = min( e->v0.y, e->v1.y );
	float bottom = max( e->v0.y, e->v1.y );
	return sf::Rect<float>( left, top, right - left, bottom - top );	
}

bool IsEdgeTouchingBox( Edge *e, const sf::Rect<float> & ir )
{
	sf::Rect<float> er = GetEdgeBox( e );

	Vector2f as[4];
	Vector2f bs[4];
	as[0] = Vector2f( ir.left, ir.top );
	bs[0] = Vector2f( ir.left + ir.width, ir.top );

	as[1] =  Vector2f( ir.left, ir.top + ir.height );
	bs[1] = Vector2f( ir.left + ir.width, ir.top + ir.height );

	as[2] = Vector2f( ir.left, ir.top );
	bs[2] = Vector2f( ir.left, ir.top + ir.height);

	as[3] = Vector2f( ir.left + ir.width, ir.top );
	bs[3] = Vector2f( ir.left + ir.width, ir.top + ir.height );

	float erLeft = er.left;
	float erRight = er.left + er.width;
	float erTop = er.top;
	float erBottom = er.top + er.height;

	if( erLeft >= ir.left && erRight <= ir.left + ir.width && erTop >= ir.top && erBottom <= ir.top + ir.height )
		return true;
	//else
	//	return false;
	
	
	for( int i = 0; i < 4; ++i )
	{
		LineIntersection li = lineIntersection( as[i], bs[i], e->v0, e->v1 );

		if( !li.parallel )
		{
			
				Vector2f a = as[i];
				Vector2f b = bs[i];
				float e1Left = min( a.x, b.x );
				float e1Right = max( a.x, b.x );
				float e1Top = min( a.y, b.y );
				float e1Bottom = max( a.y, b.y );

				
			//cout << "compares: " << e1Left << ", " << erRight << " .. " << e1Right << ", " << erLeft << endl;
			//cout << "compares y: " << e1Top << " <= " << erBottom << " && " << e1Bottom << " >= " << erTop << endl;
			if( e1Left <= erRight && e1Right >= erLeft && e1Top <= erBottom && e1Bottom >= erTop )
			{
			//	cout << "---!!!!!!" << endl;
				if( (li.position.x < e1Right || approxEquals(li.position.x, e1Right) ) && ( li.position.x > e1Left || approxEquals(li.position.x, e1Left ) ) && ( li.position.y > e1Top || approxEquals( li.position.y, e1Top ) )&& ( li.position.y < e1Bottom || approxEquals( li.position.y, e1Bottom ) ) )
				{
				//	cout << "pos: " << li.position.x << ", " << li.position.y << endl;
				//	cout << "erlrud: " << erLeft << ", " << erRight << ", " << erTop << ", " << erBottom << endl;
					if( ( li.position.x < erRight || approxEquals( li.position.x, erRight )) && ( li.position.x > erLeft || approxEquals( li.position.x, erLeft ) ) && ( li.position.y > erTop || approxEquals( li.position.y, erTop ) ) && ( li.position.y < erBottom || approxEquals( li.position.y, erBottom ) ) )
					{
				//		cout << "seg intersect!!!!!!" << endl;
					//	assert( 0 );
						return true;
					}
				}
			}
		}
	}
	//cout << "return false" << endl;
	return false;
}

bool IsBoxTouchingBox( const sf::Rect<float> & r0, const sf::Rect<float> & r1 )
{
	bool test = r0.intersects( r1 );
	bool test2 =r0.left <= r1.left + r1.width 
		&& r0.left + r0.width >= r1.left 
		&& r0.top <= r1.top + r1.height
		&& r0.top + r0.height >= r1.top;
	/*if( test != test2 )
	{
		if( test )
			cout << "test is true" << endl;
		else
			cout << "test is false" << endl;
		cout << "r0: left: " << r0.left  << ", top: " << r0.top << ", w: " << r0.width << ", h: " << r0.height << endl;
		cout << "r1: left: " << r1.left  << ", top: " << r1.top << ", w: " << r1.width << ", h: " << r1.height << endl;
		cout << "wtf" << endl;
	}
	assert( test == test2 );*/
	return test2;
}

bool IsCircleTouchingCircle( Vector2f pos0, float rad_0, Vector2f pos1, float rad_1 )
{
	return length( pos1 - pos0 ) <= rad_0 + rad_1;
}

bool IsEdgeTouchingCircle( Vector2f v0, Vector2f v1, Vector2f pos, float rad )
{
	float dist = cross( pos - v0, normalize( v1 - v0 ) );
	float q = dot( pos - v0, normalize( v1 - v0 ) );
	float edgeLength = length( v1 - v0 );


	if( q < 0 )
	{
		if( length( v0 - pos ) < rad )
		{
			return true;
		}
	}
	else if( q > edgeLength )
	{
		if( length( v1 - pos ) < rad )
		{
			return true;
		}
	}
	else
	{
		if( dist < rad )
		{
			return true;
		}
	}

	return false;
}

bool IsQuadTouchingCircle( Vector2f A, Vector2f B, Vector2f C, Vector2f D, Vector2f pos, float rad )
{
	if( IsEdgeTouchingCircle( A,B, pos, rad ) 
		|| IsEdgeTouchingCircle( B,C, pos, rad ) 
		|| IsEdgeTouchingCircle( C,D, pos, rad ) 
		|| IsEdgeTouchingCircle( D,A, pos, rad ) )
	{
		return true;
	}
	return false;
}
//top left is A then clockwise
bool isQuadTouchingQuad( Vector2f &A0, Vector2f &B0, Vector2f &C0, Vector2f &D0, Vector2f &A1, Vector2f &B1, Vector2f &C1, Vector2f &D1 )
{
	float AB = length( B0 - A0 );
	float AD = length( D0 - A0 );

	Vector2f normalizeAB = normalize( B0 - A0 );
	Vector2f normalizeAD = normalize( D0 - A0 );
	

	float min1AB = min( dot( A1 - A0, normalizeAB ), min( dot( B1 - A0, normalizeAB ), min( dot( C1 - A0, normalizeAB ),
		dot( D1 - A0, normalizeAB ) ) ) );
	float max1AB = max( dot( A1 - A0, normalizeAB ), max( dot( B1 - A0, normalizeAB ), max( dot( C1 - A0, normalizeAB ),
		dot( D1 - A0, normalizeAB ) ) ) );

	float min1AD = min( dot( A1 - A0, normalizeAD ), min( dot( B1 - A0, normalizeAD ), min( dot( C1 - A0, normalizeAD ),
		dot( D1 - A0, normalizeAD ) ) ) );
	float max1AD = max( dot( A1 - A0, normalizeAD ), max( dot( B1 - A0, normalizeAD ), max( dot( C1 - A0, normalizeAD ),
		dot( D1 - A0, normalizeAD ) ) ) );

	
	float AB1 = length( B1 - A1 );
	float AD1 = length( D1 - A1 );

	Vector2f normalizeAB1 = normalize( B1 - A1 );
	Vector2f normalizeAD1 = normalize( D1 - A1 );

	float min0AB = min( dot( A0 - A1, normalizeAB1 ), min( dot( B0 - A1, normalizeAB1 ), min( dot( C0 - A1, normalizeAB1 ),
		dot( D0 - A1, normalizeAB1 ) ) ) );
	float max0AB = max( dot( A0 - A1, normalizeAB1 ), max( dot( B0 - A1, normalizeAB1 ), max( dot( C0 - A1, normalizeAB1 ),
		dot( D0 - A1, normalizeAB1 ) ) ) );

	float min0AD = min( dot( A0 - A1, normalizeAD1 ), min( dot( B0 - A1, normalizeAD1 ), min( dot( C0 - A1, normalizeAD1 ),
		dot( D0 - A1, normalizeAD1 ) ) ) );
	float max0AD = max( dot( A0 - A1, normalizeAD1 ), max( dot( B0 - A1, normalizeAD1 ), max( dot( C0 - A1, normalizeAD1 ),
		dot( D0 - A1, normalizeAD1 ) ) ) );

	if( min1AB <= AB && max1AB >= 0 && min1AD <= AD && max1AD >= 0 
		&& min0AB <= AB1 && max0AB >= 0 && min0AB <= AB1 && max0AD >= 0 )
	{
		return true;
	}
	else
	{
		return false;
	}
}

EdgeQNode *Insert( EdgeQNode *node, Edge* e )
{
	if( node->leaf )
	{
		EdgeLeafNode *n = (EdgeLeafNode*)node;
		if( n->objCount == 4 ) //full
		{
		//	cout << "splitting" << endl;	
			EdgeParentNode *p = new EdgeParentNode( n->pos, n->rw, n->rh );
			p->parent = n->parent;
			p->debug = n->debug;

		/*	for( int i = 0; i < 4; ++i )
			{
				//EdgeLeafNode *inner = (EdgeLeafNode*)p->children[i];
				Edge * tempEdge = n->edges[i];
				sf::IntRect nw( node->pos.x - node->rw, node->pos.y - node->rh, node->rw, node->rh);
				sf::IntRect ne( node->pos.x + node->rw, node->pos.y - node->rh, node->rw, node->rh );
				sf::IntRect sw( node->pos.x - node->rw, node->pos.y + node->rh, node->rw, node->rh );
				sf::IntRect se( node->pos.x + node->rw, node->pos.y + node->rh, node->rw, node->rh );

				if( IsEdgeTouchingBox( tempEdge, nw ) )
					p->children[0] = Insert( p->children[0], tempEdge );
				if( IsEdgeTouchingBox( tempEdge, ne ) )
					p->children[1] = Insert( p->children[1], tempEdge );
				if( IsEdgeTouchingBox( tempEdge, sw ) )
					p->children[2] = Insert( p->children[2], tempEdge );
				if( IsEdgeTouchingBox( tempEdge, se ) )
					p->children[3] = Insert( p->children[3], tempEdge );
			}*/

			for( int i = 0; i < 4; ++i )
			{
			//	cout << "test: " << n->edges[i]->Normal().x << ", " << n->edges[i]->Normal().y << endl;
				Insert( p, n->edges[i] );
			}


			delete node;

			 

			return Insert( p, e );
		}
		else
		{
		//	cout << "inserting into leaf . " << n->objCount << endl;
		//	cout << "norm: " << e->Normal().x << ", " << e->Normal().y << endl;
			n->edges[n->objCount] = e;
			++(n->objCount);
			return node;
		}
	}
	else
	{
	//	cout << "inserting into parent" << endl;
		EdgeParentNode *n = (EdgeParentNode*)node;
		sf::Rect<float> nw( node->pos.x - node->rw, node->pos.y - node->rh, node->rw, node->rh);
		sf::Rect<float> ne( node->pos.x, node->pos.y - node->rh, node->rw, node->rh );
		sf::Rect<float> sw( node->pos.x - node->rw, node->pos.y, node->rw, node->rh );
		sf::Rect<float> se( node->pos.x, node->pos.y, node->rw, node->rh );

		if( IsEdgeTouchingBox( e, nw ) )
		{
	//		cout << "calling northwest insert" << endl;
			n->children[0] = Insert( n->children[0], e );
		}
		if( IsEdgeTouchingBox( e, ne ) )
		{
	//		cout << "calling northeast insert" << endl;
			n->children[1] = Insert( n->children[1], e );
		}
		if( IsEdgeTouchingBox( e, sw ) )
		{
	//		cout << "calling southwest insert" << endl;
			n->children[2] = Insert( n->children[2], e );
		}
		if( IsEdgeTouchingBox( e, se ) )
		{
	//		cout << "calling southeast insert" << endl;
			n->children[3] = Insert( n->children[3], e );
		}
	}

	


	return node;
}

void DebugDrawQuadTree( sf::RenderWindow *w, EdgeQNode *node )
{
	//cout << "pos: " << node->pos.x << ", " << node->pos.y << " , rw: " << node->rw << ", rh: " << node->rh << endl;
	if( node->leaf )
	{
		EdgeLeafNode *n = (EdgeLeafNode*)node;

		sf::RectangleShape rs( sf::Vector2f( node->rw * 2, node->rh * 2 ) );
		int trans = 100;
		if( n->objCount == 0 )
			rs.setFillColor( Color( 100, 100, 100, trans ) ); //
		else if( n->objCount == 1 )
			rs.setFillColor( Color( 255, 0, 0, trans) ); // red == 1
		else if( n->objCount == 2 )
			rs.setFillColor( Color( 0, 255, 0, trans ) ); // green == 2
		else if( n->objCount == 3 )
			rs.setFillColor( Color( 0, 0, 255, trans ) ); //blue == 3
		else
		{
			rs.setFillColor( Color( 0, 100, 255, trans ) ); //blah == 4
		}
		
		//rs.setFillColor( Color::Green );
		//rs.setOutlineColor( Color::Blue );
	//	rs.setOutlineThickness( 3 );
		//rs.setFillColor( Color::Transparent );
		//rs.setPosition( node->pos.x - node->rw, node->pos.y - node->rh );
		rs.setOrigin( rs.getLocalBounds().width / 2.f, rs.getLocalBounds().height / 2.f );
		//rs.setPosition( node->pos.x - node->rw, node->pos.y - node->rh );
		rs.setPosition( node->pos.x, node->pos.y );
		//rs.setOrigin( rs.getLocalBounds().width / 2.f, rs.getLocalBounds().height / 2.f );

		w->draw( rs );

		CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 1 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( node->pos.x, node->pos.y );
		//w->draw( cs );
	}
	else
	{
		EdgeParentNode *n = (EdgeParentNode*)node;
		sf::RectangleShape rs( sf::Vector2f( node->rw * 2, node->rh * 2 ) );
		//rs.setOutlineColor( Color::Red );
		rs.setOrigin( rs.getLocalBounds().width / 2.f, rs.getLocalBounds().height / 2.f );
		//rs.setPosition( node->pos.x - node->rw, node->pos.y - node->rh );
		rs.setPosition( node->pos.x, node->pos.y );
		rs.setFillColor( Color::Transparent );
		//rs.setOutlineThickness( 10 );
		w->draw( rs );

		CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 1 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( node->pos.x, node->pos.y );

		//w->draw( cs );

		for( int i = 0; i < 4; ++i )
			DebugDrawQuadTree( w, n->children[i] );
	}
	

	
}

void Query( EdgeQuadTreeCollider *qtc, EdgeQNode *node, const sf::Rect<float> &r )
{
	sf::Rect<float> nodeBox( node->pos.x - node->rw, node->pos.y - node->rh, node->rw * 2, node->rh * 2 );

	if( node->leaf )
	{
		EdgeLeafNode *n = (EdgeLeafNode*)node;

		if( IsBoxTouchingBox( r, nodeBox ) )
		{
			for( int i = 0; i < n->objCount; ++i )
			{
				qtc->HandleEdge( n->edges[i] );
			}
		}
	}
	else
	{
		//shouldn't this check for box touching box right here??
		EdgeParentNode *n = (EdgeParentNode*)node;

		if( r.intersects( nodeBox ) )
		{
			for( int i = 0; i < 4; ++i )
			{
				Query( qtc, n->children[i], r );
			}
		}
	}
	
}

/*void RayCast( RayCastHandler *handler, QNode *node, Vector2f startPoint, Vector2f endPoint )
{

	if( node->leaf )
	{
		EdgeLeafNode *n = (EdgeLeafNode*)node;

		Edge e;
		e.v0 = startPoint;
		e.v1 = endPoint;

		sf::Rect<float> nodeBox( node->pos.x - node->rw, node->pos.y - node->rh, node->rw * 2, node->rh * 2 );
	
		if( IsEdgeTouchingBox( &e, nodeBox ) )
		{
			for( int i = 0; i < n->objCount; ++i )
			{
				LineIntersection li = SegmentIntersect( startPoint, endPoint, n->edges[i]->v0, n->edges[i]->v1 );	
				if( !li.parallel )
				{
					handler->HandleRayCollision( n->edges[i], n->edges[i]->GetQuantity( li.position ), 
						dot( Vector2f( li.position - startPoint ), normalize( endPoint - startPoint ) ) );
				}
			}
		}
		
	}
	else
	{
		EdgeParentNode *n = (EdgeParentNode*)node;

		for( int i = 0; i < 4; ++i )
		{
			RayCast( handler, n->children[i], startPoint, endPoint );
		}
	}
}*/


//only works on edges
void RayCast( RayCastHandler *handler, QNode *node, Vector2f startPoint, Vector2f endPoint )
{
	if( node->leaf )
	{
		LeafNode *n = (LeafNode*)node;

		Edge e;
		e.v0 = startPoint;
		e.v1 = endPoint;

		sf::Rect<float> nodeBox( node->pos.x - node->rw, node->pos.y - node->rh, node->rw * 2, node->rh * 2 );
	
		if( IsEdgeTouchingBox( &e, nodeBox ) )
		{
			for( int i = 0; i < n->objCount; ++i )
			{
				LineIntersection li = SegmentIntersect( startPoint, endPoint, ((Edge*)(n->entrants[i]))->v0, ((Edge*)(n->entrants[i]))->v1 );	
				if( !li.parallel )
				{
					handler->HandleRayCollision( ((Edge*)(n->entrants[i])), ((Edge*)(n->entrants[i]))->GetQuantity( li.position ), 
						dot( Vector2f( li.position - startPoint ), normalize( endPoint - startPoint ) ) );
				}
			}
		}
		
	}
	else
	{
		ParentNode *n = (ParentNode*)node;

		Edge e;
		e.v0 = startPoint;
		e.v1 = endPoint;

		sf::Rect<float> nodeBox( node->pos.x - node->rw, node->pos.y - node->rh, node->rw * 2, node->rh * 2 );

		if( IsEdgeTouchingBox( &e, nodeBox ) )
		{
			for( list<QuadTreeEntrant*>::iterator it = n->extraChildren.begin(); it != n->extraChildren.end(); ++it )
			{
				LineIntersection li = SegmentIntersect( startPoint, endPoint, ((Edge*)(*it))->v0, ((Edge*)(*it))->v1 );	
				if( !li.parallel )
				{
					handler->HandleRayCollision( ((Edge*)(*it)), ((Edge*)(*it))->GetQuantity( li.position ), 
						dot( Vector2f( li.position - startPoint ), normalize( endPoint - startPoint ) ) );
				}
			}

			for( int i = 0; i < 4; ++i )
			{
				RayCast( handler, n->children[i], startPoint, endPoint );
			}
		}
	}
}

void Edge::HandleQuery( QuadTreeCollider * qtc )
{
	qtc->HandleEntrant( this );
}

bool Edge::IsTouchingBox( const sf::Rect<float> &r )
{
	return IsEdgeTouchingBox( this, r );
}