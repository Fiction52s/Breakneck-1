//edit mode

#include "GUI.h"
#include "EditSession.h"
#include <fstream>
#include <assert.h>
#include <iostream>
#include "poly2tri/poly2tri.h"
#include <sstream>
#include <boost/lexical_cast.hpp>

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

#define cout std::cout

const double EditSession::PRIMARY_LIMIT = .999;



TerrainPolygon::TerrainPolygon( sf::Texture *gt)
	:grassTex( gt )
{
	va = NULL;
	lines = NULL;
	selected = false;
	grassVA = NULL;
	isGrassShowing = false;
}

TerrainPolygon::~TerrainPolygon()
{
	if( lines != NULL )
		delete [] lines;
	if( va != NULL )
		delete va;

	if( grassVA != NULL )
		delete grassVA;
	for( list<ActorParams*>::iterator it = enemies.begin(); it != enemies.end(); ++it )
	{
		(*it)->group->actors.remove( (*it ) );
		delete (*it);
	}
}

void TerrainPolygon::Finalize()
{
	isGrassShowing = false;
	material = "mat";
	lines = new sf::Vertex[points.size()*2+1];
	

	FixWinding();
	//cout << "points size: " << points.size() << endl;

	vector<p2t::Point*> polyline;
	for( PointList::iterator it = points.begin(); it != points.end(); ++it )
	{
		polyline.push_back( new p2t::Point((*it).pos.x, (*it).pos.y ) );
	}

	p2t::CDT * cdt = new p2t::CDT( polyline );
	
	cdt->Triangulate();
	vector<p2t::Triangle*> tris;
	tris = cdt->GetTriangles();
	
	vaSize = tris.size() * 3;
	va = new VertexArray( sf::Triangles , vaSize );
	
	VertexArray & v = *va;
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

	//assert( tris.size() * 3 == points.size() );
	delete cdt;
	for( int i = 0; i < points.size(); ++i )
	{
		delete polyline[i];
	//	delete tris[i];
	}

	if( points.size() > 0 )
	{
		int i = 0;
		PointList::iterator it = points.begin(); 
		lines[0] = sf::Vector2f( (*it).pos.x, (*it).pos.y );
		lines[2 * points.size() - 1 ] = sf::Vector2f( (*it).pos.x, (*it).pos.y );
		++it;
		++i;
		while( it != points.end() )
		{
			lines[i] = sf::Vector2f( (*it).pos.x, (*it).pos.y );
			lines[++i] = sf::Vector2f( (*it).pos.x, (*it).pos.y ); 
			++i;
			++it;
		}
	}

	{
		PointList::iterator it = points.begin();
		left = (*it).pos.x;
		right = (*it).pos.x;
		top = (*it).pos.y;
		bottom = (*it).pos.y;
		++it;
		for( ; it != points.end(); ++it )
		{
			left = min( (*it).pos.x, left );
			right = max( (*it).pos.x, right );
			top = min( (*it).pos.y, top );
			bottom = max( (*it).pos.y, bottom );
		}
	}
	

	double grassSize = 22;
	double grassSpacing = -5;

	numGrassTotal = 0;
	int inds = 0;
	for( PointList::iterator it = points.begin(); it != points.end(); ++it )
	{
		Vector2i next;

		PointList::iterator temp = it;
		if( ++temp == points.end() )
		{
			next = points.front().pos;
		}
		else
		{
			//++temp;
			next = (*temp).pos;
			//--temp;
		}

		V2d v0( (*it).pos.x, (*it).pos.y );
		V2d v1( next.x, next.y );


		double remainder = length( v1 - v0 ) / ( grassSize + grassSpacing );
				
		int num = floor( remainder ) + 1;

		numGrassTotal += num;
		//cout << "plus: " << v0.x << ", " << v0.y << " " << v1.x << ", " << v1.y << endl;
		++inds;
	}
	//assert( numGrassTotal !=  0 );
	//cout << "total grass: " << numGrassTotal << endl;
	VertexArray *gva = new VertexArray( sf::Quads, numGrassTotal * 4 );


	VertexArray &grassVa = *gva;

	int i = 0;
	for( PointList::iterator it = points.begin(); it != points.end(); ++it )
	{
		Vector2i next;

		PointList::iterator temp = it;
		if( ++temp == points.end() )
		{
			next = points.front().pos;
		}
		else
		{
			next = (*temp).pos;
		}

		V2d v0( (*it).pos.x, (*it).pos.y );
		V2d v1( next.x, next.y );


		double remainder = length( v1 - v0 ) / ( grassSize + grassSpacing );

		int num = floor( remainder ) + 1;

		for( int j = 0; j < num; ++j )
		{
			V2d posd = v0 + (v1- v0) * ((double)j / num);
			Vector2f pos( posd.x, posd.y );

			Vector2f topLeft = pos + Vector2f( -grassSize / 2, -grassSize / 2 );
			Vector2f topRight = pos + Vector2f( grassSize / 2, -grassSize / 2 );
			Vector2f bottomLeft = pos + Vector2f( -grassSize / 2, grassSize / 2 );
			Vector2f bottomRight = pos + Vector2f( grassSize / 2, grassSize / 2 );

			//grassVa[i*4].color = Color( 0x0d, 0, 0x80 );//Color::Magenta;
			grassVa[i*4].color.a = 0;
			grassVa[i*4].position = topLeft;
			grassVa[i*4].texCoords = Vector2f( 0, 0 );

			//grassVa[i*4+1].color = Color::Blue;
			//borderVa[i*4+1].color.a = 10;
			grassVa[i*4+1].color.a = 0;
			grassVa[i*4+1].position = bottomLeft;
			grassVa[i*4+1].texCoords = Vector2f( 0, grassSize );

			//grassVa[i*4+2].color = Color::Blue;
			//borderVa[i*4+2].color.a = 10;
			grassVa[i*4+2].color.a = 0;
			grassVa[i*4+2].position = bottomRight;
			grassVa[i*4+2].texCoords = Vector2f( grassSize, grassSize );

			//grassVa[i*4+3].color = Color( 0x0d, 0, 0x80 );
			//borderVa[i*4+3].color.a = 10;
			grassVa[i*4+3].color.a = 0;
			grassVa[i*4+3].position = topRight;
			grassVa[i*4+3].texCoords = Vector2f( grassSize, 0 );
			++i;
		}
		
	
	}

	if( grassVA != NULL )
	{
		delete grassVA;
	}
	grassVA = gva;
}

bool TerrainPolygon::RemoveSelectedPoints()
{	
	PointList temp = points;

	Reset();

	PointList::iterator it = temp.begin();
	while( it != temp.end() )
	{
		if( (*it).selected ) //selected
		{
			temp.erase( it++ );
		}
		else
		{
			++it;
		}
	}

	for( PointList::iterator it2 = temp.begin(); it2 != temp.end(); ++it2 )
	{
		points.push_back( (*it2) );
	}
	//cout << "before killer finalize. poly size: " << poly->points.size() << endl;
	Finalize();
	SetSelected( true );

	return true;
}

void TerrainPolygon::Extend( TerrainPoint* startPoint, TerrainPoint*endPoint, TerrainPolygon *inProgress )
{
	if( inProgress->points.size() < 2 )
	{
		return;
	}
	bool startFirst = true;
	PointList::iterator startIt;
	PointList::iterator endIt;

	bool startFound = false;
	bool endFound = false;

	//inProgress->FixWinding();
	
	for( PointList::iterator it = points.begin(); it != points.end(); ++it )
	{
		if( &(*it) ==  startPoint )
		{
			startIt = it;

			if( endFound )
				break;

			startFound = true;
		}
		else if( &(*it) == endPoint )
		{
			endIt = it;

			if( startFound )
				break;

			endFound = true;
			startFirst = false;
		}
	}

	PointList newList;
	inProgress->points.push_back( *endPoint );
	bool inProgresscw = inProgress->IsClockwise();
	if( !inProgresscw )
	{
		inProgress->FixWinding();
		if( startFirst )
		{
			PointList::iterator temp = endIt;
			endIt = startIt;
			startIt = temp;
			startFirst = false;
			inProgresscw = true;
		}
		else
		{
			PointList::iterator temp = endIt;
			endIt = startIt;
			startIt = temp;
			startFirst = true;
			inProgresscw = true;
			//cout << "changing" << endl;
		}
	}

	inProgress->points.pop_back();

	if( startFirst )
	{	
		for( PointList::iterator it = points.begin(); it != points.end(); ++it )
		{
			cout << "normal type: " << inProgresscw << endl;
			if( it == startIt )
			{
				for( PointList::iterator pit = inProgress->points.begin(); pit != inProgress->points.end(); ++pit )
				{
					newList.push_back( (*pit) );
				}

				it = endIt;
				newList.push_back( (*endIt ) );	
			}
			else
			{
				newList.push_back( (*it) );
			}
		}
	}	else
	{
		cout << "other type: " << inProgresscw << endl;
		for( PointList::iterator it = endIt; it != points.end(); ++it )
		{
			if( it == startIt )
			{	
				for( PointList::iterator pit = inProgress->points.begin(); pit != inProgress->points.end(); ++pit )
				{
					newList.push_back( (*pit) );
				}	
				break;
			}
			else
			{
				newList.push_back( (*it) );
			}
		}
	}

	Reset();

	for( PointList::iterator it = newList.begin(); it != newList.end(); ++it )
	{
		points.push_back( (*it) );
	}

	Finalize();
}

void TerrainPolygon::SwitchGrass( V2d mousePos )
{
	
	VertexArray &grassVa = *grassVA;
	double grassSize = 22;
	double radius = grassSize / 2;
	double grassSpacing = -5;

	int i = 0;


	for( PointList::iterator it = points.begin(); it != points.end(); ++it )
	{
		Vector2i next;

		PointList::iterator temp = it;
		if( ++temp == points.end() )
		{
			next = points.front().pos;
		}
		else
		{
			next = (*temp).pos;
		}

		V2d v0( (*it).pos.x, (*it).pos.y );
		V2d v1( next.x, next.y );

		double remainder = length( v1 - v0 ) / ( grassSize + grassSpacing );

		int num = floor( remainder ) + 1;

		for( int j = 0; j < num; ++j )
		{
			V2d pos = v0 + (v1- v0) * ((double)(j )/ num);

			//Vector2f pos( posd.x, posd.y );
				
			if( length( pos - mousePos ) <= radius )
			{
				if( grassVa[i*4].color.a == 50 )
				{
					grassVa[i*4].color.a = 254;
					grassVa[i*4+1].color.a = 254;
					grassVa[i*4+2].color.a = 254;
					grassVa[i*4+3].color.a = 254;
					//cout << "making full: " << i << endl;
				}
				else if( grassVa[i*4].color.a == 255 )
				{
					grassVa[i*4].color.a = 49;
					grassVa[i*4+1].color.a = 49;
					grassVa[i*4+2].color.a = 49;
					grassVa[i*4+3].color.a = 49;
					//cout << "making seethru: " << i << endl;
				}
			}
			++i;
		}
	}
}

void TerrainPolygon::UpdateGrass()
{
	VertexArray & grassVa = *grassVA;
	for( int i = 0; i < numGrassTotal; ++i )
	{
		if( grassVa[i*4].color.a == 49 )
		{
			grassVa[i*4].color.a = 50;
			grassVa[i*4+1].color.a = 50;
			grassVa[i*4+2].color.a = 50;
			grassVa[i*4+3].color.a = 50;
		}
		else if( grassVa[i*4].color.a == 254 )
		{
			grassVa[i*4].color.a = 255;
			grassVa[i*4+1].color.a = 255;
			grassVa[i*4+2].color.a = 255;
			grassVa[i*4+3].color.a = 255;
		}
	}
}

void TerrainPolygon::Draw( bool showPath, double zoomMultiple, RenderTarget *rt, bool showPoints, TerrainPoint *dontShow )
{
	if( grassVA != NULL )
		rt->draw( *grassVA, grassTex );

	if( va != NULL )
	rt->draw( *va );



	if( selected )
	{
		if( !isGrassShowing )
		for( PointList::iterator it = points.begin(); it != points.end(); ++it )
		{
			CircleShape cs;
			cs.setRadius( 8 * zoomMultiple );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );

			if( (*it).selected )
				cs.setFillColor( Color::Red );
			else
				cs.setFillColor( Color::Green );

			cs.setPosition( (*it).pos.x, (*it).pos.y );
			rt->draw( cs );
		}
		rt->draw( lines, points.size() * 2, sf::Lines );
	}

	if( showPoints )
	{
		for( PointList::iterator it = points.begin(); it != points.end(); ++it )
		{
			if( &(*it) == dontShow )
			{
				continue;
			}
			CircleShape cs;
			cs.setRadius( 8 * zoomMultiple );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );

			cs.setFillColor( Color::Magenta );

			cs.setPosition( (*it).pos.x, (*it).pos.y );
			rt->draw( cs );
		}
	}

	Vector2i center( (right + left) / 2, (bottom + top) / 2 );


	if( showPath )
	{
		for( list<Vector2i>::iterator it = path.begin(); it != path.end(); ++it )
		{
			CircleShape cs;
			cs.setRadius( 5 * zoomMultiple );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );

			cs.setFillColor( Color::Magenta );
			cs.setPosition( center.x + (*it).x, center.y + (*it).y );
			rt->draw( cs );
	
		}


		if( path.size() > 1 )
		{
			list<Vector2i>::iterator prev = path.begin();
			list<Vector2i>::iterator curr = path.begin();
			++curr;
			while( curr != path.end() )
			{
				sf::Vertex activePreview[2] =
				{
					sf::Vertex(sf::Vector2<float>(center.x + (*prev).x, center.y + (*prev).y), Color::White ),
					sf::Vertex(sf::Vector2<float>(center.x + (*curr).x, center.y + (*curr).y), Color::White )
				};
				rt->draw( activePreview, 2, sf::Lines );

				prev = curr;
				++curr;
			}
		
		}
	}
}

void TerrainPolygon::SetSelected( bool select )
{
	selected = select;
	Color selectCol( 0x77, 0xBB, 0xDD );
	if( selected )
	{
		for( int i = 0; i < vaSize; ++i )
		{
			VertexArray & v = *va;
			v[i].color = selectCol;
		}
	}
	else
	{
		Color testColor( 0x75, 0x70, 0x90 );
		for( int i = 0; i < vaSize; ++i )
		{
			VertexArray & v = *va;
			v[i].color = testColor;
		}

		for( PointList::iterator it = points.begin(); it != points.end(); ++it )
		{
			(*it).selected = false;
		}
	}
}

bool TerrainPolygon::ContainsPoint( Vector2f test )
{
	int pointCount = points.size();

	int i, j, c = 0;

	PointList::iterator it = points.begin();
	PointList::iterator jt = points.end();
	jt--;
	
	for (; it != points.end(); jt = it++ ) 
	{
		Vector2f point((*it).pos.x, (*it).pos.y );
		Vector2f pointJ((*jt).pos.x, (*jt).pos.y );
		if ( ((point.y > test.y ) != ( pointJ.y > test.y ) ) &&
			(test.x < (pointJ.x-point.x) * (test.y-point.y) / (pointJ.y-point.y) + point.x) )
				c = !c;
	}
	return c;
}

void TerrainPolygon::FixWinding()
{
    if (IsClockwise())
    {
		//cout << "not fixing" << endl;
    }
    else
    {
		//cout << "fixing winding" << endl;
        PointList temp;
		for( PointList::iterator it = points.begin(); it != points.end(); ++it )
		{
			temp.push_front( (*it) );
		}
		points.clear();
		for( PointList::iterator it = temp.begin(); it != temp.end(); ++it )
		{
			points.push_back( (*it) );
		}          
    }
}

void TerrainPolygon::Reset()
{
	points.clear();
	if( lines != NULL )
		delete [] lines;
	if( va != NULL )
		delete va;
	lines = NULL;
	va = NULL;
}

bool TerrainPolygon::IsRemovePointsOkay( EditSession *edit )
{
	list<PointPair> removalPairs;
	removalPairs.push_back( PointPair() );
	PointList::iterator prev, next;
	bool first = true;
	bool continuation = false;

	TerrainPolygon tempPoly( grassTex );

	for( PointList::iterator it = points.begin(); it != points.end(); ++it )
	{
		if( !(*it).selected )
		{
			tempPoly.points.push_back( (*it) );
		}
	}

	return edit->IsPolygonValid( tempPoly, this );

	for( PointList::iterator it = points.begin(); it != points.end(); ++it )
	{
		if( !(*it).selected )
		{
			continue;
		}

		if( it == points.begin() )
		{
			prev = points.end();
			prev--;
		}
		else
		{
			prev = it;
			prev--;
		}

		PointList::iterator temp = it;
		++temp;
		if( temp == points.end() )
		{
			next = points.begin();
		}
		else
		{
			next = it;
			next++;
		}
		
		if( (*prev).selected && first )
		{
			if( it == points.begin() )
			{
				continuation = true;
				//removalPairs.back().second = (*it).pos;
				//removalPairs.push_back( PointPair() );
			}
			continue;
		}

		if( first )
		{
			removalPairs.back().first = (*prev).pos;
			first = false;
		}
		else if( !(*next).selected )
		{
			removalPairs.back().second = (*next).pos;
			first = true;
			removalPairs.push_back( PointPair() );
		}


		/*V2d itPos( (*it).pos.x, (*it).pos.y );
		V2d nextPos( (*next).pos.x, (*next).pos.y );
		V2d prevPos( (*prev).pos.x, (*prev).pos.y );
		V2d along = normalize( nextPos - prevPos );
		cout << "along: " << along.x << ", " << along.y << endl;
		//V2d other( along.y, -along.x );

		
		//V2d vec = tPoint - backPoint;
		//V2d normVec = normalize( vec );
		
		if( itPos.x == prevPos.x || itPos.y == prevPos.y )
		{
		}
		else
		{
			V2d extreme( 0, 0 );
			if( along.x > EditSession::PRIMARY_LIMIT )
				extreme.x = 1;
			else if( along.x < -EditSession::PRIMARY_LIMIT )
				extreme.x = -1;
			if( along.y > EditSession::PRIMARY_LIMIT )
				extreme.y = 1;
			else if( along.y < -EditSession::PRIMARY_LIMIT )
				extreme.y = -1;

			//extreme = normalize( extreme );

			if( !( extreme.x == 0 && extreme.y == 0 ) )
			{
				cout << "returning false: " << extreme.x << ", " << extreme.y << endl;
				return false;
			}
		}*/
	}

	if( continuation )
	{
		assert( !first );
		for( PointList::iterator it = points.begin(); it != points.end(); ++it )		
		{
			assert( (*it).selected );
			PointList::iterator temp = it;
		}

	}

	return true;
}

bool TerrainPolygon::IsMovePointsOkay( EditSession *edit, Vector2i delta )
{
	TerrainPolygon tempPoly( grassTex );

	for( PointList::iterator it = points.begin(); it != points.end(); ++it )
	{
		if( !(*it).selected )
		{
			tempPoly.points.push_back( (*it) );
		}
		else
		{
			TerrainPoint p = (*it);
			
			p.pos += delta;

			tempPoly.points.push_back( p );
		}
	}

	return edit->IsPolygonValid( tempPoly, this );
}

bool TerrainPolygon::IsMovePointsOkay( EditSession *edit, Vector2i pointGrabDelta, Vector2i *deltas )
{
	TerrainPolygon tempPoly( grassTex );

	int arraySize = points.size();

	int i = 0;
	for( PointList::iterator it = points.begin(); it != points.end(); ++it )
	{
		if( !(*it).selected )
		{
			tempPoly.points.push_back( (*it) );
		}
		else
		{
			TerrainPoint p = (*it);
			
			p.pos += pointGrabDelta - deltas[i];

			tempPoly.points.push_back( p );
		}

		++i;
	}

	return edit->IsPolygonValid( tempPoly, this );
}

bool TerrainPolygon::IsMovePolygonOkay( EditSession *edit, sf::Vector2i delta )
{
	TerrainPolygon tempPoly( grassTex );

	for( PointList::iterator it = points.begin(); it != points.end(); ++it )
	{
		TerrainPoint  p = (*it);
		p.pos += delta;
		tempPoly.points.push_back( p );
	}

	bool f = edit->IsPolygonValid( tempPoly, this );
	if( !f )
	{
		cout << "failed delta: " << delta.x << ", " << delta.y << endl;
	}
	return f;
}

bool TerrainPolygon::IsClockwise()
{
	assert( points.size() > 0 );

    Vector2i *pointArray = new Vector2i[points.size()];

	int i = 0;
	for( PointList::iterator it = points.begin(); it != points.end(); ++it )
	{
		pointArray[i] = (*it).pos;
		++i;
	}

    long long int sum = 0;
	for (int i = 0; i < points.size(); ++i)
    {
        Vector2<long long int> first, second;
		
        if (i == 0)
        {
			first.x = pointArray[points.size() - 1].x;
			first.y = pointArray[points.size() - 1].y;
        }
        else
        {
            first.x = pointArray[i - 1].x;
			first.y = pointArray[i - 1].y;

        }
        second.x = pointArray[i].x;
		second.y = pointArray[i].y;

        sum += (second.x - first.x) * (second.y + first.y);
    }

	delete [] pointArray;

    return sum < 0;
}

bool TerrainPolygon::IsTouching( TerrainPolygon *p )
{
	assert( p != this );
	if( left <= p->right && right >= p->left && top <= p->bottom && bottom >= p->top )
	{	
		//return true;

		//points.push_back( points.front() );
		//p->points.push_back( p->points.front() );

		PointList::iterator it = points.begin();
		Vector2i curr = (*it).pos;
		++it;
		Vector2i next;
		

		

		for( ;; ++it )
		{
			if( it == points.end() )
				it = points.begin();

			next = (*it).pos;


			PointList::iterator pit = p->points.begin();
			Vector2i pcurr = (*pit).pos;
			++pit;
			Vector2i pnext;// = (*pit);

			for( ;; ++pit )		
			{
				if( pit == p->points.end() )
					pit = p->points.begin();

				pnext = (*pit).pos;
			
				LineIntersection li = EditSession::SegmentIntersect( curr, next, pcurr, pnext );	

				if( !li.parallel )
				{
					//points.pop_back();
					//p->points.pop_back();
					cout << "touching!" << endl;
					return true;
				}

				pcurr = (*pit).pos;

				if( pit == p->points.begin() )
					break;
			}
			curr = (*it).pos;

			if( it == points.begin() )
			{
				break;
			}
		}
	}

	//points.pop_back();
	//p->points.pop_back();

	return false;
}

void TerrainPolygon::ShowGrass( bool show )
{
	
	VertexArray & grassVa = *grassVA;
	for( int i = 0; i < numGrassTotal; ++i )
	{
		if( show )
		{
			if( grassVa[i*4].color.a == 0 )
			{
				grassVa[i*4].color.a = 50;
				grassVa[i*4+1].color.a = 50;
				grassVa[i*4+2].color.a = 50;
				grassVa[i*4+3].color.a = 50;
			}
			isGrassShowing = true;
		}
		else 
		{
			if( grassVa[i*4].color.a == 50 )
			{
				grassVa[i*4].color.a = 0;
				grassVa[i*4+1].color.a = 0;
				grassVa[i*4+2].color.a = 0;
				grassVa[i*4+3].color.a = 0;
			}
			isGrassShowing = false;
		}
		/*(else if( grassVa[i*4].color.a == 255 )
		{
			grassVa[i*4].color.a = 255;
			grassVa[i*4+1].color.a = 255;
			grassVa[i*4+2].color.a = 255;
			grassVa[i*4+3].color.a = 255;
		}*/
	}
}

sf::Rect<int> TerrainPolygon::TempAABB()
{
	assert( points.size() > 1 );
	PointList::iterator it = points.begin();
	int l = (*it).pos.x;
	int r = (*it).pos.x;
	int t = (*it).pos.y;
	int b = (*it).pos.y;
	++it;
	for( ; it != points.end(); ++it )
	{
		l = min( (*it).pos.x, l);
		r = max( (*it).pos.x, r);
		t = min( (*it).pos.y, t);
		b = max( (*it).pos.y, b );
	}

	return Rect<int>( l, t, r- l, b - t );
}

StaticLight::StaticLight( sf::Color c, sf::Vector2i &pos, int rad, int bright )
	:color( c ), position( pos ), radius( rad ), brightness( bright )
{
}

void StaticLight::Draw( RenderTarget *target )
{
	CircleShape cs;
	Color c = color;
	c.a = 100;
	cs.setFillColor( c );
	cs.setRadius( radius );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	target->draw( cs );
}

void StaticLight::WriteFile( std::ofstream &of )
{
	of << position.x << " " << position.y << " " << (int)color.r << " " << (int)color.g << " " << (int)color.b << " " 
		<< radius << " " << brightness << endl;
	//of << type->name << " ";

	//if( ground != NULL )
	//{
	//	of << "-air" << " " << ground->writeIndex << " " << edgeIndex << " " << groundQuantity << endl;
	//}
	//else
	//{
	//	of << "+air" << " " << position.x << " " << position.y << endl;
	//}

	//for( list<string>::iterator it = params.begin(); it != params.end(); ++it )
	//{
	//	of << (*it) << endl;
	//}
}

