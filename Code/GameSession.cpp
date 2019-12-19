//game session

#include "GameSession.h"
#include <fstream>
#include <iostream>
#include <assert.h>
#include "Actor.h"
#include "poly2tri/poly2tri.h"
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
#include "SoundManager.h"
#include "BarrierReactions.h"
#include "EnvEffects.h"
#include "SaveFile.h"
#include "MainMenu.h"
#include "GoalExplosion.h"
#include "Minimap.h"
#include "PauseMenu.h"
#include "Parallax.h"
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
#include "Rail.h"
#include "TerrainRender.h"
#include "Enemy.h"
#include "InputVisualizer.h"
#include "TopClouds.h"
#include "ScreenRecorder.h"
#include "ShardMenu.h"
#include "KeyMarker.h"
#include "ScoreDisplay.h"
#include "Fader.h"
#include "Enemy_Badger.h"
#include "Enemy_BasicEffect.h"
#include "Enemy_BasicTurret.h"
#include "ShipPickup.h"
#include "Enemy_Bat.h"
#include "Enemy_Blocker.h"
#include "Enemy_Booster.h"
#include "Enemy_Comboer.h"
#include "Enemy_StagBeetle.h"
#include "Enemy_Cactus.h"
#include "Nexus.h"
#include "MusicPlayer.h"
#include "Enemy_GravityModifier.h"
#include "Enemy_HealthFly.h"
#include "Enemy_GrindJuggler.h"
#include "Enemy_GroundedGrindJuggler.h"
#include "Enemy_Cheetah.h"
#include "Enemy_HungryComboer.h"
#include "Enemy_RelativeComboer.h"
//#include "Enemy_Cheetah.h"
//#include "Enemy_Copycat.h"
//#include "Enemy_CoralNanobots.h"
#include "Enemy_Crawler.h"
#include "Enemy_CurveTurret.h"
#include "Enemy_FootTrap.h"
//#include "Enemy_Ghost.h"
#include "Enemy_Goal.h"
#include "GroundTrigger.h"
#include "Enemy_Airdasher.h"
#include "AirTrigger.h"
#include "FlowerPod.h"
//#include "Enemy_Gorilla.h"
//#include "Enemy_GrowingTree.h"
//#include "Enemy_Jay.h"
//#include "Enemy_Mine.h"
//#include "Enemy_Narwhal.h"
//#include "Enemy_Overgrowth.h"
#include "Enemy_GravityFaller.h"
#include "Enemy_Owl.h"
#include "Enemy_Patroller.h"
#include "Enemy_RoadRunner.h"
#include "Enemy_BirdBoss.h"
#include "Enemy_PoisonFrog.h"
#include "Enemy_Pulser.h"
#include "Enemy_RaceFightTarget.h"
#include "Background.h"
//#include "Enemy_SecurityWeb.h"
#include "Enemy_Shard.h"
#include "Enemy_Shroom.h"
#include "Enemy_CrawlerQueen.h"
//#include "Enemy_Shark.h"
//#include "Enemy_Specter.h"
#include "Enemy_Spider.h"
#include "Enemy_Spring.h"
#include "Enemy_CurveLauncher.h"
//#include "Enemy_Swarm.h"
#include "Enemy_Turtle.h"
#include "HitboxManager.h"
#include "ShaderTester.h"
#include "ControlSettingsMenu.h"
#include "TouchGrass.h"
#include "Enemy_AirdashJuggler.h"
#include "Enemy_Juggler.h"
#include "Enemy_GravityJuggler.h"
#include "Enemy_BounceJuggler.h"
#include "Enemy_SplitComboer.h"

#include "ParticleEffects.h"
#include "Enemy_JugglerCatcher.h"
#include "Enemy_BounceFloater.h"

#include "Enemy_BounceBooster.h"
#include "Enemy_Teleporter.h"
#include "Enemy_Swarm.h"
#include "Enemy_Ghost.h"
#include "Enemy_GrowingTree.h"
#include "Enemy_Shark.h"
#include "Enemy_Specter.h"
#include "Enemy_Gorilla.h"
#include "Enemy_SwingLauncher.h"
#include "Wire.h"

#define TIMESTEP 1.0 / 60.0

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

//Ground - 1155aa
//
//Steep Floor - 4488cc
//
//Steep Ceiling - 55aaff
//
//Ceiling - 88ccff
//
//Wall - 6688ee


//Ground - 0d2d7d
//
//Steep Floor - 2244cc
//
//Wall - 0088cc
//
//Steep Ceiling - 2dcded
//
//Ceiling - 99ffff


#define COLOR_GROUND Color( 0x0d, 0x2d, 0x7d )
#define COLOR_STEEP_GROUND Color( 0x22, 0x44, 0xcc )
#define COLOR_STEEP_CEILING Color( 0x2d, 0xcd, 0xed )
#define COLOR_CEILING Color( 0x99, 0xff, 0xff )
#define COLOR_WALL Color( 0x00, 0x88, 0xcc )

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


Grass::Grass(GameSession *p_owner, Tileset *p_ts_grass, int p_tileIndex, 
	V2d &p_pos, TerrainPiece *p_poly, GrassType gType )
	:tileIndex(p_tileIndex), prev(NULL), next(NULL), visible(true),
	ts_grass(p_ts_grass), owner(p_owner), poly( p_poly ), pos( p_pos ), radius( 128 / 2.0 - 20 )
{
	grassType = gType;

	explosion.isCircle = true;
	explosion.rw = 64;
	explosion.rh = 64;
	explosion.type = CollisionBox::BoxType::Hit;

	explosion.globalPosition = pos;//(A + B + C + D) / 4.0;

	explodeFrame = 0;
	explodeLimit = 20;
	exploding = false;

	aabb.left = pos.x - radius;
	aabb.top = pos.y - radius;
	int right = pos.x + radius;
	int bot = pos.y + radius;

	aabb.width = right - aabb.left;
	aabb.height = bot - aabb.top;
}

void Grass::HandleQuery(QuadTreeCollider *qtc)
{
	qtc->HandleEntrant(this);
}

bool Grass::IsTouchingBox(const Rect<double> &r)
{
	return IsQuadTouchingCircle(V2d(r.left, r.top), V2d(r.left + r.width, r.top),
		V2d(r.left + r.width, r.top + r.height), V2d(r.left, r.top + r.height),
		pos, radius);
}

bool Grass::IsTouchingCircle(V2d &otherPos, double otherRad)
{
	double distSqr = lengthSqr(pos - otherPos);
	double radSqr = pow(radius + otherRad, 2);
	return distSqr <= radSqr;
}

void Grass::Reset()
{
	next = NULL;
	prev = NULL;
	exploding = false;
	visible = true;
}

void Grass::Update()
{
	if (visible && exploding)
	{
		if (explodeFrame == 0)
		{
			SetVisible(false);
		}
		
		++explodeFrame;
		if (explodeFrame == explodeLimit)
		{
			explodeFrame = 0;
			owner->RemoveGravityGrassFromExplodeList(this);
		}
	}
}

void Grass::SetVisible(bool p_visible)
{
	visible = p_visible;
	sf::VertexArray &gva = *(poly->grassVA);

	int size = ts_grass->tileWidth;
	if (visible)
	{
		gva[tileIndex * 4 + 0].position = Vector2f( pos.x - size, pos.y - size);
		gva[tileIndex * 4 + 1].position = Vector2f( pos.x + size, pos.y - size);
		gva[tileIndex * 4 + 2].position = Vector2f( pos.x + size, pos.y + size);
		gva[tileIndex * 4 + 3].position = Vector2f( pos.x - size, pos.y + size);
	}
	else
	{
		gva[tileIndex * 4 + 0].position = Vector2f(0, 0);
		gva[tileIndex * 4 + 1].position = Vector2f(0, 0);
		gva[tileIndex * 4 + 2].position = Vector2f(0, 0);
		gva[tileIndex * 4 + 3].position = Vector2f(0, 0);
	}
}




PoiInfo::PoiInfo( const std::string &pname, Vector2i &p )
{
	name = pname;
	pos.x = p.x;
	pos.y = p.y;
	edge = NULL;
	barrier = NULL;
}

PoiInfo::PoiInfo( const std::string &pname, Edge *e, double q )
{
	name = pname;
	edge = e;
	edgeQuantity = q;
	pos = edge->GetPoint( edgeQuantity );
	barrier = NULL;
}

Barrier::Barrier( GameSession *p_owner, PoiInfo *p_poi, bool p_x, int p_pos, bool posOp,
	BarrierCallback *cb ): poi( p_poi )
{
	owner = p_owner;
	p_poi->barrier = this;
	callback = cb;
	x = p_x;
	pos = p_pos;
	triggered = false;
	positiveOpen = posOp;
}

bool Barrier::Update( Actor *player )
{
	if( triggered )
		return false;

	V2d playerPos = player->position;

	if( x )
	{
		if( positiveOpen ) //player starts right
		{
			if( playerPos.x < pos )
			{
				triggered = true;
			}
		}
		else //starts left
		{
			if( playerPos.x > pos )
			{
				triggered = true;
			}
		}
	}
	else
	{
		if( positiveOpen ) // player starts below
		{
			if( playerPos.y < pos )
			{
				triggered = true;
			}
		}
		else //player starts above
		{
			if( playerPos.y > pos )
			{
				triggered = true;
			}
		}
	}

	return triggered;
}

void Barrier::SetPositive()
{
	//should use a parameter eventually but for now just using this
	if( x )
	{
		if( owner->GetPlayer( 0 )->position.x - pos > 0 )
		{
			positiveOpen = true;
		}
		else
			positiveOpen = false;
	}
	else
	{
		if( owner->GetPlayer( 0 )->position.y - pos > 0 )
		{
			positiveOpen = true;
		}
		else
		{
			positiveOpen = false;
		}
	}
}



GameSession::GameSession(SaveFile *sf, MainMenu *p_mainMenu, 
	const boost::filesystem::path &p_filePath )
	:saveFile( sf ),
	cloud0( sf::Quads, 3 * 4 ), cloud1( sf::Quads, 3 * 4 ),
	cloudBot0( sf::Quads, 3 * 4 ), cloudBot1( sf::Quads, 3 * 4 ), fileName( p_filePath.string() ),
	filePath( p_filePath ), eHitParamsMan( NULL ), drain(true )
{	
	mainMenu = p_mainMenu;
	cam.owner = this;

	Init();
}

void GameSession::Cleanup()
{
	TerrainRender::CleanupLayers();

	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		delete (*it);
	}

	for (auto it = allVA.begin(); it != allVA.end(); ++it)
	{
		delete (*it);
	}

	for (auto it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		delete (*it);
	}

	if (goalEnergyFlowVA != NULL)
	{
		delete goalEnergyFlowVA;
	}

	for (auto it = globalBorderEdges.begin(); it != globalBorderEdges.end(); ++it)
	{
		delete (*it);
	}

	for (auto it = decorLayerMap.begin(); it != decorLayerMap.end(); ++it)
	{
		delete (*it).second;
	}

	if (background != NULL)
	{
		delete background;
	}
	

	for (auto it = scrollingBackgrounds.begin(); it != scrollingBackgrounds.end(); ++it)
	{
		delete (*it);
	}

	if (points != NULL)
	{
		delete[] points;
	}

	for (auto it = allEffectList.begin(); it != allEffectList.end(); ++it)
	{
		delete (*it);
	}

	for (auto it = allEnvPlants.begin(); it != allEnvPlants.end(); ++it)
	{
		delete (*it);
	}

	if (hitboxManager != NULL)
	{
		delete hitboxManager;
	}

	/*if (inversePoly != NULL)
	{
		delete inversePoly;
	}*/

	if (recGhost != NULL)
	{
		delete recGhost;
	}

	if (mini != NULL)
	{
		delete mini;
	}

	if (shardsCapturedField != NULL)
	{
		delete shardsCapturedField;
	}

	if (shardPop != NULL)
	{
		delete shardPop;
	}

	if (adventureHUD != NULL)
	{
		delete adventureHUD;
	}

	if (eHitParamsMan != NULL)
	{
		delete eHitParamsMan;
		eHitParamsMan = NULL;
	}

	if (absorbParticles != NULL)
	{
		delete absorbParticles;
		absorbParticles = NULL;
	}

	if (absorbDarkParticles != NULL)
	{
		delete absorbDarkParticles;
		absorbDarkParticles = NULL;
	}

	if (absorbShardParticles != NULL)
	{
		delete absorbShardParticles;
		absorbShardParticles = NULL;
	}

	if (testPar != NULL)
	{
		delete testPar;
		testPar = NULL;
	}

	if (ts_polyShaders != NULL)
	{
		delete [] ts_polyShaders;
		ts_polyShaders = NULL;
	}

	if (polyShaders != NULL)
	{
		delete [] polyShaders;
		polyShaders = NULL;
	}

	if (terrainDecorInfos != NULL)
	{
		for (int i = 0; i < numPolyTypes; ++i)
		{
			if (terrainDecorInfos[i] != NULL)
			{
				delete terrainDecorInfos[i];
			}
		}
		delete[] terrainDecorInfos;
		terrainDecorInfos = NULL;
	}

	if (goalPulse != NULL)
	{
		delete goalPulse;
		goalPulse = NULL;
	}

	for (int i = 0; i < 4; ++i)
	{
		if (players[i] != NULL)
		{
			delete players[i];
			players[i] = NULL;
		}
	}

	if (soundManager != NULL)
	{
		delete soundManager;
		soundManager = NULL;
	}

	if (topClouds != NULL)
	{
		delete topClouds;
		topClouds = NULL;
	}

	if (keyMarker != NULL)
	{
		delete keyMarker;
		keyMarker = NULL;
	}


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

	/*if (stormCeilingInfo != NULL)
	{
		stormCeilingInfo = NULL;
	}*/
	//if (crawlerReverseTree != NULL)
	//	delete crawlerReverseTree;

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

	if (gateTree != NULL)
	{
		delete gateTree;
		gateTree = NULL;
	}

	if (grassTree != NULL)
	{
		delete grassTree;
		grassTree = NULL;
	}

	if (borderTree != NULL)
	{
		delete borderTree;
		borderTree = NULL;
	}

	if (enemyTree != NULL)
	{
		delete enemyTree;
		enemyTree = NULL;
	}

	if (terrainTree != NULL)
	{
		delete terrainTree;
		terrainTree = NULL;
	}

	if (terrainBGTree != NULL)
	{
		delete terrainBGTree;
		terrainBGTree = NULL;
	}

	if (staticItemTree!= NULL)
	{
		delete staticItemTree;
		staticItemTree = NULL;
	}

	if (railDrawTree != NULL)
	{
		delete railDrawTree;
		railDrawTree = NULL;
	}

	if (railEdgeTree != NULL)
	{
		delete railEdgeTree;
		railEdgeTree = NULL;
	}

	if (scoreDisplay != NULL)
	{
		delete scoreDisplay;
		scoreDisplay = NULL;
	}

	if (soundNodeList != NULL)
	{
		delete soundNodeList;
		soundNodeList = NULL;
	}

	if (pauseSoundNodeList != NULL)
	{
		delete pauseSoundNodeList;
		pauseSoundNodeList = NULL;
	}

	if (gates != NULL)
	{
		for (int i = 0; i < numGates; ++i)
		{
			delete gates[i];
		}
		delete[] gates;

		gates = NULL;
	}

	if (edges != NULL)
	{
		for (int i = 0; i < mh->numVertices; ++i)
		{
			delete edges[i];
		}
		delete [] edges;

		edges = NULL;
	}

	/*for (list<VertexArray*>::iterator it = polygons.begin(); it != polygons.end(); ++it)
	{
		delete (*it);
	}
	polygons.clear();*/

	if (mh != NULL)
	{
		delete mh;
		mh = NULL;
	}

	if (activeItemTree != NULL)
	{
		delete activeItemTree;
		activeItemTree = NULL;
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

	if (numTotalFlies > 0 && healthFlyVA != NULL )
	{
		delete[] healthFlyVA;
	}


	for (auto it = decorBetween.begin(); it != decorBetween.end(); ++it)
	{
		delete (*it);
	}

	for (auto it = fullAirTriggerList.begin(); it != fullAirTriggerList.end(); ++it)
	{
		delete (*it);
	}

	if (bigBulletVA != NULL)
	{
		delete bigBulletVA;
	}

	for (auto it = poiMap.begin(); it != poiMap.end(); ++it)
	{
		delete (*it).second;
	}

	if (shipExitSeq != NULL)
	{
		delete shipExitSeq;
	}
	
}

GameSession::~GameSession()
{
	Cleanup();
}

//should only be used to assign a variable. don't use at runtime
Tileset * GameSession::GetTileset( const std::string & s, int tileWidth, int tileHeight,  int altColorIndex )
{
	return tm.GetTileset( s, tileWidth, tileHeight, altColorIndex );
	//make sure to set up tileset here
}


Tileset * GameSession::GetTileset( const std::string & s, int tileWidth, int tileHeight, int altColorIndex, int numColorChanges,
	sf::Color *startColorBuf, sf::Color *endColorBuf)
{
	return tm.GetTileset( s, tileWidth, tileHeight, altColorIndex, numColorChanges, startColorBuf, endColorBuf );
}

Tileset * GameSession::GetTileset(const std::string & s, int tileWidth, int tileHeight, KinSkin *skin )
{
	if (skin != NULL)
	{
		return tm.GetTileset(s, tileWidth, tileHeight, skin->index, skin->numChanges, skin->startColors, skin->endColors);
	}
	else
	{
		return tm.GetTileset(s, tileWidth, tileHeight );
	}
}

void GameSession::UpdateEnemiesPrePhysics()
{
	Actor *player = GetPlayer( 0 );
	if( player->action == Actor::INTRO || player->action == Actor::SPAWNWAIT )
	{
		return;
	}

	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
		current->UpdatePrePhysics();
		current = current->next;
	}

	
}

void GameSession::UpdateEnemiesPhysics()
{
	Actor *p = NULL;
	Actor *player = GetPlayer( 0 );
	if( player->action == Actor::INTRO || player->action == Actor::SPAWNWAIT )
	{
		return;
	}

	for( int i = 0; i < 4; ++i )
	{
		p = GetPlayer( i );
		if( p != NULL )
			p->physicsOver = false;
	}


	for( substep = 0; substep < NUM_MAX_STEPS; ++substep )
	{
		/*for( list<MovingTerrain*>::iterator it = movingPlats.begin(); it != movingPlats.end(); ++it )
		{
			(*it)->UpdatePhysics();
		}*/
		for( int i = 0; i < 4; ++i )
		{
			p = GetPlayer( i );
			if (p != NULL)
			{
				if( substep == 0 || p->highAccuracyHitboxes )
					p->UpdatePhysics();
			}
		}

		Enemy *current = activeEnemyList;
		while( current != NULL )
		{
			current->UpdatePhysics( substep );
			current = current->next;
		}
	}
}

void GameSession::UpdateEnemiesPostPhysics()
{
	Actor *player = GetPlayer( 0 );
	if( player->action == Actor::INTRO || player->action == Actor::SPAWNWAIT )
	{
		return;
	}

	int keyLength = 30;//16 * 5;
	keyFrame = totalGameFrames % keyLength;
	

	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
		Enemy *temp = current->next; //need this in case enemy is removed during its update

		current->UpdatePostPhysics();
		
		if( current->keyShader != NULL )
		{
			float halftot = keyLength / 2;
			float fac;
			if( keyFrame < keyLength / 2 )
			{
				fac = keyFrame / (halftot-1);
			}
			else
			{
				fac = 1.f - ( keyFrame - halftot ) / (halftot-1);
			}
			//cout << "fac: " << fac << endl;
			current->keyShader->setUniform( "prop", fac );
		}

		current = temp;
	}

	/*for (list<Enemy*>::iterator it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		(*it)->CheckedZoneUpdate(FloatRect(screenRect));
	}*/
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

void GameSession::UpdateInput()
{
	vector<GCC::GCController> controllers;
	if( mainMenu->gccDriverEnabled ) 
		controllers = mainMenu->gccDriver->getState();
	for (int i = 0; i < 4; ++i)
	{
		GetPrevInput(i) = GetCurrInput(i);
		GetPrevInputUnfiltered(i) = GetCurrInputUnfiltered(i);
		GameController &con = GetController(i);
		if(mainMenu->gccDriverEnabled )
			con.gcController = controllers[i];
		con.UpdateState();
		GetCurrInput(i) = con.GetState();
		GetCurrInputUnfiltered(i) = con.GetUnfilteredState();
	}
}

void GameSession::UpdateEffects()
{
	Enemy *curr;
	Enemy *next;
	for( int i = 0; i < EffectLayer::Count; ++i )
	{
		curr = effectLists[i];

		while( curr != NULL )
		{
			next = curr->next;

			curr->UpdatePrePhysics();
			if( !curr->dead )
				curr->UpdatePostPhysics();
			curr = next;
		}
	}
}

void GameSession::UpdateEnemiesDraw()
{

	Enemy *current = activeEnemyList;
	while( current != NULL )
	{
	//	cout << "draw" << endl;
		if( current->type != EnemyType::EN_BASICEFFECT && ( pauseFrames < 2 || current->receivedHit == NULL ) )
		{
			current->Draw( preScreenTex );
		}
		current = current->next;
	}

	for (list<Enemy*>::iterator it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		(*it)->CheckedZoneDraw(preScreenTex, FloatRect(screenRect));
	}

	DrawHealthFlies(preScreenTex);
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

void GameSession::DrawEffects( EffectLayer layer )
{
	sf::View oldView = preScreenTex->getView();
	if (layer == UI_FRONT)
	{
		preScreenTex->setView(uiView);
	}
	Enemy *currentEnem = effectLists[layer];
	while( currentEnem != NULL )
	{
		currentEnem->Draw( preScreenTex );	
		currentEnem = currentEnem->next;
	}

	if (layer == UI_FRONT)
	{
		preScreenTex->setView(oldView);
	}
}

void GameSession::Test( Edge *e )
{
	cout << "testing" << endl;
}

void GameSession::AddEffect( EffectLayer layer, Enemy *e )
{
	Enemy *& fxList = effectLists[layer];
	if( fxList != NULL )
	{
		fxList->prev = e;
		e->next = fxList;
		fxList = e;
	}
	else
	{
		fxList = e;
	}

	/*if( player->record > 0 )
	{
		e->spawnedByClone = true;
	}*/
}

void GameSession::AddEnemy( Enemy *e )
{
	//do not spawn shards that are already captured in the file.
	if (e->type == EnemyType::EN_SHARD)
	{
		Shard *sh = (Shard*)e;
		if ( saveFile != NULL && saveFile->ShardIsCaptured(sh->shardType))
		{
			return;
		}
		//see if the shard is spawnable!
	}
	
	//cout << "spawning enemy! of type: " << e->type << endl;
	if (e->spawned)
	{
		assert(e->spawned == false);
	}
	
	e->Reset();
	e->spawned = true;
	e->Init();

	//^^note remove this later
	//debugging only

	//debug test
	Enemy *c = activeEnemyList;
	while (c != NULL)
	{
		assert(c != e);
		c = c->next;
	}

	//assert( e->spawned );
//	cout << "ADD ENEMY:" << (int)e << ", type: " << (int)e->type << endl;
	if( e->type == EnemyType::EN_BOSS_BIRD )
	{
		//probably will not actually use this and will use a separate spacial trigger or a gate

		//cutPlayerInput = true;
	}
	//if( e->type == Enemy::BASICTURRET )
	//{
	//	cout << "ADDING BASIC TURRET NOW: " << endl;
//	}
	//cout << "adding enemy: " << e->type << endl;

	if (activeEnemyList == NULL)
	{
		activeEnemyList = e;
		activeEnemyListTail = e;
	}
	else
	{
		activeEnemyListTail->next = e;
		e->prev = activeEnemyListTail;
		activeEnemyListTail = e;
	}
}

void GameSession::RemoveEnemy( Enemy *e )
{
	if (activeEnemyList == NULL)
	{
		return;
	}
	//cout << "removing enemy: " << e << endl;
	//assert( activeEnemyList != NULL );
	Enemy *prev = e->prev;
	Enemy *next = e->next;

	if( prev == NULL && next == NULL )
	{
		//cout << "etype: " << (int)e->type << ", activelist: " << (int)activeEnemyList->type << endl;
		activeEnemyList = NULL;
		activeEnemyListTail = NULL;
		
	}
	else
	{
		if (e == activeEnemyListTail)
		{
			assert(prev != NULL);

			prev->next = NULL;

			activeEnemyListTail = prev;
		}
		else if (e == activeEnemyList)
		{
			assert(next != NULL);

			next->prev = NULL;
			activeEnemyList = next;
		}
		else
		{
			if (next != NULL)
			{
				next->prev = prev;
			}

			if (prev != NULL)
			{
				prev->next = next;
			}
		}



		/*if( e == activeEnemyList )
		{
			assert( next != NULL );
			
			next->prev = NULL;
			
			activeEnemyList = next;
		}
		else
		{
			if( prev != NULL )
			{
				prev->next = next;
			}

			if( next != NULL )
			{
				next->prev = prev;
			}
		}*/
		
	}

	//if( e->type != e->BASICEFFECT )
	//{
	//	/*if( e->hasMonitor )
	//	{
	//		cout << "adding monitor!" << endl;
	//		e->monitor->position = e->position;
	//		AddEnemy( e->monitor );
	//	}*/

	//	cout << "adding an inactive enemy!" << endl;
	//	//cout << "secret count: " << CountActiveEnemies() << endl;
	//	if( inactiveEnemyList == NULL )
	//	{
	//		inactiveEnemyList = e;
	//		e->next = NULL;
	//		e->prev = NULL;
	//	}
	//	else
	//	{
	//		//cout << "creating more dead clone enemies" << endl;
	//		e->next = inactiveEnemyList;
	//		inactiveEnemyList->prev = e;
	//		inactiveEnemyList = e;
	//	}
	//}

	//might need to give enemies a second next/prev pair for clone power?
	//totally does >.> CLONE POWER
	//if( player->record > 0 )
	//{
	//	if( cloneInactiveEnemyList == NULL )
	//	{
	//		cloneInactiveEnemyList = e;
	//		e->next = NULL;
	//		e->prev = NULL;
	//		//cout << "creating first dead clone enemy" << endl;

	//		/*int listSize = 0;
	//		Enemy *ba = cloneInactiveEnemyList;
	//		while( ba != NULL )
	//		{
	//			listSize++;
	//			ba = ba->next;
	//		}

	//		cout << "size of dead list after first add: " << listSize << endl;*/
	//	}
	//	else
	//	{
	//		//cout << "creating more dead clone enemies" << endl;
	//		e->next = cloneInactiveEnemyList;
	//		cloneInactiveEnemyList->prev = e;
	//		cloneInactiveEnemyList = e;
	//	}
	//}
	

//	cout << "number of enemies is now: " << CountActiveEnemies() << endl;


	/*if( inactiveEnemyList != NULL )
	{
		inactiveEnemyList->next = e;
	}
	else
	{
		inactiveEnemyList = e;
	}*/
	
	
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

bool GameSession::LoadMovingPlats( ifstream &is, map<int, int> &polyIndex )
{
	//remove this entirely later
	int numMovingPlats;
	is >> numMovingPlats;
	//for( int i = 0; i < numMovingPlats; ++i )
	//{
	//	//dont forget this!
	//	//int matType;
	//	//is >> matType;

	//	//matSet.insert( matType );
	//	//string matStr;
	//	//is >> matStr;


	//	int polyPoints;
	//	is >> polyPoints;

	//	list<Vector2i> poly;

	//	for( int i = 0; i < polyPoints; ++i )
	//	{
	//		int px, py;
	//		is >> px;
	//		is >> py;
	//		
	//		poly.push_back( Vector2i( px, py ) );
	//	}


	//		
	//	list<Vector2i>::iterator it = poly.begin();
	//	int left = (*it).x;
	//	int right = (*it).x;
	//	int top = (*it).y;
	//	int bottom = (*it).y;
	//		
	//	for( ;it != poly.end(); ++it )
	//	{
	//		if( (*it).x < left )
	//			left = (*it).x;

	//		if( (*it).x > right )
	//			right = (*it).x;

	//		if( (*it).y < top )
	//			top = (*it).y;

	//		if( (*it).y > bottom )
	//			bottom = (*it).y;
	//	}


	//	//might need to round for perfect accuracy here
	//	Vector2i center( (left + right ) / 2, (top + bottom) / 2 );

	//	for( it = poly.begin(); it != poly.end(); ++it )
	//	{
	//		(*it).x -= center.x;
	//		(*it).y -= center.y;
	//	}

	//	int pathPoints;
	//	is >> pathPoints;

	//	list<Vector2i> path;

	//	for( int i = 0; i < pathPoints; ++i )
	//	{
	//		int x,y;
	//		is >> x;
	//		is >> y;
	//		path.push_back( Vector2i( x, y ) );
	//	}

	//		
	//	MovingTerrain *mt = new MovingTerrain( this, center, path, poly, false, 5 );
	//	movingPlats.push_back( mt );
	//}

	return true;
}

bool GameSession::LoadBGPlats( ifstream &is, map<int, int> &polyIndex )
{
	
	int bgPlatformNum0;
	is >> bgPlatformNum0;
	for( int i = 0; i < bgPlatformNum0; ++i )
	{
		//layer is 1

		int matWorld;
		int matVariation;
		
		is >> matWorld;
		is >> matVariation;

		matSet.insert( pair<int,int>(matWorld,matVariation) );

		int polyPoints;
		is >> polyPoints;
		
		list<Vector2i> poly;
		for( int i = 0; i < polyPoints; ++i )
		{
			int x,y, special;
			is >> x;
			is >> y;
			poly.push_back( Vector2i( x, y ) );
		}

		TerrainPiece * tPiece = new TerrainPiece( this );

		list<Vector2i>::iterator it = poly.begin();
		list<Edge*> realEdges;
		double left, right, top, bottom;
		for( ; it != poly.end(); ++it )
		{
			Edge *ee = new Edge();
			ee->poly = tPiece;
  			//edges[currentEdgeIndex + i] = ee;
			ee->v0 = V2d( (*it).x, (*it).y );
			list<Vector2i>::iterator temp = it;
			++temp;
			if( temp == poly.end() )
			{
				ee->v1 = V2d( poly.front().x, poly.front().y );
			}
			else
			{
				ee->v1 = V2d( (*temp).x, (*temp).y );
			}
			
			realEdges.push_back( ee );
			terrainBGTree->Insert( ee );

			double localLeft = min( ee->v0.x, ee->v1.x );
			double localRight = max( ee->v0.x, ee->v1.x );
			double localTop = min( ee->v0.y, ee->v1.y );
			double localBottom = max( ee->v0.y, ee->v1.y ); 
			if( i == 0 )
			{
				left = localLeft;
				right = localRight;
				top = localTop;
				bottom = localBottom;
			}
			else
			{
				left = min( left, localLeft );
				right = max( right, localRight );
				top = min( top, localTop);
				bottom = max( bottom, localBottom);
			}
		}


		for( list<Edge*>::iterator eit = realEdges.begin(); eit != realEdges.end(); ++eit )
		{
			Edge * ee = (*eit);//edges[i + currentEdgeIndex];

			Edge *prev;
			if( eit == realEdges.begin() )
			{
				prev = realEdges.back();
			}
			else
			{
				eit--;
				prev = (*eit);
				eit++;
			}

			Edge *next;
			eit++;
			if( eit == realEdges.end() )
			{
				next = realEdges.front();
				eit--;
			}
			else
			{
				next = (*eit);
				eit--;
			}

			ee->edge0 = prev;
			ee->edge1 = next;
		}


		vector<p2t::Point*> polyline;
		//for( int i = 0; i < polyPoints; ++i )
		for( list<Vector2i>::iterator pit = poly.begin(); pit != poly.end(); ++pit )
		{
			polyline.push_back( new p2t::Point( (*pit).x, (*pit).y ) );
		}

		p2t::CDT * cdt = new p2t::CDT( polyline );
	
		cdt->Triangulate();
		vector<p2t::Triangle*> tris;
		tris = cdt->GetTriangles();
			
		va = new VertexArray( sf::Triangles , tris.size() * 3 );
		VertexArray & v = *va;
		Color testColor( 0x75, 0x70, 0x90 );
		testColor = Color::White;
		Vector2f topLeft( left, top );
		cout << "topLeft: " << topLeft.x << ", " << topLeft.y << endl;
		for( int i = 0; i < tris.size(); ++i )
		{	
			p2t::Point *p = tris[i]->GetPoint( 0 );	
			p2t::Point *p1 = tris[i]->GetPoint( 1 );	
			p2t::Point *p2 = tris[i]->GetPoint( 2 );	
			v[i*3] = Vertex( Vector2f( p->x, p->y ), testColor );
			v[i*3 + 1] = Vertex( Vector2f( p1->x, p1->y ), testColor );
			v[i*3 + 2] = Vertex( Vector2f( p2->x, p2->y ), testColor );

			Vector2f pp0 = (v[i*3].position - topLeft);
			Vector2f pp1 = (v[i*3+1].position - topLeft);
			Vector2f pp2 = (v[i*3+2].position - topLeft);
			if( i == 0 )
			{
				cout << "pos0: " << pp0.x << ", " << pp0.y << endl;
				cout << "pos1: " << pp1.x << ", " << pp1.y << endl;
				cout << "pos2: " << pp2.x << ", " << pp2.y << endl;
			}
			v[i*3].texCoords = pp0;
			v[i*3+1].texCoords = pp1;
			v[i*3+2].texCoords = pp2;
		}

		polygons.push_back( va );

		VertexArray *polygonVA = va;

		//ground, slope, steep, wall

		//ground

		stringstream ss;

		ss << "Borders/bor_" << matWorld + 1 << "_";

		if( matVariation < 10 )
		{
			ss << "0" << matVariation + 1;
		}
		else
		{
			ss << matVariation + 1;
		}

		ss << ".png";
		
		Tileset *ts_border = GetTileset( ss.str(), 8, 64 );//128 );
		VertexArray *groundVA = SetupBorderQuads( 1, realEdges.front(), ts_border,
			&GameSession::IsFlatGround );
		VertexArray *slopeVA = SetupBorderQuads( 1, realEdges.front(), ts_border,
			&GameSession::IsSlopedGround );
		VertexArray *steepVA = SetupBorderQuads( 1, realEdges.front(), ts_border,
			&GameSession::IsSteepGround );
		VertexArray *wallVA = SetupBorderQuads( 1, realEdges.front(), ts_border,
			&GameSession::IsWall );

		bool first = true;

		
		tPiece->next = NULL;
		//testva->va = va;
		tPiece->aabb.left = left;
		tPiece->aabb.top = top;
		tPiece->aabb.width = right - left;
		tPiece->aabb.height = bottom - top;
		tPiece->terrainVA = polygonVA;
		tPiece->grassVA = NULL;//grassVA;

		tPiece->numPoints = polyPoints;

		tPiece->ts_border = ts_border;
		tPiece->groundva = groundVA;
		tPiece->slopeva = slopeVA;
		tPiece->steepva = steepVA;
		tPiece->wallva = wallVA;

		
		
			
		//cout << "before insert border: " << insertCount << endl;
		//if( !inverse )
		//{
			borderTree->Insert(tPiece);
			allVA.push_back(tPiece);
	//	}

		//cout << "after insert border: " << insertCount << endl;
		//insertCount++;
			

		
		delete cdt;
		for( int i = 0; i < polyPoints; ++i )
		{
			delete polyline[i];
		//	delete tris[i];
		}
		
		//no grass for now
	}

	return true;
}

bool GameSession::LoadGates( ifstream &is, map<int, int> &polyIndex )
{
	is >> numGates;
	gates = new Gate*[numGates];

	

	for( int i = 0; i < numGates; ++i )
	{
		int gType;
		int poly0Index, vertexIndex0, poly1Index, vertexIndex1;
		int numKeysRequired;
		string behindyouStr;

		is >> gType;
		//is >> numKeysRequired; 
		is >> poly0Index;
		is >> vertexIndex0;
		is >> poly1Index;
		is >> vertexIndex1;

		Edge *edge0 = edges[polyIndex[poly0Index] + vertexIndex0];
		Edge *edge1 = edges[polyIndex[poly1Index] + vertexIndex1];

		V2d point0 = edge0->v0;
		V2d point1 = edge1->v0;

		Gate::GateType gateType = (Gate::GateType)gType;

		

		//if( gateType == Gate::CRITICAL )
		//{
		//	cout << "MAKING NEW CRITICAL" << endl;
		//	Critical *crit = new Critical( point0, point1 );
		//	//wastes space for the gates already made but idk what to change. make it a new system?
		//	//lets try it for now lol

		//	//the extra pointers just get stuffed at the end
		//	--numGates;
		//	--i;
		//	//will have to differentiate later for more items. but not for now!
		//	itemTree->Insert( crit );
		//	
		//	continue;
		//}

		Gate * gate = new Gate( this, gateType);

		if (gType == Gate::SHARD)
		{
			int sw, si;
			is >> sw;
			is >> si;
			gate->SetShard(sw, si);
		}

		if (!visibleTerrain[poly0Index] || !visibleTerrain[poly1Index])
		{
			gate->visible = false;
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

		//gate->v0 = point0;
		//gate->v1 = point1;
			
			
		gates[i] = gate;

		gate->SetLocked( true );

		gate->UpdateLine();

		terrainTree->Insert( gate->edgeA );
		terrainTree->Insert( gate->edgeB );

		cout << "inserting gate: " << gate->edgeA << endl;
		gateTree->Insert( gate );

		//gateMap[gate->edgeA] = gate;
		//gateMap[gate->edgeB] = gate;
	}

	return true;
}

bool GameSession::LoadRails(ifstream &is)
{
	list<Vector2i> globalPath;

	int numRails;
	is >> numRails;
	for (int i = 0; i < numRails; ++i)
	{
		int numRailPoints;
		is >> numRailPoints;

		for (int j = 0; j < numRailPoints; ++j)
		{
			int x, y;
			is >> x;
			is >> y;
			globalPath.push_back(Vector2i(x, y));
		}

		Rail *r = new Rail(this, globalPath.front(), globalPath, false, false, 1);
		globalPath.clear();
		++totalRails;
	}

	return true;
}

bool GameSession::LoadEnemies( ifstream &is, map<int, int> &polyIndex )
{
	totalNumberBullets = 0;
	int shardsLoadedCounter = 0;

	numTotalFlies = 0;
	ts_healthFly = NULL;
	healthFlyVA = NULL;

	int numGroups;
	is >> numGroups;
	for( int i = 0; i < numGroups; ++i )
	{
		string gName;
		is >> gName;
		int numActors;
		is >> numActors;

		for( int j = 0; j < numActors; ++j )
		{
			LoadEnemy(is, polyIndex);
		}
	}

	if (numTotalFlies > 0)
	{
		ts_healthFly = GetTileset("Enemies/healthfly_64x64.png", 64, 64);
		healthFlyVA = new Vertex[numTotalFlies * 4];

		for (auto it = allFlies.begin(); it != allFlies.end(); ++it)
		{
			(*it)->va = healthFlyVA;
			(*it)->ResetEnemy();
		}
	}


	if( totalNumberBullets > 0 )
	{
		bigBulletVA = new VertexArray( sf::Quads, totalNumberBullets * 4 );
		VertexArray &bva = *bigBulletVA;
		for( int i = 0; i < totalNumberBullets * 4; ++i )
		{
			bva[i].position = Vector2f( 0, 0 );
		}
		ts_basicBullets = GetTileset( "Enemies/bullet_64x64.png", 64, 64 );
	}
	else
	{
		ts_basicBullets = NULL;
		bigBulletVA = NULL;
	}

	if( raceFight != NULL )
	{
		raceFight->Init();
	}

	for (list<Enemy*>::iterator it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		(*it)->SetZoneSpritePosition();
	}

	return true;
}

void GameSession::LoadEnemy(std::ifstream &is,
	map<int, int>&polyIndex)
{
	{
		string typeName;
		is >> typeName;

		Enemy *enem = NULL;

		if (typeName == "goal" || typeName == "greengoal")
		{
			//always grounded

			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int w = 0;
			if (typeName == "greengoal")
			{
				w = 1;
			}

			//cout << "polyIndex: " << polyIndex[terrainIndex] << ", tindex: " << terrainIndex << endl;
			Goal *enemy = new Goal(this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, w);
			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);

			hasGoal = true;
			goalPos = enemy->position;

			V2d gPos = enemy->ground->GetPoint(enemy->edgeQuantity);
			V2d norm = enemy->ground->Normal();
			double nodeHeight = 104;
			goalNodePos = gPos + norm * nodeHeight;
			float space = 78.f;
			goalNodePosFinal = V2d(goalNodePos.x, goalNodePos.y - space);
			cout << "setting goalPos: " << goalPos.x << ", " << goalPos.y << endl;
		}
		else if (typeName == "poi")
		{
			string air;
			string pname;
			PoiInfo *pi = NULL;

			is >> air;

			if (air == "+air")
			{
				Vector2i pos;
				is >> pos.x;
				is >> pos.y;


				is >> pname;

				string barStr;
				is >> barStr;

				int hasCamProperties;
				is >> hasCamProperties;

				pi = new PoiInfo(pname, pos);

				pi->hasCameraProperties = hasCamProperties;

				if (pi->hasCameraProperties)
				{
					float camZoom;
					is >> camZoom;

					pi->cameraZoom = camZoom;
				}
				else
				{
				}



				if (barStr == "-")
				{
				}
				else if (barStr == "x")
				{
					/*					bleft = owner->poiMap["coyfightleft"]->barrier;
					bright = owner->poiMap["coyfightright"]->barrier;
					btop = owner->poiMap["coyfighttop"]->barrier;
					bbot = owner->poiMap["coyfightbot"]->barrier;*/

					Barrier *b = new Barrier(this, pi, true, pos.x, (GetPlayer(0)->position.x > pos.x), NULL);
					string na = b->poi->name;
					/*if( na == "coyfightleft"
					|| na == "coyfightright"
					|| na == "coyfighttop"
					|| na == "coyfightbot" )
					b->triggered = true;*/
					barriers.push_back(b);

				}
				else if (barStr == "y")
				{
					Barrier *b = new Barrier(this, pi, false, pos.y, (GetPlayer(0)->position.y > pos.y), NULL);
					//b->triggered = true;
					barriers.push_back(b);
				}
				else
				{
					assert(0);
				}

				//if( barStr != "-" )
				//{
				//	if( fileName == "Resources/Maps/poitest.brknk" )
				//	{
				//		if( pname == "birdfighttrigger" )
				//		{
				//			/*BirdMeetingCallback *bmc = new BirdMeetingCallback( this );
				//			barriers.back()->callback = bmc;*/
				//		}
				//	}
				//}
			}
			else if (air == "-air")
			{
				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				is >> pname;

				string barStr;
				is >> barStr;

				Edge *e = edges[polyIndex[terrainIndex] + edgeIndex];

				V2d p = e->GetPoint(edgeQuantity);

				pi = new PoiInfo(pname, e,
					edgeQuantity);

				if (barStr == "-")
				{
				}
				else if (barStr == "x")
				{
					barriers.push_back(new Barrier(this, pi, true, floor(p.x + .5), (GetPlayer(0)->position.x > p.x), NULL));
				}
				else if (barStr == "y")
				{
					barriers.push_back(new Barrier(this, pi, false, floor(p.y + .5), (GetPlayer(0)->position.y > p.y), NULL));
				}
				else
				{
					assert(0);
				}


				//edges[polyIndex[terrainIndex] + edgeIndex]					
			}
			else
			{
				cout << "air failure: " << air << endl;
				assert(0);
			}

			poiMap[pname] = pi;
			//poiMap
		}
		else if (typeName == "shard")
		{
			int xPos, yPos;

			//always air

			is >> xPos;
			is >> yPos;

			int w;
			is >> w;

			int localIndex;
			is >> localIndex;
			//string shardStr;
			//is >> shardStr;

			Shard *enemy = new Shard(this, Vector2i(xPos, yPos), w, localIndex);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "healthfly")
		{
			int xPos, yPos;

			//always air

			is >> xPos;
			is >> yPos;

			int level;
			is >> level;

			HealthFly *enemy = new HealthFly(this, Vector2i(xPos, yPos), level, numTotalFlies);

			allFlies.push_back(enemy);
			numTotalFlies++;
			activeItemTree->Insert(enemy);
			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "blocker" || typeName == "greenblocker")
		{
			int xPos, yPos;

			//always air


			is >> xPos;
			is >> yPos;

			int pathLength;
			list<Vector2i> localPath;
			Enemy::ReadPath(is, pathLength, localPath);

			int bType;
			is >> bType;

			if (typeName == "blocker")
			{
				bType = BlockerChain::BLUE;
			}
			else if (typeName == "greenblocker")
			{
				bType = BlockerChain::GREEN;
			}

			int armored;
			is >> armored;

			int spacing;
			is >> spacing;

			int level;
			is >> level;

			BlockerChain *enemy = new BlockerChain(this, Vector2i(xPos, yPos), localPath, bType, armored, spacing, level);

			fullEnemyList.push_back(enemy);

			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "comboer")
		{

			int xPos, yPos;

			//always air
			is >> xPos;
			is >> yPos;

			int pathLength;
			list<Vector2i> localPath;

			Enemy::ReadPath(is, pathLength, localPath);

			bool loop;
			Enemy::ReadBool(is, loop);

			int level;
			is >> level;

			Comboer *enemy = new Comboer(this, Vector2i(xPos, yPos), localPath, loop, level);


			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "splitcomboer")
		{

			int xPos, yPos;

			is >> xPos;
			is >> yPos;

			int pathLength;
			list<Vector2i> localPath;

			Enemy::ReadPath(is, pathLength, localPath);

			bool loop;
			Enemy::ReadBool(is, loop);

			int level;
			is >> level;

			SplitComboer *enemy = new SplitComboer(this, Vector2i(xPos, yPos), localPath, loop, level);


			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "booster")
		{
			int xPos, yPos;

			//always air

			is >> xPos;
			is >> yPos;

			int level;
			is >> level;

			Booster *enemy = new Booster(this, Vector2i(xPos, yPos), level);
			//GravityModifier *enemy = new GravityModifier(this, Vector2i(xPos, yPos), .5, 300);

			activeItemTree->Insert(enemy);
			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "key")
		{
			int xPos, yPos;
			is >> xPos;
			is >> yPos;

			int numKeys;
			is >> numKeys;

			int zType;
			is >> zType;

			keyNumberObjects.push_back(new KeyNumberObj(Vector2i(xPos, yPos), numKeys, zType));
		}
		else if (typeName == "spring" || typeName == "gravityspring" || typeName == "bouncespring" 
			|| typeName == "airbouncespring" )//|| typeName == "teleportspring" )
		{
			int xPos, yPos;

			//always air

			is >> xPos;
			is >> yPos;

			int speed = 0;
			//if (typeName == "spring" || typeName == "gravityspring" || ty)
			{
				is >> speed;
			}


			Vector2i other;
			is >> other.x;
			is >> other.y;

			Spring::SpringType sp;
			if (typeName == "spring")
			{
				sp = Spring::BLUE;
			}
			else if (typeName == "gravityspring")
			{
				sp = Spring::GREEN;
			}
			else if (typeName == "bouncespring")
			{
				sp = Spring::BOUNCE;
			}
			else if (typeName == "airbouncespring")
			{
				sp = Spring::AIRBOUNCE;
			}
			else if (typeName == "teleportspring")
			{
				sp = Spring::TELEPORT;
			}
			//CurveLauncher * enemy = new CurveLauncher(this, Vector2i(xPos, yPos), other, moveFrames);
			Spring *enemy = new Spring(this, sp, Vector2i(xPos, yPos), other, speed);

			activeItemTree->Insert(enemy);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "teleporter" || typeName == "onewayteleporter")
		{
			int xPos, yPos;

			//always air

			is >> xPos;
			is >> yPos;


			Vector2i other;
			is >> other.x;
			is >> other.y;

			bool bothWays = true;
			if (typeName == "onewayteleporter")
			{
				bothWays = false;
			}

			Teleporter *enemy = new Teleporter(this, Vector2i(xPos, yPos), other, bothWays);

			activeItemTree->Insert(enemy);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);

			Teleporter *secondary = enemy->CreateSecondary();

			activeItemTree->Insert(secondary);

			fullEnemyList.push_back(secondary);
			//enem = enemy;

			enemyTree->Insert(secondary);


		}
		else if (typeName == "upbouncebooster" || typeName == "omnibouncebooster")
		{
			int xPos, yPos;
			
			//always air

			is >> xPos;
			is >> yPos;
			
			int level;
			is >> level;

			bool upOnly = false;
			if (typeName == "upbouncebooster")
			{
				upOnly = true;
			}


			BounceBooster *enemy = new BounceBooster(this, Vector2i(xPos, yPos), upOnly, level);

			activeItemTree->Insert(enemy);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "rail" || typeName == "grindrail" )
		{
			
			int xPos, yPos;

			//always air


			is >> xPos;
			is >> yPos;

			int pathLength;
			list<Vector2i> localPath;
			Enemy::ReadPath(is, pathLength, localPath);

			int accelerate;
			is >> accelerate;

			int level;
			is >> level;

			bool requirePower = false;
			if (typeName == "grindrail")
			{
				requirePower = true;
			}

			//Rail *r = new Rail(this, Vector2i(xPos, yPos), localPath, requirePower, accelerate, level );

			//++totalRails;
		}
		//w1
		else if (typeName == "patroller")
		{

			int xPos, yPos;

			//always air


			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;

			int pathLength;
			list<Vector2i> localPath;

			Enemy::ReadPath(is, pathLength, localPath);

			bool loop;
			Enemy::ReadBool(is, loop);

			int level;
			is >> level;
			Patroller *enemy = new Patroller(this, hasMonitor, Vector2i(xPos, yPos), localPath, loop, level);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "bosscrawler")
		{
			//always grounded

			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			CrawlerQueen *enemy = new CrawlerQueen(this, edges[polyIndex[terrainIndex] + edgeIndex],
				edgeQuantity, false);

			fullEnemyList.push_back(enemy);

			enem = enemy;

			enemyTree->Insert(enemy);
			/*Boss_Crawler *enemy = new Boss_Crawler( this, edges[polyIndex[terrainIndex] + edgeIndex],
			edgeQuantity );

			fullEnemyList.push_back( enemy );

			b_crawler = enemy;*/
		}
		else if (typeName == "basicturret")
		{
			//always grounded

			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;
			/*double bulletSpeed;
			is >> bulletSpeed;

			int framesWait;
			is >> framesWait;*/

			BasicTurret *enemy = new BasicTurret(this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity,level);
			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "crawler")
		{
			//always grounded

			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;

			//BossCrawler *enemy = new BossCrawler( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity );
			Crawler *enemy = new Crawler(this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex],
				edgeQuantity, level);
			//RoadRunner *enemy = new RoadRunner(this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex],
			//	edgeQuantity);


			/*if( enemy->hasMonitor )
			cout << "crawler with monitor!" << endl;
			else
			cout << "no monitor here" << endl;*/

			//enemyTree = Insert( enemyTree, enemy );
			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "shroom")
		{
			//cout << "loading foottrap" << endl;
			//always grounded


			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;

			//FootTrap *enemy = new FootTrap( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity );
			Shroom *enemy = new Shroom(this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, level);
			//Cactus *enemy = new Cactus( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "airdasher")
		{

			int xPos, yPos;

			//always air


			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;


			Airdasher *enemy = new Airdasher(this, hasMonitor, Vector2i(xPos, yPos), level);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}

		//w2
		else if (typeName == "bat")
		{

			int xPos, yPos;

			//always air


			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;


			int pathLength;
			list<Vector2i> localPath;

			Enemy::ReadPath(is, pathLength, localPath);

			bool loop;
			Enemy::ReadBool(is, loop);

			int level;
			is >> level;



			Bat *enemy = new Bat(this, hasMonitor, Vector2i(xPos, yPos), localPath, loop, level);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "airdashjuggler")
		{

			int xPos, yPos;

			//always air


			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;


			int pathLength;
			list<Vector2i> localPath;

			Enemy::ReadPath(is, pathLength, localPath);

			int level;
			is >> level;

			

			AirdashJuggler *enemy = new AirdashJuggler(this, hasMonitor, Vector2i(xPos, yPos), localPath, level);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "downgravityjuggler" || typeName == "upgravityjuggler" || typeName == "bouncejuggler" )
		{

			int xPos, yPos;

			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;

			int pathLength;
			list<Vector2i> localPath;

			Enemy::ReadPath(is, pathLength, localPath);

			int level;
			is >> level;

			int numJuggles;
			is >> numJuggles;


			Enemy *enemy;
			if (typeName == "downgravityjuggler" || typeName == "upgravityjuggler")
			{
				bool reversed = false;
				if (typeName == "upgravityjuggler")
				{
					reversed = true;
				}

				enemy = new GravityJuggler(this, hasMonitor, Vector2i(xPos, yPos), localPath,
					level, numJuggles, reversed);
			}
			else if (typeName == "bouncejuggler")
			{
				enemy = new BounceJuggler(this, hasMonitor, Vector2i(xPos, yPos), localPath,
					level, numJuggles);
			}
			

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "grindjugglercw" || typeName == "grindjugglerccw" )
		{

			int xPos, yPos;

			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;

			int numJuggles;
			is >> numJuggles;

			bool cw = true;
			if (typeName == "grindjugglerccw")
			{
				cw = false;
			}

			Enemy *enemy = new GrindJuggler(this, hasMonitor, Vector2i(xPos, yPos),
				level, numJuggles, cw);


			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "groundedgrindjugglercw" || typeName == "groundedgrindjugglerccw")
		{

			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;

			int numJuggles;
			is >> numJuggles;

			bool cw = true;
			if (typeName == "groundedgrindjugglerccw")
			{
				cw = false;
			}

			Enemy *enemy = new GroundedGrindJuggler(this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex],
				edgeQuantity, level, numJuggles, cw);


			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "jugglercatcher")
		{
			int xPos, yPos;

			//always air

			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;

			JugglerCatcher *enemy = new JugglerCatcher(this, hasMonitor, Vector2i(xPos, yPos), level);
			//GravityModifier *enemy = new GravityModifier(this, Vector2i(xPos, yPos), .5, 300);

			activeEnemyItemTree->Insert(enemy);
			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "poisonfrog")
		{
			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;
			/*int gravFactor;
			is >> gravFactor;

			int jumpStrengthX;
			is >> jumpStrengthX;

			int jumpStrengthY;
			is >> jumpStrengthY;

			int jumpFramesWait;
			is >> jumpFramesWait;*/

			PoisonFrog *enemy = new PoisonFrog(this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex],
				edgeQuantity, level);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "gravityfaller")
		{
			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;

			GravityFaller *enemy = new GravityFaller(this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex],
				edgeQuantity, level);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "stagbeetle")
		{
			//always grounded

			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;


			StagBeetle *enemy = new StagBeetle(this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex],
				edgeQuantity, level );
			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "curveturret")
		{
			//always grounded

			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int hasMonitor;
			is >> hasMonitor;

			/*double bulletSpeed;
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
			}*/

			int level;
			is >> level;

			CurveTurret *enemy = new CurveTurret(this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, level);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "gravityincreaser" || typeName == "gravitydecreaser" )
		{
			bool increaser = (typeName == "gravityincreaser");

			int xPos, yPos;

			//always air

			is >> xPos;
			is >> yPos;

			int level;
			is >> level;

			GravityModifier *enemy = new GravityModifier(this, Vector2i(xPos, yPos), level, increaser);

			activeItemTree->Insert(enemy);
			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}

		else if (typeName == "bossbird")
		{

			int xPos, yPos;

			is >> xPos;
			is >> yPos;

			BirdBoss *enemy = new BirdBoss(this, Vector2i(xPos, yPos));

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
			/*Boss_Bird *enemy = new Boss_Bird( this, Vector2i ( xPos, yPos ) );

			fullEnemyList.push_back( enemy );

			b_bird = enemy;

			enemyTree->Insert( enemy );*/
		}

		//w3
		else if (typeName == "pulser")
		{

			int xPos, yPos;

			//always air


			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;


			int pathLength;
			list<Vector2i> localPath;

			Enemy::ReadPath(is, pathLength, localPath);

			bool loop;
			Enemy::ReadBool(is, loop);

			int level;
			is >> level;
			//int framesBetweenNodes;
			//is >> framesBetweenNodes;

			Pulser *enemy = new Pulser(this, hasMonitor, Vector2i(xPos, yPos), localPath,
				loop, level );

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "cactus")
		{
			//always grounded

			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int hasMonitor;
			is >> hasMonitor;

			/*int bulletSpeed;
			is >> bulletSpeed;

			int rhythm;
			is >> rhythm;

			int amplitude;
			is >> amplitude;*/

			int level;
			is >> level;

			Cactus *enemy = new Cactus(this, hasMonitor,
				edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, level );


			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "owl")
		{

			int xPos, yPos;

			//always air


			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;

			//int moveSpeed;
			//is >> moveSpeed;

			//int bulletSpeed;
			//is >> bulletSpeed;

			//int rhythmFrames;
			//is >> rhythmFrames;

			int level;
			is >> level;

			Owl *enemy = new Owl(this, hasMonitor, Vector2i(xPos, yPos), level);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "badger")
		{
			//always grounded

			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;

			Badger *enemy = new Badger(this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, level);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "roadrunner")
		{
			//always grounded

			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;

			RoadRunner *enemy = new RoadRunner(this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, level);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "bouncefloater")
		{
			int xPos, yPos;

			//always air


			is >> xPos;
			is >> yPos;

			int level;
			is >> level;

			//Airdasher *enemy = new Airdasher(this, hasMonitor, Vector2i(xPos, yPos));
			//AirdashJuggler *enemy = new AirdashJuggler(this, hasMonitor, Vector2i(xPos, yPos), level);
			BounceFloater *enemy = new BounceFloater(this, Vector2i(xPos, yPos), level);


			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "bosscoyote")
		{
			//always grounded

			Vector2i pos;

			is >> pos.x;
			is >> pos.y;

			/*Boss_Coyote *enemy = new Boss_Coyote( this, pos );
			b_coyote = enemy;

			fullEnemyList.push_back( enemy );*/
		}

		//w4
		else if (typeName == "turtle")
		{

			int xPos, yPos;

			//always air


			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;

			//Vector2i delta(1000, -1000);
			Vector2i pos(xPos, yPos);
			Turtle *enemy = new Turtle(this, hasMonitor, Vector2i(xPos, yPos), level);
			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "coral")
		{

			int xPos, yPos;

			//always air


			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;

			int moveFrames;
			is >> moveFrames;

			/*SecurityWeb * enemy = new SecurityWeb( this,
			hasMonitor, Vector2i( xPos, yPos ), 8, 0, 10 );


			fullEnemyList.push_back( enemy );
			enem = enemy;

			enemyTree->Insert( enemy );*/
		}
		else if (typeName == "cheetah")
		{
			//always grounded

			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;

			Cheetah *enemy = new Cheetah( this, hasMonitor,
			edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, level );

			fullEnemyList.push_back( enemy );
			enem = enemy;

			enemyTree->Insert( enemy );
		}
		else if (typeName == "spider")
		{
			//always grounded

			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;

			Spider *enemy = new Spider(this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity,
				level);


			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "bosstiger")
		{
			int xPos, yPos;

			is >> xPos;
			is >> yPos;

			/*Boss_Tiger *enemy = new Boss_Tiger( this, Vector2i ( xPos, yPos ) );

			fullEnemyList.push_back( enemy );

			b_tiger = enemy;

			enemyTree->Insert( enemy );*/
		}

		//w5
		else if (typeName == "hungrycomboer" || typeName == "hungryreturncomboer" )
		{
			int xPos, yPos;

			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;

			int numJuggles;
			is >> numJuggles;

			
			bool returnsToPlayer = false;
			if (typeName == "hungryreturncomboer")
			{
				returnsToPlayer = true;
			}
			
			HungryComboer *enemy = new HungryComboer(this, hasMonitor, Vector2i(xPos, yPos), level, numJuggles,
				returnsToPlayer );

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "relativecomboer" || typeName == "relativecomboerdetach")
		{
			int xPos, yPos;

			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;

			int pathLength;
			list<Vector2i> localPath;

			Enemy::ReadPath(is, pathLength, localPath);

			int level;
			is >> level;

			int numJuggles;
			is >> numJuggles;

			bool detachOnKill = false;
			if (typeName == "relativecomboerdetach")
			{
				detachOnKill = true;
			}

			RelativeComboer *enemy = new RelativeComboer(this, hasMonitor, Vector2i(xPos, yPos), localPath, level, numJuggles,
				detachOnKill);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "swarm")
		{

			int xPos, yPos;

			//always air


			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;

			Swarm *enemy = new Swarm( this, Vector2i( xPos, yPos ), hasMonitor, level );
			fullEnemyList.push_back( enemy );
			enem = enemy;

			enemyTree->Insert( enemy );
		}
		else if (typeName == "shark")
		{

			int xPos, yPos;

			//always air


			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;

			Shark *enemy = new Shark( this, hasMonitor, Vector2i( xPos, yPos ), level );
			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert( enemy );
		}
		else if (typeName == "ghost")
		{
			int xPos, yPos;

			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;

			Ghost *enemy = new Ghost( this, hasMonitor, Vector2i( xPos, yPos ), level );

			fullEnemyList.push_back( enemy );
			enem = enemy;

			enemyTree->Insert( enemy );
		}
		else if (typeName == "overgrowth")
		{
			//always grounded

			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int hasMonitor;
			is >> hasMonitor;

			//GrowingTree * enemy = new GrowingTree( this, hasMonitor,
			//	edges[polyIndex[terrainIndex] + edgeIndex], 
			//	edgeQuantity, 32, 0, 1000 );

			//
			//fullEnemyList.push_back( enemy );
			//enem = enemy;

			//enemyTree->Insert( enemy );
		}
		else if (typeName == "growingtree")
		{
			//always grounded

			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;

			GrowingTree * enemy = new GrowingTree(this, hasMonitor,
				edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, level);

			//
			fullEnemyList.push_back( enemy );
			enem = enemy;

			enemyTree->Insert( enemy );
		}
		else if (typeName == "bossgator")
		{

			int xPos, yPos;

			is >> xPos;
			is >> yPos;

			/*Boss_Gator *enemy = new Boss_Gator( this, Vector2i ( xPos, yPos ) );

			fullEnemyList.push_back( enemy );

			b_gator = enemy;

			enemyTree->Insert( enemy );*/
		}

		//w6
		else if (typeName == "specter")
		{

			int xPos, yPos;

			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;

			int level;
			is >> level;

			Specter *enemy = new Specter( this, hasMonitor, Vector2i( xPos, yPos ), level );
			fullEnemyList.push_back( enemy );
			//enem = enemy;

			enemyTree->Insert( enemy );
		}
		else if (typeName == "swinglaunchercw" || typeName == "swinglauncherccw")
		{
			int xPos, yPos;

			//always air

			is >> xPos;
			is >> yPos;

			int speed = 0;
			is >> speed;

			Vector2i other;
			is >> other.x;
			is >> other.y;

			bool cw = true;
			if (typeName == "swinglauncherccw")
			{
				cw = false;
			}

			SwingLauncher *enemy = new SwingLauncher(this, Vector2i(xPos, yPos), other, speed, cw);

			activeItemTree->Insert(enemy);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "narwhal")
		{

			int xPos, yPos;

			//always air
			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;

			Vector2i dest;
			is >> dest.x;
			is >> dest.y;

			int moveFrames;
			is >> moveFrames;


			/*Jay *enemy = new Jay( this, hasMonitor, Vector2i( xPos, yPos ), dest );
			fullEnemyList.push_back( enemy );
			enem = enemy;

			enemyTree->Insert( enemy );*/
		}
		else if (typeName == "copycat")
		{

			int xPos, yPos;

			//always air


			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;

			/*Copycat *enemy = new Copycat( this, hasMonitor, Vector2i( xPos, yPos ) );


			fullEnemyList.push_back( enemy );
			enem = enemy;

			enemyTree->Insert( enemy );*/
		}
		else if (typeName == "gorilla")
		{

			int xPos, yPos;

			//always air


			is >> xPos;
			is >> yPos;

			int hasMonitor;
			is >> hasMonitor;

			//int wallWidth;
			//is >> wallWidth;

			//int followFrames;
			//is >> followFrames;

			int level;
			is >> level;


			Gorilla *enemy = new Gorilla( this, hasMonitor, Vector2i( xPos, yPos ), level );

			fullEnemyList.push_back( enemy );
			enem = enemy;

			enemyTree->Insert( enemy );
		}
		else if (typeName == "bossskeleton")
		{
			int xPos, yPos;

			is >> xPos;
			is >> yPos;

			/*Boss_Skeleton *enemy = new Boss_Skeleton( this, Vector2i ( xPos, yPos ) );

			fullEnemyList.push_back( enemy );

			b_skeleton = enemy;*/
		}


		else if (typeName == "nexus")
		{
			//always grounded

			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int nexusIndex;
			is >> nexusIndex;

			Nexus *enemy = new Nexus(this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity);

			goalNodePos = enemy->GetKillPos();
			float space = 78.f;
			V2d end(goalNodePos.x, goalNodePos.y - space);
			hasGoal = true;
			nexus = enemy;
			//nexusPos = enemy->GetKillPos();//enemy->position;

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "shippickup")
		{
			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int facingRight;
			is >> facingRight;

			ShipPickup *enemy = new ShipPickup(this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity,
				facingRight);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);

			if (shipExitSeq == NULL)
			{
				shipExitSeq = new ShipExitSeq(this);
			}
		}
		else if (typeName == "groundtrigger")
		{
			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			int facingRight;
			is >> facingRight;

			string tType;
			is >> tType;

			GroundTrigger *enemy = new GroundTrigger(this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity,
				facingRight, tType);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		else if (typeName == "airtrigger")
		{
			Vector2i pos;

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
			AirTrigger *at = new AirTrigger(this, V2d(pos), rectWidth, rectHeight, typeStr);
			airTriggerTree->Insert(at);
			fullAirTriggerList.push_back(at);
		}
		else if (typeName == "flowerpod")
		{
			int terrainIndex;
			is >> terrainIndex;

			int edgeIndex;
			is >> edgeIndex;

			double edgeQuantity;
			is >> edgeQuantity;

			string tType;
			is >> tType;

			FlowerPod *enemy = new FlowerPod(this, tType,
				edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity);

			fullEnemyList.push_back(enemy);
			enem = enemy;

			enemyTree->Insert(enemy);
		}
		//w6
		else if (typeName == "racefighttarget")
		{
			int xPos, yPos;

			//always air

			is >> xPos;
			is >> yPos;

			if (raceFight != NULL)
			{
				raceFight->numTargets++;

				/*RaceFightTarget *enemy = new RaceFightTarget(this, Vector2i(xPos, yPos));

				fullEnemyList.push_back(enemy);
				enem = enemy;

				enemyTree->Insert(enemy);*/
			}
			else
			{
				//ignore
			}
		}
		else
		{
			assert(false && "not a valid type name: ");
		}
	}
}



bool GameSession::OpenFile( string fileName )
{
	drain = true;
	hasGoal = false;
	numTotalKeys = 0;
	numKeysCollected = 0;
	
	currentFile = fileName;
	int insertCount = 0;
	ifstream is;
	is.open( fileName );//+ ".brknk" );
	if( is.is_open() )
	{
		mh = MapSelectionMenu::ReadMapHeader(is);
		assert(mh != NULL);

		/*if (mh->numShards > 0)
		{
			TryCreateShardResources();
		}*/

		if (mh->gameMode == MapHeader::MapType::T_RACEFIGHT)
		{
			assert(raceFight == NULL);
			raceFight = new RaceFight(this, 180);
		}

		
		points = new Vector2<double>[mh->numVertices];
		
		int numDecorImages;
		is >> numDecorImages;


		map<string, list<DecorInfo>> decorListMap;
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
			//dSpr.setTexture do this after dinner

			decorListMap[dName].push_back(DecorInfo( dSpr, dLayer, ts, dTile ));

			/*if (dLayer > 0)
			{
				decorImagesBehindTerrain.push_back(DecorInfo(dSpr, dLayer, dName));
			}
			else if (dLayer < 0)
			{
				decorImagesFrontTerrain.push_back(DecorInfo(dSpr, dLayer, dName));
			}
			else if (dLayer == 0)
			{
				decorImagesBetween.push_back(DecorInfo(dSpr, dLayer, dName));
			}*/
		}

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
					SetRectRotation(betweenVerts + vi * 4, rot, xSize, ySize , pos);
					SetRectSubRect(betweenVerts + vi * 4, sub);
					vi++;
				}

				decorBetween.push_back(new DecorDraw(betweenVerts, 
					numBetweenLayer * 4, betweenList.front().ts));
			}
		}
		//decorBetween

		is >> originalPos.x;
		is >> originalPos.y;

		Actor *p0 = GetPlayer(0);
		p0->position = originalPos;

		/*is >> goalPos.x;
		is >> goalPos.y;*/

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
			assert( 0 && "error reading borderpoly info" );
		}

		int pointsLeft = mh->numVertices;

		int pointCounter = 0;

		edges = new Edge*[mh->numVertices];

		int polyCounter = 0;
		//could use an array later if i wanted to
		map<int, int> polyIndex;
	
		inversePoly = NULL;

		while( pointCounter < mh->numVertices)
		{
			bool inverse = false;
			if( !hasReadBorderPoly )
			{
				hasReadBorderPoly = true;
				inverse = true;
				polyCounter = -1;
			}
			

			int matWorld;
			is >> matWorld;

			int matVariation;
			is >> matVariation;
			
			bool currentVisible = true;
			if (matWorld == 8 && matVariation == 0)
			{
				currentVisible = false;
			}

			visibleTerrain[polyCounter] = currentVisible;

			//matWorld = 6;
			//matWorld = 2;
			matSet.insert( pair<int,int>( matWorld, matVariation ) );

			int polyPoints;
			is >> polyPoints;

			TerrainPiece * tPiece = new TerrainPiece(this);
			
			polyIndex[polyCounter] = pointCounter;
			cout << "setting poly index at : " << polyCounter << " to " << pointCounter << endl;

			int currentEdgeIndex = pointCounter;

			tPiece->startEdgeIndex = currentEdgeIndex;

			for( int i = 0; i < polyPoints; ++i )
			{
				int px, py;
				is >> px;
				is >> py;
				//is >> spec;
			
				points[pointCounter].x = px;
				points[pointCounter].y = py;
				++pointCounter;
			}

			double left, right, top, bottom;
			for( int i = 0; i < polyPoints; ++i )
			{
				Edge *ee = new Edge();
				ee->poly = tPiece;

  				edges[currentEdgeIndex + i] = ee;
				ee->v0 = points[i+currentEdgeIndex];
				if( i < polyPoints - 1 )
					ee->v1 = points[i+1 + currentEdgeIndex];
				else
					ee->v1 = points[currentEdgeIndex];

				assert(ee->v0.x != ee->v1.x || ee->v0.y != ee->v1.y );

				terrainTree->Insert( ee );

				if( inverse )
				{
					inverseEdgeTree->Insert( ee );
				}

				double localLeft = min( ee->v0.x, ee->v1.x );
				double localRight = max( ee->v0.x, ee->v1.x );
				double localTop = min( ee->v0.y, ee->v1.y );
				double localBottom = max( ee->v0.y, ee->v1.y ); 
				if( i == 0 )
				{
					left = localLeft;
					right = localRight;
					top = localTop;
					bottom = localBottom;
				}
				else
				{
					left = min( left, localLeft );
					right = max( right, localRight );
					top = min( top, localTop);
					bottom = max( bottom, localBottom);
				}

			}


			for( int i = 0; i < polyPoints; ++i )
			{
				Edge * ee = edges[i + currentEdgeIndex];
				if( i == 0 )
				{
					ee->edge0 = edges[currentEdgeIndex + polyPoints - 1];
					ee->edge1 = edges[currentEdgeIndex + 1];
				}
				else if( i == polyPoints - 1 )
				{
					ee->edge0 = edges[currentEdgeIndex + i - 1];
					ee->edge1 = edges[currentEdgeIndex];
				
				}
				else
				{
					ee->edge0 = edges[currentEdgeIndex + i - 1];
					ee->edge1 = edges[currentEdgeIndex + i + 1];
				}
			}

			int edgesWithSegments;
			is >> edgesWithSegments;

			
			list<GrassSegment> segments;
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

					segments.push_back( GrassSegment( edgeIndex, index, reps ) );
				}
			}

			
			
			Tileset *ts_bush = GetTileset( "Env/bush_01_64x64.png", 64, 64 );

			if( !inverse && currentVisible )
			{
				vector<p2t::Point*> polyline;
				for( int i = 0; i < polyPoints; ++i )
				{
					polyline.push_back( new p2t::Point( points[currentEdgeIndex +i].x, points[currentEdgeIndex +i].y ) );
				}

				p2t::CDT * cdt = new p2t::CDT( polyline );


				//cdt->add
			
				vector<p2t::Triangle*> tris;

				cdt->Triangulate();
				
				tris = cdt->GetTriangles();
																																						
			
				va = new VertexArray( sf::Triangles , tris.size() * 3 );
				VertexArray & v = *va;
				Color testColor( 0x75, 0x70, 0x90 );
				testColor = Color::White;
				Vector2f topLeft( left, top );
				//cout << "topleft: " << topLeft.x << ", " << topLeft.y << endl;
				for( int i = 0; i < tris.size(); ++i )
				{	
					p2t::Point *p = tris[i]->GetPoint( 0 );	
					p2t::Point *p1 = tris[i]->GetPoint( 1 );	
					p2t::Point *p2 = tris[i]->GetPoint( 2 );	
					v[i*3] = Vertex( Vector2f( p->x, p->y ), testColor );
					v[i*3 + 1] = Vertex( Vector2f( p1->x, p1->y ), testColor );
					v[i*3 + 2] = Vertex( Vector2f( p2->x, p2->y ), testColor );
																			/*Vector2f pp0 = (v[i*3].position - topLeft);
				Vector2f pp1 = (v[i*3+1].position - topLeft);
				Vector2f pp2 = (v[i*3+2].position - topLeft);
				pp0 = Vector2f( (int)pp0.x % 1024, (int)pp0.y % 1024 );
				pp1 = Vector2f( (int)pp1.x % 1024, (int)pp1.y % 1024 );
				pp2 = Vector2f( (int)pp2.x % 1024, (int)pp2.y % 1024 );

				if( i == 0 )
				{
					cout << "pos0: " << pp0.x << ", " << pp0.y << endl;
					cout << "pos1: " << pp1.x << ", " << pp1.y << endl;
					cout << "pos2: " << pp2.x << ", " << pp2.y << endl;
				}
				v[i*3].texCoords = pp0;
				v[i*3+1].texCoords = pp1;
				v[i*3+2].texCoords = pp2;*/
				}

				double polygonArea = 0;
				for( vector<p2t::Triangle*>::iterator it = tris.begin();
					it != tris.end(); ++it )
				{
					polygonArea += GetTriangleArea( (*it) );
				}

				//VertexArray *bushVA = SetupBushes( 0,  edges[currentEdgeIndex], ts_bush );

				tPiece->polyArea = polygonArea;
				tPiece->aabb.left = left;
				tPiece->aabb.top = top;
				tPiece->aabb.width = right - left;
				tPiece->aabb.height = bottom - top;
				tPiece->terrainVA = va;
				tPiece->visible = true;
				polygons.push_back( va );

				delete cdt;
				for( int i = 0; i < polyPoints; ++i )
				{
					delete polyline[i];
				}

			}
			else if( inverse )
			{
				inversePoly = tPiece;
				inversePoly->numPoints = polyPoints;
				//testva->va = va;
				inversePoly->visible = true;
				inversePoly->aabb.left = left;
				inversePoly->aabb.top = top;
				inversePoly->aabb.width = right - left;
				inversePoly->aabb.height = bottom - top;
				//testva->ts_bush = ts_bush;
				SetupInversePoly( ts_bush, currentEdgeIndex );
				tPiece = inversePoly;
				tPiece->ts_bush = ts_bush;
				//va = NULL;
			}
			else
			{
				tPiece = tPiece;
				tPiece->polyArea = 0;//polygonArea;
				tPiece->visible = false;
				tPiece->aabb.left = left;
				tPiece->aabb.top = top;
				tPiece->aabb.width = right - left;
				tPiece->aabb.height = bottom - top;
				tPiece->terrainVA = NULL;//va;

				//polygons.push_back(va);
			}

			if (tPiece->visible)
			{


				switch (matWorld)
				{
				case 0:
				{
					switch (matVariation)
					{
					case 0:
						//testva->AddDecorExpression( 
						//testva->bushes.push_back( 
						break;
					}
					break;
				}
				case 1:
				{
					switch (matVariation)
					{
					case 0:
						break;
					}
					break;
				}
				case 2:
				{
					switch (matVariation)
					{
					case 0:
						break;
					}
					break;
				}
				case 3:
				{
					switch (matVariation)
					{
					case 0:
						break;
					}
					break;
				}
				case 4:
				{
					switch (matVariation)
					{
					case 0:
						break;
					}
					break;
				}
				case 5:
				{
					switch (matVariation)
					{
					case 0:
						break;
					}
					break;
				}
				case 6:
				{
					switch (matVariation)
					{
					case 0:
						break;
					}
					break;
				}
				}

				//add the veins to the terrain
				//for (int i = 0; i < 6; ++i)
				//{
				//	DecorExpression *expr = CreateDecorExpression(DecorType(D_W1_VEINS1 + i), 0, edges[currentEdgeIndex]);
				//	if (expr != NULL)
				//		tPiece->AddDecorExpression(expr);
				//}


				////add the rocks and bushes to the terrain
				//DecorExpression *rock1 = CreateDecorExpression(D_W1_ROCK_1, 0, edges[currentEdgeIndex]);
				//if (rock1 != NULL)
				//	tPiece->AddDecorExpression(rock1);

				//DecorExpression *rock2 = CreateDecorExpression(D_W1_ROCK_2, 0, edges[currentEdgeIndex]);
				//if (rock2 != NULL)
				//	tPiece->AddDecorExpression(rock2);

				//DecorExpression *rock3 = CreateDecorExpression(D_W1_ROCK_3, 0, edges[currentEdgeIndex]);
				//if (rock3 != NULL)
				//	tPiece->AddDecorExpression(rock3);

				//DecorExpression *grassyRock = CreateDecorExpression(D_W1_GRASSYROCK, 0, edges[currentEdgeIndex]);
				//if (grassyRock != NULL)
				//	tPiece->AddDecorExpression(grassyRock);

				//DecorExpression *normalExpr = CreateDecorExpression(D_W1_BUSH_NORMAL, 0, edges[currentEdgeIndex]);
				//if (normalExpr != NULL)
				//	tPiece->AddDecorExpression(normalExpr);

				//DecorExpression *exprPlantRock = CreateDecorExpression(D_W1_PLANTROCK, 0, edges[currentEdgeIndex]);
				//if (exprPlantRock != NULL)
				//	tPiece->AddDecorExpression(exprPlantRock);


			}


			VertexArray *polygonVA = va;

			double totalPerimeter = 0;

			tPiece->terrainWorldType = matWorld;
			tPiece->terrainVariation = matVariation;

			tPiece->SetupGrass(segments);


			Tileset *ts_border = NULL;
			Tileset *ts_plant = NULL;
			VertexArray *plantVA = NULL;
			if (tPiece->visible)
			{


				stringstream ss;

				ss << "Borders/bor_" << matWorld + 1 << "_";

				//ss << "01_512x704";
				ss << "01_512x512";
				/*if (matVariation < 10)
				{
					ss << "0" << matVariation + 1;
				}
				else
				{
					ss << matVariation + 1;
				}*/

				//ss << "_128x64.png";
				ss << ".png";

				ts_border = GetTileset(ss.str(), 128, 64);

				assert(ts_border != NULL);
				/*VertexArray *groundVA = SetupBorderQuads( 0, edges[currentEdgeIndex], ts_border,
					&GameSession::IsFlatGround );
				VertexArray *slopeVA = SetupBorderQuads( 0, edges[currentEdgeIndex], ts_border,
					&GameSession::IsSlopedGround );
				VertexArray *steepVA = SetupBorderQuads( 0, edges[currentEdgeIndex], ts_border,
					&GameSession::IsSteepGround );
				VertexArray *wallVA = SetupBorderQuads( 0, edges[currentEdgeIndex], ts_border,
					&GameSession::IsWall );*/

				ts_plant = GetTileset("Env/testgrass.png", 32, 32);


				plantVA = SetupPlants(edges[currentEdgeIndex], ts_plant);

				tPiece->tr = new TerrainRender(&tm, terrainTree);// (terrainTree);
				tPiece->tr->startEdge = edges[currentEdgeIndex];
				tPiece->tr->GenerateBorderMesh();
				//tPiece->tr->GenerateDecor();
				tPiece->tr->ts_border = ts_border;

				tPiece->AddTouchGrass( TouchGrass::TYPE_NORMAL);
				tPiece->AddTouchGrass(TouchGrass::TYPE_TEST);
				//tPiece->tr->plan

			}
			else
			{
				tPiece->tr = NULL;
			}

			/*double polygonArea = 0;
			for( vector<p2t::Triangle*>::iterator it = tris.begin();
				it != tris.end(); ++it )
			{
				polygonArea += GetTriangleArea( (*it) );
			}*/

			
			//testva->polyArea = polygonArea;

			

			//now that I have the area, get a number of random points
			//around the polygon based on how much area there is. 
			//then put plants in those areas

			//VertexArray *decorLayer0VA = SetupDecor0( tris, ts_decor0 );


			//VertexArray *triVA = SetupBorderTris( 0, edges[currentEdgeIndex], ts_border );
			VertexArray *triVA = NULL;//SetupTransitions( 0, edges[currentEdgeIndex], ts_border );
			//VertexArray *triVA = NULL;
			Tileset *ts_energyFlow = NULL;//GetTileset( "energyFlow.png", 0, 0 );
			//VertexArray *energyFlowVA = //SetupEnergyFlow( 0, edges[currentEdgeIndex], ts_energyFlow );


			bool first = true;
			
		

			
			tPiece->plantva = NULL; //temporary
			tPiece->next = NULL;
			
			
			//tPiece->grassVA = grassVA;
			tPiece->ts_bush = ts_bush;
			//testva->bushVA = bushVA;
			

			tPiece->ts_border = ts_border;
			/*testva->groundva = groundVA;
			testva->slopeva = slopeVA;
			testva->steepva = steepVA;
			testva->wallva = wallVA;*/
			tPiece->triva = triVA;
			tPiece->plantva = plantVA;
			tPiece->ts_plant = ts_plant;
			

			//testva->flowva = energyFlowVA;
			
			//cout << "before insert border: " << insertCount << endl;
			borderTree->Insert(tPiece);
			allVA.push_back(tPiece);

			//cout << "after insert border: " << insertCount << endl;
			insertCount++;
			

			

			//cout << "loaded to here" << endl;
			
			++polyCounter;
		}
		


		LoadMovingPlats( is, polyIndex );

		LoadBGPlats( is, polyIndex );

		LoadRails(is);

		LoadEnemies( is, polyIndex );
		
		LoadGates( is, polyIndex );

		is.close();

		bool blackBorder[2];
		bool topBorderOn = false;
		
		SetupStormCeiling();
		SetupMapBorderQuads(blackBorder, topBorderOn);
		SetupMinimapBorderQuads(blackBorder, topBorderOn);

		if (topBorderOn)
		{
			topClouds = new TopClouds(this);
		}

		CreateZones();
		SetupZones();

		for( int i = 0; i < numGates; ++i )
		{
			Gate *g = gates[i];
			//if( g->type == Gate::BIRDFIGHT )
			//{
			//	if( g->zoneA != NULL )
			//		g->zoneA->showShadow = false;
			//		//ActivateZone( zone
			//		//g->zoneA->ac
			//	//g->SetLocked( false );
			//	//g->SetLocked( true );
			//}
		}
		
		if( poiMap.count( "ship" ) > 0 )
		{
			ResetShipSequence();



			//cloud0a.setPosition( pi->pos.x - 960, pi->pos.y + 270 );
			//cloud0b.setPosition( pi->pos.x - 240, pi->pos.y + 270 );
			//cloud1a.setPosition( pi->pos.x - 960, pi->pos.y + 270 );
			//cloud1b.setPosition( pi->pos.x - 240, pi->pos.y + 270);// + 540 );
			//player->position = poiMap.ship
		}
		else
		{
			
			shipSequence = false;
			//normal map
		}

		if( poiMap.count( "bc_alliance" ) > 0 )
		{
			startMapSeq = new BirdCrawlerAllianceSeq( this );
			activeSequence = startMapSeq;
			startMapSeq->Reset();
			//fadeAlpha = 255;
			//owner->Fade( false, 60, Color::Black );
			//owner->Pause( 60 );
			//activeSequence = 
			
			//player->action = Actor::Action::Se;
			//player->frame = 1;
		}

		return true;
	}
	else
	{

		//new file
		assert( false && "error getting file to play " );

		return false;
	}
}

bool cmpPairs(pair<double,int> & a, pair<double,int> & b)
{
	return a.first < b.first;
}

bool cmpPairsDesc( pair<double,int> & a, pair<double,int> & b)
{
	return a.first > b.first;
}

void GameSession::SetGlobalBorders()
{
	borderEdge = NULL;
	//borders not allowed to intersect w/ gates
	V2d topLeft( mh->leftBounds, mh->topBounds );
	V2d topRight(mh->leftBounds + mh->boundsWidth, mh->topBounds );
	V2d bottomRight(mh->leftBounds + mh->boundsWidth, mh->topBounds + mh->boundsHeight );
	V2d bottomLeft(mh->leftBounds, mh->topBounds + mh->boundsHeight );
	
	Edge *left = new Edge;
	left->v0 = topLeft;
	left->v1 = bottomLeft;
	left->edgeType = Edge::BORDER;

	Edge *right = new Edge;
	right->v0 = bottomRight;
	right->v1 = topRight;
	right->edgeType = Edge::BORDER;
	cout << "making new edge at x value: " << right->v0.x << endl;

	Edge *top = new Edge;
	top->v0 = topRight;
	top->v1 = topLeft;
	top->edgeType = Edge::BORDER;

	Edge *bot = new Edge;
	bot->v0 = bottomLeft;
	bot->v1 = bottomRight;
	bot->edgeType = Edge::BORDER;

	left->edge0 = top;
	left->edge1 = bot;

	top->edge0 = right;
	top->edge1 = left;

	right->edge0 = bot;
	right->edge1 = top;

	bot->edge0 = left;
	bot->edge1 = right;
	
	terrainTree->Insert(left);
	terrainTree->Insert(right);
	terrainTree->Insert(top);

	globalBorderEdges.push_back(left);
	globalBorderEdges.push_back(right);
	globalBorderEdges.push_back(top);
	globalBorderEdges.push_back(bot);
	//terrainTree->Insert(bot);

}

void GameSession::TryCreateShardResources()
{
	if (shardPop == NULL)
	{
		shardPop = new ShardPopup(this);
	}

	if (getShardSeq == NULL)
	{
		getShardSeq = new GetShardSequence(this);
	}
}

void GameSession::CreateZones()
{
	//no gates, no zones!
	for( int i = 0; i < numGates; ++i )
	{
		Gate *g = gates[i];
		//cout << "gate index: " << i << ", a: " << g->edgeA->v0.x << ", " << g->edgeA->v0.y << ", b: "
		//	<< g->edgeA->v1.x << ", " << g->edgeA->v1.y << endl;
		
		Edge *curr = g->edgeA;

		TerrainPolygon tp( NULL );
		V2d v0 = curr->v0;
		V2d v1 = curr->v1;
		list<Edge*> currGates;
		list<Gate*> ignoreGates;
		
		currGates.push_back( curr );
		

		tp.AddPoint( new TerrainPoint( Vector2i( curr->v0.x, curr->v0.y ), false ) );

		curr = curr->edge1;
		while( true )
		{
			if( curr->v0 == g->edgeA->v0 )//curr == g->edgeA )
			{
				//we found a zone!

				if( !tp.IsClockwise() )
				{
					//cout << "found a zone aaa!!! checking last " << zones.size() << " zones. gates: " << currGates.size() << endl;
					bool okayZone = true;


					for( list<Zone*>::iterator zit = zones.begin(); zit != zones.end() && okayZone; ++zit )
					{
						for( list<Edge*>::iterator cit = currGates.begin(); cit != currGates.end() && okayZone; ++cit )
						{
							for( list<Edge*>::iterator git = (*zit)->gates.begin(); git != (*zit)->gates.end(); ++git )
							{
								if( (*cit) == (*git) )
								{
									okayZone = false;
								}
							}
							//for( list<Gate*>::iterator git =
							
						}
					}

					if( okayZone )
					{
						Zone *z = new Zone( this, tp );
						z->gates = currGates;
						zones.push_back( z );
					//	cout << "creating a zone with " << currGates.size() << " gatesAAA" << endl;
					//	cout << "actually creating a new zone   1! with " << z->gates.size() << endl;
					}
					

				}
				else
				{
					//cout << "woulda been a zone" << endl;
				}

				break;
			}
			else if( curr == g->edgeB )
			{
				//currGates.push_back( curr );
				//cout << "not a zone even" << endl;
				break;
			}


			tp.AddPoint( new TerrainPoint( Vector2i( curr->v0.x, curr->v0.y ), false ) );

			if( curr->edgeType == Edge::CLOSED_GATE )
			{
				Gate *thisGate = (Gate*)curr->info;
				//this loop is so both sides of a gate can't be hit in the same zone
				bool okayGate = true;
				bool quitLoop = false;
				for( list<Edge*>::iterator it = currGates.begin(); it != currGates.end(); ++it )
				{
					Gate *otherGate = (Gate*)(*it)->info;
					

					if( otherGate == thisGate )
					{
						//currGates.erase( it );
						okayGate = false;
						break;
					}
				}

				if( !okayGate )
				{
					currGates.push_back( curr );
					Edge *cc = curr->edge0;
					//TerrainPoint *tempPoint = NULL;
					TerrainPoint *tempPoint = tp.pointEnd;
					tp.RemovePoint( tempPoint );
					delete tempPoint;
					//cout << "removing from a( " << g << " ) start: " << tp.numPoints << endl;
					
					while( true )
					{
						if( cc->edgeType == Edge::CLOSED_GATE )
						{
							Gate *ccGate = (Gate*)cc->info;
							if( ccGate == thisGate )
								break;
							else
							{
								bool foundIgnore = false;
								for( list<Gate*>::iterator it = ignoreGates.begin(); it != ignoreGates.end(); ++it )
								{
									if( (*it) == ccGate )
									{
										foundIgnore = true;
										break;
									}
								}

								if( foundIgnore )
								{
									Edge *temp = cc->edge1;
									ccGate->SetLocked( false );
									cc = temp->edge0;
									ccGate->SetLocked( true );
									continue;
								}
							}
						}


						if( true )
						//if( tp.pointStart != NULL )
						{
							tempPoint = tp.pointEnd;
							tp.RemovePoint( tempPoint );
							delete tempPoint;
							//cout << "removing from a: " << tp.numPoints << endl;

							if( tp.pointStart == tp.pointEnd )
							{
								quitLoop = true;
								break;
							}
						}
						else
						{
							quitLoop = true;
							break;
						}

						cc = cc->edge0;
					}

					if( quitLoop )
					{
						//cout << "quitloop a" << endl;
						break;
					}

					Edge *pr = cc->edge0;
					thisGate->SetLocked( false );
					curr = pr->edge1->edge1;
					//tp.AddPoint( new TerrainPoint( Vector2i( curr->v0.x, curr->v0.y ), false ) );
					ignoreGates.push_back( thisGate );
					thisGate->SetLocked( true );

					//cout << "GATE IS NOT OKAY A: " << tp.numPoints << endl;
					//break;
					continue;
				}
				else
				{
					//cout << "found another gate AA: " <<  << endl;
				}

				//cout << "found another gate AA" << endl;
				currGates.push_back( curr );
			}
			else
			{

			}
			curr = curr->edge1;
		}

		
		currGates.clear();
		ignoreGates.clear();

		curr = g->edgeB;

		
		currGates.push_back( curr );
		

		TerrainPolygon tpb( NULL );

		tpb.AddPoint( new TerrainPoint( Vector2i( curr->v0.x, curr->v0.y ), false ) );

		curr = curr->edge1;
		while( true )
		{
			if( curr->v0 == g->edgeB->v0 )//curr == g->edgeB )
			{
				//we found a zone!

				if( !tpb.IsClockwise() )
				{
					//cout << "found a zone bbb!!! checking last " << zones.size() << " zones. gates: " << currGates.size() << endl;
					bool okayZone = true;
					for( list<Zone*>::iterator zit = zones.begin(); zit != zones.end() && okayZone; ++zit )
					{
						for( list<Edge*>::iterator cit = currGates.begin(); cit != currGates.end() && okayZone; ++cit )
						{
							for( list<Edge*>::iterator git = (*zit)->gates.begin(); git != (*zit)->gates.end(); ++git )
							{
								if( (*cit) == (*git) )
								{
									okayZone = false;
								}
							}
							//for( list<Gate*>::iterator git =
							
						}
					}

					if( okayZone )
					{
						Zone *z = new Zone( this, tpb );
						//cout << "creating a zone with " << currGates.size() << " gatesBBB" << endl;
						z->gates = currGates;
						zones.push_back( z );
						//cout << "actually creating a new zone   2! with " << z->gates.size() << endl;
					}
					

				}
				else
				{
					//cout << "woulda been a zone" << endl;
				}

				break;
			}
			else if( curr == g->edgeA )
			{
				//currGates.push_back( curr );
				//cout << "not a zone even b" << endl;
				break;
			}


			tpb.AddPoint( new TerrainPoint( Vector2i( curr->v0.x, curr->v0.y ), false ) );

			if( curr->edgeType == Edge::CLOSED_GATE )
			{
				bool quitLoop = false;
				bool okayGate = true;
				Gate *thisGate = (Gate*)curr->info;
				for( list<Edge*>::iterator it = currGates.begin(); it != currGates.end(); ++it )
				{
					Gate *otherGate = (Gate*)(*it)->info;
					

					if( otherGate == thisGate )
					{
						okayGate = false;
						break;
					}
				}

				if( !okayGate )
				{
					currGates.push_back( curr );
					//TerrainPoint *tempPoint = NULL;
					TerrainPoint *tempPoint = tpb.pointEnd;
					tpb.RemovePoint( tempPoint );
					delete tempPoint;
					//cout << "removing from b( " << g << " ) start: " << tpb.numPoints << endl;

					Edge *cc = curr->edge0;
					
					
					while( true )
					{
						if( cc->edgeType == Edge::CLOSED_GATE )
						{
							
							Gate *ccGate = (Gate*)cc->info;
							if( ccGate == thisGate )
								break;
							else
							{
								bool foundIgnore = false;
								for( list<Gate*>::iterator it = ignoreGates.begin(); it != ignoreGates.end(); ++it )
								{
									if( (*it) == ccGate )
									{
										foundIgnore = true;
										break;
									}
								}

								if( foundIgnore )
								{
									Edge *temp = cc->edge1;
									ccGate->SetLocked( false );
									cc = temp->edge0;
									ccGate->SetLocked( true );
									continue;
								}
								//for( list<Edge*>::iterator it = currGates.begin(); it != currGates.end(); ++it )
								//{
								//	//Gate *otherGate = (Gate*)(*it)->info;
					
								//	if( 
								//	//if( otherGate == thisGate )
								//	//{
								//	//	okayGate = false;
								//	//	break;
								//	//}
								//}
							}
						}

						if( true )
						//if( tpb.pointStart != NULL )
						{
							tempPoint = tpb.pointEnd;
							tpb.RemovePoint( tempPoint );
							delete tempPoint;
							//cout << "removing from b: " << tpb.numPoints << endl;
							if( tpb.pointStart == tpb.pointEnd )
							{
								quitLoop = true;
								break;
							}
								
						}
						else
						{
							quitLoop = true;
							break;
						}
						
						
						cc = cc->edge0;

						
						
					}

					if( quitLoop )
					{
						//cout << "quitloop b" << endl;
						break;
					}

					Edge *pr = cc->edge0;
					thisGate->SetLocked( false );
					curr = pr->edge1->edge1;
					//tpb.AddPoint( new TerrainPoint( Vector2i( curr->v0.x, curr->v0.y ), false ) );
					ignoreGates.push_back( thisGate );
					thisGate->SetLocked( true );
					//cout << "GATE IS NOT OKAY B: " << tpb.numPoints << endl;
					continue;
				}
				else
				{
				//	cout << "found another gate BB: " << curr-> << endl;
				}

				
				currGates.push_back( curr );
			}

			curr = curr->edge1;
		}
		
		//tp.AddPoint( new TerrainPoint( 
	}



	for( int i = 0; i < numGates; ++i )
	{
		//gates[i]->SetLocked( true );
	for( list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it )
	{
		//cout << "setting gates in zone: " << (*it) << " which has " << (*it)->gates.size() << " gates " << endl;
		//cout << i << ", it gates: " << (*it)->gates.size() << endl;
		for( list<Edge*>::iterator eit = (*it)->gates.begin(); eit != (*it)->gates.end(); ++eit )
		{
			if( gates[i]->edgeA == (*eit) )
			{
			//	cout << "gate: " << gates[i] << ", gate zone a: " << (*it ) << endl;
				gates[i]->zoneA = (*it);
				//done++;
			}
			else if( gates[i]->edgeB == (*eit) )
			{
			//	cout << "gate: " << gates[i] << ", gate zone B: " << (*it ) << endl;
				//cout << "gate zone B: " << (*it ) << endl;
				gates[i]->zoneB = (*it);
				//done++;
			}
		}
	}
	}


	for (list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it)
	{
		for (list<Zone*>::iterator it2 = zones.begin(); it2 != zones.end(); ++it2)
		{
			if ((*it) == (*it2))
				continue;

			if ((*it)->ContainsZone((*it2)))
			{
				//cout << "contains zone!" << endl;
				(*it)->subZones.push_back((*it2));
			}
		}
	}

	for (list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it)
	{
		for (list<Zone*>::iterator it2 = (*it)->subZones.begin();
			it2 != (*it)->subZones.end(); ++it2)
		{
			if ((*it)->ContainsZoneMostSpecific((*it2)))
			{
				(*it2)->parentZone = (*it);
			}
		}
	}
	//list<Gate*> ;
	list<Edge*> outsideGates;

	int numOutsideGates = 0;
	for( int i = 0; i < numGates; ++i )
	{
		Gate *g = gates[i];
		if (g->zoneA == NULL && g->zoneB == NULL)
		{
			for (list<Zone*>::iterator zit = zones.begin(); zit != zones.end(); ++zit)
			{
				if ((*zit)->ContainsPoint(g->edgeA->v0))
				{
					g->zoneA = (*zit);
					g->zoneB = (*zit);
				}
			}
		}
		else if( g->zoneA == NULL )
		{
			if (g->zoneB->parentZone != NULL)
			{
				g->zoneA = g->zoneB->parentZone;
				g->zoneA->gates.push_back(g->edgeA);
			}
			else
			{
				outsideGates.push_back(g->edgeA);
				numOutsideGates++;
			}
		}
		else if( g->zoneB == NULL )
		{
			if (g->zoneA->parentZone != NULL)
			{
				g->zoneB = g->zoneA->parentZone;
				g->zoneB->gates.push_back(g->edgeB);
			}
			else
			{
				outsideGates.push_back(g->edgeB);
				numOutsideGates++;
			}
		}
	}

	/*for (auto it = outsideGates.begin(); it != outsideGates.end();)
	{
		for (list<Zone*>::iterator zit = zones.begin(); zit != zones.end(); ++zit)
		{
			if ((*zit)->ContainsPoint((*it)->edge0->v0))
			{
				(*it)->zoneA = (*zit);
				(*it)->zoneB = (*zit);
				outsideGates.erase(it++);
			}
			else
			{
				++it;
			}
		}
	}*/

	//cout << "numoutside gates!!: " << numOutsideGates << endl;
	//wish i knew what this was supposed to do. you turned this off because
	//borderEdge is always null now.
	if( numOutsideGates > 0 )//&& borderEdge != NULL )
	{
		assert( inverseEdgeTree != NULL );

		TerrainPolygon tp( NULL );
		Edge *startEdge = edges[0];
		Edge *curr = startEdge;
		
		tp.AddPoint( new TerrainPoint( Vector2i( curr->v0.x, curr->v0.y ), false ) );

		curr = curr->edge1;

		while( curr != startEdge)
		{
			tp.AddPoint( new TerrainPoint( Vector2i( curr->v0.x, curr->v0.y ), false ) );

			curr = curr->edge1;
		}

		tp.FixWinding();

		Zone *z = new Zone( this, tp );
		z->gates = outsideGates;
		zones.push_back( z );

		for( list<Edge*>::iterator it = outsideGates.begin(); it != outsideGates.end(); ++it )
		{
			Gate *g = (Gate*)(*it)->info;
			if( g->zoneA == NULL )
			{
				g->zoneA = z;
			}
			
			if( g->zoneB == NULL )
			{
				g->zoneB = z;
			}

		}

		
		for (list<Zone*>::iterator it2 = zones.begin(); it2 != zones.end(); ++it2)
		{
			if (z == (*it2))
				continue;

			if (z->ContainsZone((*it2)))
			{
				//cout << "contains zone!" << endl;
				z->subZones.push_back((*it2));
			}
		}
		

		
		for (list<Zone*>::iterator it2 = z->subZones.begin();
			it2 != z->subZones.end(); ++it2)
		{
			if (z->ContainsZoneMostSpecific((*it2)))
			{
				(*it2)->parentZone = z;
			}
		}
	
		//TerrainPolygon tp( NULL );
		
	}

	/*cout << "gate testing: " << endl;
	for( int i = 0; i < numGates; ++i )
	{
		cout << "gate " << i << ": " << gates[i]->zoneA << ", " << gates[i]->zoneB << endl;
	}*/
}

void GameSession::SetupZones()
{
	//cout << "setupzones" << endl;
	//setup subzones
	//for( list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it )
	//{
	//	for( list<Zone*>::iterator it2 = zones.begin(); it2 != zones.end(); ++it2 )
	//	{
	//		if( (*it) == (*it2) ) 
	//			continue;

	//		if( (*it)->ContainsZone( (*it2) ) )
	//		{
	//			//cout << "contains zone!" << endl;
	//			(*it)->subZones.push_back( (*it2) );
	//		}
	//	}
	//}

	//for (list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it)
	//{
	//	for (list<Zone*>::iterator it2 = (*it)->subZones.begin();
	//		it2 != (*it)->subZones.end(); ++it2)
	//	{
	//		if ((*it)->ContainsZoneMostSpecific((*it2)))
	//		{
	//			(*it2)->parentZone = (*it);
	//		}
	//	}
	//}


	//	cout << "1" << endl;
	//add enemies to the correct zone.
	for( list<Enemy*>::iterator it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it )
	{
		for( list<Zone*>::iterator zit = zones.begin(); zit != zones.end(); ++zit )
		{
			bool hasPoint = (*zit)->ContainsPoint( (*it)->position );
			if( hasPoint )
			{
				bool mostSpecific = true;
				for( list<Zone*>::iterator zit2 = (*zit)->subZones.begin(); zit2 != (*zit)->subZones.end(); ++zit2 )
				{
					if( (*zit2)->ContainsPoint( (*it)->position ) )
					{
						mostSpecific = false;
						break;
					}
				}

				if( mostSpecific )
				{
					(*it)->SetZone((*zit));
				}
			}
		}

		//if( (*it)->type == EnemyType::EN_CRAWLERQUEEN )
		//{
		//	//cout << "whats this" << endl;
		//	if( (*it)->zone != NULL )
		//	{
		//		(*it)->zone->action = Zone::OPEN;
		//	}
		//	
		//}
		//else
		//{
		if( (*it)->zone != NULL )
			(*it)->zone->allEnemies.push_back( (*it) );
		//}


	}

	for (list<Zone*>::iterator zit = zones.begin(); zit != zones.end(); ++zit)
	{
		int numTotalKeys = 0;
		for (auto it = (*zit)->allEnemies.begin(); it != (*zit)->allEnemies.end(); ++it)
		{
			if ((*it)->hasMonitor)
			{
				++numTotalKeys;
			}
		}
		(*zit)->totalStartingKeys = numTotalKeys;
	}

	for (list<Zone*>::iterator zit = zones.begin(); zit != zones.end(); ++zit)
	{
		(*zit)->requiredKeys = (*zit)->totalStartingKeys;
	}

	//set key number objects correctly
	for( list<KeyNumberObj*>::iterator it = keyNumberObjects.begin(); it != keyNumberObjects.end(); ++it )
	{
		Zone *assignZone = NULL;
		V2d cPos( (*it)->pos.x, (*it)->pos.y );
		for( list<Zone*>::iterator zit = zones.begin(); zit != zones.end(); ++zit )
		{
			bool hasPoint = (*zit)->ContainsPoint( cPos );
			if( hasPoint )
			{
				bool mostSpecific = true;
				for( list<Zone*>::iterator zit2 = (*zit)->subZones.begin(); zit2 != (*zit)->subZones.end(); ++zit2 )
				{
					if( (*zit2)->ContainsPoint( cPos ) )
					{
						mostSpecific = false;
						break;
					}
				}

				if( mostSpecific )
				{
					assignZone = (*zit);
				}
			}
		}

		if( assignZone != NULL )
		{
			assignZone->requiredKeys = (*it)->numKeys;
			if( (*it)->zoneType > 0 )
				assignZone->SetZoneType((Zone::ZoneType)(*it)->zoneType);
		}

		delete (*it);
	}


	keyNumberObjects.clear();

	

	cout << "2" << endl;
	//which zone is the player in?
	for( list<Zone*>::iterator zit = zones.begin(); zit != zones.end(); ++zit )
	{
		//Vector2i truePos = Vector2i( player->position.x, player->position.y );
		bool hasPoint = (*zit)->ContainsPoint( GetPlayer( 0 )->position );
		if( hasPoint )
		{
			bool mostSpecific = true;
			for( list<Zone*>::iterator zit2 = (*zit)->subZones.begin(); zit2 != (*zit)->subZones.end(); ++zit2 )
			{
				if( (*zit2)->ContainsPoint( GetPlayer( 0 )->position ) )
				{
					mostSpecific = false;
					break;
				}
			}

			if( mostSpecific )
			{
				originalZone = (*zit);
			}
		}
	}

	
	

	if( originalZone != NULL )
	{
		cout << "setting original zone to active: " << originalZone << endl;
		//originalZone->active = true;
		ActivateZone(originalZone, true);
		keyMarker->SetStartKeysZone(originalZone);
		//currentZone = originalZone;
		//keyMarker->SetStartKeysZone(currentZone);
	}
	
	//cout << "3: numgates: " << numGates << endl;
	//cout << "num zones: " << zones.size() << endl;
	//assign correct zones to gates
	//for( int i = 0; i < numGates; ++i )
	//{
	//	

	//	for( list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it )
	//	{
	//		cout << i << ", it gates: " << (*it)->gates.size() << endl;
	//		for( list<Edge*>::iterator eit = (*it)->gates.begin(); eit != (*it)->gates.end(); ++eit )
	//		{
	//			if( gates[i]->edgeA == (*eit) )
	//			{
	//				cout << "gate zone a: " << (*it ) << endl;
	//				gates[i]->zoneA = (*it);
	//				//done++;
	//			}
	//			else if( gates[i]->edgeB == (*eit) )
	//			{
	//				cout << "gate zone B: " << (*it ) << endl;
	//				gates[i]->zoneB = (*it);
	//				//done++;
	//			}
	//		}
	//	}
	//}
	

	
	

	/*for( int i = 0; i < numGates; ++i )
	{
		if( gates[i]->zoneA == gates[i]->zoneB )
		{
			gates[i]->SetLocked( false );
		}
	}*/

	for( list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it )
	{
		(*it)->Init();
	}


	/*for( int i = 0; i < numGates; ++i )
	{
		if( gates[i]->zoneA == gates[i]->zoneB )
		{
			gates[i]->SetLocked( true );
		}
	}*/
}

ControllerState &GameSession::GetPrevInput( int index )
{
	return mainMenu->GetPrevInput( index );
}

ControllerState &GameSession::GetCurrInput( int index )
{
	return mainMenu->GetCurrInput( index );
}

ControllerState &GameSession::GetPrevInputUnfiltered(int index)
{
	return mainMenu->GetPrevInputUnfiltered(index);
}

ControllerState &GameSession::GetCurrInputUnfiltered(int index)
{
	return mainMenu->GetCurrInputUnfiltered(index);
}

Actor *GameSession::GetPlayer( int index )
{
	return players[index];
}

V2d GameSession::GetPlayerPos(int index)
{
	Actor *p = players[index];
	if (p != NULL)
	{
		return p->position;
	}
	else
	{
		return V2d();
	}
}

V2d GameSession::GetPlayerTrueVel(int index)
{
	Actor *p = players[index];
	if (p != NULL)
	{
		return p->GetTrueVel();
	}
	else
	{
		return V2d();
	}
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

bool GameSession::PlayerIsMovingLeft(int index )
{
	Actor *p = players[index];
	if (p != NULL)
	{
		return p->IsMovingLeft();
	}
	else
	{
		return false;
	}
}

bool GameSession::PlayerIsMovingRight(int index )
{
	Actor *p = players[index];
	if (p != NULL)
	{
		return p->IsMovingRight();
	}
	else
	{
		return false;
	}
}

bool GameSession::PlayerIsFacingRight(int index)
{
	Actor *p = players[index];
	if (p != NULL)
	{
		return p->facingRight;
	}
	else
	{
		return false;
	}
}

void GameSession::PlayerHitGoal( int index )
{
	Actor *p = players[index];
	if (p != NULL)
	{
		p->hitGoal = true;
	}
}

V2d GameSession::GetPlayerKnockbackDirFromVel(int index )
{
	Actor *p = players[index];
	if (p != NULL)
	{
		return p->GetKnockbackDirFromVel();
	}
	else
	{
		return V2d();
	}
}

void GameSession::PlayerAddActiveComboObj(ComboObject *co, int index)
{
	Actor *p = players[index];
	if (p != NULL)
	{
		p->AddActiveComboObj(co);
	}
}

void GameSession::PlayerRemoveActiveComboer(ComboObject *co, int index )
{
	Actor *p = players[index];
	if (p != NULL)
	{
		p->RemoveActiveComboObj(co);
	}
}

void GameSession::PlayerConfirmEnemyNoKill(Enemy *en, int index )
{
	Actor *p = players[index];
	if (p != NULL)
	{
		p->ConfirmEnemyNoKill(en);
	}
}

void GameSession::PlayerConfirmEnemyKill(Enemy *en, int index)
{
	Actor *p = players[index];
	if (p != NULL)
	{
		p->ConfirmEnemyKill(en);
	}
}

void GameSession::PlayerHitNexus(int index)
{
	Actor *p = players[index];
	if (p != NULL)
	{
		p->hitNexus = true;
	}
}

void GameSession::PlayerApplyHit( HitboxInfo *hi, int index)
{
	Actor *p = players[index];
	if (p != NULL)
	{
		p->ApplyHit(hi);
	}
}

void GameSession::SetupMinimapBorderQuads( bool *blackBorder, bool topBorderOn )
{
	int miniQuadWidth = 4000;
	int inverseTerrainBorder = 4000;
	int blackMiniTop = mh->topBounds - inverseTerrainBorder;
	int blackMiniBot = mh->topBounds + mh->boundsHeight + inverseTerrainBorder;
	int blackMiniLeft = mh->leftBounds - miniQuadWidth;
	int rightBounds = mh->leftBounds + mh->boundsWidth;
	int blackMiniRight = rightBounds + miniQuadWidth;

	sf::Vertex *blackBorderQuadsMini = mini->blackBorderQuadsMini;

	blackBorderQuadsMini[1].position.x = mh->leftBounds;
	blackBorderQuadsMini[2].position.x = mh->leftBounds;
	blackBorderQuadsMini[0].position.x = mh->leftBounds - miniQuadWidth;
	blackBorderQuadsMini[3].position.x = mh->leftBounds - miniQuadWidth;

	blackBorderQuadsMini[0].position.y = blackMiniTop;
	blackBorderQuadsMini[1].position.y = blackMiniTop;

	blackBorderQuadsMini[2].position.y = blackMiniBot;
	blackBorderQuadsMini[3].position.y = blackMiniBot;


	blackBorderQuadsMini[5].position.x = rightBounds + miniQuadWidth;
	blackBorderQuadsMini[6].position.x = rightBounds + miniQuadWidth;
	blackBorderQuadsMini[4].position.x = rightBounds;
	blackBorderQuadsMini[7].position.x = rightBounds;

	blackBorderQuadsMini[4].position.y = blackMiniTop;
	blackBorderQuadsMini[5].position.y = blackMiniTop;

	blackBorderQuadsMini[6].position.y = blackMiniBot;
	blackBorderQuadsMini[7].position.y = blackMiniBot;

	Color miniBorderColor = Color(100, 100, 100);
	Color miniTopBorderColor = Color(0x10, 0x40, 0xff);
	//SetRectColor(blackBorderQuads + 4, Color( 100, 100, 100 ));

	if (blackBorder[0])
		SetRectColor(blackBorderQuadsMini, miniTopBorderColor);
	if (blackBorder[1])
		SetRectColor(blackBorderQuadsMini + 4, miniTopBorderColor);

	if (stormCeilingOn || topBorderOn )
	{
		Vertex *topBorderQuadMini = mini->topBorderQuadMini;

		SetRectColor(topBorderQuadMini, miniTopBorderColor);

		topBorderQuadMini[0].position.x = blackMiniLeft;
		topBorderQuadMini[1].position.x = blackMiniRight;
		topBorderQuadMini[2].position.x = blackMiniRight;
		topBorderQuadMini[3].position.x = blackMiniLeft;

		topBorderQuadMini[0].position.y = blackMiniTop;
		topBorderQuadMini[1].position.y = blackMiniTop;
		topBorderQuadMini[2].position.y = mh->topBounds;
		topBorderQuadMini[3].position.y = mh->topBounds;
	}
	
}

void GameSession::SetupMapBorderQuads(bool *blackBorder,
	bool &topBorderOn)
{
	double extraBorder = 100;
	if (inversePoly != NULL)
	{
		int trueTop = mh->topBounds;
		int possibleTop = inversePoly->aabb.top; //- extraBorder;
		if (possibleTop > trueTop)
			trueTop = possibleTop;
		else
		{
			topBorderOn = true;
		}
		mh->topBounds = trueTop - extraBorder / 2;
		int inversePolyBottom = inversePoly->aabb.top + inversePoly->aabb.height;
		mh->boundsHeight = (inversePolyBottom + extraBorder) - trueTop;

		int inversePolyRight = (inversePoly->aabb.left + inversePoly->aabb.width);
		blackBorder[0] = inversePoly->aabb.left < mh->leftBounds; //inverse is further left than border
		blackBorder[1] = inversePolyRight >(mh->leftBounds + mh->boundsWidth); //inverse is further right than border

		int leftB = mh->leftBounds;
		int rightB = mh->leftBounds + mh->boundsWidth;
		if (!blackBorder[0])
		{
			mh->leftBounds = inversePoly->aabb.left - extraBorder;
			mh->boundsWidth = rightB - mh->leftBounds;
		}
		if (!blackBorder[1])
		{
			mh->boundsWidth = (inversePolyRight + extraBorder) - mh->leftBounds;
		}
		else
		{
			cout << "creating black border at " << (mh->leftBounds + mh->boundsWidth) << endl;
		}
	}
	else
	{
		blackBorder[0] = true;
		blackBorder[1] = true;


		auto it = allVA.begin();
		int maxY = (*it)->aabb.top + (*it)->aabb.height;
		int minX = (*it)->aabb.left;
		int maxX = (*it)->aabb.left + (*it)->aabb.width;

		++it;
		int temp;
		for (; it != allVA.end(); ++it)
		{
			temp = (*it)->aabb.top + (*it)->aabb.height;
			if (temp > maxY)
			{
				maxY = temp;
			}
			temp = (*it)->aabb.left;
			if (temp < minX)
			{
				minX = temp;
			}
			temp = (*it)->aabb.left + (*it)->aabb.width;
			if (temp > maxX)
			{
				maxX = temp;
			}
		}
		
		mh->boundsHeight = maxY - mh->topBounds - extraBorder;
		int oldRight = mh->leftBounds + mh->boundsWidth;
		int oldLeft = mh->leftBounds;
		if (minX > oldLeft)
		{
			mh->leftBounds = minX;
		}
		if (maxX <= oldRight)
		{
			mh->boundsWidth = maxX - mh->leftBounds;
		}
	}

	
	

	

	SetGlobalBorders();

	

	int quadWidth = 200;
	int extra = 1000;

	int top = mh->topBounds;
	int lBound = mh->leftBounds;
	int rBound = mh->leftBounds + mh->boundsWidth;
	int height = mh->boundsHeight;
	
	SetRectCenter(blackBorderQuads, quadWidth, height, Vector2f(lBound + quadWidth / 2,
		top + height / 2));
	SetRectCenter(blackBorderQuads + 4, quadWidth, height, Vector2f(rBound - quadWidth / 2,
		top + height / 2));

	SetRectCenter(blackBorderQuads + 8, extra, height, Vector2f(lBound - extra / 2,
		top + height / 2));
	SetRectCenter(blackBorderQuads + 12, extra, height, Vector2f(rBound + extra / 2,
		top + height / 2));
	SetRectColor(blackBorderQuads, Color(Color::Black));
	SetRectColor(blackBorderQuads + 4, Color(Color::Black));
	SetRectColor(blackBorderQuads + 8, Color(Color::Black));
	SetRectColor(blackBorderQuads + 12, Color(Color::Black));

	if (blackBorder[0])
	{
		blackBorderQuads[1].color.a = 0;
		blackBorderQuads[2].color.a = 0;
	}
	else
	{
		SetRectColor(blackBorderQuads, Color(Color::Transparent));
		SetRectColor(blackBorderQuads + 8, Color(Color::Transparent));
	}
	if (blackBorder[1])
	{
		blackBorderQuads[4].color.a = 0;
		blackBorderQuads[7].color.a = 0;
	}
	else
	{
		SetRectColor(blackBorderQuads + 4, Color(Color::Transparent));
		SetRectColor(blackBorderQuads + 12, Color(Color::Transparent));
	}

	if (stormCeilingOn)
	{
		int oldBottom = mh->topBounds + mh->boundsHeight - extraBorder;
		mh->topBounds = stormCeilingHeight;
		mh->boundsHeight = oldBottom - stormCeilingHeight;
		assert(mh->boundsHeight > 0);
	}
}

void GameSession::SetupStormCeiling()
{
	if (poiMap.count("stormceiling") > 0)
	{
		stormCeilingOn = true;
		stormCeilingHeight = poiMap["stormceiling"]->pos.y;
	}
}

Edge *GameSession::GetEdge(int index)
{
	return edges[index];
}

void TerrainPiece::SetupGrass(std::list<GrassSegment> &segments)
{
	int numGrassTotal = 0;

	for (list<GrassSegment>::iterator it = segments.begin(); it != segments.end(); ++it)
	{
		numGrassTotal += (*it).reps + 1;
	}

	Tileset *ts_grass = owner->GetTileset("Env/grass_128x128.png", 128, 128);

	//should this even be made on invisible terrain?

	int totalGrassIndex = 0;

	Grass::GrassType gType;
	if (terrainWorldType == 0)
	{
		gType = Grass::GrassType::JUMP;
	}
	else if (terrainWorldType == 1)
	{
		gType = Grass::GrassType::GRAVITY;
	}
	else if (terrainWorldType == 2)
	{
		gType = Grass::GrassType::BOUNCE;
	}
	else if (terrainWorldType == 3)
	{
		gType = Grass::GrassType::BOOST;
	}
	else if (terrainWorldType == 5)
	{
		gType = Grass::GrassType::ANTIWIRE;
	}

	owner->hasGrass[gType] = true;
	owner->hasAnyGrass = true;

	if (numGrassTotal > 0)
	{
		grassVA = new VertexArray(sf::Quads, numGrassTotal * 4);

		//cout << "num grass total: " << numGrassTotal << endl;
		VertexArray &grassVa = *grassVA;//*va;

		int segIndex = 0;
		int totalGrass = 0;
		for (list<GrassSegment>::iterator it = segments.begin(); it != segments.end(); ++it)
		{
			Edge *segEdge = owner->GetEdge(startEdgeIndex + (*it).edgeIndex);
			V2d v0 = segEdge->v0;
			V2d v1 = segEdge->v1;

			int start = (*it).index;
			int end = (*it).index + (*it).reps;

			int grassCount = (*it).reps + 1;
			bool rem;
			int num = GetNumGrass(segEdge, rem);

			V2d along = normalize(v1 - v0);
			V2d realStart = v0 + along * (double)(grassSize + grassSpacing);

			int jj;
			for (int j = 0; j < grassCount; ++j)
			{
				jj = j + start;
				V2d posd = realStart + along * (double)((grassSize + grassSpacing) * (jj - 1));//v0 + normalize(v1 - v0) * ((grassSize + grassSpacing) * (j-1) + );

				if (jj == 0)
				{
					posd = v0;
				}
				else if (jj == num - 1 && rem)
				{
					posd = v1 + normalize(v0 - v1) * (grassSize / 2.0 + grassSpacing);//(v1 + prev) / 2.0;
				}

				Vector2f pos(posd.x, posd.y);

				Vector2f topLeft = pos + Vector2f(-grassSize / 2, -grassSize / 2);
				Vector2f topRight = pos + Vector2f(grassSize / 2, -grassSize / 2);
				Vector2f bottomLeft = pos + Vector2f(-grassSize / 2, grassSize / 2);
				Vector2f bottomRight = pos + Vector2f(grassSize / 2, grassSize / 2);

				grassVa[(j + totalGrass) * 4].position = topLeft;
				grassVa[(j + totalGrass) * 4].texCoords = Vector2f(0, 0);

				grassVa[(j + totalGrass) * 4 + 1].position = bottomLeft;
				grassVa[(j + totalGrass) * 4 + 1].texCoords = Vector2f(0, grassSize);

				grassVa[(j + totalGrass) * 4 + 2].position = bottomRight;
				grassVa[(j + totalGrass) * 4 + 2].texCoords = Vector2f(grassSize, grassSize);

				grassVa[(j + totalGrass) * 4 + 3].position = topRight;
				grassVa[(j + totalGrass) * 4 + 3].texCoords = Vector2f(grassSize, 0);

				Grass * g = new Grass(owner, ts_grass, totalGrassIndex, posd, this, gType );
				owner->grassTree->Insert(g);

				++totalGrassIndex;
			}
			totalGrass += grassCount;
			segIndex++;
		}
	}
	else
	{
		grassVA = NULL;
	}

	//int totalGrassIndex = 0;

	//for (list<GrassSegment>::iterator it = segments.begin(); it != segments.end(); ++it)
	//{
	//	V2d A, B, C, D;
	//	Edge * currE = owner->GetEdge(startEdgeIndex + (*it).edgeIndex);
	//	V2d v0 = currE->v0;
	//	V2d v1 = currE->v1;

	//	double edgeLength = length(v1 - v0);
	//	double remainder = edgeLength / (grassSize + grassSpacing);

	//	double num = floor(remainder) + 1;

	//	int reps = (*it).reps;

	//	V2d edgeDir = normalize(v1 - v0);

	//	//V2d ABmin = v0 + (v1-v0) * (double)(*it).index / num - grassSize / 2 );
	//	V2d ABmin = v0 + edgeDir * (edgeLength * (double)(*it).index / num - grassSize / 2);
	//	V2d ABmax = v0 + edgeDir * (edgeLength * (double)((*it).index + reps) / num + grassSize / 2);
	//	double height = grassSize / 2;
	//	V2d normal = normalize(v1 - v0);
	//	double temp = -normal.x;
	//	normal.x = normal.y;
	//	normal.y = temp;

	//	A = ABmin + normal * height;
	//	B = ABmax + normal * height;
	//	C = ABmax;
	//	D = ABmin;

	//	Grass * g = new Grass(this, ts_grass, totalGrassIndex, A, B, C, D, this);

	//	owner->grassTree->Insert(g);

	//	totalGrassIndex++;
	//}
}

void GameSession::LoadDecorImages()
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
			//fullName.split

			Tileset *ts = tm.GetTileset(fullName, width, height);
			assert(ts != NULL);
			decorTSMap[name] = ts;
			//decorTileIndexes[name].push_back(tile);
		}

		is.close();
	}
	else
	{
		assert(0);
	}
}

GameController &GameSession::GetController( int index )
{
	return mainMenu->GetController( index );
}

void GameSession::ApplyToggleUpdates( int index )
{
	Actor *player = GetPlayer( index );
	if( player == NULL )
		return;

	ControllerState &pCurr = player->currInput;
	GameController &controller = GetController( index );
	ControllerState &currInput = GetCurrInput( index );
	ControllerState &prevInput = GetPrevInput( index );

				//ControllerState &pPrev = player->prevInput;
	bool alreadyBounce = pCurr.X;
	bool alreadyGrind = pCurr.Y;
	bool alreadyTimeSlow = pCurr.leftShoulder;
	player->currInput = currInput;
	if( controller.keySettings.toggleBounce )
	{
		if( currInput.X && !prevInput.X )
		{
			pCurr.X = !alreadyBounce;
		}
		else
		{
			pCurr.X = alreadyBounce;
		}
	}
	if( controller.keySettings.toggleGrind )
	{
		if( currInput.Y && !prevInput.Y )
		{
			pCurr.Y = !alreadyGrind;
			//cout << "pCurr.y is now: " << (int)pCurr.Y << endl;
		}
		else
		{
			pCurr.Y = alreadyGrind;
		}
	}
	if( controller.keySettings.toggleTimeSlow )
	{
		if( currInput.leftShoulder && !prevInput.leftShoulder )
		{
			pCurr.leftShoulder = !alreadyTimeSlow;
						
		}
		else
		{
			pCurr.leftShoulder = alreadyTimeSlow;
		}
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


void GameSession::DrawHealthFlies(sf::RenderTarget *target)
{
	if (numTotalFlies > 0)
	{
		target->draw(healthFlyVA, numTotalFlies * 4, sf::Quads, ts_healthFly->texture);
	}
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

	hitboxManager = new HitboxManager;

	
	shardsCapturedField = new BitField(32 * 5);
	

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
	mini = new Minimap(this);
	

	eHitParamsMan = new EnemyParamsManager;

	absorbParticles = new AbsorbParticles( this, AbsorbParticles::ENERGY );
	absorbDarkParticles = new AbsorbParticles( this, AbsorbParticles::DARK);
	absorbShardParticles = new AbsorbParticles(this, AbsorbParticles::SHARD);



	const ConfigData &cd = mainMenu->config->GetData();

	soundNodeList = new SoundNodeList(10);
	soundNodeList->SetSoundVolume(cd.soundVolume);

	pauseSoundNodeList = new SoundNodeList(10);
	pauseSoundNodeList->SetSoundVolume(cd.soundVolume);
	//pauseSoundNodeList->SetGlobalVolume(mainMenu->config->GetData().soundVolume);
	scoreDisplay = new ScoreDisplay(this, Vector2f(1920, 0), mainMenu->arial);


	if (!glowShader.loadFromFile("Resources/Shader/glow_shader.frag", sf::Shader::Fragment))
	{
		cout << "glow SHADER NOT LOADING CORRECTLY" << endl;
	}
	glowShader.setUniform("texSize", Vector2f(1920, 1080));

	if (!hBlurShader.loadFromFile("Resources/Shader/hblur_shader.frag", sf::Shader::Fragment))
	{
		cout << "hBlurShader SHADER NOT LOADING CORRECTLY" << endl;
	}
	hBlurShader.setUniform("texSize", Vector2f(1920 / 2, 1080 / 2));

	if (!vBlurShader.loadFromFile("Resources/Shader/vblur_shader.frag", sf::Shader::Fragment))
	{
		cout << "vBlurShader SHADER NOT LOADING CORRECTLY" << endl;
	}
	vBlurShader.setUniform("texSize", Vector2f(1920 / 2, 1080 / 2));

	if (!motionBlurShader.loadFromFile("Resources/Shader/motionblur_shader.frag", sf::Shader::Fragment))
	{
		cout << "motion blur SHADER NOT LOADING CORRECTLY" << endl;
	}
	motionBlurShader.setUniform("texSize", Vector2f(1920, 1080));

	if (!shockwaveShader.loadFromFile("Resources/Shader/shockwave_shader.frag", sf::Shader::Fragment))
	{
		cout << "shockwave SHADER NOT LOADING CORRECTLY" << endl;
	}
	shockwaveShader.setUniform("resolution", Vector2f(1920, 1080));
	shockwaveShader.setUniform("texSize", Vector2f(580, 580));
	shockwaveTex.loadFromFile("Resources/FX/shockwave_580x580.png");
	shockwaveShader.setUniform("shockwaveTex", shockwaveTex);

	

	if (!flowShader.loadFromFile("Resources/Shader/flow_shader.frag", sf::Shader::Fragment))
	{
		cout << "flow SHADER NOT LOADING CORRECTLY" << endl;
	}

	flowShader.setUniform("radDiff", radDiff);
	flowShader.setUniform("Resolution", Vector2f(1920, 1080));// window->getSize().x, window->getSize().y);
	flowShader.setUniform("flowSpacing", flowSpacing);
	flowShader.setUniform("maxFlowRings", maxFlowRadius / maxFlowRings);

	if (!cloneShader.loadFromFile("Resources/Shader/clone_shader.frag", sf::Shader::Fragment))
	{
		cout << "CLONE SHADER NOT LOADING CORRECTLY" << endl;
	}

	if (!ShouldContinueLoading())
	{
		cout << "cleanup blabhagbo4a" << endl;
		Cleanup();
		return false;
	}



	terrainBGTree = new QuadTree(1000000, 1000000);
	//soon make these the actual size of the bordered level
	terrainTree = new QuadTree(1000000, 1000000);

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


	soundManager = new SoundManager;

	cout << "weird timing 1" << endl;

	//sleep(5000);
	//return;
	

	//sets up fx so that they can be used
	for (int i = 0; i < MAX_EFFECTS; ++i)
	{
		AllocateEffect();
	}


	kinMapSpawnIcon.setTexture(*mini->ts_miniIcons->texture);
	kinMapSpawnIcon.setTextureRect(mini->ts_miniIcons->GetSubRect(1));
	kinMapSpawnIcon.setOrigin(kinMapSpawnIcon.getLocalBounds().width / 2,
		kinMapSpawnIcon.getLocalBounds().height / 2);


	goalMapIcon.setTexture(*mini->ts_miniIcons->texture);
	goalMapIcon.setTextureRect(mini->ts_miniIcons->GetSubRect(0));
	goalMapIcon.setOrigin(goalMapIcon.getLocalBounds().width / 2,
		goalMapIcon.getLocalBounds().height / 2);

	//blah

	keyMarker = new KeyMarker(this);

	ts_w1ShipClouds0 = GetTileset("Ship/cloud_w1_a1_960x128.png", 960, 128);
	ts_w1ShipClouds1 = GetTileset("Ship/cloud_w1_b1_960x320.png", 960, 320);
	ts_ship = GetTileset("Ship/ship_864x400.png", 864, 400);


	shipSprite.setTexture(*ts_ship->texture);
	shipSprite.setTextureRect(ts_ship->GetSubRect(0));
	//shipSprite.setScale(.5, .5);
	shipSprite.setOrigin(shipSprite.getLocalBounds().width / 2,
		shipSprite.getLocalBounds().height / 2);


	if (!ShouldContinueLoading())
	{
		cout << "cleanup A" << endl;
		Cleanup();
		
		return false;
	}

	cout << "weird timing 2" << endl;

	//blah 2

	gameSoundBuffers[S_KEY_COMPLETE_W1] = soundManager->GetSound( "key_complete_w1.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W2] = soundManager->GetSound( "key_complete_w2.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W3] = soundManager->GetSound( "key_complete_w2.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W4] = soundManager->GetSound( "key_complete_w2.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W5] = soundManager->GetSound( "key_complete_w2.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W6] = soundManager->GetSound( "key_complete_w6.ogg" );
	gameSoundBuffers[S_KEY_ENTER_0] = soundManager->GetSound( "key_enter_1.ogg" );
	gameSoundBuffers[S_KEY_ENTER_1] = soundManager->GetSound( "key_enter_1.ogg" );
	gameSoundBuffers[S_KEY_ENTER_2] = soundManager->GetSound( "key_enter_2.ogg" );
	gameSoundBuffers[S_KEY_ENTER_3] = soundManager->GetSound( "key_enter_3.ogg" );
	gameSoundBuffers[S_KEY_ENTER_4] = soundManager->GetSound( "key_enter_4.ogg" );
	gameSoundBuffers[S_KEY_ENTER_5] = soundManager->GetSound( "key_enter_5.ogg" );
	gameSoundBuffers[S_KEY_ENTER_6] = soundManager->GetSound( "key_enter_6.ogg" );

	//blah 3

	cutPlayerInput = false;
	activeEnvPlants = NULL;
	totalGameFrames = 0;	
	totalFramesBeforeGoal = -1;
	originalZone = NULL;
	
	inactiveEnemyList = NULL;
	cloneInactiveEnemyList = NULL;
	unlockedGateList = NULL;
	activatedZoneList = NULL;


	activeSequence = NULL;

	
	//still too far

	//window->setMouseCursorVisible( true );

	view = View( Vector2f( 300, 300 ), sf::Vector2f( 960 * 2, 540 * 2 ) );
	//cout << "weird timing 3" << endl;

	uiView = View( sf::Vector2f( 960, 540 ), sf::Vector2f( 1920, 1080 ) );

	
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
	players[0] = new Actor( this, 0 );

	
	cout << "about to open file" << endl;
	
	if (progressDisplay != NULL)
		progressDisplay->SetProgressString("opening map file!", 1);
	OpenFile( fileName );

	//background = new Background(this, mh->envLevel, mh->envType);
	Background::SetupFullBG(mh->envName, tm, background, scrollingBackgrounds);

	//still too far


	cout << "done opening file" << endl;
	int maxBubbles = 5;

	if( raceFight != NULL )
	{
		players[1] = new Actor( this, 1 );
		maxBubbles = 2;
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
	}

	m_numActivePlayers = 0;
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
	assert(activePlayer);
	//int maxBubbles = activePlayer->maxBubbles;

	fBubbleFrame = new float[5 * 4];
	for (int i = 0; i < 5 * 4; ++i)
	{
		fBubbleFrame[i] = 0;
	}
	fBubblePos = new sf::Vector2f[5 * 4];
	fBubbleRadiusSize = new float[5 * 4];

	int count = 0;
	tempPlayer = NULL;
	for (int i = 0; i < 4; ++i)
	{
		if (tempPlayer = GetPlayer(i))
		{
			tempPlayer->fBubbleFrame = (fBubbleFrame + i * 5);
			tempPlayer->fBubblePos = (fBubblePos + i * 5);
			tempPlayer->fBubbleRadiusSize = (fBubbleRadiusSize + i * 5);
			++count;
		}
	}



	Actor *p = NULL;
	for( int i = 0; i < 4; ++i )
	{
		p = GetPlayer( i );
		if( p != NULL )
			p->InitAfterEnemies();
	}

	/*for (auto it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		(*it)->Init();
	}*/

	
	//too far
	

	if( mh->gameMode == MapHeader::MapType::T_STANDARD )
	{ 
		recGhost = new RecordGhost(GetPlayer(0));

		adventureHUD = new AdventureHUD(this);
	}
	
	pauseMenu = mainMenu->pauseMenu;
	pauseMenu->owner = this;
	pauseMenu->SetTab(PauseMenu::PAUSE);
	//pauseMenu->SetTab( PauseMenu::Tab::KIN );

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
	for( int i = 0; i < 1; ++i )
	{
		//temporary
		//mainMenu->GetController(i).SetFilter( pauseMenu->cOptions->xboxInputAssoc[0] );
		currProfile = pauseMenu->controlSettingsMenu->pSel->currProfile;
		GameController &con = GetController(i);
		currProfile->tempCType = con.GetCType();
		con.SetFilter(currProfile->GetCurrFilter());//mainMenu->cpm->profiles.front()->filter );
	}

	goalPulse = new GoalPulse( this, Vector2f( goalPos.x, goalPos.y ) );

	int goalTile = -1;
	
	switch(mh->envWorldType)//mh->envType )
	{
	case 0:
		goalTile = 5;
		break;
	case 1:
		goalTile = 4;
		break;
	case 2:
		goalTile = 4;
		break;
	case 3:
		goalTile = 4;
		break;
	case 4:
		goalTile = 4;
		break;
	case 5:
		goalTile = 4;
		break;
	case 6:
		goalTile = 4;
		break;
	}
	assert( goalTile >= 0 );
	goalMapIcon.setTextureRect(mini->ts_miniIcons->GetSubRect( goalTile ) );

	float numSecondsToDrain = mh->drainSeconds;
	float drainPerSecond = GetPlayer(0)->totalHealth / numSecondsToDrain;
	float drainPerFrame = drainPerSecond / 60.f;
	float drainFrames = 1.f;
	if (drainPerFrame < 1.f)
	{
		drainFrames = floor(1.f / drainPerFrame);
	}
	GetPlayer(0)->drainCounterMax = drainFrames;
	if (drainPerFrame < 1.0)
		drainPerFrame = 1.0;
	GetPlayer(0)->drainAmount = drainPerFrame;

	/*if (mh->gameMode == MapHeader::MapType::T_STANDARD)
	{
		
	}*/

	if (hasGoal)
	{
		flowShader.setUniform("goalPos", Vector2f(goalPos.x, goalPos.y));
		goalEnergyFlowVA = SetupEnergyFlow();
	}
	else
	{
		goalEnergyFlowVA = NULL;
	}
	

	

	groundTrans = Transform::Identity;
	groundTrans.translate( 0, 0 );

	cam.pos.x = GetPlayer(0)->position.x;
	cam.pos.y = GetPlayer(0)->position.y;

	if (!ShouldContinueLoading())
	{
		cout << "cleanup FF" << endl;
		Cleanup();
		return false;
	}

	numPolyTypes = matSet.size();
	polyShaders = new Shader[numPolyTypes];
	terrainDecorInfos = new TerrainDecorInfo*[numPolyTypes];
	for (int i = 0; i < numPolyTypes; ++i)
	{
		terrainDecorInfos[i] = NULL;
	}
	ts_polyShaders = new Tileset*[numPolyTypes];

	cout << "progress more" << endl;

	map<pair<int,int>, int> indexConvert;
	int index = 0;
	for( set<pair<int,int>>::iterator it = matSet.begin(); it != matSet.end(); ++it )
	{
		if (!polyShaders[index].loadFromFile("Resources/Shader/mat_shader2.frag", sf::Shader::Fragment ) )
		{
			cout << "MATERIAL SHADER NOT LOADING CORRECTLY" << endl;
			assert( 0 && "polygon shader not loaded" );
			usePolyShader = false;
		}

		int matWorld = (*it).first;
		int matVariation = (*it).second;

		cout << "matWorld: " << matWorld << ", matvar: " << matVariation << endl;

		indexConvert[pair<int,int>(matWorld,matVariation)] = index;

		stringstream ss1;
		ss1 << "Terrain/terrain_";
		
		ss1 << matWorld + 1 << "_";
		if( matVariation < 10 )
		{
			ss1 << "0" << matVariation + 1;
		}
		else
		{
			ss1 << matVariation + 1;
		}
	
		ss1 << "_512x512.png";
		ts_polyShaders[index] = GetTileset( ss1.str(), 512, 512 ); //1024, 1024 );
		cout << "loading: " << ss1.str() << endl;
		polyShaders[index].setUniform( "u_texture", 
			//*GetTileset( ss1.str(), 1024, 1024 )->texture );
			*GetTileset( ss1.str(), 512, 512 )->texture );
		//polyShaders[tType]->setUniform( "u_texture", *(ts_poly->texture) );
		polyShaders[index].setUniform( "Resolution", Vector2f( 1920, 1080 ) );
		polyShaders[index].setUniform( "AmbientColor", Glsl::Vec4( 1, 1, 1, 1 ) );
		

		ifstream is;
		ss1.clear();
		ss1.str("");

		ss1 << "Resources/Terrain/Decor/" << "terraindecor_" 
			<< (matWorld + 1) << "_0" << (matVariation + 1) << ".txt";
		is.open(ss1.str());

		if (is.is_open())
		{
			list<pair<string, int>> loadedDecorList;
			while (!is.eof())
			{
				string dStr;
				is >> dStr;

				if (dStr == "")
				{
					break;
				}

				int frequencyPercent;
				is >> frequencyPercent;

				loadedDecorList.push_back(pair<string, int>(dStr, frequencyPercent));
			}
			is.close();


			TerrainDecorInfo *tdInfo = new TerrainDecorInfo(loadedDecorList.size());
			terrainDecorInfos[index] = tdInfo;
			int dI = 0;
			for (auto it = loadedDecorList.begin(); it != loadedDecorList.end(); ++it)
			{
				tdInfo->decors[dI] = TerrainRender::GetDecorType((*it).first);
				tdInfo->percents[dI] = (*it).second;
				++dI;
			}
		}
		else
		{
			//not found, thats fine.
		}

		++index;
	}

	for( list<TerrainPiece*>::iterator it = allVA.begin(); it != allVA.end(); ++it )
	{
		int realIndex = indexConvert[pair<int,int>((*it)->terrainWorldType,
		(*it)->terrainVariation)];
		//cout << "real index: " << realIndex << endl;
		(*it)->pShader = &polyShaders[realIndex];
		(*it)->ts_terrain = ts_polyShaders[realIndex];
		(*it)->tr->tdInfo = terrainDecorInfos[realIndex];
		(*it)->tr->GenerateDecor();
	}

	LevelSpecifics();
	
	


	//this is outdated!
	testPar = new Parallax();

	//Tileset *ts1a = GetTileset( "Parallax/w1_01a.png", 1920, 1080 );
	//Tileset *ts1b = GetTileset( "Parallax/w1_01b.png", 1920, 1080 );
	//Tileset *ts1c = GetTileset( "Parallax/w1_01c.png", 1920, 1080 );

	//testPar->AddRepeatingSprite( ts1a, 0, Vector2f( 0, 0 ), 1920 * 2, 80 );
	//testPar->AddRepeatingSprite( ts1a, 0, Vector2f( 1920, 0 ), 1920 * 2, 80 );

	//testPar->AddRepeatingSprite( ts1b, 0, Vector2f( 0, 0 ), 1920 * 2, 150 );
	//testPar->AddRepeatingSprite( ts1b, 0, Vector2f( 1920, 0 ), 1920 * 2, 150 );

	//testPar->AddRepeatingSprite( ts1c, 0, Vector2f( 0, 0 ), 1920 * 2, 250 );
	//testPar->AddRepeatingSprite( ts1c, 0, Vector2f( 1920, 0 ), 1920 * 2, 250 );

	
	cout << "last one" << endl;
	for( auto it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it )
	{
		(*it)->Setup();
	}


	cout << "done loading" << endl;

	if (progressDisplay != NULL)
		progressDisplay->SetProgressString("done loading!", 0);

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

#include "StorySequence.h"
int GameSession::Run()
{
	ClearEmitters();
	bool oldMouseGrabbed = mainMenu->GetMouseGrabbed();
	bool oldMouseVisible = mainMenu->GetMouseVisible();

	mainMenu->SetMouseGrabbed(true);
	mainMenu->SetMouseVisible(false);

	currStorySequence = NULL;
	currBroadcast = NULL;

	View oldPreTexView = preScreenTex->getView();
	View oldWindowView = window->getView();
	
	//showHUD = true;

	preScreenTex->setView(view);
	
	bool showFrameRate = true;
	bool showRunningTimer = true;

	sf::Text frameRate("00", mainMenu->arial, 30);
	frameRate.setFillColor(Color::Red);

	sf::Text runningTimerText( "---- : --", mainMenu->arial, 30 );
	runningTimerText.setFillColor(Color::Red);
	runningTimerText.setOrigin(runningTimerText.getLocalBounds().left +
		runningTimerText.getLocalBounds().width, 0 );
	runningTimerText.setPosition(1920 - 30, 10);

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


	bool skipped = false;
	bool oneFrameMode = false;
	quit = false;


	
	vector<GCC::GCController> controllers;
	if (mainMenu->gccDriverEnabled)
		controllers = mainMenu->gccDriver->getState();
	for (int i = 0; i < 4; ++i)
	{
		GameController &c = GetController(i);
		if (mainMenu->gccDriverEnabled)
			c.gcController = controllers[i];
		c.UpdateState();
		GetCurrInput(i) = GetController(i).GetState();
	}

	bool t = GetCurrInput(0).start;//sf::IsKeyPressed( sf::Keyboard::Y );
	bool s = t;
	t = false;

	int returnVal = GR_EXITLEVEL;

	ts_gravityGrass = GetTileset("Env/gravity_grass_128x128.png", 128, 128);
	

	goalDestroyed = false;


	debugScreenRecorder = NULL;

	//debugScreenRecorder = new ScreenRecorder("BACKWARDS_DASH_JUMP");

	View v;
	v.setCenter(0, 0);
	v.setSize(1920 / 2, 1080 / 2);
	window->setView(v);
	lastFrameTex->setView(v);
	
	int frameCounter = 0;
	int frameCounterWait = 20;
	double total = 0;

	cloudView = View(Vector2f(0, 0), Vector2f(1920, 1080));

	int flowSize = 64;

	if (raceFight != NULL)
	{
		raceFight->victoryScreen->Reset();
		state = RUN;//RACEFIGHT_RESULTS;
		raceFight->place[0] = 1;
		raceFight->place[1] = 2;

		raceFight->raceFightResultsFrame = 0;
		raceFight->victoryScreen->SetupColumns();
	}
	else
	{
		state = RUN;
	}

	//Rain rain(this);
	sf::View rainView(Vector2f(0, 0), Vector2f(1920, 1080));

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
	bool switchState = false;

	if (GetPlayer(0)->action == Actor::INTROBOOST)
	{
		//Fade(true, 60, Color::Black, true);
	}

	while( !quit )
	{
		switchState = false;
		double newTime = gameClock.getElapsedTime().asSeconds();
		double frameTime = newTime - currentTime;

		if ( frameTime > 0.25 )
		{
			frameTime = 0.25;	
		}
		//frameTime = 0.167;//0.25;	
        currentTime = newTime;

		if( showFrameRate )
		{
			if( frameCounter == frameCounterWait )
			{
				double blah = 1.0 / frameTime;
				total += blah;
				ss << total / ( frameCounterWait + 1 ) ;
				frameRate.setString( ss.str() );
				ss.clear();
				ss.str( "" );
				frameCounter = 0;
				total = 0;

				
			}
			else
			{
				double blah = 1.0 / frameTime;
				total += blah;
				++frameCounter;
			}

			/*ss << inGameClock.getElapsedTime().asSeconds();
			frameRate.setString( ss.str() );
			ss.clear();
			ss.str("");*/
		}

		if (showRunningTimer && !scoreDisplay->active)
		{
			int tFrames = totalGameFrames;
			if (totalFramesBeforeGoal >= 0)
			{
				tFrames = totalFramesBeforeGoal;
			}
			runningTimerText.setString(GetTimeStr(tFrames));
			
		}
		//cout << "frameCounter: " << frameCounter << endl;
		//for( list<Tileset*>::iterator it = tilesetList.begin(); it != tilesetList.end(); ++it )
		//{
			//cout << "testt: " << (*it)->sourceName << ", "
			//	<< (*it)->tileWidth << ", " << (*it)->tileWidth << endl;
			//delete (*it);
		//}

		if( state == RUN )
		{
			
		
		accumulator += frameTime;

		

		window->clear(Color::Red);

		preScreenTex->clear(Color::Red);
		postProcessTex->clear(Color::Red);
		postProcessTex1->clear(Color::Red);
		postProcessTex2->clear(Color::Red);
		
		switch (mh->bossFightType)
		{
		case 0:
			break;
		case 1:
			break;
		}
		
		coll.ClearDebug();
		
		while ( accumulator >= TIMESTEP )
        {
		//	cout << "currInputleft: " << currInput.leftShoulder << endl;
			bool skipInput = IsKeyPressed( sf::Keyboard::PageUp );
			if( oneFrameMode )
			{
				//controller.UpdateState();
				

				ControllerState con;

				ControllerState con2;
				//con = controller.GetState();
				
				
				bool tookScreenShot = false;
				bool screenShot = false;
				
				while( true )
				{
					//prevInput = currInput;
					//player->prevInput = currInput;

					vector<GCC::GCController> controllers;
					if (mainMenu->gccDriverEnabled)
						controllers = mainMenu->gccDriver->getState();

					for (int i = 0; i < 4; ++i)
					{
						GameController &c = GetController(i);
						if (mainMenu->gccDriverEnabled)
							c.gcController = controllers[i];
						c.UpdateState();
					}
					/*controller.UpdateState();
					con = controller.GetState();

					if( controller2 != NULL )
					{
						controller2->UpdateState();
						con2 = controller2->GetState();
					}*/

					//UpdateControllers();

					

					
					
					//player->currInput = currInput;
					skipInput = IsKeyPressed( sf::Keyboard::PageUp );
					
					bool stopSkippingInput = IsKeyPressed( sf::Keyboard::PageDown );
					screenShot = false;//IsKeyPressed( sf::Keyboard::F );// && !tookScreenShot;
					
					if( screenShot )
					{
						//cout << "TOOK A SCREENSHOT" << endl;
						//tookScreenShot = true;
						//Image im = window->capture();

						// time_t now = time(0);
						// char* dt = ctime(&now);
						//im.saveToFile( "screenshot.png" );//+ string(dt) + ".png" );
					}
					else
					{
						if( skipInput )
						{
							tookScreenShot = false;
						}
					}
					

					if( !skipped && skipInput )//sf::IsKeyPressed( sf::Keyboard::K ) && !skipped )
					{
						skipped = true;
						accumulator = 0;//TIMESTEP;
						
						//currentTime = gameClock.getElapsedTime().asSeconds() - TIMESTEP;

						break;
					}
					if( skipped && !skipInput )//!sf::IsKeyPressed( sf::Keyboard::K ) && skipped )
					{
						skipped = false;
						//break;
					}
					if( IsKeyPressed( sf::Keyboard::L ) )
					{

						//oneFrameMode = false;
						break;
					}
					//if( sf::IsKeyPressed( sf::Keyboard::M ) )
					if( stopSkippingInput )
					{

						oneFrameMode = false;
						break;
					}
					

				}

				window->clear();
			}
			else
			{
				
			}

			if( skipInput )
				oneFrameMode = true;

			bool k = IsKeyPressed( sf::Keyboard::K );
			bool levelReset = IsKeyPressed( sf::Keyboard::L );
			Enemy *monitorList = NULL;
			//if( player->action != Actor::GOALKILLWAIT && player->action != Actor::GOALKILL && player->action != Actor::EXIT && ( k || levelReset || player->dead /*|| (currInput.start && !prevInput.start )*/ ) )
			{
				//levelReset = true;
				//RestartLevel();
			}

			if( IsKeyPressed( sf::Keyboard::Y ) )// || currInput.start )
			{
				quit = true;
				break;
			}

			if (IsKeyPressed(sf::Keyboard::Num9))
			{
				showRunningTimer = true;
			}
			if (IsKeyPressed(sf::Keyboard::Num0))
			{
				showRunningTimer = false;
			}
	
			/*if( IsKeyPressed( sf::Keyboard::Escape ) )
			{


				quit = true;
				returnVal = GR_EXITLEVEL;

				break;
			}*/

			if( goalDestroyed )
			{
				quit = true;
				
				//returnVal = GR_WIN;
				returnVal = resType;
				break;
			}

			if (nextFrameRestart)
			{
				state = GameSession::RUN;
				RestartLevel();
				gameClock.restart();
				currentTime = 0;
				accumulator = TIMESTEP + .1;
				frameCounter = 0;
			}
			
			
			
			//lastFrameTex->display();
			

			if( pauseFrames == 0 )
			{
				for( int i = 0; i < 4; ++i )
				{
					GetPrevInput( i ) = GetCurrInput( i );
					GetPrevInputUnfiltered(i) = GetCurrInputUnfiltered(i);
				}
			
			

			if( !cutPlayerInput )
			{

				Actor *pTemp = NULL;
				for( int i = 0; i < 4; ++i )
				{
					pTemp = GetPlayer(i);
					if (pTemp != NULL)
					{
						pTemp->prevInput = GetCurrInput(i);
					}		
				}
			}

			vector<GCC::GCController> controllers;
			if (mainMenu->gccDriverEnabled)
				controllers = mainMenu->gccDriver->getState();

			for( int i = 0; i < 1; ++i )
			{
				GameController &con = GetController( i );

				if (mainMenu->gccDriverEnabled)
					con.gcController = controllers[i];


				bool canControllerUpdate = con.UpdateState();
				if( !canControllerUpdate )
				{
					//KeyboardUpdate( 0 );
				}
				else
				{
					ControllerState &currInput = GetCurrInput(i);
					ControllerState &conState = con.GetState();
					currInput = conState;
					GetCurrInputUnfiltered(i) = con.GetUnfilteredState();
				}
			}


			//currently only records 1 player replays. fix this later
			if( repPlayer != NULL )//repPlayer->init )
			{
				//cout << "replay input" << repPlayer->frame << endl;
				repPlayer->UpdateInput( GetCurrInput( 0 ) );
				//repPlayer->up
			}

			if( recPlayer != NULL )
			{
				//cout << "record player " << recPlayer->frame << endl;
				recPlayer->RecordFrame();
			}

			


			if( !cutPlayerInput )
			{
				for( int i = 0; i < 4; ++i )
				{
					ApplyToggleUpdates( i );
				}
				//else
			}

			}
			else if( pauseFrames > 0 )
			{
				for (int i = 0; i < 4; ++i)
				{
					GetPrevInput(i) = GetCurrInput(i);
				}

				if (!cutPlayerInput)
				{

					Actor *pTemp = NULL;
					for (int i = 0; i < 4; ++i)
					{
						pTemp = GetPlayer(i);
						if (pTemp != NULL)
							pTemp->prevInput = GetCurrInput(i);
					}
				}

				vector<GCC::GCController> controllers;
				if (mainMenu->gccDriverEnabled)
					controllers = mainMenu->gccDriver->getState();
				
				for (int i = 0; i < 4; ++i)
				{
					GameController &con = GetController(i);
					if (mainMenu->gccDriverEnabled)
						con.gcController = controllers[i];
					bool canControllerUpdate = con.UpdateState();
					if (!canControllerUpdate)
					{
						//KeyboardUpdate( 0 );
					}
					else
					{
						con.UpdateState();
						GetCurrInput(i) = con.GetState();
					}
				}

				if (!cutPlayerInput)
				{
					for (int i = 0; i < 4; ++i)
					{
						ApplyToggleUpdates(i);
					}
					//else
				}

				Actor *pTemp = NULL;
				for (int i = 0; i < 4; ++i)
				{
					pTemp = GetPlayer(i);
					if (pTemp != NULL)
					{
						pTemp->UpdateInHitlag();
					}
				}
				//if( player->changingClone )
				//{
				//	player->percentCloneChanged += player->percentCloneRate;
				//	//if( player->percentCloneChanged >= 1 )
				//	{
				//		player->percentCloneChanged = 0;
				//	//	player->percentCloneChanged = 1;
				//		player->changingClone = false;
				//		pauseFrames = 0;
				//	}

				//	//pauseFrames--;
				//	accumulator -= TIMESTEP;
				//	//break;
				//	continue;
				//}
				Actor *p = NULL;
				for( int i = 0; i < 4; ++i )
				{
					 p = GetPlayer( i );
					 if( p != NULL )
						p->flashFrames--;
				}
				
				for (int i = 0; i < EffectLayer::Count; ++i)
				{
					Enemy *& fxList = effectLists[i];
					BasicEffect *c = (BasicEffect*)fxList;
					BasicEffect *tNext;
					while (c != NULL)
					{
						tNext = (BasicEffect*)c->next;
						if (c->pauseImmune)
						{
							c->UpdatePrePhysics();
							c->UpdatePostPhysics();
						}
						c = tNext;
					}
				}

				cam.UpdateRumble();
				

				////view fx that are outside of hitlag pausing
				//Enemy *currFX = activeEnemyList;
				//while( currFX != NULL )
				//{
				//	if( currFX->type == EnemyType::EN_BASICEFFECT )
				//	{
				//		BasicEffect * be = (BasicEffect*)currFX;
				//		if( be->pauseImmune )
				//		{
				//			currFX->UpdatePostPhysics();
				//		}
				//	}
				//	
				//	currFX = currFX->next;
				//}

				fader->Update();
				swiper->Update();
				mainMenu->UpdateEffects();
				

				pauseFrames--;
				//accumulator = 0;
				
				accumulator -= TIMESTEP;

				//if( recGhost != NULL)
				//recGhost->RecordFrame();
				

				//repGhost->UpdateReplaySprite();

				//currentTime = gameClock.getElapsedTime().asSeconds();
				//break;
				continue;
			}
 
			if( deathWipe )
			{
				deathWipeFrame++;
				if( deathWipeFrame == deathWipeLength )
				{
					deathWipe = false;
					deathWipeFrame = 0;
				}
			}

			if( activeSequence != NULL )// && activeSequence == startSeq )
			{
				State oldState = state;
				if (!activeSequence->Update())
				{
					state = RUN;
					activeSequence = NULL;
				}
				else
				{
					if (state != oldState)
					{
						switchState = true;
						break;
						//goto starttest;
					}
				}

				if (state == SEQUENCE)
				{

				}
			}
			if( activeDialogue != NULL )
			{
				if( GetCurrInput( 0 ).A && !GetPrevInput( 0 ).A )
				{
					if( activeDialogue->ConfirmDialogue() )
						activeDialogue = NULL;
					//activeDialogue->
				}
			}
			//else
			{
				//cout << "-----------updating total frames------" << endl;
				//cout << "before count: " << CountActiveEnemies() << endl;
				//testBuf.Send( totalGameFrames );
				int blafah = totalGameFrames % 60;
				float fafa = blafah / 60.f;
				//shaderTester.Update();

				totalGameFrames++;
				for( int i = 0; i < 4; ++i )
				{
					p = GetPlayer( i );
					if( p != NULL )
						p->UpdatePrePhysics();
				}

				if (currStorySequence != NULL)
				{
					if (!currStorySequence->Update(GetPrevInput(0), GetCurrInput(0)))
					{
						currStorySequence->EndSequence();
						currStorySequence = NULL;
					}
					else
					{
					}
				}

				if (currBroadcast != NULL)
				{
					if (!currBroadcast->Update() )
					{
						//currStorySequence->EndSequence();
						currBroadcast = NULL;
					}
					else
					{
					}
				}

				if (inputVis != NULL)
					inputVis->Update(GetPlayer(0)->currInput);

				UpdateEnemiesPrePhysics();

				UpdateEnemiesPhysics();

				RecordReplayEnemies();

				for( int i = 0; i < 4; ++i )
				{
					p = GetPlayer( i );
					if( p != NULL )
						p->UpdatePostPhysics();
				}

				switch (mh->bossFightType)
				{
				case 0:
					break;
				case 1:
				{
					/*PoiInfo *pi = poiMap["nexuscore"];
					assert(pi != NULL);
					double halfX = 480 * pi->cameraZoom;
					double halfY = 270 * pi->cameraZoom;
					V2d A(pi->pos.x - halfX, pi->pos.y - halfY);
					V2d B(pi->pos.x + halfX, pi->pos.y - halfY);
					V2d C(pi->pos.x + halfX, pi->pos.y + halfY);
					V2d D(pi->pos.x - halfX, pi->pos.y + halfY);
					V2d pPos = GetPlayer(0)->position;
					if (QuadContainsPoint(A, B, C, D, pPos))
					{
						if (!cam.manual)
						{
							cam.manual = true;
							cam.Ease(Vector2f(pi->pos), pi->cameraZoom, 120, CubicBezier());
						}
					}
					else
					{
						if (cam.manual)
						{
							cam.EaseOutOfManual(60);
						}
					}*/
					break;
				}
				}


				if( recGhost != NULL )
					recGhost->RecordFrame();

				//cout << "replaying ghost: " << repGhost->frame << endl;
				//if( re
				/*if( repGhost != NULL )
					repGhost->UpdateReplaySprite();*/

				for (auto it = replayGhosts.begin(); it != replayGhosts.end(); ++it)
				{
					(*it)->UpdateReplaySprite();
				}

				if( goalDestroyed )
				{
					quit = true;
					//returnVal = GR_WIN;
					returnVal = resType;
					
					/*recGhost->StopRecording();
					recGhost->WriteToFile( "testghost.bghst" );*/
					break;
				}


				for( int i = 0; i < 4; ++i )
				{
					p = GetPlayer( i );
					if( p != NULL )
					{
						if( p->hasPowerLeftWire )
							p->leftWire->UpdateQuads();

						if( p->hasPowerRightWire )
							p->rightWire->UpdateQuads();
					}
				}

				UpdateEnemiesPostPhysics();
				
				for( int i = 0; i < numGates; ++i )
				{
					gates[i]->Update();
				}

				absorbParticles->Update();
				absorbDarkParticles->Update();
				absorbShardParticles->Update();

				UpdateEffects();
				UpdateEmitters();

				keyMarker->Update();

				mini->Update();

				mainMenu->musicPlayer->Update();

				if( adventureHUD != NULL )
					adventureHUD->Update();

				scoreDisplay->Update();

				soundNodeList->Update();

				pauseSoundNodeList->Update();

				goalPulse->Update();

				if (rain != NULL)
					rain->Update();


				oldZoom = cam.GetZoom();
				oldCamBotLeft = view.getCenter();
				oldCamBotLeft.x -= view.getSize().x / 2;
				oldCamBotLeft.y += view.getSize().y / 2;

				oldView = view;


				//polyShader.setUniform( "oldZoom", cam.GetZoom() );
				//polyShader.setUniform( "oldBotLeft", view.getCenter().x - view.getSize().x / 2, 
				//	view.getCenter().y + view.getSize().y / 2 );


				if (raceFight != NULL)
				{
					cam.UpdateVS(GetPlayer(0), GetPlayer(1));
				}
				else
				{
					cam.Update(GetPlayer(0));
				}

				/*if (mh->bossFightType == 0)
				{
					if (raceFight != NULL)
					{
						cam.UpdateVS(GetPlayer(0), GetPlayer(1));
					}
					else
					{
						cam.Update(GetPlayer(0));
					}
				}
				else if( mh->bossFightType > 0 )
				{
					cam.UpdateBossFight( mh->bossFightType );
				}
				*/
				

				Vector2f camPos = cam.GetPos();
				//cout << "in game cam pos: " << camPos.x << ", " << camPos.y << endl;

				if (totalGameFrames % 60 == 0)
				{

				}


				for( list<Barrier*>::iterator it = barriers.begin();
					it != barriers.end(); ++it )
				{
					bool trig = (*it)->Update( GetPlayer( 0 ) );
					if( trig )
					{
						TriggerBarrier( (*it) );
					}
				}

				fader->Update();
				swiper->Update();
				background->Update();
				if( topClouds != NULL )
					topClouds->Update();
				//rain.Update();

				mainMenu->UpdateEffects();

				testPar->Update( camPos );

				if( raceFight != NULL )
				{
					raceFight->UpdateScore();
				}

				for( list<ScrollingBackground*>::iterator it = scrollingBackgrounds.begin();
					it != scrollingBackgrounds.end(); ++it )
				{
					(*it)->Update( camPos );
				}

				if( shipSequence )
				{
					


					float oldLeft = cloud0[0].position.x;
					float blah = 30.f;
					float newLeft = oldLeft - blah; //cloudVel.x;
					float diff = ( shipStartPos.x - 480 ) - newLeft;
					if( diff >= 480 )
					{
						//cout << "RESETING: " << diff << endl;
						newLeft = shipStartPos.x - 480 - ( diff - 480 );
					}
					else
					{
						//cout << "diff: " << diff << endl;
					}

					float allDiff = newLeft - oldLeft;
					Vector2f cl = relShipVel;

					middleClouds.move( Vector2f( 0, cl.y ) );// + Vector2f( allDiff, 0 ) );
					for( int i = 0; i < 3 * 4; ++i)
					{
						cloud0[i].position = cl + Vector2f( cloud0[i].position.x + allDiff, cloud0[i].position.y );
						cloud1[i].position = cl + Vector2f( cloud1[i].position.x + allDiff, cloud1[i].position.y );

						cloudBot0[i].position = cl + Vector2f( cloudBot0[i].position.x + allDiff, cloudBot0[i].position.y );
						cloudBot1[i].position = cl + Vector2f( cloudBot1[i].position.x + allDiff, cloudBot1[i].position.y );

					}

					if( shipSeqFrame >= 90 && shipSeqFrame <= 180 )
					{
						int tFrame = shipSeqFrame - 90;
						shipSprite.setPosition( shipSprite.getPosition() + relShipVel );

						relShipVel += Vector2f( .3, -.8 );
					}
					else if( shipSeqFrame == 240 )//121 )
					{
						Actor *player = GetPlayer( 0 );
						//cout << "relshipvel: " << relShipVel.x << ", " << relShipVel.y << endl;
						player->action = Actor::JUMP;
						player->frame = 1;
						player->velocity = V2d( 20, 10 );
						player->UpdateSprite();
						shipSequence = false;
						player->hasDoubleJump = false;
						player->hasAirDash = false;
						player->hasGravReverse = false;
						drain = true;
					}

					++shipSeqFrame;
				}

				double camWidth = 960 * cam.GetZoom();
				double camHeight = 540 * cam.GetZoom();
				
				screenRect = sf::Rect<double>( camPos.x - camWidth / 2, camPos.y - camHeight / 2, camWidth, camHeight );

				if (totalGameFrames % 60 == 0)
				{
					float ddepth = .7;
					//ActivateEffect(EffectLayer::IN_FRONT, GetTileset("hit_spack_1_128x128.png", 128, 128), V2d(camPos / ddepth), true, 0, 10, 10, true, ddepth);
				}

				//flowShader.setUniform( "radius0", flow
				
				UpdateGoalFlow();
				
				int speedLevel = p0->speedLevel;
				float quant = 0;
				if( speedLevel == 0 )
				{
					quant = (float)(p0->currentSpeedBar / p0->level1SpeedThresh);
				}
				else if( speedLevel == 1 )
				{
					quant = (float)((p0->currentSpeedBar-p0->level1SpeedThresh) / ( p0->level2SpeedThresh - p0->level1SpeedThresh) );
				}
				else 
				{
					quant = (float)((p0->currentSpeedBar-p0->level2SpeedThresh) / ( p0->maxGroundSpeed - p0->level2SpeedThresh) );
				}

				//cout << "quant: " << quant << endl;

				queryMode = "enemy";

				sf::Rect<double> spawnRect = screenRect;
				double spawnExtra = 600;//800
				spawnRect.left -= spawnExtra;
				spawnRect.width += 2 * spawnExtra;
				spawnRect.top -= spawnExtra;
				spawnRect.height += 2 * spawnExtra;

				tempSpawnRect = spawnRect;
				enemyTree->Query( this, spawnRect);

				//if( player->blah || player->record > 1 )
				//{
				//	int playback = player->recordedGhosts;
				//	if( player->record > 1 )
				//		playback--;

				//	for( int i = 0; i < playback; ++i )
				//	{
				//		PlayerGhost *g = player->ghosts[i];
				//		if( player->ghostFrame < g->totalRecorded )
				//		{
				//			//cout << "querying! " << player->ghostFrame << endl;
				//			tempSpawnRect = g->states[player->ghostFrame].screenRect;
				//			enemyTree->Query( this, g->states[player->ghostFrame].screenRect );
				//		}
				//	}
				//}
				
				EnvPlant *prevPlant = NULL;
				EnvPlant *ev = activeEnvPlants;
				while( ev != NULL )
				{
					EnvPlant *tempNext = ev->next;
					ev->particle->Update( p0->position );

					ev->frame++;
					if( ev->frame == ev->disperseLength * ev->disperseFactor )
					{
						VertexArray &eva = *ev->va;
						eva[ev->vaIndex+0].position = Vector2f(0,0);
						eva[ev->vaIndex+1].position = Vector2f(0,0);
						eva[ev->vaIndex+2].position = Vector2f(0,0);
						eva[ev->vaIndex+3].position = Vector2f(0,0);

						if( ev == activeEnvPlants )
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

				queryMode = "envplant";
				envPlantTree->Query( this, screenRect );

				while( listVA != NULL )
				{
					TerrainPiece *t = listVA->next;
					listVA->next = NULL;
					listVA = t;
				}

				//listVA is null here
				queryMode = "border";
				numBorders = 0;
				borderTree->Query( this, screenRect );

				drawInversePoly = ScreenIntersectsInversePoly( screenRect );

				UpdateDecorSprites();
				

				TerrainRender::UpdateDecorLayers();

				for( map<DecorType,DecorLayer*>::iterator mit =
					decorLayerMap.begin(); mit != decorLayerMap.end();
					++mit )
				{
					(*mit).second->Update();
				}
				//TerrainPiece::UpdateBushFrame();

				//hacky
				if( p0->dead )
				{
					RestartLevel();
				}
				else if( raceFight != NULL )
				{
					if( raceFight->gameOver || GetCurrInput( 0 ).back )
					{
						state = RACEFIGHT_RESULTS;
						raceFight->raceFightResultsFrame = 0;
						raceFight->victoryScreen->Reset();
						raceFight->victoryScreen->SetupColumns();
						break;
					}
				}
				else if( !p0->IsGoalKillAction( p0->action ) && !p0->IsExitAction(p0->action) )
				{
					ControllerState &currInput = GetCurrInput( 0 );
					ControllerState &prevInput = GetPrevInput( 0 );
					//if( IsKeyPressed( Keyboard ) )
					if( currInput.start && !prevInput.start )
					{
						state = PAUSE;
						pauseSoundNodeList->ActivateSound(soundManager->GetSound("pause_on"));
						pauseMenu->SetTab( PauseMenu::PAUSE );
						soundNodeList->Pause( true );


					}
					else if( ( currInput.back && !prevInput.back ) || IsKeyPressed( Keyboard::G ) )
					{
						state = PAUSE;
						pauseMenu->SetTab( PauseMenu::MAP );
						pauseSoundNodeList->ActivateSound(soundManager->GetSound("pause_on"));
						soundNodeList->Pause( true );
					}
				}
				
				

				/*if( player->record > 0 )
				{
					player->ghosts[player->record-1]->states[player->ghosts[player->record-1]->currFrame].screenRect =
						screenRect;
				}*/
			}
			

			accumulator -= TIMESTEP;

			if (debugScreenRecorder != NULL)
			{
				break; //for recording stuff
			}
		}

		if (switchState && state != FROZEN)
		{
			continue;
		}
		//window->display();
		//continue;

		if( debugScreenRecorder != NULL )
		if( IsKeyPressed( Keyboard::R ) )
		{
			debugScreenRecorder->StartRecording();
			//player->maxFallSpeedSlo += maxFallSpeedFactor;
			//cout << "maxFallSpeed : " << player->maxFallSpeed << endl;
		}

		
		


		sf::Event ev;
		while( window->pollEvent( ev ) )
		{
			if( ev.type == Event::MouseWheelMoved )
			{
				if( ev.mouseWheel.delta > 0 )
				{
					zoomMultiple /= 2;
				}
				else if( ev.mouseWheel.delta < 0 )
				{
					zoomMultiple *= 2;
				}
				
				if( zoomMultiple < 1 )
				{
					zoomMultiple = 1;
				}
				else if( zoomMultiple > 65536 )
				{
					zoomMultiple = 65536;
				}
			}
			else if( ev.type == Event::LostFocus )
			{
				if( state == RUN )
					state = PAUSE;
			}
			else if( ev.type == sf::Event::GainedFocus )
			{
				//if( state == PAUSE )
				//	state = RUN;
			}
		}
		Vector2f camOffset;

		Vector2f camPos = cam.GetPos();
		view.setSize(Vector2f(1920/2 * cam.GetZoom(), 1080/2 * cam.GetZoom()));

		//this is because kin's sprite is 2x size in the game as well as other stuff
		lastViewSize = view.getSize();
		view.setCenter( camPos.x, camPos.y );

		lastViewCenter = view.getCenter();

		if (hasGoal)
		{
			flowShader.setUniform("topLeft", Vector2f(view.getCenter().x - view.getSize().x / 2,
				view.getCenter().y + view.getSize().y / 2));
		}
		

		background->Draw(preScreenTex);
		
		preScreenTex->setView( view );

		for( list<ScrollingBackground*>::iterator it = scrollingBackgrounds.begin();
			it != scrollingBackgrounds.end(); ++it )
		{
			(*it)->Draw( preScreenTex );
		}
		
		cloudView.setCenter( 960, 540 );
		cloudView.setCenter( 960, 540 );	
		preScreenTex->setView( cloudView );
		
		
		preScreenTex->setView( view );
			

		UpdateEnvShaders();
		
		DrawTopClouds();

		DrawBlackBorderQuads();
		
		DrawStoryLayer(EffectLayer::BEHIND_TERRAIN);
		DrawActiveSequence(EffectLayer::BEHIND_TERRAIN);
		DrawEffects( EffectLayer::BEHIND_TERRAIN );
		DrawEmitters(EffectLayer::BEHIND_TERRAIN);
		

		DrawZones();

		
		DrawTerrainPieces(listVA);
		
		DrawGoal();

		DrawGates();

		DrawRails();


		DrawDecorBetween();

		DrawStoryLayer(EffectLayer::BEHIND_ENEMIES);
		DrawActiveSequence(EffectLayer::BEHIND_ENEMIES);
		DrawEffects( EffectLayer::BEHIND_ENEMIES );
		DrawEmitters(EffectLayer::BEHIND_ENEMIES);

		UpdateEnemiesDraw();
		
		if (activeSequence != NULL)
		{
			activeSequence->Draw(preScreenTex);
		}

		DrawStoryLayer(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES);
		DrawActiveSequence(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES);
		DrawEffects( EffectLayer::BETWEEN_PLAYER_AND_ENEMIES );
		DrawEmitters(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES);

		goalPulse->Draw( preScreenTex );

		DrawPlayerWires();
		

		if( shipSequence )
		{
			preScreenTex->draw( cloud1, ts_w1ShipClouds1->texture );
			preScreenTex->draw( cloud0, ts_w1ShipClouds0->texture );
			preScreenTex->draw( middleClouds );
			preScreenTex->draw( cloudBot1, ts_w1ShipClouds1->texture );
			preScreenTex->draw( cloudBot0, ts_w1ShipClouds0->texture );
			preScreenTex->draw( shipSprite );
		}

		DrawHitEnemies(); //whited out hit enemies

		absorbParticles->Draw(preScreenTex);


		DrawPlayers();
		
		absorbShardParticles->Draw(preScreenTex);
		
		DrawReplayGhosts();
		
		DrawStoryLayer(EffectLayer::IN_FRONT);
		DrawActiveSequence(EffectLayer::IN_FRONT);
		DrawEffects( EffectLayer::IN_FRONT );
		DrawEmitters(EffectLayer::IN_FRONT);

		if( ts_basicBullets != NULL )
		{
			preScreenTex->draw( *bigBulletVA, ts_basicBullets->texture );
		}

		rainView.setCenter( (int)view.getCenter().x % 64, (int)view.getCenter().y % 64 );
		rainView.setSize( view.getSize() );
		preScreenTex->setView( rainView );

		if (rain != NULL)
			rain->Draw(preScreenTex);

		preScreenTex->setView( view );
		
		for( list<MovingTerrain*>::iterator it = movingPlats.begin(); it != movingPlats.end(); ++it )
		{
			(*it)->Draw( preScreenTex );
		}

		//DrawActiveEnvPlants();

		UpdateDebugModifiers();

		DebugDraw();
		

		if( false )
		{
			Sprite blah;
			blah.setTexture( preScreenTex->getTexture() );
			postProcessTex2->draw( blah );
			//postProcessTex2->clear( Color::Red );
			postProcessTex2->display();



			Vector2f shockSize( 580/2, 580/2 );
			sf::RectangleShape rectPost( shockSize );
			rectPost.setOrigin( rectPost.getLocalBounds().width / 2, rectPost.getLocalBounds().height / 2 );
			rectPost.setPosition( p0->position.x, p0->position.y ); //testing for now

			Sprite shockSprite;
			shockSprite.setTexture( shockwaveTex );
			shockSprite.setOrigin( shockSprite.getLocalBounds().width / 2, shockSprite.getLocalBounds().height / 2 );
			shockSprite.setPosition( p0->position.x, p0->position.y );
			//rectPost.setPosition( 0, 0 );

			Vector2f botLeft( view.getCenter().x - view.getSize().x / 2, view.getCenter().y + view.getSize().y );

			shockwaveShader.setUniform( "underTex", postProcessTex2->getTexture() );
			shockwaveShader.setUniform( "shockSize", Vector2f( 580, 580 ) );
			shockwaveShader.setUniform( "botLeft", Vector2f( rectPost.getPosition().x - rectPost.getSize().x / 2 - botLeft.x, 
				rectPost.getPosition().y - rectPost.getSize().y / 2 + rectPost.getSize().y - botLeft.y ) );
			shockwaveShader.setUniform( "zoom", cam.GetZoom() );
			//preScreenTex->draw( shockSprite );

			shockSprite.setScale( (1. / 60.) * shockTestFrame, (1. / 60.) * shockTestFrame );
			preScreenTex->draw( shockSprite, &shockwaveShader );


			shockTestFrame++;
			if( shockTestFrame == 60 )
			{
				shockTestFrame = 0;
			}
			//postProcessTex2->draw( rectPost, &shockwaveShader );
			//postProcessTex2->display();

			//sf::Sprite pptSpr;
			//pptSpr.setTexture( postProcessTex2->getTexture() );
			//preScreenTex->draw( pptSpr );
		}


		preScreenTex->setView( uiView );

		if( raceFight != NULL )
		{
			raceFight->DrawScore( preScreenTex );
		}
		
		if( false )
		{
			sf::RectangleShape rectPost( Vector2f( 1920, 1080 ) );
			rectPost.setPosition( 0, 0 );
			//Vector2f camVel = cam.pos - oldCamCenter;
			
			Vector2f botLeft = Vector2f( view.getCenter().x - view.getSize().x / 2, 
				view.getCenter().y + view.getSize().y / 2 );
			

			motionBlurShader.setUniform( "tex", preScreenTex->getTexture() );
			motionBlurShader.setUniform( "oldBotLeft", oldCamBotLeft );
			motionBlurShader.setUniform( "botLeft", botLeft );
			motionBlurShader.setUniform( "oldZoom", oldZoom );
			motionBlurShader.setUniform( "zoom", cam.GetZoom() );

			motionBlurShader.setUniform( "g_ViewProjectionInverseMatrix", view.getTransform().getInverse().getMatrix() );
			motionBlurShader.setUniform( "g_previousViewProjectionMatrix", oldView.getTransform().getMatrix());

			
			postProcessTex->draw( rectPost, &motionBlurShader );

			postProcessTex->display();

			sf::Sprite pptSpr;
			pptSpr.setTexture( postProcessTex->getTexture() );
			//pptSpr.setScale( 2, 2 );
			//RenderStates blahRender;
			//blahRender.blendMode = sf::BlendAdd;//sf::BlendAdd;

			preScreenTex->draw( pptSpr );
			//postProcessTex->display();
			
		}
		else if( false )
		{
		sf::RectangleShape rectPost( Vector2f( 1920/2, 1080/2 ) );
		rectPost.setPosition( 0, 0 );
		glowShader.setUniform( "tex", preScreenTex->getTexture() );
		//glowShader.setUniform( "old", postProcessTex->getTexture() );
		postProcessTex->draw( rectPost, &glowShader );

		for( int i = 0; i < 3; ++i )
		{
			postProcessTex->display();
			hBlurShader.setUniform( "tex", postProcessTex->getTexture() );
			
			postProcessTex1->draw( rectPost, &hBlurShader );

			postProcessTex1->display();
			vBlurShader.setUniform( "tex", postProcessTex1->getTexture() );
			postProcessTex->draw( rectPost, &vBlurShader );
		}
		


		postProcessTex->display();

		sf::Sprite pptSpr;
		pptSpr.setTexture( postProcessTex->getTexture() );
		pptSpr.setScale( 2, 2 );
		RenderStates blahRender;
		blahRender.blendMode = sf::BlendAdd;

		preScreenTex->draw( pptSpr, blahRender );
		}
		else if( false )
		{
			sf::RectangleShape rectPost( Vector2f( 1920/2, 1080/2 ) );
			rectPost.setPosition( 0, 0 );
			for( int i = 0; i < 3; ++i )
			{
				hBlurShader.setUniform( "tex", preScreenTex->getTexture() );
				postProcessTex->draw( rectPost, &hBlurShader );

				postProcessTex->display();
				vBlurShader.setUniform( "tex", postProcessTex->getTexture() );
				preScreenTex->draw( rectPost, &vBlurShader );

				if( i < 2 )
				{
					preScreenTex->display();
				}
			}
		}
	
		if (adventureHUD != NULL)
		{
			adventureHUD->Draw(preScreenTex);
		}

		scoreDisplay->Draw(preScreenTex);

		if( showFrameRate )
		{
			preScreenTex->draw( frameRate );
		}
		
		if (showRunningTimer && !scoreDisplay->active)
		{
			preScreenTex->draw(runningTimerText);
		}

		if(inputVis != NULL)
			inputVis->Draw(preScreenTex);


		preScreenTex->setView( view );
		

		DrawDyingPlayers();
		
		UpdateTimeSlowShader();
		
		//this is so inefficient LOL
		if (false)
		{
			preScreenTex->display();
			const Texture &preTex = preScreenTex->getTexture();

			Sprite preTexSprite(preTex);
			preTexSprite.setPosition(-960 / 2, -540 / 2);
			preTexSprite.setScale(.5, .5);

			preScreenTex->setView(v);
			preScreenTex->draw(preTexSprite, &cloneShader);

			preScreenTex->setView(view);

			//draws the player again on top of everything
			/*if( player->action != Actor::DEATH )
				player->Draw( preScreenTex );*/

			for (int i = 0; i < 4; ++i)
			{
				p = GetPlayer(i);
				if (p != NULL)
				{
					p->DodecaLateDraw(preScreenTex);
				}
			}

			//enemyTree->DebugDraw( preScreenTex );

			preScreenTex->draw(*debugBorders);

			preScreenTex->setView(uiView);
			fader->Draw(preScreenTex);
			swiper->Draw(preScreenTex);

			mainMenu->DrawEffects(preScreenTex);
		}

		if (currBroadcast != NULL)
		{
			preScreenTex->setView(uiView);
			currBroadcast->Draw(preScreenTex);
		}

		DrawActiveSequence(EffectLayer::UI_FRONT);
		DrawEffects(EffectLayer::UI_FRONT);
		DrawEmitters(EffectLayer::UI_FRONT);

		preScreenTex->setView(uiView);

		absorbDarkParticles->Draw(preScreenTex);

		//absorbShardParticles->Draw(preScreenTex);
		
		fader->Draw(preScreenTex);
		swiper->Draw(preScreenTex);

		mainMenu->DrawEffects(preScreenTex);

		preScreenTex->setView(view); //sets it back to normal for any world -> pixel calcs
		if ((fader->fadeSkipKin && fader->fadeAlpha > 0) || (swiper->skipKin && swiper->IsSwiping()) )//IsFading()) //adjust later?
		{
			DrawEffects(EffectLayer::IN_FRONT);
			for (int i = 0; i < 4; ++i)
			{
				p = GetPlayer(i);
				if (p != NULL)
				{
					//if (p->action == Actor::DEATH)
					{
						p->Draw(preScreenTex);
					}
				}
			}
		}

		preScreenTex->display();

		const Texture &preTex0 = preScreenTex->getTexture();
		Sprite preTexSprite(preTex0);
		preTexSprite.setPosition(-960 / 2, -540 / 2);
		preTexSprite.setScale(.5, .5);
		preTexSprite.setTexture( preTex0 );
		
		if (debugScreenRecorder != NULL)
			debugScreenRecorder->Update(preTex0);

		window->draw( preTexSprite );//, &cloneShader );
		
		}
		else if( state == FROZEN )
		{
			sf::Event ev;
			while( window->pollEvent( ev ) )
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
				if( ev.type == sf::Event::GainedFocus )
				{
					//state = RUN;
					//soundNodeList->Pause( false );
					//break;
				}
				else if( ev.type == sf::Event::KeyPressed )
				{
					//if( ev.key.code == Keyboard::
				}
			}

			//savedinput when you enter pause

			accumulator += frameTime;

			while (accumulator >= TIMESTEP)
			{
				UpdateInput();

				if (activeSequence != NULL)
				{
					State oldState = state;
					if (!activeSequence->Update())
					{
						state = RUN;
						activeSequence = NULL;
					}
					else
					{
						if (state != oldState)
						{
							switchState = true;
							break;
							//goto starttest;
						}
					}
				}

				if (state != FROZEN)
				{
					break;
				}

				accumulator -= TIMESTEP;
			}

			if (state != FROZEN)
			{
				continue;
			}

			//pauseTex->clear( Color( 100, 100, 100, 50 ));	
			window->clear();
			Sprite preTexSprite;
			preTexSprite.setTexture( preScreenTex->getTexture() );
			preTexSprite.setPosition( -960 / 2, -540 / 2 );
			preTexSprite.setScale( .5, .5 );
			window->draw( preTexSprite );

			//pauseMenu->Draw( pauseTex );
			
			//pauseTex->display();
			//Sprite pauseMenuSprite;
			//pauseMenuSprite.setTexture( pauseTex->getTexture() );
			////bgSprite.setPosition( );
			//pauseMenuSprite.setPosition( (1920 - 1820) / 4 - 960 / 2, (1080 - 980) / 4 - 540 / 2 );
			//pauseMenuSprite.setScale( .5, .5 );
			//window->draw( pauseMenuSprite );
		}
		else if( state == MAP )
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
				UpdateInput();

				if( ( GetCurrInput( 0 ).back && !GetPrevInput( 0 ).back ) && state == MAP )
				{
					state = RUN;
					soundNodeList->Pause( false );
					pauseSoundNodeList->ActivateSound(soundManager->GetSound("pause_off"));
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

			queryMode = "border";
			numBorders = 0;
			sf::Rect<double> mapRect(vv.getCenter().x - vv.getSize().x / 2.0,
				vv.getCenter().y - vv.getSize().y / 2.0, vv.getSize().x, vv.getSize().y );

			borderTree->Query( this, mapRect );

			Color testColor( 0x75, 0x70, 0x90, 191 );
			testColor = Color::Green;
			TerrainPiece * listVAIter = listVA;
			while( listVAIter != NULL )
			{
				int vertexCount = listVAIter->terrainVA->getVertexCount();
				for( int i = 0; i < vertexCount; ++i )
				{
					(*listVAIter->terrainVA)[i].color = testColor;
				}
				mapTex->draw( *listVAIter->terrainVA );
				for( int i = 0; i < vertexCount; ++i )
				{
					(*listVAIter->terrainVA)[i].color = Color::White;
				}

				listVAIter = listVAIter->next;
			}

			testGateCount = 0;
			queryMode = "gate";
			gateList = NULL;
			gateTree->Query( this, mapRect );
			Gate *mGateList = gateList;
			while( gateList != NULL )
			{
				//gateList->Draw( preScreenTex );
				if( gateList->locked )
				{

					V2d along = normalize(gateList->edgeA->v1 - gateList->edgeA->v0);
					V2d other( along.y, -along.x );
					double width = 25;
				
				

					V2d leftGround = gateList->edgeA->v0 + other * -width;
					V2d rightGround = gateList->edgeA->v0 + other * width;
					V2d leftAir = gateList->edgeA->v1 + other * -width;
					V2d rightAir = gateList->edgeA->v1 + other * width;
					//cout << "drawing color: " << gateList->c.b << endl;
					sf::Vertex activePreview[4] =
					{
						//sf::Vertex(sf::Vector2<float>( gateList->v0.x, gateList->v0.y ), gateList->c ),
						//sf::Vertex(sf::Vector2<float>( gateList->v1.x, gateList->v1.y ), gateList->c ),

						sf::Vertex(sf::Vector2<float>( leftGround.x, leftGround.y ), gateList->c ),
						sf::Vertex(sf::Vector2<float>( leftAir.x, leftAir.y ), gateList->c ),


						sf::Vertex(sf::Vector2<float>( rightAir.x, rightAir.y ), gateList->c ),

					
						sf::Vertex(sf::Vector2<float>( rightGround.x, rightGround.y ), gateList->c )
					};
					mapTex->draw( activePreview, 4, sf::Quads );
				}

				Gate *next = gateList->next;//edgeA->edge1;
				gateList = next;
			}

			//queryMode = "enemyminimap";
			//enemyTree->Query( this, mapRect );

			//Enemy *currEnemy = activeEnemyList;
			//int counter = 0;
		
			/*while( currEnemy != NULL )
			{
				if( currEnemy->hasMonitor )
				{
					currEnemy->DrawMinimap( mapTex );
					
				}
				currEnemy = currEnemy->next;
			}*/

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

			Vector2i b1 = mapTex->mapCoordsToPixel( Vector2f( originalPos.x, originalPos.y ) );

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

			//cout << "vuiVew size: " << vuiView.getSize().x << ", " << vuiView.getSize().y << endl;
			goalMapIcon.setPosition( realPosGoal );
			mapTex->draw( goalMapIcon );
			//mapTex->clear();
			Sprite mapTexSprite;
			mapTexSprite.setTexture( mapTex->getTexture() );
			mapTexSprite.setOrigin( mapTexSprite.getLocalBounds().width / 2, mapTexSprite.getLocalBounds().height / 2 );
			mapTexSprite.setPosition( 0, 0 );
			
			//window->setView( bigV );

			//mapTexSprite.setScale( .5, -.5 );
			mapTexSprite.setScale( .5, -.5 );
			cout << "size: " << mapTexSprite.getLocalBounds().width << ", " << mapTexSprite.getLocalBounds().height << endl;
			//mapTexSprite.setColor( Color::Red );
			window->draw( mapTexSprite );

		}
		else if( state == RACEFIGHT_RESULTS )
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
			

			
				UpdateInput();

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
		else if (state == STORY)
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

				UpdateInput();

				if (currStorySequence != NULL)
				{
					//if( false )
					if (!currStorySequence->Update(GetPrevInput(0), GetCurrInput(0)))
					{
						state = RUN;
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

			if (showFrameRate)
			{
				preScreenTex->draw(frameRate);
			}

			preTexSprite.setTexture(preScreenTex->getTexture());
			preTexSprite.setPosition(-960 / 2, -540 / 2);
			preTexSprite.setScale(.5, .5);
			window->draw(preTexSprite);
			//UpdateInput();
		
		}
		else if (state == SEQUENCE)
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

			window->clear();
			window->setView(v);
			preScreenTex->clear();

			accumulator += frameTime;
			Sprite preTexSprite;
			while (accumulator >= TIMESTEP)
			{
				

				UpdateInput();

				if (activeSequence != NULL)
				{
					State oldState = state;
					if (!activeSequence->Update())
					{
						state = RUN;
						activeSequence = NULL;
					}
					else
					{
						if (state != oldState)
						{
							switchState = true;
							break;
							//goto starttest;
						}
					}
				}

				mainMenu->musicPlayer->Update();

				fader->Update();
				swiper->Update();
				mainMenu->UpdateEffects();
				UpdateEmitters();

				accumulator -= TIMESTEP;

				if (goalDestroyed)
				{
					quit = true;

					//returnVal = GR_WIN;
					returnVal = resType;
					break;
				}
			}

			if (switchState)
			{
				continue;
			}

			if (activeSequence != NULL)
			{
				//preScreenTex->setView(uiView);
				activeSequence->Draw(preScreenTex);
			}

			preScreenTex->setView(uiView);

			
			fader->Draw(preScreenTex);
			swiper->Draw(preScreenTex);

			mainMenu->DrawEffects(preScreenTex);

			if (showFrameRate)
			{
				preScreenTex->draw(frameRate);
			}

			preTexSprite.setTexture(preScreenTex->getTexture());
			preTexSprite.setPosition(-960 / 2, -540 / 2);
			preTexSprite.setScale(.5, .5);
			window->draw(preTexSprite);
			//UpdateInput();

		}
		else if (state == PAUSE)
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
				UpdateInput();

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
					state = GameSession::RUN;
					pauseSoundNodeList->ActivateSound(soundManager->GetSound("pause_off"));
					soundNodeList->Pause(false);
					pauseMenu->shardMenu->StopMusic();
					break;
				}
				case PauseMenu::R_P_RESPAWN:
				{
					state = GameSession::RUN;
					RestartLevel();
					gameClock.restart();
					currentTime = 0;
					accumulator = TIMESTEP + .1;
					frameCounter = 0;
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

				if (state != PAUSE)
				{
					break;
				}

				accumulator -= TIMESTEP;
			}

			if (state != PAUSE)
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

				queryMode = "border";
				numBorders = 0;
				sf::Rect<double> mapRect(vv.getCenter().x - vv.getSize().x / 2.0,
					vv.getCenter().y - vv.getSize().y / 2.0, vv.getSize().x, vv.getSize().y);

				borderTree->Query(this, mapRect);

				Color testColor(0x75, 0x70, 0x90, 191);
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
				}

				testGateCount = 0;
				queryMode = "gate";
				gateList = NULL;
				gateTree->Query(this, mapRect);
				Gate *mGateList = gateList;
				while (gateList != NULL)
				{
					//gateList->Draw( preScreenTex );
					if (gateList->locked && gateList->visible)
					{

						V2d along = normalize(gateList->edgeA->v1 - gateList->edgeA->v0);
						V2d other(along.y, -along.x);
						double width = 25;



						V2d leftGround = gateList->edgeA->v0 + other * -width;
						V2d rightGround = gateList->edgeA->v0 + other * width;
						V2d leftAir = gateList->edgeA->v1 + other * -width;
						V2d rightAir = gateList->edgeA->v1 + other * width;
						//cout << "drawing color: " << gateList->c.b << endl;
						sf::Vertex activePreview[4] =
						{
							//sf::Vertex(sf::Vector2<float>( gateList->v0.x, gateList->v0.y ), gateList->c ),
							//sf::Vertex(sf::Vector2<float>( gateList->v1.x, gateList->v1.y ), gateList->c ),

							sf::Vertex(sf::Vector2<float>(leftGround.x, leftGround.y), gateList->c),
							sf::Vertex(sf::Vector2<float>(leftAir.x, leftAir.y), gateList->c),


							sf::Vertex(sf::Vector2<float>(rightAir.x, rightAir.y), gateList->c),


							sf::Vertex(sf::Vector2<float>(rightGround.x, rightGround.y), gateList->c)
						};
						mapTex->draw(activePreview, 4, sf::Quads);
					}

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

				Vector2i b1 = mapTex->mapCoordsToPixel(Vector2f(originalPos.x, originalPos.y));

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
				goalMapIcon.setPosition(realPosGoal);
				mapTex->draw(goalMapIcon);

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
		localtime_s( &now, &t);





		stringstream fss;
		string mName = filePath.filename().stem().string();
		fss << "Recordings/Ghost/" << mName << "/auto/" << mName << "_ghost_"
			<< now.tm_year << "_" << now.tm_mon << "_" << now.tm_mday << "_" << now.tm_hour << "_"
			<< now.tm_min << "_" << now.tm_sec << ".bghst";

		//recGhost->WriteToFile(fss.str());
	}

	
	
	testBuf.SetRecOver( true );

	if( recPlayer != NULL )
	{
		threa->join();
		delete threa;
		
		assert( of.is_open() );
		of.close();

		ifstream is;
		is.open( "tempreplay.brep", ios::binary | ios::in );

		
		ofstream out;
		//custom file
		out.open( "testreplay.brep", ios::binary | ios::out );
		out.write( (char*)&(recPlayer->numTotalFrames), sizeof( int ) );
		//out << recPlayer->numTotalFrames << "\n";

		char c;
		while( true )
		{
			c = is.get();
			if( is.eof() ) break;
			out.put( c );
		}

		out.close();
		

		/*istreambuf_iterator<char> begin_source( is );
		istreambuf_iterator<char> end_source;
		ostreambuf_iterator<char> begin_dest( out );
		copy( begin_source, end_source, begin_dest );*/

		is.close();
			
	}
	
	soundNodeList->Reset();
	pauseSoundNodeList->Reset();
	
	for( int i = 0; i < 4; ++i )
	{
		SetFilterDefault( GetController(i).filter );
	}

	pauseMenu->owner = NULL;

	fader->Clear();

	preScreenTex->setView(oldPreTexView);
	window->setView(oldWindowView);

	mainMenu->SetMouseGrabbed(oldMouseGrabbed);
	mainMenu->SetMouseVisible(oldMouseVisible);

	return returnVal;
}

bool GameSession::IsKeyPressed(int key)
{
	return mainMenu->IsKeyPressed(key);
}

void GameSession::Fade(bool in, int frames, sf::Color c, bool skipKin)
{
	fader->Fade(in, frames, c, skipKin);
}

void GameSession::CrossFade(int fadeOutFrames,
	int pauseFrames, int fadeInFrames,
	sf::Color c, bool skipKin)
{
	fader->CrossFade(fadeOutFrames, pauseFrames, fadeInFrames, c, skipKin);
}

void GameSession::ClearFade()
{
	fader->Clear();
}

bool GameSession::IsFading()
{
	return fader->IsFading();
}

void GameSession::Init()
{
	shardPop = NULL;

	nextFrameRestart = false;

	LoadDecorImages();

	getShardSeq = NULL;

	fader = mainMenu->fader;
	swiper = mainMenu->swiper;

	shardsCapturedField = NULL;

	mini = NULL;

	level = NULL;

	boostIntro = false;

	for (int i = 0; i < 6; ++i)
	{
		hasGrass[i] = false;
	}

	totalRails = 0;

	stormCeilingOn = false;
	stormCeilingHeight = 0;

	inputVis = NULL;

	adventureHUD = NULL;

	mh = NULL;
	goalPulse = NULL;
	pauseMenu = NULL;
	progressDisplay = NULL;

	for (int i = 0; i < 4; ++i)
	{
		players[i] = NULL;
	}

	topClouds = NULL;

	soundManager = NULL;
	
	keyMarker = NULL;
	
	specterTree = NULL;

	activeItemTree = NULL;
	
	envPlantTree = NULL;
	
	itemTree = NULL;
	
	gateTree = NULL;
	
	grassTree = NULL;
	
	borderTree = NULL;
	
	enemyTree = NULL;
	
	terrainTree = NULL;

	staticItemTree = NULL;

	railEdgeTree = NULL;

	railDrawTree = NULL;
	
	terrainBGTree = NULL;
	
	scoreDisplay = NULL;
	
	soundNodeList = NULL;

	pauseSoundNodeList = NULL;
	
	originalMusic = NULL;

	rain = NULL;//new Rain(this);//NULL;
	//stormCeilingInfo = NULL;
	

	for (list<VertexArray*>::iterator it = polygons.begin(); it != polygons.end(); ++it)
	{
		delete (*it);
	}
	polygons.clear();
	
	polyShaders = NULL;
	ts_polyShaders = NULL;
	terrainDecorInfos = NULL;
	testPar = NULL;
	va = NULL;
	edges = NULL;
	activeEnemyList = NULL;
	activeEnemyListTail = NULL;
	pauseFrames = 0;

	raceFight = NULL;

	recPlayer = NULL;
	repPlayer = NULL;
	recGhost = NULL;
	//repGhost = NULL;
	showTerrainDecor = true;
	shipExitSeq = NULL;
	activeDialogue = NULL;

	keyFrame = 0;
	for (int i = 0; i < EffectLayer::Count; ++i)
	{
		effectLists[i] = NULL;
	}

	/*stormCeilingInfo = new HitboxInfo;
	stormCeilingInfo->damage = 20;
	stormCeilingInfo->drainX = .5;
	stormCeilingInfo->drainY = .5;
	stormCeilingInfo->hitlagFrames = 0;
	stormCeilingInfo->hitstunFrames = 30;
	stormCeilingInfo->knockback = 0;
	stormCeilingInfo->freezeDuringStun = true;*/
	
	window = mainMenu->window;
	preScreenTex = mainMenu->preScreenTexture;
	lastFrameTex = mainMenu->lastFrameTexture;
	postProcessTex = mainMenu->postProcessTexture;
	postProcessTex1 = mainMenu->postProcessTexture1;
	postProcessTex2 = mainMenu->postProcessTexture2;
	mapTex = mainMenu->mapTexture;
	minimapTex = mainMenu->minimapTexture;
	pauseTex = mainMenu->pauseTexture;

	currentZone = NULL;
	Movable::owner = this;
	//b_crawler = NULL;
	b_bird = NULL;
	b_coyote = NULL;
	b_tiger = NULL;
	b_gator = NULL;
	b_skeleton = NULL;

	cutPlayerInput = false;

	bigBulletVA = NULL;

	preScreenTex->setSmooth(false);
	postProcessTex->setSmooth(false);
	postProcessTex1->setSmooth(false);
	postProcessTex2->setSmooth(false);

	explodingGravityGrass = NULL;

	shockTestFrame = 0;

	usePolyShader = true;

	flowFrameCount = 60;
	flowFrame = 0;
	maxFlowRadius = 10000;
	radDiff = 100;
	flowSpacing = 600;
	maxFlowRings = 40;

	deathWipe = false;
	deathWipeFrame = 0;
	deathWipeLength = 17 * 5;

	listVA = NULL;

	inactiveEffects = NULL;
	pauseImmuneEffects = NULL;

	drawInversePoly = true;
	showDebugDraw = false;

	testBuf.SetRecOver(false);


	gates = NULL;
	edges = NULL;

	absorbParticles = NULL;
	absorbDarkParticles = NULL;
	absorbShardParticles = NULL;

	background = NULL;
}

void GameSession::HandleEntrant( QuadTreeEntrant *qte )
{
	if( queryMode == "enemy" )
	{
		Enemy *e = (Enemy*)qte;
		if (e->spawned)
			return;
		

		bool a = e->spawnRect.intersects( tempSpawnRect );
		bool b = ( e->zone == NULL || e->zone->active ); 
		
		//if( e->type == Enemy::NEXUS )
		//{
		//	cout << "zone: " << e->zone << ", " << e->zone->active << endl;
		//	cout << "orig: " << originalZone << ", " << originalZone->active << endl;
		//	cout << "blah: " << e->zone->allEnemies.size() << endl;
		//	//cout << "querying nexus: " << (int)a << ", " << (int)b << ", :: " 
		//		//<< ( e->zone == NULL ) << endl;
		//	
		//}

		//sf::Rect<double> screenRect( cam.pos.x - camWidth / 2, cam.pos.y - camHeight / 2, camWidth, camHeight );
		if( a && b )
		{
			AddEnemy( e );
		}
	}
	else if( queryMode == "enemyzone" )
	{
		Enemy *e = (Enemy*)qte;
	}
	else if( queryMode == "border" )
	{
		if( listVA == NULL )
		{
			listVA = (TerrainPiece*)qte;
		//	cout << "1" << endl;
			numBorders++;
		}
		else
		{
			
			TerrainPiece *tva = (TerrainPiece*)qte;
			TerrainPiece *temp = listVA;
			bool okay = true;
			while( temp != NULL )
			{
				if( temp == tva )
				{
					okay = false;
					break;
				}	
				temp = temp->next;
			}

			if( okay )
			{
			
			//cout << "blah: " << (unsigned)tva << endl;
				tva->next = listVA;
				listVA = tva;
				numBorders++;
				//cout << numBorders + 1 << endl;
			}
		}
		
	}
	else if( queryMode == "inverseborder" )
	{
		if( inverseEdgeList == NULL )
		{
			inverseEdgeList = (Edge*)qte;
			inverseEdgeList->info = NULL;
			//numBorders++;
		}
		else
		{
			
			Edge *tva = (Edge*)qte;
			Edge *temp = inverseEdgeList;
			bool okay = true;
			while( temp != NULL )
			{
				if( temp == tva )
				{
					okay = false;
					break;
				}	
				temp = (Edge*)temp->info;//->edge1;
			}

			if( okay )
			{
				tva->info = (void*)inverseEdgeList;
				inverseEdgeList = tva;
			}
		}
		
	}
	else if( queryMode == "gate" )
	{
		Gate *g = (Gate*)qte;

		if( gateList == NULL )
		{
			gateList = (Gate*)qte;
			gateList->next = NULL;
			gateList->prev = NULL;
			
			//cout << "setting gate: " << gateList->edgeA << endl;
		}
		else
		{
			g->next = gateList;
			gateList = g;
		}

		//cout << "gate" << endl;
		++testGateCount;
	}
	else if( queryMode == "item" )
	{
		Critical *c = (Critical*)qte;

		if( drawCritical == NULL )
		{
			drawCritical = (Critical*)qte;
			drawCritical->next = NULL;
			drawCritical->prev = NULL;
		}
		else
		{
			c->next = drawCritical;
			drawCritical = c;
		}
	}
	else if( queryMode == "envplant" )
	{
		EnvPlant *ep = (EnvPlant*)qte;

		if( !ep->activated )
		{

			int idleLength = ep->idleLength;
			int idleFactor = ep->idleFactor;

			IntRect sub = ep->ts->GetSubRect( (totalGameFrames % ( idleLength * idleFactor )) / idleFactor );
			VertexArray &eva = *ep->va;
			eva[ep->vaIndex + 0].texCoords = Vector2f( sub.left, sub.top );
			eva[ep->vaIndex + 1].texCoords = Vector2f( sub.left + sub.width, sub.top );
			eva[ep->vaIndex + 2].texCoords = Vector2f( sub.left + sub.width, sub.top + sub.height );
			eva[ep->vaIndex + 3].texCoords = Vector2f( sub.left, sub.top + sub.height );
		}
		//va[ep->
	}
	else if (queryMode == "rail")
	{
		Rail *r = (Rail*)qte;

		if (railDrawList == NULL)
		{
			railDrawList = r;
		}
		else
		{
			r->drawNext = railDrawList;
			railDrawList = r;
		}
	}
}

bool GameSession::ScreenIntersectsInversePoly( sf::Rect<double> &screenRect )
{
	double extra = 32; //for some buffer room

	screenRect.left -= extra;
	screenRect.top -= extra;
	screenRect.width += extra * 2;
	screenRect.height += extra * 2;

	inverseEdgeList = NULL;
	queryMode = "inverseborder";
	Edge *curr = inverseEdgeList;
	while( curr != NULL )
	{
		if( IsEdgeTouchingBox( curr, screenRect ) )
		{
			return true;
		}
		curr = (Edge*)curr->info;
	}

	return false;

	//IsEdgeTouchingBox
}

void GameSession::SetupInversePoly( Tileset *ts_bush, int currentEdgeIndex )
{
	assert( inversePoly != NULL );

	sf::Rect<double> aabb = inversePoly->aabb;

	double testExtra = 500;

	sf::Rect<double> finalRect = aabb;
	finalRect.left -= testExtra;
	finalRect.top -= testExtra;
	finalRect.width += testExtra * 2;
	finalRect.height += testExtra * 2;

	vector<p2t::Point*> outerQuadPoints;
	outerQuadPoints.push_back( new p2t::Point( finalRect.left, finalRect.top ) );
	outerQuadPoints.push_back( new p2t::Point( finalRect.left + finalRect.width, finalRect.top ) );
	outerQuadPoints.push_back( new p2t::Point( finalRect.left + finalRect.width, finalRect.top + finalRect.height ) );
	outerQuadPoints.push_back( new p2t::Point( finalRect.left, finalRect.top + finalRect.height ) );

	p2t::CDT * cdt = new p2t::CDT( outerQuadPoints );

	vector<p2t::Point*> polyline;
	for( int i = 0; i < inversePoly->numPoints; ++i )
	{
		
		//inverse polygon will always be the first set of points in the list
		polyline.push_back( new p2t::Point( points[i].x, points[i].y ) );
		//cout << "polyline: " << polyline.back()->x << ", " << polyline.back()->y << endl;
	}

	//cut a hole in the polygon
	cdt->AddHole( polyline );
			
	vector<p2t::Triangle*> tris;

	cdt->Triangulate();
				
	tris = cdt->GetTriangles();

	va = new VertexArray( sf::Triangles , tris.size() * 3 );
	VertexArray & v = *va;
	Color testColor( 0x75, 0x70, 0x90 );
	testColor = Color::White;
	//Vector2f topLeft( left, top );
	//cout << "topleft: " << topLeft.x << ", " << topLeft.y << endl;
	for( int i = 0; i < tris.size(); ++i )
	{	
		p2t::Point *p = tris[i]->GetPoint( 0 );	
		p2t::Point *p1 = tris[i]->GetPoint( 1 );	
		p2t::Point *p2 = tris[i]->GetPoint( 2 );	
		v[i*3] = Vertex( Vector2f( p->x, p->y ), testColor );
		v[i*3 + 1] = Vertex( Vector2f( p1->x, p1->y ), testColor );
		v[i*3 + 2] = Vertex( Vector2f( p2->x, p2->y ), testColor );
	}

	inversePoly->terrainVA = va;

	double polygonArea = 0;
	for( vector<p2t::Triangle*>::iterator it = tris.begin();
		it != tris.end(); ++it )
	{
		polygonArea += GetTriangleArea( (*it) );
	}

	inversePoly->polyArea = polygonArea;

	VertexArray *bushVA = SetupBushes( 0,  edges[currentEdgeIndex], ts_bush );

	inversePoly->bushVA = bushVA;

	//polygons.push_back( va );

	delete cdt;
	for( int i = 0; i < inversePoly->numPoints; ++i )
	{
		delete polyline[i];
	}
	
	for( int i = 0; i < 4; ++i )
	{
		delete outerQuadPoints[i];
	}
}

void GameSession::SetStorySeq(StorySequence *storySeq)
{
	storySeq->Reset();
	currStorySequence = storySeq;
	state = GameSession::STORY;
}

void GameSession::DebugDrawActors()
{
	Actor *p = NULL;
	for( int i = 0; i < 4; ++i )
	{
		p = GetPlayer( i );
		if( p != NULL )
		{
			p->DebugDraw( preScreenTex );
		}
	}
	
	Enemy *currEnemy = activeEnemyList;
	while( currEnemy != NULL )
	{
		currEnemy->DebugDraw( preScreenTex );
		currEnemy = currEnemy->next;
	}
}

void GameSession::DrawDyingPlayers()
{
	Actor *p = NULL;
	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			if (p->action == Actor::DEATH)
			{
				p->Draw(preScreenTex);
			}
		}
	}
}

void GameSession::UpdateTimeSlowShader()
{
	Actor *p0 = GetPlayer(0);

	cloneShader.setUniform("u_texture", preScreenTex->getTexture());
	cloneShader.setUniform("newscreen", p0->percentCloneChanged);
	cloneShader.setUniform("Resolution", Vector2f(1920, 1080));//window->getSize().x, window->getSize().y);
	cloneShader.setUniform("zoom", cam.GetZoom());

	cloneShader.setUniform("topLeft", Vector2f(view.getCenter().x - view.getSize().x / 2,
		view.getCenter().y + view.getSize().y / 2));

	cloneShader.setUniform("bubbleRadius0", (float)p0->bubbleRadiusSize[0]);
	cloneShader.setUniform("bubbleRadius1", (float)p0->bubbleRadiusSize[1]);
	cloneShader.setUniform("bubbleRadius2", (float)p0->bubbleRadiusSize[2]);
	cloneShader.setUniform("bubbleRadius3", (float)p0->bubbleRadiusSize[3]);
	cloneShader.setUniform("bubbleRadius4", (float)p0->bubbleRadiusSize[4]);
	cloneShader.setUniformArray("bubbleRadius", fBubbleRadiusSize, 20);//p0->maxBubbles * m_numActivePlayers);
	cloneShader.setUniformArray("bPos", fBubblePos, 20);//p0->maxBubbles * m_numActivePlayers);
	cloneShader.setUniformArray("bFrame", fBubbleFrame, 20);//p0->maxBubbles * m_numActivePlayers);
	cloneShader.setUniform("totalBubbles", p0->maxBubbles * m_numActivePlayers);
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





	cloneShader.setUniformArray("bPos", fBubblePos, 5 * 4);
	//cout << "pos0: " << pos0.x << ", " << pos0.y << endl;
	//cout << "b0frame: " << player->bubbleFramesToLive[0] << endl;
	//cout << "b1frame: " << player->bubbleFramesToLive[1] << endl;
	//cout << "b2frame: " << player->bubbleFramesToLive[2] << endl;

	//cloneShader.setUniform( "bubble0", pos0 );
	cloneShader.setUniform("b0Frame", (float)p0->bubbleFramesToLive[0]);
	//cloneShader.setUniform( "bubble1", pos1 );
	cloneShader.setUniform("b1Frame", (float)p0->bubbleFramesToLive[1]);
	//cloneShader.setUniform( "bubble2", pos2 );
	cloneShader.setUniform("b2Frame", (float)p0->bubbleFramesToLive[2]);
	//cloneShader.setUniform( "bubble3", pos3 );
	cloneShader.setUniform("b3Frame", (float)p0->bubbleFramesToLive[3]);
	//cloneShader.setUniform( "bubble4", pos4 );
	cloneShader.setUniform("b4Frame", (float)p0->bubbleFramesToLive[4]);
	//cloneShader.setUniform( "bubble5", pos5 );
	//cloneShader.setUniform( "b5Frame", player->bubbleFramesToLive[5] );
}

void GameSession::DrawStoryLayer(EffectLayer ef)
{
	if (currStorySequence != NULL)
	{
		sf::View oldV = preScreenTex->getView();
		preScreenTex->setView(uiView);
		currStorySequence->DrawLayer(preScreenTex, ef);
		preScreenTex->setView(oldV);
	}
}

void GameSession::DrawActiveSequence(EffectLayer layer )
{

	sf::View oldView = preScreenTex->getView();
	if (layer == UI_FRONT)
	{
		preScreenTex->setView(uiView);
	}

	if (activeSequence != NULL)
	{
		activeSequence->Draw(preScreenTex, layer);
	}

	if (layer == UI_FRONT)
	{
		preScreenTex->setView(oldView);
	}
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



SoundNode *GameSession::ActivateSound( V2d &pos, SoundBuffer *buffer, bool loop )
{
	sf::Rect<double> soundRect = screenRect;
	double soundExtra = 300;//800
	soundRect.left -= soundExtra;
	soundRect.width += 2 * soundExtra;
	soundRect.top -= soundExtra;
	soundRect.height += 2 * soundExtra;

	if (soundRect.contains(pos))
	{
		return soundNodeList->ActivateSound(buffer, loop);
	}
	else
	{
		return NULL;
	}
}

void GameSession::DrawZones()
{
	for (list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->Draw(preScreenTex);
	}
}

void GameSession::DrawBlackBorderQuads()
{
	bool narrowMap = mh->boundsWidth < 1920 * 2;

	if (cam.manual || narrowMap)
	{
		preScreenTex->draw(blackBorderQuads, 16, sf::Quads);
	}
	else
	{
		preScreenTex->draw(blackBorderQuads, 8, sf::Quads);
	}
}

void GameSession::DrawTopClouds()
{
	if (topClouds != NULL)
		topClouds->Draw(preScreenTex);
}

void GameSession::UpdateEnvShaders()
{
	Vector2f botLeft(view.getCenter().x - view.getSize().x / 2,
		view.getCenter().y + view.getSize().y / 2);

	Vector2f playertest = (botLeft - oldCamBotLeft) / 5.f;

	UpdatePolyShaders(botLeft, playertest);

	for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->Update(cam.GetZoom(), botLeft, playertest);
	}
}

void GameSession::DrawGates()
{
	testGateCount = 0;
	queryMode = "gate";
	gateList = NULL;
	gateTree->Query(this, screenRect);

	while (gateList != NULL)
	{
		gateList->Draw(preScreenTex);
		Gate *next = gateList->next;//(Gate*)gateList->edgeA->edge1;
		gateList = next;
	}
}

void GameSession::DrawRails()
{
	railDrawList = NULL;
	queryMode = "rail";
	railDrawTree->Query(this, screenRect);
	while (railDrawList != NULL)
	{
		railDrawList->Draw(preScreenTex);
		Rail *next = railDrawList->drawNext;
		railDrawList->drawNext = NULL;
		railDrawList = next;
	}
}

void GameSession::DrawDecorBetween()
{
	for (auto it = decorBetween.begin(); it != decorBetween.end(); ++it)
	{
		(*it)->Draw(preScreenTex);
	}
}

void GameSession::DrawGoal()
{
	if (hasGoal)
	{
		preScreenTex->draw(*goalEnergyFlowVA, &flowShader);
	}
}


void GameSession::AddEmitter(ShapeEmitter *emit,
	EffectLayer layer)
{
	ShapeEmitter *&currList = emitterLists[layer];
	if (currList == NULL)
	{
		currList = emit;
		emit->next = NULL;
	}
	else
	{
		emit->next = currList;
		currList = emit;
	}
}

void GameSession::DrawEmitters(EffectLayer layer)
{
	ShapeEmitter *curr = emitterLists[layer];
	while (curr != NULL)
	{
		curr->Draw(preScreenTex);
		curr = curr->next;
	}
}

void GameSession::UpdateEmitters()
{
	ShapeEmitter *prev = NULL;
	ShapeEmitter *curr;
	for (int i = 0; i < EffectLayer::Count; ++i)
	{
		curr = emitterLists[i];
		prev = NULL;
		while (curr != NULL)
		{
			if (curr->IsDone())
			{
				if (curr == emitterLists[i])
				{
					emitterLists[i] = curr->next;
				}
				else
				{
					prev->next = curr->next;
				}
				curr = curr->next;
			}
			else
			{
				curr->Update();

				prev = curr;
				curr = curr->next;
			}
		}
	}
}

void GameSession::ClearEmitters()
{
	for (int i = 0; i < EffectLayer::Count; ++i)
	{
		emitterLists[i] = NULL;
	}
}

void GameSession::UpdateGoalFlow()
{
	Actor *p0 = GetPlayer(0);
	if (hasGoal)
	{
		flowRadius = (maxFlowRadius - (maxFlowRadius / flowFrameCount) * flowFrame);

		flowShader.setUniform("radius", flowRadius / maxFlowRings);
		//cout << "radius: " << flowRadius / maxFlowRings << ", frame: " << flowFrame << endl;
		flowShader.setUniform("zoom", cam.GetZoom());
		flowShader.setUniform("playerPos", Vector2f(p0->position.x, p0->position.y));



		++flowFrame;
		if (flowFrame == flowFrameCount)
		{
			flowFrame = 0;
		}
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
}

void GameSession::DebugDraw()
{
	if (showDebugDraw)
	{
		DebugDrawActors();

		for (auto it = fullAirTriggerList.begin(); it != fullAirTriggerList.end(); ++it)
		{
			(*it)->DebugDraw(preScreenTex);
		}
	}
}

void GameSession::DrawTerrainPieces(TerrainPiece *tPiece)
{
	if (drawInversePoly)
	{
		if (tPiece == NULL)
		{
			tPiece = inversePoly;
			inversePoly->next = NULL;
		}
		else
		{
			tPiece->next = inversePoly;
			inversePoly->next = NULL;
		}
	}

	//draw terrain
	while (tPiece != NULL)
	{
		tPiece->Draw(preScreenTex);
		tPiece = tPiece->next;
	}
}

void GameSession::UpdateActiveEnvPlants()
{

}

void GameSession::UpdateDecorSprites()
{
	TerrainPiece *te = listVA;
	while (te != NULL)
	{

		te->UpdateTouchGrass(); //put this in its own spot soon
		if (te->tr != NULL)
			te->tr->UpdateDecorSprites();
		te = te->next;
	}
}

void GameSession::DrawPlayerWires()
{
	Actor *p = NULL;
	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			if ((p->action != Actor::GRINDBALL && p->action != Actor::GRINDATTACK) || p->leftWire->state == Wire::RETRACTING)
			{
				p->leftWire->Draw(preScreenTex);
			}
			if ((p->action != Actor::GRINDBALL && p->action != Actor::GRINDATTACK) || p->rightWire->state == Wire::RETRACTING)
			{
				p->rightWire->Draw(preScreenTex);
			}
		}
	}
}

void GameSession::DrawHitEnemies()
{
	Enemy *current = activeEnemyList;
	while (current != NULL)
	{
		if ((pauseFrames >= 2 && current->receivedHit != NULL))
		{
			current->Draw(preScreenTex);
		}
		current = current->next;
	}
}

void GameSession::DrawPlayers()
{
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

void GameSession::DrawReplayGhosts()
{
	for (auto it = replayGhosts.begin(); it != replayGhosts.end(); ++it)
	{
		(*it)->Draw(preScreenTex);
	}
}

void GameSession::KillAllEnemies()
{
	Enemy *curr = activeEnemyList;
	while( curr != NULL )
	{
		Enemy *next = curr->next;

		if( curr->type != EnemyType::EN_GOAL && curr->type != EnemyType::EN_NEXUS )
		{
			curr->DirectKill();
			//curr->health = 0;
		}
		curr = next;	
	}
}

void GameSession::UpdatePolyShaders( Vector2f &botLeft, Vector2f &playertest)
{
	for (int i = 0; i < numPolyTypes; ++i)
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
	if (mh->songLevels.size() > 0)
	{
		for (auto it = mh->songLevels.begin(); it != mh->songLevels.end(); ++it)
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

		for (auto it = mh->songLevels.begin(); it != mh->songLevels.end(); ++it)
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

void GameSession::QueryBorderTree(sf::Rect<double> &rect)
{
	queryMode = "border";
	numBorders = 0;

	borderTree->Query(this, rect);
}

void GameSession::QueryGateTree(sf::Rect<double>&rect)
{
	testGateCount = 0;
	queryMode = "gate";
	gateList = NULL;
	gateTree->Query(this, rect);
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

bool GameSession::HasPowerUnlocked( int pIndex )
{
	SaveFile *prog = GetCurrentProgress();
	if ( mainMenu->gameRunType == MainMenu::GameRunType::GRT_ADVENTURE && prog != NULL && prog->HasPowerUnlocked((Actor::PowerType)pIndex))
	{
		return true;
	}

	return false;
}

void GameSession::DrawColoredMapTerrain(sf::RenderTarget *target, sf::Color &c)
{
	//must be already filled from a query
	TerrainPiece * listVAIter = listVA;
	while (listVAIter != NULL)
	{
		if (listVAIter->visible)
		{
			int vertexCount = listVAIter->terrainVA->getVertexCount();
			for (int i = 0; i < vertexCount; ++i)
			{
				(*listVAIter->terrainVA)[i].color = c;
			}
			target->draw(*listVAIter->terrainVA);
			for (int i = 0; i < vertexCount; ++i)
			{
				(*listVAIter->terrainVA)[i].color = Color::White;
			}
		}
		listVAIter = listVAIter->next;
	}
}

void GameSession::EnemiesCheckedMiniDraw( RenderTarget *target,
	sf::FloatRect &rect)
{
	for (list<Enemy*>::iterator it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		(*it)->CheckedMiniDraw(target, rect);
	}
}

void GameSession::DrawAllMapWires(
	sf::RenderTarget *target)
{
	Actor *p;
	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->DrawMapWires(target);
		}
	}
}

void TerrainPiece::UpdateBushFrame()
{
	/*bushFrame++;
	if( bushFrame == bushAnimLength * bushAnimFactor )
	{
		bushFrame = 0;
	}*/
}

void TerrainPiece::DrawBushes( sf::RenderTarget *target )
{
	for( list<DecorExpression*>::iterator it = bushes.begin(); 
		it != bushes.end(); ++it )
	{
		Tileset *ts = (*it)->layer->ts;
		target->draw( *(*it)->va, ts->texture );
	}
	//target->draw( 
}

void TerrainPiece::UpdateBushes()
{
	//int numBushes = bushVA->getVertexCount() / 4;

	//VertexArray &bVA = *bushVA;
	//IntRect subRect = ts_bush->GetSubRect( bushFrame / bushAnimFactor );

	/*for( int i = 0; i < numBushes; ++i )
	{
		bVA[i*4+0].texCoords = Vector2f( subRect.left, subRect.top );
		bVA[i*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
		bVA[i*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
		bVA[i*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );
	}*/
	//++bushFrame;
}

void TerrainPiece::HandleQuery( QuadTreeCollider *qtc )
{
	qtc->HandleEntrant( this );
}

bool TerrainPiece::IsTouchingBox( const sf::Rect<double> &r )
{
	return IsBoxTouchingBox( aabb, r );
}

void TerrainPiece::Draw(sf::RenderTarget *target)
{
	if (visible)
	{
		if (grassVA != NULL)
			target->draw(*grassVA, owner->ts_gravityGrass->texture);

		if (owner->usePolyShader)
		{
			sf::Rect<double> screenRect = owner->screenRect;
			sf::Rect<double> polyAndScreen;
			double rightScreen = screenRect.left + screenRect.width;
			double bottomScreen = screenRect.top + screenRect.height;
			double rightPoly = aabb.left + aabb.width;
			double bottomPoly = aabb.top + aabb.height;

			double left = std::max(screenRect.left, aabb.left);

			double right = std::min(rightPoly, rightScreen);

			double top = std::max(screenRect.top, aabb.top);

			double bottom = std::min(bottomScreen, bottomPoly);


			polyAndScreen.left = left;
			polyAndScreen.top = top;
			polyAndScreen.width = right - left;
			polyAndScreen.height = bottom - top;

			assert(pShader != NULL);
			target->draw(*terrainVA, pShader);
		}
		else
		{
			target->draw(*terrainVA);
		}

		if (owner->showTerrainDecor)
		{
			tr->Draw(target);
		}

		for (auto it = touchGrassCollections.begin(); it != touchGrassCollections.end(); ++it)
		{
			(*it)->Draw(target);
		}
	}
}

void GameSession::ResetShipSequence()
{
	Actor *player = GetPlayer( 0 );
	drain = false;
	player->action = Actor::RIDESHIP;
	player->frame = 0;
	PoiInfo *pi = poiMap["ship"];
	player->position = pi->pos;
	originalPos = player->position;
	shipSprite.setPosition( pi->pos.x - 13, pi->pos.y - 124 );
	//cloud0a.setpo
	shipSequence = true;
	shipSeqFrame = 0;
	shipStartPos = shipSprite.getPosition();//Vector2f( pi->pos.x, pi->pos.y );
	cloudVel = Vector2f( -40, 0 );
	relShipVel = Vector2f( 2, 0 );
	//#0055FF
	middleClouds.setFillColor( Color( 0x00, 0x55, 0xFF ) );
	int middleHeight = 540 * 4;
	middleClouds.setSize( Vector2f( 960, middleHeight ) );
	Vector2f botExtra( 0, middleHeight );
	//middleClouds.setOrigin( middleClouds.getLocalBounds().width / 2,
	//	middleClouds.getLocalBounds().height / 2 );

	IntRect sub0 = ts_w1ShipClouds0->GetSubRect( 0 );
	IntRect sub1 = ts_w1ShipClouds1->GetSubRect( 0 );
			
	Vector2f bottomLeft = Vector2f( pi->pos.x, pi->pos.y ) + Vector2f( -480, 270 ); 
	for( int i = 0; i < 3; ++i )
	{
		Vector2f xExtra( 480 * i, 0 );
		cloud0[i*4+0].position = xExtra + bottomLeft;
		cloud0[i*4+1].position = xExtra + bottomLeft + Vector2f( 0, -sub0.height / 2 );
		cloud0[i*4+2].position = xExtra + bottomLeft + Vector2f( sub0.width / 2, -sub0.height / 2 );
		cloud0[i*4+3].position = xExtra + bottomLeft + Vector2f( sub0.width / 2, 0 );

		cloud0[i*4+0].texCoords = Vector2f( 0, sub0.height );
		cloud0[i*4+1].texCoords = Vector2f( 0, 0 );
		cloud0[i*4+2].texCoords = Vector2f( sub0.width, 0 );
		cloud0[i*4+3].texCoords = Vector2f( sub0.width, sub0.height );

		cloud1[i*4+0].position = xExtra + bottomLeft;
		cloud1[i*4+1].position = xExtra + bottomLeft + Vector2f( 0, -sub1.height / 2 );
		cloud1[i*4+2].position = xExtra + bottomLeft + Vector2f( sub1.width / 2, -sub1.height / 2 );
		cloud1[i*4+3].position = xExtra + bottomLeft + Vector2f( sub1.width / 2, 0 );

		cloud1[i*4+0].texCoords = Vector2f( 0, sub1.height );
		cloud1[i*4+1].texCoords = Vector2f( 0, 0 );
		cloud1[i*4+2].texCoords = Vector2f( sub1.width, 0 );
		cloud1[i*4+3].texCoords = Vector2f( sub1.width, sub1.height );

		cloudBot0[i*4+0].position = botExtra + xExtra + bottomLeft;
		cloudBot0[i*4+1].position = botExtra + xExtra + bottomLeft + Vector2f( 0, sub0.height / 2 );
		cloudBot0[i*4+2].position = botExtra + xExtra + bottomLeft + Vector2f( sub0.width / 2, sub0.height / 2 );
		cloudBot0[i*4+3].position = botExtra + xExtra + bottomLeft + Vector2f( sub0.width / 2, 0 );

		cloudBot0[i*4+0].texCoords = Vector2f( 0, sub0.height );
		cloudBot0[i*4+1].texCoords = Vector2f( 0, 0 );
		cloudBot0[i*4+2].texCoords = Vector2f( sub0.width, 0 );
		cloudBot0[i*4+3].texCoords = Vector2f( sub0.width, sub0.height );

		cloudBot1[i*4+0].position = botExtra + xExtra + bottomLeft;
		cloudBot1[i*4+1].position = botExtra + xExtra + bottomLeft + Vector2f( 0, sub1.height / 2 );
		cloudBot1[i*4+2].position = botExtra + xExtra + bottomLeft + Vector2f( sub1.width / 2, sub1.height / 2 );
		cloudBot1[i*4+3].position = botExtra + xExtra + bottomLeft + Vector2f( sub1.width / 2, 0 );

		cloudBot1[i*4+0].texCoords = Vector2f( 0, sub1.height );
		cloudBot1[i*4+1].texCoords = Vector2f( 0, 0 );
		cloudBot1[i*4+2].texCoords = Vector2f( sub1.width, 0 );
		cloudBot1[i*4+3].texCoords = Vector2f( sub1.width, sub1.height );
	}

	middleClouds.setPosition( pi->pos.x - 480, pi->pos.y + 270 );
}

void GameSession::RespawnPlayer( int index )
{
	//Actor *player = GetPlayer( index );
	//numKeysCollected = 0;
	//player->framesNotGrinding = 0;
	//player->runeStep = 0;
	//player->runeLength = 0;
	//player->showRune = false;
	//player->bufferedAttack = Actor::JUMP;
	//soundNodeList->Reset();
	//scoreDisplay->Reset();
	//player->hitGoal = false;
	//currentZone = originalZone;
	//if( currentZone != NULL )
	//	keyMarker->SetStartKeysZone(currentZone);
	//if( player->currentCheckPoint == NULL )
	//{
	//	player->position = originalPos;
	//	
	//	//actually keys should be set based on which ones you had at the last checkpoint
	//	for( int i = 2; i < Gate::GateType::Count; ++i )
	//	{
	//		player->numKeys = 0;
	//		//player->hasKey[i] = 0;
	//	}
	//}
	//else
	//{
	//	player->position = player->currentCheckPoint->pos;

	//	//might take out checkpoints so idk how this would work
	//	//for( int i = 2; i < Gate::GateType::Count; ++i )
	//	//{
	//		//player->hasKey[i] = player->currentCheckPoint->hadKey[i];
	//	//}
	//}

	////player->seq = Actor::SEQ_NOTHING;
	//player->followerPos = player->position;
	//player->followerVel = V2d( 0, 0 );
	//player->enemiesKilledThisFrame = 0;
	//player->gateTouched = NULL;

	//if( poiMap.count( "ship" ) > 0 )
	//{
	//	ResetShipSequence();
	//}
	//else
	//{
	//	player->action = player->INTRO;
	//	player->frame = 0;
	//}
	//


	//player->velocity.x = 0;
	//player->velocity.y = 0;
	//player->reversed = false;
	//player->b.offset.y = 0;
	//player->b.rh = player->normalHeight;
	//player->facingRight = true;
	//player->offsetX = 0;
	//player->prevInput = ControllerState();
	//player->currInput = ControllerState();
	//player->ground = NULL;
	//player->grindEdge = NULL;
	//player->bounceEdge = NULL;
	//player->dead = false;
	//player->record = 0;
	//player->recordedGhosts = 0;
	//player->blah = false;
	//player->receivedHit = NULL;
	//player->speedParticleCounter = 1;
	//player->speedLevel = 0;
	//player->speedBarTarget = 0;//60;
	//player->currentSpeedBar = 0;//60;

	//player->bounceFlameOn = false;

	//if( player->hasPowerLeftWire )
	//{
	//	player->leftWire->Reset();
	//}
	//if( player->hasPowerRightWire )
	//{
	//	player->rightWire->Reset();
	//}
	//
	//player->lastWire = 0;
	//player->desperationMode = false;

	//player->flashFrames = 0;
	//
	//
	//if( powerRing != NULL )
	//	powerRing->ResetFull();
	////currentZone = NULL;
	//cam.zoomFactor = 1;
	//cam.pos.x = player->position.x;
	//cam.pos.y = player->position.y;
	//cam.offset = Vector2f( 0, 0 );
	//cam.manual = false;
	//cam.easing = false;
	//cam.rumbling = false;

	//player->hasDoubleJump = true;
	//player->hasAirDash = true;
	//player->hasGravReverse = true;

	//if( !cam.bossCrawler )
	//{
	//	cam.zoomFactor = 1;
	//	cam.zoomLevel = 0;
	//	cam.offset = Vector2f( 0, 0 );
	//}
	//

	//for( int i = 0; i < player->maxBubbles; ++i )
	//{
	//	player->bubbleFramesToLive[i] = 0;
	//	//if( player->bubbleFramesToLive[i] > 0 )
	//	//{
	//		
	//	//}
	//}

	//for( int i = 0; i < player->maxMotionGhosts; ++i )
	//{
	//	player->motionGhosts[i].setPosition( player->position.x, player->position.y );
	//}

	//player->SetExpr( Actor::Expr::Expr_NEUTRAL );
}

void GameSession::ClearFX()
{
	for( int i = 0; i < EffectLayer::Count; ++i )
	{
		Enemy *curr = effectLists[i];
		while( curr != NULL )
		{
			Enemy *next = curr->next;
			assert( curr->type == EnemyType::EN_BASICEFFECT );
			DeactivateEffect( (BasicEffect*)curr );

			curr = next;
		}
		effectLists[i] = NULL;
	}
}


void GameSession::NextFrameRestartLevel()
{
	nextFrameRestart = true;
}


void GameSession::RestartLevel()
{
	
	ClearEmitters();
	//AddEmitter(testEmit, EffectLayer::IN_FRONT);
	//testEmit->Reset();

	for (auto it = allVA.begin(); it != allVA.end(); ++it)
	{
		(*it)->Reset();
	}

	shardsCapturedField->Reset();

	nextFrameRestart = false;
	//accumulator = TIMESTEP + .1;
	currBroadcast = NULL;
	currStorySequence = NULL;

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

	goalPulse->Reset();
	//f->Reset();

	activeDialogue = NULL;

	fader->Reset();
	numKeysCollected = 0;

	if (adventureHUD != NULL)
	{
		adventureHUD->Reset();
	}

	//crawlerFightSeq->Reset();
	//crawlerAfterFightSeq->Reset();
	//enterNexus1Seq->Reset();
	activeSequence = NULL;

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


	//testEmit->SetPos(Vector2f(GetPlayer(0)->position));

//	currentZone = originalZone;
//	if( currentZone != NULL )
//		keyMarker->SetStartKeysZone(currentZone);

	if( poiMap.count( "ship" ) > 0 )
	{
		ResetShipSequence();
	}
	//currentZone = NULL;
	cam.Reset();

	/*if( !cam.bossCrawler )
	{
		cam.zoomFactor = 1;
		cam.zoomLevel = 0;
		cam.offset = Vector2f( 0, 0 );
	}*/

	for (int i = 0; i < 4; ++i)
	{
		Actor *player = GetPlayer(i);
		if (player != NULL)
			player->Respawn();
	}

	scoreDisplay->Reset();

	absorbParticles->Reset();
	absorbDarkParticles->Reset();
	absorbShardParticles->Reset();
	//player->Respawn();

	cam.pos.x = GetPlayer( 0 )->position.x;
	cam.pos.y = GetPlayer( 0 )->position.y;

	//RespawnPlayer();
	pauseFrames = 0;

	for (list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->Reset();
	}

	ResetEnemies();
	ResetPlants(); //eventually maybe treat these to reset like the rest of the stuff
	//only w/ checkpoints. but for now its always back

	//was resetting zones here before

	currentZone = NULL;
	if (originalZone != NULL)
	{
		ActivateZone(originalZone, true);
		keyMarker->SetStartKeysZone(originalZone);
	}
	//	originalZone->active = true;
	//
	//later don't relock gates in a level unless there is a "level reset"
	for( int i = 0; i < numGates; ++i )
	{
		gates[i]->Reset();
	}

	inactiveEnemyList = NULL;

	for( list<Barrier*>::iterator it = barriers.begin(); it != barriers.end(); ++it )
	{
		(*it)->triggered = false;
	}
				
	pauseImmuneEffects = NULL;
	cloneInactiveEnemyList = NULL;

	cam.SetManual( false );

	
	
	//inGameClock.restart();
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

bool GameSession::IsShardCaptured(ShardType shardType)
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


double GameSession::GetTriangleArea(p2t::Triangle * t)
{
	p2t::Point *p_0 = t->GetPoint(0);
	p2t::Point *p_1 = t->GetPoint(0);
	p2t::Point *p_2 = t->GetPoint(0);

	V2d p0(p_0->x, p_0->y);
	V2d p1(p_1->x, p_1->y);
	V2d p2(p_2->x, p_2->y);

	double len0 = length(p1 - p0);
	double len1 = length(p2 - p1);
	double len2 = length(p0 - p2);

	//s = .5 * (a + b + c)
	//A = sqrt( s(s - a)(s - b)(s - c) )

	double s = .5 * (len0 + len1 + len2);
	double A = sqrt(s * (s - len0) * (s - len1) * (s - len2));

	return A;
}

void TerrainPiece::AddDecorExpression(DecorExpression *exp)
{
	bushes.push_back(exp);
}

TerrainPiece::TerrainPiece(GameSession *p_owner)
	:owner(p_owner)
{
	groundva = NULL;
	slopeva = NULL;
	steepva = NULL;
	wallva = NULL;
	triva = NULL;
	flowva = NULL;
	plantva = NULL;
	decorLayer0va = NULL;
	bushVA = NULL;

	terrainVA = NULL;
	grassVA = NULL;

	grassSize = 128;
	grassSpacing = -60;
}

TerrainPiece::~TerrainPiece()
{
	if (tr != NULL)
		delete tr;

	for (auto it = bushes.begin(); it != bushes.end(); ++it)
	{
		delete (*it);
	}

	for (auto it = touchGrassCollections.begin(); it != touchGrassCollections.end(); ++it)
	{
		delete (*it);
	}

	delete groundva;
	delete slopeva;
	delete steepva;
	delete wallva;
	delete triva;
	delete flowva;
	delete plantva;
	delete decorLayer0va;
	delete bushVA;

	delete terrainVA;
	delete grassVA;
}

void TerrainPiece::Reset()
{
	for (auto it = touchGrassCollections.begin(); it != touchGrassCollections.end(); ++it)
	{
		(*it)->Reset();
	}
}

int TerrainPiece::GetNumGrass(Edge *e, bool &rem)
{
	rem = false;

	V2d v0 = e->v0;
	V2d v1 = e->v1;

	double len = length(v1 - v0);
	len -= grassSize / 2 + grassSpacing;
	double reps = len / (grassSize + grassSpacing);
	double remainder = reps - floor(reps);
	if (remainder > 0)
	{
		reps += 1; //for the last one
		rem = true;
	}
	reps += 1;

	int num = reps;

	return num;
}

void TerrainPiece::SetupGrass(Edge * e, int &i)
{
	VertexArray &grassVa = *grassVA;

	V2d v0 = e->v0;
	V2d v1 = e->v1;

	bool rem;
	int num = GetNumGrass(e, rem);

	V2d along = normalize(v1 - v0);
	V2d realStart = v0 + along * (double)(grassSize + grassSpacing);

	for (int j = 0; j < num; ++j)
	{
		V2d posd = realStart + along * (double)((grassSize + grassSpacing) * (j - 1));//v0 + normalize(v1 - v0) * ((grassSize + grassSpacing) * (j-1) + );

		if (j == 0)
		{
			posd = v0;
		}
		else if (j == num - 1 && rem)
		{
			//V2d prev = ;//v0 + (v1 - v0) * ((double)(j-1) / num);
			posd = v1 + normalize(v0 - v1) * (grassSize / 2.0 + grassSpacing);//(v1 + prev) / 2.0;
		}

		Vector2f pos(posd.x, posd.y);


		Vector2f topLeft = pos + Vector2f(-grassSize / 2, -grassSize / 2);
		Vector2f topRight = pos + Vector2f(grassSize / 2, -grassSize / 2);
		Vector2f bottomLeft = pos + Vector2f(-grassSize / 2, grassSize / 2);
		Vector2f bottomRight = pos + Vector2f(grassSize / 2, grassSize / 2);

		//grassVa[i*4].color = Color( 0x0d, 0, 0x80 );//Color::Magenta;
		grassVa[i * 4].color.a = 0;
		grassVa[i * 4].position = topLeft;
		grassVa[i * 4].texCoords = Vector2f(0, 0);

		//grassVa[i*4+1].color = Color::Blue;
		//borderVa[i*4+1].color.a = 10;
		grassVa[i * 4 + 1].color.a = 0;
		grassVa[i * 4 + 1].position = bottomLeft;
		grassVa[i * 4 + 1].texCoords = Vector2f(0, grassSize);

		//grassVa[i*4+2].color = Color::Blue;
		//borderVa[i*4+2].color.a = 10;
		grassVa[i * 4 + 2].color.a = 0;
		grassVa[i * 4 + 2].position = bottomRight;
		grassVa[i * 4 + 2].texCoords = Vector2f(grassSize, grassSize);

		//grassVa[i*4+3].color = Color( 0x0d, 0, 0x80 );
		//borderVa[i*4+3].color.a = 10;
		grassVa[i * 4 + 3].color.a = 0;
		grassVa[i * 4 + 3].position = topRight;
		grassVa[i * 4 + 3].texCoords = Vector2f(grassSize, 0);
		++i;
	}
}

struct PlantInfo
{
	PlantInfo(Edge*e, double q, double w)
		:edge(e), quant(q), quadWidth(w)
	{
	}
	Edge *edge;
	double quant;
	double quadWidth;
};


void TerrainPiece::AddTouchGrass( int gt )
{
	list<PlantInfo> info;

	TouchGrass::TouchGrassType gType = (TouchGrass::TouchGrassType)gt;

	int tw = TouchGrass::GetQuadWidth( gType );

	Edge *startEdge = tr->startEdge;

	Edge *te = startEdge;
	do
	{
		bool valid = true;

		EdgeAngleType eat = GetEdgeAngleType(te->Normal());
		if (eat == EDGE_FLAT || eat == EDGE_SLOPED)
		{
			valid = true;
		}
		else
		{
			valid = false;
		}
		//valid = true;

		if (valid)
		{
			double len = length(te->v1 - te->v0);
			int numQuads = len / tw;
			bool tooThin = false;
			double quadWidth = tw;//len / numQuads;
			if (numQuads == 0)
			{
				tooThin = true;
				numQuads = 1;
			}
			if (numQuads > 0)
			{
				for (int i = 0; i < numQuads; ++i)
				{
					if (TouchGrass::IsPlacementOkay(gType, eat,
						te, i))
					{
						if (tooThin)
						{
							info.push_back(PlantInfo(te, te->GetLength() / 2.0, quadWidth));
						}
						else
						{
							info.push_back(PlantInfo(te, quadWidth * i + quadWidth / 2, quadWidth));
						}
						
					}
				}
			}
		}
		te = te->edge1;
	} while (te != startEdge);

	int infoSize = info.size();
	int vaSize = infoSize * 4;

	if (infoSize == 0)
	{
		return;
	}

	TouchGrassCollection *coll = new TouchGrassCollection;
	touchGrassCollections.push_back(coll);

	//cout << "number of plants: " << infoSize << endl;
	coll->touchGrassVA = new Vertex[vaSize];

	coll->ts_grass = TouchGrassCollection::GetTileset( owner, gType);
	coll->gType = gType;
	coll->numTouchGrasses = infoSize;

	int vaIndex = 0;
	for (list<PlantInfo>::iterator it = info.begin(); it != info.end(); ++it)
	{
		coll->CreateGrass(vaIndex, (*it).edge, (*it).quant);
		vaIndex++;
	}
}

void TerrainPiece::QueryTouchGrass(QuadTreeCollider *qtc, sf::Rect<double> &r)
{
	for (auto it = touchGrassCollections.begin(); it != touchGrassCollections.end(); ++it)
	{
		(*it)->Query(qtc, r);
	}
}

void TerrainPiece::UpdateTouchGrass()
{
	for (auto it = touchGrassCollections.begin(); it != touchGrassCollections.end(); ++it)
	{
		(*it)->UpdateGrass();
	}
}

void TerrainPiece::UpdateBushSprites()
{
	for (list<DecorExpression*>::iterator it = bushes.begin();
		it != bushes.end(); ++it)
	{
		(*it)->UpdateSprites();
	}
}

DecorExpression * GameSession::CreateDecorExpression(  DecorType dType,
	int bgLayer,
	Edge *startEdge)
{
	int minApart;
	int maxApart;
	CubicBezier apartBezier;
	int minPen;
	int maxPen;
	CubicBezier penBez;
	int animFactor = 1;

	switch (dType)
	{
	case D_W1_BUSH_NORMAL:
		minApart = 20;
		maxApart = 300;
		apartBezier = CubicBezier(0, 0, 1, 1);
		minPen = 20;
		maxPen = 1000;
		penBez = CubicBezier(0, 0, 1, 1);
		animFactor = 8;
		break;
	case D_W1_ROCK_1:
	case D_W1_ROCK_2:
	case D_W1_ROCK_3:
	case D_W1_PLANTROCK:
	case D_W1_GRASSYROCK:
	default:
		minApart = 500;
		maxApart = 700;
		apartBezier = CubicBezier(0, 0, 1, 1);
		minPen = 200;
		maxPen = 1200;
		penBez = CubicBezier(0, 0, 1, 1);
		animFactor = 1;
		break;
	}

	//assert( positions.empty() );

	int veinLoopWait = 30;

	DecorLayer *layer = NULL;
	if( decorLayerMap.count(dType) == 0 )
	{
		//int GameSession::TerrainPiece::bushFrame = 0;
		//int GameSession::TerrainPiece::bushAnimLength = 20;
		//int GameSession::TerrainPiece::bushAnimFactor = 8;

		Tileset *ts_d = NULL;
		switch (dType)
		{
		case D_W1_BUSH_NORMAL:
			ts_d = GetTileset("Env/bush_01_64x64.png", 64, 64);
			layer = new DecorLayer(ts_d, 20, 8);
			break;
		case D_W1_ROCK_1:
			ts_d = GetTileset("Env/rock_1_01_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_ROCK_2:
			ts_d = GetTileset("Env/rock_1_02_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_ROCK_3:
			ts_d = GetTileset("Env/rock_1_03_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_PLANTROCK:
			ts_d = GetTileset("Env/bush_1_01_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_GRASSYROCK:
			ts_d = GetTileset("Env/bush_1_02_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_VEINS1:
			ts_d = GetTileset("Env/veins_w1_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS2:
			ts_d = GetTileset("Env/veins_w1_2_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS3:
			ts_d = GetTileset("Env/veins_w1_3_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS4:
			ts_d = GetTileset("Env/veins_w1_4_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS5:
			ts_d = GetTileset("Env/veins_w1_5_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS6:
			ts_d = GetTileset("Env/veins_w1_6_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		}
		
		decorLayerMap[dType] = layer;
	}
	else
	{
		layer = decorLayerMap[dType];
	}

	assert( layer != NULL );
	list<Vector2f> positions;
	//int minDistanceApart = 10;
	//int maxDistanceApart = 300;
	//int minPen = 20;
	//int maxPen = 200;
	double penLimit;

	Edge *curr = startEdge;
	double quant = 0;
	double lenCurr = length( startEdge->v1 - startEdge->v0 );

	double travelDistance;
	double penDistance;
	int diffApartMax = maxApart - minApart;
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
		travelDistance = minApart + r;

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
		rayStart = curr->GetPoint( quant );

		rPen = rand() % diffPenMax;
		penDistance = minPen + rPen; //minpen times 2 cuz gotta account for the other side too

		rayEnd = rayStart - cn * ( penDistance + minPen );
		rayIgnoreEdge = curr;

		RayCast( this, qt->startNode, rayStart, rayEnd );

		if (rcEdge != NULL)
		{
			V2d rcPos = rcEdge->GetPoint(rcQuantity);
			continue;
			/*if (length(rcPos - rayStart) < minPen || length( rcPos - rayEnd ) < minPen)
			{
				continue;
			}*/
			//penLimit = length(rcEdge->GetPoint(rcQuantity) - rayStart);
			
		}
		/*diffPenMax = maxPen;
		if( rcEdge != NULL )
		{
			penLimit = length( rcEdge->GetPoint( rcQuantity ) - rayStart );
			diffPenMax = (int)penLimit - minApart;
			if( diffPenMax == 0 || penLimit < 100 )
				continue;
		}*/

		/*rPen = rand() % diffPenMax;
		penDistance = minPen + rPen;*/
		
		pos = curr->GetPoint( quant ) - curr->Normal() * penDistance;

		positions.push_back( Vector2f( pos.x, pos.y ) );
		//will have to do a raycast soon. ignore for now
		//curr = curr->edge1;
	}

	if( positions.size() == 0 )
		return NULL;
	
	DecorExpression *expr = new DecorExpression( positions, layer );

	return expr;
}




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
		V2d groundPoint = (*it).edge->GetPoint( (*it).quant );
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

VertexArray * GameSession::SetupBorderQuads( int bgLayer, 
	Edge *startEdge, Tileset *ts, int (*ValidEdge)(sf::Vector2<double> & ) )
{
	/*int worldNum = 0;
	if( envType < 1 )
	{
		worldNum = 0;
	}
	else if( envType < 2 )
	{

	}*/
	//int worldNum = (int)envType; //temporary

	rayMode = "border_quads";
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

	int tw = ts->tileWidth;//128;//64;
	//int th = 512;
	int numTotalQuads = 0;
	double intersect = 10;//tw / 6.0;//48;
	double extraLength = 0;//32.0;
	Edge *te = startEdge;//edges[currentEdgeIndex];

	map<Edge*, int> numQuadMap;

	do
	{
		V2d eNorm = te->Normal();
		int valid = ValidEdge( eNorm );
		if( valid != -1 )//eNorm.x == 0 )
		{
			double len = length(te->v1 - te->v0);// +extraLength * 2;

			int numQuads = 0;
			//if (len < 8 )//tw)
			//{
			//	numQuads = 0;
			//}
		//else
			if (len <= tw - extraLength * 2)
			{
				numQuads = 1;
			}
			else
			{
				numQuads = 2;
				double firstRight = tw - extraLength - intersect;
				double endLeft = (len + extraLength) - tw + intersect;

				//optimize later to not be a loop
				while (firstRight <= endLeft)
				{
					firstRight += tw - intersect;
					++numQuads;
				}
			}

			// = ceil(len / (tw - intersect));
			//double quadWidth = len / numQuads;
			numTotalQuads += numQuads;
			numQuadMap[te] = numQuads;
		}
		te = te->edge1;
	}
	while( te != startEdge );

	if (numTotalQuads == 0)
	{
		return NULL;
	}

	VertexArray *currVA = new VertexArray( sf::Quads, numTotalQuads * 4 );
	
	VertexArray &va = *currVA;
			
	int extra = 0;
	te = startEdge;
	int varietyCounter = 0;
	
	do
	{
		V2d eNorm = te->Normal();
		int valid = ValidEdge( eNorm );
		if( valid != -1 )
		{
			double len = length(te->v1 - te->v0);// +extraLength * 2;
			int numQuads = numQuadMap[te];//ceil( len / tw ); 
			if (numQuads == 0)
			{
				te = te->edge1;
				continue;
			}
			double quadWidth = ts->tileWidth;;//len / numQuads;

			V2d along = normalize( te->v1 - te->v0 );
			V2d other( along.y, -along.x );

			
			double out = 16;//40;//16;
			double in = 64 - out;//256 - out;//; - out;
			

			V2d startInner = te->v0 - along * extraLength - other * in;
			V2d startOuter = te->v0 - along * extraLength + other * out;

			double startAlong = -extraLength;
			double endAlong;


			for( int i = 0; i < numQuads; ++i )
			{
				//worldNum * 5
				//int valid = ValidEdge( eNorm );
				//add (worldNum * 5) to realIndex to get the correct borders
				int realIndex = valid;// *4 + varietyCounter;//32 + varietyCounter;
				//cout << "real Index: " << realIndex << ", valid: " << valid << ", variety: " << varietyCounter << endl;
				IntRect sub = ts->GetSubRect( realIndex );
				//cout << "left: " << sub.left << ", top: " << sub.top << 
				//	", w: " << sub.width << ", h: " << sub.height << endl;
				if (numQuads == 1)
				{
					startAlong = len / 2 - tw / 2;
					endAlong = startAlong + tw;
				}
				else if (numQuads == 2)
				{
					if (i == 0)
					{
						startAlong = -extraLength;
						endAlong = startAlong + tw;
					}
					else
					{
						endAlong = len + extraLength;
						startAlong = endAlong - tw;
					}
				}
				else if (numQuads % 2 == 0 )
				{
					if (i < numQuads / 2 )
					{
						startAlong = -extraLength + ((tw - intersect) * i);
						endAlong = startAlong + tw;
					}
					else 
					{
						endAlong = len + extraLength - ((tw - intersect) * ((numQuads - 1) - i));
						startAlong = endAlong - tw;
					}
				}
				else
				{
					if (i * 2 == numQuads - 1)
					{
						startAlong = len / 2 - tw / 2;
						endAlong = startAlong + tw;
					}
					else if (i < numQuads / 2)
					{
						startAlong = -extraLength + ((tw - intersect) * i);
						endAlong = startAlong + tw;
					}
					else
					{
						endAlong = len + extraLength - ((tw - intersect) * (( numQuads -1 ) - i));
						startAlong = endAlong - tw;
					}
				}
				
				
				

				V2d currStartInner = startInner + startAlong * along;
				V2d currStartOuter = startOuter + startAlong * along;
				V2d currEndInner = startInner + endAlong * along;
				V2d currEndOuter = startOuter + endAlong * along;
						
				double realHeight0 = 64;//256;//in;//sub.height;
				double realHeight1 = 64;//256;//in;//sub.height;
				
				double d0 = dot( normalize( te->edge0->v0 - te->v0 ), normalize( te->v1 - te->v0 ) );
				double c0 = cross( normalize( te->edge0->v0 - te->v0 ), normalize( te->v1 - te->v0 ) );

				double d1 = dot( normalize( te->edge1->v1 - te->v1 ), normalize( te->v0 - te->v1 ) );
				double c1 = cross( normalize( te->edge1->v1 - te->v1 ), normalize( te->v0 - te->v1 ) );

				//if( d0 <= 0

				rcEdge = NULL;
				rayIgnoreEdge = te;
				rayStart = te->v0 - along * extraLength + ( startAlong ) * along;
				rayEnd = currStartInner;//te->v0 + (double)i * quadWidth * along - other * in;
				RayCast( this, qt->startNode, rayStart, rayEnd );


				//start ray
				if( rcEdge != NULL )
				{
					//currStartInner = rcEdge->GetPoint( rcQuantity );
					//realHeight0 = length( currStartInner - currStartOuter );
				}

				rcEdge = NULL;
				rayStart = te->v0 - along * extraLength + ( endAlong ) * along;
				rayEnd = currEndInner;
				RayCast( this, qt->startNode, rayStart, rayEnd );

				//end ray
				if( rcEdge != NULL )
				{
					//currEndInner =  rcEdge->GetPoint( rcQuantity );//te->v0 + endAlong * along - rcQuantity * other;
					//realHeight1 = length( currEndInner - currStartOuter );
				}

				
				//RayCast( this, terrainTree, position, V2d( position.x - 100, position.y ) );
				
				//if( i == 0 && d0 <= 0 )
				//{
				//	Edge *prev = te->edge0;
				//	V2d pNorm = prev->Normal();
				//	V2d prevEndInner = prev->v1 - pNorm * in;
				//	V2d prevEndOuter = prev->v1 + pNorm * out;
				//	//V2d prevEndOuter = prev->v1 - 

				//	va[extra + i * 4 + 0].position = Vector2f( ( currStartOuter.x + prevEndOuter.x ) / 2.0, 
				//		( currStartOuter.y + prevEndOuter.y ) / 2.0 );
				//	
				//	va[extra + i * 4 + 3].position = Vector2f( (currStartInner.x + prevEndInner.x) / 2.0, 
				//		( currStartInner.y + prevEndInner.y ) / 2.0  );
				//}
				//else
				{
					Vector2f a = Vector2f( currStartOuter.x, currStartOuter.y );
					Vector2f b = Vector2f( currStartInner.x, currStartInner.y );
					/*a += Vector2f( .5, .5 );
					b += Vector2f( .5, .5 );
					a.x = floor( a.x );
					a.y = floor( a.y );
					b.x = floor( b.x );
					b.y = floor( b.y );*/

					va[extra + i * 4 + 0].position = a;
					va[extra + i * 4 + 3].position = b;
				}

				/*if( i == numQuads - 1 && d1 <= 0 )
				{
					Edge *next = te->edge1;
					V2d nNorm = next->Normal();
					V2d nextStartInner = next->v0 - nNorm * in;
					V2d nextStartOuter = next->v0 + nNorm * out;
					va[extra + i * 4 + 2].position = Vector2f( ( currEndInner.x + nextStartInner.x ) / 2, 
						( currEndInner.y + nextStartInner.y ) / 2 );

					va[extra + i * 4 + 1].position = Vector2f( ( currEndOuter.x + nextStartOuter.x ) / 2.0, 
						( currEndOuter.y + nextStartOuter.y ) / 2.0 );

				}
				else*/
				{
					Vector2f c = Vector2f( currEndInner.x, currEndInner.y);
					Vector2f d = Vector2f( currEndOuter.x, currEndOuter.y);

					/*c += Vector2f( .5, .5 );
					d += Vector2f( .5, .5 );
					c.x = floor( c.x );
					c.y = floor( c.y );
					d.x = floor( d.x );
					d.y = floor( d.y );*/

					va[extra + i * 4 + 2].position = c;
					va[extra + i * 4 + 1].position = d;
				}

				/*va[extra + i * 4 + 2].position = Vector2f( currEndInner.x, currEndInner.y );
				va[extra + i * 4 + 3].position = Vector2f( currStartInner.x, currStartInner.y );*/
				
				
				va[extra + i * 4 + 0].texCoords = Vector2f( sub.left, sub.top );
				va[extra + i * 4 + 1].texCoords = Vector2f( sub.left + sub.width, sub.top );
				va[extra + i * 4 + 2].texCoords = Vector2f(sub.left + sub.width, sub.top + sub.height);//realHeight1);
				va[extra + i * 4 + 3].texCoords = Vector2f(sub.left, sub.top + sub.height);//+ realHeight0);

				/*va[extra + i * 4 + 0].color = COLOR_BLUE;
				va[extra + i * 4 + 1].color = COLOR_YELLOW;
				va[extra + i * 4 + 2].color = COLOR_MAGENTA;
				va[extra + i * 4 + 3].color = COLOR_TEAL;
*/
				++varietyCounter;
				if( varietyCounter == 4 )//32 )
				{
					varietyCounter = 0;
				}

				startAlong += tw - intersect;
			}

			extra += numQuads * 4;
		}

		
		te = te->edge1;
	}
	while( te != startEdge );

	
	return currVA;
}

sf::VertexArray * GameSession::SetupEnergyFlow1( int bgLayer, Edge *start, Tileset *ts )
{
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
	//goalPos;
	rayMode = "energy_flow";

	Edge *te = start;

	//step one, get a list of the entrance points
	//check their ray distance and determine how many quads we want total,
		//while also storing start and end vectors for each quad

	//store start point and end point for each line
	cout << "part1  of the algorithm" << endl;
	int numTotalQuads = 0;
	list<pair<V2d,V2d>> rays;
	do
	{
		V2d norm = te->Normal();
		V2d v0 = te->v0;
		V2d v1 = te->v1;
		V2d along = normalize( v1 - v0 );
		double c = cross( goalPos - v0, along );
		double maxRayLength = 1000;
		//eventually make this larger because don't want to do it too close to parallel
		if( c > 0 ) //facing the goal
		{
			double len = length( te->v1 - te->v0 );
			V2d mid = (te->v0 + te->v1) / 2.0;
			double lenMid = length( goalPos - mid );
			double transition = 1000.0;
			double spacing = 8 + 8 * ( lenMid / transition );
			double width = 8;

			//double fullSpace = spacing + width;

			//int numStartPoints = len / fullSpace;
			//double sideSpacing = ( len - numStartPoints * fullSpace ) / 2;

			//for( int i = 0; i < numStartPoints; ++i )
			double quant = 4; //start point
			int quads = 0;
			while( quant < len - width )
			{
				double rayLength = 0;
				V2d rayPoint;

				rcEdge = NULL;
				rayStart = v0 + along * quant;
				rayIgnoreEdge = te;
				V2d goalDir = normalize( rayStart - goalPos );
				//cout << "goal dir: " << goalDir.x << ", " << goalDir.y << ", rayStart: " << rayStart.x << ", " << rayStart.y << 
				//	", gaolPos: " << goalPos.x << ", " << goalPos.y << endl;
				rayEnd = rayStart + goalDir * maxRayLength;//rayStart - norm * maxRayLength;
				RayCast( this, qt->startNode, rayStart, rayEnd );

				//start ray
				if( rcEdge != NULL )
				{
					rayPoint = rcEdge->GetPoint( rcQuantity );
					
					rays.push_back( pair<V2d,V2d>(rayStart, rayPoint) );
					
					
					//currStartInner = rcEdge->GetPoint( rcQuantity );
					//realHeight0 = length( currStartInner - currStartOuter );
				}
				else
				{
					rays.push_back( pair<V2d,V2d>(rayStart, rayEnd) );
				}

				//rays.push_back(  pair<V2d,V2d>(rayStart, rayEnd) );

				quant += spacing + width;
				++quads;
			}

			numTotalQuads += quads;
			//numTotalQuads += numQuads;	
			
		}
		te = te->edge1;
	}
	while( te != start );

	if( numTotalQuads == 0 )
	{
		return NULL;
	}

	cout << "part2  of the algorithm: " << numTotalQuads << endl;
	VertexArray *VA = new VertexArray( sf::Quads, numTotalQuads * 4 );

	VertexArray &va = *VA;

	int index = 0;
	for( list<pair<V2d,V2d>>::iterator it = rays.begin(); it != rays.end(); ++it )
	{
		
		V2d start = (*it).first;
		V2d end = (*it).second;

		V2d along = normalize( end - start );
		V2d other( along.y, -along.x );


		double width = 8;
		V2d startLeft = start; //- other * width / 2.0;
		V2d startRight = start + other * width;//width / 2.0;
		V2d endLeft = end;// - other * width / 2.0;
		V2d endRight = end + other * width;// * width / 2.0;

		

		va[index*4+0].color = Color::Red;
		va[index*4+1].color = Color::Red;
		va[index*4+2].color = Color::Red;
		va[index*4+3].color = Color::Red;

		va[index*4+0].position = Vector2f( startLeft.x, startLeft.y );
		va[index*4+1].position = Vector2f( startRight.x, startRight.y );
		va[index*4+2].position = Vector2f( endRight.x, endRight.y );
		va[index*4+3].position = Vector2f( endLeft.x, endLeft.y );


		//va[index*4+3].position = //Color::Red;

		index++;
	}

	return VA;

	//int extra = 0;
	//do
	//{
	//	V2d norm = te->Normal();
	//	V2d v0 = te->v0;
	//	V2d v1 = te->v1;
	//	V2d along = normalize( v1 - v0 );
	//	double c = cross( goalPos - v0, along );
	//	//eventually make this larger because don't want to do it too close to parallel
	//	if( c > 0 ) //facing the goal
	//	{
	//		double len = length( te->v1 - te->v0 );
	//		double spacing = 8;

	//		int numQuads = len / spacing;

	//		double sideSpacing = ( len - numQuads * spacing ) / 2;

	//		va[extra + i * 4 + 0].color = Color::Red;
	//		va[extra + i * 4 + 1].color = Color::Red;
	//		va[extra + i * 4 + 2].color = Color::Red;
	//		va[extra + i * 4 + 3].color = Color::Red;

	//		for( int i = 0; i < numQuads; ++i )
	//		{
	//			//va[extra + i * 4 + 0].po
	//		}

	//					

	//	}

	//	extra += numQuads * 4;
	//	te = te->edge1;
	//}
	//while( te != start );
}

typedef pair<V2d, V2d> pairV2d;
sf::VertexArray * GameSession::SetupEnergyFlow()
{
	int bgLayer = 0;
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
	//goalPos;
	rayMode = "energy_flow";
	//rayIgnoreEdge1 = NULL;
	//rayIgnoreEdge = NULL;

	double angle = 0;
	double divs = 64;
	double moveAngle = (2 * PI) / divs;
	double tau = 2 * PI;
	double startRadius = 50;
	bool insideTerrain = false;
	bool knowInside = false;
	double rayLen = 100;
	double width = 16;

	list<list<pair<V2d,bool>>> allInfo;
	double rayCheck = 0;

	for( int i = 0; i < divs; ++i )
	{
		rayIgnoreEdge1 = NULL;
		rayIgnoreEdge = NULL;

		allInfo.push_back( list<pair<V2d,bool>>() );
		list<pair<V2d,bool>> &pointList = allInfo.back();

		double angle = (tau / divs) * i;
		V2d rayDir( cos( angle ), sin( angle ) );

		rayStart = goalPos + rayDir * startRadius;
		rayEnd = rayStart + rayDir * rayLen;
		//rayIgnoreEdge->
		//while( rcEdge 
		bool rayOkay = rayEnd.x >= mh->leftBounds && rayEnd.y >= mh->topBounds && rayEnd.x <= mh->leftBounds + mh->boundsWidth
			&& rayEnd.y <= mh->topBounds + mh->boundsHeight;
		
		
		Edge *cEdge = NULL;
		//list<pair<V2d, bool>> pointList; //if true, then its facing the ray

		
		while( rayOkay )
		{
			//cout << "ray start: " << rayStart.x << ", " << rayStart.y << endl;
			rcEdge = NULL;

			RayCast( this, qt->startNode, rayStart, rayEnd );
			//rayStart = v0 + along * quant;
			//rayIgnoreEdge = te;
			//V2d goalDir = normalize( rayStart - goalPos );
			//rayEnd = rayStart + goalDir * maxRayLength;//rayStart - norm * maxRayLength;
			
			
			//start ray
			if( rcEdge != NULL )
			{
				cout << "point list size: " << pointList.size() << endl;
				if( rcEdge->edgeType == Edge::BORDER || rcEdge->edgeType == Edge::CLOSED_GATE )
				{
				//	cout << "secret break" << endl;
					break;
				}
				
				rayIgnoreEdge1 = rayIgnoreEdge;
				rayIgnoreEdge = rcEdge;

				V2d rn = rcEdge->Normal();
				double d = dot( rn, rayDir );
				V2d hitPoint = rcEdge->GetPoint( rcQuantity );
				if( d > 0 )
				{
					if( pointList.size() > 0 && pointList.back().second == false )
					{
						pointList.pop_back();
						cout << "goal lines failing here: " << i << " although nothing seems wrong"  
							<< endl;
						//assert( 0 );
					}
					else
					{
						//pointList.pop_back
						pointList.push_back( pair<V2d,bool>( hitPoint, false ) ); //not facing the ray, so im inside
						//cout << "adding false: " << hitPoint.x << ", " << hitPoint.y << "    " << pointList.size() << endl;
					}
				}
				else if( d < 0 )
				{
					if( pointList.size() > 0 && pointList.back().second == true)
					{
						pointList.pop_back();
						//cout << "failing here111 " << i << endl;
						//assert( 0 ); //commented out this assert for testing
					}
					else
					{
						
						pointList.push_back( pair<V2d,bool>( hitPoint, true ) ); // facing the ray, so im outside
						//cout << "adding true: " << hitPoint.x << ", " << hitPoint.y << "    " << pointList.size() << endl;
					}
				}
				else
				{

				}
				//rayPoint = rcEdge->GetPoint( rcQuantity );	
				//rays.push_back( pair<V2d,V2d>(rayStart, rayPoint) );
				rayStart = hitPoint;
				rayEnd = hitPoint + rayDir * rayLen;

				
					
				//currStartInner = rcEdge->GetPoint( rcQuantity );
				//realHeight0 = length( currStartInner - currStartOuter );
			
			}
			else
			{
				rayStart = rayEnd;
				rayEnd = rayStart + rayDir * rayLen;
			}

			double oldRayCheck = rayCheck;
			rayCheck = length((goalPos + rayDir * startRadius) - rayEnd);

			if (rayCheck == oldRayCheck)
			{
				rayOkay = false;
			}
			else
			{
				rayOkay = rayCheck <= 10000;
			}
			//cout << "rayLen: " << rayLen << endl;
			
			//rayOkay = rayEnd.x >= leftBounds && rayEnd.y >= topBounds && rayEnd.x <= leftBounds + boundsWidth 
			//	&& rayEnd.y <= topBounds + boundsHeight;
		}

		if( pointList.size() > 0 )
		{
			if( pointList.front().second == false )
			{
			//	cout << "adding to front!" << endl;
				//pointList.pop_front();
				pointList.push_front( pair<V2d,bool>( goalPos + rayDir * startRadius, true ) );
			}
			if( pointList.back().second == true )
			{
				//pointList.pop_back();
			//	cout << "popping from back!" << endl;
				pointList.push_back( pair<V2d,bool>( rayEnd, false ) );
			}
		}

		assert( pointList.size() % 2 == 0 );

		//true then false

		//always an even number of them
	}

	if( allInfo.empty() )
	{
		return NULL;
	}

	int totalPoints = 0;
	for(list<list<pair<V2d,bool>>>::iterator it = allInfo.begin(); it != allInfo.end(); ++it )
	{
		list<pair<V2d,bool>> &pointList = (*it);
		totalPoints += pointList.size();
	}

	cout << "number of quads: " << totalPoints / 2 << endl;
	VertexArray *VA = new VertexArray( sf::Quads, (totalPoints / 2) * 4 );
	VertexArray &va = *VA;
	int extra = 0;
	for(list<list<pair<V2d,bool>>>::iterator it2 = allInfo.begin(); it2 != allInfo.end(); ++it2 )
	{
		list<pair<V2d,bool>> &pointList = (*it2);
		for(list<pair<V2d,bool>>::iterator it = pointList.begin(); it != pointList.end(); ++it )
		{
			V2d startPoint = (*it).first;
			++it;
			V2d endPoint = (*it).first;

			V2d along = normalize( endPoint - startPoint );
			V2d other( along.y, -along.x );

			V2d startLeft = startPoint - other * width / 2.0 + along * 16.0;
			V2d startRight = startPoint + other * width / 2.0 + along * 16.0;
			V2d endLeft = endPoint - other * width / 2.0 - along * 16.0;
			V2d endRight = endPoint + other * width / 2.0 - along * 16.0;

			va[extra + 0].color = Color::Red;
			va[extra + 1].color = Color::Red;
			va[extra + 2].color = Color::Red;
			va[extra + 3].color = Color::Red;

			va[extra + 0].position = Vector2f( startLeft.x, startLeft.y );
			va[extra + 1].position = Vector2f( startRight.x, startRight.y );
			va[extra + 2].position = Vector2f( endRight.x, endRight.y );
			va[extra + 3].position = Vector2f( endLeft.x, endLeft.y );

			extra += 4;
		}
	}

	return VA;
}

sf::VertexArray * GameSession::SetupBorderTris( int bgLayer, Edge *startEdge, Tileset *ts )
{

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

	int tw = 32;
	int th = 128;
	//double an = th * PI / 6;
	double an = PI / 12.0;
	
	int numTotalTris = 0;
	Edge *te = startEdge;
	do
	{
		Edge *e1 = te->edge1;
		V2d eNorm = te->Normal();
		V2d along = normalize( te->v1 - te->v0 );
		V2d nextAlong = normalize( e1->v1 - e1->v0 );
		double c = cross( nextAlong, along );
		if( c > 0 )
		{
			V2d endVec = normalize( te->v0 - te->v1 );
			V2d startVec = normalize( e1->v1 - te->v1 );

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

			/*double temp = startAngle;
			startAngle = endAngle;
			endAngle = temp;*/

			if( endAngle > startAngle )
			{
				startAngle += 2 * PI;
			}

			

			double angleDiff = (startAngle - endAngle);

			int numTris = angleDiff / an;
			//go counter clockwise


			
			
				
			/*if( numTris == 0 )
			{
				numTris = 1;
			}*/

			numTotalTris += numTris;
		}
		te = te->edge1;
	}
	while( te != startEdge );

	if( numTotalTris == 0 )
	{
		//cout << "no tris here" << endl;
		return NULL;
	}
	else
	{
		//cout << "numtotaltris: " << numTotalTris << endl;
	}

	VertexArray *currVA = new VertexArray( sf::Quads, numTotalTris * 4 );
	

	IntRect sub = ts->GetSubRect( 0 );

	VertexArray &va = *currVA;

	int extra = 0;
	te = startEdge;
	int varietyCounter = 0;
	do
	{
		Edge *e1 = te->edge1;
		V2d eNorm = te->Normal();
		V2d along = normalize( te->v1 - te->v0 );
		V2d nextAlong = normalize( e1->v1 - e1->v0 );
		double c = cross( nextAlong, along );
		if( c > 0 )
		{
			V2d endVec = normalize( te->v0 - te->v1 );
			endVec = -V2d( endVec.y, -endVec.x );
			endVec = -te->Normal();
			V2d startVec = normalize( e1->v1 - te->v1 );
			startVec = -V2d( startVec.y, -startVec.x );
			startVec = -e1->Normal();

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

			/*double temp = startAngle;
			startAngle = endAngle;
			endAngle = temp;*/

			if( endAngle > startAngle )
			{
				startAngle += 2 * PI;
			}

			double angleDiff = (startAngle - endAngle);
			int numTris = angleDiff / an;
			//cout << "angleDiff: " << angleDiff << endl;
			
			
			//double triWidth = (startAngle - endAngle) / numTris;
			
			V2d centerPos( te->v1.x, te->v1.y );
			for( int i = 0; i < numTris; ++i )
			{
				double currAngle = startAngle - (double)i/numTris * angleDiff;
				double nextAngle = startAngle - (double)(i+1)/numTris * angleDiff;
				V2d currVec = V2d( cos( currAngle), -sin( currAngle ) );

				V2d currNorm( currVec.y, -currVec.x );

				V2d nextVec = V2d( cos( nextAngle ), -sin( nextAngle ) );

				//length( //length( inside - insideNext);

				V2d inside = centerPos + currVec * 48.0 - currNorm * (double)tw / 2.0;
				V2d insideNext = centerPos + nextVec * 48.0 + currNorm * (double)tw / 2.0;
				
				
				V2d outLeft = centerPos - currNorm * (double)tw / 2.0 - currVec * 16.0;
				V2d outRight = centerPos + currNorm * (double)tw / 2.0 - currVec * 16.0;

				//va[extra + i*3 + 0].position = Vector2f( centerPos.x, centerPos.y );
				va[extra + i*4 + 0].position = Vector2f( outLeft.x, outLeft.y );
				va[extra + i*4 + 1].position = Vector2f( outRight.x, outRight.y );
				va[extra + i*4 + 2].position = Vector2f( insideNext.x, insideNext.y );
				va[extra + i*4 + 3].position = Vector2f( inside.x, inside.y );

				//int blah = 20;
				//va[extra + i*3 + 0].color = Color( i * 20, i * 20, i * 20 );//Color::Red;
				//va[extra + i*3 + 1].color = Color( i * 20, i * 20, i * 20 );
				//va[extra + i*3 + 2].color = Color( i * 20, i * 20, i * 20 );

				va[extra + i*4 + 0].texCoords = Vector2f( sub.left, sub.top );
				va[extra + i*4 + 1].texCoords = Vector2f( sub.left + tw, sub.top );
				va[extra + i*4 + 2].texCoords = Vector2f( sub.left + tw, sub.top + sub.height );
				va[extra + i*4 + 3].texCoords = Vector2f( sub.left, sub.top + sub.height );
			}

			extra += numTris * 4;
		}
	}
	while( te != startEdge );

	return currVA;
}

sf::VertexArray *GameSession::SetupTransitionQuads(
	int bgLayer,
	Edge *startEdge,
	Tileset *ts)
{
	QuadTree *qt = NULL;
	if (bgLayer == 0)
	{
		qt = terrainTree;
	}
	else if (bgLayer == 1)
	{
		qt = terrainBGTree;
	}

	double tw = 128;//256;//64;//8;
	double th = 64;//256;

	int out = 16;//40;
	int in = th - out;
	assert(qt != NULL);


	int numtotalQuads = 0;
	Edge *te = startEdge;
	do
	{
		Edge *e1 = te->edge1;
		V2d eNorm = te->Normal();
		V2d along = normalize(te->v1 - te->v0);
		V2d nextAlong = normalize(e1->v1 - e1->v0);
		V2d nextNorm = e1->Normal();
		double c = cross(nextAlong, along);
		bool viable = true;

		V2d jutDir = normalize(normalize(te->v1 - te->v0) + normalize(te->v1 - e1->v1)) / 2.0;
		V2d jutPoint = te->v1 + jutDir * (th - out);

		//double rayTest = (th - out);

		rcEdge = NULL;
		rayIgnoreEdge = NULL;
		rayStart = te->v1 + jutDir * 1.0;
		rayEnd = jutPoint;//te->v0 + (double)i * quadWidth * along - other * in;
		RayCast(this, qt->startNode, rayStart, rayEnd);

		//start ray
		if (rcEdge != NULL)
		{
			//cout << "viable is now false" << endl;
			viable = false;
		}
		else
		{
			//rcEdge = NULL;
			//rayIgnoreEdge = te;
			//rayIgnoreEdge = NULL;
			rayStart = te->v1 - eNorm * .01;
			rayEnd = te->v1 - eNorm * (th - out);//te->v0 + (double)i * quadWidth * along - other * in;
			RayCast(this, qt->startNode, rayStart, rayEnd);


			//start ray
			if (rcEdge != NULL)
			{
				viable = false;
				//currStartInner = rcEdge->GetPoint( rcQuantity );
				//realHeight0 = length( currStartInner - currStartOuter );
			}
			else
			{
				//rayIgnoreEdge = te;
				//rayIgnoreEdge = NULL;
				rayStart = te->v1 - nextNorm * .01;
				rayEnd = te->v1 - nextNorm * (th - out);
				RayCast(this, qt->startNode, rayStart, rayEnd);

				//end ray
				if (rcEdge != NULL)
				{
					viable = false;
					//currEndInner =  rcEdge->GetPoint( rcQuantity );//te->v0 + endAlong * along - rcQuantity * other;
					//realHeight1 = length( currEndInner - currStartOuter );
				}
			}
		}


		if (viable)
		{
			numtotalQuads++;
		}
		te = te->edge1;
	} while (te != startEdge);

	if (numtotalQuads == 0)
	{
		return NULL;
	}

	VertexArray *currVA = new VertexArray(sf::Quads, numtotalQuads * 4);
	VertexArray &va = *currVA;

	//..IntRect sub = ts->GetSubRect( 0 );

	te = startEdge;
	int extra = 0;
	do
	{
		Edge *e1 = te->edge1;
		V2d eNorm = te->Normal();
		V2d along = normalize(te->v1 - te->v0);
		V2d nextAlong = normalize(e1->v1 - e1->v0);
		double c = cross(nextAlong, along);
		V2d nextNorm = e1->Normal();
		V2d point = e1->v0;
		int i = 0;
		V2d jutDir = normalize((normalize(te->v1 - te->v0) + normalize(te->v1 - e1->v1)) / 2.0);
		//cout << "length jut dir: " << length( jutDir ) << endl;
		V2d jutPoint = te->v1 + jutDir * (th - out);
		bool viable = true;

		rcEdge = NULL;
		rayIgnoreEdge = NULL;
		rayStart = te->v1 + jutDir * 1.0;
		rayEnd = jutPoint;//te->v0 + (double)i * quadWidth * along - other * in;
		RayCast(this, qt->startNode, rayStart, rayEnd);

		//start ray
		if (rcEdge != NULL)
		{
			//cout << "viable is now false" << endl;
			viable = false;
		}
		else
		{
			rcEdge = NULL;
			rayIgnoreEdge = te;
			rayStart = te->v1 - eNorm * .01;
			rayEnd = te->v1 - eNorm * (th - out);//te->v0 + (double)i * quadWidth * along - other * in;
			RayCast(this, qt->startNode, rayStart, rayEnd);


			//start ray
			//if (rcEdge != NULL)
			//{
			//	viable = false;
			//}
			//else
			//{
			//	rayIgnoreEdge = te;
			//	rayStart = te->v1 - nextNorm * .01;
			//	rayEnd = te->v1 - nextNorm * (th - out);
			//	RayCast(this, qt->startNode, rayStart, rayEnd);

			//	//end ray
			//	if (rcEdge != NULL)
			//	{
			//		viable = false;
			//	}
			//}
		}

		viable = true;
		if (c > 0 && viable)
		{
			int valid = -1;
			valid = IsFlatGround(eNorm);
			if (valid == -1)
			{
				valid = IsSlopedGround(eNorm);
				if (valid == -1)
				{
					valid = IsWall(eNorm);

					if (valid == -1)
					{
						valid = IsSteepGround(eNorm);
					}
				}

			}

			if (valid == -1)
			{
				cout << "wat: " << eNorm.x << ", " << eNorm.y << endl;
			}
			assert(valid != -1);

			int otherValid = -1;
			otherValid = IsFlatGround(nextNorm);
			if (otherValid == -1)
			{
				otherValid = IsSlopedGround(nextNorm);
				if (otherValid == -1)
				{
					otherValid = IsWall(nextNorm);

					if (otherValid == -1)
					{
						otherValid = IsSteepGround(nextNorm);
					}
				}
			}

			if (otherValid == -1)
			{
				cout << "watother: " << eNorm.x << ", " << eNorm.y << endl;
			}
			assert(otherValid != -1);

			int realIndex = valid * 4;//32;
			int realOther = otherValid * 4;// * 32;

			IntRect sub = ts->GetSubRect(realIndex);
			IntRect subOther = ts->GetSubRect(realOther);

			V2d currNormOpp = -te->Normal();
			V2d nextNormOpp = -e1->Normal();

			V2d currInPoint = point + currNormOpp * (th - out);
			V2d nextInPoint = point + nextNormOpp * (th - out);
			//cout << "mid: " << mid << endl;

			Vector2f pa = Vector2f(point.x, point.y);
			Vector2f pb = Vector2f(currInPoint.x, currInPoint.y);
			Vector2f pc = Vector2f(jutPoint.x, jutPoint.y);

			va[extra + 0].position = Vector2f(currInPoint.x, currInPoint.y );
			va[extra + 0].position = Vector2f(currInPoint.x, currInPoint.y);
			va[extra + 1].position = pb;
			va[extra + 2].position = pc;

			va[extra + 0].texCoords = Vector2f(sub.left, sub.top + out);
			va[extra + 1].texCoords = Vector2f(sub.left, sub.top + out);
			va[extra + 2].texCoords = Vector2f(sub.left, th + sub.top);
			va[extra + 3].texCoords = Vector2f(sub.left, sub.top + th);

			extra += 3;

			va[extra + 0].position = Vector2f(point.x, point.y);
			va[extra + 1].position = Vector2f(jutPoint.x, jutPoint.y);
			va[extra + 2].position = Vector2f(nextInPoint.x, nextInPoint.y);

			va[extra + 0].texCoords = Vector2f(subOther.left, subOther.top + out);
			va[extra + 1].texCoords = Vector2f(subOther.left, th + subOther.top);
			va[extra + 2].texCoords = Vector2f(subOther.left, th + subOther.top);

			extra += 3;
		}
		te = te->edge1;
	} while (te != startEdge);

	return currVA;
}

sf::VertexArray * GameSession::SetupTransitions( int bgLayer, Edge *startEdge, Tileset *ts )
{
	QuadTree *qt = NULL;
	if( bgLayer == 0 )
	{
		qt = terrainTree;
	}
	else if( bgLayer == 1 )
	{
		qt = terrainBGTree;
	}

	double tw = 128;//256;//64;//8;
	double th = 64;//256;

	int out = 16;//40;
	int in = th - out;
	assert( qt != NULL );


	int numtotalTris = 0;
	Edge *te = startEdge;
	do
	{
		Edge *e1 = te->edge1;
		V2d eNorm = te->Normal();
		V2d along = normalize( te->v1 - te->v0 );
		V2d nextAlong = normalize( e1->v1 - e1->v0 );
		V2d nextNorm = e1->Normal();
		double c = cross( nextAlong, along );
		bool viable = true;

		V2d jutDir = normalize( normalize( te->v1 - te->v0 ) + normalize( te->v1 - e1->v1 ) ) / 2.0;
		V2d jutPoint = te->v1 + jutDir * (th - out );

		//double rayTest = (th - out);

		rcEdge = NULL;
		rayIgnoreEdge = NULL;
		rayStart = te->v1 + jutDir * 1.0;
		rayEnd = jutPoint;//te->v0 + (double)i * quadWidth * along - other * in;
		RayCast( this, qt->startNode, rayStart, rayEnd );

		//start ray
		if( rcEdge != NULL )
		{
			//cout << "viable is now false" << endl;
			viable = false;
		}
		else
		{
			//rcEdge = NULL;
			//rayIgnoreEdge = te;
			//rayIgnoreEdge = NULL;
			rayStart = te->v1 - eNorm * .01;
			rayEnd = te->v1 - eNorm * (th-out);//te->v0 + (double)i * quadWidth * along - other * in;
			RayCast( this, qt->startNode, rayStart, rayEnd );


			//start ray
			if( rcEdge != NULL )
			{
				viable = false;
				//currStartInner = rcEdge->GetPoint( rcQuantity );
				//realHeight0 = length( currStartInner - currStartOuter );
			}
			else
			{
				//rayIgnoreEdge = te;
				//rayIgnoreEdge = NULL;
				rayStart = te->v1 - nextNorm * .01;
				rayEnd = te->v1 - nextNorm * (th-out);
				RayCast( this, qt->startNode, rayStart, rayEnd );

				//end ray
				if( rcEdge != NULL )
				{
					viable = false;
					//currEndInner =  rcEdge->GetPoint( rcQuantity );//te->v0 + endAlong * along - rcQuantity * other;
					//realHeight1 = length( currEndInner - currStartOuter );
				}
			}
		}


		if( c > 0 && viable )
		{
			numtotalTris+=2;
			//V2d endVec = normalize( te->v0 - te->v1 );
			//V2d startVec = normalize( e1->v1 - te->v1 );

			//double startAngle = atan2( -startVec.y, startVec.x );
			//if( startAngle < 0 )
			//{
			//	startAngle += 2 * PI;
			//}
			//double endAngle = atan2( -endVec.y, endVec.x );
			//if( endAngle < 0 )
			//{
			//	endAngle += 2 * PI;
			//}

			///*double temp = startAngle;
			//startAngle = endAngle;
			//endAngle = temp;*/

			//if( endAngle > startAngle )
			//{
			//	startAngle += 2 * PI;
			//}

			//

			//double angleDiff = (startAngle - endAngle);

			//int numTris = angleDiff / an;
			////go counter clockwise


			//
			//
			//	
			///*if( numTris == 0 )
			//{
			//	numTris = 1;
			//}*/

			//numTotalTris += numTris;
		}
		te = te->edge1;
	}
	while( te != startEdge );

	if( numtotalTris == 0 )
	{
		return NULL;
	}

	VertexArray *currVA = new VertexArray( sf::Triangles, numtotalTris * 3 );
	VertexArray &va = *currVA;

	//..IntRect sub = ts->GetSubRect( 0 );

	te = startEdge;
	int extra = 0;
	do
	{
		Edge *e1 = te->edge1;
		V2d eNorm = te->Normal();
		V2d along = normalize( te->v1 - te->v0 );
		V2d nextAlong = normalize( e1->v1 - e1->v0 );
		double c = cross( nextAlong, along );
		V2d nextNorm = e1->Normal();
		V2d point = e1->v0;
		int i = 0;
		V2d jutDir = normalize( ( normalize( te->v1 - te->v0 ) + normalize( te->v1 - e1->v1 ) ) / 2.0 );
		//cout << "length jut dir: " << length( jutDir ) << endl;
		V2d jutPoint = te->v1 + jutDir * (th - out );
		bool viable = true;

		rcEdge = NULL;
		rayIgnoreEdge = NULL;
		rayStart = te->v1 + jutDir * 1.0;
		rayEnd = jutPoint;//te->v0 + (double)i * quadWidth * along - other * in;
		RayCast( this, qt->startNode, rayStart, rayEnd );

		//start ray
		if( rcEdge != NULL )
		{
			//cout << "viable is now false" << endl;
			viable = false;
		}
		else
		{
			rcEdge = NULL;
			rayIgnoreEdge = te;
			rayStart = te->v1 - eNorm * .01;
			rayEnd = te->v1 - eNorm * (th-out);//te->v0 + (double)i * quadWidth * along - other * in;
			RayCast( this, qt->startNode, rayStart, rayEnd );


			//start ray
			if( rcEdge != NULL )
			{
				viable = false;
				//currStartInner = rcEdge->GetPoint( rcQuantity );
				//realHeight0 = length( currStartInner - currStartOuter );
			}
			else
			{
				rayIgnoreEdge = te;
				rayStart = te->v1 - nextNorm * .01;
				rayEnd = te->v1 - nextNorm * (th-out);
				RayCast( this, qt->startNode, rayStart, rayEnd );

				//end ray
				if( rcEdge != NULL )
				{
					viable = false;
					//currEndInner =  rcEdge->GetPoint( rcQuantity );//te->v0 + endAlong * along - rcQuantity * other;
					//realHeight1 = length( currEndInner - currStartOuter );
				}
			}
		}


		if( c > 0 && viable )
		{
			/*GameSession::IsFlatGround( sf::Vector2<double> &normal )
			GameSession::IsSlopedGround( sf::Vector2<double> &normal )
			GameSession::IsWall( sf::Vector2<double> &normal )*/

			
			int valid = -1;
			valid = IsFlatGround( eNorm );
			if( valid == -1 )
			{
				valid = IsSlopedGround( eNorm );
				if( valid == -1 )
				{
					valid = IsWall( eNorm );

					if( valid == -1 )
					{
						valid = IsSteepGround( eNorm );
					}
				}

			}

			if( valid == -1 )
			{
				cout << "wat: " << eNorm.x << ", " << eNorm.y << endl;
			}
			assert( valid != -1 );

			int otherValid = -1;
			otherValid = IsFlatGround( nextNorm );
			if( otherValid == -1 )
			{
				otherValid = IsSlopedGround( nextNorm );
				if( otherValid == -1 )
				{
					otherValid = IsWall( nextNorm );

					if( otherValid == -1 )
					{
						otherValid = IsSteepGround( nextNorm );
					}
				}
			}

			if( otherValid == -1 )
			{
				cout << "watother: " << eNorm.x << ", " << eNorm.y << endl;
			}
			assert( otherValid != -1 );
			
			
			//int valid = ValidEdge( eNorm );
				//add (worldNum * 5) to realIndex to get the correct borders
			

			int realIndex = valid * 4;// *8;//32;
			int realOther = otherValid * 4;// *8;// * 32;

			//cout << "valid: " << valid << ", otherValid: " << otherValid << endl;
			//cout << "norm: " << eNorm.x << ", " << eNorm.y << endl;
			//cout << "nextNorm: " << nextNorm.x << ", " << nextNorm.y << endl;
			IntRect sub = ts->GetSubRect( realIndex );
			IntRect subOther = ts->GetSubRect( realOther );

			

			V2d currNormOpp = -te->Normal();
			V2d nextNormOpp = -e1->Normal();

			V2d currInPoint = point + currNormOpp * (th - out);
			V2d nextInPoint = point + nextNormOpp * (th - out);

			double baseLength = length( jutPoint - currInPoint );
			int mid = floor( baseLength + .5 );
			//cout << "mid: " << mid << endl;
			mid = min( mid, (int)tw );
			//assert( mid <= 128 );
			//cout << "mid: " << mid << endl;
			//cout << "jut length: " << length( jutPoint - point ) << endl;

			Vector2f pa = Vector2f( point.x, point.y );
			Vector2f pb = Vector2f( currInPoint.x, currInPoint.y );
			Vector2f pc = Vector2f( jutPoint.x, jutPoint.y );

			/*pa += Vector2f( .5, .5 );
			pb += Vector2f( .5, .5 );
			pc += Vector2f( .5, .5 );
			pa.x = floor( pa.x );
			pa.y = floor( pa.y );
			pb.x = floor( pb.x );
			pb.y = floor( pb.y );
			pc.x = floor( pc.x );
			pc.y = floor( pc.y );*/

			va[extra + 0].position = pa;
			va[extra + 1].position = pb;
			va[extra + 2].position = pc;

			/*va[extra + i*3 + 0].color = Color::Red;
			va[extra + i*3 + 1].color = Color::Green;
			va[extra + i*3 + 2].color = Color::Blue;*/

			va[extra + 0].texCoords = Vector2f( sub.left + mid / 2, sub.top + out);
			va[extra + 1].texCoords = Vector2f( sub.left, th + sub.top );
			va[extra + 2].texCoords = Vector2f( sub.left + mid, sub.top + th );

			extra += 3;

			va[extra + 0].position = Vector2f( point.x, point.y );
			va[extra + 1].position = Vector2f( jutPoint.x, jutPoint.y );
			va[extra + 2].position = Vector2f( nextInPoint.x, nextInPoint.y );

			/*va[extra + i*3 + 0].color = Color::Red;
			va[extra + i*3 + 1].color = Color::Blue;
			va[extra + i*3 + 2].color = Color::Green;*/


			/*va[extra + i*3 + 0].color = Color::Red;
			va[extra + i*3 + 1].color = Color::Red;
			va[extra + i*3 + 2].color = Color::Red;*/

			va[extra + 0].texCoords = Vector2f( subOther.left + mid / 2, subOther.top + out );
			va[extra + 1].texCoords = Vector2f( subOther.left, th + subOther.top );
			va[extra + 2].texCoords = Vector2f( subOther.left + mid, th + subOther.top );

			//va[extra + i*4 + 0].position = Vector2f( outLeft.x, outLeft.y );

			//V2d half = normalize( ( currNorm + nextNorm ) / 2.0 );
			//V2d oppHalf = -half;

			//double outDist = 16.0;
			//double inDist = 128.0 - outDist;//48.0;

			//V2d out = te->v1 + half * outDist;
			//V2d in = te->v1 + oppHalf * inDist;

			//V2d otherHalf( half.y, -half.x );

			//double hw = tw / 2.0;
			//V2d outLeft = out - otherHalf * hw;
			//V2d outRight = out + otherHalf * hw;
			//V2d inLeft = in - otherHalf * hw;
			//V2d inRight = in + otherHalf * hw;

			//va[extra + i*4 + 0].position = Vector2f( outLeft.x, outLeft.y );
			//va[extra + i*4 + 1].position = Vector2f( outRight.x, outRight.y );
			//va[extra + i*4 + 2].position = Vector2f( inRight.x, inRight.y );
			//va[extra + i*4 + 3].position = Vector2f( inLeft.x, inLeft.y );

			////int blah = 20;
			////va[extra + i*3 + 0].color = Color( i * 20, i * 20, i * 20 );//Color::Red;
			////va[extra + i*3 + 1].color = Color( i * 20, i * 20, i * 20 );
			////va[extra + i*3 + 2].color = Color( i * 20, i * 20, i * 20 );

			//va[extra + i*4 + 0].texCoords = Vector2f( sub.left, sub.top );
			//va[extra + i*4 + 1].texCoords = Vector2f( sub.left + tw, sub.top );
			//va[extra + i*4 + 2].texCoords = Vector2f( sub.left + tw, sub.top + sub.height );
			//va[extra + i*4 + 3].texCoords = Vector2f( sub.left, sub.top + sub.height );

			extra += 3;
		}
		te = te->edge1;
	}
	while( te != startEdge );

	return currVA;
}

sf::VertexArray * GameSession::SetupDecor0( std::vector<p2t::Triangle*> &tris, Tileset *ts )
{
	for( vector<p2t::Triangle*>::iterator it = tris.begin(); it != tris.end(); ++it )
	{
		//random point stuff. do this after you get the enemies working
	}

	return NULL;
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
		rayStart = curr->GetPoint( quant );
		rayEnd = rayStart - cn * (double)maxPen;
		rayIgnoreEdge = curr;

		RayCast( this, qt->startNode, rayStart, rayEnd );

		if( rcEdge != NULL )
		{
			penLimit = length( rcEdge->GetPoint( rcQuantity ) - rayStart );
			diffPenMax = (int)penLimit - minDistanceApart;
		}
		if (diffPenMax == 0)
			rPen = 0;
		else
		{
			rPen = rand() % diffPenMax;
		}
		
		penDistance = minPen + rPen;
		
		pos = curr->GetPoint( quant ) - curr->Normal() * penDistance;

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

//save state to enter clone world
void GameSession::SaveState()
{
	/*stored.activeEnemyList = activeEnemyList;
	cloneInactiveEnemyList = NULL;

	Enemy *currEnemy = activeEnemyList;
	while( currEnemy != NULL )
	{
		currEnemy->SaveState();
		currEnemy = currEnemy->next;
	}*/
}

//reset from clone world
void GameSession::LoadState()
{
	//Enemy *test = cloneInactiveEnemyList;
	//int listSize = 0;
	//while( test != NULL )
	//{
	//	listSize++;
	//	test = test->next;
	//}

	//cout << "there are " << listSize << " enemies killed during the last clone process" << endl;


	////enemies killed while in the clone world
	//Enemy *deadEnemy = cloneInactiveEnemyList;
	//while( deadEnemy != NULL )
	//{
	//	
	//	Enemy *next = deadEnemy->next;
	//	if( deadEnemy->spawnedByClone )
	//	{
	//		deadEnemy->Reset();
	//		//cout << "resetting dead enemy: " << deadEnemy << endl;
	//	}
	//	else
	//	{
	//		deadEnemy->LoadState();
	//		//cout << "loading dead enemy: " << deadEnemy << endl;
	//	}
	//	deadEnemy = next;
	//}

	////enemies that are still alive
	//Enemy *currEnemy = activeEnemyList;
	//while( currEnemy != NULL )
	//{		
	//	Enemy *next = currEnemy->next;
	//	if( currEnemy->spawnedByClone )
	//	{
	//		//cout << "resetting enemy: " << currEnemy << endl;
	//		currEnemy->Reset();
	//	}
	//	else
	//	{
	//		currEnemy->LoadState();
	//		//cout << "loading enemy: " << currEnemy << endl;
	//	}

	//	currEnemy = next;
	//}

	////restore them all to their original state and then reset the list pointer

	////cloneInactiveEnemyList = NULL;
	//activeEnemyList = stored.activeEnemyList;
}

void GameSession::Pause( int frames )
{
	pauseFrames = frames;
	for (int i = 0; i < 4; ++i)
	{
		Actor *p = players[i];
		if (p != NULL)
		{
			p->ClearPauseBufferedActions();
		}
	}
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

		if( edge != rayIgnoreEdge && ( rcEdge == NULL || length( edge->GetPoint( edgeQuantity ) - rayStart ) < 
			length( rcEdge->GetPoint( rcQuantity ) - rayStart ) ) )
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
	else if( rayMode == "energy_flow" )
	{
		if( edge->edgeType == Edge::CLOSED_GATE )
		{
			return;
		}

		if( edge != rayIgnoreEdge && edge != rayIgnoreEdge1 && ( rcEdge == NULL || length( edge->GetPoint( edgeQuantity ) - rayStart ) < 
			length( rcEdge->GetPoint( rcQuantity ) - rayStart ) ) )
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
			V2d rc = rcEdge->GetPoint( rcQuantity );
			if( length( rayStart - edge->GetPoint( edgeQuantity ) ) < length( rayStart - rc ) )
			{
				rcEdge = edge;
				rcQuantity = edgeQuantity;
			}
		}
	}
	//if( rayPortion > 1 && ( rcEdge == NULL || length( edge->GetPoint( edgeQuantity ) - position ) < length( rcEdge->GetPoint( rcQuantity ) - position ) ) )
	
//	{
//		rcEdge = edge;
//		rcQuantity = edgeQuantity;
//	}
}

void GameSession::AllocateEffect()
{
	if( inactiveEffects == NULL )
	{
		inactiveEffects = new BasicEffect( this );
		allEffectList.push_back(inactiveEffects);
		inactiveEffects->prev = NULL;
		inactiveEffects->next = NULL;
	}
	else
	{
		BasicEffect *b = new BasicEffect( this ) ;
		allEffectList.push_back(b);
		b->next = inactiveEffects;
		inactiveEffects->prev = b;
		inactiveEffects = b;
	}
}

BasicEffect * GameSession::ActivateEffect( EffectLayer layer, Tileset *ts, V2d pos, bool pauseImmune, double angle, int frameCount,
	int animationFactor, bool right, int startFrame, float depth )
{
	if( inactiveEffects == NULL )
	{
		return NULL;
	}
	else
	{
		//return NULL;

		BasicEffect *b = inactiveEffects;

		if( inactiveEffects->next == NULL )
		{
			inactiveEffects = NULL;
		}
		else
		{
			inactiveEffects = (BasicEffect*)(inactiveEffects->next);
			inactiveEffects->prev = NULL;
		}

		//assert( ts != NULL );
		b->startFrame = startFrame;
		b->Init( ts, pos, angle, frameCount, animationFactor, right, depth );
		b->prev = NULL;
		b->next = NULL;
		b->pauseImmune = pauseImmune;
		b->layer = layer;
		

		AddEffect( layer, b );
		//AddEnemy( b );
		
		//cout << "activating: " << b << " blah: " << b->prev << endl;
		return b;
	}
}

void GameSession::RemoveEffect( EffectLayer layer, Enemy *e )
{
	Enemy *& fxList = effectLists[layer];
	assert( fxList != NULL );
	Enemy *prev = e->prev;
	Enemy *next = e->next;

	if( prev == NULL && next == NULL )
	{
		fxList = NULL;
	}
	else
	{
		if( e == fxList )
		{
			assert( next != NULL );
			
			next->prev = NULL;
			
			fxList = next;
		}
		else
		{
			if( prev != NULL )
			{
				prev->next = next;
			}

			if( next != NULL )
			{
				next->prev = prev;
			}
		}
		
	}
}

void GameSession::DeactivateEffect( BasicEffect *b )
{
	//cout << "deactivate " << b << endl;
	RemoveEffect( b->layer, b );
	//RemoveEnemy( b );

	if( inactiveEffects == NULL )
	{
		inactiveEffects = b;
		b->next = NULL;
		b->prev = NULL;
	}
	else
	{
		b->next = inactiveEffects;
		inactiveEffects->prev = b;
		inactiveEffects = b;
	}
}

void GameSession::ResetEnemies()
{
	rResetEnemies( enemyTree->startNode );

	/*Enemy *curr = activeEnemyList;
	while( curr != NULL )
	{
		Enemy *next = curr->next;
		if( curr->type == Enemy::BASICEFFECT )
		{
			DeactivateEffect( (BasicEffect*)curr );
		}
		curr = next;
	}
	activeEnemyList = NULL;*/
	activeEnemyList = NULL;
	activeEnemyListTail = NULL;
	for( int i = 0; i < EffectLayer::Count; ++i )
	{
		Enemy *curr = effectLists[i];
		while( curr != NULL )
		{
			Enemy *next = curr->next;
			assert( curr->type == EnemyType::EN_BASICEFFECT );
			DeactivateEffect( (BasicEffect*)curr );

			curr = next;
		}
		effectLists[i] = NULL;
		
	}


	if( b_bird != NULL ) b_bird->Reset();

	//if( b_crawler != NULL ) b_crawler->Reset();

	if( b_coyote != NULL ) b_coyote->Reset();

	if( b_skeleton != NULL ) b_skeleton->Reset();
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

void GameSession::rResetEnemies( QNode *node )
{
	if( node->leaf )
	{
		LeafNode *n = (LeafNode*)node;

		//cout << "\t\tstarting leaf reset: " << endl;
		for( int i = 0; i < n->objCount; ++i )
		{			
			//cout << "\t\t\tresetting enemy " << i << endl;
			Enemy * e = (Enemy*)(n->entrants[i]);
			e->Reset();
			//cout << e->type << endl;
			
			//((Enemy*)node)->Reset();		
		}
	}
	else
	{
		//shouldn't this check for box touching box right here??
		ParentNode *n = (ParentNode*)node;
		//cout << "start parent reset: " << endl;
		for( int i = 0; i < 4; ++i )
		{
		//	cout << "\tresetting child: " << i << endl;
			rResetEnemies( n->children[i] );
		}

		for( list<QuadTreeEntrant*>::iterator it = n->extraChildren.begin(); it != n->extraChildren.end(); ++it )
		{
			Enemy * e = (Enemy*)(*it);
			e->Reset();
		}
		
	}
}

void GameSession::LevelSpecifics()
{
	if( fileName == "test3" )
	{
		//startSeq = new GameStartSeq( this );
		//activeSequence = startSeq;
		//GameStartMovie();
		//cout << "doing stuff here" << endl;
	}
	else
	{
	//	player->velocity.x = 60;
	}
}

PowerBar::PowerBar()
{
	pointsPerDot = 2;
	dotsPerLine = 6;
	dotWidth = 9;
	dotHeight = 9;
	linesPerBar = 60;

	pointsPerLayer = 2 * 6 * 60;//3 * 6 * 60//240 * 10;
	maxLayer = 1;//6;
	points = pointsPerLayer;//pointsPerLayer * ( maxLayer + 1 );
	layer = maxLayer;//maxLayer;
	
	minUse = 1;
	
	//panelTex.loadFromFile( "lifebar.png" );
	panelTex.loadFromFile( "Resources/powerbarmockup.png" );
	panelSprite.setTexture( panelTex );
	//panelSprite.setScale( 10, 10 );
	panelSprite.setPosition( 0, 280 );
	panelSprite.setColor( Color( 255, 255, 255, 150 ) );

	//powerRect.setPosition( 42, 108 );
	//powerRect.setSize( sf::Vector2f( 4 * 4, 59 * 4 ) );
	//powerRect.setFillColor( Color::Green );
	//powerRect.

	maxRecover = 75;
	maxRecoverLayer = 0;
}

void PowerBar::Reset()
{
	points = pointsPerLayer;
	layer = maxLayer;
}

void PowerBar::Draw( sf::RenderTarget *target )
{
	int fullLines = points / pointsPerDot / dotsPerLine;
	int partial = points % ( dotsPerLine * pointsPerDot );//pointsPerLayer - fullLines * pointsPerDot * dotsPerLine;
	//cout << "fullLines: " << fullLines << endl;
	//cout << "partial: " << partial << endl;
	//Color c;
	/*switch( layer )
	{
	case 0:
		c = Color( 0, 0xee, 0xff );
		//c = Color( 0x00eeff );
		break;
	case 1:
		//c = Color( 0x0066cc );
		c = Color( 0, 0x66, 0xcc );
		break;
	case 2:
		c = Color( 0, 0xcc, 0x44 );
		break;
	case 3:
		c = Color( 0xff, 0xf0, 0 );
		break;
	case 4:
		c = Color( 0xff, 0xbb, 0 );
		break;
	case 5:
		c = Color( 0xff, 0x22, 0 );
		break;
	case 6:
		c = Color( 0xff, 0, 0xff );
		break;
	case 7:
		c = Color( 0xff, 0xff, 0xff );
		break;
	}*/
	//c = Color( 0, 0xee, 0xff );


	sf::RectangleShape rs;


	//primary portion
	rs.setPosition( 4, 180 + 600 );
	rs.setFillColor( COLOR_TEAL );
	rs.setSize( Vector2f( 60, -fullLines * dotHeight ) );
	target->draw( rs );

	//secondary portion
	rs.setPosition( rs.getGlobalBounds().left, rs.getGlobalBounds().top - dotHeight );
	rs.setSize( Vector2f( (partial / pointsPerDot) * dotWidth, dotHeight ) );
	target->draw( rs );

	//tertiary portion
	int singleDot = partial % pointsPerDot;
	if( singleDot == 1 )
	{
		rs.setPosition( rs.getGlobalBounds().left + rs.getLocalBounds().width,
			rs.getPosition().y );
		rs.setSize( Vector2f( dotWidth, dotHeight ) );
		rs.setFillColor( COLOR_BLUE );
		target->draw( rs );
	}

	//draw full tanks
	int tankWidth = 32;
	int tankHeight = 32;
	int tankSpacing = 20;
	rs.setFillColor( COLOR_TEAL );
	rs.setSize( Vector2f( tankWidth, tankHeight ) );
	rs.setPosition( 0, 180 + 600 - tankHeight );
	for( int i = 0; i < layer; ++i )
	{
		//target->draw( rs );
		//rs.setPosition( rs.getPosition().x, rs.getPosition().y - tankHeight - tankSpacing );
	}

	//only need this until I get a background

	//rs.setFillColor( Color( 100, 100, 100 ) );
	//target->draw( rs );

	/*rs.setFillColor( COLOR_TEAL );
	//in progress tank
	int off = ceil( (points / (double)pointsPerLayer) * 32.0);
	//cout << "off: " << off << endl;
	rs.setPosition( rs.getPosition().x, rs.getPosition().y + tankHeight );
	rs.setSize( Vector2f( tankWidth, -off ) );
	target->draw( rs );*/
	

	//0x99a9b9
	
	//cout << "points: " << points << endl;
	double diffz = (double)points / (double)pointsPerLayer;
	assert( diffz <= 1 );
	diffz = 1 - diffz;
	diffz *= 60 * 4;

	
	//rs.setPosition( 42, 108 + diffz );
	//rs.setPosition( 0, 108 + diffz );
	//rs.setSize( sf::Vector2f( 4 * 4, 60 * 4 - diffz ) );
	//rs.setPosition( 0, 200 );
	//rs.setSize( Vector2f( 100, 500 ) );
	

	

	//target->draw( panelSprite );
	//target->draw( rs );
}

bool PowerBar::Damage( int power )
{
	//cout << "points: " << points << ", power: " << power << endl;
	
	//cout << "newpoints: " << points << endl;
	while( power > 0 )
	{
		points -= power;
		if( points <= 0 )
		{
			power = -points;
			if( layer > 0 )
			{
				layer--;
				points = pointsPerLayer;
				//cout << "layer switch: " << points << endl;
			}
			else
			{
				points = 0;
				//cout << "DONE DEAD: " << points << endl;
				return false;
			}
		}
		else
		{
			break;
		}
	}

	return true;
}

bool PowerBar::Use( int power )
{
	if( layer == 0 )
	{
		if( points - power <= 0 )
		{
			return false;
		}
		else
		{
			points -= power;
		}
	}
	else
	{
		points -= power;
		if( points <= 0 )
		{
			points = pointsPerLayer + points;
			layer--;
		}
	}
	return true;
	/*if( layer == 0 )
	{
		if( points - power < minUse )
		{
			return false;
		}
		else
		{
			points -= power;
		}
	}
	else
	{
		points -= power;
		if( points <= 0 )
		{
			points = pointsPerLayer + points;
			layer--;
		}
	}
	return true;*/
}

void PowerBar::Recover( int power )
{
	if( layer == maxRecoverLayer )
	{
		if( points + power > maxRecover )
		{
			points = maxRecover;
		}
		else
		{
			points += power;
		}
	}
	else
	{
		if( points + power > pointsPerLayer )
		{
			layer++;
			points = points + power - pointsPerLayer;
		}
		else
		{
			points += power;
		}
	}
}

void PowerBar::Charge( int power )
{
	if( layer == maxLayer )
	{
		if( points + power > pointsPerLayer )
		{
			points = pointsPerLayer;
		}
		else
		{
			points += power;
		}
	}
	else
	{
		if( points + power > pointsPerLayer )
		{
			layer++;
			points = points + power - pointsPerLayer;
		}
		else
		{
			points += power;
		}
	}
}


 //groundLeft,airLeft,airRight,groundRight
EnvPlant::EnvPlant(sf::Vector2<double>&a, V2d &b, V2d &c, V2d &d, int vi, VertexArray *v, Tileset *t )
	:A(a),B(b),C(c),D(d), vaIndex( vi ), va( v ), frame( 0 ), activated( false ), next( NULL ), ts( t ),
	idleLength( 4 ), idleFactor( 3 )
{
	particle = new AirParticleEffect( ( a + d ) / 2.0 );
	disperseLength = particle->maxDurationToLive + particle->emitDuration;
	disperseFactor = 1;
	SetupQuad();
}

EnvPlant::~EnvPlant()
{
	delete particle;
}

void EnvPlant::SetupQuad()
{
	VertexArray &eva = *va;
	eva[vaIndex+0].position = Vector2f( A.x, A.y );
	eva[vaIndex+1].position = Vector2f( B.x, B.y );
	eva[vaIndex+2].position = Vector2f( C.x, C.y );
	eva[vaIndex+3].position = Vector2f( D.x, D.y );
}

void EnvPlant::HandleQuery( QuadTreeCollider *qtc )
{
	qtc->HandleEntrant( this );
}

bool EnvPlant::IsTouchingBox( const Rect<double> &r )
{
	return isQuadTouchingQuad( V2d( r.left, r.top ), V2d( r.left + r.width, r.top ), 
		V2d( r.left + r.width, r.top + r.height ), V2d( r.left, r.top + r.height ),
		A, B, C, D );
}

void EnvPlant::Reset()
{
	
	//cout << "resetting plant!" << endl;
	next = NULL;
	activated = false;
	frame = 0;

	SetupQuad();

	particle->Reset();
}



//only activates zones if they're inactive. 
void GameSession::ActivateZone( Zone *z, bool instant )
{
	if( z == NULL )
		return;
	//assert( z != NULL );
	//cout << "ACTIVATE ZONE!!!" << endl;
	if( !z->active )
	{
		if (instant)
		{
			z->action = Zone::OPEN;
		}
		else
		{
			z->action = Zone::OPENING;
			z->frame = 0;
		}

		for( list<Enemy*>::iterator it = z->spawnEnemies.begin(); it != z->spawnEnemies.end(); ++it )
		{
			assert( (*it)->spawned == false );

			(*it)->Init();
			(*it)->spawned = true;
			AddEnemy( (*it) );
		}

		//cout << "setting active " << z << " to true" << endl;
		
		z->active = true;


		if( activatedZoneList == NULL )
		{
			activatedZoneList = z;
			z->activeNext = NULL;
		}
		else
		{
			z->activeNext = activatedZoneList;
			activatedZoneList = z;
		}
	}
	else
	{
		z->reexplored = true;
		{
			
			if (z->action == Zone::CLOSING)
			{
				z->frame = z->openFrames - z->frame;
			}
			else
			{
				z->frame = 0;
			}
			z->action = Zone::OPENING;
		}
		//already activated
		//assert(0);
	}
	if (currentZone != NULL )
	{
		if (currentZone->action == Zone::OPENING)
		{
			currentZone->frame = z->openFrames - currentZone->frame;
		}
		else
		{
			currentZone->frame = 0;
		}
		currentZone->action = Zone::CLOSING;
	}

	currentZone = z;

	if (!instant)
	{
		int soundIndex = SoundType::S_KEY_ENTER_0 + (currentZone->requiredKeys);
		soundNodeList->ActivateSound(gameSoundBuffers[soundIndex]);
	}
}

void GameSession::UnlockGate( Gate *g )
{
	g->SetLocked( false );

	//cout << "adding gate to unlock list: " << g << endl;
	if( unlockedGateList == NULL )
	{
		unlockedGateList = g;
		g->activeNext = NULL;
	}
	else
	{
		g->activeNext = unlockedGateList;
		unlockedGateList = g;
	}

	//if( currentZone != NULL )
	//{
	//	list<Edge*> &gList = currentZone->gates;
	//	for( list<Edge*>::iterator it = gList.begin(); it != gList.end(); ++it )
	//	{
	//		Gate *gg = (Gate*)(*it)->info;
	//		if( gg == g || gg->gState == Gate::OPEN || gg->gState == Gate::DISSOLVE )
	//			continue;

	//		gg->gState = Gate::LOCKFOREVER;
	//		//g->SetLocked();
	//	}
	//}
}

void GameSession::LockGate( Gate *g )
{
	//inefficient but i can adjust it later using prev pointers
	g->SetLocked( true );

	//Enemy *prev = e->prev;
	//Enemy *next = e->next;
	assert( unlockedGateList != NULL );
	if( unlockedGateList->activeNext == NULL )
	{
		unlockedGateList = NULL;
	}
	else
	{
		Gate *gate = unlockedGateList;
		if( g == unlockedGateList )
		{
			unlockedGateList = unlockedGateList->activeNext;
			//g->activeNext = unlockedGateList->activeNext;
			//unlockedGateList = g;
			//break;
		} 
		else
		while( gate != NULL )
		{
			//do i need this? i feel like i need this
			if( gate->activeNext == g )
			{
				Gate *gate2 = gate->activeNext->activeNext;
				g->activeNext = NULL;
				gate->activeNext = gate2;
				break;
			}
			gate = gate->activeNext;
		}
	}
	
}

void GameSession::TriggerBarrier( Barrier *b )
{
	PoiInfo *poi = b->poi;
	string name = poi->name;

	if( name == "birdfighttrigger" )
	{
		assert( b_bird != NULL );
		
		b_bird->spawned = true;
		AddEnemy( b_bird );
	}
	else if( name == "crawlerfighttrigger" )
	{
		Fade( false, 60, Color::Black );
		Pause( 60 );
		//activeSequence = b_crawler->crawlerFightSeq;
		activeSequence->frame = 0;

		//assert( b_crawler != NULL );
		//b_crawler->spawned = true;
		//AddEnemy( b_crawler );

	}
	else if( name == "meetcoyotetrigger" )
	{
		Fade( false, 60, Color::Black );
		Pause( 60 );
		//powerWheel->Hide( true, 60 );
		activeSequence = b_coyote->meetCoyoteSeq;
		activeSequence->frame = 0;

		assert( b_coyote != NULL );
		b_coyote->spawned = true;
		AddEnemy( b_coyote );
	}
	else if( name == "coyotefighttrigger" )
	{
		Fade( false, 60, Color::Black );
		Pause( 60 );
		//powerWheel->Hide( true, 60 );
		activeSequence = b_coyote->coyoteFightSeq;
		activeSequence->frame = 0;
	}
	else if( name == "skeletonfighttrigger" )
	{
		Fade( false, 60, Color::Black );
		Pause( 60 );
		activeSequence = b_skeleton->skeletonFightSeq;
		activeSequence->frame = 0;

		assert( b_skeleton != NULL );
		b_skeleton->spawned = true;
		AddEnemy( b_skeleton );
	}
}

Critical::Critical( V2d &pointA, V2d &pointB )
	:bar( sf::Quads, 4 )
{
	for( int i = 0; i < Gate::GateType::Count; ++i )
	{
		hadKey[i] = false;
	}


	anchorA = pointA;
	anchorB = pointB;
	
	V2d dir( anchorB - anchorA );
	double len = length( dir );
	dir = normalize( dir );
	
	pos = anchorA + dir * len / 2.0; //+ ( anchorB - anchorA ) / 2.0;
	radius = 100;

	next = NULL;
	prev = NULL;

	box.rw = radius;
	box.rh = radius;
	box.globalPosition = pos;
	box.isCircle = true;	
	
	active = true;

	double width = 5;
	V2d along = normalize( anchorB - anchorA );
	V2d other( along.y, -along.x );
	
	V2d leftv0 = anchorA - other * width;
	V2d rightv0 = anchorA + other * width;

	V2d leftv1 = anchorB - other * width;
	V2d rightv1 = anchorB + other * width;

	
	Color c = Color::Black;
	bar[0].color = c;
	bar[1].color = c;
	bar[2].color = c;
	bar[3].color = c;

	bar[0].position = Vector2f( leftv0.x, leftv0.y );
	bar[1].position = Vector2f( leftv1.x, leftv1.y );
	bar[2].position = Vector2f( rightv1.x, rightv1.y );
	bar[3].position = Vector2f( rightv0.x, rightv0.y );
}

void Critical::HandleQuery( QuadTreeCollider * qtc )
{
	qtc->HandleEntrant( this );
}

bool Critical::IsTouchingBox( const sf::Rect<double> &r )
{
	sf::Rect<double> circleBox( pos.x - radius, pos.y - radius, radius * 2, radius * 2 );
	return circleBox.intersects( r );
}

void Critical::Draw( RenderTarget *target )
{
	if( active )
	{
		CircleShape cs( radius );
		cs.setFillColor( Color::Magenta );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( pos.x, pos.y );

		
		target->draw( bar );
		target->draw( cs );
	}
	else
	{
		CircleShape cs( radius );
		cs.setFillColor( Color::Black );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( pos.x, pos.y );

		target->draw( bar );
		target->draw( cs );
	}
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