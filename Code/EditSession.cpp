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
#include "Action.h"
#include <set>

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
double EditSession::zoomMultiple = 1;
EditSession * TerrainPolygon::session = NULL;


TerrainBrush::TerrainBrush( PolyPtr poly )
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



//--ISELECTABLE FUNCTIONS END--//



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
	reformBehindYou = false;
	thickLine[0].color = Color( 255, 0, 0, 255 );
	thickLine[1].color = Color( 255, 0, 0, 255 );
	thickLine[2].color = Color( 255, 0, 0, 255 );
	thickLine[3].color = Color( 255, 0, 0, 255 );
}

void GateInfo::SetType( const std::string &gType )
{
	if( gType == "grey" )
	{
		type = GateTypes::GREY;
	}
	else if( gType == "black" )
	{
		type = GateTypes::BLACK;
	}
	else if( gType == "blue" )
	{
		type = GateTypes::BLUE;
	}
	else if( gType == "green" )
	{
		type = GateTypes::GREEN;
	}
	else if( gType == "red" )
	{
		type = GateTypes::RED;
	}
	else if( gType == "critical" )
	{
		cout << "set type critical" << endl;
		type = GateTypes::CRITICAL;
	}
	else
	{
		assert( false );
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

	of << (int)type << " " << poly0->writeIndex << " " 
		<< index0 << " " << poly1->writeIndex << " " << index1 << " ";

	if( reformBehindYou )
	{
		of << "+reform" << endl;
	}
	else
	{
		of << "-reform" << endl;
	}
		//endl;
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
	if( type == GateTypes::GREY )
	{
		c = Color( 150, 150, 150 );
	}
	else if( type == GateTypes::BLACK )
	{
		c = Color( 50, 50, 50 );
	}
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
	else if( type == GateTypes::CRITICAL )
	{
		c = Color( 255, 255, 0 );
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
	if( type == GateTypes::GREY )
	{
		c = Color( 150, 150, 150 );
	}
	else if( type == GateTypes::BLACK )
	{
		c = Color( 50, 50, 50 );
	}
	else if( type == GateTypes::RED)
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
	else if( type == GateTypes::CRITICAL )
	{
		c = Color( 255, 255, 0 );
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
	:w( wi ), fullBounds( sf::Quads, 16 )
{
	for( int i = 0; i < 16; ++i )
	{
		fullBounds[i].color = COLOR_ORANGE;
		fullBounds[i].position = Vector2f( 0, 0 );
	}

	grabbedObject = NULL;
	zoomMultiple = 1;
	editMouseDownBox = false;
	editMouseDownMove = false;
	editMoveThresh = 5;
	editStartMove = false;
	Action::session = this;
	Brush::session = this;
	ActorParams::session = this;
	TerrainPolygon::session = this;
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
	progressBrush = new Brush();
	selectedBrush = new Brush();
	enemyQuad.setFillColor( Color( 0, 255, 0, 100 ) );
	moveActive = false;
	extendingPolygon = NULL;

	ActorGroup *playerGroup = new ActorGroup( "player" );
	groups["player"] = playerGroup;
	
	playerType = new ActorType( "player", NULL );
	types["player"] = playerType;

	player.reset( new PlayerParams( this, Vector2i( 0, 0 ) ) );
	groups["player"]->actors.push_back( player );
	
}

EditSession::~EditSession()
{
	//delete polygonInProgress;
	polygonInProgress.reset();
	for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		(*it).reset();
	}
	delete progressBrush;
}

void EditSession::Draw()
{
	
	for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
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

void EditSession::UpdateFullBounds()
{
	int boundRectWidth = 5 * zoomMultiple;
		//top rect
	fullBounds[0].position = Vector2f( leftBound, topBound - boundRectWidth );
	fullBounds[1].position = Vector2f( leftBound + boundWidth, topBound - boundRectWidth );
	fullBounds[2].position = Vector2f( leftBound + boundWidth, topBound + boundRectWidth );
	fullBounds[3].position = Vector2f( leftBound, topBound + boundRectWidth );

	//right rect
	fullBounds[4].position = Vector2f( ( leftBound + boundWidth ) - boundRectWidth, topBound );
	fullBounds[5].position = Vector2f( ( leftBound + boundWidth ) + boundRectWidth, topBound );
	fullBounds[6].position = Vector2f( ( leftBound + boundWidth ) + boundRectWidth, topBound + boundHeight );
	fullBounds[7].position = Vector2f( ( leftBound + boundWidth ) - boundRectWidth, topBound + boundHeight );

	//bottom rect
	fullBounds[8].position = Vector2f( leftBound, ( topBound + boundHeight ) - boundRectWidth );
	fullBounds[9].position = Vector2f( leftBound + boundWidth, ( topBound + boundHeight ) - boundRectWidth );
	fullBounds[10].position = Vector2f( leftBound + boundWidth, ( topBound + boundHeight ) + boundRectWidth );
	fullBounds[11].position = Vector2f( leftBound, ( topBound + boundHeight ) + boundRectWidth );

	//left rect
	fullBounds[12].position = Vector2f( leftBound - boundRectWidth, topBound );
	fullBounds[13].position = Vector2f( leftBound + boundRectWidth, topBound );
	fullBounds[14].position = Vector2f( leftBound + boundRectWidth, topBound + boundHeight );
	fullBounds[15].position = Vector2f( leftBound - boundRectWidth, topBound + boundHeight );
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
		is >> leftBound;
		is >> topBound;
		is >> boundWidth;
		is >> boundHeight;

		
		UpdateFullBounds();
		


		int numPoints;
		is >> numPoints;
		is >> player->position.x;
		is >> player->position.y;

		int goalPosX;
		int goalPosY; //discard these
		is >> goalPosX;
		is >> goalPosY;

		player->image.setPosition( player->position.x, player->position.y );
		player->SetBoundingQuad();

		while( numPoints > 0 )
		{
			PolyPtr poly(  new TerrainPolygon( &grassTex ) );
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
			PolyPtr poly( new TerrainPolygon( &grassTex ) );
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


		int bgPlatformNum0;
		is >> bgPlatformNum0;
		for( int i = 0; i < bgPlatformNum0; ++i )
		{
			PolyPtr poly( new TerrainPolygon( &grassTex ) );
			//poly->layer = 1;
			polygons.push_back( poly );
			is >> poly->material;

			int polyPoints;
			is >> polyPoints;
			
			for( int i = 0; i < polyPoints; ++i )
			{
				int x,y, special;
				is >> x;
				is >> y;
				poly->AddPoint( new TerrainPoint(  Vector2i(x,y), false ) );
			}

			poly->Finalize();
			poly->SetLayer( 1 );
			//no grass for now
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

				//ActorParams *a; //= new ActorParams;
				ActorPtr a;
				


				ActorType *at;
				cout << "typename: " << typeName << endl;
				if( types.count( typeName ) == 0 )
				{
					cout << "TYPENAME: " << typeName << endl;
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
					PolyPtr terrain( NULL );
					for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
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

					a.reset( new GoalParams( this, terrain.get(), edgeIndex, edgeQuantity ) );
					terrain->enemies[a->groundInfo->edgeStart].push_back( a );
					terrain->UpdateBounds();
					//a->SetAsGoal( terrain, edgeIndex, edgeQuantity );
				}
				else if( typeName == "patroller" )
				{
					Vector2i pos;

					//always air
					is >> pos.x;
					is >> pos.y;

					int mType;
					is >> mType;

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
					a.reset( new PatrollerParams( this, pos, globalPath, speed, loop ) );
					a->monitorType = (ActorParams::MonitorType)mType;
					
				}
				else if( typeName == "bat" )
				{
					Vector2i pos;

					//always air
					is >> pos.x;
					is >> pos.y;

					int mType;
					is >> mType;

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
					a.reset( new BatParams( this, pos, globalPath, speed, loop ) );
					a->monitorType = (ActorParams::MonitorType)mType;
					
				}
				else if( typeName == "healthfly" )
				{
					Vector2i pos;

					//always air
					is >> pos.x;
					is >> pos.y;

					int mType;
					is >> mType;

					int color;
					is >> color;

					//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
					a.reset( new HealthFlyParams( this, pos, color ) );
					a->monitorType = (ActorParams::MonitorType)mType;
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
					a.reset( new KeyParams( this, pos, globalPath, speed, loop, stayFrames, teleport, (GateInfo::GateTypes)gateType ) );
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

					int mType;
					is >> mType;

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
					PolyPtr terrain( NULL );
					for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
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
					a.reset( new CrawlerParams( this, terrain.get(), edgeIndex, edgeQuantity, clockwise, speed ) ); 
					a->monitorType = (ActorParams::MonitorType)mType;
					terrain->enemies[a->groundInfo->edgeStart].push_back( a );
					terrain->UpdateBounds();
				}
				else if( typeName == "stagbeetle" )
				{

					//always grounded

					int terrainIndex;
					is >> terrainIndex;

					int edgeIndex;
					is >> edgeIndex;

					double edgeQuantity;
					is >> edgeQuantity;

					int mType;
					is >> mType;

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
					PolyPtr terrain( NULL );
					for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
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
					a.reset( new StagBeetleParams( this, terrain.get(), edgeIndex, edgeQuantity, clockwise, speed ) ); 
					a->monitorType = (ActorParams::MonitorType)mType;
					terrain->enemies[a->groundInfo->edgeStart].push_back( a );
					terrain->UpdateBounds();
				}
				else if( typeName == "poisonfrog" )
				{

					//always grounded

					int terrainIndex;
					is >> terrainIndex;

					int edgeIndex;
					is >> edgeIndex;

					double edgeQuantity;
					is >> edgeQuantity;

					int mType;
					is >> mType;

					int gravFactor;
					is >> gravFactor;

					int xStrength;
					is >> xStrength;

					int yStrength;
					is >> yStrength;

					int jumpWaitFrames;
					is >> jumpWaitFrames;

					int testIndex = 0;
					PolyPtr terrain( NULL );
					for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
					{
						if( testIndex == terrainIndex )
						{
							terrain = (*it);
							break;
						}
						testIndex++;
					}

					if( terrain == NULL )
						assert( 0 && "failure terrain indexing poison frog" );

					if( edgeIndex == terrain->numPoints - 1 )
						edgeIndex = 0;
					else
						edgeIndex++;

					//a->SetAsCrawler( at, terrain, edgeIndex, edgeQuantity, clockwise, speed ); 
					a.reset( new PoisonFrogParams( this, terrain.get(), edgeIndex, edgeQuantity, gravFactor,
						Vector2i( xStrength, yStrength ), jumpWaitFrames ) );//, clockwise, speed ) ); 
					a->monitorType = (ActorParams::MonitorType)mType;
					terrain->enemies[a->groundInfo->edgeStart].push_back( a );
					terrain->UpdateBounds();
				}
				else if( typeName == "crawlerreverser" )
				{
					//always grounded
					int terrainIndex;
					is >> terrainIndex;

					int edgeIndex;
					is >> edgeIndex;

					double edgeQuantity;
					is >> edgeQuantity;

					int testIndex = 0;
					PolyPtr terrain( NULL );
					for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
					{
						if( testIndex == terrainIndex )
						{
							terrain = (*it);
							break;
						}
						testIndex++;
					}

					if( terrain == NULL )
						assert( 0 && "failure terrain indexing crawler_reverser" );

					if( edgeIndex == terrain->numPoints - 1 )
						edgeIndex = 0;
					else
						edgeIndex++;

					//a->SetAsFootTrap( at, terrain, edgeIndex, edgeQuantity );
					a.reset( new CrawlerReverserParams( this, terrain.get(), edgeIndex, edgeQuantity ) );
					terrain->enemies[a->groundInfo->edgeStart].push_back( a );
					terrain->UpdateBounds();
				}
				else if( typeName == "bosscrawler" )
				{
					//always grounded
					int terrainIndex;
					is >> terrainIndex;

					int edgeIndex;
					is >> edgeIndex;

					double edgeQuantity;
					is >> edgeQuantity;

					int testIndex = 0;
					PolyPtr terrain( NULL );
					for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
					{
						if( testIndex == terrainIndex )
						{
							terrain = (*it);
							break;
						}
						testIndex++;
					}

					if( terrain == NULL )
						assert( 0 && "failure terrain indexing bosscrawler" );

					if( edgeIndex == terrain->numPoints - 1 )
						edgeIndex = 0;
					else
						edgeIndex++;

					//a->SetAsFootTrap( at, terrain, edgeIndex, edgeQuantity );
					a.reset( new BossCrawlerParams( this, terrain.get(), edgeIndex, edgeQuantity ) );
					terrain->enemies[a->groundInfo->edgeStart].push_back( a );
					terrain->UpdateBounds();
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
					
					int mType;
					is >> mType;

					double bulletSpeed;
					is >> bulletSpeed;

					int framesWait;
					is >> framesWait;

					int testIndex = 0;
					PolyPtr terrain( NULL );
					for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
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
					a.reset( new BasicTurretParams( this, terrain.get(), edgeIndex, edgeQuantity, bulletSpeed, framesWait ) );
					a->monitorType = (ActorParams::MonitorType)mType;
					terrain->enemies[a->groundInfo->edgeStart].push_back( a );
					terrain->UpdateBounds();
				}
				else if( typeName == "curveturret" )
				{
					//always grounded

					int terrainIndex;
					is >> terrainIndex;

					int edgeIndex;
					is >> edgeIndex;

					double edgeQuantity;
					is >> edgeQuantity;
					
					int mType;
					is >> mType;

					double bulletSpeed;
					is >> bulletSpeed;

					int framesWait;
					is >> framesWait;

					int xGravFactor;
					is >> xGravFactor;

					int yGravFactor;
					is >> yGravFactor;

					bool relative = false;
					string relativeGravStr;
					is >> relativeGravStr;
					if( relativeGravStr == "+relative" )
					{
						relative = true;
					}

					int testIndex = 0;
					PolyPtr terrain( NULL );
					for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
					{
						if( testIndex == terrainIndex )
						{
							terrain = (*it);
							break;
						}
						testIndex++;
					}

					if( terrain == NULL )
						assert( 0 && "failure terrain indexing curveturret" );

					if( edgeIndex == terrain->numPoints - 1 )
						edgeIndex = 0;
					else
						edgeIndex++;

					//a->SetAsBasicTurret( at, terrain, edgeIndex, edgeQuantity, bulletSpeed, framesWait );
					a.reset( new CurveTurretParams( this, terrain.get(), edgeIndex, edgeQuantity, bulletSpeed, framesWait,
						Vector2i( xGravFactor, yGravFactor ), relative ) );
					a->monitorType = (ActorParams::MonitorType)mType;
					terrain->enemies[a->groundInfo->edgeStart].push_back( a );
					terrain->UpdateBounds();
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

					int mType;
					is >> mType;

					int testIndex = 0;
					PolyPtr terrain( NULL );
					for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
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
					a.reset( new FootTrapParams( this, terrain.get(), edgeIndex, edgeQuantity ) );
					a->monitorType = (ActorParams::MonitorType)mType;
					terrain->enemies[a->groundInfo->edgeStart].push_back( a );
					terrain->UpdateBounds();
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

			string reformBehindYouStr;

			is >> gType;
			is >> poly0Index;
			is >> vertexIndex0;
			is >> poly1Index;
			is >> vertexIndex1;
			is >> reformBehindYouStr;
			bool reformBehindYou;
			if( reformBehindYouStr == "+reform" )
			{
				reformBehindYou = true;
			}
			else if( reformBehindYouStr == "-reform" )
			{
				reformBehindYou = false;
			}
			else
			{
				assert( false );
			}

			int testIndex = 0;
			PolyPtr terrain0(  NULL );
			PolyPtr terrain1( NULL );
			bool first = true;
			for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
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

			//PolyPtr poly(  new TerrainPolygon( &grassTex ) );
			GateInfoPtr gi( new GateInfo );
			//GateInfo *gi = new GateInfo;
			gi->reformBehindYou = reformBehindYou;
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
		for( list<ActorPtr>::iterator it2 = group->actors.begin(); it2 != group->actors.end(); ++it2 )
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

	of << leftBound << " " << topBound << " " << boundWidth << " " << boundHeight << endl;


	int pointCount = 0;
	int movingPlatCount = 0;
	int bgPlatCount0 = 0;
	for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		//if( (*it)->path.size() == 0 )
		if( (*it)->path.size() < 2 )
		{
			if( (*it)->layer == 0 )
			{
				pointCount += (*it)->numPoints;
			}
			else if( (*it)->layer == 1 )
			{
				bgPlatCount0++;
			}
			
		}
		else
			movingPlatCount++;
	}

	

	of << pointCount << endl;
	of << player->position.x << " " << player->position.y << endl;

	bool quitLoop = false;
	for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end() && !quitLoop; ++it )
	{
		ActorGroup *ag = (*it).second;
		for( list<ActorPtr>::iterator ait = ag->actors.begin(); ait != ag->actors.end() && !quitLoop; ++ait )
		{
			if( (*ait)->type->name == "goal" )
			{
				TerrainPoint *start = (*ait)->groundInfo->edgeStart;
				TerrainPoint *end = NULL;
				if( start->next != NULL )
					end = start->next;
				else
				{
					end = (*ait)->groundInfo->ground->pointStart;
				}
				V2d s( start->pos.x, start->pos.y );
				V2d e( end->pos.x, end->pos.y );
				V2d along = normalize( e - s );
				V2d pos = s + along * (*ait)->groundInfo->groundQuantity;
				Vector2i pi( pos.x, pos.y );//floor( pos.x + .5 ), floor( pos.y + .5 );
				of << pi.x << " " << pi.y << endl;
				//of << (*ait)->position.x << " " << (*ait)->position.y << endl;
				//only should be one goal, but this isnt enforced yet
				quitLoop = true;
			}
		}
		//(*it).second->WriteFile( of );
		//(*it).second->( w );
	}


	int writeIndex = 0;
	for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		if( (*it)->layer == 0 && (*it)->path.size() < 2 )
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
	for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		if( (*it)->layer == 0 && (*it)->path.size() >= 2 )
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

	of << bgPlatCount0 << endl;
	writeIndex = 0;
	for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		if( (*it)->layer == 1 )// && (*it)->path.size() < 2 )
		{
			//writeindex doesnt matter much for these for now
			(*it)->writeIndex = writeIndex;
			++writeIndex;

			of << (*it)->material << endl;

			of <<  (*it)->numPoints << endl;

			for( TerrainPoint *pcurr = (*it)->pointStart;  pcurr != NULL; pcurr = pcurr->next )
			{
				of << pcurr->pos.x << " " << pcurr->pos.y << endl; // << " " << (int)(*it2).special << endl;
			}

			//WriteGrass( (*it), of );
		}
	}

	of << lights.size() << endl;
	for( list<StaticLight*>::iterator it = lights.begin(); it != lights.end(); ++it )
	{
		(*it)->WriteFile( of );
	}

	//minus 1 because of the player group
	of << groups.size() - 1 << endl;
	//write the stuff for goals and remove them from the enemy stuff

	for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it )
	{
		(*it).second->WriteFile( of );
		//(*it).second->( w );
	}

	of << gates.size() << endl;
	for( list<GateInfoPtr>::iterator it = gates.begin(); it != gates.end(); ++it )
	{
		(*it)->WriteFile( of );
	}

	

	//enemies here


}

void EditSession::WriteGrass( PolyPtr poly, ofstream &of )
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

bool PointOnLine( V2d &pos, V2d &p0, V2d &p1, double width = 0 )
{
	V2d dir = normalize( p1 - p0 );
	double len = length( p1 - p0 );
	double d = dot( pos - p0, dir );

	if( d >= width && d <= len - width )
	{
		double c = cross( pos - p0, dir );	
		//cout << "c: " << c << endl;
		if( abs( c ) < 1.0 )
		{
			//cout << "return true?>?? " << endl;
			return true;
		}
	}
	return false;
}

//returns true if attach is successful
bool AttachActorToPolygon( ActorPtr &actor, TerrainPolygon *poly )
{
	TerrainPoint *next;
	V2d currPos, nextPos;
	V2d aCurr, aNext;
	V2d actorPos;
	for( TerrainPoint *p = poly->pointStart; p != NULL; p = p->next )
	{
		if( p == poly->pointEnd )
		{
			next = poly->pointStart;
		}
		else
		{
			next = p->next;
		}

		currPos.x = p->pos.x;
		currPos.y = p->pos.y;

		nextPos.x = next->pos.x;
		nextPos.y = next->pos.y;

		assert( actor->groundInfo != NULL );
		double actorQuant = actor->groundInfo->groundQuantity;
		aCurr.x = actor->groundInfo->edgeStart->pos.x;
		aCurr.y = actor->groundInfo->edgeStart->pos.y;

		TerrainPoint *nextActorPoint = actor->groundInfo->edgeStart->next;
		if( actor->groundInfo->edgeStart == actor->groundInfo->ground->pointEnd )
		{
			nextActorPoint = actor->groundInfo->ground->pointStart;
		}
		aNext.x = nextActorPoint->pos.x;
		aNext.y = nextActorPoint->pos.y;

		actorPos = aCurr + normalize( aNext - aCurr ) * actorQuant;
		bool onLine = PointOnLine( actorPos, currPos, nextPos );

		double finalQuant = dot( actorPos - currPos, normalize( nextPos - currPos ) );
		
		if( onLine )
		{
			cout << "actorPos: " << actorPos.x << ", " << actorPos.y << ", currPos: "
				<< currPos.x << ", " << currPos.y << endl;
			GroundInfo gi;
			gi.edgeStart = p;
			gi.ground = poly;
			cout << "finalQuant: " << finalQuant << endl;
			gi.groundQuantity = finalQuant;
			actor->AnchorToGround( gi );
			poly->enemies[p].push_back( actor );
			return true;
			//break;
		}
	}

	return false;
}

void AttachActorsToPolygon( list<ActorPtr> &actors, TerrainPolygon *poly )
{
	//cout << "attemping to attach actors" << endl;
	bool res;
	for( list<ActorPtr>::iterator it = actors.begin(); it != actors.end(); ++it )
	{
		res = AttachActorToPolygon( (*it), poly );
		/*if( res )
		{
			cout << "saved an actor!" << endl;
		}
		else
		{
			cout << "totally didn't save an actor! QQ" << endl;
		}*/
	}
}


void EditSession::Add( PolyPtr brush, PolyPtr poly )
{
	//cout << "brush: " << brush->enemies.size() << endl;
	//cout << "poly: " << poly->enemies.size() << endl;

	//int totalEnemies = brush->enemies.size() + poly->enemies.size();
	//int totalGates = 0;

	//deleting gates for adding. need to just make this more specific. 
	/*for( TerrainPoint *curr = brush->pointStart; curr != NULL; curr = curr->next )
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
	}*/

	//if( totalEnemies > 0 || totalGates > 0)
	//{
	//	stringstream ss;
	//	ss << "destroying " << totalEnemies << " enemies, and " <<  totalGates << " gates to create the polygons.\n Sorry for how messy this is at the moment!";
	//	MessagePop( ss.str() );
	//	brush->DestroyEnemies();
	//	poly->DestroyEnemies();
	//}

	TerrainPolygon z( &grassTex );
	//1: choose start point

	Vector2i startPoint;
	bool startPointFound = false;
	bool firstPolygon = true;


	PolyPtr currentPoly = NULL;
	PolyPtr otherPoly = NULL;
	TerrainPoint *curr = poly->pointStart;
	TerrainPoint *start;	

	PolyPtr minPoly = NULL;


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

	//2. run a loop clockwise until you arrive back at the original state



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
		Vector2i otherNextPoint;
		Vector2i minPoint;
		
		LineIntersection li1 = SegmentIntersect( currPoint, nextPoint, otherPoly->pointEnd->pos, otherCurrPoint );
		Vector2i lii1( floor(li1.position.x + .5), floor(li1.position.y + .5) );
		if( !li1.parallel && ( lii1 != currPoint && lii1 != nextPoint && lii1 != otherPoly->pointEnd->pos && lii1 != otherCurrPoint ) ) 
		{
			minIntersection = lii1;
			minPoint = otherCurrPoint;
			min = otherPoly->pointStart;
			emptyInter = false;
		}

		for(; other != NULL; other = other->next )
		{
			otherNextPoint = other->pos;
			LineIntersection li = LimitSegmentIntersect( currPoint, nextPoint, otherCurrPoint, otherNextPoint );
			Vector2i lii( floor(li.position.x + .5), floor(li.position.y + .5) );
			if( !li.parallel )
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
			PolyPtr temp = currentPoly;
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
				list<ActorPtr> &en = z.enemies[tp];
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
		

		

		poly->AddPoint( tp );

	}

	
	//cout << "about to check for enemy stuff" << endl;
	for( TerrainPoint *bit = brush->pointStart; bit != NULL; bit = bit->next )
	{
		//cout << "z enems: " << z.enemies.count( zit ) << endl;
		//cout << "enems: " << 
		//if( z.enemies.count( zit ) > 0 )
		if( brush->enemies.count( bit ) > 0 )
		{
			//list<ActorPtr> &en = poly->enemies[tp];
			//en = z.enemies[zit];
			list<ActorPtr> &en = brush->enemies[bit];//z.enemies[zit];

			AttachActorsToPolygon( en, poly.get() );
		}
	}

	poly->Finalize();
}


struct SubInfo
{
	SubInfo( TerrainPolygon *p, TerrainPoint *po )
		:poly(p),point(po){}
	TerrainPolygon *poly;
	TerrainPoint *point;
};

void EditSession::Sub( TerrainPolygon *brush, std::list<PolyPtr> &orig, std::list<PolyPtr> &results )
{
	for( list<PolyPtr>::iterator polyIt = orig.begin(); polyIt != orig.end(); ++polyIt )
	{
		TerrainPolygon *poly = (*polyIt).get();
		list<TerrainPoint*> untouched;
		for( TerrainPoint *curr = poly->pointStart; curr != NULL; curr = curr->next )
		{
			if( !brush->ContainsPoint( Vector2f( curr->pos.x, curr->pos.y ) ) )
			{
				untouched.push_back( curr );
			}
		}

		if( untouched.empty() )
		{
			//list<TerrainPoint*> 
			//put a new point in between the intersection points in the old polygon?
			//dont make it part of the object its just a temporary

			//cout << "EMPTY BLARG HERE" << endl;
		}

		bool onBrush = false;
		TerrainPolygon *currentPoly = poly;
		TerrainPolygon *otherPoly = brush;

		while( !untouched.empty() )
		{
			//cout << "untouched is not empty!:" << untouched.size() << endl;
			//PolyPtr newPoly( new TerrainPolygon( &grassTex ) );
			list<pair<Vector2i,SubInfo>> newPoints;

			
			TerrainPoint *start = untouched.front();
			//untouched.pop_front();

			TerrainPoint *curr = start;
			TerrainPoint *next = NULL;

			Vector2i currPoint = curr->pos;
			bool truePoint = true;
			
			do
			{
				if( currentPoly == poly )
				{
					if( curr->next == NULL )
					{
						next = poly->pointStart;
					}
					else
					{
						next = curr->next;
					}
					
				}
				else
				{
					if( curr->prev == NULL )
					{
						next = brush->pointEnd;
					}
					else
					{
						next = curr->prev;
					}
					
				}

				//untouched.remove( curr );
				//newPoints.push_back( TerrainPoint( *curr ) );
				if( truePoint )
				{
					newPoints.push_back( pair<Vector2i,SubInfo>( currPoint, SubInfo( currentPoly, curr ) ) );
				}
				else
				{
					newPoints.push_back( pair<Vector2i,SubInfo>( currPoint, SubInfo( NULL, NULL ) ) );
				}

				if( currentPoly == poly && currPoint == curr->pos )
				{
					untouched.remove( curr );
					//cout << "untouched now has: " << untouched.size() << endl;
				}
				
				//untouched.pop_front();
			
				TerrainPoint *min;
				Vector2i minIntersection;
				bool emptyInter = true;

				//get closest intersection to my current point
				for( TerrainPoint *other = otherPoly->pointStart; other != NULL; other = other->next )
				{																															
					TerrainPoint *otherPrev;
					if( other == otherPoly->pointStart )
					{
						otherPrev = otherPoly->pointEnd;
					}
					else
					{
						otherPrev = other->prev;
					}

					LineIntersection li = LimitSegmentIntersect( currPoint, next->pos, otherPrev->pos, other->pos );
					Vector2i lii( floor(li.position.x + .5), floor(li.position.y + .5) );
					if( !li.parallel )
					{
						if( emptyInter )
						{
							emptyInter = false;
							minIntersection = lii;
							if( currentPoly == poly )
								min = other;
							else
								min = otherPrev;
						}
						else
						{
							V2d blah( minIntersection - currPoint );
							V2d blah2( lii - currPoint );
							if( length( blah2 ) < length( blah ) )
							{
								minIntersection = lii;
								if( currentPoly == poly )
									min = other;
								else
									min = otherPrev;
							}
						}
					}
				}

				if( !emptyInter )
				{
					TerrainPolygon *temp = currentPoly;
					currentPoly = otherPoly;
					otherPoly = temp;

					//newPoints.push_back( minIntersection );

					currPoint = minIntersection;

					curr = min;
					truePoint = false;

				}
				else
				{
					curr = next;
					currPoint = next->pos;
					truePoint = true;
				}
				
			}
			while( curr != start );



			PolyPtr newPoly( new TerrainPolygon( &grassTex ) );
			for( list<pair<Vector2i,SubInfo>>::iterator it = newPoints.begin(); it != newPoints.end(); ++it )
			{
				TerrainPoint *p = new TerrainPoint( (*it).first, false );

				//might need to try and preserve gate
				if( (*it).second.poly != NULL )
				{
					p->gate = (*it).second.point->gate;
					if( p->gate != NULL )
					{
						if( p->gate->poly0.get() == (*it).second.poly )
						{
							p->gate->poly0 = newPoly;
							p->gate->point0 = p;
						}
						else
						{
							p->gate->poly1 = newPoly;
							p->gate->point1 = p;
						}
						//cout << "preserving gate!" << endl;
					}
				}
				//if( (*it)
				newPoly->AddPoint( p );
				//cout << "point: " << p->pos.x << ", " << p->pos.y << endl;
			}
			newPoly->Finalize();
			results.push_back( newPoly );
			//cout << "results pushing back" << endl;
		}
	}
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

bool EditSession::QuadPolygonIntersect( TerrainPolygon* poly, Vector2i a, Vector2i b, Vector2i c, Vector2i d )
{

	//TerrainPolygon *quadPoly = new TerrainPolygon( poly->grassTex );
	//PolyPtr quadPoly( new TerrainPolygon( poly->grassTex ) );
	TerrainPolygon quadPoly( poly->grassTex );
	quadPoly.AddPoint( new TerrainPoint( a, false ) );
	quadPoly.AddPoint( new TerrainPoint( b, false ) );
	quadPoly.AddPoint( new TerrainPoint( c, false ) );
	quadPoly.AddPoint( new TerrainPoint( d, false ) );
	quadPoly.UpdateBounds();

	//PolyPtr blah( &quadPoly );

	//cout << "quad bottom: " << quadPoly.bottom << endl;
	//cout << "poly top: " << poly->top << endl;
	
	bool touching = poly->IsTouching( &quadPoly );

	//delete quadPoly;

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
	tempActor = NULL;
	int width = 1920;//1920 - w->getSize().x;
	int height = 1080; //1080 - w->getSize().y;
	uiView = View( sf::Vector2f( width / 2, height / 2), sf::Vector2f( width, height ) );
	v.setCenter( 0, 0 );
	v.setSize( 1920/ 2, 1080 / 2 );
	w->setView( v );

	modifyGate = NULL;

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
	groups[emptyGroup->name] = emptyGroup;

	//groups["player"]->actors.push_back( player );
	Panel *mapOptionsPanel = CreateOptionsPanel( "map" );
	Panel *terrainOptionsPanel = CreateOptionsPanel( "terrain" );

	

	
	ActorType *keyType = new ActorType( "key", NULL );

	ActorType *greenKeyType = new ActorType( "greenkey", NULL );
	ActorType *blueKeyType = new ActorType( "bluekey", NULL );

	types["key"] = keyType;
	types["greenkey"] = greenKeyType;
	types["bluekey"] = blueKeyType;

	Panel *patrollerPanel = CreateOptionsPanel( "patroller" );//new Panel( 300, 300, this );
	ActorType *patrollerType = new ActorType( "patroller", patrollerPanel );

	Panel *batPanel = CreateOptionsPanel( "bat" );
	ActorType *batType = new ActorType( "bat", batPanel );

	Panel *curveTurretPanel = CreateOptionsPanel( "curveturret" );
	ActorType *curveTurretType = new ActorType( "curveturret", curveTurretPanel );

	Panel *stagBeetlePanel = CreateOptionsPanel( "stagbeetle" );
	ActorType *stagBeetleType = new ActorType( "stagbeetle", stagBeetlePanel );

	Panel *poisonFrogPanel = CreateOptionsPanel( "poisonfrog" );
	ActorType *poisonFrogType = new ActorType( "poisonfrog", poisonFrogPanel );

	Panel *healthflyPanel = CreateOptionsPanel( "healthfly" );
	ActorType *healthflyType = new ActorType( "healthfly", healthflyPanel );

	Panel *crawlerPanel = CreateOptionsPanel( "crawler" );
	ActorType *crawlerType = new ActorType( "crawler", crawlerPanel );

	Panel *crawlerReverserPanel = NULL;
	ActorType *crawlerReverserType = new ActorType( "crawlerreverser", crawlerReverserPanel );

	Panel *basicTurretPanel = CreateOptionsPanel( "basicturret" );
	ActorType *basicTurretType = new ActorType( "basicturret", basicTurretPanel );

	Panel *footTrapPanel = CreateOptionsPanel( "foottrap" );
	ActorType *footTrapType = new ActorType( "foottrap", footTrapPanel );

	Panel *bossCrawlerPanel = NULL;//CreateOptionsPanel( "bosscrawler" );
	ActorType *bossCrawlerType = new ActorType( "bosscrawler", bossCrawlerPanel );

	Panel *goalPanel = CreateOptionsPanel( "goal" );
	ActorType *goalType = new ActorType( "goal", goalPanel );

	
	Panel *lightPanel = CreateOptionsPanel( "light" );

	messagePopup = CreatePopupPanel( "message" );
	errorPopup = CreatePopupPanel( "error" );

	types["patroller"] = patrollerType;
	types["bat"] = batType;
	types["curveturret"] = curveTurretType;
	types["healthfly"] = healthflyType;
	types["poisonfrog"] = poisonFrogType;
	types["stagbeetle"] = stagBeetleType;
	types["crawler"] = crawlerType;
	types["crawlerreverser"] = crawlerReverserType;
	types["basicturret"] = basicTurretType;
	types["foottrap"] = footTrapType;
	types["bosscrawler"] = bossCrawlerType;
	types["goal"] = goalType;
	

	
	
	

	Panel *keyPanel = CreateOptionsPanel( "key" );

	keyType->panel = keyPanel;
	greenKeyType->panel = keyPanel;
	blueKeyType->panel = keyPanel;

	enemySelectPanel = new Panel( "enemyselection", 200, 200, this );
	GridSelector *gs = enemySelectPanel->AddGridSelector( "world0enemies", Vector2i( 20, 20 ), 4, 4, 32, 32, false, true );
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
	sf::Sprite s6( crawlerReverserType->iconTexture );
	sf::Sprite s7( healthflyType->iconTexture );
	sf::Sprite s8( bossCrawlerType->iconTexture );

	sf::Sprite sBat( batType->iconTexture );
	sf::Sprite sCurveTurret( curveTurretType->iconTexture );
	sf::Sprite sPoisonFrog( poisonFrogType->iconTexture );
	sf::Sprite sStagBeetle( stagBeetleType->iconTexture );

	sf::Sprite ss0( greenKeyType->iconTexture );
	sf::Sprite ss1( blueKeyType->iconTexture );
	sf::Sprite ss2( blueKeyType->iconTexture );
	ss2.setColor( Color::Magenta );


	gs->Set( 0, 0, s0, "patroller" );
	gs->Set( 1, 0, s1, "crawler" );
	gs->Set( 2, 0, s2, "basicturret" );
	gs->Set( 3, 0, s3, "foottrap" );
	gs->Set( 0, 1, s4, "goal" );
	gs->Set( 1, 1, s6, "crawlerreverser" );
	gs->Set( 2, 1, s7, "healthfly" );
	gs->Set( 3, 1, s8, "bosscrawler" );

	gs->Set( 0, 2, sBat, "bat" );
	gs->Set( 1, 2, sCurveTurret, "curveturret" );
	gs->Set( 2, 2, sPoisonFrog, "poisonfrog" );
	gs->Set( 3, 2, sStagBeetle, "stagbeetle" );
	//gs->Set( 1, 2, ss0, "greenkey" );
	//gs->Set( 2, 2, ss1, "bluekey" );

	gateSelectorPopup = CreatePopupPanel( "gateselector" );
	GridSelector *gateSel = gateSelectorPopup->AddGridSelector( "gatetypes", Vector2i( 20, 20 ), 4, 2, 32, 32, false, true );
	
	sf::Texture greyTex;
	greyTex.loadFromFile( "greygatecolor.png" );
	sf::Sprite greySpr( greyTex );

	sf::Texture blackTex;
	blackTex.loadFromFile( "blackgatecolor.png" );
	sf::Sprite blackSpr( blackTex );



	gateSel->Set( 0, 0, greySpr, "grey" );
	gateSel->Set( 1, 0, blackSpr, "black" );
	gateSel->Set( 2, 0, ss1, "blue" );
	gateSel->Set( 3, 0, ss0, "green" );
	gateSel->Set( 0, 1, s5, "red" );
	gateSel->Set( 1, 1, ss2, "critical" );

	gateSelectorPopup->AddButton( "deletegate", Vector2i( 20, 80 ), Vector2f( 80, 40 ), "delete" );

	int returnVal = 0;
	w->setMouseCursorVisible( true );
	Color testColor( 0x75, 0x70, 0x90 );
	view = View( cameraPos, cameraSize );
	if( cameraSize.x == 0 && cameraSize.y == 0 )
		view.setSize( 1920, 1080 );

	preScreenTex->setView( view );
	//Texture playerTex;
	//playerTex.loadFromFile( "stand.png" );
	//sf::Sprite playerSprite( playerTex );

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

	//playerSprite.setTextureRect( IntRect(0, 0, 64, 64 ) );
	//playerSprite.setOrigin( playerSprite.getLocalBounds().width / 2, playerSprite.getLocalBounds().height / 2 );

	//w->setVerticalSyncEnabled( true );
	//w->setFramerateLimit( 60 );

	OpenFile( fileName );


//	ActorParams *ap = new ActorParams;
//	ap->CreatePatroller( patrollerType, Vector2i( playerPosition.x, playerPosition.y ), true, 10 );
//	groups["--"]->actors.push_back( ap );
	//ap->CreatePatroller( 



	//Vector2f vs(  );
	if( cameraSize.x == 0 && cameraSize.y == 0 )
		view.setCenter( (float)player->position.x, (float)player->position.y );

	//mode = "neutral";
	bool quit = false;
	polygonInProgress.reset( new TerrainPolygon(&grassTex ) );
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
		worldPosGround = ConvertPointToGround( Vector2i( worldPos.x, worldPos.y ) );

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
								if( !(showPoints && extendingPolygon) )
								{
									ExecuteTerrainCompletion();
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
									list<PolyPtr>::iterator it = polygons.begin();
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
							else if( ev.key.code == sf::Keyboard::Z && ev.key.control )
							{
								if( doneActionStack.size() > 0 )
								{
									Action *action = doneActionStack.back();
									doneActionStack.pop_back();

									//cout << "undoing an action" << endl;
									action->Undo();

									undoneActionStack.push_back( action );
								}
							}
							else if( ev.key.code == sf::Keyboard::Y && ev.key.control )
							{
								if( undoneActionStack.size() > 0 )
								{
									Action *action = undoneActionStack.back();
									undoneActionStack.pop_back();

									action->Perform();

									doneActionStack.push_back( action );
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

								bool emptysp = true;

								if( !( editMouseDownMove || editMouseDownBox ) )
								{
									if( emptysp )
										for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it )
										{
											for( list<ActorPtr>::iterator ait = (*it).second->actors.begin();
												ait != (*it).second->actors.end(); ++ait )
											{

												if( (*ait)->ContainsPoint( Vector2f( worldPos.x, worldPos.y ) ) )
												{
													SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>( (*ait) );

													if( sp->selected )
													{

													}
													else
													{
														if( !( Keyboard::isKeyPressed( Keyboard::LShift ) || 
														Keyboard::isKeyPressed( Keyboard::RShift ) ) )
														{
															selectedBrush->SetSelected( false );
															selectedBrush->Clear();
														}

														sp->SetSelected( true );

														grabbedObject = sp;
														selectedBrush->AddObject( sp );
														//sp->selected = true;
												
													}

													emptysp = false;
													break;
												}
											}
										}


										bool alt = Keyboard::isKeyPressed( Keyboard::LAlt ) ||
											Keyboard::isKeyPressed( Keyboard::RAlt );
									//concerning selecting a point
									if( alt )
									{
										PointSelectPoint( worldPos, emptysp );
									}

									else
									//if( emptysp )
									//if(  )
									for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
									{
										if( (*it)->ContainsPoint( Vector2f( worldPos.x, worldPos.y ) ) )
										{

											SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>( (*it) );

											if( sp->selected )
											{

											}
											else
											{
												if( !( Keyboard::isKeyPressed( Keyboard::LShift ) || 
												Keyboard::isKeyPressed( Keyboard::RShift ) ) )
												{
													selectedBrush->SetSelected( false );
													selectedBrush->Clear();
												}

												sp->SetSelected( true );

												grabbedObject = sp;
												selectedBrush->AddObject( sp );
												//sp->selected = true;
												
											}

											emptysp = false;
											break;
										}
									}

									
									editMouseGrabPos = Vector2i( worldPos.x, worldPos.y );

									//12345
									pointGrabPos = Vector2i( worldPos.x, worldPos.y );

									editMouseOrigPos = editMouseGrabPos;
									//editMouseDown = true;

									if( emptysp )
									{	
										editMouseDownMove = false;
										editMouseDownBox = true;
										editStartMove = false;
									}
									else
									{
										editMouseDownMove = true;
										editStartMove = false;
										editMouseDownBox = false;
									}
								}
								break;

								if( moveActive )//&& !Keyboard::isKeyPressed( Keyboard::LShift ) )
								{
									moveActive = false;

									//cout << "turning moveActive to false" << endl;

									//Action *ac = new ApplyBrushAction( selectedBrush );
									//ac->Perform();
									//delete ac;

									//selectedBrush->Clear();

									break;
								}

								if( false )
								{
								/*for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
								{
									//extended aabb 
									int range = 8;
									if( worldPos.x <= (*it)->right + range && worldPos.x >= (*it)->left - range
										&& worldPos.y <= (*it)->bottom + range && worldPos.y >= (*it)->top - range )
									{
										for( TerrainPoint *curr = (*it)->pointStart; curr != NULL; curr = curr->next )
										{
											if( length( worldPos - V2d( curr->pos.x, curr->pos.y ) ) <= range )
											{
												selected
											}
										}
									}
								}*/
								}
								else
								{
								for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
								{
									if( (*it)->ContainsPoint( Vector2f( worldPos.x, worldPos.y ) ) )
									{
										cout << "SELECTING" << endl;
										SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>( (*it) );
										selectedBrush->AddObject( sp );

										emptysp = false;
										pointMouseDown = Vector2i( worldPos.x, worldPos.y );
										moveActive = true;
										break;
									}
								}
								}

								if( emptysp )
								{
									for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it )
									{
										ActorGroup *ag = (*it).second;
										for( list<ActorPtr>::iterator it2 = ag->actors.begin(); it2 != ag->actors.end(); ++it2 )
										{
											if( (*it2)->ContainsPoint( Vector2f( worldPos.x, worldPos.y ) ) )
											{
												cout << "selecting enemy" << endl;
												SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>( (*it2) );
												selectedBrush->AddObject( sp );
												emptysp = false;
												pointMouseDown = Vector2i( worldPos.x, worldPos.y );
												moveActive = true;
											}
										}

									}
								}



								if( emptysp )
								{
									//Vector2i currMouse( worldPos.x, worldPos.y );
									//Vector2i delta = currMouse - pointMouseDown;
									//pointMouseDown = currMouse;

									cout << "applying" << endl;
									//selectedBrush->Move( delta );
									
									moveActive = false;

									Action *ac = new ApplyBrushAction( selectedBrush );
									ac->Perform();
									delete ac;

									selectedBrush->Clear();
								}

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

									for( list<PolyPtr>::iterator it = selectedPolygons.begin(); 
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
								/*if( playerSprite.getGlobalBounds().contains( worldPos.x, worldPos.y ) )
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
								}*/

								bool emptySpace = true;

								//points
								for( list<PolyPtr>::iterator it = selectedPolygons.begin(); 
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
													for( list<PolyPtr>::iterator it2 = selectedPolygons.begin(); it2 != selectedPolygons.end(); ++it2 )
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
									for( list<PolyPtr>::iterator it = polygons.begin(); 
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
														for( list<PolyPtr>::iterator selIt = 
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
									for( list<PolyPtr>::iterator it = selectedPolygons.begin(); 
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
									list<ActorPtr> &actors = it->second->actors;
									for( list<ActorPtr>::iterator it2 = actors.begin(); it2 != actors.end() && empty; ++it2 )
									{
										sf::FloatRect bounds = (*it2)->image.getGlobalBounds();
										if( bounds.contains( Vector2f( worldPos.x, worldPos.y ) ) )
										{
											//selectedActor = (*it2);
											/*selectedActorGrabbed = true;
											grabPos = Vector2i( worldPos.x, worldPos.y );

											empty = false;
											//cout << "enemy selected" << endl;

											for( list<PolyPtr>::iterator it3 = selectedPolygons.begin(); 
												it3 != selectedPolygons.end(); ++it3 )
											{
												(*it3)->SetSelected( false );
											}
											selectedPolygons.clear();*/
										}
									}
								}

								if( empty )
								{
									GateInfo *closest = NULL;
									double closestDist = 50;
									for( list<GateInfoPtr>::iterator it = gates.begin(); it != gates.end(); ++it )
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
												//closest = (*it);
												//closestDist = dist;
											}
										}
									}


									if( closest != NULL )
									{
										//selectedGate = closest;
										//empty = false;
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

								if( editStartMove )
								{
									bool done = false;
									bool single = selectedBrush->objects.size() == 1 
										&& selectedPoints.size() == 0
										&& selectedBrush->objects.front()->selectableType == ISelectable::ACTOR;
									if( single )
									{
										ActorPtr actor = boost::dynamic_pointer_cast<ActorParams>( selectedBrush->objects.front() );
										if( actor->groundInfo != NULL )
										{
											Action *gAction = new GroundAction( actor );
											gAction->performed = true;

											if( moveAction != NULL )
											{
												moveAction->subActions.push_back( gAction );
												doneActionStack.push_back( moveAction );
											}
											else
											{
												Vector2i delta = Vector2i( worldPos.x, worldPos.y ) - editMouseOrigPos;
												Action *action = new MoveBrushAction( selectedBrush, delta, false, PointMap() );

												action->Perform();

												moveAction = new CompoundAction;
												moveAction->subActions.push_back( action );
												moveAction->subActions.push_back( gAction );
												doneActionStack.push_back( moveAction );
											}
											
											done = true;
										}
									}

									if( !done )
									{
										//here the delta being subtracted is the points original position
										for( PointMap::iterator mit = selectedPoints.begin(); mit != selectedPoints.end(); ++mit )
										{
											list<PointMoveInfo> &pList = (*mit).second;
											for( list<PointMoveInfo>::iterator it = pList.begin(); it != pList.end(); ++it )
											{
												(*it).delta = (*it).point->pos - (*it).delta;
											}
										}

									//	for( PointMap::iterator mit = selectedPoints.begin(); mit != selectedPoints.end(); ++mit )
									//	{
											//(*mit).first->movingPointMode = false;
										//}

										Vector2i delta = Vector2i( worldPos.x, worldPos.y ) - editMouseOrigPos;
										Action *action = new MoveBrushAction( selectedBrush, delta, false, selectedPoints );

										
										action->Perform();

										if( moveAction != NULL )
										{

											moveAction->subActions.push_back( action );
											//cout << "moveAction size: " << moveAction->subActions.size() << endl;
											doneActionStack.push_back( moveAction );
										}
										else
										{
											doneActionStack.push_back( action );
										}
									}

									
									bool validMove = false;
									
									
									//check if valid

									if( selectedBrush->CanApply() )
									{
										validMove = true;
									}
									//selected


									if( validMove )
									{
										ClearUndoneActions();
									}
									else
									{
										Action * action = doneActionStack.back();
										doneActionStack.pop_back();

										action->Undo();

										delete action;
									}
								}
								else if( editMouseDownBox )
								{
									//selectedBrush->SetSelected( false );
									//selectedBrush->Clear();

									Vector2i currPos( worldPos.x, worldPos.y );

									int left = std::min( editMouseOrigPos.x, currPos.x );
									int right = std::max( editMouseOrigPos.x, currPos.x );
									int top = std::min( editMouseOrigPos.y, currPos.y );
									int bot = std::max( editMouseOrigPos.y, currPos.y );
									
									
									sf::Rect<int> r( left, top, right - left, bot - top );
									//check this rectangle for the intersections, but do that next

									bool selectionEmpty = true;

									bool shift = Keyboard::isKeyPressed( Keyboard::LShift ) || Keyboard::isKeyPressed( Keyboard::RShift );

									if( !shift )
									{
										cout << "clearing everything" << endl;
										selectedBrush->SetSelected( false );
										selectedBrush->Clear();
									}

									//if( selectionEmpty )
									for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it )
									{
										for( list<ActorPtr>::iterator ait = (*it).second->actors.begin();
											ait != (*it).second->actors.end(); ++ait )
										{
											if( (*ait)->Intersects( r ) )
											{
												SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>( (*ait) );

												if( shift )
												{
													if( sp->selected )
													{
														sp->SetSelected( false );
														selectedBrush->RemoveObject( sp ); //might be slow?
													}
													else
													{
														sp->SetSelected( true );
														selectedBrush->AddObject( sp );
													}
												}
												else
												{
													sp->SetSelected( true );
													selectedBrush->AddObject( sp );
												}


												selectionEmpty = false;
											}
										}
									}


									bool alt = Keyboard::isKeyPressed( Keyboard::LAlt )
										|| Keyboard::isKeyPressed( Keyboard::RAlt );
									if( alt ) //always use point selection for now
									{
										bool shift = Keyboard::isKeyPressed( Keyboard::LShift )
											|| Keyboard::isKeyPressed( Keyboard::RShift );
										if( !shift )
										{
											ClearSelectedPoints();
										}

										for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
										{
											double rad = 8 * zoomMultiple;
											IntRect adjustedR( r.left - rad, r.top, r.width, r.height );

											//aabb w/ polygon

											if( (*it)->Intersects( adjustedR ) )
											{
												TerrainPoint *curr = (*it)->pointStart;
												while( curr != NULL )
												{
													if( IsQuadTouchingCircle( V2d( r.left, r.top ), 
														V2d( r.left + r.width, r.top ),
														V2d( r.left + r.width, r.top + r.height ),
														V2d( r.left, r.top + r.height ),
														V2d( curr->pos.x, curr->pos.y ), rad ) 
														|| adjustedR.contains( curr->pos ) )
													{
														if( !curr->selected )
														{
															curr->selected = true;
															selectedPoints[(*it).get()].push_back( PointMoveInfo( curr ) );
														}
													}
													curr = curr->next;
												}
											}
										}
										
										//for( PointMap::iterator it = selectedPoints.begin(); it != selectedPoints.end(); ++it )
										//{
										//	list<PointMoveInfo> &pList = (*it).second;

										//	for( list<PointMoveInfo>::iterator pit = pList.begin(); pit != pList.end(); ++pit )
										//	{
										//		Vector2i pointPos = (*pit).point->pos;
										//		double rad = 8 * zoomMultiple;
										//		
										//		//check if the point is within the quad that i drew
										//		if( IsQuadTouchingCircle( V2d( r.left, r.top ), 
										//			V2d( r.left + r.width, r.top ),
										//			V2d( r.left + r.width, r.top + r.height ),
										//			V2d( r.left, r.top + r.height ),
										//			V2d( pointPos.x, pointPos.y ), rad ) )
										//		{
										//			bool shift = Keyboard::isKeyPressed( Keyboard::LShift )
										//				|| Keyboard::isKeyPressed( Keyboard::RShift );

										//			if( shift )
										//			{

										//			}
										//			else
										//			{
										//				//clear selectedPoints //make this a function
										//			}
										//		}
										//	}
										//}
										//Rect<double> 
										//if( IsQuadTouchingCircle( V2d( 
									}


									else
									//if( false ) //polygon selection. don't use it for a little bit
									for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
									{
										if( (*it)->Intersects( r ) )
										{

											SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>( (*it) );

											if( shift )
											{
												if( sp->selected )
												{
													sp->SetSelected( false );
													selectedBrush->RemoveObject( sp );
												}
												else
												{
													sp->SetSelected( true );
													selectedBrush->AddObject( sp );
												}
											}
											else
											{
												sp->SetSelected( true );
												selectedBrush->AddObject( sp );
											}

											selectionEmpty = false;
											//break;
										}
									}
									
									if( selectionEmpty )
									{
										selectedBrush->SetSelected( false );
										selectedBrush->Clear();
									}

									//selectedBrush->AddObject( grabbedObject );
								}

								editMouseDownBox = false;
								editMouseDownMove = false;
								editStartMove = false;

								if( !pasteBrushes.empty() )
								{
									bool validPaste = true;
									for( list<TerrainBrush*>::iterator tbIt = pasteBrushes.begin();
										tbIt != pasteBrushes.end(); ++tbIt )
									{										
										for( list<PolyPtr>::iterator it = polygons.begin(); 
											it != polygons.end() && validPaste; ++it )
										{
											PolyPtr currentBrush( new TerrainPolygon( (*it)->grassTex ) );
										
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

												if( !IsPointValid( prev->pos, curr->pos, (*it).get() ) )
												{
													validPaste = false;
												}


											}

											//delete currentBrush;
											currentBrush.reset();
										}
									}

									if( validPaste)
									{
									for( list<TerrainBrush*>::iterator tbIt = pasteBrushes.begin();
										tbIt != pasteBrushes.end(); ++tbIt )
									{
										list<PolyPtr>::iterator it = polygons.begin();
										bool added = false;
										//polygonInProgress->Finalize(); //i should check if i can remove this
										bool recursionDone = false;

										cout << "b4" << endl;
										PolyPtr currentBrush( new TerrainPolygon( (*it)->grassTex ) );
										
										//cout << "after: " << (unsigned int)((*tbIt)->pointStart) << endl;
										for( TerrainPoint *curr = (*tbIt)->pointStart; curr != NULL;
											curr = curr->next )
										{
											cout << "adding" << endl;
											currentBrush->AddPoint( new TerrainPoint(*curr) );
										}

										currentBrush->UpdateBounds();

										

									//	PolyPtr currentBrush = polygonInProgress;
										
										while( it != polygons.end() )
										{
											PolyPtr temp = (*it);
											if( currentBrush->IsTouching( temp.get() ) )
											{
												cout << "before addi: " << (*it)->numPoints << endl;
						
												Add( currentBrush, temp );

												//currentBrush->Reset();
												//delete currentBrush;
												currentBrush.reset();
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
											PolyPtr brushPoly( new TerrainPolygon( polygonInProgress->grassTex ) );
										
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
									for( list<PolyPtr>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
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

									for( list<PolyPtr>::iterator it = selectedPolygons.begin(); 
										it != selectedPolygons.end(); ++it )
									{
										TerrainBrush *tb = new TerrainBrush( (*it) );
										copyBrushes.push_back( tb );
									}
								}
							}
							else if( ev.key.code == sf::Keyboard::Z && ev.key.control )
							{
								if( doneActionStack.size() > 0 )
								{
									Action *action = doneActionStack.back();
									doneActionStack.pop_back();

									action->Undo();

									undoneActionStack.push_back( action );
								}
							}
							else if( ev.key.code == sf::Keyboard::Y && ev.key.control )
							{
								if( undoneActionStack.size() > 0 )
								{
									Action *action = undoneActionStack.back();
									undoneActionStack.pop_back();

									action->Perform();

									doneActionStack.push_back( action );
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
										for( list<PolyPtr>::iterator it = selectedPolygons.begin(); 
											it != selectedPolygons.end(); ++it )
										{
											for( EnemyMap::iterator mapIt = (*it)->enemies.begin(); mapIt != (*it)->enemies.end(); ++mapIt)
											{
												list<ActorPtr>::iterator et = (*mapIt).second.begin();
												while( et != (*mapIt).second.end() )
												{
													TerrainPoint *edgeEnd = (*et)->groundInfo->edgeStart->next;
													if( edgeEnd == NULL )
														edgeEnd = (*et)->groundInfo->ground->pointStart;

													bool deleted = (*(*et)->groundInfo->edgeStart).selected || edgeEnd->selected;
													if (deleted)
													{
														//delete enemy here


													//	(*et)->group->actors.remove( (*et ) );
													//	delete (*et); //deleting actor
													//	(*it)->enemies[(*et)->groundInfo->edgeStart].erase(et++); 
													}
													else
													{
														++et;
													}
												}
											}
										}


										for( list<PolyPtr>::iterator it = selectedPolygons.begin(); 
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
									/*if( selectedActor->groundInfo != NULL && selectedActor->groundInfo->ground != NULL )
									{
										selectedActor->groundInfo->ground->enemies[selectedActor->groundInfo->edgeStart].remove( selectedActor );
									}
									selectedActor->group->actors.remove( selectedActor );
									delete selectedActor;
									
									selectedActor = NULL;*/
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
									//gates.remove( selectedGate );
									selectedGate->point0->gate = NULL;
									selectedGate->point1->gate = NULL;
									//delete selectedGate;
									selectedGate = NULL;
								}
								else
								{									
									//cout << "performed removal" << endl;
									bool perform = true;

									//need to make sure to test for this on turning stuff into brushes
									if( selectedBrush->objects.size() == 1 )
									{
										SelectPtr test = boost::dynamic_pointer_cast<ISelectable>( player );

										if( test == selectedBrush->objects.front() )
										{
											perform = false;
										}
									}

									if( perform )
									{
										Action *remove = new RemoveBrushAction( selectedBrush );
										remove->Perform();

										doneActionStack.push_back( remove );
									
										ClearUndoneActions();
									}

									/*int erasedGates = 0;
									for( list<PolyPtr>::iterator it = selectedPolygons.begin();
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
										//delete (*it);
										(*it).reset();
									}
									if( erasedGates > 0 )
									{
										stringstream ss;
										ss << "destroyed " << erasedGates << " gates";
										MessagePop( ss.str() );
									}
									selectedPolygons.clear();*/

									//cout << "destroying terrain. eney: " << selectedActor << endl;
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
								for( list<PolyPtr>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
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
									for( list<PolyPtr>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end() && !ddone; ++it )
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

									/*if( result )
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
									}*/
								}
								else
								{
									MessagePop( "you require two points to create a gate" );
								}

								
							}
							else if( ev.key.code == Keyboard::P )
							{
								if( selectedBrush != NULL )
								{
									SelectList &sl = selectedBrush->objects;
									for( SelectList::iterator it = sl.begin(); it != sl.end(); ++it )
									{
										if( (*it)->selectableType == ISelectable::TERRAIN )
										{
											SelectPtr &select = (*it);
											PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>( select );
											poly->SetLayer( 1 );
										}
									}
								}
							}
							else if( ev.key.code == Keyboard::O )
							{
								if( selectedBrush != NULL )
								{
									SelectList &sl = selectedBrush->objects;
									for( SelectList::iterator it = sl.begin(); it != sl.end(); ++it )
									{
										if( (*it)->selectableType == ISelectable::TERRAIN )
										{
											SelectPtr &select = (*it);
											PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>( select );
											poly->SetLayer( 0 );
										}
									}
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
									for( list<PolyPtr>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
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

									for( list<PolyPtr>::iterator it = selectedPolygons.begin(); 
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
										for( list<PolyPtr>::iterator it = polygons.begin(); 
											it != polygons.end(); ++it )
										{
												
											if( tempRectPoly.IsTouching( (*it).get() )
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
														/*	for( list<PolyPtr>::iterator selIt = 
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
									for( list<PolyPtr>::iterator it = selectedPolygons.begin(); 
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

											for( list<PolyPtr>::iterator it3 = selectedPolygons.begin(); 
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
								for( list<PolyPtr>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
								{
									
									//showGrass = true;
									for( list<PolyPtr>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
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
										for( list<ActorPtr>::iterator git = ag->actors.begin(); git != ag->actors.end(); ++git )
										{
											ActorParams *params = (*git).get();
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
									else if( trackingEnemy->name == "bat" )
									{
										showPanel = trackingEnemy->panel;

										showPanel->textBoxes["name"]->text.setString( "test" );
										showPanel->textBoxes["group"]->text.setString( "not test" );
										showPanel->textBoxes["speed"]->text.setString( "10" );
										showPanel->checkBoxes["loop"]->checked = false;


										patrolPath.clear();
										patrolPath.push_back( Vector2i( worldPos.x, worldPos.y ) );
									}
									else if( trackingEnemy->name == "healthfly" )
									{
										showPanel = trackingEnemy->panel;

										showPanel->textBoxes["name"]->text.setString( "test" );
										showPanel->textBoxes["group"]->text.setString( "not test" );	

										airPos = Vector2i( worldPos.x, worldPos.y );
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
											showPanel->textBoxes["speed"]->text.setString( "1.5" );
											//trackingEnemy = NULL;
										}
									}
									else if( trackingEnemy->name == "poisonfrog" )
									{
										//groups["--"]->name
										if( enemyEdgePolygon != NULL )
										{
											tempActor = new PoisonFrogParams( this, enemyEdgePolygon, enemyEdgeIndex, 
												enemyEdgeQuantity );
											showPanel = trackingEnemy->panel;
											tempActor->SetDefaultPanelInfo();
											//trackingEnemy = NULL;
										}
									}
									else if( trackingEnemy->name == "stagbeetle" )
									{
										//groups["--"]->name
										if( enemyEdgePolygon != NULL )
										{
											showPanel = trackingEnemy->panel;
											showPanel->textBoxes["name"]->text.setString( "test" );
											showPanel->textBoxes["group"]->text.setString( "not test" );
											showPanel->checkBoxes["clockwise"]->checked = false;
											showPanel->textBoxes["speed"]->text.setString( "1.5" );
											//trackingEnemy = NULL;
										}
									}
									else if( trackingEnemy->name == "crawlerreverser" )
									{
										if( enemyEdgePolygon != NULL )
										{
											ActorPtr crawlerReverser( new CrawlerReverserParams( this, 
												enemyEdgePolygon, enemyEdgeIndex, enemyEdgeQuantity ) );
											crawlerReverser->group = groups["--"];
											//groups["--"]->actors.push_back( crawlerReverser );
											enemyEdgePolygon->enemies[crawlerReverser->groundInfo->edgeStart].push_back( crawlerReverser );
											enemyEdgePolygon->UpdateBounds();


											CreateActor( crawlerReverser );
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
									else if( trackingEnemy->name == "curveturret" )
									{
										if( enemyEdgePolygon != NULL )
										{
											//doesn't account for cancelling
											
											tempActor = new CurveTurretParams( this, enemyEdgePolygon, enemyEdgeIndex, 
												enemyEdgeQuantity );
											
											showPanel = trackingEnemy->panel;
											tempActor->SetDefaultPanelInfo();
											//CurveTurretParams *ct = (CurveTurretParams*)tempActor.get();
											//ct->SetPanelInfo();

											

											//showPanel->textBoxes["name"]->text.setString( "test" );
											//showPanel->textBoxes["group"]->text.setString( "not test" );
											//showPanel->textBoxes["bulletspeed"]->text.setString( "10" );
											//showPanel->textBoxes["waitframes"]->text.setString( "10" );
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
									else if( trackingEnemy->name == "bosscrawler" )
									{
										showPanel = enemySelectPanel;
										trackingEnemy = NULL;
										ActorPtr bossCrawler( new BossCrawlerParams( this, enemyEdgePolygon, enemyEdgeIndex,
											enemyEdgeQuantity ) );
										bossCrawler->group = groups["--"];

										CreateActor( bossCrawler );
										//groups["--"]->name
										/*if( enemyEdgePolygon != NULL )
										{
											showPanel = trackingEnemy->panel;
											showPanel->textBoxes["name"]->text.setString( "test" );
											showPanel->textBoxes["group"]->text.setString( "not test" );
											showPanel->checkBoxes["clockwise"]->checked = false;
											showPanel->textBoxes["speed"]->text.setString( "1.5" );
											//trackingEnemy = NULL;
										}*/
									}
									else if( trackingEnemy->name == "goal" )
									{
										if( enemyEdgePolygon != NULL )
										{
											//showPanel = trackingEnemy->panel;
											showPanel = enemySelectPanel;
											trackingEnemy = NULL;
											ActorPtr goal( new GoalParams( this, enemyEdgePolygon, enemyEdgeIndex, 
												enemyEdgeQuantity ) );
											goal->group = groups["--"];
											//actor->SetAsGoal( goalType, enemyEdgePolygon, enemyEdgeIndex, 
											//	enemyEdgeQuantity );
											//groups["--"]->actors.push_back( actor );

											CreateActor( goal );
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
								break;
							}

							if( ev.key.code == Keyboard::X || ev.key.code == Keyboard::Delete )
							{
								if( trackingEnemy != NULL )
								{
									trackingEnemy = NULL;
									showPanel = enemySelectPanel;
								}
							}
							else if( ev.key.code == sf::Keyboard::Z && ev.key.control )
							{
								if( doneActionStack.size() > 0 )
								{
									Action *action = doneActionStack.back();
									doneActionStack.pop_back();

									action->Undo();

									undoneActionStack.push_back( action );
								}
							}
							else if( ev.key.code == sf::Keyboard::Y && ev.key.control )
							{
								if( undoneActionStack.size() > 0 )
								{
									Action *action = undoneActionStack.back();
									undoneActionStack.pop_back();

									action->Perform();

									doneActionStack.push_back( action );
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
								
								if( menuDownStored == EDIT )
								{
									selectedBrush->SetSelected( false );
									selectedBrush->Clear();
								}
								
								/*for( list<PolyPtr>::iterator it = selectedPolygons.begin(); 
									it != selectedPolygons.end(); ++it )
								{
									(*it)->SetSelected( false );
								}
								selectedPolygons.clear();*/
							}
							else if( menuDownStored == CREATE_TERRAIN && menuSelection != "none" )
							{
								polygonInProgress->ClearPoints();
							}

							cout << "menu: " << menuSelection << endl;
							if( menuSelection == "top" )
							{
								bool single = selectedBrush->objects.size() == 1 
									&& selectedPoints.size() == 0
									&& selectedBrush->objects.front()->selectableType == ISelectable::ACTOR;
								//bool singlePoly = selectedBrush->objects.size() == 1 
								//	&& selectedPoints.size() == 0
								//	&& selectedBrush->objects.front()->selectableType == ISelectable::TERRAIN;
								bool onlyPoly = selectedBrush != NULL && !selectedBrush->objects.empty() && selectedBrush->terrainOnly;
								if( menuDownStored == EDIT && onlyPoly )
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
								else if( menuDownStored == EDIT && single )
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
								showPoints = false;
								mode = CREATE_ENEMY;
								trackingEnemy = NULL;
								showPanel = enemySelectPanel;
							}
							else if( menuSelection == "upperright" )
							{
								
								showPoints = false;
								mode = CREATE_TERRAIN;
								showPanel = NULL;
							}
							else if( menuSelection == "lowerleft" )
							{
								//mode = CREATE_LIGHTS;
								mode = CREATE_GATES;
								gatePoints = 0;
								showPanel = NULL;
								showPoints = true;
								/*for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
								{
									
								}*/
							}
							else if( menuSelection == "lowerright" )
							{
								//showPoints = true;
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
							if( ( ev.key.code == Keyboard::X || ev.key.code == Keyboard::Delete ) ) 
							{
								if( selectedBrush != NULL && !selectedBrush->objects.size() > 1 )
								{
									for( SelectIter it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it )
								//for( list<PolyPtr>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
									{
										TerrainPolygon *tp = (TerrainPolygon*)(*it).get();
										if( tp->path.size() > 1 )
										{
											tp->path.pop_back();
										}
									}
								}
								
							}
							else if( ev.key.code == Keyboard::Space )
							{
								if( selectedBrush != NULL && !selectedBrush->objects.empty() )
								{
									TerrainPolygon *tp = (TerrainPolygon*)selectedBrush->objects.front().get();
									if( tp->path.size() == 1 )
									{
										tp->path.pop_back();
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
			case CREATE_GATES:
				{
					switch( ev.type )
					{
					case Event::MouseButtonPressed:
						{
							if( ev.mouseButton.button == Mouse::Left )
							{
								modifyGate = NULL;
								bool found = false;
								for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end() && !found; ++it )
								{
									//extended aabb 
									int range = 8 * zoomMultiple;
									if( worldPos.x <= (*it)->right + range && worldPos.x >= (*it)->left - range
										&& worldPos.y <= (*it)->bottom + range && worldPos.y >= (*it)->top - range )
									{
										int index = 0;
										for( TerrainPoint *curr = (*it)->pointStart; curr != NULL && !found; curr = curr->next )
										{
											if( length( worldPos - V2d( curr->pos.x, curr->pos.y ) ) <= range )
											{
												if( gatePoints == 0 )
												{

													for( list<GateInfoPtr>::iterator git = gates.begin(); git != gates.end() && !found; ++git )
													{
														if( (*git)->point0 == curr || (*git)->point1 == curr )
														{

															GateInfoPtr gi = (*git);

															view.setCenter( curr->pos.x, curr->pos.y );
															preScreenTex->setView( view );

															modifyGate = gi;
															
															
															found = true;
														}
													}

													if( !found )
													{
														found = true;
														gatePoints = 1;
														testGateInfo.poly0 = (*it);
														testGateInfo.point0 = curr;
														testGateInfo.vertexIndex0 = index;
													}
												}
												else
												{
													found = true;
													gatePoints = 2;
													
													testGateInfo.poly1 = (*it);
													testGateInfo.point1 = curr;
													testGateInfo.vertexIndex1 = index;
													view.setCenter( testGateInfo.point1->pos.x, testGateInfo.point1->pos.y );
													preScreenTex->setView( view );
												}
											}

											++index;
										}
									}
								}

								if( !found )
								{
									gatePoints = 0;
								}

							}
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
					case Event::MouseMoved:
						{
							//delta
							break;
						}
					case Event::KeyPressed:
						{
							if( ev.key.code == sf::Keyboard::Z && ev.key.control )
							{
								if( doneActionStack.size() > 0 )
								{
									Action *action = doneActionStack.back();
									doneActionStack.pop_back();

									action->Undo();

									undoneActionStack.push_back( action );
								}
							}
							else if( ev.key.code == sf::Keyboard::Y && ev.key.control )
							{
								if( undoneActionStack.size() > 0 )
								{
									Action *action = undoneActionStack.back();
									undoneActionStack.pop_back();

									action->Perform();

									doneActionStack.push_back( action );
								}
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
				}
				break;
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
							UpdateFullBounds();
						}
						else if( ev.mouseWheel.delta < 0 )
						{
							zoomMultiple *= 2;
							UpdateFullBounds();
						}

						if( zoomMultiple < .25 )
						{
							zoomMultiple = .25;
							UpdateFullBounds();
							cout << "min zoom" << endl;
						}
						else if( zoomMultiple > 65536 )
						{
							zoomMultiple = 65536;
							UpdateFullBounds();
						}
						else if( abs(zoomMultiple - 1.0) < .1 )
						{
							zoomMultiple = 1;
							UpdateFullBounds();
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
						//else if( ev.key.code == sf::Keyboard::Z )
						//{
						//	panning = true;
						//	panAnchor = worldPos;	
						//}
						else if( ev.key.code == sf::Keyboard::Equal || ev.key.code == sf::Keyboard::Dash )
						{
							if( showPanel != NULL )
									break;

							if( ev.key.code == sf::Keyboard::Equal )
							{
								zoomMultiple /= 2;
								UpdateFullBounds();
							}
							else if( ev.key.code == sf::Keyboard::Dash )
							{
								//might be too general
								
								
								zoomMultiple *= 2;
								UpdateFullBounds();
								
								
							}

							if( zoomMultiple < .25 )
							{
								zoomMultiple = .25;
								UpdateFullBounds();
								cout << "min zoom" << endl;
							}
							else if( zoomMultiple > 65536 )
							{
								zoomMultiple = 65536;
								UpdateFullBounds();
							}
							else if( abs(zoomMultiple - 1.0) < .1 )
							{
								zoomMultiple = 1;
								UpdateFullBounds();
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
						//if( ev.key.code == sf::Keyboard::Z )
						//{
						//	panning = false;
						//}
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

			//	GroundInfo g = ConvertPointToGround( Vector2i( worldPos.x, worldPos.y ) );
			//	if( g.ground == NULL )
			//	{
			//		cout << "no ground" << endl;
			//	}
			//	else
			//	{
			//		cout << "gi: " << g.GetEdgeIndex() << endl;
			//	}

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
					for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
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
						for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
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
								for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
								{
									if( !IsPointValid( polygonInProgress->pointEnd->pos, worldi, (*it).get() ) )
									{
										validNearPolys = false;
										break;
									}
								}
							}

							//cout << "valid: " << validNearPolys << endl;

							if( validNearPolys )
							{
								if( polygonInProgress->numPoints > 0 && length( V2d( testPoint.x, testPoint.y ) 
									- Vector2<double>(polygonInProgress->pointEnd->pos.x, 
									polygonInProgress->pointEnd->pos.y )  ) >= minimumEdgeLength * std::max(zoomMultiple,1.0 ) )
								{
								//	cout << "check1" << endl;
									if( PointValid( polygonInProgress->pointEnd->pos, worldi ) )
									{
								//		cout << "blah1" << endl;
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
						for( list<PolyPtr>::iterator it = selectedPolygons.begin();
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
						for( list<PolyPtr>::iterator it = selectedPolygons.begin();
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

				if( ( editMouseDownMove && !editStartMove && length( V2d( editMouseGrabPos.x, editMouseGrabPos.y ) - worldPos ) > editMoveThresh * zoomMultiple ) )
				{
					editStartMove = true;
					Vector2i pos( worldPos.x, worldPos.y );
					Vector2i delta = pos - editMouseGrabPos;

					for( PointMap::iterator mit = selectedPoints.begin(); mit != selectedPoints.end(); ++mit )
					{
						list<PointMoveInfo> &pList = (*mit).second;
						for( list<PointMoveInfo>::iterator it = pList.begin(); it != pList.end(); ++it )
						{
							(*it).delta = (*it).point->pos;
						}
					}

					moveAction = selectedBrush->UnAnchor();
					if( moveAction != NULL )
						moveAction->Perform();

					selectedBrush->Move( delta );
					//MoveSelectedPoints( delta );
					MoveSelectedPoints( worldPos );

					editMouseGrabPos = pos;
				}
				else if( editMouseDownMove && editStartMove )
				{
					Vector2i pos( worldPos.x, worldPos.y );
					Vector2i delta = pos - editMouseGrabPos;

					if( selectedBrush->objects.size() == 1 
						&& selectedBrush->objects.front()->selectableType == ISelectable::ACTOR
						&& selectedPoints.empty() )
					{
						ActorPtr actor = boost::dynamic_pointer_cast<ActorParams>( selectedBrush->objects.front() );
						if( actor->type->canBeGrounded )
						{
							if( worldPosGround.ground != NULL )
							{
								if( actor->groundInfo != NULL )
								{
									actor->UnAnchor( actor );
								}

								actor->AnchorToGround( worldPosGround );
								worldPosGround.ground->enemies[worldPosGround.edgeStart].push_back( actor );
								worldPosGround.ground->UpdateBounds();

								//editStartMove = false;
							}
							else
							{
								if( actor->groundInfo != NULL )
								{
									actor->UnAnchor( actor );
								}

								selectedBrush->Move( delta );
								//actor->UnAnchor( actor );
							}
						}
						else
						{
							selectedBrush->Move( delta );
						}
					}
					else
					{
						selectedBrush->Move( delta );

						//MoveSelectedPoints( delta );
						MoveSelectedPoints( worldPos );

					}

					editMouseGrabPos = pos;


				}
				else if( editMouseDownBox )
				{
					//stuff
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
						for( list<PolyPtr>::iterator it = selectedPolygons.begin();
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

									for( list<PolyPtr>::iterator tit = polygons.begin();
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
					for( list<PolyPtr>::iterator it = selectedPolygons.begin();
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
						for( list<PolyPtr>::iterator it = selectedPolygons.begin();
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
										list<ActorPtr> &enemies = (*it)->enemies[curr];
										for( list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait )
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

								for( map<TerrainPoint*,list<ActorPtr>>::iterator mit = (*it)->enemies.begin();
									mit != (*it)->enemies.end(); ++mit )
								{
									list<ActorPtr> &enemies = (*mit).second;//(*it)->enemies[curr];
									for( list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait )
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
						for( list<PolyPtr>::iterator it = selectedPolygons.begin();
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
						for( list<PolyPtr>::iterator it = selectedPolygons.begin();
							it != selectedPolygons.end(); ++it )
						{
							//(*it)->Move( polyGrabDelta );



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
					for( list<PolyPtr>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
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

				if( showPanel == NULL && trackingEnemy != NULL && ( 
					   trackingEnemy->name == "crawler" 
					|| trackingEnemy->name == "crawlerreverser"
					|| trackingEnemy->name == "basicturret"
					|| trackingEnemy->name == "foottrap" 
					|| trackingEnemy->name == "bosscrawler"
					|| trackingEnemy->name == "poisonfrog"
					|| trackingEnemy->name == "stagbeetle"
					|| trackingEnemy->name == "curveturret"
					|| trackingEnemy->name == "goal" ) )
				{
					enemyEdgePolygon = NULL;
				
					double testRadius = 200;
					
					for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
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
								
								enemyEdgePolygon = (*it).get();
								

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
					if( selectedBrush != NULL && !selectedBrush->objects.empty() )
					{
						TerrainPolygon *tp = (TerrainPolygon*)selectedBrush->objects.front().get();
						if( length( ( worldPos - V2d( fullRectCenter.x, fullRectCenter.y ) ) - Vector2<double>(tp->path.back().x, 
							tp->path.back().y )  ) >= minimumPathEdgeLength * std::max(zoomMultiple,1.0 ) )
						{
							Vector2i worldi( testPoint.x - fullRectCenter.x, testPoint.y - fullRectCenter.y );

							for( SelectIter it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it )
							{
								TerrainPolygon *tp1 = (TerrainPolygon*)(*it).get();
								tp1->path.push_back( worldi );
							}
							//for( 
							//for( list<PolyPtr>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
							//{
							//	(*it)->path.push_back( worldi );
							//}
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

		for( list<GateInfoPtr>::iterator it = gates.begin(); it != gates.end(); ++it )
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

				if( editMouseDownBox )
				{
					Vector2i currPos( worldPos.x, worldPos.y );

					int left = std::min( editMouseOrigPos.x, currPos.x );
					int right = std::max( editMouseOrigPos.x, currPos.x );
					int top = std::min( editMouseOrigPos.y, currPos.y );
					int bot = std::max( editMouseOrigPos.y, currPos.y );

					sf::RectangleShape rs( Vector2f( right - left, bot - top ) );
					rs.setFillColor( Color( 200, 200, 200, 80 ) );
					rs.setPosition( left, top );
					preScreenTex->draw( rs );
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
					if( tempActor != NULL )
						tempActor->Draw( preScreenTex );
					else
					{
						preScreenTex->draw( enemySprite );
					}
					//tempActor->Draw( preScreenTex );
					//
					//preScreenTex->draw( enemyQuad );
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
				TerrainPolygon *tp = (TerrainPolygon*)selectedBrush->objects.front().get();

				int pathSize = tp->path.size();//selectedPolygons.front()->path.size();

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
					Vector2i backPoint = tp->path.back();
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

						for( list<sf::Vector2i>::iterator it = tp->path.begin(); 
							it != tp->path.end(); ++it )
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

					//for( list<sf::Vector2i>::iterator it = selectedPolygons.front()->path.begin(); 
					//		it != selectedPolygons.front()->path.end(); ++it )
					for( list<Vector2i>::iterator it = tp->path.begin(); it != tp->path.end(); ++it )
					{
						//TerrainPolygon *tp1 = (TerrainPolygon*)(*it).get();
						//cout << "drawing" << endl;
						cs.setPosition( (*it).x + fullCenter.x, (*it).y + fullCenter.y );
						preScreenTex->draw( cs );
					}		
				}

				break;

/*				CircleShape cs;
				cs.setRadius( 5 * zoomMultiple  );
				cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
				cs.setFillColor( Color::Magenta );
				cs.setPosition( fullCenter.x, fullCenter.y );
				preScreenTex->draw( cs );*/
				

			}
		case CREATE_GATES:
			{
				if( modifyGate != NULL )
				{
					GridSelectPop( "gatetype" );

					if( tempGridResult == "delete" )
					{
						Action * action = new DeleteGateAction( modifyGate );
						action->Perform();
						doneActionStack.push_back( action );

						//gates.remove( modifyGate );
						//modifyGate->point0->gate = NULL;
						//modifyGate->point1->gate = NULL;

						modifyGate = NULL;
					}
					else
					{

						Action * action = new ModifyGateAction( modifyGate, tempGridResult );
						action->Perform();
						doneActionStack.push_back( action );

						//modifyGate->SetType( tempGridResult );
						//modifyGate->UpdateLine();
						modifyGate = NULL;
					}
					break;
				}

				if( gatePoints > 0 )
				{
					CircleShape cs( 5 * zoomMultiple );
					cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
					cs.setPosition( testGateInfo.point0->pos.x, testGateInfo.point0->pos.y );
					cs.setFillColor( COLOR_TEAL );

					V2d origin( testGateInfo.point0->pos.x, testGateInfo.point0->pos.y );
					
					V2d pointB;
					if( gatePoints > 1 )
					{
						pointB = V2d( testGateInfo.point1->pos.x, testGateInfo.point1->pos.y );
					}
					else
					{
						pointB = worldPos;
					}


					V2d axis = normalize( worldPos - origin );
					V2d other( axis.y, -axis.x );


					double width = 4.0 * zoomMultiple;
					V2d closeA = origin + other * width;
					V2d closeB = origin - other * width;
					V2d farA = pointB + other * width;
					V2d farB = pointB - other * width;

					sf::Vertex quad[4] = { 
						sf::Vertex( Vector2f( closeA.x, closeA.y ), Color::White ),
						sf::Vertex( Vector2f( farA.x, farA.y ), Color::White ),
						sf::Vertex( Vector2f( farB.x, farB.y ), Color::White ),
						sf::Vertex( Vector2f( closeB.x , closeB.y ), Color::White )
					};

					preScreenTex->draw( quad, 4, sf::Quads );
					preScreenTex->draw( cs );
				}

				if( gatePoints > 1 )
				{								

					bool result = CanCreateGate( testGateInfo );

					if( result )
					{
						

						GridSelectPop( "gatetype" );

						if( tempGridResult == "delete" )
						{

						}
						else
						{			
							//MessagePop( "gate created" );
							//GateInfoPtr gi = shared_ptr<GateInfo>( new GateInfo );

							Action * action = new CreateGateAction( testGateInfo, tempGridResult );
							action->Perform();
							doneActionStack.push_back( action );

							/*GateInfoPtr gi( new GateInfo );
							//GateInfo *gi = new GateInfo;

							gi->SetType( tempGridResult );

							gi->edit = this;
							gi->poly0 = testGateInfo.poly0;
							gi->vertexIndex0 = testGateInfo.vertexIndex0;
							gi->point0 = testGateInfo.point0;
							gi->point0->gate = gi;

							gi->poly1 = testGateInfo.poly1;
							gi->vertexIndex1 = testGateInfo.vertexIndex1;
							gi->point1 = testGateInfo.point1;
							gi->point1->gate = gi;
							gi->UpdateLine();
							gates.push_back( gi );*/
						}
					}
					else
					{
						MessagePop( "gate would intersect some terrain" );
					}

					gatePoints = 0;
				}
			}
			break;
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
				/*for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
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
					player->position = Vector2i( worldPos.x, worldPos.y );
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
					
					for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
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
		
		

		//playerSprite.setPosition( player->position.x, player->position.y );

		//preScreenTex->draw( playerSprite );
		
		//preScreenTex->draw( iconSprite );

		if( false )
		//if( showPanel == NULL && sf::Keyboard::isKeyPressed( Keyboard::H ) )
		{
			alphaTextSprite.setScale( .5 * view.getSize().x / 960.0, .5 * view.getSize().y / 540.0 );
			alphaTextSprite.setOrigin( alphaTextSprite.getLocalBounds().width / 2, alphaTextSprite.getLocalBounds().height / 2 );
			alphaTextSprite.setPosition( view.getCenter().x, view.getCenter().y );
			preScreenTex->draw( alphaTextSprite );
		}

		//playerSprite.setPosition( player->position.x, player->position.y );

		if( mode == EDIT )
		{
			if( moveActive )//&& Mouse::isButtonPressed( Mouse::Left ) )
			{
				Vector2i currMouse( worldPos.x, worldPos.y );
				Vector2i delta = currMouse - pointMouseDown;
				pointMouseDown = currMouse;
				
				selectedBrush->Move( delta );
				selectedBrush->Draw( preScreenTex );
			}
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
//				sf::FloatRect bounds = playerSprite.getGlobalBounds();
//				sf::RectangleShape rs( sf::Vector2f( bounds.width, bounds.height ) );

	//			rs.setOutlineColor( Color::Cyan );				
	//			rs.setFillColor( Color::Transparent );
	//			rs.setOutlineThickness( 5 );
	//			rs.setPosition( bounds.left, bounds.top );				

	//			preScreenTex->draw( rs );
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
			playerZoomIcon.setPosition( player->position.x, player->position.y );
			playerZoomIcon.setScale( zoomMultiple * 2, zoomMultiple * 2 );
			preScreenTex->draw( playerZoomIcon );
		}
		
		preScreenTex->draw( fullBounds );

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

					bool single = selectedBrush->objects.size() == 1 
						&& selectedPoints.size() == 0
						&& selectedBrush->objects.front()->selectableType == ISelectable::ACTOR;
					bool onlyPoly = selectedBrush != NULL && !selectedBrush->objects.empty() && selectedBrush->terrainOnly;

					if( menuDownStored == EditSession::EDIT && single )// && selectedActor != NULL )
					{
						textmag.setString( "EDIT\nENEMY" );
					}
					else if( menuDownStored == EditSession::EDIT && selectedGate != NULL )
					{
						textmag.setString( "EDIT\nGATE" );
					}
					else if( menuDownStored == EditSession::EDIT && onlyPoly )
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
	for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		//cout << "polygon " << i << " out of " << polygons.size() << " ... " << (*it)->points.size()  << endl;
		++i;
		PolyPtr p = (*it);
		
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

//THIS IS ALSO DEFINED IN ACTORPARAMS NEED TO GET RID OF THE DUPLICATE
//helper function to assign monitor types
ActorParams::MonitorType GetMonitorType( Panel *p )
{
	GridSelector *gs = p->gridSelectors["monitortype"];
	string name = gs->names[gs->selectedX][gs->selectedY];

	ActorParams::MonitorType monitorType;
	if( name == "none" )
	{
		monitorType = ActorParams::NONE;
	}
	else if( name == "red" )
	{
		monitorType = ActorParams::RED;
	}
	else if( name == "green" )
	{
		monitorType = ActorParams::GREEN;
	}
	else if( name == "blue" )
	{
		monitorType = ActorParams::BLUE;
	}
	else
	{
		cout << "panel: " << p->name << ", name: " << name << endl;
		assert( false );
	}
	

	return monitorType;
}

void EditSession::ButtonCallback( Button *b, const std::string & e )
{
	//cout << "start of callback!: " << groups["--"]->actors.size() << endl;
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
			if( mode == EDIT )
			//if( mode == EDIT && selectedActor != NULL )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				PatrollerParams *patroller = (PatrollerParams*)select;
				patroller->monitorType = GetMonitorType( p );
				patroller->speed = speed;
				patroller->loop = loop;
				//patroller->SetPath( patrolPath );
			}
			else if( mode == CREATE_ENEMY )
			{
				//eventually can convert this between indexes or something to simplify when i have more types


				ActorPtr patroller( new PatrollerParams( this, patrolPath.front(), patrolPath, speed, loop ) );
				patroller->monitorType = GetMonitorType( p );
				//cout << "set patroller monitor type to: " << patroller->monitorType << endl;
				//groups["--"]->actors.push_back( patroller);
				patroller->group = groups["--"];

				CreateActor( patroller );
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
	else if( p->name == "bat_options" )
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
			if( mode == EDIT )
			//if( mode == EDIT && selectedActor != NULL )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				BatParams *bat = (BatParams*)select;
				bat->monitorType = GetMonitorType( p );
				bat->speed = speed;
				bat->loop = loop;
				//patroller->SetPath( patrolPath );
			}
			else if( mode == CREATE_ENEMY )
			{
				//eventually can convert this between indexes or something to simplify when i have more types


				ActorPtr bat( new BatParams( this, patrolPath.front(), patrolPath, speed, loop ) );
				bat->monitorType = GetMonitorType( p );
				//cout << "set patroller monitor type to: " << patroller->monitorType << endl;
				//groups["--"]->actors.push_back( patroller);
				bat->group = groups["--"];

				CreateActor( bat );
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

				
				ActorPtr key( new KeyParams( this, patrolPath.front(), patrolPath, speed, loop, stayFrames, teleport, gType ) );
				
				//groups["--"]->actors.push_back( key );
				key->group = groups["--"];

				CreateActor( key );
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

			//not sure if this is what i need
			//if( mode == EDIT && selectedActor != NULL )
			if( mode == EDIT )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				CrawlerParams *crawler = (CrawlerParams*)select;
				crawler->monitorType = GetMonitorType( p );
				crawler->speed = speed;
				crawler->clockwise = clockwise;
			}
			else if( mode == CREATE_ENEMY )
			{
				

				//eventually can convert this between indexes or something to simplify when i have more types
				

				ActorPtr crawler( new CrawlerParams( this, enemyEdgePolygon, enemyEdgeIndex, enemyEdgeQuantity, clockwise, speed ) );
				crawler->group = groups["--"];
				crawler->monitorType = GetMonitorType( p );
				//groups["--"]->actors.push_back( crawler );
				enemyEdgePolygon->enemies[crawler->groundInfo->edgeStart].push_back( crawler );
				enemyEdgePolygon->UpdateBounds();


				CreateActor( crawler );
				/*Brush b;
				SelectPtr select = boost::dynamic_pointer_cast<ISelectable>(crawler);
				b.AddObject( select );
				Action * action = new ApplyBrushAction( &b );
				action->Perform();
				doneActionStack.push_back( action );*/
				//action->p
				//trackingEnemy = NULL;
				
			}
			showPanel = NULL;
			//showPanel = enemySelectPanel;
		}
	}
	else if( p->name == "poisonfrog_options" )
	{
		if( b->name == "ok" );
		{
			//not sure if this is what i need
			//if( mode == EDIT && selectedActor != NULL )
			if( mode == EDIT )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				PoisonFrogParams *poisonFrog = (PoisonFrogParams*)select;
				poisonFrog->SetParams();
				poisonFrog->monitorType = GetMonitorType( p );
			}
			else if( mode == CREATE_ENEMY )
			{
				ActorPtr poisonFrog( tempActor );
				poisonFrog->SetParams();

				poisonFrog->group = groups["--"];
				poisonFrog->monitorType = GetMonitorType( p );


				CreateActor( poisonFrog );
				tempActor = NULL;
			}
			showPanel = NULL;
			//showPanel = enemySelectPanel;
		}
	}
	else if( p->name == "stagbeetle_options" )
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

			//not sure if this is what i need
			//if( mode == EDIT && selectedActor != NULL )
			if( mode == EDIT )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				StagBeetleParams *stagBeetle = (StagBeetleParams*)select;
				stagBeetle->monitorType = GetMonitorType( p );
				stagBeetle->speed = speed;
				stagBeetle->clockwise = clockwise;
			}
			else if( mode == CREATE_ENEMY )
			{
				

				//eventually can convert this between indexes or something to simplify when i have more types
				

				ActorPtr stagBeetle( new StagBeetleParams( this, enemyEdgePolygon, enemyEdgeIndex, enemyEdgeQuantity, clockwise, speed ) );
				stagBeetle->group = groups["--"];
				stagBeetle->monitorType = GetMonitorType( p );
				//groups["--"]->actors.push_back( crawler );
				enemyEdgePolygon->enemies[stagBeetle->groundInfo->edgeStart].push_back( stagBeetle );
				enemyEdgePolygon->UpdateBounds();


				CreateActor( stagBeetle );
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

			if( mode == EDIT )
			//if( mode == EDIT && selectedActor != NULL )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				BasicTurretParams *basicTurret = (BasicTurretParams*)select;
				basicTurret->monitorType = GetMonitorType( p );
				basicTurret->bulletSpeed = bulletSpeed;
				basicTurret->framesWait = framesWait;
			}
			else if( mode == CREATE_ENEMY )
			{
				ActorPtr basicTurret( new BasicTurretParams( this, enemyEdgePolygon, enemyEdgeIndex, 
				enemyEdgeQuantity, bulletSpeed, framesWait ) );

				enemyEdgePolygon->enemies[basicTurret->groundInfo->edgeStart].push_back( basicTurret );
				enemyEdgePolygon->UpdateBounds();

				//groups["--"]->actors.push_back( basicTurret );
				basicTurret->group = groups["--"];
				basicTurret->monitorType = GetMonitorType( p );

				CreateActor( basicTurret );
				//trackingEnemy = NULL;
				
			}
			showPanel = NULL;
			//showPanel = enemySelectPanel;
		}	
	}
	else if( p->name == "curveturret_options" )
	{	
		if( b->name == "ok" )
		{
			if( mode == EDIT )
			//if( mode == EDIT && selectedActor != NULL )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				CurveTurretParams *curveTurret = (CurveTurretParams*)select;
				curveTurret->SetParams();
				curveTurret->monitorType = GetMonitorType( p );
			}
			else if( mode == CREATE_ENEMY )
			{
				ActorPtr curveTurret( tempActor );
				curveTurret->SetParams();

				enemyEdgePolygon->enemies[tempActor->groundInfo->edgeStart].push_back( curveTurret );
				enemyEdgePolygon->UpdateBounds();

				curveTurret->group = groups["--"];
				curveTurret->monitorType = GetMonitorType( p );
				
				CreateActor( curveTurret );
				
				tempActor = NULL;
			}
			showPanel = NULL;
		}	
	}
	else if( p->name == "foottrap_options" )
	{
		if( b->name == "ok" )
		{
			if( mode == EDIT )//&& selectedActor != NULL )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				FootTrapParams *footTrap = (FootTrapParams*)select;
				//FootTrapParams *footTrap = (FootTrapParams*)selectedActor;
				footTrap->monitorType = GetMonitorType( p );
			}
			else if( mode == CREATE_ENEMY )
			{
				ActorPtr footTrap( new FootTrapParams( this, enemyEdgePolygon, enemyEdgeIndex, 
				enemyEdgeQuantity ) );

				enemyEdgePolygon->enemies[footTrap->groundInfo->edgeStart].push_back( footTrap );
				enemyEdgePolygon->UpdateBounds();

				//groups["--"]->actors.push_back( footTrap );
				footTrap->group = groups["--"];
				footTrap->monitorType = GetMonitorType( p );
				//trackingEnemy = NULL;
				showPanel = NULL;

				CreateActor( footTrap );
			}
			showPanel = NULL;
			//showPanel = enemySelectPanel;
		}
	}
	else if( p->name == "healthfly_options" )
	{
		if( b->name == "ok" )
		{
			if( mode == EDIT )
			//if( mode == EDIT && selectedActor != NULL )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				HealthFlyParams *fly = (HealthFlyParams*)select;
				fly->monitorType = GetMonitorType( p );
				fly->color = 0;
				//patroller->speed = speed;
				//patroller->loop = loop;
				//patroller->SetPath( patrolPath );
			}
			else if( mode == CREATE_ENEMY )
			{
				


				ActorPtr fly( new HealthFlyParams( this, airPos, 0 ) );
				fly->monitorType = GetMonitorType( p ); //monitorType;
				//groups["--"]->actors.push_back( patroller);
				fly->group = groups["--"];
				

				CreateActor( fly );
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
			//cout << "setting mode to create path terrain" << endl;
			mode = CREATE_TERRAIN_PATH;
			//patrolPath.clear();

			bool onlyPoly = selectedBrush != NULL && !selectedBrush->objects.empty() && selectedBrush->terrainOnly;

			
			//TerrainPolygon *tp = (TerrainPolygon*)selectedBrush->objects.front().get();

			assert( onlyPoly );//singlePoly );
			//assert( selectedPolygons.size() > 0 );

			int left, right, top, bottom;
			SelectIter it = selectedBrush->objects.begin();

			TerrainPolygon *tp = (TerrainPolygon*)(*it).get();

			left = tp->left;
			right = tp->right;
			top = tp->top;
			bottom = tp->bottom;
			tp->path.clear();
			tp->path.push_back( Vector2i( 0, 0 ) );
			++it;

			for(  ;it != selectedBrush->objects.end(); ++it )
			{
				tp = (TerrainPolygon*)(*it).get();

				tp->path.clear();

				if( tp->left < left )
					left = tp->left;

				if( tp->right > right )
					right = tp->right;

				if( tp->top < top )
					top = tp->top;

				if( tp->bottom > bottom )
					bottom = tp->bottom;

				tp->path.push_back( Vector2i( 0, 0 ) );
			}

			fullRect.left = left;
			fullRect.top = top;
			fullRect.width = right - left;
			fullRect.height = bottom - top;


			for( list<PolyPtr>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
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
	else if( p == gateSelectorPopup )
	{
		tempGridResult = "delete";
	}
	//cout <<"button" << endl;
}

void EditSession::TextBoxCallback( TextBox *tb, const std::string & e )
{
	Panel *p = tb->owner;
	if( p->name == "curveturret_options" )
	{
		if( tb->name == "xgravfactor" || tb->name == "ygravfactor"
			|| tb->name == "bulletspeed" )
		{
			if( mode == EDIT )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				CurveTurretParams *curveTurret = (CurveTurretParams*)select;
				curveTurret->SetParams();
				//curveTurret->monitorType = GetMonitorType( p );
			}
			else if( mode == CREATE_ENEMY )
			{
				CurveTurretParams *curveTurret = (CurveTurretParams*)tempActor;
				curveTurret->SetParams();
			}
		}
	}
	else if( p->name == "poisonfrog_options" )
	{
		if( tb->name == "xstrength" || tb->name == "ystrength" 
			|| tb->name == "gravfactor" )
		{
			if( mode == EDIT )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				PoisonFrogParams *poisonFrog = (PoisonFrogParams*)select;
				poisonFrog->SetParams();
				//curveTurret->monitorType = GetMonitorType( p );
			}
			else if( mode == CREATE_ENEMY )
			{
				PoisonFrogParams *poisonFrog = (PoisonFrogParams*)tempActor;
				poisonFrog->SetParams();
			}
		}
	}
}

void EditSession::GridSelectorCallback( GridSelector *gs, const std::string & p_name )
{
	cout << "grid selector callback!" << endl;
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
		cout << "callback!" << endl;
		if( name != "not set" )
		{
			cout << "real result: " << name << endl;
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
	//cout << cb->name << " was " << e << endl;
	Panel *p = cb->owner;
	if( p->name == "curveturret_options" )
	{
		if( cb->name == "relativegrav" )
		{
			cout << "BLAHBADIOHFWEIHEGHWEAOHGEAWHGEWAHG" << endl;
			if( mode == EDIT )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				CurveTurretParams *curveTurret = (CurveTurretParams*)select;
				curveTurret->SetParams();
				//curveTurret->monitorType = GetMonitorType( p );
			}
			else if( mode == CREATE_ENEMY )
			{
				CurveTurretParams *curveTurret = (CurveTurretParams*)tempActor;
				curveTurret->SetParams();
			}
		}
	}
}

void EditSession::ClearUndoneActions()
{
	for( list<Action*>::iterator it = undoneActionStack.begin(); it != undoneActionStack.end(); ++it )
	{
		delete (*it);
	}
	undoneActionStack.clear();
}

int EditSession::CountSelectedPoints()
{
	int count = 0;
	for( list<PolyPtr>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
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

void EditSession::ClearSelectedPoints()
{
	for( PointMap::iterator pmit = selectedPoints.begin();
		pmit != selectedPoints.end(); ++pmit )
	{
		list<PointMoveInfo> & pList = (*pmit).second;
		for( list<PointMoveInfo>::iterator pit = pList.begin();
			pit != pList.end(); ++pit )
		{
			(*pit).point->selected = false;
			//(*pit).point->SetSelected( false );
		}
	}
	selectedPoints.clear();
}

void EditSession::MoveSelectedPoints( V2d worldPos )//sf::Vector2i delta )
{
	//Vector2i pos( worldPos.x, worldPos.y );
					//Vector2i delta = pos - editMouseGrabPos;
	//Vector2i test( pointGrabPos.x % 32, pointGrabPos.y % 32 );
					
	//pointGrabDelta = delta;
	pointGrabDelta = Vector2i( worldPos.x, worldPos.y ) - pointGrabPos;
	
	Vector2i oldPointGrabPos = pointGrabPos;
	pointGrabPos = Vector2i( worldPos.x, worldPos.y );// - Vector2i( pointGrabDelta.x % 32, pointGrabDelta.y % 32 );
	bool validMove = true;

	//num polys
	int numSelectedPolys = selectedPoints.size();
	Vector2i** allDeltas = new Vector2i*[numSelectedPolys];
	int allDeltaIndex = 0;
	for( PointMap::iterator it = selectedPoints.begin(); it != selectedPoints.end(); ++it )
	//for( list<TerrainPolygon*>::iterator it = pointPolyList.begin();
	//		it != pointPolyList.end(); ++it )
	{
		TerrainPolygon &poly = *((*it).first);

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
		
		
		//if( validMove && !(*it)->IsMovePointsOkay( this, pointGrabDelta, deltas ) )
		//{
		//	validMove = false;
		//	break;
		//}

		++allDeltaIndex;
	}

	if( validMove )
	{
		allDeltaIndex = 0;
		for( PointMap::iterator it = selectedPoints.begin(); it != selectedPoints.end(); ++it )
		//for( list<TerrainPolygon*>::iterator it = pointPolyList.begin();
		//	it != pointPolyList.end(); ++it )
		{
			TerrainPolygon *poly = (*it).first;
			bool affected = false;

			TerrainPoint *points = poly->pointStart;
			int deltaIndex = 0;
			for( TerrainPoint *curr = points; curr != NULL; curr = curr->next )
			{
				TerrainPoint *prev;
				if( curr == poly->pointStart )
				{
					prev = poly->pointEnd;
				}
				else
				{
					prev = curr->prev;
				}


				if( curr->selected ) //selected
				{					
								
					Vector2i delta = allDeltas[allDeltaIndex][deltaIndex];

					curr->pos += pointGrabDelta - delta;

					if( curr->gate != NULL )
					{
						curr->gate->UpdateLine();
					}

					if( poly->enemies.count( curr ) > 0 )
					{
						list<ActorPtr> &enemies = poly->enemies[curr];
						for( list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait )
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

			poly->UpdateBounds();

			if( affected )
			{
				poly->movingPointMode = true;

				for( map<TerrainPoint*,list<ActorPtr>>::iterator mit = poly->enemies.begin();
					mit != poly->enemies.end(); ++mit )
				{
					list<ActorPtr> &enemies = (*mit).second;//(*it)->enemies[curr];
					for( list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait )
					{
						(*ait)->UpdateGroundedSprite();
						(*ait)->SetBoundingQuad();
					}
					//revquant is the quantity from the edge's v1
					//double revQuant = 	
				}
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

	/*for( list<TerrainPolygon*>::iterator it = pointPolysList.begin(); it != pointPolysList.end(); ++it )
	{
		
	}*/
}

void EditSession::ExtendPolygon()
{
	if( polygonInProgress->numPoints > 1 )
	{
		//test final line

		list<PolyPtr>::iterator it = polygons.begin();
		bool added = false;
		polygonInProgress->Finalize();
		bool recursionDone = false;
		PolyPtr currentBrush = polygonInProgress;

			while( it != polygons.end() )
			{
				PolyPtr temp = (*it);
				if( temp != currentBrush && currentBrush->IsTouching( temp.get() ) )
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

bool EditSession::IsExtendPointOkay( PolyPtr poly, sf::Vector2f testPoint )
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

bool EditSession::IsPointValid( sf::Vector2i oldPoint, sf::Vector2i point, TerrainPolygon* poly )
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

	for( list<PolyPtr>::iterator polyIt = polygons.begin(); polyIt != polygons.end(); ++polyIt )
	{
		if( ignore == (*polyIt ).get() )
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

			if( !IsPointValid( oldPoint, currPoint, (*polyIt).get() ) )
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
			for( list<ActorPtr>::iterator ait = (*it).second.begin(); ait != (*it).second.end(); ++ait )
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
			for( list<ActorPtr>::iterator ait = (*it).second.begin(); ait != (*it).second.end(); ++ait )
			{
				//need to round these floats probably
				//cout << "calling this" << endl;
				sf::VertexArray &bva = (*ait)->boundingQuad;
				//V2d along = (*ait)->groundInfo->
				if( QuadPolygonIntersect( (*polyIt).get(), Vector2i( bva[0].position.x, bva[0].position.y ), 
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
		for( list<ActorPtr>::iterator ait = (*it).second.begin(); ait != (*it).second.end(); ++ait )
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
	list<PolyPtr>::iterator it = polygons.begin();
	bool added = false;
	//polygonInProgress->Finalize();
	bool recursionDone = false;
	PolyPtr currentBrush = extendingPolygon;

	showPoints = false;
	extendingPolygon = NULL;
	extendingPoint = NULL;

	while( it != polygons.end() )
	{
		PolyPtr temp = (*it);
		if( temp != currentBrush && currentBrush->IsTouching( temp.get() ) )
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
			//delete currentBrush;
			currentBrush.reset();

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
	//cout << "grid select popupppp" << endl;
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
	pixelPos = Vector2i( 960, 540 );


	Vector2f uiMouse = preScreenTex->mapPixelToCoords( pixelPos );



	gateSelectorPopup->pos.x = uiMouse.x;
	gateSelectorPopup->pos.y = uiMouse.y;

	sf::Event ev;
	while( !closePopup )
	{
		pixelPos = sf::Mouse::getPosition( *w );
		pixelPos.x *= 1920.0 / w->getSize().x;
		pixelPos.y *= 1080.0 / w->getSize().y;
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
						cout << "are we here: " << uiMouse.x << ", " << uiMouse.y << endl;
						gateSelectorPopup->Update( true, uiMouse.x, uiMouse.y );
						//if you click outside of the box, delete the gate
						
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
						cout << "are we real: " << uiMouse.x << ", " << uiMouse.y << endl;
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
		Panel *p = new Panel( "gate_popup", 200, 150, this );
		return p;
	}

	return NULL;
}

//-1 means you denied it, 0 means it didnt work, and 1 means it will work
int EditSession::IsRemovePointsOkay()
{
	bool terrainOkay = true;
	for( list<PolyPtr>::iterator it = selectedPolygons.begin(); 
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
	for( list<PolyPtr>::iterator it = selectedPolygons.begin(); 
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
		Panel *p = new Panel( "patroller_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );
		p->AddLabel( "loop_label", Vector2i( 20, 150 ), 20, "loop" );
		p->AddCheckBox( "loop", Vector2i( 120, 155 ) ); 
		p->AddTextBox( "speed", Vector2i( 20, 200 ), 200, 20, "10" );
		p->AddButton( "createpath", Vector2i( 20, 250 ), Vector2f( 100, 50 ), "Create Path" );

		GridSelector *gs = p->AddGridSelector( "monitortype", Vector2i( 20, 330 ), 4, 1, 32, 32, true, true);
		gs->Set( 0, 0, sf::Sprite( types["key"]->iconTexture ), "none" );
		gs->Set( 1, 0, sf::Sprite( types["key"]->iconTexture ), "red" );
		gs->Set( 2, 0, sf::Sprite( types["greenkey"]->iconTexture ), "green" );
		gs->Set( 3, 0, sf::Sprite( types["bluekey"]->iconTexture ), "blue" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
		//p->
	}
	else if( name == "bat" )
	{
		Panel *p = new Panel( "bat_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );
		p->AddLabel( "loop_label", Vector2i( 20, 150 ), 20, "loop" );
		p->AddCheckBox( "loop", Vector2i( 120, 155 ) ); 
		p->AddTextBox( "speed", Vector2i( 20, 200 ), 200, 20, "10" );
		p->AddButton( "createpath", Vector2i( 20, 250 ), Vector2f( 100, 50 ), "Create Path" );

		GridSelector *gs = p->AddGridSelector( "monitortype", Vector2i( 20, 330 ), 4, 1, 32, 32, true, true);
		gs->Set( 0, 0, sf::Sprite( types["key"]->iconTexture ), "none" );
		gs->Set( 1, 0, sf::Sprite( types["key"]->iconTexture ), "red" );
		gs->Set( 2, 0, sf::Sprite( types["greenkey"]->iconTexture ), "green" );
		gs->Set( 3, 0, sf::Sprite( types["bluekey"]->iconTexture ), "blue" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
		//p->
	}
	else if( name == "healthfly" )
	{
		Panel *p = new Panel( "healthfly_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );

		GridSelector *gs = p->AddGridSelector( "monitortype", Vector2i( 20, 330 ), 4, 1, 32, 32, true, true);
		gs->Set( 0, 0, sf::Sprite( types["key"]->iconTexture ), "none" );
		gs->Set( 1, 0, sf::Sprite( types["key"]->iconTexture ), "red" );
		gs->Set( 2, 0, sf::Sprite( types["greenkey"]->iconTexture ), "green" );
		gs->Set( 3, 0, sf::Sprite( types["bluekey"]->iconTexture ), "blue" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
		//p->
	}
	else if( name == "key" )
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
		Panel *p = new Panel( "crawler_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		p->AddLabel( "clockwise_label", Vector2i( 20, 150 ), 20, "clockwise" );
		p->AddCheckBox( "clockwise", Vector2i( 120, 155 ) ); 
		p->AddTextBox( "speed", Vector2i( 20, 200 ), 200, 20, "1.5" );

		GridSelector *gs = p->AddGridSelector( "monitortype", Vector2i( 20, 330 ), 4, 1, 32, 32, true, true);

		//sf::Sprite ss;
		//ss.setColor( Color::White );
		//ss.setTextureRect( IntRect

		gs->Set( 0, 0, sf::Sprite( types["key"]->iconTexture ), "none" );
		gs->Set( 1, 0, sf::Sprite( types["key"]->iconTexture ), "red" );
		gs->Set( 2, 0, sf::Sprite( types["greenkey"]->iconTexture ), "green" );
		gs->Set( 3, 0, sf::Sprite( types["bluekey"]->iconTexture ), "blue" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
	}
	else if( name == "poisonfrog" )
	{
		Panel *p = new Panel( "poisonfrog_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		p->AddTextBox( "xstrength", Vector2i( 20, 150 ), 200, 20, "10" );
		p->AddTextBox( "ystrength", Vector2i( 20, 200 ), 200, 20, "10" );
		p->AddTextBox( "gravfactor", Vector2i( 20, 250 ), 200, 20, "5" );
		p->AddTextBox( "jumpwaitframes", Vector2i( 20, 300 ), 200, 20, "10" );
		//p->AddLabel( "clockwise_label", Vector2i( 20, 150 ), 20, "clockwise" );
		//p->AddCheckBox( "clockwise", Vector2i( 120, 155 ) ); 
		

		GridSelector *gs = p->AddGridSelector( "monitortype", Vector2i( 20, 330 ), 4, 1, 32, 32, true, true);

		//sf::Sprite ss;
		//ss.setColor( Color::White );
		//ss.setTextureRect( IntRect

		gs->Set( 0, 0, sf::Sprite( types["key"]->iconTexture ), "none" );
		gs->Set( 1, 0, sf::Sprite( types["key"]->iconTexture ), "red" );
		gs->Set( 2, 0, sf::Sprite( types["greenkey"]->iconTexture ), "green" );
		gs->Set( 3, 0, sf::Sprite( types["bluekey"]->iconTexture ), "blue" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
	}
	else if( name == "stagbeetle" )
	{
		Panel *p = new Panel( "stagbeetle_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		p->AddLabel( "clockwise_label", Vector2i( 20, 150 ), 20, "clockwise" );
		p->AddCheckBox( "clockwise", Vector2i( 120, 155 ) ); 
		p->AddTextBox( "speed", Vector2i( 20, 200 ), 200, 20, "1.5" );

		GridSelector *gs = p->AddGridSelector( "monitortype", Vector2i( 20, 330 ), 4, 1, 32, 32, true, true);

		//sf::Sprite ss;
		//ss.setColor( Color::White );
		//ss.setTextureRect( IntRect

		gs->Set( 0, 0, sf::Sprite( types["key"]->iconTexture ), "none" );
		gs->Set( 1, 0, sf::Sprite( types["key"]->iconTexture ), "red" );
		gs->Set( 2, 0, sf::Sprite( types["greenkey"]->iconTexture ), "green" );
		gs->Set( 3, 0, sf::Sprite( types["bluekey"]->iconTexture ), "blue" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
	}
	else if( name == "basicturret" )
	{
		Panel *p = new Panel( "basicturret_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		p->AddTextBox( "bulletspeed", Vector2i( 20, 150 ), 200, 20, "10" );
		p->AddTextBox( "waitframes", Vector2i( 20, 200 ), 200, 20, "10" );

		GridSelector *gs = p->AddGridSelector( "monitortype", Vector2i( 20, 330 ), 4, 1, 32, 32, true, true);
		gs->Set( 0, 0, sf::Sprite( types["key"]->iconTexture ), "none" );
		gs->Set( 1, 0, sf::Sprite( types["key"]->iconTexture ), "red" );
		gs->Set( 2, 0, sf::Sprite( types["greenkey"]->iconTexture ), "green" );
		gs->Set( 3, 0, sf::Sprite( types["bluekey"]->iconTexture ), "blue" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
	}
	else if( name == "curveturret" )
	{
		Panel *p = new Panel( "curveturret_options", 200, 550, this );
		p->AddButton( "ok", Vector2i( 100, 450 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		p->AddTextBox( "bulletspeed", Vector2i( 20, 150 ), 200, 20, "10" );
		p->AddTextBox( "waitframes", Vector2i( 20, 200 ), 200, 20, "10" );
		p->AddTextBox( "xgravfactor", Vector2i( 20, 250 ), 200, 20, "0" );
		p->AddTextBox( "ygravfactor", Vector2i( 20, 300 ), 200, 20, "0" );
		p->AddCheckBox( "relativegrav", Vector2i( 20, 350 ) );

		GridSelector *gs = p->AddGridSelector( "monitortype", Vector2i( 20, 400 ), 4, 1, 32, 32, true, true);
		gs->Set( 0, 0, sf::Sprite( types["key"]->iconTexture ), "none" );
		gs->Set( 1, 0, sf::Sprite( types["key"]->iconTexture ), "red" );
		gs->Set( 2, 0, sf::Sprite( types["greenkey"]->iconTexture ), "green" );
		gs->Set( 3, 0, sf::Sprite( types["bluekey"]->iconTexture ), "blue" );
		
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
	}
	else if( name == "foottrap" )
	{
		Panel *p = new Panel( "foottrap_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );

		GridSelector *gs = p->AddGridSelector( "monitortype", Vector2i( 20, 330 ), 4, 1, 32, 32, true, true);
		gs->Set( 0, 0, sf::Sprite( types["key"]->iconTexture ), "none" );
		gs->Set( 1, 0, sf::Sprite( types["key"]->iconTexture ), "red" );
		gs->Set( 2, 0, sf::Sprite( types["greenkey"]->iconTexture ), "green" );
		gs->Set( 3, 0, sf::Sprite( types["bluekey"]->iconTexture ), "blue" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
	}

	/*else if( name == "crawlerreverser" )
	{
		//Panel *p = new Panel( "crawlerreverser_options", 200, 400, this );
		//p->AddButton( "ok", Vector2i( 100, 300 ), Vector2f( 100, 50 ), "OK" );
		//p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		//p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		return p;
	}*/

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
		p->AddButton( "create_path", Vector2i( 100, 0 ), Vector2f( 100, 50 ), "Create Path" );
		
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

void EditSession::SetMonitorGrid( ActorParams::MonitorType mType, GridSelector *gs )
{
	//GridSelector &gs = *p->gridSelectors["monitortype"];
	gs->selectedY = 0;
	switch( mType )
	{
	case ActorParams::NONE:
		gs->selectedX = 0;
		break;
	case ActorParams::RED:
		gs->selectedX = 1;
		break;
	case ActorParams::GREEN:
		gs->selectedX = 2;
		break;
	case ActorParams::BLUE:
		gs->selectedX = 3;
		break;
	}
}

void EditSession::SetEnemyEditPanel()
{
	//eventually set this up so that I can give the same parameters to multiple copies of the same enemy?
	//need to be able to apply paths simultaneously to multiples also
	ISelectable *sp = selectedBrush->objects.front().get();
	assert( sp->selectableType == ISelectable::ACTOR );
	ActorParams *ap = (ActorParams*)sp;
	
	
	
	//ActorType *type = selectedActor->type;
	ActorType *type = ap->type;
	string name = type->name;

	Panel *p = type->panel;

	if( name == "patroller" )
	{
		PatrollerParams *patroller = (PatrollerParams*)ap;
		
		p->textBoxes["group"]->text.setString( patroller->group->name );

		p->textBoxes["speed"]->text.setString( boost::lexical_cast<string>( patroller->speed ) );
		p->checkBoxes["loop"]->checked = patroller->loop;
		patrolPath = patroller->GetGlobalPath();
		showPanel = p;

		SetMonitorGrid( patroller->monitorType, p->gridSelectors["monitortype"] );
	}
	else if( name == "bat" )
	{
		BatParams *bat = (BatParams*)ap;
		
		p->textBoxes["group"]->text.setString( bat->group->name );

		p->textBoxes["speed"]->text.setString( boost::lexical_cast<string>( bat->speed ) );
		p->checkBoxes["loop"]->checked = bat->loop;
		patrolPath = bat->GetGlobalPath();
		showPanel = p;

		SetMonitorGrid( bat->monitorType, p->gridSelectors["monitortype"] );
	}
	else if( name == "crawler" )
	{
		CrawlerParams *crawler = (CrawlerParams*)ap;
		
		cout << "hmm: " << name << endl;
		p->textBoxes["group"]->text.setString( crawler->group->name );

		p->checkBoxes["clockwise"]->checked = crawler->clockwise;
		p->textBoxes["speed"]->text.setString( boost::lexical_cast<string>( crawler->speed ) );
		//p->AddCheckBox( "clockwise", Vector2i( 120, 155 ) ); 
		//p->AddTextBox( "speed", Vector2i( 20, 200 ), 200, 20, "10" );

		SetMonitorGrid( crawler->monitorType, p->gridSelectors["monitortype"] );
		
		showPanel = p;
		
	}
	else if( name == "stagbeetle" )
	{
		StagBeetleParams *stagBeetle = (StagBeetleParams*)ap;
		
		//cout << "hmm: " << name << endl;
		p->textBoxes["group"]->text.setString( stagBeetle->group->name );

		p->checkBoxes["clockwise"]->checked = stagBeetle->clockwise;
		p->textBoxes["speed"]->text.setString( boost::lexical_cast<string>( stagBeetle->speed ) );
		//p->AddCheckBox( "clockwise", Vector2i( 120, 155 ) ); 
		//p->AddTextBox( "speed", Vector2i( 20, 200 ), 200, 20, "10" );

		SetMonitorGrid( stagBeetle->monitorType, p->gridSelectors["monitortype"] );
		
		showPanel = p;
		
	}
	else if( name == "poisonfrog" )
	{
		PoisonFrogParams *poisonFrog = (PoisonFrogParams*)ap;
		poisonFrog->SetPanelInfo();
		//cout << "hmm: " << name << endl;
		

		//SetMonitorGrid( poisonFrog->monitorType, p->gridSelectors["monitortype"] );
		
		showPanel = p;
		
	}
	else if( name == "basicturret" )
	{
		BasicTurretParams *basicTurret = (BasicTurretParams*)ap;

		//p->AddTextBox( "bulletspeed", Vector2i( 20, 150 ), 200, 20, "10" );
		//p->AddTextBox( "waitframes", Vector2i( 20, 200 ), 200, 20, "10" );
		p->textBoxes["group"]->text.setString( basicTurret->group->name );
		p->textBoxes["bulletspeed"]->text.setString( boost::lexical_cast<string>( basicTurret->bulletSpeed ) );
		p->textBoxes["waitframes"]->text.setString( boost::lexical_cast<string>( basicTurret->framesWait ) );

		SetMonitorGrid( basicTurret->monitorType, p->gridSelectors["monitortype"] );

		showPanel = p;
	}
	else if( name == "curveturret" )
	{
		CurveTurretParams *curveTurret = (CurveTurretParams*)ap;
		curveTurret->SetPanelInfo();
		//p->AddTextBox( "bulletspeed", Vector2i( 20, 150 ), 200, 20, "10" );
		//p->AddTextBox( "waitframes", Vector2i( 20, 200 ), 200, 20, "10" );
		
		

		showPanel = p;
	}
	else if( name == "foottrap" )
	{
		FootTrapParams *footTrap = (FootTrapParams*)ap;

		p->textBoxes["group"]->text.setString( footTrap->group->name );

		SetMonitorGrid( footTrap->monitorType, p->gridSelectors["monitortype"] );

		showPanel = p;
	}
	else if( name == "healthfly" )
	{
		HealthFlyParams *fly = (HealthFlyParams*)ap;

		p->textBoxes["group"]->text.setString( fly->group->name );

		SetMonitorGrid( fly->monitorType, p->gridSelectors["monitortype"] );
	}
	else if( name == "key" )
	{
		KeyParams *key = (KeyParams*)ap;
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
	for( list<GateInfoPtr>::iterator it = gates.begin(); it != gates.end(); ++it )
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

	for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
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
	for( list<GateInfoPtr>::iterator it = gates.begin(); it != gates.end(); ++it )
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

void EditSession::CreateActor( ActorPtr &actor )
{
	Brush b;
	SelectPtr select = boost::dynamic_pointer_cast<ISelectable>(actor);
	b.AddObject( select );
	Action * action = new ApplyBrushAction( &b );
	action->Perform();
	doneActionStack.push_back( action );
}

GroundInfo EditSession::ConvertPointToGround( sf::Vector2i testPoint )
{
	GroundInfo gi;
	double testRadius = 200;
	//PolyPtr poly = NULL;
	gi.ground = NULL;

	for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		if( (*it)->ContainsPoint( Vector2f( testPoint.x, testPoint.y ) ) )
		{
			TerrainPoint *prev = (*it)->pointEnd;
			TerrainPoint *curr = (*it)->pointStart;

			//prev is starting at 0. start normally at 1
			//int edgeIndex = 0;
			//gi.ground = NULL;
			//groundQuantity;
			//gi.edgeStart = NULL;

			double minDistance = 10000000;
			//int storedIndex;
			TerrainPoint *storedEdge = NULL;
			TerrainPolygon *storedPoly = NULL;
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

				//int hw = trackingEnemy->width / 2;
				//int hh = trackingEnemy->height / 2;
				//if( dist < 100 && testQuantity >= 0 && testQuantity <= length( cu - pr ) && testQuantity >= hw && testQuantity <= length( cu - pr ) - hw 
				//	&& length( newPoint - te ) < length( closestPoint - te ) )
				if( dist < 100 && testQuantity >= 0 && testQuantity <= length( cu - pr ) 
					&& length( newPoint - te ) < length( closestPoint - te ) )
				{
					minDistance = dist;

					storedPoly = (*it).get();
					storedEdge = prev;
					storedQuantity = testQuantity;

					//storedIndex = edgeIndex;
					double l = length( cu - pr );
										
					
					closestPoint = newPoint;
					//minDistance = length( closestPoint - te )  
										
					/*enemySprite.setOrigin( enemySprite.getLocalBounds().width / 2, enemySprite.getLocalBounds().height );
					enemySprite.setPosition( closestPoint.x, closestPoint.y );
					enemySprite.setRotation( atan2( (cu - pr).y, (cu - pr).x ) / PI * 180 );

					enemyQuad.setOrigin( enemyQuad.getLocalBounds().width / 2, enemyQuad.getLocalBounds().height );
					enemyQuad.setRotation( enemySprite.getRotation() );
					enemyQuad.setPosition( enemySprite.getPosition() );*/
				}
				else
				{
										
					//cout << "dist: " << dist << ", testquant: " << testQuantity  << endl;
				}

				prev = curr;
				//++edgeIndex;
			}

			
			gi.edgeStart = storedEdge;
			gi.groundQuantity = storedQuantity;
			gi.ground = storedPoly;
			
			//enemyEdgeIndex = storedIndex;
			
			//enemyEdgeQuantity = storedQuantity;
								
			//enemyEdgePolygon = (*it);
								

			//cout << "pos: " << closestPoint.x << ", " << closestPoint.y << endl;
			//cout << "minDist: " << minDistance << endl;

			break;
		}
		if( testPoint.x >= (*it)->left - testRadius 
			&& testPoint.x <= (*it)->right + testRadius
			&& testPoint.y >= (*it)->top - testRadius && testPoint.y <= (*it)->bottom + testRadius )
		{

		}
	}

	return gi;

	/*if( showPanel == NULL && trackingEnemy != NULL && ( trackingEnemy->name == "crawler" 
		|| trackingEnemy->name == "basicturret"
		|| trackingEnemy->name == "foottrap" 
		|| trackingEnemy->name == "goal" ) )
	{
		enemyEdgePolygon = NULL;
				
		double testRadius = 200;
					
		for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
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


	}*/
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

void EditSession::ExecuteTerrainCompletion()
{
	if( polygonInProgress->numPoints > 2 )
	{
		//test final line
		bool valid = true;


		//test for the last line segment intersecting with the polygon
		TerrainPoint * test = polygonInProgress->pointStart;
		TerrainPoint * prev = test;
		test = test->next;

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


		for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
		{
			//if( !PointValid( polygonInProgress->points.back().pos, polygonInProgress->points.front().pos, (*it) ) )
			if( !IsPointValid( polygonInProgress->pointEnd->pos, polygonInProgress->pointStart->pos, (*it).get() ) )
			{
				valid = false;
				break;
			}
		}

		if( PolyIntersectGate( *polygonInProgress ) )
		{
			valid = false;
		}

		if( !valid )
		{
			MessagePop( "unable to complete polygon" );
			//popupPanel = messagePopup;
			return;
		}

									
		list<PolyPtr>::iterator it = polygons.begin();
		bool added = false;
									
		bool recursionDone = false;
		PolyPtr currentBrush = polygonInProgress;

		list<PolyPtr> intersectingPolys;

		polygonInProgress->UpdateBounds();

		bool applyOkay = true;
		for(; it != polygons.end(); ++it )
		{
			if( polygonInProgress->LinesTooClose( (*it).get(), minimumEdgeLength ) )
			{
				//cout << "LINES TOO CLOSE" << endl;
				applyOkay = false;
				break;
			}
			else if( polygonInProgress->LinesIntersect( (*it).get() ) )
			{
				//not too close and I intersect, so I can add
				intersectingPolys.push_back( (*it) );
			}
			//polygoninprogress is already not in the polygons list
			//if( (*it).get() == this )
			//{
			//	continue;
			//}
		}

		if( !applyOkay )
		{
			MessagePop( "polygon is invalid!!! new message" );
		}
		else
		{
			if( intersectingPolys.empty() )
			{
				polygonInProgress->Finalize();

				SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>( polygonInProgress );

				progressBrush->Clear();
											

				progressBrush->AddObject( sp );
									
				Action *action = new ApplyBrushAction( progressBrush );
				action->Perform();
				doneActionStack.push_back( action );

				ClearUndoneActions();

				PolyPtr newPoly( new TerrainPolygon(&grassTex) );
				polygonInProgress = newPoly;
			}
			else
			{
				//add each of the intersecting polygons onto the polygonInProgress,
				//then do a replacebrushaction

				//polygonInProgress->Finalize();
				polygonInProgress->FixWinding();

				//hold shift ATM to activate subtraction
				if( !( Keyboard::isKeyPressed( Keyboard::LShift ) ||
					Keyboard::isKeyPressed( Keyboard::RShift ) ) )
				{
					ExecuteTerrainAdd( intersectingPolys );
				}
				else
				{
					ExecuteTerrainSubtract( intersectingPolys );
				}

			}
		}
	}
	else if( polygonInProgress->numPoints <= 2 && polygonInProgress->numPoints > 0  )
	{
		cout << "cant finalize. cant make polygon" << endl;
		polygonInProgress->ClearPoints();
	}
}


void EditSession::ExecuteTerrainAdd( list<PolyPtr> &intersectingPolys)
{
	Brush orig;
	for( list<PolyPtr>::iterator it = intersectingPolys.begin(); it != intersectingPolys.end(); ++it )
	{
		SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>( (*it) );
		orig.AddObject( sp );

		Add( (*it), polygonInProgress );
												
	}

	SelectPtr sp = boost::dynamic_pointer_cast< ISelectable>( polygonInProgress );

	progressBrush->Clear();
	progressBrush->AddObject( sp );
	cout << "adding: " << orig.objects.size() << ", " << progressBrush->objects.size() << endl;
	Action * action = new ReplaceBrushAction( &orig, progressBrush );
	action->Perform();
	doneActionStack.push_back( action );

	ClearUndoneActions();

	PolyPtr newPoly( new TerrainPolygon(&grassTex) );
	polygonInProgress = newPoly;
}

list<TerrainPoint*> InsertTemporaryPoints( TerrainPolygon *poly, list<Inter> &inters )
{
	list<TerrainPoint*> addedPoints;
	TerrainPoint *tp;
	//TerrainPoint *prev;
	TerrainPoint *next;

	map<TerrainPoint*, list<V2d>> interMap;
	for( list<Inter>::iterator it = inters.begin(); it != inters.end(); ++it )
	{
		interMap[(*it).first].push_back( (*it).second );
	}

	for( map<TerrainPoint*, list<V2d>>::iterator it = interMap.begin(); it != interMap.end(); ++it )
	{
		tp = (*it).first;
		next = tp->next;

	//	//if( prev == NULL )
	//	//	prev = poly->pointEnd;
		//if( next == NULL )
		//	next = poly->pointStart;
		list<V2d> &points = (*it).second;

		int size = (*it).second.size();
		if( size >= 2 ) //has enough intersections on one line
		{
			
			list<V2d>::iterator vit = points.begin();
			V2d prev = (*vit);
			++vit;
			for( ; vit != points.end(); ++vit )
			{
				V2d midPoint = ( (*vit) + prev ) / 2.0;
				if( midPoint.x > 0 )
					midPoint.x += .5;
				else if( midPoint.x < 0 )
					midPoint.x -= .5;

				if( midPoint.y > 0 )
					midPoint.y += .5;
				else if( midPoint.y < 0 )
					midPoint.y -= .5;
				

				TerrainPoint *newPoint = new TerrainPoint( Vector2i( midPoint.x, midPoint.y ), false );
				addedPoints.push_back( newPoint );

				//cout << "inserting new point between: 1: " << prev.x << ", " << prev.y <<
				//	" and: " << (*vit).x << ", " << (*vit).y << endl;
				//cout << "midPoint: " << midPoint.x << ", "
				//	 << midPoint.y << endl;
				poly->InsertPoint( newPoint, tp );
				tp = newPoint;

				prev = (*vit);
			}
		}

	}
	return addedPoints;
	//for( list<Inter>::iterator it = inters.begin(); it != inters.end(); ++it )
	//{
	//	tp = (*it).first;
	//	//prev = tp->prev;
	//	next = tp->next;

	//	//if( prev == NULL )
	//	//	prev = poly->pointEnd;
	//	if( next == NULL )
	//		next = poly->pointStart;

	//	TerrainPoint *newPoint = new TerrainPoint( Vector2i( (*it).second.x + .5,
	//		(*it).second.y + .5 ), false );
	//	cout << "inserting new point!" << endl;
	//	newPoint->prev = tp;
	//	newPoint->next = next;
	//	tp->next = newPoint;
	//	next->prev = newPoint;
	//}
}

void RemoveTemporaryPoints( TerrainPolygon *poly, list<TerrainPoint*> &addedPoints )
{
	TerrainPoint *tp;
	TerrainPoint *prev;
	TerrainPoint *next;
	TerrainPoint *nextnext;

	for( list<TerrainPoint*>::iterator it = addedPoints.begin(); it != addedPoints.end(); ++it )
	{
		poly->RemovePoint( (*it) );
	}
}

void EditSession::ExecuteTerrainSubtract(list<PolyPtr> &intersectingPolys)
{
	//cout << "subtracting!" << endl;
	Brush orig;
	map<TerrainPolygon*,list<TerrainPoint*>> addedPointsMap;
	for( list<PolyPtr>::iterator it = intersectingPolys.begin(); it != intersectingPolys.end(); ++it )
	{
		list<Inter> inters = (*it)->GetIntersections( polygonInProgress.get() );
		cout << "inters size: " << inters.size() << endl;
		addedPointsMap[(*it).get()] = InsertTemporaryPoints( (*it).get(), inters );

		SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>( (*it) );
		orig.AddObject( sp );

		//Add( (*it), polygonInProgress );
	}
//	return;
	

	list<PolyPtr> results;
	//cout << "calling sub!" << endl;
	Sub( polygonInProgress.get(), intersectingPolys, results );

	//before deleting the points, need to remove and delete the points of the new polygons
	//that have the same values

	for( list<PolyPtr>::iterator rit = results.begin(); 
		rit != results.end(); ++rit )
	{
		TerrainPolygon *resPoly = (*rit).get();

		for( map<TerrainPolygon*,list<TerrainPoint*>>::iterator it = addedPointsMap.begin(); it != addedPointsMap.end(); ++it )
		{
			list<TerrainPoint*> &points = (*it).second;
			for( list<TerrainPoint*>::iterator tit = points.begin(); tit != points.end(); ++tit )
			{
				Vector2i pos = (*tit)->pos;

				TerrainPoint *has = resPoly->HasPointPos( pos );
				if( has != NULL )
				{
					//delete the unneeded temp point
					resPoly->RemovePoint( has );
					delete has;
				}
			}

		}
	}


	//remove and delete points from the intersecting polys
	for( map<TerrainPolygon*,list<TerrainPoint*>>::iterator it = addedPointsMap.begin(); it != addedPointsMap.end(); ++it )
	{
		RemoveTemporaryPoints( (*it).first, (*it).second );
	}

	/*for( list<PolyPtr>::iterator it = intersectingPolys.begin(); it != intersectingPolys.end(); ++it )
	{

	}*/

	//cout << "results size: " << results.size() << endl;
	Brush resultBrush;
	for( list<PolyPtr>::iterator it = results.begin(); 
		it != results.end(); ++it )
	{
		SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>( (*it) );
		resultBrush.AddObject( sp );
	}

	for( list<PolyPtr>::iterator it = intersectingPolys.begin(); it != intersectingPolys.end(); ++it )
	{
		for( map<TerrainPoint*,std::list<ActorPtr>>::iterator 
			mit = (*it)->enemies.begin(); mit != (*it)->enemies.end(); ++mit )
		{
			for( list<PolyPtr>::iterator rit = results.begin(); 
				rit != results.end(); ++rit )
			{
				AttachActorsToPolygon( (*mit).second, (*rit).get()  );
			}
		}
	}

	//cout << "replace: " << orig.objects.size() << ", " << resultBrush.objects.size() << endl;
	Action * action = new ReplaceBrushAction( &orig, &resultBrush );
	action->Perform();
	doneActionStack.push_back( action );

	ClearUndoneActions();

	polygonInProgress->ClearPoints();
}

void EditSession::PointSelectPoint( V2d &worldPos,
	bool &emptysp )
{
	double rad = 8 * zoomMultiple;
	for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		//if its not even close dont check
		if( !(*it)->TempAABB().intersects( Rect<int>( worldPos.x - rad, worldPos.y - rad, rad * 2,
			rad * 2 ) ) )
		{
			continue;
		}

		TerrainPoint *tp = (*it)->pointStart;
		while( tp != NULL )
		{
			V2d tpPos( tp->pos.x, tp->pos.y );
			double dist = length( tpPos - worldPos );
			if( dist <= rad )
			{
				bool shift = Keyboard::isKeyPressed( Keyboard::LShift ) || Keyboard::isKeyPressed( Keyboard::RShift );
				//cout << "close enough" << endl;
				if( !tp->selected )
				{
					if( !shift )
					{
						//ClearSelectedPoints();
					}
														
					//cout << "selecting a point!" << endl;
					//select a point
					selectedPoints[(*it).get()].push_back( PointMoveInfo( tp ) );
														

					/*bool hasPoly = false;
					for( PointMap::iterator pit = selectedPoints.begin();
						pit != selectedPoints.end(); ++pit )
					{
						if( (*pit).first == (*it).get() )
						{
							hasPoly = true;
							break;
						}
					}
					if( !hasPoly )
					{
						pointPolyList.push_back( (*it).get() );
					}*/
					tp->selected = true;
					emptysp = false;
				}
				else
				{
					//point is selected

					//deselect a point
					//TerrainPolygon *removedPoly;
					//PointMap::iterator tempIt;
					//bool found = false;
					//for( PointMap::iterator it = selectedPoints.begin();
					//	it != selectedPoints.end() && !found; ++it )
					//{
					//	list<PointMoveInfo> &pList = (*it).second;
					//	for( list<PointMoveInfo>::iterator pit = pList.begin();
					//		pit != pList.end() && !found; ++pit )
					//	{
					//		if( (*pit).point == tp )
					//		{
					//			pList.erase( pit );
					//			if( (*it).second.empty() )
					//			{
					//				selectedPoints.erase( it );
					//			}
					//			found = true;
					//			
					//			//removedPoly = (*it).poly;
					//			
					//		}
					//	}
					//}

					//tp->selected = false;
					emptysp = false;
				}
													
				//selectedPoints.push_back( tp );
													
			}
			tp = tp->next;
		}
	}
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
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
	}
	else if( name == "bat" )
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
	}
	else if( name == "curveturret" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
	}
	else if( name == "stagbeetle" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
	}
	else if( name == "poisonfrog" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
	}
	else if( name == "healthfly" )
	{
		width = 32;
		height = 32;
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
	else if( name == "crawlerreverser" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
	}
	else if( name == "bosscrawler" )
	{
		width = 128;
		height = 144;
		canBeGrounded = true;
		canBeAerial = false;
	}
	else if( name == "key" )
	{
		width = 50;
		height = 50;
		canBeGrounded = false;
		canBeAerial = true;
	}
	else if( name == "player" )
	{
		width = 40;
		height = 64;
		canBeGrounded = false;
		canBeAerial = true;
	}

}

void ActorGroup::Draw( sf::RenderTarget *target )
{
	for( list<ActorPtr>::iterator it = actors.begin(); it != actors.end(); ++it )
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
	if( name == "player" )
		return;

	cout << "group size: " << actors.size() << endl;
	of << name << " " << actors.size() << endl;
	for( list<ActorPtr>::iterator it = actors.begin(); it != actors.end(); ++it )
	{
		(*it)->WriteFile( of );
	}
}



void CopyList( TerrainPoint &startPoint, 
		TerrainPoint &endPoint,
		TerrainPoint &resultStartPoint,
		TerrainPoint &resultEndPoint );