sf::Rect<double> StaticLight::GetAABB()
{
	return sf::Rect<double>( position.x - radius, position.y - radius, radius * 2, radius * 2 );
}

EditSession::EditSession( RenderWindow *wi, sf::RenderTexture *preTex )
	:w( wi ), zoomMultiple( 1 )
{
	preScreenTex = preTex;
	showTerrainPath = false;
	minAngle = .99;
	showPoints = false;
	messagePopup = NULL;
	errorPopup = NULL;
	popupPanel = NULL;
	confirm = NULL;
	//	VertexArray *va = new VertexArray( sf::Lines, 
//	progressDrawList.push( new 
}

EditSession::~EditSession()
{
	delete polygonInProgress;
	for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		delete (*it);
	}
}

void EditSession::Draw()
{
	
	for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		if( extendingPolygon == NULL )
		{
			(*it)->Draw( showTerrainPath, zoomMultiple, preScreenTex, showPoints, extendingPoint );
		}
		else
		{
			if( (*it) == extendingPolygon )
			{
				(*it)->Draw( showTerrainPath, zoomMultiple, preScreenTex, true, extendingPoint );
			}
			else
			{
				if( extendingPolygon == NULL )
				{
					(*it)->Draw( showTerrainPath, zoomMultiple, preScreenTex, showPoints, extendingPoint );
				}
				else
				{
					(*it)->Draw( showTerrainPath, zoomMultiple, preScreenTex, false, extendingPoint );
				}
			}
		}
	}

	int psize = polygonInProgress->points.size();
	if( psize > 0 )
	{
		CircleShape cs;
		cs.setRadius( 5 * zoomMultiple  );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setFillColor( Color::Green );

		
		for( PointList::iterator it = polygonInProgress->points.begin(); it != polygonInProgress->points.end(); ++it )
		{
			cs.setPosition( (*it).pos.x, (*it).pos.y );
			preScreenTex->draw( cs );
		}		
	}

	
}

bool EditSession::OpenFile( string fileName )
{
	currentFile = fileName;

	ifstream is;
	is.open( fileName );

	double grassSize = 22;
	double radius = grassSize / 2;
	double grassSpacing = -5;

	if( is.is_open() )
	{
		int numPoints;
		is >> numPoints;
		is >> playerPosition.x;
		is >> playerPosition.y;

		while( numPoints > 0 )
		{
			TerrainPolygon *poly = new TerrainPolygon( &grassTex );
			polygons.push_back( poly );
			is >> poly->material;

			int polyPoints;
			is >> polyPoints;
			
			numPoints -= polyPoints;
			int x,y;
			for( int i = 0; i < polyPoints; ++i )
			{
				is >> x;
				is >> y;
				//is >> special;
				poly->points.push_back( TerrainPoint( Vector2i(x,y), false ) );
			}


			int edgesWithSegments;
			is >> edgesWithSegments;


			list<GrassSeg> segments;
			for( int i = 0; i < edgesWithSegments; ++i )
			{
				int edgeIndex;
				is >> edgeIndex;

				int numSegments;
				is >> numSegments;

				for( int j = 0; j < numSegments; ++j )
				{
					int index;
					is >> index;
					int reps;
					is >> reps;
					segments.push_back( GrassSeg( edgeIndex, index, reps ) );

				}
			}

			poly->Finalize();


			int grassIndex = 0;
			VertexArray &grassVa = *poly->grassVA;
			int numEdges = poly->points.size();
			int *indexArray = new int[numEdges];
			int edgeIndex = 0;

			int iai = 0;

			for( PointList::iterator it = poly->points.begin(); it != poly->points.end(); ++it )
			{
				indexArray[edgeIndex] = grassIndex;

				Vector2i next;

				PointList::iterator temp = it;
				if( ++temp == poly->points.end() )
				{
					next = poly->points.front().pos;
				}
				else
				{
					next = (*temp).pos;
				}

				V2d v0( (*it).pos.x, (*it).pos.y );
				V2d v1( next.x, next.y );

				double remainder = length( v1 - v0 ) / ( grassSize + grassSpacing );

				int num = floor( remainder ) + 1;

				grassIndex += num;

				++edgeIndex;
			}

			for( list<GrassSeg>::iterator it = segments.begin(); it != segments.end(); ++it )
			{
				int vaIndex = indexArray[(*it).edgeIndex];

				for( int extra = 0; extra <= (*it).reps; ++extra )
				{
					grassVa[( vaIndex + (*it).index + extra ) * 4 ].color.a = 255;
					grassVa[( vaIndex + (*it).index + extra ) * 4 + 1 ].color.a = 255;
					grassVa[( vaIndex + (*it).index + extra ) * 4 + 2 ].color.a = 255;
					grassVa[( vaIndex + (*it).index + extra ) * 4 + 3 ].color.a = 255;
				}
			}

			delete [] indexArray;
			


			

			
		}

		int movingPlatformNum;
		is >> movingPlatformNum;
		for( int i = 0; i < movingPlatformNum; ++i )
		{
			TerrainPolygon *poly = new TerrainPolygon( &grassTex );
			polygons.push_back( poly );
			is >> poly->material;

			int polyPoints;
			is >> polyPoints;
			
			for( int i = 0; i < polyPoints; ++i )
			{
				int x,y, special;
				is >> x;
				is >> y;
				//is >> special;
				poly->points.push_back( TerrainPoint( Vector2i(x,y), false ) );
			}

			poly->Finalize();

			int pathPoints;
			is >> pathPoints;

			if( pathPoints > 0 )
				poly->path.push_back( Vector2i( 0, 0 ) );

			for( int i = 0; i < pathPoints; ++i )
			{
				int x,y;
				is >> x;
				is >> y;
				poly->path.push_back( Vector2i( x, y ) );
			}
		}
		

		//lights here
		int numLights;
		is >> numLights;
		for( int i = 0; i < numLights; ++i )
		{
			int r,g,b,x,y;
			is >> x;
			is >> y;
			is >> r;
			is >> g;
			is >> b;

			int rad;
			int bright;
			is >> rad;
			is >> bright;

			lights.push_back( new StaticLight( Color( r, g, b ), Vector2i( x,y ), rad, bright ) );
		}


		//enemies here
		int numGroups;
		is >> numGroups;
		cout << "num groups " << numGroups << endl;
		for( int i = 0; i < numGroups; ++i )
		{
			string groupName;
			is >> groupName;

			int numActors;
			is >> numActors;

			ActorGroup *gr = new ActorGroup( groupName );
			groups[groupName] = gr;

			for( int j = 0; j < numActors; ++j )
			{
				string typeName;
				is >> typeName;

				ActorParams *a; //= new ActorParams;
				


				ActorType *at;
				cout << "typename: " << typeName << endl;
				if( types.count( typeName ) == 0 )
				{
					assert( false && "bad typename" );
				//	at = new ActorType( typeName, CreateOptionsPanel( typeName ) );
				//	types[typeName] = at;
				}
				else
				{
					at = types[typeName];
				}

				if( typeName == "goal" )
				{
					//always grounded
					string airStr;
					is >> airStr;

					int terrainIndex;
					is >> terrainIndex;

					int edgeIndex;
					is >> edgeIndex;

					double edgeQuantity;
					is >> edgeQuantity;

					int testIndex = 0;
					TerrainPolygon *terrain = NULL;
					for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
					{
						if( testIndex == terrainIndex )
						{
							terrain = (*it);
							break;
						}
						testIndex++;
					}

					if( terrain == NULL )
						assert( 0 && "failure terrain indexing goal" );

					if( edgeIndex == terrain->points.size() - 1 )
						edgeIndex = 0;
					else
						edgeIndex++;

					a = new GoalParams( this, terrain, edgeIndex, edgeQuantity );
					//a->SetAsGoal( terrain, edgeIndex, edgeQuantity );
				}
				else if( typeName == "patroller" )
				{
					Vector2i pos;

					string airStr;
					is >> airStr;

					if( airStr == "+air" )
					{
						is >> pos.x;
						is >> pos.y;
					}
					else
					{
						assert( false && "air wrong" );
					}

					int pathLength;
					is >> pathLength;
					
					list<Vector2i> globalPath;
					globalPath.push_back( Vector2i( pos.x, pos.y ) );

					for( int i = 0; i < pathLength; ++i )
					{
						int localX,localY;
						is >> localX;
						is >> localY;
						globalPath.push_back( Vector2i( pos.x + localX, pos.y + localY ) );
					}


					bool loop;
					string loopStr;
					is >> loopStr;
					if( loopStr == "+loop" )
						loop = true;
					else if( loopStr == "-loop" )
						loop = false;
					else
						assert( false && "should be a boolean" );


					float speed;
					is >> speed;

					//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
					a = new PatrollerParams( this, pos, globalPath, speed, loop );
				}
				else if( typeName == "crawler" )
				{

					//always grounded
					string airStr;
					is >> airStr;

					int terrainIndex;
					is >> terrainIndex;

					int edgeIndex;
					is >> edgeIndex;

					

					double edgeQuantity;
					is >> edgeQuantity;

					bool clockwise;
					string cwStr;
					is >> cwStr;

					if( cwStr == "+clockwise" )
						clockwise = true;
					else if( cwStr == "-clockwise" )
						clockwise = false;
					else
					{
						assert( false && "boolean problem" );
					}

					float speed;
					is >> speed;

					int testIndex = 0;
					TerrainPolygon *terrain = NULL;
					for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
					{
						if( testIndex == terrainIndex )
						{
							terrain = (*it);
							break;
						}
						testIndex++;
					}

					if( terrain == NULL )
						assert( 0 && "failure terrain indexing crawler" );

					if( edgeIndex == terrain->points.size() - 1 )
						edgeIndex = 0;
					else
						edgeIndex++;

					//a->SetAsCrawler( at, terrain, edgeIndex, edgeQuantity, clockwise, speed ); 
					a = new CrawlerParams( this, terrain, edgeIndex, edgeQuantity, clockwise, speed ); 
				}
				else if( typeName == "basicturret" )
				{
					//always grounded
					string airStr;
					is >> airStr;

					int terrainIndex;
					is >> terrainIndex;

					int edgeIndex;
					is >> edgeIndex;

					double edgeQuantity;
					is >> edgeQuantity;

					double bulletSpeed;
					is >> bulletSpeed;

					int framesWait;
					is >> framesWait;

					int testIndex = 0;
					TerrainPolygon *terrain = NULL;
					for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
					{
						if( testIndex == terrainIndex )
						{
							terrain = (*it);
							break;
						}
						testIndex++;
					}

					if( terrain == NULL )
						assert( 0 && "failure terrain indexing basicturret" );

					if( edgeIndex == terrain->points.size() - 1 )
						edgeIndex = 0;
					else
						edgeIndex++;

					//a->SetAsBasicTurret( at, terrain, edgeIndex, edgeQuantity, bulletSpeed, framesWait );
					a = new BasicTurretParams( this, terrain, edgeIndex, edgeQuantity, bulletSpeed, framesWait );
				}
				else if( typeName == "foottrap" )
				{
					//always grounded
					string airStr;
					is >> airStr;

					int terrainIndex;
					is >> terrainIndex;

					int edgeIndex;
					is >> edgeIndex;

					double edgeQuantity;
					is >> edgeQuantity;

					int testIndex = 0;
					TerrainPolygon *terrain = NULL;
					for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
					{
						if( testIndex == terrainIndex )
						{
							terrain = (*it);
							break;
						}
						testIndex++;
					}

					if( terrain == NULL )
						assert( 0 && "failure terrain indexing foottrap" );

					if( edgeIndex == terrain->points.size() - 1 )
						edgeIndex = 0;
					else
						edgeIndex++;

					//a->SetAsFootTrap( at, terrain, edgeIndex, edgeQuantity );
					a = new FootTrapParams( this, terrain, edgeIndex, edgeQuantity );
				}
				else
				{
					assert( false && "unkown enemy type!" );
				}
				
				gr->actors.push_back( a );
				a->group = gr;
			}
		}



		is.close();




	}
	else
	{

		//new file
		cout << "filename: " << fileName << endl;
		assert( false && "error getting file to edit " );
	}

	grassTex.loadFromFile( "newgrass2.png" );
	
}

void EditSession::WriteFile(string fileName)
{
	bool hasGoal = false;
	for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it )
	{
		ActorGroup *group = (*it).second;
		for( list<ActorParams*>::iterator it2 = group->actors.begin(); it2 != group->actors.end(); ++it2 )
		{
			if( (*it2)->type == types["goal"] )
			{
				hasGoal = true;
				break;
			}
		}
	}

	if( !hasGoal )
	{
		cout << "you need to place a goal in the map. file not written to!. add a popup to this alert later"
			<< endl;
		return;
	}



	ofstream of;
	of.open( fileName );//+ ".brknk" );

	int pointCount = 0;
	int movingPlatCount = 0;
	for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		
		

		if( (*it)->path.size() == 0 )
			pointCount += (*it)->points.size();
		else
			movingPlatCount++;
	}

	

	of << pointCount << endl;
	of << playerPosition.x << " " << playerPosition.y << endl;

	int writeIndex = 0;
	for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		if( (*it)->path.size() < 2 )
		{
			(*it)->writeIndex = writeIndex;
			++writeIndex;

			of << (*it)->material << endl;

			of <<  (*it)->points.size() << endl;

			for( PointList::iterator it2 = (*it)->points.begin(); it2 != (*it)->points.end(); ++it2 )
			{
				of << (*it2).pos.x << " " << (*it2).pos.y << endl; // << " " << (int)(*it2).special << endl;
			}

			WriteGrass( (*it), of );
		}
	}	

	of << movingPlatCount << endl;

	writeIndex = 0;
	for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		if( (*it)->path.size() > 1 )
		{
			(*it)->writeIndex = writeIndex;
			++writeIndex;

			of << (*it)->material << endl;
			
			of <<  (*it)->points.size() << endl;

			for( PointList::iterator it2 = (*it)->points.begin(); it2 != (*it)->points.end(); ++it2 )
			{
				of << (*it2).pos.x << " " << (*it2).pos.y << endl;
			}


			of << (*it)->path.size() - 1 << endl;
		
			list<Vector2i>::iterator pathit = (*it)->path.begin();
			++pathit;

			for( ; pathit != (*it)->path.end(); ++pathit )
			{
				of << (*pathit).x << " " << (*pathit).y << endl;
			}	
		}
	}

	

	of << lights.size() << endl;
	for( list<StaticLight*>::iterator it = lights.begin(); it != lights.end(); ++it )
	{
		(*it)->WriteFile( of );
	}

	of << groups.size() << endl;
	//write the stuff for goals and remove them from the enemy stuff

	for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it )
	{
		(*it).second->WriteFile( of );
		//(*it).second->( w );
	}

	//enemies here


}

void EditSession::WriteGrass( TerrainPolygon* poly, ofstream &of )
{
	int edgesWithSegments = 0;

	VertexArray &grassVa = *poly->grassVA;
	double grassSize = 22;
	double radius = grassSize / 2;
	double grassSpacing = -5;

	int edgeIndex = 0;
	int i = 0;
	list<list<GrassSeg>> grassListList;
	for( PointList::iterator it = poly->points.begin(); it != poly->points.end(); ++it )
	{
		Vector2i next;

		PointList::iterator temp = it;
		if( ++temp == poly->points.end() )
		{
			next = poly->points.front().pos;
		}
		else
		{
			next = (*temp).pos;
		}

		V2d v0( (*it).pos.x, (*it).pos.y );
		V2d v1( next.x, next.y );

		double remainder = length( v1 - v0 ) / ( grassSize + grassSpacing );

		int num = floor( remainder ) + 1;

		grassListList.push_back( list<GrassSeg>() );

		list<GrassSeg> &grassList = grassListList.back();
		
		GrassSeg *gPtr = NULL;
		bool hasGrass = false;
		for( int j = 0; j < num; ++j )
		{
			//V2d pos = v0 + (v1 - v0) * ((double)(j )/ num);

			if( grassVa[i*4].color.a == 255 || grassVa[i*4].color.a == 254 )
			{
				hasGrass = true;
				if( gPtr == NULL )
				{
					grassList.push_back( GrassSeg( edgeIndex, j, 0 ) );
					gPtr = &grassList.back();
				}
				else
				{
					grassList.back().reps++;
				}
			}
			else
			{
				if( gPtr != NULL )
					gPtr = NULL;
			}
			
			++i;
		}

		if( hasGrass )
		{
			++edgesWithSegments;
		}

		++edgeIndex;

	}

	//cout << "saving edges with segments: " << edgesWithSegments << endl;
	of << edgesWithSegments << endl;

	for( list<list<GrassSeg>>::iterator it = grassListList.begin(); it != grassListList.end(); ++it )
	{
		int numSegments = (*it).size();

		if( numSegments > 0 )
		{
			int edgeIndex = (*it).front().edgeIndex;
			of << edgeIndex << " " << numSegments << endl;

			for( list<GrassSeg>::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2 )
			{
				of << (*it2).index << " "<< (*it2).reps << endl;
				//cout << "index: " << (*it2).index << ", reps: " << (*it2).reps << endl;
			}
		}
		
		
	}
}

void EditSession::Add( TerrainPolygon *brush, TerrainPolygon *poly )
{

	TerrainPolygon z( &grassTex );
	//1: choose start point

	Vector2i startPoint;
	bool startPointFound = false;
	bool firstPolygon = true;


	TerrainPolygon *currentPoly = NULL;
	TerrainPolygon *otherPoly = NULL;
	PointList::iterator it = poly->points.begin();
	PointList::iterator startIt;

	TerrainPolygon *minPoly = NULL;


	//get which polygon I should start on
	for(; it != poly->points.end(); ++it )
	{
		//if( !brush->ContainsPoint( Vector2f( (*it).pos.x, (*it).pos.y) ) )
		//{
			if( !startPointFound )
			{
				startPoint = (*it).pos;
				startIt = it;
				startPointFound = true;
			}
			else
			{
				if( (*it).pos.y > startPoint.y )
				{
					startPoint = (*it).pos;
					startIt = it;
				}
			}
		//}
	}
	
	it = brush->points.begin();
	for(; it != brush->points.end(); ++it )
	{
		//if( !poly->ContainsPoint( Vector2f( (*it).pos.x, (*it).pos.y) ) )
		if( !startPointFound )
		{
			startPoint = (*it).pos;
			startPointFound = true;
			firstPolygon = false;
			startIt = it;
		}
		else
		{
			if( (*it).pos.y > startPoint.y )
			{
				startPoint = (*it).pos;
				firstPolygon = false;
				startIt = it;
			}
		}
	}

	if( firstPolygon )
	{
		currentPoly = poly;
		otherPoly = brush;
	}
	else
	{
		currentPoly = brush;
		otherPoly = poly;
	}
	

	assert( startPointFound );
	


	it = startIt;
	Vector2i currPoint = startPoint;
	++it;
	if( it == currentPoly->points.end() )
	{
		it = currentPoly->points.begin();
	}
	Vector2i nextPoint = (*it).pos;


	//z.points.push_back( startPoint );

	//2. run a loobclockwise until you arrive back at the original state



	bool firstTime = true;

	while( firstTime || currPoint != startPoint )
	{
		/*CircleShape cs;
		cs.setRadius( 3 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setFillColor( Color::Magenta );
		cs.setPosition( currPoint.x, currPoint.y );*/
		
		//preScreenTex->clear();
		//this->Draw();
		//preScreenTex->draw( cs );

		//cs.setPosition( nextPoint.x, nextPoint.y );
		//cs.setFillColor( Color::Yellow );
		//preScreenTex->draw( cs );

		//preScreenTex->display();

		
		PointList::iterator min;
		Vector2i minIntersection;
		bool emptyInter = true;
		

		PointList::iterator otherIt = otherPoly->points.begin();
		Vector2i otherCurrPoint = (*otherIt).pos;
		++otherIt;
		Vector2i otherNextPoint;// = (*++bit);
		Vector2i minPoint;
		
		LineIntersection li1 = SegmentIntersect( currPoint, nextPoint, otherPoly->points.back().pos, otherCurrPoint );
		Vector2i lii1( floor(li1.position.x + .5), floor(li1.position.y + .5) );
		if( !li1.parallel && ( lii1 != currPoint && lii1 != nextPoint && lii1 != otherPoly->points.back().pos && lii1 != otherCurrPoint ) ) 
		{
			minIntersection = lii1;
			minPoint = otherCurrPoint;//otherCurrPoint;
			min = otherPoly->points.begin();//--otherIt;
			//++otherIt;
			emptyInter = false;
			//cout << "using this" << endl;
		}

		for(; otherIt != otherPoly->points.end(); ++otherIt )
		{
			otherNextPoint = (*otherIt).pos;
			LineIntersection li = LimitSegmentIntersect( currPoint, nextPoint, otherCurrPoint, otherNextPoint );
			Vector2i lii( floor(li.position.x + .5), floor(li.position.y + .5) );
			//cout << "li.par: " << li.parallel << ", others: lii: " << lii.x << ", " << lii.y << ", curr: " << currPoint.x << ", " << currPoint.y << endl;
			if( !li.parallel )//&& ( lii != currPoint && lii != nextPoint && lii != otherCurrPoint && lii != otherNextPoint ) ) //&& (abs( lii.x - currPoint.x ) >= 1 || abs( lii.y - currPoint.y ) >= 1 ))//&& length( li.position - V2d(currPoint.x, currPoint.y) ) >= 5 )
			{
				if( emptyInter )
				{
					emptyInter = false;
					minIntersection = lii;
					//minIntersection.x = //li.position.x;
					//minIntersection.y = //li.position.y;
					minPoint = otherNextPoint;
					min = otherIt;
				}
				else
				{
					V2d blah( minIntersection - currPoint );
					V2d blah2( lii - currPoint );
					//cout << "lengths: " << length( li.position - V2d(currPoint.x, currPoint.y) ) << ", " << length( V2d( blah.x, blah.y ) ) << endl;
					if( length( blah2 ) < length( blah ) )
					{
						minIntersection = lii;
						minPoint = otherNextPoint;
						min = otherIt;
					}
				}

					
			}
			otherCurrPoint = otherNextPoint;
			
		}

		if( !emptyInter )
		{

			if( currPoint == startPoint && !firstTime )
			{
			//	cout << "secondary break" << endl;
				break;
			}
			//cout << "switching polygon and adding point" << endl;
			
			//push back intersection
			TerrainPolygon *temp = currentPoly;
			currentPoly = otherPoly;
			otherPoly = temp;
			it = min;
			
			cout << "adding new intersection: " << minIntersection.x << ", " << minIntersection.y << endl;

			currPoint = minIntersection;

			z.points.push_back( TerrainPoint( currPoint, false ) );
			
			
			nextPoint = (*it).pos;
		}
		else
		{

			currPoint = (*it).pos;

			z.points.push_back( TerrainPoint( currPoint, false ) );

			cout << "adding point: " << currPoint.x << ", " << currPoint.y << endl;

			if( currPoint == startPoint && !firstTime )
				break;
			

			++it;
			if( it == currentPoly->points.end() )
			{
				it = currentPoly->points.begin();
			}
			nextPoint = (*it).pos;
	//		cout << "nextpoing from adding: " << nextPoint.x << ", " << nextPoint.y << endl;
		}
		firstTime = false;
	}

	poly->Reset();
	//cout << "poly size: " << z.points.size() << endl;
	for( PointList::iterator zit = z.points.begin(); zit != z.points.end(); ++zit )
	{
		poly->points.push_back( (*zit) );
	//	cout << (*zit).pos.x << ", " << (*zit).pos.y << endl;
	}
	//cout << "before killer finalize. poly size: " << poly->points.size() << endl;
	poly->Finalize();
}

LineIntersection EditSession::SegmentIntersect( Vector2i a, Vector2i b, Vector2i c, Vector2i d )
{
	LineIntersection li = lineIntersection( V2d( a.x, a.y ), V2d( b.x, b.y ), 
				V2d( c.x, c.y ), V2d( d.x, d.y ) );
	if( !li.parallel )
	{
		double e1Left = min( a.x, b.x );
		double e1Right = max( a.x, b.x );
		double e1Top = min( a.y, b.y );
		double e1Bottom = max( a.y, b.y );

		double e2Left = min( c.x, d.x );
		double e2Right = max( c.x, d.x );
		double e2Top = min( c.y, d.y );
		double e2Bottom = max( c.y, d.y );
		//cout << "compares: " << e1Left << ", " << e2Right << " .. " << e1Right << ", " << e2Left << endl;
		//cout << "compares y: " << e1Top << " <= " << e2Bottom << " && " << e1Bottom << " >= " << e2Top << endl;
		if( e1Left <= e2Right && e1Right >= e2Left && e1Top <= e2Bottom && e1Bottom >= e2Top )
		{
			//cout << "---!!!!!!" << endl;
			if( li.position.x <= e1Right && li.position.x >= e1Left && li.position.y >= e1Top && li.position.y <= e1Bottom)
			{
				if( li.position.x <= e2Right && li.position.x >= e2Left && li.position.y >= e2Top && li.position.y <= e2Bottom)
				{
					//cout << "seg intersect!!!!!!" << endl;
					//assert( 0 );
					return li;
				}
			}
		}
	}
	//cout << "return false" << endl;
	li.parallel = true;
	return li;
}

