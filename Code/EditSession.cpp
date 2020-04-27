//edit mode

#include "GUI.h"
#include "EditSession.h"
#include <fstream>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "Physics.h"
#include "Action.h"
#include <set>
#include "MainMenu.h"
#include "Background.h"
#include "Enemy_Shard.h"
#include "ActorParams.h"
#include "EditorRail.h"
#include "EditorGraph.h"
#include "Actor.h"
#include "ControlProfile.h"
#include "SaveFile.h"
#include "Wire.h"

#include "clipper.hpp"

#include "TransformTools.h"
//using namespace ClipperLib;

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
const double EditSession::SLIVER_LIMIT = PI / 10.0;
double EditSession::zoomMultiple = 1;
EditSession * EditSession::currSession = NULL;

#define TIMESTEP (1.0 / 60.0)

void EditSession::SetTrackingEnemy(ActorType *type, int level)
{
	if (trackingEnemyParams == NULL)
	{
		ClearSelectedBrush();
		//cout << "copy of level : " << level << endl;
		trackingEnemyParams = type->defaultParamsVec[level-1]->Copy();
		trackingEnemyParams->group = groups["--"];
		//GetPolygon((0);

		//trackingEnemyParams->AnchorToGround();
		trackingEnemyParams->myEnemy = trackingEnemyParams->GenerateEnemy();
		grabbedActor = trackingEnemyParams;
		SelectObject(grabbedActor);

		trackingEnemyParams->myEnemy->SetActionEditLoop();
		trackingEnemyParams->MoveTo(Vector2i(worldPos));
		//extraDelta = Vector2i(worldPos) - Vector2i(grabCenter);
		grabbedActor->myEnemy->UpdateFromEditParams(0);
		

		editMouseGrabPos = Vector2i(worldPos.x, worldPos.y);
		pointGrabPos = Vector2i(worldPos.x, worldPos.y);
		editMouseOrigPos = editMouseGrabPos;


		editMouseDownMove = true;
		editStartMove = false;
		editMouseDownBox = false;

		createEnemyModeUI->SetShown(false);
	}
	else
	{
		int x = 5;
	}
}

void EditSession::UpdateDecorSprites()
{
	for (auto it = polygons.begin(); it != polygons.end(); ++it)
	{
		(*it)->UpdateDecorSprites();
		(*it)->UpdateTouchGrass();
	}
}

V2d EditSession::GetPlayerSpawnPos()
{
	if (HoldingControl())
	{
		return worldPos;
	}
	else
	{
		return player->GetPosition();
	}
}

void EditSession::ClearSelectedBrush()
{
	selectedBrush->SetSelected(false);
	selectedBrush->Clear();
	grabbedActor = NULL;
	grabbedObject = NULL;
	grabbedPoint = NULL;
}

void EditSession::SelectObject(SelectPtr sel)
{
	sel->SetSelected(true);
	selectedBrush->AddObject(sel);
}

void EditSession::DeselectObject(SelectPtr sel)
{
	sel->SetSelected(false);
	selectedBrush->RemoveObject(sel);
}

void EditSession::TestPlayerModeUpdate()
{
	double newTime = gameClock.getElapsedTime().asSeconds();
	double frameTime = newTime - currentTime;

	if (frameTime > 0.25)
	{
		frameTime = 0.25;
	}
	currentTime = newTime;

	accumulator += frameTime;

	while (accumulator >= TIMESTEP)
	{
		for (int i = 0; i < 4; ++i)
		{
			GetPrevInput(i) = GetCurrInput(i);
			GetPrevInputUnfiltered(i) = GetCurrInputUnfiltered(i);
		}

		Actor *pTemp = NULL;
		for (int i = 0; i < 4; ++i)
		{
			pTemp = GetPlayer(i);
			if (pTemp != NULL)
			{
				pTemp->prevInput = GetCurrInput(i);
			}
		}

		UpdateControllers();

		UpdateAllPlayersInput();
		
		UpdatePrePhysics();
		UpdatePhysics();
		UpdatePostPhysics();

		UpdateDecorSprites();
		UpdateDecorLayers();
		
		//UpdatePlayerWireQuads();

		accumulator -= TIMESTEP;
		totalGameFrames++;
	}
}

void EditSession::TestPlayerMode()
{
	if (mode == TEST_PLAYER)
	{
		//GetPlayer(0)->Respawn();
		Actor *p;
		for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			p = GetPlayer(i);
			if (p != NULL)
			{
				p->Respawn();
				p->velocity = worldPos - oldWorldPosTest;
			}
			
		}

		auto testPolys = GetCorrectPolygonList(0);
		for (auto it = testPolys.begin(); it != testPolys.end(); ++it)
		{
			(*it)->ResetTouchGrass();
		}

		for (auto it = groups.begin(); it != groups.end(); ++it)
		{
			for (auto enit = (*it).second->actors.begin(); enit != (*it).second->actors.end(); ++enit)
			{
				if ((*enit)->myEnemy != NULL)
				{
					RemoveEnemy((*enit)->myEnemy);
				}
			}
		}

		for (auto it = groups.begin(); it != groups.end(); ++it)
		{
			for (auto enit = (*it).second->actors.begin(); enit != (*it).second->actors.end(); ++enit)
			{
				if ((*enit)->myEnemy != NULL)
				{
					(*enit)->myEnemy->Reset();
					AddEnemy((*enit)->myEnemy);
				}
			}
		}
		return;
	}

	
	ClearSelectedPoints();
	ClearSelectedBrush();

	currentTime = 0;
	accumulator = TIMESTEP + .1;
	SetMode(TEST_PLAYER);
	totalGameFrames = 0;
	gameClock.restart();

	if (terrainTree != NULL)
	{
		terrainTree->Clear();
		specialTerrainTree->Clear();
		borderTree->Clear();

		Actor *p;
		for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			p = GetPlayer(i);
			if (p != NULL)
				p->Respawn();
		}

		for (auto it = groups.begin(); it != groups.end(); ++it)
		{
			for (auto enit = (*it).second->actors.begin(); enit != (*it).second->actors.end(); ++enit)
			{
				if ((*enit)->myEnemy != NULL)
				{
					(*enit)->myEnemy->Reset();
					RemoveEnemy((*enit)->myEnemy);
				}
			}
		}

		for (auto it = groups.begin(); it != groups.end(); ++it)
		{
			for (auto enit = (*it).second->actors.begin(); enit != (*it).second->actors.end(); ++enit)
			{
				if ((*enit)->myEnemy != NULL)
				{
					AddEnemy((*enit)->myEnemy);
				}
			}
		}

		/*for (auto it = allCurrEnemies.begin(); it != allCurrEnemies.end(); ++it)
		{
			RemoveEnemy((*it));
			(*it)->Reset();
			AddEnemy((*it));
		}*/
		
		//reset enemies
	}
	else
	{
		terrainTree = new QuadTree(1000000, 1000000);
		specialTerrainTree = new QuadTree(1000000, 1000000);
		borderTree = new QuadTree(1000000, 1000000);

		railEdgeTree = new QuadTree(1000000, 1000000);
		barrierTree = new QuadTree(1000000, 1000000);

		staticItemTree = new QuadTree(1000000, 1000000);

		Actor *p;
		for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			p = GetPlayer(i);
			if (p != NULL)
				p->SetToOriginalPos();
		}

		for (auto it = groups.begin(); it != groups.end(); ++it)
		{
			for (auto enit = (*it).second->actors.begin(); enit != (*it).second->actors.end(); ++enit)
			{
				if ((*enit)->myEnemy != NULL)
				{
					AddEnemy((*enit)->myEnemy);
				}
			}
		}
	}

	auto testPolys = GetCorrectPolygonList(0);
	for (auto it = testPolys.begin(); it != testPolys.end(); ++it)
	{
		borderTree->Insert((*it));
		(*it)->AddEdgesToQuadTree(terrainTree);
	}


	//terrainBGTree = new QuadTree(1000000, 1000000);
	////soon make these the actual size of the bordered level
	//terrainTree = new QuadTree(1000000, 1000000);

	//barrierTree = new QuadTree(1000000, 1000000);

	//specialTerrainTree = new QuadTree(1000000, 1000000);

	//inverseEdgeTree = new QuadTree(1000000, 1000000);

	//staticItemTree = new QuadTree(1000000, 1000000);
	//railDrawTree = new QuadTree(1000000, 1000000);
	//railEdgeTree = new QuadTree(1000000, 1000000);

	//enemyTree = new QuadTree(1000000, 1000000);

	//borderTree = new QuadTree(1000000, 1000000);

	//grassTree = new QuadTree(1000000, 1000000);

	//gateTree = new QuadTree(1000000, 1000000);

	//itemTree = new QuadTree(1000000, 1000000);

	//envPlantTree = new QuadTree(1000000, 1000000);

	//specterTree = new QuadTree(1000000, 1000000);

	//activeItemTree = new QuadTree(1000000, 1000000);

	//activeEnemyItemTree = new QuadTree(1000000, 1000000);

	//airTriggerTree = new QuadTree(1000000, 1000000);
}

void EditSession::EndTestMode()
{
	SetMode(EDIT);
}

void EditSession::UpdatePrePhysics()
{
	Actor *player = GetPlayer(0);
	if (player->action == Actor::INTRO || player->action == Actor::SPAWNWAIT)
	{
		return;
	}

	Actor *p;
	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
			p->UpdatePrePhysics();
	}

	Enemy *current = activeEnemyList;
	while (current != NULL)
	{
		current->UpdatePrePhysics();
		current = current->next;
	}
}

void EditSession::ProcessDecorFromFile(const std::string &name,
	int tile)
{
	decorTileIndexMap[name].push_back(tile);
}

void EditSession::UpdatePhysics()
{
	Actor *p = NULL;
	Actor *player = GetPlayer(0);
	if (player->action == Actor::INTRO || player->action == Actor::SPAWNWAIT)
	{
		return;
	}

	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
			p->physicsOver = false;
	}


	for (substep = 0; substep < NUM_MAX_STEPS; ++substep)
	{
		for (int i = 0; i < 4; ++i)
		{
			p = GetPlayer(i);
			if (p != NULL)
			{
				if (substep == 0 || p->highAccuracyHitboxes)
					p->UpdatePhysics();
			}
		}

		Enemy *current = activeEnemyList;
		while (current != NULL)
		{
			current->UpdatePhysics(substep);
			current = current->next;
		}
	}
}

void EditSession::UpdatePostPhysics()
{
	Actor *player = GetPlayer(0);
	if (player->action == Actor::INTRO || player->action == Actor::SPAWNWAIT)
	{
		return;
	}

	Actor *p;
	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
			p->UpdatePostPhysics();
	}

	UpdatePlayerWireQuads();

	int keyLength = 30;//16 * 5;
	keyFrame = totalGameFrames % keyLength;

	Enemy *current = activeEnemyList;
	while (current != NULL)
	{
		Enemy *temp = current->next; //need this in case enemy is removed during its update

		current->UpdatePostPhysics();

		if (current->hasMonitor)
		{
			float halftot = keyLength / 2;
			float fac;
			if (keyFrame < keyLength / 2)
			{
				fac = keyFrame / (halftot - 1);
			}
			else
			{
				fac = 1.f - (keyFrame - halftot) / (halftot - 1);
			}
			//cout << "fac: " << fac << endl;
			current->keyShader.setUniform("prop", fac);
		}

		current = temp;
	}
}




EditSession *EditSession::GetSession()
{
	return currSession;
}

EditSession::EditSession( MainMenu *p_mainMenu, const boost::filesystem::path &p_filePath)
	:Session( Session::SESS_EDIT, p_filePath ), fullBounds( sf::Quads, 16 ), mainMenu( p_mainMenu ), arial( p_mainMenu->arial )
{
	createEnemyModeUI = NULL;
	enemyEdgePolygon = NULL;
	moveAction = NULL;
	AllocatePolyShaders(TERRAIN_WORLDS * MAX_TERRAINTEX_PER_WORLD);

	transformTools = new TransformTools();

	initialViewSet = false;

	SaveFile *currFile = mainMenu->GetCurrentProgress();
	if (currFile != NULL)
	{
		bool set = mainMenu->SetCurrProfileByName(currFile->controlProfileName);
		if (!set)
		{
			//error. profile does not exist
			currFile->controlProfileName = "KIN Default";
			currFile->Save();
		}
	}
	//update this later

	ControlProfile *currProfile = mainMenu->GetCurrSelectedProfile();
	for (int i = 0; i < 4; ++i)
	{
		
		GameController &con = GetController(i);
		if (con.IsConnected())
		{
			currProfile->tempCType = con.GetCType();
			con.SetFilter(currProfile->GetCurrFilter());
		}
	}

	inversePolygon = NULL;
	currSession = this;

	minZoom = .25 / 16.0;//.25;
	maxZoom = 65536;

	copiedBrush = NULL;
	newMapHeader.ver1 = 1;
	newMapHeader.ver2 = 5;
	newMapHeader.description = "no description";
	newMapHeader.collectionName = "default";
	newMapHeader.gameMode = MapHeader::T_STANDARD;//"default";
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
	grabbedActor = NULL;
	grabbedPoint = NULL;
	zoomMultiple = 1;
	editMouseDownBox = false;
	editMouseDownMove = false;
	editMoveThresh = 5;
	editStartMove = false;
	//adding 5 for random distance buffer

	preScreenTex = mainMenu->preScreenTexture;
	
	//minAngle = .99;
	showPoints = false;
	messagePopup = NULL;
	errorPopup = NULL;
	confirm = NULL;
	progressBrush = new Brush();
	selectedBrush = new Brush();
	mapStartBrush = new Brush();
	moveActive = false;

	ActorGroup *playerGroup = new ActorGroup( "player" );
	groups["player"] = playerGroup;
	
	ParamsInfo playerPI("player", NULL, NULL,
		Vector2i(), Vector2i(22, 42), false, false, false, false, 1, 0,
		GetTileset("Kin/jump_64x64.png", 64, 64));

	playerType = new ActorType(playerPI);
	types["player"] = playerType;

	player = new PlayerParams( playerType, Vector2i( 0, 0 ) ) ;
	groups["player"]->actors.push_back( player );

	mapStartBrush->AddObject(player);
	

	grassSize = 128;//64;
	grassSpacing = -60;//-40;//-20;//-10;

	//extras

	//monitor,level, path, loop, 

	AddGeneralEnemies();
	AddW1Enemies();
	AddW2Enemies();
	AddW3Enemies();
	AddW4Enemies();
	AddW5Enemies();
	AddW6Enemies();
}



PolyPtr EditSession::GetPolygon(int index )
{
	PolyPtr terrain = NULL;
	if (index == -1)
	{
		terrain = inversePolygon;
	}
	else
	{
		int testIndex = 0;

		auto it = polygons.begin();
		if (inversePolygon != NULL)
			++it;

		for (; it != polygons.end(); ++it)
		{
			if (testIndex == index)
			{
				terrain = (*it);
				break;
			}
			testIndex++;
		}
	}

	/*if (edgeIndex == terrain->GetNumPoints() - 1)
		edgeIndex = 0;
	else
		edgeIndex++;*/

	if (terrain == NULL)
		assert(0 && "failure terrain indexing goal");

	return terrain;
}

RailPtr EditSession::GetRail(int index)
{
	TerrainRail* rail = NULL;

	int pSize = polygons.size();
	if (inversePolygon != NULL)
	{
		pSize--;
	}

	int realIndex = index - pSize;

	int testIndex = 0;
	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		if (testIndex == realIndex)
		{
			rail = (*it);
			break;
		}
		++testIndex;
	}

	if (rail == NULL)
		assert(0 && "failure rail indexing");

	return rail;
}

EditSession::~EditSession()
{
	delete transformTools;

	delete graph;

	delete polygonInProgress;
	delete railInProgress;

	/*for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		delete (*it);
	}*/

	/*for (int i = 0; i < 2; ++i)
	{
		auto & polyList = GetCorrectPolygonList(i);
		for (auto it = polyList.begin(); it != polyList.end(); ++it)
		{
			delete (*it);
		}
	}*/


	for (auto it = doneActionStack.begin(); it != doneActionStack.end(); ++it)
	{
		delete (*it);
	}

	for (auto it = undoneActionStack.begin(); it != undoneActionStack.end(); ++it)
	{
		delete (*it);
	}


	delete progressBrush;
	delete selectedBrush;
	if (copiedBrush != NULL)
	{
		copiedBrush->Destroy();
		delete copiedBrush;
	}
		

	mapStartBrush->Destroy();

	delete mapStartBrush;

	for (auto it = allPopups.begin(); it != allPopups.end(); ++it)
	{
		delete (*it);
	}


	

	//delete groups, but not actors
	

	delete[] decorTileIndexes;

	currSession = NULL;
}

void EditSession::SnapPointToGraph(Vector2f &p, int gridSize )
{
	V2d pCopy(p);
	SnapPointToGraph(pCopy, gridSize);
	p = Vector2f(pCopy);
}

void EditSession::SnapPointToGraph(V2d &p, int gridSize)
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

	p = V2d(adjX, adjY);
}

bool EditSession::IsKeyPressed(int k)
{
	return mainMenu->IsKeyPressed(k);
}

bool EditSession::IsMousePressed(int m)
{
	return mainMenu->IsMousePressed(m);
}

bool EditSession::IsDrawMode(Emode em)
{
	return ((mode == em) || (menuDownStored == em && mode == SELECT_MODE)
		|| (stored == em && mode == PAUSED));
}

void EditSession::Draw()
{
	preScreenTex->clear();
	preScreenTex->setView(view);

	background->Draw(preScreenTex);

	preScreenTex->draw(border, 8, sf::Lines);

	DrawDecorBehind();

	DrawGates();

	DrawPolygons();

	DrawRails();
	//DrawPolygonInProgress();

	DrawDecorBetween();
	DrawActors();
	

	if (IsDrawMode( PASTE ) )
	{
		copiedBrush->Draw(preScreenTex);
	}

	if (IsDrawMode( TEST_PLAYER ))
	{
		Enemy *current = activeEnemyList;
		while (current != NULL)
		{
			current->Draw(preScreenTex);
			current = current->next;
		}

		DrawPlayerWires(preScreenTex);

		Actor *p = NULL;
		for (int i = 0; i < 4; ++i)
		{
			p = GetPlayer(i);
			if (p != NULL)
			{
				p->Draw(preScreenTex);
			}
		}
	}

	if (IsDrawMode(CREATE_ENEMY))
	{
		createEnemyModeUI->Draw(preScreenTex);

		if( grabbedActor != NULL )
			grabbedActor->Draw(preScreenTex);
	}

	if (IsDrawMode(TRANSFORM))
	{
		transformTools->Draw(preScreenTex);
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

bool EditSession::ReadDecor(std::ifstream &is)
{
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


		
	}

	return true;
}

void EditSession::ProcessDecorSpr( const std::string &dName, sf::Sprite &dSpr,
	int dLayer, Tileset *d_ts, int dTile)
{
	DecorPtr dec = new EditorDecorInfo(dSpr, dLayer, dName, dTile);
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

	mapStartBrush->AddObject(dec);
}

void EditSession::ProcessTerrain(PolyPtr poly)
{
	mapStartBrush->AddObject(poly);
	if (poly->inverse)
	{
		inversePolygon = poly;
	}

	GetCorrectPolygonList(poly).push_back(poly);

	poly->Finalize();

	if (enemyEdgePolygon == NULL)
	{
		//this is for creating default enemies. im immediately going to detach them from terrain
		enemyEdgePolygon = poly;
		enemyEdgeIndex = 0;
		enemyEdgeQuantity = 0;
	}
	
}

bool EditSession::ReadBGTerrain(std::ifstream &is)
{
	int bgPlatformNum0;
	is >> bgPlatformNum0;
	for (int i = 0; i < bgPlatformNum0; ++i)
	{
		PolyPtr poly(new TerrainPolygon());
		polygons.push_back(poly);

		mapStartBrush->AddObject(poly);

		int matWorld;
		int matVariation;
		is >> matWorld;
		is >> matVariation;

		poly->terrainWorldType = (TerrainPolygon::TerrainWorldType)matWorld;
		poly->terrainVariation = matVariation;

		int polyPoints;
		is >> polyPoints;

		for (int j = 0; j < polyPoints; ++j)
		{
			int x, y, special;
			is >> x;
			is >> y;
			poly->AddPoint(Vector2i(x, y), false);
		}

		poly->Finalize();
		poly->SetLayer(1);
		//no grass for now
	}
	return true;
}

void EditSession::ProcessBGTerrain(PolyPtr poly)
{
	polygons.push_back(poly);
	mapStartBrush->AddObject(poly);
	
}

bool EditSession::ReadRails(std::ifstream &is)
{
	int numRails;
	is >> numRails;
	for (int i = 0; i < numRails; ++i)
	{
		RailPtr rail(new TerrainRail());
		rails.push_back(rail);

		rail->Load(is);

		mapStartBrush->AddObject(rail);
	}
	return true;
}

void EditSession::ProcessRail(RailPtr rail)
{
	rails.push_back(rail);
	mapStartBrush->AddObject(rail);
}

bool EditSession::ReadSpecialTerrain(std::ifstream &is)
{
	int specialPolyNum;
	is >> specialPolyNum;

	for (int i = 0; i < specialPolyNum; ++i)
	{
		PolyPtr poly(new TerrainPolygon());

		mapStartBrush->AddObject(poly);

		int matWorld;
		int matVariation;
		is >> matWorld;
		is >> matVariation;

		poly->SetMaterialType(matWorld, matVariation);
		int polyPoints;
		is >> polyPoints;

		GetCorrectPolygonList(poly).push_back(poly);

		for (int j = 0; j < polyPoints; ++j)
		{
			int x, y, special;
			is >> x;
			is >> y;
			poly->AddPoint(Vector2i(x, y), false);
		}

		poly->Finalize();
	}

	return true;
}

void EditSession::ProcessSpecialTerrain(PolyPtr poly)
{
	GetCorrectPolygonList(poly).push_back(poly);
	mapStartBrush->AddObject(poly);
}

void EditSession::ProcessActor( ActorPtr a)
{
	mapStartBrush->AddObject(a);
	AddRecentEnemy(a);
}

void EditSession::AddRecentEnemy(ActorPtr a)
{
	bool found = false;

	for (auto it = recentEnemies.begin(); it != recentEnemies.end(); ++it)
	{
		if ((*it).first == a->type && (*it).second == a->GetLevel())
		{
			found = true;
			recentEnemies.erase(it);
			break;
		}
	}

	if (!found && recentEnemies.size() == MAX_RECENT_ENEMIES )
	{
		recentEnemies.pop_front();
	}

	recentEnemies.push_back(make_pair(a->type, a->GetLevel()));

	if (createEnemyModeUI != NULL )
	{
		createEnemyModeUI->UpdateHotbarTypes();
	}
	
}

void EditSession::ProcessGate(int gType,
	int poly0Index, int vertexIndex0, int poly1Index,
	int vertexIndex1, int shardWorld, int shardIndex)
{
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

	GateInfoPtr gi(new GateInfo);
	gi->poly0 = terrain0;
	gi->poly1 = terrain1;
	gi->vertexIndex0 = vertexIndex0;
	gi->vertexIndex1 = vertexIndex1;
	gi->type = (Gate::GateType)gType;
	gi->edit = this;

	if (gType == Gate::SHARD)
	{
		gi->SetShard(shardWorld, shardIndex);
	}

	TerrainPoint *giPoint0 = gi->poly0->GetPoint(vertexIndex0);
	giPoint0->gate = gi;
	gi->point0 = giPoint0;

	TerrainPoint *giPoint1 = gi->poly1->GetPoint(vertexIndex1);
	giPoint1->gate = gi;
	gi->point1 = giPoint1;

	gi->UpdateLine();
	gates.push_back(gi);

	mapStartBrush->AddObject(gi);
}

void EditSession::ProcessPlayerStartPos()
{
	player->SetPosition(playerOrigPos);
	player->image.setPosition(player->GetFloatPos());
	player->SetBoundingQuad();
}

void EditSession::ProcessHeader()
{
	newMapHeader = *mapHeader;

	envName = mapHeader->envName;

	//newMapHeader.numVertices = mh->numVertices;

	envWorldType = mapHeader->envWorldType;

	leftBound = mapHeader->leftBounds;
	topBound = mapHeader->topBounds;
	boundWidth = mapHeader->boundsWidth;
	boundHeight = mapHeader->boundsHeight;

	drainSeconds = mapHeader->drainSeconds;

	background = Background::SetupFullBG(envName, this);
	background->Hide();

	bossType = mapHeader->bossFightType;

	UpdateFullBounds();
}

void EditSession::WriteMapHeader(ofstream &of)
{
	newMapHeader.leftBounds = leftBound;
	newMapHeader.topBounds = topBound;
	newMapHeader.boundsWidth = boundWidth;
	newMapHeader.boundsHeight = boundHeight;
	newMapHeader.bossFightType = bossType;

	newMapHeader.shardNameList.clear();
	ShardParams *sp = NULL;
	int numShards = 0;
	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		std::list<ActorPtr> &aList = (*it).second->actors;
		for (auto ait = aList.begin(); ait != aList.end(); ++ait)
		{
			if ((*ait)->type->info.name == "shard")
			{
				numShards++;
				sp = (ShardParams*)(*ait);
				newMapHeader.shardNameList.push_back(sp->shardStr);
			}
		}
	}

	newMapHeader.numShards = numShards;

	newMapHeader.drainSeconds = drainSeconds;

	newMapHeader.envName = envName;

	newMapHeader.envWorldType = envWorldType;

	newMapHeader.Save(of);
}

void EditSession::WriteDecor(ofstream &of)
{
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
}

void EditSession::WriteInversePoly(std::ofstream &of)
{
	if (inversePolygon != NULL)
	{
		of << "borderpoly" << endl;
		inversePolygon->writeIndex = -1;//writeIndex;

		inversePolygon->WriteFile(of);
	}
	else
	{
		of << "no_borderpoly" << endl;
	}
}

void EditSession::WriteSpecialPolygons(std::ofstream &of)
{
	int numSpecialPolys = waterPolygons.size();
	of << numSpecialPolys << endl;

	for (auto it = waterPolygons.begin(); it != waterPolygons.end(); ++it)
	{
		(*it)->WriteFile(of);
	}
}

void EditSession::WritePolygons(std::ofstream &of, int bgPlatCount0)
{
	int writeIndex = 0;

	cout << "writing to file with : " << polygons.size() << " polygons" << endl;
	for (list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it)
	{
		if ((*it)->inverse) continue;

		//cout << "layerrr: " << (*it)->layer << ", pathsize: " << (*it)->path.size() << endl;
		if ((*it)->layer == 0 && (*it)->path.size() < 2)
		{
			cout << "writing polygon of write index: " << writeIndex << endl;
			(*it)->writeIndex = writeIndex;
			++writeIndex;

			(*it)->WriteFile(of);
		}
	}

	tempWriteIndex = writeIndex;


	WriteSpecialPolygons(of);
	//of << "0" << endl; //writing the number of moving platforms. remove this when possible


	//write moving platorms
	/*writeIndex = 0;
	for (list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it)
	{
		if ((*it)->layer == 0 && (*it)->path.size() >= 2)
		{
			(*it)->writeIndex = writeIndex;
			++writeIndex;

			of << (*it)->terrainWorldType << " "
				<< (*it)->terrainVariation << endl;

			of << (*it)->numPoints << endl;

			for (TerrainPoint *pcurr = (*it)->pointStart; pcurr != NULL; pcurr = pcurr->next)
			{
				of << pcurr->pos.x << " " << pcurr->pos.y << endl;
			}


			of << (*it)->path.size() - 1 << endl;

			list<Vector2i>::iterator pathit = (*it)->path.begin();
			++pathit;

			for (; pathit != (*it)->path.end(); ++pathit)
			{
				of << (*pathit).x << " " << (*pathit).y << endl;
			}
		}
	}*/

	of << bgPlatCount0 << endl;

	writeIndex = 0;
	for (list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it)
	{
		if ((*it)->inverse) continue;

		if ((*it)->layer == 1)// && (*it)->path.size() < 2 )
		{
			//writeindex doesnt matter much for these for now
			(*it)->writeIndex = writeIndex;
			++writeIndex;

			of << (*it)->terrainWorldType << " "
				<< (*it)->terrainVariation << endl;

			int numP = (*it)->GetNumPoints();

			of << numP << endl;

			TerrainPoint *curr;
			for (int i = 0; i < numP; ++i)
			{
				curr = (*it)->GetPoint(i);
				of << curr->pos.x << " " << curr->pos.y << endl;
			}
			//WriteGrass( (*it), of );
		}
	}
}

void EditSession::WriteActors(ofstream &of)
{
	//minus 1 because of the player group
	of << groups.size() - 1 << endl;
	//write the stuff for goals and remove them from the enemy stuff

	for (map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		ActorGroup *ag = (*it).second;
		if (ag->name == "player")
			continue;

		//cout << "group size: " << ag->actors.size() << endl;
		of << ag->name << " " << ag->actors.size() << endl;
		for (list<ActorPtr>::iterator it = ag->actors.begin(); it != ag->actors.end(); ++it)
		{
			if ((*it)->type == types["poi"])
			{
				(*it)->WriteFile(of);
			}
		}
	}

	for (map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		ActorGroup *ag = (*it).second;
		if (ag->name == "player")
			continue;

		//cout << "group size: " << ag->actors.size() << endl;
		//of << ag->name << " " << ag->actors.size() << endl;
		for (list<ActorPtr>::iterator it = ag->actors.begin(); it != ag->actors.end(); ++it)
		{
			if ((*it)->type != types["poi"])
			{
				(*it)->WriteFile(of);
			}
		}
		//(*it).second->WriteFile( of );

	}
}

