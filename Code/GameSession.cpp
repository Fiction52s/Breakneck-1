#include "GameSession.h"
#include <fstream>
#include <iostream>
#include <assert.h>
#include "Actor.h"
#include "VectorMath.h"
#include "Camera.h"
#include <sstream>
#include <ctime>
#include <boost/bind.hpp>
#include "EditSession.h"
#include "Zone.h"
#include "Flow.h"
#include "Boss.h"
#include "PowerOrbs.h"
#include "Sequence.h"
#include "BarrierReactions.h"
#include "EnvEffects.h"
#include "SaveFile.h"
#include "MainMenu.h"
#include "GoalExplosion.h"
#include "Minimap.h"
#include "PauseMenu.h"
#include <boost/thread.hpp>
#include <iostream>
#include "ImageText.h"
#include "VictoryScreen.h"
#include "UIWindow.h"
#include "Config.h"
#include "ControlProfile.h"
#include "MusicSelector.h"
#include <boost/thread.hpp>
#include <ctime>
#include "PlayerRecord.h"
#include "Buf.h"
#include "HUD.h"
#include "InputVisualizer.h"
#include "TopClouds.h"
#include "ScreenRecorder.h"
#include "ShardMenu.h"
#include "KeyMarker.h"
#include "ScoreDisplay.h"
#include "Fader.h"
#include "ShipPickup.h"
#include "Nexus.h"
#include "MusicPlayer.h"
#include "Background.h"
#include "ShaderTester.h"
#include "ControlSettingsMenu.h"
#include "TouchGrass.h"

#include "Barrier.h"
#include "SequenceW4.h"
#include "ParticleEffects.h"
#include "Wire.h"
#include "Grass.h"
#include "EnvPlant.h"
#include "AbsorbParticles.h"
#include "AirTrigger.h"
#include "StorySequence.h"
#include "Enemy.h"
#include "EnemiesW1.h"
#include "ActorParamsBase.h"
#include "HitboxManager.h"
#include "EditorRail.h"
#include "GateMarker.h"
#include "DeathSequence.h"
#include "GoalFlow.h"

#include "GameMode.h"
//#include "Enemy_Badger.h"
//#include "Enemy_Bat.h"
//#infclude "Enemy_StagBeetle.h"
//#include "Enemy_Cactus.h"
//#include "Enemy_GravityModifier.h"
//#include "Enemy_HealthFly.h"
//#include "Enemy_GrindJuggler.h"
//#include "Enemy_GroundedGrindJuggler.h"
//#include "Enemy_Cheetah.h"
//#include "Enemy_HungryComboer.h"
//#include "Enemy_RelativeComboer.h"
//#include "Enemy_WireJuggler.h"
//#include "Enemy_CurveTurret.h"
//#include "Enemy_GravityFaller.h"
//#include "Enemy_Owl.h"
//#include "Enemy_RoadRunner.h"
//#include "Enemy_BirdBoss.h"
//#include "Enemy_PoisonFrog.h"
//#include "Enemy_Pulser.h"
//#include "Enemy_Spider.h"
//#include "Enemy_CurveLauncher.h"
//#include "Enemy_Turtle.h"
//#include "Enemy_AirdashJuggler.h"
//#include "Enemy_GravityJuggler.h"
//#include "Enemy_BounceJuggler.h"
//#include "Enemy_JugglerCatcher.h"
//#include "Enemy_BounceFloater.h"
//#include "Enemy_BounceBooster.h"
//#include "Enemy_Teleporter.h"
//#include "Enemy_Swarm.h"
//#include "Enemy_Ghost.h"
//#include "Enemy_GrowingTree.h"
//#include "Enemy_Shark.h"
//#include "Enemy_Specter.h"
//#include "Enemy_Gorilla.h"
//#include "Enemy_SwingLauncher.h"
//#include "Enemy_WireTarget.h"

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

#define COLOR_GROUND Color( 0x0d, 0x2d, 0x7d )
#define COLOR_STEEP_GROUND Color( 0x22, 0x44, 0xcc )
#define COLOR_STEEP_CEILING Color( 0x2d, 0xcd, 0xed )
#define COLOR_CEILING Color( 0x99, 0xff, 0xff )
#define COLOR_WALL Color( 0x00, 0x88, 0xcc )


GameSession * GameSession::currSession = NULL;

bool GameSession::UpdateRunModeBackAndStartButtons()
{
	Actor *p0 = GetPlayer(0);
	if (raceFight != NULL)
	{
		if (raceFight->gameOver || GetCurrInput(0).back)
		{
			gameState = RACEFIGHT_RESULTS;
			raceFight->raceFightResultsFrame = 0;
			raceFight->victoryScreen->Reset();
			raceFight->victoryScreen->SetupColumns();
			return true;
		}
	}
	else if (!p0->IsGoalKillAction(p0->action) && !p0->IsExitAction(p0->action))
	{
		ControllerState &currInput = GetCurrInput(0);
		ControllerState &prevInput = GetPrevInput(0);
		//if( IsKeyPressed( Keyboard ) )
		if (currInput.start && !prevInput.start)
		{
			gameState = PAUSE;
			ActivatePauseSound(GetSound("pause_on"));
			pauseMenu->SetTab(PauseMenu::PAUSE);
			soundNodeList->Pause(true);
			return true;
		}
		else if ((currInput.back && !prevInput.back) || IsKeyPressed(Keyboard::G))
		{
			gameState = PAUSE;
			pauseMenu->SetTab(PauseMenu::MAP);
			ActivatePauseSound(GetSound("pause_on"));
			soundNodeList->Pause(true);
			return true;
		}
	}

	return false;
}

void GameSession::UpdateEnvPlants()
{
	Actor *p0 = GetPlayer(0);
	EnvPlant *prevPlant = NULL;
	EnvPlant *ev = activeEnvPlants;
	while (ev != NULL)
	{
		EnvPlant *tempNext = ev->next;
		ev->particle->Update(p0->position);

		ev->frame++;
		if (ev->frame == ev->disperseLength * ev->disperseFactor)
		{
			VertexArray &eva = *ev->va;
			eva[ev->vaIndex + 0].position = Vector2f(0, 0);
			eva[ev->vaIndex + 1].position = Vector2f(0, 0);
			eva[ev->vaIndex + 2].position = Vector2f(0, 0);
			eva[ev->vaIndex + 3].position = Vector2f(0, 0);

			if (ev == activeEnvPlants)
			{
				activeEnvPlants = ev->next;
			}
			else
			{
				prevPlant->next = ev->next;
			}
		}
		else
		{
			prevPlant = ev;
		}

		ev = tempNext;
	}

	queryMode = QUERY_ENVPLANT;
	envPlantTree->Query(this, screenRect);
}

void GameSession::UpdateCamera()
{
	oldCamBotLeft = view.getCenter();
	oldCamBotLeft.x -= view.getSize().x / 2;
	oldCamBotLeft.y += view.getSize().y / 2;

	oldView = view;

	switch (mapHeader->gameMode)
	{
	case MapHeader::T_BASIC:
	{
		cam.SetCamType(Camera::CamType::BASIC);
		cam.playerIndex = 0;
		cam.Update();
		break;
	}
	case MapHeader::T_FIGHT:
	{
		cam.SetCamType(Camera::CamType::FIGHTING);
		cam.Update();
		//cam.playerIndex = 0;
		break;
	}
	}
	

	Vector2f camPos = cam.GetPos();
	double camWidth = 960 * cam.GetZoom();
	double camHeight = 540 * cam.GetZoom();

	screenRect = sf::Rect<double>(camPos.x - camWidth / 2, camPos.y - camHeight / 2, camWidth, camHeight);

	view.setSize(Vector2f(1920 / 2 * cam.GetZoom(), 1080 / 2 * cam.GetZoom()));

	//this is because kin's sprite is 2x size in the game as well as other stuff
	lastViewSize = view.getSize();
	view.setCenter(camPos.x, camPos.y);

	lastViewCenter = view.getCenter();
}

void GameSession::DrawShockwaves(sf::RenderTarget *target)
{
	DrawSceneToPostProcess(postProcessTex2);
	/*for (auto it = shockwaves.begin(); it != shockwaves.end(); ++it)
	{
		(*it)->Draw(postProcessTex2, target);
	}*/
}


void GameSession::DrawRaceFightScore(sf::RenderTarget *target)
{
	if (raceFight != NULL)
	{
		raceFight->DrawScore(target);
	}
}




void GameSession::DrawTerrain(sf::RenderTarget *target)
{
	PolyPtr poly = polyQueryList;
	while (poly != NULL)
	{
		poly->Draw(preScreenTex);
		poly = poly->queryNext;
	}
}

void GameSession::DrawFlyTerrain(sf::RenderTarget *target)
{
	PolyPtr fp = flyTerrainList;
	while (fp != NULL)
	{
		fp->DrawFlies(preScreenTex);
		fp = fp->queryNext;
	}
}

void GameSession::DrawSpecialTerrain(sf::RenderTarget *target)
{
	PolyPtr sp = specialPieceList;
	while (sp != NULL)
	{
		sp->Draw(preScreenTex);
		sp = sp->queryNext;
	}
}

void GameSession::UpdateReplayGhostSprites()
{
	for (auto it = replayGhosts.begin(); it != replayGhosts.end(); ++it)
	{
		(*it)->UpdateReplaySprite();
	}
}

void GameSession::TryToActivateBonus()
{
	Actor *p = NULL;
	if (parentGame == NULL && bonusGame != NULL)
	{
		if (GetCurrInputUnfiltered(0).rightShoulder &&
			!GetPrevInputUnfiltered(0).rightShoulder)
		{
			currSession = bonusGame;
			for (int i = 0; i < MAX_PLAYERS; ++i)
			{
				p = GetPlayer(i);
				if (p != NULL)
				{
					p->SetSession(bonusGame, bonusGame, NULL);
				}
			}
			pauseMenu->owner = bonusGame;

			bonusGame->Run();

			pauseMenu->owner = this;
			currSession = this;
			for (int i = 0; i < MAX_PLAYERS; ++i)
			{
				p = GetPlayer(i);
				if (p != NULL)
				{
					p->SetSession(this, this, NULL);
					p->Respawn(); //special respawn for leaving bonus later
				}
			}

		}
	}
}





void GameSession::DrawSceneToPostProcess(sf::RenderTexture *tex)
{
	Sprite blah;
	blah.setTexture(preScreenTex->getTexture());
	tex->draw(blah);
	tex->display();
}

bool GameSession::RunPreUpdate()
{
	if (IsKeyPressed(sf::Keyboard::Y))
	{
		quit = true;
		return false;
	}

	UpdateDebugModifiers();



	/*if (goalDestroyed)
	{
		quit = true;
		returnVal = resType;
		return false;
	}*/

	if (nextFrameRestart)
	{
		gameState = GameSession::RUN;
		RestartLevel();
		gameClock.restart();
		currentTime = 0;
		accumulator = TIMESTEP + .1;
		frameRateDisplay.Reset();
	}

	return true;
}

bool GameSession::RunPostUpdate()
{
	if (goalDestroyed)
	{
		quit = true;
		returnVal = resType;
		/*recGhost->StopRecording();
		recGhost->WriteToFile( "testghost.bghst" );*/
		return false;
	}

	return true;
}

void GameSession::UpdateRaceFightScore()
{
	if (raceFight != NULL)
	{
		raceFight->UpdateScore();
	}
}

void GameSession::SequenceGameModeRespondToGoalDestroyed()
{
	quit = true;
	returnVal = resType;
}




PolyPtr GameSession::GetPolygon(int index)
{
	PolyPtr terrain = NULL;
	if (index == -1)
	{
		terrain = inversePolygon;
	}
	else
	{
		if (inversePolygon != NULL)
			index++;
		terrain = allPolysVec[index];
	}

	if (terrain == NULL)
		assert(0 && "failure terrain indexing goal");

	return terrain;
}

EdgeAngleType GetEdgeAngleType(V2d &normal)
{
	if (GameSession::IsFlatGround(normal) == 0)
	{
		if (normal.y < 0)
			return EDGE_FLAT;
		else
			return EDGE_FLATCEILING;
	}
	else if (GameSession::IsSlopedGround(normal) == 1)
	{
		if (normal.y < 0)
		{
			return EDGE_SLOPED;
		}
		else
		{
			return EDGE_SLOPEDCEILING;
		}
	}
	else
	{
		int steep = GameSession::IsSteepGround(normal);
		if (steep == 2)
		{
			return EDGE_STEEPSLOPE;
		}
		else if (steep == 3)
		{
			return EDGE_STEEPCEILING;
		}
		else if( GameSession::IsWall( normal ) > 0 )
		{
			return EDGE_WALL;
		}
		else
		{
			assert(0 && "couldn't find edge angle type");
			return EDGE_FLAT;
		}
	}
}

PoiInfo::PoiInfo( const std::string &pname, Vector2i &p )
{
	name = pname;
	pos.x = p.x;
	pos.y = p.y;
	edge = NULL;
	poly = NULL;
	edgeIndex = -1;
}

PoiInfo::PoiInfo( const std::string &pname, PolyPtr p, int eIndex, double q )
{
	poly = p;
	name = pname;
	edgeIndex = eIndex;
	edgeQuantity = q;
	edge = p->GetEdge(eIndex);

	pos = edge->GetPosition( edgeQuantity );
	
}

void GameSession::Reload(const boost::filesystem::path &p_filePath)
{
	//partial cleanup
	originalMusic = NULL;

	for (int i = 0; i < allPolysVec.size(); ++i)
	{
		delete allPolysVec[i];
	}
	allPolysVec.clear();

	CleanupZones();

	CleanupBarriers();

	CleanupCameraShots();

	for (auto it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		delete (*it);
	}
	fullEnemyList.clear();

	CleanupGlobalBorders();

	for (auto it = decorLayerMap.begin(); it != decorLayerMap.end(); ++it)
	{
		delete (*it).second;
	}
	decorLayerMap.clear();

	for (auto it = allSpecialTerrain.begin(); it != allSpecialTerrain.end(); ++it)
	{
		delete (*it);
	}
	allSpecialTerrain.clear();

	/*for (auto it = flyTerrain.begin(); it != flyTerrain.end(); ++it)
	{
		delete (*it);
	}
	flyTerrain.clear();*/

	for (auto it = allEnvPlants.begin(); it != allEnvPlants.end(); ++it)
	{
		delete (*it);
	}
	allEnvPlants.clear();

	//only if the new map doesn't have a new shard??
	if (shardPop != NULL)
	{
		delete shardPop;
	}
	shardPop = NULL;

	if (getShardSeq != NULL)
	{
		delete getShardSeq;
	}
	getShardSeq = NULL;

	//might be able to setup in setuptopclouds
	CleanupTopClouds();

	CleanupGates();

	CleanupDecor();

	for (auto it = fullAirTriggerList.begin(); it != fullAirTriggerList.end(); ++it)
	{
		delete (*it);
	}
	fullAirTriggerList.clear();

	CleanupPoi();


	CleanupShipExit();

	CleanupShipEntrance();
	

	activeEnemyList = NULL;
	activeEnemyListTail = NULL;
	inversePolygon = NULL;
	inactiveEnemyList = NULL;

	ClearEffects();

	GetPlayer(0)->Respawn();
	GetPlayer(0)->Init();
	//need setup key marker
	//create actors

	//load
	filePath = p_filePath;
	filePathStr = filePath.string();
	SetContinueLoading(true);

	ResetTilesetAccessCount();

	Load();

	CleanupUnusedTilests();
}

GameSession::GameSession(SaveFile *sf, const boost::filesystem::path &p_filePath )
	:Session( Session::SESS_GAME, p_filePath), saveFile( sf )
{
	currSession = this;
	Init();
}

void GameSession::Cleanup()
{
	if (bonusGame != NULL)
	{
		delete bonusGame;
		bonusGame = NULL;
	}

	for( int i = 0; i < allPolysVec.size(); ++i)
	{
		delete allPolysVec[i];
	}
	allPolysVec.clear();

	for (auto it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		delete (*it);
	}
	fullEnemyList.clear();

	CleanupGoalFlow();

	for (auto it = decorLayerMap.begin(); it != decorLayerMap.end(); ++it)
	{
		delete (*it).second;
	}
	decorLayerMap.clear();

	for (auto it = allSpecialTerrain.begin(); it != allSpecialTerrain.end(); ++it)
	{
		delete (*it);
	}
	allSpecialTerrain.clear();

	for (auto it = allRails.begin(); it != allRails.end(); ++it)
	{
		delete (*it);
	}
	allRails.clear();

	/*for (auto it = flyTerrain.begin(); it != flyTerrain.end(); ++it)
	{
		delete (*it);
	}
	flyTerrain.clear();*/

	for (auto it = allEnvPlants.begin(); it != allEnvPlants.end(); ++it)
	{
		delete (*it);
	}
	allEnvPlants.clear();

	if ( parentGame == NULL && recGhost != NULL)
	{
		delete recGhost;
		recGhost = NULL;
	}

	CleanupGoalPulse();

	if (specterTree != NULL)
	{
		delete specterTree;
		specterTree = NULL;
	}

	if (envPlantTree != NULL)
	{
		delete envPlantTree;
		envPlantTree = NULL;
	}

	if (itemTree != NULL)
	{
		delete itemTree;
		itemTree = NULL;
	}

	if (inverseEdgeTree != NULL)
	{
		delete inverseEdgeTree;
		inverseEdgeTree = NULL;
	}

	if (terrainBGTree != NULL)
	{
		delete terrainBGTree;
		terrainBGTree = NULL;
	}

	if (railDrawTree != NULL)
	{
		delete railDrawTree;
		railDrawTree = NULL;
	}

	if (activeEnemyItemTree != NULL)
	{
		delete activeEnemyItemTree;
		activeEnemyItemTree = NULL;
	}

	if (airTriggerTree != NULL)
	{
		delete airTriggerTree;
		airTriggerTree = NULL;
	}

	CleanupDecor();

	for (auto it = fullAirTriggerList.begin(); it != fullAirTriggerList.end(); ++it)
	{
		delete (*it);
	}
}

GameSession::~GameSession()
{
	Cleanup();

	currSession = NULL;
}

GameSession *GameSession::GetSession()
{
	return currSession;
}




void GameSession::RecordReplayEnemies()
{
	Actor *player = GetPlayer( 0 );
	if( player->action == Actor::INTRO || player->action == Actor::SPAWNWAIT )
	{
		return;
	}

	int index = 0;

	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
		current->Record( index );
		//current->UpdatePrePhysics();
		current = current->next;
		++index;
	}
}

void GameSession::UnlockPower(int pType)
{
	mainMenu->GetCurrentProgress()->UnlockUpgrade(pType);
}

void GameSession::UpdateEnemiesSprites()
{
	Actor *player = GetPlayer( 0 );
	if( player->action == Actor::INTRO || player->action == Actor::SPAWNWAIT )
	{
		return;
	}

	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
	//	current->up();
		current = current->next;
	}
}

