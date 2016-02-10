#include "Zone.h"
#include "poly2tri/poly2tri.h"

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

using namespace sf;
using namespace std;

Zone::Zone( TerrainPolygon &tp )
{
	vector<p2t::Point*> polyline;

	tp.FixWinding();

	int numPoints = 0;
	TerrainPoint * curr = tp.pointStart;
	while( curr != NULL )
	{
		points.push_back( curr->pos );
		polyline.push_back( new p2t::Point(curr->pos.x, curr->pos.y ) );
		++numPoints;
		curr = curr->next;
	}

	p2t::CDT * cdt = new p2t::CDT( polyline );
	cdt->Triangulate();
	vector<p2t::Triangle*> tris;
	tris = cdt->GetTriangles();
	
	int vaSize = tris.size() * 3;
	definedArea = new VertexArray( sf::Triangles , vaSize );
	
	VertexArray & v = *definedArea;
	Color shadowColor( 50, 50, 50, 200 );
	//Color testColor( 0x75, 0x70, 0x90 );
	//Color selectCol( 0x77, 0xBB, 0xDD );

	for( int i = 0; i < tris.size(); ++i )
	{	
		p2t::Point *p = tris[i]->GetPoint( 0 );	
		p2t::Point *p1 = tris[i]->GetPoint( 1 );	
		p2t::Point *p2 = tris[i]->GetPoint( 2 );	
		v[i*3] = Vertex( Vector2f( p->x, p->y ), shadowColor );
		v[i*3 + 1] = Vertex( Vector2f( p1->x, p1->y ), shadowColor );
		v[i*3 + 2] = Vertex( Vector2f( p2->x, p2->y ), shadowColor );
	}

	//assert( tris.size() * 3 == points.size() );
	delete cdt;
	for( int i = 0; i < numPoints; ++i )
	{
		delete polyline[i];
	//	delete tris[i];
	}
}

Zone::~Zone()
{
	delete definedArea;
}

void Zone::Draw( RenderTarget *target )
{
	target->draw( *definedArea );
}

bool Zone::ContainsPoint( sf::Vector2i test )
{
	int pointCount = points.size();

	bool c = false;

	Vector2i prev = points.back();
	for( list<Vector2i>::iterator it = points.begin(); it != points.end(); ++it )
	{
		if ( ( ( (*it).y > test.y ) != ( prev.y > test.y ) ) &&
			(test.x < (prev.x-(*it).x) * (test.y-(*it).y) / (prev.y-(*it).y) + (*it).x) )
				c = !c;

		prev = (*it);
	}

	return c;
}

bool Zone::ContainsZone( Zone *z )
{
	Vector2i p( z->gates.front()->edge0->v0.x, z->gates.front()->edge0->v0.y );
	return ContainsPoint( p );
}

bool Zone::ContainsPlayer()
{

}

Zone* Zone::ContainsPointMostSpecific( sf::Vector2i test )
{
	bool contains = ContainsPoint( test );

	if( contains )
	{
		if( subZones.empty() )
		{
			return this;
		}
		else
		{
			for( list<Zone*>::iterator it = subZones.begin(); it != subZones.end(); ++it )
			{
				Zone * z = ContainsPointMostSpecific( test );
				if( z != NULL )
				{
					return z;
				}
			}

			return this;
		}
	}
	else
	{
		return NULL;
	}
}