void EditSession::WriteGates(ofstream &of)
{
	of << gates.size() << endl;
	for (list<GateInfoPtr>::iterator it = gates.begin(); it != gates.end(); ++it)
	{
		(*it)->WriteFile(of);
	}
}

void EditSession::WriteRails(ofstream &of)
{
	int writeIndex = tempWriteIndex;

	of << rails.size() << endl;

	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		(*it)->writeIndex = writeIndex;
		++writeIndex;

		(*it)->WriteFile(of);
		
	}
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

	/*if( !hasGoal )
	{
		MessagePop( "Map not saved because no goal is in place. \nPlease add it from the CREATE ENEMIES mode." );
		cout << "you need to place a goal in the map. file not written to!. add a popup to this alert later"
			<< endl;
		return;
	}*/

	ofstream of;
	of.open(fileName);

	WriteMapHeader(of);

	int pointCount = 0;
	int bgPlatCount0 = 0;

	for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		if ((*it)->layer == 0)
		{
			pointCount += (*it)->GetNumPoints();
		}
		else if ((*it)->layer == 1)
		{
			bgPlatCount0++;
		}
	}
	of << pointCount << endl;

	WriteDecor(of);

	Vector2i playerIntPos(player->GetIntPos());
	of << playerIntPos.x << " " << playerIntPos.y << endl;

	WriteInversePoly(of);

	WritePolygons(of, bgPlatCount0);

	WriteRails(of);
	//going to use this for number of rails
	//of << "0" << endl; //writing the number of static lights for consistency. Remove this when possible.

	WriteActors(of);

	WriteGates(of);
	

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

	int polyNumP = poly->GetNumPoints();
	TerrainPoint *curr, *next;
	for (int i = 0; i < polyNumP; ++i)
	{
		curr = poly->GetPoint(i);
		next = poly->GetNextPoint(i);

		V2d v0(curr->pos.x, curr->pos.y);
		V2d v1(next->pos.x, next->pos.y);

		bool rem;
		int num = poly->GetNumGrass(i, rem);//floor( remainder ) + 1;

		grassListList.push_back(list<GrassSeg>());

		list<GrassSeg> &grassList = grassListList.back();

		GrassSeg *gPtr = NULL;
		bool hasGrass = false;
		for (int j = 0; j < num; ++j)
		{
			//V2d pos = v0 + (v1 - v0) * ((double)(j )/ num);

			if (grassVa[i * 4].color.a == 255 || grassVa[i * 4].color.a == 254)
			{
				hasGrass = true;
				if (gPtr == NULL)//|| (j == num - 1 && rem ))
				{
					grassList.push_back(GrassSeg(edgeIndex, j, 0));
					gPtr = &grassList.back();
				}
				else
				{
					grassList.back().reps++;
				}
			}
			else
			{
				if (gPtr != NULL)
					gPtr = NULL;
			}

			++i;
		}

		if (hasGrass)
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
					testGateInfo.vertexIndex0 = closePoint->GetIndex();
				}
			}
			else
			{
				found = true;
				gatePoints = 2;

				testGateInfo.poly1 = (*it);
				testGateInfo.point1 = closePoint;
				testGateInfo.vertexIndex1 = closePoint->GetIndex();
				//view.setCenter(testGateInfo.point1->pos.x, testGateInfo.point1->pos.y);
				//preScreenTex->setView(view);
			}
		}
	}

	if (!found && gatePoints == 0 )
	{
		for (auto it = gates.begin(); it != gates.end(); ++it)
		{
			if ((*it)->ContainsPoint(pos))
			{
				modifyGate = (*it);
			}
		}
	}
}

//returns true if attach is successful
ActorParams * EditSession::AttachActorToPolygon( ActorPtr actor, PolyPtr poly )
{
	TerrainPoint *next;
	V2d currPos, nextPos;
	V2d aCurr, aNext;
	V2d actorPos;

	int numP = poly->GetNumPoints();
	TerrainPoint *polyCurr, *polyNext;
	TerrainPoint *nextActorPoint;
	for (int i = 0; i < numP; ++i)
	{
		polyCurr = poly->GetPoint(i);
		polyNext = poly->GetNextPoint(i);

		currPos.x = polyCurr->pos.x;
		currPos.y = polyCurr->pos.y;

		nextPos.x = polyNext->pos.x;
		nextPos.y = polyNext->pos.y;

		assert(actor->posInfo.ground != NULL);

		actorPos = actor->posInfo.GetPosition();//aCurr + normalize(aNext - aCurr) * actorQuant;//V2d( actor->image.getPosition() );//
		bool onLine = PointOnLine(actorPos, currPos, nextPos);

		double finalQuant = dot(actorPos - currPos, normalize(nextPos - currPos));


		if (onLine)
		{
			cout << "actorPos: " << actorPos.x << ", " << actorPos.y << ", currPos: "
				<< currPos.x << ", " << currPos.y << endl;
			PositionInfo gi;

			gi.SetGround(poly, i, finalQuant);
			//might need to make sure it CAN be grounded

			ActorParams *newActor = actor->Copy();
			newActor->AnchorToGround(gi); //might be unnecessary

			assert(newActor != NULL);
			return newActor;
		}

	}

	return NULL;
	//return false;
}

void EditSession::AddDoneAction( Action *a )
{
	if (!doneActionStack.empty() && doneActionStack.back() == a)
	{
		int xxxxx = 5;
		assert(0);
	}
	doneActionStack.push_back(a);
}

void EditSession::UndoMostRecentAction()
{
	if (doneActionStack.size() > 0)
	{
		Action *action = doneActionStack.back();
		doneActionStack.pop_back();

		action->Undo();

		undoneActionStack.push_back(action);

		//clearing this so things don't get messing with deleted items being selected etc
		ClearSelectedBrush();
	}
}

void EditSession::RedoMostRecentUndoneAction()
{
	if (undoneActionStack.size() > 0)
	{
		Action *action = undoneActionStack.back();
		undoneActionStack.pop_back();

		action->Perform();

		AddDoneAction(action);

		ClearSelectedBrush();
	}
}