int GameSession::CountActiveEnemies()
{
	Enemy *currEnemy = activeEnemyList;
	int counter = 0;
	while( currEnemy != NULL )
	{
		if( currEnemy->type != EnemyType::EN_BASICEFFECT )
		{
			counter++;	
		}
		currEnemy = currEnemy->next;
	}

	return counter;
}

void GameSession::ProcessSpecialTerrain(PolyPtr poly)
{
	int specialType = poly->GetSpecialPolyIndex();
	if (specialType == 1)
	{
		allSpecialTerrain.push_back(poly);
		specialTerrainTree->Insert(poly);
	}
	else if (specialType == 2)
	{
		poly->AddFliesToWorldTrees();
		poly->AddFliesToQuadTree(enemyTree);
		allSpecialTerrain.push_back(poly);
		flyTerrainTree->Insert(poly);
	}
	//matSet.insert(make_pair(poly->terrainWorldType, poly->terrainVariation));
	
}

void GameSession::ProcessGate(int gCat,
	int gVar,
	int numToOpen,
	int poly0Index, int vertexIndex0, int poly1Index,
	int vertexIndex1, int shardWorld,
	int shardIndex)
{
	if (inversePolygon != NULL)
	{
		poly0Index++;
		poly1Index++;
	}

	Edge *edge0 = allPolysVec[poly0Index]->GetEdge(vertexIndex0);
	Edge *edge1 = allPolysVec[poly1Index]->GetEdge(vertexIndex1);

	V2d point0 = edge0->v0;
	V2d point1 = edge1->v0;

	Gate * gate = new Gate(this, gCat, gVar);

	if (gCat == Gate::SHARD)
	{
		gate->SetShard(shardWorld, shardIndex);
	}
	else if (gCat == Gate::KEY || gCat == Gate::PICKUP)
	{
		gate->SetNumToOpen(numToOpen);
	}

	gate->temp0prev = edge0->edge0;
	gate->temp0next = edge0;
	gate->temp1prev = edge1->edge0;
	gate->temp1next = edge1;

	gate->edgeA = new Edge;
	gate->edgeA->edgeType = Edge::CLOSED_GATE;
	gate->edgeA->info = gate;
	gate->edgeB = new Edge;
	gate->edgeB->edgeType = Edge::CLOSED_GATE;
	gate->edgeB->info = gate;

	gate->edgeA->v0 = point0;
	gate->edgeA->v1 = point1;

	gate->edgeB->v0 = point1;
	gate->edgeB->v1 = point0;

	gate->next = NULL;
	gate->prev = NULL;

	gate->CalcAABB();

	gates.push_back(gate);

	gate->SetLocked(true);

	gate->Init();

	terrainTree->Insert(gate->edgeA);
	terrainTree->Insert(gate->edgeB);

	cout << "inserting gate: " << gate->edgeA << endl;
	gateTree->Insert(gate);

	gate->Reset();
}

void GameSession::ProcessRail(RailPtr rail)
{
	railDrawTree->Insert(rail);
	totalRails++; //is this really even needed?
	allRails.push_back(rail);
}

void GameSession::ProcessHeader()
{
	/*if (mapHeader->gameMode == MapHeader::MapType::T_RACEFIGHT)
	{
		assert(raceFight == NULL);
		raceFight = new RaceFight(this, 180);
	}*/
}

void GameSession::ProcessDecorSpr(const std::string &name,
	Tileset *d_ts, int dTile, int dLayer, sf::Vector2f &centerPos,
	float rotation, sf::Vector2f &scale)
{
	Sprite dSpr;
	dSpr.setScale(scale);
	dSpr.setRotation(rotation);
	dSpr.setPosition(centerPos);

	dSpr.setTexture(*d_ts->texture);
	dSpr.setTextureRect(d_ts->GetSubRect(dTile));
	dSpr.setOrigin(dSpr.getLocalBounds().width / 2, dSpr.getLocalBounds().height / 2);


	decorListMap[name].push_back(DecorInfo(dSpr, dLayer, d_ts, dTile));
}

void GameSession::ProcessAllDecorSpr()
{
	for (auto it = decorListMap.begin(); it != decorListMap.end(); ++it)
	{
		int numBetweenLayer = 0;
		list<DecorInfo> betweenList;
		for (auto it2 = (*it).second.begin(); it2 != (*it).second.end(); ++it2)
		{
			if ((*it2).layer == 0)
			{
				numBetweenLayer++;
				betweenList.push_back((*it2));
			}
		}

		if (numBetweenLayer > 0)
		{
			Vertex *betweenVerts = new Vertex[numBetweenLayer * 4];
			int vi = 0;
			for (auto itb = betweenList.begin(); itb != betweenList.end(); ++itb)
			{
				Sprite &s = (*itb).spr;
				double rot = s.getRotation() / 180.f * PI;
				float xSize = s.getTextureRect().width * s.getScale().x;
				float ySize = s.getTextureRect().height * s.getScale().y;
				Vector2f pos = s.getPosition();
				FloatRect sub = FloatRect(s.getTextureRect());
				SetRectRotation(betweenVerts + vi * 4, rot, xSize, ySize, pos);
				SetRectSubRect(betweenVerts + vi * 4, sub);
				vi++;
			}

			decor[BETWEEN_PLAYER_AND_ENEMIES].push_back(new DecorDraw(betweenVerts,
				numBetweenLayer * 4, betweenList.front().ts));
		}
	}

	decorListMap.clear();
}

void GameSession::ProcessTerrain(PolyPtr poly)
{
	matSet.insert(make_pair(poly->terrainWorldType, poly->terrainVariation));	
	allPolygonsList.push_back(poly);
}

void GameSession::ProcessAllTerrain()
{
	for (auto it = terrainDecorInfoMap.begin(); it != terrainDecorInfoMap.end(); ++it)
	{
		delete (*it).second;
	}
	terrainDecorInfoMap.clear();

	AllocatePolyShaders(matSet.size());

	int index = 0;
	for (set<pair<int, int>>::iterator it = matSet.begin(); it != matSet.end(); ++it)
	{
		if (!LoadPolyShader(index, (*it).first, (*it).second))
		{
			assert(0);
		}
		++index;
	}

	PolyPtr poly;
	allPolysVec.reserve(allPolygonsList.size());
	for (auto it = allPolygonsList.begin(); it != allPolygonsList.end(); ++it)
	{
		poly = (*it);
		poly->Finalize();
		poly->AddEdgesToQuadTree(terrainTree);
		//poly->AddGrassToQuadTree(grassTree);

		if (poly->inverse)
		{
			poly->AddEdgesToQuadTree(inverseEdgeTree);
			inversePolygon = poly;
		}
		borderTree->Insert(poly);

		allPolysVec.push_back((*it));
	}
	allPolygonsList.clear();
}

void GameSession::ProcessActor(ActorPtr a)
{
	Enemy *enemy = a->GenerateEnemy();

	const string &typeName = a->type->info.name;
	if (enemy != NULL)
	{
		fullEnemyList.push_back(enemy);
		enemyTree->Insert(enemy);

		enemy->AddToWorldTrees(); //adds static objects etc

		if (typeName == "shippickup")
		{
			if (shipExitScene == NULL)
			{
				shipExitScene = new ShipExitScene;
				shipExitScene->Init();
			}
		}
	}
	else
	{
		if (typeName == "xbarrier")
		{
			XBarrierParams *xbp = (XBarrierParams*)a;
			AddBarrier(xbp);
		}
		//else if (typeName == "extrascene")
		//{
		//	ExtraSceneParams *xp = (ExtraSceneParams*)a;
		//	BasicBossScene *scene = BasicBossScene::CreateScene(xp->GetName());
		//	if (xp->extraSceneType == 0)//prelevel
		//	{
		//		preLevelScene = scene;
		//	}
		//	else if (xp->extraSceneType == 1)//postlevel
		//	{
		//		postLevelScene = scene;
		//	}
		//}
		else if (typeName == "camerashot")
		{
			CameraShotParams *csp = (CameraShotParams*)a;
			AddCameraShot(csp);
		}
		else if (typeName == "poi")
		{
			PoiParams *pp = (PoiParams*)a;
			AddPoi(pp);
			
		}
		else if (typeName == "birdnode" || typeName == "coyotenode" 
			|| typeName == "tigernode" || typeName == "gatornode"
			|| typeName == "skeletonnode" )
		{
			PoiParams *pp = (PoiParams*)a;
			AddBossNode( typeName, pp);
		}
		else if (typeName == "ship")
		{
			if (shipEnterScene == NULL)
			{
				shipEnterScene = new ShipEnterScene;
				shipEnterScene->Init();
				shipEnterScene->shipEntrancePos = a->GetPosition();
				//shipEnterScene->Reset();
			}
			//shipEntrancePos = a->GetPosition();
			//hasShipEntrance = true;
		
			//ResetShipSequence();
		}
		else if (typeName == "zoneproperties")
		{
			ZonePropertiesParams *zp = (ZonePropertiesParams*)a;
			ZonePropertiesObj *obj = new ZonePropertiesObj(zp->GetIntPos(), zp->zoneType,
				zp->drainFactor);
			zoneObjects.push_back(obj);
		}
		else
		{
			cout << "cant handle enemy of type: " << typeName << endl;
			assert(0);
		}
	}

	delete a; //eventually probably delete these all at once or something
}

void GameSession::ProcessAllActors()
{
	CreateBulletQuads();


	if (raceFight != NULL)
	{
		raceFight->Init();
	}

	if (mapHeader->preLevelSceneName != "NONE")
	{
		preLevelScene = Sequence::CreateScene(mapHeader->preLevelSceneName);
	}

	//create sequences for the barriers after all enemies have already been loaded
	SetupBarrierScenes();

	SetupEnemyZoneSprites();
	
}

bool cmpPairs(pair<double,int> & a, pair<double,int> & b)
{
	return a.first < b.first;
}

bool cmpPairsDesc( pair<double,int> & a, pair<double,int> & b)
{
	return a.first > b.first;
}



int GameSession::GetPlayerEnemiesKilledLastFrame(int index )
{
	Actor *p = players[index];
	if (p != NULL)
	{
		return p->enemiesKilledLastFrame;
	}
	else
	{
		return 0;
	}
}

void GameSession::PlayerRestoreDoubleJump(int index )
{
	Actor *p = players[index];
	if (p != NULL)
	{
		p->RestoreDoubleJump();
	}
}

void GameSession::PlayerRestoreAirDash(int index )
{
	Actor *p = players[index];
	if (p != NULL)
	{
		p->RestoreAirDash();
	}
}

int GameSession::GetPlayerHitstunFrames(int index)
{
	Actor *p = players[index];
	if (p != NULL)
	{
		return p->hitstunFrames;
	}
	else
	{
		return 0;
	}
}

int GameSession::GetPlayerTeamIndex(int index)
{
	Actor *p = players[index];
	if (p != NULL)
	{
		return p->team;
	}
	else
	{
		return -1;
	}
}

void GameSession::KeyboardUpdate( int index )
{
	bool up = IsKeyPressed( Keyboard::Up );// || IsKeyPressed( Keyboard::W );
	bool down = IsKeyPressed( Keyboard::Down );// || IsKeyPressed( Keyboard::S );
	bool left = IsKeyPressed( Keyboard::Left );// || IsKeyPressed( Keyboard::A );
	bool right = IsKeyPressed( Keyboard::Right );// || IsKeyPressed( Keyboard::D );

//	bool altUp = IsKeyPressed( Keyboard::U );
//	bool altLeft = IsKeyPressed( Keyboard::H );
//	bool altRight = IsKeyPressed( Keyboard::K );
//	bool altDown = IsKeyPressed( Keyboard::J );

	ControllerState keyboardInput;    
	keyboardInput.B = IsKeyPressed( Keyboard::X );// || IsKeyPressed( Keyboard::Period );
	keyboardInput.rightShoulder = IsKeyPressed( Keyboard::C );// || IsKeyPressed( Keyboard::Comma );
	keyboardInput.Y = IsKeyPressed( Keyboard::D );// || IsKeyPressed( Keyboard::M );
	keyboardInput.A = IsKeyPressed( Keyboard::Z ) || IsKeyPressed( Keyboard::Space );// || IsKeyPressed( Keyboard::Slash );
	//keyboardInput.leftTrigger = 255 * (IsKeyPressed( Keyboard::F ) || IsKeyPressed( Keyboard::L ));
	keyboardInput.leftShoulder = IsKeyPressed( Keyboard::LShift );
	keyboardInput.X = IsKeyPressed( Keyboard::F );
	keyboardInput.start = IsKeyPressed( Keyboard::J );
	keyboardInput.back = IsKeyPressed( Keyboard::H );
	keyboardInput.rightTrigger = 255 * IsKeyPressed( Keyboard::LControl );
	keyboardInput.leftTrigger = 255 * IsKeyPressed( Keyboard::RControl );
			
	keyboardInput.rightStickPad = 0;
	if( IsKeyPressed( Keyboard::A ) )
	{
		keyboardInput.rightStickPad += 1 << 1;
	}
	else if( IsKeyPressed( Keyboard::S ) )
	{
		keyboardInput.rightStickPad += 1;
	}
				

				
				
	//keyboardInput.rightStickMagnitude
				

	/*if( altRight )
		currInput .altPad += 1 << 3;
	if( altLeft )
		currInput .altPad += 1 << 2;
	if( altUp )
		currInput .altPad += 1;
	if( altDown )
		currInput .altPad += 1 << 1;*/
				

	ControllerState &prevInput = GetPrevInput(index);

	if( up && down )
	{
		
		if( prevInput.LUp() )
			keyboardInput.leftStickPad += 1;
		else if( prevInput.LDown() )
			keyboardInput.leftStickPad += ( 1 && down ) << 1;
	}
	else
	{
		keyboardInput.leftStickPad += 1 && up;
		keyboardInput.leftStickPad += ( 1 && down ) << 1;
	}

	if( left && right )
	{
		if( prevInput.LLeft() )
		{
			keyboardInput.leftStickPad += ( 1 && left ) << 2;
		}
		else if( prevInput.LRight() )
		{
			keyboardInput.leftStickPad += ( 1 && right ) << 3;
		}
	}
	else
	{
		keyboardInput.leftStickPad += ( 1 && left ) << 2;
		keyboardInput.leftStickPad += ( 1 && right ) << 3;
	}

	GetCurrInput(index) = keyboardInput;
}

bool GameSession::sLoad( GameSession *gs )
{
	
	gs->SetContinueLoading(true);

	
	return gs->Load();
}

void GameSession::SetContinueLoading( bool cont )
{
	continueLoadingLock.lock();

	continueLoading = cont;

	continueLoadingLock.unlock();
}

bool GameSession::ShouldContinueLoading()
{
	bool b;
	continueLoadingLock.lock();
		b = continueLoading;
	continueLoadingLock.unlock();

	return b;
}