LineIntersection EditSession::LimitSegmentIntersect( Vector2i a, Vector2i b, Vector2i c, Vector2i d )
{
	LineIntersection li = lineIntersection( V2d( a.x, a.y ), V2d( b.x, b.y ), 
				V2d( c.x, c.y ), V2d( d.x, d.y ) );
	if( !li.parallel )
	{
		double e1Left = min( a.x, b.x );
		double e1Right = max( a.x, b.x );
		double e1Top = min( a.y, b.y );
		double e1Bottom = max( a.y, b.y );

		double e2Left = min( c.x, d.x );
		double e2Right = max( c.x, d.x );
		double e2Top = min( c.y, d.y );
		double e2Bottom = max( c.y, d.y );
		//cout << "compares: " << e1Left << ", " << e2Right << " .. " << e1Right << ", " << e2Left << endl;
		//cout << "compares y: " << e1Top << " <= " << e2Bottom << " && " << e1Bottom << " >= " << e2Top << endl;
		if( e1Left <= e2Right && e1Right >= e2Left && e1Top <= e2Bottom && e1Bottom >= e2Top )
		{
			//cout << "---!!!!!!" << endl;
			if( li.position.x <= e1Right && li.position.x >= e1Left && li.position.y >= e1Top && li.position.y <= e1Bottom)
			{
				if( li.position.x <= e2Right && li.position.x >= e2Left && li.position.y >= e2Top && li.position.y <= e2Bottom)
				{
					V2d &pos = li.position;
					if( length( li.position - V2d( a.x, a.y ) ) > 1 &&  length( li.position - V2d( b.x, b.y ) ) > 1
						&&  length( li.position - V2d( c.x, c.y ) ) > 1 &&  length( li.position - V2d( d.x, d.y ) ) > 1 )
					{
						return li;
					}
					//cout << "seg intersect!!!!!!" << endl;
					//assert( 0 );
					
				}
			}
		}
	}
	//cout << "return false" << endl;
	li.parallel = true;
	return li;
}