void EditSession::AttachActorsToPolygon( list<ActorPtr> &actors, PolyPtr poly )
{
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

bool IsWithinOne(sf::Vector2i &a, sf::Vector2i &b)
{
	return (abs(a.x - b.x) <= 1 && abs(a.y - b.y) <= 1);
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

void EditSession::SetInitialView(sf::Vector2f &center,
	sf::Vector2f &size)
{
	if (size.x == 0 && size.y == 0)
	{
		return;
	}
	view.setCenter(center);
	view.setSize(size);
	initialViewSet = true;
}

int EditSession::Run()
{
	TestLoad();

	players[0] = new Actor(NULL, this, 0);
	players[0]->InitAfterEnemies();
	for (int i = 1; i < MAX_PLAYERS; ++i)
	{
		if (GetController(i).IsConnected())
		{
			players[i] = new Actor(NULL, this, i);
			players[i]->InitAfterEnemies();
		}
	}
	

	oldShaderZoom = -1;
	complexPaste = NULL;

	testGateInfo.edit = EditSession::GetSession();
	bool oldMouseGrabbed = mainMenu->GetMouseGrabbed();
	bool oldMouseVis = mainMenu->GetMouseVisible();

	mainMenu->SetMouseGrabbed(true);
	mainMenu->SetMouseVisible(true);

	sf::View oldPreTexView = preScreenTex->getView();//mainMenu->preScreenTexture->
	sf::View oldWindowView = window->getView();

	currTool = TOOL_ADD;
	currentFile = filePath.string();

	tempActor = NULL;
	v.setCenter( 0, 0 );
	v.setSize( 1920/ 2, 1080 / 2 );
	window->setView( v );

	shardSelectPopup = CreatePopupPanel("shardselector");

	confirm = CreatePopupPanel( "confirmation" );
	validityRadius = 4;


	modifyGate = NULL;


	showGrass = false;
	showGraph = false;

	justCompletedPolyWithClick = false;

	trackingEnemyParams = NULL;
	showPanel = NULL;

	sf::Texture playerZoomIconTex;
	playerZoomIconTex.loadFromFile( "Resources/Editor/playerzoomicon.png" );
	sf::Sprite playerZoomIcon( playerZoomIconTex );
	
	playerZoomIcon.setOrigin( playerZoomIcon.getLocalBounds().width / 2, playerZoomIcon.getLocalBounds().height / 2 );	

	SetupEnemyTypes();

	InitDecorPanel();

	mapOptionsPanel = CreateOptionsPanel("map");
	terrainOptionsPanel = CreateOptionsPanel("terrain");
	railOptionsPanel = CreateOptionsPanel("rail");

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

	enemySelectPanel = new Panel("enemyselection", 100, 150, this);//1920, 150, this );
	allPopups.push_back(enemySelectPanel);

	
	/*int gridSizeX = 80;
	int gridSizeY = 80;

	GridSelector *gs = NULL;
	int counter = 0;
	for (int i = 0; i < 4; ++i)
	{
		gs = enemySelectPanel->AddGridSelector(to_string(i), Vector2i(0, 0), 15, 10, gridSizeX,
			gridSizeY, false, true);
		gs->active = false;
		enemyGrid[i] = gs;

		counter = 0;
		for (int j = 0; j < 8; ++j)
		{
			auto &wen = worldEnemyNames[j];
			counter = 0;
			for (auto it = wen.begin(); it != wen.end(); ++it)
			{
				if ((*it).numLevels >= i+1)
				{
					SetEnemyGridIndex(gs, counter, j, (*it).name);
				}
				++counter;
			}
		}

		counter = 0;
		int row = 8;
		for (auto it = extraEnemyNames.begin(); it != extraEnemyNames.end(); ++it)
		{
			if ((*it).numLevels >= i + 1)
			{
				SetEnemyGridIndex(gs, counter, row, (*it).name);
			}
			++counter;
			if (counter == gs->xSize)
			{
				counter = 0;
				++row;
			}
		}

	}*/

	//SetActiveEnemyGrid(0);

	gateSelectorPopup = CreatePopupPanel( "gateselector" );
	GridSelector *gateSel = gateSelectorPopup->AddGridSelector( "gatetypes", Vector2i( 20, 20 ), 7, 4, 32, 32, false, true );

	sf::Texture whiteTex; //temp
	whiteTex.loadFromFile( "Resources/Editor/whitesquare.png" );
	Sprite tempSq;
	tempSq.setTexture( whiteTex );

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

	tempSq.setColor(Color(100, 255, 10));
	gateSel->Set(6, 0, tempSq, "shard");

	gateSelectorPopup->AddButton( "deletegate", Vector2i( 20, 300 ), Vector2f( 80, 40 ), "delete" );

	SetupTerrainTypeSelector();

	currTerrainWorld = 0;
	currTerrainVar = 0;
	currTerrainTypeSpr.setPosition(0, 160);
	UpdateCurrTerrainType();

	returnVal = 0;
	Color testColor( 0x75, 0x70, 0x90 );

	//view = View( cameraPos, cameraSize );
	//if( cameraSize.x == 0 && cameraSize.y == 0 )
	//	view.setSize( 1920, 1080 );

	preScreenTex->setView( view );

	//recentEnemies.reserve(MAX_RECENT_ENEMIES);
	/*for (int i = 0; i < MAX_RECENT_ENEMIES; ++i)
	{
		recentEnemies.push_back(make_pair((ActorType*)NULL, 0));
	}*/



	ReadFile();

	for (auto it = types.begin(); it != types.end(); ++it)
	{
		(*it).second->CreateDefaultEnemy();
	}

	createEnemyModeUI = new CreateEnemyModeUI();
	
	//enemyChooser = new EnemyChooser(types, enemySelectPanel);
	//enemySelectPanel->AddEnemyChooser("blah", enemyChooser);

	if (!initialViewSet)
	{
		view.setSize(1920, 1080);
		view.setCenter(player->GetFloatPos());
	}

	quit = false;
	polygonInProgress = new TerrainPolygon();
	railInProgress = new TerrainRail();
	//inversePolygon.reset( NULL );

	zoomMultiple = 2;

	view.setSize(Vector2f(960 * (zoomMultiple), 540 * (zoomMultiple)));
	preScreenTex->setView(view);

	UpdateFullBounds();
	
	panning = false;
	minimumEdgeLength = 8;//8;

	Color borderColor = sf::Color::Green;
	int max = 1000000;
	
	border[0] = sf::Vertex(sf::Vector2<float>(-max, -max), borderColor);
	border[1] = sf::Vertex(sf::Vector2<float>(-max, max), borderColor);
	border[2] = sf::Vertex(sf::Vector2<float>(-max, max), borderColor);
	border[3] = sf::Vertex(sf::Vector2<float>(max, max), borderColor);
	border[4] = sf::Vertex(sf::Vector2<float>(max, max), borderColor);
	border[5] = sf::Vertex(sf::Vector2<float>(max, -max), borderColor);
	border[6] = sf::Vertex(sf::Vector2<float>(max, -max), borderColor);
	border[7] = sf::Vertex(sf::Vector2<float>(-max, -max), borderColor);

	sf::Texture guiMenuCubeTexture;
	guiMenuCubeTexture.loadFromFile( "Resources/Editor/guioptions.png" );
	guiMenuSprite.setTexture( guiMenuCubeTexture );
	guiMenuSprite.setOrigin( guiMenuSprite.getLocalBounds().width / 2, guiMenuSprite.getLocalBounds().height / 2 );

	graph = new EditorGraph;
	

	//bool s = IsKeyPressed( sf::Keyboard::T );

	SetMode(EDIT);
	stored = mode;
	bool canCreatePoint = true;

	menuCircleDist = 100;
	menuCircleRadius = 50;

	circleTopPos =  V2d( 0, -1 ) * menuCircleDist;

	circleUpperRightPos = V2d( sqrt( 3.0 ) / 2, -.5 ) * menuCircleDist;
	circleLowerRightPos = V2d( sqrt( 3.0 ) / 2, .5 ) * menuCircleDist;

	circleUpperLeftPos = V2d( -sqrt( 3.0 ) / 2, -.5 ) * menuCircleDist;
	circleLowerLeftPos = V2d( -sqrt( 3.0 ) / 2, .5 ) * menuCircleDist;

	circleBottomPos = V2d( 0, 1 ) * menuCircleDist;

	menuSelection = "";

	borderMove = 100;

	Vector2f uiMouse;

	while( !quit )
	{
		if (mode == EDIT || mode == CREATE_ENEMY)
		{
			double newTime = editClock.getElapsedTime().asSeconds();
			double frameTime = newTime - editCurrentTime;
			editCurrentTime = newTime;

			editAccumulator += frameTime;
			double mult;
			spriteUpdateFrames = 0;
			while (editAccumulator >= TIMESTEP)
			{
				mult = floor(editAccumulator / TIMESTEP);
				spriteUpdateFrames = mult;

				editAccumulator -= mult * TIMESTEP;
			}
		}

		pixelPos = GetPixelPos();

		oldWorldPosTest = worldPos;
		worldPos = V2d(preScreenTex->mapPixelToCoords(pixelPos));
		//eventually also use this in create enemy mode
		if (IsSingleActorSelected())
		{
			//no longer used because of multi-move
			//worldPosGround = ConvertPointToGround(Vector2i(worldPos.x, worldPos.y));
			worldPosRail = ConvertPointToRail(Vector2i(worldPos));
		}
		

		preScreenTex->setView( uiView );
		uiMouse = preScreenTex->mapPixelToCoords( pixelPos );
		uiMousePos = uiMouse;
		
		preScreenTex->setView( view );
		
		testPoint.x = worldPos.x;
		testPoint.y = worldPos.y;

		if (mode == CREATE_PATROL_PATH || mode == SET_DIRECTION)
		{
			if (showPanel != NULL)
			{

			}
			else
			{
				V2d pathBack(patrolPath.back());
				V2d temp = V2d(testPoint.x, testPoint.y) - pathBack;

				if (IsKeyPressed(Keyboard::LShift))
				{
					double angle = atan2(-temp.y, temp.x);
					if (angle < 0)
					{
						angle += PI * 2.0;
					}
					double len = length(temp);
					double mult = angle / (PI / 4.0);
					double dec = mult - floor(mult);
					int iMult = mult;
					if (dec >= .5)
					{
						iMult++;
					}

					angle = iMult * PI / 4.0;
					V2d testVec(len, 0);
					RotateCCW(testVec, angle);
					testPoint = Vector2f(pathBack + testVec);
					temp = testVec;
				}
			}
		}

		HandleEvents();

		if( quit )
			break;

		showGraph = false;

		if (IsKeyPressed(Keyboard::Num5))
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

		//--------

		UpdateMode();
		
		

		UpdatePanning();
		
		background->Update(view.getCenter());

		UpdatePolyShaders();
		
		Draw();
		
		DrawMode();		

		TempMoveSelectedBrush();

		DrawGraph();

		DrawDecorFront();

		if( zoomMultiple > 7 )
		{
			playerZoomIcon.setPosition( player->GetFloatPos() );
			playerZoomIcon.setScale( zoomMultiple * 1.8, zoomMultiple * 1.8 );
			preScreenTex->draw( playerZoomIcon );
		}

		preScreenTex->draw( fullBounds );

		if (mode == CREATE_IMAGES)
		{
			if( showPanel == NULL )
				preScreenTex->draw(tempDecorSprite);
		}

		DrawUI();

		preScreenTex->setView( view );

		Display();
	}
	
	preScreenTex->setView(oldPreTexView);
	window->setView(oldWindowView);

	mainMenu->SetMouseGrabbed(oldMouseGrabbed);
	mainMenu->SetMouseVisible(oldMouseVis);

	return returnVal;
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
			if (mode == EDIT)
			{
				SelectPtr select = selectedBrush->objects.front();
				AirTriggerParams *airTrigger = (AirTriggerParams*)select;
				rectCreatingTrigger = airTrigger;
			}
			else if (mode == CREATE_ENEMY)
			{
				rectCreatingTrigger = (AirTriggerParams*)tempActor;
			}

			showPanel = NULL;
			
			SetMode(CREATE_RECT);
			drawingCreateRect = false;
		}
	}
	else if (p->name == "camerashot_options")
	{
		if (b->name == "ok")
		{
			RegularOKButton();
		}
		else if (b->name == "setzoom")
		{
			if (mode == EDIT)
			{
				SelectPtr select = selectedBrush->objects.front();
				CameraShotParams *camShot = (CameraShotParams*)select;
				currentCameraShot = camShot;
			}
			else if (mode == CREATE_ENEMY)
			{
				currentCameraShot = (CameraShotParams*)tempActor;
			}

			showPanel = NULL;
			SetMode(SET_CAM_ZOOM);
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
	}
	else if (p->name == "rail_options")
	{
		SelectPtr select = selectedBrush->objects.front();
		TerrainRail *tr = (TerrainRail*)select;
		if (b->name == "ok")
		{
			tr->SetParams(railOptionsPanel);
			showPanel = NULL;
		}
		else if (b->name == "reverse")
		{
			tr->SwitchDirection();
			//reverse single rail
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
	else if (p == shardSelectPopup)
	{
		if (b->name == "ok")
		{
			tempGridResult = "shardclose";
		}
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
		else if (b->name == "setdirection")
		{
			RegularCreatePathButton();
			SetMode(SET_DIRECTION);
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
				SelectPtr select = selectedBrush->objects.front();
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
				SelectPtr select = selectedBrush->objects.front();
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
				SelectPtr select = selectedBrush->objects.front();
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
			//trackingEnemy = types[name];

			//enemySprite = trackingEnemy->GetSprite(false);

			//enemyQuad.setSize( Vector2f( trackingEnemy->info.size.x, trackingEnemy->info.size.y) );

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
		if (name != "not set" )
		{
			if (background == NULL || name != background->name)
			{
				Background *newBG = Background::SetupFullBG(name, this);

				if (newBG != NULL)
				{
					if (background != NULL)
					{
						background->DestroyTilesets();
						delete background;
					}

					tempGridResult = name;
					envName = name;
				}
				
			}
		}
	}
	else if (panel->name == "shard_options" )
	{
		int world = gs->selectedX / 11;
		int realX = gs->selectedX % 11;
		int realY = gs->selectedY;

		ShardParams *shard = NULL;
		if (mode == EDIT)
		{
			SelectPtr select = selectedBrush->objects.front();
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
	else if (panel->name == "shardselector")
	{
		tempGridResult = name;
		tempGridX = gs->selectedX;
		tempGridY = gs->selectedY;
		panel->labels["shardtype"]->setString(name);
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
				SelectPtr select = selectedBrush->objects.front();
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
				SelectPtr select = selectedBrush->objects.front();
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

bool EditSession::TrySnapPosToPoint(sf::Vector2f &p, double radius, PolyPtr &poly, TerrainPoint *&point)
{
	auto & currPolyList = GetCorrectPolygonList();

	for (auto it = currPolyList.begin(); it != currPolyList.end(); ++it)
	{
		TerrainPoint *closePoint = (*it)->GetClosePoint(radius, V2d(p));
		if (closePoint != NULL)
		{
			p = Vector2f(closePoint->pos);
			poly = (*it);
			point = closePoint;
			return true;
		}
	}

	return false;
}

bool EditSession::TrySnapPosToPoint(V2d &p, double radius, PolyPtr &poly, TerrainPoint *&point)
{
	auto & currPolyList = GetCorrectPolygonList();

	for (auto it = currPolyList.begin(); it != currPolyList.end(); ++it)
	{
		TerrainPoint *closePoint = (*it)->GetClosePoint(radius, V2d(p));
		if (closePoint != NULL)
		{
			p = V2d(closePoint->pos);
			poly = (*it);
			point = closePoint;
			return true;
		}
	}

	return false;
}

bool EditSession::IsSpecialTerrainMode()
{
	if (currTerrainWorld >= 8)
	{
		return true;
	}

	return false;
}

void EditSession::InitDecorPanel()
{
	int w = 8;
	int h = 5;
	int sw = 128;
	int sh = 128;
	ReadDecorImagesFile();
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
			(*pit).GetPolyPoint()->selected = false;
		}
	}

	for (auto pmit = selectedRailPoints.begin();
		pmit != selectedRailPoints.end(); ++pmit)
	{
		list<PointMoveInfo> & pList = (*pmit).second;
		for (list<PointMoveInfo>::iterator pit = pList.begin();
			pit != pList.end(); ++pit)
		{
			(*pit).GetRailPoint()->selected = false;
		}
	}

	selectedPoints.clear();
	selectedRailPoints.clear();
}

void EditSession::RemovePointFromPolygonInProgress()
{
	polygonInProgress->RemoveLastPoint();
}

void EditSession::RemovePointFromRailInProgress()
{
	if (railInProgress->GetNumPoints() > 0)
	{
		railInProgress->RemoveLastPoint();
	}
}

void EditSession::TryAttachActorsToPolys(
	std::list<PolyPtr> &origPolys,
	std::list<PolyPtr> &newPolys,
	Brush *b)
{
	for (auto it = origPolys.begin(); it != origPolys.end(); ++it)
	{
		TryAttachActorsToPoly((*it), newPolys, b);
	}
}

void EditSession::TryAttachActorsToPolys(
	std::set<PolyPtr> &origPolys,
	std::list<PolyPtr> &newPolys,
	Brush *b)
{
	for (auto it = origPolys.begin(); it != origPolys.end(); ++it)
	{
		TryAttachActorsToPoly((*it), newPolys, b);
	}
}

void EditSession::TryAttachActorsToPoly( PolyPtr orig, std::list<PolyPtr> & newPolys, Brush *b)
{
	for (auto mit = orig->enemies.begin(); mit != orig->enemies.end(); ++mit)
	{
		for (auto bit = (*mit).second.begin(); bit != (*mit).second.end(); ++bit)
		{
			for (auto rit = newPolys.begin();
				rit != newPolys.end(); ++rit)
			{
				ActorParams *ac = AttachActorToPolygon((*bit), (*rit));
				if (ac != NULL)
				{
					b->AddObject(ac);
				}
			}
		}
	}
}

void EditSession::TryKeepGates( list<GateInfoPtr> &gateInfoList, list<PolyPtr> &newPolys, Brush *b )
{
	for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
	{
		TerrainPoint *test = NULL;
		TerrainPoint * outTest = NULL;

		TerrainPoint *p0 = NULL;
		TerrainPoint *p1 = NULL;
		TerrainPoint *testPoint = NULL;


		PolyPtr poly0 = NULL;
		PolyPtr poly1 = NULL;

		for (auto rit = newPolys.begin();
			rit != newPolys.end(); ++rit)
		{
			if (p0 != NULL && p1 != NULL)
				break;

			if (p0 == NULL)
			{
				testPoint = (*rit)->GetSamePoint((*it)->point0->pos);
				if (testPoint != NULL)
				{
					p0 = testPoint;
					poly0 = (*rit);
				}
			}

			if (p1 == NULL)
			{
				testPoint = (*rit)->GetSamePoint((*it)->point1->pos);
				if (testPoint != NULL)
				{
					p1 = testPoint;
					poly1 = (*rit);
				}
			}
		}


		if (p0 == NULL && p1 == NULL)
			continue;

		GateInfoPtr gi = new GateInfo(*(*it));
		gi->edit = NULL;

		if (p0 != NULL)
		{
			gi->point0 = p0;
			gi->poly0 = poly0;
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
			gi->poly1 = poly1;
			p1->gate = gi;

		}
		else
		{
			gi->point1 = (*it)->point1;
			gi->poly1 = (*it)->poly1;
			gi->point1->gate = gi;
		}

		b->AddObject(gi);
	}
}

void EditSession::TryRemoveSelectedPoints()
{
	//need to make this into an undoable action soon

	//int removeSuccess = IsRemovePointsOkay();

	//if (removeSuccess == 1)
	{
		Brush orig;
		Brush result;

		list<GateInfoPtr> gateInfoList;
		list<PolyPtr> affectedPolys;
		list<PolyPtr> newPolys;
		bool valid = true;
		for (PointMap::iterator it = selectedPoints.begin(); it != selectedPoints.end(); ++it)
		{
			PolyPtr tp = (*it).first;
			affectedPolys.push_back(tp);

			PolyPtr newPoly(tp->CreateCopyWithSelectedPointsRemoved());

			if (newPoly != NULL)
			{
				newPoly->RemoveSlivers();
				newPoly->AlignExtremes();
			}
			else
			{
				valid = false;
			}
			
			if (valid)
			{
				if (!IsPolygonValid(newPoly, tp))
				{
					valid = false;
				}
			}

			if (!valid)
			{
				delete newPoly;
				for (auto pit = newPolys.begin(); pit != newPolys.end(); ++pit)
				{
					delete (*pit);
				}
				MessagePop("problem removing points");
				return;
			}
			
			newPolys.push_back(newPoly);
		}

		for (auto it = newPolys.begin(); it != newPolys.end(); ++it)
		{
			(*it)->Finalize();
			result.AddObject((*it));
		}

		ClearSelectedPoints();

		AddFullPolysToBrush(affectedPolys, gateInfoList, &orig);

		TryAttachActorsToPolys(affectedPolys, newPolys, &result);

		TryKeepGates(gateInfoList, newPolys, &result);
		selectedPoints.clear();

		ClearUndoneActions();

		Action * action = new ReplaceBrushAction(&orig, &result, mapStartBrush);

		action->Perform();
		AddDoneAction(action);
	}
	/*else if (removeSuccess == 0)
	{
		MessagePop("problem removing points");
	}*/
}

bool EditSession::PointSelectActor( V2d &pos )
{
	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		for (auto ait = (*it).second->actors.begin();
			ait != (*it).second->actors.end(); ++ait)
		{
			if ((*ait)->ContainsPoint(Vector2f(pos)))
			{
				if ((*ait)->selected)
				{
					grabbedActor = (*ait);
					if ((*ait)->myEnemy != NULL)
						(*ait)->myEnemy->SetActionEditLoop();
					ClearSelectedPolys();
				}
				else
				{
					if (!HoldingShift())
					{
						ClearSelectedBrush();
					}

					
					grabbedActor = (*ait);
					//grabbedObject = (*ait);
					if( (*ait)->myEnemy != NULL )
						(*ait)->myEnemy->SetActionEditLoop(); //just for testing

					ClearSelectedPolys();
					//(*ait)->myEnemy->action = (*)

					SelectObject((*ait));
				}
				return true;
			}
		}
	}

	return false;
}

bool EditSession::PointSelectDecor(V2d &pos)
{
	for (auto it = decorImagesBehindTerrain.begin();
		it != decorImagesBehindTerrain.end(); ++it)
	{

	}

	for (auto it = decorImagesBetween.begin();
		it != decorImagesBetween.end(); ++it)
	{
		if ((*it)->ContainsPoint(Vector2f(worldPos.x, worldPos.y)))
		{
		
			if ((*it)->selected)
			{

			}
			else
			{
				if (!HoldingShift())
				{
					ClearSelectedBrush();
				}

				grabbedObject = (*it);
				SelectObject((*it));
			}

			return true;
		}
	}

	for (auto it = decorImagesFrontTerrain.begin();
		it != decorImagesFrontTerrain.end(); ++it)
	{

	}

	return false;
}

bool EditSession::AnchorSelectedEnemies()
{
	ActorPtr actor;
	for( auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it )
	{
		actor = (*it)->GetAsActor();
		if (actor == NULL)
			continue;

		if (actor->posInfo.ground != NULL) //might need a thing here for rails too
		{
			Action *gAction = new GroundAction(actor);
			gAction->performed = true;

			if (moveAction != NULL)
			{
				//was already ungrounded at the beginning of the move
				moveAction->subActions.push_back(gAction);
			}
			else
			{
				//started in the air
				Vector2i delta = Vector2i(worldPos.x, worldPos.y) - editMouseOrigPos;
				Action *action = new MoveBrushAction(selectedBrush, delta, false, NULL, RailPointMap());

				action->Perform();

				moveAction = new CompoundAction;
				moveAction->subActions.push_back(action);
				moveAction->subActions.push_back(gAction);
			}

			//return true;
		}
	}

	Action *apply = NULL;
	if (mode == CREATE_ENEMY)
	{
		apply = new ApplyBrushAction(selectedBrush);
		apply->performed = true;

		grabbedActor->group->actors.push_back(grabbedActor);
		trackingEnemyParams = NULL;

		createEnemyModeUI->SetShown(true);
	}

	if (moveAction != NULL)
	{
		if (mode == CREATE_ENEMY)
		{
			moveAction->subActions.push_front(apply);
		}

		AddDoneAction(moveAction);
		moveAction = NULL;
		return true;
	}
	else
	{
		if (mode == CREATE_ENEMY)
		{
			AddDoneAction(apply);
		}
	}
		

	return false;
}

void EditSession::TryCompleteSelectedMove()
{
	bool validMove = false;

	//check if valid
	if (selectedBrush->CanApply())
	{
		validMove = true;
	}

	//if (mode == CREATE_ENEMY )
	//{

	//	AddRecentEnemy(grabbedActor);
	//	ClearSelectedBrush();

	//	/*if (enemySelectPanel->ContainsPoint(Vector2i(uiMousePos)))
	//	{
	//		validMove = false;
	//	}*/

	//	
	//}

	if (validMove)
	{
		ClearUndoneActions();

		if (mode == CREATE_ENEMY)
		{
			AddRecentEnemy(grabbedActor);
			ClearSelectedBrush();
			createEnemyModeUI->SetLibraryShown(false);
		}

	}
	else
	{
		if (mode == CREATE_ENEMY)
		{
			ClearSelectedBrush();
		}

		Action * action = doneActionStack.back();
		doneActionStack.pop_back();

		action->Undo();

		delete action;
	}

}

void EditSession::SetupTerrainTypeSelector()
{
	terrainSelectorPopup = CreatePopupPanel("terrainselector");
	GridSelector *terrainSel = terrainSelectorPopup->AddGridSelector(
		"terraintypes", Vector2i(20, 20), TERRAIN_WORLDS, MAX_TERRAINTEX_PER_WORLD, 64, 64, false, true);

	for (int worldI = 0; worldI < TERRAIN_WORLDS; ++worldI)
	{
		int ind;
		for (int i = 0; i < MAX_TERRAINTEX_PER_WORLD; ++i)
		{
			ind = worldI * MAX_TERRAINTEX_PER_WORLD + i;
			
			if (LoadPolyShader(ind, worldI, i))
			{
				terrainSel->Set(worldI, i, Sprite(*ts_polyShaders[ind]->texture, sf::IntRect(0, 0, 64, 64)),
					"xx");
			}
		}
	}
}

void EditSession::SetEnemyGridIndex( GridSelector *gs, int x, int y, const std::string &eName)
{
	gs->Set(x, y, types[eName]->GetSprite(gs->tileSizeX, gs->tileSizeY), eName);
}

void EditSession::SetActiveEnemyGrid(int index)
{
	for (int i = 0; i < 4; ++i)
	{
		enemyGrid[i]->active = false;
	}
	enemyGrid[index]->active = true;
	enemySelectLevel = index+1;
}



void EditSession::RegularOKButton()
{
	if (mode == EDIT)
	{
		ISelectable *select = selectedBrush->objects.front();
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
	SetMode(CREATE_PATROL_PATH);
	showPanel = NULL;
	Vector2i front = patrolPath.front();
	patrolPath.clear();
	patrolPath.push_back(front);
	patrolPathLengthSize = 0;
}

void EditSession::SelectPoint(PolyPtr poly,
	TerrainPoint *point)
{
	if (!point->selected)
	{
		PointMoveInfo pi;
		pi.poly = poly;
		pi.pointIndex = point->GetIndex();
		selectedPoints[poly].push_back(pi);
		point->selected = true;
	}
}

void EditSession::DeselectPoint(PolyPtr poly,
	TerrainPoint *point)
{
	if (point->selected)
	{
		point->selected = false;
		auto & infoList = selectedPoints[poly];
		for (auto it = infoList.begin(); it != infoList.end(); ++it)
		{
			if ((*it).GetPolyPoint() == point)
			{
				infoList.erase(it);
				break;
			}
		}
	}
}

void EditSession::SelectPoint(RailPtr rail,
	TerrainPoint *point)
{
	if (!point->selected)
	{
		PointMoveInfo pi;
		pi.rail = rail;
		pi.pointIndex = point->GetIndex();
		selectedRailPoints[rail].push_back(pi);
		point->selected = true;
	}
}

void EditSession::DeselectPoint( RailPtr rail,
	TerrainPoint *point)
{
	if (point->selected)
	{
		point->selected = false;
		auto & infoList = selectedRailPoints[rail];
		for (auto it = infoList.begin(); it != infoList.end(); ++it)
		{
			if ((*it).GetRailPoint() == point)
			{
				infoList.erase(it);
				break;
			}
		}
	}
}




void EditSession::PerformMovePointsAction()
{
	Vector2i delta = Vector2i(worldPos.x, worldPos.y) - editMouseOrigPos;
	//here the delta being subtracted is the points original positionv

	//commented the grabbedPoint thing out because it was crashing. but it maybe shouldn't be?

	PointMover *pm = new PointMover;//need to worry about alignextremes but for now just use the actual moving points.

	PolyPtr poly;
	for (auto mit = selectedPoints.begin(); mit != selectedPoints.end(); ++mit)
	{
		poly = (*mit).first;

		poly->AlignExtremes(); //adjust this later!!! need to take this into account

		vector<PointMoveInfo> &pmVec = pm->movePoints[poly];
		pmVec.reserve((*mit).second.size());
		for (auto it = (*mit).second.begin(); it != (*mit).second.end(); ++it)
		{
			(*it).newPos = (*it).GetPolyPoint()->pos;
			pmVec.push_back((*it));
		}
		pm->oldEnemyPosInfo.insert(pm->oldEnemyPosInfo.end(),
			poly->enemyPosBackups.begin(), poly->enemyPosBackups.end());

		poly->StoreEnemyPositions(pm->newEnemyPosInfo);

		poly->SoftReset();
		poly->Finalize();
		poly->SetRenderMode(TerrainPolygon::RENDERMODE_NORMAL);

		for (auto it = pm->newEnemyPosInfo.begin(); it != pm->newEnemyPosInfo.end(); ++it)
		{
			(*it).first->posInfo = (*it).second;
			(*it).first->UpdateGroundedSprite();
			(*it).first->SetBoundingQuad();
		}
	}

	for (auto mit = selectedRailPoints.begin(); mit != selectedRailPoints.end(); ++mit)
	{
		list<PointMoveInfo> &pList = (*mit).second;
		for (list<PointMoveInfo>::iterator it = pList.begin(); it != pList.end(); ++it)
		{
			//(*it).delta = (*it).GetRailPoint()->pos - (*it).delta;
		}
	}

	MoveBrushAction *action = new MoveBrushAction(selectedBrush, delta, false, pm, selectedRailPoints);
	action->performed = true;//Perform();

	CompoundAction *testAction = NULL;
	if (moveAction != NULL)
	{
		testAction = moveAction;
	}
	else
	{
		testAction = new CompoundAction;
	}
	testAction->subActions.push_back(action);


	//might want to pass pm to IsGateAttachedToAffectedPoint etc

	//check for validity
	if (true )//action->moveValid)
	{
		int gateActionsAdded = 0;
		for (auto it = gates.begin(); it != gates.end(); ++it)
		{
			bool gateAttachedToAffectedPoly = false;
			PolyPtr poly;
			bool a = true;
			bool polyMove = true;

			//if (IsGateAttachedToAffectedPoints((*it), selectedPoints, a))
			if (IsGateAttachedToAffectedPoints((*it), pm, a))
			{
				polyMove = false;
				gateAttachedToAffectedPoly = true;
			}

			if (IsGateAttachedToAffectedPoly((*it), selectedBrush, a))
			{
				//should this even happen if the points are moving?
				gateAttachedToAffectedPoly = true;
			}


			if (gateAttachedToAffectedPoly)
			{
				GateInfo *gi = (*it);
				Vector2i adjust;
				Vector2i pA, pB;

				GateAdjustOption gaOption;
				if (polyMove)
				{
					if (a)
					{
						gaOption = GATEADJUST_A;
					}
					else
					{
						gaOption = GATEADJUST_B;
					}
				}
				else
				{
					if (a)
					{
						gaOption = GATEADJUST_POINT_B;
					}
					else
					{
						gaOption = GATEADJUST_POINT_A;
					}
				}

				if (GetPrimaryAdjustment(gi->point0->pos, gi->point1->pos, adjust))
				{

					if (!TryGateAdjustAction(gaOption, gi, adjust, testAction))
					{
						testAction->Undo();

						if (testAction == moveAction)
						{
							moveAction = NULL;
						}

						delete testAction;
						return;
					}
					else
					{
						gateActionsAdded++;
					}
				}
				else
				{
					//action->Perform();
					//doneActionStack.push_back(action);
				}
			}



		}

		testAction->performed = true;
		AddDoneAction(testAction);
		/*if (gateActionsAdded > 0)
		{
		testAction->performed = true;
		doneActionStack.push_back(testAction);
		}
		else
		{
		if (moveAction != NULL)
		{
		moveAction->subActions.push_back(action);
		doneActionStack.push_back(moveAction);
		}
		else
		{
		doneActionStack.push_back(action);
		}
		}*/
	}
	else
	{
		testAction->Undo();

		if (testAction == moveAction)
		{
			moveAction = NULL;
		}

		delete testAction;
	}
}



void EditSession::StartMoveSelectedPoints()
{
	//Brush origBrush;
	//list<GateInfoPtr> gateInfoList;

	//if (selectedPoints.empty())
	//{
	//	movePointsAction = NULL;
	//}

	//for (auto it = selectedPoints.begin(); it != selectedPoints.end(); ++it)
	//{
	//	(*it).first->BackupPoints();

	//	
	//	//AddFullPolyToBrush((*it).first, gateInfoList, &origBrush);
	//}

	PolyPtr poly;
	for (auto it = selectedPoints.begin(); it != selectedPoints.end(); ++it)
	{
		poly = (*it).first;

		poly->BackupEnemyPositions();
		for (auto pit = (*it).second.begin(); pit != (*it).second.end(); ++pit)
		{
			(*pit).origPos = (*pit).GetPolyPoint()->pos;
		}
	}
	

	


	//Brush newBrush;
	//for (auto it = selectedPoints.begin(); it != selectedPoints.end(); ++it)
	//{
	//	(*it).first->Copy();
	//	//AddFullPolyToBrush((*it).first, gateInfoList, &origBrush);
	//}

	//movePointsAction = new MovePointsAction(&origBrush, gateInfoList, selectedPoints);
	//movePointsAction->performed = true;

	
}

void EditSession::NewMoveSelectedPoints()
{

}

void EditSession::NewPerformMovePointsAction()
{
	/*if (movePointsAction != NULL)
	{
		AddDoneAction(movePointsAction);
	}*/
}

void EditSession::MoveSelectedPoints()//sf::Vector2i delta )
{
	bool affected;
	int polyNumP;
	TerrainPoint *curr, *prev;
	PolyPtr poly;

	Edge *edge; //use prev edge also
	Edge *prevEdge;

	double edgeLen;
	double prevEdgeLen;

	bool failMove;
	double oldPrevLength;

	int i;
	//can I use the pointmap instead of iterating through everything?
	for( auto it = selectedPoints.begin(); it != selectedPoints.end(); ++it )
	{
		poly = (*it).first;
	
		if (poly->selected)
		{
			DeselectObject(poly);
		}

		affected = false;

		polyNumP = poly->GetNumPoints();
		
		failMove = false;

		for (auto pit = (*it).second.begin(); pit != (*it).second.end(); ++pit)
		{
			i = (*pit).pointIndex;

			curr = poly->GetPoint(i);
			prev = poly->GetPrevPoint(i);

			(*pit).oldPos = curr->pos; //for reverting this specific piece of a move
			//rather than the whole thing

			//depreciated. remove.
			//curr->oldPos = curr->pos;

			edge = poly->GetEdge(i);
			prevEdge = poly->GetPrevEdge(i);

			oldPrevLength = prevEdge->GetLength();

			//only need to do for the prevs, since anything moved on my edge stays at the same
			//quantity, the edge just moves.
			auto enemyIt = poly->enemies.find(prev);
			if (enemyIt != poly->enemies.end() )
			{
				list<ActorPtr> &enemies = (*enemyIt).second;
				for (list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait)
				{
					(*ait)->oldQuant = (*ait)->posInfo.groundQuantity;
				}
			}

			poly->MovePoint(i, pointGrabDelta);

			edgeLen = edge->GetLength();
			prevEdgeLen = prevEdge->GetLength();

			if (prevEdgeLen != oldPrevLength)
			{
				auto enemyIt = poly->enemies.find(prev);
				if (enemyIt != poly->enemies.end())
				{
					list<ActorPtr> &enemies = (*enemyIt).second;
					for (list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait)
					{
						(*ait)->posInfo.groundQuantity -= (oldPrevLength - prevEdgeLen);
					}
				}
			}

			//doesnt yet cover both sides at once
			double maxQuant;
			ActorPtr furthest = poly->GetFurthestEnemy(i, maxQuant);
			if (furthest != NULL && maxQuant > edgeLen)
			{
				poly->SetPointPos(i, Vector2i(edge->v1 - edge->Along() * maxQuant)); //works!
			}

			double minQuant;
			ActorPtr closest = poly->GetClosestEnemy(prev->GetIndex(), minQuant);
			if (closest != NULL && minQuant < 0)
			{
				poly->SetPointPos(i, Vector2i(prevEdge->v0 + prevEdge->Along() * (prevEdgeLen - minQuant)));

				if (prevEdgeLen != oldPrevLength)
				{
					auto enemyIt = poly->enemies.find(prev);
					if (enemyIt != poly->enemies.end())
					{
						list<ActorPtr> &enemies = (*enemyIt).second;
						for (list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait)
						{
							(*ait)->posInfo.groundQuantity += -minQuant;
						}
					}
				}
			}

			poly->UpdateLineColor(prev->index);
			poly->UpdateLineColor(i);

			affected = true;
		}

		//not sure why this is needed, but the edges are colliding w/ the enemies again
		auto currIt = poly->enemies.find(curr);
		if (currIt != poly->enemies.end())
		{
			list<ActorPtr> &currList = (*currIt).second;
			for (auto it = currList.begin(); it != currList.end(); ++it)
			{
				(*it)->UpdateGroundedSprite();
				(*it)->SetBoundingQuad();
			}
		}
		currIt = poly->enemies.find(prev);
		if (currIt != poly->enemies.end())
		{
			list<ActorPtr> &currList = (*currIt).second;
			for (auto it = currList.begin(); it != currList.end(); ++it)
			{
				(*it)->UpdateGroundedSprite();
				(*it)->SetBoundingQuad();
			}
		}


		if (!poly->IsInternallyValid())
		{
			for (auto pit = (*it).second.begin(); pit != (*it).second.end(); ++pit)
			{
				poly->SetPointPos((*pit).pointIndex, (*pit).oldPos);
			}

			//for (int i = 0; i < polyNumP; ++i)
			//{
			//	curr = poly->GetPoint(i);
			//	if (curr->selected) //selected
			//	{
			//		poly->SetPointPos(i, curr->oldPos);
			//	}
			//}
			//affected = false;

			auto enemyIt = poly->enemies.find(prev);
			if (enemyIt != poly->enemies.end())
			{
				list<ActorPtr> &enemies = (*enemyIt).second;
				for (list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait)
				{
					(*ait)->posInfo.groundQuantity = (*ait)->oldQuant;
				}
			}

			//should probably put the enemies back to their oldQuants too
		}
		else if (affected)
		{
			poly->SetRenderMode(TerrainPolygon::RENDERMODE_MOVING_POINTS);

			//for (map<TerrainPoint*, list<ActorPtr>>::iterator mit = poly->enemies.begin();
			//	mit != poly->enemies.end(); ++mit)
			//{
			//	list<ActorPtr> &enemies = (*mit).second;//(*it)->enemies[curr];
			//	for (list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait)
			//	{
			//		(*ait)->UpdateGroundedSprite();
			//		(*ait)->SetBoundingQuad();
			//	}
			//}
			
			poly->UpdateBounds();
		}
	}
}

void EditSession::MoveSelectedRailPoints(V2d worldPos)
{
	bool affected;
	int rNumP;
	TerrainPoint *curr, *prev;
	RailPtr rail;
	for (auto it = selectedRailPoints.begin(); it != selectedRailPoints.end(); ++it)
	{

		affected = false;
		rail = ((*it).first);

		if (rail->selected)
		{
			DeselectObject(rail);
		}

		rNumP = rail->GetNumPoints();

		for (int i = 0; i < rNumP; ++i)
		{
			curr = rail->GetPoint(i);
			prev = rail->GetPrevPoint(i);

			if (curr->selected) //selected
			{
				curr->pos += pointGrabDelta;

				rail->UpdateLineColor(rail->lines, prev->index, prev->index * 2);
				rail->UpdateLineColor(rail->lines, i, i * 2);

				if (rail->enemies.count(curr) > 0)
				{
					list<ActorPtr> &enemies = rail->enemies[curr];
					for (list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait)
					{
						//(*ait)->UpdateGroundedSprite();
					}
				}

				affected = true;
			}

		}

		rail->UpdateBounds();

		if (affected)
		{
			rail->movingPointMode = true;

			for (auto mit = rail->enemies.begin();
				mit != rail->enemies.end(); ++mit)
			{
				list<ActorPtr> &enemies = (*mit).second;
				for (auto ait = enemies.begin(); ait != enemies.end(); ++ait)
				{
					(*ait)->UpdateGroundedSprite();
					(*ait)->SetBoundingQuad();
				}
			}
		}
	}
}


//bool EditSession::IsGateAttachedToAffectedPoints(
//	GateInfoPtr gi, PointMover *pm,
//	bool &a)
//{
//	PolyPtr poly;
//	for (auto pit = pm->movePoints.begin(); pit != pm->movePoints.end(); ++pit)
//	{
//		poly = (*pit).first;
//		if (gi->poly0 == poly || gi->poly1 == poly)
//		{
//			if (gi->poly0 == poly)
//			{
//				a = true;
//			}
//			else
//			{
//				a = false;
//			}
//			return true;
//		}
//	}
//
//	return false;
//}

bool EditSession::IsGateAttachedToAffectedPoints(
	GateInfoPtr gi, PointMover *pm,
	bool &a)
{
	PolyPtr poly;
	for (auto pit = pm->movePoints.begin(); pit != pm->movePoints.end(); ++pit)
	{
		poly = (*pit).first;
		if (gi->poly0 == poly || gi->poly1 == poly)
		{
			if (gi->poly0 == poly)
			{
				a = true;
			}
			else
			{
				a = false;
			}
			return true;
		}
	}

	return false;
}

bool EditSession::IsGateAttachedToAffectedPoly(
	GateInfoPtr gi, Brush *b,
	bool &a)
{
	PolyPtr p;
	for (auto bit = b->objects.begin(); bit != b->objects.end(); ++bit)
	{
		p = (*bit)->GetAsTerrain();
		if (p != NULL)
		{
			if (gi->poly0 == p || gi->poly1 == p)
			{
				if (gi->poly0 == p)
				{
					a = true;
				}
				else
				{
					a = false;
				}

				return true;
			}
		}
	}

	return false;
}



bool EditSession::PolyContainsPolys(PolyPtr p, PolyPtr ignore)
{
	auto &testPolygons = GetCorrectPolygonList(p);
	for (auto it = testPolygons.begin(); it != testPolygons.end(); ++it)
	{
		if ((*it) == ignore)
			continue;

		if (p->Contains((*it)))
		{
			return true;
		}
	}
}

bool EditSession::PolyIsContainedByPolys(PolyPtr p, PolyPtr ignore)
{
	auto &testPolygons = GetCorrectPolygonList(p);
	for (auto it = testPolygons.begin(); it != testPolygons.end(); ++it)
	{
		if ((*it) == ignore)
			continue;

		if ((*it)->Contains(p))
		{
			return true;
		}
	}
}

bool EditSession::PolyIsTouchingEnemiesOrBeingTouched( PolyPtr p, PolyPtr ignore)
{
	//this also tests for your own enemies to check for validity.
	auto &testPolygons = GetCorrectPolygonList(p);
	for (auto it = testPolygons.begin(); it != testPolygons.end(); ++it)
	{
		if ((*it) == ignore)
			continue;

		if ((*it)->IsTouchingEnemiesFromPoly(p))
		{
			return true;
		}

		if (p->IsTouchingEnemiesFromPoly((*it)))
		{
			return true;
		}
	}


	return false;
}

void EditSession::ClearSelectedPolys()
{
	PolyPtr p;
	for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end();)
	{
		p = (*it)->GetAsTerrain();
		if (p != NULL)
		{
			p->SetSelected(false);
			it = selectedBrush->objects.erase(it);
		}
		else
		{
			++it;
		}
	}
}

bool EditSession::GateIsTouchingEnemies(GateInfo *gi)
{
	for (map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		auto actorList = (*it).second->actors;
		for (auto ait = actorList.begin(); ait != actorList.end(); ++ait)
		{
			if (gi->IsTouchingEnemy((*ait)))
			{
				return true;
			}
		}
	}
	return false;
}

bool EditSession::PolyIntersectsGates(PolyPtr poly)
{
	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		if (poly->IntersectsGate((*it)))
		{
			return true;
		}
	}

	return false;
}

bool EditSession::GateIntersectsPolys(GateInfo *gi)
{
	auto &testPolygons = GetCorrectPolygonList(0);

	for (auto pit = testPolygons.begin(); pit != testPolygons.end(); ++pit)
	{
		if ((*pit)->IntersectsGate(gi))
		{
			return true;
		}
	}

	return false;
}

bool EditSession::GateIntersectsGates(GateInfo *gi)
{
	Vector2i myPoint0, myPoint1, otherPoint0, otherPoint1;
	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		if ((*it) == gi)
		{
			continue;
		}

		myPoint0 = gi->point0->pos;
		myPoint1 = gi->point1->pos;

		otherPoint0 = (*it)->point0->pos;
		otherPoint1 = (*it)->point1->pos;

		LineIntersection li = EditSession::SegmentIntersect(myPoint0, myPoint1, otherPoint0, otherPoint1);
		if (!li.parallel)
		{
			return true;
		}
	}
	return false;
}

bool EditSession::PolyGatesIntersectOthers(PolyPtr poly)
{
	auto &testPolygons = GetCorrectPolygonList(poly);

	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		if ((*it)->poly0 == poly || (*it)->poly1 == poly)
		{	
			for (auto pit = testPolygons.begin(); pit != testPolygons.end(); ++pit)
			{
				if ((*pit) == poly)
					continue;

				if ((*pit)->IntersectsGate((*it)))
				{
					return true;
				}		
			}
		}
	}

	return false;
}

bool EditSession::IsGateInProgressValid(PolyPtr startPoly,
	TerrainPoint *startPoint)
{
	if (gateInProgressTestPoly == NULL)
	{
		return false;
	}
	else
	{
		GateInfo tempGate;
		tempGate.poly0 = startPoly;
		tempGate.point0 = startPoint;
		tempGate.poly1 = gateInProgressTestPoly;
		tempGate.point1 = gateInProgressTestPoint;
		return IsGateValid(&tempGate);

	}
}

bool EditSession::IsGateValid(GateInfo *gi)
{
	if (gi->point0 == gi->point1)
		return false;

	if (GateMakesSliverAngles(gi))
	{
		return false;
	}

	if (GateIntersectsPolys(gi))
	{
		return false;
	}

	if (GateIntersectsGates(gi))
	{
		return false;
	}

	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		if ((*it)->point0 == gi->point0 || (*it)->point0 == gi->point1
			|| (*it)->point1 == gi->point0 || (*it)->point1 == gi->point1)
		{
			return false;
		}
	}

	if (GateIsTouchingEnemies(gi))
	{
		return false;
	}

	Vector2f center(Vector2f(gi->point0->pos + gi->point1->pos) / 2.f);
	if (gi->poly0 == gi->poly1)
	{
		if (gi->poly0->ContainsPoint(center))
		{
			return false;
		}
	}

	//check slivers

	//check intersections

	//make sure you cant go within a single poly.
	return true;
}

bool EditSession::GateMakesSliverAngles(GateInfo *gi)
{
	PolyPtr poly0 = gi->poly0;
	PolyPtr poly1 = gi->poly1;

	TerrainPoint *p0 = gi->point0;
	TerrainPoint *p1 = gi->point1;

	int p0Index = p0->index;
	int p1Index = p1->index;

	TerrainPoint *prev = poly0->GetPrevPoint(p0Index);
	TerrainPoint *next = poly0->GetNextPoint(p0Index);

	if (IsSliver(prev, p0, p1))
	{
		return true;
	}

	if (IsSliver(p1, p0, next))
	{
		return true;
	}

	prev = poly1->GetPrevPoint(p1Index);
	next = poly1->GetNextPoint(p1Index);

	if (IsSliver(prev, p1, p0))
	{
		return true;
	}

	if (IsSliver(p0, p1, next))
	{
		return true;
	}

	return false;
}


bool EditSession::IsSliver( TerrainPoint *prev, TerrainPoint *curr, TerrainPoint *next)
{
	V2d pos(curr->pos.x, curr->pos.y);
	V2d prevPos(prev->pos.x, prev->pos.y);
	V2d nextPos(next->pos.x, next->pos.y);
	V2d dirA = normalize(prevPos - pos);
	V2d dirB = normalize(nextPos - pos);

	double diff = GetVectorAngleDiffCCW(dirA, dirB);
	double diffCW = GetVectorAngleDiffCW(dirA, dirB);
	if (diff < SLIVER_LIMIT)
	{
		return true;
	}
	else if (diffCW < SLIVER_LIMIT)
	{
		return true;
	}

	return false;
}

bool EditSession::PolyGatesMakeSliverAngles(PolyPtr poly)
{
	for (auto it = gates.begin(); it != gates.end(); ++it)
	{
		if ((*it)->poly0 == poly || (*it)->poly1 == poly)
		{
			if (GateMakesSliverAngles((*it)))
			{
				return true;
			}
		}
	}

	return false;
}

bool EditSession::IsCloseToPrimary(sf::Vector2i &p0,
	sf::Vector2i &p1, sf::Vector2i &prim)
{
	Vector2i diff = p1 - p0;
	prim = Vector2i(0, 0);

	if (diff.x == 0 || diff.y == 0)
		return false;

	V2d diffDir = normalize(V2d(diff));

	if (diffDir.x > PRIMARY_LIMIT)
		prim.x = 1;
	else if (diffDir.x < -PRIMARY_LIMIT)
		prim.x = -1;
	if (diffDir.y > PRIMARY_LIMIT)
		prim.y = 1;
	else if (diffDir.y < -PRIMARY_LIMIT)
		prim.y = -1;

	if (prim.x != 0 || prim.y != 0)
	{
		return true;
	}

	return false;
}

bool EditSession::GetPrimaryAdjustment(sf::Vector2i &p0,
	sf::Vector2i &p1, sf::Vector2i &adjust)
{
	adjust = Vector2i(0, 0);
	Vector2i prim;
	if (IsCloseToPrimary(p0, p1, prim))
	{
		if (prim.x != 0 )
		{
			adjust.y = p1.y - p0.y;
		}
		else if (prim.y != 0)
		{
			adjust.x = p1.x - p0.x;
		}
		return true;
	}
	return false;
}

bool EditSession::TryGateAdjustAction( GateAdjustOption option,
	GateInfo *gi, Vector2i &adjust, CompoundAction *compound)
{
	Action *adjustAction = NULL;
	bool success = false;
	switch (option)
	{
	case GATEADJUST_A:
		assert(gi->poly0 != gi->poly1);
		success = TryGateAdjustActionPoly( gi, adjust, true, gi->poly0,compound);
		break;
	case GATEADJUST_B:
		assert(gi->poly0 != gi->poly1);
		success = TryGateAdjustActionPoly( gi, -adjust, false, gi->poly1,compound);
		break;
	case GATEADJUST_MIDDLE:
		break;
	case GATEADJUST_POINT_A:
	{
		success = TryGateAdjustActionPoint(gi, adjust, true,compound);
		break;
	}
	case GATEADJUST_POINT_B:
	{
		success = TryGateAdjustActionPoint(gi, -adjust, false,compound);
		break;
	}
	case GATEADJUST_POINT_MIDDLE:
		break;
	}
	

	return success;
}

bool EditSession::TryGateAdjustActionPoly( GateInfo *gi, sf::Vector2i &adjust, bool a, PolyPtr p, CompoundAction *compound)
{
	Brush b;
	b.AddObject(p);

	MoveBrushAction *action = new MoveBrushAction(&b, adjust, true, NULL, RailPointMap());
	action->Perform();

	//check for validity
	if (true)//action->moveValid)
	{
		compound->subActions.push_back(action);

		Brush attachedPolys;
		PolyPtr p0, p1;
		for (auto it = gates.begin(); it != gates.end(); ++it)
		{
			if ((*it) == gi)
				continue;

			p0 = (*it)->poly0;
			p1 = (*it)->poly1;

			if ( p0 == p && p1 == p )
			{
				continue;
			}
			else if (p0 == p)
			{
				attachedPolys.AddObject(p1);
			}
			else if( p1 == p )
			{
				attachedPolys.AddObject(p0);
			}
		}


		for (auto it = gates.begin(); it != gates.end(); ++it)
		{
			if ((*it) == gi)
				continue;

			bool gateAttachedToAffectedPoly = false;
			PolyPtr poly;
			bool a = true;
			if (IsGateAttachedToAffectedPoly((*it),&attachedPolys,a))
			{
				GateInfo *gi = (*it);
				Vector2i adjust;
				Vector2i pA, pB;

				GateAdjustOption gaOption;

				if (a)
				{
					gaOption = GATEADJUST_B;
				}
				else
				{
					gaOption = GATEADJUST_A;
				}


				if (GetPrimaryAdjustment(gi->point0->pos, gi->point1->pos, adjust))
				{
					if (!TryGateAdjustAction(gaOption, gi, adjust, compound))
					{
						return false;
					}
				}
			}
		}
		return true;


		compound->subActions.push_back(action);
	}
	else
	{
		action->Undo();
		delete action;
		return false;
	}

	//check if its okay!

	return true;
}

bool EditSession::TryGateAdjustActionPoint( GateInfo *gi, Vector2i &adjust, bool a, CompoundAction *compound)
{
	PointMover * pmap = new PointMover;;
	PolyPtr poly;
	TerrainPoint *point;

	if (a)
	{
		poly = gi->poly0;
		point = gi->point0;
	}
	else
	{
		poly = gi->poly1;
		point = gi->point1;
	}

	PointMoveInfo pi;
	pi.poly = poly;
	pi.pointIndex = point->index;
	pi.origPos = point->pos;
	pi.newPos = point->pos + adjust;
	pmap->movePoints[poly].push_back(pi);


	//need to add enemies here. just this for now.


	//pmap->movePoints[poly].push

	//auto &pVec = pmap.myMap[poly];

	////need to store enemy stuff here...

	//int polyNumP = poly->GetNumPoints();

	//pVec.reserve(polyNumP);

	//TerrainPoint *polyCurr;
	//for (int i = 0; i < polyNumP; ++i)
	//{
	//	polyCurr = poly->GetPoint(i);

	//	PointMoveInfo pi;
	//	pi.poly = poly;
	//	pi.pointIndex = i;
	//	pi.origPos = polyCurr->pos;
	//	if (polyCurr == point)
	//	{
	//		pi.newPos = polyCurr->pos + adjust; //delta
	//		pi.moveIntent = true;
	//	}
	//	else
	//	{
	//		pi.newPos = polyCurr->pos;
	//	}

	//	pVec.push_back(pi);
	//}
	
	MoveBrushAction * action = new MoveBrushAction(selectedBrush, Vector2i(), true, pmap, RailPointMap());
	action->performed = true;
	//action->Perform();

	//check validity here

	if (true)//action->moveValid)
	{
		compound->subActions.push_back(action);

		for (auto it = gates.begin(); it != gates.end(); ++it)
		{
			if ((*it) == gi)
				continue;

			bool gateAttachedToAffectedPoly = false;
			PolyPtr poly;
			bool a = true;
			if (IsGateAttachedToAffectedPoints((*it), pmap, a))
			{
				GateInfo *gi = (*it);
				Vector2i adjust;
				Vector2i pA, pB;

				GateAdjustOption gaOption;

				if (a)
				{
					gaOption = GATEADJUST_POINT_B;
				}
				else
				{
					gaOption = GATEADJUST_POINT_A;
				}


				if (GetPrimaryAdjustment(gi->point0->pos, gi->point1->pos, adjust))
				{
					if (!TryGateAdjustAction(gaOption, gi, adjust, compound))
					{
						return false;
					}
				}
			}
		}
		return true;
	}
	else
	{
		action->Undo();
		delete action;
		return false;
	}
}

void EditSession::GetNearPrimaryGateList(PointMap &pmap, list<GateInfoPtr> & gList)
{
	//this doesnt work because the gate hasnt been assigned to the points yet.
	Vector2i prim;
	for (auto it = pmap.begin(); it != pmap.end(); ++it)
	{
		for (auto pit = (*it).second.begin(); pit != (*it).second.end(); ++pit)
		{
			GateInfo *gi = (*pit).GetPolyPoint()->gate;
			if (gi != NULL)
			{
				if (IsCloseToPrimary(gi->point0->pos, gi->point1->pos, prim))
				{
					gList.push_back((*pit).GetPolyPoint()->gate);
				}
			}
		}
	}
}

bool EditSession::IsEnemyValid(ActorPtr actor)
{
	auto &polyList = GetCorrectPolygonList(0);
	for (auto it = polyList.begin(); it != polyList.end(); ++it)
	{
		if ((*it)->IntersectsActorParams(actor))
		{
			return false;
		}
	}

	return true;
}

bool EditSession::IsPolygonExternallyValid( PolyPtr poly, PolyPtr ignore )
{
	list<PolyPtr> intersections;
	GetIntersectingPolys(poly, intersections);
	if (intersections.size() > 0)
		return false;

	if (PolyContainsPolys(poly, ignore ))
	{
		return false;
	}

	if (PolyIsContainedByPolys(poly, ignore ))
	{
		return false;
	}

	if (PolyIsTouchingEnemiesOrBeingTouched(poly, ignore ))
	{
		return false;
	}

	if (PolyIntersectsGates(poly))
	{
		return false;
	}

	if (PolyGatesIntersectOthers(poly))
	{
		return false;
	}

	if (PolyGatesMakeSliverAngles(poly))
	{
		return false;
	}
	return true;
}

bool EditSession::IsPolygonValid( PolyPtr poly, PolyPtr ignore )
{
	bool a = IsPolygonExternallyValid(poly, ignore);
	bool b = poly->IsInternallyValid();

	return a && b;
}

bool EditSession::ConfirmationPop( const std::string &question )
{

	confirm->labels["question"]->setString( question );

	confirmChoice = ConfirmChoices::NONE;

	window->setView( v );
	
	//preScreenTex->setView( uiView );	
	
	preScreenTex->display();
	const Texture &preTex = preScreenTex->getTexture();
	Sprite preTexSprite( preTex );
	preTexSprite.setPosition( -960 / 2, -540 / 2 );
	preTexSprite.setScale( .5, .5 );	

	preScreenTex->setView( uiView );

	sf::Event ev;
	while( confirmChoice == ConfirmChoices::NONE )
	{
		Vector2i pPos = GetPixelPos();
		Vector2f uiMouse = preScreenTex->mapPixelToCoords(pPos);
		window->clear();
		while( window->pollEvent( ev ) )
		{
			switch( ev.type )
			{
			case Event::MouseButtonPressed:
				{
					if( ev.mouseButton.button == Mouse::Left )
					{
						confirm->Update( true, false, uiMouse.x, uiMouse.y );		
					}			
					break;
				}
			case Event::MouseButtonReleased:
				{
					confirm->Update( false, false, uiMouse.x, uiMouse.y );
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

		window->setView( v );

		window->draw( preTexSprite );

		window->setView( uiView );

		confirm->Draw(window);

		window->setView( v );

		window->display();
		//preScreenTex->display();
	}

	preScreenTex->setView( view );
	window->setView( v );
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
	window->setView( v );
	
	preScreenTex->display();
	const Texture &preTex = preScreenTex->getTexture();
	Sprite preTexSprite( preTex );
	preTexSprite.setPosition( -960 / 2, -540 / 2 );
	preTexSprite.setScale( .5, .5 );	

	preScreenTex->setView( uiView );

	sf::Event ev;
	while( !closePopup )
	{
		Vector2i pPos = GetPixelPos();
		Vector2f uiMouse = preScreenTex->mapPixelToCoords(pPos);
		window->clear();

		while(window->pollEvent( ev ) )
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

		window->setView( v );

		window->draw( preTexSprite );

		window->setView( uiView );

		messagePopup->Draw(window);

		window->setView( v );

		window->display();
	}

	preScreenTex->setView( view );
	window->setView( v );
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
	else if (type == "shardselector")
	{
		panel = shardSelectPopup;//types["shard"]->panel;
	}

	assert( panel != NULL );
	int selectedIndex = -1;
	tempGridResult = "nothing";
	bool closePopup = false;
	window->setView( v );
	
	preScreenTex->display();
	const Texture &preTex = preScreenTex->getTexture();
	Sprite preTexSprite( preTex );
	preTexSprite.setPosition( -960 / 2, -540 / 2 );
	preTexSprite.setScale( .5, .5 );	


	preScreenTex->setView( uiView );

	Vector2i pPos = Vector2i(960, 540) - Vector2i( panel->size.x / 2, panel->size.y / 2 );
	pPos.x *= 1920 / window->getSize().x;
	pPos.y *= 1920 / window->getSize().y;

	Vector2f uiMouse = preScreenTex->mapPixelToCoords(pPos);



	panel->pos.x = uiMouse.x;
	panel->pos.y = uiMouse.y;

	sf::Event ev;
	while( !closePopup )
	{
		pPos = GetPixelPos();
		uiMouse = preScreenTex->mapPixelToCoords(pPos);
		window->clear();

		bool shardClose = tempGridResult == "shardclose";
		if (panel != shardSelectPopup)
		{
			if (tempGridResult != "nothing")
			{
				return;
			}
		}
		else
		{
			if (tempGridResult == "shardclose")
			{
				return;
			}
		}

		while(window->pollEvent( ev ) )
		{
			switch( ev.type )
			{
			case Event::MouseButtonPressed:
				{
					if( ev.mouseButton.button == Mouse::Left )
					{
						cout << "are we here: " << uiMouse.x << ", " << uiMouse.y << endl;
						panel->Update( true, false, uiMouse.x, uiMouse.y );
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
						panel->Update( false, false, uiMouse.x, uiMouse.y );
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

		window->setView( v );

		window->draw( preTexSprite );

		window->setView( uiView );

		//messagePopup->Draw( w );
		panel->Draw(window);

		window->setView( v );

		window->display();
	}

	preScreenTex->setView( view );
	window->setView( v );
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
	else if (type == "shardselector")
	{
		p = new Panel("shardselector", 700, 1080, this);
		p->AddLabel("shardtype", Vector2i(20, 900), 24, "SHARD_W1_TEACH_JUMP");
		CreateShardGridSelector(p, Vector2i(0, 0));
		p->AddButton("ok", Vector2i(100, 1000), Vector2f(100, 50), "OK");
	}

	if( p != NULL )
		allPopups.push_back(p);

	return p;
}

//-1 means you denied it, 0 means it didnt work, and 1 means it will work
int EditSession::IsRemovePointsOkay()
{
	/*bool terrainOkay = true;
	for( PointMap::iterator it = selectedPoints.begin(); it != selectedPoints.end(); ++it )
	{
		PolyPtr tp = (*it).first;
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
	}*/

	//return 1;
	return 0;
}

Panel * EditSession::CreateOptionsPanel( const std::string &name )
{
	Panel *p = NULL;
	
	if( name == "map" )
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
	else if (name == "rail")
	{
		p = new Panel("rail_options", 200, 600, this);
		p->AddButton("ok", Vector2i(100, 500), Vector2f(100, 50), "OK");
		p->AddLabel( "requirepower", Vector2i( 20, 50), 20, "require power:" );
		p->AddCheckBox("requirepower", Vector2i(20, 100));
		p->AddLabel("accelerate", Vector2i(20, 150), 20, "accelerate:");
		p->AddCheckBox("accelerate", Vector2i(20, 200));
		p->AddButton("reverse", Vector2i(20, 300), Vector2f(100, 50), "Reverse Dir");
		p->AddTextBox("level", Vector2i(20, 400), 200, 20, "");
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
	SelectPtr sp = selectedBrush->objects.front();
	assert( sp->selectableType == ISelectable::ACTOR );
	ActorParams *ap = (ActorParams*)sp;
	
	ActorType *type = ap->type;
	string name = type->info.name;

	Panel *p = type->panel;

	ap->SetPanelInfo();

	showPanel = p;
}

void EditSession::SetDecorEditPanel()
{
	SelectPtr sp = selectedBrush->objects.front();
	assert(sp->selectableType == ISelectable::IMAGE);
	EditorDecorInfo *di = (EditorDecorInfo*)sp;

	editDecorPanel->textBoxes["xpos"]->text.setString(boost::lexical_cast<string>(di->spr.getPosition().x));
	editDecorPanel->textBoxes["ypos"]->text.setString(boost::lexical_cast<string>(di->spr.getPosition().y));

	editDecorPanel->textBoxes["rotation"]->text.setString(boost::lexical_cast<string>(di->spr.getRotation()));
	editDecorPanel->textBoxes["xscale"]->text.setString(boost::lexical_cast<string>(di->spr.getScale().x));
	editDecorPanel->textBoxes["yscale"]->text.setString(boost::lexical_cast<string>(di->spr.getScale().y));
	
	editDecorPanel->textBoxes["layer"]->text.setString(boost::lexical_cast<string>(di->layer));
}

void EditSession::SetDecorParams()
{
	SelectPtr sp = selectedBrush->objects.front();
	assert(sp->selectableType == ISelectable::IMAGE);
	EditorDecorInfo *di = (EditorDecorInfo*)sp;
	
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
	//if (testGate.poly0 == NULL || testGate.poly1 == NULL)
	//	return false;
	//this function can later be moved into IsGateValid and cleaned up

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
			|| testGate.vertexIndex0 == 0 && testGate.vertexIndex1 == testGate.poly1->GetNumPoints()
			|| testGate.vertexIndex1 == 0 && testGate.vertexIndex0 == testGate.poly1->GetNumPoints() )
		{
			return false;
		}
	}
	
	//get aabb, check intersection with polygons. check line intersections with those polygons

	int left = min( v0.x, v1.x );
	int right = max( v0.x, v1.x );
	int top = min( v0.y, v1.y );
	int bot = max( v0.y, v1.y );

	TerrainPoint *curr, *prev;
	int numP;
	for( list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it )
	{
		//aabb collide
		if (left <= (*it)->right && right >= (*it)->left && top <= (*it)->bottom && bot >= (*it)->top)
		{
			numP = (*it)->GetNumPoints();
			for (int i = 0; i < numP; ++i)
			{
				curr = (*it)->GetPoint(i);
				prev = (*it)->GetPrevPoint(i);

				Vector2i prevPos = prev->pos;
				Vector2i pos = curr->pos;

				if (prevPos == v0 || prevPos == v1 || pos == v0 || pos == v1)
				{
					continue;
				}

				//LineIntersection li = LimitSegmentIntersect(prevPos, pos, v0, v1);
				LineIntersection li = SegmentIntersect(prevPos, pos, v0, v1);

				if (!li.parallel)
				{
					return false;
				}
			}
		}
	}

	//make sure its not within a single polygon and cutting through the middle of it.
	Vector2f center(Vector2f(v0 + v1) / 2.f);
	if (testGate.poly0 == testGate.poly1)
	{
		if (testGate.poly0->ContainsPoint(center))
		{
			return false;
		}
	}


	return true;
}



void EditSession::CreateActor(ActorPtr actor)
{
	Brush b;
	b.AddObject(actor);
	Action * action = new ApplyBrushAction(&b);
	action->Perform();
	AddDoneAction(action);
}

void EditSession::CreateDecorImage(DecorPtr dec)
{
	Brush b;
	b.AddObject(dec);
	Action * action = new ApplyBrushAction(&b);
	action->Perform();
	AddDoneAction(action);
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
		for (auto it = polygons.begin(); it != polygons.end(); ++it)
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
			if ((*it2)->type->info.name == "poi" )
			{
				PoiParams *pp = (PoiParams*)((*it2));
				if (pp->name == "stormceiling")
				{
					top = pp->GetPosition().y;
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
		if (ts_polyShaders[i] != NULL)
		{
			polyShaders[i].setUniform("zoom", zoom);
			polyShaders[i].setUniform("topLeft", botLeft);
			//just need to change the name topleft  to botleft eventually
		}

	}

	oldShaderZoom = -1; //updates the shader back to normal after this is over

	mapPreviewTex->clear(Color::Black);

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

	//mapPreviewTex->draw(*tempva);ke
	//delete tempva;

	for( auto it = polygons.begin(); it != polygons.end(); ++it )
	{
		(*it)->Draw( false, 1, mapPreviewTex, false, NULL );
	}

	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		//(*it)->Draw(1, false, mapPreviewTex);
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
			/*if ((*it2)->type->IsGoalType())
			{
				goalCS.setPosition((*it2)->GetFloatPos());
				mapPreviewTex->draw(goalCS);
			}
			else
			{
				cs.setPosition((*it2)->GetFloatPos());
				mapPreviewTex->draw(cs);
			}*/

			(*it2)->DrawPreview(mapPreviewTex);
			
				
		}

			
		//(*it).second->DrawPreview( mapPreviewTex );
	}

	cs.setPosition(player->GetFloatPos());
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
	ssPrev << filePath.parent_path().relative_path().string() << "/Previews/" << filePath.stem().string() << "_preview_" << imageSize.x << "x" << imageSize.y << ".png";
	std::string previewFile = ssPrev.str();
	img.saveToFile( previewFile );
	//currentFile
}

//needs cleanup badly
PositionInfo EditSession::ConvertPointToGround( sf::Vector2i testPoint, ActorPtr a )
{
	

	PositionInfo gi;

	
	
	//PolyPtr poly = NULL;
	gi.ground = NULL;
	gi.railGround = NULL;

	if (mode == CREATE_ENEMY && enemySelectPanel->ContainsPoint(Vector2i(uiMousePos)))
	{
		//cout << "uipos: " << uiMousePos.x << ", " << uiMousePos.y << endl;
		return gi;
	}

	bool contains;

	TerrainPoint *curr, *prev;
	int numP;

	//assumes singleactor only
	//assert(IsSingleActorSelected());
	//ActorPtr a = selectedBrush->objects.front()->GetAsActor();

	//int width = type->info.size.x;
	//int height = type->info.size.y;
	IntRect actorAABB(a->GetAABB());//a->GetGrabAABB());
	if (actorAABB.width > actorAABB.height)
	{
		int diff = actorAABB.width - actorAABB.height;
		actorAABB.top -= diff / 2;
		actorAABB.height += diff;
	}
	else if (actorAABB.height > actorAABB.width)
	{
		int diff = actorAABB.height - actorAABB.width;
		actorAABB.left -= diff / 2;
		actorAABB.width += diff;
	}

	Edge *edge;

	Vector2i actorSize = a->GetSize();


	double testRadius = actorSize.y * ( 2.0 / 3.0 );//actorAABB.width / 3;//a->type->info.size.y /2;//actorAABB.height;//200
	//testPoint = a->GetIntPos();

	double minQuant = actorSize.x / 2;
	double extra = actorSize.x;

	for( auto it = polygons.begin(); it != polygons.end(); ++it )
	{
		bool pointInPoly = (*it)->ContainsPoint(Vector2f(testPoint.x, testPoint.y));
		//contains = (*it)->ContainsPoint(Vector2f(testPoint.x, testPoint.y));;//(*it)->Intersects(actorAABB);//true;//
		contains = (*it)->Intersects(actorAABB);

		if (contains )//(contains && !(*it)->inverse) || (!contains && (*it)->inverse))
		{
			//TerrainPoint *prev = (*it)->pointEnd;
			//TerrainPoint *curr = (*it)->pointStart;

			double minDistance = 10000000;
			TerrainPoint *storedEdge = NULL;
			PolyPtr storedPoly = NULL;
			double storedQuantity = 0;
							
			V2d closestPoint;

			numP = (*it)->GetNumPoints();

			for (int i = 0; i < numP; ++i)
			{
				curr = (*it)->GetPoint(i);
				prev = (*it)->GetPrevPoint(i);
				edge = (*it)->GetEdge(i);

				double dist = //abs(
					cross(
						V2d(testPoint.x - prev->pos.x, testPoint.y - prev->pos.y),
						normalize(V2d(curr->pos.x - prev->pos.x, curr->pos.y - prev->pos.y)));
				double testQuantity = dot(
					V2d(testPoint.x - prev->pos.x, testPoint.y - prev->pos.y),
					normalize(V2d(curr->pos.x - prev->pos.x, curr->pos.y - prev->pos.y)));

				V2d pr(prev->pos.x, prev->pos.y);
				V2d cu(curr->pos.x, curr->pos.y);
				V2d te(testPoint.x, testPoint.y);

				//these should only apply to single actors
				//if ( a == grabbedActor )//IsSingleActorSelected())
				if( IsSingleActorSelected())
				{
					if (testQuantity >= 0 && testQuantity < minQuant)
						testQuantity = minQuant;
					else if (testQuantity > length(cu - pr) - minQuant && testQuantity <= length(cu - pr))
					{
						testQuantity = floor(length(cu - pr) - minQuant);
					}
				}

				

				V2d newPoint(pr.x + (cu.x - pr.x) * (testQuantity / length(cu - pr)), pr.y + (cu.y - pr.y) *
					(testQuantity / length(cu - pr)));

				V2d norm = edge->Normal();
				//if( cross( worldPos - edge->v0,edge->Along()))

				/*if (!(*it)->CheckOtherSideRay(worldPos, newPoint, edge))
				{
					continue;
				}*/


				if (((dist >= 0 && dist < testRadius) || (pointInPoly && dist < 0 && dist > - 200 )) && testQuantity >= minQuant && testQuantity <= length(cu - pr) - minQuant
					&& length(newPoint - te) < length(closestPoint - te))
				{
					minDistance = dist;

					storedPoly = (*it);
					storedEdge = prev;
					storedQuantity = testQuantity;

					//storedIndex = edgeIndex;
					double l = length(cu - pr);


					closestPoint = newPoint;
				}
				else
				{
				}
			}

			//this function is ugly af and should fix later ^^^^
			if( storedPoly != NULL )
				gi.SetGround(storedPoly, storedEdge->index, storedQuantity);

			//break;
		}

		if( testPoint.x >= (*it)->left - testRadius 
			&& testPoint.x <= (*it)->right + testRadius
			&& testPoint.y >= (*it)->top - testRadius && testPoint.y <= (*it)->bottom + testRadius )
		{

		}
	}

	return gi;
}

//needs cleanup badly
PositionInfo EditSession::ConvertPointToRail(sf::Vector2i testPoint)
{
	PositionInfo gi;
	gi.railGround = NULL;
	gi.ground = NULL;

	double testRadius = 200;

	TerrainPoint *curr, *next;
	int numP;
	RailPtr rail;
	bool contains;
	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		rail = (*it);
		numP = rail->GetNumPoints();
		if (testPoint.x >= rail->left - testRadius && testPoint.x <= rail->right + testRadius
			&& testPoint.y >= rail->top - testRadius && testPoint.y <= rail->bottom + testRadius)
		{
			contains = rail->ContainsPoint(Vector2f(testPoint.x, testPoint.y), 32);

			if (contains)
			{
				V2d closestPoint;
				double minDistance = 10000000;

				TerrainPoint *storedEdge = NULL;
				TerrainRail *storedRail = NULL;
				double storedQuantity = 0;

				for (int i = 0; i < numP - 1; ++i)
				{
					curr = rail->GetPoint(i);
					next = rail->GetNextPoint(i);

					double dist = abs(
						cross(
							V2d(testPoint.x - curr->pos.x, testPoint.y - curr->pos.y),
							normalize(V2d(next->pos.x - curr->pos.x, next->pos.y - curr->pos.y))));
					double testQuantity = dot(
						V2d(testPoint.x - curr->pos.x, testPoint.y - curr->pos.y),
						normalize(V2d(next->pos.x - curr->pos.x, next->pos.y - curr->pos.y)));

					V2d pr(curr->pos.x, curr->pos.y);
					V2d cu(next->pos.x, next->pos.y);
					V2d te(testPoint.x, testPoint.y);

					V2d newPoint(pr.x + (cu.x - pr.x) * (testQuantity / length(cu - pr)), pr.y + (cu.y - pr.y) *
						(testQuantity / length(cu - pr)));
					double edgeLength = length(cu - pr);
					double newDist = length(newPoint - te);
					double closestDist = length(closestPoint - te);

					if (dist < 100 && testQuantity >= 0 && testQuantity <= edgeLength
						&& newDist < closestDist)
					{
						minDistance = dist;
						storedRail = (*it);
						storedEdge = curr;
						storedQuantity = testQuantity;

						closestPoint = newPoint;
					}
				}

				if (storedRail != NULL )
				{
					gi.SetRail(storedRail, storedEdge->index, storedQuantity);
					break;
				}
			}
		}
	}

	return gi;
}

list<PolyPtr> & EditSession::GetCorrectPolygonList(PolyPtr t)
{
	if (t->IsSpecialPoly())
	{
		return waterPolygons;
	}
	else 
	{
		return polygons;
	}
}

list<PolyPtr> & EditSession::GetCorrectPolygonList(int ind)
{
	switch (ind)
	{
	case 0:
		return polygons;
	case 1:
		return waterPolygons;
	default:
		assert(0);
		return polygons;
	}
}

list<PolyPtr> & EditSession::GetCorrectPolygonList()
{
	if (IsSpecialTerrainMode())
	{
		return waterPolygons;
	}
	else
	{
		return polygons;
	}
}

void EditSession::GetIntersectingPolys(
	PolyPtr p,
	std::list<PolyPtr> & intersections)
{
	auto &testPolygons = GetCorrectPolygonList( p );

	for( auto it = testPolygons.begin(); it != testPolygons.end(); ++it )
	{
		if (p->IsTouching((*it)))
		{
			intersections.push_back((*it));
		}
	}
}

bool EditSession::ExecuteTerrainCompletion()
{	
	if (!polygonInProgress->IsCompletionValid())
	{
		return false;
	}

	polygonInProgress->SetMaterialType(currTerrainWorld,
		currTerrainVar);

	polygonInProgress->UpdateBounds();

	bool applyOkay = true;

	int liRes;

	list<PolyPtr> intersectingPolys;
	//list<PolyPtr> containedPolys;

	bool tryMakeInverse = IsKeyPressed(Keyboard::LAlt);

	auto &testPolygons = GetCorrectPolygonList(polygonInProgress);
	for (auto it = testPolygons.begin(); it != testPolygons.end(); ++it)
	{
		if (!(*it)->inverse && (*it)->Contains(polygonInProgress))
		{
			applyOkay = false;
			polygonInProgress->ClearPoints();
			break;
		}

		if (tryMakeInverse)
		{
			liRes = polygonInProgress->LinesIntersect((*it));
			if (liRes > 0)
			{
				applyOkay = false;
				break;
			}
		}
	}

	if (!applyOkay)
	{
		//MessagePop( "polygon is invalid!!! new message" );
	}
	else
	{
		if (tryMakeInverse)
		{
			polygonInProgress->MakeInverse();
			SetInversePoly();
		}
		else
		{
			Brush orig;
			Brush result;

			list<PolyPtr> inProgress;
			inProgress.push_back(polygonInProgress);

			bool add = !(IsKeyPressed(Keyboard::LShift) || IsKeyPressed(Keyboard::RShift));
			if (add)
			{
				ExecuteTerrainMultiAdd(inProgress, orig, result);
			}
			else
			{
				ExecuteTerrainMultiSubtract(inProgress, orig, result);
			}

			if (!orig.IsEmpty() || !result.IsEmpty())
			{
				ClearUndoneActions(); //critical to have this before the deactivation

				Action *replaceAction = new ReplaceBrushAction(&orig, &result, mapStartBrush);
				replaceAction->Perform();

				AddDoneAction(replaceAction);

				polygonInProgress->ClearPoints();
			}
		}
		return true;
	}

	return false;
}

void EditSession::ExecuteRailCompletion()
{
	int numP = railInProgress->GetNumPoints();
	if (numP > 2)
	{
		//test final line
		bool valid = true;
		if (!valid)
		{
			//MessagePop( "unable to complete polygon" );
			//popupPanel = messagePopup;
			//return;
		}

		list<RailPtr>::iterator it = rails.begin();
		bool added = false;

		bool recursionDone = false;
		RailPtr currentBrush = railInProgress;

		railInProgress->UpdateBounds();
		bool applyOkay = true;
		if (!applyOkay)
		{
			MessagePop("polygon is invalid!!! new message");
		}
		else
		{
			bool empty = true;
			//eventually be able to combine rails by putting your start/end points at their starts/ends
			if (empty)
			{
				railInProgress->Finalize();

				progressBrush->Clear();

				progressBrush->AddObject(railInProgress);

				ClearUndoneActions();

				Action *action = new ApplyBrushAction(progressBrush);

				action->Perform();
				AddDoneAction(action);
				

				RailPtr newRail(new TerrainRail());
				railInProgress = newRail;
			}
			else
			{
				//eventually combine rails here

				//Action *action = ChooseAddOrSub(intersectingPolys);

				//action->Perform();
				//doneActionStack.push_back(action);

			}
		}
	}
	else if (numP <= 2 && numP > 0)
	{
		cout << "cant finalize. cant make rail" << endl;
		railInProgress->ClearPoints();
	}
}

void EditSession::SetInversePoly()
{
	polygonInProgress->FixWinding();
	//polygonInProgress->Finalize();

	polygonInProgress->FinalizeInverse();
	

	Brush orig;
	if( inversePolygon != NULL )
	{
		orig.AddObject(inversePolygon);
	}

	progressBrush->Clear();
	progressBrush->AddObject(polygonInProgress);

	ClearUndoneActions();

	Action * action = new ReplaceBrushAction( &orig, progressBrush, mapStartBrush);

	action->Perform();
	AddDoneAction(action);

	PolyPtr newPoly( new TerrainPolygon() );
	polygonInProgress = newPoly;

	progressBrush->Clear();
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

void EditSession::CreateShardGridSelector( Panel *p, sf::Vector2i &pos )
{
	int xSize = 11;
	int ySize = 2;

	GridSelector *gs = p->AddGridSelector("shardselector", pos, xSize, ySize * 7, 64, 64, true, true);
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
}

void EditSession::GetShardWorldAndIndex(int selX, int selY,
	int &w, int &li)
{
	int world = selX / 11;
	int realX = selX % 11;
	int realY = selY;

	w = world;
	li = realX + realY * 11;
}

void EditSession::PasteTerrain(Brush *b)
{
	list<PolyPtr> brushPolys;
	PolyPtr poly;
	for (auto bit = b->objects.begin(); bit != b->objects.end(); ++bit)
	{
		poly = (*bit)->GetAsTerrain();
		if (poly != NULL)
		{
			brushPolys.push_back(poly);
		}
	}

	Brush orig;
	Brush result;
	if (HoldingControl())
	{
		ExecuteTerrainMultiSubtract(brushPolys, orig, result);
	}
	else
	{
		ExecuteTerrainMultiAdd(brushPolys, orig, result);
	}

	if( !orig.IsEmpty() || !result.IsEmpty() )
	{
		ClearUndoneActions(); //critical to have this before the deactivation

		orig.Deactivate();
		result.Activate();

		if( complexPaste == NULL )
		{
			//assert(complexPaste == NULL);
			complexPaste = new ComplexPasteAction(mapStartBrush);	
			AddDoneAction(complexPaste);

			lastBrushPastePos = worldPos;
			brushRepeatDist = 20.0;
			complexPaste->SetNewest(orig, result);
			lastBrushPastePos = worldPos;
		}
		else
		{
			complexPaste->SetNewest(orig, result);
			lastBrushPastePos = worldPos;
		}
	}
}

void EditSession::AddFullPolysToBrush(
	std::list<PolyPtr> & polyList,
	std::list<GateInfoPtr> &gateInfoList,
	Brush *b)
{
	for (list<PolyPtr>::iterator it = polyList.begin(); it != polyList.end(); ++it)
	{
		AddFullPolyToBrush((*it), gateInfoList, b);
	}
}

void EditSession::AddFullPolysToBrush(
	std::set<PolyPtr> & polySet,
	std::list<GateInfoPtr> &gateInfoList,
	Brush *b)
{
	for (auto it = polySet.begin(); it != polySet.end(); ++it)
	{
		AddFullPolyToBrush((*it), gateInfoList, b);
	}
}

void EditSession::AddFullPolyToBrush(
	PolyPtr p,
	std::list<GateInfoPtr> &gateInfoList,
	Brush *b)
{
	b->AddObject(p);
	p->AddGatesToBrush(b, gateInfoList);
	p->AddEnemiesToBrush(b);
}

void EditSession::FusePathClusters(ClipperLib::Path &p, ClipperLib::Path &clipperIntersections,
	ClipperIntPointSet &fusedPoints)
{
	//std::list<ClipperLib::IntPoint> newPoints;

	bool isNewPoint;
	int pathSize = p.size();

	ClipperLib::IntPoint *curr, *prev, *next, *temp;
	int tempI;
	V2d cPos, nPos, pPos;
	for (int i = 0; i < pathSize; ++i )
	{
		curr = &p[i];
		isNewPoint = false;
		//t = AddPoint(Vector2i((*it).X, (*it).Y), false);
		for (auto intersectIt = clipperIntersections.begin(); intersectIt != clipperIntersections.end(); ++intersectIt)
		{
			if ((*intersectIt).X == curr->X && (*intersectIt).Y == curr->Y)
			{
				//newPoints.push_back((*it));
				isNewPoint = true;
				break;
			}
		}

		prev = NULL;
		next = NULL;

		if (isNewPoint)
		{
			tempI = i;
			do
			{	
				tempI = tempI - 1;
				if (tempI < 0)
					tempI = pathSize - 1;

				prev = &p[tempI];
			} 
			while (fusedPoints.find(make_pair(prev->X,prev->Y)) != fusedPoints.end());

			

			tempI = i;
			do
			{
				tempI = tempI + 1;
				if (tempI == pathSize)
					tempI = 0;

				next = &p[tempI];
			} 
			while (fusedPoints.find(make_pair(next->X,next->Y)) != fusedPoints.end());

			


			/*if (i == 0)
				prev = &p[pathSize - 1];
			else
				prev = &p[i - 1];

			if (i == pathSize - 1)
				next = &p[0];
			else
				next = &p[i + 1];*/

			cPos = V2d(curr->X, curr->Y);
			nPos = V2d(next->X, next->Y);
			pPos = V2d(prev->X, prev->Y);

			double minDist = EditSession::POINT_SIZE;
			if (length(nPos - cPos) < minDist)
			{
				fusedPoints.insert(make_pair(curr->X, curr->Y));
			}
			else if (length(cPos - pPos) < minDist)
			{
				fusedPoints.insert(make_pair(curr->X, curr->Y));
			}
			else
			{
				
			}
		}
	}

}

void EditSession::FixPathSlivers(ClipperLib::Path &p)
{
	double minAngle = EditSession::SLIVER_LIMIT;
	ClipperLib::IntPoint *curr, *prev, *next;
	//TerrainPoint *curr, *prev, *next;
	int pSize = p.size();
	for (int i = 0; i < pSize; ++i)
	{
		curr = &p[i];
		if (i == 0)
			prev = &p[pSize - 1];
		else
			prev = &p[i - 1];

		if (i == pSize - 1)
			next = &p[0];
		else
			next = &p[i + 1];

		V2d pos(curr->X, curr->Y);
		V2d prevPos(prev->X, prev->Y);
		V2d nextPos(next->X, next->Y);
		V2d dirA = normalize(prevPos - pos);
		V2d dirB = normalize(nextPos - pos);

		double diff = GetVectorAngleDiffCCW(dirA, dirB);
		double diffCW = GetVectorAngleDiffCW(dirA, dirB);
		if (diff < minAngle)
		{
			Vector2i trimPos = TerrainPolygon::TrimSliverPos(prevPos, pos, nextPos, minAngle, true);
			curr->X = trimPos.x;
			curr->Y = trimPos.y;

			/*if (curr->pos == prev->pos || curr->pos == next->pos)
			{
				int b = 6;
				assert(0);
			}*/
		}
		else if (diffCW < minAngle)
		{
			Vector2i trimPos = TerrainPolygon::TrimSliverPos(prevPos, pos, nextPos, minAngle, false);
			curr->X = trimPos.x;
			curr->Y = trimPos.y;

			/*if (curr->pos == prev->pos || curr->pos == next->pos)
			{
				int b = 6;
				assert(0);
			}*/
		}
	}
}

//returns true is success. returns false if poly is invalid
bool EditSession::FixPathSlivers(ClipperLib::Path &p,
	ClipperIntPointSet &fusedPoints)
{
	double minAngle = EditSession::SLIVER_LIMIT;
	ClipperLib::IntPoint *curr, *prev, *next;
	//TerrainPoint *curr, *prev, *next;
	int pSize = p.size();
	int temp;
	for (int i = 0; i < pSize; ++i)
	{
		curr = &p[i];
		if (fusedPoints.find(make_pair(curr->X, curr->Y)) != fusedPoints.end())
		{
			continue;
		}

		prev = NULL;
		for (int j = 1; j < pSize; ++j)
		{
			temp = i - j;
			if (temp < 0)
			{
				temp += pSize;
			}
			prev = &p[temp];
			if (fusedPoints.find(make_pair(prev->X, prev->Y)) != fusedPoints.end())
			{
				prev = NULL;
			}
			else
			{
				break;
			}
		}

		if (prev == NULL)
		{
			return false;
			//assert(prev != NULL);
		}
		

		next = NULL;
		for (int j = 1; j < pSize; ++j)
		{
			temp = i + j;
			if (temp >= pSize)
			{
				temp -= pSize;
			}
			next = &p[temp];

			if (fusedPoints.find(make_pair(next->X, next->Y)) != fusedPoints.end())
			{
				next = NULL;
			}
			else
			{
				break;
			}
		}

		if (next == NULL)
		{
			return false;
			//assert(next != NULL);
		}
		


		V2d pos(curr->X, curr->Y);
		V2d prevPos(prev->X, prev->Y);
		V2d nextPos(next->X, next->Y);
		V2d dirA = normalize(prevPos - pos);
		V2d dirB = normalize(nextPos - pos);

		double diff = GetVectorAngleDiffCCW(dirA, dirB);
		double diffCW = GetVectorAngleDiffCW(dirA, dirB);

		if (dirA == -dirB) //works if going opposite directions or the same point. haven't tested with add, only sub
		{
			//int xxx = 5;
			fusedPoints.insert(make_pair(curr->X, curr->Y));
		}
		else if (diff < minAngle)
		{
			Vector2i trimPos = TerrainPolygon::TrimSliverPos(prevPos, pos, nextPos, minAngle, true);
			curr->X = trimPos.x;
			curr->Y = trimPos.y;

			/*if (curr->pos == prev->pos || curr->pos == next->pos)
			{
			int b = 6;
			assert(0);
			}*/
		}
		else if (diffCW < minAngle)
		{
			Vector2i trimPos = TerrainPolygon::TrimSliverPos(prevPos, pos, nextPos, minAngle, false);
			curr->X = trimPos.x;
			curr->Y = trimPos.y;

			/*if (curr->pos == prev->pos || curr->pos == next->pos)
			{
			int b = 6;
			assert(0);
			}*/
		}
	}

	return true;
}

bool EditSession::ExecuteTerrainMultiSubtract(list<PolyPtr> &brushPolys,
	Brush &orig, Brush &resultBrush)
{
	//change this eventually to reflect the actual layer. maybe pass in which layer im on?
	auto &testPolygons = GetCorrectPolygonList(polygonInProgress);
	bool removeBrush;
	int liRes;

	list<PolyPtr> inverseBrushes;
	map<PolyPtr, list<PolyPtr>> nonInverseIntersections;
	list<PolyPtr> tempContained;
	list<PolyPtr> containedPolys;
	int i;


	//Brush orig;
	//Brush resultBrush;
	list<GateInfoPtr> gateInfoList;

	ClipperIntPointSet fusedPoints;
	list<PolyPtr> attachList;
	ClipperLib::Clipper c;

	ClipperLib::Paths solution;
	ClipperLib::Path clipperIntersections;

	bool sliverResult;

	for (auto brushIt = brushPolys.begin(); brushIt != brushPolys.end();)
	{
		removeBrush = false;
		tempContained.clear();
		//(*brushIt)->isBrushTest = true;

		for (auto it = testPolygons.begin(); it != testPolygons.end(); ++it)
		{
			liRes = (*brushIt)->LinesIntersect((*it));
			if (liRes == 2)
			{
				if ((*it)->inverse)
				{
					inverseBrushes.push_back((*brushIt));
				}
				else
				{
					nonInverseIntersections[(*it)].push_back((*brushIt));
				}
			}
			/*else if (liRes == 1)
			{
				removeBrush = true;
				break;
			}*/
			else if( liRes == 0 )
			{
				//only check contains when there are no line intersections
				if ((*it)->Contains((*brushIt)))
				{
					removeBrush = true;
					break;
				}

				if ((*brushIt)->Contains((*it)))
				{
					tempContained.push_back((*it));
				}
			}
		}

		if (removeBrush)
		{
			brushIt = brushPolys.erase(brushIt);
		}
		else
		{
			for (auto tempIt = tempContained.begin(); tempIt != tempContained.end(); ++tempIt)
			{
				//cant be a duplicate only because brushes won't be on top of one another.
				containedPolys.push_back((*tempIt));
			}
			++brushIt;
		}
	}

	list<PolyPtr> nonInverseIntersList;
	for (auto it = nonInverseIntersections.begin(); it != nonInverseIntersections.end(); ++it)
	{
		nonInverseIntersList.push_back((*it).first);

		c.Clear();
		solution.clear();
		clipperIntersections.clear();
		fusedPoints.clear();

		ClipperLib::Path p;

		(*it).first->CopyPointsToClipperPath(p);
		c.AddPath(p, ClipperLib::PolyType::ptSubject, true);

		i = 0;
		ClipperLib::Paths pBrushes((*it).second.size());
		for (auto pit = (*it).second.begin(); pit != (*it).second.end(); ++pit)
		{
			(*pit)->CopyPointsToClipperPath(pBrushes[i]);
			(*pit)->CopyPointsToClipperPath(clipperIntersections);
			++i;
		}
		c.AddPaths(pBrushes, ClipperLib::PolyType::ptClip, true);

		c.Execute(ClipperLib::ClipType::ctDifference, solution, ClipperLib::PolyFillType::pftEvenOdd);

		ClipperLib::Path &intersectPath = c.GetIntersectPath();
		clipperIntersections.reserve(clipperIntersections.size() + intersectPath.size());
		clipperIntersections.insert(clipperIntersections.end(), intersectPath.begin(), intersectPath.end());

		for (auto sit = solution.begin(); sit != solution.end(); ++sit)
		{
			PolyPtr newPoly(new TerrainPolygon);
			//FusePathClusters((*sit), clipperIntersections, fusedPoints);
			//sliverResult = FixPathSlivers((*sit), fusedPoints);
			
			/*if (!sliverResult)
			{
				newPoly.reset();
				continue;
			}*/

			//FusePathClusters((*sit), clipperIntersections, fusedPoints);

			newPoly->Reserve((*sit).size());
			newPoly->AddPointsFromClipperPath((*sit), fusedPoints);

			if (newPoly->GetNumPoints() < 3)
			{
				delete newPoly;
				continue;
			}

			if (!newPoly->TryFixAllSlivers())
			{
				delete newPoly;
				continue;
			}

			//just for debugging

			if (newPoly->GetNumPoints() < 3)
			{
				delete newPoly;
				continue;
			}
			//newPoly->RemoveSlivers();

			//commented out for paste testing
			newPoly->AlignExtremes();

			//pretty sure this will never happen ever with subtract..breakpoint to check.
			if (!newPoly->IsClockwise())
			{
				//assert(0);
				delete newPoly;
				continue;
			}

			if (newPoly->LinesIntersectMyself())
			{
				newPoly->TryFixPointsTouchingLines();
				//assert(0);
			}

			newPoly->SetMaterialType((*it).first->terrainWorldType, (*it).first->terrainVariation);
			newPoly->Finalize();

			resultBrush.AddObject(newPoly);
			attachList.push_back(newPoly);
		}
		

		AddFullPolyToBrush((*it).first, gateInfoList, &orig);
		TryAttachActorsToPoly(inversePolygon, attachList, &resultBrush);
	}

	if (inverseBrushes.size() > 0)
	{
		c.Clear();
		solution.clear();
		clipperIntersections.clear();
		fusedPoints.clear();

		ClipperLib::Path inversePath;
		inversePolygon->CopyPointsToClipperPath(inversePath);
		
		AddFullPolyToBrush(inversePolygon, gateInfoList, &orig);

		ClipperLib::Paths brushPaths( inverseBrushes.size() );
		i = 0;
		for (auto it = inverseBrushes.begin(); it != inverseBrushes.end(); ++it)
		{
			(*it)->CopyPointsToClipperPath(brushPaths[i]);
			++i;
		}

		c.AddPath(inversePath, ClipperLib::PolyType::ptSubject, true);
		c.AddPaths(brushPaths, ClipperLib::PolyType::ptClip, true);
		c.Execute(ClipperLib::ClipType::ctUnion, solution, ClipperLib::PolyFillType::pftEvenOdd);


		list<PolyPtr> inverseResults;
		for (auto it = solution.begin(); it != solution.end(); ++it)
		{
			PolyPtr newPoly = new TerrainPolygon;
			newPoly->Reserve((*it).size());
			newPoly->AddPointsFromClipperPath((*it));// , fusedPoints);

			if (newPoly->GetNumPoints() < 3)
			{
				delete newPoly;
				continue;
			}

			if (!newPoly->TryFixAllSlivers())
			{
				delete newPoly;
				continue;
			}

			//just for debugging

			if (newPoly->GetNumPoints() < 3)
			{
				delete newPoly;
				continue;
			}
			//newPoly->RemoveSlivers();

			//commented out for paste testing
			newPoly->AlignExtremes();

			//if (!newPoly->IsClockwise())
			//{
			//	//assert(0);
			//	delete newPoly;
			//	continue;
			//}

			if (newPoly->LinesIntersectMyself())
			{
				newPoly->TryFixPointsTouchingLines();
				//assert(0);
			}

			newPoly->SetMaterialType(inversePolygon->terrainWorldType,
				inversePolygon->terrainVariation);

			inverseResults.push_back(newPoly);
			resultBrush.AddObject(newPoly);
			attachList.push_back(newPoly);
		}

		//figure out which polygon should be the new inverse polygon
		bool isOuter;
		for (auto it = inverseResults.begin(); it != inverseResults.end(); ++it)
		{
			isOuter = true;
			for (auto it2 = inverseResults.begin(); it2 != inverseResults.end(); ++it2)
			{
				if ((*it) == (*it2))
					continue;

				if (!(*it)->Contains((*it2)))
				{
					isOuter = false;
				}
			}

			if (isOuter)
			{
				(*it)->MakeInverse();
				break;
			}
		}

		

		for (auto it = inverseResults.begin(); it != inverseResults.end(); ++it)
		{
			(*it)->Finalize();
		}

		
	}


	AddFullPolysToBrush(containedPolys, gateInfoList, &orig);
	//AddFullPolysToBrush(nonInverseIntersList, gateInfoList, &orig);
	
	
	TryAttachActorsToPolys(nonInverseIntersList, attachList, &resultBrush);
	//TryAttachActorsToPolys(inverseConnectedInters, attachList, &resultBrush);

	TryKeepGates(gateInfoList, attachList, &resultBrush);
	
	return true;
}

bool EditSession::ExecuteTerrainMultiAdd(list<PolyPtr> &brushPolys,
	Brush &orig, Brush &resultBrush)
{
	//change this eventually to reflect the actual layer. maybe pass in which layer im on?
	auto &testPolygons = GetCorrectPolygonList(polygonInProgress);
	
	list<PolyPtr> nonIntersectingBrushes;
	list<PolyPtr> nonInverseBrushes;
	list<PolyPtr> inverseOnlyBrushes;
	set<PolyPtr> nonInverseInters;
	list<PolyPtr> containedPolys;
	set<PolyPtr> inverseConnectedPolys;
	list<PolyPtr> inverseConnectedInters;

	bool removeBrush;
	int liRes;

	int numIntersections;
	bool intersectsInverse;

	list<PolyPtr> tempIntersections;
	list<PolyPtr> tempContained;

	list<list<PolyPtr>> allIntersectsList;

	//Brush orig;
	//Brush resultBrush;

	list<GateInfoPtr> gateInfoList;

	ClipperLib::Paths solution;
	ClipperLib::Path clipperIntersections;
	ClipperLib::Clipper c;
	int i;
	ClipperIntPointSet fusedPoints;
	list<PolyPtr> attachList;
	bool found;

	//===PREPARATION STEP===//
	//get rid of brush polys that are contained by existent polys
	for (auto brushIt = brushPolys.begin(); brushIt != brushPolys.end();)
	{
		removeBrush = false;
		intersectsInverse = false;
		numIntersections = 0;
		tempIntersections.clear();
		tempContained.clear();
		(*brushIt)->isBrushTest = true;

		for (auto it = testPolygons.begin(); it != testPolygons.end(); ++it)
		{
			(*it)->isBrushTest = false;

			liRes = (*brushIt)->LinesIntersect((*it));
			if (liRes == 2)
			{
				if ((*it)->inverse)
				{
					intersectsInverse = true;
				}
				else
				{
					tempIntersections.push_back((*it));
				}
				++numIntersections;
			}
			else if (liRes == 1)
			{
				removeBrush = true;
				break;
			}
			else
			{
				//only check contains when there are no line intersections
				if ((*it)->Contains((*brushIt)))
				{
					removeBrush = true;
					break;
				}

				if ((*brushIt)->Contains((*it)))
				{
					tempContained.push_back((*it));
				}
			}
		}

		if (removeBrush)
		{
			brushIt = brushPolys.erase(brushIt);
		}
		else
		{
			if (intersectsInverse && numIntersections == 1)
			{
				inverseOnlyBrushes.push_back((*brushIt));
			}
			else if (numIntersections == 0)
			{
				nonIntersectingBrushes.push_back((*brushIt));
			}
			else
			{
				for (auto tempIt = tempContained.begin(); tempIt != tempContained.end(); ++tempIt)
				{
					//cant be a duplicate only because brushes won't be on top of one another.
					containedPolys.push_back((*tempIt));
				}

				list<PolyPtr> *tempTestList;
				if (intersectsInverse)
				{
					allIntersectsList.push_front(list<PolyPtr>());
					tempTestList = &allIntersectsList.front();
					tempTestList->push_back(inversePolygon);
				}
				else
				{
					allIntersectsList.push_back(list<PolyPtr>());
					tempTestList = &allIntersectsList.back();
				}

				tempTestList->push_back((*brushIt));
				for (auto tempIt = tempIntersections.begin(); tempIt != tempIntersections.end(); ++tempIt)
				{
					tempTestList->push_back((*tempIt));
				}
			}
			++brushIt;
		}
	}

	inverseConnectedPolys.insert(inversePolygon);
	
	//testlist is only things that are intersecting
	for (auto it = allIntersectsList.begin(); it != allIntersectsList.end(); ++it)
	{
		list<PolyPtr> &temp = (*it);
		found = false;
		for (auto it2 = temp.begin(); it2 != temp.end(); ++it2)
		{
			if (inverseConnectedPolys.find((*it2)) != inverseConnectedPolys.end())
			{
				found = true;
				break;
			}
		}

		if (found)
			for (auto it2 = temp.begin(); it2 != temp.end(); ++it2)
			{
				inverseConnectedPolys.insert((*it2));
			}
	}

	inverseConnectedPolys.erase(inverseConnectedPolys.find(inversePolygon));
	for (auto it = allIntersectsList.begin(); it != allIntersectsList.end(); ++it)
	{
		list<PolyPtr> &temp = (*it);
		for (auto it2 = temp.begin(); it2 != temp.end(); ++it2)
		{
			if ((*it2)->inverse)
				continue;

			if (inverseConnectedPolys.find((*it2)) == inverseConnectedPolys.end())
			{
				if ((*it2)->isBrushTest)
				{
					nonInverseBrushes.push_back((*it2));
				}
				else
				{
					nonInverseInters.insert((*it2));
				}
			}
		}
	}

	//===NON-INVERSE STEP===//
	if (nonInverseInters.size() > 0)
	{
		i = 0;
		ClipperLib::Paths nonInverseInterPaths(nonInverseInters.size());
		for (auto it = nonInverseInters.begin(); it != nonInverseInters.end(); ++it)
		{
			(*it)->CopyPointsToClipperPath(nonInverseInterPaths[i]);
			++i;
		}
		c.AddPaths(nonInverseInterPaths, ClipperLib::PolyType::ptSubject, true);

		i = 0;
		ClipperLib::Paths nonInverseBrushPaths(nonInverseBrushes.size());
		for (auto it = nonInverseBrushes.begin(); it != nonInverseBrushes.end(); ++it)
		{
			(*it)->CopyPointsToClipperPath(nonInverseBrushPaths[i]);
			(*it)->CopyPointsToClipperPath(clipperIntersections);
			++i;
		}
		c.AddPaths(nonInverseBrushPaths, ClipperLib::PolyType::ptClip, true);

		c.Execute(ClipperLib::ClipType::ctUnion, solution, ClipperLib::PolyFillType::pftEvenOdd);

		ClipperLib::Path &intersectPath = c.GetIntersectPath();
		clipperIntersections.reserve(clipperIntersections.size() + intersectPath.size());
		clipperIntersections.insert(clipperIntersections.end(), intersectPath.begin(), intersectPath.end());

		vector<pair<PolyPtr,bool>> finalCheckVec;
		finalCheckVec.reserve(solution.size());

		for (auto sit = solution.begin(); sit != solution.end(); ++sit)
		{
			PolyPtr newPoly(new TerrainPolygon());
			//FusePathClusters((*sit), clipperIntersections, fusedPoints);
			//FixPathSlivers((*sit));

			//haven't tested this with add yet but it makes sense. taken from new multi subtract
			//FixPathSlivers((*sit), fusedPoints);

			newPoly->Reserve((*sit).size());
			newPoly->AddPointsFromClipperPath((*sit), fusedPoints);

			if (newPoly->GetNumPoints() < 3)
			{
				delete newPoly;
				continue;
			}

			if (!newPoly->TryFixAllSlivers())
			{
				delete newPoly;
				continue;
			}

			//newPoly->RemoveSlivers(); //this is replaced by FixPathSlivers
			newPoly->AlignExtremes();

			if (!newPoly->IsClockwise())
			{
				delete newPoly;
				continue;
			}

			if (newPoly->LinesIntersectMyself())
			{
				newPoly->TryFixPointsTouchingLines();
			}

			else
			{
				finalCheckVec.push_back(make_pair(newPoly, false));
			}
			//newPoly->Finalize();
			//newPoly->SetMaterialType((*it)->terrainWorldType, (*it)->terrainVariation);
		}

		int checkVecSize = finalCheckVec.size();
		for (i = 0; i < checkVecSize; ++i)
		{
			if (finalCheckVec[i].second) //has already been contained
			{
				continue;
			}

			for (int j = 0; j < checkVecSize; ++j)
			{	
				if (i == j || finalCheckVec[j].second )
					continue;

				//dont need to check for line intersection here,
				//because these are all polygons that just got created
				if (finalCheckVec[i].first->Contains(finalCheckVec[j].first))
				{
					finalCheckVec[j].second = true;
				}
			}
		}

		for (i = 0; i < checkVecSize; ++i)
		{
			if (finalCheckVec[i].second)
			{
				delete finalCheckVec[i].first;
			}
			else
			{
				finalCheckVec[i].first->Finalize();
				resultBrush.AddObject(finalCheckVec[i].first);
				attachList.push_back(finalCheckVec[i].first);
			}
		}
	}

	// if this triggers it means something went really wrong
	assert(inverseConnectedPolys.size() != 1); 
	

	//===INVERSE STEP===//
	if (inverseConnectedPolys.size() > 1 || inverseOnlyBrushes.size() > 0)
	{
		
		solution.clear();
		fusedPoints.clear();
		clipperIntersections.clear();

		if (inverseConnectedPolys.size() > 1)
		{
			c.Clear();

			i = 0;
			ClipperLib::Paths inverseConnectPolysPaths(inverseConnectedPolys.size());
			for (auto it = inverseConnectedPolys.begin(); it != inverseConnectedPolys.end(); ++it)
			{
				(*it)->CopyPointsToClipperPath(inverseConnectPolysPaths[i]);
				if ((*it)->isBrushTest)
				{
					c.AddPath(inverseConnectPolysPaths[i], ClipperLib::PolyType::ptClip, true);
					(*it)->CopyPointsToClipperPath(clipperIntersections);
				}
				else
				{
					inverseConnectedInters.push_back((*it));
					c.AddPath(inverseConnectPolysPaths[i], ClipperLib::PolyType::ptSubject, true);
				}
				++i;
			}

			c.Execute(ClipperLib::ClipType::ctUnion, solution);

			ClipperLib::Path &intersectPath = c.GetIntersectPath();
			clipperIntersections.reserve(clipperIntersections.size() + intersectPath.size());
			clipperIntersections.insert(clipperIntersections.end(), intersectPath.begin(), intersectPath.end());

			//for (auto it = solution.begin(); it != solution.end(); ++it)
			//{
			//	FusePathClusters((*it), clipperIntersections, fusedPoints);
			//	//FixPathSlivers((*it));
			//}
		}

		ClipperLib::Paths inverseSolution;
		ClipperLib::Path inversePath;

		c.Clear();

		inversePolygon->CopyPointsToClipperPath(inversePath);

		c.AddPath(inversePath, ClipperLib::PolyType::ptSubject, true);

		if (!solution.empty())
		{
			c.AddPaths(solution, ClipperLib::PolyType::ptClip, true);
		}

		if (inverseOnlyBrushes.size() > 0)
		{
			i = 0;
			ClipperLib::Paths inverseBrushPaths(inverseOnlyBrushes.size());
			for (auto it = inverseOnlyBrushes.begin(); it != inverseOnlyBrushes.end(); ++it)
			{
				(*it)->CopyPointsToClipperPath(inverseBrushPaths[i]);
				(*it)->CopyPointsToClipperPath(clipperIntersections);
				++i;
			}
			c.AddPaths(inverseBrushPaths, ClipperLib::PolyType::ptClip, true);
		}


		c.Execute(ClipperLib::ClipType::ctDifference, inverseSolution);

		ClipperLib::Path &intersectPath = c.GetIntersectPath();
		clipperIntersections.reserve(clipperIntersections.size() + intersectPath.size());
		clipperIntersections.insert(clipperIntersections.end(), intersectPath.begin(), intersectPath.end());
		
		int playerInsideIndex = 0;
		int numInverseSolutions = inverseSolution.size();
		if ( numInverseSolutions > 1)
		{
			//this is to handle when more than 1 inverse is created and its ambiguous
			//go with the one the player is inside. if the player isn't in any of them, ignore the operation
			playerInsideIndex = -1;
			Vector2i playerIntPos(player->GetIntPos());
			ClipperLib::IntPoint clipperPlayerIntPos(playerIntPos.x, playerIntPos.y);
			for (i = 0; i < numInverseSolutions; ++i)
			{
				if (ClipperLib::PointInPolygon(clipperPlayerIntPos,
					inverseSolution[i]))
				{
					playerInsideIndex = i;
					break;
				}
			}
		}
		
		if (playerInsideIndex >= 0)
		{
			PolyPtr newInverse(new TerrainPolygon());
			newInverse->inverse = true;

			newInverse->Reserve(inverseSolution[playerInsideIndex].size());
			//FusePathClusters(inverseSolution[playerInsideIndex], clipperIntersections, fusedPoints);
			//FixPathSlivers(inverseSolution[playerInsideIndex]);

			

			newInverse->AddPointsFromClipperPath(inverseSolution[playerInsideIndex], fusedPoints);

			if (!newInverse->TryFixAllSlivers())
			{
				delete newInverse;
			}
			else if (newInverse->GetNumPoints() < 3)
			{
				//should never happen
				delete newInverse;
				assert(0);
			}
			else
			{
				//newInverse->RemoveSlivers();
				newInverse->AlignExtremes();
				newInverse->Finalize();
				//newPoly->SetMaterialType((*it)->terrainWorldType, (*it)->terrainVariation);

				AddFullPolyToBrush(inversePolygon, gateInfoList, &orig);
				resultBrush.AddObject(newInverse);
				attachList.push_back(newInverse);
			}
		}
	}

	//===NON-INTERSECTING STEP===//
	if (nonIntersectingBrushes.size() > 0)
	{
		for (auto it = nonIntersectingBrushes.begin(); it != nonIntersectingBrushes.end(); ++it)
		{
			PolyPtr newPoly((*it)->Copy());
			resultBrush.AddObject(newPoly);
			attachList.push_back(newPoly);
		}
	}

	AddFullPolysToBrush(nonInverseInters, gateInfoList, &orig);
	AddFullPolysToBrush(inverseConnectedInters, gateInfoList, &orig);
	AddFullPolysToBrush(containedPolys, gateInfoList, &orig);

	TryAttachActorsToPolys(nonInverseInters, attachList, &resultBrush);
	TryAttachActorsToPolys(inverseConnectedInters, attachList, &resultBrush);
	TryKeepGates(gateInfoList, attachList, &resultBrush);
	return true;
}

bool EditSession::PointSelectTerrain(V2d &pos)
{
	bool pointSelectKeyHeld = IsKeyPressed(Keyboard::B);

	if (pointSelectKeyHeld)
	{
		if (PointSelectPolyPoint(pos))
		{
			return true;
		}
	}
	else
	{
		if (PointSelectPoly(pos))
		{
			return true;
		}
	}

	return false;
}

bool EditSession::PointSelectPolyPoint( V2d &pos )
{
	bool shift = IsKeyPressed(Keyboard::LShift) || IsKeyPressed(Keyboard::RShift);

	auto & currPolyList = GetCorrectPolygonList();

	TerrainPoint *foundPoint = NULL;
	for (auto it = currPolyList.begin(); it != currPolyList.end(); ++it)
	{
		foundPoint = (*it)->GetClosePoint( 8 * zoomMultiple, pos);
		if (foundPoint != NULL)
		{
			if (shift && foundPoint->selected )
			{
				DeselectPoint((*it), foundPoint);
			}
			else
			{	
				if (!foundPoint->selected)
				{
					if (!shift)
						ClearSelectedPoints();

					SelectPoint((*it), foundPoint);
					grabbedPoint = foundPoint;
				}			
			}
			return true;
		}
	}
	return false;
}

bool EditSession::PointSelectRail(V2d &pos)
{
	bool pointSelectKeyHeld = IsKeyPressed(Keyboard::B);

	if (pointSelectKeyHeld)
	{
		if (PointSelectRailPoint(worldPos))
		{
			return true;
		}
	}
	else
	{
		if (PointSelectGeneralRail(worldPos))
		{
			return true;
		}
	}

	return false;
}

bool EditSession::PointSelectGeneralRail(V2d &pos)
{
	for (list<RailPtr>::iterator it = rails.begin(); it != rails.end(); ++it)
	{
		bool sel = (*it)->ContainsPoint(Vector2f(pos));
		if (sel)
		{
			if ((*it)->selected)
			{

			}
			else
			{
				if (!HoldingShift())
				{
					ClearSelectedBrush();
				}


				grabbedObject = (*it);

				SelectObject((*it));
			}

			return true;
		}
	}

	return false;
}

bool EditSession::PointSelectRailPoint(V2d &pos)
{
	bool shift = HoldingShift();

	TerrainPoint *foundPoint = NULL;
	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		foundPoint = (*it)->GetClosePoint(8 * zoomMultiple, pos);
		if (foundPoint != NULL)
		{
			if (shift && foundPoint->selected)
			{
				DeselectPoint((*it), foundPoint);
			}
			else
			{
				if (!foundPoint->selected)
				{
					if (!shift)
						ClearSelectedPoints();

					SelectPoint((*it), foundPoint);
				}
			}
			return true;
		}
	}
	return false;
}

bool EditSession::PointSelectPoly(V2d &pos)
{
	auto & currPolyList = GetCorrectPolygonList();
	for (auto it = currPolyList.begin(); it != currPolyList.end(); ++it)
	{
		//bool pressF1 = IsKeyPressed(Keyboard::F1);
		//if ((pressF1 && !(*it)->inverse) || !pressF1 && (*it)->inverse)
		//	continue;

		bool sel = (*it)->ContainsPoint(Vector2f(pos));
		/*if ((*it)->inverse)
		{
			sel = !sel;
		}*/

		if (sel)
		{
			if ((*it)->selected)
			{

			}
			else
			{
				if (!HoldingShift())
				{
					ClearSelectedBrush();
				}


				grabbedObject = (*it);
				SelectObject((*it));
			}

			return true;
		}
	}

	return false;
}

bool EditSession::BoxSelectPoints(sf::IntRect &r,
	double radius)
{
	auto & currPolyList = GetCorrectPolygonList();

	bool specialMode = IsSpecialTerrainMode();

	bool found = false;

	int numP;
	TerrainPoint *curr;
	for (list<PolyPtr>::iterator it = currPolyList.begin(); it != currPolyList.end(); ++it)
	{
		IntRect adjustedR(r.left, r.top, r.width, r.height);
		//IntRect adjustedR(r.left - radius, r.top, r.width, r.height);
		//why was this here with only the left coordinate changed?

		//aabb w/ polygon
		if ((*it)->Intersects(adjustedR))
		{
			numP = (*it)->GetNumPoints();
			for (int i = 0; i < numP; ++i)
			{
				curr = (*it)->GetPoint(i);

				if (IsQuadTouchingCircle(V2d(r.left, r.top),
					V2d(r.left + r.width, r.top),
					V2d(r.left + r.width, r.top + r.height),
					V2d(r.left, r.top + r.height),
					V2d(curr->pos.x, curr->pos.y), radius)
					|| adjustedR.contains(curr->pos))
				{
					SelectPoint((*it), curr);
					found = true;
				}
			}
		}
	}

	if (!specialMode)
	{
		for (list<RailPtr>::iterator it = rails.begin(); it != rails.end(); ++it)
		{
			IntRect adjustedR(r.left, r.top, r.width, r.height);

			if ((*it)->Intersects(adjustedR))
			{
				TerrainPoint *curr;
				int rNumP = (*it)->GetNumPoints();

				for (int i = 0; i < rNumP; ++i)
				{
					curr = (*it)->GetPoint(i);
					if (IsQuadTouchingCircle(V2d(r.left, r.top),
						V2d(r.left + r.width, r.top),
						V2d(r.left + r.width, r.top + r.height),
						V2d(r.left, r.top + r.height),
						V2d(curr->pos.x, curr->pos.y), radius)
						|| adjustedR.contains(curr->pos))
					{
						SelectPoint((*it), curr);
						found = true;
					}
				}
			}
		}
	}

	return found;
}

bool EditSession::BoxSelectActors(sf::IntRect &rect)
{
	bool found = false;
	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		for (auto ait = (*it).second->actors.begin();
			ait != (*it).second->actors.end(); ++ait)
		{
			if ((*ait)->Intersects(rect))
			{
				if (HoldingShift())
				{
					if ((*ait)->selected)
					{
						DeselectObject((*ait));
					}
					else
					{
						SelectObject((*ait));
						if ((*ait)->myEnemy != NULL)
							(*ait)->myEnemy->SetActionEditLoop();
					}
				}
				else
				{
					SelectObject((*ait));
					if ((*ait)->myEnemy != NULL)
						(*ait)->myEnemy->SetActionEditLoop();
				}


				found = true;
			}
		}
	}

	return found;
}

bool EditSession::BoxSelectDecor(sf::IntRect &rect)
{
	bool found = false;
	for (auto it = decorImagesBetween.begin(); it != decorImagesBetween.end(); ++it)
	{
		if ((*it)->Intersects(rect))
		{
			if (HoldingShift())
			{
				if ((*it)->selected)
				{
					DeselectObject((*it));
				}
				else
				{
					SelectObject((*it));
				}
			}
			else
			{
				SelectObject((*it));
			}


			found = true;
		}
	}

	return found;
}

bool EditSession::BoxSelectPolys(sf::IntRect &rect)
{
	bool found = false;

	auto & currPolyList = GetCorrectPolygonList();

	for (auto it = currPolyList.begin(); it != currPolyList.end(); ++it)
	{
		if ((*it)->Intersects(rect))
		{
			if (HoldingShift())
			{
				if ((*it)->selected)
				{
					DeselectObject((*it));
				}
				else
				{
					SelectObject((*it));
				}
			}
			else
			{
				SelectObject((*it));
			}

			found = true;
		}
	}

	return true;
}

bool EditSession::BoxSelectRails(sf::IntRect &rect)
{
	bool found = false;
	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		if ((*it)->Intersects(rect))
		{
			if (HoldingShift())
			{
				if ((*it)->selected)
				{
					DeselectObject((*it));
				}
				else
				{
					SelectObject((*it));
				}
			}
			else
			{
				SelectObject((*it));
			}

			found = true;
		}
	}

	return found;
}

void EditSession::TryBoxSelect()
{
	Vector2i currPos(worldPos.x, worldPos.y);

	int left = std::min(editMouseOrigPos.x, currPos.x);
	int right = std::max(editMouseOrigPos.x, currPos.x);
	int top = std::min(editMouseOrigPos.y, currPos.y);
	int bot = std::max(editMouseOrigPos.y, currPos.y);


	sf::Rect<int> r(left, top, right - left, bot - top);
	//check this rectangle for the intersections, but do that next

	bool selectionEmpty = true;

	bool specialTerrain = IsSpecialTerrainMode();

	if (!HoldingShift())
	{
		//clear everything
		ClearSelectedBrush();
	}

	if ( !specialTerrain && BoxSelectActors(r))
	{
		selectionEmpty = false;
	}

	if ( !specialTerrain && BoxSelectDecor(r))
	{
		selectionEmpty = false;
	}

	bool pointSelectButtonHeld = IsKeyPressed(Keyboard::B);
	if (pointSelectButtonHeld) //always use point selection for now
	{
		if (HoldingShift())
		{
			//ClearSelectedPoints();
		}

		BoxSelectPoints(r, 8 * zoomMultiple);
	}
	else if (!showPoints)//polygon selection. don't use it for a little bit
	{
		if (BoxSelectPolys(r))
		{
			selectionEmpty = false;
		}

		if (!specialTerrain && BoxSelectRails(r))
		{
			selectionEmpty = false;
		}
	}

	if (selectionEmpty)
	{
		ClearSelectedBrush();
	}
}

double EditSession::GetZoomedMinEdgeLength()
{
	 return minimumEdgeLength * std::max(zoomMultiple, 1.0);
}

void EditSession::UpdateGrass()
{
	if (showGrass)
	{
		PolyPtr tp;
		for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
		{
			tp = (*it)->GetAsTerrain();
			if (tp != NULL)
				tp->UpdateGrass();
		}
	}
}

void EditSession::ModifyGrass()
{
	if (showGrass && IsMousePressed(Mouse::Left))
	{
		PolyPtr tp;
		for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
		{
			tp = (*it)->GetAsTerrain();
			if (tp != NULL)
				tp->SwitchGrass(worldPos);
		}
	}
}

void EditSession::SetMode(Emode m)
{
	Emode oldMode = mode;
	if (oldMode == SELECT_MODE)
	{
		oldMode = menuDownStored;
	}

	mode = m;

	switch (oldMode)
	{
	case TEST_PLAYER:
		for (auto it = groups.begin(); it != groups.end(); ++it)
		{
			for (auto enit = (*it).second->actors.begin(); enit != (*it).second->actors.end(); ++enit)
			{
				if ((*enit)->myEnemy != NULL)
				{
					(*enit)->myEnemy->Reset();
					//AddEnemy((*enit)->myEnemy);
				}
			}
		}
		break;
	}

	switch (mode)
	{
	case CREATE_ENEMY:
		grabbedActor = NULL;
		selectedBrush->Clear();
		editClock.restart();
		editCurrentTime = 0;
		editAccumulator = TIMESTEP + .1;
		break;
	case EDIT:
	{
		editClock.restart();
		editCurrentTime = 0;
		editAccumulator = TIMESTEP + .1;
		break;
	}
	}
}

bool EditSession::IsOnlyPlayerSelected()
{
	if (selectedBrush->objects.size() == 1)
	{
		if (player == selectedBrush->objects.front())
		{
			return true;
		}
	}

	return false;
}

void EditSession::RemoveSelectedObjects()
{
	//get list of selected ACTORs only
	SelectList actorsList;
	for (auto it = selectedBrush->objects.begin();
		it != selectedBrush->objects.end(); ++it)
	{
		if ((*it)->selectableType == ISelectable::ACTOR)
		{
			actorsList.push_back((*it));
		}
	}

	//get list of additional ACTORs from selected terrain
	SelectList addedActorsList;
	list<GateInfoPtr> tempGateList;

	PolyPtr poly;
	for (auto it = selectedBrush->objects.begin();
		it != selectedBrush->objects.end(); ++it)
	{
		poly = (*it)->GetAsTerrain();
		if (poly != NULL)
		{
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
					if (!alreadyHere)
					{
						addedActorsList.push_back((*api));
					}
				}
			}

			poly->AddGatesToList(tempGateList);
		}
	}

	for (auto it = addedActorsList.begin(); it != addedActorsList.end(); ++it)
	{
		selectedBrush->AddObject((*it));
	}

	for (auto it = tempGateList.begin(); it != tempGateList.end(); ++it)
	{
		selectedBrush->AddObject((*it));
	}

	ClearUndoneActions();

	Action *remove = new RemoveBrushAction(selectedBrush, mapStartBrush );

	remove->Perform();
	AddDoneAction(remove);
	

	ClearSelectedBrush();
}

void EditSession::TryRemoveSelectedObjects()
{
	bool perform = true;

	if (IsOnlyPlayerSelected())
	{
		perform = false;
	}

	if (perform)
	{
		RemoveSelectedObjects();
	}
}

bool EditSession::IsSingleActorSelected()
{
	return (selectedBrush->objects.size() == 1
		&& selectedBrush->objects.front()->selectableType == ISelectable::ACTOR);
}

void EditSession::AddActorMove(Action *a)
{
	//if( moveAction )
}

void EditSession::MoveSelectedActors(sf::Vector2i &delta)
{
	ActorPtr actor;
	Vector2i extraDelta = Vector2i(0, 0);
	bool unanchored = false;

	PositionInfo pi;

	Vector2i diffPerActor;

	std::vector<PositionInfo> piVec;
	piVec.resize(selectedBrush->objects.size());

	int piIndex = 0;
	
	int numCanBeAnchored = 0;
	int numWillBeAnchored = 0;

	PositionInfo grabbedActorInfo;

	V2d grabCenter = V2d(grabbedActor->GetGrabAABBCenter());
	extraDelta = Vector2i(worldPos) - Vector2i(grabCenter);

	V2d actorRealignDiff;
	for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it, ++piIndex)
	{
		actor = (*it)->GetAsActor();
		if (actor == NULL )
			continue;

		if (actor->type->CanBeGrounded())
		{
			if (!actor->posInfo.IsAerial())
			{
				actor->UnAnchor();
				if (actor != grabbedActor)
				{
					actorRealignDiff = grabCenter + actor->diffFromGrabbed;
					actorRealignDiff = actorRealignDiff - actor->GetPosition();
					actor->Move(Vector2i(round(actorRealignDiff.x), round(actorRealignDiff.y)));
				}
			}
		}
	}
	
	piIndex = 0;
	for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it, ++piIndex)
	{
		actor = (*it)->GetAsActor();
		if (actor == NULL)
			continue;

		if (actor->posInfo.ground == NULL && actor->type->CanBeGrounded() && !actor->type->CanBeAerial())
		{
			numCanBeAnchored++;
			piVec[piIndex] = ConvertPointToGround(Vector2i(worldPos + actor->diffFromGrabbed), actor);

			if (piVec[piIndex].ground == NULL)
				break;

			if (actor == grabbedActor)
			{
				grabbedActorInfo = piVec[piIndex];
			}
			numWillBeAnchored++;
		}
	}

	if ( numCanBeAnchored == 0 || numCanBeAnchored != numWillBeAnchored)
	{
		selectedBrush->Move(delta + extraDelta);
	}
	else
	{
		bool alignmentMaintained = true;
		V2d currDiff;
		double currDiffAngle;
		piIndex = 0;
		for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it, ++piIndex)
		{
			if (piVec[piIndex].ground == NULL)
			{
				continue;
			}

			actor = (*it)->GetAsActor();
			currDiff = piVec[piIndex].GetPosition() - grabbedActorInfo.GetPosition();
			currDiffAngle = piVec[piIndex].GetEdge()->GetNormalAngleRadians() - grabbedActorInfo.GetEdge()->GetNormalAngleRadians();
		}

		if (alignmentMaintained)
		{
			piIndex = 0;
			for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it, ++piIndex)
			{
				if (piVec[piIndex].ground == NULL)
				{
					continue;
				}

				actor = (*it)->GetAsActor();
				actor->AnchorToGround(piVec[piIndex]);
				piVec[piIndex].AddActor(actor);
			}
		}
		else
		{
			selectedBrush->Move(delta + extraDelta);
		}
	}
	
	//doesnt SEEM like this is needed but im not convinced yet
	for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
	{
		actor = (*it)->GetAsActor();
		if (actor == NULL)
			continue;
		//actor->myEnemy->UpdateFromEditParams(0);
	}

	bool canApply = selectedBrush->CanApply();
	for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
	{
		actor = (*it)->GetAsActor();
		if (actor == NULL)
			continue;
		
		if (canApply)
		{		
			actor->SetAABBOutlineColor(Color::Green);
		}
		else
		{
			actor->SetAABBOutlineColor(Color::Red);
		}
	}
}

