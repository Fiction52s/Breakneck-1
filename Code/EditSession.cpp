//edit mode

#include "GUI.h"
#include "EditSession.h"
#include <fstream>
#include <assert.h>
#include <iostream>
#include "poly2tri/poly2tri.h"
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "Physics.h"

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

TerrainBrush::TerrainBrush( TerrainPolygon *poly )
	:pointStart(NULL),pointEnd(NULL),lines( sf::Lines, poly->numPoints * 2 ), numPoints( 0 )
{
	//assert( poly->finalized );

	TerrainPoint *curr = poly->pointStart;
	left = curr->pos.x;
	right = curr->pos.x; 
	top = curr->pos.y; 
	bot = curr->pos.y;

	TerrainPoint *p = new TerrainPoint( *curr );
	p->gate = NULL;
	AddPoint( p );

	curr = curr->next;
	for( ; curr != NULL; curr = curr->next )
	{
		if( curr->pos.x < left )
			left = curr->pos.x;
		else if( curr->pos.x > right )
			right = curr->pos.x;

		if( curr->pos.y < top )
			top = curr->pos.y;
		else if( curr->pos.y > bot )
			bot = curr->pos.y;

		TerrainPoint *tp = new TerrainPoint( *curr );
		tp->gate = NULL;
		AddPoint( tp );
	}
	UpdateLines();
	//centerPos = Vector2f( left + width / 2.f, top + height / 2.f );
}

TerrainBrush::TerrainBrush( TerrainBrush &brush )
	:pointStart( NULL ), pointEnd( NULL ), numPoints( 0 ),
		lines( sf::Lines, brush.numPoints * 2 )
{
	left = brush.left;
	right = brush.right;
	top = brush.top;
	bot = brush.bot;
	pointStart = NULL;
	pointEnd = NULL;



	for( TerrainPoint *tp = brush.pointStart; tp != NULL; tp = tp->next )
	{
		AddPoint( new TerrainPoint( *tp ) );
	}

	UpdateLines();
}

TerrainBrush::~TerrainBrush()
{
	TerrainPoint *curr = pointStart; 
	while( curr != NULL )
	{
		TerrainPoint *temp = curr->next;
		delete curr;
		curr = temp;
	}
}

void TerrainBrush::UpdateLines()
{
	int index = 0;
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		TerrainPoint *prev;
		if( curr == pointStart )
			prev = pointEnd;
		else
			prev = curr->prev;

		lines[index*2].position = Vector2f( prev->pos.x, prev->pos.y );
		lines[index*2+1].position = Vector2f( curr->pos.x, curr->pos.y );

		++index;
	}
}

void TerrainBrush::Draw( sf::RenderTarget *target )
{
	target->draw( lines );

	CircleShape cs;
	cs.setRadius( 5 );
	cs.setFillColor( Color::Red );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		cs.setPosition( curr->pos.x, curr->pos.y );
		target->draw( cs );
	}
}

void TerrainBrush::Move( Vector2i delta )
{
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		curr->pos.x += delta.x;
		curr->pos.y += delta.y;
	}
	UpdateLines();
	//centerPos.x += delta.x;
	//centerPos.y += delta.y;
}

void TerrainBrush::AddPoint( TerrainPoint *tp )
{
	if( pointStart == NULL )
	{
		pointStart = tp;
		pointEnd = tp;
		tp->prev = NULL;
		tp->next = NULL;
	}
	else
	{
		pointEnd->next = tp;
		tp->prev = pointEnd;
		pointEnd = tp;
		pointEnd->next = NULL;
	}

	++numPoints;
}

TerrainPolygon::TerrainPolygon( sf::Texture *gt)
	:grassTex( gt )
{
	va = NULL;
	lines = NULL;
	selected = false;
	grassVA = NULL;
	isGrassShowing = false;
	finalized = false;
	numPoints = 0;
	pointStart = NULL;
	pointEnd = NULL;
	movingPointMode = false;
}

void TerrainPolygon::AlignExtremes( double primLimit )
{
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		TerrainPoint *prev;
		if( curr == pointStart )
		{
			prev = pointEnd;
		}
		else
		{
			prev = curr->prev;
		}

		TerrainPoint *next;
		if( curr == pointEnd )
		{
			next = pointStart;
		}
		else
		{
			next = curr->next;
		}

		V2d prevExtreme( 0, 0 );
		V2d nextExtreme( 0, 0 );
		Vector2i prevVec = curr->pos - prev->pos;
		Vector2i nextVec = curr->pos - next->pos;
		V2d prevNormVec = normalize( V2d( prevVec.x, prevVec.y ) );
		V2d nextNormVec = normalize( V2d( nextVec.x, nextVec.y ) );

		if( prevNormVec.x > primLimit )
			prevExtreme.x = 1;
		else if( prevNormVec.x < -primLimit )
			prevExtreme.x = -1;
		if( prevNormVec.y > primLimit )
			prevExtreme.y = 1;
		else if( prevNormVec.y < -primLimit )
			prevExtreme.y = -1;

		if( nextNormVec.x > primLimit )
			nextExtreme.x = 1;
		else if( nextNormVec.x < -primLimit )
			nextExtreme.x = -1;
		if( nextNormVec.y > primLimit )
			nextExtreme.y = 1;
		else if( nextNormVec.y < -primLimit )
			nextExtreme.y = -1;


		if( finalized )
		{
			if( !curr->selected )
			{
				continue;
			}

			bool prevValid = true, nextValid = true;
			if( nextNormVec.x == 0 || nextNormVec.y == 0 )
			{
				nextValid = false;
			} 

			if( prevNormVec.x == 0 || prevNormVec.y == 0 )
			{
				prevValid = false;
			} 

			if( prevValid && nextValid )
			{
				if( prevExtreme.x != 0 )
				{
					if( nextExtreme.x != 0 )
					{
						cout << "a" << endl;
						prev->pos.y = curr->pos.y;
						next->pos.y = curr->pos.y;
					}
					else if( nextExtreme.y != 0 )
					{
						cout << "b" << endl;
						curr->pos.y = prev->pos.y;
						curr->pos.x = next->pos.x;
					}
					else
					{
						cout << "c" << endl;
						curr->pos.y = prev->pos.y;
					}
				}
				else if( prevExtreme.y != 0 )
				{
					if( nextExtreme.y != 0 )
					{
						cout << "d" << endl;
						prev->pos.x = curr->pos.x;
						next->pos.x = curr->pos.x;
					}
					else if( nextExtreme.x != 0 )
					{
						cout << "e" << endl;
						curr->pos.x = prev->pos.x;
						curr->pos.y = next->pos.y;
					}
					else
					{
						cout << "f" << endl;
						curr->pos.x = prev->pos.x;
					}
				}
			}
			else if( prevValid )
			{
				if( prevExtreme.y != 0 )
				{
					curr->pos.x = prev->pos.x;
				}
				else if( prevExtreme.x != 0 )
				{
					curr->pos.y = prev->pos.y;
				}
			}
			else if( nextValid )
			{
				if( nextExtreme.y != 0 )
				{
					curr->pos.x = next->pos.x;
				}
				else if( nextExtreme.x != 0 )
				{
					curr->pos.y = next->pos.y;
				}
			}
			else
			{
				continue;
			}
		}
		else
		{
			if( nextNormVec.x == 0 || nextNormVec.y == 0 )
			{
				continue;
			}


			if( nextExtreme.x != 0 )
			{
				curr->pos.y = next->pos.y;
				//cout << "lining up x" << endl;
			}

			if( nextExtreme.y != 0 )
			{
				curr->pos.x = next->pos.x;
				//cout << "lining up y" << endl;
			}
		}

		


	}
}

TerrainPolygon::~TerrainPolygon()
{
	if( lines != NULL )
		delete [] lines;
	if( va != NULL )
		delete va;

	if( grassVA != NULL )
		delete grassVA;

	//DestroyEnemies();

	ClearPoints();
}

void TerrainPolygon::DestroyEnemies()
{
	for( EnemyMap::iterator mapIt = enemies.begin(); mapIt != enemies.end(); ++mapIt )
	{
		for( list<ActorParams*>::iterator it = (*mapIt).second.begin(); it != (*mapIt).second.end(); ++it )
		{
			(*it)->group->actors.remove( (*it ) );
			delete (*it);
		}
	}
	enemies.clear();
}

void TerrainPolygon::Move( Vector2i move )
{
	assert( finalized );
	
	TerrainPoint *curr = pointStart;
	while( curr != NULL )
	{
		TerrainPoint *temp = curr->next;
		curr->pos += move;
		if( curr->gate != NULL )
		{
			curr->gate->UpdateLine();
			//cout << "updating line" << endl;
		}
		curr = temp;
	}

	for( int i = 0; i < numPoints; ++i )
	{
		//lines
		lines[i*2].position += Vector2f( move.x, move.y );
		lines[i*2+1].position += Vector2f( move.x, move.y );
	}

	for( int i = 0; i < vaSize; ++i )
	{
		VertexArray &vaa = *va;

		//triangles
		vaa[i].position += Vector2f( move.x, move.y );
		//vaa[i*3+1].position += Vector2f( move.x, move.y );
		//vaa[i*3+2].position += Vector2f( move.x, move.y );
	}

	for( int i = 0; i < numGrassTotal; ++i )
	{
		//quads
		VertexArray &gva = *grassVA;
		gva[i*4].position += Vector2f( move.x, move.y );
		gva[i*4+1].position += Vector2f( move.x, move.y );
		gva[i*4+2].position += Vector2f( move.x, move.y );
		gva[i*4+3].position += Vector2f( move.x, move.y );
	}

	for( EnemyMap::iterator it = enemies.begin(); it != enemies.end(); ++it )
	{
		list<ActorParams*> &actorList = (*it).second;
		for( list<ActorParams*>::iterator ait = actorList.begin(); ait != actorList.end(); ++ait )
		{
			(*ait)->UpdateGroundedSprite();
			(*ait)->SetBoundingQuad();
		}
	}

	UpdateBounds();
	return;
}

void TerrainPolygon::UpdateBounds()
{
	TerrainPoint *curr = pointStart;
	//PointList::iterator it = points.begin();
	left = curr->pos.x;
	right = curr->pos.x;
	top = curr->pos.y;
	bottom = curr->pos.y;
	curr = curr->next;
	while( curr != NULL )
	{
		left = min( curr->pos.x, left );
		right = max( curr->pos.x, right );
		top = min( curr->pos.y, top );
		bottom = max( curr->pos.y, bottom );
		curr = curr->next;
	}

	for( EnemyMap::iterator it = enemies.begin(); it != enemies.end(); ++it )
	{
		list<ActorParams*> &en = (*it).second;
		for( list<ActorParams*>::iterator ait = en.begin(); ait != en.end(); ++ait )
		{
			sf::VertexArray & bq = (*ait)->boundingQuad;
			for( int i = 0; i < 4; ++i )
			{
				int x = bq[i].position.x;
				int y = bq[i].position.y;
				if( x < left )
				{
					left = x;
				}
				if( x > right )
				{
					right = x;
				}
				if( y < top )
				{
					top = y;
				}
				if( y > bottom )
				{
					//cout << "adjusting botton from: " << bottom << " to " << y << endl;
					bottom = y;
					
				}
					
			}

		}
	}
}

void TerrainPolygon::Finalize()
{
	finalized = true;
	isGrassShowing = false;
	material = "mat";
	lines = new sf::Vertex[numPoints*2+1];
	
	FixWinding();
	//cout << "points size: " << points.size() << endl;

	vector<p2t::Point*> polyline;
	TerrainPoint *curr = pointStart;
	while( curr != NULL )
	{
		TerrainPoint *temp = curr->next;
		polyline.push_back( new p2t::Point(curr->pos.x, curr->pos.y ) );
		curr = temp;
	}

	p2t::CDT * cdt = new p2t::CDT( polyline );
	
	cdt->Triangulate();
	vector<p2t::Triangle*> tris;
	tris = cdt->GetTriangles();
	
	vaSize = tris.size() * 3;
	va = new VertexArray( sf::Triangles , vaSize );
	
	VertexArray & v = *va;
	Color testColor( 0x75, 0x70, 0x90 );
	Color selectCol( 0x77, 0xBB, 0xDD );

	if( selected )
	{
		testColor = selectCol;
	}
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
	for( int i = 0; i < numPoints; ++i )
	{
		delete polyline[i];
	//	delete tris[i];
	}

	if( numPoints > 0 )
	{
		int i = 0;
		curr = pointStart;
		lines[0] = sf::Vector2f( curr->pos.x, curr->pos.y );
		lines[2 * numPoints - 1 ] = sf::Vector2f( curr->pos.x, curr->pos.y );
		curr = curr->next;
		++i;
		while( curr != NULL )
		{
			lines[i] = sf::Vector2f( curr->pos.x, curr->pos.y );
			lines[++i] = sf::Vector2f( curr->pos.x, curr->pos.y ); 
			++i;
			curr = curr->next;
		}
	}

	UpdateBounds();
	

	double grassSize = 22;
	double grassSpacing = -5;

	numGrassTotal = 0;
	int inds = 0;
	for( curr = pointStart; curr != NULL; curr = curr->next )
	{
		Vector2i next;

		TerrainPoint *temp = curr->next;
		if( temp == NULL )
		{
			next = pointStart->pos;
		}
		else
		{
			//++temp;
			next = temp->pos;
			//--temp;
		}

		V2d v0( curr->pos.x, curr->pos.y );
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
	for( curr = pointStart; curr != NULL; curr = curr->next )
	{
		Vector2i next;

		TerrainPoint *temp = curr->next;
		if( temp == NULL )
		{
			next = pointStart->pos;
		}
		else
		{
			next = temp->pos;
		}

		V2d v0( curr->pos.x, curr->pos.y );
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

	/*if( grassVA != NULL )
	{
		delete grassVA;
	}*/
	grassVA = gva;	
}

void TerrainPolygon::RemoveSelectedPoints()
{	
	SoftReset();
	for( TerrainPoint *curr = pointStart; curr != NULL; )
	{
		if( curr->selected )
		{
			TerrainPoint *temp = curr->next;
			RemovePoint( curr );
			curr = temp;
		}
		else
		{
			curr = curr->next;
		}
	}
	//Reset();


	Finalize();
	SetSelected( true );
}

void TerrainPolygon::Extend( TerrainPoint* startPoint, TerrainPoint*endPoint, TerrainPolygon *inProgress )
{
	if( inProgress->numPoints < 2 )
	{
		return;
	}

	bool startFirst = true;
	TerrainPoint *start;
	TerrainPoint *end;

	bool startFound = false;
	bool endFound = false;

	//inProgress->FixWinding();
	
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( curr == startPoint )
		{
			start = curr;

			if( endFound )
				break;

			startFound = true;
		}
		else if( curr == endPoint )
		{
			end = curr;

			if( startFound )
				break;

			endFound = true;
			startFirst = false;
		}
	}

	TerrainPolygon newList( grassTex );
	inProgress->AddPoint( new TerrainPoint( *endPoint ) );
	bool inProgresscw = inProgress->IsClockwise();
	if( !inProgresscw )
	{
		inProgress->FixWinding();
		if( startFirst )
		{
			TerrainPoint *temp = end;
			end = start;
			start = temp;
			startFirst = false;
			inProgresscw = true;
		}
		else
		{
			TerrainPoint *temp = end;
			end = start;
			start = temp;
			startFirst = true;
			inProgresscw = true;
			//cout << "changing" << endl;
		}
	}

	inProgress->RemovePoint( inProgress->pointEnd );

	if( startFirst )
	{	
		for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
		{
			//cout << "normal type: " << inProgresscw << endl;
			if( curr == start )
			{
				for( TerrainPoint *progressCurr = inProgress->pointStart; progressCurr != NULL; progressCurr = progressCurr->next )
				{
					TerrainPoint *tp = new TerrainPoint( *progressCurr );
					tp->gate = progressCurr->gate;
					if( tp->gate != NULL )
					{
						if( progressCurr == tp->gate->point0 )
						{
							tp->gate->point0 = tp;
						}
						else if( progressCurr == tp->gate->point1 )
						{
							tp->gate->point1 = tp;	
						}
						else
						{
							assert( false );
						}
					}

					//cant be enemies here because its the in progress polygon
		
					newList.AddPoint( tp );
				}

				curr = end;
				TerrainPoint *tp = new TerrainPoint( *end );
				tp->gate = end->gate;
				if( tp->gate != NULL )
				{
					if( end == tp->gate->point0 )
					{
						tp->gate->point0 = tp;
					}
					else if( end == tp->gate->point1 )
					{
						tp->gate->point1 = tp;	
					}
					else
					{
						assert( false );
					}
				}

				if( enemies.count( curr ) > 0 )
				{
					list<ActorParams*> &en = newList.enemies[tp];
					en = enemies[curr];
				}

				newList.AddPoint( tp );	
			}
			else
			{
				TerrainPoint *tp = new TerrainPoint( *curr );
				tp->gate = curr->gate;
				if( tp->gate != NULL )
				{
					if( curr == tp->gate->point0 )
					{
						tp->gate->point0 = tp;
					}
					else if( curr == tp->gate->point1 )
					{
						tp->gate->point1 = tp;	
					}
					else
					{
						assert( false );
					}
				}

				if( enemies.count( curr ) > 0 )
				{
					list<ActorParams*> &en = newList.enemies[tp];
					en = enemies[curr];
				}

				newList.AddPoint( tp );
			}
		}
	}	
	else
	{
		//cout << "other type: " << inProgresscw << endl;
		for( TerrainPoint *curr = end; curr != NULL; curr = curr->next )
		{
			if( curr == start )
			{	
				for( TerrainPoint *progressCurr = inProgress->pointStart; progressCurr != NULL; progressCurr = progressCurr->next )
				{
					TerrainPoint *tp = new TerrainPoint( *progressCurr ); 
					tp->gate = progressCurr->gate;
					if( tp->gate != NULL )
					{
						if( progressCurr == tp->gate->point0 )
						{
							tp->gate->point0 = tp;
						}
						else if( progressCurr == tp->gate->point1 )
						{
							tp->gate->point1 = tp;	
						}
						else
						{
							assert( false );
						}
					}

					//in progress so no enemies

					newList.AddPoint( tp );
				}	
				break;
			}
			else
			{
				TerrainPoint *tp = new TerrainPoint( *curr );
				tp->gate = curr->gate;
				if( tp->gate != NULL )
				{
					if( curr == tp->gate->point0 )
					{
						tp->gate->point0 = tp;
					}
					else if( curr == tp->gate->point1 )
					{
						tp->gate->point1 = tp;	
					}
					else
					{
						assert( false );
					}
				}

				if( enemies.count( curr ) > 0 )
				{
					list<ActorParams*> &en = newList.enemies[tp];
					en = enemies[curr];
				}

				newList.AddPoint( tp );
			}
		}
	}

	Reset();


	for( TerrainPoint *curr = newList.pointStart; curr != NULL; curr = curr->next )
	{
		TerrainPoint *tp = new TerrainPoint( *curr );
		tp->gate = curr->gate;
		if( tp->gate != NULL )
		{
			if( curr == tp->gate->point0 )
			{
				tp->gate->point0 = tp;
			}
			else if( curr == tp->gate->point1 )
			{
				tp->gate->point1 = tp;	
			}
			else
			{
				assert( false );
			}
		}

		if( newList.enemies.count( curr ) > 0 )
		{
			list<ActorParams*> &en = enemies[tp];
			en = newList.enemies[curr];
			//cout << "zsize: " << en.size() << endl;
			for( list<ActorParams*>::iterator it = en.begin(); it != en.end(); ++it )
			{
				//cout << "setting new ground on actor params" << endl;
				(*it)->groundInfo->ground = this;
				(*it)->groundInfo->edgeStart = tp;
			}
		}

		AddPoint( tp );
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


	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		Vector2i next;

		TerrainPoint *temp = curr->next;
		if( temp == NULL )
		{
			next = pointStart->pos;
		}
		else
		{
			next = temp->pos;
		}

		V2d v0( curr->pos.x, curr->pos.y );
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
	if( movingPointMode )
	{

			int i = 0;
			TerrainPoint *curr = pointStart;
			while( curr != NULL )
			{
				lines[i*2].position = Vector2f( curr->pos.x, curr->pos.y );

				TerrainPoint *temp = curr->next;
				if( temp == NULL )
				{
					lines[i*2+1].position = Vector2f( pointStart->pos.x, pointStart->pos.y );
				}
				else
				{
					lines[i*2+1].position = Vector2f( temp->pos.x, temp->pos.y );
				}
				
				curr = temp;
				++i;
			}

			rt->draw( lines, numPoints * 2, sf::Lines );


			//lines
			
		
		return;
	}


	if( grassVA != NULL )
		rt->draw( *grassVA, grassTex );

	if( va != NULL )
		rt->draw( *va );

	if( selected )
	{
		if( !isGrassShowing )
		{
			for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
			{
				CircleShape cs;
				cs.setRadius( 8 * zoomMultiple );
				cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );

				if( curr->selected )
					cs.setFillColor( Color::Red );
				else
					cs.setFillColor( Color::Green );

				cs.setPosition( curr->pos.x, curr->pos.y );
				rt->draw( cs );
			}
		}
		rt->draw( lines, numPoints * 2, sf::Lines );
	}

	if( showPoints )
	{
		for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
		{
			if( curr == dontShow )
			{
				continue;
			}
			CircleShape cs;
			cs.setRadius( 8 * zoomMultiple );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );

			cs.setFillColor( Color::Magenta );

			cs.setPosition( curr->pos.x, curr->pos.y );
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

		for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
		{
			curr->selected = false;
		}
	}
}

bool TerrainPolygon::ContainsPoint( Vector2f test )
{
	int pointCount = numPoints;

	int i, j, c = 0;

	TerrainPoint *it = pointStart;
	TerrainPoint *jt = pointEnd;

	for( ; it != NULL; )
	{
		Vector2f point(it->pos.x, it->pos.y );
		Vector2f pointJ(jt->pos.x, jt->pos.y );
		if ( ((point.y > test.y ) != ( pointJ.y > test.y ) ) &&
			(test.x < (pointJ.x-point.x) * (test.y-point.y) / (pointJ.y-point.y) + point.x) )
				c = !c;
		jt = it;
		it = it->next;
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
		for( TerrainPoint *curr = pointStart; curr != NULL; )
		{
			TerrainPoint *tt = curr->next;
			TerrainPoint *oldPrev = curr->prev;
			curr->prev = curr->next;
			curr->next = oldPrev;
			curr = tt;
		}
		TerrainPoint *tt = pointStart;
		pointStart = pointEnd;
		pointEnd = tt;

    }
}

void TerrainPolygon::AddPoint( TerrainPoint* tp)
{
	if( pointStart == NULL )
	{
		pointStart = tp;
		pointEnd = tp;
		tp->prev = NULL;
		tp->next = NULL;
	}
	else
	{
		pointEnd->next = tp;
		tp->prev = pointEnd;
		pointEnd = tp;
		pointEnd->next = NULL;
	}
	++numPoints;
}

void TerrainPolygon::RemovePoint( TerrainPoint *tp )
{
	assert( pointStart != NULL );

	if( tp->prev != NULL )
	{
		tp->prev->next = tp->next;
	}
	if( tp->next != NULL )
	{
		tp->next->prev = tp->prev;
	}

	if( tp == pointStart )
	{
		pointStart = tp->next;
	}
	if( tp == pointEnd )
	{
		pointEnd = tp->prev;
	}

	GateInfo *gi = tp->gate;
	if( gi != NULL )
	{
		gi->point0->gate = NULL;
		gi->point1->gate = NULL;
		gi->edit->gates.remove( gi );
		delete gi;
	}
	//delete tp;


	--numPoints;
}

void TerrainPolygon::Reset()
{
	ClearPoints();
	if( lines != NULL )
		delete [] lines;
	if( va != NULL )
		delete va;
	if( grassVA != NULL )
		delete grassVA;

	lines = NULL;
	va = NULL;
	grassVA = NULL;
	finalized = false;
}

void TerrainPolygon::SoftReset()
{
	if( lines != NULL )
		delete [] lines;
	if( va != NULL )
		delete va;
	if( grassVA != NULL )
		delete grassVA;

	lines = NULL;
	va = NULL;
	grassVA = NULL;
	finalized = false;
}

void TerrainPolygon::ClearPoints()
{
	TerrainPoint *curr = pointStart;
	while( curr != NULL )
	{
		TerrainPoint *temp = curr->next;
		delete curr;
		curr = temp;
	}

	pointStart = NULL;
	pointEnd = NULL;
	numPoints = 0;
}

bool TerrainPolygon::IsRemovePointsOkayTerrain( EditSession *edit )
{
	TerrainPolygon tempPoly( grassTex );

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( !curr->selected )
		{
			tempPoly.AddPoint( new TerrainPoint(*curr) );
		}
	}

	bool isPolyValid = edit->IsPolygonValid( tempPoly, this );

	
	return isPolyValid;
}