int EditSession::Run( string fileName, Vector2f cameraPos, Vector2f cameraSize )
{
	uiView = View( sf::Vector2f( 960, 540 ), sf::Vector2f( 1920, 1080 ) );
	v.setCenter( 0, 0 );
	v.setSize( 1920/ 2, 1080 / 2 );
	w->setView( v );

	confirm = CreatePopupPanel( "confirmation" );
	popupPanel = NULL;
	validityRadius = 4;

	extendingPolygon = NULL;
	extendingPoint = NULL;

	radiusOption = false;
	lightPosDown = false;

	showGrass = false;

	pointGrab = false;
	polyGrab = false;
	makingRect = false;

	bool showGraph = false;

	selectedActor = NULL;
	selectedLight = NULL;
	selectedLightGrabbed = false;

	trackingEnemy = NULL;
	showPanel = NULL;

	sf::Font arial;
	arial.loadFromFile( "arial.ttf" );



	sf::Texture playerZoomIconTex;
	playerZoomIconTex.loadFromFile( "playerzoomicon.png" );
	sf::Sprite playerZoomIcon( playerZoomIconTex );
	
	playerZoomIcon.setOrigin( playerZoomIcon.getLocalBounds().width / 2, playerZoomIcon.getLocalBounds().height / 2 );

//	Panel p( 300, 300, this );
//	p.active = true;
//	p.AddButton( Vector2i( 50, 100 ), Vector2f( 50, 50 ), "LOL");
//	p.AddTextBox( Vector2i( 200, 200 ), 200, 15, "testing" );

	ActorGroup *emptyGroup = new ActorGroup( "--" );
	//emptyGroup->name = "";
	groups[emptyGroup->name] = emptyGroup;


	Panel *mapOptionsPanel = CreateOptionsPanel( "map" );
	Panel *terrainOptionsPanel = CreateOptionsPanel( "terrain" );

	Panel *patrollerPanel = CreateOptionsPanel( "patroller" );//new Panel( 300, 300, this );
	ActorType *patrollerType = new ActorType( "patroller", patrollerPanel );

	Panel *crawlerPanel = CreateOptionsPanel( "crawler" );
	ActorType *crawlerType = new ActorType( "crawler", crawlerPanel );

	Panel *basicTurretPanel = CreateOptionsPanel( "basicturret" );
	ActorType *basicTurretType = new ActorType( "basicturret", basicTurretPanel );

	Panel *footTrapPanel = CreateOptionsPanel( "foottrap" );
	ActorType *footTrapType = new ActorType( "foottrap", footTrapPanel );

	Panel *goalPanel = CreateOptionsPanel( "goal" );
	ActorType *goalType = new ActorType( "goal", goalPanel );

	Panel *lightPanel = CreateOptionsPanel( "light" );

	messagePopup = CreatePopupPanel( "message" );
	errorPopup = CreatePopupPanel( "error" );

	types["patroller"] = patrollerType;
	types["crawler"] = crawlerType;
	types["basicturret"] = basicTurretType;
	types["foottrap"] = footTrapType;
	types["goal"] = goalType;


	GridSelector gs( 3, 2, 32, 32, this );
	gs.active = false;

	sf::Sprite s0( patrollerType->iconTexture );
	sf::Sprite s1( crawlerType->iconTexture );
	sf::Sprite s2( basicTurretType->iconTexture );
	sf::Sprite s3( footTrapType->iconTexture );
	sf::Sprite s4( goalType->iconTexture );


	gs.Set( 0, 0, s0, "patroller" );
	gs.Set( 1, 0, s1, "crawler" );
	gs.Set( 0, 1, s2, "basicturret" );
	gs.Set( 1, 1, s3, "foottrap" );
	gs.Set( 2, 0, s4, "goal" );

	int returnVal = 0;
	w->setMouseCursorVisible( true );
	Color testColor( 0x75, 0x70, 0x90 );
	view = View( cameraPos, cameraSize );
	if( cameraSize.x == 0 && cameraSize.y == 0 )
		view.setSize( 1920, 1080 );

	preScreenTex->setView( view );
	Texture playerTex;
	playerTex.loadFromFile( "stand.png" );
	sf::Sprite playerSprite( playerTex );

	//Texture goalTex;
	//goalTex.loadFromFile( "goal.png" );
	//Sprite goalSprite( goalTex );

	/*sf::Texture iconsTex;
	iconsTex.loadFromFile( "editoricons.png" );
	Sprite iconSprite( iconsTex );*/

	sf::Texture alphaTex;
	alphaTex.loadFromFile( "alphatext.png" );
	sf::Sprite alphaTextSprite( alphaTex );
	alphaTextSprite.setOrigin( alphaTextSprite.getLocalBounds().width / 2, alphaTextSprite.getLocalBounds().height / 2 );

	
	sf::Vector2u wSize = w->getSize();
	
	//sf::View uiView( Vector2f( wSize.x / 2, wSize.y / 2 ), Vector2f( wSize.x, wSize.y ) );

	//goalSprite.setOrigin( goalSprite.getLocalBounds().width / 2, goalSprite.getLocalBounds().height / 2 );

	playerSprite.setTextureRect( IntRect(0, 0, 64, 64 ) );
	playerSprite.setOrigin( playerSprite.getLocalBounds().width / 2, playerSprite.getLocalBounds().height / 2 );

	w->setVerticalSyncEnabled( true );

	OpenFile( fileName );


//	ActorParams *ap = new ActorParams;
//	ap->CreatePatroller( patrollerType, Vector2i( playerPosition.x, playerPosition.y ), true, 10 );
//	groups["--"]->actors.push_back( ap );
	//ap->CreatePatroller( 



	//Vector2f vs(  );
	if( cameraSize.x == 0 && cameraSize.y == 0 )
		view.setCenter( (float)playerPosition.x, (float)playerPosition.y );

	//mode = "neutral";
	bool quit = false;
	polygonInProgress = new TerrainPolygon(&grassTex );
	zoomMultiple = 1;
	Vector2<double> prevWorldPos;
	Vector2i pixelPos;
	Vector2f tempWorldPos = preScreenTex->mapPixelToCoords(sf::Mouse::getPosition( *w ));
	Vector2<double> worldPos = Vector2<double>( tempWorldPos.x, tempWorldPos.y );
	bool panning = false;
	Vector2<double> panAnchor;
	minimumEdgeLength = 8;

	Color borderColor = sf::Color::Green;
	int max = 1000000;
	sf::Vertex border[] =
	{
		sf::Vertex(sf::Vector2<float>(-max, -max), borderColor ),
		sf::Vertex(sf::Vector2<float>(-max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(-max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, -max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, -max), borderColor),
		sf::Vertex(sf::Vector2<float>(-max, -max), borderColor)
	};

	sf::Texture guiMenuCubeTexture;
	guiMenuCubeTexture.loadFromFile( "guioptions.png" );
	sf::Sprite guiMenuSprite;
	guiMenuSprite.setTexture( guiMenuCubeTexture );
	guiMenuSprite.setOrigin( guiMenuSprite.getLocalBounds().width / 2, guiMenuSprite.getLocalBounds().height / 2 );

	Color graphColor = Color( 200, 50, 50, 100 );
	//int max = 1000000;
	int numLines = 30;
	sf::VertexArray graphLines( sf::Lines, numLines * 8 );
	int graphSep = 32;
	int graphMax = graphSep * numLines;
	int temp = -graphMax;

	//horiz
	for( int i = 0; i < numLines * 4; i += 2 )
	{
		graphLines[i] = sf::Vertex(sf::Vector2<float>(-graphMax, temp), graphColor );
		graphLines[i+1] = sf::Vertex(sf::Vector2<float>(graphMax, temp), graphColor );
		temp += graphSep;
	}

	//vert
	temp = -graphMax;
	for( int i = numLines * 4; i < numLines * 8; i += 2 )
	{
		graphLines[i] = sf::Vertex(sf::Vector2<float>(temp, -graphMax), graphColor );
		graphLines[i+1] = sf::Vertex(sf::Vector2<float>(temp, graphMax), graphColor );
		temp += graphSep;
	}
	

	bool s = sf::Keyboard::isKeyPressed( sf::Keyboard::T );
	
	V2d menuDownPos;
	Emode menuDownStored;

	mode = CREATE_TERRAIN;
	Emode stored = mode;
	bool canCreatePoint = true;
	gs.active = true;

	


	double circleDist = 100;
	double circleRadius = 50;

	V2d topPos =  V2d( 0, -1 ) * circleDist;
	V2d upperRightPos = V2d( sqrt( 3.0 ) / 2, -.5 ) * circleDist;
	V2d lowerRightPos = V2d( sqrt( 3.0 ) / 2, .5 ) * circleDist;

	V2d upperLeftPos = V2d( -sqrt( 3.0 ) / 2, -.5 ) * circleDist;
	V2d lowerLeftPos = V2d( -sqrt( 3.0 ) / 2, .5 ) * circleDist;

	V2d bottomPos = V2d( 0, 1 ) * circleDist;

	string menuSelection = "";

	selectedPlayer = false;
	selectedActorGrabbed = false;
	selectedLightGrabbed = false;

	for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		for( PointList::iterator it2 = (*it)->points.begin(); it2 != (*it)->points.end(); ++it2 )
		{
			//(*it2).grass.push_back( GrassSeg( 0, 0 ) );
		}
	}

	while( !quit )
	{
		
		prevWorldPos = worldPos;
		pixelPos = sf::Mouse::getPosition( *w );
		Vector2f tempWorldPos = preScreenTex->mapPixelToCoords(pixelPos);
		worldPos.x = tempWorldPos.x;
		worldPos.y = tempWorldPos.y;

		preScreenTex->setView( uiView );
		Vector2f uiMouse = preScreenTex->mapPixelToCoords( pixelPos );
		
		preScreenTex->setView( view );
		

		testPoint.x = worldPos.x;
		testPoint.y = worldPos.y;
		
		sf::Event ev;
		while( w->pollEvent( ev ) )
		{
			switch( mode )
			{
			case CREATE_TERRAIN:
				{
					switch( ev.type )
					{
					case Event::MouseButtonPressed:
						{
							if( ev.mouseButton.button == Mouse::Left )
							{
								if( popupPanel != NULL )
								{
									popupPanel->Update( true, uiMouse.x, uiMouse.y );
								}
								else if( showPanel != NULL )
								{	
									showPanel->Update( true, uiMouse.x, uiMouse.y );
									break;
								}
							}
							
							break;
						}
					case Event::MouseButtonReleased:
						{
							if( popupPanel != NULL )
							{
								popupPanel->Update( false, uiMouse.x, uiMouse.y );
							}
							else if( showPanel != NULL )
							{	
								showPanel->Update( false, uiMouse.x, uiMouse.y );
							}
							break;
						}
					case Event::MouseWheelMoved:
						{
							break;
						}
					case Event::KeyPressed:
						{
							if( popupPanel != NULL )
							{
								popupPanel->SendKey( ev.key.code, ev.key.shift );
								break;
							}

							if( showPanel != NULL )
							{
								showPanel->SendKey( ev.key.code, ev.key.shift );
								break;
							}


							if( ev.key.code == Keyboard::Space )
							{
							//	ConfirmationPop();
								MessagePop( "hello" );
								if( showPoints && extendingPolygon )
								{
								}
								else
								{
								if( polygonInProgress->points.size() > 2 )
								{
									//test final line
									bool valid = true;


									PointList::iterator testIt = polygonInProgress->points.begin();
									PointList::iterator prevIt = testIt;
									testIt++;

									//cout << "lastline: " << polygonInProgress->points.back().pos.x << ", " << polygonInProgress->points.back().pos.y <<
									//	" .. " << polygonInProgress->points.front().pos.x << ", " << polygonInProgress->points.front().pos.y << endl;
									for( ; testIt != polygonInProgress->points.end(); ++testIt )
									{
										Vector2i a = (*prevIt).pos;
										Vector2i b = (*testIt).pos;
										Vector2i c = polygonInProgress->points.back().pos;
										Vector2i d = polygonInProgress->points.front().pos;
										LineIntersection li = LimitSegmentIntersect( a,b,c,d );
										Vector2i lii( floor(li.position.x + .5), floor(li.position.y + .5) );
										//if( !li.parallel  && (abs( lii.x - currPoint.x ) >= 1 || abs( lii.y - currPoint.y ) >= 1 ))
										if( !li.parallel )//(abs( lii.x - currPoint.x ) > 1 || abs( lii.y - currPoint.y ) > 1 ) )//&& lii != a && lii != b && lii != c && lii != d )
										{
											valid = false;
										}

										

										prevIt = testIt;
									}


									for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
									{
										//if( !PointValid( polygonInProgress->points.back().pos, polygonInProgress->points.front().pos, (*it) ) )
										if( !IsPointValid( polygonInProgress->points.back().pos, polygonInProgress->points.front().pos, (*it) ) )
										{
											valid = false;
											break;
										}
									}
									
									if( !valid )
									{
										popupPanel = messagePopup;
										break;
									}

									//if( !PointValid( polygonInProgress->points.back().pos, polygonInProgress->points.front().pos ) )
									//	break;

									list<TerrainPolygon*>::iterator it = polygons.begin();
									bool added = false;
									polygonInProgress->Finalize();
									bool recursionDone = false;
									TerrainPolygon *currentBrush = polygonInProgress;

										while( it != polygons.end() )
										{
											TerrainPolygon *temp = (*it);
											if( temp != currentBrush && currentBrush->IsTouching( temp ) )
											{
												cout << "before addi: " << (*it)->points.size() << endl;
						
												Add( currentBrush, temp );

												polygonInProgress->Reset();
						
												cout << "after adding: " << (*it)->points.size() << endl;
												polygons.erase( it );

												currentBrush = temp;

												it = polygons.begin();

												added = true;
							
												continue;
											}
											else
											{
												//cout << "not" << endl;
											}
											++it;
										}
				
									//add final check for validity here
				
									if( !added )
									{
										polygonInProgress->Finalize();
										polygons.push_back( polygonInProgress );
										polygonInProgress = new TerrainPolygon(&grassTex );
									}
									else
									{

										polygons.push_back( currentBrush );
										polygonInProgress->Reset();
									}
								}

								if( polygonInProgress->points.size() <= 2 && polygonInProgress->points.size() > 0  )
								{
									cout << "cant finalize. cant make polygon" << endl;
									polygonInProgress->points.clear();
								}
								}
							}
							else if( ev.key.code == sf::Keyboard::V || ev.key.code == sf::Keyboard::Delete )
							{
								//cout << "PRESSING V: " << polygonInProgress->points.size() << endl;
								if( polygonInProgress->points.size() > 0 )
								{
									polygonInProgress->points.pop_back();
								}
								/*else if( mode == SELECT_POLYGONS )
								{
									list<TerrainPolygon*>::iterator it = polygons.begin();
									while( it != polygons.end() )
									{
										if( (*it)->selected )
										{
											delete (*it);
											polygons.erase( it++ );
										}
										else
											++it;
									}
								}*/
							}
							else if( ev.key.code == sf::Keyboard::E )
							{
								if( !showPoints )
								{
									showPoints = true;
									extendingPolygon = NULL;
									extendingPoint = NULL;
									polygonInProgress->points.clear();
								}
							}
							
							break;
						}
					case Event::KeyReleased:
						{
							if( popupPanel != NULL )
							{
								break;
							}


							if( ev.key.code == sf::Keyboard::E )
							{
								showPoints = false;
								extendingPolygon = NULL;
								extendingPoint = NULL;
								polygonInProgress->points.clear();
							}
							break;
						}
					case Event::LostFocus:
						{
							break;
						}
					case Event::GainedFocus:
						{
							break;
						}
					}
					break;	
				}
			case EDIT:
				{
					switch( ev.type )
					{
					case Event::MouseButtonPressed:
						{
							if( ev.mouseButton.button == Mouse::Left )
							{
								if( popupPanel != NULL )
								{
									popupPanel->Update( true, uiMouse.x, uiMouse.y );
									break;
								}

								if( showPanel != NULL )
								{	
									//cout << "edit mouse update" << endl;
									showPanel->Update( true, uiMouse.x, uiMouse.y );
									break;
								}

								if( showGrass )
									break;

								if( sf::Keyboard::isKeyPressed( Keyboard::F ) )
								{
									StaticLight *closest = NULL;
									bool foundAny = false;
									for( list<StaticLight*>::iterator it = lights.begin(); it != lights.end(); ++it )
									{
										if( (*it)->GetAABB().contains( worldPos.x, worldPos.y ) )
										{
											if( !foundAny )
											{
												foundAny = true;
												closest = (*it);
											}
											else
											{
												if( length( V2d( closest->position.x, closest->position.y ) 
													- worldPos ) > length( V2d( (*it)->position.x, (*it)->position.y ) 
													- worldPos ) )
												{
													closest = (*it);
												}
											}
										}	
									}

									if( closest == NULL )
									{
										selectedLightGrabbed = false;
										selectedLight = NULL;
										break;
									}

									selectedLightGrabbed = true;
									lightGrabPos = Vector2i( worldPos.x, worldPos.y );

									selectedLight = closest;
									selectedActor = NULL;
									selectedPlayer = false;

									for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
										it != selectedPolygons.end(); ++it )
									{
										(*it)->SetSelected( false );

									}
									selectedPolygons.clear();


									break;
									//if( selectedLight
									
								}
								else
								{
									selectedLightGrabbed = false;
									selectedLight = NULL;
								}

								if( playerSprite.getGlobalBounds().contains( worldPos.x, worldPos.y ) )
								{
									selectedActor = NULL;
									selectedLight = NULL;
									selectedLightGrabbed = false;
									selectedPlayer = true;
									grabPlayer = true;
									grabPos = Vector2i( worldPos.x, worldPos.y );
									
									break;
								}
								else
								{
									grabPlayer = false;
									selectedPlayer = false;
								}

								bool emptySpace = true;

								for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
									it != selectedPolygons.end(); ++it )
								{
									for( PointList::iterator it2 = (*it)->points.begin(); 
										it2 != (*it)->points.end(); ++it2 )
									{
										if( length( worldPos - V2d( (*it2).pos.x, (*it2).pos.y ) ) < 8 * zoomMultiple )
										{
											if( (*it2).selected ) //selected 
											{
												(*it2).selected = false;
												emptySpace = false;
												break;
											}
											else
											{
												if( Keyboard::isKeyPressed( Keyboard::LShift ) )
												{
													
												}
												else
												{
													for( PointList::iterator tempIt = (*it)->points.begin();
														tempIt != (*it)->points.end(); ++tempIt )
													{
														(*tempIt).selected = false;
													}
												}

												(*it2).selected = true;
												emptySpace = false;
												break;


											}
											
											
										}
									}
								}

								
								if( emptySpace )
								{
									for( list<TerrainPolygon*>::iterator it = polygons.begin(); 
										it != polygons.end(); ++it )
									{
											if((*it)->ContainsPoint( Vector2f(worldPos.x, worldPos.y ) ) )
											{
												emptySpace = false;
												//(*it)->SetSelected( !((*it)->selected ) );
												if( (*it)->selected )
												{
													//selectedPolygons.push_back( (*it) );
													selectedPolygons.remove( (*it ) );
													(*it)->SetSelected( false );
												}
												else
												{
													if( sf::Keyboard::isKeyPressed( Keyboard::LShift ) )
													{
														selectedPolygons.push_back( (*it) );
														(*it)->SetSelected( true );
													}
													else
													{
														for( list<TerrainPolygon*>::iterator selIt = 
															selectedPolygons.begin(); 
															selIt != selectedPolygons.end(); ++selIt )
														{
															(*selIt)->SetSelected( false );
														}
														selectedPolygons.clear();
														selectedPolygons.push_back( (*it) );
														(*it)->SetSelected( true );
													}
													//selectedPolygons.remove( (*it ) );
												}
												break;
											}
									}
								}

								if( emptySpace )
								{
									for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
										it != selectedPolygons.end(); ++it )
									{
										(*it)->SetSelected( false );

									}
									selectedPolygons.clear();
								}
								
							//	cout << "here before loop" << endl;
								bool empty = true;
								for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end() && empty; ++it )
								{
									list<ActorParams*> &actors = it->second->actors;
									for( list<ActorParams*>::iterator it2 = actors.begin(); it2 != actors.end() && empty; ++it2 )
									{
										sf::FloatRect bounds = (*it2)->image.getGlobalBounds();
										if( bounds.contains( Vector2f( worldPos.x, worldPos.y ) ) )
										{
											selectedActor = (*it2);
											selectedActorGrabbed = true;
											grabPos = Vector2i( worldPos.x, worldPos.y );

											empty = false;
											//cout << "enemy selected" << endl;

											for( list<TerrainPolygon*>::iterator it3 = selectedPolygons.begin(); 
												it3 != selectedPolygons.end(); ++it3 )
											{
												(*it3)->SetSelected( false );
											}
											selectedPolygons.clear();
										}
									}
								}


							//	cout << "made it!!!" << endl;
								if( empty )
								{
									selectedActor = NULL;
									selectedLight = NULL;
									selectedActorGrabbed = false;
								}

							}
							break;
						}
					case Event::MouseButtonReleased:
						{
							if( ev.mouseButton.button == Mouse::Left )
							{
								if( popupPanel != NULL )
								{
									popupPanel->Update( false, uiMouse.x, uiMouse.y );
									break;
								}

								if( showPanel != NULL )
								{	
									showPanel->Update( false, uiMouse.x, uiMouse.y );
									break;
								}

								if( showGrass )
								{
									for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
									{
										(*it)->UpdateGrass();
									}
									//showGrass = false;
								}



								grabPlayer = false;
								selectedActorGrabbed = false;
								selectedLightGrabbed = false;
							}
							break;
						}
					case Event::MouseWheelMoved:
						{
							break;
						}
					case Event::KeyPressed:
						{
							if( popupPanel != NULL )
							{
								popupPanel->SendKey( ev.key.code, ev.key.shift );
								break;
							}

							if( showPanel != NULL )
							{
								showPanel->SendKey( ev.key.code, ev.key.shift );
								break;
							}


							if( ev.key.code == Keyboard::V || ev.key.code == Keyboard::Delete )
							{
								if( CountSelectedPoints() > 0 )
								{
									bool removeSuccess = IsRemovePointsOkay();

									if( removeSuccess )
									{
										for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
											it != selectedPolygons.end(); ++it )
										{
											(*it)->RemoveSelectedPoints();
										}
									}
									else
									{
										messagePopup->labels["message"]->setString( "problem removing points" );
										popupPanel = messagePopup;
									}
								}
								else if( selectedActor != NULL )
								{
									if( selectedActor->ground != NULL )
									{
										selectedActor->ground->enemies.remove( selectedActor );
									}
									selectedActor->group->actors.remove( selectedActor );
									delete selectedActor;
									
									selectedActor = NULL;
								}
								else if( selectedLight != NULL )
								{
									lights.remove( selectedLight );
									delete selectedLight;
									selectedLight = NULL;
									selectedLightGrabbed = false;
								}
								else
								{
									
									for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin();
										it != selectedPolygons.end(); ++it )
									{
										polygons.remove( (*it) );
										delete (*it);
									}
									selectedPolygons.clear();

									cout << "destroying terrain. eney: " << selectedActor << endl;
								}
							}
							else if( ev.key.code == Keyboard::W )
							{
								int countPoints = CountSelectedPoints();
								if( countPoints > 0 && !pointGrab )
								{
									pointGrab = true;

									
									if( Keyboard::isKeyPressed( Keyboard::G ) )
									{
										V2d graphPos = GraphPos( worldPos );
										pointGrabPos = Vector2i( graphPos.x, graphPos.y );
										//pointGrabPos = Vector2i( worldPos.x, worldPos.y );
									}
									else
									{
										pointGrabPos = Vector2i( worldPos.x, worldPos.y );
									}
								}
								else if( selectedPolygons.size() > 0 )
								{
									polyGrab = true;

									if( Keyboard::isKeyPressed( Keyboard::G ) )
									{
										V2d graphPos = GraphPos( worldPos );
										polyGrabPos = Vector2i( graphPos.x, graphPos.y );
									}
									else
									{
										polyGrabPos = Vector2i( worldPos.x, worldPos.y );
									}
								}
							}
							else if( ev.key.code == Keyboard::Q )
							{
								if( !makingRect )
								{
									makingRect = true;
									rectStart = Vector2i( worldPos.x, worldPos.y );
								}
							}
							else if( ev.key.code == Keyboard::R )
							{
								showGrass = true;
								for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
								{
									(*it)->ShowGrass( true );
								}
							}
							break;
						}
					case Event::KeyReleased:
						{
							if( popupPanel != NULL )
							{
								break;
							}


							if( ev.key.code == Keyboard::W )
							{
								pointGrab = false;
								polyGrab = false;
							}
							else if( ev.key.code == Keyboard::Q )
							{
								makingRect = false;

								int xDiff = ((int)worldPos.x) - rectStart.x;
								int yDiff = ((int)worldPos.y) - rectStart.y;

								if( abs(xDiff) > 10 && abs( yDiff) > 10 )
								{
									int left, top, width, height;
									if( xDiff > 0 )
									{
										left = rectStart.x;
										width = xDiff;
									}
									else
									{
										left = (int)worldPos.x;
										width = -xDiff;
									}

									if( yDiff > 0 )
									{
										top = rectStart.y;
										height = yDiff;
									}
									else
									{
										top = (int)worldPos.y;
										height = -yDiff;
									}

									sf::Rect<float> selectRect = sf::Rect<float>( left, top, width, height );

									/*if( playerSprite.getGlobalBounds().contains( worldPos.x, worldPos.y ) )
									{
										selectedActor = NULL;
										selectedPlayer = true;
										grabPlayer = true;
										grabPos = Vector2i( worldPos.x, worldPos.y );
									
										break;
									}
									else
									{
										grabPlayer = false;
										selectedPlayer = false;
									}*/

									bool emptySpace = true;

									for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
										it != selectedPolygons.end(); ++it )
									{
										for( PointList::iterator it2 = (*it)->points.begin(); 
											it2 != (*it)->points.end(); ++it2 )
										{
											if( selectRect.contains( Vector2f( (*it2).pos.x, (*it2).pos.y ) ) )
											{
												if( (*it2).selected ) //selected 
												{
													if( Keyboard::isKeyPressed( Keyboard::LShift ) )
													{
														(*it2).selected = false;
													}
													else
													{
														emptySpace = false;
													}
													
													//break;
												}
												else
												{
													if( Keyboard::isKeyPressed( Keyboard::LShift ) )
													{
													
													}
													else
													{
														//for( PointList::iterator tempIt = (*it)->points.begin();
														//	tempIt != (*it)->points.end(); ++tempIt )
														//{
														//	(*tempIt).second = false;
														//}
													}

													(*it2).selected = true;
													emptySpace = false;
												//	break;


												}
											
											
											}
											else
											{
												if( Keyboard::isKeyPressed( Keyboard::LShift ) )
												{

												}
												else
												{
													(*it2).selected = false;
												}
											}
										}
									}

									TerrainPolygon tempRectPoly(&grassTex );
									tempRectPoly.points.push_back( TerrainPoint(Vector2i( selectRect.left, selectRect.top ), false ) );
									tempRectPoly.points.push_back( TerrainPoint(Vector2i( selectRect.left + selectRect.width, selectRect.top ), false ) );
									tempRectPoly.points.push_back( TerrainPoint(Vector2i( selectRect.left + selectRect.width, selectRect.top + selectRect.height ), false ) );
									tempRectPoly.points.push_back( TerrainPoint(Vector2i( selectRect.left, selectRect.top + selectRect.height ), false ) );
									tempRectPoly.Finalize();


									if( emptySpace )
									{
										for( list<TerrainPolygon*>::iterator it = polygons.begin(); 
											it != polygons.end(); ++it )
										{
												
												if( tempRectPoly.IsTouching( (*it) )
													|| (
													(*it)->left >= tempRectPoly.left
													&& (*it)->right <= tempRectPoly.right
													&& (*it)->bottom <= tempRectPoly.bottom
													&& (*it)->top >= tempRectPoly.top
													)
													)
												//if((*it)->ContainsPoint( Vector2f(worldPos.x, worldPos.y ) ) )
												{
													emptySpace = false;
													
													if( (*it)->selected )
													{
														if( sf::Keyboard::isKeyPressed( Keyboard::LShift ) )
														{
														}
														else
														{
														//	selectedPolygons.remove( (*it ) );
														//	(*it)->SetSelected( false );
														}
														//selectedPolygons.push_back( (*it) );
														
													}
													else
													{
														if( sf::Keyboard::isKeyPressed( Keyboard::LShift ) )
														{
															
														}
														else
														{
														/*	for( list<TerrainPolygon*>::iterator selIt = 
																selectedPolygons.begin(); 
																selIt != selectedPolygons.end(); ++selIt )
															{
																(*selIt)->SetSelected( false );
															}
															selectedPolygons.clear();*/
															//selectedPolygons.push_back( (*it) );
															//(*it)->SetSelected( true );
														}

														selectedPolygons.push_back( (*it) );
														(*it)->SetSelected( true );
														//selectedPolygons.remove( (*it ) );
													}
													//break;
												}
										}
									}

								if( emptySpace )
								{
									for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
										it != selectedPolygons.end(); ++it )
									{
										(*it)->SetSelected( false );

									}
									selectedPolygons.clear();
								}
								
							//	cout << "here before loop" << endl;
							/*	bool empty = true;
								for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end() && empty; ++it )
								{
									list<ActorParams*> &actors = it->second->actors;
									for( list<ActorParams*>::iterator it2 = actors.begin(); it2 != actors.end() && empty; ++it2 )
									{
										sf::FloatRect bounds = (*it2)->image.getGlobalBounds();
										if( bounds.contains( Vector2f( worldPos.x, worldPos.y ) ) )
										{
											selectedActor = (*it2);
											selectedActorGrabbed = true;
											grabPos = Vector2i( worldPos.x, worldPos.y );

											empty = false;
											//cout << "enemy selected" << endl;

											for( list<TerrainPolygon*>::iterator it3 = selectedPolygons.begin(); 
												it3 != selectedPolygons.end(); ++it3 )
											{
												(*it3)->SetSelected( false );
											}
											selectedPolygons.clear();
										}
									}
								}


							//	cout << "made it!!!" << endl;
								if( empty )
								{
									selectedActor = NULL;
									selectedActorGrabbed = false;
								}*/
							}
							}
							else if( ev.key.code == Keyboard::R )
							{
								showGrass = false;
								for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
								{
									
									//showGrass = true;
									for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
									{
										(*it)->ShowGrass( false );
									}
								}
							}
							break;
						}
					case Event::LostFocus:
						{
							break;
						}
					case Event::GainedFocus:
						{
							break;
						}
					}
					break;
				}
			case CREATE_ENEMY:
				{
					switch( ev.type )
					{
					case Event::MouseButtonPressed:
						{
							if( ev.mouseButton.button == Mouse::Left )
							{
								if( popupPanel != NULL )
								{
									popupPanel->Update( true, uiMouse.x, uiMouse.y );
									break;
								}
								if( showPanel != NULL )
								{	
									showPanel->Update( true, uiMouse.x, uiMouse.y );
								}
								else if( gs.active )
								{
									gs.Update( true, uiMouse.x, uiMouse.y );
								}
							}
							break;
						}
					case Event::MouseButtonReleased:
						{
							if( ev.mouseButton.button == Mouse::Left )
							{
								if( popupPanel != NULL )
								{
									popupPanel->Update( false, uiMouse.x, uiMouse.y );
									break;
								}

								if( showPanel == NULL && trackingEnemy != NULL )
								{
									if( trackingEnemy->name == "patroller" )
									{
										showPanel = trackingEnemy->panel;
										patrolPath.clear();
										patrolPath.push_back( Vector2i( worldPos.x, worldPos.y ) );

										//mode = CREATE_PATROL_PATH;
										//patrolPath.clear();
										//patrolPath.push_back( Vector2i( worldPos.x, worldPos.y ) );
									}
									else if( trackingEnemy->name == "crawler" )
									{
										//groups["--"]->name
										if( enemyEdgePolygon != NULL )
										{
											showPanel = trackingEnemy->panel;
											//trackingEnemy = NULL;
										}
									}
									else if( trackingEnemy->name == "basicturret" )
									{
										if( enemyEdgePolygon != NULL )
										{
											showPanel = trackingEnemy->panel;
										}
									}
									else if( trackingEnemy->name == "foottrap" )
									{
										if( enemyEdgePolygon != NULL )
										{
											showPanel = trackingEnemy->panel;

											/*showPanel = trackingEnemy->panel;
											trackingEnemy = NULL;
											ActorParams *actor = new ActorParams;
											actor->group = groups["--"];
											actor->SetAsFootTrap( footTrapType, enemyEdgePolygon, enemyEdgeIndex, 
												enemyEdgeQuantity );
											groups["--"]->actors.push_back( actor );*/
										}
									}
									else if( trackingEnemy->name == "goal" )
									{
										if( enemyEdgePolygon != NULL )
										{
											showPanel = trackingEnemy->panel;
											trackingEnemy = NULL;
											ActorParams *actor = new GoalParams( this, enemyEdgePolygon, enemyEdgeIndex, 
												enemyEdgeQuantity );
											actor->group = groups["--"];
											//actor->SetAsGoal( goalType, enemyEdgePolygon, enemyEdgeIndex, 
											//	enemyEdgeQuantity );
											groups["--"]->actors.push_back( actor );
										}
									}
								}

								if( showPanel != NULL )
								{	
									showPanel->Update( false, uiMouse.x, uiMouse.y );
								}
								else if( gs.active )
								{
									if( gs.Update( false, uiMouse.x, uiMouse.y ) )
									{
										cout << "selected enemy index: " << gs.focusX << ", " << gs.focusY << endl;
									}
								}

								
							}
							break;
						}
					case Event::MouseWheelMoved:
						{
							break;
						}
					case Event::KeyPressed:
						{
							if( popupPanel != NULL )
							{
								popupPanel->SendKey( ev.key.code, ev.key.shift );
								break;
							}

							if( showPanel != NULL )
							{
								showPanel->SendKey( ev.key.code, ev.key.shift );
							}
							break;
						}
					case Event::KeyReleased:
						{
							break;
						}
					}
					break;
				}
			case PAUSED:
				{
					switch( ev.type )
					{
						case Event::MouseButtonPressed:
							{
								/*if( popupPanel != NULL )
								{
									popupPanel->Update( true, uiMouse.x, uiMouse.y );
									break;
								}*/
								break;
							}
						case Event::MouseButtonReleased:
							{
								/*if( popupPanel != NULL )
								{
									popupPanel->Update( false, uiMouse.x, uiMouse.y );
									break;
								}*/
								break;
							}
						case Event::GainedFocus:
						{
							mode = stored;
							break;
						}
					}
					break;
				}
			case SELECT_MODE:
				{
					switch( ev.type )
					{
					case Event::MouseButtonPressed:
						{
							break;
						}
					case Event::MouseButtonReleased:
						{
							V2d releasePos(uiMouse.x, uiMouse.y);
							
							V2d worldTop = menuDownPos + topPos;
							V2d worldUpperLeft = menuDownPos + upperLeftPos;
							V2d worldUpperRight = menuDownPos + upperRightPos;
							V2d worldLowerRight = menuDownPos + lowerRightPos;
							V2d worldLowerLeft = menuDownPos + lowerLeftPos;
							V2d worldBottom = menuDownPos + bottomPos;


							if( length( releasePos - worldTop ) < circleRadius )
							{
								menuSelection = "top";
							}
							else if( length( releasePos - worldUpperLeft ) < circleRadius )
							{
								menuSelection = "upperleft";
							}
							else if( length( releasePos - worldUpperRight ) < circleRadius )
							{
								menuSelection = "upperright";
							}
							else if( length( releasePos - worldLowerLeft ) < circleRadius )
							{
								menuSelection = "lowerleft";
							}
							else if( length( releasePos - worldLowerRight ) < circleRadius )
							{
								menuSelection = "lowerright";
							}
							else if( length( releasePos - worldBottom ) < circleRadius )
							{
								menuSelection = "bottom";
							}
							else
							{
								mode = menuDownStored;
								menuSelection = "none";
							}

							if( menuDownStored == EDIT && menuSelection != "none" && menuSelection != "top" )
							{
								selectedPlayer = false;
								selectedActor = NULL;
								for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
									it != selectedPolygons.end(); ++it )
								{
									(*it)->SetSelected( false );
								}
								selectedPolygons.clear();
							}
							else if( menuDownStored == CREATE_TERRAIN && menuSelection != "none" )
							{
								polygonInProgress->points.clear();
							}

							cout << "menu: " << menuSelection << endl;
							if( menuSelection == "top" )
							{
								if( menuDownStored == EDIT && selectedPolygons.size() > 0 )
								{
									showPanel = terrainOptionsPanel;
									mode = menuDownStored;
								}
								else if( menuDownStored == EDIT && selectedLight != NULL )
								{
									//lightPanel->
									string rStr = boost::lexical_cast<string>( (int)selectedLight->color.r );
									string gStr = boost::lexical_cast<string>( (int)selectedLight->color.g );
									string bStr = boost::lexical_cast<string>( (int)selectedLight->color.b );
									
	
									lightPanel->textBoxes["red"]->text.setString( rStr );
									lightPanel->textBoxes["green"]->text.setString( gStr );
									lightPanel->textBoxes["blue"]->text.setString( bStr );
									
									showPanel = lightPanel;
									mode = menuDownStored;
								}
								else if( menuDownStored == EDIT && selectedActor != NULL )
								{
									SetEnemyEditPanel();
									mode = menuDownStored;
								}
								else

								{
									mode = EDIT;
								}
							}
							else if( menuSelection == "upperleft" )
							{
								mode = CREATE_ENEMY;
							}
							else if( menuSelection == "upperright" )
							{
								mode = CREATE_TERRAIN;
							}
							else if( menuSelection == "lowerleft" )
							{
								mode = CREATE_LIGHTS;
							}
							else if( menuSelection == "lowerright" )
							{
								showPanel = mapOptionsPanel;
								mode = menuDownStored;
							}
							else if( menuSelection == "bottom" )
							{
							}
							

							break;
						}
					case Event::MouseWheelMoved:
						{
							break;
						}
					case Event::KeyPressed:
						{
							break;
						}
					case Event::KeyReleased:
						{
							break;
						}
					case Event::LostFocus:
						{
							break;
						}
					case Event::GainedFocus:
						{
							break;
						}
					}
					break;
				}
			case CREATE_PATROL_PATH:
				{
					minimumPathEdgeLength = 16;
					switch( ev.type )
					{
					case Event::MouseButtonPressed:
						{

							break;
						}
					case Event::MouseButtonReleased:
						{
							break;
						}
					case Event::MouseWheelMoved:
						{
							break;
						}
					case Event::KeyPressed:
						{
							if( ( ev.key.code == Keyboard::V || ev.key.code == Keyboard::Delete ) && patrolPath.size() > 1 )
							{
								patrolPath.pop_back();
							}
							else if( ev.key.code == Keyboard::Space )
							{
								if( selectedActor != NULL )
								{
									showPanel = selectedActor->type->panel;
									mode = EDIT;
								}
								else
								{
									showPanel = trackingEnemy->panel;
									mode = CREATE_ENEMY;
								}
								
								/*showPanel = trackingEnemy->panel;
								trackingEnemy = NULL;
								ActorParams *actor = new ActorParams;
								actor->SetAsPatroller( patrollerType, patrolPath.front(), patrolPath, 10, false );
								groups["--"]->actors.push_back( actor);
								actor->group = groups["--"];
								patrolPath.clear();
								mode = CREATE_ENEMY;*/
							}
							break;
						}
					case Event::KeyReleased:
						{
							break;
						}
					case Event::LostFocus:
						{
							break;
						}
					case Event::GainedFocus:
						{
							break;
						}
					}
					break;
				}
			case CREATE_TERRAIN_PATH:
				{
					minimumPathEdgeLength = 16;
					switch( ev.type )
					{
					case Event::MouseButtonPressed:
						{
							if( popupPanel != NULL )
							{
								popupPanel->Update( true, uiMouse.x, uiMouse.y );
								break;
							}
							break;
						}
					case Event::MouseButtonReleased:
						{
							if( popupPanel != NULL )
							{
								popupPanel->Update( false, uiMouse.x, uiMouse.y );
								break;
							}
							break;
						}
					case Event::MouseWheelMoved:
						{
							break;
						}
					case Event::KeyPressed:
						{
							if( popupPanel != NULL )
							{
								popupPanel->SendKey( ev.key.code, ev.key.shift );
								break;
							}


							if( ( ev.key.code == Keyboard::V || ev.key.code == Keyboard::Delete ) && selectedPolygons.front()->path.size() > 1 )
							{
								for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
								{
									(*it)->path.pop_back();
								}
							}
							else if( ev.key.code == Keyboard::Space )
							{
								if( selectedPolygons.front()->path.size() == 1 )
								{
									for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
									{
										(*it)->path.pop_back();
									}
								}
								showPanel = terrainOptionsPanel;
								mode = EDIT;
							}
							break;
						}
					case Event::KeyReleased:
						{
							break;
						}
					case Event::LostFocus:
						{
							break;
						}
					case Event::GainedFocus:
						{
							break;
						}
					}
					break;
				}
			case CREATE_LIGHTS:
				{
					switch( ev.type )
					{
					case Event::MouseButtonPressed:
						{
							if( ev.mouseButton.button == Mouse::Left )
							{
								if( popupPanel != NULL )
								{
									popupPanel->Update( true, uiMouse.x, uiMouse.y );
									break;
								}
								if( showPanel != NULL )
								{	
									showPanel->Update( true, uiMouse.x, uiMouse.y );
									break;
								}
								else
								{
									lightPosDown = true;
									lightPos = Vector2i( worldPos.x, worldPos.y );
								}
							}
							break;
						}
					case Event::MouseButtonReleased:
						{
							if( popupPanel != NULL )
							{
								popupPanel->Update( false, uiMouse.x, uiMouse.y );
								break;
							}
							if( showPanel != NULL )
							{	
								showPanel->Update( false, uiMouse.x, uiMouse.y );
							}
							else if( ev.mouseButton.button == Mouse::Left )
							{
								if( showPanel == NULL )
								{
									//if( !radiusOption )
									//{
									//	radiusOption = true;
									//}
									//else
									//{
									//	radiusOption = false;
									lightPosDown = false;
										showPanel = lightPanel;
									//}
									//cout << "make light panel" << endl;
									//showPanel = lightPanel;
								}
							}
							break;
						}
					case Event::MouseWheelMoved:
						{
							
						}
					case Event::KeyPressed:
						{
							if( popupPanel != NULL )
							{
								popupPanel->SendKey( ev.key.code, ev.key.shift );
								break;
							}
							if( showPanel != NULL )
							{
								showPanel->SendKey( ev.key.code, ev.key.shift );
								break;
							}
							break;
						}
					case Event::KeyReleased:
						{
							break;
						}
					case Event::LostFocus:
						{
							break;
						}
					case Event::GainedFocus:
						{
							break;
						}
					}
					break;
				}
			}

			

			//ones that aren't specific to mode
			
			if( mode != PAUSED && mode != SELECT_MODE && popupPanel == NULL )
			{
				switch( ev.type )
				{
				case Event::MouseButtonPressed:
					{
						if( ev.mouseButton.button == Mouse::Button::Middle )
						{
							panning = true;
							panAnchor = worldPos;
						}
						else if( ev.mouseButton.button == Mouse::Button::Right )
						{
							menuDownStored = mode;
							mode = SELECT_MODE;
							menuDownPos = V2d( uiMouse.x, uiMouse.y );
							guiMenuSprite.setPosition( uiMouse.x, uiMouse.y );//pixelPos.x, pixelPos.y );//uiMouse.x, uiMouse.y );
						}
						break;
					}
				case Event::MouseButtonReleased:
					{
						if( ev.mouseButton.button == Mouse::Button::Middle )
						{
							panning = false;
						}
						break;
					}
				case Event::MouseWheelMoved:
					{
						if( ev.mouseWheel.delta > 0 )
						{
							zoomMultiple /= 2;
						}
						else if( ev.mouseWheel.delta < 0 )
						{
							zoomMultiple *= 2;
						}

						if( zoomMultiple < .25 )
						{
							zoomMultiple = .25;
							cout << "min zoom" << endl;
						}
						else if( zoomMultiple > 65536 )
						{
							zoomMultiple = 65536;
						}
						else if( abs(zoomMultiple - 1.0) < .1 )
						{
							zoomMultiple = 1;
						}
				
						Vector2<double> ff = Vector2<double>(view.getCenter().x, view.getCenter().y );//worldPos - ( - (  .5f * view.getSize() ) );
						view.setSize( Vector2f( 960 * (zoomMultiple), 540 * ( zoomMultiple ) ) );
						preScreenTex->setView( view );
						Vector2f newWorldPosTemp = preScreenTex->mapPixelToCoords(pixelPos);
						Vector2<double> newWorldPos( newWorldPosTemp.x, newWorldPosTemp.y );
						Vector2<double> tempCenter = ff + ( worldPos - newWorldPos );
						view.setCenter( tempCenter.x, tempCenter.y );
						preScreenTex->setView( view );
						break;
					}
				case Event::KeyPressed:
					{
						if( ev.key.code == Keyboard::S && ev.key.control )
						{
							polygonInProgress->points.clear();
							cout << "writing to file: " << currentFile << ".brknk" << endl;
							WriteFile(currentFile);
						}
						else if( ev.key.code == Keyboard::T )
						{
							quit = true;
						}
						else if( ev.key.code == Keyboard::Escape )
						{
							if( sf::Keyboard::isKeyPressed( sf::Keyboard::Escape ) )
							{
								quit = true;
								returnVal = 1;
							}
						}
						else if( ev.key.code == sf::Keyboard::Z )
						{
							panning = true;
							panAnchor = worldPos;	
						}
						else if( ev.key.code == sf::Keyboard::Equal || ev.key.code == sf::Keyboard::Dash )
						{
							if( ev.key.code == sf::Keyboard::Equal )
							{
								zoomMultiple /= 2;
							}
							else if( ev.key.code == sf::Keyboard::Dash )
							{
								zoomMultiple *= 2;
							}

							if( zoomMultiple < .25 )
							{
								zoomMultiple = .25;
								cout << "min zoom" << endl;
							}
							else if( zoomMultiple > 65536 )
							{
								zoomMultiple = 65536;
							}
							else if( abs(zoomMultiple - 1.0) < .1 )
							{
								zoomMultiple = 1;
							}
				
							Vector2<double> ff = Vector2<double>(view.getCenter().x, view.getCenter().y );//worldPos - ( - (  .5f * view.getSize() ) );
							view.setSize( Vector2f( 960 * (zoomMultiple), 540 * ( zoomMultiple ) ) );
							preScreenTex->setView( view );
							Vector2f newWorldPosTemp = preScreenTex->mapPixelToCoords(pixelPos);
							Vector2<double> newWorldPos( newWorldPosTemp.x, newWorldPosTemp.y );
							Vector2<double> tempCenter = ff + ( worldPos - newWorldPos );
							view.setCenter( tempCenter.x, tempCenter.y );
							preScreenTex->setView( view );

							break;
						}
						break;
					}
				case Event::KeyReleased:
					{
						if( ev.key.code == sf::Keyboard::Z )
						{
							panning = false;
						}
						break;
					}
				case Event::LostFocus:
					{
						stored = mode;
						mode = PAUSED;
						break;
					}
				case Event::GainedFocus:
					{
						mode = stored;
						break;
					}
				}
			}
			
		
		}

		

		if( quit )
			break;

		showGraph = false;

		showTerrainPath = true;

		switch( mode )
		{
		case CREATE_TERRAIN:
			{
				/*if( polygonInProgress->points.size() > 0 && Keyboard::isKeyPressed( Keyboard::LShift ) ) 
				{

					Vector2i last = polygonInProgress->points.back();
					Vector2f diff = testPoint - Vector2f(last.x, last.y);

					double len;
					double angle = atan2( -diff.y, diff.x );
					if( angle < 0 )
						angle += 2 * PI;
					Vector2f dir;
			
					//cout << "angle : " << angle << endl;
					if( angle + PI / 8 >= 2 * PI || angle < PI / 8 )
					{
						len = dot( V2d( diff.x, diff.y ), V2d( 1, 0 ) );
						dir = Vector2f( 1, 0 );
					}
					else if( angle < 3 * PI / 8 )
					{
						len = dot( V2d( diff.x, diff.y ), normalize( V2d( 1, -1 ) ) );
						V2d tt = normalize( V2d( 1, -1 ) );
						dir = Vector2f( tt.x, tt.y );
					}
					else if( angle < 5 * PI / 8 )
					{
						len = dot( V2d( diff.x, diff.y ), V2d( 0, -1 ) );
						dir = Vector2f( 0, -1 );
					}
					else if( angle < 7 * PI / 8 )
					{
						len = dot( V2d( diff.x, diff.y ), normalize(V2d( -1, -1 )) );
						V2d tt = normalize( V2d( -1, -1 ) );
						dir = Vector2f( tt.x, tt.y );
					}
					else if( angle < 9 * PI / 8 )
					{
						len = dot( V2d( diff.x, diff.y ), V2d( -1, 0 ) );
						dir = Vector2f( -1, 0 );
					}
					else if( angle < 11 * PI / 8 )
					{
						len = dot( V2d( diff.x, diff.y ), normalize(V2d( -1, 1 )) );
						V2d tt = normalize( V2d( -1, 1 ) );
						dir = Vector2f( tt.x, tt.y );
					}
					else if( angle < 13 * PI / 8 )
					{
						len = dot( V2d( diff.x, diff.y ), V2d( 0, 1 ) );
						dir = Vector2f( 0, 1 );
					}
					else //( angle < 15 * PI / 8 )
					{
						len = dot( V2d( diff.x, diff.y ), normalize(V2d( 1, 1 )) );
						V2d tt = normalize( V2d( 1, 1 ) );
						dir = Vector2f( tt.x, tt.y );
					}

					testPoint = Vector2f(last.x, last.y) + dir * (float)len;
					//angle = asin( dot( ground->Normal(), V2d( 1, 0 ) ) ); 
				}*/

				

				if( popupPanel != NULL || showPanel != NULL )
					break;

				if( //polygonInProgress->points.size() > 0 && 
					Keyboard::isKeyPressed( Keyboard::G ) )
				{
					int adjX, adjY;
					
					testPoint.x /= 32;
					testPoint.y /= 32;

					if( testPoint.x > 0 )
						testPoint.x += .5f;
					else if( testPoint.x < 0 )
						testPoint.x -= .5f;

					if( testPoint.y > 0 )
						testPoint.y += .5f;
					else if( testPoint.y < 0 )
						testPoint.y -= .5f;

					adjX = ((int)testPoint.x) * 32;
					adjY = ((int)testPoint.y) * 32;
					
					//V2d tempTest = GraphPos( testPoint
					testPoint = Vector2f( adjX, adjY );
					showGraph = true;
				}

				if( polygonInProgress->points.size() > 0 )
				{
							
					V2d backPoint = V2d( polygonInProgress->points.back().pos.x, polygonInProgress->points.back().pos.y );
					V2d tPoint( testPoint.x, testPoint.y );
					V2d extreme( 0, 0 );
					V2d vec = tPoint - backPoint;
					V2d normVec = normalize( vec );
					
					if( normVec.x > PRIMARY_LIMIT )
						extreme.x = 1;
					else if( normVec.x < -PRIMARY_LIMIT )
						extreme.x = -1;
					if( normVec.y > PRIMARY_LIMIT )
						extreme.y = 1;
					else if( normVec.y < -PRIMARY_LIMIT )
						extreme.y = -1;

					//extreme = normalize( extreme );

					if( !( extreme.x == 0 && extreme.y == 0 ) )
					{
						//double test = abs( cross( normalize( V2d( testPoint.x, testPoint.y ) - backPoint ), extreme ) );
						//cout << "test: " << test << endl;
						//if( test  < 1 )
						{
						//	cout << "ADJUSTING TESTPOINt BLAH STRAIGHT : " << extreme.x  << ", " << extreme.y << endl;
							testPoint = Vector2f( backPoint + extreme * length( vec ) );
						}
					}
							
				}

				if( !panning && Mouse::isButtonPressed( Mouse::Left ) )
				{
					bool emptySpace = true;
					for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
					{
						if((*it)->ContainsPoint( testPoint ) )
						{
							//emptySpace = false;
						
							break;
						}
					}

					if( showPoints && extendingPolygon == NULL )
					{
						bool none = true;
						for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
						{
							for( PointList::iterator pit = (*it)->points.begin(); pit != (*it)->points.end(); ++pit )
							{
								Vector2i pointPos = (*pit).pos;
								double dist = length( V2d( pointPos.x, pointPos.y ) - V2d( testPoint.x, testPoint.y ) );
								if( dist < 8 * zoomMultiple )
								{
									extendingPolygon = (*it);
									extendingPoint = &(*pit);
									none = false;
									break;
								}
							}
							if( !none )
								break;
						}	
					}

					if( ( !showPoints || (showPoints && extendingPolygon != NULL )) && emptySpace )
					{
						
						Vector2i worldi( testPoint.x, testPoint.y );

						bool okay = true;
						if( extendingPolygon != NULL && polygonInProgress->points.size() > 0 )
						{
							okay = !extendingPolygon->ContainsPoint( testPoint );
							

							if( okay )
							{
								PointList::iterator okayIt = extendingPolygon->points.begin(); 
								PointList::iterator okayPrev = extendingPolygon->points.end();
								--okayPrev;
								for( ; okayIt != extendingPolygon->points.end(); ++okayIt )
								{
								
									//LineIntersection li = SegmentIntersect( , worldi, (*okayPrev).pos, (*okayIt).pos );
									Vector2i a = polygonInProgress->points.back().pos;
									Vector2i b = worldi;
									Vector2i c = (*okayPrev).pos;
									Vector2i d = (*okayIt).pos;

								
									LineIntersection li = LimitSegmentIntersect( a,b,c,d );
									Vector2i lii( floor(li.position.x + .5), floor(li.position.y + .5) );
									//if( !li.parallel  && (abs( lii.x - currPoint.x ) >= 1 || abs( lii.y - currPoint.y ) >= 1 ))
									if( !li.parallel )//&& lii != a && lii != b && lii != c && lii != d )
									{
										okay = false;
										break;
									}
									okayPrev = okayIt;
								}
								
								
							}
							// &&  IsPointValid( polygonInProgress->points.back().pos, worldi, extendingPolygon );
							//okay = IsPointValid( polygonInProgress->points.back().pos, worldi, extendingPolygon );
						}

						
						//okay = true;

						bool done = false;
						if( extendingPolygon != NULL )
						{
							for( PointList::iterator pit = extendingPolygon->points.begin(); pit != extendingPolygon->points.end(); ++pit )
							{
								if( &(*pit) == extendingPoint )
								{
									continue;
								}
								Vector2i pointPos = (*pit).pos;
								double dist = length( V2d( pointPos.x, pointPos.y ) - V2d( testPoint.x, testPoint.y ) );
								if( dist < 8 * zoomMultiple )
								{
									//ExtendPolygon();
									extendingPolygon->Extend( extendingPoint, &(*pit), polygonInProgress );

									ExtendAdd();

									//polygonInProgress->points.clear();
									//polygonInProgress->Reset();
									//cout << "EXTENDING POLYGON" << endl;
									

									polygonInProgress->Reset();

									extendingPolygon = NULL;
									extendingPoint = NULL;
									done = true;
									//cout << "done!" << endl;
									break;
								}
							}
						}

						//okay = true;
						if( !done && okay )
						{
							bool validNearPolys = true;
							if( polygonInProgress->points.size() > 0 )
							{
								for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
								{
									if( !IsPointValid( polygonInProgress->points.back().pos, worldi, (*it) ) )
									{
										validNearPolys = false;
										break;
									}
								}
							}

							if( validNearPolys )
							{
								if( !polygonInProgress->points.empty() && length( V2d( testPoint.x, testPoint.y ) - Vector2<double>(polygonInProgress->points.back().pos.x, 
									polygonInProgress->points.back().pos.y )  ) >= minimumEdgeLength * std::max(zoomMultiple,1.0 ) )
								{
									if( PointValid( polygonInProgress->points.back().pos, worldi ) )
									{
										polygonInProgress->points.push_back( TerrainPoint( worldi, false ) );
									}
								}
								else if( polygonInProgress->points.empty() )
								{
									if( extendingPolygon != NULL )
									{
										polygonInProgress->points.push_back( *extendingPoint );
									}
									else
									{
										polygonInProgress->points.push_back( TerrainPoint( worldi, false ) );
									}
									//cout << "showPoints: " << showPoints << ", " << (extendingPolygon == NULL) << endl;
								
							
								}
							}
						}
					}
					else
					{
						//polygonInProgress->points.clear();
					}

					
					
				}
				break;
			}
		case EDIT:
			{

				V2d pPoint = worldPos;
				Vector2i extra( 0, 0 );
				bool blah = false;
				if( //polygonInProgress->points.size() > 0 && 
					Keyboard::isKeyPressed( Keyboard::G ) )
				{
					pPoint = GraphPos( worldPos );
					showGraph = true;

					int countSelected = CountSelectedPoints();
					if( countSelected == 1 )
					{
						Vector2i pointSelected;
						bool done = false;
						for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin();
							it != selectedPolygons.end(); ++it, !done )
						{
							for( PointList::iterator pit = (*it)->points.begin(); pit != (*it)->points.end(); ++pit, !done )
							{
								if( (*pit).selected )
								{
									pointSelected = (*pit).pos;
									done = true;
								}
							}
						}

						int xrem = pointSelected.x % 32;
						int yrem = pointSelected.y % 32;
						if( pointSelected.x > 0 )
						{
							if( xrem >= 32 / 2 )
							{
								extra.x = 32 - xrem;
							}
							else
							{
								extra.x = xrem;
							}
						}
						else if( pointSelected.x < 0 )
						{
							if( xrem <= -32 / 2 )
							{
								extra.x = -32 - xrem;
							}
							else
							{
								extra.x = xrem;
							}
						}

						if( pointSelected.y > 0 )
						{
							if( yrem >= 32 / 2 )
							{
								extra.y = 32 - yrem;
							}
							else
							{
								extra.y = yrem;
							}
						}
						else if( pointSelected.y < 0 )
						{
							if( yrem <= -32 / 2 )
							{
								extra.y = -32 - yrem;
							}
							else
							{
								extra.y = yrem;
							}
						}
						extra.y = -extra.y;
						blah = true;
						pointGrabDelta = Vector2i( pPoint.x, pPoint.y ) - pointSelected;
						//extra = Vector2i( x );
					}
					else
					{
						//Vector2i pointSelected;
						/*int numOkay = 0;
						bool done = false;
						for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin();
							it != selectedPolygons.end(); ++it, !done )
						{
							for( PointList::iterator pit = (*it)->points.begin(); pit != (*it)->points.end(); ++pit, !done )
							{
								if( (*pit).selected )
								{
									if( (*pit).pos.x % 32 == (*pit).pos.y % 32 )
									{
									}
									numOkay++;
									done = true;
								}
							}
						}*/
					}
				}


				if( pointGrab )
				{

					Vector2i test( pointGrabPos.x % 32, pointGrabPos.y % 32 );
				//	cout << test.x << ", " << test.y << endl;
					
					if( blah )
					{
						
					}
					else
					{
						pointGrabDelta = Vector2i( pPoint.x, pPoint.y ) - pointGrabPos;
					}
					//pointGrabDelta += extra;
					//pointGrabDelta -= test;
					Vector2i oldPointGrabPos = pointGrabPos;
					pointGrabPos = Vector2i( pPoint.x, pPoint.y );// - Vector2i( pointGrabDelta.x % 32, pointGrabDelta.y % 32 );
					bool validMove = true;
					/*if( true )
					{
						for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin();
							it != selectedPolygons.end(); ++it )
						{
							bool affected = false;

							PointList & points = (*it)->points;

							for( PointList::iterator pointIt = points.begin();
								pointIt != points.end(); ++pointIt )
							{
								if( (*pointIt).selected ) //selected
								{
									Vector2i prev;
									if( pointIt == points.begin() )
									{
										prev = (*(--points.end())).pos;
									}
									else
									{
										PointList::iterator tempIt = pointIt;
										--tempIt;
										prev = (*tempIt).pos;
									}

									for( list<TerrainPolygon*>::iterator tit = polygons.begin();
										tit != polygons.end(); ++tit )
									{
										if( (*tit) != (*it) )
										if( !IsPointValid( prev, (*pointIt).pos, (*tit) ) )
										{
											validMove = false;
											break;
										}
									}
									
									if( !validMove )
										break;
									//(*pointIt).pos += pointGrabDelta;
									//affected = true;
								}
							}

							if( !validMove )
								break;
						}
					}*/
					
					

					int numSelectedPolys = selectedPolygons.size();
					Vector2i** allDeltas = new Vector2i*[numSelectedPolys];
					int allDeltaIndex = 0;
					for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin();
							it != selectedPolygons.end(); ++it )
					{
						TerrainPolygon &poly = *(*it);

						int polySize = poly.points.size();
						Vector2i *deltas = new Vector2i[polySize];
						allDeltas[allDeltaIndex] = deltas;
						int deltaIndex = 0;
						

						double prim_limit = PRIMARY_LIMIT;
						if( Keyboard::isKeyPressed( Keyboard::LShift ) )
						{
							prim_limit = .99;
						}

						for( PointList::iterator it2 = poly.points.begin(); it2 != poly.points.end(); ++it2 )
						{
							
							deltas[deltaIndex] = Vector2i( 0, 0 );

							if( !(*it2).selected )
							{
								++deltaIndex;
								continue;
							}

							Vector2i diff;

							PointList::iterator prev, next;
							if( it2 == poly.points.begin() )
							{
								prev = poly.points.end();
								prev--;
							}
							else
							{
								prev = it2;
								prev--;
							}

							PointList::iterator temp = it2;
							++temp;
							if( temp == poly.points.end() )
							{
								next = poly.points.begin();
							}
							else
							{
								next = it2;
								next++;
							}


							V2d pos((*it2).pos.x + pointGrabDelta.x, (*it2).pos.y + pointGrabDelta.y );
							V2d prevPos( (*prev).pos.x, (*prev).pos.y );
							V2d nextPos( (*next).pos.x, (*next).pos.y );

							V2d extreme( 0, 0 );
							Vector2i vec = (*it2).pos - (*prev).pos;
							V2d normVec = normalize( V2d( vec.x, vec.y ) );

							V2d newVec = normalize( pos - V2d( (*prev).pos.x, (*prev).pos.y ) );
		
							if( !(*prev).selected )
							{
								if( normVec.x == 0 || normVec.y == 0 )
								{
									if( newVec.x > prim_limit )
										extreme.x = 1;
									else if( newVec.x < -prim_limit )
										extreme.x = -1;
									if( newVec.y > prim_limit )
										extreme.y = 1;
									else if( newVec.y < -prim_limit )
										extreme.y = -1;
									/*double ff = dot( normalize( prevPos - pos ), extreme );
									if( ff > prim_limit )
									{
										if( normVec.x == 0 )
										{
											pointGrabPos.x = oldPointGrabPos.x;
										}
										else
										{
											pointGrabPos.y = oldPointGrabPos.y;
										}
										
										validMove = false;
										break;
									} */
									if( extreme.x != 0 )
									{
										pointGrabPos.y = oldPointGrabPos.y;
										pointGrabDelta.y = 0;
									}
									
									if( extreme.y != 0 )
									{
										pointGrabPos.x = oldPointGrabPos.x;
										pointGrabDelta.x = 0;
									}

									
									//pointGrabPos = oldPointGrabPos;
								//	pointGrabPos = oldPointGrabPos;
								}
								else
								{
									
									if( normVec.x > prim_limit )
										extreme.x = 1;
									else if( normVec.x < -prim_limit )
										extreme.x = -1;
									if( normVec.y > prim_limit )
										extreme.y = 1;
									else if( normVec.y < -prim_limit )
										extreme.y = -1;
									//extreme = normalize( extreme );

								
									if( extreme.x != 0 )
									{
										//int diff = ;
										diff.y = (*it2).pos.y - (*prev).pos.y;
									
										//(*it2).pos.y = (*prev).pos.y;
										cout << "lining up x: " << diff.y << endl;
									}

									if( extreme.y != 0 )
									{
										diff.x = (*it2).pos.x - (*prev).pos.x;

										cout << "lining up y: " << diff.x << endl;
									}
								}
							}
							
							if( !(*next).selected )
							{
								vec = (*it2).pos - (*next).pos;
								normVec = normalize( V2d( vec.x, vec.y ) );

								extreme = V2d( 0, 0 );

								newVec = normalize( pos - V2d( (*next).pos.x, (*next).pos.y ) );
								
								if( normVec.x == 0 || normVec.y == 0 )
								{
									if( newVec.x > prim_limit )
										extreme.x = 1;
									else if( newVec.x < -prim_limit )
										extreme.x = -1;
									if( newVec.y > prim_limit )
										extreme.y = 1;
									else if( newVec.y < -prim_limit )
										extreme.y = -1;
									
									if( extreme.x != 0 )
									{
										pointGrabPos.y = oldPointGrabPos.y;
										pointGrabDelta.y = 0;
									}
									
									if( extreme.y != 0 )
									{
										pointGrabPos.x = oldPointGrabPos.x;
										pointGrabDelta.x = 0;
									}
									//pointGrabPos = oldPointGrabPos;
								//	pointGrabPos = oldPointGrabPos;
								}
								else
								{
									if( normVec.x > prim_limit )
										extreme.x = 1;
									else if( normVec.x < -prim_limit )
										extreme.x = -1;
									if( normVec.y > prim_limit )
										extreme.y = 1;
									else if( normVec.y < -prim_limit )
										extreme.y = -1;

									if( extreme.x != 0 )
									{
										//int diff = ;
										diff.y = (*it2).pos.y - (*next).pos.y;
									
										//(*it2).pos.y = (*prev).pos.y;
										cout << "lining up x222: " << diff.y << endl;
									}

									if( extreme.y != 0 )
									{
										diff.x = (*it2).pos.x - (*next).pos.x;

										cout << "lining up y222: " << diff.x << endl;
									}
								}
							}

							if( !( diff.x == 0 && diff.y == 0 ) )
							{
								cout << "allindex: " << allDeltaIndex << ", deltaIndex: " << deltaIndex << endl;
								cout << "diff: " << diff.x << ", " << diff.y << endl;
								//pointGrabPos = oldPointGrabPos;
							}
							deltas[deltaIndex] = diff;
							
							

							++deltaIndex;
						}


						//if( !(*it)->IsMovePointsOkay( this, pointGrabDelta - diff ) )
						if( validMove && !(*it)->IsMovePointsOkay( this, pointGrabDelta, deltas ) )
						{
							validMove = false;
						//	cout << "invalid" << endl;
							break;
						}

						++allDeltaIndex;
					}

					if( validMove )
					{
						//cout << "valid move" << endl;
						//int 
						allDeltaIndex = 0;
						for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin();
							it != selectedPolygons.end(); ++it )
						{

							bool affected = false;

							PointList & points = (*it)->points;


							int deltaIndex = 0;
							for( PointList::iterator pointIt = points.begin();
								pointIt != points.end(); ++pointIt )
							{
								if( (*pointIt).selected ) //selected
								{					

									//Vector2i temp = (*pointIt).pos + pointGrabDelta;
									
									Vector2i delta = allDeltas[allDeltaIndex][deltaIndex];
									cout << "allindex: " << allDeltaIndex << ", deltaIndex: " << deltaIndex << endl;
									cout << "moving: " << delta.x << ", " << delta.y << endl;
									(*pointIt).pos += pointGrabDelta - delta; //pointGrabDelta - ;
									affected = true;
								}

								++deltaIndex;
							}

							if( affected )
							{
								PointList temp = (*it)->points;

								(*it)->Reset();

								for( PointList::iterator tempIt = temp.begin(); tempIt != temp.end(); 
									++tempIt )
								{
									(*it)->points.push_back( (*tempIt ) );
								}
								(*it)->Finalize();
								(*it)->SetSelected( true );
								
							}

							++allDeltaIndex;
						}
					}
					else
					{
						//cout << "NOT VALID move" << endl;
					}

					for( int i = 0; i < numSelectedPolys; ++i )
					{
						delete [] allDeltas[i];
					}
					delete [] allDeltas;

				}
				else if( polyGrab )
				{
					polyGrabDelta = Vector2i( pPoint.x, pPoint.y ) - polyGrabPos;
					polyGrabPos = Vector2i( pPoint.x, pPoint.y );
					
					bool moveOkay = true;
					if( polyGrabDelta.x != 0 || polyGrabDelta.y != 0 )
					{
						for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin();
						it != selectedPolygons.end(); ++it )
						{
							if( !(*it)->IsMovePolygonOkay(this, polyGrabDelta ) )
							{
								moveOkay = false;
								break;
							}
						}
					}
					else
					{
						moveOkay = false;
					}

					if( moveOkay )
					{
						for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin();
							it != selectedPolygons.end(); ++it )
						{
							PointList & points = (*it)->points;

							for( PointList::iterator pointIt = points.begin();
								pointIt != points.end(); ++pointIt )
							{
								(*pointIt).pos += polyGrabDelta;		
							}

							PointList temp = (*it)->points;

							(*it)->Reset();

							for( PointList::iterator tempIt = temp.begin(); tempIt != temp.end(); 
								++tempIt )
							{
								(*it)->points.push_back( (*tempIt ) );
							}
							(*it)->Finalize();
							(*it)->SetSelected( true );
						}
					}
				}
				

				if( showGrass && Mouse::isButtonPressed( Mouse::Button::Left ) )
				{
					for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
					{
						(*it)->SwitchGrass( worldPos );
					}
				}
				
				

				break;
			}
		case CREATE_ENEMY:
			{
				if( trackingEnemy != NULL && showPanel == NULL )
				{
					enemySprite.setOrigin( enemySprite.getLocalBounds().width / 2, enemySprite.getLocalBounds().height / 2 );
					enemySprite.setRotation( 0 );
					enemySprite.setPosition( preScreenTex->mapPixelToCoords( sf::Mouse::getPosition( *w ) ) );
				}

				if( showPanel == NULL && trackingEnemy != NULL && ( trackingEnemy->name == "crawler" 
					|| trackingEnemy->name == "basicturret"
					|| trackingEnemy->name == "foottrap" 
					|| trackingEnemy->name == "goal" ) )
				{
					enemyEdgePolygon = NULL;
				
					double testRadius = 200;
					
					for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
					{
						if( testPoint.x >= (*it)->left - testRadius && testPoint.x <= (*it)->right + testRadius
							&& testPoint.y >= (*it)->top - testRadius && testPoint.y <= (*it)->bottom + testRadius )
						{
							PointList::iterator prevIt = (*it)->points.end();
							prevIt--;
							PointList::iterator currIt = (*it)->points.begin();

							if( (*it)->ContainsPoint( Vector2f( testPoint.x, testPoint.y ) ) )
							{
								//prev is starting at 0. start normally at 1
								int edgeIndex = 0;
								double minDistance = 10000000;
								int storedIndex;
								double storedQuantity;
							
								V2d closestPoint;

								for( ; currIt != (*it)->points.end(); ++currIt )
								{
									double dist = abs(
										cross( 
										V2d( testPoint.x - (*prevIt).pos.x, testPoint.y - (*prevIt).pos.y ), 
										normalize( V2d( (*currIt).pos.x - (*prevIt).pos.x, (*currIt).pos.y - (*prevIt).pos.y ) ) ) );
									double testQuantity =  dot( 
											V2d( testPoint.x - (*prevIt).pos.x, testPoint.y - (*prevIt).pos.y ), 
											normalize( V2d( (*currIt).pos.x - (*prevIt).pos.x, (*currIt).pos.y - (*prevIt).pos.y ) ) );

									V2d pr( (*prevIt).pos.x, (*prevIt).pos.y );
									V2d cu( (*currIt).pos.x, (*currIt).pos.y );
									V2d te( testPoint.x, testPoint.y );
									
									V2d newPoint( pr.x + (cu.x - pr.x) * (testQuantity / length( cu - pr ) ), pr.y + (cu.y - pr.y ) *
											(testQuantity / length( cu - pr ) ) );

									//int testA = dist < 100;
									//int testB = testQuantity >= 0 && testQuantity <= length( cu - pr );
									//int testC = testQuantity >= enemySprite.getLocalBounds().width / 2 && testQuantity <= length( cu - pr ) - enemySprite.getLocalBounds().width / 2;
									//int testD = length( newPoint - te ) < length( closestPoint - te );
									
									//cout << testA << " " << testB << " " << testC << " " << testD << endl;

									if( dist < 100 && testQuantity >= 0 && testQuantity <= length( cu - pr ) && testQuantity >= enemySprite.getLocalBounds().width / 2 && testQuantity <= length( cu - pr ) - enemySprite.getLocalBounds().width / 2 
										&& length( newPoint - te ) < length( closestPoint - te ) )
									{
										minDistance = dist;
										storedIndex = edgeIndex;
										double l = length( cu - pr );
										
										storedQuantity = testQuantity;
										closestPoint = newPoint ;
										//minDistance = length( closestPoint - te )  
										
										enemySprite.setOrigin( enemySprite.getLocalBounds().width / 2, enemySprite.getLocalBounds().height );
										enemySprite.setPosition( closestPoint.x, closestPoint.y );
										enemySprite.setRotation( atan2( (cu - pr).y, (cu - pr).x ) / PI * 180 );
									}
									else
									{
										
										//cout << "dist: " << dist << ", testquant: " << testQuantity  << endl;
									}

									prevIt = currIt;
									++edgeIndex;
								}

								enemyEdgeIndex = storedIndex;

								enemyEdgeQuantity = storedQuantity;
								
								enemyEdgePolygon = (*it);
								

								//cout << "pos: " << closestPoint.x << ", " << closestPoint.y << endl;
								//cout << "minDist: " << minDistance << endl;

								break;
							}
						}
					}


				}

				
				break;
			}
		case PAUSED:
			{
				break;
			}
		case CREATE_PATROL_PATH:
			{
				/*if( //polygonInProgress->points.size() > 0 && 
					Keyboard::isKeyPressed( Keyboard::G ) )
				{
					int adjX, adjY;
					
					testPoint.x /= 32;
					testPoint.y /= 32;

					if( testPoint.x > 0 )
						testPoint.x += .5f;
					else if( testPoint.x < 0 )
						testPoint.x -= .5f;

					if( testPoint.y > 0 )
						testPoint.y += .5f;
					else if( testPoint.y < 0 )
						testPoint.y -= .5f;

					adjX = ((int)testPoint.x) * 32;
					adjY = ((int)testPoint.y) * 32;
					
					testPoint = Vector2f( adjX, adjY );
					showGraph = true;
				}*/
				if( showPanel != NULL )
					break;


				if( !panning && Mouse::isButtonPressed( Mouse::Left ) )
				{
					if( length( worldPos - Vector2<double>(patrolPath.back().x, 
						patrolPath.back().y )  ) >= minimumPathEdgeLength * std::max(zoomMultiple,1.0 ) )
					{
						Vector2i worldi( testPoint.x, testPoint.y );

						patrolPath.push_back( worldi );
					}					
				}
				break;
			}
		case CREATE_TERRAIN_PATH:
			{
				showTerrainPath = false;
				if( showPanel != NULL )
					break;

				
				Vector2i fullRectCenter( fullRect.left + fullRect.width / 2.0, fullRect.top + fullRect.height / 2.0 );
				if( !panning && Mouse::isButtonPressed( Mouse::Left ) )
				{
					if( length( ( worldPos - V2d( fullRectCenter.x, fullRectCenter.y ) ) - Vector2<double>(selectedPolygons.front()->path.back().x, 
						selectedPolygons.front()->path.back().y )  ) >= minimumPathEdgeLength * std::max(zoomMultiple,1.0 ) )
					{
						Vector2i worldi( testPoint.x - fullRectCenter.x, testPoint.y - fullRectCenter.y );

						for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
						{
							(*it)->path.push_back( worldi );
						}
					}					
				}
				break;
			}
		}

		//cout << "here before crash" << endl;
		

		if( panning )
		{
			Vector2<double> temp = panAnchor - worldPos;
			view.move( Vector2f( temp.x, temp.y ) );
		}
		
		


	/*	if( mode == PLACE_PLAYER )
		{
			playerSprite.setPosition( preScreenTex->mapPixelToCoords(sf::Mouse::getPosition( *w )) );
			//cout << "placing: " << playerSprite.getPosition().x << ", " << playerSprite.getPosition().y << endl;
		}
		else
			playerSprite.setPosition( playerPosition.x, playerPosition.y );*/




	/*	if( mode == PLACE_GOAL )
		{
			goalSprite.setPosition( preScreenTex->mapPixelToCoords( sf::Mouse::getPosition( *w )) );
		}
		else
			goalSprite.setPosition( goalPosition.x, goalPosition.y );*/
		
		preScreenTex->clear();

		preScreenTex->setView( view );

		/*sf::RectangleShape parTest( Vector2f( 1000, 1000 ) );
		parTest.setFillColor( Color::Red );
		parTest.setPosition( 0, 0 );
		preScreenTex->draw( parTest );*/

		preScreenTex->draw(border, 8, sf::Lines);

		Draw();

		for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it )
		{
			(*it).second->Draw( preScreenTex );
		}

		
		switch( mode )
		{
		case CREATE_TERRAIN:
			{
				int progressSize = polygonInProgress->points.size();
				if( progressSize > 0 )
				{
					Vector2i backPoint = polygonInProgress->points.back().pos;
			
					Color validColor = Color::Green;
					Color invalidColor = Color::Red;
					Color colorSelection;
					if( true )
					{
						colorSelection = validColor;
					}

					if( popupPanel == NULL )
					{
						sf::Vertex activePreview[2] =
						{
							sf::Vertex(sf::Vector2<float>(backPoint.x, backPoint.y), colorSelection ),
							sf::Vertex(sf::Vector2<float>(testPoint.x, testPoint.y), colorSelection)
						};


						preScreenTex->draw( activePreview, 2, sf::Lines );
					}

					if( progressSize > 1 )
					{
						VertexArray v( sf::LinesStrip, progressSize );
						int i = 0;
						for( PointList::iterator it = polygonInProgress->points.begin(); 
							it != polygonInProgress->points.end(); ++it )
						{
							v[i] = Vertex( Vector2f( (*it).pos.x, (*it).pos.y ) );
							++i;
						}
						preScreenTex->draw( v );
					}
				}
				break;
			}
		case EDIT:
			{
				if( makingRect )
				{
					int xDiff = ((int)worldPos.x) - rectStart.x;
					int yDiff = ((int)worldPos.y) - rectStart.y;

					if( abs(xDiff) > 10 && abs( yDiff) > 10 )
					{
						int left, top, width, height;
						if( xDiff > 0 )
						{
							left = rectStart.x;
							width = xDiff;
						}
						else
						{
							left = (int)worldPos.x;
							width = -xDiff;
						}

						if( yDiff > 0 )
						{
							top = rectStart.y;
							height = yDiff;
						}
						else
						{
							top = (int)worldPos.y;
							height = -yDiff;
						}

		//				sf::Rect<float> selectRect = sf::Rect<float>( left, top, width, height );

						sf::RectangleShape rs;
						rs.setSize( Vector2f( width, height ) );
						rs.setFillColor( Color::Transparent );
						rs.setOutlineColor( Color::Magenta );
						rs.setOutlineThickness( 2 );
						rs.setPosition( left, top );

						preScreenTex->draw( rs );
					}
				}
				break;
			}
		case CREATE_ENEMY:
			{
				if( trackingEnemy != NULL )
				{
					preScreenTex->draw( enemySprite );
				}
				break;
			}
		case CREATE_PATROL_PATH:
			{
				if( trackingEnemy != NULL )
				{
					preScreenTex->draw( enemySprite );
				}
				int pathSize = patrolPath.size();
				if( pathSize > 0 )
				{
					Vector2i backPoint = patrolPath.back();
			
					Color validColor = Color::Green;
					Color invalidColor = Color::Red;
					Color colorSelection;
					if( true )
					{
						colorSelection = validColor;
					}
					sf::Vertex activePreview[2] =
					{
						sf::Vertex(sf::Vector2<float>(backPoint.x, backPoint.y), colorSelection ),
						sf::Vertex(sf::Vector2<float>(testPoint.x, testPoint.y), colorSelection)
					};
					preScreenTex->draw( activePreview, 2, sf::Lines );

					if( pathSize > 1 )
					{
						VertexArray v( sf::LinesStrip, pathSize );
						int i = 0;
						for( list<sf::Vector2i>::iterator it = patrolPath.begin(); 
							it != patrolPath.end(); ++it )
						{
							v[i] = Vertex( Vector2f( (*it).x, (*it).y ) );
							++i;
						}
						preScreenTex->draw( v );
					}
				}
				
				if( pathSize > 0 ) //always
				{
					CircleShape cs;
					cs.setRadius( 5 * zoomMultiple  );
					cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
					cs.setFillColor( Color::Green );

		
					for( list<Vector2i>::iterator it = patrolPath.begin(); it != patrolPath.end(); ++it )
					{
						cs.setPosition( (*it).x, (*it).y );
						preScreenTex->draw( cs );
					}		
				}
				break;
			}
		case SELECT_MODE:
			{
				

			

				break;
			}
		case CREATE_TERRAIN_PATH:
			{
				
				int pathSize = selectedPolygons.front()->path.size();

				sf::FloatRect bounds;
				bounds.left = fullRect.left;
				bounds.top = fullRect.top;
				bounds.width = fullRect.width;
				bounds.height = fullRect.height;

				sf::RectangleShape rs( sf::Vector2f( bounds.width, bounds.height ) );
				
				rs.setOutlineColor( Color::Cyan );				
				rs.setFillColor( Color::Transparent );
				rs.setOutlineThickness( 5 );
				rs.setPosition( bounds.left, bounds.top );

				preScreenTex->draw( rs );

				Vector2i fullCenter( fullRect.left + fullRect.width / 2, fullRect.top + fullRect.height / 2 );
				if( pathSize > 0 )
				{
					Vector2i backPoint = selectedPolygons.front()->path.back();
					backPoint += fullCenter;
			
					Color validColor = Color::Magenta;
					Color invalidColor = Color::Red;
					Color colorSelection;
					if( true )
					{
						colorSelection = validColor;
					}
					sf::Vertex activePreview[2] =
					{
						sf::Vertex(sf::Vector2<float>(backPoint.x, backPoint.y), colorSelection ),
						sf::Vertex(sf::Vector2<float>(testPoint.x, testPoint.y), colorSelection)
					};
					preScreenTex->draw( activePreview, 2, sf::Lines );

					if( pathSize > 1 )
					{
						VertexArray v( sf::LinesStrip, pathSize );
						int i = 0;

						for( list<sf::Vector2i>::iterator it = selectedPolygons.front()->path.begin(); 
							it != selectedPolygons.front()->path.end(); ++it )
						{
							v[i] = Vertex( Vector2f( (*it).x + fullCenter.x, (*it).y + fullCenter.y) );
							++i;
						}
						preScreenTex->draw( v );
					}
				}
				
				if( pathSize >= 0 ) //always
				{
					CircleShape cs;
					cs.setRadius( 5 * zoomMultiple  );
					cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
					cs.setFillColor( Color::Magenta );

					//Vector2i fullCenter( fullRect.left + fullRect.width / 2, fullRect.top + fullRect.height / 2 );

					for( list<sf::Vector2i>::iterator it = selectedPolygons.front()->path.begin(); 
							it != selectedPolygons.front()->path.end(); ++it )
					{
						//cout << "drawing" << endl;
						cs.setPosition( (*it).x + fullCenter.x, (*it).y + fullCenter.y );
						preScreenTex->draw( cs );
					}		
				}

/*				CircleShape cs;
				cs.setRadius( 5 * zoomMultiple  );
				cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
				cs.setFillColor( Color::Magenta );
				cs.setPosition( fullCenter.x, fullCenter.y );
				preScreenTex->draw( cs );*/
				

			}
		}
		
		for( list<StaticLight*>::iterator it = lights.begin(); it != lights.end(); ++it )
		{
			(*it)->Draw( preScreenTex );
		}

		//iconSprite.setScale( view.getSize().x / 960.0, view.getSize().y / 540.0 );
		//iconSprite.setPosition( view.getCenter().x + 200 * iconSprite.getScale().x, view.getCenter().y - 250 * iconSprite.getScale().y );
		
		if( mode == EDIT )
		{
			if( selectedPlayer && grabPlayer && length( V2d( grabPos.x, grabPos.y ) - worldPos ) > 10 )
			{
				playerPosition = Vector2i( worldPos.x, worldPos.y );
			}
			else if( selectedActorGrabbed && length( V2d( grabPos.x, grabPos.y ) - worldPos ) > 10 )
			{
				/*if(  false && selectedActor != NULL && ( selectedActor->type->name == "crawler" 
					|| selectedActor->type->name== "basicturret"
					|| selectedActor->type->name == "foottrap" 
					|| selectedActor->type->name == "goal" ) )
				{
					enemyEdgePolygon = NULL;
					V2d testPoint = worldPos;
					double testRadius = 200;
					
					for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
					{
						if( testPoint.x >= (*it)->left - testRadius && testPoint.x <= (*it)->right + testRadius
							&& testPoint.y >= (*it)->top - testRadius && testPoint.y <= (*it)->bottom + testRadius )
						{
							list<Vector2i>::iterator prevIt = (*it)->points.end();
							prevIt--;
							list<Vector2i>::iterator currIt = (*it)->points.begin();

							if( (*it)->ContainsPoint( Vector2f( testPoint.x, testPoint.y ) ) )
							{
								//prev is starting at 0. start normally at 1
								int edgeIndex = 0;
								double minDistance = 10000000;
								int storedIndex;
								double storedQuantity;
							
								V2d closestPoint;

								for( ; currIt != (*it)->points.end(); ++currIt )
								{
									double dist = abs(
										cross( 
										V2d( testPoint.x - (*prevIt).x, testPoint.y - (*prevIt).y ), 
										normalize( V2d( (*currIt).x - (*prevIt).x, (*currIt).y - (*prevIt).y ) ) ) );
									double testQuantity =  dot( 
											V2d( testPoint.x - (*prevIt).x, testPoint.y - (*prevIt).y ), 
											normalize( V2d( (*currIt).x - (*prevIt).x, (*currIt).y - (*prevIt).y ) ) );

									V2d pr( (*prevIt).x, (*prevIt).y );
									V2d cu( (*currIt).x, (*currIt).y );
									V2d te( testPoint.x, testPoint.y );
									
									V2d newPoint( pr.x + (cu.x - pr.x) * (testQuantity / length( cu - pr ) ), pr.y + (cu.y - pr.y ) *
											(testQuantity / length( cu - pr ) ) );

									//int testA = dist < 100;
									//int testB = testQuantity >= 0 && testQuantity <= length( cu - pr );
									//int testC = testQuantity >= enemySprite.getLocalBounds().width / 2 && testQuantity <= length( cu - pr ) - enemySprite.getLocalBounds().width / 2;
									//int testD = length( newPoint - te ) < length( closestPoint - te );
									
									//cout << testA << " " << testB << " " << testC << " " << testD << endl;

									if( dist < 100 && testQuantity >= 0 && testQuantity <= length( cu - pr ) && testQuantity >= enemySprite.getLocalBounds().width / 2 && testQuantity <= length( cu - pr ) - enemySprite.getLocalBounds().width / 2 
										&& length( newPoint - te ) < length( closestPoint - te ) )
									{
										minDistance = dist;
										storedIndex = edgeIndex;
										double l = length( cu - pr );
										
										storedQuantity = testQuantity;
										closestPoint = newPoint ;
										//minDistance = length( closestPoint - te )  
										
										enemySprite.setOrigin( enemySprite.getLocalBounds().width / 2, enemySprite.getLocalBounds().height );
										enemySprite.setPosition( closestPoint.x, closestPoint.y );
										enemySprite.setRotation( atan2( (cu - pr).y, (cu - pr).x ) / PI * 180 );
									}
									else
									{
										
										//cout << "dist: " << dist << ", testquant: " << testQuantity  << endl;
									}

									prevIt = currIt;
									++edgeIndex;
								}

								enemyEdgeIndex = storedIndex;

								enemyEdgeQuantity = storedQuantity;
								
								enemyEdgePolygon = (*it);
								
								//selectedActor->ground = enemyEdgeIndex;
								//selectedActor->groundQuantity = enemyEdgeQuantity;

								
								//cout << "pos: " << closestPoint.x << ", " << closestPoint.y << endl;
								//cout << "minDist: " << minDistance << endl;

								//break;
							}
						}
					}
				}
				else*/
				{
					if( selectedActor->type->name == "patroller" )
					{
						
						selectedActor->position = Vector2i( worldPos.x, worldPos.y );
						selectedActor->image.setPosition( worldPos.x, worldPos.y );
					}
				}
				
			}
			else if( selectedLightGrabbed && length( V2d( lightGrabPos.x, lightGrabPos.y ) - worldPos ) > 10 )
			{
				selectedLight->position = Vector2i( worldPos.x, worldPos.y );
			}
		}
		
		

		playerSprite.setPosition( playerPosition.x, playerPosition.y );

		preScreenTex->draw( playerSprite );
		
		//preScreenTex->draw( iconSprite );

		if( false )
		//if( showPanel == NULL && sf::Keyboard::isKeyPressed( Keyboard::H ) )
		{
			alphaTextSprite.setScale( .5 * view.getSize().x / 960.0, .5 * view.getSize().y / 540.0 );
			alphaTextSprite.setOrigin( alphaTextSprite.getLocalBounds().width / 2, alphaTextSprite.getLocalBounds().height / 2 );
			alphaTextSprite.setPosition( view.getCenter().x, view.getCenter().y );
			preScreenTex->draw( alphaTextSprite );
		}

		playerSprite.setPosition( playerPosition.x, playerPosition.y );

		if( mode == EDIT )
		{
			if( selectedActor != NULL )
			{
				
				sf::FloatRect bounds = selectedActor->image.getGlobalBounds();
				sf::RectangleShape rs( sf::Vector2f( bounds.width, bounds.height ) );
				
				rs.setOutlineColor( Color::Cyan );				
				rs.setFillColor( Color::Transparent );
				rs.setOutlineThickness( 5 );
				rs.setPosition( bounds.left, bounds.top );
				//rs.setFillColor( Color::Magenta );
				preScreenTex->draw( rs );
				//cout << "draw rectangle"  << endl;
			}
			else if( selectedPlayer )
			{
				sf::FloatRect bounds = playerSprite.getGlobalBounds();
				sf::RectangleShape rs( sf::Vector2f( bounds.width, bounds.height ) );

				rs.setOutlineColor( Color::Cyan );				
				rs.setFillColor( Color::Transparent );
				rs.setOutlineThickness( 5 );
				rs.setPosition( bounds.left, bounds.top );				

				preScreenTex->draw( rs );
			}
			else if( selectedLight != NULL )
			{
				//sf::FloatRect bounds = selectedLight->position.getGlobalBounds();
				sf::RectangleShape lightAABB( sf::Vector2f( selectedLight->radius * 2, selectedLight->radius * 2 ) );

				lightAABB.setOutlineColor( Color::Cyan );				
				lightAABB.setFillColor( Color::Transparent );
				lightAABB.setOrigin( lightAABB.getLocalBounds().width / 2, lightAABB.getLocalBounds().height / 2 );
				lightAABB.setOutlineThickness( 5 );
				lightAABB.setPosition( selectedLight->position.x, selectedLight->position.y );				

				preScreenTex->draw( lightAABB );
			}
		}

		//display graph
		if( showGraph )
		{
			Vector2f adjustment;
			for( int i = 0; i < numLines * 8; ++i )
			{
				int adjX, adjY;
				float x = view.getCenter().x;
				float y = view.getCenter().y;

				x /= 32;
				y /= 32;

				if( x > 0 )
					x += .5f;
				else if( y < 0 )
					y -= .5f;

				if( y > 0 )
					y += .5f;
				else if( y < 0 )
					y -= .5f;

				adjX = ((int)x) * 32;
				adjY = ((int)y) * 32;
					
				adjustment = Vector2f( adjX, adjY );
				
				graphLines[i].position += adjustment;
			}
			
			preScreenTex->draw( graphLines );

			for( int i = 0; i < numLines * 8; ++i )
			{
				graphLines[i].position -= adjustment;
			}
		}

		if( mode == CREATE_LIGHTS )
		{
			if( lightPosDown || showPanel )
			{
				CircleShape cs;
				if( lightPosDown )
				{
					lightRadius = length( V2d( lightPos.x, lightPos.y ) - worldPos );
					int lRad = lightRadius;
					string lightRadstr = boost::lexical_cast<string>( lRad );
					lightPanel->textBoxes["rad"]->text.setString( lightRadstr );
				}
				cs.setRadius( lightRadius );
				cs.setFillColor( Color::White );
				cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
				cs.setPosition( lightPos.x, lightPos.y );
				preScreenTex->draw( cs );
			}
		}

		if( zoomMultiple > 7 )
		{
			playerZoomIcon.setPosition( playerPosition.x, playerPosition.y );
			playerZoomIcon.setScale( zoomMultiple * 2, zoomMultiple * 2 );
			preScreenTex->draw( playerZoomIcon );
		}
		
		preScreenTex->setView( uiView );


		switch( mode )
		{
			case CREATE_TERRAIN:
				{
					break;
				}
			case CREATE_ENEMY:
				{
					gs.Draw( preScreenTex );
					//if( showPanel != NULL )
					//{
					//	showPanel->Draw( w );
					//}
					break;
				}
			case SELECT_MODE:
				{
					preScreenTex->draw( guiMenuSprite );


					Color c;


					CircleShape cs;
					cs.setRadius( circleRadius );
					cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );


					cs.setFillColor( COLOR_BLUE );
					cs.setPosition( (menuDownPos + upperRightPos).x, (menuDownPos + upperRightPos).y );
					preScreenTex->draw( cs );

					sf::Text textblue;
					textblue.setCharacterSize( 14 );
					textblue.setFont( arial );
					textblue.setString( "CREATE\nTERRAIN" );
					textblue.setColor( sf::Color::White );
					textblue.setOrigin( textblue.getLocalBounds().width / 2, textblue.getLocalBounds().height / 2 );
					textblue.setPosition( (menuDownPos + upperRightPos).x, (menuDownPos + upperRightPos).y );
					preScreenTex->draw( textblue);


					cs.setFillColor( COLOR_GREEN );
					cs.setPosition( (menuDownPos + lowerRightPos).x, (menuDownPos + lowerRightPos).y );
					preScreenTex->draw( cs );

					sf::Text textgreen;
					textgreen.setCharacterSize( 14 );
					textgreen.setFont( arial );
					textgreen.setString( "MAP\nOPTIONS" );
					textgreen.setColor( sf::Color::White );
					textgreen.setOrigin( textgreen.getLocalBounds().width / 2, textgreen.getLocalBounds().height / 2 );
					textgreen.setPosition( (menuDownPos + lowerRightPos).x, (menuDownPos + lowerRightPos).y );
					preScreenTex->draw( textgreen );


					cs.setFillColor( COLOR_YELLOW );
					cs.setPosition( (menuDownPos + bottomPos).x, (menuDownPos + bottomPos).y );
					preScreenTex->draw( cs );

					cs.setFillColor( COLOR_ORANGE );
					cs.setPosition( (menuDownPos + lowerLeftPos).x, (menuDownPos + lowerLeftPos).y );
					preScreenTex->draw( cs );

					sf::Text textorange;
					textorange.setString( "CREATE\nLIGHTS" );
					textorange.setFont( arial );
					textorange.setCharacterSize( 14 );
					textorange.setColor( sf::Color::White );
					textorange.setOrigin( textorange.getLocalBounds().width / 2, textorange.getLocalBounds().height / 2 );
					textorange.setPosition( (menuDownPos + lowerLeftPos).x, (menuDownPos + lowerLeftPos).y );
					preScreenTex->draw( textorange );

					cs.setFillColor( COLOR_RED );
					cs.setPosition( (menuDownPos + upperLeftPos).x, (menuDownPos + upperLeftPos).y );
					preScreenTex->draw( cs );

					sf::Text textred;
					textred.setString( "CREATE\nENEMIES" );
					textred.setFont( arial );
					textred.setCharacterSize( 14 );
					textred.setColor( sf::Color::White );
					textred.setOrigin( textred.getLocalBounds().width / 2, textred.getLocalBounds().height / 2 );
					textred.setPosition( (menuDownPos + upperLeftPos).x, (menuDownPos + upperLeftPos).y );
					preScreenTex->draw( textred );

					cs.setFillColor( COLOR_MAGENTA );
					cs.setPosition( (menuDownPos + topPos).x, (menuDownPos + topPos).y );
					preScreenTex->draw( cs );

					sf::Text textmag;
					if( menuDownStored == EditSession::EDIT && selectedActor != NULL )
					{
						textmag.setString( "EDIT\nENEMY" );
					}
					else if( menuDownStored == EditSession::EDIT && selectedPolygons.size() > 0 )
					{
						textmag.setString( "TERRAIN\nOPTIONS" );
					}
					else if( menuDownStored == EditSession::EDIT && selectedLight != NULL )
					{
						textmag.setString( "LIGHT\nOPTIONS" );
					}
					else
					{
						textmag.setString( "EDIT" );
					}
				
					
					textmag.setFont( arial );
					textmag.setCharacterSize( 14 );
					textmag.setColor( sf::Color::White );
					textmag.setOrigin( textmag.getLocalBounds().width / 2, textmag.getLocalBounds().height / 2 );
					textmag.setPosition( (menuDownPos + topPos).x, (menuDownPos + topPos).y );
					preScreenTex->draw( textmag );

					break;
				}
			case EDIT:
				{
					
					break;
				}
		}

		if( showPanel != NULL )
		{
			//cout << "drawing panel" << endl;
			showPanel->Draw( preScreenTex );
		}

		if( popupPanel != NULL )
		{
			popupPanel->Draw( preScreenTex );
		}

		preScreenTex->setView( view );


		preScreenTex->display();
		const Texture &preTex = preScreenTex->getTexture();
		
		Sprite preTexSprite( preTex );
		preTexSprite.setPosition( -960 / 2, -540 / 2 );
		preTexSprite.setScale( .5, .5 );	
		w->clear();
		w->draw( preTexSprite  );
		w->display();
	}
	

	
	return returnVal;
	
}