void EditSession::StartSelectedMove()
{
	editStartMove = true;
	Vector2i pos(worldPos.x, worldPos.y);
	Vector2i delta = pos - editMouseGrabPos;

	for (auto mit = selectedPoints.begin(); mit != selectedPoints.end(); ++mit)
	{
		/*list<PointMoveInfo> &pList = (*mit).second;
		for (auto it = pList.begin(); it != pList.end(); ++it)
		{
			(*it).delta = (*it).GetPolyPoint()->pos;
		}*/
	}

	/*for (auto mit = selectedRailPoints.begin(); mit != selectedRailPoints.end(); ++mit)
	{
		list<PointMoveInfo> &pList = (*mit).second;
		for (auto it = pList.begin(); it != pList.end(); ++it)
		{
			(*it).delta = (*it).GetRailPoint()->pos;
		}
	}*/

	/*if (IsSingleActorSelected())
	{
		ActorPtr a = selectedBrush->objects.front()->GetAsActor();
		moveAction = new CompoundAction;
		
		Action *newAction = new LeaveGroundAction(a, worldPos - V2d(a->GetGrabAABBCenter()));
		newAction->Perform();
		moveAction->AddSubAction(newAction);
	}*/

	//assumption that all are grounded atm
	if (grabbedActor != NULL) //need to figure out how to separate terrain selection from enemies
	{
		moveAction = selectedBrush->UnAnchor();
		if (moveAction != NULL)
		{
			ActorPtr actor;

			grabbedActor->diffFromGrabbed = V2d(0, 0);

			V2d grabbedGroundPos = grabbedActor->posInfo.GetPosition();
			double grabbedGroundAngle = grabbedActor->posInfo.GetEdge()->GetNormalAngleRadians();

			for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
			{
				actor = (*it)->GetAsActor();
				if (actor == NULL || actor == grabbedActor)
					continue;

				if (actor->posInfo.ground != NULL)
				{
					actor->diffFromGrabbed = actor->posInfo.GetPosition() - grabbedGroundPos;
				}
			}

			moveAction->Perform();
		}
	}

	

	selectedBrush->Move(delta);

	pointGrabDelta = Vector2i(worldPos.x, worldPos.y) - pointGrabPos;
	oldPointGrabPos = pointGrabPos;
	pointGrabPos = Vector2i(worldPos.x, worldPos.y);

	//NewMoveSelectedPoints();
	StartMoveSelectedPoints();
	MoveSelectedPoints();
	MoveSelectedRailPoints(worldPos);

	editMouseGrabPos = pos;
}