//0 means a window came up and they canceled. -1 means no enemies were in danger on that polygon, 1 means that you confirmed to delete the enemies
int TerrainPolygon::IsRemovePointsOkayEnemies( EditSession *edit )
{
	for( EnemyMap::iterator mapIt = enemies.begin(); mapIt != enemies.end(); ++mapIt )
	{
		for( list<ActorParams*>::iterator it = (*mapIt).second.begin(); it != (*mapIt).second.end(); ++it )
		{
			TerrainPoint *edgeEnd = (*it)->groundInfo->edgeStart->next;
			if( edgeEnd == NULL )
				edgeEnd = (*it)->groundInfo->ground->pointStart;

			if( (*it)->type->canBeGrounded && 
				( (*(*it)->groundInfo->edgeStart).selected || edgeEnd->selected ) )
			{
				bool removeSelectedActors = edit->ConfirmationPop("1 or more enemies will be removed by deleting these points.");

				if( removeSelectedActors )
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
		}
	}

	return -1;	
}

bool TerrainPolygon::IsMovePointsOkay( EditSession *edit, Vector2i delta )
{
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( curr->selected )
		{
			curr->pos += delta;
		}
	}

	bool result = edit->IsPolygonValid( *this, this );

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( curr->selected )
		{
			curr->pos -= delta;
		}
	}

	return result;
}

bool TerrainPolygon::IsMovePointsOkay( EditSession *edit, Vector2i pointGrabDelta, Vector2i *deltas )
{
	//TerrainPolygon tempPoly( grassTex );

	int arraySize = numPoints;

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		TerrainPoint *next;
		if( curr == pointEnd )
			next = pointStart;
		else
			next = curr->next;
		//eventually this will let you move the points and keep the actors in the right spots. for now just give a popup
		//more of the code is below
		
		if( ( curr->selected || next->selected ) && enemies.count( curr ) > 0 )
		{
			//cout << "move not okay" << endl;
			edit->pointGrab = false;
			edit->MessagePop( "sorry, in this build you can't yet move points\n that have enemies attached to their edges" );
			return false;
		}
	}

	int i = 0;
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{


		if( curr->selected )
		{
			curr->pos += pointGrabDelta - deltas[i];
		//	tempPoly.AddPoint( new TerrainPoint( *curr ) );
		}
		/*else
		{
			TerrainPoint *tp = new TerrainPoint( *curr );
			
			tp->pos += pointGrabDelta - deltas[i];

			tempPoly.AddPoint( tp );
		}*/

		++i;
	}

	//eventually going to need this back again!

	/*for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		TerrainPoint *next;
		if( curr == pointEnd )
			next = pointStart;
		else
			next = curr->next;


		//eventually this will let you move the points and keep the actors in the right spots. for now just give a popup
		
		if( curr->selected || next->selected )
		{
			//edit->MessagePop( "sorry, in this build you can't yet move points\n that have enemies attached to their edges" );
			//return false;
			if( enemies.count( curr ) > 0 )
			{
				list<ActorParams*> &en = enemies[curr];
				for( list<ActorParams*>::iterator it = en.begin(); it != en.end(); ++it )
				{
					(*it)->SetBoundingQuad();
				}
			}
		}
	}*/

	UpdateBounds();

	bool res = edit->IsPolygonValid( *this, this );


	
	bool result = true;
	if( !res )
	{
		result = false;
	}
	/*else
	{
		bool res2 = true;
		for( std::map<std::string, ActorGroup*>::iterator it = edit->groups.begin(); it != edit->groups.end() && res2; ++it )
		{
			for( list<ActorParams*>::iterator ait = (*it).second->actors.begin(); ait != (*it).second->actors.end(); ++ait )
			{
				//need to round these floats probably

				sf::VertexArray &bva = (*ait)->boundingQuad;
				if( edit->QuadPolygonIntersect( this, Vector2i( bva[0].position.x, bva[0].position.y ), 
					Vector2i( bva[1].position.x, bva[1].position.y ), Vector2i( bva[2].position.x, bva[2].position.y ),
					 Vector2i( bva[3].position.x, bva[3].position.y ) ) )
				{
					cout << "polygon collide with quad" << endl;
					res2 = false;
					break;
				}
				else
				{
					cout << "no collision with quad" << endl;
				}
			}
		}
		result = res2;
	}*/
	

	i = 0;
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( curr->selected )
		{
			curr->pos -= pointGrabDelta - deltas[i];
		}
		++i;
	}

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( curr->selected )
		{
			if( enemies.count( curr ) > 0 )
			{
				list<ActorParams*> &en = enemies[curr];
				for( list<ActorParams*>::iterator it = en.begin(); it != en.end(); ++it )
				{
					(*it)->SetBoundingQuad();
				}
			}
		}
	}

	UpdateBounds();

	return result;
}

void TerrainPolygon::MoveSelectedPoints( Vector2i move )
{
	movingPointMode = true;

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( curr->selected )
		{
			curr->pos += move;
			if( curr->gate != NULL )
			{
				curr->gate->UpdateLine();
			}
		}
	}
}

bool TerrainPolygon::IsMovePolygonOkay( EditSession *edit, sf::Vector2i delta )
{
	TerrainPolygon tempPoly( grassTex );

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		//TerrainPoint  *tp =  new TerrainPoint( *curr );
		//tp->pos += delta;
		//tempPoly.AddPoint( tp );

		curr->pos += delta;
	}

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( enemies.count( curr ) > 0 )
		{
			list<ActorParams*> &actors = enemies[curr];
			for( list<ActorParams*>::iterator it = actors.begin(); it != actors.end(); ++it )
			{
				(*it)->UpdateGroundedSprite();
				(*it)->SetBoundingQuad();
			}
		}
	}


	UpdateBounds();
	//tempPoly.UpdateBounds();

	bool f = edit->IsPolygonExternallyValid( *this, this );
	if( !f )
	{

		cout << "failed delta: " << delta.x << ", " << delta.y << endl;
	}

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		curr->pos -= delta;

		
	}

	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		if( enemies.count( curr ) > 0 )
		{
			list<ActorParams*> &actors = enemies[curr];
			for( list<ActorParams*>::iterator it = actors.begin(); it != actors.end(); ++it )
			{
				(*it)->UpdateGroundedSprite();
				(*it)->SetBoundingQuad();
			}
		}
	}

	

	UpdateBounds();

	return f;
}