bool GameSession::Load()
{
	//sf::sleep(sf::seconds(2));
	//while (true);
	//cout << "start load" << endl;
	//sf::sleep(sf::seconds(5));
	//return true;

	//testEmit = new BoxEmitter(4, 100, PI, PI / 6, 1, 5, 1000, 1000);



	//testEmit = new LeafEmitter;// (4, 1000);// PI, PI / 6, 0, 0, 1000, 1000);
	//testEmit->SetTileset(GetTileset("Env/leaves_128x128.png", 128, 128));
	//testEmit->CreateParticles();
	
	//testEmit->SetRatePerSecond(120);

	//testEmit->posSpawner = new BoxPosSpawner(400, 400);
		//leaf_1_128x128
	//for (int i = 0; i < 50; ++i)
	//{
	//	if (!speedBarShader.loadFromFile("Resources/Shader/speedbar_shader.frag", sf::Shader::Fragment))
	//	{
	//		cout << "speed bar SHADER NOT LOADING CORRECTLY" << endl;
	//		//assert( 0 && "polygon shader not loaded" );
	//	}
	//	speedBarShader.setUniform("u_texture", sf::Shader::CurrentTexture);
	//}

	//while (true);

	AddGeneralEnemies();
	AddW1Enemies();
	AddW2Enemies();
	AddW3Enemies();
	AddW4Enemies();
	AddW5Enemies();
	AddW6Enemies();
	SetupEnemyTypes();

	SetupHitboxManager();
	SetupSoundManager();
	SetupSoundLists();

	SetupShardsCapturedField();
	

	//return true;

	if( progressDisplay != NULL )
		progressDisplay->SetProgressString("started loading!", 0);

	if (!ShouldContinueLoading())
	{
		cout << "cleanup 0" << endl;
		Cleanup();
		return false;
	}

	//return true;
	
	//inputVis = new InputVisualizer;


	SetupAbsorbParticles();

	const ConfigData &cd = mainMenu->config->GetData();
	soundNodeList->SetSoundVolume(cd.soundVolume);
	pauseSoundNodeList->SetSoundVolume(cd.soundVolume);

	SetupScoreDisplay();

	SetupShaders();

	if (!ShouldContinueLoading())
	{
		cout << "cleanup blabhagbo4a" << endl;
		Cleanup();
		return false;
	}

	SetupQuadTrees();	

	cout << "weird timing 1" << endl;

	AllocateEffects();
	
	if (!ShouldContinueLoading())
	{
		cout << "cleanup A" << endl;
		Cleanup();
		
		return false;
	}

	cout << "weird timing 2" << endl;

	//blah 2

	gameSoundBuffers[S_KEY_COMPLETE_W1] = GetSound( "key_complete_w1.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W2] = GetSound( "key_complete_w2.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W3] = GetSound( "key_complete_w2.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W4] = GetSound( "key_complete_w2.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W5] = GetSound( "key_complete_w2.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W6] = GetSound( "key_complete_w6.ogg" );
	gameSoundBuffers[S_KEY_ENTER_0] = GetSound( "key_enter_1.ogg" );
	gameSoundBuffers[S_KEY_ENTER_1] = GetSound( "key_enter_1.ogg" );
	gameSoundBuffers[S_KEY_ENTER_2] = GetSound( "key_enter_2.ogg" );
	gameSoundBuffers[S_KEY_ENTER_3] = GetSound( "key_enter_3.ogg" );
	gameSoundBuffers[S_KEY_ENTER_4] = GetSound( "key_enter_4.ogg" );
	gameSoundBuffers[S_KEY_ENTER_5] = GetSound( "key_enter_5.ogg" );
	gameSoundBuffers[S_KEY_ENTER_6] = GetSound( "key_enter_6.ogg" );

	//blah 3

	cutPlayerInput = false;
	activeEnvPlants = NULL;
	totalGameFrames = 0;	
	totalFramesBeforeGoal = -1;
	originalZone = NULL;
	
	unlockedGateList = NULL;
	activatedZoneList = NULL;


	activeSequence = NULL;

	//view = View( Vector2f( 300, 300 ), sf::Vector2f( 960 * 2, 540 * 2 ) );

	//repGhost = new ReplayGhost( player );

	//recPlayer = new RecordPlayer( player );
	//repPlayer = new ReplayPlayer( player );


	cout << "weird timing 3" << endl;
	if (!ShouldContinueLoading())
	{
		cout << "cleanup B" << endl;
		Cleanup();
		
		return false;
	}
	cout << "weird timing 4" << endl;

	if (parentGame != NULL)
	{
		players[0] = parentGame->players[0];
	}
	else
	{
		if (players[0] == NULL)
			players[0] = new Actor(this, NULL, 0);
	}
	
	cout << "about to open file" << endl;
	
	if (progressDisplay != NULL)
		progressDisplay->SetProgressString("opening map file!", 1);

	matSet.clear();

	ReadFile();

	SetupPlayers();

	

	//for (int i = 1; i < mapHeader->GetNumPlayers(); ++i)
	//{
	//	players[i] = new Actor(this, NULL, i);
	//}
	//m_numActivePlayers = mapHeader->GetNumPlayers(); //not really used

	SetupGameMode();
	gameMode->Setup();

	

	SetupHUD();

	bool blackBorder[2];
	bool topBorderOn = false;
	SetupGlobalBorderQuads(blackBorder, topBorderOn);
	hud->mini->SetupBorderQuads(blackBorder, topBorderOn, mapHeader);


	kinMapSpawnIcon.setTexture(*hud->mini->ts_miniIcons->texture);
	kinMapSpawnIcon.setTextureRect(hud->mini->ts_miniIcons->GetSubRect(1));
	kinMapSpawnIcon.setOrigin(kinMapSpawnIcon.getLocalBounds().width / 2,
		kinMapSpawnIcon.getLocalBounds().height / 2);


	if (topBorderOn)
	{
		topClouds = new TopClouds;
		topClouds->SetToHeader();
	}

	

	CreateZones();

	SetupGateMarkers();

	SetupZones();

	

	SetupBackground();
	

	//still too far


	//cout << "done opening file" << endl;

	//SetupPlayers();

	
	SetupTimeBubbles();


	SetPlayersGameMode();

	SetupDeathSequence();

	/*for (auto it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		(*it)->Init();
	}*/

	
	//too far
	

	SetupRecGhost();
	

	SetupPauseMenu();

	SetupControlProfiles();

	

	GetPlayer(0)->SetupDrain();

	if (hasGoal)
	{
		SetupGoalFlow();
		SetupGoalPulse();
	}
	else
	{
		CleanupGoalPulse();
		CleanupGoalFlow();
	}
	
	cam.Init(GetPlayerPos(0));

	if (!ShouldContinueLoading())
	{
		cout << "cleanup FF" << endl;
		Cleanup();
		return false;
	}

	
	
	cout << "last one" << endl;
	for( auto it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it )
	{
		(*it)->Setup();
	}
	
	if (parentGame == NULL)
	{

		//this is a temporary test. create a bonus whenever you have a parent level
		/*boost::filesystem::path p("Resources/Maps//W2//gateblank9.brknk");
		bonusGame = new GameSession(saveFile, p);
		bonusGame->SetParentGame(this);
		bonusGame->Load();

		currSession = this;
		pauseMenu->owner = this;*/
	}

	//bonusPaths.push_back(p);

	/*for (auto it = bonusPaths.begin(); it != bonusPaths.end(); ++it)
	{
		
	}*/

	cout << "done loading" << endl;

	if (progressDisplay != NULL)
		progressDisplay->SetProgressString("done loading!", 0);

	return true;
}

void GameSession::SetupPlayers()
{
	if (parentGame != NULL)
	{
		for (int i = 1; i < MAX_PLAYERS; ++i)
		{
			players[i] = parentGame->players[i];
			//if( players[i] != NULL )
			//	players[i]->Respawn(); //need a special bonus respawn later
		}

		m_numActivePlayers = parentGame->m_numActivePlayers;
		return;
	}
	else
	{
		for (int i = 1; i < mapHeader->GetNumPlayers(); ++i)
		{
			players[i] = new Actor(this, NULL, i);
			//if( players[i] != NULL )
			//	players[i]->Respawn(); //need a special bonus respawn later
		}
	}

	Actor *p;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->position = V2d(playerOrigPos[i]);
		}
	}

	/*if (raceFight != NULL)
	{
		if( players[1] == NULL )
			players[1] = new Actor(this, NULL, 1);
	}
	else
	{
		players[1] = NULL;
	}

	players[2] = NULL;
	players[3] = NULL;

	Actor *tempP = NULL;;
	for (int i = 1; i < 4; ++i)
	{
		tempP = GetPlayer(i);
		if (tempP != NULL)
		{
			tempP->position = GetPlayer(0)->position;
		}
	}*/

	/*m_numActivePlayers = 0;
	Actor *activePlayer = NULL;
	Actor *tempPlayer = NULL;
	for (int i = 0; i < 4; ++i)
	{
		if (tempPlayer = GetPlayer(i))
		{
			if (!activePlayer)
			{
				activePlayer = tempPlayer;
			}
			++m_numActivePlayers;
		}
	}
	assert(activePlayer);*/
}

void GameSession::SetupShaders()
{
	//since these are not pointers, cannot transfer them from the parentGame. Might want to change that?

	if (shadersLoaded)
		return;

	shadersLoaded = true;

	if (!timeSlowShader.loadFromFile("Resources/Shader/clone_shader.frag", sf::Shader::Fragment))
	{
		cout << "CLONE SHADER NOT LOADING CORRECTLY" << endl;
	}
}

void GameSession::SetupBackground()
{
	/*if (parentGame != NULL)
	{
		if (mapHeader->envName == parentGame->mapHeader->envName)
		{
			background = parentGame->background;
			return;
		}
	}*/
	
	if (background != NULL)
	{
		delete background;
		background = NULL;
	}

	background = Background::SetupFullBG(mapHeader->envName, this);
}



void GameSession::SetupQuadTrees()
{
	//bonus levels have their own trees

	if (terrainBGTree != NULL)
	{
		terrainBGTree->Clear();
		terrainTree->Clear();
		barrierTree->Clear();
		specialTerrainTree->Clear();
		flyTerrainTree->Clear();
		inverseEdgeTree->Clear();
		staticItemTree->Clear();
		railDrawTree->Clear();
		railEdgeTree->Clear();
		enemyTree->Clear();
		borderTree->Clear();
		grassTree->Clear();
		gateTree->Clear();
		itemTree->Clear();
		envPlantTree->Clear();
		specterTree->Clear();
		activeItemTree->Clear();
		activeEnemyItemTree->Clear();
		airTriggerTree->Clear();
		return;
	}

	terrainBGTree = new QuadTree(1000000, 1000000);
	//soon make these the actual size of the bordered level
	terrainTree = new QuadTree(1000000, 1000000);

	barrierTree = new QuadTree(1000000, 1000000);

	specialTerrainTree = new QuadTree(1000000, 1000000);

	flyTerrainTree = new QuadTree(1000000, 1000000);

	inverseEdgeTree = new QuadTree(1000000, 1000000);

	staticItemTree = new QuadTree(1000000, 1000000);
	railDrawTree = new QuadTree(1000000, 1000000);
	railEdgeTree = new QuadTree(1000000, 1000000);

	enemyTree = new QuadTree(1000000, 1000000);

	borderTree = new QuadTree(1000000, 1000000);

	grassTree = new QuadTree(1000000, 1000000);

	gateTree = new QuadTree(1000000, 1000000);

	itemTree = new QuadTree(1000000, 1000000);

	envPlantTree = new QuadTree(1000000, 1000000);

	specterTree = new QuadTree(1000000, 1000000);

	activeItemTree = new QuadTree(1000000, 1000000);

	activeEnemyItemTree = new QuadTree(1000000, 1000000);

	airTriggerTree = new QuadTree(1000000, 1000000);
}

void GameSession::SetupRecGhost()
{
	//will need to figure out how to do this later for recordings transferring through to bonus levels
	if (parentGame != NULL)
	{
		recGhost = parentGame->recGhost;
	}
	else if (mapHeader->gameMode == MapHeader::MapType::T_BASIC && recGhost == NULL)
	{
		recGhost = new RecordGhost(GetPlayer(0));
	}
}

void GameSession::SetupPauseMenu()
{
	pauseMenu = mainMenu->pauseMenu;
	pauseMenu->owner = this;

	if (parentGame == NULL)
	{
		pauseMenu->SetTab(PauseMenu::PAUSE);
	}
}

bool GameSession::SetupControlProfiles()
{
	if (parentGame != NULL)
		return true;

	ControlProfile *currProfile;
	SaveFile *currFile = mainMenu->GetCurrentProgress();
	if (currFile != NULL)
	{
		bool set = pauseMenu->controlSettingsMenu->pSel->SetCurrProfileByName(currFile->controlProfileName);
		if (!set)
		{
			//error. profile does not exist
			currFile->controlProfileName = "KIN Default";
			currFile->Save();
		}
	}
	for (int i = 0; i < 1; ++i)
	{
		//temporary
		//mainMenu->GetController(i).SetFilter( pauseMenu->cOptions->xboxInputAssoc[0] );
		currProfile = pauseMenu->GetCurrSelectedProfile();
		GameController &con = GetController(i);
		currProfile->tempCType = con.GetCType();
		con.SetFilter(currProfile->GetCurrFilter());//mainMenu->cpm->profiles.front()->filter );
	}

	return true;
}

void GameSession::SetupGhosts(std::list<GhostEntry*> &ghostEntries)
{
	for( auto it = ghostEntries.begin(); it != ghostEntries.end(); ++it )
	{
		boost::filesystem::path &p = (*it)->gPath;
		ReplayGhost *rg = new ReplayGhost(players[0]);
		rg->OpenGhost(p);
		replayGhosts.push_back(rg);

		rg->frame = 0;
	}
}