void EditSession::ContinueSelectedMove()
{
	Vector2i pos(worldPos.x, worldPos.y);
	Vector2i delta = pos - editMouseGrabPos;
	
	//if (IsSingleActorSelected() && selectedPoints.empty())
	if (selectedPoints.empty() && grabbedActor != NULL)
	{
		MoveSelectedActors(delta);
	}
	else
	{
		selectedBrush->Move(delta);

		pointGrabDelta = Vector2i(worldPos.x, worldPos.y) - pointGrabPos;
		oldPointGrabPos = pointGrabPos;
		pointGrabPos = Vector2i(worldPos.x, worldPos.y);

		MoveSelectedPoints();
		MoveSelectedRailPoints(worldPos);
	}

	editMouseGrabPos = Vector2i(worldPos);
}

void EditSession::TrySelectedMove()
{
	//this secondary calculation makes the move not count for some window
	if ((editMouseDownMove && !editStartMove ))// && length(V2d(editMouseGrabPos.x, editMouseGrabPos.y) - worldPos) > editMoveThresh * zoomMultiple))
	{
		StartSelectedMove();
	}
	else if (editMouseDownMove && editStartMove)
	{
		ContinueSelectedMove();
	}
	else if (editMouseDownBox)
	{
		//stuff
	}
}