bool EditSession::PointValid( Vector2i prev, Vector2i point)
{
	//return true;
	float eLeft = min( prev.x, point.x );
	float eRight= max( prev.x, point.x );
	float eTop = min( prev.y, point.y );
	float eBottom = max( prev.y, point.y );

	{
		PointList::iterator it = polygonInProgress->points.begin();
		//polygonInProgress->points.push_back( polygonInProgress->points.back() )
		Vector2i pre = (*it).pos;
		++it;
		
		//minimum angle
		{
			if( polygonInProgress->points.size() >= 2 )
			{
				PointList::reverse_iterator rit = polygonInProgress->points.rbegin();
				rit++;
				double ff = dot( normalize( V2d( point.x, point.y ) - V2d( polygonInProgress->points.back().pos.x, polygonInProgress->points.back().pos.y ) )
					, normalize( V2d((*rit).pos.x, (*rit).pos.y ) - V2d( polygonInProgress->points.back().pos.x, polygonInProgress->points.back().pos.y ) ) );
				if( ff > minAngle )
				{
					//cout << "ff: " << ff << endl;
					return false;
				}
			}
		}

		//return true;

		//make sure I'm not too close to the very first point and that my line isn't too close to the first point either
		//if( length( V2d( point.x, point.y ) - V2d( polygonInProgress->points.front().pos.x, polygonInProgress->points.front().pos.y ) ) < 8 )
		{
			double separation = length( V2d(point.x, point.y) - V2d(pre.x, pre.y) );
			if( separation < minimumEdgeLength )
			{
				return false;
			}

			if( polygonInProgress->points.size() > 2  )
			{
				if( abs( cross( V2d( point.x, point.y ) - V2d( prev.x, prev.y), 
					normalize( V2d( pre.x, pre.y ) - V2d( prev.x, prev.y ) ) ) ) < minimumEdgeLength
					&& dot( V2d( point.x, point.y ) - V2d( prev.x, prev.y ), normalize( V2d( pre.x, pre.y ) - V2d( prev.x, prev.y )) ) 
					>= length( V2d( pre.x, pre.y ) - V2d( prev.x, prev.y ) ) )
				{
					return false;
				}
			}
		}

		//check for distance to point in the polygon and edge distances

		if( point.x == polygonInProgress->points.front().pos.x && point.y == polygonInProgress->points.front().pos.y )
		{
			pre = (*it).pos;
			++it;
		}

		{
 		for( ; it != polygonInProgress->points.end(); ++it )
		{
			if( (*it).pos == polygonInProgress->points.back().pos )
				continue;

			LineIntersection li = lineIntersection( V2d( prev.x, prev.y ), V2d( point.x, point.y ),
						V2d( pre.x, pre.y ), V2d( (*it).pos.x, (*it).pos.y ) );
			float tempLeft = min( pre.x, (*it).pos.x ) - 0;
			float tempRight = max( pre.x, (*it).pos.x ) + 0;
			float tempTop = min( pre.y, (*it).pos.y ) - 0;
			float tempBottom = max( pre.y, (*it).pos.y ) + 0;
			if( !li.parallel )
			{
				
				double separation = length( V2d(point.x, point.y) - V2d((*it).pos.x,(*it).pos.y ) );
				
				if( li.position.x <= tempRight && li.position.x >= tempLeft && li.position.y >= tempTop && li.position.y <= tempBottom )
				{
					if( li.position.x <= eRight && li.position.x >= eLeft && li.position.y >= eTop && li.position.y <= eBottom )
					{
						CircleShape cs;
						cs.setRadius( 30  );
						cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
						cs.setFillColor( Color::Magenta );
						cs.setPosition( li.position.x, li.position.y );
						preScreenTex->draw( cs );

						
						return false;
					}

				}

				if( separation < minimumEdgeLength )
				{
					
					return false;
				}

				Vector2i ai = point - pre;
				Vector2i bi = (*it).pos - pre;
				V2d a(ai.x, ai.y);
				V2d b(bi.x, bi.y);
				double res = abs(cross( a, normalize( b )));
				double des = dot( a, normalize( b ));

				Vector2i ci = (*it).pos - prev;
				Vector2i di = point - prev;
				V2d c( ci.x, ci.y);
				V2d d( di.x, di.y );

				double res2 = abs( cross( c, normalize( d ) ) );
				double des2 = dot( c, normalize( d ) );

				//cout << "minedgelength: " << minimumEdgeLength <<  ", " << res << endl;

				if( point.x == polygonInProgress->points.front().pos.x && point.y == polygonInProgress->points.front().pos.y )
				{
				}
				else

				if(( res  < minimumEdgeLength && ( des >= 0 && des <= length( b ) ) )
					|| ( res2  < minimumEdgeLength && ( des2 >= 0 && des2 <= length( d ) ) ) )
				{
					return false;
				}
			}
			else
			{
				//cout << "parallel" << endl;
				//return false;
			}
			pre = (*it).pos;
		}
		}
	}
	return true;

	int i = 0;
	for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		//cout << "polygon " << i << " out of " << polygons.size() << " ... " << (*it)->points.size()  << endl;
		++i;
		TerrainPolygon *p = (*it);
		
		if( eLeft <= p->right && eRight >= p->left && eTop <= p->bottom && eBottom >= p->top )
		{	
		
			//aabbCollision = true;
		
		}
		else
		{
			continue;
		}
	//	if( point.x <= p->right && point.x >= p->left && point.y >= p->top && point.y <= p->bottom )
	//	{
			PointList::iterator it2 = p->points.begin();
			Vector2i prevPoint = (*it2).pos;
			++it2;
			for( ; it2 != p->points.end(); ++it2 )
			{
				LineIntersection li = lineIntersection( V2d( prevPoint.x, prevPoint.y ), V2d((*it2).pos.x, (*it2).pos.y),
					V2d( prev.x, prev.y ), V2d( point.x, point.y ) );
				float tempLeft = min( prevPoint.x, (*it2).pos.x );
				float tempRight = max( prevPoint.x, (*it2).pos.x );
				float tempTop = min( prevPoint.y, (*it2).pos.y );
				float tempBottom = max( prevPoint.y, (*it2).pos.y );
				if( !li.parallel )
				{
					if( li.position.x <= tempRight && li.position.x >= tempLeft && li.position.y >= tempTop && li.position.y <= tempBottom )
					{
						if( li.position.x <= eRight && li.position.x >= eLeft && li.position.y >= eTop && li.position.y <= eBottom )
						{
							return false;
						}

					}
				}
				prevPoint = (*it2).pos;
				
			}
	}
	return true;
}

