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
#include "MainMenu.h"
#include "Background.h"
#include "Parallax.h"
#include "Enemy_Shard.h"
//#include "TerrainRender.h"

using namespace std;
using namespace sf;

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
EditSession * EditSession::currSession = NULL;


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
	:ISelectable( ISelectable::GATE ), thickLine( sf::Quads, 4 )
{
	edit = EditSession::GetSession();
	reformBehindYou = true;
	numKeysRequired = -1;
	thickLine[0].color = Color( 255, 0, 0, 255 );
	thickLine[1].color = Color( 255, 0, 0, 255 );
	thickLine[2].color = Color( 255, 0, 0, 255 );
	thickLine[3].color = Color( 255, 0, 0, 255 );
}

void GateInfo::Deactivate(EditSession *editSession, SelectPtr select)
{
	if (edit != NULL)
	{
		GateInfoPtr g = boost::dynamic_pointer_cast<GateInfo>(select);
		edit = NULL;
		editSession->gates.remove(g);
	}
}

void GateInfo::Activate(EditSession *editSession, SelectPtr select)
{
	if (edit == NULL)
	{
		GateInfoPtr g = boost::dynamic_pointer_cast<GateInfo>(select);
		edit = editSession;
		editSession->gates.push_back(g);
	}
}

void GateInfo::SetType( const std::string &gType )
{
	if( gType == "black" )
	{
		type = Gate::BLACK;
	}
	else if( gType == "keygate" )
	{
		type = Gate::KEYGATE;
		reformBehindYou = false;
	}
	else if( gType == "birdfight" )
	{
		type = Gate::CRAWLER_UNLOCK;
	}
	else if( gType == "secret" )
	{
		type = Gate::SECRET;
		//reformBehindYou = true;
	}
	else if( gType == "crawlerunlock" )
	{
		type = Gate::CRAWLER_UNLOCK;
	}
	else if( gType == "nexus1unlock" )
	{
		type = Gate::CRAWLER_UNLOCK;
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

	//will eventually spit out the gate value
	//but for now its just a constant to resave all the files
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

	//cout << "a: " << dv0.x << ", " << dv0.y << ", b: " << dv1.x << ", " << dv1.y << endl;
	
	//Color c;
	if( type == Gate::BLACK )
	{
		color = Color( 200, 200, 200 );
	}
	else if( type == Gate::KEYGATE )
	{
		if( !reformBehindYou )
			color = Color( 100, 100, 100 );
		else
		{
			color = Color( 200, 200, 200 );
		}
	}
	else if( type == Gate::SECRET)
	{
		color = Color( 255, 0, 0);
	}
	else if( type == Gate::CRAWLER_UNLOCK )
	{
		color = Color( 0, 0, 255);
	}
	thickLine[0].color = color;
	thickLine[1].color = color;
	thickLine[2].color = color;
	thickLine[3].color = color;

	thickLine[0].position = Vector2f( leftv0.x, leftv0.y );
	thickLine[1].position = Vector2f( leftv1.x, leftv1.y );
	thickLine[2].position = Vector2f( rightv1.x, rightv1.y );
	thickLine[3].position = Vector2f( rightv0.x, rightv0.y );
}

void GateInfo::Draw( sf::RenderTarget *target )
{

	CircleShape cs( 5 );
	cs.setFillColor( color );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );

	cs.setPosition( point0->pos.x, point0->pos.y );
	target->draw( cs );

	cs.setPosition( point1->pos.x, point1->pos.y );
	target->draw( cs );

	target->draw( thickLine );
}

void GateInfo::DrawPreview(sf::RenderTarget * target)
{
	sf::Vertex thickerLine[4];
	double width = 80;
	V2d dv0(point0->pos.x, point0->pos.y);
	V2d dv1(point1->pos.x, point1->pos.y);
	V2d along = normalize(dv1 - dv0);
	V2d other(along.y, -along.x);

	V2d leftv0 = dv0 - other * width;
	V2d rightv0 = dv0 + other * width;

	V2d leftv1 = dv1 - other * width;
	V2d rightv1 = dv1 + other * width;

	//cout << "a: " << dv0.x << ", " << dv0.y << ", b: " << dv1.x << ", " << dv1.y << endl;

	//Color c;
	if (type == Gate::BLACK)
	{
		color = Color::Cyan;
		//color = Color(150, 150, 150);
	}
	else if (type == Gate::KEYGATE)
	{
		color = Color::Cyan;//Color(100, 100, 100);
	}
	
	SetRectColor(thickerLine, color);

	thickerLine[0].position = Vector2f(leftv0.x, leftv0.y);
	thickerLine[1].position = Vector2f(leftv1.x, leftv1.y);
	thickerLine[2].position = Vector2f(rightv1.x, rightv1.y);
	thickerLine[3].position = Vector2f(rightv0.x, rightv0.y);

	target->draw(thickerLine, 4, sf::Quads);
}

EditSession *EditSession::GetSession()
{
	return currSession;
}

EditSession::EditSession( MainMenu *p_mainMenu )
	:w( p_mainMenu->window ), fullBounds( sf::Quads, 16 ), mainMenu( p_mainMenu )
{
	currSession = this;
	for (int i = 0; i < MAX_TERRAINTEX_PER_WORLD * 9; ++i)
	{
		terrainTextures[i] = NULL;
	}

	copiedBrush = NULL;
	mapHeader.ver1 = 1;
	mapHeader.ver2 = 5;
	mapHeader.description = "no description";
	mapHeader.collectionName = "default";
	mapHeader.gameMode = MapHeader::T_STANDARD;//"default";
	//arialFont.loadFromFile( "Breakneck_Font_01.ttf" );
	cursorLocationText.setFont( mainMenu->arial );
	cursorLocationText.setCharacterSize( 16 );
	cursorLocationText.setFillColor( Color::White );
	cursorLocationText.setPosition( 0, 0 );
	
	scaleSprite.setPosition(0, 80);
	scaleSpriteBGRect.setPosition(0, 80);
	scaleSpriteBGRect.setFillColor(Color( 255, 255, 255, 200 ));
	scaleSpriteBGRect.setSize(Vector2f( 80, 100 ));
	

	scaleText.setFont(mainMenu->arial);
	scaleText.setCharacterSize(32);
	scaleText.setFillColor(Color::White);
	scaleText.setPosition(5, 30);

	Tileset *ts_kinScale = p_mainMenu->tilesetManager.GetTileset("Kin/stand_64x64.png", 64, 64);
	scaleSprite.setTexture(*ts_kinScale->texture);
	scaleSprite.setTextureRect(ts_kinScale->GetSubRect(0));

	PoiParams::font = &mainMenu->arial;
	
	mapPreviewTex = MainMenu::mapPreviewTexture;


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
	//adding 5 for random distance buffer
	playerHalfWidth = 32;
	playerHalfHeight = 32;
	preScreenTex = mainMenu->preScreenTexture;
	showTerrainPath = false;
	
	//minAngle = .99;
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

	player.reset( new PlayerParams( Vector2i( 0, 0 ) ) );
	groups["player"]->actors.push_back( player );
	

	grassSize = 128;//64;
	grassSpacing = -60;//-40;//-20;//-10;

	AddExtraEnemy("goal");
	AddExtraEnemy("poi");
	AddExtraEnemy("key");
	AddExtraEnemy("shippickup");
	AddExtraEnemy("shard");
	AddExtraEnemy("racefighttarget");
	AddExtraEnemy("blocker");
	AddExtraEnemy("groundtrigger");
	AddExtraEnemy("airtrigger");
	AddExtraEnemy("flowerpod");
	AddExtraEnemy("nexus");

	AddWorldEnemy("comboer", 1);
	AddWorldEnemy("patroller", 1);
	AddWorldEnemy("crawler", 1);
	AddWorldEnemy("basicturret", 1);
	AddWorldEnemy("airdasher", 1);
	AddWorldEnemy("bosscrawler", 1);
	AddWorldEnemy("booster", 1);
	AddWorldEnemy("spring", 1);

	AddWorldEnemy("bat", 2);
	AddWorldEnemy("curveturret", 2);
	AddWorldEnemy("poisonfrog", 2);
	AddWorldEnemy("stagbeetle", 2);
	AddWorldEnemy("gravityfaller", 2);
	AddWorldEnemy("gravityspring", 2);
	AddWorldEnemy("bossbird", 2);


	AddWorldEnemy("pulser", 3);
	AddWorldEnemy("badger", 3);
	AddWorldEnemy("owl", 3);
	AddWorldEnemy("cactus", 3);
	AddWorldEnemy("bosscoyote", 3);

	AddWorldEnemy("spider", 4);
	AddWorldEnemy("turtle", 4);
	AddWorldEnemy("cheetah", 4);
	AddWorldEnemy("coral", 4);
	AddWorldEnemy("bosstiger", 4);
	AddExtraEnemy("rail");

	AddWorldEnemy("swarm", 5);
	AddWorldEnemy("shark", 5);
	AddWorldEnemy("overgrowth", 5);
	AddWorldEnemy("ghost", 5);
	AddWorldEnemy("bossgator", 5);

	AddWorldEnemy("specter", 6);
	AddWorldEnemy("narwhal", 6);
	AddWorldEnemy("copycat", 6);
	AddWorldEnemy("gorilla", 6);
	AddWorldEnemy("bossskeleton", 6);
}

void EditSession::AddWorldEnemy( const std::string &name, int w)
{
	worldEnemyNames[w - 1].push_back(name);
}

void EditSession::AddExtraEnemy(const std::string &name)
{
	extraEnemyNames.push_back(name);
}

TerrainPolygon *EditSession::GetPolygon(int index, int &edgeIndex )
{
	TerrainPolygon* terrain = NULL;
	if (index == -1)
	{
		terrain = inversePolygon.get();
	}
	else
	{
		int testIndex = 0;
		list<PolyPtr>::iterator it = polygons.begin();
		if (inversePolygon != NULL)
			++it;

		for (; it != polygons.end(); ++it)
		{
			if (testIndex == index)
			{
				terrain = (*it).get();
				break;
			}
			testIndex++;
		}
	}

	if (terrain == NULL)
		assert(0 && "failure terrain indexing goal");

	if (edgeIndex == terrain->numPoints - 1)
		edgeIndex = 0;
	else
		edgeIndex++;

	return terrain;
}

EditSession::~EditSession()
{
	polygonInProgress.reset();

	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		(*it).reset();
	}

	for (auto it = polygons.begin(); it != polygons.end(); ++it)
	{
		(*it).reset();
	}

	delete progressBrush;
	delete selectedBrush;

	for (auto it = allPopups.begin(); it != allPopups.end(); ++it)
	{
		delete (*it);
	}

	delete currBackground;
	for (auto it = scrollingBackgrounds.begin(); it != scrollingBackgrounds.end(); ++it)
	{
		delete (*it);
	}

	for (auto it = types.begin(); it != types.end(); ++it)
	{
		delete (*it).second;
	}

	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		delete(*it).second;
	}

	delete[] decorTileIndexes;

	for (int i = 0; i < 9 * MAX_TERRAINTEX_PER_WORLD; ++i)
	{
		if (terrainTextures[i] != NULL)
			delete terrainTextures[i];
	}
}

sf::Vector2f EditSession::SnapPointToGraph(Vector2f &p, int gridSize )
{
	return Vector2f(SnapPointToGraph( V2d( p ), gridSize ));
}

V2d EditSession::SnapPointToGraph(V2d &p, int gridSize)
{
	int adjX, adjY;

	p.x /= gridSize;
	p.y /= gridSize;

	if (p.x > 0)
		p.x += .5f;
	else if (p.x < 0)
		p.x -= .5f;

	if (p.y > 0)
		p.y += .5f;
	else if (p.y < 0)
		p.y -= .5f;

	adjX = ((int)p.x) * gridSize;
	adjY = ((int)p.y) * gridSize;

	return V2d(adjX, adjY);
}

bool EditSession::IsKeyPressed(int k)
{
	return mainMenu->IsKeyPressed(k);
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

	//if( inversePolygon != NULL && inversePolygon->finalized )
	//{
	//	if( extendingPolygon == NULL )
	//	{
	//		inversePolygon->Draw( false, zoomMultiple, preScreenTex, showPoints, NULL );
	//		//(*it)->Draw( showTerrainPath, zoomMultiple, preScreenTex, showPoints, extendingPoint );
	//	}
	//	else
	//	{
	//		if( inversePolygon == extendingPolygon )
	//		{
	//			inversePolygon->Draw( false, zoomMultiple, preScreenTex, true, extendingPoint );
	//		}
	//		else
	//		{
	//			if( extendingPolygon == NULL )
	//			{
	//				inversePolygon->Draw( false, zoomMultiple, preScreenTex, showPoints, extendingPoint );
	//			}
	//			else
	//			{
	//				inversePolygon->Draw( false, zoomMultiple, preScreenTex, false, extendingPoint );
	//			}
	//		}
	//	}
	//	
	//}
	
	if (mode == PASTE)
	{
		copiedBrush->Draw(preScreenTex);
	}

	if( cutChoose || cutChooseUp )
	{
		cutPoly0->Draw( preScreenTex );
		cutPoly1->Draw( preScreenTex );
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
	fullBounds[8].position = Vector2f(0, 0);//Vector2f( leftBound, ( topBound + boundHeight ) - boundRectWidth );
	fullBounds[9].position = Vector2f(0, 0);//Vector2f( leftBound + boundWidth, ( topBound + boundHeight ) - boundRectWidth );
	fullBounds[10].position = Vector2f(0, 0);//Vector2f( leftBound + boundWidth, ( topBound + boundHeight ) + boundRectWidth );
	fullBounds[11].position = Vector2f(0, 0);//Vector2f( leftBound, ( topBound + boundHeight ) + boundRectWidth );

	//left rect
	fullBounds[12].position = Vector2f( leftBound - boundRectWidth, topBound );
	fullBounds[13].position = Vector2f( leftBound + boundRectWidth, topBound );
	fullBounds[14].position = Vector2f( leftBound + boundRectWidth, topBound + boundHeight );
	fullBounds[15].position = Vector2f( leftBound - boundRectWidth, topBound + boundHeight );
}

bool EditSession::OpenFile()
{
	ifstream is;
	is.open( currentFile );

	if( is.is_open() )
	{
		MapHeader *mh = MapSelectionMenu::ReadMapHeader(is);
		//mapHeader.description = mh->description;
		//mapHeader.collectionName = mh->collectionName;
		//mapHeader.ver1 = mh->ver1;
		//mapHeader.ver2 = mh->ver2;

		mapHeader = *mh;
		
		envName = mh->envName;

		envWorldType = mh->envWorldType;
		
		leftBound = mh->leftBounds;
		topBound = mh->topBounds;
		boundWidth = mh->boundsWidth;
		boundHeight  = mh->boundsHeight;

		drainSeconds = mh->drainSeconds;

		Background::SetupFullBG(envName, *this, currBackground, scrollingBackgrounds);

		int numPoints = mh->numVertices;

		bossType = mh->bossFightType;

		delete mh;
		mh = NULL;

		UpdateFullBounds();

		int numDecorImages;
		is >> numDecorImages;

		for (int i = 0; i < numDecorImages; ++i)
		{
			string dName;
			is >> dName;
			int dLayer;
			is >> dLayer;

			Vector2f dPos;
			is >> dPos.x;
			is >> dPos.y;

			float dRot;
			is >> dRot;

			Vector2f dScale;
			is >> dScale.x;
			is >> dScale.y;

			int dTile;
			is >> dTile;

			Sprite dSpr;
			dSpr.setScale(dScale);
			dSpr.setRotation(dRot);
			dSpr.setPosition(dPos);

			//string fullDName = dName + string(".png");
			Tileset *ts = decorTSMap[dName];
			dSpr.setTexture(*ts->texture);
			dSpr.setTextureRect(ts->GetSubRect(dTile));
			dSpr.setOrigin(dSpr.getLocalBounds().width / 2, dSpr.getLocalBounds().height / 2);
			dSpr.setColor(Color(255, 255, 255, 100));
			//dSpr.setTexture do this after dinner


			DecorPtr dec(new DecorInfo(dSpr, dLayer, dName, dTile));
			if (dLayer > 0)
			{
				dec->myList = &decorImagesBehindTerrain;
				//decorImagesBehindTerrain.sort(CompareDecorInfo);
				//decorImagesBehindTerrain.push_back(dec);
			}
			else if (dLayer < 0)
			{
				dec->myList = &decorImagesFrontTerrain;
				//decorImagesFrontTerrain.push_back(dec);
			}
			else if (dLayer == 0)
			{
				dec->myList = &decorImagesBetween;
				//decorImagesBetween.push_back(dec);
			}

			CreateDecorImage(dec);
		}
		/*of << (*it).decorName << endl;
		of << (*it).layer << endl;
		of << (*it).spr.getPosition().x << " " << (*it).spr.getPosition().y << endl;
		of << (*it).spr.getRotation() << endl;
		of << (*it).spr.getScale().x << " " << (*it).spr.getScale().y << endl;*/

		is >> player->position.x;
		is >> player->position.y;

		//mh->topBounds = player->position.y - 1000;
		//topBound = mh->topBounds;
		
		//UpdateFullBounds();


		//int goalPosX;
		//int goalPosY; //discard these
		//is >> goalPosX;
		//is >> goalPosY;

		string hasBorderPolyStr;
		is >> hasBorderPolyStr;
		bool hasBorderPoly;
		bool hasReadBorderPoly;
		if( hasBorderPolyStr == "borderpoly" )
		{
			hasBorderPoly = true;
			hasReadBorderPoly = false;
		}
		else if( hasBorderPolyStr == "no_borderpoly" )
		{
			hasBorderPoly = false;
			hasReadBorderPoly = true;
		}
		else
		{
			cout << hasBorderPolyStr << endl;
			assert( 0 && "what is this string?" );
		}

		player->image.setPosition( player->position.x, player->position.y );
		player->SetBoundingQuad();

		
		while( numPoints > 0 )
		{
			PolyPtr poly(  new TerrainPolygon( &grassTex ) );
			

			int matWorld;
			int matVariation;
			is >> matWorld;
			is >> matVariation;
			
			poly->terrainWorldType = (TerrainPolygon::TerrainWorldType)matWorld;
			poly->terrainVariation = matVariation;
			
			if( !hasReadBorderPoly )
			{
				poly->inverse = true;
				inversePolygon = poly;
				hasReadBorderPoly = true;
			}
			/*else
			{
				polygons.push_back( poly );
			}*/
			polygons.push_back(poly);

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

				//double remainder = length( v1 - v0 ) / ( grassSize + grassSpacing );
				bool rem;
				int num = poly->GetNumGrass(polyCurr, rem);//floor( remainder ) + 1;

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

			int matWorld;
			int matVariation;
			is >> matWorld;
			is >> matVariation;

			poly->terrainWorldType = (TerrainPolygon::TerrainWorldType)matWorld;
			poly->terrainVariation = matVariation;

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

			int matWorld;
			int matVariation;
			is >> matWorld;
			is >> matVariation;
			
			poly->terrainWorldType = (TerrainPolygon::TerrainWorldType)matWorld;
			poly->terrainVariation = matVariation;

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
				ActorPtr a(NULL);
				


				ActorType *at = NULL;
				cout << "typename: " << typeName << endl;
				if( types.count( typeName ) == 0 )
				{
					cout << "TYPENAME: " << typeName << endl;
					assert( false && "bad typename" );
				}
				else
				{
					at = types[typeName];
				}
				
				at->LoadEnemy(is, a);
				
				gr->actors.push_back( a );
				a->group = gr;
			}
		}

		int numGates;
		is >> numGates;
		cout << "numgates: " << numGates << endl;
		for (int i = 0; i < numGates; ++i)
		{
			int gType;
			int poly0Index, vertexIndex0, poly1Index, vertexIndex1;
			int numKeysRequired = -1;

			string reformBehindYouStr;

			is >> gType;
			//is >> numKeysRequired;
			is >> poly0Index;
			is >> vertexIndex0;
			is >> poly1Index;
			is >> vertexIndex1;
			is >> reformBehindYouStr;
			bool reformBehindYou;
			if (reformBehindYouStr == "+reform")
			{
				reformBehindYou = true;
			}
			else if (reformBehindYouStr == "-reform")
			{
				reformBehindYou = false;
			}
			else
			{
				assert(false);
			}

			int testIndex = 0;
			PolyPtr terrain0(NULL);
			PolyPtr terrain1(NULL);
			bool first = true;

			if (poly0Index == -1)
			{
				terrain0 = inversePolygon;
			}
			if (poly1Index == -1)
			{
				terrain1 = inversePolygon;
			}
			for (list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it)
			{
				if ((*it)->inverse) continue;

				if (terrain0 != NULL && terrain1 != NULL)
					break;

				if (testIndex == poly0Index && terrain0 == NULL)
				{
					terrain0 = (*it);

					if (first)
						first = false;
					else
						break;
				}
				if (testIndex == poly1Index && terrain1 == NULL)
				{
					terrain1 = (*it);

					if (first)
						first = false;
					else
						break;
				}
				testIndex++;
			}
			if (terrain0 == NULL || terrain1 == NULL)
			{
				int zzzerwr = 56;
			}
			//PolyPtr poly(  new TerrainPolygon( &grassTex ) );
			GateInfoPtr gi( new GateInfo );
			//GateInfo *gi = new GateInfo;
			gi->reformBehindYou = reformBehindYou;
			gi->numKeysRequired = numKeysRequired;
			gi->poly0 = terrain0;
			gi->poly1 = terrain1;
			gi->vertexIndex0 = vertexIndex0;
			gi->vertexIndex1 = vertexIndex1;
			gi->type = (Gate::GateType)gType;
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
		cout << "filename: " << currentFile << endl;
		assert( false && "error getting file to edit " );

		return false;
	}

	//grassTex.loadFromFile( "Resources/Env/placeholdergrass_22x22.png" );
	grassTex.loadFromFile("Resources/Env/grass_128x128.png");

	return true;
	
}

void EditSession::WriteFile(string fileName)
{
	bool hasGoal = false;
	for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it )
	{
		ActorGroup *group = (*it).second;
		for( list<ActorPtr>::iterator it2 = group->actors.begin(); it2 != group->actors.end(); ++it2 )
		{
			if( (*it2)->type->IsGoalType() )
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

	mapHeader.leftBounds = leftBound;
	mapHeader.topBounds = topBound;
	mapHeader.boundsWidth = boundWidth;
	mapHeader.boundsHeight = boundHeight;

	mapHeader.bossFightType = bossType;
	/*int totalPoints = 0;
	for (auto it = polygons.begin(); it != polygons.end(); ++it)
	{
		totalPoints += (*it)->numPoints;
	}*/
	


	mapHeader.shardNameList.clear();
	ShardParams *sp = NULL;
	int numShards = 0;
	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		std::list<ActorPtr> &aList = (*it).second->actors;
		for (auto ait = aList.begin(); ait != aList.end(); ++ait)
		{
			if ((*ait)->type->name == "shard")
			{
				numShards++;
				sp = (ShardParams*)(ait->get());
				mapHeader.shardNameList.push_back(sp->shardStr);
			}
		}
	}

	mapHeader.numShards = numShards;

	mapHeader.drainSeconds = drainSeconds;

	mapHeader.envName = envName;

	mapHeader.envWorldType = envWorldType;

	ofstream of;
	of.open( fileName );//+ ".brknk" );

	mapHeader.Save(of);

	

	//need to make an edit popup where you can set the numbers manually or edit with the mouse

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

	int totalDecor = 0;
	totalDecor += decorImagesBehindTerrain.size() + decorImagesBetween.size() + decorImagesFrontTerrain.size();

	of << totalDecor << endl;

	for (auto it = decorImagesBehindTerrain.begin(); it != decorImagesBehindTerrain.end(); ++it)
	{
		(*it)->WriteFile(of);
	}

	for (auto it = decorImagesBetween.begin(); it != decorImagesBetween.end(); ++it)
	{
		(*it)->WriteFile(of);
	}

	for (auto it = decorImagesFrontTerrain.begin(); it != decorImagesFrontTerrain.end(); ++it)
	{
		(*it)->WriteFile(of);
	}

	of << player->position.x << " " << player->position.y << endl;

	bool quitLoop = false;


	int writeIndex = 0;
	
	if( inversePolygon != NULL )
	{
		of << "borderpoly" << endl;
		//polygons.remove(inversePolygon);
		//polygons.push_front( inversePolygon );
		writeIndex = -1;

		inversePolygon->writeIndex = writeIndex;
		++writeIndex;

		of << inversePolygon->terrainWorldType << " "
			<< inversePolygon->terrainVariation << endl;

		cout << "numpoints: " << inversePolygon->numPoints << endl;
		of << inversePolygon->numPoints << endl;

		for (TerrainPoint *pcurr = inversePolygon->pointStart; pcurr != NULL; pcurr = pcurr->next)
		{
			of << pcurr->pos.x << " " << pcurr->pos.y << endl; // << " " << (int)(*it2).special << endl;
		}

		WriteGrass(inversePolygon, of);

	}
	else
	{
		of << "no_borderpoly" << endl;
	}
	cout << "writing to file with : " << polygons.size() << " polygons" << endl;
	for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		if ((*it)->inverse) continue;

		//cout << "layerrr: " << (*it)->layer << ", pathsize: " << (*it)->path.size() << endl;
		if( (*it)->layer == 0 && (*it)->path.size() < 2 )
		{
			cout << "writing polygon of write index: " << writeIndex << endl;
			(*it)->writeIndex = writeIndex;
			++writeIndex;

			of << (*it)->terrainWorldType << " " 
				<< (*it)->terrainVariation << endl;

			cout << "numpoints: " << (*it)->numPoints << endl;
			of <<  (*it)->numPoints << endl;

			for( TerrainPoint *pcurr = (*it)->pointStart;  pcurr != NULL; pcurr = pcurr->next )
			{
				of << pcurr->pos.x << " " << pcurr->pos.y << endl; // << " " << (int)(*it2).special << endl;
			}

			WriteGrass( (*it), of );
		}
	}	
	/*if( inversePolygon != NULL )
	{
		polygons.pop_front();
	}*/

	of << movingPlatCount << endl;

	writeIndex = 0;
	for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		if( (*it)->layer == 0 && (*it)->path.size() >= 2 )
		{
			(*it)->writeIndex = writeIndex;
			++writeIndex;

			of << (*it)->terrainWorldType << " " 
				<< (*it)->terrainVariation << endl;
			
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
		if ((*it)->inverse) continue;

		if( (*it)->layer == 1 )// && (*it)->path.size() < 2 )
		{
			//writeindex doesnt matter much for these for now
			(*it)->writeIndex = writeIndex;
			++writeIndex;

			of << (*it)->terrainWorldType << " " 
				<< (*it)->terrainVariation << endl;

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
		ActorGroup *ag = (*it).second;
		if( ag->name == "player" )
			continue;

		//cout << "group size: " << ag->actors.size() << endl;
		of << ag->name << " " << ag->actors.size() << endl;
		for( list<ActorPtr>::iterator it = ag->actors.begin(); it != ag->actors.end(); ++it )
		{
			if( (*it)->type == types["poi"] )
			{
				(*it)->WriteFile( of );
			}
		}
	}

	for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it )
	{
		ActorGroup *ag = (*it).second;
		if( ag->name == "player" )
			continue;

		//cout << "group size: " << ag->actors.size() << endl;
		//of << ag->name << " " << ag->actors.size() << endl;
		for( list<ActorPtr>::iterator it = ag->actors.begin(); it != ag->actors.end(); ++it )
		{
			if( (*it)->type != types["poi"] )
			{
				(*it)->WriteFile( of );
			}
		}
		//(*it).second->WriteFile( of );
		
	}

	of << gates.size() << endl;
	for( list<GateInfoPtr>::iterator it = gates.begin(); it != gates.end(); ++it )
	{
		(*it)->WriteFile( of );
	}

	CreatePreview(Vector2i( 1920 / 2 - 48, 1080 / 2 - 48 ));
	//CreatePreview(Vector2i(960 * 1.25f, 540 * ));

	//enemies here


}

void EditSession::WriteGrass( PolyPtr poly, ofstream &of )
{
	int edgesWithSegments = 0;

	VertexArray &grassVa = *poly->grassVA;

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

		bool rem;
		int num = poly->GetNumGrass(curr, rem);//floor( remainder ) + 1;

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
				if( gPtr == NULL )//|| (j == num - 1 && rem ))
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

bool EditSession::PointOnLine( V2d &pos, V2d &p0, V2d &p1, double width)
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


void EditSession::TryPlaceGatePoint(V2d &pos)
{
	modifyGate = NULL;
	bool found = false;

	for (list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end() && !found; ++it)
	{
		//extended aabb 
		TerrainPoint *closePoint = (*it)->GetClosePoint(8 * zoomMultiple, pos);

		if (closePoint != NULL)
		{
			if (gatePoints == 0)
			{
				for (list<GateInfoPtr>::iterator git = gates.begin(); git != gates.end() && !found; ++git)
				{
					if ((*git)->point0 == closePoint || (*git)->point1 == closePoint)
					{
						GateInfoPtr gi = (*git);

						view.setCenter(closePoint->pos.x, closePoint->pos.y);
						preScreenTex->setView(view);

						modifyGate = gi;


						found = true;
					}
				}

				if (!found)
				{
					found = true;
					gatePoints = 1;
					testGateInfo.poly0 = (*it);
					testGateInfo.point0 = closePoint;
					testGateInfo.vertexIndex0 = (*it)->GetPointIndex(closePoint);
				}
			}
			else
			{
				found = true;
				gatePoints = 2;

				testGateInfo.poly1 = (*it);
				testGateInfo.point1 = closePoint;
				testGateInfo.vertexIndex1 = (*it)->GetPointIndex(closePoint);
				view.setCenter(testGateInfo.point1->pos.x, testGateInfo.point1->pos.y);
				preScreenTex->setView(view);
			}
		}
	}

	if (!found)
	{
		gatePoints = 0;
	}
}

//returns true if attach is successful
ActorParams * EditSession::AttachActorToPolygon( ActorPtr actor, TerrainPolygon *poly )
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

		actorPos = aCurr + normalize(aNext - aCurr) * actorQuant;//V2d( actor->image.getPosition() );//
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

			//might need to make sure it CAN be grounded
			assert( actor->groundInfo != NULL ); 


			//ActorPtr newActor( actor->Copy() );
			ActorParams *newActor = actor->Copy();
			newActor->groundInfo = NULL;
			//newActor->UnAnchor( newActor );
			//newActor->groundInfo = NULL;
			newActor->AnchorToGround(gi);
			//GroundInfo *oldGI = actor->groundInfo;

			//GroundInfo old = *actor->groundInfo;
			assert(gi.edgeStart != NULL);
			assert(newActor != NULL);
			//assert( newActor.get() != NULL );

			//newActor->UnAnchor( newActor );
			//newActor->groundInfo->edgeStart = gi.edgeStart;
			//newActor->groundInfo->ground = gi.ground;
			//newActor->groundInfo->groundQuantity= gi.groundQuantity;

			//poly->enemies[p].push_back( newActor );

			//tempActors.push_back( newActor );

			return newActor;
			//b.AddObject( newActor );
			//actor->UnAnchor( actor );
			//actor->groundInfo->ground->enemies[actor->groundInfo->edgeStart].remove( actor );
			//actor->AnchorToGround( gi );

			//old.ground->enemies[old.edgeStart].remove( actor );
			
			//return true;
			//break;
		}
	}

	return NULL;
	//return false;
}

void EditSession::AttachActorsToPolygon( list<ActorPtr> &actors, TerrainPolygon *poly )
{
	//cout << "attemping to attach actors" << endl;
	/*bool res;
	for( list<ActorPtr>::iterator it = actors.begin(); it != actors.end(); ++it )
	{
		res = AttachActorToPolygon( (*it), poly );
		if( res )
		{
			cout << "saved an actor!" << endl;
		}
		else
		{
			cout << "totally didn't save an actor! QQ" << endl;
		}
	}*/
}

void EditSession::Extend(boost::shared_ptr<TerrainPolygon> extension,
	boost::shared_ptr<TerrainPolygon> poly )
{
	bool extendCW = extension->IsClockwise();
	poly->FixWinding();
	//both of these polygons must be confirmed to be valid already in their individual shape

	TerrainPoint *endTransfer = NULL;

	if (!extendCW) //ccw
	{
		for (TerrainPoint *t = poly->pointEnd; t != NULL; t = t->prev)
		{
			if (t->pos == extension->pointStart->pos)
			{
				endTransfer = t;
			}
		}
	}
	else //cw
	{
		for (TerrainPoint *t = poly->pointStart; t != NULL; t = t->next)
		{
			if (t->pos == extension->pointEnd->pos)
			{
				endTransfer = t;
			}
		}
	}

	bool inverse = false;
	if ( poly->inverse)
	{
		inverse = true;
	}

	TerrainPolygon z(&grassTex);
	//1: choose start point

	Vector2i startPoint;
	bool startPointFound = false;
	bool firstPolygon = true;


	PolyPtr currentPoly = NULL;
	PolyPtr otherPoly = NULL;
	TerrainPoint *currP = extension->pointStart;

	TerrainPoint *nextP = NULL;
	TerrainPoint *startP = NULL;

	TerrainPoint *outStart = NULL;
	TerrainPoint *outEnd = NULL;

	PolyPtr minPoly = NULL;
	
	currentPoly = extension;
	otherPoly = poly;

	currP = startP;
	bool firstRun = true;
	TerrainPoint *tp = new TerrainPoint(currP->pos, false);
	z.AddPoint(tp);

	Vector2i startSegPos;// = currP->pos;
	Vector2i endSegPos;

	startSegPos = currP->pos;
	while (true)
	{

		if (!extendCW) //ccw extension
		{
			nextP = currP->prev;
			if (nextP == NULL)
			{
				if (currentPoly == extension && endTransfer != NULL)
				{
					currentPoly = poly;
					otherPoly = extension;
					currP = endTransfer;
					nextP = currP->prev;
					if (nextP == NULL)
					{
						nextP = currentPoly->pointEnd;
					}
				}
				else
				{
					nextP = currentPoly->pointEnd;
				}

			}
		}
		else
		{
			nextP = currP->next;
			if (nextP == NULL)
			{
				if (currentPoly == extension && endTransfer != NULL)
				{
					currentPoly = poly;
					otherPoly = extension;
					currP = endTransfer;
					nextP = currP->next;
					if (nextP == NULL)
					{
						nextP = currentPoly->pointStart;
					}
				}
				else
				{
					nextP = currentPoly->pointStart;
				}

			}
		}

		
		


		LineIntersection li = otherPoly->GetSegmentFirstIntersection(startSegPos, nextP->pos, outStart, outEnd);

		//there was an intersection
		if (!li.parallel)
		{
			Vector2i intersectPoint = Vector2i(round(li.position.x), round(li.position.y));

			PolyPtr temp = currentPoly;
			currentPoly = otherPoly;
			otherPoly = temp;

			TerrainPoint *tp = new TerrainPoint(intersectPoint, false);
			z.AddPoint(tp);

			startSegPos = intersectPoint;

			if (!extendCW)
			{
				currP = outEnd;
			}
			else
			{
				currP = outStart;
			}
			
			
			

		}
		//no intersection
		else
		{
			if (!firstRun && nextP->pos == startP->pos)
				break;

			TerrainPoint *tp = new TerrainPoint(nextP->pos, false);
			z.AddPoint(tp);

			if (!extendCW)
			{
				currP = currP->prev;
				if (currP == NULL)
				{
					if (currentPoly == extension && endTransfer != NULL)
					{
						currentPoly = poly;
						otherPoly = extension;
						currP = endTransfer;
					}
					else
					{
						currP = currentPoly->pointEnd;
					}
				}
			}
			else
			{
				currP = currP->next;
				if (currP == NULL)
				{
					if (currentPoly == extension && endTransfer != NULL)
					{
						currentPoly = poly;
						otherPoly = extension;
						currP = endTransfer;
					}
					else
					{
						currP = currentPoly->pointStart;
					}
				}
			}
			
			


			startSegPos = currP->pos;

			/*if (currentPoly->enemies.count(curr) > 0)
			{
			list<ActorPtr> &en = z.enemies[tp];
			en = currentPoly->enemies[curr];
			}*/
		}

		firstRun = false;
	}

	poly->Reset();

	for (TerrainPoint *zit = z.pointStart; zit != NULL; zit = zit->next)
	{
		TerrainPoint *tp = new TerrainPoint(*zit);
		if (tp->gate != NULL)
		{
			//cout << "new polygon will have gate" << endl;
			if (zit == tp->gate->point0)
			{
				tp->gate->point0 = tp;
				tp->gate->poly0 = poly;
			}
			else
			{
				tp->gate->point1 = tp;
				tp->gate->poly1 = poly;
			}
		}
		poly->AddPoint(tp);
	}


	//cout << "about to check for enemy stuff" << endl;
	//for (TerrainPoint *bit = brush->pointStart; bit != NULL; bit = bit->next)
	//{
	//	//cout << "z enems: " << z.enemies.count( zit ) << endl;
	//	//cout << "enems: " << 
	//	//if( z.enemies.count( zit ) > 0 )
	//	if (brush->enemies.count(bit) > 0)
	//	{
	//		//list<ActorPtr> &en = poly->enemies[tp];
	//		//en = z.enemies[zit];
	//		list<ActorPtr> &en = brush->enemies[bit];//z.enemies[zit];

	//		AttachActorsToPolygon(en, poly.get());
	//	}
	//}

	poly->inverse = inverse;
	poly->Finalize();

	return;
}