void EditSession::PreventNearPrimaryAnglesOnPolygonInProgress()
{
	if (polygonInProgress->GetNumPoints() > 0)
	{
		TerrainPoint *end = polygonInProgress->GetEndPoint();
		V2d backPoint = V2d(end->pos.x, end->pos.y);
		V2d tPoint(testPoint.x, testPoint.y);
		V2d extreme(0, 0);
		V2d vec = tPoint - backPoint;
		V2d normVec = normalize(vec);

		if (normVec.x > PRIMARY_LIMIT)
			extreme.x = 1;
		else if (normVec.x < -PRIMARY_LIMIT)
			extreme.x = -1;
		if (normVec.y > PRIMARY_LIMIT)
			extreme.y = 1;
		else if (normVec.y < -PRIMARY_LIMIT)
			extreme.y = -1;

		if (!(extreme.x == 0 && extreme.y == 0))
		{
			testPoint = Vector2f(backPoint + extreme * length(vec));
		}
	}
}

void EditSession::PreventNearPrimaryAnglesOnRailInProgress()
{
	int numP = railInProgress->GetNumPoints();
	if (numP > 0)
	{
		TerrainPoint *end = railInProgress->GetEndPoint();
		V2d backPoint = V2d(end->pos.x, end->pos.y);
		V2d tPoint(testPoint.x, testPoint.y);
		V2d extreme(0, 0);
		V2d vec = tPoint - backPoint;
		V2d normVec = normalize(vec);

		if (normVec.x > PRIMARY_LIMIT)
			extreme.x = 1;
		else if (normVec.x < -PRIMARY_LIMIT)
			extreme.x = -1;
		if (normVec.y > PRIMARY_LIMIT)
			extreme.y = 1;
		else if (normVec.y < -PRIMARY_LIMIT)
			extreme.y = -1;

		if (!(extreme.x == 0 && extreme.y == 0))
		{
			testPoint = Vector2f(backPoint + extreme * length(vec));
		}
	}
}

void EditSession::TryAddPointToPolygonInProgress()
{
	if (!panning && IsMousePressed(Mouse::Left))
	{
		Vector2i worldi(round(testPoint.x), round(testPoint.y));

		bool validPoint = polygonInProgress->IsValidInProgressPoint(worldi);//true;

		//test validity later
		if (validPoint)
		{
			if ( polygonInProgress->GetNumPoints() >= 3 && polygonInProgress->IsCloseToFirstPoint(GetZoomedPointSize(), V2d(worldi)))
			{
				if (ExecuteTerrainCompletion())
				{
					justCompletedPolyWithClick = true;
				}
				//complete polygon
			}
			else
			{
				polygonInProgress->AddPoint(worldi, false);
			}
		}
	}
}

void EditSession::TryAddPointToRailInProgress()
{
	if (!panning && IsMousePressed(Mouse::Left))
	{
		bool validPoint = true;

		//test validity later
		if (validPoint)
		{
			Vector2i worldi(testPoint.x, testPoint.y);

			int numP = railInProgress->GetNumPoints();
			TerrainPoint *end = railInProgress->GetEndPoint();
			if (numP == 0 || (numP > 0 &&
				length(V2d(testPoint.x, testPoint.y)
					- Vector2<double>(end->pos.x,
						end->pos.y)) >= minimumEdgeLength * std::max(zoomMultiple, 1.0)))
			{
				railInProgress->AddPoint(worldi, false);
			}
		}
	}
}

void EditSession::SetSelectedTerrainLayer(int layer)
{
	assert(layer == 0 || layer == 1);

	if (selectedBrush != NULL)
	{
		PolyPtr poly;
		SelectList &sl = selectedBrush->objects;
		for (auto it = sl.begin(); it != sl.end(); ++it)
		{
			poly = (*it)->GetAsTerrain();
			if (poly != NULL)
				poly->SetLayer(layer);
		}
	}
}

void EditSession::MoveTopBorder(int amount)
{
	topBound += amount;
	boundHeight -= amount;
	UpdateFullBounds();
}

void EditSession::MoveLeftBorder(int amount)
{
	leftBound += amount;
	boundWidth -= amount;
	UpdateFullBounds();
}

void EditSession::MoveRightBorder(int amount)
{
	boundWidth += amount;
	UpdateFullBounds();
}

void EditSession::ShowGrass(bool s)
{
	showGrass = s;

	PolyPtr tp;
	for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
	{
		tp = (*it)->GetAsTerrain();
		if (tp != NULL)
			tp->ShowGrass(s);
	}
}