void EditSession::ButtonCallback( Button *b, const std::string & e )
{
	Panel *p = b->owner;
	if( p->name == "patroller_options" )
	{
		if( b->name == "ok" )
		{
			bool loop = p->checkBoxes["loop"]->checked;
			float speed = 1; 

			try
			{
				speed = boost::lexical_cast<int>( p->textBoxes["speed"]->text.getString().toAnsiString() );
			}
			catch(boost::bad_lexical_cast &)
			{
				//error
			}

			//showPanel = trackingEnemy->panel;
			//PatrollerParams *patroller = (PatrollerParams*)trackingEnemy;
			if( mode == EDIT && selectedActor != NULL )
			{
				PatrollerParams *patroller = (PatrollerParams*)selectedActor;
				patroller->speed = speed;
				patroller->loop = loop;
				patroller->SetPath( patrolPath );
			}
			else if( mode == CREATE_ENEMY )
			{
				PatrollerParams *patroller = new PatrollerParams( this, patrolPath.front(), patrolPath, speed, loop );
				groups["--"]->actors.push_back( patroller);
				patroller->group = groups["--"];
				trackingEnemy = NULL;
			}
			
			

			//ActorParams *actor = new PatrollerParams( this, patrolPath.front(), patrolPath, speed, loop );
			
			//patrolPath.clear();
			//actor->SetAsPatroller( types["patroller"], patrolPath.front(), patrolPath, speed, loop );
			
			//mode = CREATE_ENEMY;
			//patroller path should get set only from hitting the button within it to start the path check

			showPanel = NULL;
		}
		else if( b->name == "createpath" )
		{
			//PatrollerParams *patroller = (PatrollerParams*)selectedActor;

			showPanel = NULL;
			mode = CREATE_PATROL_PATH;
			Vector2i front = patrolPath.front();
			patrolPath.clear();
			patrolPath.push_back( front );
			//patrolPath.push_back( Vector2i( worldPos.x, worldPos.y ) );
		}
	}
	else if( p->name == "crawler_options" )
	{
		if( b->name == "ok" );
		{
			bool clockwise = p->checkBoxes["clockwise"]->checked;
			double speed;

			stringstream ss;
			string s = p->textBoxes["speed"]->text.getString().toAnsiString();
			ss << s;

			ss >> speed;

			if( ss.fail() )
			{
				cout << "stringstream to integer parsing error" << endl;
				ss.clear();
				assert( false );
			}

			if( mode == EDIT && selectedActor != NULL )
			{
				CrawlerParams *crawler = (CrawlerParams*)selectedActor;
				crawler->speed = speed;
				crawler->clockwise = clockwise;
			}
			else if( mode == CREATE_ENEMY )
			{
				CrawlerParams *crawler = new CrawlerParams( this, enemyEdgePolygon, enemyEdgeIndex, 
				enemyEdgeQuantity, clockwise, speed );
				groups["--"]->actors.push_back( crawler);
				crawler->group = groups["--"];
				trackingEnemy = NULL;
			}

			showPanel = NULL;
		}
	}
	else if( p->name == "basicturret_options" )
	{	
		if( b->name == "ok" )
		{
			stringstream ss;
			string bulletSpeedString = p->textBoxes["bulletspeed"]->text.getString().toAnsiString();
			string framesWaitString = p->textBoxes["waitframes"]->text.getString().toAnsiString();
			ss << bulletSpeedString;
			

			double bulletSpeed;
			ss >> bulletSpeed;

			if( ss.fail() )
			{
				assert( false );
			}

			ss.clear();

			ss << framesWaitString;

			int framesWait;
			ss >> framesWait;

			if( ss.fail() )
			{
				assert( false );
			}

			if( mode == EDIT && selectedActor != NULL )
			{
				BasicTurretParams *basicTurret = (BasicTurretParams*)selectedActor;
				basicTurret->bulletSpeed = bulletSpeed;
				basicTurret->framesWait = framesWait;
			}
			else if( mode == CREATE_ENEMY )
			{
				BasicTurretParams *basicTurret = new BasicTurretParams( this, enemyEdgePolygon, enemyEdgeIndex, 
				enemyEdgeQuantity, bulletSpeed, framesWait );
				groups["--"]->actors.push_back( basicTurret );
				basicTurret->group = groups["--"];
				trackingEnemy = NULL;
			}

			showPanel = NULL;
		}	
	}
	else if( p->name == "foottrap_options" )
	{
		if( b->name == "ok" )
		{
			if( mode == EDIT && selectedActor != NULL )
			{
				//FootTrapParams *footTrap = (FootTrapParams*)selectedActor;
				
			}
			else if( mode == CREATE_ENEMY )
			{
				FootTrapParams *footTrap = new FootTrapParams( this, enemyEdgePolygon, enemyEdgeIndex, 
				enemyEdgeQuantity );
				groups["--"]->actors.push_back( footTrap );
				footTrap->group = groups["--"];
				trackingEnemy = NULL;
			}

			showPanel = NULL;
		}
	}
	else if( p->name == "map_options" )
	{
		if( b->name == "ok" );
		{
			int minEdgeSize;

			stringstream ss;
			string s = p->textBoxes["minedgesize"]->text.getString().toAnsiString();
			ss << s;

			ss >> minEdgeSize;

			if( ss.fail() )
			{
				cout << "stringstream to integer parsing error" << endl;
				ss.clear();
				assert( false );
			}

			if( minEdgeSize < 8 )
			{
				minimumEdgeLength = 8;
				popupPanel = messagePopup;
				p->textBoxes["minedgesize"]->text.setString( "8" );
				messagePopup->labels["message"]->setString( "minimum edge length too low.\n Set to minimum of 8" );
				//assert( false && "made min edge length too small!" );
			}
			else
			{
				minimumEdgeLength = minEdgeSize;
			}

			showPanel = NULL;
		}
	}
	else if( p->name == "terrain_options" )
	{
		if( b->name == "ok" )
		{
			showPanel = NULL;
		}
		else if( b->name == "create_path" )
		{
			cout << "setting mode to create path terrain" << endl;
			mode = CREATE_TERRAIN_PATH;
			//patrolPath.clear();

			

			assert( selectedPolygons.size() > 0 );

			int left, right, top, bottom;
			list<TerrainPolygon*>::iterator it = selectedPolygons.begin();
			left = (*it)->left;
			right = (*it)->right;
			top = (*it)->top;
			bottom = (*it)->bottom;
			(*it)->path.clear();
			(*it)->path.push_back( Vector2i( 0, 0 ) );
			++it;

			for(  ;it != selectedPolygons.end(); ++it )
			{
				(*it)->path.clear();

				if( (*it)->left < left )
					left = (*it)->left;

				if( (*it)->right > right )
					right = (*it)->right;

				if( (*it)->top < top )
					top = (*it)->top;

				if( (*it)->bottom > bottom )
					bottom = (*it)->bottom;

				(*it)->path.push_back( Vector2i( 0, 0 ) );
			}

			fullRect.left = left;
			fullRect.top = top;
			fullRect.width = right - left;
			fullRect.height = bottom - top;


			for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
			{
				//it doesnt need to push this cuz its just storing the locals. draw from the center of the entire bounding box!

			//	(*it)->path.push_back( Vector2i( ((*it)->right + (*it)->left) / 2.0, ((*it)->bottom - (*it)->top) / 2.0 ) );
			}
			//patrolPath.push_back( Vector2i( worldPos.x, worldPos.y ) );
			showPanel = NULL;
		}
	}
	else if( p->name == "light_options" )
	{
		if( b->name == "ok" )
		{
			//cout << "OKAY!!!" << endl;

			int red;
			int green;
			int blue;
			int rad;
			int bright;

			stringstream ss;
			string redstr = p->textBoxes["red"]->text.getString().toAnsiString();
			string greenstr = p->textBoxes["green"]->text.getString().toAnsiString();
			string bluestr = p->textBoxes["blue"]->text.getString().toAnsiString();
			string radstr = p->textBoxes["rad"]->text.getString().toAnsiString();
			string brightstr = p->textBoxes["bright"]->text.getString().toAnsiString();

			ss << redstr << " " << greenstr << " " << bluestr << " " << radstr << " " << brightstr;

			ss >> red;
			ss >> green;
			ss >> blue;
			ss >> rad;
			ss >> bright;

			if( ss.fail() )
			{
				cout << "stringstream to integer parsing error" << endl;
				ss.clear();
				assert( false );
			}

			if( mode == EDIT && selectedLight != NULL )
			{
				selectedLight->color = Color( red, green, blue );
				selectedLight->radius = rad;
				selectedLight->brightness = bright;
			}
			else
			{
				lights.push_back( new StaticLight( Color( red, green, blue ), lightPos, rad, bright ) );
			}
			showPanel = NULL;
		}
	}
	/*else if( p->name == "message_popup" )
	{
		if( b->name == "ok" )
		{
			//showPanel = NULL;
			cout << "setting to null" << endl;
			popupPanel = NULL;
		}
	}*/
	else if( p->name == "error_popup" )
	{
		if( b->name == "ok" )
		{
			showPanel = NULL;
		}
	}
	else if( p->name == "confirmation_popup" )
	{
		if( b->name == "confirm" )
		{
			confirmChoice = ConfirmChoices::CONFIRM;
		}
		else if( b->name == "cancel" )
		{
			confirmChoice = ConfirmChoices::CANCEL;
		}
	}
	//cout <<"button" << endl;
}