double GetClockwiseAngleDifference(const V2d &A, const V2d &B)
{
	double angleA = atan2(-A.y, A.x);
	if (angleA < 0)
	{
		angleA += PI * 2;
	}
	double angleB = atan2(-B.y, B.x);
	double xxx = atan2(0, 1.0);
	double yyy = atan2(0, -1.0);
	double ggg = atan2(-1.0, 0);
	double fff = atan2(1.0, 0);
	if (angleB < 0)
	{
		angleB += PI * 2;
	}

	if (angleA > angleB)
	{
		return angleA - angleB;
	}
	else if (angleA < angleB)
	{
		return ((2 * PI) - angleB) + angleA;
		/*if (angleA == 0)
		{
			return angleB;
		}
		else
		{
			return angleB + ((PI * 2) - angleA);
		}*/
		

	}

	/*if (angle < 0)
	{
		angle += PI * 2;
	}*/
}



//int EditSession::CompareAngle(bool cw, V2d &origDir,
//	V2d stayDir, V2d otherDir);
//{
//	double stay = GetClockwiseAngleDifference(origDir, stayDir);
//	double other = GetClockwiseAngleDifference(origDir, otherDir);
//
//	if (stay < other)
//	{
//		return -1;
//	}
//	else if( stay == other )
//	{
//		return 0; 
//	}
//	else
//	{
//		return 1;
//	}
//}
//

TerrainPoint * EditSession::GetNextAddPoint( TerrainPoint *previousPoint, sf::Vector2i &startSegPos, TerrainPoint *&currP,
	PolyPtr &currentPoly, PolyPtr &otherPoly, bool &skipBorderCase, bool &replaceLastPoint )
{

	TerrainPoint *nextP = NULL;
	replaceLastPoint = false;
	TerrainPoint *outPoint = NULL;
	TerrainPoint *outStart = NULL;
	TerrainPoint *outEnd = NULL;
	//assert(outPoly->numPoints <= brush->numPoints + poly->numPoints);
	if (otherPoly->inverse)
	{
		nextP = currP->prev;
		if (nextP == NULL)
			nextP = currentPoly->pointEnd;
	}
	else
	{
		nextP = currP->next;
		if (nextP == NULL)
			nextP = currentPoly->pointStart;
	}

	LineIntersection li = otherPoly->GetSegmentFirstIntersection(startSegPos, nextP->pos, outStart, outEnd, skipBorderCase);
	skipBorderCase = false;

	bool pointFromOtherLine = !li.parallel && length(li.position - V2d(outStart->pos)) != 0
		&& length(li.position - V2d(outEnd->pos)) != 0;
	bool nextPointOnLine = !li.parallel && (length(li.position - V2d(nextP->pos)) == 0.0);
	bool currPointOnLine = !li.parallel && (length(li.position - V2d(startSegPos)) == 0.0);

	bool onOtherStart = !li.parallel && (length(li.position - V2d(outStart->pos)) == 0.0);
	bool onOtherEnd = !li.parallel && (length(li.position - V2d(outEnd->pos)) == 0.0);

	if (currPointOnLine) //collision at start point
	{
		//could be a line or point
		//	if (onOtherStart || onOtherEnd) //point/point collision
		{
			TerrainPoint *prev = currP->prev;
			if (prev == NULL)
				prev = currentPoly->pointEnd;

			V2d cp = V2d(startSegPos);
			V2d np = V2d(nextP->pos);
			V2d pp = V2d(prev->pos);

			V2d startDir, otherTest;

			V2d currPrevTest = normalize(pp - cp);
			V2d currTest = normalize(np - cp);

			TerrainPoint *otherCurr = NULL;
			TerrainPoint *otherPrev = NULL;
			TerrainPoint *otherNext = NULL;

			if (previousPoint == NULL) //startPoint
			{
				V2d otherPrevTest;

				//TerrainPoint *otherPrev = NULL;

				TerrainPoint *otherNext = NULL;

				if (onOtherEnd)
				{
					otherPrev = outStart;
					otherCurr = outEnd;
					otherNext = otherCurr->next;
					if (otherNext == NULL)
						otherNext = otherPoly->pointStart;
				}
				else
					//if (onOtherStart)
				{
					otherPrev = outStart->prev;
					if (otherPrev == NULL)
						otherPrev = otherPoly->pointEnd;

					otherCurr = outStart;
					otherNext = outEnd;
				}

				otherPrevTest = normalize(V2d(otherPrev->pos) - cp);
				otherTest = normalize(V2d(otherNext->pos) - cp);

				if (otherPoly->inverse)
				{
					if (GetVectorAngleDiffCCW(currPrevTest, currTest) <= GetVectorAngleDiffCCW(otherPrevTest, currTest)) //we startAngle on mine
					{
						startDir = currPrevTest;
					}
					else
					{
						startDir = otherPrevTest;
					}
				}
				else
				{
					if (GetVectorAngleDiffCW(currPrevTest, currTest) <= GetVectorAngleDiffCW(otherPrevTest, currTest)) //we startAngle on mine
					{
						startDir = currPrevTest;
					}
					else
					{
						startDir = otherPrevTest;
					}
				}
			}
			else
			{
				if (onOtherEnd)
				{
					otherPrev = outStart;
					otherCurr = outEnd;
					otherNext = otherCurr->next;
					if (otherNext == NULL)
						otherNext = otherPoly->pointStart;
				}
				else
				{
					otherPrev = outStart->prev;
					if (otherPrev == NULL)
						otherPrev = otherPoly->pointEnd;

					otherCurr = outStart;
					otherNext = outEnd;
				}

				otherTest = normalize(V2d(otherNext->pos) - cp);
				startDir = normalize(V2d( previousPoint->pos) - cp);
			}

			if (otherPoly->inverse)
			{
				if (GetVectorAngleDiffCCW(startDir, currTest) <= GetVectorAngleDiffCCW(startDir, otherTest))
				{
					//stay
					skipBorderCase = true;

					return NULL;
				}
				else
				{
					//switch

					skipBorderCase = true;

					PolyPtr temp = currentPoly;
					currentPoly = otherPoly;
					otherPoly = temp;

					currP = otherCurr;
					//startSegPos = currP->pos;
					return NULL;

				}
			}
			else
			{
				if (GetVectorAngleDiffCW(startDir, currTest) <= GetVectorAngleDiffCW(startDir, otherTest))
				{
					//stay
					skipBorderCase = true;
					return NULL;
				}
				else
				{
					//switch

					skipBorderCase = true;

					PolyPtr temp = currentPoly;
					currentPoly = otherPoly;
					otherPoly = temp;

					currP = otherCurr;
					//startSegPos = currP->pos;
					return NULL;

				}
			}
		}
	}

	//there was an intersection
	if (!li.parallel )
	{
		Vector2i intersectPoint;
		if (length(li.position - V2d(outEnd->pos)) < 8.0)
		{

			intersectPoint = outEnd->pos;
			currP = outEnd;
		}
		else if (length(li.position - V2d(outStart->pos)) < 8.0)
		{
			//V2d enterDir = 


			//do the angle thing here! check difference. dont need the point map this way

			intersectPoint = outStart->pos;
			currP = outStart;
		}
		else if (length(li.position - V2d(startSegPos)) < 8.0)
		{
			V2d middlePos = (V2d(startSegPos) + li.position) / 2.0;
			intersectPoint.x = round(middlePos.x);
			intersectPoint.y = round(middlePos.y);

			//TerrainPoint *endPoint = outPoly->pointEnd;
			//outPoly->RemovePoint(outPoly->pointEnd);

			currP = outStart;

			//delete endPoint;

			skipBorderCase = true;

			replaceLastPoint = true;
			//TODO: memory leak
		}

		else
		{
			//if (nextPointOnLine)
			{
				skipBorderCase = true;
			}

			intersectPoint = Vector2i(round(li.position.x), round(li.position.y));
			if (currentPoly->inverse)
			{
				currP = outEnd;
			}
			else
			{
				currP = outStart;
			}
		}

		PolyPtr temp = currentPoly;
		currentPoly = otherPoly;
		otherPoly = temp;

		TerrainPoint *tp = new TerrainPoint(intersectPoint, false);

		startSegPos = intersectPoint;

		return tp;
		//outPoly->AddPoint(tp);

		
	}
	//no intersection
	else
	{
		TerrainPoint *tp = new TerrainPoint(nextP->pos, false);
		//outPoly->AddPoint(tp);

		if (otherPoly->inverse)
		{
			currP = currP->prev;
			if (currP == NULL)
			{
				currP = currentPoly->pointEnd;
			}
		}
		else
		{
			currP = currP->next;
			if (currP == NULL)
			{
				currP = currentPoly->pointStart;
			}
		}
		startSegPos = currP->pos;

		return tp;
	}

	assert(0);
}

EditSession::AddResult EditSession::Add( PolyPtr brush, PolyPtr poly, TerrainPolygon *&outPoly, list<boost::shared_ptr<GateInfo>> &gateInfoList )
{	
	TerrainPoint *gCurr = poly->pointStart;
	bool okGate = true;
	for (; gCurr != NULL; gCurr = gCurr->next)
	{
		if (gCurr->gate != NULL)
		{
			okGate = true;
			for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
			{
				if ((*it) == gCurr->gate)
				{
					okGate = false;
					break;
				}
			}
			if (okGate)
			{
				gateInfoList.push_back(gCurr->gate);
			}
		}
	}

	map<TerrainPoint*, DetailedInter> pointInterMap;
	//map<TerrainPoint*, Inter> pointInterMapFromPoly;
	outPoly = NULL;
	brush->FixWinding();
	poly->FixWinding();

	list<DetailedInter> brushInters; 
	brush->GetDetailedIntersections(poly.get(), brushInters);

	//might add 2 inters for an intersection right on a point
	for (auto it = brushInters.begin(); it != brushInters.end(); ++it)
	{
		TerrainPoint *itNext = (*it).inter.point->next;
		if (itNext == NULL)
			itNext = brush->pointStart;

		if (length((*it).inter.position - V2d((*it).inter.point->pos)) < 1.0)
		{
			pointInterMap[(*it).inter.point] = (*it);
		}
		else if (length((*it).inter.position - V2d(itNext->pos)) < 1.0)
		{
			pointInterMap[itNext] = (*it);
		}
	}

	//referenced by the brush pointer
	map< TerrainPoint*, TerrainPoint*> samePointMap;
	for (TerrainPoint *tp = brush->pointStart; tp != NULL; tp = tp->next)
	{
		TerrainPoint *same = poly->GetSamePoint(tp->pos);
		if (same != NULL)
		{
			samePointMap[tp] = same;
			//samePointMap[same] = tp;
		}
	}
	outPoly = NULL;
	brush->FixWinding();
	poly->FixWinding();
	//both of these polygons must be confirmed to be valid already in their individual shape
	outPoly = new TerrainPolygon(&grassTex);
	bool inverse = false;
	if( brush->inverse || poly->inverse )
	{
		list<PolyPtr> results;
		list<PolyPtr> orig;
		
		if (brush->inverse)
		{
			orig.push_back(brush);
			brush->inverse = false;

			Sub(poly, orig, results);

			outPoly->CopyPoints(results.front().get());
			//results.front()->CopyPoints(outPoly->pointStart, outPoly->pointEnd);
			//outPoly->CopyPoints(results.front()->pointStart, results.front()->pointEnd);
			outPoly->inverse = true;
			brush->inverse = true;
		}
		else
		{
			orig.push_back(poly);
			poly->inverse = false;

			Sub(brush, orig, results);

			//outPoly->CopyPoints(results.front()->pointStart, results.front()->pointEnd);
			outPoly->CopyPoints(results.front().get());
			//results.front()->CopyPoints(outPoly->pointStart, outPoly->pointEnd);
			outPoly->inverse = true;
			//results.front()->CopyPoints(outPoly->pointStart, outPoly->pointEnd);
			poly->inverse = true;



			
		}
		//return;
		inverse = true;

		outPoly->RemoveSlivers(PI / 10.0);
		outPoly->AlignExtremes(PRIMARY_LIMIT);


		//outPoly->inverse = inverse;
		outPoly->Finalize();

		if (brush->inverse)
		{
			brush->FixWindingInverse();
		}
		else
		{
			brush->FixWinding();
		}

		if (poly->inverse)
		{
			poly->FixWindingInverse();
		}
		else
		{
			poly->FixWinding();
		}
		return AddResult::ADD_SUCCESS;
	}

	
	//TerrainPolygon z( &grassTex );
	//1: choose start point

	Vector2i startPoint;
	bool startPointFound = false;
	bool firstPolygon = true;


	PolyPtr currentPoly = NULL;
	PolyPtr otherPoly = NULL;
	TerrainPoint *currP = poly->pointStart;
	
	TerrainPoint *nextP = NULL;
	TerrainPoint *startP = NULL;
	sf::Vector2i startSegPosStart;

	TerrainPoint *outStart = NULL;
	TerrainPoint *outEnd = NULL;

	PolyPtr minPoly = NULL;

	//get extreme left point. this way it cant be in a cave
	TerrainPoint *leftPoint = currP;
	currentPoly = poly;
	otherPoly = brush;

	for (; currP != NULL; currP = currP->next)
	{
		if (currP->pos.x < leftPoint->pos.x )
		{
			leftPoint = currP;
			currentPoly = poly;
			otherPoly = brush;
		}
	}
	currP = brush->pointStart;
	for (; currP != NULL; currP = currP->next)
	{
		// the <= is there because brush should be prioritized 
		if (currP->pos.x <= leftPoint->pos.x)
		{
			leftPoint = currP;
			currentPoly = brush;
			otherPoly = poly;
		}
	}

	if (inverse)
	{
		if (currentPoly->inverse)
		{
			startP = leftPoint;
			startSegPosStart = startP->pos;
		}
		else
		{
			currP = leftPoint;

			while (true)
			{
				nextP = currP->next;
				if (nextP == NULL)
					nextP = currentPoly->pointStart;

				//might not account for hitting pointsin this area yet
				LineIntersection li = otherPoly->GetSegmentFirstIntersection(currP->pos, nextP->pos, outStart, outEnd);

				if (!li.parallel)
				{
					startP = outEnd;
					PolyPtr temp = otherPoly;
					otherPoly = currentPoly;
					currentPoly = temp;
					startSegPosStart = Vector2i( round( li.position.x ), round( li.position.y ) );
					break;
				}

				currP = currP->next;
				if (currP == NULL)
					currP = currentPoly->pointStart;
			}
			assert(startP != NULL);
		}
	}
	else
	{
		startP = leftPoint;
		startSegPosStart = startP->pos;
	}

	currP = startP;
	bool firstRun = true;
	TerrainPoint *tp = new TerrainPoint(startSegPosStart, false);
	outPoly->AddPoint(tp);

	Vector2i startSegPos;// = currP->pos;
	Vector2i endSegPos;

	startSegPos = startSegPosStart;//currP->pos;



	bool skipBorderCase = false;
	while (true)
	{

		//assert(outPoly->numPoints <= brush->numPoints + poly->numPoints);
		if (inverse && !currentPoly->inverse)
		{
			nextP = currP->prev;
			if (nextP == NULL)
				nextP = currentPoly->pointEnd;
		}
		else
		{
			nextP = currP->next;
			if (nextP == NULL)
				nextP = currentPoly->pointStart;
		}
		  
		if (!firstRun && startSegPos == startSegPosStart)
		{
			break;
		}
		LineIntersection li = otherPoly->GetSegmentFirstIntersection(startSegPos, nextP->pos, outStart, outEnd, skipBorderCase);
		skipBorderCase = false;

		bool pointFromOtherLine = !li.parallel && length(li.position - V2d(outStart->pos)) != 0
			&& length(li.position - V2d(outEnd->pos)) != 0;
		bool nextPointOnLine = !li.parallel && (length(li.position - V2d(nextP->pos)) < 0.0);
		bool currPointOnLine = !li.parallel && (length(li.position - V2d(startSegPos)) == 0.0);
		
		bool onOtherStart = !li.parallel && (length(li.position - V2d(outStart->pos)) == 0.0);
		bool onOtherEnd = !li.parallel && (length(li.position - V2d(outEnd->pos)) == 0.0);
		
		if (nextPointOnLine && nextP->pos == startP->pos)
			break;
		//if (nextPointOnLine)
		//	li.parallel = true;

		if (currPointOnLine) //collision at start point
		{
			//could be a line or point
		//	if (onOtherStart || onOtherEnd) //point/point collision
			
			{
				TerrainPoint *prev = currP->prev;
				if (prev == NULL)
					prev = currentPoly->pointEnd;

				V2d cp = V2d(startSegPos);
				V2d np = V2d(nextP->pos);
				V2d pp = V2d(prev->pos);

				V2d startDir, otherTest;

				V2d currPrevTest = normalize(pp - cp);
				V2d currTest = normalize(np - cp);

				TerrainPoint *otherCurr = NULL;
				TerrainPoint *otherPrev = NULL;
				TerrainPoint *otherNext = NULL;

				if (outPoly->numPoints == 1) //startPoint
				{
					V2d otherPrevTest;

					//TerrainPoint *otherPrev = NULL;
					
					TerrainPoint *otherNext = NULL;

					if (onOtherEnd)
					{
						otherPrev = outStart;
						otherCurr = outEnd;
						otherNext = otherCurr->next;
						if (otherNext == NULL)
							otherNext = otherPoly->pointStart;
					}
					else
						//if (onOtherStart)
					{
						otherPrev = outStart->prev;
						if (otherPrev == NULL)
							otherPrev = otherPoly->pointEnd;

						otherCurr = outStart;
						otherNext = outEnd;
					} 

					otherPrevTest = normalize(V2d(otherPrev->pos) - cp);
					otherTest = normalize(V2d(otherNext->pos) - cp );

					if (otherPoly->inverse)
					{
						if (GetVectorAngleDiffCCW(currPrevTest, currTest) <= GetVectorAngleDiffCCW(otherPrevTest, currTest)) //we startAngle on mine
						{
							startDir = currPrevTest;
						}
						else
						{
							startDir = otherPrevTest;
						}
					}
					else
					{
						if (GetVectorAngleDiffCW(currPrevTest, currTest) <= GetVectorAngleDiffCW(otherPrevTest, currTest)) //we startAngle on mine
						{
							startDir = currPrevTest;
						}
						else
						{
							startDir = otherPrevTest;
						}
					}

					
				}
				else
				{
					
					if (onOtherEnd)
					{
						otherPrev = outStart;
						otherCurr = outEnd;
						otherNext = otherCurr->next;
						if (otherNext == NULL)
							otherNext = otherPoly->pointStart;
					}
					else
					{
						otherPrev = outStart->prev;
						if (otherPrev == NULL)
							otherPrev = otherPoly->pointEnd;

						otherCurr = outStart;
						otherNext = outEnd;
					}

					otherTest = normalize(V2d(otherNext->pos)-cp);
					startDir = normalize(V2d( outPoly->pointEnd->prev->pos ) - cp );
				}

				if (otherPoly->inverse)
				{
					if (GetVectorAngleDiffCCW(startDir, currTest) <= GetVectorAngleDiffCCW(startDir, otherTest))
					{
						//stay
						skipBorderCase = true;
						continue;
					}
					else
					{
						//switch

						skipBorderCase = true;

						PolyPtr temp = currentPoly;
						currentPoly = otherPoly;
						otherPoly = temp;

						currP = otherCurr;
						//startSegPos = currP->pos;
						continue;

					}
				}
				else
				{
					if (GetVectorAngleDiffCW(startDir, currTest) <= GetVectorAngleDiffCW(startDir, otherTest))
					{
						//stay
						skipBorderCase = true;
						continue;
					}
					else
					{
						//switch

						skipBorderCase = true;

						PolyPtr temp = currentPoly;
						currentPoly = otherPoly;
						otherPoly = temp;

						currP = otherCurr;
						//startSegPos = currP->pos;
						continue;

					}
				}
				
			}
		}

		bool stayWhenHitLine = false;
		/*if (nextPointOnLine)
		{
			TerrainPoint *nn = nextP->next;
			if (nn == NULL)
			{
				nn = currentPoly->pointStart;
			}
			V2d origDir = normalize(V2d(currP->pos) - V2d(nextP->pos));
			V2d stayDir = normalize(V2d(nn->pos) - V2d(nextP->pos));
			V2d otherDir = normalize(V2d(outEnd->pos) - V2d(outStart->pos));

			double stay = GetClockwiseAngleDifference(origDir, stayDir);
			double other = GetClockwiseAngleDifference(origDir, otherDir);

			if (stay < other)
			{
				stayWhenHitLine = true;
				skipBorderCase = true;
			}
			else
			{
				skipBorderCase = true;
			}
		}
		else
		{
			skipBorderCase = false;
		}*/
		//there was an intersection
		if (!li.parallel && !stayWhenHitLine)
		{
			Vector2i intersectPoint;
			if (onOtherEnd )//length(li.position - V2d(outEnd->pos)) < 8.0)
			{
				intersectPoint = outEnd->pos;
				currP = outEnd;

				if (currP->pos == startP->pos)
					break;
			}
			else if (onOtherStart )//length(li.position - V2d(outStart->pos)) < 8.0)
			{
				//V2d enterDir = 


				//do the angle thing here! check difference. dont need the point map this way

				intersectPoint = outStart->pos;
				currP = outStart;

				if (currP->pos == startP->pos)
					break;
				//skipBorderCase = true;
			}
			//else if (length(li.position - V2d(startSegPos)) < 8.0)
			//{

			//	V2d middlePos = (V2d(startSegPos) + li.position) / 2.0;
			//	intersectPoint.x = round(middlePos.x);
			//	intersectPoint.y = round(middlePos.y);

			//	TerrainPoint *endPoint = outPoly->pointEnd;
			//	outPoly->RemovePoint(outPoly->pointEnd);

			//	currP = outStart;

			//	delete endPoint;

			//	skipBorderCase = true;
			//	//TODO: memory leak
			//}

			else
			{
				//if (nextPointOnLine)
				{
					skipBorderCase = true;
				}

				intersectPoint = Vector2i(round(li.position.x), round(li.position.y));
				if (inverse && currentPoly->inverse)
				{
					currP = outEnd;
				}
				else
				{
					currP = outStart;
				}
			}

			PolyPtr temp = currentPoly;
			currentPoly = otherPoly;
			otherPoly = temp;

			TerrainPoint *tp = new TerrainPoint(intersectPoint, false);
			if (tp->pos == outPoly->pointEnd->pos)
			{
				int xxxx = 65;
			}
			outPoly->AddPoint(tp);

			startSegPos = intersectPoint;
		}
		//no intersection
		else
		{
			if (!firstRun && nextP->pos == startP->pos)
				break;

			TerrainPoint *tp = new TerrainPoint(nextP->pos, false);
			if (tp->pos == outPoly->pointEnd->pos)
			{
				int xxxx = 65;
			}
			outPoly->AddPoint(tp);

			if (inverse && !currentPoly->inverse)
			{
				currP = currP->prev;
				if (currP == NULL)
				{
					currP = currentPoly->pointEnd;
				}
			}
			else
			{
				currP = currP->next;
				if (currP == NULL)
				{
					currP = currentPoly->pointStart;
				}
			}
			startSegPos = currP->pos;
		}

		firstRun = false;
	}

	

	//poly->Reset();

	//for (TerrainPoint *zit = outPoly->pointStart; zit != NULL; zit = zit->next)
	//{
	//	TerrainPoint *tp = new TerrainPoint(*zit);
	//	if (tp->gate != NULL)
	//	{
	//		//cout << "new polygon will have gate" << endl;
	//		if (zit == tp->gate->point0)
	//		{
	//			tp->gate->point0 = tp;
	//			tp->gate->poly0 = poly;
	//		}
	//		else
	//		{
	//			tp->gate->point1 = tp;
	//			tp->gate->poly1 = poly;
	//		}
	//	}
	//	poly->AddPoint(tp);
	//}


	//cout << "about to check for enemy stuff" << endl;
	//for (TerrainPoint *bit = brush->pointStart; bit != NULL; bit = bit->next)
	//{
	//	//cout << "z enems: " << z.enemies.count( zit ) << endl;
	//	//cout << "enems: " << 
	//	//if( z.enemies.count( zit ) > 0 )
	//	if (brush->enemies.count(bit) > 0)
	//	{
	//		//list<ActorPtr> &en = poly->enemies[tp];
	//		//en = z.enemies[zit];
	//		list<ActorPtr> &en = brush->enemies[bit];//z.enemies[zit];

	//		AttachActorsToPolygon(en, poly.get());
	//	}
	//}

	outPoly->RemoveSlivers(PI / 10.0);
	outPoly->AlignExtremes(PRIMARY_LIMIT);


	outPoly->inverse = inverse;
	outPoly->Finalize();

	if (brush->inverse)
	{
		brush->FixWindingInverse();
	}
	else
	{
		brush->FixWinding();
	}

	if (poly->inverse)
	{
		poly->FixWindingInverse();
	}
	else
	{
		poly->FixWinding();
	}
	
	//poly->FixWinding();

	return AddResult::ADD_SUCCESS;
}

EditSession::AddResult EditSession::InverseAdd(PolyPtr brush, PolyPtr poly, list<TerrainPolygon*> &outPolyList)
{
	map<TerrainPoint*, DetailedInter> pointInterMap;
	//map<TerrainPoint*, Inter> pointInterMapFromPoly;
	brush->FixWinding();
	poly->FixWinding();

	list<DetailedInter> brushInters;
	brush->GetDetailedIntersections(poly.get(), brushInters);

	//might add 2 inters for an intersection right on a point
	for (auto it = brushInters.begin(); it != brushInters.end(); ++it)
	{
		TerrainPoint *itNext = (*it).inter.point->next;
		if (itNext == NULL)
			itNext = brush->pointStart;

		if (length((*it).inter.position - V2d((*it).inter.point->pos)) < 1.0)
		{
			pointInterMap[(*it).inter.point] = (*it);
		}
		else if (length((*it).inter.position - V2d(itNext->pos)) < 1.0)
		{
			pointInterMap[itNext] = (*it);
		}
	}

	//referenced by the brush pointer
	map< TerrainPoint*, TerrainPoint*> samePointMap;
	for (TerrainPoint *tp = brush->pointStart; tp != NULL; tp = tp->next)
	{
		TerrainPoint *same = poly->GetSamePoint(tp->pos);
		if (same != NULL)
		{
			samePointMap[tp] = same;
			//samePointMap[same] = tp;
		}
	}

	/*list<Inter> polyInters = poly->GetIntersections(brush.get());

	for (auto it = polyInters.begin(); it != polyInters.end(); ++it)
	{
	for (TerrainPoint *tp = brush->pointStart; tp != NULL; tp = tp->next)
	{
	TerrainPoint *tpNext = tp->next;
	if (itNext == NULL)
	itNext = poly->pointStart;
	}


	if (length((*it).position - V2d((*it).point->pos)) < 1.0)
	{
	pointInterMapFromPoly[(*it).point] = (*it);
	}
	}*/
	/*for (auto it = inters.begin(); it != inters.end(); ++it)
	{
	for (TerrainPoint *tp = brush->pointStart; tp != NULL; tp = tp->next)
	{
	if (length((*it).position - V2d(tp->pos.x, tp->pos.y)) < 1.0)
	{

	}
	}

	}*/


	TerrainPolygon *outPoly = NULL;
	brush->FixWinding();
	poly->FixWinding();
	//both of these polygons must be confirmed to be valid already in their individual shape

	bool inverse = false;
	if (brush->inverse || poly->inverse)
	{
		inverse = true;
	}

	outPoly = new TerrainPolygon(&grassTex);
	//TerrainPolygon z( &grassTex );
	//1: choose start point

	Vector2i startPoint;
	bool startPointFound = false;
	bool firstPolygon = true;


	PolyPtr currentPoly = NULL;
	PolyPtr otherPoly = NULL;
	TerrainPoint *currP = poly->pointStart;

	TerrainPoint *nextP = NULL;
	TerrainPoint *startP = NULL;
	sf::Vector2i startSegPosStart;

	TerrainPoint *outStart = NULL;
	TerrainPoint *outEnd = NULL;

	PolyPtr minPoly = NULL;

	//get extreme left point. this way it cant be in a cave
	TerrainPoint *leftPoint = currP;
	currentPoly = poly;
	otherPoly = brush;

	for (; currP != NULL; currP = currP->next)
	{
		if (currP->pos.x < leftPoint->pos.x)
		{
			leftPoint = currP;
			currentPoly = poly;
			otherPoly = brush;
		}
	}
	currP = brush->pointStart;
	for (; currP != NULL; currP = currP->next)
	{
		// the <= is there because brush should be prioritized 
		if (currP->pos.x <= leftPoint->pos.x)
		{
			leftPoint = currP;
			currentPoly = brush;
			otherPoly = poly;
		}
	}

	if (inverse)
	{
		if (currentPoly->inverse)
		{
			startP = leftPoint;
			startSegPosStart = startP->pos;
		}
		else
		{
			currP = leftPoint;

			while (true)
			{
				nextP = currP->next;
				if (nextP == NULL)
					nextP = currentPoly->pointStart;

				//might not account for hitting pointsin this area yet
				LineIntersection li = otherPoly->GetSegmentFirstIntersection(currP->pos, nextP->pos, outStart, outEnd);

				if (!li.parallel)
				{
					startP = outEnd;
					PolyPtr temp = otherPoly;
					otherPoly = currentPoly;
					currentPoly = temp;
					startSegPosStart = Vector2i(round(li.position.x), round(li.position.y));
					break;
				}

				currP = currP->next;
				if (currP == NULL)
					currP = currentPoly->pointStart;
			}
			assert(startP != NULL);
		}
	}
	else
	{
		startP = leftPoint;
		startSegPosStart = startP->pos;
	}

	currP = startP;
	bool firstRun = true;
	TerrainPoint *tp = new TerrainPoint(startSegPosStart, false);
	outPoly->AddPoint(tp);

	Vector2i startSegPos;// = currP->pos;
	Vector2i endSegPos;

	startSegPos = startSegPosStart;//currP->pos;



	bool skipBorderCase = false;
	while (true)
	{

		//assert(outPoly->numPoints <= brush->numPoints + poly->numPoints);
		if (inverse && !currentPoly->inverse)
		{
			nextP = currP->prev;
			if (nextP == NULL)
				nextP = currentPoly->pointEnd;
		}
		else
		{
			nextP = currP->next;
			if (nextP == NULL)
				nextP = currentPoly->pointStart;
		}

		if (!firstRun && startSegPos == startSegPosStart)
		{
			break;
		}
		LineIntersection li = otherPoly->GetSegmentFirstIntersection(startSegPos, nextP->pos, outStart, outEnd, skipBorderCase);
		skipBorderCase = false;

		bool pointFromOtherLine = !li.parallel && length(li.position - V2d(outStart->pos)) != 0
			&& length(li.position - V2d(outEnd->pos)) != 0;
		bool nextPointOnLine = !li.parallel && (length(li.position - V2d(nextP->pos)) == 0.0);
		bool currPointOnLine = !li.parallel && (length(li.position - V2d(startSegPos)) == 0.0);

		bool onOtherStart = !li.parallel && (length(li.position - V2d(outStart->pos)) == 0.0);
		bool onOtherEnd = !li.parallel && (length(li.position - V2d(outEnd->pos)) == 0.0);

		if (nextPointOnLine && nextP->pos == startP->pos)
			break;
		//if (nextPointOnLine)
		//	li.parallel = true;

		if (currPointOnLine) //collision at start point
		{
			//could be a line or point
			//	if (onOtherStart || onOtherEnd) //point/point collision

			{
				TerrainPoint *prev = currP->prev;
				if (prev == NULL)
					prev = currentPoly->pointEnd;

				V2d cp = V2d(startSegPos);
				V2d np = V2d(nextP->pos);
				V2d pp = V2d(prev->pos);

				V2d startDir, otherTest;

				V2d currPrevTest = normalize(pp - cp);
				V2d currTest = normalize(np - cp);

				TerrainPoint *otherCurr = NULL;
				TerrainPoint *otherPrev = NULL;
				TerrainPoint *otherNext = NULL;

				if (outPoly->numPoints == 1) //startPoint
				{
					V2d otherPrevTest;

					//TerrainPoint *otherPrev = NULL;

					TerrainPoint *otherNext = NULL;

					if (onOtherEnd)
					{
						otherPrev = outStart;
						otherCurr = outEnd;
						otherNext = otherCurr->next;
						if (otherNext == NULL)
							otherNext = otherPoly->pointStart;
					}
					else
						//if (onOtherStart)
					{
						otherPrev = outStart->prev;
						if (otherPrev == NULL)
							otherPrev = otherPoly->pointEnd;

						otherCurr = outStart;
						otherNext = outEnd;
					}

					otherPrevTest = normalize(V2d(otherPrev->pos) - cp);
					otherTest = normalize(V2d(otherNext->pos) - cp);

					if (otherPoly->inverse)
					{
						if (GetVectorAngleDiffCCW(currPrevTest, currTest) <= GetVectorAngleDiffCCW(otherPrevTest, currTest)) //we startAngle on mine
						{
							startDir = currPrevTest;
						}
						else
						{
							startDir = otherPrevTest;
						}
					}
					else
					{
						if (GetVectorAngleDiffCW(currPrevTest, currTest) <= GetVectorAngleDiffCW(otherPrevTest, currTest)) //we startAngle on mine
						{
							startDir = currPrevTest;
						}
						else
						{
							startDir = otherPrevTest;
						}
					}


				}
				else
				{

					if (onOtherEnd)
					{
						otherPrev = outStart;
						otherCurr = outEnd;
						otherNext = otherCurr->next;
						if (otherNext == NULL)
							otherNext = otherPoly->pointStart;
					}
					else
					{
						otherPrev = outStart->prev;
						if (otherPrev == NULL)
							otherPrev = otherPoly->pointEnd;

						otherCurr = outStart;
						otherNext = outEnd;
					}

					otherTest = normalize(V2d(otherNext->pos) - cp);
					startDir = normalize(V2d(outPoly->pointEnd->prev->pos) - cp);
				}

				if (otherPoly->inverse)
				{
					if (GetVectorAngleDiffCCW(startDir, currTest) <= GetVectorAngleDiffCCW(startDir, otherTest))
					{
						//stay
						skipBorderCase = true;
						continue;
					}
					else
					{
						//switch

						skipBorderCase = true;

						PolyPtr temp = currentPoly;
						currentPoly = otherPoly;
						otherPoly = temp;

						currP = otherCurr;
						//startSegPos = currP->pos;
						continue;

					}
				}
				else
				{
					if (GetVectorAngleDiffCW(startDir, currTest) <= GetVectorAngleDiffCW(startDir, otherTest))
					{
						//stay
						skipBorderCase = true;
						continue;
					}
					else
					{
						//switch

						skipBorderCase = true;

						PolyPtr temp = currentPoly;
						currentPoly = otherPoly;
						otherPoly = temp;

						currP = otherCurr;
						//startSegPos = currP->pos;
						continue;

					}
				}

			}
		}

		bool stayWhenHitLine = false;
		/*if (nextPointOnLine)
		{
		TerrainPoint *nn = nextP->next;
		if (nn == NULL)
		{
		nn = currentPoly->pointStart;
		}
		V2d origDir = normalize(V2d(currP->pos) - V2d(nextP->pos));
		V2d stayDir = normalize(V2d(nn->pos) - V2d(nextP->pos));
		V2d otherDir = normalize(V2d(outEnd->pos) - V2d(outStart->pos));

		double stay = GetClockwiseAngleDifference(origDir, stayDir);
		double other = GetClockwiseAngleDifference(origDir, otherDir);

		if (stay < other)
		{
		stayWhenHitLine = true;
		skipBorderCase = true;
		}
		else
		{
		skipBorderCase = true;
		}
		}
		else
		{
		skipBorderCase = false;
		}*/
		//there was an intersection
		if (!li.parallel && !stayWhenHitLine)
		{
			Vector2i intersectPoint;
			if (length(li.position - V2d(outEnd->pos)) < 8.0)
			{

				intersectPoint = outEnd->pos;
				currP = outEnd;

				if (currP->pos == startP->pos)
					break;
			}
			else if (length(li.position - V2d(outStart->pos)) < 8.0)
			{
				//V2d enterDir = 


				//do the angle thing here! check difference. dont need the point map this way

				intersectPoint = outStart->pos;
				currP = outStart;

				if (currP->pos == startP->pos)
					break;
				//skipBorderCase = true;
			}
			else if (length(li.position - V2d(startSegPos)) < 8.0)
			{

				V2d middlePos = (V2d(startSegPos) + li.position) / 2.0;
				intersectPoint.x = round(middlePos.x);
				intersectPoint.y = round(middlePos.y);

				TerrainPoint *endPoint = outPoly->pointEnd;
				outPoly->RemovePoint(outPoly->pointEnd);

				currP = outStart;

				delete endPoint;

				skipBorderCase = true;
				//TODO: memory leak
			}

			else
			{
				//if (nextPointOnLine)
				{
					skipBorderCase = true;
				}

				intersectPoint = Vector2i(round(li.position.x), round(li.position.y));
				if (inverse && currentPoly->inverse)
				{
					currP = outEnd;
				}
				else
				{
					currP = outStart;
				}
			}

			PolyPtr temp = currentPoly;
			currentPoly = otherPoly;
			otherPoly = temp;

			TerrainPoint *tp = new TerrainPoint(intersectPoint, false);
			outPoly->AddPoint(tp);

			startSegPos = intersectPoint;
		}
		//no intersection
		else
		{
			if (!firstRun && nextP->pos == startP->pos)
				break;

			TerrainPoint *tp = new TerrainPoint(nextP->pos, false);
			outPoly->AddPoint(tp);

			if (inverse && !currentPoly->inverse)
			{
				currP = currP->prev;
				if (currP == NULL)
				{
					currP = currentPoly->pointEnd;
				}
			}
			else
			{
				currP = currP->next;
				if (currP == NULL)
				{
					currP = currentPoly->pointStart;
				}
			}
			startSegPos = currP->pos;
		}

		firstRun = false;
	}

	//poly->Reset();

	//for (TerrainPoint *zit = outPoly->pointStart; zit != NULL; zit = zit->next)
	//{
	//	TerrainPoint *tp = new TerrainPoint(*zit);
	//	if (tp->gate != NULL)
	//	{
	//		//cout << "new polygon will have gate" << endl;
	//		if (zit == tp->gate->point0)
	//		{
	//			tp->gate->point0 = tp;
	//			tp->gate->poly0 = poly;
	//		}
	//		else
	//		{
	//			tp->gate->point1 = tp;
	//			tp->gate->poly1 = poly;
	//		}
	//	}
	//	poly->AddPoint(tp);
	//}


	//cout << "about to check for enemy stuff" << endl;
	//for (TerrainPoint *bit = brush->pointStart; bit != NULL; bit = bit->next)
	//{
	//	//cout << "z enems: " << z.enemies.count( zit ) << endl;
	//	//cout << "enems: " << 
	//	//if( z.enemies.count( zit ) > 0 )
	//	if (brush->enemies.count(bit) > 0)
	//	{
	//		//list<ActorPtr> &en = poly->enemies[tp];
	//		//en = z.enemies[zit];
	//		list<ActorPtr> &en = brush->enemies[bit];//z.enemies[zit];

	//		AttachActorsToPolygon(en, poly.get());
	//	}
	//}

	outPoly->RemoveSlivers(PI / 10.0);
	outPoly->AlignExtremes(PRIMARY_LIMIT);


	outPoly->inverse = inverse;
	outPoly->Finalize();

	if (brush->inverse)
	{
		brush->FixWindingInverse();
	}
	else
	{
		brush->FixWinding();
	}

	if (poly->inverse)
	{
		poly->FixWindingInverse();
	}
	else
	{
		poly->FixWinding();
	}

	return AddResult::ADD_SUCCESS;
}