void EditSession::ModifyZoom(double factor)
{
	double old = zoomMultiple;

	zoomMultiple *= factor;

	if (zoomMultiple < minZoom)
		zoomMultiple = minZoom;
	else if (zoomMultiple > maxZoom)
		zoomMultiple = maxZoom;
	else if (abs(zoomMultiple - 1.0) < .1)
	{
		zoomMultiple = 1;
	}

	if (old != zoomMultiple)
	{
		UpdateFullBounds();

		Vector2<double> ff = Vector2<double>(view.getCenter().x, view.getCenter().y);//worldPos - ( - (  .5f * view.getSize() ) );
		view.setSize(Vector2f(960 * (zoomMultiple), 540 * (zoomMultiple)));
		preScreenTex->setView(view);
		Vector2f newWorldPosTemp = preScreenTex->mapPixelToCoords(GetPixelPos());
		Vector2<double> newWorldPos(newWorldPosTemp.x, newWorldPosTemp.y);
		Vector2<double> tempCenter = ff + (worldPos - newWorldPos);
		view.setCenter(tempCenter.x, tempCenter.y);
		preScreenTex->setView(view);
	}
}

Vector2i EditSession::GetPixelPos()
{
	Vector2i pPos = Mouse::getPosition(*window);
	pPos.x *= 1920.f / window->getSize().x;
	pPos.y *= 1080.f / window->getSize().y;

	return pPos;
}

void EditSession::UpdateCurrTerrainType()
{
	int ind = currTerrainWorld * MAX_TERRAINTEX_PER_WORLD + currTerrainVar;
	currTerrainTypeSpr.setTexture(*ts_polyShaders[ind]->texture);
	currTerrainTypeSpr.setTextureRect(IntRect(0, 0, 64, 64));
}

void EditSession::TryAddToPatrolPath()
{
	V2d pathBack(patrolPath.back());
	V2d temp = V2d(testPoint.x, testPoint.y) - pathBack;

	if (!panning && IsMousePressed(Mouse::Left))
	{
		//double test = 100;
		//worldPos before testPoint

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
}

void EditSession::SetEnemyLevel()
{
	if (IsMousePressed(Mouse::Left))
	{
		for (map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it)
		{
			list<ActorPtr> &actors = it->second->actors;
			for (list<ActorPtr>::iterator it2 = actors.begin(); it2 != actors.end(); ++it2)
			{
				sf::FloatRect bounds = (*it2)->image.getGlobalBounds();
				if (bounds.contains(Vector2f(worldPos.x, worldPos.y)))
				{
					(*it2)->SetLevel(setLevelCurrent);
				}
			}
		}
	}
}

void EditSession::UpdatePanning()
{
	V2d tempWorldPos = V2d(preScreenTex->mapPixelToCoords(pixelPos));
	if (panning)
	{
		Vector2<double> temp = panAnchor - tempWorldPos;
		view.move(Vector2f(temp.x, temp.y));
	}
}

void EditSession::UpdatePolyShaders()
{
	Vector2f vSize = view.getSize();
	float zoom = vSize.x / 960;
	Vector2f botLeft(view.getCenter().x - vSize.x / 2, view.getCenter().y + vSize.y / 2);
	bool first = oldShaderZoom < 0;


	if (first || oldShaderZoom != zoom ) //first run
	{
		oldShaderZoom = zoom;
		for (int i = 0; i < 9 * MAX_TERRAINTEX_PER_WORLD; ++i)
		{
			if (ts_polyShaders[i] != NULL)
			{
				polyShaders[i].setUniform("zoom", zoom);
			}
		}
	}

	if (first || oldShaderBotLeft != botLeft)
	{
		oldShaderBotLeft = botLeft;
		for (int i = 0; i < 9 * MAX_TERRAINTEX_PER_WORLD; ++i)
		{
			if (ts_polyShaders[i] != NULL)
			{
				//just need to change the name topleft to botleft in the shader
				polyShaders[i].setUniform("topLeft", botLeft);
			}
		}
	}
	
}

void EditSession::TempMoveSelectedBrush()
{
	if (mode == EDIT)
	{
		if (moveActive)
		{
			Vector2i currMouse(worldPos.x, worldPos.y);
			Vector2i delta = currMouse - pointMouseDown;
			pointMouseDown = currMouse;

			selectedBrush->Move(delta);
			selectedBrush->Draw(preScreenTex);
		}
	}
}

void EditSession::DrawGraph()
{
	if (showGraph)
	{
		graph->SetCenterAbsolute(view.getCenter());
		graph->Draw(preScreenTex);
	}
}

void EditSession::DrawPolygons()
{
	if (inversePolygon != NULL)
	{
		inversePolygon->Draw(false, zoomMultiple, preScreenTex, showPoints, NULL);
	}

	for (int i = 0; i < 2; ++i)
	{
		auto & currPolyList = GetCorrectPolygonList(i);
		for (auto it = currPolyList.begin(); it != currPolyList.end(); ++it)
		{
			if ((*it)->inverse)
				continue;

			(*it)->Draw(false, zoomMultiple, preScreenTex, showPoints, NULL);
		}
	}
}

void EditSession::DrawRails()
{
	for (list<RailPtr>::iterator it = rails.begin(); it != rails.end(); ++it)
	{
		(*it)->Draw(zoomMultiple, showPoints, preScreenTex);
	}
}

void EditSession::DrawPolygonInProgress()
{
	int progressSize = polygonInProgress->GetNumPoints();
	if (progressSize > 0)
	{
		Vector2i backPoint = polygonInProgress->GetEndPoint()->pos;
		Vector2i worldi = Vector2i(round(testPoint.x), round(testPoint.y));


		Color validColor = Color::Green;
		Color invalidColor = Color::Red;
		Color colorSelection;

		bool valid = polygonInProgress->IsValidInProgressPoint(worldi);
		//cout << "draw testpoint: " << testPoint.x << ", " << testPoint.y << endl;
		if (valid)
		{
			colorSelection = validColor;
		}
		else
		{
			colorSelection = invalidColor;
		}

		int pSize = progressSize;
		if (progressSize == 1)
		{
			pSize += 1;
		}
		else
		{
			pSize += 2;
		}

		VertexArray v(sf::LinesStrip, pSize);

		int vIndex = 0;

		TerrainPoint *curr;
		if (progressSize > 1)
		{
			for (int i = 0; i < progressSize; ++i)
			{
				curr = polygonInProgress->GetPoint(i);
				v[vIndex] = Vertex(Vector2f(curr->pos.x, curr->pos.y), validColor);
				++vIndex;
			}
		}

		v[vIndex] = Vertex(Vector2f(backPoint), colorSelection);
		v[vIndex + 1] = Vertex(Vector2f(worldi), colorSelection);

		
		preScreenTex->draw(v);

		CircleShape cs;
		cs.setRadius(POINT_SIZE * zoomMultiple);
		cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
		cs.setFillColor(Color::Green);

		for (int i = 0; i < progressSize; ++i)
		{
			curr = polygonInProgress->GetPoint(i);

			cs.setPosition(curr->pos.x, curr->pos.y);
			preScreenTex->draw(cs);
		}
	}
}

void EditSession::DrawRailInProgress()
{
	int progressSize = railInProgress->GetNumPoints();
	if (progressSize > 0)
	{
		TerrainPoint *end = railInProgress->GetEndPoint();
		TerrainPoint *curr;
		Vector2i backPoint = end->pos;

		Color validColor = Color::Yellow;
		Color invalidColor = Color::Red;
		Color colorSelection;
		if (true)
		{
			colorSelection = validColor;
		}

		{
			sf::Vertex activePreview[2] =
			{
				sf::Vertex(sf::Vector2<float>(backPoint.x, backPoint.y), colorSelection),
				sf::Vertex(sf::Vector2<float>(testPoint.x, testPoint.y), colorSelection)
			};


			preScreenTex->draw(activePreview, 2, sf::Lines);
		}

		if (progressSize > 1)
		{
			VertexArray v(sf::LinesStrip, progressSize);

			for (int i = 0; i < progressSize; ++i)
			{
				curr = railInProgress->GetPoint(i);
				v[i] = Vertex(Vector2f(curr->pos.x, curr->pos.y));
			}
			preScreenTex->draw(v);
		}

		CircleShape cs;
		cs.setRadius(POINT_SIZE * zoomMultiple);
		cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
		cs.setFillColor(Color::Red);

		for (int i = 0; i < progressSize; ++i)
		{
			curr = railInProgress->GetPoint(i);
			cs.setPosition(curr->pos.x, curr->pos.y);
			preScreenTex->draw(cs);
		}
	}
}

void EditSession::DrawActors()
{
	for (map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		(*it).second->Draw(preScreenTex);
	}
}

void EditSession::DrawGates()
{
	for (list<GateInfoPtr>::iterator it = gates.begin(); it != gates.end(); ++it)
	{
		(*it)->Draw(preScreenTex);
	}
}

void EditSession::DrawDecorBehind()
{
	for (auto it = decorImagesBehindTerrain.begin(); it != decorImagesBehindTerrain.end(); ++it)
	{
		preScreenTex->draw((*it)->spr);
	}
}

void EditSession::DrawDecorBetween()
{
	for (auto it = decorImagesBetween.begin(); it != decorImagesBetween.end(); ++it)
	{
		(*it)->Draw(preScreenTex);
	}
}

void EditSession::DrawBoxSelection()
{
	if (editMouseDownBox)
	{
		Vector2i currPos(worldPos.x, worldPos.y);

		int left = std::min(editMouseOrigPos.x, currPos.x);
		int right = std::max(editMouseOrigPos.x, currPos.x);
		int top = std::min(editMouseOrigPos.y, currPos.y);
		int bot = std::max(editMouseOrigPos.y, currPos.y);

		sf::RectangleShape rs(Vector2f(right - left, bot - top));
		rs.setFillColor(Color(200, 200, 200, 80));
		rs.setPosition(left, top);
		preScreenTex->draw(rs);
	}
}

void EditSession::DrawTrackingEnemy()
{
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
	}*/
}

void EditSession::DrawPatrolPathInProgress()
{
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
			for (auto it = patrolPath.begin();
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
		cs.setRadius(POINT_SIZE * zoomMultiple);
		cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
		cs.setFillColor(Color::Green);


		for (auto it = patrolPath.begin(); it != patrolPath.end(); ++it)
		{
			cs.setPosition((*it).x, (*it).y);
			preScreenTex->draw(cs);
		}
	}
}

double EditSession::GetZoomedPointSize()
{
	return POINT_SIZE * zoomMultiple;
}

void EditSession::DrawGateInProgress()
{
	if (gatePoints > 0)
	{
		CircleShape cs(POINT_SIZE * zoomMultiple);
		cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);
		cs.setPosition(testGateInfo.point0->pos.x, testGateInfo.point0->pos.y);
		cs.setFillColor(COLOR_TEAL);

		V2d origin(testGateInfo.point0->pos.x, testGateInfo.point0->pos.y);

		V2d pointB;
		if (gatePoints > 1)
		{
			pointB = V2d(testGateInfo.point1->pos.x, testGateInfo.point1->pos.y);
		}
		else
		{
			pointB = worldPos;
		}


		V2d axis = normalize(worldPos - origin);
		V2d other(axis.y, -axis.x);


		double width = 4.0 * zoomMultiple;
		V2d closeA = origin + other * width;
		V2d closeB = origin - other * width;
		V2d farA = pointB + other * width;
		V2d farB = pointB - other * width;

		Color c;
		if (gatePoints == 1)
		{
			if (IsGateInProgressValid(testGateInfo.poly0, testGateInfo.point0))
			{
				c = Color::White;
			}
			else
			{
				c = Color::Red;
			}
		}

		sf::Vertex quad[4] = {
			sf::Vertex(Vector2f(closeA.x, closeA.y), c),
			sf::Vertex(Vector2f(farA.x, farA.y),c),
			sf::Vertex(Vector2f(farB.x, farB.y), c),
			sf::Vertex(Vector2f(closeB.x , closeB.y), c)
		};

		preScreenTex->draw(quad, 4, sf::Quads);
		preScreenTex->draw(cs);
	}
}

void EditSession::DrawDecorFront()
{
	for (auto it = decorImagesFrontTerrain.begin(); it != decorImagesFrontTerrain.end(); ++it)
	{
		preScreenTex->draw((*it)->spr);
	}
}

void EditSession::DrawMode()
{
	switch (mode)
	{
	case CREATE_TERRAIN:
	{
		DrawPolygonInProgress();
		break;
	}
	case CREATE_RAILS:
	{
		DrawRailInProgress();
		break;
	}
	case EDIT:
	{
		DrawBoxSelection();

		break;
	}
	case CREATE_ENEMY:
	{
		DrawTrackingEnemy();
		break;
	}
	case CREATE_RECT:
	{
		rectCreatingTrigger->Draw(preScreenTex);
		break;
	}
	case SET_CAM_ZOOM:
	{
		currentCameraShot->Draw(preScreenTex);
		break;
	}
	case CREATE_PATROL_PATH:
	{
		DrawTrackingEnemy();
		DrawPatrolPathInProgress();
		break;
	}
	case SET_DIRECTION:
	{
		DrawTrackingEnemy();
		DrawPatrolPathInProgress();
		break;
	}
	case SELECT_MODE:
	{
		break;
	}
	case CREATE_GATES:
	{
		DrawGateInProgress();
		break;
	}
	}
}

void EditSession::DrawModeUI()
{
	switch (mode)
	{
	case CREATE_TERRAIN:
	{
		break;
	}
	case CREATE_ENEMY:
	{
		break;
	}
	case SELECT_MODE:
	{
		preScreenTex->draw(guiMenuSprite);


		Color c;


		CircleShape cs;
		cs.setRadius(menuCircleRadius);
		cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);


		cs.setFillColor(COLOR_BLUE);
		cs.setPosition((menuDownPos + circleUpperRightPos).x, (menuDownPos + circleUpperRightPos).y);
		preScreenTex->draw(cs);

		sf::Text textblue;
		textblue.setCharacterSize(14);
		textblue.setFont(arial);
		textblue.setString("CREATE\nTERRAIN");
		textblue.setFillColor(sf::Color::White);
		textblue.setOrigin(textblue.getLocalBounds().width / 2, textblue.getLocalBounds().height / 2);
		textblue.setPosition((menuDownPos + circleUpperRightPos).x, (menuDownPos + circleUpperRightPos).y);
		preScreenTex->draw(textblue);


		cs.setFillColor(COLOR_GREEN);
		cs.setPosition((menuDownPos + circleLowerRightPos).x, (menuDownPos + circleLowerRightPos).y);
		preScreenTex->draw(cs);

		sf::Text textgreen;
		textgreen.setCharacterSize(14);
		textgreen.setFont(arial);
		textgreen.setString("MAP\nOPTIONS");
		textgreen.setFillColor(sf::Color::White);
		textgreen.setOrigin(textgreen.getLocalBounds().width / 2, textgreen.getLocalBounds().height / 2);
		textgreen.setPosition((menuDownPos + circleLowerRightPos).x, (menuDownPos + circleLowerRightPos).y);
		preScreenTex->draw(textgreen);


		cs.setFillColor(COLOR_YELLOW);
		cs.setPosition((menuDownPos + circleBottomPos).x, (menuDownPos + circleBottomPos).y);
		preScreenTex->draw(cs);

		cs.setFillColor(COLOR_ORANGE);
		cs.setPosition((menuDownPos + circleLowerLeftPos).x, (menuDownPos + circleLowerLeftPos).y);
		preScreenTex->draw(cs);

		sf::Text textorange;
		textorange.setString("CREATE\nLIGHTS");
		textorange.setFont(arial);
		textorange.setCharacterSize(14);
		textorange.setFillColor(sf::Color::White);
		textorange.setOrigin(textorange.getLocalBounds().width / 2, textorange.getLocalBounds().height / 2);
		textorange.setPosition((menuDownPos + circleLowerLeftPos).x, (menuDownPos + circleLowerLeftPos).y);
		preScreenTex->draw(textorange);

		cs.setFillColor(COLOR_RED);
		cs.setPosition((menuDownPos + circleUpperLeftPos).x, (menuDownPos + circleUpperLeftPos).y);
		preScreenTex->draw(cs);

		sf::Text textred;
		textred.setString("CREATE\nENEMIES");
		textred.setFont(arial);
		textred.setCharacterSize(14);
		textred.setFillColor(sf::Color::White);
		textred.setOrigin(textred.getLocalBounds().width / 2, textred.getLocalBounds().height / 2);
		textred.setPosition((menuDownPos + circleUpperLeftPos).x, (menuDownPos + circleUpperLeftPos).y);
		preScreenTex->draw(textred);

		cs.setFillColor(COLOR_MAGENTA);
		cs.setPosition((menuDownPos + circleTopPos).x, (menuDownPos + circleTopPos).y);
		preScreenTex->draw(cs);

		sf::Text textmag;

		bool singleObj, singleActor, singleImage, singleRail, onlyPoly; 
		
		singleObj = selectedBrush->objects.size() == 1 && selectedPoints.size() == 0;

		singleActor = singleObj && selectedBrush->objects.front()->selectableType == ISelectable::ACTOR;
		singleImage = singleObj && selectedBrush->objects.front()->selectableType == ISelectable::IMAGE;
		singleRail = singleObj && selectedBrush->objects.front()->selectableType == ISelectable::RAIL;
		onlyPoly = selectedBrush != NULL && !selectedBrush->objects.empty() && selectedBrush->terrainOnly;

		if (menuDownStored == EditSession::EDIT && singleActor)
		{
			textmag.setString("EDIT\nENEMY");
		}
		else if (menuDownStored == EditSession::EDIT && singleImage)
		{
			textmag.setString("EDIT\nIMAGE");
		}
		else if (menuDownStored == EditSession::EDIT && onlyPoly)
		{
			textmag.setString("TERRAIN\nOPTIONS");
		}
		else if(menuDownStored == EditSession::EDIT && singleRail)
		{
			textmag.setString("RAIL\nOPTIONS");
		}
		else
		{
			textmag.setString("EDIT");
		}

		textmag.setFont(arial);
		textmag.setCharacterSize(14);
		textmag.setFillColor(sf::Color::White);
		textmag.setOrigin(textmag.getLocalBounds().width / 2, textmag.getLocalBounds().height / 2);
		textmag.setPosition((menuDownPos + circleTopPos).x, (menuDownPos + circleTopPos).y);
		preScreenTex->draw(textmag);

		break;
	}
	case EDIT:
	{

		break;
	}
	case CREATE_IMAGES:

		break;
	}
}

void EditSession::DrawUI()
{
	preScreenTex->setView(uiView);
	stringstream cursorPosSS;
	stringstream scaleTextSS;
	if (mode == CREATE_PATROL_PATH || mode == SET_DIRECTION)
	{
		V2d temp = V2d(testPoint.x, testPoint.y) - Vector2<double>(patrolPath.back().x,
			patrolPath.back().y);
		cursorPosSS << (int)temp.x << ", " << (int)temp.y;
	}
	else
	{
		cursorPosSS << (int)worldPos.x << ", " << (int)worldPos.y;
	}
	cursorLocationText.setString(cursorPosSS.str());

	Vector2f size = uiView.getSize();
	float sca = view.getSize().x / 960.f / 2.f;
	scaleSprite.setScale(1.f / sca, 1.f / sca);
	scaleTextSS << "scale: x" << scaleSprite.getScale().x;
	scaleSpriteBGRect.setSize(Vector2f(scaleSprite.getGlobalBounds().width,
		scaleSprite.getGlobalBounds().height));
	scaleText.setString(scaleTextSS.str());

	if (mode == CREATE_TERRAIN || mode == EDIT || mode == SELECT_MODE)
	{
		preScreenTex->draw(currTerrainTypeSpr);
	}
	

	//preScreenTex->draw(scaleSpriteBGRect);
	//preScreenTex->draw(scaleSprite);
	preScreenTex->draw(cursorLocationText);
	preScreenTex->draw(scaleText);

	DrawModeUI();

	if (showPanel != NULL)
	{
		showPanel->Draw(preScreenTex);
	}
}

void EditSession::Display()
{
	preScreenTex->display();
	const Texture &preTex = preScreenTex->getTexture();

	Sprite preTexSprite(preTex);
	preTexSprite.setPosition(-960 / 2, -540 / 2);
	preTexSprite.setScale(.5, .5);
	window->clear();
	window->draw(preTexSprite);
	window->display();
}

void EditSession::HandleEvents()
{
	while (window->pollEvent(ev))
	{
		if (ev.type == Event::KeyPressed)
		{
			if (ev.key.code == Keyboard::F5)
			{
				background->FlipShown();
				continue;
			}
		}

		switch (mode)
		{
		case CREATE_TERRAIN:
		{
			CreateTerrainModeHandleEvent();
			break;
		}
		case CREATE_RAILS:
		{
			CreateRailsModeHandleEvent();
			break;
		}
		case EDIT:
		{
			EditModeHandleEvent();
			break;
		}
		case PASTE:
		{
			PasteModeHandleEvent();
			break;
		}
		case CREATE_ENEMY:
		{
			CreateEnemyModeHandleEvent();
			break;
		}
		case PAUSED:
		{
			PausedModeHandleEvent();
			break;
		}
		case SELECT_MODE:
		{
			SelectModeHandleEvent();
			break;
		}
		case CREATE_PATROL_PATH:
		{
			CreatePatrolPathModeHandleEvent();
			break;
		}
		case CREATE_RECT:
		{
			CreateRectModeHandleEvent();
			break;
		}
		case SET_CAM_ZOOM:
		{
			SetCamZoomModeHandleEvent();
			break;
		}
		case SET_DIRECTION:
		{
			SetDirectionModeHandleEvent();
			break;
		}
		case CREATE_GATES:
		{
			CreateGatesModeHandleEvent();
			break;
		}
		case CREATE_IMAGES:
		{
			CreateImagesHandleEvent();
			break;
		}
		case SET_LEVEL:
		{
			SetLevelModeHandleEvent();
			break;
		}
		case TRANSFORM:
		{
			TransformModeHandleEvent();
			break;
		}

		}
		//ones that aren't specific to mode

		if (mode != PAUSED && mode != SELECT_MODE)
		{
			switch (ev.type)
			{
			case Event::MouseButtonPressed:
			{
				if (ev.mouseButton.button == Mouse::Button::Middle)
				{
					panning = true;
					panAnchor = worldPos;
					//cout << "setting panAnchor: " << panAnchor.x << " , " << panAnchor.y << endl;
				}
				else if (ev.mouseButton.button == Mouse::Button::Right)
				{
					if (mode != PASTE)
					{
						menuDownStored = mode;
						mode = SELECT_MODE;
						//SetMode(SELECT_MODE);
						menuDownPos = V2d(uiMousePos.x, uiMousePos.y);
						guiMenuSprite.setPosition(uiMousePos.x, uiMousePos.y);
					}
				}
				break;
			}
			case Event::MouseButtonReleased:
			{
				if (ev.mouseButton.button == Mouse::Button::Middle)
				{
					panning = false;
				}
				break;
			}
			case Event::MouseWheelMoved:
			{
				if (ev.mouseWheel.delta > 0)
				{
					ModifyZoom(.5);
				}
				else if (ev.mouseWheel.delta < 0)
				{
					ModifyZoom(2);
				}
				break;
			}
			case Event::KeyPressed:
			{
				if (ev.key.code == Keyboard::S && ev.key.control)
				{
					polygonInProgress->ClearPoints();
					cout << "writing to file: " << currentFile << endl;
					WriteFile(currentFile);
				}
				else if (ev.key.code == Keyboard::T && showPanel == NULL)
				{
					TestPlayerMode();
					//quit = true;
				}
				else if (ev.key.code == Keyboard::Escape)
				{
					quit = true;
					returnVal = 1;
				}
				else if (ev.key.code == sf::Keyboard::Equal || ev.key.code == sf::Keyboard::Dash)
				{
					if (showPanel != NULL)
						break;

					if (ev.key.code == sf::Keyboard::Equal)
					{
						ModifyZoom(.5);
					}
					else if (ev.key.code == sf::Keyboard::Dash)
					{
						ModifyZoom(2);
					}
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
				stored = mode;
				mode = PAUSED;
				break;
			}
			case Event::GainedFocus:
			{
				mode = stored;
				//SetMode(stored);
				break;
			}
			}
		}

	}
}

void EditSession::CreateTerrainModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			if (showPanel != NULL)
			{
				showPanel->Update(true, false, uiMousePos.x, uiMousePos.y);
				break;
			}
		}

		justCompletedPolyWithClick = false; //just make this always false here.

		break;
	}
	case Event::MouseButtonReleased:
	{
		if (showPanel != NULL)
		{
			showPanel->Update(false, false, uiMousePos.x, uiMousePos.y);
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

		if (ev.key.code == Keyboard::Space)
		{
			ExecuteTerrainCompletion();
		}
		else if (ev.key.code == sf::Keyboard::X || ev.key.code == sf::Keyboard::Delete)
		{
			RemovePointFromPolygonInProgress();
		}
		else if (ev.key.code == sf::Keyboard::E)
		{
			GridSelectPop("terraintypeselect");
			currTerrainWorld = tempGridX;
			currTerrainVar = tempGridY;
			UpdateCurrTerrainType();
		}
		else if (ev.key.code == sf::Keyboard::R)
		{
			SetMode(CREATE_RAILS);
			railInProgress->CopyPointsFromPoly(polygonInProgress);
			polygonInProgress->ClearPoints();
		}
		else if (ev.key.code == sf::Keyboard::Z && ev.key.control)
		{
			UndoMostRecentAction();
		}
		else if (ev.key.code == sf::Keyboard::Y && ev.key.control)
		{
			RedoMostRecentUndoneAction();
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

void EditSession::CreateRailsModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			if (showPanel != NULL)
			{
				showPanel->Update(true, false, uiMousePos.x, uiMousePos.y);
				break;
			}
		}
		break;
	}
	case Event::MouseButtonReleased:
	{
		if (showPanel != NULL)
		{
			showPanel->Update(false, false, uiMousePos.x, uiMousePos.y);
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

		if (ev.key.code == Keyboard::Space)
		{
			ExecuteRailCompletion();
		}
		else if (ev.key.code == sf::Keyboard::X || ev.key.code == sf::Keyboard::Delete)
		{
			RemovePointFromRailInProgress();
		}
		else if (ev.key.code == sf::Keyboard::E)
		{
		}
		else if (ev.key.code == sf::Keyboard::R)
		{
			SetMode(CREATE_TERRAIN);
			railInProgress->ClearPoints();
		}
		else if (ev.key.code == sf::Keyboard::Z && ev.key.control)
		{
			UndoMostRecentAction();
		}
		else if (ev.key.code == sf::Keyboard::Y && ev.key.control)
		{
			RedoMostRecentUndoneAction();
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

void EditSession::EditModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			if (showPanel != NULL)
			{
				showPanel->Update(true, false, uiMousePos.x, uiMousePos.y);
				break;
			}

			if (showGrass)
				break;

			bool emptysp = true;
			bool specialTerrain = IsSpecialTerrainMode();

			if (!(editMouseDownMove || editMouseDownBox))
			{
				if (emptysp && !specialTerrain && PointSelectActor(worldPos))
				{
					emptysp = false;
				}

				if (emptysp && !specialTerrain && PointSelectRail(worldPos))
				{
					emptysp = false;
				}

				if (emptysp && PointSelectTerrain(worldPos))
				{
					emptysp = false;
				}

				if (emptysp && !specialTerrain && PointSelectDecor(worldPos))
				{
					emptysp = false;
				}

				editMouseGrabPos = Vector2i(worldPos.x, worldPos.y);
				pointGrabPos = Vector2i(worldPos.x, worldPos.y);
				editMouseOrigPos = editMouseGrabPos;

				if (emptysp)
				{
					editMouseDownMove = false;
					editMouseDownBox = true;
					editStartMove = false;
					grabbedActor = NULL;
					grabbedPoint = NULL;
				}
				else
				{
					editMouseDownMove = true;
					editStartMove = false;
					editMouseDownBox = false;
				}
			}
		}
		break;
	}
	case Event::MouseButtonReleased:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			if (showPanel != NULL)
			{
				showPanel->Update(false, false, uiMousePos.x, uiMousePos.y);
				break;
			}

			if (editStartMove)
			{
				bool done = false;
				if (AnchorSelectedEnemies() )
				{
					done = true;
				}

				if (!done)
				{
					PerformMovePointsAction();
					//NewPerformMovePointsAction();
				}

				TryCompleteSelectedMove();
			}
			else if (editMouseDownBox)
			{
				TryBoxSelect();
			}

			editMouseDownBox = false;
			editMouseDownMove = false;
			editStartMove = false;
			grabbedActor = NULL;
			grabbedPoint = NULL;

			UpdateGrass();
		}
		break;
	}
	case Event::MouseWheelMoved:
	{
		break;
	}
	case Event::MouseMoved:
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

		if (ev.key.code == Keyboard::Tilde)
		{
			SetMode(SET_LEVEL);
			setLevelCurrent = 1;
			showPanel = NULL;
		}

		if (ev.key.code == Keyboard::I && ev.key.control)
		{
			SetMode(CREATE_IMAGES);
			currImageTool = ITOOL_EDIT;
			showPanel = decorPanel;
		}
		if (ev.key.code == Keyboard::C && ev.key.control)
		{
			copiedBrush = selectedBrush->Copy();
		}
		else if (ev.key.code == sf::Keyboard::Z && ev.key.control)
		{
			UndoMostRecentAction();
		}
		else if (ev.key.code == sf::Keyboard::Y && ev.key.control)
		{
			RedoMostRecentUndoneAction();
		}
		else if (ev.key.code == Keyboard::V && ev.key.control)
		{
			if (copiedBrush != NULL)
			{
				Vector2i pos = Vector2i(worldPos.x, worldPos.y);
				copiedBrush->CenterOnPoint(pos);// (pos - copiedBrush->GetCenter());
				editMouseGrabPos = pos;
				editMouseOrigPos = pos;
				SetMode(PASTE);
				if (complexPaste != NULL)
				{
					delete complexPaste;
					complexPaste = NULL;
				}
				
				pasteAxis = -1;
				ClearSelectedBrush();
			}
		}
		else if (ev.key.code == Keyboard::X || ev.key.code == Keyboard::Delete)
		{
			if (CountSelectedPoints() > 0)
			{
				TryRemoveSelectedPoints();
			}
			else
			{
				TryRemoveSelectedObjects();
			}
		}
		else if (ev.key.code == Keyboard::N)
		{
			if (selectedBrush->IsEmpty())
				break;

			SetMode(TRANSFORM);
			
			transformTools->Reset(selectedBrush->GetCenterF(),
				selectedBrush->GetTerrainSize());
			//selectedBrush->Scale(1.05f);
			PolyPtr p;
			for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
			{
				p = (*it)->GetAsTerrain();
				if (p != NULL)
				{
					p->SetRenderMode(TerrainPolygon::RENDERMODE_TRANSFORM);
					//p->SoftReset();
					//p->Scale(1.05f);
					//p->Scale(1.1f);
					//p->Finalize();
				}
					
			}
		}
		else if (ev.key.code == Keyboard::M)
		{
			//selectedBrush->Rotate(-1.f);
			/*PolyPtr p;
			for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
			{
				p = (*it)->GetAsTerrain();
				if (p != NULL)
				{
					p->Scale(.95f);
				}

			}*/
		}
		else if (ev.key.code == Keyboard::R)
		{
			ShowGrass(true);
		}
		else if (ev.key.code == Keyboard::P)
		{
			SetSelectedTerrainLayer(1);
		}
		else if (ev.key.code == Keyboard::O)
		{
			SetSelectedTerrainLayer(0);
		}
		else if (ev.key.code == Keyboard::E)
		{
			GridSelectPop("terraintypeselect");

			if (selectedBrush->objects.size() > 0)
			{
				Action * action = new ModifyTerrainTypeAction(
					selectedBrush, tempGridX, tempGridY);
				action->Perform();
				AddDoneAction(action);
			}

			currTerrainWorld = tempGridX;
			currTerrainVar = tempGridY;
			UpdateCurrTerrainType();
		}
		else if (ev.key.code == Keyboard::I)
		{
			if (ev.key.shift)
			{
				MoveTopBorder(borderMove);
			}
			else
			{
				MoveTopBorder(-borderMove);
			}
		}
		else if (ev.key.code == Keyboard::J)
		{
			if (ev.key.shift)
			{
				MoveLeftBorder(borderMove);
			}
			else
			{
				MoveLeftBorder(-borderMove);
			}
		}
		else if (ev.key.code == Keyboard::L)
		{
			if (ev.key.shift)
			{
				MoveRightBorder(-borderMove);
			}
			else
			{
				MoveRightBorder(borderMove);
			}
		}
		break;
	}
	case Event::KeyReleased:
	{
		if (ev.key.code == Keyboard::R)
		{
			ShowGrass(false);
		}
		else if (ev.key.code == Keyboard::N)
		{
			/*PolyPtr p;
			for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
			{
				p = (*it)->GetAsTerrain();
				if (p != NULL)
				{
					p->CompleteTransformation();
				}		
			}*/
		}
		else if (ev.key.code == Keyboard::M)
		{
			/*PolyPtr p;
			for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
			{
				p = (*it)->GetAsTerrain();
				if (p != NULL)
				{
					p->CompleteTransformation();
				}
			}*/
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
}

void EditSession::PasteModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		if (ev.mouseButton.button == sf::Mouse::Button::Left)
		{
			PasteTerrain(copiedBrush);
		}
		break;
		
	}
	case Event::MouseButtonReleased:
	{
		if (ev.mouseButton.button == sf::Mouse::Button::Left)
		{
			if (complexPaste != NULL)
			{
				//cout << "completing complex paste" << endl;
				//ClearUndoneActions();
				complexPaste = NULL;
			}
		}
		break;
	}
	case Event::KeyPressed:
	{
		if (ev.key.code == Keyboard::X)
		{
			SetMode(EDIT);
			if (complexPaste != NULL)
			{
				delete complexPaste;
				complexPaste = NULL;
			}
		}
		else if (ev.key.code == sf::Keyboard::Z && ev.key.control)
		{
			UndoMostRecentAction();
		}
		else if (ev.key.code == sf::Keyboard::Y && ev.key.control)
		{
			RedoMostRecentUndoneAction();
		}
		break;
	}
	}
}