int GameSession::Run()
{
	goalDestroyed = false;
	frameRateDisplay.showFrameRate = true;
	runningTimerDisplay.showRunningTimer = true;

	ClearEmitters();
	bool oldMouseGrabbed = mainMenu->GetMouseGrabbed();
	bool oldMouseVisible = mainMenu->GetMouseVisible();

	mainMenu->SetMouseGrabbed(true);
	mainMenu->SetMouseVisible(false);

	currStorySequence = NULL;

	View oldPreTexView = preScreenTex->getView();
	View oldWindowView = window->getView();

	preScreenTex->setView(view);
	
	Actor *p0 = GetPlayer(0);
	Actor *p = NULL;

	sf::CircleShape circle(30);
	circle.setFillColor(Color::Blue);

	sf::Clock gameClock;
	double currentTime = 0;
	accumulator = TIMESTEP + .1;

	Vector2<double> otherPlayerPos;

	double zoomMultiple = 1;

	Color borderColor = sf::Color::Green;
	int max = 1000000;
	sf::Vertex border[] =
	{
		sf::Vertex(sf::Vector2<float>(-max, -max), borderColor),
		sf::Vertex(sf::Vector2<float>(-max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(-max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, -max), borderColor),
		sf::Vertex(sf::Vector2<float>(max, -max), borderColor),
		sf::Vertex(sf::Vector2<float>(-max, -max), borderColor)
	};

	
	skipped = false;
	oneFrameMode = false;
	quit = false;

	returnVal = GR_EXITLEVEL;

	goalDestroyed = false;

	debugScreenRecorder = NULL;

	//debugScreenRecorder = new ScreenRecorder("BACKWARDS_DASH_JUMP");

	View v;
	v.setCenter(0, 0);
	v.setSize(1920 / 2, 1080 / 2);
	window->setView(v);
	
	frameRateDisplay.Reset();

	int flowSize = 64;

	if (raceFight != NULL)
	{
		raceFight->victoryScreen->Reset();
		gameState = RUN;//RACEFIGHT_RESULTS;
		raceFight->place[0] = 1;
		raceFight->place[1] = 2;

		raceFight->raceFightResultsFrame = 0;
		raceFight->victoryScreen->SetupColumns();
	}
	else
	{
		gameState = RUN;
	}

	//might move replay stuff later
	cout << "loop about to start" << endl;

	if (recGhost != NULL)
	{
		recGhost->StartRecording();
	}

	/*if (repGhost != NULL)
	{
		repGhost->OpenGhost("testghost.bghst");
	}*/

	if (recPlayer != NULL)
		recPlayer->StartRecording();

	if (repPlayer != NULL)
		repPlayer->OpenReplay("testreplay.brep");

	testBuf.byteIndex = 0;

	boost::thread *threa = NULL;
	ofstream of;
	if (recPlayer != NULL)//&& !repPlayer->init )
	{
		of.open("tempreplay.brep", ios::binary | ios::out);
		threa = new boost::thread(&(Buf::ThreadedBufferWrite), &testBuf, &of );
	}

	//mainMenu->musicPlayer->StopCurrentMusic();
	SetOriginalMusic();

	std::stringstream ss;
	switchGameState = false;

	if (GetPlayer(0)->action == Actor::INTROBOOST)
	{
		//Fade(true, 60, Color::Black, true);
	}
	
	if (preLevelScene != NULL)
	{
		SetActiveSequence(preLevelScene);
	}
	else if( shipEnterScene != NULL )
	{
		shipEnterScene->Reset();
		SetActiveSequence(shipEnterScene);
	}

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (GetPlayer(i) != NULL)
		{
			SetPlayerOptionField(i);
		}
	}

	if (parentGame != NULL)
	{
		for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			p = GetPlayer(i);
			if (p != NULL)
			{
				p->Respawn(); //need a special bonus respawn later
			}
		}
	}
	
	gameMode->StartGame();
	currSuperPlayer = NULL;

	while( !quit )
	{
		switchGameState = false;
		double newTime = gameClock.getElapsedTime().asSeconds();
		double frameTime = newTime - currentTime;

		if ( frameTime > 0.25 )
		{
			frameTime = 0.25;	
		}
		//frameTime = 0.167;//0.25;	
        currentTime = newTime;

		frameRateDisplay.Update(frameTime);
		UpdateRunningTimerText();

		accumulator += frameTime;

		if(gameState == RUN )
		{
			window->clear(Color::Red);
			preScreenTex->clear(Color::Red);
			postProcessTex2->clear(Color::Red);

			if (!RunGameModeUpdate())
			{
				continue;
			}

			sf::Event ev;
			while (window->pollEvent(ev))
			{
				if (ev.type == Event::LostFocus)
				{
					if (gameState == RUN)
						gameState = PAUSE;
				}
				else if (ev.type == sf::Event::GainedFocus)
				{
					//if( state == PAUSE )
					//	state = RUN;
				}
			}

			DrawGame(preScreenTex);

			preScreenTex->display();

			const Texture &preTex0 = preScreenTex->getTexture();
			Sprite preTexSprite(preTex0);
			preTexSprite.setPosition(-960 / 2, -540 / 2);
			preTexSprite.setScale(.5, .5);
			preTexSprite.setTexture(preTex0);

			if (debugScreenRecorder != NULL)
				debugScreenRecorder->Update(preTex0);

			window->draw(preTexSprite);//, &timeSlowShader );
		}
		else if(gameState == FROZEN )
		{
			window->clear();

			sf::Event ev;
			while (window->pollEvent(ev))
			{
			}

			if (!FrozenGameModeUpdate())
			{
				continue;
			}

			Sprite preTexSprite;
			preTexSprite.setTexture(preScreenTex->getTexture());
			preTexSprite.setPosition(-960 / 2, -540 / 2);
			preTexSprite.setScale(.5, .5);
			window->draw(preTexSprite);
		}
		else if(gameState == MAP )
		{
			window->clear();

			window->setView( v );

			View bigV;
			bigV.setCenter( 0, 0 );
			bigV.setSize( 1920, 1080 );

			Sprite preTexSprite;
			preTexSprite.setTexture( preScreenTex->getTexture() );
			preTexSprite.setPosition( -960 / 2, -540 / 2 );
			preTexSprite.setScale( .5, .5 );
			window->draw( preTexSprite );

			accumulator += frameTime;

			while ( accumulator >= TIMESTEP  )
			{
				UpdateControllers();

				if( ( GetCurrInput( 0 ).back && !GetPrevInput( 0 ).back ) && gameState == MAP )
				{
					gameState = RUN;
					soundNodeList->Pause( false );
					ActivatePauseSound(GetSound("pause_off"));
				}

				accumulator -= TIMESTEP;
			}
		
			View vv;
			vv.setCenter( pauseMenu->mapCenter );
			vv.setSize(  mapTex->getSize().x * pauseMenu->mapZoomFactor, mapTex->getSize().y * pauseMenu->mapZoomFactor );

			mapTex->clear();
			mapTex->setView( vv );
			mapTex->clear( Color( 0, 0, 0, 255 ) );
			
			View vuiView;
			vuiView.setSize( Vector2f( mapTex->getSize().x * 1.f, mapTex->getSize().y * 1.f ) );
			vuiView.setCenter( 0, 0 );
			
			for( list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it )
			{
				(*it)->Draw( mapTex );
			}

			queryMode = QUERY_BORDER;
			numBorders = 0;
			polyQueryList = NULL;
			sf::Rect<double> mapRect(vv.getCenter().x - vv.getSize().x / 2.0,
				vv.getCenter().y - vv.getSize().y / 2.0, vv.getSize().x, vv.getSize().y );

			borderTree->Query( this, mapRect );

			DrawColoredMapTerrain(mapTex, Color(Color::Green));

			testGateCount = 0;
			queryMode = QUERY_GATE;
			gateList = NULL;
			gateTree->Query( this, mapRect );
			Gate *mGateList = gateList;
			while( gateList != NULL )
			{
				gateList->MapDraw(mapTex);

				Gate *next = gateList->next;//edgeA->edge1;
				gateList = next;
			}

			if( currentZone != NULL )
			{
				for( list<Enemy*>::iterator it = currentZone->allEnemies.begin(); it != currentZone->allEnemies.end(); ++it )
				{
					//cout << "drawing map" << endl;
					(*it)->DrawMinimap( mapTex );
					/*if( (*it)->spawned && !(*it)->dead )
					{

					}*/
				}
			}

			Vector2i b = mapTex->mapCoordsToPixel( Vector2f( p0->position.x, p0->position.y ) );

			mapTex->setView( vuiView );

			Vector2f realPos = mapTex->mapPixelToCoords( b );
			realPos.x = floor( realPos.x + .5f );
			realPos.y = floor( realPos.y + .5f );

			//cout << "vuiVew size: " << vuiView.getSize().x << ", " << vuiView.getSize().y << endl;
			//kinMinimapIcon.setPosition( realPos );
			//mapTex->draw( kinMinimapIcon );

			mapTex->setView( vv );			

			Vector2i b1 = mapTex->mapCoordsToPixel( Vector2f(playerOrigPos[0].x, playerOrigPos[0].y ) );

			mapTex->setView( vuiView );

			Vector2f realPos1 = mapTex->mapPixelToCoords( b1 );
			realPos1.x = floor( realPos1.x + .5f );
			realPos1.y = floor( realPos1.y + .5f );

			//cout << "vuiVew size: " << vuiView.getSize().x << ", " << vuiView.getSize().y << endl;
			kinMapSpawnIcon.setPosition( realPos1 );
			mapTex->draw( kinMapSpawnIcon );
			
			mapTex->setView( vv );

			Vector2i bGoal = mapTex->mapCoordsToPixel( Vector2f( goalPos.x, goalPos.y ) );

			mapTex->setView( vuiView );

			Vector2f realPosGoal = mapTex->mapPixelToCoords( bGoal );
			realPosGoal.x = floor( realPosGoal.x + .5f );
			realPosGoal.y = floor( realPosGoal.y + .5f );

			Sprite mapTexSprite;
			mapTexSprite.setTexture( mapTex->getTexture() );
			mapTexSprite.setOrigin( mapTexSprite.getLocalBounds().width / 2, mapTexSprite.getLocalBounds().height / 2 );
			mapTexSprite.setPosition( 0, 0 );

			mapTexSprite.setScale( .5, -.5 );
			//cout << "size: " << mapTexSprite.getLocalBounds().width << ", " << mapTexSprite.getLocalBounds().height << endl;
			window->draw( mapTexSprite );
		}
		else if(gameState == RACEFIGHT_RESULTS )
		{
			/*quit = true;
			returnVal = 1;
			break;*/
			//TODO
			window->setView( v );

			preScreenTex->clear();
			window->clear();

			preScreenTex->setView( uiView );

			accumulator += frameTime;

			while ( accumulator >= TIMESTEP  )
			{
				accumulator -= TIMESTEP;
			

			
				UpdateControllers();

			//if( GetCurrInput( 0 ).start )
			//{
			//	//break;
			//	quit = true;
			//	returnVal = 1;

			//	break;
			//}

			raceFight->victoryScreen->Update();

			if (raceFight->victoryScreen->IsDone())
			{
				quit = true;
				returnVal = GR_EXITLEVEL;
				break;
			}
			//raceFight->testWindow->Update( GetCurrInput( 0 ), GetPrevInput( 0 ) );

			}

			raceFight->victoryScreen->Draw( preScreenTex );
			//raceFight->testWindow->Draw( preScreenTex );

			preScreenTex->display();
			const Texture &preTex = preScreenTex->getTexture();
		
			Sprite preTexSprite( preTex );
			preTexSprite.setPosition( -960 / 2, -540 /2 );//-960 / 2, -540 / 2 );
			
			preTexSprite.setScale( .5, .5 );		

			window->draw( preTexSprite );

			++raceFight->raceFightResultsFrame;
		}
		else if (gameState == STORY)
		{
			sf::Event ev;
			while (window->pollEvent(ev))
			{
				/*if( ev.type == sf::Event::KeyPressed )
				{
				if( ev.key.code = Keyboard::O )
				{
				state = RUN;
				soundNodeList->Pause( false );
				break;
				}
				}*/
				if (ev.type == sf::Event::GainedFocus)
				{
					//state = RUN;
					//soundNodeList->Pause( false );
					//break;
				}
				else if (ev.type == sf::Event::KeyPressed)
				{
					//if( ev.key.code == Keyboard::
				}
			}

			accumulator += frameTime;
			Sprite preTexSprite;
			if (accumulator >= TIMESTEP)
			{
				window->clear();
				window->setView(v);
				preScreenTex->clear();

				UpdateControllers();

				if (currStorySequence != NULL)
				{
					//if( false )
					if (!currStorySequence->Update(GetPrevInput(0), GetCurrInput(0)))
					{
						gameState = RUN;
						//preScreenTex->setView(uiView);
						//currStorySequence->Draw(preScreenTex);
						currStorySequence->EndSequence();
						currStorySequence = NULL;
					}
					else
					{
					}
				}
				
				mainMenu->musicPlayer->Update();

				fader->Update();
				swiper->Update();
				mainMenu->UpdateEffects();
				accumulator -= TIMESTEP;
			}

			

			if (currStorySequence != NULL)
			{
				preScreenTex->setView(uiView);
				currStorySequence->Draw(preScreenTex);
			}

			preScreenTex->setView(uiView);
			fader->Draw(preScreenTex);
			swiper->Draw(preScreenTex);

			mainMenu->DrawEffects(preScreenTex);

			DrawFrameRate(preScreenTex);

			preTexSprite.setTexture(preScreenTex->getTexture());
			preTexSprite.setPosition(-960 / 2, -540 / 2);
			preTexSprite.setScale(.5, .5);
			window->draw(preTexSprite);
			//UpdateInput();
		
		}
		else if (gameState == SEQUENCE)
		{
			sf::Event ev;
			while (window->pollEvent(ev))
			{
			}

			window->clear();;
			preScreenTex->clear();

			if (!SequenceGameModeUpdate())
			{
				continue;
			}

			Sprite preTexSprite;
			preTexSprite.setTexture(preScreenTex->getTexture());
			preTexSprite.setPosition(-960 / 2, -540 / 2);
			preTexSprite.setScale(.5, .5);
			window->draw(preTexSprite);
			//UpdateInput();

		}
		else if (gameState == PAUSE)
		{
			sf::Event ev;
			while (window->pollEvent(ev))
			{
				/*if( ev.type == sf::Event::KeyPressed )
				{
				if( ev.key.code = Keyboard::O )
				{
				state = RUN;
				soundNodeList->Pause( false );
				break;
				}
				}*/
				if (ev.type == sf::Event::GainedFocus)
				{
					//state = RUN;
					//soundNodeList->Pause( false );
					//break;
				}
				else if (ev.type == sf::Event::KeyPressed)
				{
					//if( ev.key.code == Keyboard::
				}
			}

			//savedinput when you enter pause

			accumulator += frameTime;

			while (accumulator >= TIMESTEP)
			{
				UpdateControllers();

				PauseMenu::UpdateResponse ur = pauseMenu->Update(GetCurrInputUnfiltered(0), GetPrevInputUnfiltered(0));
				switch (ur)
				{
				case PauseMenu::R_NONE:
				{
					//do nothing as usual
					break;
				}
				case PauseMenu::R_P_RESUME:
				{
					gameState = GameSession::RUN;
					ActivatePauseSound(GetSound("pause_off"));
					soundNodeList->Pause(false);
					pauseMenu->shardMenu->StopMusic();
					break;
				}
				case PauseMenu::R_P_RESPAWN:
				{
					gameState = GameSession::RUN;
					RestartLevel();
					gameClock.restart();
					currentTime = 0;
					accumulator = TIMESTEP + .1;
					frameRateDisplay.Reset();
					//soundNodeList->Pause( false );
					//kill sounds on respawn
					break;
				}
				case PauseMenu::R_P_EXITLEVEL:
				{
					quit = true;
					returnVal = GR_EXITLEVEL;
					break;
				}
				case PauseMenu::R_P_EXITTITLE:
				{
					quit = true;
					returnVal = GR_EXITTITLE;
					break;
				}
				case PauseMenu::R_P_EXITGAME:
				{
					quit = true;
					returnVal = GR_EXITGAME;
					break;
				}

				}

				if (gameState != PAUSE)
				{
					break;
				}

				accumulator -= TIMESTEP;
			}

			if (gameState != PAUSE)
			{
				continue;
			}

			//if( currInput.

			/*if( IsKeyPressed( Keyboard::O ) )
			{
			state = RUN;
			soundNodeList->Pause( false );
			}*/
			pauseTex->clear();
			window->clear();
			Sprite preTexSprite;
			preTexSprite.setTexture(preScreenTex->getTexture());
			preTexSprite.setPosition(-960 / 2, -540 / 2);
			preTexSprite.setScale(.5, .5);
			window->draw(preTexSprite);

			pauseMenu->Draw(pauseTex);

			pauseTex->display();
			Sprite pauseMenuSprite;
			pauseMenuSprite.setTexture(pauseTex->getTexture());
			//bgSprite.setPosition( );
			pauseMenuSprite.setPosition((1920 - 1820) / 4 - 960 / 2, (1080 - 980) / 4 - 540 / 2);
			pauseMenuSprite.setScale(.5, .5);
			window->draw(pauseMenuSprite);

			//draw map

			if (pauseMenu->currentTab == PauseMenu::MAP)
			{
				View vv;
				vv.setCenter(pauseMenu->mapCenter);
				vv.setSize(mapTex->getSize().x * pauseMenu->mapZoomFactor, mapTex->getSize().y * pauseMenu->mapZoomFactor);

				mapTex->clear();
				mapTex->setView(vv);
				mapTex->clear(Color(0, 0, 0, 255));

				View vuiView;
				vuiView.setSize(Vector2f(mapTex->getSize().x * 1.f, mapTex->getSize().y * 1.f));
				vuiView.setCenter(0, 0);

				for (list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it)
				{
					(*it)->Draw(mapTex);
				}

				queryMode = QUERY_BORDER;
				numBorders = 0;
				sf::Rect<double> mapRect(vv.getCenter().x - vv.getSize().x / 2.0,
					vv.getCenter().y - vv.getSize().y / 2.0, vv.getSize().x, vv.getSize().y);

				borderTree->Query(this, mapRect);

				
				DrawColoredMapTerrain(mapTex, Color(Color::Green));

				/*Color testColor(0x75, 0x70, 0x90, 191);
				testColor = Color::Green;
				TerrainPiece * listVAIter = listVA;
				while (listVAIter != NULL)
				{
					if (listVAIter->visible)
					{
						int vertexCount = listVAIter->terrainVA->getVertexCount();
						for (int i = 0; i < vertexCount; ++i)
						{
							(*listVAIter->terrainVA)[i].color = testColor;
						}
						mapTex->draw(*listVAIter->terrainVA);
						for (int i = 0; i < vertexCount; ++i)
						{
							(*listVAIter->terrainVA)[i].color = Color::White;
						}
					}

					listVAIter = listVAIter->next;
				}*/

				testGateCount = 0;
				queryMode = QUERY_GATE;
				gateList = NULL;
				gateTree->Query(this, mapRect);
				Gate *mGateList = gateList;
				while (gateList != NULL)
				{
					gateList->MapDraw(mapTex);

					Gate *next = gateList->next;//edgeA->edge1;
					gateList = next;
				}




				Vector2i b = mapTex->mapCoordsToPixel(Vector2f(p0->position.x, p0->position.y));

				mapTex->setView(vuiView);

				Vector2f realPos = mapTex->mapPixelToCoords(b);
				realPos.x = floor(realPos.x + .5f);
				realPos.y = floor(realPos.y + .5f);

				//cout << "vuiVew size: " << vuiView.getSize().x << ", " << vuiView.getSize().y << endl;

				//mapTex->draw( kinMinimapIcon );

				mapTex->setView(vv);

				Vector2i b1 = mapTex->mapCoordsToPixel(Vector2f(playerOrigPos[0].x, playerOrigPos[0].y));

				mapTex->setView(vuiView);

				Vector2f realPos1 = mapTex->mapPixelToCoords(b1);
				realPos1.x = floor(realPos1.x + .5f);
				realPos1.y = floor(realPos1.y + .5f);

				//cout << "vuiVew size: " << vuiView.getSize().x << ", " << vuiView.getSize().y << endl;
				kinMapSpawnIcon.setPosition(realPos1);
				mapTex->draw(kinMapSpawnIcon);

				mapTex->setView(vv);

				Vector2i bGoal = mapTex->mapCoordsToPixel(Vector2f(goalPos.x, goalPos.y));

				mapTex->setView(vuiView);

				Vector2f realPosGoal = mapTex->mapPixelToCoords(bGoal);
				realPosGoal.x = floor(realPosGoal.x + .5f);
				realPosGoal.y = floor(realPosGoal.y + .5f);

				//cout << "vuiVew size: " << vuiView.getSize().x << ", " << vuiView.getSize().y << endl;
				
				
				
				//goalMapIcon.setPosition(realPosGoal);
				//mapTex->draw(goalMapIcon);

				if (currentZone != NULL)
				{
					for (list<Enemy*>::iterator it = currentZone->allEnemies.begin(); it != currentZone->allEnemies.end(); ++it)
					{
						//cout << "drawing map" << endl;
						(*it)->DrawMinimap(mapTex);
					}
				}

				//mapTex->clear();
				Sprite mapTexSprite;
				mapTexSprite.setTexture(mapTex->getTexture());
				mapTexSprite.setOrigin(mapTexSprite.getLocalBounds().width / 2, mapTexSprite.getLocalBounds().height / 2);
				mapTexSprite.setPosition(0, 0);


				//pauseTex->setView( bigV );
				//window->setView( bigV );

				//mapTexSprite.setScale( .5, -.5 );
				mapTexSprite.setScale(.5, -.5);
				//mapTexSprite.setScale( 1, -1 );

				window->draw(mapTexSprite);
				//pauseTex->draw( mapTexSprite );

				//pauseTex->setV
			}




		}

		window->display();

		
		
	}

	if (parentGame == NULL)
	{


		if (recGhost != NULL)
		{
			recGhost->StopRecording();

			//string fName = fileName
			//1. get folder. if it doesn't exist, make it.
			//2. include map name in ghost name
			//3. tag ghost with timestamp
			//4. 

			time_t t = time(0);
			struct tm now;
			localtime_s(&now, &t);





			stringstream fss;
			string mName = filePath.filename().stem().string();
			fss << "Recordings/Ghost/" << mName << "/auto/" << mName << "_ghost_"
				<< now.tm_year << "_" << now.tm_mon << "_" << now.tm_mday << "_" << now.tm_hour << "_"
				<< now.tm_min << "_" << now.tm_sec << ".bghst";

			//recGhost->WriteToFile(fss.str());
		}



		testBuf.SetRecOver(true);

		if (recPlayer != NULL)
		{
			threa->join();
			delete threa;

			assert(of.is_open());
			of.close();

			ifstream is;
			is.open("tempreplay.brep", ios::binary | ios::in);


			ofstream out;
			//custom file
			out.open("testreplay.brep", ios::binary | ios::out);
			out.write((char*)&(recPlayer->numTotalFrames), sizeof(int));
			//out << recPlayer->numTotalFrames << "\n";

			char c;
			while (true)
			{
				c = is.get();
				if (is.eof()) break;
				out.put(c);
			}

			out.close();


			/*istreambuf_iterator<char> begin_source( is );
			istreambuf_iterator<char> end_source;
			ostreambuf_iterator<char> begin_dest( out );
			copy( begin_source, end_source, begin_dest );*/

			is.close();

		}

	}

	soundNodeList->Reset();
	pauseSoundNodeList->Reset();
	
	if (parentGame == NULL)
	{
		for (int i = 0; i < 4; ++i)
		{
			SetFilterDefault(GetController(i).filter);
		}
	}

	if (parentGame != NULL)
	{
		pauseMenu->owner = parentGame;
	}
	else
	{
		pauseMenu->owner = NULL;
	}

	

	fader->Clear();

	preScreenTex->setView(oldPreTexView);
	window->setView(oldWindowView);

	mainMenu->SetMouseGrabbed(oldMouseGrabbed);
	mainMenu->SetMouseVisible(oldMouseVisible);

	return returnVal;
}



void GameSession::Init()
{
	mapTex = mainMenu->mapTexture;
	pauseTex = mainMenu->pauseTexture;

	bonusGame = NULL;
	gateMarkers = NULL;
	inversePolygon = NULL;
	
	postLevelScene = NULL;
	shardsCapturedField = NULL;
	level = NULL;
	inputVis = NULL;
	
	pauseMenu = NULL;
	progressDisplay = NULL;
	topClouds = NULL;
	specterTree = NULL;
	envPlantTree = NULL;
	itemTree = NULL;
	gateTree = NULL;
	enemyTree = NULL;
	staticItemTree = NULL;
	railDrawTree = NULL;
	terrainBGTree = NULL;
	scoreDisplay = NULL;
	va = NULL;
	activeEnemyList = NULL;
	activeEnemyListTail = NULL;
	raceFight = NULL;
	recPlayer = NULL;
	repPlayer = NULL;
	recGhost = NULL;
	//repGhost = NULL;
	explodingGravityGrass = NULL;
	polyQueryList = NULL;
	specialPieceList = NULL;
	flyTerrainList = NULL;
	absorbParticles = NULL;
	absorbDarkParticles = NULL;
	absorbShardParticles = NULL;
	for (int i = 0; i < 4; ++i)
	{
		players[i] = NULL;
	}

	shadersLoaded = false;

	hasGoal = false;
	boostIntro = false;
	nextFrameRestart = false;
	showTerrainDecor = true;
	cutPlayerInput = false;
	usePolyShader = true;
	showDebugDraw = false;
	for (int i = 0; i < 6; ++i)
	{
		hasGrass[i] = false;
	}

	numTotalKeys = 0;
	pauseFrames = 0;
	totalRails = 0;
	numKeysCollected = 0;

	preScreenTex->setSmooth(false);
	postProcessTex2->setSmooth(false);
	ReadDecorImagesFile();
	testBuf.SetRecOver(false);
}

void GameSession::SetStorySeq(StorySequence *storySeq)
{
	storySeq->Reset();
	currStorySequence = storySeq;
	gameState = GameSession::STORY;
}