struct SubInfo
{
	SubInfo(TerrainPolygon *p, TerrainPoint *po)
		:poly(p), point(po) {}
	TerrainPolygon *poly;
	TerrainPoint *point;
};

bool IsWithinOne(sf::Vector2i &a, sf::Vector2i &b)
{
	return (abs(a.x - b.x) <= 1 && abs(a.y - b.y) <= 1);
}

void EditSession::Sub(PolyPtr brushPtr, std::list<PolyPtr> &orig, std::list<PolyPtr> &results)
{
	list<PolyPtr> potentialInversePolys;
	brushPtr->FixWinding();
	for (list<PolyPtr>::iterator polyIt = orig.begin(); polyIt != orig.end(); ++polyIt)
	{
		(*polyIt)->FixWinding();

		TerrainPolygon *poly = (*polyIt).get();

		list<Inter> inters;
		brushPtr->GetIntersections(poly, inters);
		assert(!inters.empty() );
		list<TerrainPoint*> interStarts;

		LineIntersection li;

		list<Inter> starts;

		TerrainPoint *currP;
		TerrainPoint *nextP;

		TerrainPoint *outSegStart;
		TerrainPoint *outSegEnd;

		PolyPtr currentPoly = NULL;
		PolyPtr otherPoly = NULL;

		bool alreadyTested = false;
		//start moving along brush
		for (auto it = inters.begin(); it != inters.end(); ++it)
		{
			currP = (*it).point;
			nextP = currP->next;
			if (nextP == NULL)
				nextP = brushPtr->pointStart;

			bool pointInside = (*polyIt)->ContainsPoint(Vector2f(currP->pos.x, currP->pos.y));
			bool pointOnLine = (*it).position == V2d((*it).point->pos) && (*polyIt)->PointOnBorder((*it).position);
			bool firstPoint;

			

			if (length((*it).position - V2d(nextP->pos) ) == 0.0 )
			{
				continue;
			}

			bool fail = false;
			for (auto it2 = starts.begin(); it2 != starts.end(); ++it2)
			{
				if ((*it2).position == (*it).position)
				{
					fail = true;
					break;
				}
			}
			if (fail)
				continue;
			int intersectionNumber = (*polyIt)->GetIntersectionNumber(currP->pos, nextP->pos, (*it), outSegStart, firstPoint);
			
			TerrainPoint *outNext = outSegStart->next;
			if (outNext == NULL)
				outNext = (*polyIt)->pointStart;

			if (length((*it).position - V2d(outNext->pos)) == 0.0)
			{
				continue;
			}

			V2d currDir = normalize(V2d(nextP->pos) - V2d(currP->pos));

			
			V2d otherDir = normalize(V2d(outNext->pos) - V2d( outSegStart->pos ) );

			bool lineGoingIn = cross(otherDir, currDir) > 0;

			bool startOutAndNowOut = (!pointInside && intersectionNumber % 2 == 0);
			bool startInAndNowOut = (pointInside && intersectionNumber % 2 != 0);


			if(  (pointOnLine && lineGoingIn ) || ( !pointOnLine && !firstPoint && ( startOutAndNowOut || startInAndNowOut) )
				||(!pointOnLine && firstPoint && ( !startOutAndNowOut && !startInAndNowOut )))
			{
				TerrainPoint *outEnd = outSegStart->next;
				if (outEnd == NULL)
					outEnd = (*polyIt)->pointStart;



				(*it).point = outSegStart;
				starts.push_back((*it));
			}

			//V2d dir = normalize(V2d(nextP->pos.x, nextP->pos.y) - V2d(currP->pos.x, currP->pos.y));
			//
			////if (poly->ContainsPoint(Vector2f(currP->pos.x, currP->pos.y)))
			////	continue;

			//LineIntersection li = poly->GetSegmentFirstIntersection(currP->pos, nextP->pos, outSegStart, outSegEnd);

			//assert(!li.parallel);
			//
			//V2d otherEdge = normalize(V2d(outSegEnd->pos.x, outSegEnd->pos.y) - V2d(outSegStart->pos.x, outSegStart->pos.y));
			//if (cross(dir, otherEdge) >= 0)
			//{
			//	//this result will be less than 0 if you're coming from outside the polygon
			//	continue;
			//}

			//alreadyTested = false;
			//for (auto it2 = interStarts.begin(); it2 != interStarts.end(); ++it2)
			//{
			//	if ((*it2) == currP)
			//	{
			//		alreadyTested = true;
			//		break;
			//	}
			//}
			//if (alreadyTested)
			//	continue;

			//interStarts.push_back(currP);

			
		}

		bool startsEmpty = false;
		TerrainPoint *leftPoint = NULL;
		if (starts.empty())
		{
			startsEmpty = true;
			//this means that all the points that lead to intersections are actually inside the polygon. in this case, we can just
			//get the leftmost and start there
			leftPoint = (*polyIt)->GetMostLeftPoint();
			starts.push_back(Inter(leftPoint, V2d(leftPoint->pos.x, leftPoint->pos.y)));
		}

		sf::Vector2i startSegPos;
		bool firstRun = true;
		TerrainPoint *startNext = NULL;
		for (auto it = starts.begin(); it != starts.end(); ++it)
		{
			list<TerrainPoint*> newPolyPoints;
			currentPoly = (*polyIt);
			otherPoly = brushPtr;

			startSegPos = Vector2i(round((*it).position.x), round((*it).position.y));
			Vector2i startPos = startSegPos;
			TerrainPoint *tp = new TerrainPoint(startSegPos, false);
			newPolyPoints.push_back(tp);
			firstRun = true;

			currP = (*it).point;

			bool skipFirstPoint = true;

			while (true)
			{
				/*if (inverse && !currentPoly->inverse)
				{
					nextP = currP->prev;
					if (nextP == NULL)
						nextP = currentPoly->pointEnd;
				}
				else*/



				if (currentPoly == (*polyIt) || (*polyIt)->inverse )
				{
					nextP = currP->next;
					if (nextP == NULL)
						nextP = currentPoly->pointStart;
				}
				else
				{
					//go ccw on brush if not inverse
					nextP = currP->prev;
					if (nextP == NULL)
						nextP = currentPoly->pointEnd;
				}

				/*{
					nextP = currP->next;
					if (nextP == NULL)
						nextP = currentPoly->pointStart;
				}*/

				outSegStart = NULL;
				outSegEnd = NULL;
				LineIntersection li = otherPoly->GetSegmentFirstIntersection(startSegPos, nextP->pos, outSegStart, outSegEnd, skipFirstPoint);
				skipFirstPoint = false;
				

				if (!li.parallel && length(V2d(nextP->pos) - li.position) == 0.0)
				{
					

					li.parallel = true;
				}

				if (!li.parallel && length(V2d(outSegEnd->pos) - li.position) == 0.0)
				{
					outSegStart = outSegEnd;
					outSegEnd = outSegEnd->next;
					if (outSegEnd == NULL)
						outSegEnd = otherPoly->pointStart;
				}
				//there was an intersection
				if (!li.parallel)
				{
					Vector2i intersectPoint = Vector2i(round(li.position.x), round(li.position.y));
					
					if ( IsWithinOne( intersectPoint, startPos ) )
					{
						break;
					}

					skipFirstPoint = true;

					PolyPtr temp = currentPoly;
					currentPoly = otherPoly;
					otherPoly = temp;

					if (currentPoly == (*polyIt) || (*polyIt)->inverse)
					{
						currP = outSegStart;
					}
					else
					{
						if (length(V2d(outSegStart->pos) - li.position) == 0.0)
						{
							currP = outSegStart;
						}
						else
						{
							currP = outSegEnd;
						}
						
					}

					if (newPolyPoints.back()->pos != intersectPoint)
					{
						TerrainPoint *tp = new TerrainPoint(intersectPoint, false);
						newPolyPoints.push_back(tp);
					}
					

					startSegPos = intersectPoint;

					/*if (inverse && !currentPoly->inver se)
					{
						currP = outEnd;
					}
					else*/

					

				}
				//no intersection
				else
				{
					
					//&& startsEmpty
					if (!firstRun && nextP->pos == startPos)
					{
						break;
					}
						

					TerrainPoint *tp = new TerrainPoint(nextP->pos, false);
					//z.AddPoint(tp);
					newPolyPoints.push_back(tp);

					//deal with enemies and gates
					//tp->gate = curr->gate;
					//if (tp->gate != NULL)
					//{
					//	//	cout << "other gate not null!" << endl;
					//	if (curr == tp->gate->point0)
					//	{
//		//		cout << "putting a" << endl;
//		tp->gate->point0 = tp;
//	}
//	else if (curr == tp->gate->point1)
//	{
//		//		cout << "putting b at: " << tp->pos.x << ", " << tp->pos.y << endl;
//		tp->gate->point1 = tp;

					//	}
					//	else
					//	{
					//		//cout << "gate: " << tp->gate->point0->pos.x << ", " << tp->gate->point0->pos.y << ", "
					//		//	<< tp->gate->point1->pos.x << ", " << tp->gate->point1->pos.y << ", " << endl;
					//		//cout << "tp: " << tp->pos.x << ", " << tp->pos.y << endl;
					//		assert(false);
					//		//tp->gate = NULL;
					//		//tp->gate == NULL;
					//	}
					//}

					/*if (inverse && !currentPoly->inverse)
					{
						currP = currP->prev;
						if (currP == NULL)
						{
							currP = currentPoly->pointEnd;
						}
					}
					else*/
					/*{
						currP = currP->next;
						if (currP == NULL)
						{
							currP = currentPoly->pointStart;
						}
					}*/

					if (currentPoly == (*polyIt) || (*polyIt)->inverse )
					{
						currP = currP->next;
						if (currP == NULL)
						{
							currP = currentPoly->pointStart;
						}
					}
					else
					{
						currP = currP->prev;
						if (currP == NULL)
						{
							currP = currentPoly->pointEnd;
						}
					}

					startSegPos = currP->pos;

					/*if (currentPoly->enemies.count(curr) > 0)
					{
					list<ActorPtr> &en = z.enemies[tp];
					en = currentPoly->enemies[curr];
					}*/
				}

				firstRun = false;
			}

			bool fail = false;
			for (auto pit = newPolyPoints.begin(); pit != newPolyPoints.end() && !fail; ++pit)
			{
				for (auto pit2 = newPolyPoints.begin(); pit2 != pit; pit2++)
				{
					if ((*pit)->pos == (*pit2)->pos)
					{
						fail = true;
						break;
					}
				}
			}
			if (fail)
				continue;


			PolyPtr newPoly(new TerrainPolygon(&grassTex));


			//for (list<pair<Vector2i, SubInfo>>::iterator it = newPoints.begin(); it != newPoints.end(); ++it)
			for (auto pit = newPolyPoints.begin(); pit != newPolyPoints.end(); ++pit)
			{
				newPoly->AddPoint((*pit));
				//TerrainPoint *p = new TerrainPoint((*it).first, false);

				//might need to try and preserve gate
				//if ((*it).second.poly != NULL)
				//{
				//	p->gate = (*it).second.point->gate;
				//	if (p->gate != NULL)
				//	{
				//		if (p->gate->poly0.get() == (*it).second.poly)
				//		{
				//			p->gate->poly0 = newPoly;
				//			p->gate->point0 = p;
				//		}
				//		else
				//		{
				//			p->gate->poly1 = newPoly;
				//			p->gate->point1 = p;
				//		}
				//		//cout << "preserving gate!" << endl;
				//	}
				//}
				////if( (*it)
				//newPoly->AddPoint(p);
				//cout << "point: " << p->pos.x << ", " << p->pos.y << endl;
			}

			bool sharesPoints = false;
			/*for (auto rit = results.begin(); rit != results.end(); ++rit )
			{
				if (newPoly->SharesPoints( (*rit).get() ) )
				{
					sharesPoints = true;
					break;
				}
			}*/

			if (!sharesPoints)
			{
				//newPoly->Finalize();
				if ((*polyIt)->inverse)
				{
					potentialInversePolys.push_back(newPoly);
				}
				results.push_back(newPoly);
			}
		}

	}

	bool isOuter;
	for (auto it = potentialInversePolys.begin(); it != potentialInversePolys.end(); ++it)
	{
		isOuter = true;
		for (auto it2 = potentialInversePolys.begin(); it2 != potentialInversePolys.end(); ++it2)
		{
			if ((*it) == (*it2))
				continue;

			if (!(*it)->Contains((*it2).get()))
			{
				isOuter = false;
			}
		}

		if (isOuter)
		{
			(*it)->inverse = true;
			break;
		}
	}

	for (list<PolyPtr>::iterator polyIt = orig.begin(); polyIt != orig.end(); ++polyIt)
	{
		if ((*polyIt)->inverse)
		{
			(*polyIt)->FixWindingInverse();
		}
	}

	//finalize now that we've found our inverse
	for (auto it = results.begin(); it != results.end(); ++it)
	{
		(*it)->Finalize();
	}

	//check for duplicate point, this means a polygon is not just sharing a point, but is totally invalid and shouldn't count

	

	return;
	//for( list<PolyPtr>::iterator polyIt = orig.begin(); polyIt != orig.end(); ++polyIt )
	//{
	//	TerrainPolygon *poly = (*polyIt).get();
	//	list<TerrainPoint*> untouched;
	//	for( TerrainPoint *curr = poly->pointStart; curr != NULL; curr = curr->next )
	//	{
	//		if( !brushPtr->ContainsPoint( Vector2f( curr->pos.x, curr->pos.y ) ) )
	//		{
	//			untouched.push_back( curr );
	//		}
	//	}

	//	if( untouched.empty() )
	//	{
	//		//list<TerrainPoint*> 
	//		//put a new point in between the intersection points in the old polygon?
	//		//dont make it part of the object its just a temporary

	//		//cout << "EMPTY BLARG HERE" << endl;
	//	}

	//	bool onBrush = false;
	//	TerrainPolygon *currentPoly = poly;
	//	TerrainPolygon *otherPoly = brush;

	//	while( !untouched.empty() )
	//	{
	//		//cout << "untouched is not empty!:" << untouched.size() << endl;
	//		//PolyPtr newPoly( new TerrainPolygon( &grassTex ) );
	//		list<pair<Vector2i,SubInfo>> newPoints;

	//		
	//		TerrainPoint *start = untouched.front();
	//		//untouched.pop_front();

	//		TerrainPoint *curr = start;
	//		TerrainPoint *next = NULL;

	//		Vector2i currPoint = curr->pos;
	//		bool truePoint = true;
	//		
	//		do
	//		{
	//			if( currentPoly == poly )
	//			{
	//				if( curr->next == NULL )
	//				{
	//					next = poly->pointStart;
	//				}
	//				else
	//				{
	//					next = curr->next;
	//				}
	//				
	//			}
	//			else
	//			{
	//				if( curr->prev == NULL )
	//				{
	//					next = brush->pointEnd;
	//				}
	//				else
	//				{
	//					next = curr->prev;
	//				}
	//				
	//			}

	//			//untouched.remove( curr );
	//			//newPoints.push_back( TerrainPoint( *curr ) );
	//			if( truePoint )
	//			{
	//				newPoints.push_back( pair<Vector2i,SubInfo>( currPoint, SubInfo( currentPoly, curr ) ) );
	//			}
	//			else
	//			{
	//				newPoints.push_back( pair<Vector2i,SubInfo>( currPoint, SubInfo( NULL, NULL ) ) );
	//			}

	//			if( currentPoly == poly && currPoint == curr->pos )
	//			{
	//				untouched.remove( curr );
	//				//cout << "untouched now has: " << untouched.size() << endl;
	//			}
	//			
	//			//untouched.pop_front();
	//		
	//			TerrainPoint *min = NULL;
	//			Vector2i minIntersection;
	//			bool emptyInter = true;

	//			//get closest intersection to my current point
	//			for( TerrainPoint *other = otherPoly->pointStart; other != NULL; other = other->next )
	//			{																															
	//				TerrainPoint *otherPrev;
	//				if( other == otherPoly->pointStart )
	//				{
	//					otherPrev = otherPoly->pointEnd;
	//				}
	//				else
	//				{
	//					otherPrev = other->prev;
	//				}

	//				LineIntersection li = LimitSegmentIntersect( currPoint, next->pos, otherPrev->pos, other->pos );
	//				Vector2i lii( floor(li.position.x + .5), floor(li.position.y + .5) );
	//				if( !li.parallel )
	//				{
	//					if( emptyInter )
	//					{
	//						emptyInter = false;
	//						minIntersection = lii;
	//						if( currentPoly == poly )
	//							min = other;
	//						else
	//							min = otherPrev;
	//					}
	//					else
	//					{
	//						V2d blah( minIntersection - currPoint );
	//						V2d blah2( lii - currPoint );
	//						if( length( blah2 ) < length( blah ) )
	//						{
	//							minIntersection = lii;
	//							if( currentPoly == poly )
	//								min = other;
	//							else
	//								min = otherPrev;
	//						}
	//					}
	//				}
	//			}

	//			if( !emptyInter )
	//			{
	//				TerrainPolygon *temp = currentPoly;
	//				currentPoly = otherPoly;
	//				otherPoly = temp;

	//				//newPoints.push_back( minIntersection );

	//				currPoint = minIntersection;

	//				curr = min;
	//				truePoint = false;

	//			}
	//			else
	//			{
	//				curr = next;
	//				currPoint = next->pos;
	//				truePoint = true;
	//			}
	//			
	//		}
	//		while( curr != start );



	//		PolyPtr newPoly( new TerrainPolygon( &grassTex ) );
	//		for( list<pair<Vector2i,SubInfo>>::iterator it = newPoints.begin(); it != newPoints.end(); ++it )
	//		{
	//			TerrainPoint *p = new TerrainPoint( (*it).first, false );

	//			//might need to try and preserve gate
	//			if( (*it).second.poly != NULL )
	//			{
	//				p->gate = (*it).second.point->gate;
	//				if( p->gate != NULL )
	//				{
	//					if( p->gate->poly0.get() == (*it).second.poly )
	//					{
	//						p->gate->poly0 = newPoly;
	//						p->gate->point0 = p;
	//					}
	//					else
	//					{
	//						p->gate->poly1 = newPoly;
	//						p->gate->point1 = p;
	//					}
	//					//cout << "preserving gate!" << endl;
	//				}
	//			}
	//			//if( (*it)
	//			newPoly->AddPoint( p );
	//			//cout << "point: " << p->pos.x << ", " << p->pos.y << endl;
	//		}
	//		newPoly->Finalize();
	//		results.push_back( newPoly );
	//		//cout << "results pushing back" << endl;
	//	}
	//}
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
	else
	{
		/*V2d dir0 = normalize(V2d(b) - V2d(a));
		V2d dir1 = normalize(V2d(d) - V2d(c));
		if ( abs( dot( dir1, dir0 ) ) == 1 )
		{
			double dc = dot(V2d(c) - V2d(a), dir0);
			double dd = dot(V2d(d) - V2d(a), dir0);
			double da = 0;
			double db = length(V2d(b) - V2d(a));

			if (dc >= da && dc <= db )
			{
				li.parallel = false;
				li.position = V2d(c);
			}
			if (dd >= da && dd <= db)
			{
				li.parallel = false;
				li.position = V2d(d);
			}
		}*/
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

LineIntersection EditSession::LimitSegmentIntersect( Vector2i a, Vector2i b, Vector2i c, Vector2i d, bool firstLimitOnly )
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
					if( ( length( li.position - V2d( a.x, a.y ) ) > 1.0 ) && ( firstLimitOnly || length( li.position - V2d( b.x, b.y ) ) > 1.0 ) 
						&&  ( firstLimitOnly || ( ( length( li.position - V2d( c.x, c.y ) ) > 0 &&  length( li.position - V2d( d.x, d.y ) ) > 1.0 ))) )
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

int EditSession::Run( const boost::filesystem::path &p_filePath, Vector2f cameraPos, Vector2f cameraSize )
{
	bool oldMouseGrabbed = mainMenu->GetMouseGrabbed();
	bool oldMouseVis = mainMenu->GetMouseVisible();

	mainMenu->SetMouseGrabbed(true);
	mainMenu->SetMouseVisible(true);
	//w->setMouseCursorVisible(true);

	sf::View oldPreTexView = preScreenTex->getView();//mainMenu->preScreenTexture->
	sf::View oldWindowView = w->getView();

	currTool = TOOL_ADD;
	//bosstype = 0;
	currentFile = p_filePath.string();
	currentPath = p_filePath;

	cutChoose = false;
	cutChooseUp = false;
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
	arial.loadFromFile( "Resources/Fonts/Breakneck_Font_01.ttf" );

	sf::Texture playerZoomIconTex;
	playerZoomIconTex.loadFromFile( "Resources/Editor/playerzoomicon.png" );
	sf::Sprite playerZoomIcon( playerZoomIconTex );
	
	playerZoomIcon.setOrigin( playerZoomIcon.getLocalBounds().width / 2, playerZoomIcon.getLocalBounds().height / 2 );	

	SetupEnemyTypes();

	InitDecorPanel();

	Panel *mapOptionsPanel = CreateOptionsPanel("map");
	Panel *terrainOptionsPanel = CreateOptionsPanel("terrain");

	messagePopup = CreatePopupPanel( "message" );
	errorPopup = CreatePopupPanel( "error" );
	bgPopup = CreatePopupPanel("bg");

	GridSelector *bgSel = bgPopup->AddGridSelector(
		"terraintypes", Vector2i(20, 20), 6, 7, 1920/8, 1080/8, false, true);

	Tileset *bgTS;
	string path = "BGInfo/";
	string bgName; //"w1_0";
	string png = ".png";
	string numStr;
	string fullName;
	for (int w = 0; w < 2; ++w)
	{
		for (int i = 0; i < 6; ++i)
		{
			numStr = to_string(i + 1);
			bgName = "w" + to_string(w + 1) + "_0";// +to_string(i + 1);
			fullName = path + bgName + numStr + png;
			bgTS = GetTileset(fullName, 1920, 1080);
			if (bgTS == NULL)
			{
				continue;
			}
			Sprite bgSpr(*bgTS->texture);
			bgSpr.setScale(.125, .125);
			bgSel->Set(i, w, bgSpr, bgName + numStr);
		}
	}

	enemySelectPanel = new Panel( "enemyselection", 200, 200, this );
	allPopups.push_back(enemySelectPanel);
	int gridSizeX = 80;
	int gridSizeY = 80;
	GridSelector *gs = enemySelectPanel->AddGridSelector( "world0enemies", Vector2i( 0, 0 ), 10, 10, gridSizeX,
		gridSizeY, false, true );
	gs->active = false;
	
	int counter = 0;
	for (int i = 0; i < 8; ++i)
	{
		list<string> &wen = worldEnemyNames[i];
		counter = 0;
		for (auto it = wen.begin(); it != wen.end(); ++it)
		{
			SetEnemyGridIndex(gs, counter, i, (*it));
			++counter;
		}
	}

	counter = 0;
	int row = 8;
	for (auto it = extraEnemyNames.begin(); it != extraEnemyNames.end(); ++it)
	{
		SetEnemyGridIndex(gs, counter, row, (*it));
		++counter;
		if (counter == gs->xSize)
		{
			counter = 0;
			++row;
		}
	}


	gateSelectorPopup = CreatePopupPanel( "gateselector" );
	GridSelector *gateSel = gateSelectorPopup->AddGridSelector( "gatetypes", Vector2i( 20, 20 ), 6, 4, 32, 32, false, true );

	sf::Texture whiteTex; //temp
	whiteTex.loadFromFile( "Resources/Editor/whitesquare.png" );
	Sprite tempSq;
	tempSq.setTexture( whiteTex );

	/*Sprite reformSq;
	reformSq.setTexture( whiteTex );
	reformSq.setColor( Color::Blue );*/
	
	
	
	tempSq.setColor( Color::Black );
	gateSel->Set( 0, 0, tempSq, "black" );

	tempSq.setColor( Color( 100, 100, 100 ) );
	gateSel->Set( 1, 0, tempSq, "keygate" );

	tempSq.setColor( Color::Red );
	gateSel->Set( 2, 0, tempSq, "secret" );

	tempSq.setColor( Color( 0, 255, 40 ) );
	gateSel->Set( 3, 0, tempSq, "birdfight" );

	tempSq.setColor( Color::Blue );
	gateSel->Set( 4, 0, tempSq, "crawlerunlock" );

	tempSq.setColor( Color::Cyan );
	gateSel->Set( 5, 0 , tempSq, "nexus1unlock" );
	
	
	/*tempSq.setColor( Color::Blue );
	gateSel->Set( 0, 1, tempSq, "blue" );
	tempSq.setColor( Color::Green );
	gateSel->Set( 1, 1, tempSq, "green" );
	tempSq.setColor( Color::Yellow );
	gateSel->Set( 2, 1, tempSq, "yellow" );
	tempSq.setColor( COLOR_ORANGE );
	gateSel->Set( 3, 1, tempSq, "orange" );
	tempSq.setColor( Color::Red );
	gateSel->Set( 4, 1, tempSq, "red" );
	tempSq.setColor( Color::Magenta );
	gateSel->Set( 5, 1, tempSq, "magenta" );
*/
	
	//tempSq.setColor( Color::Black );
	//gateSel->Set( 1, 1, tempSq, "critical" );

	gateSelectorPopup->AddButton( "deletegate", Vector2i( 20, 300 ), Vector2f( 80, 40 ), "delete" );

	terrainSelectorPopup = CreatePopupPanel( "terrainselector" );
	GridSelector *terrainSel = terrainSelectorPopup->AddGridSelector(
		"terraintypes", Vector2i( 20, 20 ), 9, MAX_TERRAINTEX_PER_WORLD, 64, 64, false, true );

	
	int numWorlds = 9;//6 plus core plus bear core plus special terrain
	for( int worldI = 0; worldI < numWorlds; ++worldI )
	{
		int ind;
		for( int i = 0; i < MAX_TERRAINTEX_PER_WORLD; ++i )
		{
			ind = worldI * MAX_TERRAINTEX_PER_WORLD + i;
			stringstream ss;
			ss << "Resources/Terrain/" << "terrain_" << (worldI+1) << "_0" << (i+1) << "_512x512.png";
			terrainTextures[ind] = new Texture;
			if (!terrainTextures[ind]->loadFromFile(ss.str()))
			{
				delete terrainTextures[ind];
				terrainTextures[ind] = NULL;
				break;
			}

			terrainSel->Set(worldI, i, Sprite(*terrainTextures[ind], sf::IntRect(0, 0, 64, 64)),
				"xx");

			if (!polyShaders[ind].loadFromFile("Resources/Shader/mat_shader2.frag", sf::Shader::Fragment))
			{
				cout << "MATERIAL SHADER NOT LOADING CORRECTLY EDITOR" << endl;
				assert(0 && "polygon shader not loaded editor");
			}

			polyShaders[ind].setUniform("u_texture", *terrainTextures[ind]);
			polyShaders[ind].setUniform("Resolution", Vector2f(1920, 1080));
			polyShaders[ind].setUniform("AmbientColor", Glsl::Vec4(1, 1, 1, 1));
			polyShaders[ind].setUniform("skyColor", ColorGL(Color::White));
		}
	}

	int returnVal = 0;
	Color testColor( 0x75, 0x70, 0x90 );
	view = View( cameraPos, cameraSize );
	if( cameraSize.x == 0 && cameraSize.y == 0 )
		view.setSize( 1920, 1080 );

	preScreenTex->setView( view );

	OpenFile();

	//Vector2f vs(  );
	if( cameraSize.x == 0 && cameraSize.y == 0 )
		view.setCenter( (float)player->position.x, (float)player->position.y );

	//mode = "neutral";
	bool quit = false;
	polygonInProgress.reset( new TerrainPolygon(&grassTex ) );
	//inversePolygon.reset( NULL );

	zoomMultiple = 2;

	view.setSize(Vector2f(960 * (zoomMultiple), 540 * (zoomMultiple)));
	preScreenTex->setView(view);

	UpdateFullBounds();
	Vector2<double> prevWorldPos;
	Vector2i pixelPos;
	Vector2f tempWorldPos = preScreenTex->mapPixelToCoords(sf::Mouse::getPosition( *w ));
	worldPos = Vector2<double>( tempWorldPos.x, tempWorldPos.y );
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
	guiMenuCubeTexture.loadFromFile( "Resources/Editor/guioptions.png" );
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
	

	bool s = IsKeyPressed( sf::Keyboard::T );
	
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

	showBG = false;

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
		int borderMove = 100;
		sf::Event ev;
		while( w->pollEvent( ev ) )
		{
			if (ev.type == Event::KeyPressed)
			{
				if (ev.key.code == Keyboard::F5)
				{
					showBG = !showBG;
					continue;
				}
			}
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

								if( cutChoose )
								{
									bool c0 = cutPoly0->ContainsPoint( testPoint );
									bool c1 = cutPoly1->ContainsPoint( testPoint );
									if( cutPoly0->inverse )
										c0 = !c0;
									if( cutPoly1->inverse )
										c1 = !c1;

									//contains point returns the opposite when you're inverse
									if( c0 )
									{
										ChooseCutPoly( cutPoly0 );
									}
									else if( c1 )
									{
										ChooseCutPoly( cutPoly1 );
									}

									extendingPolygon = NULL;
									extendingPoint = NULL;
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
								//if( !(showPoints && extendingPolygon) )
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
								// // this is only turned off for the beta build so I don't have to debug this.
								//{
								//	showPoints = true;
								//	extendingPolygon = NULL;
								//	extendingPoint = NULL;
								//	polygonInProgress->ClearPoints();
								//}
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
														if( !( IsKeyPressed( Keyboard::LShift ) || 
														IsKeyPressed( Keyboard::RShift ) ) )
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


									bool pointSelectKeyHeld = IsKeyPressed(Keyboard::B);//IsKeyPressed( Keyboard::LAlt ) ||
											//IsKeyPressed( Keyboard::RAlt );
									//concerning selecting a point
									if(pointSelectKeyHeld)
									{
										PointSelectPoint( worldPos, emptysp );
									}

									else
									//if( emptysp )
									//if(  )
									for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
									{
										bool pressF1 = IsKeyPressed(Keyboard::F1);
										if ((pressF1 && !(*it)->inverse) || !pressF1 && (*it)->inverse)
											continue;

										bool sel = (*it)->ContainsPoint(Vector2f(worldPos.x, worldPos.y));
										if ((*it)->inverse)
										{
											sel = !sel;
										}

										if( sel )
										//if( (*it)->ContainsPoint( Vector2f( worldPos.x, worldPos.y ) ) )
										{

											SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>( (*it) );

											if( sp->selected )
											{

											}
											else
											{
												if( !HoldingShift() )
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

									
									if (emptysp)
									{
										for (auto it = decorImagesBehindTerrain.begin();
											it != decorImagesBehindTerrain.end(); ++it)
										{

										}

										for (auto it = decorImagesBetween.begin();
											it != decorImagesBetween.end(); ++it)
										{
											SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>((*it));
											if ((*it)->ContainsPoint(Vector2f(worldPos.x, worldPos.y)))
											{
												emptysp = false;
												SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>((*it));

												if (sp->selected)
												{

												}
												else
												{
													if (!(IsKeyPressed(Keyboard::LShift) ||
														IsKeyPressed(Keyboard::RShift)))
													{
														selectedBrush->SetSelected(false);
														selectedBrush->Clear();
													}

													sp->SetSelected(true);

													grabbedObject = sp;
													selectedBrush->AddObject(sp);
												}

												emptysp = false;
												break;
											}
											
		/*									{
												cout << "contains!" << endl;
												selectedBrush->AddObject((*it));
												
											}*/
										}

										for (auto it = decorImagesFrontTerrain.begin();
											it != decorImagesFrontTerrain.end(); ++it)
										{

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

								if( moveActive )//&& !IsKeyPressed( Keyboard::LShift ) )
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
								if( false )//sf::IsKeyPressed( Keyboard::F ) )
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
												if( IsKeyPressed( Keyboard::LShift ) )
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
													if( IsKeyPressed( Keyboard::LShift ) )
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
									bool singleActor = selectedBrush->objects.size() == 1 
										&& selectedPoints.size() == 0
										&& selectedBrush->objects.front()->selectableType == ISelectable::ACTOR;
									if(singleActor)
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

									//bool shift = IsKeyPressed( Keyboard::LShift ) || IsKeyPressed( Keyboard::RShift );

									if( !HoldingShift() )
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

												if( HoldingShift() )
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

									for (auto it = decorImagesBetween.begin(); it != decorImagesBetween.end(); ++it )
									{
										if ((*it)->Intersects(r))
										{
											SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>((*it));

											if (HoldingShift())
											{
												if (sp->selected)
												{
													sp->SetSelected(false);
													selectedBrush->RemoveObject(sp); //might be slow?
												}
												else
												{
													sp->SetSelected(true);
													selectedBrush->AddObject(sp);
												}
											}
											else
											{
												//cout << "selected blah" << endl;
												sp->SetSelected(true);
												selectedBrush->AddObject(sp);
											}


											selectionEmpty = false;
										}
									}

									bool pointSelectButtonHeld = IsKeyPressed(Keyboard::B);//alt = IsKeyPressed( Keyboard::LAlt )
										//|| IsKeyPressed( Keyboard::RAlt );
									if(pointSelectButtonHeld) //always use point selection for now
									{
										if( HoldingShift() )
										{
											//ClearSelectedPoints();
										}

										BoxSelectPoints(r, 8 * zoomMultiple);
									}
									else if (!showPoints)//polygon selection. don't use it for a little bit
									{
										for (list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it)
										{
											if ((*it)->Intersects(r))
											{

												SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>((*it));

												if (HoldingShift())
												{
													if (sp->selected)
													{
														sp->SetSelected(false);
														selectedBrush->RemoveObject(sp);
													}
													else
													{
														sp->SetSelected(true);
														selectedBrush->AddObject(sp);
													}
												}
												else
												{
													sp->SetSelected(true);
													selectedBrush->AddObject(sp);
												}

												selectionEmpty = false;
												//break;
											}
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

												if( (*it)->SegmentTooClose( prev->pos, curr->pos, minimumEdgeLength ) )
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
										std::list<boost::shared_ptr<GateInfo>> gateInfoList;
										while( it != polygons.end() )
										{
											PolyPtr temp = (*it);
											if( currentBrush->IsTouching( temp.get() ) )
											{
												cout << "before addi: " << (*it)->numPoints << endl;
												
												TerrainPolygon *outPoly = NULL;
												
												Add( currentBrush, temp, outPoly, gateInfoList);

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
									for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
									{
										if ((*it)->selectableType == ISelectable::ISelectableType::TERRAIN)
										{
											boost::shared_ptr<TerrainPolygon> d = boost::static_pointer_cast<TerrainPolygon>((*it));
											d->UpdateGrass();
										}
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

							if (ev.key.code == Keyboard::I && ev.key.control )
							{
								mode = CREATE_IMAGES;
								currImageTool = ITOOL_EDIT;
								showPanel = decorPanel;
							}
							if( ev.key.code == Keyboard::C && ev.key.control )
							{
								copiedBrush = selectedBrush->Copy();
								/*if( selectedPolygons.size() > 0 )
								{
									ClearCopyBrushes();

									for( list<PolyPtr>::iterator it = selectedPolygons.begin(); 
										it != selectedPolygons.end(); ++it )
									{
										TerrainBrush *tb = new TerrainBrush( (*it) );
										copyBrushes.push_back( tb );
									}
								}*/
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
								//ClearPasteBrushes();
								if (copiedBrush != NULL)
								{
									Vector2i pos = Vector2i(worldPos.x, worldPos.y);
									copiedBrush->Move( pos - copiedBrush->GetCenter());
									editMouseGrabPos = pos;
									mode = PASTE;

									selectedBrush->SetSelected(false);
									selectedBrush->Clear();
									
									/*Action *ac = new ApplyBrushAction(copiedBrush);
									ac->Perform();
									doneActionStack.push_back(ac);
									ClearUndoneActions();
									copiedBrush = NULL;*/
								}
								
								//copiedBrush->a
								if( copyBrushes.size() > 0 )
								{

									//pasteBrushes = copyBrushes;
									
									//CopyToPasteBrushes();
									//
									//pastePos = Vector2i( worldPos.x, worldPos.y );

									////find the overall bounding box of all the copied polygons
									//list<TerrainBrush*>::iterator tbIt = pasteBrushes.begin();
									//int trueLeft = (*tbIt)->left;
									//int trueRight = (*tbIt)->right;
									//int trueTop = (*tbIt)->top;								
									//int trueBot = (*tbIt)->bot;

									//++tbIt;
									//for( ; tbIt != pasteBrushes.end(); ++tbIt )
									//{
									//	if( (*tbIt)->left < trueLeft )
									//		trueLeft = (*tbIt)->left;
									//	if( (*tbIt)->right > trueRight )
									//		trueRight = (*tbIt)->right;
									//	if( (*tbIt)->top < trueTop )
									//		trueTop = (*tbIt)->top;
									//	if( (*tbIt)->bot > trueBot )
									//		trueBot = (*tbIt)->bot;
									//}

									//Vector2i trueCenter( (trueRight + trueLeft) / 2, (trueTop + trueBot)/2 );
								
									////move it to the cursors position originally
									//Vector2i startDiff = pastePos - trueCenter;
									//for( tbIt = pasteBrushes.begin(); tbIt != pasteBrushes.end(); ++tbIt )
									//{
									//	(*tbIt)->Move( startDiff );
									//}
								}

							}
							else if (ev.key.code == Keyboard::D && ev.key.control)
							{
								if (selectedBrush->objects.size() == 1)
								{
									SelectPtr sp = selectedBrush->objects.front();
									if (sp->selectableType == ISelectable::ACTOR)
									{
										ActorParams *ap = (ActorParams*)sp.get();

										selectedBrush->SetSelected(false);
										selectedBrush->Clear();

										ActorPtr aPtr(ap->Copy());
										aPtr->group = groups["--"];

										if (aPtr->groundInfo != NULL)
										{
											aPtr->AnchorToGround(*aPtr->groundInfo);
										}

										selectedBrush->AddObject(aPtr);
										selectedBrush->SetSelected(true);

										CreateActor(aPtr);
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

										for( PointMap::iterator it = selectedPoints.begin(); it != selectedPoints.end(); ++it )
										{
											(*it).first->RemoveSelectedPoints();
										}

										selectedPoints.clear();

										//go through each polygon and get rid of the actors which are deleted by deleting points
										//for( list<PolyPtr>::iterator it = selectedPolygons.begin(); 
										//	it != selectedPolygons.end(); ++it )
										//{
										//	for( EnemyMap::iterator mapIt = (*it)->enemies.begin(); mapIt != (*it)->enemies.end(); ++mapIt)
										//	{
										//		list<ActorPtr>::iterator et = (*mapIt).second.begin();
										//		while( et != (*mapIt).second.end() )
										//		{
										//			TerrainPoint *edgeEnd = (*et)->groundInfo->edgeStart->next;
										//			if( edgeEnd == NULL )
										//				edgeEnd = (*et)->groundInfo->ground->pointStart;

										//			bool deleted = (*(*et)->groundInfo->edgeStart).selected || edgeEnd->selected;
										//			if (deleted)
										//			{
										//				//delete enemy here


										//			//	(*et)->group->actors.remove( (*et ) );
										//			//	delete (*et); //deleting actor
										//			//	(*it)->enemies[(*et)->groundInfo->edgeStart].erase(et++); 
										//			}
										//			else
										//			{
										//				++et;
										//			}
										//		}
										//	}
										//}


										/*for( list<PolyPtr>::iterator it = selectedPolygons.begin(); 
											it != selectedPolygons.end(); ++it )
										{
											(*it)->RemoveSelectedPoints();
										}*/
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

										//these big loops add the terrain's actors to the
										//selected brush just on deletion, but not ones 
										//that are already selected
										SelectList actorsList;
										for (auto it = selectedBrush->objects.begin();
											it != selectedBrush->objects.end(); ++it)
										{
											if ((*it)->selectableType == ISelectable::ACTOR)
											{
												actorsList.push_back((*it));
											}
										}
										SelectList addedActorsList;
										for (auto it = selectedBrush->objects.begin();
											it != selectedBrush->objects.end(); ++it)
										{
											if ((*it)->selectableType == ISelectable::TERRAIN)
											{
												PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>((*it));

												for (auto objs = poly->enemies.begin(); objs != poly->enemies.end(); ++objs)
												{
													list<ActorPtr> &ap = (*objs).second;
													for (auto api = ap.begin(); api != ap.end(); ++api)
													{
														bool alreadyHere = false;
														for (auto ita = actorsList.begin();
															ita != actorsList.end(); ++ita)
														{
															if ((*api) == (*ita))
															{
																alreadyHere = true;
															}
														}
														if( !alreadyHere )
														{
															addedActorsList.push_back((*api));
														}
													}
												}

												
											}
										}

										for (auto it = addedActorsList.begin(); it != addedActorsList.end(); ++it)
										{
											selectedBrush->AddObject((*it));
										}
										Action *remove = new RemoveBrushAction( selectedBrush );
										remove->Perform();

										doneActionStack.push_back( remove );
									
										ClearUndoneActions();

										selectedBrush->SetSelected(false);
										selectedBrush->Clear();
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

									
									if( IsKeyPressed( Keyboard::G ) )
									{
										V2d graphPos = SnapPointToGraph(worldPos, 32);
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

									if( IsKeyPressed( Keyboard::G ) )
									{
										V2d graphPos = SnapPointToGraph( worldPos, 32 );
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
								for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
								{
									if ((*it)->selectableType == ISelectable::ISelectableType::TERRAIN)
									{
										boost::shared_ptr<TerrainPolygon> d = boost::static_pointer_cast<TerrainPolygon>((*it));
										d->ShowGrass(true);
									}
								}
								/*for( list<PolyPtr>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
								{
									(*it)->ShowGrass( true );
								}*/
							}
							else if( ev.key.code == Keyboard::B )
							{
								//showPoints = true;
								//showPoints = true;
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
											SelectPtr select = (*it);
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
											SelectPtr select = (*it);
											PolyPtr poly = boost::dynamic_pointer_cast<TerrainPolygon>( select );
											poly->SetLayer( 0 );
										}
									}
								}
							}
							else if( ev.key.code == Keyboard::E )
							{
								GridSelectPop( "terraintypeselect" );

								Action * action = new ModifyTerrainTypeAction(
									selectedBrush, tempGridX, tempGridY );
								action->Perform();
								doneActionStack.push_back( action );
							}
							else if (ev.key.code == Keyboard::I)
							{
								if (ev.key.shift)
								{
									topBound += borderMove;
									boundHeight -= borderMove;
								}
								else
								{
									topBound -= borderMove;
									boundHeight += borderMove;
								}
								UpdateFullBounds();
							}
							else if (ev.key.code == Keyboard::J)
							{
								if (ev.key.shift)
								{
									leftBound += borderMove;
									boundWidth -= borderMove;
								}
								else
								{
									leftBound -= borderMove;
									boundWidth += borderMove;
								}
								UpdateFullBounds();
							}
							else if (ev.key.code == Keyboard::L)
							{
								if (ev.key.shift)
								{
									boundWidth -= borderMove;
								}
								else
								{
									boundWidth += borderMove;
								}
								UpdateFullBounds();
							}
							/*else if (ev.key.code == Keyboard::K)
							{
								if (ev.key.shift)
								{
									boundHeight -= borderMove;
								}
								else
								{
									boundHeight += borderMove;
								}
								UpdateFullBounds();
							}*/
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
													if( IsKeyPressed( Keyboard::LShift ) )
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
													if( IsKeyPressed( Keyboard::LShift ) )
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
												if( IsKeyPressed( Keyboard::LShift ) )
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
														if( IsKeyPressed( Keyboard::LShift ) )
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
														if( IsKeyPressed( Keyboard::LShift ) )
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
								for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
								{
									if ((*it)->selectableType == ISelectable::ISelectableType::TERRAIN)
									{
										boost::shared_ptr<TerrainPolygon> d = boost::static_pointer_cast<TerrainPolygon>((*it));
										d->ShowGrass(false);
									}
								}

								//for( list<PolyPtr>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
								//{
								//	
								//	//showGrass = true;
								//	for( list<PolyPtr>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
								//	{
								//		(*it)->ShowGrass( false );
								//	}
								//}
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
			case PASTE:
			{
				switch (ev.type)
				{
					case Event::MouseButtonPressed:
					{
						if (ev.mouseButton.button == sf::Mouse::Button::Left)
						{
							PasteTerrain(copiedBrush);
							if (!HoldingControl())
							{
								mode = EDIT;
							}
						}
						break;
					}
					case Event::KeyPressed:
					{
						if (ev.key.code == Keyboard::X)
						{
							mode = EDIT;
						}
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
									else
									{
										trackingEnemy->PlaceEnemy();
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
								bool singleActor = selectedBrush->objects.size() == 1 
									&& selectedPoints.size() == 0
									&& selectedBrush->objects.front()->selectableType == ISelectable::ACTOR;
								bool singleImage = selectedBrush->objects.size() == 1
									&& selectedPoints.size() == 0
									&& selectedBrush->objects.front()->selectableType == ISelectable::IMAGE;
								//bool singlePoly = selectedBrush->objects.size() == 1 
								//	&& selectedPoints.size() == 0
								//	&& selectedBrush->objects.front()->selectableType == ISelectable::TERRAIN;
								bool onlyPoly = selectedBrush != NULL && !selectedBrush->objects.empty() && selectedBrush->terrainOnly;
								if( menuDownStored == EDIT && onlyPoly )
								{
									showPanel = terrainOptionsPanel;
									mode = menuDownStored;
								}
								else if( menuDownStored == EDIT && singleActor )
								{
									SetEnemyEditPanel();
									mode = menuDownStored;
								}
								else if (menuDownStored == EDIT && singleImage)
								{
									showPanel = editDecorPanel;
									SetDecorEditPanel();
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
								mapOptionsPanel->textBoxes["draintime"]->text.setString(to_string(drainSeconds));
								mapOptionsPanel->textBoxes["bosstype"]->text.setString(to_string(bossType));
								//mapOptionsPanel->textBoxes["envtype"]->text.setString(envName);
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
								if( selectedBrush->objects.size() == 1 ) //EDIT
								{
									//showPanel = trackingEnemy->panel;
									ISelectable *select = selectedBrush->objects.front().get();
									ActorParams *actor = (ActorParams*)select;
									showPanel = actor->type->panel;
									actor->SetPath( patrolPath );
									//((PatrollerParams*)selectedActor)->SetPath( patrolPath );
									mode = EDIT;
								}
								else
								{
									showPanel = trackingEnemy->panel;
									//ISelectable *select = selectedBrush->objects.front().get();
									//ActorParams *actor = (ActorParams*)select;
									tempActor->SetPath( patrolPath );
									//((PatrollerParams*)selectedActor)->SetPath( patrolPath );
									mode = CREATE_ENEMY;
								}
								//if( selectedActor != NULL )
								//{
								//	showPanel = selectedActor->type->panel;
								//	((PatrollerParams*)selectedActor)->SetPath( patrolPath );
								//	mode = EDIT;
								//}
								//else
								//{
								//	showPanel = trackingEnemy->panel;
								//	
								//	assert( selectedBrush->objects.size() == 1 );

								//	ISelectable *select = selectedBrush->objects.front().get();
								//	ActorParams *actor = (ActorParams*)select;
								//	actor->SetPath( patrolPath );
								//	//((PatrollerParams*)selectedActor)->SetPath( patrolPath );
								//	mode = EDIT;
								//	//mode = CREATE_ENEMY;
								//}
								
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
			case CREATE_BLOCKER_CHAIN:
				{
				minimumPathEdgeLength = 16;
				switch (ev.type)
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
					if ((ev.key.code == Keyboard::X || ev.key.code == Keyboard::Delete) && patrolPath.size() > 1)
					{
						patrolPath.pop_back();
					}
					else if (ev.key.code == Keyboard::Space)
					{
						if (selectedBrush->objects.size() == 1) //EDIT
						{
							//showPanel = trackingEnemy->panel;
							ISelectable *select = selectedBrush->objects.front().get();
							ActorParams *actor = (ActorParams*)select;
							showPanel = actor->type->panel;
							actor->SetPath(patrolPath);
							//((PatrollerParams*)selectedActor)->SetPath( patrolPath );
							mode = EDIT;
						}
						else
						{
							showPanel = trackingEnemy->panel;
							//ISelectable *select = selectedBrush->objects.front().get();
							//ActorParams *actor = (ActorParams*)select;
							tempActor->SetPath(patrolPath);
							//((PatrollerParams*)selectedActor)->SetPath( patrolPath );
							mode = CREATE_ENEMY;
						}
						//if( selectedActor != NULL )
						//{
						//	showPanel = selectedActor->type->panel;
						//	((PatrollerParams*)selectedActor)->SetPath( patrolPath );
						//	mode = EDIT;
						//}
						//else
						//{
						//	showPanel = trackingEnemy->panel;
						//	
						//	assert( selectedBrush->objects.size() == 1 );

						//	ISelectable *select = selectedBrush->objects.front().get();
						//	ActorParams *actor = (ActorParams*)select;
						//	actor->SetPath( patrolPath );
						//	//((PatrollerParams*)selectedActor)->SetPath( patrolPath );
						//	mode = EDIT;
						//	//mode = CREATE_ENEMY;
						//}

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
			case CREATE_RECT:
			{
				switch (ev.type)
				{
				case Event::MouseButtonPressed:
				{
					if (ev.mouseButton.button == Mouse::Left)
					{
						if (showPanel != NULL)
						{
							//cout << "edit mouse update" << endl;
							showPanel->Update(true, uiMouse.x, uiMouse.y);
							break;
						}

						if (!drawingCreateRect)
						{
							drawingCreateRect = true;
							createRectStartPoint = Vector2i(worldPos);
							createRectCurrPoint = Vector2i(worldPos);
						}
					}
					break;
				}
				case Event::MouseButtonReleased:
				{
					if (drawingCreateRect)
					{
						drawingCreateRect = false;

						createRectCurrPoint = Vector2i(worldPos);

						Vector2i rc = (createRectStartPoint + createRectCurrPoint) / 2;
						float width = abs(createRectCurrPoint.x - createRectStartPoint.x);
						float height = abs(createRectCurrPoint.y - createRectStartPoint.y);
						rectCreatingTrigger->SetRect(width, height, rc);

						if (trackingEnemy != NULL)
						{
							enemySprite.setPosition(Vector2f(rc));
							enemyQuad.setPosition(enemySprite.getPosition());
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
					//if ((ev.key.code == Keyboard::X || ev.key.code == Keyboard::Delete) && patrolPath.size() > 1)
					//{
					//	patrolPath.pop_back();
					//}
					if (ev.key.code == Keyboard::Space)
					{
						if (selectedBrush->objects.size() == 1) //EDIT
						{
							//showPanel = trackingEnemy->panel;
							ISelectable *select = selectedBrush->objects.front().get();
							AirTriggerParams *actor = (AirTriggerParams*)select;
							showPanel = actor->type->panel;
							//actor->SetPath(patrolPath);
							//((PatrollerParams*)selectedActor)->SetPath( patrolPath );
							mode = EDIT;
						}
						else
						{
							showPanel = trackingEnemy->panel;
							//ISelectable *select = selectedBrush->objects.front().get();
							//ActorParams *actor = (ActorParams*)select;
							//tempActor->SetPath(patrolPath);
							//((PatrollerParams*)selectedActor)->SetPath( patrolPath );
							mode = CREATE_ENEMY;
						}
					}
					//	else
					//	{
					//		showPanel = trackingEnemy->panel;
					//		//ISelectable *select = selectedBrush->objects.front().get();
					//		//ActorParams *actor = (ActorParams*)select;
					//		tempActor->SetPath(patrolPath);
					//		//((PatrollerParams*)selectedActor)->SetPath( patrolPath );
					//		mode = CREATE_ENEMY;
					//	}
					//}
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
			case SET_DIRECTION:
			{
				minimumPathEdgeLength = 16;

				switch (ev.type)
				{
				case Event::MouseButtonPressed:
				{
					if (ev.mouseButton.button == Mouse::Left)
					{
						Vector2i worldi(testPoint.x, testPoint.y);
						patrolPath.push_back(worldi);

						ActorParams *actor;
						if (tempActor != NULL)
						{
							actor = tempActor;
							mode = CREATE_ENEMY;
						}
						else
						{
							ISelectable *select = selectedBrush->objects.front().get();
							actor = (ActorParams*)select;
							mode = EDIT;
						}
						
						showPanel = actor->type->panel;
						actor->SetPath(patrolPath);
					}
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
			case CREATE_GATES:
				{
					switch( ev.type )
					{
					case Event::MouseButtonPressed:
						{
							if( ev.mouseButton.button == Mouse::Left )
							{
								TryPlaceGatePoint(worldPos);
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
					break;
				}
			case CREATE_IMAGES:
			{
				switch (ev.type)
				{
				case Event::MouseButtonPressed:
				{
					if (ev.mouseButton.button == Mouse::Left)
					{
						if (showPanel != NULL)
						{
							showPanel->Update(true, uiMouse.x, uiMouse.y);
						}
						else
						{
							DecorPtr dec(new DecorInfo(tempDecorSprite, currDecorLayer, currDecorName, currDecorTile));
							/*if (currDecorLayer > 0)
							{
								
								decorImagesBehindTerrain.push_back(dec);
								decorImagesBehindTerrain.sort(CompareDecorInfo);
							}
							else if (currDecorLayer < 0)
							{
								decorImagesFrontTerrain.push_back(DecorInfo(tempDecorSprite, currDecorLayer, currDecorName, currDecorTile));
								decorImagesFrontTerrain.sort(CompareDecorInfo);
							}
							else
							{
								decorImagesBetween.push_back(DecorInfo(tempDecorSprite, currDecorLayer, currDecorName, currDecorTile));
								decorImagesBetween.sort(CompareDecorInfo);
							}*/
							if (currDecorLayer > 0)
							{
								dec->myList = &decorImagesBehindTerrain;
							}
							else if (currDecorLayer < 0)
							{
								dec->myList = &decorImagesFrontTerrain;
							}
							else if (currDecorLayer == 0)
							{
								dec->myList = &decorImagesBetween;
							}
							CreateDecorImage(dec);
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
					if (ev.mouseButton.button == Mouse::Left)
					{
						if (showPanel != NULL)
						{
							showPanel->Update(false, uiMouse.x, uiMouse.y);
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
					if (showPanel != NULL)
					{
						showPanel->SendKey(ev.key.code, ev.key.shift);
						break;
					}

					if (ev.key.code == Keyboard::X || ev.key.code == Keyboard::Delete)
					{
						showPanel = decorPanel;
						/*if (trackingEnemy != NULL)
						{
							trackingEnemy = NULL;
							showPanel = enemySelectPanel;
						}*/
					}
					else if (ev.key.code == sf::Keyboard::Z && ev.key.control)
					{
						if (doneActionStack.size() > 0)
						{
							Action *action = doneActionStack.back();
							doneActionStack.pop_back();

							action->Undo();

							undoneActionStack.push_back(action);
						}
					}
					else if (ev.key.code == sf::Keyboard::Y && ev.key.control)
					{
						if (undoneActionStack.size() > 0)
						{
							Action *action = undoneActionStack.back();
							undoneActionStack.pop_back();

							action->Perform();

							doneActionStack.push_back(action);
						}
					}
					break;
				}
				}
				/*switch (ev.type)
				{
				case Event::MouseButtonPressed:
					break;
				}*/


				

				break;
			}
			case EDIT_IMAGES:
			{
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
						if( ev.key.code == Keyboard::S && ev.key.control  )
						{
							polygonInProgress->ClearPoints();
							cout << "writing to file: " << currentFile << endl;
							WriteFile(currentFile);
						}
						else if( ev.key.code == Keyboard::T && showPanel == NULL  )
						{
							quit = true;
						}
						else if( ev.key.code == Keyboard::Escape )
						{
							//if( sf::IsKeyPressed( sf::Keyboard::Escape ) )
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


		if (IsKeyPressed(Keyboard::Num4))
		{
			Vector2f halfSize(scaleSprite.getGlobalBounds().width / 2.f, 
				scaleSprite.getGlobalBounds().height / 2.f);
			scaleSprite.setPosition(Vector2f(pixelPos) - halfSize );
			scaleSpriteBGRect.setPosition(Vector2f(pixelPos) - halfSize );
		}
		else
		{
			scaleSprite.setPosition(0, 80);
			scaleSpriteBGRect.setPosition(0, 80);
		}

		// temp;
		//V2d temp1;
		//double tempQuant;
		switch( mode )
		{
		case CREATE_TERRAIN:
			{
				/*if( polygonInProgress->points.size() > 0 && IsKeyPressed( Keyboard::LShift ) ) 
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

				if( IsKeyPressed( Keyboard::G ) )
				{
					testPoint = SnapPointToGraph(testPoint, 32);
					showGraph = true;
				}
				else if (IsKeyPressed(Keyboard::F))
				{
					testPoint = SnapPosToPoint(testPoint, 8 * zoomMultiple);
					showPoints = true;
				}
				else
				{
					showPoints = false;
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

					
					if( cutChoose )
					{
						break;
					}

					if( false && showPoints && extendingPolygon == NULL )
					{

						bool none = true;
						if( inversePolygon != NULL )
						{
							for( TerrainPoint *curr = inversePolygon->pointStart; curr != NULL;
								curr = curr->next )
							{
								Vector2i pointPos = curr->pos;
								double dist = length( V2d( pointPos.x, pointPos.y ) - V2d( testPoint.x, testPoint.y ) );
								if( dist < 8 * zoomMultiple )
								{
									extendingPolygon = inversePolygon;
									extendingPoint = curr;
									none = false;
									break;
								}
							}
						}

						if( none )
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

					//if( ( !showPoints || ( false && showPoints && extendingPolygon != NULL )) && emptySpace )
					if( emptySpace )
					{
						
						Vector2i worldi( testPoint.x, testPoint.y );

						bool okay = true;
						if( extendingPolygon != NULL && polygonInProgress->numPoints > 0 )
						{
							//no error checking for extending polygon yet. ugh T_T you got this

							bool containsPoint = extendingPolygon->ContainsPoint( testPoint );
							/*if( !extendingPolygon->inverse )
							{
								okay = !containsPoint;
							}*/
							
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
										//okay = false;
										//break;
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
								if( curr == extendingPoint || ( polygonInProgress->pointEnd != NULL 
									&& curr->pos == polygonInProgress->pointEnd->pos ) )
								{
									continue;
								}
								Vector2i pointPos = curr->pos;
								double dist = length( V2d( pointPos.x, pointPos.y ) - V2d( testPoint.x, testPoint.y ) );
								if( dist < 8 * zoomMultiple )
								{
									for( TerrainPoint *progCurr = polygonInProgress->pointStart;
										progCurr != NULL; progCurr = progCurr->next )
									{
										//if( !extendingPolygon->PointTooCloseToPoints( progCurr->pos, minimumEdgeLength ) )
										{
											bool contains = extendingPolygon->ContainsPoint( 
												Vector2f( progCurr->pos.x, progCurr->pos.y ) );
											if( extendingPolygon->inverse )
												contains = !contains;

											if( polygonInProgress->numPoints == 1 )
											{
												contains = true;
											}
											//cout << "polygonin projgresaghgg: " << polygonInProgress->numPoints << endl;

											//okay this is buggy fix it in the morning. trying to make small cuts with
											//no in between points

											if( contains )
											{
												extendingPolygon->Cut2( extendingPoint, curr, polygonInProgress );	
												polygonInProgress->Reset();
												done = true;
												break;
											}
										}
									}

									if( !done )
									{
										ExtendPolygon( extendingPoint, curr, polygonInProgress );
										done = true;
									}
										//else if( extendEndTemp != NULL )
										//{
										//	MessagePop( "you must select a point next to the last point you selected to "
										//		"determine the winding" );
										//	//done = true;
										//	okay = false;
										//}
									
									
									//if( !done && okay )
									//{
									//	extendingPolygon->Extend( extendingPoint, curr, polygonInProgress, NULL );

									//	ExtendAdd();

									//	//polygonInProgress->points.clear();
									//	//polygonInProgress->Reset();
									//	//cout << "EXTENDING POLYGON" << endl;
									//

									//	polygonInProgress->Reset();

									//	extendingPolygon = NULL;
									//	extendingPoint = NULL;
									//	extendEndTemp = NULL;

									//	done = true;
									//	//cout << "done!" << endl;
									//	
									//}
									break;
									//ExtendPolygon();
									//polygonInProgress->FixWindingInverse();
									
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
									if( (*it)->SegmentTooClose( polygonInProgress->pointEnd->pos, worldi, minimumEdgeLength ) )
									{
										//validNearPolys = false;
										//break;
									}
								}
							}

							//cout << "valid: " << validNearPolys << endl;

							//if( validNearPolys )
							{
								if( polygonInProgress->numPoints > 0 && length( V2d( testPoint.x, testPoint.y ) 
									- Vector2<double>(polygonInProgress->pointEnd->pos.x, 
									polygonInProgress->pointEnd->pos.y )  ) >= minimumEdgeLength * std::max(zoomMultiple,1.0 ) )
								{
								//	cout << "check1" << endl;
									//if( PointValid( polygonInProgress->pointEnd->pos, worldi ) )
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
					IsKeyPressed( Keyboard::G ) )
				{
					pPoint = SnapPointToGraph( worldPos, 32 );
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
				
				if (IsKeyPressed(Keyboard::B))
				{
					showPoints = true;

				}
				else
				{
					ClearSelectedPoints();
					showPoints = false;
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
						if( IsKeyPressed( Keyboard::LShift ) )
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
						if( validMove && !(*it)->IsMovePointsOkay( pointGrabDelta, deltas ) )
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

									(*it)->UpdateLineColor((*it)->lines, prev, (*it)->GetPointIndex(prev) * 2);
									(*it)->UpdateLineColor((*it)->lines, curr, (*it)->GetPointIndex(curr) * 2);

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
							if( !(*it)->IsMovePolygonOkay(polyGrabDelta ) )
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
					/*for( list<PolyPtr>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
					{
						(*it)->SwitchGrass( worldPos );
					}*/
					for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
					{
						if ((*it)->selectableType == ISelectable::ISelectableType::TERRAIN)
						{
							boost::shared_ptr<TerrainPolygon> d = boost::static_pointer_cast<TerrainPolygon>((*it));
							d->SwitchGrass( worldPos );
						}
					}
				}
				
				}

				break;
			}
		case PASTE:
		{
			Vector2i pos(worldPos.x, worldPos.y);
			Vector2i delta = pos - editMouseGrabPos;
			copiedBrush->Move(delta);
			editMouseGrabPos = pos;
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

				if( showPanel == NULL && trackingEnemy != NULL  )
				{
					string name = trackingEnemy->name;
					
					if(trackingEnemy->canBeGrounded)
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

								bool contains = (*it)->ContainsPoint(Vector2f(testPoint.x, testPoint.y));

								if( ( contains && !(*it)->inverse ) || ( !contains && (*it)->inverse ) )
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
										
											if( name != "poi" )
											{
												enemySprite.setOrigin( enemySprite.getLocalBounds().width / 2, enemySprite.getLocalBounds().height );
												enemyQuad.setOrigin( enemyQuad.getLocalBounds().width / 2, enemyQuad.getLocalBounds().height );
											}
											/*else
											{
												enemyQuad.setOrigin( enemyQuad.getLocalBounds().width / 2, enemyQuad.getLocalBounds().height / 2 );
											}*/
											
											enemySprite.setPosition( closestPoint.x, closestPoint.y );
											enemySprite.setRotation( atan2( (cu - pr).y, (cu - pr).x ) / PI * 180 );

											
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

									break;
								}
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
		case CREATE_RECT:
		{
			if (showPanel != NULL)
			break;


			if (!panning && Mouse::isButtonPressed(Mouse::Left))
			{
				createRectCurrPoint = Vector2i(worldPos);

				Vector2i rc = (createRectStartPoint + createRectCurrPoint) / 2;
				float width = abs(createRectCurrPoint.x - createRectStartPoint.x);
				float height = abs(createRectCurrPoint.y - createRectStartPoint.y);
				rectCreatingTrigger->SetRect(width, height, rc);

				if (trackingEnemy != NULL)
				{
					enemySprite.setPosition(Vector2f(rc));
					enemyQuad.setPosition(enemySprite.getPosition());
				}
			}
			break;
		}
		case CREATE_PATROL_PATH:
			{
				/*if( //polygonInProgress->points.size() > 0 && 
					IsKeyPressed( Keyboard::G ) )
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
					//double test = 100;
					//worldPos before testPoint
					V2d temp = V2d( testPoint.x, testPoint.y ) - Vector2<double>(patrolPath.back().x, 
						patrolPath.back().y );
					double tempQuant = length( temp );
					if( tempQuant >= minimumPathEdgeLength * std::max(zoomMultiple,1.0 ) 
						&& tempQuant > patrolPathLengthSize / 2 )
					{

						if( patrolPathLengthSize > 0 )
						{
							V2d temp1 = V2d(patrolPath.back().x, patrolPath.back().y );
							temp = normalize( V2d( testPoint.x, testPoint.y ) -  temp1 ) 
								* (double)patrolPathLengthSize + temp1;
							Vector2i worldi( temp.x, temp.y );
							patrolPath.push_back( worldi );
						}
						else
						{
							Vector2i worldi( testPoint.x, testPoint.y );
							patrolPath.push_back( worldi );
						}
						

						
					}					
				}
				break;
			}
		case CREATE_BLOCKER_CHAIN:
		{
			if (showPanel != NULL)
				break;


			if (!panning && Mouse::isButtonPressed(Mouse::Left))
			{
				//double test = 100;
				//worldPos before testPoint
				V2d temp = V2d(testPoint.x, testPoint.y) - Vector2<double>(patrolPath.back().x,
					patrolPath.back().y);
				double tempQuant = length(temp);
				if (tempQuant >= minimumPathEdgeLength * std::max(zoomMultiple, 1.0)
					&& tempQuant > patrolPathLengthSize / 2)
				{

					if (patrolPathLengthSize > 0)
					{
						V2d temp1 = V2d(patrolPath.back().x, patrolPath.back().y);
						temp = normalize(V2d(testPoint.x, testPoint.y) - temp1)
							* (double)patrolPathLengthSize + temp1;
						Vector2i worldi(temp.x, temp.y);
						patrolPath.push_back(worldi);
					}
					else
					{
						Vector2i worldi(testPoint.x, testPoint.y);
						patrolPath.push_back(worldi);
					}



				}
			}
			break;
		}
		case SET_DIRECTION:
		{
			if (showPanel != NULL)
				break;


			//if (!panning && Mouse::isButtonPressed(Mouse::Left))
			//{
			//	//double test = 100;
			//	//worldPos before testPoint
			//	//V2d temp = V2d(testPoint.x, testPoint.y) - Vector2<double>(patrolPath.back().x,
			//	//	patrolPath.back().y);
			//	Vector2i worldi(testPoint.x, testPoint.y);
			//	patrolPath.push_back(worldi);

			//	/*double tempQuant = length(temp);
			//	if (tempQuant >= minimumPathEdgeLength * std::max(zoomMultiple, 1.0)
			//		&& tempQuant > patrolPathLengthSize / 2)
			//	{

			//		if (patrolPathLengthSize > 0)
			//		{
			//			V2d temp1 = V2d(patrolPath.back().x, patrolPath.back().y);
			//			temp = normalize(V2d(testPoint.x, testPoint.y) - temp1)
			//				* (double)patrolPathLengthSize + temp1;
			//			Vector2i worldi(temp.x, temp.y);
			//			patrolPath.push_back(worldi);
			//		}
			//		else
			//		{
			//			Vector2i worldi(testPoint.x, testPoint.y);
			//			patrolPath.push_back(worldi);
			//		}



			//	}*/
			//}
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
		case CREATE_IMAGES:
		{
			if (showPanel == NULL)
			{
				Vector2f p = preScreenTex->mapPixelToCoords(pixelPos);

				//p.x *= 
				//p.y *= 1080.f / w->getSize().y;
				//cout << "p: " << p.x << ", " << p.y << endl;
				tempDecorSprite.setPosition(p);

				tempDecorSprite.setOrigin(tempDecorSprite.getLocalBounds().width / 2, tempDecorSprite.getLocalBounds().height / 2);
				tempDecorSprite.setRotation(0);
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
		
		
		Vector2f vSize = view.getSize();

		float zoom = vSize.x / 960;
		Vector2f botLeft(view.getCenter().x - vSize.x / 2, view.getCenter().y + vSize.y / 2);
		for (int i = 0; i < 9 * MAX_TERRAINTEX_PER_WORLD; ++i)
		{
			if (terrainTextures[i] != NULL)
			{
				polyShaders[i].setUniform("zoom", zoom);
				polyShaders[i].setUniform("topLeft", botLeft); 
				//just need to change the name topleft  to botleft eventually
			}
			
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


		if (showBG)
		{
			DrawBG(preScreenTex);
		}
		/*sf::RectangleShape parTest( Vector2f( 1000, 1000 ) );
		parTest.setFillColor( Color::Red );
		parTest.setPosition( 0, 0 );
		preScreenTex->draw( parTest );*/




		preScreenTex->draw(border, 8, sf::Lines);

		for (auto it = decorImagesBehindTerrain.begin(); it != decorImagesBehindTerrain.end(); ++it)
		{
			preScreenTex->draw((*it)->spr);
		}

		Draw();


		for (auto it = decorImagesBetween.begin(); it != decorImagesBetween.end(); ++it)
		{
			(*it)->Draw(preScreenTex);
		}

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
					{
						tempActor->Draw( preScreenTex );
					}
					else
					{
						preScreenTex->draw( enemySprite );
					}
					preScreenTex->draw( enemyQuad );
				}
				break;
			}
		case CREATE_RECT:
		{
			rectCreatingTrigger->Draw(preScreenTex);
			/*if (trackingEnemy != NULL)
			{
				if (tempActor != NULL)
				{
					tempActor->Draw(preScreenTex);
				}
				else
				{
					preScreenTex->draw(enemySprite);
				}
				preScreenTex->draw(enemyQuad);
			}
			else
			{
				ISelectable *select = selectedBrush->objects.front().get();
				AirTriggerParams *airTrigger = (AirTriggerParams*)select;

				airTrigger->Draw(preScreenTex);
			}*/
			

			break;
		}
		case CREATE_PATROL_PATH:
			{

				if( trackingEnemy != NULL )
				{
					if( tempActor != NULL )
						tempActor->Draw( preScreenTex );
					else
					{
						preScreenTex->draw( enemySprite );
					}					
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
		case CREATE_BLOCKER_CHAIN:
		{

			if (trackingEnemy != NULL)
			{
				if (tempActor != NULL)
					tempActor->Draw(preScreenTex);
				else
				{
					preScreenTex->draw(enemySprite);
				}
				preScreenTex->draw(enemyQuad);
			}
			int pathSize = patrolPath.size();
			if (pathSize > 0)
			{
				Vector2i backPoint = patrolPath.back();

				Color validColor = Color::Green;
				Color invalidColor = Color::Red;
				Color colorSelection;
				if (true)
				{
					colorSelection = validColor;
				}
				sf::Vertex activePreview[2] =
				{
					sf::Vertex(sf::Vector2<float>(backPoint.x, backPoint.y), colorSelection),
					sf::Vertex(sf::Vector2<float>(testPoint.x, testPoint.y), colorSelection)
				};
				preScreenTex->draw(activePreview, 2, sf::Lines);

				if (pathSize > 1)
				{
					VertexArray v(sf::LinesStrip, pathSize);
					int i = 0;
					for (list<sf::Vector2i>::iterator it = patrolPath.begin();
						it != patrolPath.end(); ++it)
					{
						v[i] = Vertex(Vector2f((*it).x, (*it).y));
						++i;
					}
					preScreenTex->draw(v);
				}
			}

			if (pathSize > 0) //always
			{
				CircleShape cs;
				cs.setRadius(5 * zoomMultiple);
				cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
				cs.setFillColor(Color::Green);


				for (list<Vector2i>::iterator it = patrolPath.begin(); it != patrolPath.end(); ++it)
				{
					cs.setPosition((*it).x, (*it).y);
					preScreenTex->draw(cs);
				}
			}
			break;
		}
		case SET_DIRECTION:
		{

			if (trackingEnemy != NULL)
			{
				if (tempActor != NULL)
					tempActor->Draw(preScreenTex);
				else
				{
					preScreenTex->draw(enemySprite);
				}
				preScreenTex->draw(enemyQuad);
			}
			int pathSize = patrolPath.size();
			if (pathSize > 0)
			{
				Vector2i backPoint = patrolPath.back();

				Color validColor = Color::Green;
				Color invalidColor = Color::Red;
				Color colorSelection;
				if (true)
				{
					colorSelection = validColor;
				}
				sf::Vertex activePreview[2] =
				{
					sf::Vertex(sf::Vector2<float>(backPoint.x, backPoint.y), colorSelection),
					sf::Vertex(sf::Vector2<float>(testPoint.x, testPoint.y), colorSelection)
				};
				preScreenTex->draw(activePreview, 2, sf::Lines);

				if (pathSize > 1)
				{
					VertexArray v(sf::LinesStrip, pathSize);
					int i = 0;
					for (list<sf::Vector2i>::iterator it = patrolPath.begin();
						it != patrolPath.end(); ++it)
					{
						v[i] = Vertex(Vector2f((*it).x, (*it).y));
						++i;
					}
					preScreenTex->draw(v);
				}
			}

			if (pathSize > 0) //always
			{
				CircleShape cs;
				cs.setRadius(5 * zoomMultiple);
				cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
				cs.setFillColor(Color::Green);


				for (list<Vector2i>::iterator it = patrolPath.begin(); it != patrolPath.end(); ++it)
				{
					cs.setPosition((*it).x, (*it).y);
					preScreenTex->draw(cs);
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
					GridSelectPop( "gateselect" );

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
						GridSelectPop( "gateselect" );

						if( tempGridResult == "delete" )
						{

						}
						else
						{
							Action * action = new CreateGateAction( testGateInfo, tempGridResult );
							action->Perform();
							doneActionStack.push_back( action );
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
		
		

		//playerSprite.setPosition( player->position.x, player->position.y );

		//preScreenTex->draw( playerSprite );
		
		//preScreenTex->draw( iconSprite );

		

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


		for (auto it = decorImagesFrontTerrain.begin(); it != decorImagesFrontTerrain.end(); ++it)
		{
			preScreenTex->draw((*it)->spr);
		}


		if( zoomMultiple > 7 )
		{
			playerZoomIcon.setPosition( player->position.x, player->position.y );
			playerZoomIcon.setScale( zoomMultiple * 1.8, zoomMultiple * 1.8 );
			preScreenTex->draw( playerZoomIcon );
		}




		preScreenTex->draw( fullBounds );

		if (mode == CREATE_IMAGES)
		{
			if( showPanel == NULL )
				preScreenTex->draw(tempDecorSprite);
		}

		preScreenTex->setView( uiView );

		stringstream cursorPosSS;
		stringstream scaleTextSS;
		if( mode == CREATE_PATROL_PATH || mode == CREATE_BLOCKER_CHAIN || mode == SET_DIRECTION )
		{
			V2d temp = V2d( testPoint.x, testPoint.y ) - Vector2<double>(patrolPath.back().x, 
				patrolPath.back().y );
			cursorPosSS << (int)temp.x << ", " << (int)temp.y;
		}
		else
		{
			cursorPosSS << (int)worldPos.x << ", " << (int)worldPos.y;
		}

		Vector2f size = uiView.getSize();


		//float scaleDiff = zoomMultiple - 1.f;//1.f / zoomMultiple;
		//cout << "viewsize: " << view.getSize().x << ", mult: " << zoomMultiple << endl;
		float sca = view.getSize().x / 960.f / 2.f;
		scaleSprite.setScale( 1.f / sca, 1.f / sca );
		scaleTextSS << "scale: x" << scaleSprite.getScale().x;
		scaleSpriteBGRect.setSize(Vector2f( scaleSprite.getGlobalBounds().width, 
			scaleSprite.getGlobalBounds().height ) );
		preScreenTex->draw(scaleSpriteBGRect);
		preScreenTex->draw(scaleSprite);
		cursorLocationText.setString( cursorPosSS.str() );
		preScreenTex->draw( cursorLocationText );
		scaleText.setString(scaleTextSS.str());
		
		preScreenTex->draw(scaleText);

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
					textblue.setFillColor( sf::Color::White );
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
					textgreen.setFillColor( sf::Color::White );
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
					textorange.setFillColor( sf::Color::White );
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
					textred.setFillColor( sf::Color::White );
					textred.setOrigin( textred.getLocalBounds().width / 2, textred.getLocalBounds().height / 2 );
					textred.setPosition( (menuDownPos + upperLeftPos).x, (menuDownPos + upperLeftPos).y );
					preScreenTex->draw( textred );

					cs.setFillColor( COLOR_MAGENTA );
					cs.setPosition( (menuDownPos + topPos).x, (menuDownPos + topPos).y );
					preScreenTex->draw( cs );

					sf::Text textmag;

					bool singleActor = selectedBrush->objects.size() == 1 
						&& selectedPoints.size() == 0
						&& selectedBrush->objects.front()->selectableType == ISelectable::ACTOR;
					bool singleImage = selectedBrush->objects.size() == 1
						&& selectedPoints.size() == 0
						&& selectedBrush->objects.front()->selectableType == ISelectable::IMAGE;
					bool onlyPoly = selectedBrush != NULL && !selectedBrush->objects.empty() && selectedBrush->terrainOnly;

					if( menuDownStored == EditSession::EDIT && singleActor )// && selectedActor != NULL )
					{
						textmag.setString( "EDIT\nENEMY" );
					}
					else if (menuDownStored == EditSession::EDIT && singleImage)// && selectedActor != NULL )
					{
						textmag.setString("EDIT\nIMAGE");
					}
					else if( menuDownStored == EditSession::EDIT && selectedGate != NULL )
					{
						textmag.setString( "EDIT\nGATE" );
					}
					else if( menuDownStored == EditSession::EDIT && onlyPoly )
					{
						textmag.setString( "TERRAIN\nOPTIONS" );
					}
					else
					{
						textmag.setString( "EDIT" );
					}
				
					
					textmag.setFont( arial );
					textmag.setCharacterSize( 14 );
					textmag.setFillColor( sf::Color::White );
					textmag.setOrigin( textmag.getLocalBounds().width / 2, textmag.getLocalBounds().height / 2 );
					textmag.setPosition( (menuDownPos + topPos).x, (menuDownPos + topPos).y );
					preScreenTex->draw( textmag );

					break;
				}
			case EDIT:
				{
					
					break;
				}
			case CREATE_IMAGES:
				
				break;
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
	
	preScreenTex->setView(oldPreTexView);
	w->setView(oldWindowView);
	//w->setMouseCursorVisible(false);

	mainMenu->SetMouseGrabbed(oldMouseGrabbed);
	mainMenu->SetMouseVisible(oldMouseVis);

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
					if( ff > .99 )
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

void EditSession::ButtonCallback( Button *b, const std::string & e )
{
	//cout << "start of callback!: " << groups["--"]->actors.size() << endl;
	Panel *p = b->owner;
	
	if (p == editDecorPanel)
	{
		if (b->name == "ok")
		{
			showPanel = NULL;
		}
	}
	else if (p->name == "airtrigger_options")
	{
		if (b->name == "ok")
		{
			RegularOKButton();
		}
		else if (b->name == "createrect")
		{
			//PatrollerParams *patroller = (PatrollerParams*)selectedActor;
			if (mode == EDIT)
			{
				ISelectable *select = selectedBrush->objects.front().get();
				AirTriggerParams *airTrigger = (AirTriggerParams*)select;
				rectCreatingTrigger = airTrigger;
			}
			else if (mode == CREATE_ENEMY)
			{
				rectCreatingTrigger = (AirTriggerParams*)tempActor;
			}

			showPanel = NULL;
			
			mode = CREATE_RECT;
			drawingCreateRect = false;
		}
	}
	else if( p->name == "map_options" )
	{
		if (b->name == "ok")
		{
			int minEdgeSize;

			stringstream ss;
			//string s = p->textBoxes["minedgesize"]->text.getString().toAnsiString();
			string s = p->textBoxes["minedgesize"]->text.getString().toAnsiString();
			string drainStr = p->textBoxes["draintime"]->text.getString().toAnsiString();
			string bossTypeStr = p->textBoxes["bosstype"]->text.getString().toAnsiString();
			ss << s;

			ss >> minEdgeSize;

			if (ss.fail())
			{
				cout << "stringstream to integer parsing error" << endl;
				ss.clear();
				assert(false);
			}

			if (minEdgeSize < 8)
			{
				minimumEdgeLength = 8;

				p->textBoxes["minedgesize"]->text.setString("8");

				MessagePop("minimum edge length too low.\n Set to minimum of 8");
				//assert( false && "made min edge length too small!" );
			}
			else
			{
				minimumEdgeLength = minEdgeSize;
			}
			//stringstream ss2;
			ss.clear();

			ss << drainStr;

			int dSecs;
			ss >> dSecs;

			if (!ss.fail())
			{
				drainSeconds = dSecs;
			}

			ss.clear();

			ss << bossTypeStr;

			int bType;
			ss >> bType;

			if (!ss.fail())
			{
				bossType = bType;
			}


			showPanel = NULL;
		}
		else if (b->name == "envtype")
		{
			GridSelectPop("bg");
		}
	}
	else if( p->name == "terrain_options" )
	{
		if (b->name == "ok")
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
	else if( p->name == "error_popup" )
	{
		if (b->name == "ok")
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
	else
	{
		if (b->name == "ok")
		{
			RegularOKButton();
		}
		else if (b->name == "createpath" || b->name == "createrail")
		{
			RegularCreatePathButton();
		}
		else if (b->name == "createchain")
		{
			RegularCreatePathButton();
			mode = CREATE_BLOCKER_CHAIN;
		}
		else if (b->name == "setdirection")
		{
			RegularCreatePathButton();
			mode = SET_DIRECTION;
		}
	}
}

void EditSession::TextBoxCallback( TextBox *tb, const std::string & e )
{
	//to be able to show previews in real time
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
	else if( p->name == "poi_options" )
	{
		if( tb->name == "camzoom" )
		{
			if( mode == EDIT )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				PoiParams *poi = (PoiParams*)select;
				poi->SetParams();
				//curveTurret->monitorType = GetMonitorType( p );
			}
			else if( mode == CREATE_ENEMY )
			{
				PoiParams *poi = (PoiParams*)tempActor;
				poi->SetParams();
			}
		}
	}
	else if (p == editDecorPanel)
	{
		SetDecorParams();
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
			trackingEnemy = types[name];

			enemySprite = trackingEnemy->GetSprite(false);

			enemyQuad.setSize( Vector2f( trackingEnemy->width, trackingEnemy->height ) );

			showPanel = NULL;

			cout << "set your cursor as the image" << endl;
		}
		else
		{
			cout << "not set" << endl;
		}
	}
	else if( panel == gateSelectorPopup || panel == terrainSelectorPopup )
	{
		cout << "callback!" << endl;
		if( name != "not set" )
		{
			cout << "real result: " << name << endl;
			tempGridResult = name;
			tempGridX = gs->selectedX;
			tempGridY = gs->selectedY;
			//showPanel = NULL;
		}
		else
		{
		//	cout << "not set" << endl;
		}
	}
	else if (panel == decorPanel)
	{
		if (name != "not set")
		{
			//cout << "real result: " << name << endl;
			currDecorName = name;
			ts_currDecor = decorTSMap[currDecorName];
			int ind = gs->selectedY * gs->xSize + gs->selectedX;
			currDecorTile = decorTileIndexes[ind];


			tempDecorSprite.setTexture(*ts_currDecor->texture);
			tempDecorSprite.setTextureRect(ts_currDecor->GetSubRect(currDecorTile));

			string layerStr = decorPanel->textBoxes["layer"]->text.getString().toAnsiString();
			stringstream tempSS;

			tempSS << layerStr;

			int cdLayer;
			tempSS >> cdLayer;

			if (!tempSS.fail())
			{
				currDecorLayer = cdLayer;
			}
			else
			{
				decorPanel->textBoxes["layer"]->text.setString("0");
			}

			showPanel = NULL;
			//ts_currDecor = GetTileset( currDecorName + string(".png"),  )
			//tempDecorSprite.setTexture(ts_currDecor)
			/*tempGridResult = name;
			tempGridX = gs->selectedX;
			tempGridY = gs->selectedY;*/
		}
		else
		{
			//	cout << "not set" << endl;
		}
	}
	else if (panel == bgPopup)
	{
		if (name != "not set")
		{
			if (Background::SetupFullBG(name, *this, currBackground, scrollingBackgrounds))
			{
				tempGridResult = name;
				envName = name;
			}
		}
	}
	else if (panel->name == "shard_options")
	{
		int world = gs->selectedX / 11;
		int realX = gs->selectedX % 11;
		int realY = gs->selectedY;

		ShardParams *shard = NULL;
		if (mode == EDIT)
		{
			ISelectable *select = selectedBrush->objects.front().get();
			shard = (ShardParams*)select;
		}
		else if (mode == CREATE_ENEMY)
		{
			shard = (ShardParams*)tempActor;
		}
		else
		{
			assert(0);
		}
		shard->SetShard(world, realX, realY);

		panel->labels["shardtype"]->setString(name);
	}
}

void EditSession::LoadDecorImages()
{
	ifstream is;
	is.open("Resources/decor.txt");
	if (is.is_open())
	{
		string name;
		int width;
		int height;
		int tile;
		while (!is.eof())
		{
			is >> name;

			is >> width;
			is >> height;

			is >> tile;

			string fullName = name + string(".png");
			
			Tileset *ts = GetTileset(fullName, width, height);
			assert(ts != NULL);
			decorTSMap[name] = ts;
			decorTileIndexMap[name].push_back(tile);
		}
	}
	else
	{
		assert(0);
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
			//cout << "BLAHBADIOHFWEIHEGHWEAOHGEAWHGEWAHG" << endl;
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
	else if( p->name == "poi_options" )
	{
		if( cb->name == "camprops" )
		{
			if( mode == EDIT )
			{
				ISelectable *select = selectedBrush->objects.front().get();				
				PoiParams *poi = (PoiParams*)select;
				poi->SetParams();
				//curveTurret->monitorType = GetMonitorType( p );
			}
			else if( mode == CREATE_ENEMY )
			{
				PoiParams *poi = (PoiParams*)tempActor;
				poi->SetParams();
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

sf::Vector2f EditSession::SnapPosToPoint(sf::Vector2f &p, double radius)
{
	for (auto it = polygons.begin(); it != polygons.end(); ++it)
	{
		TerrainPoint *closePoint = (*it)->GetClosePoint( radius, V2d(p));
		if (closePoint != NULL)
		{
			return Vector2f(closePoint->pos);
		}
	}

	return p;
}

void EditSession::InitDecorPanel()
{
	int w = 8;
	int h = 5;
	int sw = 128;
	int sh = 128;
	LoadDecorImages();
	decorPanel = new Panel("decorpanel", 650, 800, this);
	allPopups.push_back(decorPanel);
	GridSelector *gs = decorPanel->AddGridSelector("decorselector", Vector2i(0, 0), w, h, sw, sh, false, true );
	decorPanel->AddTextBox("layer", Vector2i( 0, 800), 100, 3, "0");
	decorTileIndexes = new int[w*h];
	

	//decorPanel->textBoxes["layer"]->text

	gs->active = true;

	int ind = 0;
	int x, y;
	for (auto it = decorTSMap.begin(); it != decorTSMap.end(); ++it)
	{
		for (auto tit = decorTileIndexMap[(*it).first].begin(); 
			tit != decorTileIndexMap[(*it).first].end(); ++tit)
		{
			x = ind % w;
			y = ind / w;
			Tileset *ts = (*it).second;
			Sprite s(*ts->texture);
			s.setTextureRect(ts->GetSubRect((*tit)));
			decorTileIndexes[ind] = (*tit);
			float texX = ts->texture->getSize().x;
			float texY = ts->texture->getSize().y;
			s.setScale(((float)sw) / ts->tileWidth, ((float)sh) / ts->tileHeight);
			//s.setTextureRect(IntRect(0, 0, sw, sh));
			//gs->Set(x, y, s, (*it).first);
			gs->Set(x, y, s, (*it).first);

			++ind;
		}	
	}

	editDecorPanel = new Panel("editdecorpanel", 500, 500, this);
	allPopups.push_back(editDecorPanel);
	editDecorPanel->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");

	editDecorPanel->AddTextBox("xpos", Vector2i(20, 20), 200, 20, "x");
	editDecorPanel->AddTextBox("ypos", Vector2i( 200, 20), 200, 20, "y");
	editDecorPanel->AddTextBox("rotation", Vector2i(20, 100), 200, 20, "r");
	editDecorPanel->AddTextBox("xscale", Vector2i(20, 180), 200, 20, "x");
	editDecorPanel->AddTextBox("yscale", Vector2i(200, 180), 200, 20, "y");
	editDecorPanel->AddTextBox("layer", Vector2i(20, 280), 200, 20, "l");
	//editDecorPanel->AddTextBox("rotation", Vector2i(200, 100), 200, 20, "r");
	//editDecorPanel->AddTextBox("strength", Vector2i(20, 200), 200, 3, "");
}

int EditSession::CountSelectedPoints()
{
	int count = 0;
	for( PointMap::iterator it = selectedPoints.begin(); it != selectedPoints.end(); ++it )
	{
		count += (*it).second.size();
	}
	return count;
	//return selectedPoints[(*it).get()].push_back( PointMoveInfo( tp ) );
	
	//int count = 0;
	//for( list<PolyPtr>::iterator it = selectedPolygons.begin(); it != selectedPolygons.end(); ++it )
	//{
	//	for( TerrainPoint *curr = (*it)->pointStart; curr != NULL; curr = curr->next )
	//	{
	//		if( curr->selected ) //selected
	//		{
	//			++count;
	//		}
	//	}
	//}
	//return count;
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

void EditSession::SetEnemyGridIndex( GridSelector *gs, int x, int y, const std::string &eName)
{
	gs->Set(x, y, types[eName]->GetSprite(gs->tileSizeX, gs->tileSizeY), eName);
}

void EditSession::SetupEnemyTypes()
{
	for (int i = 0; i < 8; ++i)
	{
		list<string> &wen = worldEnemyNames[i];
		for (auto it = wen.begin(); it != wen.end(); ++it)
		{
			SetupEnemyType((*it));
		}
	}

	for (auto it = extraEnemyNames.begin(); it != extraEnemyNames.end(); ++it)
	{
		SetupEnemyType((*it));
	}
}

void EditSession::SetupEnemyType(const std::string &name)
{
	types[name] = new ActorType(name, CreateOptionsPanel(name));
}

void EditSession::RegularOKButton()
{
	if (mode == EDIT)
		//if( mode == EDIT && selectedActor != NULL )
	{
		ISelectable *select = selectedBrush->objects.front().get();
		ActorParams *ap = (ActorParams*)select;
		ap->SetParams();
	}
	else if (mode == CREATE_ENEMY)
	{
		ActorPtr ac(tempActor);
		ac->SetParams();
		ac->group = groups["--"];

		CreateActor(ac);

		tempActor = NULL;
	}
	showPanel = NULL;
}

void EditSession::RegularCreatePathButton()
{
	mode = CREATE_PATROL_PATH;
	showPanel = NULL;
	Vector2i front = patrolPath.front();
	patrolPath.clear();
	patrolPath.push_back(front);
	patrolPathLengthSize = 0;
}

void EditSession::DrawBG(sf::RenderTarget *target)
{
	currBackground->Draw(target);

	sf::View viewP = view;
	viewP.setSize(1920, 1080);
	viewP.setCenter(960, 0);
	target->setView(viewP);
	for (list<ScrollingBackground*>::iterator it = scrollingBackgrounds.begin();
		it != scrollingBackgrounds.end(); ++it)
	{
		(*it)->Draw(target);
	}
	target->setView(view);
}

void EditSession::SelectPoint(TerrainPolygon *poly,
	TerrainPoint *point)
{
	if (!point->selected)
	{
		selectedPoints[poly].push_back(PointMoveInfo(point));
		point->selected = true;
	}
}

void EditSession::DeselectPoint(TerrainPolygon *poly,
	TerrainPoint *point)
{
	if (point->selected)
	{
		point->selected = false;
		auto & infoList = selectedPoints[poly];
		for (auto it = infoList.begin(); it != infoList.end(); ++it)
		{
			if ((*it).point == point)
			{
				infoList.erase(it);
				break;
			}
		}
	}
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
		if( IsKeyPressed( Keyboard::LShift ) )
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

					poly->UpdateLineColor(poly->lines, prev, poly->GetPointIndex(prev) * 2);
					poly->UpdateLineColor(poly->lines, curr, poly->GetPointIndex(curr) * 2);

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

void EditSession::ExtendPolygon( TerrainPoint *startPoint,
		TerrainPoint *endPoint, PolyPtr inProgress )
{
	//TerrainPolygon *p 
	TerrainPoint *newStartPoint = NULL;
	TerrainPoint *newEndPoint = NULL;
	PolyPtr newPoly( new TerrainPolygon(*extendingPolygon, true ) ); //this should be false
	//but i haven't implemented the false stuff yet
	for( TerrainPoint *curr = newPoly->pointStart; curr != NULL; curr = curr->next )
	{
		if( curr->pos == startPoint->pos )
		{
			newStartPoint = curr;
			break;
		}
	}

	assert( newStartPoint != NULL );

	for( TerrainPoint *curr = newPoly->pointStart; curr != NULL; curr = curr->next )
	{
		if( curr->pos == endPoint->pos )
		{
			newEndPoint = curr;
			break;
		}
	}

	assert( newEndPoint != NULL );

	newPoly->Extend2( newStartPoint, newEndPoint, polygonInProgress );

	//PolyPtr chosen( choice );
	SelectPtr sp = boost::dynamic_pointer_cast< ISelectable>( newPoly );

	progressBrush->Clear();
	progressBrush->AddObject( sp );

	Brush orig;
	
	SelectPtr spe = boost::dynamic_pointer_cast<ISelectable>( extendingPolygon );
	orig.AddObject( spe );
	
	Action * action = new ReplaceBrushAction( &orig, progressBrush );
	action->Perform();
	doneActionStack.push_back( action );

	ClearUndoneActions();

	progressBrush->Clear();

	//ExtendAdd();

	polygonInProgress->Reset();

	extendingPolygon = NULL;
	extendingPoint = NULL;

	//if( polygonInProgress->numPoints > 1 )
	//{
	//	//test final line

	//	list<PolyPtr>::iterator it = polygons.begin();
	//	bool added = false;
	//	polygonInProgress->Finalize();
	//	bool recursionDone = false;
	//	PolyPtr currentBrush = polygonInProgress;

	//		while( it != polygons.end() )
	//		{
	//			PolyPtr temp = (*it);
	//			if( temp != currentBrush && currentBrush->IsTouching( temp.get() ) )
	//			{
	//				//cout << "before addi: " << (*it)->points.size() << endl;
	//					
	//				Add( currentBrush, temp );

	//				polygonInProgress->Reset();
	//					
	//				//cout << "after adding: " << (*it)->points.size() << endl;
	//				polygons.erase( it );

	//				currentBrush = temp;

	//				it = polygons.begin();

	//				added = true;
	//						
	//				continue;
	//			}
	//			else
	//			{
	//				//cout << "not" << endl;
	//			}
	//			++it;
	//		}
	//			
	//	//add final check for validity here
	//			
	//	if( !added )
	//	{
	//		//polygonInProgress->Finalize();
	//		//polygons.push_back( polygonInProgress );
	//		//polygonInProgress = new TerrainPolygon(&grassTex );
	//	}
	//	else
	//	{
	//		polygons.push_back( currentBrush );
	//		//polygonInProgress->Reset();
	//	}
	//}


	//polygonInProgress->Reset();
}

void EditSession::ChooseCutPoly( TerrainPolygon *choice )
{
	cutChoose = false;
	cutChooseUp = false;

	if( choice == cutPoly0 )
		delete cutPoly1;
	else
		delete cutPoly0;

	choice->SetMaterialType( extendingPolygon->terrainWorldType, extendingPolygon->terrainVariation );
	if( extendingPolygon->inverse )
	{
		choice->SoftReset();
		choice->FinalizeInverse();
	}

	PolyPtr chosen( choice );
	SelectPtr sp = boost::dynamic_pointer_cast< ISelectable>( chosen );

	progressBrush->Clear();
	progressBrush->AddObject( sp );

	Brush orig;
	
	SelectPtr spe = boost::dynamic_pointer_cast<ISelectable>( extendingPolygon );
	orig.AddObject( spe );
	
	Action * action = new ReplaceBrushAction( &orig, progressBrush );
	action->Perform();
	doneActionStack.push_back( action );

	ClearUndoneActions();

	polygonInProgress->Reset();
	//PolyPtr newPoly( new TerrainPolygon(&grassTex) );
	//polygonInProgress = newPoly;

	progressBrush->Clear();	
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

		if (poly.LinesTooClose((*polyIt).get(), minimumEdgeLength))
			return false;

		if ((*polyIt)->LinesTooClose(&poly, minimumEdgeLength) )
			return false;

		//this is only if its not touching stuff
		{
			list<Inter> pInters;
			poly.GetIntersections((*polyIt).get(), pInters);
			if (!pInters.empty())
				return false;
		}
		

		//return false if one of the points is inside my poly, change this along with the prev one
		//externally valid is dependent on what you're trying to do
		for( TerrainPoint *pcurr = (*polyIt)->pointStart; pcurr != NULL; pcurr = pcurr->next )
		{
			if( poly.ContainsPoint( Vector2f( pcurr->pos.x, pcurr->pos.y ) ) )
			{
				//cout << "c" << endl;
				return false;
			}
		}
	
		for( TerrainPoint *my = poly.pointStart; my != NULL; my = my->next )
		{
			if( (*polyIt)->ContainsPoint( Vector2f( my->pos.x, my->pos.y ) ) )
			{
				//cout << "d" << endl;
				return false;
			}
		}

		//for( TerrainPoint *my = poly.pointStart; my != NULL; my = my->next )
		//{
		//	Vector2i oldPoint, currPoint;
		//	if( my == poly.pointStart )
		//	{
		//		oldPoint = poly.pointEnd->pos;
		//	}
		//	else
		//	{
		//		oldPoint = my->prev->pos;
		//	}

		//	currPoint = my->pos;
		//}

		//for( TerrainPoint *pcurr = (*polyIt)->pointStart; pcurr != NULL; pcurr = pcurr->next )
		//{
		//	Vector2i oldPoint, currPoint;
		//	if( pcurr == (*polyIt)->pointStart )
		//	{
		//		oldPoint = (*polyIt)->pointEnd->pos;
		//	}
		//	else
		//	{
		//		oldPoint = pcurr->prev->pos;
		//	}

		//	currPoint = pcurr->pos;

		//	/*for( list<GateInfo*>::iterator it = poly.attachedGates.begin(); it != poly.attachedGates.end(); ++it )
		//	{
		//		LineIntersection li = LimitSegmentIntersect( oldPoint, currPoint, (*it)->v0, (*it)->v1 );
		//		if( !li.parallel )
		//		{
		//			return false;
		//		}
		//	}*/
		//}

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

	//check for slivers that are at too extreme of an angle. tiny triangle type things
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
		if( ff > .99 )
		{
			//cout << "ff: " << ff << endl;
			return false;
		}
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

	std::list<boost::shared_ptr<GateInfo>> gateInfoList;
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
			TerrainPolygon *outPoly = NULL;
			
			Add( currentBrush, temp, outPoly, gateInfoList);
			
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
	Panel *panel = NULL;
	if( type == "gateselect" )
	{
		panel = gateSelectorPopup;
	}
	else if( type == "terraintypeselect" )
	{
		panel = terrainSelectorPopup;
	}
	else if (type == "bg")
	{
		panel = bgPopup;
	}

	assert( panel != NULL );
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
	//pixelPos = Vector2i( 960, 540 );


	Vector2f uiMouse = preScreenTex->mapPixelToCoords( pixelPos );



	panel->pos.x = uiMouse.x;
	panel->pos.y = uiMouse.y;

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
						panel->Update( true, uiMouse.x, uiMouse.y );
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
						panel->Update( false, uiMouse.x, uiMouse.y );
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
		panel->Draw( w );

		w->setView( v );

		w->display();
	}

	preScreenTex->setView( view );
	w->setView( v );
}
Panel * EditSession::CreatePopupPanel( const std::string &type )
{
	Panel *p = NULL;
	if( type == "message" )
	{
		p = new Panel( "message_popup", 400, 100, this );
		p->pos.x = 300;
		p->pos.y = 300;
		//p->AddButton( "ok", Vector2i( 250, 25 ), Vector2f( 100, 50 ), "OK" );
		p->AddLabel( "message", Vector2i( 10, 10 ), 12, "_EMPTY\n_MESSAGE_" );
		p->pos = Vector2i( 960 - p->size.x / 2, 540 - p->size.y );
		//p->
	}
	else if( type == "error" )
	{
		p = new Panel( "error_popup", 400, 100, this );
		//p->AddButton( "ok", Vector2i( 250, 25 ), Vector2f( 100, 50 ), "OK" );
		p->AddLabel( "message", Vector2i( 25, 50 ), 12, "_EMPTY_ERROR_" );
		p->pos = Vector2i( 960 - p->size.x / 2, 540 - p->size.y );
	}
	else if( type == "confirmation" )
	{
		p = new Panel( "confirmation_popup", 400, 100, this );
		p->AddButton( "confirmOK", Vector2i( 50, 25 ), Vector2f( 100, 50 ), "OK" );
		p->AddButton( "cancel", Vector2i( 250, 25 ), Vector2f( 100, 50 ), "Cancel" );
		p->AddLabel( "question", Vector2i( 10, 10 ), 12, "_EMPTY\n_QUESTION_" );
		p->pos = Vector2i( 960 - p->size.x / 2, 540 - p->size.y );
	}
	else if( type == "gateselector" )
	{
		p = new Panel( "gate_popup", 200, 500, this );
	}
	else if( type == "terrainselector" )
	{
		p = new Panel( "terrain_popup", 100, 100, this );
	}
	else if (type == "bg")
	{
		p = new Panel("bg_popup", 1500, 600, this);
	}


	if( p != NULL )
		allPopups.push_back(p);

	return p;
}

//-1 means you denied it, 0 means it didnt work, and 1 means it will work
int EditSession::IsRemovePointsOkay()
{
	bool terrainOkay = true;
	for( PointMap::iterator it = selectedPoints.begin(); it != selectedPoints.end(); ++it )
		//list<PolyPtr>::iterator it = selectedPolygons.begin(); 
		//it != selectedPolygons.end(); ++it )
	{
		TerrainPolygon *tp = (*it).first;
		bool res = tp->IsRemovePointsOkayTerrain( this );
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

	/*bool enemiesOkay = false;
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
	}*/

	return 1;
}

Panel * EditSession::CreateOptionsPanel( const std::string &name )
{
	Panel *p = NULL;
	if( name == "healthfly" )
	{
		p = new Panel( "healthfly_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );

		p->AddCheckBox( "monitor", Vector2i( 20, 330 ) );
		
	}
	else if( name == "poi" )
	{
		p = new Panel( "poi_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "NO NAME" );
		p->AddTextBox( "camzoom", Vector2i( 20, 180 ), 200, 20, "not test" );
		p->AddCheckBox( "camprops", Vector2i( 20, 240 ) );
		p->AddTextBox( "barrier", Vector2i( 20, 330 ), 50, 1, "-" );
	}
	else if( name == "shippickup" )
	{
		p = new Panel( "shippickup_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "NO NAME" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );


		//p->AddTextBox( "barrier", Vector2i( 20, 330 ), 50, 1, "-" );
		p->AddCheckBox( "facingright", Vector2i( 20, 250 ) );
	}
	else if( name == "key" )
	{
		p = new Panel( "key_options", 200, 600, this );
		p->AddButton( "ok", Vector2i( 100, 450 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );
		p->AddTextBox( "numkeys", Vector2i( 20, 150 ), 200, 20, "3" );
		p->AddTextBox("zonetype", Vector2i(20, 200), 200, 20, "0" );
	}
	else if (name == "blocker")
	{
		p = new Panel("blocker_options", 200, 500, this);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");
		//p->AddLabel("type_label", Vector2i(20, 150), 20, "type");
		p->AddCheckBox("armored", Vector2i(120, 155));
		p->AddTextBox("btype", Vector2i(20, 200), 200, 20, "0");
		p->AddTextBox("spacing", Vector2i(20, 250), 200, 20, "0");
		p->AddButton("createchain", Vector2i(20, 300), Vector2f(100, 50), "Create Chain");
		//p->
	}
	else if (name == "comboer")
	{
		p = new Panel("comboer_options", 200, 500, this);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");
		p->AddLabel("loop_label", Vector2i(20, 150), 20, "loop");
		p->AddCheckBox("loop", Vector2i(120, 155));
		p->AddTextBox("speed", Vector2i(20, 200), 200, 20, "10");
		p->AddButton("createpath", Vector2i(20, 250), Vector2f(100, 50), "Create Path");

		p->AddCheckBox("monitor", Vector2i(20, 330));
	}
	else if (name == "shard")
	{
		p = new Panel("shard_options", 700, 1080, this);
		p->AddLabel("shardtype", Vector2i(20, 900), 24, "SHARD_W1_TEACH_JUMP");

		int xSize = 11;
		int ySize = 2;
		//GridSelector *gs = new GridSelector(Vector2i(0, 0), 3, 7, 64, 64, true, true, p);
		GridSelector *gs = p->AddGridSelector( "shardselector", Vector2i(0, 0), xSize, ySize * 7, 64, 64, true, true);
		Sprite spr;

		ts_shards[0] = GetTileset("Shard/shards_w1_48x48.png", 48, 48);
		ts_shards[1] = GetTileset("Shard/shards_w2_48x48.png", 48, 48);
		ts_shards[2] = GetTileset("Shard/shards_w2_48x48.png", 48, 48);
		ts_shards[3] = GetTileset("Shard/shards_w2_48x48.png", 48, 48);
		ts_shards[4] = GetTileset("Shard/shards_w2_48x48.png", 48, 48);
		ts_shards[5] = GetTileset("Shard/shards_w2_48x48.png", 48, 48);
		ts_shards[6] = GetTileset("Shard/shards_w2_48x48.png", 48, 48);

		
		Tileset *ts_currShards;
		int sInd = 0;
		
		for (int w = 0; w < 7; ++w)
		{
			ts_currShards = ts_shards[w];
			spr.setTexture(*ts_currShards->texture);
			for (int y = 0; y < ySize; ++y)
			{
				for (int x = 0; x < xSize; ++x)
				{
					sInd = y * xSize + x;
					spr.setTextureRect(ts_currShards->GetSubRect(sInd));
					int shardT = (sInd + (xSize * ySize) * w);
					if (shardT >= SHARD_Count)
					{
						gs->Set(x, y + ySize * w, spr, "---"); //need a way to set the names later
					}
					else
					{
						gs->Set(x, y + ySize * w, spr, Shard::GetShardString((ShardType)shardT));
					}

				}
			}
		}
		

		p->AddButton("ok", Vector2i(100, 1000), Vector2f(100, 50), "OK");
	}
	else if (name == "rail")
	{
		p = new Panel("rail_options", 200, 500, this);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");
		
		p->AddCheckBox("energized", Vector2i(120, 155));
		p->AddButton("createrail", Vector2i(20, 300), Vector2f(100, 50), "Create Rail");

	}
	else if (name == "booster")
	{
		p = new Panel("booster_options", 200, 500, this);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");

		p->AddTextBox("strength", Vector2i(20, 200), 200, 3, "");
	}

	else if (name == "spring")
	{
		p = new Panel("spring_options", 200, 500, this);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");

		p->AddButton("setdirection", Vector2i(20, 300), Vector2f(100, 50), "Set Direction");

		p->AddTextBox("moveframes", Vector2i(20, 200), 200, 3, "");
	}
	//w1
	else if( name == "patroller" )
	{
		p = new Panel( "patroller_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );
		p->AddLabel( "loop_label", Vector2i( 20, 150 ), 20, "loop" );
		p->AddCheckBox( "loop", Vector2i( 120, 155 ) ); 
		p->AddTextBox( "speed", Vector2i( 20, 200 ), 200, 20, "10" );
		p->AddButton( "createpath", Vector2i( 20, 250 ), Vector2f( 100, 50 ), "Create Path" );

		p->AddCheckBox( "monitor", Vector2i( 20, 330 ) );

		
		//p->
	}
	else if( name == "foottrap" )
	{
		p = new Panel( "foottrap_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );

		p->AddCheckBox( "monitor", Vector2i( 20, 330 ) );
	}
	else if( name == "basicturret" )
	{
		p = new Panel( "basicturret_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		p->AddTextBox( "bulletspeed", Vector2i( 20, 150 ), 200, 20, "10" );
		p->AddTextBox( "waitframes", Vector2i( 20, 200 ), 200, 20, "10" );

		p->AddCheckBox( "monitor", Vector2i( 20, 330 ) );
	}
	else if( name == "crawler" )
	{
		p = new Panel( "crawler_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		p->AddLabel( "clockwise_label", Vector2i( 20, 150 ), 20, "clockwise" );
		p->AddCheckBox( "clockwise", Vector2i( 120, 155 ) ); 
		p->AddTextBox( "speed", Vector2i( 20, 200 ), 200, 20, "1.5" );

		p->AddCheckBox( "monitor", Vector2i( 20, 330 ) );
	}
	else if (name == "airdasher")
	{
		p = new Panel("airdasher_options", 200, 500, this);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "not test");
		p->AddCheckBox("monitor", Vector2i(20, 330));
	}

	//w2
	else if( name == "bat" )
	{
		p = new Panel( "bat_options", 200, 600, this );
		p->AddButton( "ok", Vector2i( 100, 450 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );
		p->AddLabel( "loop_label", Vector2i( 20, 150 ), 20, "loop" );
		p->AddCheckBox( "loop", Vector2i( 120, 155 ) ); 
		p->AddTextBox( "bulletspeed", Vector2i( 20, 200 ), 200, 20, "10" );
		//p->AddTextBox( "nodedistance", Vector2i( 20, 250 ), 200, 20, "10" );
		p->AddTextBox( "framesbetweennodes", Vector2i( 20, 300 ), 200, 20, "10" );
		p->AddButton( "createpath", Vector2i( 20, 350 ), Vector2f( 100, 50 ), "Create Path" );

		p->AddCheckBox( "monitor", Vector2i( 20, 400 ) );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		//p->
	}
	else if( name == "poisonfrog" )
	{
		p = new Panel( "poisonfrog_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		p->AddTextBox( "xstrength", Vector2i( 20, 150 ), 200, 20, "10" );
		p->AddTextBox( "ystrength", Vector2i( 20, 200 ), 200, 20, "10" );
		p->AddTextBox( "gravfactor", Vector2i( 20, 250 ), 200, 20, "5" );
		p->AddTextBox( "jumpwaitframes", Vector2i( 20, 300 ), 200, 20, "10" );
		//p->AddLabel( "clockwise_label", Vector2i( 20, 150 ), 20, "clockwise" );
		//p->AddCheckBox( "clockwise", Vector2i( 120, 155 ) ); 
		

		p->AddCheckBox( "monitor", Vector2i( 20, 330 ) );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if (name == "gravityfaller")
	{
		p = new Panel("gravityfaller_options", 200, 500, this);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");
		p->AddTextBox("var", Vector2i(20, 150), 200, 20, "0");
		//p->AddLabel( "clockwise_label", Vector2i( 20, 150 ), 20, "clockwise" );
		//p->AddCheckBox( "clockwise", Vector2i( 120, 155 ) ); 


		p->AddCheckBox("monitor", Vector2i(20, 330));
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if( name == "stagbeetle" )
	{
		p = new Panel( "stagbeetle_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		p->AddLabel( "clockwise_label", Vector2i( 20, 150 ), 20, "clockwise" );
		p->AddCheckBox( "clockwise", Vector2i( 120, 155 ) ); 
		p->AddTextBox( "speed", Vector2i( 20, 200 ), 200, 20, "3" );

		p->AddCheckBox( "monitor", Vector2i( 20, 330 ) );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if( name == "curveturret" )
	{
		p = new Panel( "curveturret_options", 200, 550, this );
		p->AddButton( "ok", Vector2i( 100, 450 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		p->AddTextBox( "bulletspeed", Vector2i( 20, 150 ), 200, 20, "10" );
		p->AddTextBox( "waitframes", Vector2i( 20, 200 ), 200, 20, "10" );
		p->AddTextBox( "xgravfactor", Vector2i( 20, 250 ), 200, 20, "0" );
		p->AddTextBox( "ygravfactor", Vector2i( 20, 300 ), 200, 20, "0" );
		p->AddCheckBox( "relativegrav", Vector2i( 20, 350 ) );

		p->AddCheckBox( "monitor", Vector2i( 20, 400 ) );
		
	}

	//w3
	else if( name == "pulser" )
	{
		p = new Panel( "pulser_options", 200, 600, this );
		p->AddButton( "ok", Vector2i( 100, 450 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );
		p->AddLabel( "loop_label", Vector2i( 20, 150 ), 20, "loop" );
		p->AddCheckBox( "loop", Vector2i( 120, 155 ) ); 
		p->AddTextBox( "framesbetweennodes", Vector2i( 20, 300 ), 200, 20, "10" );
		p->AddButton( "createpath", Vector2i( 20, 350 ), Vector2f( 100, 50 ), "Create Path" );

		p->AddCheckBox( "monitor", Vector2i( 20, 400 ) );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		//p->
	}
	else if( name == "cactus" )
	{
		p = new Panel( "cactus_options", 200, 550, this );
		p->AddButton( "ok", Vector2i( 100, 450 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );

		p->AddTextBox( "bulletspeed", Vector2i( 20, 150 ), 200, 20, "10" );
		p->AddTextBox( "rhythm", Vector2i( 20, 200 ), 200, 20, "60" );
		p->AddTextBox( "amplitude", Vector2i( 20, 250 ), 200, 20, "10" );


		p->AddCheckBox( "monitor", Vector2i( 20, 400 ) );
		
	}
	else if( name == "badger" )
	{
		p = new Panel( "badger_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		p->AddLabel( "clockwise_label", Vector2i( 20, 150 ), 20, "clockwise" );
		p->AddCheckBox( "clockwise", Vector2i( 120, 155 ) ); 

		p->AddTextBox( "speed", Vector2i( 20, 200 ), 200, 20, "3" );
		p->AddTextBox( "jumpstrength", Vector2i( 20, 200 ), 250, 20, "3" );

		p->AddCheckBox( "monitor", Vector2i( 20, 330 ) );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if( name == "owl" )
	{
		p = new Panel( "owl_options", 200, 600, this );
		p->AddButton( "ok", Vector2i( 100, 450 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );

		p->AddTextBox( "movespeed", Vector2i( 20, 150 ), 200, 1, "1" ); 
		p->AddTextBox( "bulletspeed", Vector2i( 20, 200 ), 200, 1, "1" ); 
		p->AddTextBox( "rhythmframes", Vector2i( 20, 250 ), 200, 1, "1" ); 
		
		

		p->AddCheckBox( "monitor", Vector2i( 20, 400 ) );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
		//p->
	}
	
	//w4
	else if( name == "turtle" )
	{
		p = new Panel( "turtle_options", 200, 600, this );
		p->AddButton( "ok", Vector2i( 100, 450 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );

		//maybe bullet speed later but it might be too hard if it has variation
		//could have params to have them teleport to offsets around your position
		//instead of always DIRECTLY on it

		//p->AddTextBox( "movespeed", Vector2i( 20, 150 ), 200, 1, "1" ); 
		//p->AddTextBox( "bulletspeed", Vector2i( 20, 200 ), 200, 1, "1" ); 
		//p->AddTextBox( "rhythmframes", Vector2i( 20, 250 ), 200, 1, "1" ); 
		
		

		p->AddCheckBox( "monitor", Vector2i( 20, 400 ) );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if( name == "cheetah" )
	{
		p = new Panel( "cheetah_options", 200, 550, this );
		p->AddButton( "ok", Vector2i( 100, 450 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		p->AddCheckBox( "monitor", Vector2i( 20, 400 ) );
		
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if( name == "spider" )
	{
		p = new Panel( "spider_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		p->AddLabel( "clockwise_label", Vector2i( 20, 150 ), 20, "clockwise" );
		p->AddCheckBox( "clockwise", Vector2i( 120, 155 ) ); 

		//ground speed will probably be the param. not figured out fully yet

		p->AddTextBox( "speed", Vector2i( 20, 200 ), 200, 20, "10" );
		//p->AddTextBox( "jumpstrength", Vector2i( 20, 200 ), 250, 20, "3" );

		p->AddCheckBox( "monitor", Vector2i( 20, 330 ) );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if( name == "coral" )
	{
		p = new Panel( "coral_options", 200, 600, this );
		p->AddButton( "ok", Vector2i( 100, 450 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );

		p->AddTextBox( "moveframes", Vector2i( 20, 150 ), 200, 20, "60" );

		p->AddCheckBox( "monitor", Vector2i( 20, 400 ) );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	
	//w5
	else if( name == "shark" )
	{
		p = new Panel( "shark_options", 200, 600, this );
		p->AddButton( "ok", Vector2i( 100, 450 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );

		p->AddTextBox( "circleframes", Vector2i( 20, 150 ), 200, 20, "60" );
		

		//p->AddTextBox( "movespeed", Vector2i( 20, 150 ), 200, 1, "1" ); 
		//p->AddTextBox( "bulletspeed", Vector2i( 20, 200 ), 200, 1, "1" ); 
		//p->AddTextBox( "rhythmframes", Vector2i( 20, 250 ), 200, 1, "1" ); 
		
		

		p->AddCheckBox( "monitor", Vector2i( 20, 400 ) );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if( name == "overgrowth" )
	{
		p = new Panel( "overgrowth_options", 200, 550, this );
		p->AddButton( "ok", Vector2i( 100, 450 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );
		p->AddCheckBox( "monitor", Vector2i( 20, 400 ) );
		
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if( name == "ghost" )
	{
		p = new Panel( "ghost_options", 200, 600, this );
		p->AddButton( "ok", Vector2i( 100, 450 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );

		p->AddTextBox( "speed", Vector2i( 20, 150 ), 200, 20, "1" );

		p->AddCheckBox( "monitor", Vector2i( 20, 400 ) );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if( name == "swarm" )
	{
		p = new Panel( "swarm_options", 200, 600, this );
		p->AddButton( "ok", Vector2i( 100, 450 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );

		p->AddTextBox( "liveframes", Vector2i( 20, 150 ), 200, 3, "120" );
		//maybe frames swarm is alive once launched?

		p->AddCheckBox( "monitor", Vector2i( 20, 400 ) );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if( name == "specter" )
	{
		p = new Panel( "specter_options", 200, 600, this );
		p->AddButton( "ok", Vector2i( 100, 450 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );

		p->AddCheckBox( "monitor", Vector2i( 20, 400 ) );
	}
	else if( name == "gorilla" )
	{
		p = new Panel( "gorilla_options", 200, 600, this );
		p->AddButton( "ok", Vector2i( 100, 450 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );

		p->AddTextBox( "wallwidth", Vector2i( 20, 150 ), 200, 20, "1" );
		p->AddTextBox( "followframes", Vector2i( 20, 200 ), 200, 20, "1" );

		p->AddCheckBox( "monitor", Vector2i( 20, 400 ) );
		//p->AddLabel( "label1", Vector2i( 20, 200 ), 30, "blah" );
	}
	else if( name == "copycat" )
	{
		p = new Panel( "copycat_options", 200, 600, this );
		p->AddButton( "ok", Vector2i( 100, 450 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );

		p->AddCheckBox( "monitor", Vector2i( 20, 400 ) );
	}
	else if( name == "narwhal" )
	{
		p = new Panel( "narwhal_options", 200, 600, this );
		p->AddButton( "ok", Vector2i( 100, 450 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "not test" );

		p->AddTextBox( "moveframes", Vector2i( 20, 300 ), 200, 20, "10" );
		p->AddButton( "createpath", Vector2i( 20, 350 ), Vector2f( 100, 50 ), "Create Path" );

		p->AddCheckBox( "monitor", Vector2i( 20, 400 ) );
	}
	else if( name == "nexus" )
	{
		p = new Panel( "nexus_options", 200, 500, this );
		p->AddButton( "ok", Vector2i( 100, 410 ), Vector2f( 100, 50 ), "OK" );
		p->AddTextBox( "name", Vector2i( 20, 20 ), 200, 20, "name_test" );
		p->AddTextBox( "group", Vector2i( 20, 100 ), 200, 20, "group_test" );

		p->AddTextBox( "nexusindex", Vector2i( 20, 150 ), 200, 20, "1" );
	}

	else if (name == "groundtrigger")
	{
		p = new Panel("groundtrigger_options", 200, 500, this);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");

		p->AddCheckBox("facingright", Vector2i(20, 250));
		p->AddTextBox("triggertype", Vector2i(20, 150), 200, 20, "0");
	}
	else if (name == "airtrigger")
	{
		p = new Panel("airtrigger_options", 200, 500, this);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");
		p->AddTextBox("triggertype", Vector2i(20, 150), 200, 20, "0");
		p->AddButton("createrect", Vector2i(20, 350), Vector2f(100, 50), "Create Rect");
	}
	//else if( name == "racefighttarget" )
	//{
	


	else if( name == "map" )
	{
		p = new Panel( "map_options", 200, 800, this );
		p->AddButton( "ok", Vector2i( 100, 600 ), Vector2f( 100, 50 ), "OK" );
		p->AddLabel( "minedgesize_label", Vector2i( 20, 150 ), 20, "minimum edge size:" );
		p->AddTextBox( "minedgesize", Vector2i( 20, 20 ), 200, 20, "8" );
		p->AddLabel("draintime_label", Vector2i(20, 200), 20, "drain seconds:");
		p->AddTextBox("draintime", Vector2i(20, 250), 200, 20, "60");
		p->AddTextBox("bosstype", Vector2i(20, 300), 200, 20, "0");
		p->AddButton( "envtype", Vector2i(20, 400), Vector2f(100, 50), "Choose BG");
		//p->AddTextBox("envtype", Vector2i(20, 400), 200, 20, "w1_01");
	}
	else if( name == "terrain" )
	{
		p = new Panel( "terrain_options", 200, 400, this );
		p->AddButton( "ok", Vector2i( 100, 300 ), Vector2f( 100, 50 ), "OK" );
		//p->AddLabel( "minedgesize_label", Vector2i( 20, 150 ), 20, "minimum edge size:" );
		//p->AddTextBox( "minedgesize", Vector2i( 20, 20 ), 200, 20, "8" );
		p->AddButton( "create_path", Vector2i( 100, 0 ), Vector2f( 100, 50 ), "Create Path" );
	}
	else if (name == "flowerpod")
	{
		p = new Panel("flowerpod_options", 200, 500, this);
		p->AddButton("ok", Vector2i(100, 410), Vector2f(100, 50), "OK");
		p->AddTextBox("name", Vector2i(20, 20), 200, 20, "name_test");
		p->AddTextBox("group", Vector2i(20, 100), 200, 20, "group_test");

		p->AddTextBox("podtype", Vector2i(20, 150), 200, 20, "0");
	}
	if( p != NULL )
		allPopups.push_back(p);

	return p;
}

void EditSession::SetPanelDefault( ActorType *type )
{
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

	ap->SetPanelInfo();

	//if( name == "healthfly" )
	//{
	//	HealthFlyParams *fly = (HealthFlyParams*)ap;

	//	p->textBoxes["group"]->text.setString( fly->group->name );

	//	p->checkBoxes["monitor"]->checked = false;
	//	//SetMonitorGrid( fly->monitorType, p->gridSelectors["monitortype"] );

	//	
	//}
	//else if( name == "foottrap" )
	//{
	//	FootTrapParams *footTrap = (FootTrapParams*)ap;
	//	p->textBoxes["group"]->text.setString( footTrap->group->name );
	//	p->checkBoxes["monitor"]->checked = false;
	//}

	//else
	//{
	//	ap->SetPanelInfo();
	//}

	showPanel = p;
}

void EditSession::SetDecorEditPanel()
{
	ISelectable *sp = selectedBrush->objects.front().get();
	assert(sp->selectableType == ISelectable::IMAGE);
	DecorInfo *di = (DecorInfo*)sp;

	editDecorPanel->textBoxes["xpos"]->text.setString(boost::lexical_cast<string>(di->spr.getPosition().x));
	editDecorPanel->textBoxes["ypos"]->text.setString(boost::lexical_cast<string>(di->spr.getPosition().y));

	editDecorPanel->textBoxes["rotation"]->text.setString(boost::lexical_cast<string>(di->spr.getRotation()));
	editDecorPanel->textBoxes["xscale"]->text.setString(boost::lexical_cast<string>(di->spr.getScale().x));
	editDecorPanel->textBoxes["yscale"]->text.setString(boost::lexical_cast<string>(di->spr.getScale().y));
	
	editDecorPanel->textBoxes["layer"]->text.setString(boost::lexical_cast<string>(di->layer));
}

void EditSession::SetDecorParams()
{
	ISelectable *sp = selectedBrush->objects.front().get();
	assert(sp->selectableType == ISelectable::IMAGE);
	DecorInfo *di = (DecorInfo*)sp;
	
	string xposStr = editDecorPanel->textBoxes["xpos"]->text.getString().toAnsiString();
	string yposStr = editDecorPanel->textBoxes["ypos"]->text.getString().toAnsiString();
	string rotStr = editDecorPanel->textBoxes["rotation"]->text.getString().toAnsiString();
	string xscaleStr = editDecorPanel->textBoxes["xscale"]->text.getString().toAnsiString();
	string yscaleStr = editDecorPanel->textBoxes["yscale"]->text.getString().toAnsiString();
	string layerStr = editDecorPanel->textBoxes["layer"]->text.getString().toAnsiString();

	stringstream ss;
	ss << xposStr;

	float posx;
	ss >> posx;

	if (!ss.fail())
	{
		di->spr.setPosition(posx, di->spr.getPosition().y);
	}

	ss.clear();

	ss << yposStr;

	float posy;
	ss >> posy;

	if (!ss.fail())
	{
		di->spr.setPosition(di->spr.getPosition().x, posy );
	}

	ss.clear();

	ss << rotStr;

	float rot;
	ss >> rot;

	if (!ss.fail())
	{
		di->spr.setRotation(rot);
	}

	ss.clear();

	ss << xscaleStr;

	float xScale;
	ss >> xScale;

	if (!ss.fail())
	{
		di->spr.setScale( xScale, di->spr.getScale().y );
	}

	ss.clear();

	ss << yscaleStr;

	float yScale;
	ss >> yScale;

	if (!ss.fail())
	{
		di->spr.setScale(di->spr.getScale().x, yScale );
	}

	ss.clear();

	ss << layerStr;

	int layer;
	ss >> layer;
	if (!ss.fail())
	{
		di->layer = layer;
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

				LineIntersection li = LimitSegmentIntersect(prevPos, pos, v0, v1);

				if (!li.parallel)
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
	for (list<TerrainBrush*>::iterator it = copyBrushes.begin(); it != copyBrushes.end();
		++it)
	{
		delete (*it);
	}
	copyBrushes.clear();
}

void EditSession::ClearPasteBrushes()
{
	for (list<TerrainBrush*>::iterator it = pasteBrushes.begin(); it != pasteBrushes.end();
		++it)
	{
		delete (*it);
	}
	pasteBrushes.clear();
}

void EditSession::CopyToPasteBrushes()
{
	for (list<TerrainBrush*>::iterator it = copyBrushes.begin(); it != copyBrushes.end();
		++it)
	{
		TerrainBrush* tb = new TerrainBrush(*(*it));
		pasteBrushes.push_back(tb);
	}
}

bool EditSession::PolyIntersectGate(TerrainPolygon &poly)
{
	//can be optimized with bounding box checks.
	for (list<GateInfoPtr>::iterator it = gates.begin(); it != gates.end(); ++it)
	{
		Vector2i point0 = (*it)->point0->pos;
		Vector2i point1 = (*it)->point1->pos;

		for (TerrainPoint *my = poly.pointStart; my != NULL; my = my->next)
		{
			TerrainPoint *prev;
			if (my == poly.pointStart)
				prev = poly.pointEnd;
			else
			{
				prev = my->prev;
			}

			LineIntersection li = LimitSegmentIntersect(point0, point1, prev->pos, my->pos);
			if (!li.parallel)
			{
				return true;
			}
		}
	}

	return false;
}

void EditSession::CreateActor(ActorPtr actor)
{
	Brush b;
	SelectPtr select = boost::dynamic_pointer_cast<ISelectable>(actor);
	b.AddObject(select);
	Action * action = new ApplyBrushAction(&b);
	action->Perform();
	doneActionStack.push_back(action);
}

void EditSession::CreateDecorImage(DecorPtr dec)
{
	Brush b;
	SelectPtr select = boost::dynamic_pointer_cast<ISelectable>(dec);
	b.AddObject(select);
	Action * action = new ApplyBrushAction(&b);
	action->Perform();
	doneActionStack.push_back(action);
}

void EditSession::CreatePreview(Vector2i imageSize)
{
	


	int extraBound = 0;
	int left;
	int top;
	int right;
	int bot;
	cout << "CREATING PREVIEW" << endl;

	if (inversePolygon != NULL)
	{
		left = inversePolygon->left;
		top = inversePolygon->top;
		right = inversePolygon->right;
		bot = inversePolygon->bottom;

		extraBound = 200;
	}
	else
	{
		int pLeft;
		int pTop;
		int pRight;
		int pBot;
		for (list<boost::shared_ptr<TerrainPolygon>>::iterator it
			= polygons.begin(); it != polygons.end(); ++it)
		{
			if (polygons.front() == (*it))
			{
				pLeft = (*it)->left;
				pTop = (*it)->top;
				pRight = (*it)->right;
				pBot = (*it)->bottom;
			}
			else
			{
				pLeft = min((*it)->left, pLeft);
				pRight = max((*it)->right, pRight);
				pTop = min((*it)->top, pTop);
				pBot = max((*it)->bottom, pBot );
			}
		}

		left = pLeft;
		top = topBound;
		right = pRight;
		bot = pBot;
	}

	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		for (auto it2 = (*it).second->actors.begin(); it2 != (*it).second->actors.end(); ++it2)
		{
			if ((*it2)->type->name == "poi" )
			{
				boost::shared_ptr<PoiParams> pp = boost::static_pointer_cast<PoiParams>((*it2));
				if (pp->name == "stormceiling")
				{
					top = pp->position.y;
				}
			}
		}
	}

		
	if (left < leftBound)
	{
		left = leftBound;
	}
	else
	{
		left -= extraBound;
	}

	if (top < topBound)
	{
		top = topBound;
	}
	else
	{
		top -= extraBound;
	}
	int bRight = leftBound + boundWidth;
	if (right > bRight)
	{
		right = bRight;
	}
	else
	{
		right += extraBound;
	}

		
	bot += extraBound;

	int width = right - left;
	int height = bot - top;

	Vector2f middle( left + width / 2, top + height / 2 );

	int remX = (right - left) % imageSize.x;
	int remY = (bot - top) % imageSize.y;

	double idealXYRatio = (double)imageSize.x/imageSize.y;
	double realXYRatio = (double)width/(double)height;

	double facX = (right - left) / (double)imageSize.x;
	double facY = (bot - top) / (double)imageSize.y;

	if( realXYRatio > idealXYRatio )
	{
		//wider than it should be
			

		height = ceil(height * (realXYRatio / idealXYRatio));

		if( height % imageSize.y == 1 )
			height--;
		else if( height % imageSize.y == imageSize.y - 1 )
			height++;
	}
	else if( realXYRatio < idealXYRatio )
	{
		//taller than it should be

		width = ceil( width * (idealXYRatio / realXYRatio) );

		if( width % imageSize.x == 1 )
			width--;
		if( width % imageSize.x == imageSize.x - 1 )
			width++;
	}
	else
	{
		//its exactly right
	}

	sf::View pView;
	pView.setCenter( middle );
	pView.setSize(Vector2f(width, -height));// *1.05f );

	Vector2f vSize = pView.getSize();
	float zoom = vSize.x / 960;
	Vector2f botLeft(pView.getCenter().x - vSize.x / 2, pView.getCenter().y + vSize.y / 2);
	for (int i = 0; i < 9 * MAX_TERRAINTEX_PER_WORLD; ++i)
	{
		if (terrainTextures[i] != NULL)
		{
			polyShaders[i].setUniform("zoom", zoom);
			polyShaders[i].setUniform("topLeft", botLeft);
			//just need to change the name topleft  to botleft eventually
		}

	}

	//Color inversePolyTypeColor;// = Color::Blue;

	Color purp(109, 82, 190);
	//inversePolyTypeColor = Color::Blue;

	mapPreviewTex->clear(Color::Black);

	//DrawBG(mapPreviewTex);
	

	mapPreviewTex->setView( pView );


	CircleShape cs;
	cs.setRadius( 10.f * ( (float)width / 1920 ) );
	cs.setFillColor( Color::Red );
	cs.setOrigin( cs.getLocalBounds().width / 2, 
		cs.getLocalBounds().height / 2 );

	CircleShape goalCS;
	goalCS.setRadius(16.f * ((float)width / 1920));
	goalCS.setFillColor(Color::Magenta);
	goalCS.setOrigin(cs.getLocalBounds().width / 2,
		cs.getLocalBounds().height / 2);

	//mapPreviewTex->draw(*tempva);
	//delete tempva;

	for( list<boost::shared_ptr<TerrainPolygon>>::iterator it
		= polygons.begin(); it != polygons.end(); ++it )
	{
		//if( (*it)->IsTouching( inversePolygon.get() ) )
		//	continue;

		(*it)->Draw( false, 1, mapPreviewTex, false, NULL );
	}

	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		(*it)->DrawPreview(mapPreviewTex);
	}

	sf::RectangleShape borderRect;
	borderRect.setFillColor(Color( 30, 30, 30));
	borderRect.setSize(Vector2f(1000000, bot - top));
	borderRect.setPosition(left, top);
	borderRect.setOrigin(borderRect.getLocalBounds().width, 0);
	mapPreviewTex->draw(borderRect);

	borderRect.setOrigin(0, 0);
	borderRect.setPosition(right, top);
	mapPreviewTex->draw(borderRect);
	//sf::Vertex borderRect[4];
	//SetRectColor(borderRect, Color::Cyan);
		
		

	for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it )
	{
		for( list<ActorPtr>::iterator it2 = (*it).second->actors.begin();
			it2 != (*it).second->actors.end(); ++it2 )
		{
			if ((*it2)->type == types["goal"])
			{
				goalCS.setPosition((*it2)->position.x, (*it2)->position.y);
				mapPreviewTex->draw(goalCS);
			}
			else
			{
				cs.setPosition((*it2)->position.x, (*it2)->position.y);
				mapPreviewTex->draw(cs);
			}
				
		}

			
		//(*it).second->DrawPreview( mapPreviewTex );
	}

	cs.setPosition(player->position.x, player->position.y );
	cs.setFillColor(Color::Green);
	mapPreviewTex->draw(cs);

	sf::RectangleShape rs;
	rs.setPosition(pView.getCenter().x - pView.getSize().x / 2, top);// pView.getCenter().y);
	rs.setSize(Vector2f(pView.getSize().x, top - (pView.getCenter().y - pView.getSize().y / 2 )));
	rs.setFillColor(Color::Cyan);
	mapPreviewTex->draw(rs);
	//this rectangle shape is just a placeholder, because eventually we will texture stuff.

		

	Image img = mapPreviewTex->getTexture().copyToImage();
		
	std::stringstream ssPrev;
	ssPrev << currentPath.parent_path().relative_path().string() << "/Previews/" << currentPath.stem().string() << "_preview_" << imageSize.x << "x" << imageSize.y << ".png";
	std::string previewFile = ssPrev.str();
	img.saveToFile( previewFile );
	//currentFile
}

GroundInfo EditSession::ConvertPointToGround( sf::Vector2i testPoint )
{
	GroundInfo gi;
	double testRadius = 200;
	//PolyPtr poly = NULL;
	gi.ground = NULL;

	/*if( inversePolygon != NULL )
		polygons.push_back( inversePolygon );*/
	bool contains;
	for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		contains = (*it)->ContainsPoint(Vector2f(testPoint.x, testPoint.y));

		if ((contains && !(*it)->inverse) || (!contains && (*it)->inverse))
		//if( (*it)->ContainsPoint( Vector2f( testPoint.x, testPoint.y ) ) )
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
			double storedQuantity = 0;
							
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
				//dist < 100 
				if( dist < testRadius && testQuantity >= 0 && testQuantity <= length( cu - pr ) 
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
	/*if( inversePolygon != NULL )
		polygons.pop_back();*/

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
			if( (*it)->SegmentTooClose( polygonInProgress->pointEnd->pos, polygonInProgress->pointStart->pos, minimumEdgeLength ) )
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
			//MessagePop( "unable to complete polygon" );
			//popupPanel = messagePopup;
			//return;
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
			//if( polygonInProgress->LinesTooClose( (*it).get(), minimumEdgeLength ) )
			//{
			//	//cout << "LINES TOO CLOSE" << endl;
			//	applyOkay = false;
			//	break;
			//}
			//else 
			if( polygonInProgress->LinesIntersect( (*it).get() ) )
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

		
		//bool intersectInverse = false;
		//if (inversePolygon != NULL)
		//{
		//	if (polygonInProgress->LinesTooClose(inversePolygon.get(), minimumEdgeLength))
		//	{
		//		//cout << "LINES TOO CLOSE" << endl;
		//		applyOkay = false;
		//	}
		//	else if (polygonInProgress->LinesIntersect(inversePolygon.get()))
		//	{
		//		//not too close and I intersect, so I can add
		//		intersectingPolys.push_back(inversePolygon);
		//		intersectInverse = true;
		//	}
		//}

		//temporary to make sure it doesnt add on top of stuff
		

		if( !applyOkay )
		{
			MessagePop( "polygon is invalid!!! new message" );
		}
		else
		{
			bool empty = intersectingPolys.empty();

			if( empty && IsKeyPressed(Keyboard::LAlt))
			{
				polygonInProgress->inverse = true;
			}
			
			/*else if (intersectInverse)
			{
				bool oldSelected = inversePolygon->selected;
				inversePolygon->SetSelected( true );
				inversePolygon->FixWinding();

				Add( inversePolygon, polygonInProgress );
				
				inversePolygon->SetSelected( oldSelected );

				SetInversePoly();
			}*/
			if( empty )
			{
				polygonInProgress->FixWinding();
				polygonInProgress->RemoveSlivers(PI / 20.0);
				polygonInProgress->Finalize();

				/*if (polygonInProgress->inverse)
				{
					inversePolygon = polygonInProgress;
				}*/

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
				//polygonInProgress->FixWinding();

				//hold shift ATM to activate subtraction
				Action *action = ChooseAddOrSub(intersectingPolys);

				action->Perform();
				doneActionStack.push_back(action);

				ClearUndoneActions();
			}
		}
	}
	else if( polygonInProgress->numPoints <= 2 && polygonInProgress->numPoints > 0  )
	{
		cout << "cant finalize. cant make polygon" << endl;
		polygonInProgress->ClearPoints();
	}
}

void EditSession::SetInversePoly()
{
	polygonInProgress->FixWinding();
	//polygonInProgress->Finalize();

	//touches inverse
	if( inversePolygon != NULL )//&& polygonInProgress->IsTouching( inversePolygon.get() ) )
	{

		/*bool oldSelected = inversePolygon->selected;
		inversePolygon->SetSelected( true );
		inversePolygon->FixWinding();
		Add( inversePolygon, polygonInProgress );
		inversePolygon->SetSelected( oldSelected );*/
	}
	else
	{
		
	}
	polygonInProgress->FinalizeInverse();
	

	//SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>( polygonInProgress );

	//inversePolygon = polygonInProgress;				

	Brush orig;
	if( inversePolygon != NULL )
	{
		SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>( inversePolygon );
		orig.AddObject( sp );
	}
	
	//for( list<PolyPtr>::iterator it = intersectingPolys.begin(); it != intersectingPolys.end(); ++it )
	//{
	//	SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>( (*it) );
	//	orig.AddObject( sp );
	//	bool oldSelected = (*it)->selected;
	//	(*it)->SetSelected( true );
	//	Add( (*it), polygonInProgress );
	//	(*it)->SetSelected( oldSelected ); //should i even store the old one?							
	//}

	SelectPtr sp = boost::dynamic_pointer_cast< ISelectable>( polygonInProgress );

	progressBrush->Clear();
	progressBrush->AddObject( sp );

	Action * action = new ReplaceBrushAction( &orig, progressBrush );
	action->Perform();
	doneActionStack.push_back( action );

	ClearUndoneActions();

	PolyPtr newPoly( new TerrainPolygon(&grassTex) );
	polygonInProgress = newPoly;

	progressBrush->Clear();
	//progressBrush->AddObject( sp );
									
	//Action *action = new ApplyBrushAction( progressBrush );
	//action->Perform();
	//doneActionStack.push_back( action );

	//ClearUndoneActions();

	//PolyPtr newPoly( new TerrainPolygon(&grassTex) );
	//polygonInProgress = newPoly;
}

bool EditSession::HoldingShift()
{
	return ((IsKeyPressed(Keyboard::LShift) ||
		IsKeyPressed(Keyboard::RShift)));
}

bool EditSession::HoldingControl()
{
	return ((IsKeyPressed(Keyboard::LControl) ||
		IsKeyPressed(Keyboard::RControl)));
}

Action * EditSession::ChooseAddOrSub( list<PolyPtr> &intersectingPolys)
{
	if (!(IsKeyPressed(Keyboard::LShift) ||
		IsKeyPressed(Keyboard::RShift)))
	{
		return ExecuteTerrainAdd( intersectingPolys);
	}
	else
	{
		return ExecuteTerrainSubtract(intersectingPolys);
	}
}

void EditSession::PasteTerrain(Brush *b)
{
	

	CompoundAction *compoundAction = new CompoundAction;
	Brush applyBrush;
	for (auto bit = b->objects.begin(); bit != b->objects.end(); ++bit)
	{
		if ((*bit)->selectableType == ISelectable::TERRAIN)
		{
			TerrainPolygon *tp = (TerrainPolygon*)((*bit).get());
			polygonInProgress.reset(tp->Copy());
			list<PolyPtr> intersectingPolys;
			for (auto it = polygons.begin(); it != polygons.end(); ++it)
			{
				if (polygonInProgress->LinesIntersect((*it).get()))
				{
					//not too close and I intersect, so I can add
					intersectingPolys.push_back((*it));
				}
			}

			if (intersectingPolys.empty())
			{
				applyBrush.AddObject((*bit));
				//finalResultBrush.AddObject((*bit));
				//Action *ac = new ApplyBrushAction(copiedBrush);
				//ac->Perform();
				//doneActionStack.push_back(ac);
				//ClearUndoneActions();
				//PolyPtr newPoly(new TerrainPolygon(&grassTex));
				//polygonInProgress = newPoly;
				//PolyPtr newPoly(new TerrainPolygon(&grassTex));
				//polygonInProgress = newPoly;
			}
			else
			{
				Action * a = ChooseAddOrSub(intersectingPolys);
				compoundAction->subActions.push_back(a);
			}
		}
		else if ((*bit)->selectableType == ISelectable::ACTOR)
		{
			ActorPtr a = boost::dynamic_pointer_cast<ActorParams>((*bit));
			ActorPtr ap(a->Copy());
			if (ap->groundInfo != NULL)
			{
				ap->AnchorToGround(*ap->groundInfo);
			}
			applyBrush.AddObject(ap);
		}
	}

	if (applyBrush.objects.size() > 0)
	{
		Action *ac = new ApplyBrushAction(&applyBrush);
		compoundAction->subActions.push_back(ac);

		PolyPtr newPoly(new TerrainPolygon(&grassTex));
		polygonInProgress = newPoly;
	}

	compoundAction->Perform();
	doneActionStack.push_back(compoundAction);
	ClearUndoneActions();

	copiedBrush = copiedBrush->Copy();
}

Action* EditSession::ExecuteTerrainAdd( list<PolyPtr> &intersectingPolys)
{
	Brush orig;
	Brush resultBrush;
	tempActors.clear();

	TerrainPolygon *outPoly = NULL;
	//Brush orig;
	list<boost::shared_ptr<GateInfo>> gateInfoList;
	list<ActorPtr> actorList;

	for( list<PolyPtr>::iterator it = intersectingPolys.begin(); it != intersectingPolys.end(); ++it )
	{
		for (auto eit = (*it)->enemies.begin(); eit != (*it)->enemies.end(); ++eit)
		{
			for (auto eit2 = (*eit).second.begin(); eit2 != (*eit).second.end(); ++eit2)
			{
				actorList.push_back((*eit2));
			}
		}

		SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>( (*it) );
		orig.AddObject( sp );
		for (auto eit = (*it)->enemies.begin(); eit != (*it)->enemies.end(); ++eit)
		{
			for (auto eit2 = (*eit).second.begin(); eit2 != (*eit).second.end(); ++eit2)
			{
				SelectPtr sp1 = boost::dynamic_pointer_cast<ISelectable>((*eit2));
				orig.AddObject(sp1);
			}
		}

		TerrainPoint *curr = (*it)->pointStart;
		while (curr != NULL)
		{
			if (curr->gate != NULL)
			{
				SelectPtr sp1 = boost::dynamic_pointer_cast<ISelectable>(curr->gate);
				if (!orig.Has(sp1))
				{
					orig.AddObject(sp1);
				}
				else
				{
					//no duplicates
				}
			}
			curr = curr->next;
		}

		bool oldSelected = (*it)->selected;
		(*it)->SetSelected( true );
		Add(polygonInProgress, (*it), outPoly, gateInfoList);
 		polygonInProgress.reset(outPoly);
		(*it)->SetSelected( oldSelected ); //should i even store the old one?							
	}

	

	//assert(orig.objects.size() == 2);
	//assert(actorList.size() == 1);

	SelectPtr sp = boost::dynamic_pointer_cast< ISelectable>(polygonInProgress);

	/*for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
	{*/


	resultBrush.Clear();
	resultBrush.AddObject(sp);

	for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
	{
		TerrainPoint *test = NULL;
		TerrainPoint * outTest = NULL;

		TerrainPoint *p0 = polygonInProgress->GetSamePoint((*it)->point0->pos);
		TerrainPoint *p1 = polygonInProgress->GetSamePoint((*it)->point1->pos);

		if (p0 == NULL && p1 == NULL)
			continue;

		GateInfoPtr gi(new GateInfo( *((*it).get()) ));
		gi->edit = NULL;

		if( p0 != NULL )
		{
			gi->point0 = p0;
			gi->poly0 = polygonInProgress;
			p0->gate = gi;
		}
		else
		{
			gi->point0 = (*it)->point0;
			gi->poly0 = (*it)->poly0;
			gi->point0->gate = gi;
		}
		if( p1 != NULL )
		{
			gi->point1 = p1;
			gi->poly1 = polygonInProgress;
			p1->gate = gi;
			
		}
		else
		{
			gi->point1 = (*it)->point1;
			gi->poly1 = (*it)->poly1;
			gi->point1->gate = gi;
		}

		SelectPtr sp1 = boost::dynamic_pointer_cast<ISelectable>(gi);
		resultBrush.AddObject(sp1);
	}

	
	for (auto it = actorList.begin(); it != actorList.end(); ++it)
	{
		//ActorPtr newActor((*it)->Copy());
		//newActor->UnAnchor(newActor);
		//bool res = AttachActorToPolygon(newActor, polygonInProgress.get() );
		ActorParams * ac = AttachActorToPolygon((*it), polygonInProgress.get());
		if (ac != NULL)
		{
			ActorPtr newActor( ac );
			resultBrush.AddObject(newActor);
		}
		//newActor->AnchorToGround( )
		//newActor->groundInfo
		//newActor->groundInfo->edgeStart
	}

	

	/*for (auto eit = polygonInProgress->enemies.begin(); 
		eit != polygonInProgress->enemies.end(); ++eit)
	{
		for (auto eit2 = (*eit).second.begin(); eit2 != (*eit).second.end(); ++eit2)
		{
			SelectPtr sp1 = boost::dynamic_pointer_cast<ISelectable>((*eit2));
			resultBrush.AddObject(sp1);
		}

	}*/
	
	Action * action = new ReplaceBrushAction(&orig, &resultBrush);

	/*if (resultBrush.objects.size() == 2 && orig.objects.size() == 2 )
	{

	}
	else
	{
		cout << "orig: " << orig.objects.size() << " , actors: " << actorList.size() << endl;
		assert(0);
	}*/

	PolyPtr newPoly(new TerrainPolygon(&grassTex));
	polygonInProgress = newPoly;

	/*action->Perform();
	doneActionStack.push_back(action);

	ClearUndoneActions();*/

	return action;

	//cout << "adding: " << orig.objects.size() << ", " << progressBrush->objects.size() << endl;
	
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
		interMap[(*it).point].push_back( (*it).position );
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

Action* EditSession::ExecuteTerrainSubtract( list<PolyPtr> &intersectingPolys)
{
	Brush orig;
	Brush resultBrush;
	tempActors.clear();

	//Brush orig;
	map<TerrainPolygon*,list<TerrainPoint*>> addedPointsMap;
	list<boost::shared_ptr<GateInfo>> gateInfoList;

	for( list<PolyPtr>::iterator it = intersectingPolys.begin(); it != intersectingPolys.end(); ++it )
	{
		SelectPtr sp = boost::dynamic_pointer_cast<ISelectable>( (*it) );
		orig.AddObject( sp );

		TerrainPoint *pCurr = (*it)->pointStart;
		bool okGate = true;
		while (pCurr != NULL)
		{
			okGate = true;
			if (pCurr->gate != NULL)
			{
				for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
				{
					if ((*it) == pCurr->gate)
					{
						okGate = false;
						break;
					}
				}
				if (okGate)
				{
					SelectPtr sp1 = boost::dynamic_pointer_cast<ISelectable>(pCurr->gate);
					orig.AddObject(sp1);
					gateInfoList.push_back(pCurr->gate);
				}
					
			}
			pCurr = pCurr->next;
		}
		
		for (map<TerrainPoint*, std::list<ActorPtr>>::iterator
			mit = (*it)->enemies.begin(); mit != (*it)->enemies.end(); ++mit)
		{
			for (auto eit = (*mit).second.begin(); eit != (*mit).second.end(); ++eit)
			{
				SelectPtr sp1 = boost::dynamic_pointer_cast<ISelectable>((*eit));
				orig.AddObject(sp1);
			}
		}
			
	}

	list<PolyPtr> results;
	Sub( polygonInProgress, intersectingPolys, results );

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
			for (auto bit = (*mit).second.begin(); bit != (*mit).second.end(); ++bit)
			{
				for (list<PolyPtr>::iterator rit = results.begin();
					rit != results.end(); ++rit)
				{
					ActorParams *ac = AttachActorToPolygon((*bit), (*rit).get());
					if (ac != NULL)
					{
						ActorPtr newActor(ac);
						resultBrush.AddObject(newActor);
					}
					//AttachActorsToPolygon((*mit).second, (*rit).get());
					
					/*for (auto eit = polygonInProgress->enemies.begin();
						eit != polygonInProgress->enemies.end(); ++eit)
					{
						for (auto eit2 = (*eit).second.begin(); eit2 != (*eit).second.end(); ++eit2)
						{
							
						}

					}*/
				}
			}
		}
	}

	/*for (list<PolyPtr>::iterator rit = results.begin();
		rit != results.end(); ++rit)
	{
		for (auto eit = (*rit)->enemies.begin(); eit != (*rit)->enemies.end(); ++eit)
		{
			for (auto eit2 = (*eit).second.begin(); eit2 != (*eit).second.end(); ++eit2)
			{
				SelectPtr sp1 = boost::dynamic_pointer_cast<ISelectable>((*eit2));
				resultBrush.AddObject(sp1);
			}
		}
	}*/

	for (list<PolyPtr>::iterator rit = results.begin();
		rit != results.end(); ++rit)
	{
		for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
		{
			TerrainPoint *test = NULL;
			TerrainPoint * outTest = NULL;

			TerrainPoint *p0 = (*rit)->GetSamePoint((*it)->point0->pos);
			TerrainPoint *p1 = (*rit)->GetSamePoint((*it)->point1->pos);

			if (p0 == NULL && p1 == NULL)
				continue;

			GateInfoPtr gi(new GateInfo(*((*it).get())));
			gi->edit = NULL;

			if (p0 != NULL)
			{
				gi->point0 = p0;
				gi->poly0 = (*rit);
				p0->gate = gi;
			}
			else
			{
				gi->point0 = (*it)->point0;
				gi->poly0 = (*it)->poly0;
				gi->point0->gate = gi;
			}
			if (p1 != NULL)
			{
				gi->point1 = p1;
				gi->poly1 = (*rit);
				p1->gate = gi;

			}
			else
			{
				gi->point1 = (*it)->point1;
				gi->poly1 = (*it)->poly1;
				gi->point1->gate = gi;
			}

			SelectPtr sp1 = boost::dynamic_pointer_cast<ISelectable>(gi);
			resultBrush.AddObject(sp1);
		}
	}

	polygonInProgress->ClearPoints();

	Action * action = new ReplaceBrushAction(&orig, &resultBrush);

	/*action->Perform();
	doneActionStack.push_back(action);

	ClearUndoneActions();*/

	return action;
}

void EditSession::PointSelectPoint( V2d &worldPos,
	bool &emptysp )
{
	bool shift = IsKeyPressed(Keyboard::LShift) || IsKeyPressed(Keyboard::RShift);

	TerrainPoint *foundPoint = NULL;
	for (auto it = polygons.begin(); it != polygons.end(); ++it)
	{
		foundPoint = (*it)->GetClosePoint( 8 * zoomMultiple, worldPos );
		if (foundPoint != NULL)
		{
			emptysp = false;

			if (shift && foundPoint->selected )
			{
				DeselectPoint((*it).get(), foundPoint);
			}
			else
			{	
				if (!foundPoint->selected)
				{
					if (!shift)
						ClearSelectedPoints();

					SelectPoint((*it).get(), foundPoint);
				}			
			}
			break;
		}
	}

	if (foundPoint == NULL)
	{
	//	ClearSelectedPoints();
	}

	

	
	//double rad = 8 * zoomMultiple;
	//for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	//{
	//	//if its not even close dont check
	//	if( !(*it)->TempAABB().intersects( Rect<int>( worldPos.x - rad, worldPos.y - rad, rad * 2,
	//		rad * 2 ) ) )
	//	{
	//		continue;
	//	}

	//	TerrainPoint *tp = (*it)->pointStart;
	//	while( tp != NULL )
	//	{
	//		V2d tpPos( tp->pos.x, tp->pos.y );
	//		double dist = length( tpPos - worldPos );
	//		if( dist <= rad )
	//		{
	//			bool shift = IsKeyPressed( Keyboard::LShift ) || IsKeyPressed( Keyboard::RShift );
	//			if( !tp->selected )
	//			{
	//				if( !shift )
	//				{
	//					//ClearSelectedPoints();
	//				}
	//													
	//				selectedPoints[(*it).get()].push_back( PointMoveInfo( tp ) );
	//													

	//				tp->selected = true;
	//				emptysp = false;
	//			}
	//			else
	//			{
	//				emptysp = false;
	//			}
	//												
	//			//selectedPoints.push_back( tp );
	//												
	//		}
	//		tp = tp->next;
	//	}
	//}
}

void EditSession::BoxSelectPoints(sf::IntRect r,
	double radius)
{
	for (list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it)
	{
		//double rad = 8 * zoomMultiple;
		IntRect adjustedR(r.left - radius, r.top, r.width, r.height);

		//aabb w/ polygon

		if ((*it)->Intersects(adjustedR))
		{
			TerrainPoint *curr = (*it)->pointStart;
			while (curr != NULL)
			{
				if (IsQuadTouchingCircle(V2d(r.left, r.top),
					V2d(r.left + r.width, r.top),
					V2d(r.left + r.width, r.top + r.height),
					V2d(r.left, r.top + r.height),
					V2d(curr->pos.x, curr->pos.y), radius)
					|| adjustedR.contains(curr->pos))
				{
					SelectPoint((*it).get(), curr);
				}
				curr = curr->next;
			}
		}
	}
}

ActorType::ActorType(const std::string & n, Panel *p)
	:name(n), panel(p), imageTileIndex(0)
{
	EditSession *session = EditSession::GetSession();
	ts_image = NULL;
	//ts_icon = session->GetTileset(editorStr + name + string("_icon.png"));
	ts_image = session->GetTileset(string( "Editor/") + name + string("_editor.png"));

	Init();
}

sf::Sprite ActorType::GetSprite( int xSize, int ySize )
{
	if (ts_image == NULL)
	{
		return Sprite();
	}

	Sprite spr(*ts_image->texture);
	spr.setTextureRect(ts_image->GetSubRect(imageTileIndex));

	if (xSize != 0 && ySize != 0)
	{
		float xx = ((float)xSize) / ts_image->tileWidth;
		float yy = ((float)ySize) / ts_image->tileHeight;
		float scale = min(xx, yy);
		spr.setScale(scale, scale);
	}
	return spr;
}

sf::Sprite ActorType::GetSprite(bool grounded)
{
	Sprite s = GetSprite();
	if (grounded)
	{
		if (name != "poi")
			s.setOrigin(s.getLocalBounds().width / 2 + imageOffset.x, s.getLocalBounds().height + imageOffset.y);
		else
		{
			s.setOrigin(s.getLocalBounds().width / 2 + imageOffset.x, s.getLocalBounds().height / 2 + imageOffset.y);
		}
	}
	else
	{
		s.setOrigin(s.getLocalBounds().width / 2 + imageOffset.x, s.getLocalBounds().height / 2 + imageOffset.y);
	}

	return s;
}


void ActorType::LoadEnemy(std::ifstream &is, ActorPtr &a)
{
	EditSession *edit = EditSession::GetSession();
	int terrainIndex;
	int edgeIndex;
	Vector2i pos;
	double edgeQuantity;
	int hasMonitor;

	if (name == "goal")
	{
		//always grounded
		is >> terrainIndex;
		is >> edgeIndex;
		is >> edgeQuantity;

		TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);

		a.reset(new GoalParams(terrain, edgeIndex, edgeQuantity));
		terrain->enemies[a->groundInfo->edgeStart].push_back(a);
		terrain->UpdateBounds();
		//a->SetAsGoal( terrain, edgeIndex, edgeQuantity );
	}
	else if (name == "healthfly")
	{
		//always air
		is >> pos.x;
		is >> pos.y;
		is >> hasMonitor;

		int color;
		is >> color;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new HealthFlyParams(pos, color));
		a->hasMonitor = (bool)hasMonitor;
	}
	else if (name == "poi")
	{
		string air;
		is >> air;
		if (air == "+air")
		{
			is >> pos.x;
			is >> pos.y;

			string pname;
			is >> pname;

			string marker;
			is >> marker;

			PoiParams::Barrier b;
			if (marker == "-")
			{
				b = PoiParams::NONE;
			}
			else if (marker == "x")
			{
				b = PoiParams::X;
			}
			else if (marker == "y")
			{
				b = PoiParams::Y;
			}

			int hasCamProps;
			is >> hasCamProps;

			float camZoom = 1;
			if (hasCamProps)
			{
				is >> camZoom;
			}




			a.reset(new PoiParams(pos, b, pname, hasCamProps, camZoom));
		}
		else if (air == "-air")
		{
			
			is >> terrainIndex;
			is >> edgeIndex;
			is >> edgeQuantity;

			TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);


			string pname;
			is >> pname;

			string marker;
			is >> marker;

			PoiParams::Barrier b;
			if (marker == "-")
			{
				b = PoiParams::NONE;
			}
			else if (marker == "x")
			{
				b = PoiParams::X;
			}
			else if (marker == "y")
			{
				b = PoiParams::Y;
			}

			a.reset(new PoiParams(
				terrain, edgeIndex, edgeQuantity, b, pname));

			terrain->enemies[a->groundInfo->edgeStart].push_back(a);
			terrain->UpdateBounds();
		}
		else
		{
			assert(0);
		}

	}
	else if (name == "key")
	{
		//always air
		is >> pos.x;
		is >> pos.y;

		int numKeys;
		is >> numKeys;

		int zType;
		is >> zType;
		//int hasMonitor;
		//is >> hasMonitor;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new KeyParams(pos, numKeys, zType));


		//a->hasMonitor = false;//(bool)hasMonitor;
	}
	else if (name == "shippickup")
	{
		//always grounded

		
		is >> terrainIndex;
		is >> edgeIndex;
		is >> edgeQuantity;

		int facingRight;
		is >> facingRight;

		TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);

		a.reset(new ShipPickupParams(terrain, edgeIndex, edgeQuantity, facingRight));
		terrain->enemies[a->groundInfo->edgeStart].push_back(a);
		terrain->UpdateBounds();
	}
	else if (name == "groundtrigger")
	{
		//always grounded
		assert(is.good());
		is >> terrainIndex;
		is >> edgeIndex;
		is >> edgeQuantity;

		int facingRight;
		is >> facingRight;

		string typeStr;
		is >> typeStr;

		TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);

		a.reset(new GroundTriggerParams(terrain, edgeIndex, edgeQuantity, facingRight,
			typeStr));
		terrain->enemies[a->groundInfo->edgeStart].push_back(a);
		terrain->UpdateBounds();
	}
	else if (name == "flowerpod")
	{
		//always grounded
		assert(is.good());
		is >> terrainIndex;
		is >> edgeIndex;
		is >> edgeQuantity;

		string typeStr;
		is >> typeStr;

		TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);

		a.reset(new FlowerPodParams(terrain, edgeIndex, edgeQuantity,
			typeStr));
		terrain->enemies[a->groundInfo->edgeStart].push_back(a);
		terrain->UpdateBounds();
	}
	else if (name == "airtrigger")
	{
		//always air
		is >> pos.x;
		is >> pos.y;

		string typeStr;
		is >> typeStr;

		int rectWidth;
		is >> rectWidth;

		int rectHeight;
		is >> rectHeight;
		//int hasMonitor;
		//is >> hasMonitor;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new AirTriggerParams(pos, typeStr, rectWidth, rectHeight));
		//a->hasMonitor = (bool)hasMonitor;
	}
	else if (name == "shard")
	{

		//always air
		is >> pos.x;
		is >> pos.y;

		int w;
		is >> w;

		int li;
		is >> li;
		//int hasMonitor;
		//is >> hasMonitor;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new ShardParams(pos, w, li));
		//a->hasMonitor = (bool)hasMonitor;
	}
	else if (name == "racefighttarget")
	{

		//always air
		is >> pos.x;
		is >> pos.y;

		//int hasMonitor;
		//is >> hasMonitor;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new RaceFightTargetParams(pos));
		//a->hasMonitor = (bool)hasMonitor;
	}
	else if (name == "blocker")
	{

		//always air
		is >> pos.x;
		is >> pos.y;

		int pathLength;
		is >> pathLength;

		list<Vector2i> globalPath;
		globalPath.push_back(Vector2i(pos.x, pos.y));

		for (int i = 0; i < pathLength; ++i)
		{
			int localX, localY;
			is >> localX;
			is >> localY;
			globalPath.push_back(Vector2i(pos.x + localX, pos.y + localY));
		}

		int bType;
		is >> bType;

		int armored;
		is >> armored;

		int spacing;
		is >> spacing;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new BlockerParams(pos, globalPath, bType, armored, spacing));
		//a->hasMonitor = (bool)hasMonitor;
	}
	else if (name == "comboer")
	{

		//always air
		is >> pos.x;
		is >> pos.y;

		int hasMonitor;
		is >> hasMonitor;

		int pathLength;
		is >> pathLength;

		list<Vector2i> globalPath;
		globalPath.push_back(Vector2i(pos.x, pos.y));

		for (int i = 0; i < pathLength; ++i)
		{
			int localX, localY;
			is >> localX;
			is >> localY;
			globalPath.push_back(Vector2i(pos.x + localX, pos.y + localY));
		}


		bool loop;
		string loopStr;
		is >> loopStr;
		if (loopStr == "+loop")
			loop = true;
		else if (loopStr == "-loop")
			loop = false;
		else
			assert(false && "should be a boolean");


		float speed;
		is >> speed;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new ComboerParams(pos, globalPath, speed, loop));
		a->hasMonitor = (bool)hasMonitor;
	}
	else if (name == "rail")
	{

		//always air
		is >> pos.x;
		is >> pos.y;

		int pathLength;
		is >> pathLength;

		list<Vector2i> globalPath;
		globalPath.push_back(Vector2i(pos.x, pos.y));

		for (int i = 0; i < pathLength; ++i)
		{
			int localX, localY;
			is >> localX;
			is >> localY;
			globalPath.push_back(Vector2i(pos.x + localX, pos.y + localY));
		}

		int energized;
		is >> energized;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new RailParams(pos, globalPath, energized));
		//a->hasMonitor = (bool)hasMonitor;
	}
	else if (name == "booster")
	{

		//always air
		is >> pos.x;
		is >> pos.y;

		int strength;
		is >> strength;
		//int hasMonitor;
		//is >> hasMonitor;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new BoosterParams(pos, strength));
		//a->hasMonitor = (bool)hasMonitor;
	}
	else if (name == "spring")
	{

		//always air
		is >> pos.x;
		is >> pos.y;

		//int hasMonitor;
		//is >> hasMonitor;
		int moveFrames;
		is >> moveFrames;

		Vector2i other;
		is >> other.x;
		is >> other.y;


		list<Vector2i> globalPath;
		globalPath.push_back(Vector2i(pos.x, pos.y));
		globalPath.push_back(pos + other);

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new SpringParams(pos, globalPath, moveFrames));
		//a->hasMonitor = (bool)hasMonitor;
	}
	//w1
	else if (name == "bosscrawler")
	{
		//always grounded
		is >> terrainIndex;
		is >> edgeIndex;
		is >> edgeQuantity;

		TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);

		//a->SetAsFootTrap( at, terrain, edgeIndex, edgeQuantity );
		a.reset(new BossCrawlerParams(terrain, edgeIndex, edgeQuantity));
		terrain->enemies[a->groundInfo->edgeStart].push_back(a);
		terrain->UpdateBounds();
	}
	else if (name == "basicturret")
	{
		//always grounded

		is >> terrainIndex;
		is >> edgeIndex;
		is >> edgeQuantity;

		is >> hasMonitor;

		double bulletSpeed;
		is >> bulletSpeed;

		int framesWait;
		is >> framesWait;

		TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);

		//a->SetAsBasicTurret( at, terrain, edgeIndex, edgeQuantity, bulletSpeed, framesWait );
		a.reset(new BasicTurretParams(terrain, edgeIndex, edgeQuantity, bulletSpeed, framesWait));
		a->hasMonitor = (bool)hasMonitor;
		terrain->enemies[a->groundInfo->edgeStart].push_back(a);
		terrain->UpdateBounds();
	}
	else if (name == "foottrap")
	{
		//always grounded
		is >> terrainIndex;
		is >> edgeIndex;
		is >> edgeQuantity;

		is >> hasMonitor;

		TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);

		//a->SetAsFootTrap( at, terrain, edgeIndex, edgeQuantity );
		a.reset(new FootTrapParams(terrain, edgeIndex, edgeQuantity));
		a->hasMonitor = (bool)hasMonitor;
		terrain->enemies[a->groundInfo->edgeStart].push_back(a);
		terrain->UpdateBounds();
	}
	else if (name == "patroller")
	{

		//always air
		is >> pos.x;
		is >> pos.y;

		is >> hasMonitor;

		int pathLength;
		is >> pathLength;

		list<Vector2i> globalPath;
		globalPath.push_back(Vector2i(pos.x, pos.y));

		for (int i = 0; i < pathLength; ++i)
		{
			int localX, localY;
			is >> localX;
			is >> localY;
			globalPath.push_back(Vector2i(pos.x + localX, pos.y + localY));
		}


		bool loop;
		string loopStr;
		is >> loopStr;
		if (loopStr == "+loop")
			loop = true;
		else if (loopStr == "-loop")
			loop = false;
		else
			assert(false && "should be a boolean");


		float speed;
		is >> speed;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new PatrollerParams(pos, globalPath, speed, loop));
		a->hasMonitor = (bool)hasMonitor;

	}
	else if (name == "crawler")
	{

		//always grounded

		is >> terrainIndex;
		is >> edgeIndex;
		is >> edgeQuantity;
		is >> hasMonitor;

		bool clockwise;
		string cwStr;
		is >> cwStr;

		if (cwStr == "+clockwise")
			clockwise = true;
		else if (cwStr == "-clockwise")
			clockwise = false;
		else
		{
			assert(false && "boolean problem");
		}

		int speed;
		is >> speed;

		int dist;
		is >> dist;

		TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);

		//a->SetAsCrawler( at, terrain, edgeIndex, edgeQuantity, clockwise, speed ); 
		a.reset(new CrawlerParams(terrain, edgeIndex, edgeQuantity, clockwise, speed));
		a->hasMonitor = (bool)hasMonitor;
		terrain->enemies[a->groundInfo->edgeStart].push_back(a);
		terrain->UpdateBounds();
	}
	else if (name == "airdasher")
	{
		//always air
		is >> pos.x;
		is >> pos.y;

		is >> hasMonitor;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new AirdasherParams(pos));
		a->hasMonitor = (bool)hasMonitor;
	}

	//w2
	else if (name == "bat")
	{
		

		//always air
		is >> pos.x;
		is >> pos.y;

		is >> hasMonitor;

		int pathLength;
		is >> pathLength;

		list<Vector2i> globalPath;
		globalPath.push_back(Vector2i(pos.x, pos.y));

		for (int i = 0; i < pathLength; ++i)
		{
			int localX, localY;
			is >> localX;
			is >> localY;
			globalPath.push_back(Vector2i(pos.x + localX, pos.y + localY));
		}


		bool loop;
		string loopStr;
		is >> loopStr;
		if (loopStr == "+loop")
			loop = true;
		else if (loopStr == "-loop")
			loop = false;
		else
			assert(false && "should be a boolean");

		int bulletSpeed;
		is >> bulletSpeed;

		//int nodeDistance;
		//is >> nodeDistance;

		int framesBetweenNodes;
		is >> framesBetweenNodes;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new BatParams(pos, globalPath, bulletSpeed,
			framesBetweenNodes, loop));
		a->hasMonitor = (bool)hasMonitor;

	}
	else if (name == "curveturret")
	{
		//always grounded

		is >> terrainIndex;
		is >> edgeIndex;
		is >> edgeQuantity;
		is >> hasMonitor;

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
		if (relativeGravStr == "+relative")
		{
			relative = true;
		}

		TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);

		//a->SetAsBasicTurret( at, terrain, edgeIndex, edgeQuantity, bulletSpeed, framesWait );
		a.reset(new CurveTurretParams(terrain, edgeIndex, edgeQuantity, bulletSpeed, framesWait,
			Vector2i(xGravFactor, yGravFactor), relative));
		a->hasMonitor = (bool)hasMonitor;
		terrain->enemies[a->groundInfo->edgeStart].push_back(a);
		terrain->UpdateBounds();
	}
	else if (name == "stagbeetle")
	{

		//always grounded

		is >> terrainIndex;
		is >> edgeIndex;
		is >> edgeQuantity;
		is >> hasMonitor;

		bool clockwise;
		string cwStr;
		is >> cwStr;

		if (cwStr == "+clockwise")
			clockwise = true;
		else if (cwStr == "-clockwise")
			clockwise = false;
		else
		{
			assert(false && "boolean problem");
		}

		float speed;
		is >> speed;

		TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);

		//a->SetAsCrawler( at, terrain, edgeIndex, edgeQuantity, clockwise, speed ); 
		a.reset(new StagBeetleParams(terrain, edgeIndex, edgeQuantity, clockwise, speed));
		a->hasMonitor = (bool)hasMonitor;
		terrain->enemies[a->groundInfo->edgeStart].push_back(a);
		terrain->UpdateBounds();
	}
	else if (name == "poisonfrog")
	{

		//always grounded

		is >> terrainIndex;
		is >> edgeIndex;
		is >> edgeQuantity;
		is >> hasMonitor;

		int gravFactor;
		is >> gravFactor;

		int xStrength;
		is >> xStrength;

		int yStrength;
		is >> yStrength;

		int jumpWaitFrames;
		is >> jumpWaitFrames;

		TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);

		//a->SetAsCrawler( at, terrain, edgeIndex, edgeQuantity, clockwise, speed ); 
		a.reset(new PoisonFrogParams(terrain, edgeIndex, edgeQuantity, gravFactor,
			Vector2i(xStrength, yStrength), jumpWaitFrames));//, clockwise, speed ) ); 
		a->hasMonitor = (bool)hasMonitor;
		terrain->enemies[a->groundInfo->edgeStart].push_back(a);
		terrain->UpdateBounds();
	}
	else if (name == "gravityfaller")
	{

		//always grounded

		is >> terrainIndex;
		is >> edgeIndex;
		is >> edgeQuantity;
		is >> hasMonitor;

		int var;
		is >> var;

		TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);

		a.reset(new GravityFallerParams(terrain, edgeIndex, edgeQuantity, var));
		a->hasMonitor = (bool)hasMonitor;
		terrain->enemies[a->groundInfo->edgeStart].push_back(a);
		terrain->UpdateBounds();
	}
	else if (name == "bossbird")
	{
		is >> pos.x;
		is >> pos.y;

		a.reset(new BossBirdParams(pos));
	}

	//w3
	else if (name == "pulser")
	{
		//always air
		is >> pos.x;
		is >> pos.y;

		is >> hasMonitor;

		int pathLength;
		is >> pathLength;

		list<Vector2i> globalPath;
		globalPath.push_back(Vector2i(pos.x, pos.y));

		for (int i = 0; i < pathLength; ++i)
		{
			int localX, localY;
			is >> localX;
			is >> localY;
			globalPath.push_back(Vector2i(pos.x + localX, pos.y + localY));
		}


		bool loop;
		string loopStr;
		is >> loopStr;
		if (loopStr == "+loop")
			loop = true;
		else if (loopStr == "-loop")
			loop = false;
		else
			assert(false && "should be a boolean");

		int framesBetweenNodes;
		is >> framesBetweenNodes;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new PulserParams(pos, globalPath, framesBetweenNodes, loop));
		a->hasMonitor = (bool)hasMonitor;

	}
	else if (name == "badger")
	{

		//always grounded

		is >> terrainIndex;
		is >> edgeIndex;
		is >> edgeQuantity;
		is >> hasMonitor;

		int speed;
		is >> speed;

		int jumpStrength;
		is >> jumpStrength;

		TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);

		//a->SetAsCrawler( at, terrain, edgeIndex, edgeQuantity, clockwise, speed ); 
		a.reset(new BadgerParams(terrain, edgeIndex, edgeQuantity, speed, jumpStrength));
		a->hasMonitor = (bool)hasMonitor;
		terrain->enemies[a->groundInfo->edgeStart].push_back(a);
		terrain->UpdateBounds();
	}
	else if (name == "cactus")
	{

		//always grounded

		is >> terrainIndex;
		is >> edgeIndex;
		is >> edgeQuantity;
		is >> hasMonitor;

		int bulletSpeed;
		is >> bulletSpeed;

		int rhythm;
		is >> rhythm;

		int amplitude;
		is >> amplitude;

		TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);

		//a->SetAsCrawler( at, terrain, edgeIndex, edgeQuantity, clockwise, speed ); 
		a.reset(new CactusParams(terrain, edgeIndex, edgeQuantity, bulletSpeed, rhythm, amplitude));
		a->hasMonitor = (bool)hasMonitor;
		terrain->enemies[a->groundInfo->edgeStart].push_back(a);
		terrain->UpdateBounds();
	}
	else if (name == "owl")
	{

		//always air
		is >> pos.x;
		is >> pos.y;

		is >> hasMonitor;

		int moveSpeed;
		is >> moveSpeed;

		int bulletSpeed;
		is >> bulletSpeed;

		int rhythmFrames;
		is >> rhythmFrames;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new OwlParams(pos, moveSpeed, bulletSpeed, rhythmFrames));
		a->hasMonitor = (bool)hasMonitor;
	}
	else if (name == "bosscoyote")
	{
		is >> pos.x;
		is >> pos.y;

		//a->SetAsFootTrap( at, terrain, edgeIndex, edgeQuantity );
		a.reset(new BossCoyoteParams(pos));
	}

	//w4
	else if (name == "cheetah")
	{

		//always grounded

		is >> terrainIndex;
		is >> edgeIndex;
		is >> edgeQuantity;
		is >> hasMonitor;

		TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);

		//a->SetAsCrawler( at, terrain, edgeIndex, edgeQuantity, clockwise, speed ); 
		a.reset(new CheetahParams(terrain, edgeIndex, edgeQuantity));
		a->hasMonitor = (bool)hasMonitor;
		terrain->enemies[a->groundInfo->edgeStart].push_back(a);
		terrain->UpdateBounds();
	}
	else if (name == "spider")
	{

		//always grounded

		is >> terrainIndex;
		is >> edgeIndex;
		is >> edgeQuantity;
		is >> hasMonitor;

		int speed;
		is >> speed;

		TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);

		//a->SetAsCrawler( at, terrain, edgeIndex, edgeQuantity, clockwise, speed ); 
		a.reset(new SpiderParams(terrain, edgeIndex, edgeQuantity,
			speed));
		a->hasMonitor = (bool)hasMonitor;
		terrain->enemies[a->groundInfo->edgeStart].push_back(a);
		terrain->UpdateBounds();
	}
	else if (name == "coral")
	{
		//always air
		is >> pos.x;
		is >> pos.y;

		is >> hasMonitor;

		int moveFrames;
		is >> moveFrames;



		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new CoralParams(pos, moveFrames));
		a->hasMonitor = (bool)hasMonitor;
	}
	else if (name == "turtle")
	{

		//always air
		is >> pos.x;
		is >> pos.y;
		is >> hasMonitor;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new TurtleParams(pos));
		a->hasMonitor = (bool)hasMonitor;
	}
	else if (name == "bosstiger")
	{
		is >> pos.x;
		is >> pos.y;

		a.reset(new BossTigerParams(pos));
	}

	//w5
	else if (name == "overgrowth")
	{

		//always grounded

		is >> terrainIndex;
		is >> edgeIndex;
		is >> edgeQuantity;
		is >> hasMonitor;

		TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);

		//a->SetAsCrawler( at, terrain, edgeIndex, edgeQuantity, clockwise, speed ); 
		a.reset(new OvergrowthParams(terrain, edgeIndex, edgeQuantity));
		a->hasMonitor = (bool)hasMonitor;
		terrain->enemies[a->groundInfo->edgeStart].push_back(a);
		terrain->UpdateBounds();
	}
	else if (name == "swarm")
	{
		cout << "loading swarm" << endl;

		//always air
		is >> pos.x;
		is >> pos.y;

		is >> hasMonitor;

		int liveFrames;
		is >> liveFrames;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new SwarmParams(pos, liveFrames));
		a->hasMonitor = (bool)hasMonitor;
	}
	else if (name == "ghost")
	{
		cout << "loading ghost" << endl;

		//always air
		is >> pos.x;
		is >> pos.y;

		is >> hasMonitor;

		int speed;
		is >> speed;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new GhostParams(pos, speed));
		a->hasMonitor = (bool)hasMonitor;
	}
	else if (name == "shark")
	{

		//always air
		is >> pos.x;
		is >> pos.y;

		is >> hasMonitor;

		int circleFrames;
		is >> circleFrames;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new SharkParams(pos, circleFrames));
		a->hasMonitor = (bool)hasMonitor;
	}
	else if (name == "bossgator")
	{
		Vector2i pos;

		is >> pos.x;
		is >> pos.y;

		a.reset(new BossGatorParams(pos));
	}

	//w6
	else if (name == "specter")
	{

		//always air
		is >> pos.x;
		is >> pos.y;

		is >> hasMonitor;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new SpecterParams(pos));
		a->hasMonitor = (bool)hasMonitor;
	}
	else if (name == "gorilla")
	{

		//always air
		is >> pos.x;
		is >> pos.y;

		is >> hasMonitor;

		int wallWidth;
		is >> wallWidth;

		int followFrames;
		is >> followFrames;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new GorillaParams(pos, wallWidth, followFrames));
		a->hasMonitor = (bool)hasMonitor;
	}
	else if (name == "narwhal")
	{

		//always air
		is >> pos.x;
		is >> pos.y;

		is >> hasMonitor;

		//int pathLength;
		//is >> pathLength;

		Vector2i destinationPos;
		is >> destinationPos.x;
		is >> destinationPos.y;

		int moveFrames;
		is >> moveFrames;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new NarwhalParams(pos, destinationPos, moveFrames));
		a->hasMonitor = (bool)hasMonitor;
	}
	else if (name == "copycat")
	{

		//always air? should have a grounded mode too
		is >> pos.x;
		is >> pos.y;

		is >> hasMonitor;

		//a->SetAsPatroller( at, pos, globalPath, speed, loop );	
		a.reset(new CopycatParams(pos));
		a->hasMonitor = (bool)hasMonitor;
	}
	else if (name == "bossskeleton")
	{

		is >> pos.x;
		is >> pos.y;

		a.reset(new BossSkeletonParams(pos));
	}

	else if (name == "nexus")
	{
		//always grounded

		is >> terrainIndex;
		is >> edgeIndex;
		is >> edgeQuantity;

		int nexusIndex;
		is >> nexusIndex;

		TerrainPolygon *terrain = edit->GetPolygon(terrainIndex, edgeIndex);

		a.reset(new NexusParams(terrain, edgeIndex, edgeQuantity, nexusIndex));
		terrain->enemies[a->groundInfo->edgeStart].push_back(a);
		terrain->UpdateBounds();
	}

	else
	{
		assert(false && "unkown enemy type!");
	}
}

bool ActorType::IsGoalType()
{
	return name == "goal"
		|| name == "shippickup"
		|| name == "nexus";
}

void ActorType::Init()
{
	EditSession *session = EditSession::GetSession();

	if( name == "healthfly" )
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
	}
	else if( name == "goal" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
		ts_image = session->GetTileset("Goal/goal_w01_a_288x320.png", 288, 320);
		imageOffset.y = -32;
	}
	else if( name == "player" )
	{
		width = 22;//40;
		height = 42;//64;
		canBeGrounded = false;
		canBeAerial = true;
		ts_image = session->GetTileset("Kin/jump_64x64.png", 64, 64);
		imageTileIndex = 2;
	}
	else if( name == "poi" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = true;
		ts_image = session->GetTileset("Editor/poi_editor.png");
	}
	else if( name == "key" )
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
		ts_image = session->GetTileset("Editor/key_editor.png");
	}
	if( name == "shippickup" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
		ts_image = session->GetTileset("Ship/shipleave_128x128.png", 128, 128);
	}
	if (name == "groundtrigger")
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
		ts_image = session->GetTileset("Ship/shipleave_128x128.png", 128, 128);
	}
	if (name == "airtrigger")
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
		ts_image = session->GetTileset("Enemies/jayshield_128x128.png", 128, 128);
	}
	else if( name == "shard" )
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
		ts_image = session->GetTileset("Shard/shards_w1_192x192.png", 192, 192);
	}
	else if( name == "racefighttarget" )
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
		ts_image = session->GetTileset("Editor/racefighttarget_editor.png");
	}
	else if (name == "blocker")
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
		ts_image = session->GetTileset("Enemies/blocker_w1_192x192.png", 192, 192);
	}
	else if (name == "flowerpod")
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
		ts_image = session->GetTileset("Momenta/momentaflower_128x128.png", 128, 128);
	}
	else if (name == "comboer")
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
		ts_image = session->GetTileset("Enemies/comboer_128x128.png", 128, 128);
	}
	else if (name == "rail")
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
		ts_image = session->GetTileset("Editor/rail_editor.png");
	}
	else if (name == "booster")
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
		ts_image = session->GetTileset("Enemies/Booster_512x512.png", 512, 512);
	}
	else if (name == "spring")
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
		ts_image = session->GetTileset("Enemies/spring_idle_256x256.png", 256, 256);
	}
	//w1
	else if( name == "patroller" )
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
		ts_image = session->GetTileset("Editor/patroller_editor.png");
	}
	else if( name == "crawler" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
		ts_image = session->GetTileset("Enemies/crawler_160x160.png", 160, 160);
	}
	else if( name == "basicturret" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
		ts_image = session->GetTileset("Enemies/basicturret_128x80.png", 128, 80);
	}
	else if( name == "foottrap" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
		ts_image = session->GetTileset("Enemies/foottrap_editor.png");
	}
	else if( name == "bosscrawler" )
	{
		width = 128;
		height = 144;
		canBeGrounded = true;
		canBeAerial = false;
		ts_image = session->GetTileset("Bosses/Crawler/crawler_queen_256x256.png", 256, 256);
	}
	else if (name == "airdasher")
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
		ts_image = session->GetTileset("Enemies/dasher_208x144.png", 208, 144);
	}

	//w2
	else if( name == "bat" )
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
		ts_image = session->GetTileset("Enemies/bat_144x176.png", 144, 176);
	}
	else if( name == "curveturret" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
		ts_image = session->GetTileset("Enemies/curveturret_144x96.png", 144, 96);
	}
	else if( name == "stagbeetle" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
		ts_image = session->GetTileset("Enemies/stag_idle_192x144.png", 192, 144);
	}
	else if( name == "poisonfrog" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
		ts_image = session->GetTileset("Enemies/frog_80x80.png", 80, 80);
	}
	else if (name == "gravityfaller")
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
		ts_image = session->GetTileset("Enemies/gravity_faller_128x128.png", 128, 128);
	}
	else if( name == "bossbird" )
	{
		width = 64;
		height = 64;
		canBeGrounded = false;
		canBeAerial = true;
		ts_image = session->GetTileset("Enemies/bossbird_editor.png");
	}
	
	//w3
	else if( name == "pulser" )
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
		//ts_image = session->GetTileset("Editor//gravity_faller_128x128.png", 128, 128);
	}
	else if( name == "badger" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
	}
	else if( name == "cactus" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
	}
	else if( name == "owl" )
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
	}
	else if( name == "bosscoyote" )
	{
		width = 200;
		height = 200;
		canBeGrounded = false;
		canBeAerial = true;
	}
	
	//w4
	else if( name == "turtle" )
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
	}
	else if( name == "coral" )
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
	}
	else if( name == "spider" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
	}
	else if( name == "cheetah" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
	}
	else if( name == "bosstiger" )
	{
		width = 64;
		height = 64;
		canBeGrounded = false;
		canBeAerial = true;
	}
	
	//w5
	else if( name == "swarm" )
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
	}
	else if( name == "shark" )
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
	}
	else if( name == "ghost" )
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
	}
	else if( name == "overgrowth" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
	}
	else if( name == "bossgator" )
	{
		width = 32;
		height = 128;
		canBeGrounded = false;
		canBeAerial = true;
	}

	//w6
	else if( name == "specter" )
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
	}
	else if( name == "narwhal" )
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
	}
	else if( name == "gorilla" )
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
	}
	else if( name == "copycat" )
	{
		width = 32;
		height = 32;
		canBeGrounded = false;
		canBeAerial = true;
	}
	else if( name == "bossskeleton" )
	{
		width = 32;
		height = 128;
		canBeGrounded = false;
		canBeAerial = true;
	}

	//extra
	else if( name == "nexus" )
	{
		width = 32;
		height = 32;
		canBeGrounded = true;
		canBeAerial = false;
	}
}

void ActorType::PlaceEnemy()
{
	EditSession *edit = EditSession::GetSession();

	Vector2i worldPos(edit->worldPos);

	if (name == "healthfly")
	{
		edit->showPanel = panel;

		edit->showPanel->textBoxes["name"]->text.setString("test");
		edit->showPanel->textBoxes["group"]->text.setString("not test");

		edit->airPos = worldPos;
	}
	else if (name == "goal")
	{
		if (edit->enemyEdgePolygon != NULL)
		{
			edit->showPanel = edit->enemySelectPanel;
			edit->trackingEnemy = NULL;
			ActorPtr goal(new GoalParams( edit->enemyEdgePolygon, edit->enemyEdgeIndex,
				edit->enemyEdgeQuantity));
			goal->group = edit->groups["--"];

			edit->CreateActor(goal);
		}
	}
	else if (name == "poi")
	{
		if (edit->enemyEdgePolygon != NULL)
		{
			edit->tempActor = new PoiParams(
				edit->enemyEdgePolygon, edit->enemyEdgeIndex,
				edit->enemyEdgeQuantity);
			edit->showPanel = panel;
			edit->tempActor->SetPanelInfo();
		}
		else
		{
			edit->tempActor = new PoiParams( worldPos);
			edit->showPanel = panel;
			edit->tempActor->SetPanelInfo();
		}
	}
	else if (name == "key")
	{
		edit->tempActor = new KeyParams( worldPos);
		edit->showPanel = panel;
		edit->tempActor->SetPanelInfo();
	}
	else if (name == "shippickup")
	{
		if (edit->enemyEdgePolygon != NULL)
		{
			edit->showPanel = edit->enemySelectPanel;
			edit->trackingEnemy = NULL;
			ActorPtr shipPickup(new ShipPickupParams( edit->enemyEdgePolygon, edit->enemyEdgeIndex,
				edit->enemyEdgeQuantity));
			shipPickup->group = edit->groups["--"];

			edit->CreateActor(shipPickup);
		}
	}
	else if (name == "groundtrigger")
	{
		if (edit->enemyEdgePolygon != NULL)
		{
			/*edit->showPanel = edit->enemySelectPanel;
			edit->trackingEnemy = NULL;
			ActorPtr groundTrigger(new GroundTriggerParams( edit->enemyEdgePolygon, edit->enemyEdgeIndex,
			edit->enemyEdgeQuantity));
			groundTrigger->group = edit->groups["--"];

			edit->CreateActor(groundTrigger);*/


			edit->tempActor = new GroundTriggerParams( edit->enemyEdgePolygon, edit->enemyEdgeIndex,
				edit->enemyEdgeQuantity);
			edit->tempActor->SetPanelInfo();
			edit->showPanel = panel;
		}
	}
	else if (name == "airtrigger")
	{
		edit->tempActor = new AirTriggerParams( worldPos);
		edit->tempActor->SetPanelInfo();
		edit->showPanel = panel;
	}
	else if (name == "shard")
	{
		edit->tempActor = new ShardParams(worldPos);
		edit->tempActor->SetPanelInfo();
		edit->showPanel = panel;
	}
	else if (name == "racefighttarget")
	{
		edit->trackingEnemy = NULL;
		ActorPtr raceFightTarget(new RaceFightTargetParams( worldPos));
		raceFightTarget->group = edit->groups["--"];
		edit->CreateActor(raceFightTarget);
		edit->showPanel = edit->enemySelectPanel;
	}
	else if (name == "blocker")
	{
		//edit->trackingEnemy = NULL;
		edit->tempActor = new BlockerParams( worldPos);
		//blocker->group = edit->groups["--"];
		//edit->CreateActor(blocker);
		edit->tempActor->SetPanelInfo();
		edit->showPanel = panel;


		edit->patrolPath.clear();
		edit->patrolPath.push_back(worldPos);
		//edit->showPanel = panel;
		//edit->tempActor->SetPanelInfo();
		//edit->showPanel = edit->enemySelectPanel;
	}
	else if (name == "flowerpod")
	{
		if (edit->enemyEdgePolygon != NULL)
		{
			/*edit->showPanel = edit->enemySelectPanel;
			edit->trackingEnemy = NULL;
			ActorPtr groundTrigger(new GroundTriggerParams( edit->enemyEdgePolygon, edit->enemyEdgeIndex,
			edit->enemyEdgeQuantity));
			groundTrigger->group = edit->groups["--"];

			edit->CreateActor(groundTrigger);*/


			edit->tempActor = new FlowerPodParams( edit->enemyEdgePolygon, edit->enemyEdgeIndex,
				edit->enemyEdgeQuantity);
			edit->tempActor->SetPanelInfo();
			edit->showPanel = panel;
		}
	}
	else if (name == "comboer")
	{
		edit->tempActor = new ComboerParams( worldPos);
		edit->tempActor->SetPanelInfo();
		//edit->tempActor->SetDefaultPanelInfo();

		edit->showPanel = panel;

		edit->patrolPath.clear();
		edit->patrolPath.push_back(worldPos);
	}

	else if (name == "rail")
	{
		//edit->trackingEnemy = NULL;
		edit->tempActor = new RailParams( worldPos);
		//blocker->group = edit->groups["--"];
		//edit->CreateActor(blocker);
		edit->tempActor->SetPanelInfo();
		edit->showPanel = panel;


		edit->patrolPath.clear();
		edit->patrolPath.push_back(worldPos);
		//edit->showPanel = panel;
		//edit->tempActor->SetPanelInfo();
		//edit->showPanel = edit->enemySelectPanel;
	}
	else if (name == "booster")
	{
		edit->tempActor = new BoosterParams( worldPos);
		edit->tempActor->SetPanelInfo();
		edit->showPanel = panel;


		/*edit->trackingEnemy = NULL;
		ActorPtr booster(new BoosterParams( Vector2i(worldPos.x,
		worldPos.y)));
		booster->group = edit->groups["--"];
		edit->CreateActor(booster);
		edit->showPanel = panel;*/
	}
	else if (name == "spring")
	{
		/*edit->trackingEnemy = NULL;
		ActorPtr spring(new SpringParams( Vector2i(worldPos.x,
		worldPos.y)));
		spring->group = edit->groups["--"];
		edit->CreateActor(spring);
		edit->showPanel = edit->enemySelectPanel;*/



		edit->tempActor = new SpringParams( worldPos);
		edit->tempActor->SetPanelInfo();
		edit->showPanel = panel;

		edit->patrolPath.clear();
		edit->patrolPath.push_back(worldPos);
	}


	//w1
	else if (name == "patroller")
	{

		edit->tempActor = new PatrollerParams( worldPos);
		edit->tempActor->SetPanelInfo();
		//edit->tempActor->SetDefaultPanelInfo();

		edit->showPanel = panel;


		edit->patrolPath.clear();
		edit->patrolPath.push_back(worldPos);
	}
	else if (name == "crawler")
	{
		if (edit->enemyEdgePolygon != NULL)
		{
			edit->tempActor = new CrawlerParams(
				edit->enemyEdgePolygon, edit->enemyEdgeIndex,
				edit->enemyEdgeQuantity);
			edit->showPanel = panel;
			edit->tempActor->SetPanelInfo();
		}
	}
	else if (name == "basicturret")
	{
		if (edit->enemyEdgePolygon != NULL)
		{
			edit->tempActor = new BasicTurretParams(
				edit->enemyEdgePolygon, edit->enemyEdgeIndex, edit->enemyEdgeQuantity);
			edit->showPanel = panel;
			edit->tempActor->SetPanelInfo();
		}
	}
	else if (name == "foottrap")
	{
		if (edit->enemyEdgePolygon != NULL)
		{

			edit->tempActor = new FootTrapParams( edit->enemyEdgePolygon, edit->enemyEdgeIndex,
				edit->enemyEdgeQuantity);
			edit->showPanel = panel;
			edit->tempActor->SetPanelInfo();
		}
	}
	else if (name == "bosscrawler")
	{
		edit->showPanel = edit->enemySelectPanel;
		edit->trackingEnemy = NULL;
		ActorPtr bossCrawler(new BossCrawlerParams( edit->enemyEdgePolygon, edit->enemyEdgeIndex,
			edit->enemyEdgeQuantity));
		bossCrawler->group = edit->groups["--"];

		edit->CreateActor(bossCrawler);
	}
	else if (name == "airdasher")
	{
		edit->tempActor = new AirdasherParams( worldPos);
		edit->tempActor->SetPanelInfo();
		edit->showPanel = panel;
	}

	//w2
	else if (name == "bat")
	{
		edit->tempActor = new BatParams( worldPos);
		edit->tempActor->SetPanelInfo();
		//edit->tempActor->SetDefaultPanelInfo();

		edit->showPanel = panel;

		edit->patrolPath.clear();
		edit->patrolPath.push_back(worldPos);
	}
	else if (name == "poisonfrog")
	{
		//edit->groups["--"]->name
		if (edit->enemyEdgePolygon != NULL)
		{
			edit->tempActor = new PoisonFrogParams( edit->enemyEdgePolygon, edit->enemyEdgeIndex,
				edit->enemyEdgeQuantity);
			edit->showPanel = panel;
			edit->tempActor->SetPanelInfo();
			//edit->tempActor->SetDefaultPanelInfo();
			//edit->trackingEnemy = NULL;
		}
	}
	else if (name == "gravityfaller")
	{
		if (edit->enemyEdgePolygon != NULL)
		{
			edit->tempActor = new GravityFallerParams( edit->enemyEdgePolygon, edit->enemyEdgeIndex,
				edit->enemyEdgeQuantity);
			edit->showPanel = panel;
			edit->tempActor->SetPanelInfo();
		}
	}
	else if (name == "stagbeetle")
	{
		//edit->groups["--"]->name
		if (edit->enemyEdgePolygon != NULL)
		{
			edit->tempActor = new StagBeetleParams( edit->enemyEdgePolygon,
				edit->enemyEdgeIndex, edit->enemyEdgeQuantity);
			edit->showPanel = panel;
			edit->tempActor->SetPanelInfo();
			//edit->tempActor->SetDefaultPanelInfo();
			//edit->trackingEnemy = NULL;
		}
	}
	else if (name == "curveturret")
	{
		if (edit->enemyEdgePolygon != NULL)
		{
			//doesn't account for cancelling

			edit->tempActor = new CurveTurretParams( edit->enemyEdgePolygon, edit->enemyEdgeIndex,
				edit->enemyEdgeQuantity);

			edit->showPanel = panel;
			//edit->tempActor->SetDefaultPanelInfo();
			edit->tempActor->SetPanelInfo();
			//CurveTurretParams *ct = (CurveTurretParams*)edit->tempActor.get();
			//ct->SetPanelInfo();

			edit->patrolPath.clear();
			edit->patrolPath.push_back(worldPos);

			//edit->showPanel->textBoxes["name"]->text.setString( "test" );
			//edit->showPanel->textBoxes["group"]->text.setString( "not test" );
			//edit->showPanel->textBoxes["bulletspeed"]->text.setString( "10" );
			//edit->showPanel->textBoxes["waitframes"]->text.setString( "10" );
		}
	}
	else if (name == "bossbird")
	{
		edit->showPanel = edit->enemySelectPanel;
		edit->trackingEnemy = NULL;
		ActorPtr bossBird(new BossBirdParams( worldPos));
		bossBird->group = edit->groups["--"];

		edit->CreateActor(bossBird);
	}

	//w3
	else if (name == "pulser")
	{
		edit->tempActor = new PulserParams( worldPos);
		edit->tempActor->SetPanelInfo();
		//edit->tempActor->SetDefaultPanelInfo();

		edit->showPanel = panel;

		edit->patrolPath.clear();
		edit->patrolPath.push_back(worldPos);
	}
	else if (name == "cactus")
	{
		if (edit->enemyEdgePolygon != NULL)
		{
			edit->tempActor = new CactusParams( edit->enemyEdgePolygon,
				edit->enemyEdgeIndex, edit->enemyEdgeQuantity);
			edit->showPanel = panel;
			edit->tempActor->SetPanelInfo();
			//edit->tempActor->SetDefaultPanelInfo();
			//edit->trackingEnemy = NULL;
		}
	}
	else if (name == "badger")
	{
		if (edit->enemyEdgePolygon != NULL)
		{
			edit->tempActor = new BadgerParams( edit->enemyEdgePolygon,
				edit->enemyEdgeIndex, edit->enemyEdgeQuantity);
			edit->showPanel = panel;
			edit->tempActor->SetPanelInfo();
			//edit->tempActor->SetDefaultPanelInfo();
			//edit->trackingEnemy = NULL;
		}
	}
	else if (name == "owl")
	{
		edit->tempActor = new OwlParams( worldPos);
		edit->tempActor->SetPanelInfo();
		edit->showPanel = panel;
	}
	else if (name == "bosscoyote")
	{
		edit->showPanel = edit->enemySelectPanel;
		edit->trackingEnemy = NULL;
		ActorPtr bossCoyote(new BossCoyoteParams(
			worldPos));
		bossCoyote->group = edit->groups["--"];

		edit->CreateActor(bossCoyote);
	}

	//w4
	else if (name == "coral")
	{
		edit->tempActor = new CoralParams( worldPos);
		edit->tempActor->SetPanelInfo();
		edit->showPanel = panel;
	}
	else if (name == "cheetah")
	{
		if (edit->enemyEdgePolygon != NULL)
		{
			edit->tempActor = new CheetahParams( edit->enemyEdgePolygon,
				edit->enemyEdgeIndex, edit->enemyEdgeQuantity);
			edit->showPanel = panel;
			edit->tempActor->SetPanelInfo();
		}
	}
	else if (name == "turtle")
	{
		edit->tempActor = new TurtleParams( worldPos);
		edit->tempActor->SetPanelInfo();
		edit->showPanel = panel;
	}
	else if (name == "spider")
	{
		if (edit->enemyEdgePolygon != NULL)
		{
			edit->tempActor = new SpiderParams( edit->enemyEdgePolygon,
				edit->enemyEdgeIndex, edit->enemyEdgeQuantity);
			edit->showPanel = panel;
			edit->tempActor->SetPanelInfo();
		}
	}
	else if (name == "bosstiger")
	{
		edit->showPanel = edit->enemySelectPanel;
		edit->trackingEnemy = NULL;
		ActorPtr bossTiger(new BossTigerParams( worldPos));

		bossTiger->group = edit->groups["--"];

		edit->CreateActor(bossTiger);
		((BossTigerParams*)bossTiger.get())->CreateFormation();
	}

	//w5
	else if (name == "overgrowth")
	{
		if (edit->enemyEdgePolygon != NULL)
		{
			edit->tempActor = new OvergrowthParams( edit->enemyEdgePolygon,
				edit->enemyEdgeIndex, edit->enemyEdgeQuantity);
			edit->showPanel = panel;
			edit->tempActor->SetPanelInfo();
		}
	}
	else if (name == "ghost")
	{
		edit->tempActor = new GhostParams( worldPos);
		edit->tempActor->SetPanelInfo();
		edit->showPanel = panel;
	}
	else if (name == "shark")
	{
		edit->tempActor = new SharkParams( worldPos);
		edit->tempActor->SetPanelInfo();
		edit->showPanel = panel;
	}
	else if (name == "swarm")
	{
		edit->tempActor = new SwarmParams( worldPos);
		edit->tempActor->SetPanelInfo();
		edit->showPanel = panel;
	}
	else if (name == "bossgator")
	{
		edit->showPanel = edit->enemySelectPanel;
		edit->trackingEnemy = NULL;
		ActorPtr bossGator(new BossGatorParams( worldPos));
		bossGator->group = edit->groups["--"];

		edit->CreateActor(bossGator);
	}

	//w6
	else if (name == "specter")
	{
		edit->tempActor = new SpecterParams( worldPos);
		edit->tempActor->SetPanelInfo();
		edit->showPanel = panel;
	}
	else if (name == "gorilla")
	{
		edit->tempActor = new GorillaParams( worldPos);
		edit->tempActor->SetPanelInfo();
		edit->showPanel = panel;
		//if( edit->enemyEdgePolygon != NULL )
		//{
		//	/*edit->tempActor = new GorillaParams( edit->enemyEdgePolygon, 
		//		edit->enemyEdgeIndex, edit->enemyEdgeQuantity );
		//	edit->showPanel = panel;
		//	edit->tempActor->SetPanelInfo();*/
		//}
	}
	else if (name == "narwhal")
	{
		edit->tempActor = new NarwhalParams( worldPos);
		edit->tempActor->SetPanelInfo();

		edit->showPanel = panel;

		edit->patrolPath.clear();
		edit->patrolPath.push_back(worldPos);
	}
	else if (name == "copycat")
	{
		edit->tempActor = new CopycatParams( worldPos);
		edit->tempActor->SetPanelInfo();
		edit->showPanel = panel;
	}
	else if (name == "bossskeleton")
	{
		edit->showPanel = edit->enemySelectPanel;
		edit->trackingEnemy = NULL;
		ActorPtr bossSkeleton(new BossSkeletonParams( worldPos));
		bossSkeleton->group = edit->groups["--"];

		edit->CreateActor(bossSkeleton);
	}


	//w7
	else if (name == "nexus")
	{
		if (edit->enemyEdgePolygon != NULL)
		{
			edit->showPanel = edit->enemySelectPanel;
			edit->trackingEnemy = NULL;
			ActorPtr nexus(new NexusParams( edit->enemyEdgePolygon, edit->enemyEdgeIndex,
				edit->enemyEdgeQuantity));
			nexus->group = edit->groups["--"];

			edit->CreateActor(nexus);
		}
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

void ActorGroup::DrawPreview( sf::RenderTarget *target )
{
	//CircleShape cs;
	//cs.setFillColor( Color::Red );

	for( list<ActorPtr>::iterator it = actors.begin(); it != actors.end(); ++it )
	{

		//(*it)->DrawPreview( target );
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

bool CompareDecorInfo(EditSession::DecorInfo &di0, EditSession::DecorInfo &di1)
{
	return di0.layer < di1.layer;
}

bool EditSession::DecorInfo::ContainsPoint(sf::Vector2f test)
{
	sf::Transform trans = spr.getTransform();
	FloatRect fr = spr.getLocalBounds();
	Vector2f points[4];
	points[0] = trans * Vector2f(fr.left, fr.top);
	points[1] = trans * Vector2f(fr.left + fr.width, fr.top);
	points[2] = trans * Vector2f(fr.left + fr.width, fr.top + fr.height);
	points[3] = trans * Vector2f(fr.left, fr.top + fr.height);

	bool result = QuadContainsPoint(V2d( points[0] ), 
		V2d(points[1]), 
		V2d(points[2]), 
		V2d(points[3]), V2d(test.x, test.y));

	//cout << "result: " << result << endl;
	return result;
}

bool EditSession::DecorInfo::Intersects(sf::IntRect rect)
{
	FloatRect fr(rect);
	return fr.intersects(spr.getGlobalBounds());
}

void EditSession::DecorInfo::Move(boost::shared_ptr<ISelectable> me,
	sf::Vector2i delta) 
{
	spr.setPosition(spr.getPosition().x + delta.x, spr.getPosition().y + delta.y);
}

void EditSession::DecorInfo::BrushDraw(sf::RenderTarget *target,
	bool valid) 
{
	target->draw(spr);
}

void EditSession::DecorInfo::Draw(sf::RenderTarget *target)
{
	target->draw(spr);
	if (selected)
	{
		sf::RectangleShape rs;
		rs.setFillColor(Color::Transparent);
		rs.setOutlineColor(Color::Green);
		rs.setOutlineThickness(3 * EditSession::zoomMultiple);
		rs.setPosition(spr.getGlobalBounds().left, spr.getGlobalBounds().top);
		rs.setSize(Vector2f(spr.getGlobalBounds().width, spr.getGlobalBounds().height));
		target->draw(rs);
		//cout << "selected draw" << endl;
	}
}

void EditSession::DecorInfo::Deactivate(EditSession *edit,
	boost::shared_ptr<ISelectable> select)
{
	cout << "deactivating decor" << endl;
	DecorPtr dec = boost::dynamic_pointer_cast<DecorInfo>(select);
	
	myList->remove(dec);
}

void EditSession::DecorInfo::Activate(EditSession *edit,
	boost::shared_ptr<ISelectable> select)
{
	cout << "adding image" << endl;
	DecorPtr dec = boost::dynamic_pointer_cast<DecorInfo>(select);

	myList->push_back(dec);
}

void EditSession::DecorInfo::SetSelected(bool select)
{
	selected = select;
}

void EditSession::DecorInfo::WriteFile(std::ofstream &of )
{
	of << decorName << endl;
	of << layer << endl;
	of << spr.getPosition().x << " " << spr.getPosition().y << endl;
	of << spr.getRotation() << endl;
	of << spr.getScale().x << " " << spr.getScale().y << endl;
	of << tile << endl;
}