void EditSession::CreateEnemyModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		/*if (ev.mouseButton.button == Mouse::Left)
		{
			if (showPanel != NULL)
			{
				showPanel->Update(true, uiMousePos.x, uiMousePos.y);
			}
		}*/
		break;
	}
	case Event::MouseButtonReleased:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			/*if (showPanel != NULL)
			{
				showPanel->Update(false, uiMousePos.x, uiMousePos.y);
				break;
			}*/

			if (grabbedActor != NULL )
			{
				bool done = false;
				if (AnchorSelectedEnemies())
				{
					done = true;
				}

				if (!done)
				{
					//PerformMovePointsAction();
					//NewPerformMovePointsAction();
				}

				TryCompleteSelectedMove();
			}
			/*else if (editMouseDownBox)
			{
				TryBoxSelect();
			}*/

			editMouseDownBox = false;
			editMouseDownMove = false;
			editStartMove = false;
			//editMouseDownBox = false;
			//editMouseDownMove = false;
			//editStartMove = false;
			grabbedActor = NULL;
			//grabbedPoint = NULL;

			//UpdateGrass();
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
			if (showPanel == enemySelectPanel)
			{
				if (ev.key.code == Keyboard::Num1)
				{
					SetActiveEnemyGrid(0);
				}
				else if (ev.key.code == Keyboard::Num2)
				{
					SetActiveEnemyGrid(1);
				}
				else if (ev.key.code == Keyboard::Num3)
				{
					SetActiveEnemyGrid(2);
				}
				else if (ev.key.code == Keyboard::Num4)
				{
					SetActiveEnemyGrid(3);
				}
			}

			showPanel->SendKey(ev.key.code, ev.key.shift);
			break;
		}

		if (ev.key.code == Keyboard::X || ev.key.code == Keyboard::Delete)
		{
			/*if (trackingEnemy != NULL)
			{
				trackingEnemy = NULL;
				showPanel = enemySelectPanel;
			}*/
		}
		else if (ev.key.code == sf::Keyboard::Z && ev.key.control)
		{
			UndoMostRecentAction();
		}
		else if (ev.key.code == sf::Keyboard::Y && ev.key.control)
		{
			RedoMostRecentUndoneAction();
		}
		break;
	}
	case Event::KeyReleased:
	{
		break;
	}
	}
}

void EditSession::PausedModeHandleEvent()
{
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
	case Event::GainedFocus:
	{
		SetMode(stored);
		break;
	}
	}
}

void EditSession::SelectModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		break;
	}
	case Event::MouseButtonReleased:
	{
		V2d releasePos(uiMousePos.x, uiMousePos.y);

		V2d worldTop = menuDownPos + circleTopPos;
		V2d worldUpperLeft = menuDownPos + circleUpperLeftPos;
		V2d worldUpperRight = menuDownPos + circleUpperRightPos;
		V2d worldLowerRight = menuDownPos + circleLowerRightPos;
		V2d worldLowerLeft = menuDownPos + circleLowerLeftPos;
		V2d worldBottom = menuDownPos + circleBottomPos;


		if (length(releasePos - worldTop) < menuCircleRadius)
		{
			menuSelection = "top";
		}
		else if (length(releasePos - worldUpperLeft) < menuCircleRadius)
		{
			menuSelection = "upperleft";
		}
		else if (length(releasePos - worldUpperRight) < menuCircleRadius)
		{
			menuSelection = "upperright";
		}
		else if (length(releasePos - worldLowerLeft) < menuCircleRadius)
		{
			menuSelection = "lowerleft";
		}
		else if (length(releasePos - worldLowerRight) < menuCircleRadius)
		{
			menuSelection = "lowerright";
		}
		else if (length(releasePos - worldBottom) < menuCircleRadius)
		{
			menuSelection = "bottom";
		}
		else
		{
			mode = menuDownStored;
			//SetMode(menuDownStored);
			menuSelection = "none";
		}

		if (menuDownStored == EDIT && menuSelection != "none" && menuSelection != "top")
		{
			if (menuDownStored == EDIT)
			{
				ClearSelectedBrush();
			}
		}
		else if (menuDownStored == CREATE_TERRAIN && menuSelection != "none")
		{
			polygonInProgress->ClearPoints();
		}

		if (menuSelection == "top")
		{
			bool singleObject = selectedBrush->objects.size() == 1
				&& selectedPoints.size() == 0;
			bool singleActor = singleObject
				&& selectedBrush->objects.front()->selectableType == ISelectable::ACTOR;
			bool singleImage = singleObject
				&& selectedBrush->objects.front()->selectableType == ISelectable::IMAGE;
			bool singleRail = singleObject
				&& selectedBrush->objects.front()->selectableType == ISelectable::RAIL;

			//bool singlePoly = selectedBrush->objects.size() == 1 
			//	&& selectedPoints.size() == 0
			//	&& selectedBrush->objects.front()->selectableType == ISelectable::TERRAIN;

			bool onlyPoly = selectedBrush != NULL && !selectedBrush->objects.empty() && selectedBrush->terrainOnly;

			if (menuDownStored == EDIT && onlyPoly)
			{
				showPanel = terrainOptionsPanel;
				mode = menuDownStored;
				//SetMode(menuDownStored);
			}
			else if (menuDownStored == EDIT && singleActor)
			{
				SetEnemyEditPanel();
				//SetMode(menuDownStored);
				mode = menuDownStored;
			}
			else if (menuDownStored == EDIT && singleImage)
			{
				showPanel = editDecorPanel;
				SetDecorEditPanel();
				//SetMode(menuDownStored);
				mode = menuDownStored;
			}
			else if (menuDownStored == EDIT && singleRail)
			{
				showPanel = railOptionsPanel;

				SelectPtr select = selectedBrush->objects.front();
				TerrainRail *tr = (TerrainRail*)select;
				tr->UpdatePanel(railOptionsPanel);
				
				//SetMode(menuDownStored);

				mode = menuDownStored;
			}
			else
			{
				SetMode(EDIT);
				showPanel = NULL;
			}
		}
		else if (menuSelection == "upperleft")
		{
			showPoints = false;
			SetMode(CREATE_ENEMY);
			//trackingEnemy = NULL;
			//showPanel = enemySelectPanel;
		}
		else if (menuSelection == "upperright")
		{
			showPoints = false;
			justCompletedPolyWithClick = false;
			SetMode(CREATE_TERRAIN);
			showPanel = NULL;
		}
		else if (menuSelection == "lowerleft")
		{
			SetMode(CREATE_GATES);
			gatePoints = 0;
			showPanel = NULL;
			showPoints = true;
		}
		else if (menuSelection == "lowerright")
		{
			showPanel = mapOptionsPanel;
			mapOptionsPanel->textBoxes["draintime"]->text.setString(to_string(drainSeconds));
			mapOptionsPanel->textBoxes["bosstype"]->text.setString(to_string(bossType));
			mode = menuDownStored;
//			SetMode(menuDownStored);
		}
		else if (menuSelection == "bottom")
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
}

void EditSession::CreatePatrolPathModeHandleEvent()
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
				SelectPtr select = selectedBrush->objects.front();
				ActorParams *actor = (ActorParams*)select;
				showPanel = actor->type->panel;
				actor->SetPath(patrolPath);
				SetMode(EDIT);
			}
			else
			{
				//showPanel = trackingEnemy->panel;
				tempActor->SetPath(patrolPath);
				SetMode(CREATE_ENEMY);
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

void EditSession::CreateRectModeHandleEvent()
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
				showPanel->Update(true, false, uiMousePos.x, uiMousePos.y);
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

			/*if (trackingEnemy != NULL)
			{
				enemySprite.setPosition(Vector2f(rc));
				enemyQuad.setPosition(enemySprite.getPosition());
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
		if (ev.key.code == Keyboard::Space)
		{
			if (selectedBrush->objects.size() == 1) //EDIT
			{
				SelectPtr select = selectedBrush->objects.front();
				AirTriggerParams *actor = (AirTriggerParams*)select;
				showPanel = actor->type->panel;
				SetMode(EDIT);
			}
			else
			{
				//showPanel = trackingEnemy->panel;
				SetMode(CREATE_ENEMY);
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

void EditSession::SetCamZoomModeHandleEvent()
{
	//minimumPathEdgeLength = 16;

	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			//Vector2i worldi(testPoint.x, testPoint.y);
			//patrolPath.push_back(worldi);

			currentCameraShot->SetZoom(Vector2i(testPoint));

			if (tempActor != NULL)
			{
				SetMode(CREATE_ENEMY);
			}
			else
			{
				SetMode(EDIT);
			}

			showPanel = currentCameraShot->type->panel;
		}
		break;
	}
	}

}

void EditSession::SetDirectionModeHandleEvent()
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
				SetMode(CREATE_ENEMY);
			}
			else
			{
				SelectPtr select = selectedBrush->objects.front();
				actor = (ActorParams*)select;
				SetMode(EDIT);
			}

			showPanel = actor->type->panel;
			actor->SetPath(patrolPath);
		}
		break;
	}
	}

}

void EditSession::CreateGatesModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		if (ev.mouseButton.button == Mouse::Left)
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
		break;
	}
	case Event::KeyPressed:
	{
		if (ev.key.code == sf::Keyboard::Z && ev.key.control)
		{
			UndoMostRecentAction();
		}
		else if (ev.key.code == sf::Keyboard::Y && ev.key.control)
		{
			RedoMostRecentUndoneAction();
		}
		else if (ev.key.code == sf::Keyboard::X)
		{
			gatePoints = 0;
			//modifyGate = NULL;
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

void EditSession::CreateImagesHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			if (showPanel != NULL)
			{
				showPanel->Update(true, false, uiMousePos.x, uiMousePos.y);
			}
			else
			{
				DecorPtr dec = new EditorDecorInfo(tempDecorSprite, currDecorLayer, currDecorName, currDecorTile);
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
		}
		break;
	}
	case Event::MouseButtonReleased:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			if (showPanel != NULL)
			{
				showPanel->Update(false, false, uiMousePos.x, uiMousePos.y);
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
		}
		else if (ev.key.code == sf::Keyboard::Z && ev.key.control)
		{
			UndoMostRecentAction();
		}
		else if (ev.key.code == sf::Keyboard::Y && ev.key.control)
		{
			RedoMostRecentUndoneAction();
		}
		break;
	}
	}
}

void EditSession::SetLevelModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
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
		if (ev.key.code == sf::Keyboard::Num1)
		{
			setLevelCurrent = 1;
		}
		else if (ev.key.code == sf::Keyboard::Num2)
		{
			setLevelCurrent = 2;
		}
		else if (ev.key.code == sf::Keyboard::Num3)
		{
			setLevelCurrent = 3;
		}
		else if (ev.key.code == sf::Keyboard::Num4)
		{
			setLevelCurrent = 4;
		}
		break;
	}
	case Event::KeyReleased:
	{
		break;
	}
	}
}

void EditSession::TransformModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
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
		if (ev.key.code == sf::Keyboard::Space)
		{
			SetMode(EDIT);
			PolyPtr p;
			Brush origBrush;
			Brush resultBrush;
			PolyPtr temp;
			for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
			{
				p = (*it)->GetAsTerrain();
				if (p != NULL)
				{
					temp = p->CompleteTransformation();
					if (temp != NULL)
					{
						resultBrush.AddObject(temp);
					}
					origBrush.AddObject((*it));
				}
			}
			ClearSelectedBrush();

			ClearUndoneActions();

			Action *replaceAction = new ReplaceBrushAction(&origBrush, &resultBrush, mapStartBrush);
			replaceAction->Perform();

			AddDoneAction(replaceAction);
		}
		else if (ev.key.code == sf::Keyboard::BackSpace)
		{
			SetMode(EDIT);
			PolyPtr p;
			for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
			{
				p = (*it)->GetAsTerrain();
				if (p != NULL)
				{
					p->CancelTransformation();
				}
			}
		}
		break;
	}
	case Event::KeyReleased:
	{
		break;
	}
	}
}

void EditSession::UpdateMode()
{
	switch (mode)
	{
	case CREATE_TERRAIN:
	{
		CreateTerrainModeUpdate();
		break;
	}
	case CREATE_RAILS:
	{
		CreateRailsModeUpdate();
		break;
	}
	case EDIT:
	{
		EditModeUpdate();
		break;
	}
	case PASTE:
	{
		PasteModeUpdate();
		break;
	}
	case CREATE_ENEMY:
	{
		CreateEnemyModeUpdate();
		break;
	}
	case CREATE_RECT:
	{
		CreateRectModeUpdate();
		break;
	}
	case SET_CAM_ZOOM:
	{
		SetCamZoomModeUpdate();
		break;
	}
	case CREATE_PATROL_PATH:
	{
		CreatePatrolPathModeUpdate();
		break;
	}
	case SET_DIRECTION:
	{
		SetDirectionModeUpdate();
		break;
	}
	case CREATE_GATES:
	{
		CreateGatesModeUpdate();
		break;
	}
	case CREATE_IMAGES:
	{
		CreateImagesModeUpdate();
		break;
	}
	case SET_LEVEL:
	{
		SetLevelModeUpdate();
		break;
	}
	case TEST_PLAYER:
	{
		TestPlayerModeUpdate();
		break;
	}
	case TRANSFORM:
	{
		TransformModeUpdate();
		break;
	}
		
	}
}

void EditSession::CreateTerrainModeUpdate()
{
	if (showPanel != NULL)
		return;

	if (IsKeyPressed(Keyboard::G))
	{
		SnapPointToGraph(testPoint, graph->graphSpacing);
		showGraph = true;
	}
	else if (IsKeyPressed(Keyboard::F))
	{
		PolyPtr p = NULL;
		TerrainPoint *pPoint;
		TrySnapPosToPoint(testPoint, 8 * zoomMultiple, p, pPoint );
		showPoints = true;
	}
	else
	{
		showPoints = false;
	}

	PreventNearPrimaryAnglesOnPolygonInProgress();

	if (!justCompletedPolyWithClick)
	{
		TryAddPointToPolygonInProgress();
	}
}

void EditSession::CreateRailsModeUpdate()
{
	if (showPanel != NULL)
		return;

	if (IsKeyPressed(Keyboard::G))
	{
		SnapPointToGraph(testPoint, graph->graphSpacing);
		showGraph = true;
	}
	else if (IsKeyPressed(Keyboard::F))
	{
		PolyPtr p;
		TerrainPoint *pPoint;
		TrySnapPosToPoint(testPoint, 8 * zoomMultiple, p, pPoint );
		showPoints = true;
	}
	else
	{
		showPoints = false;
	}

	PreventNearPrimaryAnglesOnRailInProgress();

	TryAddPointToRailInProgress();
}

void EditSession::EditModeUpdate()
{
	if (IsKeyPressed(Keyboard::G))
	{
		SnapPointToGraph(worldPos, graph->graphSpacing);
		showGraph = true;
	}

	bool pressedB = IsKeyPressed(Keyboard::B);
	if ( !showPoints && pressedB )
	{
		showPoints = true;
	}
	else if( !pressedB && grabbedPoint == NULL )
	{
		/*for (PointMap::iterator pmit = selectedPoints.begin();
			pmit != selectedPoints.end(); ++pmit)
		{
			list<PointMoveInfo> & pList = (*pmit).second;
			for (list<PointMoveInfo>::iterator pit = pList.begin();
				pit != pList.end(); ++pit)
			{
				(*pit).poly->SetRenderMode(TerrainPolygon::RenderMode::RENDERMODE_NORMAL);
			}
		}*/
		showPoints = false;
		ClearSelectedPoints();
	}

	//cleanup this later
	if (grabbedActor != NULL && grabbedActor->myEnemy != NULL)
	{
		if (selectedBrush->objects.size() > 1)
		{
			ActorPtr actor;
			for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
			{
				actor = (*it)->GetAsActor();
				if (actor != NULL)
				{
					if (actor->myEnemy != NULL)
					{
						actor->myEnemy->UpdateFromEditParams(spriteUpdateFrames);
					}
				}
			}
		}
		else
		{
			grabbedActor->myEnemy->UpdateFromEditParams(spriteUpdateFrames);
		}
	}
		
	TrySelectedMove();

	ModifyGrass();
}

void EditSession::ChooseRectEvent(ChooseRect *cr, int eventType )
{
	
	if (eventType == ChooseRect::E_CLICKED)
	{
		EnemyChooseRect *ceRect = cr->GetAsEnemyChooseRect();
		if (ceRect != NULL)
		{
			SetTrackingEnemy(ceRect->actorType, ceRect->level);
		}
		else
		{
			ImageChooseRect *icRect = cr->GetAsImageChooseRect();
			if (icRect != NULL && icRect->rectIdentity == ChooseRect::I_SEARCHENEMYLIBRARY )
			{
				createEnemyModeUI->FlipLibraryShown();
			}
		}

	}
	else if (eventType == ChooseRect::E_FOCUSED)
	{
		ImageChooseRect *icRect = cr->GetAsImageChooseRect();
		if (icRect != NULL && icRect->rectIdentity == ChooseRect::I_WORLDCHOOSER)
		{
			createEnemyModeUI->SetActiveLibraryWorld(icRect->tileIndex);
		}
	}
	/*else if (eventType == ChooseRect::E_UNFOCUSED)
	{
		ImageChooseRect *icRect = cr->GetAsImageChooseRect();
		if (icRect != NULL)
		{
			createEnemyModeUI->SetActiveLibraryWorld(-1);
		}
	}*/
	
}

void EditSession::PasteModeUpdate()
{
	Vector2i pos(worldPos.x, worldPos.y);
	Vector2i delta = pos - editMouseGrabPos;

	if (HoldingShift())
	{
		if (pasteAxis < 0)
		{
			editMouseOrigPos = pos;
			pasteAxis = 0;
		}
		else// if (pasteAxis == 0)
		{
			Vector2i test = pos - editMouseOrigPos;
			if (test.x != 0 && test.y != 0)
			{
				if (abs(test.x) >= abs(test.y))
				{
					pasteAxis = 1;
				}
				else if (abs(test.y) > abs(test.x))
				{
					pasteAxis = 2;
				}
			}
		}
	}
	else
	{
		pasteAxis = -1;
	}

	//if (pasteAxis == 1)
	//	delta.y = 0;
	//else if (pasteAxis == 2)
	//	delta.x = 0;

	//copiedBrush->Move(delta);
	if (pasteAxis <= 0)
	{
		copiedBrush->CenterOnPoint(pos);
	}
	else if (pasteAxis == 1)
	{
		copiedBrush->CenterOnPoint(Vector2i(pos.x, editMouseOrigPos.y));
	}
	else if (pasteAxis == 2)
	{
		copiedBrush->CenterOnPoint(Vector2i(editMouseOrigPos.x, pos.y));
	}
	editMouseGrabPos = pos;
	
	if (!panning && IsMousePressed(Mouse::Left) && (delta.x != 0 || delta.y != 0)
		&& length(lastBrushPastePos - worldPos ) >= brushRepeatDist )
	{
		PasteTerrain(copiedBrush);
	}
}

void EditSession::CreateEnemyModeUpdate()
{
	createEnemyModeUI->Update(IsMousePressed(Mouse::Left), IsMousePressed(Mouse::Right), Vector2i(uiMousePos.x, uiMousePos.y));
	//showPanel->Update(IsMousePressed( Mouse::Left ), IsMousePressed( Mouse::Right ), uiMousePos.x, uiMousePos.y);
	createEnemyModeUI->UpdateSprites(spriteUpdateFrames);

	if (grabbedActor != NULL)
	{
		grabbedActor->myEnemy->UpdateFromEditParams(spriteUpdateFrames);
		TrySelectedMove();
	}
		
	//MoveTrackingEnemy();
}

void EditSession::CreatePatrolPathModeUpdate()
{
	if (showPanel != NULL)
		return;

	TryAddToPatrolPath();
}

void EditSession::CreateRectModeUpdate()
{
	if (showPanel != NULL)
		return;


	if (!panning && IsMousePressed(Mouse::Left))
	{
		createRectCurrPoint = Vector2i(worldPos);

		Vector2i rc = (createRectStartPoint + createRectCurrPoint) / 2;
		float width = abs(createRectCurrPoint.x - createRectStartPoint.x);
		float height = abs(createRectCurrPoint.y - createRectStartPoint.y);
		rectCreatingTrigger->SetRect(width, height, rc);

		/*if (trackingEnemy != NULL)
		{
			enemySprite.setPosition(Vector2f(rc));
			enemyQuad.setPosition(enemySprite.getPosition());
		}*/
	}
}

void EditSession::SetCamZoomModeUpdate()
{
	if (showPanel != NULL)
		return;

	currentCameraShot->SetZoom(Vector2i(testPoint));
	/*if (!panning && IsMousePressed(Mouse::Left))
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
	}*/
}

void EditSession::SetDirectionModeUpdate()
{
	if (showPanel != NULL)
		return;
}

void EditSession::CreateGatesModeUpdate()
{
	PolyPtr p = NULL;
	TerrainPoint *pPoint = NULL;
	if (gatePoints == 1)
	{
		TrySnapPosToPoint(worldPos, 8 * zoomMultiple, p, pPoint );
	}
	gateInProgressTestPoly = p;
	gateInProgressTestPoint = pPoint;

	if (modifyGate != NULL)
	{
		GridSelectPop("gateselect");

		string gateResult = tempGridResult;

		if (gateResult == "delete")
		{
			Action * action = new DeleteGateAction(modifyGate, mapStartBrush);
			action->Perform();
			AddDoneAction(action);

			modifyGate = NULL;
		}
		else
		{

			Action * action = new ModifyGateAction(modifyGate, gateResult);
			action->Perform();

			if (gateResult == "shard")
			{
				GridSelectPop("shardselector");

				int sw, si;
				GetShardWorldAndIndex(tempGridX, tempGridY, sw, si);
				modifyGate->SetShard(sw, si);
			}


			AddDoneAction(action);
			modifyGate = NULL;
		}
		return;
	}

	//DrawGateInProgress();

	if (gatePoints > 1)
	{
		bool result = IsGateValid(&testGateInfo);//CanCreateGate(testGateInfo);

		if (result)
		{
			GridSelectPop("gateselect");
			string gateResult = tempGridResult;

			if (gateResult == "delete")
			{
			}
			else
			{

				if (gateResult == "shard")
				{
					GridSelectPop("shardselector");
					int sw, si;
					GetShardWorldAndIndex(tempGridX, tempGridY, sw, si);
					testGateInfo.SetShard(sw, si);
				}

				Vector2i adjust(0, 0);
				
				Action *action = new CreateGateAction(testGateInfo, gateResult);
				action->Perform();

				if (GetPrimaryAdjustment(testGateInfo.point0->pos, testGateInfo.point1->pos, adjust))
				{
					CompoundAction *testAction = new CompoundAction;
					testAction->subActions.push_back(action);

					if (!TryGateAdjustAction(GATEADJUST_POINT_B, &testGateInfo, adjust, testAction))
					{
						action->Undo();
						delete action;
						gatePoints = 0;
						return;
					}
					else
					{
						testAction->performed = true;
						AddDoneAction(testAction);
					}
				}
				else
				{
					AddDoneAction(action);
				}
			}
			gatePoints = 0;
		}
		else
		{
			gatePoints = 1;
			testGateInfo.poly1 = NULL;
			testGateInfo.point1 = NULL;
			//MessagePop("gate would intersect some terrain");
		}

		
	}
}

void EditSession::CreateImagesModeUpdate()
{
	if (showPanel == NULL)
	{
		Vector2f p = preScreenTex->mapPixelToCoords(pixelPos);
		tempDecorSprite.setPosition(p);

		tempDecorSprite.setOrigin(tempDecorSprite.getLocalBounds().width / 2, tempDecorSprite.getLocalBounds().height / 2);
		tempDecorSprite.setRotation(0);
	}
}

void EditSession::SetLevelModeUpdate()
{
	SetEnemyLevel();
}

void EditSession::TransformModeUpdate()
{
	Vector2f fWorldPos(worldPos);
	transformTools->Update(fWorldPos, IsMousePressed( Mouse::Left));

	PolyPtr p;
	for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
	{
		p = (*it)->GetAsTerrain();
		if (p != NULL)
		{
			p->UpdateTransformation(transformTools);
		}
	}
}