void GameSession::UpdateTimeSlowShader()
{
	Actor *p0 = GetPlayer(0);

	timeSlowShader.setUniform("u_texture", preScreenTex->getTexture());
	timeSlowShader.setUniform("Resolution", Vector2f(1920, 1080));//window->getSize().x, window->getSize().y);
	timeSlowShader.setUniform("zoom", cam.GetZoom());

	timeSlowShader.setUniform("topLeft", Vector2f(view.getCenter().x - view.getSize().x / 2,
		view.getCenter().y + view.getSize().y / 2));

	timeSlowShader.setUniform("bubbleRadius0", (float)p0->bubbleRadiusSize[0]);
	timeSlowShader.setUniform("bubbleRadius1", (float)p0->bubbleRadiusSize[1]);
	timeSlowShader.setUniform("bubbleRadius2", (float)p0->bubbleRadiusSize[2]);
	timeSlowShader.setUniform("bubbleRadius3", (float)p0->bubbleRadiusSize[3]);
	timeSlowShader.setUniform("bubbleRadius4", (float)p0->bubbleRadiusSize[4]);
	timeSlowShader.setUniformArray("bubbleRadius", fBubbleRadiusSize, 20);//p0->maxBubbles * m_numActivePlayers);
	timeSlowShader.setUniformArray("bPos", fBubblePos, 20);//p0->maxBubbles * m_numActivePlayers);
	timeSlowShader.setUniformArray("bFrame", fBubbleFrame, 20);//p0->maxBubbles * m_numActivePlayers);
	timeSlowShader.setUniform("totalBubbles", p0->maxBubbles * m_numActivePlayers);
	//too many assumptions that p0 will always be here lots of refactoring to do


	float windowx = 1920;//window->getSize().x;
	float windowy = 1080;//window->getSize().y;

	Vector2i vi0, vi1, vi2, vi3, vi4;
	Vector2f tpos[5];
	Actor *tPlayer = NULL;
	for (int i = 0; i < 4; ++i)
	{
		if (tPlayer = GetPlayer(i))
		{
			vi0 = preScreenTex->mapCoordsToPixel(Vector2f(tPlayer->bubblePos[0].x, tPlayer->bubblePos[0].y));
			tpos[0] = Vector2f(vi0.x / windowx, -1 + vi0.y / windowy);

			vi1 = preScreenTex->mapCoordsToPixel(Vector2f(tPlayer->bubblePos[1].x, tPlayer->bubblePos[1].y));
			tpos[1] = Vector2f(vi1.x / windowx, -1 + vi1.y / windowy);

			vi2 = preScreenTex->mapCoordsToPixel(Vector2f(tPlayer->bubblePos[2].x, tPlayer->bubblePos[2].y));
			tpos[2] = Vector2f(vi2.x / windowx, -1 + vi2.y / windowy);

			vi3 = preScreenTex->mapCoordsToPixel(Vector2f(tPlayer->bubblePos[3].x, tPlayer->bubblePos[3].y));
			tpos[3] = Vector2f(vi3.x / windowx, -1 + vi3.y / windowy);

			vi4 = preScreenTex->mapCoordsToPixel(Vector2f(tPlayer->bubblePos[4].x, tPlayer->bubblePos[4].y));
			tpos[4] = Vector2f(vi4.x / windowx, -1 + vi4.y / windowy);

			for (int j = 0; j < 5; ++j)
			{
				fBubblePos[i * 5 + j] = tpos[j];
			}

			//vi5 = preScreenTex->mapCoordsToPixel(Vector2f(tPlayer->bubblePos[5].x, tPlayer->bubblePos[5].y));
			//Vector2f pos5(vi5.x / windowx, -1 + vi5.y / windowy);
		}
	}





	timeSlowShader.setUniformArray("bPos", fBubblePos, 5 * 4);
	//cout << "pos0: " << pos0.x << ", " << pos0.y << endl;
	//cout << "b0frame: " << player->bubbleFramesToLive[0] << endl;
	//cout << "b1frame: " << player->bubbleFramesToLive[1] << endl;
	//cout << "b2frame: " << player->bubbleFramesToLive[2] << endl;

	//timeSlowShader.setUniform( "bubble0", pos0 );
	timeSlowShader.setUniform("b0Frame", (float)p0->bubbleFramesToLive[0]);
	//timeSlowShader.setUniform( "bubble1", pos1 );
	timeSlowShader.setUniform("b1Frame", (float)p0->bubbleFramesToLive[1]);
	//timeSlowShader.setUniform( "bubble2", pos2 );
	timeSlowShader.setUniform("b2Frame", (float)p0->bubbleFramesToLive[2]);
	//timeSlowShader.setUniform( "bubble3", pos3 );
	timeSlowShader.setUniform("b3Frame", (float)p0->bubbleFramesToLive[3]);
	//timeSlowShader.setUniform( "bubble4", pos4 );
	timeSlowShader.setUniform("b4Frame", (float)p0->bubbleFramesToLive[4]);
	//timeSlowShader.setUniform( "bubble5", pos5 );
	//timeSlowShader.setUniform( "b5Frame", player->bubbleFramesToLive[5] );
}





void GameSession::SuppressEnemyKeys( Gate *g )
{
	if( g->IsTwoWay() )
		return;

	for( list<Enemy*>::iterator it = currentZone->allEnemies.begin();
		it != currentZone->allEnemies.end(); ++it )
	{
		(*it)->suppressMonitor = true;
	}
	//cout << "suppress keys???" << endl;
	//Enemy *currEnemy = activeEnemyList;
	//int mType;
	//while( currEnemy != NULL )
	//{
	//	
	//	if( currEnemy->hasMonitor )
	//	{
	//	}
	//	//currEnemy->moni
	//	currEnemy = currEnemy->next;
	//}
}

void GameSession::DrawActiveEnvPlants()
{
	EnvPlant *drawPlant = activeEnvPlants;
	while( drawPlant != NULL )
	{
		preScreenTex->draw( *drawPlant->particle->particles );
		drawPlant = drawPlant->next;
	}
}

void GameSession::SoftenGates(int gCat)
{
	Gate *g;
	for (int i = 0; i < numGates; ++i)
	{
		g = gates[i];
		g->gState = Gate::SOFTEN;
		g->frame = 0;
	}
}

void GameSession::ReformGates(int gCat)
{
	Gate *g;
	for (int i = 0; i < numGates; ++i)
	{
		g = gates[i];
		g->Reform();
	}
}

void GameSession::CloseGates(int gCat)
{
	Gate *g;
	for (int i = 0; i < numGates; ++i)
	{
		g = gates[i];
		g->Close();
	}
}



void GameSession::OpenGates(int gCat)
{
	Gate *g;
	for (int i = 0; i < numGates; ++i)
	{
		g = gates[i];
		g->gState = Gate::OPEN;
		g->frame = 0;
		UnlockGate(g);
	}
}





void GameSession::UpdateEnvShaders()
{
	Vector2f botLeft(view.getCenter().x - view.getSize().x / 2,
		view.getCenter().y + view.getSize().y / 2);

	Vector2f playertest = (botLeft - oldCamBotLeft) / 5.f;

	UpdatePolyShaders(botLeft, playertest);

	/*for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->Update(cam.GetZoom(), botLeft, playertest);
	}*/
}



void GameSession::DrawRails(sf::RenderTarget *target)
{
	//put this query within the frame update, not the draw call
	railDrawList = NULL;
	queryMode = QUERY_RAIL;
	railDrawTree->Query(this, screenRect);
	while (railDrawList != NULL)
	{
		railDrawList->Draw(target);
		RailPtr next = railDrawList->queryNext;
		railDrawList->queryNext = NULL;
		railDrawList = next;
	}
}

void GameSession::DrawDecor(EffectLayer ef, sf::RenderTarget *target)
{
	auto &dList = decor[ef];

	for (auto it = dList.begin(); it != dList.end(); ++it)
	{
		(*it)->Draw(target);
	}
}



void GameSession::UpdateDebugModifiers()
{
	if (IsKeyPressed(Keyboard::Num1))
	{
		showDebugDraw = true;
	}
	else if (IsKeyPressed(Keyboard::Num2))
	{
		showDebugDraw = false;
	}
	else if (IsKeyPressed(Keyboard::Num3))
	{
		showTerrainDecor = false;
	}
	else if (IsKeyPressed(Keyboard::Num4))
	{
		showTerrainDecor = true;
	}
	else if (IsKeyPressed(sf::Keyboard::Num9))
	{
		runningTimerDisplay.showRunningTimer = true;
	}
	else if (IsKeyPressed(sf::Keyboard::Num0))
	{
		runningTimerDisplay.showRunningTimer = false;
	}
}

void GameSession::DebugDraw(sf::RenderTarget *target)
{
	if (showDebugDraw)
	{
		for (auto it = barriers.begin();
			it != barriers.end(); ++it)
		{
			(*it)->DebugDraw(target);
		}

		DebugDrawActors(target);

		for (auto it = fullAirTriggerList.begin(); it != fullAirTriggerList.end(); ++it)
		{
			(*it)->DebugDraw(target);
		}
	}
}

void GameSession::UpdateDecorSprites()
{
	PolyPtr poly = polyQueryList;
	while (poly != NULL)
	{
		poly->UpdateDecorSprites();
		poly->UpdateTouchGrass(); //put this in its own spot soon
		poly = poly->queryNext;
	}
}

void GameSession::DrawReplayGhosts(sf::RenderTarget *target)
{
	for (auto it = replayGhosts.begin(); it != replayGhosts.end(); ++it)
	{
		(*it)->Draw(target);
	}
}

void GameSession::UpdatePolyShaders( Vector2f &botLeft, Vector2f &playertest)
{
	//inefficient. should only update these when they are changed.
	for (int i = 0; i < numPolyShaders; ++i)
	{
		polyShaders[i].setUniform("zoom", cam.GetZoom());
		polyShaders[i].setUniform("topLeft", botLeft); //just need to change the name topleft eventually
		polyShaders[i].setUniform("playertest", playertest);
		polyShaders[i].setUniform("skyColor", ColorGL(background->GetSkyColor()));
	}
}

void GameSession::SetOriginalMusic()
{
	int pointsTotal = 0;
	if (mapHeader->songLevels.size() > 0)
	{
		for (auto it = mapHeader->songLevels.begin(); it != mapHeader->songLevels.end(); ++it)
		{
			if (mainMenu->musicManager->songMap.count((*it).first) == 0)
			{
				continue;
			}

			pointsTotal += (*it).second;
		}
	}

	//TODO : use a better random algorithm later
	srand(time(0));

	if (pointsTotal > 0)
	{
		int r = rand() % (pointsTotal);

		for (auto it = mapHeader->songLevels.begin(); it != mapHeader->songLevels.end(); ++it)
		{
			if (mainMenu->musicManager->songMap.count((*it).first) == 0)
			{
				//song doesnt exist!
				continue;
			}

			r -= (*it).second;
			if (r < 0)
			{
				//assumes that this is a valid check
				

				originalMusic = mainMenu->musicManager->songMap[(*it).first];

				if (originalMusic == NULL)
				{
					assert(0);
				}
				//musicMap[(*it).first] = originalMusic;
				originalMusic->Load();
				if (originalMusic == mainMenu->musicPlayer->currMusic)
				{

				}
				else
				{
					mainMenu->musicPlayer->TransitionMusic(originalMusic, 60);
				}
				
				//originalMusic->music->setVolume(mainMenu->config->GetData().musicVolume);
				//originalMusic->music->setLoop(true);
				//originalMusic->music->play();
				
				break;
			}
		}
	}
	else
	{
		mainMenu->musicPlayer->FadeOutCurrentMusic(60);
	}
	if (originalMusic == NULL && pointsTotal > 0)
	{
		assert(0);
	}
}

SaveFile *GameSession::GetCurrentProgress()
{
	if (mainMenu->gameRunType == MainMenu::GRT_ADVENTURE)
	{
		return mainMenu->GetCurrentProgress();
	}
	else
	{
		return NULL;
	}
}


void GameSession::NextFrameRestartLevel()
{
	nextFrameRestart = true;
}


void GameSession::RestartGame()
{
	//RestartLevel(); //not sure why i need to do this. fix this soon
	NextFrameRestartLevel();
}

void GameSession::RestartLevel()
{
	if( gateMarkers != NULL)
		gateMarkers->Reset();
	//OpenGates(Gate::CRAWLER_UNLOCK);

	ClearEmitters();

	//AddEmitter(testEmit, EffectLayer::IN_FRONT);
	//testEmit->Reset();

	for (auto it = allPolysVec.begin(); it != allPolysVec.end(); ++it)
	{
		(*it)->ResetTouchGrass();
	}

	SetDrainOn(true);

	shardsCapturedField->Reset();

	nextFrameRestart = false;
	//accumulator = TIMESTEP + .1;
	currStorySequence = NULL;
	currSuperPlayer = NULL;

	if( raceFight != NULL )
		raceFight->Reset();

	background->Reset();

	soundNodeList->Clear();

	totalGameFrames = 0;
	totalFramesBeforeGoal = -1;
	/*if( GetPlayer->record > 1 )
	{
		player->LoadState();
		LoadState();
	}*/

	if( goalPulse != NULL )
		goalPulse->Reset();
	//f->Reset();


	fader->Reset();
	numKeysCollected = 0;

	if (hud != NULL)
	{
		hud->Reset();
	}

	//crawlerFightSeq->Reset();
	//enterNexus1Seq->Reset();
	

	soundNodeList->Reset();
	scoreDisplay->Reset();
	if( recPlayer != NULL )
	{
		recPlayer->StopRecording();
		recPlayer->StartRecording();
	}

	if( recGhost != NULL )
	{
		recGhost->StopRecording();
		recGhost->StartRecording();
	}

	if( repPlayer != NULL )
		repPlayer->frame = 0;

	/*if( repGhost != NULL )
		repGhost->frame = 0;*/

	for (auto it = replayGhosts.begin(); it != replayGhosts.end(); ++it)
	{
		(*it)->frame = 0;//players[0]->actionLength[Actor::Action::SPAWNWAIT];
	}

	for (auto it = fullAirTriggerList.begin(); it != fullAirTriggerList.end(); ++it)
	{
		(*it)->Reset();
	}


	//testEmit->SetPos(Vector2f(GetPlayer(0)->position))


	cam.Reset();

	cutPlayerInput = false;


	Actor *p;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->position = V2d(playerOrigPos[i]);
		}
	}

	for (int i = 0; i < 4; ++i)
	{
		Actor *player = GetPlayer(i);
		if (player != NULL)
			player->Respawn();
	}

	scoreDisplay->Reset();

	ResetAbsorbParticles();

	//player->Respawn();

	cam.pos.x = GetPlayer( 0 )->position.x;
	cam.pos.y = GetPlayer( 0 )->position.y;

	//RespawnPlayer();
	pauseFrames = 0;

	ResetZones();

	ResetEnemies();
	ResetPlants(); //eventually maybe treat these to reset like the rest of the stuff
	//only w/ checkpoints. but for now its always back

	//was resetting zones here before
	
	ResetGates();

	currentZone = NULL;
	if (originalZone != NULL)
	{
		currentZoneNode = zoneTree;
		ActivateZone(originalZone, true);
		gateMarkers->SetToZone(currentZone);

		AdventureHUD *ah = GetAdventureHUD();
		if (ah != NULL) ah->keyMarker->Reset();
	}
	//	originalZone->active = true;
	//
	//later don't relock gates in a level unless there is a "level reset"
	

	inactiveEnemyList = NULL;

	ResetBarriers();

	cam.SetManual( false );

	activeSequence = NULL;

	if (shipEnterScene != NULL)
	{
		shipEnterScene->Reset();
		SetActiveSequence(shipEnterScene);
	}

	gameMode->StartGame();
	//later can have a setting for this if needed
	/*if (preLevelScene != NULL)
	{
		preLevelScene->Reset();
		SetActiveSequence(preLevelScene);
	}
	else
	{
		activeSequence = NULL;
	}*/
}

void GameSession::AddGravityGrassToExplodeList(Grass *g)
{
	if (explodingGravityGrass == NULL)
	{
		explodingGravityGrass = g;
		explodingGravityGrass->next = NULL;
		explodingGravityGrass->prev = NULL;
	}
	else
	{
		g->next = explodingGravityGrass;
		explodingGravityGrass->prev = g;
		explodingGravityGrass = g;
		explodingGravityGrass->prev = NULL;
	}
}

void GameSession::RemoveGravityGrassFromExplodeList(Grass *g)
{
	assert(explodingGravityGrass != NULL);

	if (explodingGravityGrass == g )
	{
		explodingGravityGrass = g->next;
	}
	else
	{
		if (g->prev != NULL)
			g->prev->next = g->next;
		if (g->next != NULL)
			g->next->prev = g->prev;
		g->SetVisible(false);
	}
}

bool GameSession::IsShardCaptured(int shardType)
{
	if (saveFile != NULL)
	{
		return saveFile->ShardIsCaptured(shardType);
	}
	else
	{
		return shardsCapturedField->GetBit(shardType);
	}
}

void GameSession::UpdateExplodingGravityGrass()
{
	Grass *curr = explodingGravityGrass;
	Grass *next;
	while (curr != NULL)
	{
		next = curr->next;
		curr->Update();
		curr = next;
	}
}

//double GameSession::GetTriangleArea(p2t::Triangle * t)
//{
//	p2t::Point *p_0 = t->GetPoint(0);
//	p2t::Point *p_1 = t->GetPoint(0);
//	p2t::Point *p_2 = t->GetPoint(0);
//
//	V2d p0(p_0->x, p_0->y);
//	V2d p1(p_1->x, p_1->y);
//	V2d p2(p_2->x, p_2->y);
//
//	double len0 = length(p1 - p0);
//	double len1 = length(p2 - p1);
//	double len2 = length(p0 - p2);
//
//	//s = .5 * (a + b + c)
//	//A = sqrt( s(s - a)(s - b)(s - c) )
//
//	double s = .5 * (len0 + len1 + len2);
//	double A = sqrt(s * (s - len0) * (s - len1) * (s - len2));
//
//	return A;
//}

sf::VertexArray * GameSession::SetupPlants( Edge *startEdge, Tileset *ts )//, int (*ValidEdge)(sf::Vector2<double> &) )
{
	list<PlantInfo> info;

	int tw = 32;
	int th = 32;

	Edge *te = startEdge;
	do
	{
		int valid = 0;
		if( valid != -1 )
		{
			double len = length( te->v1 - te->v0 );
			int numQuads = len / tw;
			double quadWidth = len / numQuads;
				
			if( numQuads > 0 )
			{
				for(int i = 0; i < numQuads; ++i )
				{
					int r = rand() % 2;
					if( r == 0 )
					{
						info.push_back( PlantInfo( te, quadWidth * i, quadWidth ) );
					}
				}
			}
		}
		te = te->edge1;
	}
	while( te != startEdge );

	int infoSize = info.size();
	int vaSize = infoSize * 4;

	if( infoSize == 0 )
	{
		return NULL;
	}

	//cout << "number of plants: " << infoSize << endl;
	VertexArray *va = new VertexArray( sf::Quads, vaSize );

	int vaIndex = 0;
	for( list<PlantInfo>::iterator it = info.begin(); it != info.end(); ++it )
	{
		V2d groundPoint = (*it).edge->GetPosition( (*it).quant );
		V2d norm = (*it).edge->Normal();
		double w = (*it).quadWidth;
		V2d along = normalize( (*it).edge->v1 - (*it).edge->v0 );
		//V2d other( along.y, -along.x );
		
		V2d groundLeft = groundPoint;
		V2d groundRight = groundPoint + w * along;
		V2d airLeft = groundPoint + norm * (double)th;
		V2d airRight = groundPoint + w * along + norm * (double)th;

		EnvPlant * ep = new EnvPlant( groundLeft,airLeft,airRight,groundRight, vaIndex, va, ts );
		allEnvPlants.push_back(ep);

		envPlantTree->Insert( ep );

		vaIndex += 4;
	}

	return va;
	/*for( int i = 0; i < infoSize; ++i )
	{

	}*/
}