bool TerrainPolygon::IsClockwise()
{
	assert( numPoints > 0 );

	int pointCount = numPoints;
    Vector2i *pointArray = new Vector2i[pointCount];

	int i = 0;
	for( TerrainPoint *curr = pointStart; curr != NULL; curr = curr->next )
	{
		pointArray[i] = curr->pos;
		++i;
	}

    long long int sum = 0;
	for (int i = 0; i < pointCount; ++i)
    {
        Vector2<long long int> first, second;
		
        if (i == 0)
        {
			first.x = pointArray[pointCount - 1].x;
			first.y = pointArray[pointCount - 1].y;
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

		
		TerrainPoint *curr = pointStart;
		Vector2i currPos = curr->pos;
		curr = curr->next;
		Vector2i nextPos;
		

		

		for( ;; curr = curr->next )
		{
			if( curr == NULL )
				curr = pointStart;

			nextPos = curr->pos;


			TerrainPoint *pit = p->pointStart;
			Vector2i pcurr = pit->pos;
			pit = pit->next;
			Vector2i pnext;// = (*pit);

			for( ;; pit = pit->next )		
			{
				if( pit == NULL )
					pit = p->pointStart;

				pnext = pit->pos;
			
				LineIntersection li = EditSession::SegmentIntersect( currPos, nextPos, pcurr, pnext );	

				if( !li.parallel )
				{
					//points.pop_back();
					//p->points.pop_back();
					cout << "touching!" << endl;
					return true;
				}

				pcurr = pit->pos;

				if( pit == p->pointStart )
					break;
			}
			currPos = curr->pos;

			if( curr == pointStart )
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
	assert( numPoints > 1 );
	TerrainPoint *curr = pointStart;
	int l = curr->pos.x;
	int r = curr->pos.x;
	int t = curr->pos.y;
	int b = curr->pos.y;
	curr = curr->next;
	for( ; curr != NULL; curr = curr->next )
	{
		l = min( curr->pos.x, l);
		r = max( curr->pos.x, r);
		t = min( curr->pos.y, t);
		b = max( curr->pos.y, b );
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

GateInfo::GateInfo()
	:thickLine( sf::Quads, 4 )
{
	thickLine[0].color = Color( 255, 0, 0, 255 );
	thickLine[1].color = Color( 255, 0, 0, 255 );
	thickLine[2].color = Color( 255, 0, 0, 255 );
	thickLine[3].color = Color( 255, 0, 0, 255 );
}

void GateInfo::SetType( const std::string &gType )
{
	if( gType == "red" )
	{
		type = GateTypes::RED;
	}
	else if( gType == "green" )
	{
		type = GateTypes::GREEN;
	}
	else if( gType == "blue" )
	{
		type = GateTypes::BLUE;
	}
}

void GateInfo::WriteFile( ofstream &of )
{
	int index0 = 0, index1 = 0;
	TerrainPoint *curr = poly0->pointStart;
	while( curr != NULL )
	{
		if( curr == point0 )
		{
			break;
		}
		++index0;
		curr = curr->next;
	}

	curr = poly1->pointStart;
	while( curr != NULL )
	{
		if( curr == point1 )
		{
			break;
		}
		++index1;
		curr = curr->next;
	}

	of << (int)type << " " << poly0->writeIndex << " " << index0 << " " << poly1->writeIndex << " " << index1 << endl;
}

void GateInfo::UpdateLine()
{
	double width = 5;
	V2d dv0( point0->pos.x, point0->pos.y );
	V2d dv1( point1->pos.x, point1->pos.y );
	V2d along = normalize( dv1 - dv0 );
	V2d other( along.y, -along.x );
	
	V2d leftv0 = dv0 - other * width;
	V2d rightv0 = dv0 + other * width;

	V2d leftv1 = dv1 - other * width;
	V2d rightv1 = dv1 + other * width;

	cout << "a: " << dv0.x << ", " << dv0.y << ", b: " << dv1.x << ", " << dv1.y << endl;
	
	Color c;
	if( type == GateTypes::RED )
	{
		c = Color( 255, 0, 0 );
	}
	else if( type == GateTypes::GREEN )
	{
		c = Color( 0, 255, 0 );
	}
	else if( type == GateTypes::BLUE )
	{
		c = Color( 0, 0, 255 );
	}
	thickLine[0].color = c;
	thickLine[1].color = c;
	thickLine[2].color = c;
	thickLine[3].color = c;

	thickLine[0].position = Vector2f( leftv0.x, leftv0.y );
	thickLine[1].position = Vector2f( leftv1.x, leftv1.y );
	thickLine[2].position = Vector2f( rightv1.x, rightv1.y );
	thickLine[3].position = Vector2f( rightv0.x, rightv0.y );
}

void GateInfo::Draw( sf::RenderTarget *target )
{
	Color c;
	if( type == GateTypes::RED)
	{
		c = Color( 255, 0, 0 );
	}
	else if( type == GateTypes::GREEN )
	{
		c = Color( 0, 255, 0 );
	}
	else if( type == GateTypes::BLUE )
	{
		c = Color( 0, 0, 255 );
	}

	CircleShape cs( 5 );
	cs.setFillColor( c );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );

	cs.setPosition( point0->pos.x, point0->pos.y );
	target->draw( cs );

	cs.setPosition( point1->pos.x, point1->pos.y );
	target->draw( cs );

	target->draw( thickLine );
}

EditSession::EditSession( RenderWindow *wi, sf::RenderTexture *preTex )
	:w( wi ), zoomMultiple( 1 )
{
	//adding 5 for random distance buffer
	playerHalfWidth = 32;
	playerHalfHeight = 32;
	preScreenTex = preTex;
	showTerrainPath = false;
	minAngle = .99;
	showPoints = false;
	messagePopup = NULL;
	errorPopup = NULL;
	confirm = NULL;
	enemyQuad.setFillColor( Color( 0, 255, 0, 100 ) );
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

	int psize = polygonInProgress->numPoints;
	if( psize > 0 )
	{
		CircleShape cs;
		cs.setRadius( 5 * zoomMultiple  );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setFillColor( Color::Green );

		
		for( TerrainPoint *progressCurr = polygonInProgress->pointStart; progressCurr != NULL; progressCurr = progressCurr->next )
		{
			cs.setPosition( progressCurr->pos.x, progressCurr->pos.y );
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
				poly->AddPoint( new TerrainPoint( Vector2i(x,y), false ) );
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
			int numEdges = poly->numPoints;
			int *indexArray = new int[numEdges];
			int edgeIndex = 0;

			int iai = 0;

			for( TerrainPoint *polyCurr = poly->pointStart; polyCurr != NULL; polyCurr = polyCurr->next )
			{
				indexArray[edgeIndex] = grassIndex;

				Vector2i next;

				TerrainPoint *temp = polyCurr->next;
				if( temp == NULL )
				{
					next = poly->pointStart->pos;
				}
				else
				{
					next = temp->pos;
				}

				V2d v0( polyCurr->pos.x, polyCurr->pos.y );
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
				poly->AddPoint( new TerrainPoint(  Vector2i(x,y), false ) );

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

					if( edgeIndex == terrain->numPoints - 1 )
						edgeIndex = 0;
					else
						edgeIndex++;

					a = new GoalParams( this, terrain, edgeIndex, edgeQuantity );
					//a->SetAsGoal( terrain, edgeIndex, edgeQuantity );
				}
				else if( typeName == "patroller" )
				{
					Vector2i pos;

					//always air
					is >> pos.x;
					is >> pos.y;

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
				else if( typeName == "key" )
				{
					Vector2i pos;

					//always in air
					is >> pos.x;
					is >> pos.y;

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

					int gateType;
					is >> gateType;

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

					int stayFrames;
					is >> stayFrames;

					bool teleport;
					string teleStr;
					is >> teleStr;
					if( teleStr == "+tele" )
					{
						teleport = true;
					}
					else if( teleStr == "-tele" )
					{
						teleport = false;
					}

					//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
					//a = new PatrollerParams( this, pos, globalPath, speed, loop );
					a = new KeyParams( this, pos, globalPath, speed, loop, stayFrames, teleport, (GateInfo::GateTypes)gateType );
				}
				else if( typeName == "crawler" )
				{

					//always grounded

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

					if( edgeIndex == terrain->numPoints - 1 )
						edgeIndex = 0;
					else
						edgeIndex++;

					//a->SetAsCrawler( at, terrain, edgeIndex, edgeQuantity, clockwise, speed ); 
					a = new CrawlerParams( this, terrain, edgeIndex, edgeQuantity, clockwise, speed ); 
				}
				else if( typeName == "basicturret" )
				{
					//always grounded

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

					if( edgeIndex == terrain->numPoints - 1 )
						edgeIndex = 0;
					else
						edgeIndex++;

					//a->SetAsBasicTurret( at, terrain, edgeIndex, edgeQuantity, bulletSpeed, framesWait );
					a = new BasicTurretParams( this, terrain, edgeIndex, edgeQuantity, bulletSpeed, framesWait );
				}
				else if( typeName == "foottrap" )
				{
					//always grounded
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

					if( edgeIndex == terrain->numPoints - 1 )
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

		int numGates;
		is >> numGates;
		cout << "numgates: " << numGates << endl;
		for( int i = 0; i < numGates; ++i )
		{
			int gType;
			int poly0Index, vertexIndex0, poly1Index, vertexIndex1;
			is >> gType;
			is >> poly0Index;
			is >> vertexIndex0;
			is >> poly1Index;
			is >> vertexIndex1;

			int testIndex = 0;
			TerrainPolygon *terrain0 = NULL;
			TerrainPolygon *terrain1 = NULL;
			bool first = true;
			for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
			{
				if( testIndex == poly0Index )
				{
					terrain0 = (*it);

					if( first )
						first = false;
					else
						break;
				}
				if( testIndex == poly1Index )
				{
					terrain1 = (*it);

					if( first )
						first = false;
					else
						break;
				}
				testIndex++;
			}

			GateInfo *gi = new GateInfo;
			gi->poly0 = terrain0;
			gi->poly1 = terrain1;
			gi->vertexIndex0 = vertexIndex0;
			gi->vertexIndex1 = vertexIndex1;
			gi->type = (GateInfo::GateTypes)gType;
			gi->edit = this;

			int index = 0;
			for( TerrainPoint *curr = gi->poly0->pointStart; curr != NULL; curr = curr->next )
			{
				if( index == vertexIndex0 )
				{
					gi->point0 = curr;
					curr->gate = gi;					
					break;
				}
				++index;
			}

			index = 0;
			//cout << "poly1: " << gi->poly1 << endl;
			for( TerrainPoint *curr = gi->poly1->pointStart; curr != NULL; curr = curr->next )
			{
				if( index == vertexIndex1 )
				{
					gi->point1 = curr;
					curr->gate = gi;
					break;
				}
				++index;
			}

			gi->UpdateLine();
			gates.push_back( gi );
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
		MessagePop( "Map not saved because no goal is in place. \nPlease add it from the CREATE ENEMIES mode." );
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
			pointCount += (*it)->numPoints;
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

			of <<  (*it)->numPoints << endl;

			for( TerrainPoint *pcurr = (*it)->pointStart;  pcurr != NULL; pcurr = pcurr->next )
			{
				of << pcurr->pos.x << " " << pcurr->pos.y << endl; // << " " << (int)(*it2).special << endl;
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
			
			of <<  (*it)->numPoints << endl;

			for( TerrainPoint *pcurr = (*it)->pointStart;  pcurr != NULL; pcurr = pcurr->next )
			{
				of << pcurr->pos.x << " " << pcurr->pos.y << endl;
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

	of << gates.size() << endl;
	for( list<GateInfo*>::iterator it = gates.begin(); it != gates.end(); ++it )
	{
		(*it)->WriteFile( of );
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
	for( TerrainPoint *curr = poly->pointStart;  curr != NULL; curr = curr->next )
	{
		Vector2i next;

		TerrainPoint *temp = curr->next;
		if( temp == NULL )
		{
			next = poly->pointStart->pos;
		}
		else
		{
			next = temp->pos;
		}

		V2d v0( curr->pos.x, curr->pos.y );
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
	//cout << "brush: " << brush->enemies.size() << endl;
	//cout << "poly: " << poly->enemies.size() << endl;

	int totalEnemies = brush->enemies.size() + poly->enemies.size();
	int totalGates = 0;

	for( TerrainPoint *curr = brush->pointStart; curr != NULL; curr = curr->next )
	{
		if( curr->gate != NULL )
		{
			GateInfo *g = curr->gate;
			curr->gate->point0->gate = NULL;	
			curr->gate->point1->gate = NULL;
			gates.remove( g );
			totalGates++;
			delete g;
		}
	}
	for( TerrainPoint *curr = poly->pointStart; curr != NULL; curr = curr->next )
	{
		if( curr->gate != NULL )
		{
			GateInfo *g = curr->gate;
			gates.remove( g );
			g->point0->gate = NULL;	
			g->point1->gate = NULL;
			
			totalGates++;
			delete g;
		}
	}

	if( totalEnemies > 0 || totalGates > 0)
	{
		stringstream ss;
		ss << "destroying " << totalEnemies << " enemies, and " <<  totalGates << " gates to create the polygons.\n Sorry for how messy this is at the moment!";
		MessagePop( ss.str() );
		brush->DestroyEnemies();
		poly->DestroyEnemies();
	}

	TerrainPolygon z( &grassTex );
	//1: choose start point

	Vector2i startPoint;
	bool startPointFound = false;
	bool firstPolygon = true;


	TerrainPolygon *currentPoly = NULL;
	TerrainPolygon *otherPoly = NULL;
	TerrainPoint *curr = poly->pointStart;
	TerrainPoint *start;	

	TerrainPolygon *minPoly = NULL;


	//get which polygon I should start on
	for(; curr != NULL; curr = curr->next )
	{
		//if( !brush->ContainsPoint( Vector2f( (*it).pos.x, (*it).pos.y) ) )
		//{
			if( !startPointFound )
			{
				startPoint = curr->pos;
				start = curr;
				startPointFound = true;
			}
			else
			{
				if( curr->pos.y > startPoint.y )
				{
					startPoint = curr->pos;
					start = curr;
				}
			}
		//}
	}
	
	curr = brush->pointStart;
	for(; curr != NULL; curr = curr->next )
	{
		//if( !poly->ContainsPoint( Vector2f( (*it).pos.x, (*it).pos.y) ) )
		if( !startPointFound )
		{
			startPoint = curr->pos;
			startPointFound = true;
			firstPolygon = false;
			start = curr;
		}
		else
		{
			if( curr->pos.y > startPoint.y )
			{
				startPoint = curr->pos;
				firstPolygon = false;
				start = curr;
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
	


	curr = start;
	Vector2i currPoint = startPoint;
	curr = curr->next;
	if( curr == NULL )
	{
		curr = currentPoly->pointStart;
	}
	Vector2i nextPoint = curr->pos;


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

		TerrainPoint *min;
		Vector2i minIntersection;
		bool emptyInter = true;
		

		TerrainPoint * other = otherPoly->pointStart;
		Vector2i otherCurrPoint = other->pos;
		other = other->next;
		Vector2i otherNextPoint;// = (*++bit);
		Vector2i minPoint;
		
		LineIntersection li1 = SegmentIntersect( currPoint, nextPoint, otherPoly->pointEnd->pos, otherCurrPoint );
		Vector2i lii1( floor(li1.position.x + .5), floor(li1.position.y + .5) );
		if( !li1.parallel && ( lii1 != currPoint && lii1 != nextPoint && lii1 != otherPoly->pointEnd->pos && lii1 != otherCurrPoint ) ) 
		{
			minIntersection = lii1;
			minPoint = otherCurrPoint;//otherCurrPoint;
			min = otherPoly->pointStart;//--otherIt;
			//++otherIt;
			emptyInter = false;
			//cout << "using this" << endl;
		}

		for(; other != NULL; other = other->next )
		{
			otherNextPoint = other->pos;
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
					min = other;
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
						min = other;
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
			curr = min;
			
			//cout << "adding new intersection: " << minIntersection.x << ", " << minIntersection.y << endl;

			currPoint = minIntersection;

			TerrainPoint *tp = new TerrainPoint( currPoint, false );
			
			
			z.AddPoint( tp );
			
			
			nextPoint = curr->pos;
		}
		else
		{

			currPoint = curr->pos;

			TerrainPoint *tp = new TerrainPoint( currPoint, false );
			tp->gate = curr->gate;
			if( tp->gate != NULL )
			{
			//	cout << "other gate not null!" << endl;
				if( curr == tp->gate->point0 )
				{
			//		cout << "putting a" << endl;
					tp->gate->point0 = tp;
				}
				else if( curr == tp->gate->point1 )
				{
			//		cout << "putting b at: " << tp->pos.x << ", " << tp->pos.y << endl;
					tp->gate->point1 = tp;
					
				}
				else
				{
					//cout << "gate: " << tp->gate->point0->pos.x << ", " << tp->gate->point0->pos.y << ", "
					//	<< tp->gate->point1->pos.x << ", " << tp->gate->point1->pos.y << ", " << endl;
					//cout << "tp: " << tp->pos.x << ", " << tp->pos.y << endl;
					assert( false );
					//tp->gate = NULL;
					//tp->gate == NULL;
				}
			}

			

			if( currentPoly->enemies.count( curr ) > 0 )
			{
				list<ActorParams*> &en = z.enemies[tp];
				en = currentPoly->enemies[curr];
			}
			
			z.AddPoint( tp );


			//cout << "adding point: " << currPoint.x << ", " << currPoint.y << endl;

			if( currPoint == startPoint && !firstTime )
				break;
			

			curr = curr->next;
			if( curr == NULL )
			{
				curr = currentPoly->pointStart;
			}
			nextPoint = curr->pos;
	//		cout << "nextpoing from adding: " << nextPoint.x << ", " << nextPoint.y << endl;
		}
		firstTime = false;
	}

	poly->Reset();
	//cout << "poly size: " << z.points.size() << endl;
	for( TerrainPoint *zit = z.pointStart; zit != NULL; zit = zit->next )
	{
		TerrainPoint *tp = new TerrainPoint(*zit);
		if( tp->gate != NULL )
		{
			//cout << "new polygon will have gate" << endl;
			if( zit == tp->gate->point0 )
			{
				tp->gate->point0 = tp;
				tp->gate->poly0 = poly;
			//	cout << "checking a at: " << tp->pos.x << ", " << tp->pos.y << endl;
			}
			else
			{
				tp->gate->point1 = tp;
				tp->gate->poly1 = poly;
		//		cout << "checking b at: " << tp->pos.x << ", " << tp->pos.y << endl;
			}

		//	cout << "tpgate0: " << tp->gate->point0->pos.x << ", " << tp->gate->point0->pos.y << ", tpgate1: " <<
		//		tp->gate->point1->pos.x << ", " << tp->gate->point1->pos.y << endl;
			//tp->gate->UpdateLine();
		}
		

		if( z.enemies.count( zit ) > 0 )
		{
			list<ActorParams*> &en = poly->enemies[tp];
			en = z.enemies[zit];
			//cout << "zsize: " << en.size() << endl;
			for( list<ActorParams*>::iterator it = en.begin(); it != en.end(); ++it )
			{
				//cout << "setting new ground on actor params" << endl;
				(*it)->groundInfo->ground = poly;
				(*it)->groundInfo->edgeStart = tp;
			}
		}

		poly->AddPoint( tp );

	}

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

bool EditSession::QuadPolygonIntersect( TerrainPolygon *poly, Vector2i a, Vector2i b, Vector2i c, Vector2i d )
{
	TerrainPolygon quadPoly( poly->grassTex );
	quadPoly.AddPoint( new TerrainPoint( a, false ) );
	quadPoly.AddPoint( new TerrainPoint( b, false ) );
	quadPoly.AddPoint( new TerrainPoint( c, false ) );
	quadPoly.AddPoint( new TerrainPoint( d, false ) );
	quadPoly.UpdateBounds();

	//cout << "quad bottom: " << quadPoly.bottom << endl;
	//cout << "poly top: " << poly->top << endl;
	

	bool touching = poly->IsTouching( &quadPoly );
	return touching;

	/*int qLeft = min( a.x, min( b.x, min( c.x, d.x ) ) );
	int qRight = max( a.x, min( b.x, min( c.x, d.x ) ) );
	int qTop = min( a.y, min( b.y, min( c.y, d.y ) ) );
	int qBot = max( a.y, min( b.y, min( c.y, d.y ) ) );

	if( poly->left >= qLeft && poly->right <= qRight && poly->top >= qTop && poly->bottom <= qBot )
	{
		return true;
	}

	IntRect ri( qLeft, qTop, qRight - qLeft, qBot - qTop );
	IntRect riPoly( poly->left, poly->top, poly->right - poly->left, poly->bottom - poly->top );

	if( !ri.intersects( riPoly ) )
	{
		return false;
	}

	bool containsA = poly->ContainsPoint( Vector2f( a.x, a.y ) );
	bool containsB = poly->ContainsPoint( Vector2f( b.x, b.y ) );
	bool containsC = poly->ContainsPoint( Vector2f( c.x, c.y ) );
	bool containsD = poly->ContainsPoint( Vector2f( d.x, d.y ) );

	if( containsA || containsB || containsC || containsD )
	{
		return true;
	}

	for( PointList::iterator it = poly->points.begin(); it != poly->points.end(); ++it )
	{
		Vector2i &p = (*it).pos;
		
	}

	return false;*/
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
	int width = 1920;//1920 - w->getSize().x;
	int height = 1080; //1080 - w->getSize().y;
	uiView = View( sf::Vector2f( width / 2, height / 2), sf::Vector2f( width, height ) );
	v.setCenter( 0, 0 );
	v.setSize( 1920, 1080 );
	w->setView( v );

	confirm = CreatePopupPanel( "confirmation" );
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
	selectedGate = NULL;
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

	
	ActorType *keyType = new ActorType( "key", NULL );

	ActorType *greenKeyType = new ActorType( "greenkey", NULL );
	ActorType *blueKeyType = new ActorType( "bluekey", NULL );

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
	types["key"] = keyType;
	types["crawler"] = crawlerType;
	types["basicturret"] = basicTurretType;
	types["foottrap"] = footTrapType;
	types["goal"] = goalType;
	types["greenkey"] = greenKeyType;
	types["bluekey"] = blueKeyType;

	Panel *keyPanel = CreateOptionsPanel( "key" );

	keyType->panel = keyPanel;
	greenKeyType->panel = keyPanel;
	blueKeyType->panel = keyPanel;

	enemySelectPanel = new Panel( "enemyselection", 200, 200, this );
	GridSelector *gs = enemySelectPanel->AddGridSelector( "world0enemies", Vector2i( 20, 20 ), 3, 3, 32, 32, false, true );
	//gs->selectedX = -1;
	//gs->selectedY = -1;
	//GridSelector gs( 3, 2, 32, 32, this );
	gs->active = false;

	sf::Sprite s0( patrollerType->iconTexture );
	sf::Sprite s1( crawlerType->iconTexture );
	sf::Sprite s2( basicTurretType->iconTexture );
	sf::Sprite s3( footTrapType->iconTexture );
	sf::Sprite s4( goalType->iconTexture );
	sf::Sprite s5( keyType->iconTexture );

	sf::Sprite ss0( greenKeyType->iconTexture );
	sf::Sprite ss1( blueKeyType->iconTexture );


	gs->Set( 0, 0, s0, "patroller" );
	gs->Set( 1, 0, s1, "crawler" );
	gs->Set( 0, 1, s2, "basicturret" );
	gs->Set( 1, 1, s3, "foottrap" );
	gs->Set( 2, 0, s4, "goal" );
	gs->Set( 0, 2, s5, "key" );
	//gs->Set( 1, 2, ss0, "greenkey" );
	//gs->Set( 2, 2, ss1, "bluekey" );

	gateSelectorPopup = CreatePopupPanel( "gateselector" );
	GridSelector *gateSel = gateSelectorPopup->AddGridSelector( "gatetypes", Vector2i( 20, 20 ), 3, 1, 32, 32, false, true );
	

	gateSel->Set( 0, 0, s5, "red" );
	gateSel->Set( 1, 0, ss0, "green" );
	gateSel->Set( 2, 0, ss1, "blue" );

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

	
	//sf::Vector2u wSize = w->getSize();
	
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

	//mode = CREATE_TERRAIN;
	mode = EDIT;
	Emode stored = mode;
	bool canCreatePoint = true;
	gs->active = true;

	


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


	while( !quit )
	{
		
		prevWorldPos = worldPos;
		pixelPos = sf::Mouse::getPosition( *w );
		pixelPos.x *= 1920.f / w->getSize().x;
		pixelPos.y *= 1080.f / w->getSize().y;


		Vector2f tempWorldPos = preScreenTex->mapPixelToCoords(pixelPos);
		worldPos.x = tempWorldPos.x;
		worldPos.y = tempWorldPos.y;

		preScreenTex->setView( uiView );
		Vector2f uiMouse = preScreenTex->mapPixelToCoords( pixelPos );
		//uiMouse.x *= 1920.f / w->getSize().x;
		//uiMouse.y *= 1080.f / w->getSize().y;
		//cout << "uiMouse: " << uiMouse.x << ", " << uiMouse.y << endl;
		
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
								if( showPanel != NULL )
								{	
									showPanel->Update( true, uiMouse.x, uiMouse.y );
									break;
								}
							}
							
							break;
						}
					case Event::MouseButtonReleased:
						{
							if( showPanel != NULL )
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
							if( showPanel != NULL )
							{
								showPanel->SendKey( ev.key.code, ev.key.shift );
								break;
							}


							if( ev.key.code == Keyboard::Space )
							{
							//	ConfirmationPop();
								//MessagePop( "hello" );
								if( showPoints && extendingPolygon )
								{
								}
								else
								{
								if( polygonInProgress->numPoints > 2 )
								{
									//test final line
									bool valid = true;


									TerrainPoint * test = polygonInProgress->pointStart;
									TerrainPoint * prev = test;
									test = test->next;

									//cout << "lastline: " << polygonInProgress->points.back().pos.x << ", " << polygonInProgress->points.back().pos.y <<
									//	" .. " << polygonInProgress->points.front().pos.x << ", " << polygonInProgress->points.front().pos.y << endl;
									for( ; test != NULL; test = test->next )
									{
										Vector2i a = prev->pos;
										Vector2i b = test->pos;
										Vector2i c = polygonInProgress->pointEnd->pos;
										Vector2i d = polygonInProgress->pointStart->pos;
										LineIntersection li = LimitSegmentIntersect( a,b,c,d );
										Vector2i lii( floor(li.position.x + .5), floor(li.position.y + .5) );
										//if( !li.parallel  && (abs( lii.x - currPoint.x ) >= 1 || abs( lii.y - currPoint.y ) >= 1 ))
										if( !li.parallel )//(abs( lii.x - currPoint.x ) > 1 || abs( lii.y - currPoint.y ) > 1 ) )//&& lii != a && lii != b && lii != c && lii != d )
										{
											valid = false;
										}

										

										prev = test;
									}


									for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
									{
										//if( !PointValid( polygonInProgress->points.back().pos, polygonInProgress->points.front().pos, (*it) ) )
										if( !IsPointValid( polygonInProgress->pointEnd->pos, polygonInProgress->pointStart->pos, (*it) ) )
										{
											valid = false;
											break;
										}
									}

									if( PolyIntersectGate( *polygonInProgress ) )
									{
										valid = false;
									}

									//if( !IsPolygonValid( *polygonInProgress, NULL ) )
									//	valid = false;

									if( !valid )
									{
										MessagePop( "unable to complete polygon" );
										//popupPanel = messagePopup;
										break;
									}

									//if( !PointValid( polygonInProgress->points.back().pos, polygonInProgress->points.front().pos ) )
									//	break;

									list<TerrainPolygon*>::iterator it = polygons.begin();
									bool added = false;
									polygonInProgress->Finalize(); //i should check if i can remove this
									bool recursionDone = false;
									TerrainPolygon *currentBrush = polygonInProgress;

										while( it != polygons.end() )
										{
											TerrainPolygon *temp = (*it);
											if( temp != currentBrush && currentBrush->IsTouching( temp ) )
											{
												cout << "before addi: " << (*it)->numPoints << endl;
						
												Add( currentBrush, temp );

												polygonInProgress->Reset();
						
												cout << "after adding: " << (*it)->numPoints << endl;

												

												polygons.erase( it );

												currentBrush = temp;

												for( TerrainPoint *tp = currentBrush->pointStart; tp != NULL; tp = tp->next )
												{
													if( tp->gate != NULL )
													{
														cout << "gate: " << tp->gate->point0->pos.x << ", " << tp->gate->point0->pos.y
															<< ", " << tp->gate->point1->pos.x << ", " << tp->gate->point1->pos.y << endl;
														//cout << "gate pos: " << tp->pos.x << ", " << tp->pos.y << endl;
													}
												}

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
										
										for( TerrainPoint *tp = currentBrush->pointStart; tp != NULL; tp = tp->next )
										{
											//if( tp->gate != NULL )
											//{
											//	cout << "gate: " << tp->gate->point0->pos.x << ", " << tp->gate->point0->pos.y
											//		<< ", " << tp->gate->point1->pos.x << ", " << tp->gate->point1->pos.y << endl;
											//	//cout << "gate pos: " << tp->pos.x << ", " << tp->pos.y << endl;
											//}
												
										}

										polygons.push_back( currentBrush );
										polygonInProgress->Reset();

										

									}
								}

								if( polygonInProgress->numPoints <= 2 && polygonInProgress->numPoints > 0  )
								{
									cout << "cant finalize. cant make polygon" << endl;
									polygonInProgress->ClearPoints();
								}
								}
							}
							else if( ev.key.code == sf::Keyboard::X || ev.key.code == sf::Keyboard::Delete )
							{
								//cout << "PRESSING V: " << polygonInProgress->points.size() << endl;
								if( polygonInProgress->numPoints > 0 )
								{
									polygonInProgress->RemovePoint( polygonInProgress->pointEnd );
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
								//if( !showPoints )
								if( false ) // this is only turned off for the beta build so I don't have to debug this.
								{
									showPoints = true;
									extendingPolygon = NULL;
									extendingPoint = NULL;
									polygonInProgress->ClearPoints();
								}
							}
							
							break;
						}
					case Event::KeyReleased:
						{
							if( ev.key.code == sf::Keyboard::E )
							{
								if( false ) //only for this build
								{

								showPoints = false;
								extendingPolygon = NULL;
								extendingPoint = NULL;
								polygonInProgress->ClearPoints();
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
			case EDIT:
				{
					switch( ev.type )
					{
					case Event::MouseButtonPressed:
						{
							if( ev.mouseButton.button == Mouse::Left )
							{
								if( showPanel != NULL )
								{	
									//cout << "edit mouse update" << endl;
									showPanel->Update( true, uiMouse.x, uiMouse.y );
									break;
								}

								if( showGrass )
									break;

								//lights
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
									selectedGate = NULL;
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

								//grab player
								if( playerSprite.getGlobalBounds().contains( worldPos.x, worldPos.y ) )
								{
									selectedActor = NULL;
									selectedLight = NULL;
									selectedGate = NULL;
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

								//points
								for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
									it != selectedPolygons.end(); ++it )
								{
									for( TerrainPoint *curr = (*it)->pointStart; curr != NULL; curr = curr->next )
									{
										if( length( worldPos - V2d( curr->pos.x, curr->pos.y ) ) < 8 * zoomMultiple )
										{
											if( curr->selected ) //selected 
											{
												curr->selected = false;
												emptySpace = false;
												selectedGate = NULL;
												break;
											}
											else
											{
												if( Keyboard::isKeyPressed( Keyboard::LShift ) )
												{
													
												}
												else
												{
													for( list<TerrainPolygon*>::iterator it2 = selectedPolygons.begin(); it2 != selectedPolygons.end(); ++it2 )
													{
														for( TerrainPoint *curr = (*it2)->pointStart; curr != NULL; curr = curr->next )
														{
															curr->selected = false;
														}
													}
												}

												curr->selected = true;
												emptySpace = false;
												selectedGate = NULL;
												break;


											}
											
											
										}
									}
								}

								//polygons
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
														selectedActor = NULL;
														selectedGate = NULL;
														selectedLight = NULL;
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
														selectedActor = NULL;
														selectedGate = NULL;
														selectedLight = NULL;
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
								
								//enemies
								bool empty = emptySpace;
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

								if( empty )
								{
									GateInfo *closest = NULL;
									double closestDist = 50;
									for( list<GateInfo*>::iterator it = gates.begin(); it != gates.end(); ++it )
									{
										double extra = 50;
										double gLeft = std::min( (*it)->point0->pos.x, (*it)->point1->pos.x ) - extra;
										double gRight = std::max( (*it)->point0->pos.x, (*it)->point1->pos.x ) + extra;
										double gTop = std::min( (*it)->point0->pos.y, (*it)->point1->pos.y ) - extra;
										double gBot = std::max( (*it)->point0->pos.y, (*it)->point1->pos.y ) + extra;
										Rect<double> r( gLeft, gTop, gRight - gLeft, gBot - gTop );

										//aabb collision
										if( gLeft <= worldPos.x && gRight >= worldPos.x && gTop <= worldPos.y && gBot >= worldPos.y )
										{
											V2d v0((*it)->point0->pos.x, (*it)->point0->pos.y );

											double dist = abs(cross( worldPos - v0, normalize( V2d( (*it)->point1->pos.x, (*it)->point1->pos.y ) - v0 ) ));
											if( dist < closestDist )
											{
												closest = (*it);
												closestDist = dist;
											}
										}
									}


									if( closest != NULL )
									{
										selectedGate = closest;
										empty = false;
									}

								}
							//	cout << "made it!!!" << endl;
								if( empty )
								{
									selectedActor = NULL;
									selectedLight = NULL;
									selectedGate = NULL;
									selectedActorGrabbed = false;
								}

							}
							break;
						}
					case Event::MouseButtonReleased:
						{
							if( ev.mouseButton.button == Mouse::Left )
							{
								if( showPanel != NULL )
								{	
									showPanel->Update( false, uiMouse.x, uiMouse.y );
									break;
								}

								if( !pasteBrushes.empty() )
								{
									bool validPaste = true;
									for( list<TerrainBrush*>::iterator tbIt = pasteBrushes.begin();
										tbIt != pasteBrushes.end(); ++tbIt )
									{										
										for( list<TerrainPolygon*>::iterator it = polygons.begin(); 
											it != polygons.end() && validPaste; ++it )
										{
											TerrainPolygon *currentBrush = new TerrainPolygon( (*it)->grassTex );
										
											for( TerrainPoint *curr = (*tbIt)->pointStart; curr != NULL;
												curr = curr->next )
											{
												currentBrush->AddPoint( new TerrainPoint(*curr) );
											}

											currentBrush->UpdateBounds();

											for( TerrainPoint *curr = currentBrush->pointStart; curr != NULL && validPaste;
												curr = curr->next )
											{
												TerrainPoint *prev;
												if( curr == currentBrush->pointStart )
												{
													prev = currentBrush->pointEnd;
												}
												else
												{
													prev = curr->prev;
												}

												if( !IsPointValid( prev->pos, curr->pos, (*it) ) )
												{
													validPaste = false;
												}


											}

											delete currentBrush;
										}
									}

									if( validPaste)
									{
									for( list<TerrainBrush*>::iterator tbIt = pasteBrushes.begin();
										tbIt != pasteBrushes.end(); ++tbIt )
									{
										list<TerrainPolygon*>::iterator it = polygons.begin();
										bool added = false;
										//polygonInProgress->Finalize(); //i should check if i can remove this
										bool recursionDone = false;

										cout << "b4" << endl;
										TerrainPolygon *currentBrush = new TerrainPolygon( (*it)->grassTex );
										
										//cout << "after: " << (unsigned int)((*tbIt)->pointStart) << endl;
										for( TerrainPoint *curr = (*tbIt)->pointStart; curr != NULL;
											curr = curr->next )
										{
											cout << "adding" << endl;
											currentBrush->AddPoint( new TerrainPoint(*curr) );
										}

										currentBrush->UpdateBounds();

										

									//	TerrainPolygon *currentBrush = polygonInProgress;
										
										while( it != polygons.end() )
										{
											TerrainPolygon *temp = (*it);
											if( currentBrush->IsTouching( temp ) )
											{
												cout << "before addi: " << (*it)->numPoints << endl;
						
												Add( currentBrush, temp );

												//currentBrush->Reset();
												delete currentBrush;
												currentBrush = NULL;
												//polygonInProgress->Reset();
						
												cout << "after adding: " << (*it)->numPoints << endl;

												

												polygons.erase( it );

												currentBrush = temp;

												/*for( TerrainPoint *tp = currentBrush->pointStart; tp != NULL; tp = tp->next )
												{
													if( tp->gate != NULL )
													{
														cout << "gate: " << tp->gate->point0->pos.x << ", " << tp->gate->point0->pos.y
															<< ", " << tp->gate->point1->pos.x << ", " << tp->gate->point1->pos.y << endl;
														//cout << "gate pos: " << tp->pos.x << ", " << tp->pos.y << endl;
													}
												}*/

												it = polygons.begin();

												added = true;
							
												continue;
											}
											else
											{
												cout << "not" << endl;
											}
											++it;
										}
				
									//add final check for validity here
				
										if( !added )
										{
											cout << "not added" << endl;
											TerrainPolygon *brushPoly = new TerrainPolygon( polygonInProgress->grassTex );
										
											for( TerrainPoint *curr = (*tbIt)->pointStart; curr !=  NULL;
												curr = curr->next )
											{
												brushPoly->AddPoint( new TerrainPoint(*curr) );
											}

											brushPoly->Finalize();
											polygons.push_back( brushPoly );
											
										}
										else
										{
											cout << "was added" << endl;
											for( TerrainPoint *tp = currentBrush->pointStart; tp != NULL; tp = tp->next )
											{
												//if( tp->gate != NULL )
												//{
												//	cout << "gate: " << tp->gate->point0->pos.x << ", " << tp->gate->point0->pos.y
												//		<< ", " << tp->gate->point1->pos.x << ", " << tp->gate->point1->pos.y << endl;
												//	//cout << "gate pos: " << tp->pos.x << ", " << tp->pos.y << endl;
												//}
												
											}

											polygons.push_back( currentBrush );
											
											//polygonInProgress->Reset();

										

										}
									}

									ClearPasteBrushes();
									}
									else
									{
										MessagePop( "invalid paste" );
									}
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
					case Event::MouseMoved:
						{
							//delta
							//cout << "delta mouse: " << ev.mouseMove.x << ", " << ev.mouseMove.y << endl;
							break;
						}
					case Event::KeyPressed:
						{
							if( showPanel != NULL )
							{
								showPanel->SendKey( ev.key.code, ev.key.shift );
								break;
							}

							if( ev.key.code == Keyboard::C && ev.key.control )
							{
								if( selectedPolygons.size() > 0 )
								{
									ClearCopyBrushes();

									for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
										it != selectedPolygons.end(); ++it )
									{
										TerrainBrush *tb = new TerrainBrush( (*it) );
										copyBrushes.push_back( tb );
									}
								}
							}
							else if( ev.key.code == Keyboard::V && ev.key.control )
							{
								ClearPasteBrushes();

								if( copyBrushes.size() > 0 )
								{
									//pasteBrushes = copyBrushes;
									
									CopyToPasteBrushes();
									
									pastePos = Vector2i( worldPos.x, worldPos.y );

									//find the overall bounding box of all the copied polygons
									list<TerrainBrush*>::iterator tbIt = pasteBrushes.begin();
									int trueLeft = (*tbIt)->left;
									int trueRight = (*tbIt)->right;
									int trueTop = (*tbIt)->top;								
									int trueBot = (*tbIt)->bot;

									++tbIt;
									for( ; tbIt != pasteBrushes.end(); ++tbIt )
									{
										if( (*tbIt)->left < trueLeft )
											trueLeft = (*tbIt)->left;
										if( (*tbIt)->right > trueRight )
											trueRight = (*tbIt)->right;
										if( (*tbIt)->top < trueTop )
											trueTop = (*tbIt)->top;
										if( (*tbIt)->bot > trueBot )
											trueBot = (*tbIt)->bot;
									}

									Vector2i trueCenter( (trueRight + trueLeft) / 2, (trueTop + trueBot)/2 );
								
									//move it to the cursors position originally
									Vector2i startDiff = pastePos - trueCenter;
									for( tbIt = pasteBrushes.begin(); tbIt != pasteBrushes.end(); ++tbIt )
									{
										(*tbIt)->Move( startDiff );
									}
								}

							}
							else if( ev.key.code == Keyboard::X || ev.key.code == Keyboard::Delete )
							{
								if( !pasteBrushes.empty() )
								{
									ClearPasteBrushes();
								}
								else if( CountSelectedPoints() > 0 )
								{
									int removeSuccess = IsRemovePointsOkay();

									if( removeSuccess == 1 )
									{
										//go through each polygon and get rid of the actors which are deleted by deleting points
										for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
											it != selectedPolygons.end(); ++it )
										{
											for( EnemyMap::iterator mapIt = (*it)->enemies.begin(); mapIt != (*it)->enemies.end(); ++mapIt)
											{
												list<ActorParams*>::iterator et = (*mapIt).second.begin();
												while( et != (*mapIt).second.end() )
												{
													TerrainPoint *edgeEnd = (*et)->groundInfo->edgeStart->next;
													if( edgeEnd == NULL )
														edgeEnd = (*et)->groundInfo->ground->pointStart;

													bool deleted = (*(*et)->groundInfo->edgeStart).selected || edgeEnd->selected;
													if (deleted)
													{
														(*et)->group->actors.remove( (*et ) );
														delete (*et); //deleting actor
														(*it)->enemies[(*et)->groundInfo->edgeStart].erase(et++); 
													}
													else
													{
														++et;
													}
												}
											}
										}


										for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
											it != selectedPolygons.end(); ++it )
										{
											(*it)->RemoveSelectedPoints();
										}
									}
									else if( removeSuccess == 0 )
									{
										MessagePop( "problem removing points" );
										//messagePopup->labels["message"]->setString( "problem removing points" );
									}
								}
								else if( selectedActor != NULL )
								{
									if( selectedActor->groundInfo != NULL && selectedActor->groundInfo->ground != NULL )
									{
										selectedActor->groundInfo->ground->enemies[selectedActor->groundInfo->edgeStart].remove( selectedActor );
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
								else if( selectedGate != NULL )
								{
									gates.remove( selectedGate );
									selectedGate->point0->gate = NULL;
									selectedGate->point1->gate = NULL;
									delete selectedGate;
									selectedGate = NULL;
								}
								else
								{
									
									int erasedGates = 0;
									for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin();
										it != selectedPolygons.end(); ++it )
									{
										polygons.remove( (*it) );
										
										for( list<GateInfo*>::iterator git = gates.begin(); git != gates.end(); )
										{
											if( (*git)->poly0 == (*it) || (*git)->poly1 == (*it) )
											{
												delete (*git);
												git = gates.erase( git );
												++erasedGates;
											}
											else
											{
												++git;
											}
										}
										(*it)->DestroyEnemies();
										delete (*it);
									}
									if( erasedGates > 0 )
									{
										stringstream ss;
										ss << "destroyed " << erasedGates << " gates";
										MessagePop( ss.str() );
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
								else if( selectedPolygons.size() > 0 && !polyGrab )
								{
									polyGrab = true;

									if( Keyboard::isKeyPressed( Keyboard::G ) )
									{
										V2d graphPos = GraphPos( worldPos );
										polyGrabPos = Vector2i( graphPos.x, graphPos.y );//Vector2i( graphPos.x, graphPos.y );
									}
									else
									{
										//cout << "grab pos: " << endl;
										polyGrabPos = Vector2i( worldPos.x, worldPos.y );

										//polyGrabPos = //pixelPos;//Vector2i( worldPos.x, worldPos.y );
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
							else if( ev.key.code == Keyboard::B )
							{
								int countPoints = CountSelectedPoints();
								bool first = true;
								bool ddone = false;

								GateInfo testInfo;
								if( countPoints == 2 )
								{
									for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end() && !ddone; ++it )
									{
										int index = 0;
										for( TerrainPoint *curr = (*it)->pointStart; curr != NULL && !ddone; curr = curr->next )
										{
											if( curr->selected ) //selected
											{
												if( first )
												{
													testInfo.poly0 = (*it);
													testInfo.vertexIndex0 = index;
													testInfo.point0 = curr;
													first = false;
												}
												else
												{
													testInfo.poly1 = (*it);
													testInfo.vertexIndex1 = index;
													ddone = true;
													testInfo.point1 = curr;
												}
											}
											++index;
										}
									}

									bool result = CanCreateGate( testInfo );

									if( result )
									{
										GridSelectPop( "gatetype" );

										
										//MessagePop( "gate created" );
										GateInfo *gi = new GateInfo;

										gi->SetType( tempGridResult );
										//gi->type = tempGridResult;

										gi->edit = this;
										gi->poly0 = testInfo.poly0;
										gi->vertexIndex0 = testInfo.vertexIndex0;
										gi->point0 = testInfo.point0;
										gi->point0->gate = gi;

										gi->poly1 = testInfo.poly1;
										gi->vertexIndex1 = testInfo.vertexIndex1;
										gi->point1 = testInfo.point1;
										gi->point1->gate = gi;
										gi->UpdateLine();


										

										gates.push_back( gi );
									}
									else
									{
										MessagePop( "gate would intersect some terrain" );
									}
								}
								else
								{
									MessagePop( "you require two points to create a gate" );
								}

								
							}
							break;
						}
					case Event::KeyReleased:
						{
							if( ev.key.code == Keyboard::W )
							{
								if( pointGrab )
								{
									for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
									{
										if( (*it)->movingPointMode )
										{
											(*it)->SoftReset();
											(*it)->Finalize();
											(*it)->movingPointMode = false;
										}
										
									}
								}

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
										for( TerrainPoint *curr = (*it)->pointStart; curr != NULL; curr = curr->next )
										{
											if( selectRect.contains( Vector2f( curr->pos.x, curr->pos.y ) ) )
											{
												if( curr->selected ) //selected 
												{
													if( Keyboard::isKeyPressed( Keyboard::LShift ) )
													{
														curr->selected = false;
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

													curr->selected = true;
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
													curr->selected = false;
												}
											}
										}
									}

									TerrainPolygon tempRectPoly(&grassTex );
									tempRectPoly.AddPoint( new TerrainPoint( Vector2i( selectRect.left, selectRect.top ), false ) );
									tempRectPoly.AddPoint( new TerrainPoint( Vector2i( selectRect.left + selectRect.width, selectRect.top ), false ) );
									tempRectPoly.AddPoint( new TerrainPoint( Vector2i( selectRect.left + selectRect.width, selectRect.top + selectRect.height ), false ) );
									tempRectPoly.AddPoint( new TerrainPoint( Vector2i( selectRect.left, selectRect.top + selectRect.height ), false ) );
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
								if( showPanel != NULL )
								{	
									showPanel->Update( true, uiMouse.x, uiMouse.y );
								}
								/*else if( gs.active )
								{
									gs.Update( true, uiMouse.x, uiMouse.y );
								}*/
							}
							break;
						}
					case Event::MouseButtonReleased:
						{
							if( ev.mouseButton.button == Mouse::Left )
							{


								if( showPanel == NULL && trackingEnemy != NULL )
								{
									bool placementOkay = true;
									//air enemy
									if( enemyQuad.getLocalBounds().width == 0 ) 
									{
										
									}
									else
									{
									

									sf::Transform tf = enemyQuad.getTransform();
									
									Vector2f fa = tf.transformPoint( enemyQuad.getPoint( 0 ) );
									Vector2f fb = tf.transformPoint( enemyQuad.getPoint( 1 ) );
									Vector2f fc = tf.transformPoint( enemyQuad.getPoint( 2 ) );
									Vector2f fd = tf.transformPoint( enemyQuad.getPoint( 3 ) );
									V2d a( fa.x, fa.y );
									V2d b( fb.x, fb.y );
									V2d c( fc.x, fc.y );
									V2d d( fd.x, fd.y );

									
									for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end() && placementOkay; ++it )
									{
										ActorGroup *ag = (*it).second;
										for( list<ActorParams*>::iterator git = ag->actors.begin(); git != ag->actors.end(); ++git )
										{
											ActorParams *params = (*git);
											V2d pa( params->boundingQuad[0].position.x, params->boundingQuad[0].position.y );
											V2d pb( params->boundingQuad[1].position.x, params->boundingQuad[1].position.y );
											V2d pc( params->boundingQuad[2].position.x, params->boundingQuad[2].position.y );
											V2d pd( params->boundingQuad[3].position.x, params->boundingQuad[3].position.y );
											//isQuadTouchingQuad( 

											cout << "a: " << a.x << ", " << a.y << ", b: " << b.x << ", " << b.y <<
												", " << c.x << ", " << c.y << ", " << d.x << ", " << d.y << endl;
											cout << "pa: " << pa.x << ", " << pa.y << ", pb: " << pb.x << ", " << pb.y <<
												", " << pc.x << ", " << pc.y << ", " << pd.x << ", " << pd.y << endl;

											cout << "testing vs: " << params->type->height << endl;
											if( isQuadTouchingQuad( pa, pb, pc, pd, a, b, c, d ) )
											{
												cout << "IS TOUCHING" << endl;
												placementOkay = false;
												break;
											}
											
										}
										
									}

									}

									if( !placementOkay )
									{
										MessagePop( "can't place on top of another actor" );
									}
									else if( trackingEnemy->name == "patroller" )
									{
										showPanel = trackingEnemy->panel;

										showPanel->textBoxes["name"]->text.setString( "test" );
										showPanel->textBoxes["group"]->text.setString( "not test" );
										showPanel->textBoxes["speed"]->text.setString( "10" );
										showPanel->checkBoxes["loop"]->checked = false;


										patrolPath.clear();
										patrolPath.push_back( Vector2i( worldPos.x, worldPos.y ) );

										//mode = CREATE_PATROL_PATH;
										//patrolPath.clear();
										//patrolPath.push_back( Vector2i( worldPos.x, worldPos.y ) );
									}
									else if( trackingEnemy->name == "key" )
									{
										showPanel = trackingEnemy->panel;

										showPanel->textBoxes["name"]->text.setString( "test" );
										showPanel->textBoxes["group"]->text.setString( "not test" );
										showPanel->textBoxes["speed"]->text.setString( "10" );
										showPanel->checkBoxes["loop"]->checked = false;
										showPanel->checkBoxes["teleport"]->checked = false;

										showPanel->gridSelectors["keytype"]->selectedX = 0;
										
										//SetPanelDefault( trackingEnemy );
										patrolPath.clear();
										patrolPath.push_back( Vector2i( worldPos.x, worldPos.y ) );
									}
									else if( trackingEnemy->name == "crawler" )
									{
										//groups["--"]->name
										if( enemyEdgePolygon != NULL )
										{
											showPanel = trackingEnemy->panel;
											showPanel->textBoxes["name"]->text.setString( "test" );
											showPanel->textBoxes["group"]->text.setString( "not test" );
											showPanel->checkBoxes["clockwise"]->checked = false;
											showPanel->textBoxes["speed"]->text.setString( "10" );
											//trackingEnemy = NULL;
										}
									}
									else if( trackingEnemy->name == "basicturret" )
									{
										if( enemyEdgePolygon != NULL )
										{
											showPanel = trackingEnemy->panel;
											showPanel->textBoxes["name"]->text.setString( "test" );
											showPanel->textBoxes["group"]->text.setString( "not test" );
											showPanel->textBoxes["bulletspeed"]->text.setString( "10" );
											showPanel->textBoxes["waitframes"]->text.setString( "10" );
										}
									}
									else if( trackingEnemy->name == "foottrap" )
									{
										if( enemyEdgePolygon != NULL )
										{
											showPanel = trackingEnemy->panel;
											showPanel->textBoxes["name"]->text.setString( "test" );
											showPanel->textBoxes["group"]->text.setString( "not test" );
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
											//showPanel = trackingEnemy->panel;
											showPanel = enemySelectPanel;
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
								/*else if( gs.active )
								{
									if( gs.Update( false, uiMouse.x, uiMouse.y ) )
									{
										cout << "selected enemy index: " << gs.focusX << ", " << gs.focusY << endl;
									}
								}*/

								
							}
							break;
						}
					case Event::MouseWheelMoved:
						{
							break;
						}
					case Event::KeyPressed:
						{
							if( showPanel != NULL )
							{
								showPanel->SendKey( ev.key.code, ev.key.shift );
							}

							if( ev.key.code == Keyboard::X || ev.key.code == Keyboard::Delete )
							{
								if( trackingEnemy != NULL )
								{
									trackingEnemy = NULL;
									showPanel = enemySelectPanel;
								}
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
								break;
							}
						case Event::MouseButtonReleased:
							{
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
								selectedGate = NULL;
								for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
									it != selectedPolygons.end(); ++it )
								{
									(*it)->SetSelected( false );
								}
								selectedPolygons.clear();
							}
							else if( menuDownStored == CREATE_TERRAIN && menuSelection != "none" )
							{
								polygonInProgress->ClearPoints();
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
								else if( menuDownStored == EDIT && selectedGate != NULL )
								{
									//
									//mode = menuDownStored;
								}
								else
								{
									mode = EDIT;
									showPanel = NULL;
								}
							}
							else if( menuSelection == "upperleft" )
							{
								mode = CREATE_ENEMY;
								trackingEnemy = NULL;
								showPanel = enemySelectPanel;
							}
							else if( menuSelection == "upperright" )
							{
								mode = CREATE_TERRAIN;
								showPanel = NULL;
							}
							else if( menuSelection == "lowerleft" )
							{
								mode = CREATE_LIGHTS;
								showPanel = NULL;
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
							if( ( ev.key.code == Keyboard::X || ev.key.code == Keyboard::Delete ) && patrolPath.size() > 1 )
							{
								patrolPath.pop_back();
							}
							else if( ev.key.code == Keyboard::Space )
							{
								if( selectedActor != NULL )
								{
									showPanel = selectedActor->type->panel;
									((PatrollerParams*)selectedActor)->SetPath( patrolPath );
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
							if( ( ev.key.code == Keyboard::X || ev.key.code == Keyboard::Delete ) && selectedPolygons.front()->path.size() > 1 )
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
			
			if( mode != PAUSED && mode != SELECT_MODE )
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
						view.setSize( Vector2f( 1920 * (zoomMultiple), 1080 * ( zoomMultiple ) ) );
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
							polygonInProgress->ClearPoints();
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
							view.setSize( Vector2f( 1920 * (zoomMultiple), 1080 * ( zoomMultiple ) ) );
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

				

				if( showPanel != NULL )
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

				
				if( polygonInProgress->numPoints > 0 )
				{
							
					V2d backPoint = V2d( polygonInProgress->pointEnd->pos.x, polygonInProgress->pointEnd->pos.y );
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
							for( TerrainPoint *curr = (*it)->pointStart; curr != NULL; curr = curr->next )
							{
								Vector2i pointPos = curr->pos;
								double dist = length( V2d( pointPos.x, pointPos.y ) - V2d( testPoint.x, testPoint.y ) );
								if( dist < 8 * zoomMultiple )
								{
									extendingPolygon = (*it);
									extendingPoint = curr;
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
						if( extendingPolygon != NULL && polygonInProgress->numPoints > 0 )
						{
							//no error checking for extending polygon yet. ugh T_T you got this


							okay = !extendingPolygon->ContainsPoint( testPoint );
							
							/*if( polygonInProgress->numPoints > 1 )
							{
								okay = IsPointValid( polygonInProgress->pointStart->pos, 
								Vector2i( testPoint.x, testPoint.y ), polygonInProgress );
							}*/

							if( okay )
							{
								TerrainPoint * okay = extendingPolygon->pointStart;
								TerrainPoint *okayPrev = extendingPolygon->pointEnd;
								for( ; okay != NULL; okay = okay->next )
								{
								
									//LineIntersection li = SegmentIntersect( , worldi, (*okayPrev).pos, (*okayIt).pos );
									Vector2i a = polygonInProgress->pointEnd->pos;
									Vector2i b = worldi;
									Vector2i c = okayPrev->pos;
									Vector2i d = okay->pos;

								
									LineIntersection li = LimitSegmentIntersect( a,b,c,d );
									Vector2i lii( floor(li.position.x + .5), floor(li.position.y + .5) );
									//if( !li.parallel  && (abs( lii.x - currPoint.x ) >= 1 || abs( lii.y - currPoint.y ) >= 1 ))
									if( !li.parallel )//&& lii != a && lii != b && lii != c && lii != d )
									{
										okay = false;
										break;
									}
									okayPrev = okay;
								}
								
								
							}
							// &&  IsPointValid( polygonInProgress->points.back().pos, worldi, extendingPolygon );
							//okay = IsPointValid( polygonInProgress->points.back().pos, worldi, extendingPolygon );
						}

						
						//okay = true;

						bool done = false;
						if( extendingPolygon != NULL )
						{
							for( TerrainPoint *curr = extendingPolygon->pointStart; curr != NULL; curr = curr->next )
							{
								if( curr == extendingPoint )
								{
									continue;
								}
								Vector2i pointPos = curr->pos;
								double dist = length( V2d( pointPos.x, pointPos.y ) - V2d( testPoint.x, testPoint.y ) );
								if( dist < 8 * zoomMultiple )
								{
									//ExtendPolygon();
									extendingPolygon->Extend( extendingPoint, curr, polygonInProgress );

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
							if( polygonInProgress->numPoints > 0 )
							{
								for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
								{
									if( !IsPointValid( polygonInProgress->pointEnd->pos, worldi, (*it) ) )
									{
										validNearPolys = false;
										break;
									}
								}
							}

							cout << "valid: " << validNearPolys << endl;

							if( validNearPolys )
							{
								if( polygonInProgress->numPoints > 0 && length( V2d( testPoint.x, testPoint.y ) - Vector2<double>(polygonInProgress->pointEnd->pos.x, 
									polygonInProgress->pointEnd->pos.y )  ) >= minimumEdgeLength * std::max(zoomMultiple,1.0 ) )
								{
									cout << "check1" << endl;
									if( PointValid( polygonInProgress->pointEnd->pos, worldi ) )
									{
										cout << "blah1" << endl;
										polygonInProgress->AddPoint( new TerrainPoint( worldi, false ) );
									}
								}
								else if( polygonInProgress->numPoints == 0 )
								{
									if( extendingPolygon != NULL )
									{
										polygonInProgress->AddPoint( new TerrainPoint( *extendingPoint ) );
									}
									else
									{
										polygonInProgress->AddPoint( new TerrainPoint( worldi, false ) );
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


				if( !pasteBrushes.empty() )
				{
					Vector2i pasteGrabDelta = Vector2i( pPoint.x, pPoint.y ) - pastePos;
					pastePos = Vector2i( pPoint.x, pPoint.y );
					for( list<TerrainBrush*>::iterator it = pasteBrushes.begin(); it != pasteBrushes.end(); ++it )
					{
						(*it)->Move( pasteGrabDelta );
					}
				}
				else
				{

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
							it != selectedPolygons.end() && !done; ++it )
						{
							for( TerrainPoint *curr = (*it)->pointStart; curr != NULL && !done; curr = curr->next )
							{
								if( curr->selected )
								{
									pointSelected = curr->pos;
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

						int polySize = poly.numPoints;
						Vector2i *deltas = new Vector2i[polySize];
						allDeltas[allDeltaIndex] = deltas;
						int deltaIndex = 0;
						

						double prim_limit = PRIMARY_LIMIT;
						if( Keyboard::isKeyPressed( Keyboard::LShift ) )
						{
							prim_limit = .99;
						}


						for( TerrainPoint *curr = poly.pointStart; curr != NULL ; curr = curr->next )
						{
							
							deltas[deltaIndex] = Vector2i( 0, 0 );

							if( !curr->selected )
							{
								++deltaIndex;
								continue;
							}

							Vector2i diff;

							TerrainPoint *prev, *next;
							if( curr == poly.pointStart )
							{
								prev = poly.pointEnd;
							}
							else
							{
								prev = curr->prev;
							}

							TerrainPoint *temp = curr->next;
							if( temp == NULL )
							{
								next = poly.pointStart;
							}
							else
							{
								next = temp;
							}


							V2d pos(curr->pos.x + pointGrabDelta.x, curr->pos.y + pointGrabDelta.y );
							V2d prevPos( prev->pos.x, prev->pos.y );
							V2d nextPos( next->pos.x, next->pos.y );

							V2d extreme( 0, 0 );
							Vector2i vec = curr->pos - prev->pos;
							V2d normVec = normalize( V2d( vec.x, vec.y ) );

							V2d newVec = normalize( pos - V2d( prev->pos.x, prev->pos.y ) );
		
							if( !prev->selected )
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
										diff.y = curr->pos.y - prev->pos.y;
									
										//(*it2).pos.y = (*prev).pos.y;
										cout << "lining up x: " << diff.y << endl;
									}

									if( extreme.y != 0 )
									{
										diff.x = curr->pos.x - prev->pos.x;

										cout << "lining up y: " << diff.x << endl;
									}
								}
							}
							
							if( !next->selected )
							{
								vec = curr->pos - next->pos;
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
										//diff.y = curr->pos.y - next->pos.y;
									
										//(*it2).pos.y = (*prev).pos.y;
										cout << "lining up x222: " << diff.y << endl;
									}

									if( extreme.y != 0 )
									{
										//diff.x = curr->pos.x - next->pos.x;

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
						//cout << "moving" << endl;
						//cout << "valid move" << endl;
						//int 
						allDeltaIndex = 0;
						for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin();
							it != selectedPolygons.end(); ++it )
						{

							bool affected = false;

							TerrainPoint *points = (*it)->pointStart;
							int deltaIndex = 0;
							for( TerrainPoint *curr = points; curr != NULL; curr = curr->next )
							{
								TerrainPoint *prev;
								if( curr == (*it)->pointStart )
								{
									prev = (*it)->pointEnd;
								}
								else
								{
									prev = curr->prev;
								}


								if( curr->selected ) //selected
								{					
									
									
									//Vector2i temp = (*pointIt).pos + pointGrabDelta;
									
									Vector2i delta = allDeltas[allDeltaIndex][deltaIndex];
								//	cout << "allindex: " << allDeltaIndex << ", deltaIndex: " << deltaIndex << endl;
								//	cout << "moving: " << delta.x << ", " << delta.y << endl;
									curr->pos += pointGrabDelta - delta; //pointGrabDelta - ;

									if( curr->gate != NULL )
									{
										curr->gate->UpdateLine();
									}

									if( (*it)->enemies.count( curr ) > 0 )
									{
										list<ActorParams*> &enemies = (*it)->enemies[curr];
										for( list<ActorParams*>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait )
										{
											//(*ait)->UpdateGroundedSprite();
											
										}
										//revquant is the quantity from the edge's v1
										//double revQuant = 
									}

									affected = true;
								}

								++deltaIndex;
							}

							(*it)->UpdateBounds();

							if( affected )
							{
								(*it)->movingPointMode = true;

								for( map<TerrainPoint*,list<ActorParams*>>::iterator mit = (*it)->enemies.begin();
									mit != (*it)->enemies.end(); ++mit )
								{
									list<ActorParams*> &enemies = (*mit).second;//(*it)->enemies[curr];
									for( list<ActorParams*>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait )
									{
										(*ait)->UpdateGroundedSprite();
										(*ait)->SetBoundingQuad();
									}
									//revquant is the quantity from the edge's v1
									//double revQuant = 	
								}

								/*PointList temp = (*it)->points;

								(*it)->Reset();

								for( PointList::iterator tempIt = temp.begin(); tempIt != temp.end(); 
									++tempIt )
								{
									(*it)->points.push_back( (*tempIt ) );
								}
								(*it)->Finalize();
								(*it)->SetSelected( true );
								
								Vector2i newPos0;
								Vector2i newPos1;*/
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
					//Vector2i test = (pixelPos - polyGrabPos);
					//V2d blahDelta( test.x, test.y );
					polyGrabDelta = Vector2i( pPoint.x, pPoint.y ) - Vector2i( polyGrabPos.x, polyGrabPos.y );
					//polyGrabDelta.x = blahDelta.x * 1080.0/1920.0;
					//polyGrabDelta.y = blahDelta.y;
					bool moveOkay = true;
					if( polyGrabDelta.x != 0 || polyGrabDelta.y != 0 )
					{
						polyGrabPos = Vector2i( pPoint.x, pPoint.y );//Vector2i( pPoint.x, pPoint.y );
					
						//cout << "delta: " << polyGrabDelta.x << ", " << polyGrabDelta.y << endl;
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
							(*it)->Move( polyGrabDelta );
							/*PointList & points = (*it)->points;

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
							(*it)->SetSelected( true );*/
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
				
				}

				break;
			}
		case CREATE_ENEMY:
			{
				if( trackingEnemy != NULL && showPanel == NULL )
				{
					enemySprite.setOrigin( enemySprite.getLocalBounds().width / 2, enemySprite.getLocalBounds().height / 2 );
					enemySprite.setRotation( 0 );

					
					//Vector2i mouse = sf::Mouse::getPosition( *w );
					//Vector2f realmouse( mouse.x, mouse.y );
					
					Vector2f p = preScreenTex->mapPixelToCoords( pixelPos );

					//p.x *= 
					//p.y *= 1080.f / w->getSize().y;
					//cout << "p: " << p.x << ", " << p.y << endl;
					enemySprite.setPosition( p );

					enemyQuad.setOrigin( enemyQuad.getLocalBounds().width / 2, enemyQuad.getLocalBounds().height / 2 );
					enemyQuad.setRotation( 0 );
					enemyQuad.setPosition( enemySprite.getPosition() );
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
							TerrainPoint *prev = (*it)->pointEnd;
							TerrainPoint *curr = (*it)->pointStart;

							if( (*it)->ContainsPoint( Vector2f( testPoint.x, testPoint.y ) ) )
							{
								//prev is starting at 0. start normally at 1
								int edgeIndex = 0;
								double minDistance = 10000000;
								int storedIndex;
								double storedQuantity;
							
								V2d closestPoint;

								for( ; curr != NULL; curr = curr->next )
								{
									double dist = abs(
										cross( 
										V2d( testPoint.x - prev->pos.x, testPoint.y - prev->pos.y ), 
										normalize( V2d( curr->pos.x - prev->pos.x, curr->pos.y - prev->pos.y ) ) ) );
									double testQuantity =  dot( 
											V2d( testPoint.x - prev->pos.x, testPoint.y - prev->pos.y ), 
											normalize( V2d( curr->pos.x - prev->pos.x, curr->pos.y - prev->pos.y ) ) );

									V2d pr( prev->pos.x, prev->pos.y );
									V2d cu( curr->pos.x, curr->pos.y );
									V2d te( testPoint.x, testPoint.y );
									
									V2d newPoint( pr.x + (cu.x - pr.x) * (testQuantity / length( cu - pr ) ), pr.y + (cu.y - pr.y ) *
											(testQuantity / length( cu - pr ) ) );

									//int testA = dist < 100;
									//int testB = testQuantity >= 0 && testQuantity <= length( cu - pr );
									//int testC = testQuantity >= enemySprite.getLocalBounds().width / 2 && testQuantity <= length( cu - pr ) - enemySprite.getLocalBounds().width / 2;
									//int testD = length( newPoint - te ) < length( closestPoint - te );
									
									//cout << testA << " " << testB << " " << testC << " " << testD << endl;

									int hw = trackingEnemy->width / 2;
									int hh = trackingEnemy->height / 2;
									if( dist < 100 && testQuantity >= 0 && testQuantity <= length( cu - pr ) && testQuantity >= hw && testQuantity <= length( cu - pr ) - hw 
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

										enemyQuad.setOrigin( enemyQuad.getLocalBounds().width / 2, enemyQuad.getLocalBounds().height );
										enemyQuad.setRotation( enemySprite.getRotation() );
										enemyQuad.setPosition( enemySprite.getPosition() );
									}
									else
									{
										
										//cout << "dist: " << dist << ", testquant: " << testQuantity  << endl;
									}

									prev = curr;
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

		for( list<GateInfo*>::iterator it = gates.begin(); it != gates.end(); ++it )
		{
			//cout << "drawing gate" << endl;
			(*it)->Draw( preScreenTex );
		}

		
		switch( mode )
		{
		case CREATE_TERRAIN:
			{
				int progressSize = polygonInProgress->numPoints;
				if( progressSize > 0 )
				{
					Vector2i backPoint = polygonInProgress->pointEnd->pos;
			
					Color validColor = Color::Green;
					Color invalidColor = Color::Red;
					Color colorSelection;
					if( true )
					{
						colorSelection = validColor;
					}

					//if( popupPanel == NULL )
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
						for( TerrainPoint *curr = polygonInProgress->pointStart; curr != NULL; curr = curr->next )
						{
							v[i] = Vertex( Vector2f( curr->pos.x, curr->pos.y ) );
							++i;
						}
						preScreenTex->draw( v );
					}
				}
				break;
			}
		case EDIT:
			{
				if( !pasteBrushes.empty() )
				{
					for( list<TerrainBrush*>::iterator it = pasteBrushes.begin(); it != pasteBrushes.end(); ++it )
					{
						(*it)->Draw( preScreenTex );
					}
				}

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
					preScreenTex->draw( enemyQuad );
				}
				break;
			}
		case CREATE_PATROL_PATH:
			{
				if( trackingEnemy != NULL )
				{
					preScreenTex->draw( enemySprite );
					preScreenTex->draw( enemyQuad );
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
				bool okay = true;
				/*for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
				{
					Vector2i a( playerPosition.x - playerHalfWidth, playerPosition.y - playerHalfHeight );
					Vector2i b( playerPosition.x + playerHalfWidth, playerPosition.y - playerHalfHeight );
					Vector2i c( playerPosition.x + playerHalfWidth, playerPosition.y + playerHalfHeight );
					Vector2i d( playerPosition.x - playerHalfWidth, playerPosition.y + playerHalfHeight );
					if( QuadPolygonIntersect( (*it), a,b,c,d ) )
					{
						okay = false;
						break;
					}
				}*/
				
				if( okay )
				{
					playerPosition = Vector2i( worldPos.x, worldPos.y );
				}
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
					if( selectedActor->type->name == "patroller" || selectedActor->type->name == "key" )
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
			else if( selectedGate != NULL )
			{
				int gLeft = std::min( selectedGate->point0->pos.x, selectedGate->point1->pos.x );
				int gRight = std::max( selectedGate->point0->pos.x, selectedGate->point1->pos.x );
				int gTop = std::min( selectedGate->point0->pos.y, selectedGate->point1->pos.y );
				int gBot = std::max( selectedGate->point0->pos.y, selectedGate->point1->pos.y );
				sf::RectangleShape rs( sf::Vector2f( gRight - gLeft, gBot - gTop ) );
				
				rs.setOutlineColor( Color::Cyan );				
				rs.setFillColor( Color::Transparent );
				rs.setOutlineThickness( 5 );
				rs.setPosition( gLeft, gTop );
				preScreenTex->draw( rs );
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
					//enemySelectPanel->Draw( preScreenTex );
					//gs.Draw( preScreenTex );
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
					else if( menuDownStored == EditSession::EDIT && selectedGate != NULL )
					{
						textmag.setString( "EDIT\nGATE" );
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

		preScreenTex->setView( view );


		preScreenTex->display();
		const Texture &preTex = preScreenTex->getTexture();
		
		Sprite preTexSprite( preTex );
		preTexSprite.setPosition( -960, -540 );
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
		TerrainPoint *curr = polygonInProgress->pointStart;
		//polygonInProgress->points.push_back( polygonInProgress->points.back() )
		Vector2i pre = curr->pos;
		curr = curr->next;
		
		//minimum angle
		{
			if( polygonInProgress->numPoints >= 2 )
			{
				TerrainPoint *rcurr = polygonInProgress->pointEnd;
				rcurr = rcurr->prev;

				//for( ; rcurr != NULL; rcurr = rcurr->prev )
				//{
					double ff = dot( normalize( V2d( point.x, point.y ) - V2d( polygonInProgress->pointEnd->pos.x, polygonInProgress->pointEnd->pos.y ) )
					, normalize( V2d(rcurr->pos.x, rcurr->pos.y ) - V2d( polygonInProgress->pointEnd->pos.x, polygonInProgress->pointEnd->pos.y ) ) );
					if( ff > minAngle )
					{
						cout << "ff: " << ff << endl;
						return false;
					}
				//}
			}
		}

		//return true;

		//make sure I'm not too close to the very first point and that my line isn't too close to the first point either
		//if( length( V2d( point.x, point.y ) - V2d( polygonInProgress->points.front().pos.x, polygonInProgress->points.front().pos.y ) ) < 8 )
		{
			double separation = length( V2d(point.x, point.y) - V2d(pre.x, pre.y) );
			if( separation < minimumEdgeLength )
			{
				cout << "return a" << endl;
				return false;
			}

			if( polygonInProgress->numPoints > 2  )
			{
				if( abs( cross( V2d( point.x, point.y ) - V2d( prev.x, prev.y), 
					normalize( V2d( pre.x, pre.y ) - V2d( prev.x, prev.y ) ) ) ) < minimumEdgeLength
					&& dot( V2d( point.x, point.y ) - V2d( prev.x, prev.y ), normalize( V2d( pre.x, pre.y ) - V2d( prev.x, prev.y )) ) 
					>= length( V2d( pre.x, pre.y ) - V2d( prev.x, prev.y ) ) )
				{
					cout << "return b" << endl;
					return false;
				}
			}
		}

		//check for distance to point in the polygon and edge distances

		if( point.x == polygonInProgress->pointStart->pos.x && point.y == polygonInProgress->pointStart->pos.y )
		{
			pre = curr->pos;
			curr = curr->next;
		}

		{
			for( ; curr != NULL; curr = curr->next )
		{
			if( curr->pos == polygonInProgress->pointEnd->pos )
				continue;

			LineIntersection li = lineIntersection( V2d( prev.x, prev.y ), V2d( point.x, point.y ),
						V2d( pre.x, pre.y ), V2d( curr->pos.x, curr->pos.y ) );
			float tempLeft = min( pre.x, curr->pos.x ) - 0;
			float tempRight = max( pre.x, curr->pos.x ) + 0;
			float tempTop = min( pre.y, curr->pos.y ) - 0;
			float tempBottom = max( pre.y, curr->pos.y ) + 0;
			if( !li.parallel )
			{
				
				double separation = length( V2d(point.x, point.y) - V2d(curr->pos.x,curr->pos.y ) );
				
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
				Vector2i bi = curr->pos - pre;
				V2d a(ai.x, ai.y);
				V2d b(bi.x, bi.y);
				double res = abs(cross( a, normalize( b )));
				double des = dot( a, normalize( b ));

				Vector2i ci = curr->pos - prev;
				Vector2i di = point - prev;
				V2d c( ci.x, ci.y);
				V2d d( di.x, di.y );

				double res2 = abs( cross( c, normalize( d ) ) );
				double des2 = dot( c, normalize( d ) );

				//cout << "minedgelength: " << minimumEdgeLength <<  ", " << res << endl;

				if( point.x == polygonInProgress->pointStart->pos.x && point.y == polygonInProgress->pointStart->pos.y )
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
			pre = curr->pos;
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
			TerrainPoint *pcurr =  p->pointStart;
			Vector2i prevPoint = pcurr->pos;
			pcurr = pcurr->next;
			for( ; pcurr != NULL; pcurr = pcurr->next )
			{
				LineIntersection li = lineIntersection( V2d( prevPoint.x, prevPoint.y ), V2d(pcurr->pos.x, pcurr->pos.y),
					V2d( prev.x, prev.y ), V2d( point.x, point.y ) );
				float tempLeft = min( prevPoint.x, pcurr->pos.x );
				float tempRight = max( prevPoint.x, pcurr->pos.x );
				float tempTop = min( prevPoint.y, pcurr->pos.y );
				float tempBottom = max( prevPoint.y, pcurr->pos.y );
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
				prevPoint = pcurr->pos;
				
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
				//patroller->SetPath( patrolPath );
			}
			else if( mode == CREATE_ENEMY )
			{
				PatrollerParams *patroller = new PatrollerParams( this, patrolPath.front(), patrolPath, speed, loop );
				groups["--"]->actors.push_back( patroller);
				patroller->group = groups["--"];
				//trackingEnemy = NULL;

				//trackingEnemy = types[name];
				//enemySprite.setTexture( trackingEnemy->imageTexture );

				//enemySprite.setTextureRect( sf::IntRect( 0, 0, trackingEnemy->imageTexture.getSize().x, 
				//	trackingEnemy->imageTexture.getSize().y ) );

				//enemySprite.setOrigin( enemySprite.getLocalBounds().width /2 , enemySprite.getLocalBounds().height / 2 );
		
				//enemyQuad.setSize( Vector2f( trackingEnemy->width, trackingEnemy->height ) );

			
				
			}
			showPanel = NULL;
			

			//ActorParams *actor = new PatrollerParams( this, patrolPath.front(), patrolPath, speed, loop );
			
			//patrolPath.clear();
			//actor->SetAsPatroller( types["patroller"], patrolPath.front(), patrolPath, speed, loop );
			
			//mode = CREATE_ENEMY;
			//patroller path should get set only from hitting the button within it to start the path check

			//showPanel = enemySelectPanel;
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
	
	else if( p->name == "key_options" )
	{
		if( b->name == "ok" )
		{
			bool loop = p->checkBoxes["loop"]->checked;
			float speed = 1; 
			int stayFrames = 0;
			bool teleport = p->checkBoxes["teleport"]->checked;

			try
			{
				speed = boost::lexical_cast<int>( p->textBoxes["speed"]->text.getString().toAnsiString() );
			}
			catch(boost::bad_lexical_cast &)
			{
				//error
			}

			try
			{
				stayFrames = boost::lexical_cast<int>( p->textBoxes["stayframes"]->text.getString().toAnsiString() );
			}
			catch(boost::bad_lexical_cast &)
			{
				//error
			}

			//showPanel = trackingEnemy->panel;
			//PatrollerParams *patroller = (PatrollerParams*)trackingEnemy;
			if( mode == EDIT && selectedActor != NULL )
			{
				KeyParams *key = (KeyParams*)selectedActor;
				key->speed = speed;
				key->loop = loop;
				key->stayFrames = stayFrames;
				key->teleport = teleport;
				key->SetPath( patrolPath );
				
			}
			else if( mode == CREATE_ENEMY )
			{
				GridSelector * gs = p->gridSelectors["keytype"];

				//eventually can convert this between indexes or something to simplify when i have more types
				string name = gs->names[gs->selectedX][gs->selectedY];

				GateInfo::GateTypes gType;
				if( name == "red" )
				{
					gType = GateInfo::RED;
				}
				else if( name == "green" )
				{
					gType = GateInfo::GREEN;
				}
				else if( name == "blue" )
				{
					gType = GateInfo::BLUE;
				}

				
				KeyParams *key = new KeyParams( this, patrolPath.front(), patrolPath, speed, loop, stayFrames, teleport, gType );
				
				groups["--"]->actors.push_back( key );
				key->group = groups["--"];
				//trackingEnemy = NULL;
				
			}
			showPanel = NULL;
			//showPanel = enemySelectPanel;
		}
		else if( b->name == "createpath" )
		{
			showPanel = NULL;
			mode = CREATE_PATROL_PATH;
			Vector2i front = patrolPath.front();
			patrolPath.clear();
			patrolPath.push_back( front );
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
				//trackingEnemy = NULL;
				
			}
			showPanel = NULL;
			//showPanel = enemySelectPanel;
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
				//trackingEnemy = NULL;
				
			}
			showPanel = NULL;
			//showPanel = enemySelectPanel;
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
				showPanel = NULL;
			}
			showPanel = NULL;
			//showPanel = enemySelectPanel;
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
				
				p->textBoxes["minedgesize"]->text.setString( "8" );
				
				MessagePop( "minimum edge length too low.\n Set to minimum of 8" );
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
	else if( p->name == "error_popup" )
	{
		if( b->name == "ok" )
		{
			showPanel = NULL;
		}
	}
	else if( p->name == "confirmation_popup" )
	{
		if( b->name == "confirmOK" )
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

void EditSession::GridSelectorCallback( GridSelector *gs, const std::string & p_name )
{
	string name = p_name;
	Panel *panel = gs->owner;
	if( panel == enemySelectPanel )
	{
		if( name != "not set" )
		{
			//if( name == "greenkey" || name == "bluekey" )
			//{
			//	name = "key";
		//	}
			trackingEnemy = types[name];
			enemySprite.setTexture( trackingEnemy->imageTexture );

			enemySprite.setTextureRect( sf::IntRect( 0, 0, trackingEnemy->imageTexture.getSize().x, 
				trackingEnemy->imageTexture.getSize().y ) );

			enemySprite.setOrigin( enemySprite.getLocalBounds().width /2 , enemySprite.getLocalBounds().height / 2 );
		
			enemyQuad.setSize( Vector2f( trackingEnemy->width, trackingEnemy->height ) );

			
			showPanel = NULL;
			

			cout << "set your cursor as the image" << endl;
		}
		else
		{
			cout << "not set" << endl;
		}
	}
	else if( panel == gateSelectorPopup )
	{
		//cout << "callback!" << endl;
		if( name != "not set" )
		{
			//cout << "real result" << endl;
			tempGridResult = name;
			//showPanel = NULL;
		}
		else
		{
		//	cout << "not set" << endl;
		}
	}
}

void EditSession::CheckBoxCallback( CheckBox *cb, const std::string & e )
{
	cout << cb->name << " was " << e << endl;
}



int EditSession::CountSelectedPoints()
{
	int count = 0;
	for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
	{
		for( TerrainPoint *curr = (*it)->pointStart; curr != NULL; curr = curr->next )
		{
			if( curr->selected ) //selected
			{
				++count;
			}
		}
	}
	return count;
}

void EditSession::ExtendPolygon()
{
	if( polygonInProgress->numPoints > 1 )
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

bool EditSession::IsExtendPointOkay( TerrainPolygon *poly, sf::Vector2f testPoint )
{
	Vector2i worldi( testPoint.x, testPoint.y );
	assert( extendingPolygon != NULL );

	bool okay = !extendingPolygon->ContainsPoint( testPoint );

	if( okay )
	{
		TerrainPoint * okayPoint = extendingPolygon->pointStart;
		TerrainPoint *okayPrevPoint = extendingPolygon->pointEnd;
		for( ; okayPoint != NULL; okayPoint = okayPoint->next )
		{
								
			//LineIntersection li = SegmentIntersect( , worldi, (*okayPrev).pos, (*okayIt).pos );
			Vector2i a = polygonInProgress->pointEnd->pos;
			Vector2i b = worldi;
			Vector2i c = okayPrevPoint->pos;
			Vector2i d = okayPoint->pos;

								
			LineIntersection li = LimitSegmentIntersect( a,b,c,d );
			Vector2i lii( floor(li.position.x + .5), floor(li.position.y + .5) );
			//if( !li.parallel  && (abs( lii.x - currPoint.x ) >= 1 || abs( lii.y - currPoint.y ) >= 1 ))
			if( !li.parallel )//&& lii != a && lii != b && lii != c && lii != d )
			{
				okay = false;
				break;
			}
			okayPrevPoint = okayPoint;
		}
								
								
	}

	return okay;
}

bool EditSession::IsPointValid( sf::Vector2i oldPoint, sf::Vector2i point, TerrainPolygon *poly )
{
	//cout << "checking if the point is valid!!" << endl;
	//check distance from points first

	V2d p( point.x, point.y );
	//cout << "p: " << p.x << ", " << p.y << endl;
	for( TerrainPoint *curr = poly->pointStart; curr != NULL; curr = curr->next )
	{
		if( curr->pos != point )
		{
			V2d temp( curr->pos.x, curr->pos.y );
			if( length( p - temp ) < validityRadius )
			{
				cout << "false type one:" << length( p - temp ) << " .. " << temp.x << ", " << temp.y << ", p: " << p.x << ", " << p.y << endl;
				return false;
			}
		}
	}
	TerrainPoint *pcurr = poly->pointStart;
	TerrainPoint *prev = poly->pointEnd;

	for( ; pcurr != NULL; pcurr = pcurr->next )
	{
		if( pcurr->pos == oldPoint || pcurr->pos == oldPoint )
		{
			prev = pcurr;
			continue;
		}

		V2d v0 = V2d( prev->pos.x, prev->pos.y );
		V2d v1 = V2d( pcurr->pos.x, pcurr->pos.y );
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


		prev = pcurr;
	}

	/*for( list<GateInfo*>::iterator git = poly->attachedGates.begin(); git != poly->attachedGates.end(); ++git )
	{
		LineIntersection li = LimitSegmentIntersect( (*git)->v0, (*git)->v1, oldPoint, point );
		if( !li.parallel )
		{
		//	return false;
		}
	}*/


	return true;
}

bool EditSession::IsPolygonExternallyValid( TerrainPolygon &poly, TerrainPolygon *ignore )
{
	Rect<int> polyAABB( poly.left, poly.top, poly.right - poly.left, poly.bottom - poly.top );
	polyAABB.left -= minimumEdgeLength;
	polyAABB.top -= minimumEdgeLength;
	polyAABB.width += minimumEdgeLength * 2;
	polyAABB.height += minimumEdgeLength * 2;

	for( list<TerrainPolygon*>::iterator polyIt = polygons.begin(); polyIt != polygons.end(); ++polyIt )
	{
		if( ignore == (*polyIt ) )
		{
			continue;
		}
		//eventually do a quad tree for this to speed it up
		Rect<int> currAABB( (*polyIt)->left, (*polyIt)->top, (*polyIt)->right - (*polyIt)->left,
			(*polyIt)->bottom - (*polyIt)->top);
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
		for( TerrainPoint *my = poly.pointStart; my != NULL; my = my->next )
		{
			Vector2i oldPoint, currPoint;
			if( my == poly.pointStart )
			{
				oldPoint = poly.pointEnd->pos;
			}
			else
			{
				oldPoint = my->prev->pos;
			}

			currPoint = my->pos;

			if( !IsPointValid( oldPoint, currPoint, (*polyIt) ) )
			{

				cout << "a: old: " << oldPoint.x << ", " << oldPoint.y << ", curr: " << currPoint.x << ", " << currPoint.y << endl;
				return false;
			}
			//IsPointValid(
		}

		//his points vs my lines
		for( TerrainPoint *pcurr = (*polyIt)->pointStart; pcurr != NULL; pcurr = pcurr->next )
		{
			Vector2i oldPoint, currPoint;
			if( pcurr == (*polyIt)->pointStart )
			{
				oldPoint = (*polyIt)->pointEnd->pos;
			}
			else
			{
				oldPoint = pcurr->prev->pos;
			}

			currPoint = pcurr->pos;

			if( !IsPointValid( oldPoint, currPoint, &poly ) )
			{
				cout << "b: old: " << oldPoint.x << ", " << oldPoint.y << ", curr: " << currPoint.x << ", " << currPoint.y << endl;
				return false;
			}
			//IsPointValid(
		}

		//my lines vs his lines
		for( TerrainPoint *my = poly.pointStart; my != NULL; my = my->next )
		{
			TerrainPoint *myPrev;
			if( my == poly.pointStart )
			{
				myPrev = poly.pointEnd;
			}
			else
			{
				myPrev = my->prev;
			}

			for( TerrainPoint *pcurr = (*polyIt)->pointStart; pcurr != NULL; pcurr = pcurr->next )
			{
				TerrainPoint *prev;
				if( pcurr == (*polyIt)->pointStart )
				{
					prev = (*polyIt)->pointEnd;
				}
				else
				{
					prev = pcurr->prev;
				}

				LineIntersection li = SegmentIntersect( (*myPrev).pos, my->pos, (*prev).pos, pcurr->pos );
				if( !li.parallel )
				{
					return false;
				}
			}
		}



		//hes inside me w/ no intersection
		for( TerrainPoint *pcurr = (*polyIt)->pointStart; pcurr != NULL; pcurr = pcurr->next )
		//for( PointList::iterator pit = (*polyIt)->points.begin(); pit != (*polyIt)->points.end(); ++pit )
		{
			if( poly.ContainsPoint( Vector2f( pcurr->pos.x, pcurr->pos.y ) ) )
			{
				cout << "c" << endl;
				return false;
			}
		}


		//im inside him w/ no intersection
		//for( PointList::iterator myPit = poly.points.begin(); myPit != poly.points.end(); ++myPit )
		for( TerrainPoint *my = poly.pointStart; my != NULL; my = my->next )
		{
			if( (*polyIt)->ContainsPoint( Vector2f( my->pos.x, my->pos.y ) ) )
			{
				cout << "d" << endl;
				return false;
			}
		}

		//for( PointList::iterator pit = poly.points.begin(); pit != poly.points.end(); ++pit )
		for( TerrainPoint *my = poly.pointStart; my != NULL; my = my->next )
		{
			Vector2i oldPoint, currPoint;
			if( my == poly.pointStart )
			{
				oldPoint = poly.pointEnd->pos;
			}
			else
			{
				oldPoint = my->prev->pos;
			}

			currPoint = my->pos;

			/*for( list<GateInfo*>::iterator it = (*polyIt)->attachedGates.begin(); it != (*polyIt)->attachedGates.end(); ++it )
			{
				LineIntersection li = LimitSegmentIntersect( oldPoint, currPoint, (*it)->v0, (*it)->v1 );
				if( !li.parallel )
				{
					return false;
				}
			}*/
		}


		for( TerrainPoint *pcurr = (*polyIt)->pointStart; pcurr != NULL; pcurr = pcurr->next )
		//for( PointList::iterator pit = (*polyIt)->points.begin(); pit != (*polyIt)->points.end(); ++pit )
		{
			Vector2i oldPoint, currPoint;
			if( pcurr == (*polyIt)->pointStart )
			{
				oldPoint = (*polyIt)->pointEnd->pos;
			}
			else
			{
				oldPoint = pcurr->prev->pos;
			}

			currPoint = pcurr->pos;

			/*for( list<GateInfo*>::iterator it = poly.attachedGates.begin(); it != poly.attachedGates.end(); ++it )
			{
				LineIntersection li = LimitSegmentIntersect( oldPoint, currPoint, (*it)->v0, (*it)->v1 );
				if( !li.parallel )
				{
					return false;
				}
			}*/
		}

		//me touching his enemies
		for( EnemyMap::iterator it = (*polyIt)->enemies.begin(); it != (*polyIt)->enemies.end(); ++it )
		{
			for( list<ActorParams*>::iterator ait = (*it).second.begin(); ait != (*it).second.end(); ++ait )
			{
				//need to round these floats probably
				//cout << "calling this" << endl;
				sf::VertexArray &bva = (*ait)->boundingQuad;
				//V2d along = (*ait)->groundInfo->
				if( QuadPolygonIntersect( &poly, Vector2i( bva[0].position.x, bva[0].position.y ), 
					Vector2i( bva[1].position.x, bva[1].position.y ), Vector2i( bva[2].position.x, bva[2].position.y ),
						Vector2i( bva[3].position.x, bva[3].position.y ) ) )
				{
					
					//cout << "poly top: " << poly.top << endl;
					//cout << "other bottom: " << (*polyIt)->bottom << endl;

					//cout << "touched polygon!" << endl;
					return false;
				}
				else
				{
					//cout << "no collision" << endl;
				}
			}
		}

		//him touching my enemies
		for( EnemyMap::iterator it = poly.enemies.begin(); it != poly.enemies.end(); ++it )
		{
			for( list<ActorParams*>::iterator ait = (*it).second.begin(); ait != (*it).second.end(); ++ait )
			{
				//need to round these floats probably
				//cout << "calling this" << endl;
				sf::VertexArray &bva = (*ait)->boundingQuad;
				//V2d along = (*ait)->groundInfo->
				if( QuadPolygonIntersect( (*polyIt), Vector2i( bva[0].position.x, bva[0].position.y ), 
					Vector2i( bva[1].position.x, bva[1].position.y ), Vector2i( bva[2].position.x, bva[2].position.y ),
						Vector2i( bva[3].position.x, bva[3].position.y ) ) )
				{
					
					//cout << "poly top: " << poly.top << endl;
					//cout << "other bottom: " << (*polyIt)->bottom << endl;

					//cout << "touched polygon!" << endl;
					return false;
				}
				else
				{
					//cout << "no collision" << endl;
				}
			}
		}

		
		
	}

	if( PolyIntersectGate( poly ) )
		return false;



	//cout << "true" << endl;
	return true;
}

bool EditSession::IsPolygonInternallyValid( TerrainPolygon &poly )
{
	poly.AlignExtremes( PRIMARY_LIMIT );


	if( !poly.IsClockwise() )
	{
		return false;
	}

	//points close to other points on myself
	for( TerrainPoint *curr = poly.pointStart; curr != NULL; curr = curr->next )
	{
		for( TerrainPoint *curr2 = poly.pointStart; curr2 != NULL; curr2 = curr2->next )
		{
			if( curr->pos.x == curr2->pos.x && curr->pos.y == curr2->pos.y )
			{
				continue;
			}

			V2d a( curr->pos.x, curr->pos.y );
			V2d b( curr2->pos.x, curr2->pos.y );
			if( length( a - b ) < validityRadius )
			{
				//cout << "len: " << length( a - b ) << endl;
				return false;
			}
		}
	}

	//points close to lines on myself. do i need the previous stuff
	for( TerrainPoint *curr = poly.pointStart; curr != NULL; curr = curr->next )
	{
		TerrainPoint *prev, *next;
		if( curr == poly.pointStart )
		{
			prev = poly.pointEnd;
		}
		else
		{
			prev = curr->prev;
		}

		TerrainPoint *temp = curr->next;
		if( temp == NULL )
		{
			next = poly.pointStart;
		}
		else
		{
			next = curr->next;
		}

		//test for minimum angle difference between edges
		V2d pos(curr->pos.x, curr->pos.y );
		V2d prevPos( prev->pos.x, prev->pos.y );
		V2d nextPos( next->pos.x, next->pos.y );

		
		double ff = dot( normalize( prevPos - pos ), normalize( nextPos - pos ) );
		if( ff > minAngle )
		{
			//cout << "ff: " << ff << endl;
			return false;
		} 



		/*if( !IsPointValid( prev->pos, curr->pos, &poly ) )
		{
			cout << "blahzzz" << endl;
			return false;
		}*/
	}


	//line intersection on myself
	for( TerrainPoint *curr = poly.pointStart; curr != NULL; curr = curr->next )
	{
		TerrainPoint *prev;
		if( curr == poly.pointStart )
		{
			prev = poly.pointEnd;
		}
		else
		{
			prev = curr->prev;
		}

		for( TerrainPoint *curr2 = poly.pointStart; curr2 != NULL; curr2 = curr2->next )
		{
			TerrainPoint *prev2;
			
			if( curr2 == poly.pointStart )
			{
				prev2 = poly.pointEnd;
			}
			else
			{
				prev2 = curr2->prev;
			}

			if( prev2 == prev || prev2 == curr || curr2 == prev || curr2 == curr )
			{
				continue;
			}

			LineIntersection li = LimitSegmentIntersect( prev->pos, curr->pos, prev2->pos, curr2->pos );

			if( !li.parallel )
			{
				return false;
			}
		}
	}

	/*for( EnemyMap::iterator it = poly.enemies.begin(); it != poly.enemies.end(); ++it )
	{
		V2d a,b,c,d;
		TerrainPoint *curr = (*it).first;
		TerrainPoint *next;
		if( curr == poly.pointEnd )
			next = poly.pointStart;
		else
			next = curr->next;

		V2d along = normalize( V2d( next->pos.x, next->pos.y ) - V2d( curr->pos.x, curr->pos.y ) );
		V2d other( along.y, -along.x );

		for( list<ActorParams*>::iterator ait = (*it).second.begin(); ait != (*it).second.end(); ++ait )
		{
			double groundQuant = (*ait)->groundInfo->groundQuantity;
			double halfWidth = (*ait)->type->width / 2.0;
			double height = (*ait)->type->height;

			V2d groundPos = V2d( curr->pos.x, curr->pos.y ) + along * groundQuant;

			a = groundPos - along * halfWidth;
			b = a + other * height;
			d = groundPos + along * halfWidth;
			c = d + other * height;
			
			bool quadIntersect = QuadPolygonIntersect( &poly,
				Vector2i( a.x, a.y ), Vector2i( b.x, b.y ), Vector2i( c.x, c.y ), Vector2i( d.x, d.y ) );

			cout << "quadiunter: " << quadIntersect << endl;
			if( quadIntersect )
				return false;
		}

		
	}*/
	//for( std::map<std::string, ActorGroup*>::iterator it = edit->groups.begin(); it != edit->groups.end() && res2; ++it )
	for( EnemyMap::iterator it = poly.enemies.begin(); it != poly.enemies.end(); ++it )
	{
		for( list<ActorParams*>::iterator ait = (*it).second.begin(); ait != (*it).second.end(); ++ait )
		{
			//need to round these floats probably

			sf::VertexArray &bva = (*ait)->boundingQuad;
			if( QuadPolygonIntersect( &poly, Vector2i( bva[0].position.x, bva[0].position.y ), 
				Vector2i( bva[1].position.x, bva[1].position.y ), Vector2i( bva[2].position.x, bva[2].position.y ),
					Vector2i( bva[3].position.x, bva[3].position.y ) ) )
			{
				return false;
			}
			else
			{
				
			}
		}
	}


	TerrainPoint *prev;
	for( TerrainPoint *curr = poly.pointStart; curr != NULL; curr = curr->next )
	{
		if( curr->gate != NULL )
		{
			if( curr == poly.pointStart )
			{
				prev = poly.pointEnd;
			}
			else
			{
				prev = curr->prev;
			}

			Vector2i prevPos = prev->pos;
			Vector2i pos = curr->pos;

			LineIntersection li = LimitSegmentIntersect( prevPos, pos, curr->gate->point0->pos, curr->gate->point1->pos );

			if( !li.parallel )
			{
				return false;
			}
			
		}
	}
	


	return true;
}

bool EditSession::IsPolygonValid( TerrainPolygon &poly, TerrainPolygon *ignore )
{
	return IsPolygonExternallyValid( poly, ignore ) && IsPolygonInternallyValid( poly );
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

bool EditSession::ConfirmationPop( const std::string &question )
{

	confirm->labels["question"]->setString( question );

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
		pixelPos.x *= 1920 / w->getSize().x;
		pixelPos.y *= 1920 / w->getSize().x;
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
		pixelPos.x *= 1920 / w->getSize().x;
		pixelPos.y *= 1920 / w->getSize().x;
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

void EditSession::GridSelectPop( const std::string &type )
{
	int selectedIndex = -1;
	tempGridResult = "nothing";
	//messagePopup->labels["message"]->setString( message );
	bool closePopup = false;
	w->setView( v );
	
	preScreenTex->display();
	const Texture &preTex = preScreenTex->getTexture();
	Sprite preTexSprite( preTex );
	preTexSprite.setPosition( -960 / 2, -540 / 2 );
	preTexSprite.setScale( .5, .5 );	

	preScreenTex->setView( uiView );

	Vector2i pixelPos = sf::Mouse::getPosition( *w );
	pixelPos.x *= 1920 / w->getSize().x;
	pixelPos.y *= 1920 / w->getSize().y;
	Vector2f uiMouse = preScreenTex->mapPixelToCoords( pixelPos );

	gateSelectorPopup->pos.x = uiMouse.x;
	gateSelectorPopup->pos.y = uiMouse.y;

	sf::Event ev;
	while( !closePopup )
	{
		pixelPos = sf::Mouse::getPosition( *w );
		pixelPos.x *= 1920 / w->getSize().x;
		pixelPos.y *= 1920 / w->getSize().y;
		uiMouse = preScreenTex->mapPixelToCoords( pixelPos );
		w->clear();

		if( tempGridResult != "nothing" )
		{
			return;
		}

		while( w->pollEvent( ev ) )
		{
			switch( ev.type )
			{
			case Event::MouseButtonPressed:
				{
					if( ev.mouseButton.button == Mouse::Left )
					{
						gateSelectorPopup->Update( true, uiMouse.x, uiMouse.y );
						//if( uiMouse.x < messagePopup->pos.x 
						//messagePopup->Update( true, uiMouse.x, uiMouse.y );		
					}			
					break;
				}
			case Event::MouseButtonReleased:
				{
					//closePopup = true;
					if( ev.mouseButton.button == Mouse::Left )
					{
						gateSelectorPopup->Update( false, uiMouse.x, uiMouse.y );
					}
					break;
				}
			case Event::MouseWheelMoved:
				{
					break;
				}
			case Event::KeyPressed:
				{
					//closePopup = true;
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

		//messagePopup->Draw( w );
		gateSelectorPopup->Draw( w );

		w->setView( v );

		w->display();
	}

	preScreenTex->setView( view );
	w->setView( v );
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

		p->AddButton( "confirmOK", Vector2i( 50, 25 ), Vector2f( 100, 50 ), "OK" );
		p->AddButton( "cancel", Vector2i( 250, 25 ), Vector2f( 100, 50 ), "Cancel" );
		p->AddLabel( "question", Vector2i( 10, 10 ), 12, "_EMPTY\n_QUESTION_" );
		p->pos = Vector2i( 960 - p->size.x / 2, 540 - p->size.y );
		//p->AddLabel( "Cancel", Vector2i( 25, 50 ), 12, "_EMPTY_ERROR_" );
		return p;
	}
	else if( type == "gateselector" )
	{
		Panel *p = new Panel( "gate_popup", 100, 150, this );
		return p;
	}

	return NULL;
}

//-1 means you denied it, 0 means it didnt work, and 1 means it will work
int EditSession::IsRemovePointsOkay()
{
	bool terrainOkay = true;
	for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
		it != selectedPolygons.end(); ++it )
	{
		bool res = (*it)->IsRemovePointsOkayTerrain( this );
		if( !res )
		{
			terrainOkay = false;
			break;
		}
	}

	if( !terrainOkay )
	{
		return 0;
	}

	bool enemiesOkay = false;
	for( list<TerrainPolygon*>::iterator it = selectedPolygons.begin(); 
		it != selectedPolygons.end(); ++it )
	{
		int res = (*it)->IsRemovePointsOkayEnemies( this );
		if( res == 1 )
		{
			return 1;
		}
		else if( res == 0 )
		{
			return -1;
		}
	}

	return 1;
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
	if( name == "key" )
	{
		Panel *p = new Panel( "key_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 400 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );
		p->AddLabel( "loop_label", Vector2i( 20, 150 ), 20, "loop" );
		p->AddLabel( "teleport_label", Vector2i( 100, 150 ), 20, "teleport" );
		p->AddCheckBox( "loop", Vector2i( 120, 155 ) ); 
		p->AddCheckBox( "teleport", Vector2i( 180, 155 ) ); 
		p->AddTextBox( "speed", Vector2i( 20, 200 ), 100, 20, "10" );
		p->AddTextBox( "stayframes", Vector2i(130, 200 ), 100, 20, "0" );
		p->AddButton( "createpath", Vector2i( 20, 250 ), Vector2f( 100, 50 ), "Create Path" );
		GridSelector *gs = p->AddGridSelector( "keytype", Vector2i( 20, 330 ), 3, 1, 32, 32, true, true);
		gs->Set( 0, 0, sf::Sprite( types["key"]->iconTexture ), "red" );
		gs->Set( 1, 0, sf::Sprite( types["greenkey"]->iconTexture ), "green" );
		gs->Set( 2, 0, sf::Sprite( types["bluekey"]->iconTexture ), "blue" );
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

void EditSession::SetPanelDefault( ActorType *type )
{
	if( type->name == "key" )
	{

	}
}

void EditSession::SetEnemyEditPanel()
{
	//eventually set this up so that I can give the same parameters to multiple copies of the same enemy?
	//need to be able to apply paths simultaneously to multiples also
	ActorType *type = selectedActor->type;
	string name = type->name;

	Panel *p = type->panel;

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
		


		p->textBoxes["group"]->text.setString( patroller->group->name );

		p->textBoxes["speed"]->text.setString( boost::lexical_cast<string>( patroller->speed ) );
		p->checkBoxes["loop"]->checked = patroller->loop;
		patrolPath = patroller->GetGlobalPath();
		showPanel = p;
	}
	else if( name == "crawler" )
	{
		CrawlerParams *crawler = (CrawlerParams*)selectedActor;
		
		p->textBoxes["group"]->text.setString( crawler->group->name );

		p->checkBoxes["clockwise"]->checked = crawler->clockwise;
		p->textBoxes["speed"]->text.setString( boost::lexical_cast<string>( crawler->speed ) );
		//p->AddCheckBox( "clockwise", Vector2i( 120, 155 ) ); 
		//p->AddTextBox( "speed", Vector2i( 20, 200 ), 200, 20, "10" );

		
		showPanel = p;
	}
	else if( name == "basicturret" )
	{
		BasicTurretParams *basicTurret = (BasicTurretParams*)selectedActor;

		//p->AddTextBox( "bulletspeed", Vector2i( 20, 150 ), 200, 20, "10" );
		//p->AddTextBox( "waitframes", Vector2i( 20, 200 ), 200, 20, "10" );
		p->textBoxes["group"]->text.setString( basicTurret->group->name );
		p->textBoxes["bulletspeed"]->text.setString( boost::lexical_cast<string>( basicTurret->bulletSpeed ) );
		p->textBoxes["waitframes"]->text.setString( boost::lexical_cast<string>( basicTurret->framesWait ) );
		showPanel = p;
	}
	else if( name == "foottrap" )
	{
		FootTrapParams *footTrap = (FootTrapParams*)selectedActor;

		p->textBoxes["group"]->text.setString( footTrap->group->name );

		showPanel = p;
	}
	else if( name == "key" )
	{
		KeyParams *key = (KeyParams*)selectedActor;
		Panel *p = type->panel;

		/*p->AddButton( "ok", Vector2i( 100, 400 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );
		p->AddLabel( "loop_label", Vector2i( 20, 150 ), 20, "loop" );
		p->AddLabel( "teleport_label", Vector2i( 100, 150 ), 20, "teleport" );
		p->AddCheckBox( "loop", Vector2i( 120, 155 ) ); 
		p->AddCheckBox( "teleport", Vector2i( 180, 155 ) ); 
		p->AddTextBox( "speed", Vector2i( 20, 200 ), 100, 20, "10" );
		p->AddTextBox( "stayframes", Vector2i(130, 200 ), 100, 20, "0" );
		p->AddButton( "createpath", Vector2i( 20, 250 ), Vector2f( 100, 50 ), "Create Path" );
		GridSelector *gs = p->AddGridSelector( "keytype", Vector2i( 20, 330 ), 3, 1, 32, 32 );
*/
		patrolPath = key->GetGlobalPath();
		
		p->textBoxes["group"]->text.setString( key->group->name );
		p->textBoxes["speed"]->text.setString( boost::lexical_cast<string>(key->speed) );
		p->textBoxes["stayframes"]->text.setString( boost::lexical_cast<string>(key->stayFrames) );
		p->checkBoxes["loop"]->checked = key->loop;
		p->checkBoxes["teleport"]->checked = key->loop;
		
		GridSelector &gs = *p->gridSelectors["keytype"];
		gs.selectedY = 0;
		switch( key->gateType )
		{
		case GateInfo::RED:
			gs.selectedX = 0;
			break;
		case GateInfo::GREEN:
			gs.selectedX = 1;
			break;
		case GateInfo::BLUE:
			gs.selectedX = 2;
			break;
		}

		showPanel = p;
	}
}

bool EditSession::CanCreateGate( GateInfo &testGate )
{
	Vector2i v0 = testGate.point0->pos;
	Vector2i v1 = testGate.point1->pos;

	//no duplicate points
	for( list<GateInfo*>::iterator it = gates.begin(); it != gates.end(); ++it )
	{
		if( v0 == (*it)->point0->pos || v0 == (*it)->point1->pos || v1 == (*it)->point0->pos || v1 == (*it)->point1->pos )
		{
			return false;
		}
	}

	if( testGate.poly0 == testGate.poly1 )
	{
		if( testGate.vertexIndex0 + 1 == testGate.vertexIndex1 
			|| testGate.vertexIndex0 - 1 == testGate.vertexIndex1 
			|| testGate.vertexIndex0 == 0 && testGate.vertexIndex1 == testGate.poly1->numPoints
			|| testGate.vertexIndex1 == 0 && testGate.vertexIndex0 == testGate.poly1->numPoints )
		{
			return false;
		}
	}
	
	//get aabb, check intersection with polygons. check line intersections with those polygons

	int left = min( v0.x, v1.x );
	int right = max( v0.x, v1.x );
	int top = min( v0.y, v1.y );
	int bot = max( v0.y, v1.y );

	for( list<TerrainPolygon*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		//aabb collide
		if( left <= (*it)->right && right >= (*it)->left && top <= (*it)->bottom && bot >= (*it)->top )
		{
			TerrainPoint *prev;
			for( TerrainPoint *pcurr = (*it)->pointStart; pcurr != NULL; pcurr = pcurr->next )
			{
				if( pcurr == (*it)->pointStart )
				{
					prev = (*it)->pointEnd;
				}
				else
				{
					prev = pcurr->prev;
				}

				Vector2i prevPos = prev->pos;
				Vector2i pos = pcurr->pos;

				LineIntersection li = LimitSegmentIntersect( prevPos, pos, v0, v1 );

				if( !li.parallel )
				{
					return false;
				}
			}
		}
	}

	return true;
}

void EditSession::ClearCopyBrushes()
{
	for( list<TerrainBrush*>::iterator it = copyBrushes.begin(); it != copyBrushes.end();
		++it )
	{
		delete (*it);
	}
	copyBrushes.clear();
}

void EditSession::ClearPasteBrushes()
{
	for( list<TerrainBrush*>::iterator it = pasteBrushes.begin(); it != pasteBrushes.end();
		++it )
	{
		delete (*it);
	}
	pasteBrushes.clear();
}

void EditSession::CopyToPasteBrushes()
{
	for( list<TerrainBrush*>::iterator it = copyBrushes.begin(); it != copyBrushes.end();
		++it )
	{
		TerrainBrush* tb  = new TerrainBrush( *(*it) );
		pasteBrushes.push_back( tb );
	}
}

bool EditSession::PolyIntersectGate( TerrainPolygon &poly )
{
	//can be optimized with bounding box checks.
	for( list<GateInfo*>::iterator it = gates.begin(); it != gates.end(); ++it )
	{
		Vector2i point0 = (*it)->point0->pos;
		Vector2i point1 = (*it)->point1->pos;

		for( TerrainPoint *my = poly.pointStart; my != NULL; my = my->next )
		{
			TerrainPoint *prev;
			if( my == poly.pointStart )
				prev = poly.pointEnd;
			else
			{
				prev = my->prev;
			}

			LineIntersection li = LimitSegmentIntersect( point0, point1, prev->pos, my->pos );
			if( !li.parallel )
			{
				return true;
			}
		}	
	}

	return false;
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
	Init();
}

void ActorType::Init()
{
	if( name == "patroller" )
	{
		width = 0;
		height = 0;
		canBeGrounded = false;
		canBeAerial = true;
	}
	else if( name == "foottrap" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
	}
	else if( name == "basicturret" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
	}
	else if( name == "goal" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
	}
	else if( name == "crawler" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
	}
	else if( name == "key" )
	{
		width = 0;
		height = 0;
		canBeGrounded = false;
		canBeAerial = true;
	}

}

ActorParams::ActorParams()
	:boundingQuad( sf::Quads, 4 ) //, ground( NULL ), groundQuantity( 420.69 ), 
{
	groundInfo = NULL;

	for( int i = 0; i < 4; ++i )
		boundingQuad[i].color = Color( 0, 255, 0, 100);
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

	if( type->canBeGrounded && type->canBeAerial )
	{
		if( groundInfo != NULL )
		{
			int edgeIndex = groundInfo->GetEdgeIndex();
			of << "-air" << " " << groundInfo->ground->writeIndex << " " << edgeIndex << " " << groundInfo->groundQuantity << endl;
		}
		else
		{
			of << "+air" << " " << position.x << " " << position.y << endl;
		}
	}
	else if( type->canBeGrounded )
	{
		assert( groundInfo != NULL );

		int edgeIndex = groundInfo->GetEdgeIndex();

		of << groundInfo->ground->writeIndex << " " << edgeIndex << " " << groundInfo->groundQuantity << endl;
	}
	else if( type->canBeAerial )
	{
		of << position.x << " " << position.y << endl;
	}
	else
	{
		assert( false );
	}
	

	/*for( list<string>::iterator it = params.begin(); it != params.end(); ++it )
	{
		of << (*it) << endl;
	}*/
	WriteParamFile( of );
}

void ActorParams::DrawQuad( sf::RenderTarget *target )
{
	target->draw( boundingQuad );
}

void ActorGroup::Draw( sf::RenderTarget *target )
{
	for( list<ActorParams*>::iterator it = actors.begin(); it != actors.end(); ++it )
	{
		(*it)->Draw( target );
		(*it)->DrawQuad( target );
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
	:pos( p ), selected( s ), gate( NULL )
{
}

int ActorParams::GroundInfo::GetEdgeIndex()
{
	int index = 0;
	for( TerrainPoint *curr = ground->pointStart; curr != NULL; curr = curr->next )
	{
		if( curr == edgeStart )
			return index;
		++index;
	}

	
	//assert( false && "could not find correct edge index" );
	return -1;
}

void ActorParams::SetBoundingQuad()
{
	//float note
	if( type->canBeGrounded && groundInfo != NULL )
	{
		V2d v0( (*groundInfo->edgeStart).pos.x, (*groundInfo->edgeStart).pos.y );
		TerrainPoint *edgeEnd = groundInfo->edgeStart->next;
		if( edgeEnd == NULL )
			edgeEnd = groundInfo->ground->pointStart;
		V2d v1( edgeEnd->pos.x, edgeEnd->pos.y );
		V2d along = normalize( v1 - v0 );
		V2d other( along.y, -along.x );

		V2d startGround = v0 + along * groundInfo->groundQuantity;
		V2d leftGround = startGround - along * ( type->width / 2.0) + other * 1.0;
		V2d rightGround = startGround + along * ( type->width / 2.0) + other * 1.0;
		V2d leftAir = leftGround + other * (double)(type->height - 1) ;
		V2d rightAir = rightGround + other * (double)(type->height - 1 );

		boundingQuad[0].position = Vector2f( leftGround.x, leftGround.y );
		boundingQuad[1].position = Vector2f( leftAir.x, leftAir.y );
		boundingQuad[2].position = Vector2f( rightAir.x, rightAir.y );
		boundingQuad[3].position = Vector2f( rightGround.x, rightGround.y );
	}
	else
	{
		//patroller doesnt need a box because its not physical with the environment
		boundingQuad[0].position = Vector2f( position.x - type->width / 2, position.y - type->height / 2);
		boundingQuad[1].position = Vector2f( position.x + type->width / 2, position.y - type->height / 2);
		boundingQuad[2].position = Vector2f( position.x + type->width / 2, position.y + type->height / 2);
		boundingQuad[3].position = Vector2f( position.x - type->width / 2, position.y + type->height / 2);
	}
}

void ActorParams::UpdateGroundedSprite()
{	
	assert( groundInfo != NULL && groundInfo->ground != NULL );
	
	TerrainPoint *edge = groundInfo->edgeStart;
	TerrainPoint *next = edge->next;
	if( next == NULL )
		next = groundInfo->ground->pointStart;

	

	V2d pr( edge->pos.x, edge->pos.y );
	V2d cu( next->pos.x, next->pos.y );


	//this shouldn't remain here. i need more detailed checking.
	double groundLength = length( pr - cu );
	if( groundInfo->groundQuantity + type->width / 2 > groundLength )
	{
		groundInfo->groundQuantity = groundLength - type->width / 2;
	}
	else if( groundInfo->groundQuantity - type->width / 2 < 0 )
	{
		groundInfo->groundQuantity = type->width / 2;
	}

	V2d newPoint( pr.x + (cu.x - pr.x) * (groundInfo->groundQuantity / length( cu - pr ) ), pr.y + (cu.y - pr.y ) *
									(groundInfo->groundQuantity / length( cu - pr ) ) );

	double angle = atan2( (cu - pr).y, (cu - pr).x ) / PI * 180;

	image.setPosition( newPoint.x, newPoint.y );
	image.setRotation( angle );
}

void ActorParams::AnchorToGround( TerrainPolygon *poly, int edgeIndex, double quantity )
{
	if( groundInfo != NULL )
	{
		delete groundInfo;
		groundInfo = NULL;
	}

	groundInfo = new GroundInfo;
	
	groundInfo->ground = poly;
	

	//groundInfo->edgeIndex = eIndex;
	groundInfo->groundQuantity = quantity;

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height );
	
	int testIndex = 0;

	Vector2i point;

	TerrainPoint *prev = groundInfo->ground->pointEnd;
	TerrainPoint *curr = groundInfo->ground->pointStart;

	for( ; curr != NULL; curr = curr->next )
	{
		if( edgeIndex == testIndex )
		{
			V2d pr( prev->pos.x, prev->pos.y );
			V2d cu( curr->pos.x, curr->pos.y );

			V2d newPoint( pr.x + (cu.x - pr.x) * (groundInfo->groundQuantity / length( cu - pr ) ), pr.y + (cu.y - pr.y ) *
											(groundInfo->groundQuantity / length( cu - pr ) ) );

			double angle = atan2( (cu - pr).y, (cu - pr).x ) / PI * 180;

			groundInfo->edgeStart = prev;

			
			UpdateGroundedSprite();
			SetBoundingQuad();
			//groundInfo->edgeEnd = curr;

			break;
		}
		prev = curr;
		++testIndex;
	}

	poly->enemies[groundInfo->edgeStart].push_back( this );

	poly->UpdateBounds();

	//adjust for ordery
	/*if( groundInfo->edgeIndex == 0 )
		groundInfo->edgeIndex = groundInfo->ground->numPoints - 1;
	else
		groundInfo->edgeIndex--;*/
}

KeyParams::KeyParams( EditSession *edit, sf::Vector2i pos, list<Vector2i> &globalPath, float p_speed, bool p_loop,
					 int p_stayFrames, bool p_teleport, GateInfo::GateTypes gType )
{	
	lines = NULL;
	position = pos;	
	gateType = gType;

	type = edit->types["key"];

	if( gateType == GateInfo::RED )
	{
		image.setTexture( type->imageTexture );
	}
	else if( gateType == GateInfo::GREEN )
	{
		image.setTexture( edit->types["greenkey"]->imageTexture );
	}
	else if( gateType == GateInfo::BLUE )
	{
		image.setTexture( edit->types["bluekey"]->imageTexture );
	}
	

	
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	SetPath( globalPath );

	loop = p_loop;
	speed = p_speed;
	stayFrames = p_stayFrames;
	teleport = p_teleport;


	SetBoundingQuad();
}

void KeyParams::SetPath(std::list<sf::Vector2i> &globalPath)
{
	if( lines != NULL )
	{
		delete lines;
		lines = NULL;
	}

	localPath.clear();
	if( globalPath.size() > 1 )
	{
		int numLines = globalPath.size();
	
		lines = new VertexArray( sf::LinesStrip, numLines );
		VertexArray &li = *lines;
		li[0].position = Vector2f( 0, 0 );
		li[0].color = Color::Magenta;

		int index = 1;
		list<Vector2i>::iterator it = globalPath.begin();
		++it;
		for( ; it != globalPath.end(); ++it )
		{
			Vector2i temp( (*it).x - position.x, (*it).y - position.y );
			localPath.push_back( temp );

			li[index].position = Vector2f( temp.x, temp.y );
			li[index].color = Color::Magenta;
			++index;
		}
	}
}

void KeyParams::Draw( sf::RenderTarget *target )
{
	int localPathSize = localPath.size();

	if( localPathSize > 0 )
	{
		VertexArray &li = *lines;
	
	
			for( int i = 0; i < localPathSize+1; ++i )
			{
				li[i].position += Vector2f( position.x, position.y );
			}
	
	
		target->draw( li );

	

		if( loop )
		{

			//draw the line between the first and last
			sf::Vertex vertices[2] =
			{
				sf::Vertex(li[localPathSize].position, Color::Magenta),
				sf::Vertex(li[0].position, Color::White )
			};

			target->draw(vertices, 2, sf::Lines);
		}

	
		for( int i = 0; i < localPathSize+1; ++i )
		{
			li[i].position -= Vector2f( position.x, position.y );
		}
	}

	target->draw( image );
}

std::list<sf::Vector2i> KeyParams::GetGlobalPath()
{
	list<Vector2i> globalPath;
	globalPath.push_back( position );
	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		globalPath.push_back( position + (*it) );
	}
	return globalPath;
}

void KeyParams::WriteParamFile( ofstream &of )
{
	of << localPath.size() << endl;

	for( list<Vector2i>::iterator it = localPath.begin(); it != localPath.end(); ++it )
	{
		of << (*it).x  << " " << (*it).y << endl;
	}

	of << (int)gateType << endl;

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

	of << stayFrames << endl;
	
	if( teleport )
	{
		of << "+tele" << endl;
	}
	else
	{
		of << "-tele" << endl;
	}
}

PatrollerParams::PatrollerParams( EditSession *edit, sf::Vector2i pos, list<Vector2i> &globalPath, float p_speed, bool p_loop )
{	
	lines = NULL;
	position = pos;	
	type = edit->types["patroller"];

	image.setTexture( type->imageTexture );
	image.setOrigin( image.getLocalBounds().width / 2, image.getLocalBounds().height / 2 );
	image.setPosition( pos.x, pos.y );

	//list<Vector2i> localPath;
	SetPath( globalPath );

	loop = p_loop;
	speed = p_speed;

	SetBoundingQuad();
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
	if( lines != NULL )
	{
		delete lines;
		lines = NULL;
	}
	
	
	

	localPath.clear();
	if( globalPath.size() > 1 )
	{

		int numLines = globalPath.size();
	
		lines = new VertexArray( sf::LinesStrip, numLines );
		VertexArray &li = *lines;
		li[0].position = Vector2f( 0, 0 );
		li[0].color = Color::Magenta;

		int index = 1;
		list<Vector2i>::iterator it = globalPath.begin();
		++it;
		for( ; it != globalPath.end(); ++it )
		{
			
			Vector2i temp( (*it).x - position.x, (*it).y - position.y );
			localPath.push_back( temp );

			//cout << "temp: " << index << ", " << temp.x << ", " << temp.y << endl;
			li[index].position = Vector2f( temp.x, temp.y );
			li[index].color = Color::Magenta;
			++index;
		}
	}

	
	
}

void PatrollerParams::Draw( sf::RenderTarget *target )
{
	int localPathSize = localPath.size();

	if( localPathSize > 0 )
	{
		VertexArray &li = *lines;
	
	
			for( int i = 0; i < localPathSize+1; ++i )
			{
				li[i].position += Vector2f( position.x, position.y );
			}
	
	
		target->draw( li );

	

		if( loop )
		{

			//draw the line between the first and last
			sf::Vertex vertices[2] =
			{
				sf::Vertex(li[localPathSize].position, Color::Magenta),
				sf::Vertex(li[0].position, Color::White )
			};

			target->draw(vertices, 2, sf::Lines);
		}

	
		for( int i = 0; i < localPathSize+1; ++i )
		{
			li[i].position -= Vector2f( position.x, position.y );
		}
	}

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

	SetBoundingQuad();
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

	SetBoundingQuad();
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

	SetBoundingQuad();
}

void FootTrapParams::WriteParamFile( ofstream &of )
{
}

GoalParams::GoalParams( EditSession *edit, TerrainPolygon *p_edgePolygon, int p_edgeIndex, double p_edgeQuantity )
{
	type = edit->types["goal"];
	AnchorToGround( p_edgePolygon, p_edgeIndex, p_edgeQuantity );

	SetBoundingQuad();
}

void GoalParams::WriteParamFile( ofstream &of )
{
}