void EditSession::TextBoxCallback( TextBox *tb, const std::string & e )
{
}

void EditSession::GridSelectorCallback( GridSelector *gs, const std::string & name )
{
	if( name != "not set" )
	{
		trackingEnemy = types[name];
		enemySprite.setTexture( trackingEnemy->imageTexture );

		enemySprite.setTextureRect( sf::IntRect( 0, 0, trackingEnemy->imageTexture.getSize().x, 
			trackingEnemy->imageTexture.getSize().y ) );

		enemySprite.setOrigin( enemySprite.getLocalBounds().width /2 , enemySprite.getLocalBounds().height / 2 );
	
		cout << "set your cursor as the image" << endl;
	}
	else
	{
		cout << "not set" << endl;
	}
}

void EditSession::CheckBoxCallback( CheckBox *cb, const std::string & e )
{
	cout << cb->name << " was " << e << endl;
}

Panel * EditSession::CreateOptionsPanel( const std::string &name )
{
	if( name == "patroller" )
	{
		Panel *p = new Panel( "patroller_options", 200, 400, this );
		p->AddButton( "ok", Vector2i( 100, 300 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );
		p->AddLabel( "loop_label", Vector2i( 20, 150 ), 20, "loop" );
		p->AddCheckBox( "loop", Vector2i( 120, 155 ) ); 
		p->AddTextBox( "speed", Vector2i( 20, 200 ), 200, 20, "10" );
		p->AddButton( "createpath", Vector2i( 20, 250 ), Vector2f( 100, 50 ), "Create Path" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
		//p->
	}
	else if( name == "crawler" )
	{
		Panel *p = new Panel( "crawler_options", 200, 400, this );
		p->AddButton( "ok", Vector2i( 100, 300 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		p->AddLabel( "clockwise_label", Vector2i( 20, 150 ), 20, "clockwise" );
		p->AddCheckBox( "clockwise", Vector2i( 120, 155 ) ); 
		p->AddTextBox( "speed", Vector2i( 20, 200 ), 200, 20, "10" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
	}
	else if( name == "basicturret" )
	{
		Panel *p = new Panel( "basicturret_options", 200, 400, this );
		p->AddButton( "ok", Vector2i( 100, 300 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		p->AddTextBox( "bulletspeed", Vector2i( 20, 150 ), 200, 20, "10" );
		p->AddTextBox( "waitframes", Vector2i( 20, 200 ), 200, 20, "10" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
	}
	else if( name == "foottrap" )
	{
		Panel *p = new Panel( "foottrap_options", 200, 400, this );
		p->AddButton( "ok", Vector2i( 100, 300 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
	}
	else if( name == "map" )
	{
		Panel *p = new Panel( "map_options", 200, 400, this );
		p->AddButton( "ok", Vector2i( 100, 300 ), Vector2f( 100, 50 ), "OK" );
		p->AddLabel( "minedgesize_label", Vector2i( 20, 150 ), 20, "minimum edge size:" );
		p->AddTextBox( "minedgesize", Vector2i( 20, 20 ), 200, 20, "8" );
		return p;
	}
	else if( name == "terrain" )
	{
		Panel *p = new Panel( "terrain_options", 200, 400, this );
		p->AddButton( "ok", Vector2i( 100, 300 ), Vector2f( 100, 50 ), "OK" );
		//p->AddLabel( "minedgesize_label", Vector2i( 20, 150 ), 20, "minimum edge size:" );
		//p->AddTextBox( "minedgesize", Vector2i( 20, 20 ), 200, 20, "8" );
		//p->AddButton( "create_path", Vector2i( 100, 0 ), Vector2f( 100, 50 ), "Create Path" );
		
		return p;
	}
	else if( name == "light" )
	{
		Panel *p = new Panel( "light_options", 240, 300, this );
		int textBoxX = 130;
		p->AddButton( "ok", Vector2i( 100, 230 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "red", Vector2i( textBoxX, 20 ), 60, 3, "255" );
		p->AddTextBox( "green", Vector2i( textBoxX, 60 ), 60, 3, "0" );
		p->AddTextBox( "blue", Vector2i( textBoxX, 100 ), 60, 3, "0" );
		p->AddTextBox( "rad", Vector2i( textBoxX, 140 ), 60, 3, "1" );
		p->AddTextBox( "bright", Vector2i( textBoxX, 180 ), 60, 3, "1" );
		
		p->AddLabel( "red_label", Vector2i( 20, 20 ), 20, "Red: " );
		p->AddLabel( "green_label", Vector2i( 20, 60 ), 20, "Green: " );
		p->AddLabel( "blue_label", Vector2i( 20, 100 ), 20, "Blue: " );
		p->AddLabel( "rad_label", Vector2i( 20, 140 ), 20, "Radius: " );
		p->AddLabel( "bright_label", Vector2i( 20, 180 ), 20, "Brightness: " );
		return p;
	}
	return NULL;
}

int EditSession::CountSelectedPoints()
{
	int count = 0;
	for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
	{
		for( PointList::iterator it2 = (*it)->points.begin(); it2 != (*it)->points.end(); ++it2 )
		{
			if( (*it2).selected ) //selected
			{
				++count;
			}
		}
	}
	return count;
}

void EditSession::ExtendPolygon()
{
	if( polygonInProgress->points.size() > 1 )
	{
		//test final line

		list<TerrainPolygon*>::iterator it = polygons.begin();
		bool added = false;
		polygonInProgress->Finalize();
		bool recursionDone = false;
		TerrainPolygon *currentBrush = polygonInProgress;

			while( it != polygons.end() )
			{
				TerrainPolygon *temp = (*it);
				if( temp != currentBrush && currentBrush->IsTouching( temp ) )
				{
					//cout << "before addi: " << (*it)->points.size() << endl;
						
					Add( currentBrush, temp );

					polygonInProgress->Reset();
						
					//cout << "after adding: " << (*it)->points.size() << endl;
					polygons.erase( it );

					currentBrush = temp;

					it = polygons.begin();

					added = true;
							
					continue;
				}
				else
				{
					//cout << "not" << endl;
				}
				++it;
			}
				
		//add final check for validity here
				
		if( !added )
		{
			//polygonInProgress->Finalize();
			//polygons.push_back( polygonInProgress );
			//polygonInProgress = new TerrainPolygon(&grassTex );
		}
		else
		{
			polygons.push_back( currentBrush );
			//polygonInProgress->Reset();
		}
	}


	polygonInProgress->Reset();
}

bool EditSession::IsPointValid( sf::Vector2i oldPoint, sf::Vector2i point, TerrainPolygon *poly )
{
	//cout << "checking if the point is valid!!" << endl;
	//check distance from points first

	V2d p( point.x, point.y );
	//cout << "p: " << p.x << ", " << p.y << endl;
	for( PointList::iterator it = poly->points.begin(); it != poly->points.end(); ++it )
	{
		if( (*it).pos != point )
		{
			V2d temp( (*it).pos.x, (*it).pos.y );
			if( length( p - temp ) < validityRadius )
			{
				cout << "false type one:" << length( p - temp ) << " .. " << temp.x << ", " << temp.y << ", p: " << p.x << ", " << p.y << endl;
				return false;
			}
		}
	}
	PointList::iterator it = poly->points.begin();
	PointList::iterator prev = poly->points.end(); 
	--prev;
	for( ; it != poly->points.end(); ++it )
	{
		if( (*it).pos == oldPoint || (*prev).pos == oldPoint )
		{
			prev = it;
			continue;
		}

		V2d v0 = V2d( (*prev).pos.x, (*prev).pos.y );
		V2d v1 = V2d( (*it).pos.x, (*it).pos.y );
		V2d edgeDir = normalize( v1 - v0 );

		double quant = dot( p - v0, edgeDir );
		double offQuant = cross( p - v0, edgeDir );
		//cout << "quant: " << quant << ", l: " << length( v1 - v0 ) << endl;
		bool nearOnAxis = quant > 0 && quant < length( v1 - v0 );
		bool nearOffAxis = abs( offQuant ) < validityRadius;

		if( nearOnAxis && nearOffAxis )
		{
			cout << "false type two. quant:" << quant << ", offquant: " << offQuant << endl;
			cout << "p: " << p.x << ", " << p.y << endl;
			cout << "v0: " << v0.x << ", " << v0.y << endl;
			cout << "v1: " << v1.x << ", " << v1.y << endl;
			return false;
		}

		V2d pointDir;
		pointDir.x = point.x - oldPoint.x;
		pointDir.y = point.y - oldPoint.y;
		pointDir = normalize( pointDir );

		V2d old( oldPoint.x, oldPoint.y );
		double otherQuant = dot( v1 - old, pointDir );
		double otherOffQuant = cross( v1 - old, pointDir );
		
		bool otherNearOnAxis = otherQuant > 0 && otherQuant < length( V2d( point.x, point.y ) - old );
		bool otherNearOffAxis = abs( otherOffQuant ) < validityRadius;//otherOffQuant >= 0 && otherOffQuant < validityRadius;//abs( otherOffQuant ) < validityRadius;

		
		if( otherNearOnAxis && otherNearOffAxis )
		{
			
			cout << "false type three. quant: " << otherQuant << ", offQuant: " << otherOffQuant << ", " << (v1-old).x <<", " << (v1-old).y << endl;
			//cout << "v1: " << v1.x << ", " << v1.y << ", old: " << old.x << ", " << old.y << endl;

			/*CircleShape cs;
			cs.setRadius( 50 );
			cs.setFillColor( Color::Red );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( point.x, point.y );
			preScreenTex->draw( cs );*/
			return false;
		}


		prev = it;
	}

	


	return true;
}

bool EditSession::IsPolygonValid( TerrainPolygon &poly, TerrainPolygon *ignore )
{
	Rect<int> polyAABB = poly.TempAABB();
	polyAABB.left -= minimumEdgeLength;
	polyAABB.top -= minimumEdgeLength;
	polyAABB.width += minimumEdgeLength * 2;
	polyAABB.height += minimumEdgeLength * 2;

	for( PointList::iterator it = poly.points.begin(); it != poly.points.end(); ++it )
	{
		PointList::iterator prev;
		if( it == poly.points.begin() )
		{
			prev = poly.points.end();
			prev--;
		}
		else
		{
			prev = it;
			prev--;
		}

		V2d extreme( 0, 0 );
		Vector2i vec = (*it).pos - (*prev).pos;
		V2d normVec = normalize( V2d( vec.x, vec.y ) );
		

		if( normVec.x == 0 || normVec.y == 0 )
		{
			continue;
		}

		if( normVec.x > PRIMARY_LIMIT )
			extreme.x = 1;
		else if( normVec.x < -PRIMARY_LIMIT )
			extreme.x = -1;
		if( normVec.y > PRIMARY_LIMIT )
			extreme.y = 1;
		else if( normVec.y < -PRIMARY_LIMIT )
			extreme.y = -1;

		//extreme = normalize( extreme );

		if( extreme.x != 0 )
		{
			(*it).pos.y = (*prev).pos.y;
			cout << "lining up x" << endl;
		}

		if( extreme.y != 0 )
		{
			(*it).pos.x = (*prev).pos.x;
			cout << "lining up y" << endl;
		}
		/*if( !( extreme.x == 0 && extreme.y == 0 ) )
		{


				return false;
		}*/
	}

	if( !poly.IsClockwise() )
	{
		return false;
	}


	//points close to other points on myself
	for( PointList::iterator it = poly.points.begin(); it != poly.points.end(); ++it )
	{
		for( PointList::iterator it2 = poly.points.begin(); it2 != poly.points.end(); ++it2 )
		{
			if( (*it).pos.x == (*it2).pos.x && (*it).pos.y == (*it2).pos.y )
			{
				continue;
			}

			V2d a( (*it).pos.x, (*it).pos.y );
			V2d b( (*it2).pos.x, (*it2).pos.y );
			if( length( a - b ) < validityRadius )
			{
				//cout << "len: " << length( a - b ) << endl;
				return false;
			}
		}
	}

	//points close to lines on myself. do i need the previous stuff
	for( PointList::iterator it = poly.points.begin(); it != poly.points.end(); ++it )
	{
		PointList::iterator prev, next;
		if( it == poly.points.begin() )
		{
			prev = poly.points.end();
			prev--;
		}
		else
		{
			prev = it;
			prev--;
		}

		PointList::iterator temp = it;
		++temp;
		if( temp == poly.points.end() )
		{
			next = poly.points.begin();
		}
		else
		{
			next = it;
			next++;
		}

		//test for minimum angle difference between edges
		V2d pos((*it).pos.x, (*it).pos.y );
		V2d prevPos( (*prev).pos.x, (*prev).pos.y );
		V2d nextPos( (*next).pos.x, (*next).pos.y );

		
		double ff = dot( normalize( prevPos - pos ), normalize( nextPos - pos ) );
		if( ff > minAngle )
		{
			//cout << "ff: " << ff << endl;
			return false;
		} 

		if( !IsPointValid( (*prev).pos, (*it).pos, &poly ) )
		{
			return false;
		}
	}


	//line intersection on myself
	for( PointList::iterator it = poly.points.begin(); it != poly.points.end(); ++it )
	{
		PointList::iterator prev;
		if( it == poly.points.begin() )
		{
			prev = poly.points.end();
			prev--;
		}
		else
		{
			prev = it;
			prev--;
		}

		for( PointList::iterator it2 = poly.points.begin(); it2 != poly.points.end(); ++it2 )
		{
			PointList::iterator prev2;
			if( it2 == poly.points.begin() )
			{
				prev2 = poly.points.end();
					prev2--;
			}
			else
			{
				prev2 = it2;
				prev2--;
			}

			if( prev2 == prev || prev2 == it || it2 == prev || it2 == it )
			{
				continue;
			}

			LineIntersection li = LimitSegmentIntersect( (*prev).pos, (*it).pos, (*prev2).pos, (*it2).pos );

			if( !li.parallel )
			{
				return false;
			}
		}
	}

	for( list<TerrainPolygon*>::iterator polyIt = polygons.begin(); polyIt != polygons.end(); ++polyIt )
	{
		if( ignore == (*polyIt ) )
		{
			continue;
		}
		//eventually do a quad tree for this to speed it up
		Rect<int> currAABB = (*polyIt)->TempAABB();
		currAABB.left -= minimumEdgeLength;
		currAABB.top -= minimumEdgeLength;
		currAABB.width += minimumEdgeLength * 2;
		currAABB.height += minimumEdgeLength * 2;

		if( !polyAABB.intersects( currAABB ) )
		{
			continue;
		}

		//points vs points
		/*for( PointList::iterator pit = (*polyIt)->points.begin(); pit != (*polyIt)->points.end(); ++pit )
		{
			for( PointList::iterator myPit = poly.points.begin(); myPit != poly.points.end(); ++myPit )
			{
				V2d mine( (*myPit).pos.x, (*myPit).pos.y );
				V2d other( (*pit).pos.x, (*pit).pos.y );

				if( length( mine - other ) < minimumEdgeLength )
				{
					return false;
				}
			}
		}*/


		//my points vs his lines
		for( PointList::iterator myPit = poly.points.begin(); myPit != poly.points.end(); ++myPit )
		{
			Vector2i oldPoint, currPoint;
			if( myPit == poly.points.begin() )
			{
				PointList::iterator temp = poly.points.end();
				--temp;
				oldPoint = (*temp).pos;
			}
			else
			{
				PointList::iterator temp = myPit;
				--temp;
				oldPoint = (*temp).pos;
			}

			currPoint = (*myPit).pos;

			if( !IsPointValid( oldPoint, currPoint, (*polyIt) ) )
			{

				cout << "a: old: " << oldPoint.x << ", " << oldPoint.y << ", curr: " << currPoint.x << ", " << currPoint.y << endl;
				return false;
			}
			//IsPointValid(
		}

		//his points vs my lines
		for( PointList::iterator pit = (*polyIt)->points.begin(); pit != (*polyIt)->points.end(); ++pit )
		{
			Vector2i oldPoint, currPoint;
			if( pit == (*polyIt)->points.begin() )
			{
				PointList::iterator temp = (*polyIt)->points.end();
				--temp;
				oldPoint = (*temp).pos;
			}
			else
			{
				PointList::iterator temp = pit;
				--temp;
				oldPoint = (*temp).pos;
			}

			currPoint = (*pit).pos;

			if( !IsPointValid( oldPoint, currPoint, &poly ) )
			{
				cout << "b: old: " << oldPoint.x << ", " << oldPoint.y << ", curr: " << currPoint.x << ", " << currPoint.y << endl;
				return false;
			}
			//IsPointValid(
		}

		//my lines vs his lines
		for( PointList::iterator myPit = poly.points.begin(); myPit != poly.points.end(); ++myPit )
		{
			PointList::iterator myPrev;
			if( myPit == poly.points.begin() )
			{
				PointList::iterator temp = poly.points.end();
				--temp;
				myPrev = temp;
			}
			else
			{
				myPrev = myPit;
				myPrev--;
			}

			for( PointList::iterator pit = (*polyIt)->points.begin(); pit != (*polyIt)->points.end(); ++pit )
			{
				PointList::iterator prev;
				if( pit == (*polyIt)->points.begin() )
				{
					PointList::iterator temp = (*polyIt)->points.end();
					--temp;
					prev = temp;
				}
				else
				{
					prev = pit;
					--prev;
				}

				LineIntersection li = SegmentIntersect( (*myPrev).pos, (*myPit).pos, (*prev).pos, (*pit).pos );
				if( !li.parallel )
				{
					return false;
				}
			}
		}



		//hes inside me w/ no intersection
		for( PointList::iterator pit = (*polyIt)->points.begin(); pit != (*polyIt)->points.end(); ++pit )
		{
			if( poly.ContainsPoint( Vector2f( (*pit).pos.x, (*pit).pos.y ) ) )
			{
				cout << "c" << endl;
				return false;
			}
		}


		//im inside him w/ no intersection
		for( PointList::iterator myPit = poly.points.begin(); myPit != poly.points.end(); ++myPit )
		{
			if( (*polyIt)->ContainsPoint( Vector2f( (*myPit).pos.x, (*myPit).pos.y ) ) )
			{
				cout << "d" << endl;
				return false;
			}
		}
	}

	//cout << "true" << endl;
	return true;
}

void EditSession::ExtendAdd()
{
	list<TerrainPolygon*>::iterator it = polygons.begin();
	bool added = false;
	//polygonInProgress->Finalize();
	bool recursionDone = false;
	TerrainPolygon *currentBrush = extendingPolygon;

	showPoints = false;
	extendingPolygon = NULL;
	extendingPoint = NULL;

	while( it != polygons.end() )
	{
		TerrainPolygon *temp = (*it);
		if( temp != currentBrush && currentBrush->IsTouching( temp ) )
		{
			//cout << "before addi: " << (*it)->points.size() << endl;
			/*cout << "currisze: " << currentBrush->points.size() << ", tempsize: " << temp->points.size() << endl;	
			cout << "curr: "<< endl;
			for( PointList::iterator pit = currentBrush->points.begin(); pit != currentBrush->points.end(); ++pit )
			{
				cout << (*pit).pos.x << ", " << (*pit).pos.y << endl;
			}
			cout << "temp: "<< endl;
			for( PointList::iterator pit = temp->points.begin(); pit != temp->points.end(); ++pit )
			{
				cout << (*pit).pos.x << ", " << (*pit).pos.y << endl;
			}*/
			Add( currentBrush, temp );
			
			//currentBrush->Reset();
			polygons.remove( currentBrush );
			delete currentBrush;

			//polygonInProgress->Reset();
						
			//cout << "after adding: " << (*it)->points.size() << endl;
			polygons.erase( it );

			currentBrush = temp;

			it = polygons.begin();

			cout << "number of polygons!: " << polygons.size() << endl;

			added = true;
							
			continue;
		}
		else
		{
			//cout << "not" << endl;
		}
		++it;
	}
				
	//add final check for validity here
				
	if( !added )
	{
		//polygonInProgress->Finalize();
		//polygons.push_back( polygonInProgress );
		//polygonInProgress = new TerrainPolygon( &grassTex );
	}
	else
	{

		polygons.push_back( currentBrush );
		//polygonInProgress->Reset();
	}
}

bool EditSession::ConfirmationPop()
{
	confirmChoice = ConfirmChoices::NONE;

	w->setView( v );
	
	//preScreenTex->setView( uiView );	
	
	preScreenTex->display();
	const Texture &preTex = preScreenTex->getTexture();
	Sprite preTexSprite( preTex );
	preTexSprite.setPosition( -960 / 2, -540 / 2 );
	preTexSprite.setScale( .5, .5 );	

	preScreenTex->setView( uiView );
	
	//preScreenTex->setView( view );
	
	//cout << "uiMouse: " << uiMouse.x << ", " << uiMouse.y << endl;

	sf::Event ev;
	while( confirmChoice == ConfirmChoices::NONE )
	{
		Vector2i pixelPos = sf::Mouse::getPosition( *w );
		Vector2f uiMouse = preScreenTex->mapPixelToCoords( pixelPos );
		w->clear();
		while( w->pollEvent( ev ) )
		{
			switch( ev.type )
			{
			case Event::MouseButtonPressed:
				{
					if( ev.mouseButton.button == Mouse::Left )
					{
						confirm->Update( true, uiMouse.x, uiMouse.y );		
					}			
					break;
				}
			case Event::MouseButtonReleased:
				{
					confirm->Update( false, uiMouse.x, uiMouse.y );
					break;
				}
			case Event::MouseWheelMoved:
				{
					break;
				}
			case Event::KeyPressed:
				{
					confirm->SendKey( ev.key.code, ev.key.shift );
					break;
				}
			case Event::KeyReleased:
				{
					break;
				}
			case Event::LostFocus:
				{
					break;
				}
			case Event::GainedFocus:
				{
					break;
				}
			}
					break;	
		}
		//cout << "drawing confirm" << endl;

		w->setView( v );

		w->draw( preTexSprite );

		w->setView( uiView );

		confirm->Draw( w );

		w->setView( v );

		w->display();
		//preScreenTex->display();
	}

	preScreenTex->setView( view );
	w->setView( v );
	//preScreenTex->setView( view );

	if( confirmChoice == ConfirmChoices::CONFIRM )
	{
		return true;
	}
	else if( confirmChoice == ConfirmChoices::CANCEL )
	{
		return false;
	}
}

void EditSession::MessagePop( const std::string &message )
{
	messagePopup->labels["message"]->setString( message );
	bool closePopup = false;
	w->setView( v );
	
	preScreenTex->display();
	const Texture &preTex = preScreenTex->getTexture();
	Sprite preTexSprite( preTex );
	preTexSprite.setPosition( -960 / 2, -540 / 2 );
	preTexSprite.setScale( .5, .5 );	

	preScreenTex->setView( uiView );

	sf::Event ev;
	while( !closePopup )
	{
		Vector2i pixelPos = sf::Mouse::getPosition( *w );
		Vector2f uiMouse = preScreenTex->mapPixelToCoords( pixelPos );
		w->clear();

		while( w->pollEvent( ev ) )
		{
			switch( ev.type )
			{
			case Event::MouseButtonPressed:
				{
					if( ev.mouseButton.button == Mouse::Left )
					{
						//if( uiMouse.x < messagePopup->pos.x 
						//messagePopup->Update( true, uiMouse.x, uiMouse.y );		
					}			
					break;
				}
			case Event::MouseButtonReleased:
				{
					closePopup = true;
					//messagePopup->Update( false, uiMouse.x, uiMouse.y );
					break;
				}
			case Event::MouseWheelMoved:
				{
					break;
				}
			case Event::KeyPressed:
				{
					closePopup = true;
					//messagePopup->SendKey( ev.key.code, ev.key.shift );
					break;
				}
			case Event::KeyReleased:
				{
					break;
				}
			case Event::LostFocus:
				{
					break;
				}
			case Event::GainedFocus:
				{
					break;
				}
			}
			break;	
		}

		w->setView( v );

		w->draw( preTexSprite );

		w->setView( uiView );

		messagePopup->Draw( w );

		w->setView( v );

		w->display();
	}

	preScreenTex->setView( view );
	w->setView( v );
	//preScreenTex->setView( view );
}

void EditSession::ErrorPop( const std::string &error )
{

}

Panel * EditSession::CreatePopupPanel( const std::string &type )
{
	if( type == "message" )
	{
		Panel *p = new Panel( "message_popup", 400, 100, this );
		p->pos.x = 300;
		p->pos.y = 300;
		//p->AddButton( "ok", Vector2i( 250, 25 ), Vector2f( 100, 50 ), "OK" );
		p->AddLabel( "message", Vector2i( 10, 10 ), 12, "_EMPTY\n_MESSAGE_" );
		p->pos = Vector2i( 960 - p->size.x / 2, 540 - p->size.y );
		return p;
		//p->
	}
	else if( type == "error" )
	{
		Panel *p = new Panel( "error_popup", 400, 100, this );
		//p->AddButton( "ok", Vector2i( 250, 25 ), Vector2f( 100, 50 ), "OK" );
		p->AddLabel( "message", Vector2i( 25, 50 ), 12, "_EMPTY_ERROR_" );
		p->pos = Vector2i( 960 - p->size.x / 2, 540 - p->size.y );
		return p;
	}
	else if( type == "confirmation" )
	{
		Panel *p = new Panel( "confirmation_popup", 400, 100, this );
		p->AddButton( "confirm", Vector2i( 50, 25 ), Vector2f( 100, 50 ), "Confirm" );
		p->AddButton( "cancel", Vector2i( 250, 25 ), Vector2f( 100, 50 ), "Cancel" );
		p->pos = Vector2i( 960 - p->size.x / 2, 540 - p->size.y );
		//p->AddLabel( "Cancel", Vector2i( 25, 50 ), 12, "_EMPTY_ERROR_" );
		return p;
	}

	return NULL;
}

bool EditSession::IsRemovePointsOkay()
{
	for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
		it != selectedPolygons.end(); ++it )
	{
		if( !(*it)->IsRemovePointsOkay( this ) )
			return false;
		//(*it)->RemoveSelectedPoints();
	}
	return true;
}

void EditSession::SetEnemyEditPanel()
{
	//eventually set this up so that I can give the same parameters to multiple copies of the same enemy?
	//need to be able to apply paths simultaneously to multiples also
	ActorType *type = selectedActor->type;
	string name = type->name;

	if( name == "patroller" )
	{
		PatrollerParams *patroller = (PatrollerParams*)selectedActor;
		
		/*Panel *p = new Panel( "patroller_options", 200, 400, this );
		p->AddButton( "ok", Vector2i( 100, 300 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );
		p->AddLabel( "loop_label", Vector2i( 20, 150 ), 20, "loop" );
		p->AddCheckBox( "loop", Vector2i( 120, 155 ) ); 
		p->AddTextBox( "speed", Vector2i( 20, 200 ), 200, 20, "10" );
		p->AddButton( "createpath", Vector2i( 20, 250 ), Vector2f( 100, 50 ), "Create Path" );*/
		Panel *p = type->panel;
		p->textBoxes["speed"]->text.setString( boost::lexical_cast<string>( patroller->speed ) );
		p->checkBoxes["loop"]->checked = patroller->loop;
		patrolPath = patroller->GetGlobalPath();
		showPanel = p;
	}
	else if( name == "crawler" )
	{
		CrawlerParams *crawler = (CrawlerParams*)selectedActor;
	}
	else if( name == "basicturret" )
	{
		BasicTurretParams *basicTurret = (BasicTurretParams*)selectedActor;
	}
	else if( name == "foottrap" )
	{
		FootTrapParams *footTrap = (FootTrapParams*)selectedActor;
	}
}

sf::Vector2<double> EditSession::GraphPos( sf::Vector2<double> realPos )
{
	int adjX, adjY;			
	realPos.x /= 32;
	realPos.y /= 32;

	if( realPos.x > 0 )
		realPos.x += .5f;
	else if( realPos.x < 0 )
		realPos.x -= .5f;

	if( realPos.y > 0 )
		realPos.y += .5f;
	else if( realPos.y < 0 )
		realPos.y -= .5f;

	adjX = ((int)realPos.x) * 32;
	adjY = ((int)realPos.y) * 32;

	return V2d( adjX, adjY );
}

ActorType::ActorType( const std::string & n, Panel *p )
	:name( n ), panel( p )
{
	
	iconTexture.loadFromFile( name + "_icon.png" );
	//icon.setTexture( iconTexture );
	imageTexture.loadFromFile( name + "_editor.png" );
	//image.setTexture( imageTexture );
}


ActorParams::ActorParams()
	:ground( NULL ), groundQuantity( 420.69 )
{
}

void ActorParams::Draw( sf::RenderTarget *target )
{
	target->draw( image );
}

void ActorParams::WriteFile( ofstream &of )
{
	//if( params.size() == 0 )
	//{
	//	assert( false && "no params" );
	//}
	
	//dont need number of params because the actortype determines that.
	of << type->name << " ";

	if( ground != NULL )
	{
		of << "-air" << " " << ground->writeIndex << " " << edgeIndex << " " << groundQuantity << endl;
	}
	else
	{
		of << "+air" << " " << position.x << " " << position.y << endl;
	}

	/*for( list<string>::iterator it = params.begin(); it != params.end(); ++it )
	{
		of << (*it) << endl;
	}*/
	WriteParamFile( of );
}

void ActorGroup::Draw( sf::RenderTarget *target )
{
	for( list<ActorParams*>::iterator it = actors.begin(); it != actors.end(); ++it )
	{
		(*it)->Draw( target );
	}
}

ActorGroup::ActorGroup( const std::string &n )
	:name( n )
{

}

void ActorGroup::WriteFile( std::ofstream &of )
{
	//group name and number of actors in the group
	of << name << " " << actors.size() << endl;
	for( list<ActorParams*>::iterator it = actors.begin(); it != actors.end(); ++it )
	{
		(*it)->WriteFile( of );
	}
}

TerrainPoint::TerrainPoint( sf::Vector2i &p, bool s )
	:pos( p ), selected( s )
{
}

void ActorParams::AnchorToGround( TerrainPolygon *poly, int eIndex, double quantity )
{
	ground = poly;
	poly->enemies.push_back( this );
	edgeIndex = eIndex;
	groundQuantity = quantity;

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height );
	
	int testIndex = 0;

	Vector2i point;

	PointList::iterator prev = ground->points.end();
	prev--;
	PointList::iterator curr = ground->points.begin();

	for( ; curr != ground->points.end(); ++curr )
	{
		if( edgeIndex == testIndex )
		{
			V2d pr( (*prev).pos.x, (*prev).pos.y );
			V2d cu( (*curr).pos.x, (*curr).pos.y );

			V2d newPoint( pr.x + (cu.x - pr.x) * (groundQuantity / length( cu - pr ) ), pr.y + (cu.y - pr.y ) *
											(groundQuantity / length( cu - pr ) ) );

			double angle = atan2( (cu - pr).y, (cu - pr).x ) / PI * 180;

			image.setPosition( newPoint.x, newPoint.y );
			image.setRotation( angle );

			break;
		}
		prev = curr;
		++testIndex;
	}
	//adjust for ordery
	if( edgeIndex == 0 )
		edgeIndex = ground->points.size() - 1;
	else
		edgeIndex--;
}

PatrollerParams::PatrollerParams( EditSession *edit, sf::Vector2i pos, list<Vector2i> &globalPath, float p_speed, bool p_loop )
{	
	position = pos;	
	type = edit->types["patroller"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	//list<Vector2i> localPath;
	SetPath( globalPath );

	loop = p_loop;
	speed = p_speed;
	//ss << localPath.size();
	//params.push_back( ss.str() );
	//ss.str( "" );

	/*for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		ss << (*it).x  << " " << (*it).y;
		params.push_back( ss.str() );
		ss.str( "" );
	}

	if( loop )
		params.push_back( "+loop" );
	else
		params.push_back( "-loop" );
	
	ss.precision( 5 );
	ss << fixed << speed;
	params.push_back( ss.str() );*/
}

void PatrollerParams::SetPath(std::list<sf::Vector2i> &globalPath)
{
	localPath.clear();
	if( globalPath.size() > 1 )
	{
		list<Vector2i>::iterator it = globalPath.begin();
		++it;
		for( ; it != globalPath.end(); ++it )
		{
			Vector2i temp( (*it).x - position.x, (*it).y - position.y );
			localPath.push_back( temp );
		}
	}
}

void PatrollerParams::Draw( sf::RenderTarget *target )
{
	target->draw( image );
}

std::list<sf::Vector2i> PatrollerParams::GetGlobalPath()
{
	list<Vector2i> globalPath;
	globalPath.push_back( position );
	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		globalPath.push_back( position + (*it) );
	}
	return globalPath;
}

void PatrollerParams::WriteParamFile( ofstream &of )
{
	of << localPath.size() << endl;

	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		of << (*it).x  << " " << (*it).y << endl;
	}

	if( loop )
	{
		of << "+loop" << endl;
	}
	else
	{
		of << "-loop" << endl;
	}

	of.precision( 5 );
	of << fixed << speed << endl;
}

CrawlerParams::CrawlerParams( EditSession *edit, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity, bool p_clockwise, float p_speed )
{
	clockwise = p_clockwise;
	speed = p_speed;

	type = edit->types["crawler"];
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

void CrawlerParams::WriteParamFile( ofstream &of )
{
	if( clockwise )
		of << "+clockwise" << endl;
	else
		of << "-clockwise" << endl;
	
	of.precision( 5 );
	of << fixed << speed << endl;
}

BasicTurretParams::BasicTurretParams( EditSession *edit, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity, double p_bulletSpeed, int p_framesWait )
{
	bulletSpeed = p_bulletSpeed;
	framesWait = p_framesWait;

	type = edit->types["basicturret"];
	
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

void BasicTurretParams::WriteParamFile( ofstream &of )
{
	of << bulletSpeed << endl;
	of << framesWait << endl;
}

FootTrapParams::FootTrapParams( EditSession *edit, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity )
{
	type = edit->types["foottrap"];
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

void FootTrapParams::WriteParamFile( ofstream &of )
{
}

GoalParams::GoalParams( EditSession *edit, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity )
{
	type = edit->types["goal"];
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );
}

void GoalParams::WriteParamFile( ofstream &of )
{
}