sf::VertexArray *GameSession::SetupBushes( int bgLayer, Edge *startEdge, Tileset *ts )
{
	int minDistanceApart = 10;
	int maxDistanceApart = 300;
	int minPen = 20;
	int maxPen = 200;
	double penLimit;

	list<Vector2f> positions;

	Edge *curr = startEdge;
	double quant = 0;
	double lenCurr = length( startEdge->v1 - startEdge->v0 );

	double travelDistance;
	double penDistance;
	int diffApartMax = maxDistanceApart - minDistanceApart;
	int diffPenMax = maxPen - minPen;
	int r;
	int rPen;
	double momentum;
	V2d pos;

	bool loopOver = false;
	V2d cn;

	rayMode = "decor";
	QuadTree *qt = NULL;
	if( bgLayer == 0 )
	{
		qt = terrainTree;
	}
	else if( bgLayer == 1 )
	{
		qt = terrainBGTree;
	}

	assert( qt != NULL );

	while( true )
	{
		//cout << "running loop" << endl;
		r = rand() % diffApartMax;
		travelDistance = minDistanceApart + r;

		momentum = travelDistance;
		
		while( !approxEquals( momentum, 0 ) )
		{
			if( (lenCurr - quant) > momentum )
			{
				quant += momentum;
				momentum = 0;
			}
			else
			{
				curr = curr->edge1;

				if( curr == startEdge )
				{
					loopOver = true;
					break;
				}
				else
				{
					momentum = momentum - ( lenCurr - quant );
					quant = 0;
					lenCurr = length( curr->v1 - curr->v0 );
				}
			}
		}

		if( loopOver )
			break;

		cn = curr->Normal();
		rcEdge = NULL;
		rayStart = curr->GetPosition( quant );
		rayEnd = rayStart - cn * (double)maxPen;
		rayIgnoreEdge = curr;

		RayCast( this, qt->startNode, rayStart, rayEnd );

		if( rcEdge != NULL )
		{
			penLimit = length( rcEdge->GetPosition( rcQuantity ) - rayStart );
			diffPenMax = (int)penLimit - minDistanceApart;
		}
		if (diffPenMax == 0)
			rPen = 0;
		else
		{
			rPen = rand() % diffPenMax;
		}
		
		penDistance = minPen + rPen;
		
		pos = curr->GetPosition( quant ) - curr->Normal() * penDistance;

		positions.push_back( Vector2f( pos.x, pos.y ) );
		//will have to do a raycast soon. ignore for now
		//curr = curr->edge1;
	}

	
	int numBushes = positions.size();
	//cout << "numBushes: " << numBushes << endl;


	VertexArray *va = new VertexArray( sf::Quads, numBushes * 4 );
	VertexArray &VA = *va;

	IntRect subRect = ts->GetSubRect( 0 );
	list<Vector2f>::iterator posIt;
	if( numBushes > 0 )
		posIt = positions.begin();

	Vector2f p;
	for( int i = 0; i < numBushes; ++i )
	{
		p = (*posIt);
		//cout << "i: " << i << ", p: " <<  p.x << ", " << p.y << endl;
		VA[i*4+0].position = Vector2f( p.x - subRect.width / 2, p.y - subRect.height / 2 );
		VA[i*4+1].position = Vector2f( p.x + subRect.width / 2, p.y - subRect.height / 2 );
		VA[i*4+2].position = Vector2f( p.x + subRect.width / 2, p.y + subRect.height / 2 );
		VA[i*4+3].position = Vector2f( p.x - subRect.width / 2, p.y + subRect.height / 2 );

		/*VA[i*4+0].color= Color::Red;
		VA[i*4+1].color= Color::Red;
		VA[i*4+2].color= Color::Red;
		VA[i*4+3].color= Color::Red;*/

		VA[i*4+0].texCoords = Vector2f( subRect.left, subRect.top );
		VA[i*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
		VA[i*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
		VA[i*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );

		++posIt;
	}
	//int penLimit =

	/*int numBushes = 0;
	
	int trisSize = tris.size();
	for( int i = 0; i < trisSize; ++i )
	{
		numBushes++;
	}

	VertexArray *va = new VertexArray( sf::Quads, numBushes * 4 );
	VertexArray &VA = *va;

	Vector2f testPos;
	Vector2f p0, p1, p2;
	Vector2f avg;

	IntRect subRect = ts->GetSubRect( 0 );
	for( int i = 0; i < numBushes; ++i )
	{
		p0.x = tris[i]->GetPoint( 0 )->x;
		p0.y = tris[i]->GetPoint( 0 )->y;

		p1.x = tris[i]->GetPoint( 1 )->x;
		p1.y = tris[i]->GetPoint( 1 )->y;

		p2.x = tris[i]->GetPoint( 2 )->x;
		p2.y = tris[i]->GetPoint( 2 )->y;

		avg = ( p0 + p1 + p2 ) / 3.f;

		VA[i*4+0].position = Vector2f( avg.x - subRect.width / 2, avg.y - subRect.height / 2 );
		VA[i*4+1].position = Vector2f( avg.x + subRect.width / 2, avg.y - subRect.height / 2 );
		VA[i*4+2].position = Vector2f( avg.x + subRect.width / 2, avg.y + subRect.height / 2 );
		VA[i*4+3].position = Vector2f( avg.x - subRect.width / 2, avg.y + subRect.height / 2 );

		VA[i*4+0].texCoords = Vector2f( subRect.left, subRect.top );
		VA[i*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
		VA[i*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
		VA[i*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );
	}*/
	return va;
}

int GameSession::IsFlatGround( sf::Vector2<double> &normal )
{
	if( normal.x == 0 )
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int GameSession::IsSlopedGround( sf::Vector2<double> &normal )
{
	//.4 is the current steepthresh value
	double steepThresh = .4;
	if( abs( normal.y ) > steepThresh && normal.x != 0  )
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

int GameSession::IsSteepGround(  sf::Vector2<double> &normal )
{
	double steepThresh = .4;
	double wallThresh = .9999;
	if( abs( normal.y ) <= steepThresh && abs( normal.x ) < wallThresh )
	{
		if( normal.y > 0 )
		{
			return 3;
		}
		else if( normal.y < 0 )
		{
			return 2;
		}
	}
	else
	{
		return -1;
	}
}

int GameSession::IsWall( sf::Vector2<double> &normal )
{
	double wallThresh = .9999;
	if( abs( normal.x ) >= wallThresh )
	{
		return 4;
	}
	else
	{
		return -1;
	}
}

bool GameSession::IsWithinBounds(V2d &p)
{
	return (((p.x >= mapHeader->leftBounds)
		&& (p.y >= mapHeader->topBounds)
		&& (p.x <= mapHeader->leftBounds + mapHeader->boundsWidth)
		&& (p.y <= mapHeader->topBounds + mapHeader->boundsHeight)));
}

bool GameSession::IsWithinBarrierBounds(V2d &p)
{
	for (auto it = barriers.begin(); it != barriers.end(); ++it)
	{
		if (!(*it)->IsPointWithinBarrier(p))
		{
			return false;
		}
	}

	return true;
}

bool GameSession::IsWithinCurrentBounds(V2d &p)
{
	return (IsWithinBounds(p) && IsWithinBarrierBounds(p));
}



void GameSession::HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion )
{
	if( rayMode == "border_quads" )
	{
		if( rayIgnoreEdge != NULL )
		{
		double d0 = dot(normalize( rayIgnoreEdge->v1 - rayIgnoreEdge->v0 ),
			normalize( rayIgnoreEdge->edge0->v1 - rayIgnoreEdge->edge0->v0 ));
		double c0 = cross( normalize( rayIgnoreEdge->v1 - rayIgnoreEdge->v0 ),
			normalize( rayIgnoreEdge->edge1->v0 - rayIgnoreEdge->edge0->v0 ) );


		double d1 = dot( normalize( rayIgnoreEdge->edge1->v1 - rayIgnoreEdge->edge1->v0 ), 
			normalize( rayIgnoreEdge->v1 - rayIgnoreEdge->v0 ) );
		double c1 = cross( normalize( rayIgnoreEdge->edge1->v1 - rayIgnoreEdge->edge1->v0 ), 
			normalize( rayIgnoreEdge->v1 - rayIgnoreEdge->v0 ) );
		if( edge == rayIgnoreEdge->edge1 && ( d1 >= 0  || c1 > 0 ) )
			return;
		if( edge == rayIgnoreEdge->edge0 && ( d0 >= 0 || c0 > 0 ) )
		{
			return;
		}

		if( edge != rayIgnoreEdge && ( rcEdge == NULL || length( edge->GetPosition( edgeQuantity ) - rayStart ) <
			length( rcEdge->GetPosition( rcQuantity ) - rayStart ) ) )
		{
			rcEdge = edge;
			rcQuantity = edgeQuantity;
		}
		}
		else
		{
			rcEdge = edge;
			rcQuantity = edgeQuantity;
		}
	}
	else if( rayMode == "decor" )
	{
		if( edge == rayIgnoreEdge )
			return; 

		if( rcEdge == NULL )
		{
			rcEdge = edge;
			rcQuantity = edgeQuantity;
		}
		else
		{
			V2d rc = rcEdge->GetPosition( rcQuantity );
			if( length( rayStart - edge->GetPosition( edgeQuantity ) ) < length( rayStart - rc ) )
			{
				rcEdge = edge;
				rcQuantity = edgeQuantity;
			}
		}
	}
	//if( rayPortion > 1 && ( rcEdge == NULL || length( edge->GetPosition( edgeQuantity ) - position ) < length( rcEdge->GetPoint( rcQuantity ) - position ) ) )
	
//	{
//		rcEdge = edge;
//		rcQuantity = edgeQuantity;
//	}
}



void GameSession::ResetPlants()
{
	rResetPlants( envPlantTree->startNode );

	activeEnvPlants = NULL;
}

void GameSession::rResetPlants( QNode *node )
{
	if( node->leaf )
	{
		LeafNode *n = (LeafNode*)node;

		for( int i = 0; i < n->objCount; ++i )
		{			
			EnvPlant *ev = (EnvPlant*)(n->entrants[i]);
			//cout << "reset1" << endl;
			ev->Reset();
		}
	}
	else
	{
		//shouldn't this check for box touching box right here??
		ParentNode *n = (ParentNode*)node;
		for( int i = 0; i < 4; ++i )
		{
			rResetPlants( n->children[i] );
		}

		for( list<QuadTreeEntrant*>::iterator it = n->extraChildren.begin(); it != n->extraChildren.end(); ++it )
		{
			EnvPlant *ev = (EnvPlant *)(*it);
			//cout << "reset2" << endl;
			ev->Reset();
		}
		
	}
}

void GameSession::ResetInactiveEnemies()
{
	Enemy *e = inactiveEnemyList;

	e = inactiveEnemyList;
	while( e != NULL )
	{
		//cout << "reset inactive enemy" << endl;
		Enemy *temp = e->next;

		e->Reset();
		e = temp;
	}

	inactiveEnemyList = NULL;
}



void GameSession::CleanupDecor()
{
	for (int i = 0; i < EffectLayer::Count; ++i)
	{
		auto &dList = decor[i];
		for (auto it = dList.begin(); it != dList.end(); ++it)
		{
			delete (*it);
		}
		dList.clear();
	}
}

void GameSession::UpdateSoundNodeLists()
{
	soundNodeList->Update();
	pauseSoundNodeList->Update();
}

void GameSession::RecPlayerRecordFrame()
{
	if (recPlayer != NULL)
	{
		recPlayer->RecordFrame();
	}
}

void GameSession::RepPlayerUpdateInput()
{
	if (repPlayer != NULL)
	{
		//currently only records 1 player replays. fix this later
		repPlayer->UpdateInput(GetCurrInput(0));
	}
}

void GameSession::RecGhostRecordFrame()
{
	if (recGhost != NULL)
		recGhost->RecordFrame();
}


GameSession::RaceFight::RaceFight( GameSession *p_owner, int raceFightMaxSeconds )
	: owner( p_owner ), playerScore( 0 ), player2Score( 0 ), hitByPlayerList( NULL ),
	hitByPlayer2List( NULL ), numTargets( 0 )
{
	hud = new RaceFightHUD(this);

	Tileset *scoreTS = owner->GetTileset( "HUD/number_score_80x80.png", 80, 80 );
	Tileset *score2TS = scoreTS;
	int numDigits = 2;
	playerScoreImage = new ImageText( 2, scoreTS );
	playerScoreImage->topRight = Vector2f( 1920/2 - 200, 0 );
	player2ScoreImage = new ImageText( 2, score2TS );
	player2ScoreImage->topRight = Vector2f( 1920/2 + 200 + 80 * 2, 0 );

	gameTimer = new TimerText( scoreTS );
	gameTimer->topRight = Vector2f( 1920/2 + 80 * 2.5, 0 );
	gameTimer->SetNumber( raceFightMaxSeconds );

	victoryScreen = new ResultsScreen( owner );

	Reset();

	tempAllTargets.setFont( owner->mainMenu->arial );
	tempAllTargets.setCharacterSize( 12 );
	tempAllTargets.setFillColor( Color::Red );
}

int GameSession::RaceFight::NumDigits( int number )
{
	int digits = 0;
	do
	{
		++digits;
		number /= 10;
	}
	while( number > 0 );

	return digits;
}

void GameSession::RaceFight::Init()
{
	//after we know how many total targets we have
	int digits = NumDigits( numTargets );
	numberTargetsRemainingImage = new ImageText( digits, playerScoreImage->ts );
	numberTargetsRemainingImage->topRight = Vector2f( 1920/2 + 80 * 3, 90 );
	numberTargetsTotalImage = new ImageText( digits, playerScoreImage->ts );
	numberTargetsTotalImage->topRight = Vector2f( 1920/2 + 80 * 2, 90 );
	numberTargetsTotalImage->UpdateSprite();

	raceWinnerIndex = -1;
	gameOver = false;
	memset(place, 0, sizeof(place));

	playerScore = 0;
	player2Score = 0;
}

void GameSession::RaceFight::RemoveFromPlayerHitList( RaceFightTarget *target )
{
	assert( hitByPlayerList != NULL );

	--playerScore;
	cout << "subbig playerscore is now: " << playerScore << endl;
	if( hitByPlayerList->pPrev == NULL && hitByPlayerList->pNext == NULL )
	{
		cout << "a" << endl;
		assert( hitByPlayerList == target );
		hitByPlayerList = NULL;
	}
	else if( target == hitByPlayerList )
	{
		cout << "b" << endl;
		target->pNext->pPrev = NULL;
		RaceFightTarget *newListHead = target->pNext;
		target->pNext = NULL;
		hitByPlayerList = newListHead;
	}
	else if( target->pPrev != NULL && target->pNext != NULL )
	{
		cout << "c" << endl;
		target->pPrev->pNext = target->pNext;
		target->pNext->pPrev = target->pPrev;
		target->pPrev = NULL;
		target->pNext = NULL;
	}
	else if( target->pNext == NULL )
	{
		cout << "d" << endl;
		target->pPrev->pNext = NULL;
		target->pPrev = NULL;
	}
}

void GameSession::RaceFight::RemoveFromPlayer2HitList( RaceFightTarget *target )
{
	assert( hitByPlayer2List != NULL );

	--player2Score;
	//cout << "subbig player2score is now: " << player2Score << endl;
	if( hitByPlayer2List->p2Prev == NULL && hitByPlayer2List->p2Next == NULL )
	{
		//cout << "e" << endl;
		assert( hitByPlayer2List == target );
		hitByPlayer2List = NULL;
	}
	else if( target == hitByPlayer2List )
	{
		//cout << "f" << endl;
		target->p2Next->p2Prev = NULL;
		RaceFightTarget *newListHead = target->p2Next;
		target->p2Next = NULL;
		hitByPlayer2List = newListHead;
	}
	else if( target->p2Prev != NULL && target->p2Next != NULL )
	{
		//cout << "g" << endl;
		target->p2Prev->p2Next = target->p2Next;
		target->p2Next->p2Prev = target->p2Prev;
		target->p2Prev = NULL;
		target->p2Next = NULL;
	}
	else if( target->p2Next == NULL )
	{
		//cout << "h" << endl;
		target->p2Prev->p2Next = NULL;
		target->p2Prev = NULL;
	}
}

void GameSession::RaceFight::HitByPlayer( int playerIndex,
			RaceFightTarget *target )
{
	assert( target != NULL );
	Actor *player = NULL;
	if( playerIndex == 0 )
	{
		player = owner->GetPlayer( 0 );
		
		playerScore++;
		hud->ScorePoint(RaceFightHUD::PlayerColor::BLUE);
		if( target->action == RaceFightTarget::Action::PLAYER2 )
		{
			RemoveFromPlayer2HitList( target );
			target->action = RaceFightTarget::Action::PLAYER1;
			//playerScoreImage->SetNumber( playerScore );
			//hud->UpdateScore();
		}

		target->gameTimeP1Hit = gameTimer->value;
		//player2ScoreImage->SetNumber( player2Score );
		

		if( hitByPlayerList == NULL )
		{
			hitByPlayerList = target;
			target->pPrev = NULL;
			target->pNext = NULL;
		}
		else
		{
			target->pNext = hitByPlayerList;
			hitByPlayerList->pPrev = target;
			hitByPlayerList = target;
		}

		if( raceWinnerIndex == -1 && playerScore == numTargets )
		{
			gameOver = true;
			place[0] = 1;
			place[1] = 2;
			return;
		}
		else if( raceWinnerIndex == -1 && GetNumRemainingTargets() == 0 )
		{
			raceWinnerIndex = 0;
			hud->SetRaceWinner(RaceFightHUD::BLUE);
		}
	}
	else if( playerIndex == 1 )
	{
		player = owner->GetPlayer( 1 );

		player2Score++;
		hud->ScorePoint(RaceFightHUD::PlayerColor::RED);
		if( target->action == RaceFightTarget::Action::PLAYER1 )
		{
			RemoveFromPlayerHitList( target );
			target->action = RaceFightTarget::Action::PLAYER2;
			//playerScoreImage->SetNumber( playerScore );
		}
		

		target->gameTimeP2Hit = gameTimer->value;
		//player2ScoreImage->SetNumber( player2Score );
		
		if( hitByPlayer2List == NULL )
		{
			hitByPlayer2List = target;
			target->p2Prev = NULL;
			target->p2Next = NULL;
		}
		else
		{
			target->p2Next = hitByPlayer2List;
			hitByPlayer2List->p2Prev = target;
			hitByPlayer2List = target;
		}

		if( raceWinnerIndex == -1 && player2Score == numTargets )
		{
			gameOver = true;
			place[0] = 2;
			place[1] = 1;
			return;
		}
		else if( raceWinnerIndex == -1 && GetNumRemainingTargets() == 0 )
		{
			raceWinnerIndex = 1;
			hud->SetRaceWinner(RaceFightHUD::RED);
		}
	}

	hud->UpdateScore();
}

void GameSession::RaceFight::Reset()
{
	playerScore = 0;
	player2Score = 0;
	hitByPlayerList = NULL;
	hitByPlayer2List = NULL;
	frameCounter = 0;
	playerHitCounter = 0;
	player2HitCounter = 0;
	gameOver = false;
	memset(place, 0, sizeof(place));
}

void GameSession::RaceFight::PlayerHitByPlayer( int attacker,
			int defender )
{
	Actor *at = NULL;
	Actor *def = NULL;
	if( attacker == 0 )
	{
		at = owner->GetPlayer( 0 );
		def = owner->GetPlayer( 1 );

		++playerHitCounter;
		if( raceWinnerIndex == -1 )
		{
			if( hitByPlayer2List != NULL )
			{
				HitByPlayer( attacker, hitByPlayer2List );
			}
		}
		else
		{
			if( playerScore < player2Score 
				|| ( playerScore == player2Score && raceWinnerIndex == 1 ) )
			{
				//losing
				assert( hitByPlayer2List != NULL );
				
				
				//inefficient but I'm lazy
				RaceFightTarget *last = hitByPlayer2List;
				while( last->next != NULL )
				{
					last = last->p2Next;
				}
				last->action = RaceFightTarget::Action::PLAYER1;

				HitByPlayer( attacker, last );
			}
			else
			{
				//winning
				gameOver = true;
				place[0] = 1;
				place[1] = 2;
			}
		}
	}
	else
	{
		++player2HitCounter;

		at = owner->GetPlayer( 1 );
		def = owner->GetPlayer( 0 );

		if( raceWinnerIndex == -1 ) //race is not over
		{
			if( hitByPlayerList != NULL )
			{
				//hitByPlayerList->action = RaceFightTarget::Action::PLAYER2;
				HitByPlayer( attacker, hitByPlayerList );
			}
		}
		else //race is over
		{
			if( player2Score < playerScore 
				|| ( player2Score == playerScore && raceWinnerIndex == 0 ) )
			{
				//losing
				assert( hitByPlayerList != NULL );
				
				
				//inefficient but I'm lazy
				RaceFightTarget *last = hitByPlayerList;
				while( last->next != NULL )
				{
					last = last->pNext;
				}
				last->action = RaceFightTarget::Action::PLAYER2;

				HitByPlayer( attacker, last );
			}
			else
			{
				//winning
				gameOver = true;
				place[0] = 2;
				place[1] = 1;
			}
		}
	}


}

int GameSession::RaceFight::GetNumRemainingTargets()
{
	return numTargets - ( playerScore + player2Score );
}

void GameSession::RaceFight::DrawScore( sf::RenderTarget *target )
{
	//target->draw( scoreTestSprite );
	hud->Draw(target);
	//playerScoreImage->Draw( target );
	//player2ScoreImage->Draw( target );

	//gameTimer->Draw( target );

	//numberTargetsTotalImage->Draw( target );
	//numberTargetsRemainingImage->Draw( target );
}

void GameSession::RaceFight::UpdateScore()
{
	gameTimer->UpdateSprite();
	playerScoreImage->UpdateSprite();
	player2ScoreImage->UpdateSprite();

	numberTargetsRemainingImage->SetNumber( GetNumRemainingTargets() );
	numberTargetsRemainingImage->UpdateSprite();

	TickFrame();
}

void GameSession::RaceFight::TickClock()
{
	testWindow->Update( owner->GetCurrInput( 0 ), owner->GetPrevInput( 0 ) );

	if( gameTimer->value > 0 )
		gameTimer->SetNumber( gameTimer->value-- );

	if( gameTimer->value == 0 )
	{
		gameOver = true;

		if( raceWinnerIndex == -1 ) //race isnt over
		{
			 if( playerScore < player2Score )
			 {
				 place[0] = 2;
				 place[1] = 1;
			 }
			 else if( playerScore > player2Score )
			 {
				 place[0] = 1;
				 place[1] = 2;
			 }
			 else
			 {
				 if( playerHitCounter < player2HitCounter )
				 {
					 place[0] = 2;
					 place[1] = 1;
				 }
				 else if( playerHitCounter > player2HitCounter )
				 {
					 place[0] = 1;
					 place[1] = 2;
				 }
				 else
				 {
					 if( playerScore == 0 )
					 {
						 //draw
						 assert( player2Score == 0 );
						 place[0] = 1;
						 place[1] = 1;
					 }
					 else
					 {
						 RaceFightTarget *last1 = hitByPlayerList;
						 RaceFightTarget *last2 = hitByPlayer2List;

						 while( last1->pNext != NULL )
						 {
							 last1 = last1->pNext;
						 }
						 while( last2->p2Next != NULL )
						 {
							 last2 = last2->p2Next;
						 }

						 int lastTimeP1 = last1->gameTimeP1Hit;
						 int lastTimeP2 = last2->gameTimeP2Hit;

						 if( lastTimeP1 > lastTimeP2 ) //greater time means hit sooner
						 {
							 place[0] = 1;
							 place[1] = 2;
						 }
						 else if( lastTimeP1 < lastTimeP2 )//greater time means hit sooner
						 {
							 place[0] = 2;
							 place[1] = 1;
						 }
						 else
						 {
							 //draw
							 place[0] = 1;
							 place[1] = 1;
						 }
					 }
				 }
			 }
		}
		else
		{

		}
	}
}

void GameSession::RaceFight::TickFrame()
{
	if( frameCounter == 60 )
	{
		int val = gameTimer->value - 1;
		if( val > 0 )		
		{
			gameTimer->SetNumber( val );
			
		}
		frameCounter = 0;
	}
	else
	{
		++frameCounter;
	}
}

GameSession::DecorDraw::DecorDraw(sf::Vertex *q,
	int numVerts,
	Tileset *t)
	:quads(q), numVertices(numVerts),
	ts(t)
{}
GameSession::DecorDraw::~DecorDraw()
{
	delete[] quads;
}
void GameSession::DecorDraw::Draw(sf::RenderTarget *target)
{
	target->draw(quads,	numVertices, sf::Quads, ts->texture);
}




//void GameSession::LoadEnemy(std::ifstream &is )
//{
//	string typeName;
//	is >> typeName;
//
//	Enemy *enem = NULL;
//
//	Edge *loadedEdge;
//	double edgeQuantity;
//	int hasMonitor;
//	int level;
//	Vector2i pos;
//	std::string pName;
//
//	int pathLength;
//	vector<Vector2i> localPath;
//	//LoadStandardGroundedEnemy(is, loadedEdge, edgeQuantity, hasMonitor, level);
//
//	if (typeName == "goal" || typeName == "greengoal")
//	{
//		//always grounded
//
//		LoadEdgeInfo(is, loadedEdge, edgeQuantity);
//
//		int w = 0;
//		if (typeName == "greengoal")
//		{
//			w = 1;
//		}
//
//		//cout << "polyIndex: " << polyIndex[terrainIndex] << ", tindex: " << terrainIndex << endl;
//		//Goal *enemy = new Goal(loadedEdge, edgeQuantity, w);
//		Enemy *enemy = NULL;
//		assert(0);
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);
//
//		hasGoal = true;
//		goalPos = enemy->position;
//
//		V2d gPos = enemy->ground->GetPosition(enemy->edgeQuantity);
//		V2d norm = enemy->ground->Normal();
//		double nodeHeight = 104;
//		goalNodePos = gPos + norm * nodeHeight;
//		float space = 78.f;
//		goalNodePosFinal = V2d(goalNodePos.x, goalNodePos.y - space);
//		//cout << "setting goalPos: " << goalPos.x << ", " << goalPos.y << endl;
//	}
//	else if (typeName == "poi")
//	{
//		string air;
//		//string pname;
//		PoiInfo *pi = NULL;
//
//		int posType;
//		is >> posType;
//
//		if (posType == 0)
//		{
//			LoadNamedAirInfo(is, pos, pName);
//			pi = new PoiInfo(pName, pos);
//		}
//
//		else if (posType == 1)
//		{
//			LoadEdgeInfo(is, loadedEdge, edgeQuantity);
//			is >> pName;
//
//			pi = new PoiInfo(pName, loadedEdge,
//				edgeQuantity);				
//		}
//		else
//		{
//			cout << "air failure: " << air << endl;
//			assert(0);
//		}
//
//		poiMap[pName] = pi;
//	}
//	else if (typeName == "xbarrier")
//	{
//		LoadNamedAirInfo(is, pos, pName);
//
//		int hEdge;
//		is >> hEdge;
//		bool hEdgeB = hEdge;
//
//		Barrier *b = new Barrier(this, pName, true, pos.x, hEdgeB, NULL);
//
//		barrierMap[pName] = b;
//		barriers.push_back(b);
//	}
//	else if (typeName == "extrascene")
//	{
//		LoadNamedAirInfo(is, pos, pName);
//
//		int extraSceneType;
//		is >> extraSceneType;
//
//		BasicBossScene *scene = BasicBossScene::CreateScene(this, pName);
//		if (extraSceneType == 0)//prelevel
//		{
//			preLevelScene = scene;
//		}
//		else if (extraSceneType == 1)//postlevel
//		{
//			postLevelScene = scene;
//		}
//	}	
//	else if (typeName == "camerashot")
//	{
//		LoadNamedAirInfo(is, pos, pName);
//
//		float z;
//		is >> z;
//
//		CameraShot *shot = new CameraShot(pName, Vector2f(pos), z);
//		if (cameraShotMap.count(pName) > 0 )
//		{
//			assert(false);
//		}
//
//		cameraShotMap[pName] = shot;
//	}
//	else if (typeName == "shard")
//	{
//		int xPos, yPos;
//
//		//always air
//
//		is >> xPos;
//		is >> yPos;
//
//		int w;
//		is >> w;
//
//		int localIndex;
//		is >> localIndex;
//
//		Shard *enemy = new Shard(Vector2i(xPos, yPos), w, localIndex);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);
//	}
//	else if (typeName == "ship")
//	{
//		LoadAirInfo(is, pos);
//
//		shipEntrancePos = V2d(pos);
//		hasShipEntrance = true;
//
//		ResetShipSequence();
//	}
//	else if (typeName == "healthfly")
//	{
//		int xPos, yPos;
//
//		//always air
//
//		is >> xPos;
//		is >> yPos;
//
//		int level;
//		is >> level;
//
//		/*HealthFly *enemy = new HealthFly(this, Vector2i(xPos, yPos), level, numTotalFlies);
//
//		allFlies.push_back(enemy);
//		numTotalFlies++;
//		activeItemTree->Insert(enemy);
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "blocker" || typeName == "greenblocker")
//	{
//		int xPos, yPos;
//
//		is >> xPos;
//		is >> yPos;
//
//		
//		Enemy::ReadPath(is, pathLength, localPath);
//
//		int bType;
//		is >> bType;
//
//		if (typeName == "blocker")
//		{
//			bType = BlockerChain::BLUE;
//		}
//		else if (typeName == "greenblocker")
//		{
//			bType = BlockerChain::GREEN;
//		}
//
//		int armored;
//		is >> armored;
//
//		int spacing;
//		is >> spacing;
//
//		int level;
//		is >> level;
//
//		Enemy *enemy = NULL;
//		assert(0);
//		//BlockerChain *enemy = new BlockerChain(Vector2i(xPos, yPos), localPath, bType, armored, spacing, level);
//
//		fullEnemyList.push_back(enemy);
//
//		enem = enemy;
//
//		enemyTree->Insert(enemy);
//	}
//	else if (typeName == "comboer")
//	{
//
//		int xPos, yPos;
//
//		//always air
//		is >> xPos;
//		is >> yPos;
//
//		Enemy::ReadPath(is, pathLength, localPath);
//
//		bool loop;
//		Enemy::ReadBool(is, loop);
//
//		int level;
//		is >> level;
//
//
//		//Comboer *enemy = new Comboer(Vector2i(xPos, yPos), localPath, loop, level);
//		Enemy *enemy = NULL;
//		assert(0);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);
//	}
//	else if (typeName == "splitcomboer")
//	{
//
//		int xPos, yPos;
//
//		is >> xPos;
//		is >> yPos;
//
//		Enemy::ReadPath(is, pathLength, localPath);
//
//		bool loop;
//		Enemy::ReadBool(is, loop);
//
//		int level;
//		is >> level;
//
//		Enemy *enemy = NULL;
//		//SplitComboer *enemy = new SplitComboer(Vector2i(xPos, yPos), localPath, loop, level);
//
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);
//	}
//	else if (typeName == "booster")
//	{
//		int xPos, yPos;
//
//		//always air
//
//		is >> xPos;
//		is >> yPos;
//
//		int level;
//		is >> level;
//
//		Booster *enemy = new Booster(Vector2i(xPos, yPos), level);
//		//GravityModifier *enemy = new GravityModifier(this, Vector2i(xPos, yPos), .5, 300);
//
//		activeItemTree->Insert(enemy);
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);
//	}
//	else if (typeName == "key")
//	{
//		int xPos, yPos;
//		is >> xPos;
//		is >> yPos;
//
//		int numKeys;
//		is >> numKeys;
//
//		int zType;
//		is >> zType;
//
//		keyNumberObjects.push_back(new KeyNumberObj(Vector2i(xPos, yPos), numKeys, zType));
//	}
//	else if (typeName == "spring" || typeName == "gravityspring" || typeName == "bouncespring" 
//		|| typeName == "airbouncespring" )//|| typeName == "teleportspring" )
//	{
//		int xPos, yPos;
//
//		//always air
//
//		is >> xPos;
//		is >> yPos;
//
//		int speed = 0;
//		//if (typeName == "spring" || typeName == "gravityspring" || ty)
//		{
//			is >> speed;
//		}
//
//
//		Vector2i other;
//		is >> other.x;
//		is >> other.y;
//
//		Spring::SpringType sp;
//		if (typeName == "spring")
//		{
//			sp = Spring::BLUE;
//		}
//		else if (typeName == "gravityspring")
//		{
//			sp = Spring::GREEN;
//		}
//		else if (typeName == "bouncespring")
//		{
//			sp = Spring::BOUNCE;
//		}
//		else if (typeName == "airbouncespring")
//		{
//			sp = Spring::AIRBOUNCE;
//		}
//		else if (typeName == "teleportspring")
//		{
//			sp = Spring::TELEPORT;
//		}
//		//CurveLauncher * enemy = new CurveLauncher(this, Vector2i(xPos, yPos), other, moveFrames);
//		Spring *enemy = new Spring( sp, Vector2i(xPos, yPos), other, speed);
//
//		activeItemTree->Insert(enemy);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);
//	}
//	else if (typeName == "teleporter" || typeName == "onewayteleporter")
//	{
//		int xPos, yPos;
//
//		//always air
//
//		is >> xPos;
//		is >> yPos;
//
//
//		Vector2i other;
//		is >> other.x;
//		is >> other.y;
//
//		bool bothWays = true;
//		if (typeName == "onewayteleporter")
//		{
//			bothWays = false;
//		}
//
//		/*Teleporter *enemy = new Teleporter(this, Vector2i(xPos, yPos), other, bothWays);
//
//		activeItemTree->Insert(enemy);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);
//
//		Teleporter *secondary = enemy->CreateSecondary();
//
//		activeItemTree->Insert(secondary);
//
//		fullEnemyList.push_back(secondary);
//
//		enemyTree->Insert(secondary);*/
//
//
//	}
//	else if (typeName == "upbouncebooster" || typeName == "omnibouncebooster")
//	{
//		int xPos, yPos;
//			
//		//always air
//
//		is >> xPos;
//		is >> yPos;
//			
//		int level;
//		is >> level;
//
//		bool upOnly = false;
//		if (typeName == "upbouncebooster")
//		{
//			upOnly = true;
//		}
//
//
//		/*BounceBooster *enemy = new BounceBooster(this, Vector2i(xPos, yPos), upOnly, level);
//
//		activeItemTree->Insert(enemy);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "rail" || typeName == "grindrail" )
//	{
//			
//		int xPos, yPos;
//
//		//always air
//
//
//		is >> xPos;
//		is >> yPos;
//
//		Enemy::ReadPath(is, pathLength, localPath);
//
//		int accelerate;
//		is >> accelerate;
//
//		int level;
//		is >> level;
//
//		bool requirePower = false;
//		if (typeName == "grindrail")
//		{
//			requirePower = true;
//		}
//
//		//Rail *r = new Rail(this, Vector2i(xPos, yPos), localPath, requirePower, accelerate, level );
//
//		//++totalRails;
//	}
//	//w1
//	else if (typeName == "patroller")
//	{
//
//		int xPos, yPos;
//
//		//always air
//
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//		Enemy::ReadPath(is, pathLength, localPath);
//
//		bool loop;
//		Enemy::ReadBool(is, loop);
//
//		int level;
//		is >> level;
//		Patroller *enemy = new Patroller(hasMonitor, Vector2i(xPos, yPos), localPath, loop, level);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);
//	}
//	else if (typeName == "bosscrawler")
//	{
//		//always grounded
//
//		LoadEdgeInfo(is, loadedEdge, edgeQuantity);
//
//		CrawlerQueen *enemy = new CrawlerQueen(loadedEdge,
//			edgeQuantity, false);
//
//		fullEnemyList.push_back(enemy);
//
//		enem = enemy;
//
//		enemyTree->Insert(enemy);
//	}
//	else if (typeName == "basicturret")
//	{
//		//always grounded
//
//
//		LoadStandardGroundedEnemy(is, loadedEdge, edgeQuantity, hasMonitor, level);
//		BasicTurret *enemy = new BasicTurret(hasMonitor, loadedEdge, edgeQuantity,level);
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);
//	}
//	else if (typeName == "crawler")
//	{
//		//always grounded
//
//		LoadStandardGroundedEnemy(is, loadedEdge, edgeQuantity, hasMonitor, level);
//
//		Crawler *enemy = new Crawler(hasMonitor, loadedEdge,
//			edgeQuantity, level);
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);
//	}
//	else if (typeName == "shroom")
//	{
//		LoadStandardGroundedEnemy(is, loadedEdge, edgeQuantity, hasMonitor, level);
//
//		Shroom *enemy = new Shroom(hasMonitor, loadedEdge, edgeQuantity, level);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);
//	}
//	else if (typeName == "airdasher")
//	{
//
//		int xPos, yPos;
//
//		//always air
//
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//		int level;
//		is >> level;
//
//
//		Airdasher *enemy = new Airdasher(hasMonitor, Vector2i(xPos, yPos), level);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);
//	}
//
//	//w2
//	else if (typeName == "bat")
//	{
//
//		int xPos, yPos;
//
//		//always air
//
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//
//		Enemy::ReadPath(is, pathLength, localPath);
//
//		bool loop;
//		Enemy::ReadBool(is, loop);
//
//		int level;
//		is >> level;
//
//
//
//		/*Bat *enemy = new Bat(this, hasMonitor, Vector2i(xPos, yPos), localPath, loop, level);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "airdashjuggler")
//	{
//
//		int xPos, yPos;
//
//		//always air
//
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//
//		Enemy::ReadPath(is, pathLength, localPath);
//
//		int level;
//		is >> level;
//
//			
//
//		/*AirdashJuggler *enemy = new AirdashJuggler(this, hasMonitor, Vector2i(xPos, yPos), localPath, level);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "downgravityjuggler" || typeName == "upgravityjuggler" || typeName == "bouncejuggler" 
//		|| typeName == "wirejuggler" )
//	{
//
//		int xPos, yPos;
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//
//		Enemy::ReadPath(is, pathLength, localPath);
//
//		int level;
//		is >> level;
//
//		int numJuggles;
//		is >> numJuggles;
//
//
//		/*Enemy *enemy;
//		if (typeName == "downgravityjuggler" || typeName == "upgravityjuggler")
//		{
//			bool reversed = false;
//			if (typeName == "upgravityjuggler")
//			{
//				reversed = true;
//			}
//
//			enemy = new GravityJuggler(this, hasMonitor, Vector2i(xPos, yPos), localPath,
//				level, numJuggles, reversed);
//		}
//		else if (typeName == "bouncejuggler")
//		{
//			enemy = new BounceJuggler(this, hasMonitor, Vector2i(xPos, yPos), localPath,
//				level, numJuggles);
//		}
//		else if (typeName == "wirejuggler")
//		{
//			enemy = new WireJuggler(this, hasMonitor, Vector2i(xPos, yPos), localPath,
//				level, numJuggles, WireJuggler::T_BLUE);
//		}
//			
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "grindjugglercw" || typeName == "grindjugglerccw" )
//	{
//
//		int xPos, yPos;
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//		int level;
//		is >> level;
//
//		int numJuggles;
//		is >> numJuggles;
//
//		bool cw = true;
//		if (typeName == "grindjugglerccw")
//		{
//			cw = false;
//		}
//
//		/*Enemy *enemy = new GrindJuggler(this, hasMonitor, Vector2i(xPos, yPos),
//			level, numJuggles, cw);
//
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "groundedgrindjugglercw" || typeName == "groundedgrindjugglerccw")
//	{
//
//		LoadStandardGroundedEnemy(is, loadedEdge, edgeQuantity, hasMonitor, level);
//
//		int numJuggles;
//		is >> numJuggles;
//
//		bool cw = true;
//		if (typeName == "groundedgrindjugglerccw")
//		{
//			cw = false;
//		}
//
//	/*	Enemy *enemy = new GroundedGrindJuggler(this, hasMonitor, loadedEdge,
//			edgeQuantity, level, numJuggles, cw);
//
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "jugglercatcher")
//	{
//		int xPos, yPos;
//
//		//always air
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//		int level;
//		is >> level;
//
//		//JugglerCatcher *enemy = new JugglerCatcher(this, hasMonitor, Vector2i(xPos, yPos), level);
//		////GravityModifier *enemy = new GravityModifier(this, Vector2i(xPos, yPos), .5, 300);
//
//		//activeEnemyItemTree->Insert(enemy);
//		//fullEnemyList.push_back(enemy);
//		//enem = enemy;
//
//		//enemyTree->Insert(enemy);
//	}
//	else if (typeName == "poisonfrog")
//	{
//		LoadStandardGroundedEnemy(is, loadedEdge, edgeQuantity, hasMonitor, level);
//		/*int gravFactor;
//		is >> gravFactor;
//
//		int jumpStrengthX;
//		is >> jumpStrengthX;
//
//		int jumpStrengthY;
//		is >> jumpStrengthY;
//
//		int jumpFramesWait;
//		is >> jumpFramesWait;*/
//
//		/*PoisonFrog *enemy = new PoisonFrog(this, hasMonitor, loadedEdge,
//			edgeQuantity, level);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "gravityfaller")
//	{
//		LoadStandardGroundedEnemy(is, loadedEdge, edgeQuantity, hasMonitor, level);
//
//		/*GravityFaller *enemy = new GravityFaller(this, hasMonitor, loadedEdge,
//			edgeQuantity, level);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "stagbeetle")
//	{
//		//always grounded
//
//		LoadStandardGroundedEnemy(is, loadedEdge, edgeQuantity, hasMonitor, level);
//
//
//		/*StagBeetle *enemy = new StagBeetle(this, hasMonitor, loadedEdge,
//			edgeQuantity, level );
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "curveturret")
//	{
//		//always grounded
//
//		LoadStandardGroundedEnemy(is, loadedEdge, edgeQuantity, hasMonitor, level);
//
//		/*CurveTurret *enemy = new CurveTurret(this, hasMonitor, loadedEdge, edgeQuantity, level);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "gravityincreaser" || typeName == "gravitydecreaser" )
//	{
//		bool increaser = (typeName == "gravityincreaser");
//
//		int xPos, yPos;
//
//		//always air
//
//		is >> xPos;
//		is >> yPos;
//
//		int level;
//		is >> level;
//
//	/*	GravityModifier *enemy = new GravityModifier(this, Vector2i(xPos, yPos), level, increaser);
//
//		activeItemTree->Insert(enemy);
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//
//	else if (typeName == "bossbird")
//	{
//
//		int xPos, yPos;
//
//		is >> xPos;
//		is >> yPos;
//
//		//BirdBoss *enemy = new BirdBoss(this, Vector2i(xPos, yPos));
//
//		//fullEnemyList.push_back(enemy);
//		//enem = enemy;
//
//		//enemyTree->Insert(enemy);
//	}
//
//	//w3
//	else if (typeName == "pulser")
//	{
//
//		int xPos, yPos;
//
//		//always air
//
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//
//
//		Enemy::ReadPath(is, pathLength, localPath);
//
//		bool loop;
//		Enemy::ReadBool(is, loop);
//
//		int level;
//		is >> level;
//
//		/*Pulser *enemy = new Pulser(this, hasMonitor, Vector2i(xPos, yPos), localPath,
//			loop, level );
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "cactus")
//	{
//		//always grounded
//
//		LoadStandardGroundedEnemy(is, loadedEdge, edgeQuantity, hasMonitor, level);
//
//		/*Cactus *enemy = new Cactus(this, hasMonitor,
//			loadedEdge, edgeQuantity, level );
//
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "owl")
//	{
//
//		int xPos, yPos;
//
//		//always air
//
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//		int level;
//		is >> level;
//
//		/*Owl *enemy = new Owl(this, hasMonitor, Vector2i(xPos, yPos), level);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "badger")
//	{
//		//always grounded
//
//		LoadStandardGroundedEnemy(is, loadedEdge, edgeQuantity, hasMonitor, level);
//
//		/*Badger *enemy = new Badger(this, hasMonitor, loadedEdge, edgeQuantity, level);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "roadrunner")
//	{
//		//always grounded
//
//		LoadStandardGroundedEnemy(is, loadedEdge, edgeQuantity, hasMonitor, level);
//
//		/*RoadRunner *enemy = new RoadRunner(this, hasMonitor, loadedEdge, edgeQuantity, level);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "bouncefloater")
//	{
//		int xPos, yPos;
//
//		//always air
//
//
//		is >> xPos;
//		is >> yPos;
//
//		int level;
//		is >> level;
//
//		/*BounceFloater *enemy = new BounceFloater(this, Vector2i(xPos, yPos), level);
//
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "bosscoyote")
//	{
//		//always grounded
//
//		Vector2i pos;
//
//		is >> pos.x;
//		is >> pos.y;
//
//		/*Boss_Coyote *enemy = new Boss_Coyote( this, pos );
//		b_coyote = enemy;
//
//		fullEnemyList.push_back( enemy );*/
//	}
//
//	//w4
//	else if (typeName == "turtle")
//	{
//
//		int xPos, yPos;
//
//		//always air
//
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//		int level;
//		is >> level;
//
//		
//		/*Vector2i pos(xPos, yPos);
//		Turtle *enemy = new Turtle(this, hasMonitor, Vector2i(xPos, yPos), level);
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "coral")
//	{
//
//		int xPos, yPos;
//
//		//always air
//
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//		int moveFrames;
//		is >> moveFrames;
//
//		/*SecurityWeb * enemy = new SecurityWeb( this,
//		hasMonitor, Vector2i( xPos, yPos ), 8, 0, 10 );
//
//
//		fullEnemyList.push_back( enemy );
//		enem = enemy;
//
//		enemyTree->Insert( enemy );*/
//	}
//	else if (typeName == "cheetah")
//	{
//		//always grounded
//
//		LoadStandardGroundedEnemy(is, loadedEdge, edgeQuantity, hasMonitor, level);
//
//		/*Cheetah *enemy = new Cheetah( this, hasMonitor,
//		loadedEdge, edgeQuantity, level );
//
//		fullEnemyList.push_back( enemy );
//		enem = enemy;
//
//		enemyTree->Insert( enemy );*/
//	}
//	else if (typeName == "spider")
//	{
//		//always grounded
//
//		LoadStandardGroundedEnemy(is, loadedEdge, edgeQuantity, hasMonitor, level);
//
//		/*Spider *enemy = new Spider(this, hasMonitor, loadedEdge, edgeQuantity,
//			level);
//
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "bosstiger")
//	{
//		int xPos, yPos;
//
//		is >> xPos;
//		is >> yPos;
//
//		/*Boss_Tiger *enemy = new Boss_Tiger( this, Vector2i ( xPos, yPos ) );
//
//		fullEnemyList.push_back( enemy );
//
//		b_tiger = enemy;
//
//		enemyTree->Insert( enemy );*/
//	}
//
//	//w5
//	else if (typeName == "hungrycomboer" || typeName == "hungryreturncomboer" )
//	{
//		int xPos, yPos;
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//		int level;
//		is >> level;
//
//		int numJuggles;
//		is >> numJuggles;
//
//			
//		bool returnsToPlayer = false;
//		if (typeName == "hungryreturncomboer")
//		{
//			returnsToPlayer = true;
//		}
//			
//		/*HungryComboer *enemy = new HungryComboer(this, hasMonitor, Vector2i(xPos, yPos), level, numJuggles,
//			returnsToPlayer );
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "relativecomboer" || typeName == "relativecomboerdetach")
//	{
//		int xPos, yPos;
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//		Enemy::ReadPath(is, pathLength, localPath);
//
//		int level;
//		is >> level;
//
//		int numJuggles;
//		is >> numJuggles;
//
//		bool detachOnKill = false;
//		if (typeName == "relativecomboerdetach")
//		{
//			detachOnKill = true;
//		}
//
//		/*RelativeComboer *enemy = new RelativeComboer(this, hasMonitor, Vector2i(xPos, yPos), localPath, level, numJuggles,
//			detachOnKill);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "swarm")
//	{
//
//		int xPos, yPos;
//
//		//always air
//
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//		int level;
//		is >> level;
//
//		/*Swarm *enemy = new Swarm( this, Vector2i( xPos, yPos ), hasMonitor, level );
//		fullEnemyList.push_back( enemy );
//		enem = enemy;
//
//		enemyTree->Insert( enemy );*/
//	}
//	else if (typeName == "shark")
//	{
//
//		int xPos, yPos;
//
//		//always air
//
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//		int level;
//		is >> level;
//
//		/*Shark *enemy = new Shark( this, hasMonitor, Vector2i( xPos, yPos ), level );
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert( enemy );*/
//	}
//	else if (typeName == "ghost")
//	{
//		int xPos, yPos;
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//		int level;
//		is >> level;
//
//		/*Ghost *enemy = new Ghost( this, hasMonitor, Vector2i( xPos, yPos ), level );
//
//		fullEnemyList.push_back( enemy );
//		enem = enemy;
//
//		enemyTree->Insert( enemy );*/
//	}
//	else if (typeName == "overgrowth")
//	{
//		//always grounded
//
//		/*int terrainIndex;
//		is >> terrainIndex;
//
//		int edgeIndex;
//		is >> edgeIndex;
//
//		double edgeQuantity;
//		is >> edgeQuantity;
//
//		int hasMonitor;
//		is >> hasMonitor;*/
//
//		//GrowingTree * enemy = new GrowingTree( this, hasMonitor,
//		//	loadedEdge, 
//		//	edgeQuantity, 32, 0, 1000 );
//
//		//
//		//fullEnemyList.push_back( enemy );
//		//enem = enemy;
//
//		//enemyTree->Insert( enemy );
//	}
//	else if (typeName == "growingtree")
//	{
//		//always grounded
//
//		LoadStandardGroundedEnemy(is, loadedEdge, edgeQuantity, hasMonitor, level);
//
//		//GrowingTree * enemy = new GrowingTree(this, hasMonitor,
//		//	loadedEdge, edgeQuantity, level);
//
//		////
//		//fullEnemyList.push_back( enemy );
//		//enem = enemy;
//
//		//enemyTree->Insert( enemy );
//	}
//	else if (typeName == "bossgator")
//	{
//
//		int xPos, yPos;
//
//		is >> xPos;
//		is >> yPos;
//
//		/*Boss_Gator *enemy = new Boss_Gator( this, Vector2i ( xPos, yPos ) );
//
//		fullEnemyList.push_back( enemy );
//
//		b_gator = enemy;
//
//		enemyTree->Insert( enemy );*/
//	}
//
//	//w6
//	else if (typeName == "specter")
//	{
//
//		int xPos, yPos;
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//		int level;
//		is >> level;
//
//		//Specter *enemy = new Specter( this, hasMonitor, Vector2i( xPos, yPos ), level );
//		//fullEnemyList.push_back( enemy );
//		////enem = enemy;
//
//		//enemyTree->Insert( enemy );
//	}
//	else if (typeName == "swinglaunchercw" || typeName == "swinglauncherccw")
//	{
//		int xPos, yPos;
//
//		//always air
//
//		is >> xPos;
//		is >> yPos;
//
//		int speed = 0;
//		is >> speed;
//
//		Vector2i other;
//		is >> other.x;
//		is >> other.y;
//
//		bool cw = true;
//		if (typeName == "swinglauncherccw")
//		{
//			cw = false;
//		}
//
//		/*SwingLauncher *enemy = new SwingLauncher(this, Vector2i(xPos, yPos), other, speed, cw);
//
//		activeItemTree->Insert(enemy);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "narwhal")
//	{
//
//		int xPos, yPos;
//
//		//always air
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//		Vector2i dest;
//		is >> dest.x;
//		is >> dest.y;
//
//		int moveFrames;
//		is >> moveFrames;
//
//
//		/*Jay *enemy = new Jay( this, hasMonitor, Vector2i( xPos, yPos ), dest );
//		fullEnemyList.push_back( enemy );
//		enem = enemy;
//
//		enemyTree->Insert( enemy );*/
//	}
//	else if (typeName == "copycat")
//	{
//
//		int xPos, yPos;
//
//		//always air
//
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//		/*Copycat *enemy = new Copycat( this, hasMonitor, Vector2i( xPos, yPos ) );
//
//
//		fullEnemyList.push_back( enemy );
//		enem = enemy;
//
//		enemyTree->Insert( enemy );*/
//	}
//	else if (typeName == "gorilla")
//	{
//
//		int xPos, yPos;
//
//		//always air
//
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//		//int wallWidth;
//		//is >> wallWidth;
//
//		//int followFrames;
//		//is >> followFrames;
//
//		int level;
//		is >> level;
//
//
//		/*Gorilla *enemy = new Gorilla( this, hasMonitor, Vector2i( xPos, yPos ), level );
//
//		fullEnemyList.push_back( enemy );
//		enem = enemy;
//
//		enemyTree->Insert( enemy );*/
//	}
//	else if (typeName == "wiretarget")
//	{
//
//		int xPos, yPos;
//
//		is >> xPos;
//		is >> yPos;
//
//		int hasMonitor;
//		is >> hasMonitor;
//
//		int level;
//		is >> level;
//
//		/*WireTarget *enemy = new WireTarget(this, hasMonitor, Vector2i(xPos, yPos), level);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);*/
//	}
//	else if (typeName == "bossskeleton")
//	{
//		int xPos, yPos;
//
//		is >> xPos;
//		is >> yPos;
//
//		/*Boss_Skeleton *enemy = new Boss_Skeleton( this, Vector2i ( xPos, yPos ) );
//
//		fullEnemyList.push_back( enemy );
//
//		b_skeleton = enemy;*/
//	}
//
//
//	else if (typeName == "nexus")
//	{
//		//always grounded
//
//		LoadEdgeInfo(is, loadedEdge, edgeQuantity);
//
//		int nexusIndex;
//		is >> nexusIndex;
//
//		Nexus *enemy = new Nexus(loadedEdge, edgeQuantity);
//
//		goalNodePos = enemy->GetKillPos();
//		float space = 78.f;
//		V2d end(goalNodePos.x, goalNodePos.y - space);
//		hasGoal = true;
//		nexus = enemy;
//		//nexusPos = enemy->GetKillPos();//enemy->position;
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);
//	}
//	else if (typeName == "shippickup")
//	{
//		LoadEdgeInfo(is, loadedEdge, edgeQuantity);
//		int facingRight;
//		is >> facingRight;
//
//		ShipPickup *enemy = new ShipPickup(loadedEdge, edgeQuantity,
//			facingRight);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);
//
//
//		if (shipExitScene == NULL )
//		{
//			shipExitScene = new ShipExitScene(this);
//			shipExitScene->Init();
//		}
//		/*if (shipExitSeq == NULL)
//		{
//			shipExitSeq = new ShipExitSeq(this);
//		}*/
//	}
//	else if (typeName == "groundtrigger")
//	{
//		LoadEdgeInfo(is, loadedEdge, edgeQuantity);
//
//		int facingRight;
//		is >> facingRight;
//
//		string tType;
//		is >> tType;
//
//		GroundTrigger *enemy = new GroundTrigger(loadedEdge, edgeQuantity,
//			facingRight, tType);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);
//	}
//	else if (typeName == "airtrigger")
//	{
//		Vector2i pos;
//
//		//always air
//		is >> pos.x;
//		is >> pos.y;
//
//		string typeStr;
//		is >> typeStr;
//
//		int rectWidth;
//		is >> rectWidth;
//
//		int rectHeight;
//		is >> rectHeight;
//		//int hasMonitor;
//		//is >> hasMonitor;
//		AirTrigger *at = new AirTrigger( this, V2d(pos), rectWidth, rectHeight, typeStr);
//		airTriggerTree->Insert(at);
//		fullAirTriggerList.push_back(at);
//	}
//	else if (typeName == "flowerpod")
//	{
//		LoadEdgeInfo(is, loadedEdge, edgeQuantity);
//
//		string tType;
//		is >> tType;
//
//		FlowerPod *enemy = new FlowerPod(tType,
//			loadedEdge, edgeQuantity);
//
//		fullEnemyList.push_back(enemy);
//		enem = enemy;
//
//		enemyTree->Insert(enemy);
//	}
//	//w6
//	else if (typeName == "racefighttarget")
//	{
//		int xPos, yPos;
//
//		//always air
//
//		is >> xPos;
//		is >> yPos;
//
//		if (raceFight != NULL)
//		{
//			raceFight->numTargets++;
//
//			/*RaceFightTarget *enemy = new RaceFightTarget(this, Vector2i(xPos, yPos));
//
//			fullEnemyList.push_back(enemy);
//			enem = enemy;
//
//			enemyTree->Insert(enemy);*/
//		}
//		else
//		{
//			//ignore
//		}
//	}
//	else
//	{
//		assert(false && "not a valid type name: ");
//	}
//}
