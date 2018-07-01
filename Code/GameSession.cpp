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
//#include "Enemy_Badger.h"
#include "Enemy_BasicEffect.h"
#include "Enemy_BasicTurret.h"
#include "ShipPickup.h"
//#include "Enemy_Bat.h"
#include "Enemy_Blocker.h"
#include "Enemy_Booster.h"
//#include "Enemy_Cactus.h"
//#include "Enemy_Cheetah.h"
//#include "Enemy_Copycat.h"
//#include "Enemy_CoralNanobots.h"
#include "Enemy_Crawler.h"
//#include "Enemy_CurveTurret.h"
#include "Enemy_FootTrap.h"
//#include "Enemy_Ghost.h"
#include "Enemy_Goal.h"
//#include "Enemy_Gorilla.h"
//#include "Enemy_GrowingTree.h"
//#include "Enemy_HealthFly.h"
//#include "Enemy_Jay.h"
//#include "Enemy_Mine.h"
//#include "Enemy_Narwhal.h"
//#include "Enemy_Overgrowth.h"
//#include "Enemy_Owl.h"
#include "Enemy_Patroller.h"
#include "Enemy_BirdBoss.h"
//#include "Enemy_PoisonFrog.h"
//#include "Enemy_Pulser.h"
#include "Enemy_RaceFightTarget.h"
#include "Background.h"
//#include "Enemy_SecurityWeb.h"
#include "Enemy_Shard.h"
#include "Enemy_Shroom.h"
#include "Enemy_CrawlerQueen.h"
//#include "Enemy_Shark.h"
//#include "Enemy_Specter.h"
//#include "Enemy_Spider.h"
//#include "Enemy_Spring.h"
//#include "Enemy_StagBeetle.h"
//#include "Enemy_Swarm.h"
//#include "Enemy_Turtle.h"
#include "HitboxManager.h"
#include "ShaderTester.h"

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


Grass::Grass(GameSession *p_owner, Tileset *p_ts_grass, int p_tileIndex,
	sf::Vector2<double> &pA, sf::Vector2<double> &pB,
	sf::Vector2<double> &pC, sf::Vector2<double> &pD, GameSession::TestVA *p_poly)
	:tileIndex(p_tileIndex), prev(NULL), next(NULL), visible(true),
	ts_grass(p_ts_grass), A(pA), B(pB), C(pC), D(pD), owner(p_owner), poly( p_poly )
{

	explosion.isCircle = true;
	explosion.rw = 64;
	explosion.rh = 64;
	explosion.type = CollisionBox::BoxType::Hit;

	explosion.globalPosition = (A + B + C + D) / 4.0;

	explodeFrame = 0;
	explodeLimit = 20;
	exploding = false;

	aabb.left = min(min(A.x, B.x), min(C.x, D.x));
	aabb.top = min(min(A.y, B.y), min(C.y, D.y));
	int right = max(max(A.x, B.x), max(C.x, D.x));
	int bot = max(max(A.y, B.y), max(C.y, D.y));

	aabb.width = right - aabb.left;
	aabb.height = bot - aabb.top;
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
	if (visible)
	{
		gva[tileIndex * 4 + 0].position = Vector2f( A.x, A.y );
		gva[tileIndex * 4 + 1].position = Vector2f(B.x, B.y);
		gva[tileIndex * 4 + 2].position = Vector2f(C.x, C.y);
		gva[tileIndex * 4 + 3].position = Vector2f(D.x, D.y);
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
	:groundPar( sf::Quads, 2 * 4 ), undergroundPar( sf::Quads, 4 ), underTransPar( sf::Quads, 2 * 4 ),
	onTopPar( sf::Quads, 4 * 6 ), miniVA( sf::Quads, 4 ), saveFile( sf ),
	cloud0( sf::Quads, 3 * 4 ), cloud1( sf::Quads, 3 * 4 ),
	cloudBot0( sf::Quads, 3 * 4 ), cloudBot1( sf::Quads, 3 * 4 ), fileName( p_filePath.string() ),
	filePath( p_filePath ), eHitParamsMan( NULL ), drain(true )
{	
	mainMenu = p_mainMenu;

	Init();
}

void GameSession::Cleanup()
{
	TerrainRender::CleanupLayers();

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

	if (powerRing != NULL)
	{
		delete powerRing;
		powerRing = NULL;
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

	if (gateTree != NULL)
	{
		delete gateTree;
		gateTree = NULL;
	}

	if (lightTree != NULL)
	{
		delete lightTree;
		lightTree = NULL;
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

	for (list<VertexArray*>::iterator it = polygons.begin(); it != polygons.end(); ++it)
	{
		delete (*it);
	}
	polygons.clear();

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


	for( int substep = 0; substep < NUM_MAX_STEPS; ++substep )
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
			cout << "fac: " << fac << endl;
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
	for (int i = 0; i < 4; ++i)
	{
		GetPrevInput(i) = GetCurrInput(i);
		GetPrevInputUnfiltered(i) = GetCurrInputUnfiltered(i);
		GameController &con = GetController(i);
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
	
	e->spawned = true;


	//^^note remove this later
	//debugging only
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
	cout << "removing enemy: " << e << endl;
	assert( activeEnemyList != NULL );
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

bool GameSession::LoadLights( ifstream &is, map<int, int> &polyIndex )
{
	int numLights;
	is >> numLights;
	for( int i = 0; i < numLights; ++i )
	{
		int x,y,r,g,b;
		int rad;
		int bright;
		is >> x;
		is >> y;
		is >> r;
		is >> g;
		is >> b;
		is >> rad;
		is >> bright;

		Light *light = new Light( this, Vector2i( x,y ), Color( r,g,b ), rad, bright );
		lightTree->Insert( light );
	}

	return true;
}

bool GameSession::LoadMovingPlats( ifstream &is, map<int, int> &polyIndex )
{
	int numMovingPlats;
	is >> numMovingPlats;
	for( int i = 0; i < numMovingPlats; ++i )
	{
		//dont forget this!
		//int matType;
		//is >> matType;

		//matSet.insert( matType );
		//string matStr;
		//is >> matStr;


		int polyPoints;
		is >> polyPoints;

		list<Vector2i> poly;

		for( int i = 0; i < polyPoints; ++i )
		{
			int px, py;
			is >> px;
			is >> py;
			
			poly.push_back( Vector2i( px, py ) );
		}


			
		list<Vector2i>::iterator it = poly.begin();
		int left = (*it).x;
		int right = (*it).x;
		int top = (*it).y;
		int bottom = (*it).y;
			
		for( ;it != poly.end(); ++it )
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


		//might need to round for perfect accuracy here
		Vector2i center( (left + right ) / 2, (top + bottom) / 2 );

		for( it = poly.begin(); it != poly.end(); ++it )
		{
			(*it).x -= center.x;
			(*it).y -= center.y;
		}

		int pathPoints;
		is >> pathPoints;

		list<Vector2i> path;

		for( int i = 0; i < pathPoints; ++i )
		{
			int x,y;
			is >> x;
			is >> y;
			path.push_back( Vector2i( x, y ) );
		}

			
		MovingTerrain *mt = new MovingTerrain( this, center, path, poly, false, 5 );
		movingPlats.push_back( mt );
	}

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



		list<Vector2i>::iterator it = poly.begin();
		list<Edge*> realEdges;
		double left, right, top, bottom;
		for( ; it != poly.end(); ++it )
		{
			Edge *ee = new Edge();
					
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

		if( mh->envLevel < 10 )
		{
			ss << "0" << matVariation + 1;
		}
		else
		{
			ss << matVariation + 1;
		}

		ss << ".png";
		
		//Tileset *ts_border = GetTileset( "w1_borders_64x64.png", 8, 64 );
		Tileset *ts_border = GetTileset( ss.str(), 8, 64 );//128 );
		//ts_border->texture->setSmooth( true );
		//ts_border->texture->setSmooth( true );
		VertexArray *groundVA = SetupBorderQuads( 1, realEdges.front(), ts_border,
			&GameSession::IsFlatGround );
		VertexArray *slopeVA = SetupBorderQuads( 1, realEdges.front(), ts_border,
			&GameSession::IsSlopedGround );
		VertexArray *steepVA = SetupBorderQuads( 1, realEdges.front(), ts_border,
			&GameSession::IsSteepGround );
		VertexArray *wallVA = SetupBorderQuads( 1, realEdges.front(), ts_border,
			&GameSession::IsWall );

		bool first = true;

		TestVA * testva = new TestVA;
		testva->next = NULL;
		//testva->va = va;
		testva->aabb.left = left;
		testva->aabb.top = top;
		testva->aabb.width = right - left;
		testva->aabb.height = bottom - top;
		testva->terrainVA = polygonVA;
		testva->grassVA = NULL;//grassVA;

		testva->numPoints = polyPoints;

		testva->ts_border = ts_border;
		testva->groundva = groundVA;
		testva->slopeva = slopeVA;
		testva->steepva = steepVA;
		testva->wallva = wallVA;

		
		
			
		//cout << "before insert border: " << insertCount << endl;
		//if( !inverse )
		//{
			borderTree->Insert( testva );
			allVA.push_back( testva );
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
		is >> behindyouStr;
		
		bool reformBehindYou;
		if( behindyouStr == "+reform" )
		{
			reformBehindYou = true;
		}
		else if( behindyouStr == "-reform" )
		{
			reformBehindYou = false;
		}
		else
		{
			cout << "behind you error: " << behindyouStr << endl;
			assert( false );
		}

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

		Gate * gate = new Gate( this, gateType, reformBehindYou );

		

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

bool GameSession::LoadEnemies( ifstream &is, map<int, int> &polyIndex )
{
	totalNumberBullets = 0;
	int shardsLoadedCounter = 0;

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
			string typeName;
			is >> typeName;

			Enemy *enem = NULL;

			if( typeName == "goal" )
			{
				//always grounded

				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				cout << "polyIndex: " << polyIndex[terrainIndex] << ", tindex: " << terrainIndex << endl;
				Goal *enemy = new Goal( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity );

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );

				hasGoal = true;
				goalPos = enemy->position;

				V2d gPos = enemy->ground->GetPoint( enemy->edgeQuantity );
				V2d norm = enemy->ground->Normal();
				double nodeHeight = 104;
				goalNodePos = gPos + norm * nodeHeight;
				cout << "setting goalPos: " << goalPos.x << ", " << goalPos.y << endl;
			}
			else if( typeName == "healthfly" )
			{
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;

				int color;
				is >> color;
				
				//HealthFly::FlyType fType = (HealthFly::FlyType)color;
				
				/*HealthFly *enemy = new HealthFly( this, Vector2i( xPos, yPos ), fType );
				
				

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );*/
			}
			else if( typeName == "poi" )
			{
				string air;
				string pname;
				PoiInfo *pi = NULL;

				is >> air;

				if( air == "+air" )
				{
					Vector2i pos;
					is >> pos.x;
					is >> pos.y;
					
					
					is >> pname;

					string barStr;
					is >> barStr;

					int hasCamProperties;
					is >> hasCamProperties;

					pi = new PoiInfo( pname, pos );

					pi->hasCameraProperties = hasCamProperties;

					if( pi->hasCameraProperties )
					{
						float camZoom;
						is >> camZoom;

						pi->cameraZoom = camZoom;
					}
					else
					{
					}

					

					if( barStr == "-" )
					{
					}
					else if( barStr == "x" )
					{
	/*					bleft = owner->poiMap["coyfightleft"]->barrier;
	bright = owner->poiMap["coyfightright"]->barrier;
	btop = owner->poiMap["coyfighttop"]->barrier;
	bbot = owner->poiMap["coyfightbot"]->barrier;*/

						Barrier *b = new Barrier( this, pi, true, pos.x, ( GetPlayer( 0 )->position.x > pos.x ),  NULL );
						string na = b->poi->name;
						/*if( na == "coyfightleft"
							|| na == "coyfightright"
							|| na == "coyfighttop"
							|| na == "coyfightbot" )
							b->triggered = true;*/
						barriers.push_back( b );
						
					}
					else if( barStr == "y" )
					{
						Barrier *b = new Barrier( this, pi, false, pos.y, ( GetPlayer( 0 )->position.y > pos.y ), NULL );
						//b->triggered = true;
						barriers.push_back( b );
					}
					else
					{
						assert( 0 );
					}

					if( barStr != "-" )
					{
						if( fileName == "Maps/poitest.brknk" )
						{
							if( pname == "birdfighttrigger" )
							{
								/*BirdMeetingCallback *bmc = new BirdMeetingCallback( this );
								barriers.back()->callback = bmc;*/
							}
						}
					}
				}
				else if( air == "-air" )
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

					V2d p = e->GetPoint( edgeQuantity );

					pi = new PoiInfo( pname, e,
						edgeQuantity );

					if( barStr == "-" )
					{
					}
					else if( barStr == "x" )
					{
						barriers.push_back( new Barrier( this, pi, true, floor( p.x + .5 ), ( GetPlayer( 0 )->position.x > p.x ), NULL  ) );
					}
					else if( barStr == "y" )
					{
						barriers.push_back( new Barrier( this, pi, false, floor( p.y + .5 ), ( GetPlayer( 0 )->position.y > p.y ), NULL ) );
					}
					else
					{
						assert( 0 );
					}

					
					//edges[polyIndex[terrainIndex] + edgeIndex]					
				}
				else
				{
					cout << "air failure: " << air << endl;
					assert( 0 );
				}

				poiMap[pname] = pi;
				//poiMap
			}
			else if( typeName == "shard" )
			{
				int xPos,yPos;

				//always air

				is >> xPos;
				is >> yPos;
				
				string shardStr;
				is >> shardStr;

				ShardType sType = Shard::GetShardType(shardStr);

				Shard *enemy = new Shard( this, Vector2i( xPos, yPos ), sType );

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
			}
			else if (typeName == "blocker")
			{
				int xPos, yPos;

				//always air


				is >> xPos;
				is >> yPos;

				int pathLength;
				is >> pathLength;

				list<Vector2i> localPath;
				for (int i = 0; i < pathLength; ++i)
				{
					int localX, localY;
					is >> localX;
					is >> localY;
					localPath.push_back(Vector2i(localX, localY));
				}

				int bType;
				is >> bType;

				int armored;
				is >> armored;

				int spacing;
				is >> spacing;

				BlockerChain *enemy =new BlockerChain(this, Vector2i(xPos, yPos), localPath, bType, armored, spacing);

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

				int strength;
				is >> strength;

				Booster *enemy = new Booster(this, Vector2i(xPos, yPos),strength);

				activeItemTree->Insert(enemy);
				//fullEnemyList.push_back(enemy);
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
			else if (typeName == "spring")
			{
				int xPos, yPos;

				//always air

				is >> xPos;
				is >> yPos;

				int moveFrames;
				is >> moveFrames;

				Vector2i other;
				is >> other.x;
				is >> other.y;

				
				
				/*Spring *enemy = new Spring(this, Vector2i( xPos, yPos ), other, moveFrames );

				fullEnemyList.push_back(enemy);
				enem = enemy;

				enemyTree->Insert(enemy);*/
			}
			else if (typeName == "rail")
			{
				int xPos, yPos;

				//always air


				is >> xPos;
				is >> yPos;

				int pathLength;
				is >> pathLength;

				list<Vector2i> localPath;
				for (int i = 0; i < pathLength; ++i)
				{
					int localX, localY;
					is >> localX;
					is >> localY;
					localPath.push_back(Vector2i(localX, localY));
				}

				int energized;
				is >> energized;

				//Rail *r = new Rail(this, Vector2i( xPos, yPos ), localPath, energized);
			}


			//w1
			else if( typeName == "patroller" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;
				

				int pathLength;
				is >> pathLength;

				list<Vector2i> localPath;
				for( int i = 0; i < pathLength; ++i )
				{
					int localX,localY;
					is >> localX;
					is >> localY;
					localPath.push_back( Vector2i( localX, localY ) );
				}


				bool loop;
				string loopStr;
				is >> loopStr;

				if( loopStr == "+loop" )
				{
					loop = true;
				}
				else if( loopStr == "-loop" )
				{
					loop = false;
				}
				else
				{
					assert( false && "should be a boolean" );
				}

				int speed;
				is >> speed;
				Patroller *enemy = new Patroller( this, hasMonitor, Vector2i( xPos, yPos ), localPath, loop, speed );

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
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

				CrawlerQueen *enemy = new CrawlerQueen( this, edges[polyIndex[terrainIndex] + edgeIndex],
					edgeQuantity, false);

				fullEnemyList.push_back(enemy);

				enem = enemy;

				enemyTree->Insert(enemy);
				/*Boss_Crawler *enemy = new Boss_Crawler( this, edges[polyIndex[terrainIndex] + edgeIndex],
					edgeQuantity );

				fullEnemyList.push_back( enemy );

				b_crawler = enemy;*/
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

				int hasMonitor;
				is >> hasMonitor;

				double bulletSpeed;
				is >> bulletSpeed;

				int framesWait;
				is >> framesWait;

				BasicTurret *enemy = new BasicTurret( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, bulletSpeed, framesWait );
				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
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

				int hasMonitor;
				is >> hasMonitor;

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

				int speed;
				is >> speed;

				int dist;
				is >> dist;

				if( hasMonitor )
				{
					cout << "preview has monitor" << endl;
				}
				else
				{
					cout << "preview NO MONITOR" << endl;
				}

				//BossCrawler *enemy = new BossCrawler( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity );
				Crawler *enemy = new Crawler( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], 
					edgeQuantity, clockwise, speed, dist );


				/*if( enemy->hasMonitor )
					cout << "crawler with monitor!" << endl;
				else
					cout << "no monitor here" << endl;*/

				//enemyTree = Insert( enemyTree, enemy );
				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
			}
			else if( typeName == "foottrap" )
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

				//FootTrap *enemy = new FootTrap( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity );
				Shroom *enemy = new Shroom( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity);

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );
			}

			//w2
			else if( typeName == "bat" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;
				

				int pathLength;
				is >> pathLength;

				list<Vector2i> localPath;
				for( int i = 0; i < pathLength; ++i )
				{
					int localX,localY;
					is >> localX;
					is >> localY;
					localPath.push_back( Vector2i( localX, localY ) );
				}


				bool loop;
				string loopStr;
				is >> loopStr;

				if( loopStr == "+loop" )
				{
					loop = true;
				}
				else if( loopStr == "-loop" )
				{
					loop = false;
				}
				else
				{
					assert( false && "should be a boolean" );
				}

				int bulletSpeed;
				is >> bulletSpeed;

				//int nodeDistance;
				//is >> nodeDistance;

				int framesBetweenNodes;
				is >> framesBetweenNodes;

				/*Bat *enemy = new Bat( this, hasMonitor, Vector2i( xPos, yPos ), localPath, 
					bulletSpeed, framesBetweenNodes, loop );

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );*/
			}
			else if( typeName == "poisonfrog" )
			{
				int terrainIndex;
				is >> terrainIndex;

				int edgeIndex;
				is >> edgeIndex;

				double edgeQuantity;
				is >> edgeQuantity;

				int hasMonitor;
				is >> hasMonitor;

				int gravFactor;
				is >> gravFactor;

				int jumpStrengthX;
				is >> jumpStrengthX;

				int jumpStrengthY;
				is >> jumpStrengthY;

				int jumpFramesWait;
				is >> jumpFramesWait;

				/*PoisonFrog *enemy = new PoisonFrog( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], 
					edgeQuantity, gravFactor, Vector2i( jumpStrengthX, jumpStrengthY ), jumpFramesWait );

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );*/
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

				int hasMonitor;
				is >> hasMonitor;

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

				
				/*StagBeetle *enemy = new StagBeetle( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], 
					edgeQuantity, clockwise, speed );
				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );*/
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

				int hasMonitor;
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
				if( relativeGravStr == "+relative" )
				{
					relative = true;
				}

				/*CurveTurret *enemy = new CurveTurret( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, bulletSpeed, framesWait,
					Vector2i( xGravFactor, yGravFactor ), relative );

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );*/
			}
			else if( typeName == "bossbird" )
			{

				int xPos,yPos;

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
			else if( typeName == "pulser" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;
				

				int pathLength;
				is >> pathLength;

				list<Vector2i> localPath;
				for( int i = 0; i < pathLength; ++i )
				{
					int localX,localY;
					is >> localX;
					is >> localY;
					localPath.push_back( Vector2i( localX, localY ) );
				}


				bool loop;
				string loopStr;
				is >> loopStr;

				if( loopStr == "+loop" )
				{
					loop = true;
				}
				else if( loopStr == "-loop" )
				{
					loop = false;
				}
				else
				{
					assert( false && "should be a boolean" );
				}

				int framesBetweenNodes;
				is >> framesBetweenNodes;

				/*Pulser *enemy = new Pulser( this,hasMonitor, Vector2i( xPos, yPos ), localPath,
					framesBetweenNodes, loop );

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );*/
			}
			else if( typeName == "cactus" )
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

				int bulletSpeed;
				is >> bulletSpeed;

				int rhythm;
				is >> rhythm;

				int amplitude;
				is >> amplitude;

				/*Cactus *enemy = new Cactus( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, bulletSpeed, rhythm,
					amplitude );

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );*/
			}
			else if( typeName == "owl" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;
				
				int moveSpeed;
				is >> moveSpeed;

				int bulletSpeed;
				is >> bulletSpeed;

				int rhythmFrames;
				is >> rhythmFrames;
				

				/*Owl *enemy = new Owl( this, hasMonitor, Vector2i( xPos, yPos ), moveSpeed,
					bulletSpeed, rhythmFrames );

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );*/
			}
			else if( typeName == "badger" )
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

				int speed;
				is >> speed;

				int jumpStrength;
				is >> jumpStrength;
				
				//Badger *enemy = new Badger( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity, true,
				//	speed, jumpStrength );

				//fullEnemyList.push_back( enemy );
				//enem = enemy;

				//enemyTree->Insert( enemy );
			}
			else if( typeName == "bosscoyote" )
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
			else if( typeName == "turtle" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;				

				Vector2i delta( 1000, -1000 );
				Vector2i pos( xPos, yPos );
				/*Turtle *enemy = new Turtle( this, hasMonitor, Vector2i( xPos, yPos ) );
				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );*/
			}
			else if( typeName == "coral" )
			{
					
				int xPos,yPos;

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
			else if( typeName == "cheetah" )
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
				
				/*Cheetah *enemy = new Cheetah( this, hasMonitor,
					edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity );
				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );*/
			}
			else if( typeName == "spider" )
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

				int speed;
				is >> speed;
				
				//Spider *enemy = new Spider( this, hasMonitor, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity,
				//	speed );


				//fullEnemyList.push_back( enemy );
				//enem = enemy;

				//enemyTree->Insert( enemy );
			}
			else if( typeName == "bosstiger" )
			{
				int xPos,yPos;

				is >> xPos;
				is >> yPos;

				/*Boss_Tiger *enemy = new Boss_Tiger( this, Vector2i ( xPos, yPos ) );

				fullEnemyList.push_back( enemy );

				b_tiger = enemy;

				enemyTree->Insert( enemy );*/
			}

			//w5
			else if( typeName == "swarm" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;				

				int liveFrames;
				is >> liveFrames;

				/*Swarm *enemy = new Swarm( this, Vector2i( xPos, yPos ), liveFrames );
				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );*/
			}
			else if( typeName == "shark" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;				

				int circleFrames;
				is >> circleFrames;

				/*Shark *enemy = new Shark( this, hasMonitor, Vector2i( xPos, yPos ), circleFrames );
				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );*/
			}
			else if( typeName == "ghost" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;				

				int speed;
				is >> speed;

				/*Ghost *enemy = new Ghost( this, hasMonitor, Vector2i( xPos, yPos ), speed );

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );*/
			}
			else if( typeName == "overgrowth" )
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
			else if( typeName == "bossgator" )
			{

				int xPos,yPos;

				is >> xPos;
				is >> yPos;

				/*Boss_Gator *enemy = new Boss_Gator( this, Vector2i ( xPos, yPos ) );

				fullEnemyList.push_back( enemy );

				b_gator = enemy;

				enemyTree->Insert( enemy );*/
			}

			//w6
			else if( typeName == "specter" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;				

				//Specter *enemy = new Specter( this, hasMonitor, Vector2i( xPos, yPos ) );
				//fullEnemyList.push_back( enemy );
				//enem = enemy;

				//enemyTree->Insert( enemy );
			}
			else if( typeName == "narwhal" )
			{
					
				int xPos,yPos;

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
			else if( typeName == "copycat" )
			{
					
				int xPos,yPos;

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
			else if( typeName == "gorilla" )
			{
					
				int xPos,yPos;

				//always air


				is >> xPos;
				is >> yPos;
				
				int hasMonitor;
				is >> hasMonitor;	

				int wallWidth;
				is >> wallWidth;

				int followFrames;
				is >> followFrames;

				
				/*Gorilla *enemy = new Gorilla( this, hasMonitor, Vector2i( xPos, yPos ),
					wallWidth, followFrames );
				
				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );*/
			}
			else if( typeName == "bossskeleton" )
			{
				int xPos,yPos;

				is >> xPos;
				is >> yPos;

				/*Boss_Skeleton *enemy = new Boss_Skeleton( this, Vector2i ( xPos, yPos ) );

				fullEnemyList.push_back( enemy );

				b_skeleton = enemy;*/
			}


			else if( typeName == "nexus" )
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

				/*Nexus *enemy = new Nexus( this, edges[polyIndex[terrainIndex] + edgeIndex], edgeQuantity,
					nexusIndex );

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );*/
			}
			else if( typeName == "shippickup" )
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

				fullEnemyList.push_back( enemy );
				enem = enemy;

				enemyTree->Insert( enemy );

				if( shipExitSeq == NULL )
				{
					shipExitSeq = new ShipExitSeq( this );
				}
			}
			//w6
			else if( typeName == "racefighttarget" )
			{
				int xPos,yPos;

				//always air

				is >> xPos;
				is >> yPos;

				if( raceFight != NULL )
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
				assert( false && "not a valid type name" );
			}

			//if( enem != NULL )
			//{
			//	totalNumberBullets += enem->NumTotalBullets();
			//}

		}
	}

	//create VA

	if( totalNumberBullets > 0 )
	{
		bigBulletVA = new VertexArray( sf::Quads, totalNumberBullets * 4 );
		VertexArray &bva = *bigBulletVA;
		for( int i = 0; i < totalNumberBullets * 4; ++i )
		{
			bva[i].position = Vector2f( 0, 0 );
		}
		ts_basicBullets = GetTileset( "bullet_64x64.png", 64, 64 );
	}
	else
	{
		ts_basicBullets = NULL;
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

bool GameSession::OpenFile( string fileName )
{
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

		if (mh->gameMode == MapHeader::MapType::T_RACEFIGHT)
		{
			assert(raceFight == NULL);
			raceFight = new RaceFight(this, 180);
		}

		

		//cout << "-------------" << endl;
		//pauseMenu = new PauseMenu( this );
		//pauseMenu->SetTab( PauseMenu::Tab::KIN );
		//cout << "after-------------" << endl;
		
		points = new Vector2<double>[mh->numVertices];
		

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
			
			//matWorld = 6;
			//matWorld = 2;
			matSet.insert( pair<int,int>( matWorld, matVariation ) );

			int polyPoints;
			is >> polyPoints;

			polyIndex[polyCounter] = pointCounter;
			cout << "setting poly index at : " << polyCounter << " to " << pointCounter << endl;

			int currentEdgeIndex = pointCounter;
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

			
			TestVA * testva = NULL;
			Tileset *ts_bush = GetTileset( "bush_01_64x64.png", 64, 64 );

			if( !inverse )
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

				testva = new TestVA;


				//int minDistanceApart = 10;
	//int maxDistanceApart = 300;
	//int minPen = 20;
	//int maxPen = 200;
				//list<Vector2f> bushPositions;
				


				

				
				testva->polyArea = polygonArea;
				//testva->bushVA = bushVA;
				//testva->va = va;
				testva->aabb.left = left;
				testva->aabb.top = top;
				testva->aabb.width = right - left;
				testva->aabb.height = bottom - top;
				testva->terrainVA = va;

				polygons.push_back( va );

				delete cdt;
				for( int i = 0; i < polyPoints; ++i )
				{
					delete polyline[i];
				}

			}
			else
			{
				inversePoly = new TestVA;
				inversePoly->numPoints = polyPoints;
				//testva->va = va;
				inversePoly->aabb.left = left;
				inversePoly->aabb.top = top;
				inversePoly->aabb.width = right - left;
				inversePoly->aabb.height = bottom - top;
				//testva->ts_bush = ts_bush;
				SetupInversePoly( ts_bush, currentEdgeIndex );
				testva = inversePoly;
				testva->ts_bush = ts_bush;
				//va = NULL;
			}

			switch( matWorld )
				{
				case 0:
					{
						switch( matVariation )
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
						switch( matVariation )
						{
						case 0:
							break;
						}
						break;
					}
				case 2:
					{
						switch( matVariation )
						{
						case 0:
							break;
						}
						break;
					}
				case 3:
					{
						switch( matVariation )
						{
						case 0:
							break;
						}
						break;
					}
				case 4:
					{
						switch( matVariation )
						{
						case 0:
							break;
						}
						break;
					}
				case 5:
					{
						switch( matVariation )
						{
						case 0:
							break;
						}
						break;
					}
				case 6:
					{
						switch( matVariation )
						{
						case 0:
							break;
						}
						break;
					}
				}

			for (int i = 0; i < 6; ++i)
			{
				DecorExpression *expr = CreateDecorExpression(DecorType(D_W1_VEINS1 + i), 0, edges[currentEdgeIndex]);
				if (expr != NULL)
					testva->AddDecorExpression(expr);
			}

			Tileset *ts_testBush = GetTileset("bush_1_01_512x512.png", 512, 512);

			DecorExpression *rock1 = CreateDecorExpression(D_W1_ROCK_1, 0, edges[currentEdgeIndex]);
			if (rock1 != NULL)
				testva->AddDecorExpression(rock1);

			DecorExpression *rock2 = CreateDecorExpression(D_W1_ROCK_2, 0, edges[currentEdgeIndex]);
			if (rock2 != NULL)
				testva->AddDecorExpression(rock2);

			DecorExpression *rock3 = CreateDecorExpression(D_W1_ROCK_3, 0, edges[currentEdgeIndex]);
			if (rock3 != NULL)
				testva->AddDecorExpression(rock3);

			DecorExpression *grassyRock= CreateDecorExpression(D_W1_GRASSYROCK, 0, edges[currentEdgeIndex]);
			if (grassyRock != NULL)
				testva->AddDecorExpression(grassyRock);

			DecorExpression *normalExpr = CreateDecorExpression(D_W1_BUSH_NORMAL, 0, edges[currentEdgeIndex] );
			if( normalExpr != NULL )
				testva->AddDecorExpression( normalExpr );
 
			DecorExpression *exprPlantRock = CreateDecorExpression(D_W1_PLANTROCK, 0, edges[currentEdgeIndex]);
			if (exprPlantRock != NULL)
				testva->AddDecorExpression(exprPlantRock);

			
			

			

		/*	DecorExpression *testExpr = GetBush_NORMAL_Points(0, edges[currentEdgeIndex], 20,
				300, CubicBezier(0, 0, 1, 1), 20, 1000, CubicBezier(0, 0, 1, 1));

			if (testExpr != NULL)
			{
				testva->AddDecorExpression(testExpr);
			}*/

			VertexArray *polygonVA = va;

			double totalPerimeter = 0;


			double grassSize = 22;
			double grassSpacing = -5;

			Edge * testEdge = edges[currentEdgeIndex];

			int numGrassTotal = 0;

			for( list<GrassSegment>::iterator it = segments.begin(); it != segments.end(); ++it )
			{
				numGrassTotal += (*it).reps + 1;
			}

			VertexArray *grassVA = NULL;
			
			if( numGrassTotal > 0 )
			{
				grassVA = new VertexArray( sf::Quads, numGrassTotal * 4 );

				//cout << "num grass total: " << numGrassTotal << endl;
				VertexArray &grassVa = *grassVA;//*va;

				int segIndex = 0;
				int totalGrass = 0;
				for( list<GrassSegment>::iterator it = segments.begin(); it != segments.end(); ++it )
				{	
					Edge *segEdge = edges[currentEdgeIndex + (*it).edgeIndex];
					V2d v0 = segEdge->v0;
					V2d v1 = segEdge->v1;

					int start = (*it).index;
					int end = (*it).index + (*it).reps;

					int grassCount = (*it).reps + 1;
					//cout << "Grasscount: " << grassCount << endl;

					double remainder = length( v1 - v0 ) / ( grassSize + grassSpacing );

					int num = floor( remainder ) + 1;

					for( int i = 0; i < grassCount; ++i )
					{
						//cout << "indexing at: " << i*4 + segIndex * 4 << endl;
						V2d posd = v0 + (v1 - v0 ) * ((double)( i + start ) / num);
						Vector2f pos( posd.x, posd.y );

						Vector2f topLeft = pos + Vector2f( -grassSize / 2, -grassSize / 2 );
						Vector2f topRight = pos + Vector2f( grassSize / 2, -grassSize / 2 );
						Vector2f bottomLeft = pos + Vector2f( -grassSize / 2, grassSize / 2 );
						Vector2f bottomRight = pos + Vector2f( grassSize / 2, grassSize / 2 );

							//grassVa[i*4].color = Color( 0x0d, 0, 0x80 );//Color::Magenta;
					//borderVa[i*4].color.a = 10;
						grassVa[(i+totalGrass)*4].position = topLeft;
						grassVa[(i+totalGrass)*4].texCoords = Vector2f( 0, 0 );

							//grassVa[i*4+1].color = Color::Blue;
					//borderVa[i*4+1].color.a = 10;

						int stuff = (i + totalGrass) * 4 + 1;

						cout << "i: " << i << ", stuff: " << stuff << ", total: " << totalGrass << endl;
						grassVa[(i+totalGrass)*4+1].position = bottomLeft;
						grassVa[(i+totalGrass)*4+1].texCoords = Vector2f( 0, grassSize );

							//grassVa[i*4+2].color = Color::Blue;
					//borderVa[i*4+2].color.a = 10;
						grassVa[(i+totalGrass)*4+2].position = bottomRight;
						grassVa[(i+totalGrass)*4+2].texCoords = Vector2f( grassSize, grassSize );

							//grassVa[i*4+3].color = Color( 0x0d, 0, 0x80 );
					//borderVa[i*4+3].color.a = 10;
						grassVa[(i+totalGrass)*4+3].position = topRight;
						grassVa[(i+totalGrass)*4+3].texCoords = Vector2f( grassSize, 0 );
						//++i;
					}
					totalGrass += grassCount;
					segIndex++;
				}
			}
			else
			{
				grassVA = NULL;
			}

			int totalGrassIndex = 0;
			Tileset *ts_grass = GetTileset("grass_32x32.png", 32, 32);
			if( false )
			for (list<GrassSegment>::iterator it = segments.begin(); it != segments.end(); ++it)
			{
				V2d A, B, C, D;
				Edge * currE = edges[currentEdgeIndex + (*it).edgeIndex];
				V2d v0 = currE->v0;
				V2d v1 = currE->v1;

				double grassSize = 22;
				double grassSpacing = -5;

				double edgeLength = length(v1 - v0);
				double remainder = edgeLength / (grassSize + grassSpacing);

				double num = floor(remainder) + 1;

				int reps = (*it).reps;

				V2d edgeDir = normalize(v1 - v0);

				//V2d ABmin = v0 + (v1-v0) * (double)(*it).index / num - grassSize / 2 );
				V2d ABmin = v0 + edgeDir * (edgeLength * (double)(*it).index / num - grassSize / 2);
				V2d ABmax = v0 + edgeDir * (edgeLength * (double)((*it).index + reps) / num + grassSize / 2);
				double height = grassSize / 2;
				V2d normal = normalize(v1 - v0);
				double temp = -normal.x;
				normal.x = normal.y;
				normal.y = temp;

				A = ABmin + normal * height;
				B = ABmax + normal * height;
				C = ABmax;
				D = ABmin;

				/*Grass * g = new Grass(this, ts_grass, tdCotalGrassIndex,);
				g->A = A;
				g->B = B;
				g->C = C;
				g->D = D;

				grassTree->Insert(g);*/

				totalGrassIndex++;
			}

			stringstream ss;

			ss << "Borders/bor_" << matWorld + 1 << "_";

			if(mh->envLevel < 10 )
			{
				ss << "0" << matVariation + 1;
			}
			else
			{
				ss << matVariation + 1;
			}

			//ss << "_128x64.png";
			ss << "_32x64.png";
		
			//Tileset *ts_border = GetTileset( "w1_borders_64x64.png", 8, 64 );
			Tileset *ts_border = GetTileset( ss.str(), 32, 64 );

			VertexArray *groundVA = SetupBorderQuads( 0, edges[currentEdgeIndex], ts_border,
				&GameSession::IsFlatGround );
			VertexArray *slopeVA = SetupBorderQuads( 0, edges[currentEdgeIndex], ts_border,
				&GameSession::IsSlopedGround );
			VertexArray *steepVA = SetupBorderQuads( 0, edges[currentEdgeIndex], ts_border,
				&GameSession::IsSteepGround );
			VertexArray *wallVA = SetupBorderQuads( 0, edges[currentEdgeIndex], ts_border,
				&GameSession::IsWall );

			Tileset *ts_plant = GetTileset( "testgrass.png", 32, 32 );
			

			VertexArray *plantVA = SetupPlants( edges[currentEdgeIndex], ts_plant );

			Tileset *ts_border1 = GetTileset("Borders/bor_1_01_512x704.png", 128, 64);
			testva->tr = new TerrainRender( &tm, terrainTree );// (terrainTree);
			testva->tr->startEdge = edges[currentEdgeIndex];
			testva->tr->GenerateBorderMesh();
			testva->tr->GenerateDecor();
			testva->tr->ts_border = ts_border1;
			

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
			
		

			
			testva->plantva = NULL; //temporary
			testva->next = NULL;
			
			
			testva->grassVA = grassVA;
			testva->ts_bush = ts_bush;
			//testva->bushVA = bushVA;
			

			testva->ts_border = ts_border;
			testva->groundva = groundVA;
			testva->slopeva = slopeVA;
			testva->steepva = steepVA;
			testva->wallva = wallVA;
			testva->triva = triVA;
			testva->plantva = plantVA;
			testva->ts_plant = ts_plant;
			testva->terrainWorldType = matWorld;
			testva->terrainVariation = matVariation;

			//testva->flowva = energyFlowVA;
			
			//cout << "before insert border: " << insertCount << endl;
			borderTree->Insert( testva );
			allVA.push_back( testva );

			//cout << "after insert border: " << insertCount << endl;
			insertCount++;
			

			

			//cout << "loaded to here" << endl;
			
			++polyCounter;
		}
		
		

		LoadMovingPlats( is, polyIndex );

		LoadBGPlats( is, polyIndex );

		LoadLights( is, polyIndex );

		LoadEnemies( is, polyIndex );
		
		LoadGates( is, polyIndex );

		is.close();



		
	
		

		//loading done. now setup

		SetGlobalBorders();

		double extraBorder = 100;
		bool topBorderOn = false;
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



			//mh->topBounds = ;
			//mh->leftBounds = inversePoly->aabb.left - extra;
			//mh->boundsWidth = inversePoly->aabb.width + extra * 2;
			mh->topBounds = trueTop;
			mh->boundsHeight = (inversePoly->aabb.top + inversePoly->aabb.height + extraBorder) - trueTop;
		}

		bool blackBorder[2];
		int newRight = (inversePoly->aabb.left + inversePoly->aabb.width);
		blackBorder[0]= inversePoly->aabb.left < mh->leftBounds;
		blackBorder[1] = newRight > (mh->leftBounds + mh->boundsWidth);

		int leftB = mh->leftBounds;
		int rightB = mh->leftBounds + mh->boundsWidth;
		if (!blackBorder[0])
		{
			mh->leftBounds = inversePoly->aabb.left - extraBorder;
		}
		if (!blackBorder[1])
		{
			int newRight = (inversePoly->aabb.left + inversePoly->aabb.width);
			mh->boundsWidth = (newRight + extraBorder) - mh->leftBounds;
		}

		for (int i = 0; i < 8; ++i)
		{
			//blackBorderQuads[i].color = Color::Black;
			blackBorderQuads[i].position.y = mh->topBounds;
		}

		int quadWidth = 200;
		blackBorderQuads[0].position.x = mh->leftBounds;
		blackBorderQuads[3].position.x = mh->leftBounds;

		blackBorderQuads[1].position.x = mh->leftBounds + quadWidth;
		blackBorderQuads[2].position.x = mh->leftBounds + quadWidth;

		blackBorderQuads[5].position.x = mh->leftBounds + mh->boundsWidth;
		blackBorderQuads[6].position.x = mh->leftBounds + mh->boundsWidth;

		blackBorderQuads[4].position.x = blackBorderQuads[5].position.x - quadWidth;
		blackBorderQuads[7].position.x = blackBorderQuads[5].position.x - quadWidth;

		blackBorderQuads[2].position.y += mh->boundsHeight;
		blackBorderQuads[3].position.y += mh->boundsHeight;
		blackBorderQuads[6].position.y += mh->boundsHeight;
		blackBorderQuads[7].position.y += mh->boundsHeight;

		if (blackBorder[0])
		{
			//SetRectColor(blackBorderQuads, Color(100, 100, 100));
			SetRectColor(blackBorderQuads, Color(Color::Black));
			blackBorderQuads[1].color.a = 0;
			blackBorderQuads[2].color.a = 0;
		}
		else
		{
			SetRectColor(blackBorderQuads, Color(Color::Transparent));
		}
		if (blackBorder[1])
		{
			SetRectColor(blackBorderQuads + 4, Color(Color::Black));
			blackBorderQuads[4].color.a = 0;
			blackBorderQuads[7].color.a = 0;
		}
		else
		{
			SetRectColor(blackBorderQuads + 4, Color(Color::Transparent));
		}

		int miniQuadWidth = 4000;
		int inverseTerrainBorder = 4000;
		int blackMiniTop = mh->topBounds - inverseTerrainBorder;
		int blackMiniBot = mh->topBounds + mh->boundsHeight + inverseTerrainBorder;
		int blackMiniLeft = mh->leftBounds - miniQuadWidth;
		int rightBounds = mh->leftBounds + mh->boundsWidth;
		int blackMiniRight = rightBounds + miniQuadWidth;
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

		if( blackBorder[0] )
			SetRectColor(blackBorderQuadsMini, miniTopBorderColor);
		if (blackBorder[1])
			SetRectColor(blackBorderQuadsMini + 4, miniTopBorderColor);
		

		if (topBorderOn)
		{
			SetRectColor(topBorderQuadMini, Color(0x10, 0x40, 0xff));

			topBorderQuadMini[0].position.x = blackMiniLeft;
			topBorderQuadMini[1].position.x = blackMiniRight;
			topBorderQuadMini[2].position.x = blackMiniRight;
			topBorderQuadMini[3].position.x = blackMiniLeft;

			topBorderQuadMini[0].position.y = blackMiniTop;
			topBorderQuadMini[1].position.y = blackMiniTop;
			topBorderQuadMini[2].position.y = mh->topBounds;
			topBorderQuadMini[3].position.y = mh->topBounds;

			topClouds = new TopClouds(this);
		}

		if (poiMap.count("stormceiling") > 0)
		{
			stormCeilingOn = true;
			stormCeilingHeight = poiMap["stormceiling"]->pos.y;

			int oldBottom = mh->topBounds + mh->boundsHeight - extraBorder;
			mh->topBounds = stormCeilingHeight;
			mh->boundsHeight = oldBottom - stormCeilingHeight;
			assert(mh->boundsHeight > 0);

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


		CreateZones();
		SetupZones();

		if( b_crawler != NULL )
		{
			b_crawler->Init();
		}

		if( b_bird != NULL )
		{
			b_bird->Init();
		}

		if( b_tiger != NULL )
		{
			b_tiger->Init();
		}

		if( b_gator != NULL )
		{
			b_gator->Init();
		}

		if( b_skeleton != NULL )
		{
			b_skeleton->Init();
		}

		for( int i = 0; i < numGates; ++i )
		{
			Gate *g = gates[i];
			if( g->type == Gate::BIRDFIGHT )
			{
				if( g->zoneA != NULL )
					g->zoneA->showShadow = false;
					//ActivateZone( zone
					//g->zoneA->ac
				//g->SetLocked( false );
				//g->SetLocked( true );
			}
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
			drain = true;
			shipSequence = false;
			//normal map
		}

		if( poiMap.count( "bc_alliance" ) > 0 )
		{
			startMapSeq = new BirdCrawlerAllianceSeq( this );
			activeSequence = startMapSeq;
			startMapSeq->Reset();
			fadeAlpha = 255;
			//owner->Fade( false, 60, Color::Black );
			//owner->Pause( 60 );
			//activeSequence = 
			
			//player->action = Actor::Action::Se;
			//player->frame = 1;
		}

		


		//cout << "end of this" << endl;

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
	//terrainTree->Insert(bot);

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

	//list<Gate*> ;
	list<Edge*> outsideGates;

	int numOutsideGates = 0;
	for( int i = 0; i < numGates; ++i )
	{
		Gate *g = gates[i];
		if (g->zoneA == NULL || g->zoneB == NULL)
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
		/*if( g->zoneA == NULL )
		{
			outsideGates.push_back( g->edgeA );
			numOutsideGates++;
		}
		else if( g->zoneB == NULL )
		{
			outsideGates.push_back( g->edgeB );
			numOutsideGates++;
		}*/
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
	if( numOutsideGates > 0 && borderEdge != NULL )
	{
		assert( borderEdge != NULL );

		TerrainPolygon tp( NULL );
		Edge *curr = borderEdge;
		
		tp.AddPoint( new TerrainPoint( Vector2i( curr->v0.x, curr->v0.y ), false ) );

		curr = curr->edge1;

		while( curr != borderEdge )
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
	for( list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it )
	{
		for( list<Zone*>::iterator it2 = zones.begin(); it2 != zones.end(); ++it2 )
		{
			if( (*it) == (*it2) ) 
				continue;

			if( (*it)->ContainsZone( (*it2) ) )
			{
				cout << "contains zone!" << endl;
				(*it)->subZones.push_back( (*it2) );
			}
		}
	}

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
					(*it)->zone = (*zit);
				}
			}
		}

		/*if( (*it)->type == Enemy::BOSS_BIRD )
		{
			cout << "whats this" << endl;
			if( (*it)->zone != NULL )
			{
				(*it)->zone->spawnEnemies.push_back( (*it) );
			}
			
		}*/
		//else
		//{
		if( (*it)->zone != NULL )
			(*it)->zone->allEnemies.push_back( (*it) );
		//}


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

	if( originalZone != NULL )
	{
		cout << "setting original zone to active: " << originalZone << endl;
		//originalZone->active = true;
		ActivateZone(originalZone, true);
		//currentZone = originalZone;
		//keyMarker->SetStartKeysZone(currentZone);
	}
	
	cout << "3: numgates: " << numGates << endl;
	cout << "num zones: " << zones.size() << endl;
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

	//cout << "init zones over" << endl;

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
	return mainMenu->GetPrevInput(index);
}

ControllerState &GameSession::GetCurrInputUnfiltered(int index)
{
	return mainMenu->GetCurrInput(index);
}

Actor *GameSession::GetPlayer( int index )
{
	return players[index];
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
	bool up = Keyboard::isKeyPressed( Keyboard::Up );// || Keyboard::isKeyPressed( Keyboard::W );
	bool down = Keyboard::isKeyPressed( Keyboard::Down );// || Keyboard::isKeyPressed( Keyboard::S );
	bool left = Keyboard::isKeyPressed( Keyboard::Left );// || Keyboard::isKeyPressed( Keyboard::A );
	bool right = Keyboard::isKeyPressed( Keyboard::Right );// || Keyboard::isKeyPressed( Keyboard::D );

//	bool altUp = Keyboard::isKeyPressed( Keyboard::U );
//	bool altLeft = Keyboard::isKeyPressed( Keyboard::H );
//	bool altRight = Keyboard::isKeyPressed( Keyboard::K );
//	bool altDown = Keyboard::isKeyPressed( Keyboard::J );

	ControllerState keyboardInput;    
	keyboardInput.B = Keyboard::isKeyPressed( Keyboard::X );// || Keyboard::isKeyPressed( Keyboard::Period );
	keyboardInput.rightShoulder = Keyboard::isKeyPressed( Keyboard::C );// || Keyboard::isKeyPressed( Keyboard::Comma );
	keyboardInput.Y = Keyboard::isKeyPressed( Keyboard::D );// || Keyboard::isKeyPressed( Keyboard::M );
	keyboardInput.A = Keyboard::isKeyPressed( Keyboard::Z ) || Keyboard::isKeyPressed( Keyboard::Space );// || Keyboard::isKeyPressed( Keyboard::Slash );
	//keyboardInput.leftTrigger = 255 * (Keyboard::isKeyPressed( Keyboard::F ) || Keyboard::isKeyPressed( Keyboard::L ));
	keyboardInput.leftShoulder = Keyboard::isKeyPressed( Keyboard::LShift );
	keyboardInput.X = Keyboard::isKeyPressed( Keyboard::F );
	keyboardInput.start = Keyboard::isKeyPressed( Keyboard::J );
	keyboardInput.back = Keyboard::isKeyPressed( Keyboard::H );
	keyboardInput.rightTrigger = 255 * Keyboard::isKeyPressed( Keyboard::LControl );
	keyboardInput.leftTrigger = 255 * Keyboard::isKeyPressed( Keyboard::RControl );
			
	keyboardInput.rightStickPad = 0;
	if( Keyboard::isKeyPressed( Keyboard::A ) )
	{
		keyboardInput.rightStickPad += 1 << 1;
	}
	else if( Keyboard::isKeyPressed( Keyboard::S ) )
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
	cout << "start load" << endl;
	hitboxManager = new HitboxManager;

	momentumBar = new MomentumBar(this);
	momentumBar->SetTopLeft(Vector2f(202, 117));

	if( progressDisplay != NULL )
		progressDisplay->SetProgressString("started loading!", 0);

	if (!ShouldContinueLoading())
	{
		cout << "cleanup 0" << endl;
		Cleanup();
		return false;
	}
	
	//inputVis = new InputVisualizer;

	

	eHitParamsMan = new EnemyParamsManager;

	absorbParticles = new AbsorbParticles( this, AbsorbParticles::ENERGY );
	absorbDarkParticles = new AbsorbParticles( this, AbsorbParticles::DARK);
	absorbShardParticles = new AbsorbParticles(this, AbsorbParticles::SHARD);

	soundNodeList = new SoundNodeList(10);
	soundNodeList->SetGlobalVolume(mainMenu->config->GetData().soundVolume);
	scoreDisplay = new ScoreDisplay(this, Vector2f(1920, 0), mainMenu->arial);

	lifeBarTex.loadFromFile("lifebar_768x768.png");
	lifeBarSprite.setTexture(lifeBarTex);
	lifeBarSprite.setPosition(30, 200);

	if (!minimapShader.loadFromFile("Shader/minimap_shader.frag", sf::Shader::Fragment))
	{
		cout << "minimap SHADER NOT LOADING CORRECTLY" << endl;
		assert(0 && "minimap shader not loaded");
	}
	minimapShader.setUniform("imageSize", Vector2f(minimapTex->getSize().x,
		minimapTex->getSize().y));

	minimapSprite.setTexture(minimapTex->getTexture());
	minimapSprite.setOrigin(minimapSprite.getLocalBounds().width / 2,
		minimapSprite.getLocalBounds().height / 2);
	minimapSprite.setPosition(Vector2f(200, preScreenTex->getSize().y - 200));
	minimapSprite.setScale(1, -1);

	if (!speedBarShader.loadFromFile("Shader/speedbar_shader.frag", sf::Shader::Fragment))
	{
		cout << "speed bar SHADER NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}
	speedBarShader.setUniform("u_texture", sf::Shader::CurrentTexture);

	if (!glowShader.loadFromFile("Shader/glow_shader.frag", sf::Shader::Fragment))
	{
		cout << "glow SHADER NOT LOADING CORRECTLY" << endl;
	}
	glowShader.setUniform("texSize", Vector2f(1920, 1080));

	if (!hBlurShader.loadFromFile("Shader/hblur_shader.frag", sf::Shader::Fragment))
	{
		cout << "hBlurShader SHADER NOT LOADING CORRECTLY" << endl;
	}
	hBlurShader.setUniform("texSize", Vector2f(1920 / 2, 1080 / 2));

	if (!vBlurShader.loadFromFile("Shader/vblur_shader.frag", sf::Shader::Fragment))
	{
		cout << "vBlurShader SHADER NOT LOADING CORRECTLY" << endl;
	}
	vBlurShader.setUniform("texSize", Vector2f(1920 / 2, 1080 / 2));

	if (!motionBlurShader.loadFromFile("Shader/motionblur_shader.frag", sf::Shader::Fragment))
	{
		cout << "motion blur SHADER NOT LOADING CORRECTLY" << endl;
	}
	motionBlurShader.setUniform("texSize", Vector2f(1920, 1080));

	if (!shockwaveShader.loadFromFile("Shader/shockwave_shader.frag", sf::Shader::Fragment))
	{
		cout << "shockwave SHADER NOT LOADING CORRECTLY" << endl;
	}
	shockwaveShader.setUniform("resolution", Vector2f(1920, 1080));
	shockwaveShader.setUniform("texSize", Vector2f(580, 580));
	shockwaveTex.loadFromFile("shockwave_580x580.png");
	shockwaveShader.setUniform("shockwaveTex", shockwaveTex);

	ts_keyHolder = GetTileset("keyholder.png", 115, 40);
	keyHolderSprite.setTexture(*ts_keyHolder->texture);
	keyHolderSprite.setPosition(10, 50);

	if (!onTopShader.loadFromFile("Shader/ontop_shader.frag", sf::Shader::Fragment))
	{
		cout << "on top SHADER NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}

	if (!flowShader.loadFromFile("Shader/flow_shader.frag", sf::Shader::Fragment))
	{
		cout << "flow SHADER NOT LOADING CORRECTLY" << endl;
	}

	flowShader.setUniform("radDiff", radDiff);
	flowShader.setUniform("Resolution", Vector2f(1920, 1080));// window->getSize().x, window->getSize().y);
	flowShader.setUniform("flowSpacing", flowSpacing);
	flowShader.setUniform("maxFlowRings", maxFlowRadius / maxFlowRings);

	if (!mountainShader.loadFromFile("Shader/mountain_shader.frag", sf::Shader::Fragment))
	{
		cout << "mountain SHADER NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}

	mountainShader.setUniform("u_texture", *GetTileset("w1mountains.png", 1920, 512)->texture);

	if (!mountainShader1.loadFromFile("Shader/mountain_shader.frag", sf::Shader::Fragment))
	{
		cout << "mountain SHADER 1 NOT LOADING CORRECTLY" << endl;
		//assert( 0 && "polygon shader not loaded" );
	}

	mountainShader1.setUniform("u_texture", *GetTileset("w1mountains2.png", 1920, 406)->texture);

	onTopShader.setUniform("u_texture", *GetTileset("w1undertrans.png", 1920, 540)->texture);

	if (!underShader.loadFromFile("Shader/under_shader.frag", sf::Shader::Fragment))
	{
		cout << "under shader not loading correctly!" << endl;
		assert(false);
	}

	if (!cloneShader.loadFromFile("Shader/clone_shader.frag", sf::Shader::Fragment))
	{
		cout << "CLONE SHADER NOT LOADING CORRECTLY" << endl;
	}

	stringstream ss;

	for (int i = 1; i <= 17; ++i)
	{
		ss << i;
		string texName = "deathbg" + ss.str() + ".png";
		ss.str("");
		ss.clear();
		wipeTextures[i - 1].loadFromFile(texName);
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
	//testTree = new EdgeLeafNode( V2d( 0, 0), 1000000, 1000000);
	//testTree->parent = NULL;
	//testTree->debug = rw;

	enemyTree = new QuadTree(1000000, 1000000);

	borderTree = new QuadTree(1000000, 1000000);

	grassTree = new QuadTree(1000000, 1000000);

	lightTree = new QuadTree(1000000, 1000000);

	gateTree = new QuadTree(1000000, 1000000);

	itemTree = new QuadTree(1000000, 1000000);

	envPlantTree = new QuadTree(1000000, 1000000);

	specterTree = new QuadTree(1000000, 1000000);

	activeItemTree = new QuadTree(1000000, 1000000);

	cout << "weird timing 1" << endl;

	//sets up fx so that they can be used
	for (int i = 0; i < MAX_EFFECTS; ++i)
	{
		AllocateEffect();
	}

	for (int i = 0; i < MAX_DYN_LIGHTS; ++i)
	{
		AllocateLight();
	}

	ts_miniIcons = GetTileset("minimap_icons_64x64.png", 64, 64);
	kinMinimapIcon.setTexture(*ts_miniIcons->texture);
	kinMinimapIcon.setTextureRect(ts_miniIcons->GetSubRect(0));
	kinMinimapIcon.setOrigin(kinMinimapIcon.getLocalBounds().width / 2,
		kinMinimapIcon.getLocalBounds().height / 2);
	kinMinimapIcon.setPosition(minimapSprite.getPosition());//180, preScreenTex->getSize().y - 180 );


	kinMapSpawnIcon.setTexture(*ts_miniIcons->texture);
	kinMapSpawnIcon.setTextureRect(ts_miniIcons->GetSubRect(1));
	kinMapSpawnIcon.setOrigin(kinMapSpawnIcon.getLocalBounds().width / 2,
		kinMapSpawnIcon.getLocalBounds().height / 2);


	goalMapIcon.setTexture(*ts_miniIcons->texture);
	goalMapIcon.setTextureRect(ts_miniIcons->GetSubRect(0));
	goalMapIcon.setOrigin(goalMapIcon.getLocalBounds().width / 2,
		goalMapIcon.getLocalBounds().height / 2);

	ts_miniCircle = GetTileset("minimap_circle_320x320.png", 320, 320);
	miniCircle.setTexture(*ts_miniCircle->texture);
	miniCircle.setScale(2, 2);
	miniCircle.setOrigin(miniCircle.getLocalBounds().width / 2, miniCircle.getLocalBounds().height / 2);


	miniCircle.setPosition(kinMinimapIcon.getPosition());

	ts_minimapGateDirection = GetTileset("minimap_gate_w02_192x64.png", 192, 64);
	for (int i = 0; i < 6; ++i)
	{
		Sprite &gds = gateDirections[i];
		gds.setTexture(*ts_minimapGateDirection->texture);
		gds.setTextureRect(ts_minimapGateDirection->GetSubRect(0));
		gds.setOrigin(gds.getLocalBounds().width / 2, 120 + gds.getLocalBounds().height);
		gds.setPosition(miniCircle.getPosition());
	}

	ts_testParallax = GetTileset("parallax_w2_01.png", 960, 540);
	testParallaxSprite.setTexture(*ts_testParallax->texture);
	testParallaxSprite.setOrigin(testParallaxSprite.getLocalBounds().width / 2,
		testParallaxSprite.getLocalBounds().height / 2);

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

	soundManager = new SoundManager;

	gameSoundBuffers[S_KEY_COMPLETE_W1] = soundManager->GetSound( "Audio/Sounds/key_complete_w1.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W2] = soundManager->GetSound( "Audio/Sounds/key_complete_w2.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W3] = soundManager->GetSound( "Audio/Sounds/key_complete_w2.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W4] = soundManager->GetSound( "Audio/Sounds/key_complete_w2.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W5] = soundManager->GetSound( "Audio/Sounds/key_complete_w2.ogg" );
	gameSoundBuffers[S_KEY_COMPLETE_W6] = soundManager->GetSound( "Audio/Sounds/key_complete_w6.ogg" );
	gameSoundBuffers[S_KEY_ENTER_0] = soundManager->GetSound( "Audio/Sounds/key_enter_1.ogg" );
	gameSoundBuffers[S_KEY_ENTER_1] = soundManager->GetSound( "Audio/Sounds/key_enter_1.ogg" );
	gameSoundBuffers[S_KEY_ENTER_2] = soundManager->GetSound( "Audio/Sounds/key_enter_2.ogg" );
	gameSoundBuffers[S_KEY_ENTER_3] = soundManager->GetSound( "Audio/Sounds/key_enter_3.ogg" );
	gameSoundBuffers[S_KEY_ENTER_4] = soundManager->GetSound( "Audio/Sounds/key_enter_4.ogg" );
	gameSoundBuffers[S_KEY_ENTER_5] = soundManager->GetSound( "Audio/Sounds/key_enter_5.ogg" );
	gameSoundBuffers[S_KEY_ENTER_6] = soundManager->GetSound( "Audio/Sounds/key_enter_6.ogg" );

	

	currMusic = NULL;
	cutPlayerInput = false;
	activeEnvPlants = NULL;
	totalGameFrames = 0;	
	originalZone = NULL;
	
	inactiveEnemyList = NULL;
	cloneInactiveEnemyList = NULL;
	unlockedGateList = NULL;
	activatedZoneList = NULL;

	ts_leftHUD = GetTileset( "lefthud_560x1080.png", 560, 1080 );
	ts_speedBar = GetTileset("momentumbar_115x115.png", 115, 115);
	ts_speedBar = GetTileset( "momentumbar_560x210.png", 560, 210 );
	speedBarSprite.setTexture( *ts_speedBar->texture );

	leftHUDSprite.setTexture( *ts_leftHUD->texture );
	leftHUDBlankSprite.setTexture( *ts_leftHUD->texture );
	leftHUDBlankSprite.setTextureRect( ts_leftHUD->GetSubRect( 0 ) );
	leftHUDSprite.setTextureRect( ts_leftHUD->GetSubRect( 1 ) );

	cloudTileset = GetTileset( "cloud01.png", 1920, 1080 );
	sf::Texture &mountain01Tex = *GetTileset( "mountain01.png", 1920, 1080 / 2 /*540*/ )->texture;

	sf::Texture &underTrans01Tex = *GetTileset( "undertrans01.png", 1920, 650 / 2 )->texture;

	SetupClouds();
	
	undergroundTileset = GetTileset( "terrainworld1.png", 128, 128 );//GetTileset( "underground01.png", 32, 32 );
	undergroundTilesetNormal = GetTileset( "terrainworld1_NORMALS.png", 128, 128 );

	undergroundPar[0].position = Vector2f( 0, 0 );
	undergroundPar[1].position = Vector2f( 0, 0 );
	undergroundPar[2].position = Vector2f( 0, 0 );
	undergroundPar[3].position = Vector2f( 0, 0 );

	activeSequence = NULL;

	window->setMouseCursorVisible( true );

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

	background = new Background( this, mh->envLevel, mh->envType);

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

	

	

	if( mh->gameMode == MapHeader::MapType::T_STANDARD )
	{ 
		recGhost = new RecordGhost(GetPlayer(0));
	}
	
	pauseMenu = mainMenu->pauseMenu;
	pauseMenu->owner = this;
	pauseMenu->SetTab(PauseMenu::PAUSE);
	//pauseMenu->SetTab( PauseMenu::Tab::KIN );

	for( int i = 0; i < 4; ++i )
	{
		//temporary
		//mainMenu->GetController(i).SetFilter( pauseMenu->cOptions->xboxInputAssoc[0] );
		GetController(i).SetFilter( mainMenu->cpm->profiles.front()->filter );
	}

	goalPulse = new GoalPulse( this, Vector2f( goalPos.x, goalPos.y ) );

	int goalTile = -1;
	switch(mh->envType )
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
	goalMapIcon.setTextureRect( ts_miniIcons->GetSubRect( goalTile ) );

	float numSecondsToDrain = mh->drainSeconds;
	float totalHealth = 3600;
	float drainPerSecond = totalHealth / numSecondsToDrain;
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

	if (mh->gameMode == MapHeader::MapType::T_STANDARD)
	{
		FillRingSection *blah[] = {
			//new FillRingSection(tm, Color::Black, sf::Color::Black, sf::Color::Black,0, 300, 0),
			new FillRingSection(tm, Color::Cyan, sf::Color::Cyan, sf::Color::Cyan,1, totalHealth/3, 0),
			new FillRingSection(tm, Color::Cyan, sf::Color::Cyan, sf::Color::Cyan,2, totalHealth / 3, 0),
			new FillRingSection(tm, Color::Cyan, sf::Color::Cyan, sf::Color::Cyan,3, totalHealth / 3, 0)
		};

		Vector2f powerRingPos(80, 220);
		powerRing = new PowerRing(powerRingPos, sizeof(blah) / sizeof(FillRingSection*), blah);
		despOrb = new DesperationOrb(tm, powerRingPos);
	}

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

	ts_polyShaders = new Tileset*[numPolyTypes];

	cout << "progress more" << endl;

	map<pair<int,int>, int> indexConvert;
	int index = 0;
	for( set<pair<int,int>>::iterator it = matSet.begin(); it != matSet.end(); ++it )
	{
		if (!polyShaders[index].loadFromFile("Shader/mat_shader2.frag", sf::Shader::Fragment ) )
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
		ss1 << "terrain_";
		
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
		//polyShaders[index].setUniform( "u_normals", *undergroundTilesetNormal->texture );
		
		++index;
	}

	for( list<TestVA*>::iterator it = allVA.begin(); it != allVA.end(); ++it )
	{
		int realIndex = indexConvert[pair<int,int>((*it)->terrainWorldType,
		(*it)->terrainVariation)];
		//cout << "real index: " << realIndex << endl;
		(*it)->pShader = &polyShaders[realIndex];
		(*it)->ts_terrain = ts_polyShaders[realIndex];
	}

	LevelSpecifics();

	stringstream pss;
	pss << "Parallax/w" << (mh->envType + 1) << "_0" << (mh->envLevel + 1);
	string pStr = pss.str();
	string eStr = ".png";
	string cloudStr = "_cloud_";
	
	Tileset *cloud1 = GetTileset(pStr + cloudStr + string("1") + eStr, 1920, 1080);
	Tileset *cloud2 = GetTileset(pStr + cloudStr + string("2") + eStr, 1920, 1080);

	scrollingBackgrounds.push_back( 
		new ScrollingBackground( 
		GetTileset( pStr + string( "c" ) + eStr, 1920, 1080 ), 0, 3 ) );
	if (cloud1 != NULL)
	{
		scrollingBackgrounds.push_back(
			new ScrollingBackground(cloud1, 0, 4, 10));
	}
	
	scrollingBackgrounds.push_back( 
		new ScrollingBackground( 
		GetTileset(pStr + string("b") + eStr, 1920, 1080 ), 0, 5 ) );
	if (cloud2 != NULL)
	{
		scrollingBackgrounds.push_back(
			new ScrollingBackground(cloud2, 0, 8, 10));
	}
	
	scrollingBackgrounds.push_back( 
		new ScrollingBackground( 
		GetTileset(pStr + string("a") + eStr, 1920, 1080 ), 0, 10 ) );


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


int GameSession::Run()
{
	//ShaderTester shaderTester(ShaderTester::FIRE, this);
	
	//Tileset *ts_perlin = GetTileset("Shader/perlin01.png", 400, 400);
	//Tileset *ts_grad = GetTileset("Shader/gradient01.png", 400, 400);
	//sf::Shader fireShader;
	//if (!fireShader.loadFromFile("Shader/fire.frag", sf::Shader::Fragment))
	//{
	//	assert(0);
	//}
	//fireShader.setUniform("u_noiseTex", *ts_perlin->texture);
	//fireShader.setUniform("u_gradTex", *ts_grad->texture);

	//sf::Vertex fireV[4];
	//fireV.
	/*while (true)
	{
		window->clear(Color::Green);
		window->display();
		continue;
	}*/

	//rain = new Rain(this);

	preScreenTex->setView(view);
	
	bool showFrameRate = true;

	sf::Text frameRate("00", mainMenu->arial, 30);
	frameRate.setFillColor(Color::Red);

	sf::Texture alphaTex;
	alphaTex.loadFromFile("alphatext.png");
	Sprite alphaTextSprite(alphaTex);

	sf::RectangleShape bDraw;
	bDraw.setFillColor(Color::Red);
	bDraw.setSize(sf::Vector2f(32 * 2, 32 * 2));
	bDraw.setOrigin(bDraw.getLocalBounds().width / 2, bDraw.getLocalBounds().height / 2);
	bool bdrawdraw = false;

	Actor *p0 = GetPlayer(0);
	Actor *p = NULL;
	//use player->setactivepowers to set it up from the level. need to add it
	//to the editor

	//sf::Vertex *line = new sf::Vertex[mh->numVertices * 2];
	//for (int i = 0; i < mh->numVertices; ++i)
	//{
	//	//cout << "i: " << i << endl;
	//	line[i * 2] = sf::Vertex(Vector2f(edges[i]->v0.x, edges[i]->v0.y));
	//	line[i * 2 + 1] = sf::Vertex(Vector2f(edges[i]->v1.x, edges[i]->v1.y));
	//}

	//sf::Vector2<double> nLine((line[1].position - line[0].position).x, (line[1].position - line[0].position).y);
	//nLine = normalize(nLine);

	//sf::Vector2<double> lineNormal(-nLine.y, nLine.x);

	sf::CircleShape circle(30);
	circle.setFillColor(Color::Blue);


	//sf::Clock inGameClock;

	sf::Clock gameClock;
	double currentTime = 0;
	double accumulator = TIMESTEP + .1;

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

	for (int i = 0; i < 4; ++i)
	{
		GetController(i).UpdateState();
		GetCurrInput(i) = GetController(i).GetState();
	}

	bool t = GetCurrInput(0).start;//sf::Keyboard::isKeyPressed( sf::Keyboard::Y );
	bool s = t;
	t = false;

	int returnVal = GR_EXITLEVEL;

	Texture & borderTex = *GetTileset("borders.png", 16, 16)->texture;

	//Texture & grassTex = *GetTileset("placeholdergrass_22x22.png", 22, 22)->texture;
	ts_gravityGrass = GetTileset("placeholdergrass_22x22.png", 22, 22);
	

	goalDestroyed = false;


	debugScreenRecorder = NULL;

	//debugScreenRecorder = new ScreenRecorder("BACKWARDS_DASH_JUMP");
	
	//#define SYNCEDDRAWWITHPHYSICS

	//lights.push_back( new Light( this ) );

	View v;
	v.setCenter(0, 0);
	v.setSize(1920 / 2, 1080 / 2);
	window->setView(v);
	lastFrameTex->setView(v);

	//stringstream ss;
	//ss.clear(); //needed?

	
	int frameCounter = 0;
	int frameCounterWait = 20;
	double total = 0;

	cloudView = View(Vector2f(0, 0), Vector2f(1920, 1080));

	int flowSize = 64;
	//GPUFlow *f = new GPUFlow( Vector2i( player->position.x + 100, player->position.y ), flowSize, flowSize ); 
	//Flow *f = new Flow( Vector2i( player->position.x + 100, player->position.y ), flowSize, flowSize );
	//f->player = &player;

	//Cutscene cut( this, Vector2i( player->position.x, player->position.y ) );
	//cut.LoadFromFile( "gametest" );
	//int cutFrame = 0;


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

	//scrollingTest = new ScrollingBackground( GetTileset( "Parallax/w1_01a.png", 1920, 1080 ), 0, 10 );

	//Tileset *ts_blah = GetTileset( "Parallax/w2_tree_01_1920x1080.png", 1920, 1080 );
	//Tileset *ts_cloud0 = GetTileset( "Parallax/w1_cloud_01_1920x1080.png", 1920, 1080 );
	//Tileset *ts_cloud1 = GetTileset( "Parallax/w1_cloud_02_1920x1080.png", 1920, 1080 );
	//Tileset *ts_cloud2 = GetTileset( "Parallax/w1_cloud_03_1920x1080.png", 1920, 1080 );




	//testPar->AddRepeatingSprite( ts_blah, 0, Vector2f( 0, 0 ), 1920 * 2, 10 );
	//testPar->AddRepeatingSprite( ts_blah, 0, Vector2f( 1920, 0 ), 1920 * 2, 10 );
	//testPar->AddRepeatingSprite( ts_cloud0, 0, Vector2f( 0, 0 ), 1920 * 2, 50 );
	//testPar->AddRepeatingSprite( ts_cloud1, 0, Vector2f( 500, -200 ), 1920 * 2, 30 );
	//testPar->AddRepeatingSprite( ts_cloud2, 0, Vector2f( 1500, 300 ), 1920 * 2, 25 );

	/*Tileset *ts_mountain0 = GetTileset( "Parallax/parallax_w1_mountain_01.png", 1920, 1080 );
	Tileset *ts_mountain1 = GetTileset( "Parallax/parallax_w1_mountain_02.png", 1920, 1080 );
	Tileset *ts_mountain2 = GetTileset( "Parallax/parallax_w1_mountain_03.png", 1920, 1080 );
	Tileset *ts_mountain3 = GetTileset( "Parallax/parallax_w1_mountain_04.png", 1920, 1080 );
	Tileset *ts_mountain4 = GetTileset( "Parallax/parallax_w1_mountain_05.png", 1920, 1080 );
	Tileset *ts_mountain5 = GetTileset( "Parallax/parallax_w1_mountain_06.png", 1920, 1080 );*/


	/*Tileset *ts_mountain0 = GetTileset( "Parallax/parallax_w2_tree_01.png", 1920, 1080 );
	Tileset *ts_mountain1 = GetTileset( "Parallax/parallax_w2_tree_02.png", 1920, 1080 );
	Tileset *ts_mountain2 = GetTileset( "Parallax/parallax_w2_tree_03.png", 1920, 1080 );
	Tileset *ts_mountain3 = GetTileset( "Parallax/parallax_w2_tree_04.png", 1920, 1080 );
	Tileset *ts_mountain4 = GetTileset( "Parallax/parallax_w2_tree_05.png", 1920, 1080 );
	Tileset *ts_mountain5 = GetTileset( "Parallax/parallax_w2_tree_06.png", 1920, 1080 );
	testPar->AddRepeatingSprite( ts_mountain5, 0, Vector2f( 0, 0 ), 1920 * 2, 5 );
	testPar->AddRepeatingSprite( ts_mountain5, 0, Vector2f( 1920, 0 ), 1920 * 2, 5 );
	testPar->AddRepeatingSprite( ts_mountain4, 0, Vector2f( 0, 0 ), 1920 * 2, 10 );
	testPar->AddRepeatingSprite( ts_mountain4, 0, Vector2f( 1920, 0 ), 1920 * 2, 10 );
	testPar->AddRepeatingSprite( ts_mountain3, 0, Vector2f( 0, 0 ), 1920 * 2, 15 );
	testPar->AddRepeatingSprite( ts_mountain3, 0, Vector2f( 1920, 0 ), 1920 * 2, 15 );
	testPar->AddRepeatingSprite( ts_mountain2, 0, Vector2f( 0, 0 ), 1920 * 2, 20 );
	testPar->AddRepeatingSprite( ts_mountain2, 0, Vector2f( 1920, 0 ), 1920 * 2, 20 );
	testPar->AddRepeatingSprite( ts_mountain1, 0, Vector2f( 0, 0 ), 1920 * 2, 25 );
	testPar->AddRepeatingSprite( ts_mountain1, 0, Vector2f( 1920, 0 ), 1920 * 2, 25 );
	testPar->AddRepeatingSprite( ts_mountain0, 0, Vector2f( 0, 0 ), 1920 * 2, 30 );
	testPar->AddRepeatingSprite( ts_mountain0, 0, Vector2f( 1920, 0 ), 1920 * 2, 30 );*/

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

	int pointsTotal = 0;
	if( mh->songLevels.size() > 0 )
	{
		for (auto it = mh->songLevels.begin(); it != mh->songLevels.end(); ++it)
		{
			pointsTotal += (*it).second;
		}
	}


	//TODO : use a better random algorithm later
	srand(time(0));

	if (levelMusic == NULL && pointsTotal > 0)
	{
		int r = rand() % ( pointsTotal );

		for (auto it = mh->songLevels.begin(); it != mh->songLevels.end(); ++it)
		{
			r -= (*it).second;
			if (r < 0)
			{
				//assumes that this is a valid check
				if (mainMenu->musicManager->songMap.count((*it).first) == 0)
				{
					assert(0);
				}
				
				levelMusic = mainMenu->musicManager->songMap[(*it).first];
				if (levelMusic == NULL)
				{
					assert(0);
				}
				break;
			}
		}
	}
	if (levelMusic == NULL && pointsTotal > 0 )
	{
		assert(0);
	}
	else if( levelMusic != NULL )
	{
		levelMusic->Load();
		levelMusic->music->setVolume(mainMenu->config->GetData().musicVolume);
		levelMusic->music->setLoop(true);
		levelMusic->music->play();
	}


	std::stringstream ss;

	

	while( !quit )
	{
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
		
		
		coll.ClearDebug();
		
		while ( accumulator >= TIMESTEP )
        {
		//	cout << "currInputleft: " << currInput.leftShoulder << endl;
			bool skipInput = sf::Keyboard::isKeyPressed( sf::Keyboard::PageUp );
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

					for( int i = 0; i < 4; ++i )
					{
						GetController( i ).UpdateState();
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
					skipInput = sf::Keyboard::isKeyPressed( sf::Keyboard::PageUp );
					
					bool stopSkippingInput = sf::Keyboard::isKeyPressed( sf::Keyboard::PageDown );
					screenShot = false;//Keyboard::isKeyPressed( sf::Keyboard::F );// && !tookScreenShot;
					
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
					

					if( !skipped && skipInput )//sf::Keyboard::isKeyPressed( sf::Keyboard::K ) && !skipped )
					{
						skipped = true;
						accumulator = 0;//TIMESTEP;
						
						//currentTime = gameClock.getElapsedTime().asSeconds() - TIMESTEP;

						break;
					}
					if( skipped && !skipInput )//!sf::Keyboard::isKeyPressed( sf::Keyboard::K ) && skipped )
					{
						skipped = false;
						//break;
					}
					if( sf::Keyboard::isKeyPressed( sf::Keyboard::L ) )
					{

						//oneFrameMode = false;
						break;
					}
					//if( sf::Keyboard::isKeyPressed( sf::Keyboard::M ) )
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

			bool k = sf::Keyboard::isKeyPressed( sf::Keyboard::K );
			bool levelReset = sf::Keyboard::isKeyPressed( sf::Keyboard::L );
			Enemy *monitorList = NULL;
			//if( player->action != Actor::GOALKILLWAIT && player->action != Actor::GOALKILL && player->action != Actor::EXIT && ( k || levelReset || player->dead /*|| (currInput.start && !prevInput.start )*/ ) )
			{
				//levelReset = true;
				//RestartLevel();
			}

			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Y ) )// || currInput.start )
			{
				
				quit = true;
				break;
			}
	
			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Escape ) )
			{


				quit = true;
				returnVal = GR_EXITLEVEL;

				break;
			}

			if( goalDestroyed )
			{
				quit = true;
				
				//returnVal = GR_WIN;
				returnVal = resType;
				break;
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

			for( int i = 0; i < 4; ++i )
			{
				GameController &con = GetController( i );
				bool canControllerUpdate = con.UpdateState();
				if( !canControllerUpdate )
				{
					//KeyboardUpdate( 0 );
				}
				else
				{
					con.UpdateState();
					GetCurrInput( i ) = con.GetState();
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

				for (int i = 0; i < 4; ++i)
				{
					GameController &con = GetController(i);
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

				UpdateFade();
				//powerWheel->UpdateHide();

				//miniCircle.setPosition( 180 + ( powerWheel->basePos.x - powerWheel->origBasePos.x ), preScreenTex->getSize().y - 180 );
				miniCircle.setPosition(180, preScreenTex->getSize().y - 180);
				for( int i = 0; i < 6; ++i )
				{
					Sprite &gds = gateDirections[i];
					gds.setPosition( miniCircle.getPosition() );
				}

				Vector2f miniPos = Vector2f( 30, 750 );
				//miniPos.x += ( powerWheel->basePos.x - powerWheel->origBasePos.x );
				miniVA[0].position = miniPos + Vector2f( 0, 0 );
				miniVA[1].position = miniPos + Vector2f( 300, 0 );
				miniVA[2].position = miniPos + Vector2f( 300, 300 );
				miniVA[3].position = miniPos + Vector2f( 0, 300 );

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
				if( !activeSequence->Update() )
				{
					activeSequence = NULL;	
				}
				else
				{
					//cout << "player frame: " << player->frame << endl;
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

				//if( GetPlayer( 0 )->action != Actor::Action::SPAWNWAIT || GetPlayer( 0 )->frame > 20 )
				//	powerWheel->UpdateSections();

				//if (GetPlayer(0)->action != Actor::Action::SPAWNWAIT || GetPlayer(0)->frame > 20)
				if( powerRing != NULL )
					powerRing->Update();

				absorbParticles->Update();
				absorbDarkParticles->Update();
				absorbShardParticles->Update();

				UpdateEffects();

				keyMarker->Update();

				scoreDisplay->Update();

				soundNodeList->Update();

				goalPulse->Update();

				if (rain != NULL)
					rain->Update();

				
				//powerWheel->UpdateHide();


				//miniCircle.setPosition( 180 + ( powerWheel->basePos.x - powerWheel->origBasePos.x ), preScreenTex->getSize().y - 180 );
				miniCircle.setPosition(180, preScreenTex->getSize().y - 180);
				for( int i = 0; i < 6; ++i )
				{
					Sprite &gds = gateDirections[i];
					gds.setPosition( miniCircle.getPosition() );
				}

				Vector2f miniPos = Vector2f( 30, 750 );
				//miniPos.x += ( powerWheel->basePos.x - powerWheel->origBasePos.x );
				miniVA[0].position = miniPos + Vector2f( 0, 0 );
				miniVA[1].position = miniPos + Vector2f( 300, 0 );
				miniVA[2].position = miniPos + Vector2f( 300, 300 );
				miniVA[3].position = miniPos + Vector2f( 0, 300 );
				//kinMinimapIcon.setPosition

				

				//rainView.setCenter(

				

				oldZoom = cam.GetZoom();
				oldCamBotLeft = view.getCenter();
				oldCamBotLeft.x -= view.getSize().x / 2;
				oldCamBotLeft.y += view.getSize().y / 2;

				oldView = view;


				//polyShader.setUniform( "oldZoom", cam.GetZoom() );
				//polyShader.setUniform( "oldBotLeft", view.getCenter().x - view.getSize().x / 2, 
				//	view.getCenter().y + view.getSize().y / 2 );

				if( raceFight != NULL )
				{
					cam.UpdateVS( GetPlayer( 0 ), GetPlayer( 1 ) );
				}
				else
				{
					cam.Update( GetPlayer( 0 ) );
				}
				

				Vector2f camPos = cam.GetPos();

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

				UpdateFade();
				background->Update();
				if( topClouds != NULL )
					topClouds->Update();
				//rain.Update();

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
				//scrollingTest->Update( camPos );

				//Vector2f diff = cam.pos - oldCam;

				//cloudVel = Vector2f( -40, 0 );
				//cloud0a.move( cloudVel );
				//cloud0b.move( cloudVel );
				//cloud1a.move( Vector2f( -2, 0 ) );
				//cloud1b.move( Vector2f( -2, 0 ) );
				if( shipSequence )
				{
					


					float oldLeft = cloud0[0].position.x;
					//float oldLeft1 = cloud0[1].position.x;
					//float blah = std::max( 15.f, ( 30.f - relShipVel.x ) );
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

					float allDiff = newLeft - oldLeft;// - relShipVel.x;
					//cout << "all diff: " << allDiff << endl;
					//allDiff = .5;
					Vector2f cl = relShipVel;
					//cl.y = cl.y / 2;

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
						//cout << "tFrame: " << tFrame << endl;
						//CubicBezier b( 0, 0, 1, 1 );
						//double a = tFrame / 60.0;
						//double v = b.GetValue( a );
						shipSprite.setPosition( shipSprite.getPosition() + relShipVel );

						relShipVel += Vector2f( .3, -.8 );

						/*if( shipSeqFrame >= 100 )
						{
							relShipVel = Vector2f( 0, 0 );
						}
						else
						{
							relShipVel += Vector2f( .3, -.8 );
						}*/
						

						//V2d shipStart = V2d( shipStartPos.x, shipStartPos.y );
						//V2d stuff = shipStart * ( 1.0 - v ) 
						//	+ ( shipStart + V2d( 500, - 500 ) ) * v;
						//shipSprite.setPosition( stuff.x, stuff.y );


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
						//player->
						//player->rightWire->= false;
					}

					++shipSeqFrame;
				}
				/*if( shipStartPos.x - cloud1b.getPosition().x > 960 )
				{
					
					cout << "condition CLOUD B: " << shipStartPos.x - cloud1b.getPosition().x << endl;
					cloud1b.setPosition( shipStartPos.x + 480, cloud1b.getPosition().y );
				}
				else if( shipStartPos.x - cloud1a.getPosition().x > 960 )
				{
					cout << "condition CLOUD A: " << shipStartPos.x - cloud1a.getPosition().x << endl;
					cloud1a.setPosition( shipStartPos.x + 480, cloud1a.getPosition().y );
				}*/
				//cloud0a.setPosition( pi->pos.x - 480, pi->pos.y + 270 );
				//cloud0b.setPosition( pi->pos.x, pi->pos.y + 270 );
				
				//cloud1a.setPosition( pi->pos.x - 480, pi->pos.y + 270 );
				//cloud1b.setPosition( pi->pos.x, pi->pos.y + 270);// + 540 );


				double camWidth = 960 * cam.GetZoom();
				double camHeight = 540 * cam.GetZoom();
				
				screenRect = sf::Rect<double>( camPos.x - camWidth / 2, camPos.y - camHeight / 2, camWidth, camHeight );

				if (totalGameFrames % 60 == 0)
				{
					float ddepth = .7;
					//ActivateEffect(EffectLayer::IN_FRONT, GetTileset("hit_spack_1_128x128.png", 128, 128), V2d(camPos / ddepth), true, 0, 10, 10, true, ddepth);
				}

				//flowShader.setUniform( "radius0", flow
				
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
				
				int speedLevel = p0->speedLevel;
				//speedBarShader.setUniform( "onPortion", (float)speedLevel );
				//speedBarShader.setUniform( "quant", (float)currentSpeedBar );
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
				speedBarShader.setUniform( "onPortion", quant );

				queryMode = "enemy";

				tempSpawnRect = screenRect;
				enemyTree->Query( this, screenRect );

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
					TestVA *t = listVA->next;
					listVA->next = NULL;
					listVA = t;
				}

				//listVA is null here
				queryMode = "border";
				numBorders = 0;
				borderTree->Query( this, screenRect );

				drawInversePoly = ScreenIntersectsInversePoly( screenRect );

				
				TestVA *te = listVA;
				while( te != NULL )
				{
					te->tr->UpdateDecorSprites();
					//te->tr->UpdateDecorLayers();
					//te->UpdateBushSprites();
					te = te->next;
				}

				TerrainRender::UpdateDecorLayers();

				for( map<DecorType,DecorLayer*>::iterator mit =
					decorLayerMap.begin(); mit != decorLayerMap.end();
					++mit )
				{
					(*mit).second->Update();
				}
				//TestVA::UpdateBushFrame();

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
					//if( Keyboard::isKeyPressed( Keyboard ) )
					if( currInput.start && !prevInput.start )
					{
						state = PAUSE;
						pauseMenu->SetTab( PauseMenu::PAUSE );
						soundNodeList->Pause( true );


					}
					else if( ( currInput.back && !prevInput.back ) || Keyboard::isKeyPressed( Keyboard::G ) )
					{
						state = PAUSE;
						pauseMenu->SetTab( PauseMenu::MAP );
					
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

		//window->display();
		//continue;

		if( debugScreenRecorder != NULL )
		if( Keyboard::isKeyPressed( Keyboard::R ) )
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
		
		
		
		
		if( false )//activeSequence != NULL && activeSequence == startSeq )
		{
			//activeSequence->Draw( preScreenTex );
			

			preScreenTex->display();
			const Texture &preTex = preScreenTex->getTexture();
		
			Sprite preTexSprite( preTex );
			preTexSprite.setPosition( -960 / 2, -540 / 2 );
			//preTexSprite.setScale( .5, .5 );	
			window->draw( preTexSprite  );
			
		}
		else
		{

		Vector2f camPos = cam.GetPos();
	//	view.setSize( Vector2f( 1920 * cam.GetZoom(), 1080 * cam.GetZoom()) );
		view.setSize(Vector2f(1920/2 * cam.GetZoom(), 1080/2 * cam.GetZoom()));

		//this is because kin's sprite is 2x size in the game as well as other stuff

		//view.setSize( cut.cameras[cutFrame].getSize() );
		lastViewSize = view.getSize();

		//view.setCenter( player->position.x + camOffset.x, player->position.y + camOffset.y );
		view.setCenter( camPos.x, camPos.y );
		
		
		//view.setCenter( cut.cameras[cutFrame].getCenter() );



		//cout << "center: " << view.getCenter().x << ", " << view.getCenter().y << endl;
		//view = //cut.cameras[cutFrame];
		
		//
		//view.setCenter( cut.cameras[cutFrame].getCenter() );
		//cout << "view zoom: " << view.getSize().x << ", " << view.getSize().y << endl;

		//preScreenTex->setView( cut.GetView( cutFrame ) );
		lastViewCenter = view.getCenter();

		if (hasGoal)
		{
			flowShader.setUniform("topLeft", Vector2f(view.getCenter().x - view.getSize().x / 2,
				view.getCenter().y + view.getSize().y / 2));
		}
		

		background->Draw(preScreenTex);
		
		//window->setView( bgView );
		//preScreenTex->setView( bgView );

		//preScreenTex->draw(backgroundSky, 4, sf::Quads);
		//preScreenTex->draw( background );

		//temporary parallax
		View pView;
		float depth = .3;
		int px = floor( view.getCenter().x * depth + .5 );
		int pxx;
		
		if( px >= 0 )
		{
			pxx = (px % (1920 * 2)) - 1920;
			//cout << ">0: " << pxx << ", realx: " << px << endl;
		}
		else
		{
			pxx = -(-px % (1920 * 2)) + 1920;
			//cout << "<0: " << pxx << ", realx: " << px << endl;
		}
		pView.setCenter( Vector2f( pxx, 0 ) );
		pView.setSize( 1920, 1080 );
		//preScreenTex->setView( pView );
		//preScreenTex->draw( testParallaxSprite );
		preScreenTex->setView( view );

		//testPar->Draw( preScreenTex );
		for( list<ScrollingBackground*>::iterator it = scrollingBackgrounds.begin();
			it != scrollingBackgrounds.end(); ++it )
		{
			(*it)->Draw( preScreenTex );
		}
		//scrollingTest->Draw( preScreenTex );

		//window->draw( background );

		
		
		//window->setView( view );

		

		
		//cloudView.setSize( 1920, 1080 );
		cloudView.setCenter( 960, 540 );
		
		//preScreenTex->setView( cut.cameras[cutFrame] );
		

	//	SetParMountains( preScreenTex );

	//	SetParMountains1( preScreenTex );

	//	SetParOnTop( preScreenTex );

		//cavedepth
		//if( SetGroundPar() )
		{
		//	preScreenTex->draw( groundPar, &mountain01Tex );
			//preScreenTex->draw( underTransPar, &underTrans01Tex );
		}
	
		cloudView.setCenter( 960, 540 );	
		preScreenTex->setView( cloudView );
		
		//float depth = 3;
		//parTest.setPosition( orig / depth + ( cam.pos - orig ) / depth );
		//SetCloudParAndDraw();


		
		
		
		
		//cloudView.setCenter( 0, 0 );
		//preScreenTex->setView( cloudView );
		//preScreenTex->setView( cloudView );
		//SetUndergroundParAndDraw();

		
		//float scale = 1 + ( 1 - 1 / ( cam.GetZoom() * depth ) );
		//parTest.setScale( scale, scale );
		//preScreenTex->draw( parTest );
		
		preScreenTex->setView( view );
		
		bDraw.setSize( sf::Vector2f(p0->b.rw * 2, p0->b.rh * 2) );
		bDraw.setOrigin( bDraw.getLocalBounds().width /2, bDraw.getLocalBounds().height / 2 );
		bDraw.setPosition( p0->position.x + p0->b.offset.x , p0->position.y + p0->b.offset.y );
	//	bDraw.setRotation( player->sprite->getRotation() );
		if( bdrawdraw)
		{
			preScreenTex->draw( bDraw );
		}
		//window->draw( bDraw );

	/*	CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 10 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( player->position.x, player->position.y );
		window->draw( cs );*/

	
		//player->sh.setUniform( "u_texture", *GetTileset( "testrocks.png", 25, 25 )->texture );
		//player->sh.setUniform( "u_texture1", *GetTileset( "testrocksnormal.png", 25, 25 )->texture );
		
		
		
		
		
		


		//player->sprite->setTextureRect( IntRect( 0, 0, 300, 225 ) );
		//if( false )
		
		while( lightList != NULL )
		{
			Light *l = lightList->next;
			lightList->next = NULL;
			lightList = l;
		}

		queryMode = "lightdisplay";
		lightTree->Query( this, screenRect );

		Light *lightListIter = lightList;
		while( lightListIter != NULL )
		{
			lightListIter->Draw( preScreenTex );
			lightListIter = lightListIter->next;
		}

		
		
		
		
		sf::RectangleShape rs;
		rs.setSize( Vector2f(64, 64) );
		rs.setOrigin( rs.getLocalBounds().width / 2, rs.getLocalBounds().height / 2 );
		rs.setPosition( otherPlayerPos.x, otherPlayerPos.y  );
		rs.setFillColor( Color::Blue );
		//window->draw( circle );
		//window->draw(line, numPoints * 2, sf::Lines);
		
		//polyShader.setUniform( "u_texture", *GetTileset( "terrainworld1.png" , 128, 128 )->texture ); //*GetTileset( "testrocks.png", 25, 25 )->texture );
		//polyShader.setUniform( "u_normals", *GetTileset( "terrainworld1_NORMALS.png", 128, 128 )->texture );

		


		Vector2i vi = Mouse::getPosition();
		//Vector2i vi = window->mapCoordsToPixel( Vector2f( player->position.x, player->position.y ) );
		//Vector2i vi = window->mapCoordsToPixel( sf::Vector2f( 0, -300 ) );
		//vi -= Vector2i( view.getSize().x / 2, view.getSize().y / 2 );
		Vector3f blahblah( vi.x / 1920.f, (1080 - vi.y) / 1080.f, .015 );
		blahblah.y = 1 - blahblah.y;


		//polyShader.setUniform( "LightPos", blahblah );//Vector3f( 0, -300, .075 ) );
		//polyShader.setUniform( "LightColor", 1, .8, .6, 1 );
		
		//polyShader.setUniform( "Falloff", Vector3f( .4, 3, 20 ) );
		//cout << "window size: " << window->getSize().x << ", " << window->getSize().y << endl;


		Vector2f botLeft( view.getCenter().x - view.getSize().x / 2, 
			view.getCenter().y + view.getSize().y / 2 );

		Vector2f playertest = ( botLeft - oldCamBotLeft ) / 5.f;
		//cout << "test: " << playertest.x << ", " << playertest.y << endl;
		// window->getSize().x, window->getSize().y);
		
		for( int i = 0; i < numPolyTypes; ++i )
		{
			polyShaders[i].setUniform( "zoom", cam.GetZoom() );
			polyShaders[i].setUniform( "topLeft", botLeft ); //just need to change the name topleft eventually
			polyShaders[i].setUniform( "playertest", playertest );
			polyShaders[i].setUniform("skyColor", ColorGL(background->GetSkyColor()));
		}

		for (auto it = zones.begin(); it != zones.end(); ++it)
		{
			(*it)->Update(cam.GetZoom(), botLeft, playertest);
		}
		
		//polyShader.setUniform( "zoom", cam.GetZoom() );
		//polyShader.setUniform( "topLeft", view.getCenter().x - view.getSize().x / 2, 
		//	view.getCenter().y + view.getSize().y / 2 );

		
		//polyShader.setUniform( "u_texture", *GetTileset( "testterrain.png", 32, 32 )->texture );


		//polyShader.setUniform(  = GetTileset( "testterrain.png", 25, 25 )->texture;

		//for( list<VertexArray*>::iterator it = polygons.begin(); it != polygons.end(); ++it )
		//{
		//	if( usePolyShader )
		//	{
		//		

		//		UpdateTerrainShader();

		//		preScreenTex->draw( *(*it ), &polyShader);
		//	}
		//	else
		//	{
		//		preScreenTex->draw( *(*it ) );
		//	}
		//	//GetTileset( "testrocks.png", 25, 25 )->texture );
		//}
		
		

		sf::Rect<double> testRect( view.getCenter().x - view.getSize().x / 2, view.getCenter().y - view.getSize().y / 2,
			view.getSize().x, view.getSize().y );

		

		
		
		

		//screenRect = sf::Rect<double>( cam.pos.x - camWidth / 2, cam.pos.y - camHeight / 2, camWidth, camHeight );
		
		if( topClouds != NULL )
			topClouds->Draw(preScreenTex);

		preScreenTex->draw(blackBorderQuads, 8, sf::Quads);
		
		

		DrawEffects( EffectLayer::BEHIND_TERRAIN );
		
		int timesDraw = 0;
		TestVA * listVAIter = listVA;
		//listVAIter->next = NULL;

		UpdateTerrainShader( screenRect );

		if( drawInversePoly )
		{
			if( listVAIter == NULL )
			{
				listVAIter = inversePoly;
				inversePoly->next = NULL;
			}
			else
			{
				listVAIter->next = inversePoly;
				inversePoly->next = NULL;
			}
		}

		for (list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it)
		{
			(*it)->Draw(preScreenTex);
		}

		//draw terrain
		while( listVAIter != NULL )
		//for( int i = 0; i < numBorders; ++i )
		{
			if( listVAIter->grassVA != NULL )
				preScreenTex->draw(*listVAIter->grassVA , ts_gravityGrass->texture );

			if( usePolyShader )
			//if(false )
			{

				sf::Rect<double> polyAndScreen;
				sf::Rect<double> aabb = listVAIter->aabb;
				double rightScreen = screenRect.left + screenRect.width;
				double bottomScreen = screenRect.top + screenRect.height;
				double rightPoly = aabb.left + aabb.width;
				double bottomPoly = aabb.top + aabb.height;

				double left = std::max( screenRect.left, aabb.left );

				double right = std::min( rightPoly, rightScreen );
				
				double top = std::max( screenRect.top, aabb.top );

				double bottom = std::min( bottomScreen, bottomPoly );


				polyAndScreen.left = left;
				polyAndScreen.top = top;
				polyAndScreen.width = right - left;
				polyAndScreen.height = bottom - top;
			
				assert( listVAIter->pShader != NULL );
				preScreenTex->draw( *listVAIter->terrainVA, listVAIter->pShader );// listVAIter->ts_terrain->texture );//listVAIter->pShader );//listVAIter->pShader );
			}
			else
			{
				preScreenTex->draw( *listVAIter->terrainVA );
			}
			//cout << "drawing border" << endl;
			//preScreenTex->draw( *listVAIter->va );



			sf::RenderStates rs;
			rs.texture = listVAIter->ts_border->texture;

			if( showTerrainDecor )
			{
			/*if( listVAIter->triva != NULL )
				preScreenTex->draw( *listVAIter->triva, rs );
			if(listVAIter->wallva != NULL )
				preScreenTex->draw(*listVAIter->wallva, rs);


			if (listVAIter->steepva)
			{
				preScreenTex->draw(*listVAIter->steepva, rs);
			}
			
			if (listVAIter->slopeva)
			{
				preScreenTex->draw(*listVAIter->slopeva, rs);
			}
			
			if (listVAIter->groundva)
			{
				preScreenTex->draw(*listVAIter->groundva, rs);
			}*/
				//listVAIter->tr->UpdateDecor();
				listVAIter->tr->Draw(preScreenTex);
			

			//preScreenTex->setSmooth( false );
			//listVAIter->DrawBushes( preScreenTex );
			/*if( listVAIter->bushVA != NULL )
			{
				RenderStates bushRS;
				bushRS.texture = listVAIter->ts_bush->texture;

				preScreenTex->draw( *listVAIter->bushVA, bushRS );
			}*/

			if( listVAIter->plantva != NULL )
			{
				//rs.texture = listVAIter->ts_plant->texture;
				//preScreenTex->draw( *listVAIter->plantva, rs );
			}

			}

		/*	if( listVAIter->flowva != NULL )
			{
				preScreenTex->draw( *listVAIter->flowva );
			}*/
			//preScreenTex->draw( *listVAIter->va );
			listVAIter = listVAIter->next;
			timesDraw++; 
		}
		
		if (hasGoal)
		{
			preScreenTex->draw(*goalEnergyFlowVA, &flowShader);
		}
		

		
		//motion blur
		if( false )
		{
			preScreenTex->display();
			//for motion blur


			preScreenTex->setView( uiView );
			sf::RectangleShape rectPost( Vector2f( 1920, 1080 ) );
			rectPost.setPosition( 0, 0 );
			//Vector2f camVel = cam.pos - oldCamCenter;
			
			Vector2f botLeft = Vector2f( view.getCenter().x - view.getSize().x / 2, 
				view.getCenter().y + view.getSize().y / 2 );
			

			motionBlurShader.setUniform( "tex", preScreenTex->getTexture() );
			/*motionBlurShader.setUniform( "oldBotLeft", oldCamBotLeft );
			motionBlurShader.setUniform( "botLeft", botLeft );
			motionBlurShader.setUniform( "oldZoom", oldZoom );
			motionBlurShader.setUniform( "zoom", cam.GetZoom() );*/
			//negative player y because of bottom left origin

			V2d t = p0->velocity;
			double maxBlur = 8;
			t.x = t.x / p0->maxGroundSpeed * maxBlur;
			t.y = t.y / p0->maxGroundSpeed * maxBlur;

			
			Vector2f testVel = Vector2f( p0->velocity.x, -p0->velocity.y );

			motionBlurShader.setUniform( "testVel", Vector2f( t.x, t.y ) );

			motionBlurShader.setUniform( "g_ViewProjectionInverseMatrix", view.getTransform().getInverse().getMatrix() );
			motionBlurShader.setUniform( "g_previousViewProjectionMatrix", oldView.getTransform().getMatrix() );

			
			postProcessTex->draw( rectPost, &motionBlurShader );

			postProcessTex->display();

			sf::Sprite pptSpr;
			pptSpr.setTexture( postProcessTex->getTexture() );
			//pptSpr.setScale( 2, 2 );
			//RenderStates blahRender;
			//blahRender.blendMode = sf::BlendAdd;//sf::BlendAdd;

			preScreenTex->draw( pptSpr );
			//postProcessTex->display();
			preScreenTex->setView( view );
		}

		

		testGateCount = 0;
		queryMode = "gate";
		gateList = NULL;
		gateTree->Query( this, screenRect );

		while( gateList != NULL )
		{
			gateList->Draw( preScreenTex );
			Gate *next = gateList->next;//(Gate*)gateList->edgeA->edge1;
			gateList = next;
		}

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

		DrawEffects( EffectLayer::BEHIND_ENEMIES );

		//cout << "enemies draw" << endl;
		UpdateEnemiesDraw();
		
		if (activeSequence != NULL)
		{
			activeSequence->Draw(preScreenTex);
		}
		
		DrawEffects( EffectLayer::BETWEEN_PLAYER_AND_ENEMIES );
		//bigBulletVA->draw( preScreenTex );


		//window->display();
		//continue;

		goalPulse->Draw( preScreenTex );

		for( int i = 0; i < 4; ++i )
		{
			p = GetPlayer( i );
			if( p != NULL )
			{
				if( ( p->action != Actor::GRINDBALL && p->action != Actor::GRINDATTACK ) || p->leftWire->state == Wire::RETRACTING )
				{
					p->leftWire->Draw( preScreenTex );
				}
				if( ( p->action != Actor::GRINDBALL && p->action != Actor::GRINDATTACK ) || p->rightWire->state == Wire::RETRACTING )
				{
					p->rightWire->Draw( preScreenTex );
				}
			}
		}

		if( shipSequence )
		{
			//preScreenTex->draw( cloud0a );
			preScreenTex->draw( cloud1, ts_w1ShipClouds1->texture );
			preScreenTex->draw( cloud0, ts_w1ShipClouds0->texture );
			preScreenTex->draw( middleClouds );
			preScreenTex->draw( cloudBot1, ts_w1ShipClouds1->texture );
			preScreenTex->draw( cloudBot0, ts_w1ShipClouds0->texture );
			//preScreenTex->draw( cloud0b );
			preScreenTex->draw( shipSprite );
		}

		Enemy *current = activeEnemyList;
		while (current != NULL)
		{
			//	cout << "draw" << endl;
			if ((pauseFrames >= 2 && current->receivedHit != NULL))
			{
				current->Draw(preScreenTex);
			}
			current = current->next;
		}

		absorbParticles->Draw(preScreenTex);



		for( int i = 0; i < 4; ++i )
		{
			p = GetPlayer( i );
			if( p != NULL )
			{
				p->Draw( preScreenTex );
			
				/*if( repGhost != NULL && i == 0 )
					repGhost->Draw( preScreenTex );*/
				
			}
		}

		for (auto it = replayGhosts.begin(); it != replayGhosts.end(); ++it)
		{
			(*it)->Draw(preScreenTex);
		}

		if( shipSequence )
		{
			///preScreenTex->draw( cloud1a );
			//preScreenTex->draw( cloud1b );
			
		}

		//whited out hit enemies
		

		DrawEffects( EffectLayer::IN_FRONT );

		if( ts_basicBullets != NULL )
		{
			preScreenTex->draw( *bigBulletVA, ts_basicBullets->texture );
		}

		rainView.setCenter( (int)view.getCenter().x % 64, (int)view.getCenter().y % 64 );
		rainView.setSize( view.getSize() );
		preScreenTex->setView( rainView );

		if (rain != NULL)
			rain->Draw(preScreenTex);
		//rain.Draw( preScreenTex );
		preScreenTex->setView( view );

		//view.set
		
		//cut.Draw( preScreenTex, cutFrame );
		//cutFrame++;
		//if( cutFrame == cut.totalFrames )
		//{
		//	cutFrame = 0;
		//}
		preScreenTex->setView( view );
		//f->Draw( preScreenTex );
		

		if( false )//if( currInput.back || sf::Keyboard::isKeyPressed( sf::Keyboard::H ) )
		{
			//alphaTextSprite.setOrigin( alphaTextSprite.getLocalBounds().width / 2, alphaTextSprite.getLocalBounds().height / 2 );
//			alphaTextSprite.setScale( .5, .5 );
			alphaTextSprite.setScale( .5 * view.getSize().x / 960.0, .5 * view.getSize().y / 540.0 );
			alphaTextSprite.setOrigin( alphaTextSprite.getLocalBounds().width / 2, alphaTextSprite.getLocalBounds().height / 2 );
			alphaTextSprite.setPosition( view.getCenter().x, view.getCenter().y );

			preScreenTex->draw( alphaTextSprite );
			//window->draw( alphaTextSprite );
		}

		/*Enemy *currFX = active;
		while( currFX != NULL )
		{
			currFX->Draw( window );
			currFX = currFX->next;
		}*/
		
		for( list<MovingTerrain*>::iterator it = movingPlats.begin(); it != movingPlats.end(); ++it )
		{
			//(*it)->DebugDraw( preScreenTex );
			(*it)->Draw( preScreenTex );
		}


		/*EnvPlant *drawPlant = activeEnvPlants;
		while( drawPlant != NULL )
		{
			preScreenTex->draw( *drawPlant->particle->particles );
			drawPlant = drawPlant->next;
		}*/
		//dont forget to actually remove this

		
		if( Keyboard::isKeyPressed( Keyboard::Num1 ) )
		{
			showDebugDraw = true;
		}
		else if( Keyboard::isKeyPressed( Keyboard::Num2 ) )
		{
			showDebugDraw = false;
		}
		else if( Keyboard::isKeyPressed( Keyboard::Num3 ) )
		{
			showTerrainDecor = false;
		}
		else if( Keyboard::isKeyPressed( Keyboard::Num4 ) )
		{
			showTerrainDecor = true;
		}
		

		if( showDebugDraw )
			DebugDrawActors();

		



		//grassTree->DebugDraw( preScreenTex );


		//coll.DebugDraw( preScreenTex );

		//double minimapZoom = 8;// * cam.GetZoom();// + cam.GetZoom();
		double minimapZoom = 16;//12;// * cam.GetZoom();// + cam.GetZoom();

		

		View vv;
		vv.setCenter( p0->position.x, p0->position.y );
		vv.setSize( minimapTex->getSize().x * minimapZoom, minimapTex->getSize().y * minimapZoom );

		minimapTex->setView( vv );
		minimapTex->clear( Color( 0, 0, 0, 191 ) );
		//minimapTex->clear( Color( 0, 0, 0, 255 ) );
		

		for( list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it )
		{
			(*it)->Draw( minimapTex );
		}
		
		

		queryMode = "border";
		numBorders = 0;
		sf::Rect<double> minimapRect(vv.getCenter().x - vv.getSize().x / 2.0,
			vv.getCenter().y - vv.getSize().y / 2.0, vv.getSize().x, vv.getSize().y );

		borderTree->Query( this, minimapRect );

		Color testColor( 0x75, 0x70, 0x90, 191 );
		listVAIter = listVA;
		while( listVAIter != NULL )
		{
			int vertexCount = listVAIter->terrainVA->getVertexCount();
			for( int i = 0; i < vertexCount; ++i )
			{
				(*listVAIter->terrainVA)[i].color = testColor;
			}
			minimapTex->draw( *listVAIter->terrainVA );
			for( int i = 0; i < vertexCount; ++i )
			{
				(*listVAIter->terrainVA)[i].color = Color::White;
			}

			listVAIter = listVAIter->next;
		}

		minimapTex->draw(blackBorderQuadsMini, 8, sf::Quads);
		minimapTex->draw(topBorderQuadMini, 4, sf::Quads);
		//minimapTex->draw(topBorderQuad, 4, sf::Quads);
		

		queryMode = "item";
		drawCritical = NULL;
		itemTree->Query( this, minimapRect );
		while( drawCritical != NULL )
		{
			//cout << "draw crit " << endl;
			drawCritical->Draw( preScreenTex );
			drawCritical = drawCritical->next;
		}

		testGateCount = 0;
		queryMode = "gate";
		gateList = NULL;
		gateTree->Query( this, minimapRect );
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
				minimapTex->draw( activePreview, 4, sf::Quads );
			}

			Gate *next = gateList->next;//edgeA->edge1;
			gateList = next;
		}
		

		//CircleShape playerCircle;
		//playerCircle.setFillColor( COLOR_TEAL );
		//playerCircle.setRadius( 60 );//60 );
		//playerCircle.setOrigin( playerCircle.getLocalBounds().width / 2, playerCircle.getLocalBounds().height / 2 );
		//playerCircle.setPosition( vv.getCenter().x, vv.getCenter().y );
		
		for( int i = 0; i < 4; ++i )
		{
			p = GetPlayer( 0 );
			if( p != NULL )
			{
				if( ( p->action != Actor::GRINDBALL && p->action != Actor::GRINDATTACK ) )
				{
					p->rightWire->DrawMinimap( minimapTex );
					p->leftWire->DrawMinimap( minimapTex );
				}
			}
		}
		

		/*queryMode = "enemyminimap";
		enemyTree->Query( this, minimapRect );

		Enemy *currEnemy = activeEnemyList;
		int counter = 0;
		
		
		while( currEnemy != NULL )
		{
			currEnemy->DrawMinimap( minimapTex );
			currEnemy = currEnemy->next;
		}*/


		/*queryMode = "enemyminimap";
		enemyTree->Query(this, minimapRect);

		while (listVA != NULL)
		{
			TestVA *t = listVA->next;
			listVA->next = NULL;
			listVA = t;
		}*/

		//shouldn't this draw all enemies that are active not just the ones from the current
		//zone?
		for (list<Enemy*>::iterator it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
		{
			(*it)->CheckedMiniDraw(minimapTex, FloatRect(minimapRect));
		}

		//if( currentZone != NULL )
		//{
		//	for( list<Enemy*>::iterator it = currentZone->allEnemies.begin(); it != currentZone->allEnemies.end(); ++it )
		//	{
		//		(*it)->DrawMinimap( minimapTex );
		//	}
		//}
		//else
		//{
		//	//probably inefficient. only happens when there arent any gates. do a little
		//	//collision check to make sure they're relevant before drawing
		//	//also dont make circles every frame. just store it in the enemy
		//	for( list<Enemy*>::iterator it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it )
		//	{
		//		(*it)->DrawMinimap( minimapTex );
		//	}
		//}

		/*if( player->action != Actor::DEATH )
		{
			player->Draw( minimapTex );
		}*/

		minimapTex->display();
		const Texture &miniTex = minimapTex->getTexture();
		minimapShader.setUniform( "u_texture", minimapTex->getTexture() );

		minimapSprite.setTexture( miniTex );

		if( false )
		{

			
			//sf::RectangleShape blah( Vector2f( 1920, 1080 ) );
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

		//shaderTester.Draw( preScreenTex );

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

		//postProcessTex->display();

		

		//preScreenTex->draw( minimapSprite );

		
		//preScreenTex->draw( leftHUDBlankSprite );
		//preScreenTex->draw( speedBarSprite, &speedBarShader );
		
		if (mh->gameMode == MapHeader::MapType::T_STANDARD)
		{
			/*if (p0->speedLevel == 0)
			{
				preScreenTex->draw(p0->kinUnderOutline);
				preScreenTex->draw(p0->kinTealOutline, &speedBarShader);
			}
			else if (p0->speedLevel == 1)
			{
				preScreenTex->draw(p0->kinTealOutline);
				preScreenTex->draw(p0->kinBlueOutline, &speedBarShader);
			}
			else if (p0->speedLevel == 2)
			{
				preScreenTex->draw(p0->kinBlueOutline);
				preScreenTex->draw(p0->kinPurpleOutline, &speedBarShader);
			}*/


			if (p0->desperationMode)
			{
				preScreenTex->draw(p0->kinFaceBG, &(p0->despFaceShader));
			}
			else
			{
				preScreenTex->draw(p0->kinFaceBG);
			}
			preScreenTex->draw(p0->kinFace);
		}

		momentumBar->SetMomentumInfo(p0->speedLevel, p0->GetSpeedBarPart());
		momentumBar->Draw(preScreenTex);
		
		/*sf::Sprite testNumbers;
		testNumbers.setTexture(*(GetTileset("keynum_95x100.png", 95, 100)->texture));
		testNumbers.setOrigin(testNumbers.getLocalBounds().width, 0);
		testNumbers.setPosition(1920, 0);
		preScreenTex->draw(testNumbers);*/

		//else 

		/*sf::Vertex blah[] = 
		{ 
			Vertex(  ),
			Vertex( Vector2f( 300, 0 )),
			Vertex( Vector2f( 300, 300 )),
			Vertex( Vector2f( 0, 300 ) )
		};*/
		//VertexArray va( sf::Quads, 4 );
	
		preScreenTex->draw( minimapSprite, &minimapShader);
		//preScreenTex->draw( lifeBarSprite );
		//preScreenTex->draw( minimapVA, &minimapShader );
		//preScreenTex->draw( miniCircle );

		//draw gate directions
		/*if( currentZone != NULL )
		{
			int index = 0;
			list<Edge*> gList = currentZone->gates;
			for( list<Edge*>::iterator it = gList.begin(); it != gList.end(); ++it )
			{
				Gate *tGate = (Gate*)(*it)->info;
				if( tGate->gState == Gate::OPEN || tGate->gState == Gate::DISSOLVE
					|| tGate->gState == Gate::REFORM
					|| tGate->gState == Gate::LOCKFOREVER ||
					tGate->type == Gate::BLACK )
				{
					continue;
				}

				V2d avg = ( tGate->edgeA->v1 + tGate->edgeA->v0 ) / 2.0;

				double rad = minimapRect.width / 2 - 100;
				if( length( p0->position - avg ) < rad )
				{
					continue;
				}
				
				V2d dir = normalize( avg - p0->position );
				double angle = atan2( dir.y, -dir.x );
				gateDirections[index].setRotation( -angle / PI * 180 - 90 );
				preScreenTex->draw( gateDirections[index] );
				index++;

			}
			
		}*/
		
		/*for( int i = 0; i < 6; ++i )
		{
			Sprite &gds = gateDirections[i];
			gds.setTexture( *ts_minimapGateDirection->texture );
			gds.setTextureRect( ts_minimapGateDirection->GetSubRect( 0 ) );
			gds.setOrigin( gds.getLocalBounds().width / 2, 300 + gds.getLocalBounds().height );
			gds.setPosition( miniCircle.getPosition() );
		}*/
		
		//inefficient because its in the draw call
		//kinMinimapIcon.setPosition( 180 + ( powerWheel->basePos.x - powerWheel->origBasePos.x ), preScreenTex->getSize().y - 180 );
		
		


		preScreenTex->draw( kinMinimapIcon );
	//minimapSprite.draw( preScreenTex );
		//preScreenTex->draw( minimapSprite, &minimapShader );
		
		
		
		//powerOrbs->Draw( preScreenTex );
		
		//powerWheel->Draw( preScreenTex );
		if (powerRing != NULL )
		{
			powerRing->Draw(preScreenTex);
			despOrb->Draw(preScreenTex);
		}
		keyMarker->Draw( preScreenTex );
		scoreDisplay->Draw( preScreenTex );
		//preScreenTex->draw( leftHUDSprite );

		//window->setView( uiView );
	//	window->draw( healthSprite );
		

		//preScreenTex->draw( topbarSprite );

		//preScreenTex->draw( player->kinFace );
		//topbarSprite.draw( preScreenTex );

		//preScreenTex->draw( keyHolderSprite );


		//note: gotta fix these later for number of keys
	


		if( showFrameRate )
		{
			preScreenTex->draw( frameRate );
		}
		
		if(inputVis != NULL)
		inputVis->Draw(preScreenTex);


		preScreenTex->setView( view );
		//window->setView( view );

		

		

		

		
		//terrainTree->DebugDraw( window );
		//DebugDrawQuadTree( window, enemyTree );
	//	enemyTree->DebugDraw( window );
		

		//if( deathWipe )
		//{
		//	//cout << "showing death wipe frame: " << deathWipeFrame << " panel: " << deathWipeFrame / 5 << endl;
		//	wipeSprite.setTexture( wipeTextures[deathWipeFrame / 5] );
		//	wipeSprite.setTextureRect( IntRect( 0, 0, wipeSprite.getTexture()->getSize().x, 
		//		wipeSprite.getTexture()->getSize().y) );
		//	wipeSprite.setOrigin( wipeSprite.getLocalBounds().width / 2, wipeSprite.getLocalBounds().height / 2 );
		//	wipeSprite.setPosition( player->position.x, player->position.y );//view.getCenter().x, view.getCenter().y );
		//	preScreenTex->draw( wipeSprite );
		//}

		
		for( int i = 0; i < 4; ++i )
		{
			p = GetPlayer( i );
			if( p != NULL )
			{
				if( p->action == Actor::DEATH )
				{
					p->Draw( preScreenTex );
				}
			}
		}

		cloneShader.setUniform( "u_texture", preScreenTex->getTexture() );
		cloneShader.setUniform( "newscreen", p0->percentCloneChanged );
		cloneShader.setUniform( "Resolution", Vector2f( 1920, 1080 ) );//window->getSize().x, window->getSize().y);
		cloneShader.setUniform( "zoom", cam.GetZoom() );

		cloneShader.setUniform( "topLeft", Vector2f( view.getCenter().x - view.getSize().x / 2, 
			view.getCenter().y + view.getSize().y / 2 ) );

		cloneShader.setUniform( "bubbleRadius0", (float)p0->bubbleRadiusSize[0] );
		cloneShader.setUniform( "bubbleRadius1", (float)p0->bubbleRadiusSize[1] );
		cloneShader.setUniform( "bubbleRadius2", (float)p0->bubbleRadiusSize[2] );
		cloneShader.setUniform( "bubbleRadius3", (float)p0->bubbleRadiusSize[3] );
		cloneShader.setUniform( "bubbleRadius4", (float)p0->bubbleRadiusSize[4] );
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
		cloneShader.setUniform( "b0Frame", (float)p0->bubbleFramesToLive[0] );
		//cloneShader.setUniform( "bubble1", pos1 );
		cloneShader.setUniform( "b1Frame", (float)p0->bubbleFramesToLive[1] );
		//cloneShader.setUniform( "bubble2", pos2 );
		cloneShader.setUniform( "b2Frame", (float)p0->bubbleFramesToLive[2] );
		//cloneShader.setUniform( "bubble3", pos3 );
		cloneShader.setUniform( "b3Frame", (float)p0->bubbleFramesToLive[3] );
		//cloneShader.setUniform( "bubble4", pos4 );
		cloneShader.setUniform( "b4Frame", (float)p0->bubbleFramesToLive[4] );
		//cloneShader.setUniform( "bubble5", pos5 );
		//cloneShader.setUniform( "b5Frame", player->bubbleFramesToLive[5] );
		

		
		
		


		

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
			DrawFade(preScreenTex);
		}

		DrawEffects(EffectLayer::UI_FRONT);

		preScreenTex->setView(uiView);

		absorbDarkParticles->Draw(preScreenTex);

		absorbShardParticles->Draw(preScreenTex);

		preScreenTex->setView(view); //sets it back to normal for any world -> pixel calcs

		preScreenTex->display();

		const Texture &preTex0 = preScreenTex->getTexture();
		Sprite preTexSprite(preTex0);
		preTexSprite.setPosition(-960 / 2, -540 / 2);
		preTexSprite.setScale(.5, .5);
		preTexSprite.setTexture( preTex0 );
		
		//screenRecorder.Update( preTex0 );
		//preTexSprite.setPosition( -960 / 2, -540 / 2 );
		//preTexSprite.setScale( .5, .5 );
		//lastFrameTex->draw(preTexSprite);
		if (debugScreenRecorder != NULL)
			debugScreenRecorder->Update(preTex0);

		window->draw( preTexSprite );//, &cloneShader );
		}
		}
		else if( state == PAUSE )
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

			UpdateInput();
			
			//cout << "up: " << (int)currInput.LUp() << "down: " << (int)currInput.LDown() <<
			//	", left: " << (int)currInput.LLeft() << ", right: " << (int)currInput.LRight() << endl;
			PauseMenu::UpdateResponse ur = pauseMenu->Update( GetCurrInputUnfiltered( 0 ), GetPrevInputUnfiltered( 0 ) );
			switch( ur )
			{
			case PauseMenu::R_NONE:
				{
					//do nothing as usual
					break;
				}
			case PauseMenu::R_P_RESUME:
				{
					state = GameSession::RUN;
					soundNodeList->Pause( false );
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

			
			//if( currInput.

			/*if( Keyboard::isKeyPressed( Keyboard::O ) )
			{
				state = RUN;
				soundNodeList->Pause( false );
			}*/
			pauseTex->clear();		
			window->clear();
			Sprite preTexSprite;
			preTexSprite.setTexture( preScreenTex->getTexture() );
			preTexSprite.setPosition( -960 / 2, -540 / 2 );
			preTexSprite.setScale( .5, .5 );
			window->draw( preTexSprite );

			pauseMenu->Draw( pauseTex );
			
			pauseTex->display();
			Sprite pauseMenuSprite;
			pauseMenuSprite.setTexture( pauseTex->getTexture() );
			//bgSprite.setPosition( );
			pauseMenuSprite.setPosition( (1920 - 1820) / 4 - 960 / 2, (1080 - 980) / 4 - 540 / 2 );
			pauseMenuSprite.setScale( .5, .5 );
			window->draw( pauseMenuSprite );

			//draw map

			if( pauseMenu->currentTab == PauseMenu::MAP )
			{
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
			TestVA * listVAIter = listVA;
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

			//mapTex->clear();
			Sprite mapTexSprite;
			mapTexSprite.setTexture( mapTex->getTexture() );
			mapTexSprite.setOrigin( mapTexSprite.getLocalBounds().width / 2, mapTexSprite.getLocalBounds().height / 2 );
			mapTexSprite.setPosition( 0, 0 );
			

			//pauseTex->setView( bigV );
			//window->setView( bigV );

			//mapTexSprite.setScale( .5, -.5 );
			mapTexSprite.setScale( .5, -.5 );
			//mapTexSprite.setScale( 1, -1 );

  			window->draw( mapTexSprite );
			//pauseTex->draw( mapTexSprite );

			//pauseTex->setV
			}


			

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
				for( int i = 0; i < 4; ++i )
				{
					GetPrevInput( i ) = GetCurrInput( i );
					GetPrevInputUnfiltered(i) = GetCurrInputUnfiltered(i);
					GameController &con = GetController( i );
					bool canControllerUpdate = con.UpdateState();
					if( !canControllerUpdate )
					{
						//KeyboardUpdate( 0 );
					}
					else
					{
						con.UpdateState();
						GetCurrInput( i ) = con.GetState();
						GetCurrInputUnfiltered(i) = con.GetUnfilteredState();
					}
				}

			

				

				if( ( GetCurrInput( 0 ).back && !GetPrevInput( 0 ).back ) && state == MAP )
				{
					state = RUN;
					soundNodeList->Pause( false );
				}

				accumulator -= TIMESTEP;
			}

			if( state == RUN )
			{
			//	continue;
			}


			//if( !cutPlayerInput )
			//	player->currInput = currInput;


			//double mapZoom = 16;

			//window->clear();
			
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
			TestVA * listVAIter = listVA;
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

	if (levelMusic != NULL)
	{
		levelMusic->music->stop();
		levelMusic->music->setVolume(0);
		levelMusic = NULL;
	}
	
	//delete [] line;

	//window->setView( window->getDefaultView() );
	//window->clear( Color::Red );
	//window->display();
	if( currMusic != NULL )
	{
		currMusic->stop();
		currMusic = NULL;
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
	
	for( int i = 0; i < 4; ++i )
	{
		SetFilterDefault( GetController(i).filter );
	}

	return returnVal;
}


void GameSession::Init()
{
	stormCeilingOn = false;
	stormCeilingHeight = 0;

	inputVis = NULL;

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
	
	lightTree = NULL;
	
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
	
	levelMusic = NULL;

	rain = NULL;
	//stormCeilingInfo = NULL;
	

	for (list<VertexArray*>::iterator it = polygons.begin(); it != polygons.end(); ++it)
	{
		delete (*it);
	}
	polygons.clear();

	powerRing = NULL;
	
	polyShaders = NULL;
	ts_polyShaders = NULL;
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
	b_crawler = NULL;
	b_bird = NULL;
	b_coyote = NULL;
	b_tiger = NULL;
	b_gator = NULL;
	b_skeleton = NULL;

	cutPlayerInput = false;

	Vector2f miniPos = Vector2f(30, 750);
	miniVA[0].position = miniPos + Vector2f(0, 0);
	miniVA[1].position = miniPos + Vector2f(300, 0);
	miniVA[2].position = miniPos + Vector2f(300, 300);
	miniVA[3].position = miniPos + Vector2f(0, 300);

	miniVA[0].texCoords = Vector2f(0, 0);
	miniVA[1].texCoords = Vector2f(300, 0);
	miniVA[2].texCoords = Vector2f(300, 300);
	miniVA[3].texCoords = Vector2f(0, 300);

	miniVA[0].color = Color::Red;
	miniVA[1].color = Color::Blue;
	miniVA[2].color = Color::Green;
	miniVA[3].color = Color::Magenta;

	miniRect.setSize(Vector2f(300, 300));
	miniRect.setPosition(500, 500);

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
	lightList = NULL;

	inactiveEffects = NULL;
	pauseImmuneEffects = NULL;
	inactiveLights = NULL;

	drawInversePoly = true;
	showDebugDraw = false;

	fadingIn = false;
	fadingOut = false;
	fadeRect.setSize(Vector2f(1920, 1080));


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

		//if( e->spawnRect.intersects( tempSpawnRect ) )
		//{
			//cout << "spawning enemy! of type: " << e->type << endl;
		//if (e->zonedSprite.getGlobalBounds().intersects(FloatRect(tempSpawnRect)))
		//{
		//	//e->ZoneDraw( )
		//}


		//	if (rect.intersects(zonedSprite.getGlobalBounds()))
			//if( !e->spawned )
		//	{
		//		e->DrawMinimap( minimapTex );
		//	}
			
			//e->spawned = true;

			

			//AddEnemy( e );
		//}
	}
	else if( queryMode == "border" )
	{
		if( listVA == NULL )
		{
			listVA = (TestVA*)qte;
		//	cout << "1" << endl;
			numBorders++;
		}
		else
		{
			
			TestVA *tva = (TestVA*)qte;
			TestVA *temp = listVA;
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
	else if( queryMode == "lightdisplay" )
	{
		if( lightList == NULL )
		{
			lightList = (Light*)qte;
		}
		else
		{
			
			Light *tlight = (Light*)qte;
			Light *temp = lightList;
			bool okay = true;
			while( temp != NULL )
			{
				if( temp == tlight )
				{
					okay = false;
					break;
				}	
				temp = temp->next;
			}

			if( okay )
			{
				tlight->next = lightList;
				lightList = tlight;
			}
		}
	}
	else if( queryMode == "lights" )
	{
		Light *light = (Light*)qte;

		if( lightsAtOnce < tempLightLimit )
		{
			touchedLights[lightsAtOnce] = light;
			lightsAtOnce++;
		}
		else
		{
			//for( int i = 0; i < lightsAtOnce; ++i )
			//{
			//	if( length( V2d( touchedLights[i]->pos.x, touchedLights[i]->pos.y ) - position ) > length( V2d( light->pos.x, light->pos.y ) - position ) )//some calculation here
			//	{
			//		touchedLights[i] = light;
			//		break;
			//	}
					
			//}
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

void GameSession::SuppressEnemyKeys( Gate::GateType gType )
{
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
	//		currEnemy->suppressMonitor = true;
	//	}
	//	//currEnemy->moni
	//	currEnemy = currEnemy->next;
	//}
}

void GameSession::KillAllEnemies()
{
	Enemy *curr = activeEnemyList;
	while( curr != NULL )
	{
		Enemy *next = curr->next;

		if( curr->type != EnemyType::EN_GOAL )
		{
			curr->DirectKill();
			//curr->health = 0;
		}
		curr = next;	
	}
}

void GameSession::TestVA::UpdateBushFrame()
{
	/*bushFrame++;
	if( bushFrame == bushAnimLength * bushAnimFactor )
	{
		bushFrame = 0;
	}*/
}

void GameSession::TestVA::DrawBushes( sf::RenderTarget *target )
{
	for( list<DecorExpression*>::iterator it = bushes.begin(); 
		it != bushes.end(); ++it )
	{
		Tileset *ts = (*it)->layer->ts;
		target->draw( *(*it)->va, ts->texture );
	}
	//target->draw( 
}

void GameSession::TestVA::UpdateBushes()
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

void GameSession::TestVA::HandleQuery( QuadTreeCollider *qtc )
{
	qtc->HandleEntrant( this );
}

bool GameSession::TestVA::IsTouchingBox( const sf::Rect<double> &r )
{
	return IsBoxTouchingBox( aabb, r );
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

void GameSession::RestartLevel()
{
	if( raceFight != NULL )
		raceFight->Reset();

	background->Reset();

	soundNodeList->Clear();

	totalGameFrames = 0;

	/*if( GetPlayer->record > 1 )
	{
		player->LoadState();
		LoadState();
	}*/

	goalPulse->Reset();
	//f->Reset();

	activeDialogue = NULL;

	fadingIn = false;
	fadingOut = false;
	numKeysCollected = 0;

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

//	currentZone = originalZone;
//	if( currentZone != NULL )
//		keyMarker->SetStartKeysZone(currentZone);

	if( poiMap.count( "ship" ) > 0 )
	{
		ResetShipSequence();
	}
	//currentZone = NULL;
	cam.zoomFactor = 1;
	
	cam.offset = Vector2f( 0, 0 );
	cam.manual = false;
	cam.rumbling = false;
	cam.easing = false;

	if( !cam.bossCrawler )
	{
		cam.zoomFactor = 1;
		cam.zoomLevel = 0;
		cam.offset = Vector2f( 0, 0 );
	}

	for( int i = 0; i < 4; ++i )
	{
		Actor *player = GetPlayer( i );
		if( player != NULL )
			player->Respawn();
	}

	absorbParticles->Reset();
	absorbDarkParticles->Reset();
	absorbShardParticles->Reset();
	//player->Respawn();
	
	cam.pos.x = GetPlayer( 0 )->position.x;
	cam.pos.y = GetPlayer( 0 )->position.y;

	//RespawnPlayer();
	pauseFrames = 0;

	ResetEnemies();
	ResetPlants(); //eventually maybe treat these to reset like the rest of the stuff
	//only w/ checkpoints. but for now its always back

	for( list<Zone*>::iterator it = zones.begin(); it != zones.end(); ++it )
	{
		(*it)->Reset();
	}

	currentZone = NULL;
	if (originalZone != NULL)
	{
		ActivateZone(originalZone, true);
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

void GameSession::UpdateTerrainShader( const sf::Rect<double> &aabb )
{
	lightsAtOnce = 0;
	tempLightLimit = 9;

	queryMode = "lights"; 
	lightTree->Query( this, aabb );

	Vector2i vi = Mouse::getPosition();
	Vector3f blahblah( vi.x / 1920.f,  -1 + vi.y / 1080.f, .015 );
	//polyShader.setUniform( "stuff", 10, 10, 10 );
	
/*	Vector3f pos0( vi0.x / 1920.f, (1080 - vi0.y) / 1080.f, .015 ); 
	pos0.y = 1 - pos0.y;
	Vector3f pos1( vi1.x / 1920.f, (1080 - vi1.y) / 1080.f, .015 ); 
	pos1.y = 1 - pos1.y;
	Vector3f pos2( vi2.x / 1920.f, (1080 - vi2.y) / 1080.f, .015 ); 
	pos2.y = 1 - pos2.y;*/
	
	bool on[9];
	for( int i = 0; i < 9; ++i )
	{
		on[i] = false;
	}

	
	float windowx = 1920;//window->getSize().x;
	float windowy = 1080;//window->getSize().y;
	//cout << "windowx: " << windowx << ", " << windowy << endl;

	//if( lightsAtOnce > 0 )
	//{
	//	float depth0 = touchedLights[0]->depth;
	//	Vector2i vi0 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[0]->pos.x, touchedLights[0]->pos.y ) );
	//	

	//	Vector3f pos0( vi0.x / windowx, -1 + vi0.y / windowy, depth0 ); 
	//	//Vector3f pos0( vi0.x / (float)window->getSize().x, ((float)window->getSize().y - vi0.y) / (float)window->getSize().y, depth0 ); 
	//	Color c0 = touchedLights[0]->color;
	//	
	//	//underShader.setUniform( "On0", true );
	//	on[0] = true;
	//	polyShader.setUniform( "LightPos0", pos0 );//Vector3f( 0, -300, .075 ) );
	//	polyShader.setUniform( "LightColor0", c0.r / 255.0, c0.g / 255.0, c0.b / 255.0, 1 );
	//	polyShader.setUniform( "Radius0", touchedLights[0]->radius );
	//	polyShader.setUniform( "Brightness0", touchedLights[0]->brightness);
	//	
	//}
	//if( lightsAtOnce > 1 )
	//{
	//	float depth1 = touchedLights[1]->depth;
	//	Vector2i vi1 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[1]->pos.x, touchedLights[1]->pos.y ) ); 
	//	Vector3f pos1( vi1.x / windowx, -1 + vi1.y / windowy, depth1 ); 
	//	//Vector3f pos1( vi1.x / (float)window->getSize().x, ((float)window->getSize().y - vi1.y) / (float)window->getSize().y, depth1 ); 
	//	Color c1 = touchedLights[1]->color;
	//	
	//	on[1] = true;
	//	//underShader.setUniform( "On1", true );
	//	polyShader.setUniform( "LightPos1", pos1 );//Vector3f( 0, -300, .075 ) );
	//	polyShader.setUniform( "LightColor1", c1.r / 255.0, c1.g / 255.0, c1.b / 255.0, 1 );
	//	polyShader.setUniform( "Radius1", touchedLights[1]->radius );
	//	polyShader.setUniform( "Brightness1", touchedLights[1]->brightness);
	//}
	//if( lightsAtOnce > 2 )
	//{
	//	float depth2 = touchedLights[2]->depth;
	//	Vector2i vi2 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[2]->pos.x, touchedLights[2]->pos.y ) );
	//	Vector3f pos2( vi2.x / windowx, -1 + vi2.y / windowy, depth2 ); 
	//	//Vector3f pos2( vi2.x / (float)window->getSize().x, ((float)window->getSize().y - vi2.y) / (float)window->getSize().y, depth2 ); 
	//	Color c2 = touchedLights[2]->color;
	//	
	//	on[2] = true;
	//	//underShader.setUniform( "On2", true );
	//	polyShader.setUniform( "LightPos2", pos2 );//Vector3f( 0, -300, .075 ) );
	//	polyShader.setUniform( "LightColor2", c2.r / 255.0, c2.g / 255.0, c2.b / 255.0, 1 );
	//	polyShader.setUniform( "Radius2", touchedLights[2]->radius );
	//	polyShader.setUniform( "Brightness2", touchedLights[2]->brightness);
	//}
	//if( lightsAtOnce > 3 )
	//{
	//	float depth3 = touchedLights[3]->depth;
	//	Vector2i vi3 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[3]->pos.x, touchedLights[3]->pos.y ) );
	//	Vector3f pos3( vi3.x / windowx, -1 + vi3.y / windowy, depth3 ); 
	//	//Vector3f pos3( vi3.x / (float)window->getSize().x, ((float)window->getSize().y - vi3.y) / (float)window->getSize().y, depth3 ); 
	//	Color c3 = touchedLights[3]->color;
	//	
	//	on[3] = true;
	//	//underShader.setUniform( "On3", true );
	//	polyShader.setUniform( "LightPos3", pos3 );
	//	polyShader.setUniform( "LightColor3", c3.r / 255.0, c3.g / 255.0, c3.b / 255.0, 1 );
	//	polyShader.setUniform( "Radius3", touchedLights[3]->radius );
	//	polyShader.setUniform( "Brightness3", touchedLights[3]->brightness);
	//}
	//if( lightsAtOnce > 4 )
	//{
	//	float depth4 = touchedLights[4]->depth;
	//	Vector2i vi4 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[4]->pos.x, touchedLights[4]->pos.y ) );
	//	Vector3f pos4( vi4.x / windowx, -1 + vi4.y / windowy, depth4 ); 
	//	//Vector3f pos4( vi4.x / (float)window->getSize().x, ((float)window->getSize().y - vi4.y) / (float)window->getSize().y, depth4 ); 
	//	Color c4 = touchedLights[4]->color;
	//	
	//	
	//	on[4] = true;
	//	polyShader.setUniform( "LightPos4", pos4 );
	//	polyShader.setUniform( "LightColor4", c4.r / 255.0, c4.g / 255.0, c4.b / 255.0, 1 );
	//	polyShader.setUniform( "Radius4", touchedLights[4]->radius );
	//	polyShader.setUniform( "Brightness4", touchedLights[4]->brightness);
	//}
	//if( lightsAtOnce > 5 )
	//{
	//	float depth5 = touchedLights[5]->depth;
	//	Vector2i vi5 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[5]->pos.x, touchedLights[5]->pos.y ) );
	//	Vector3f pos5( vi5.x / windowx, -1 + vi5.y / windowy, depth5 ); 
	//	//Vector3f pos5( vi5.x / (float)window->getSize().x, ((float)window->getSize().y - vi5.y) / (float)window->getSize().y, depth5 ); 
	//	Color c5 = touchedLights[5]->color;
	//	
	//	
	//	on[5] = true;
	//	polyShader.setUniform( "LightPos5", pos5 );
	//	polyShader.setUniform( "LightColor5", c5.r / 255.0, c5.g / 255.0, c5.b / 255.0, 1 );
	//	polyShader.setUniform( "Radius5", touchedLights[5]->radius );
	//	polyShader.setUniform( "Brightness5", touchedLights[5]->brightness);
	//}
	//if( lightsAtOnce > 6 )
	//{
	//	float depth6 = touchedLights[6]->depth;
	//	Vector2i vi6 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[6]->pos.x, touchedLights[6]->pos.y ) );
	//	Vector3f pos6( vi6.x / windowx, -1 + vi6.y / windowy, depth6 ); 
	//	//Vector3f pos6( vi6.x / (float)window->getSize().x, ((float)window->getSize().y - vi6.y) / (float)window->getSize().y, depth6 ); 
	//	Color c6 = touchedLights[6]->color;
	//	
	//	on[6] = true;
	//	polyShader.setUniform( "LightPos6", pos6 );
	//	polyShader.setUniform( "LightColor6", c6.r / 255.0, c6.g / 255.0, c6.b / 255.0, 1 );
	//	polyShader.setUniform( "Radius6", touchedLights[0]->radius );
	//	polyShader.setUniform( "Brightness6", touchedLights[0]->brightness);
	//}
	//if( lightsAtOnce > 7 )
	//{
	//	float depth7 = touchedLights[7]->depth;
	//	Vector2i vi7 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[7]->pos.x, touchedLights[7]->pos.y ) );
	//	Vector3f pos7( vi7.x / windowx, -1 + vi7.y / windowy, depth7 ); 
	//	//Vector3f pos7( vi7.x / (float)window->getSize().x, ((float)window->getSize().y - vi7.y) / (float)window->getSize().y, depth7 ); 
	//	Color c7 = touchedLights[7]->color;
	//	
	//	on[7] = true;
	//	polyShader.setUniform( "LightPos7", pos7 );
	//	polyShader.setUniform( "LightColor7", c7.r / 255.0, c7.g / 255.0, c7.b / 255.0, 1 );
	//	polyShader.setUniform( "Radius7", touchedLights[7]->radius );
	//	polyShader.setUniform( "Brightness7", touchedLights[7]->brightness);
	//}
	//if( lightsAtOnce > 8 )
	//{
	//	float depth8 = touchedLights[8]->depth;
	//	Vector2i vi8 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[8]->pos.x, touchedLights[8]->pos.y ) );
	//	Vector3f pos8( vi8.x / windowx, -1 + vi8.y / windowy, depth8 ); 
	//	//Vector3f pos8( vi8.x / (float)window->getSize().x, ((float)window->getSize().y - vi8.y) / (float)window->getSize().y, depth8 ); 
	//	Color c8 = touchedLights[8]->color;
	//	
	//	on[8] = true;
	//	polyShader.setUniform( "LightPos8", pos8 );
	//	polyShader.setUniform( "LightColor8", c8.r / 255.0, c8.g / 255.0, c8.b / 255.0, 1 );
	//	polyShader.setUniform( "Radius8", touchedLights[8]->radius );
	//	polyShader.setUniform( "Brightness8", touchedLights[8]->brightness);
	//}

	//polyShader.setUniform( "On0", on[0] );
	//polyShader.setUniform( "On1", on[1] );
	//polyShader.setUniform( "On2", on[2] );
	//polyShader.setUniform( "On3", on[3] );
	//polyShader.setUniform( "On4", on[4] );
	//polyShader.setUniform( "On5", on[5] );
	//polyShader.setUniform( "On6", on[6] );
	//polyShader.setUniform( "On7", on[7] );
	//polyShader.setUniform( "On8", on[8] );

	//Color c = player->testLight->color;
	//Vector2i vip = preScreenTex->mapCoordsToPixel( Vector2f( player->testLight->pos.x, player->testLight->pos.y ) );
	//Vector3f posp( vip.x / windowx, -1 + vip.y / windowy, player->testLight->depth ); 
	//polyShader.setUniform( "LightPosPlayer", posp );
	//polyShader.setUniform( "LightColorPlayer", c.r / 255.0, c.g / 255.0, c.b / 255.0, 1 );
	//polyShader.setUniform( "RadiusPlayer", player->testLight->radius );
	//polyShader.setUniform( "BrightnessPlayer", player->testLight->brightness );
	//polyShader.setUniform( "OnD0", true );
}

double GameSession::GetTriangleArea( p2t::Triangle * t )
{
	p2t::Point *p_0 = t->GetPoint( 0 );
	p2t::Point *p_1 = t->GetPoint( 0 );
	p2t::Point *p_2 = t->GetPoint( 0 );

	V2d p0( p_0->x, p_0->y );
	V2d p1( p_1->x, p_1->y );
	V2d p2( p_2->x, p_2->y );

	double len0 = length( p1 - p0 );
	double len1 = length( p2 - p1 );
	double len2 = length( p0 - p2 );

	//s = .5 * (a + b + c)
	//A = sqrt( s(s - a)(s - b)(s - c) )

	double s = .5 * ( len0 + len1 + len2 );
	double A = sqrt( s * ( s - len0 ) * ( s - len1 ) * ( s - len2 ) );

	return A;
}

void GameSession::TestVA::AddDecorExpression( GameSession::DecorExpression *exp )
{
	bushes.push_back( exp );
}

void GameSession::TestVA::UpdateBushSprites()
{
	for (list<DecorExpression*>::iterator it = bushes.begin();
		it != bushes.end(); ++it)
	{
		(*it)->UpdateSprites();
	}
}

GameSession::DecorExpression::DecorExpression( std::list<sf::Vector2f> &pointList,
	GameSession::DecorLayer *p_layer )
	:layer( p_layer )
{
	int numBushes = pointList.size();
	//cout << "numBushes: " << numBushes << endl;
	Tileset *ts = layer->ts;

	va = new VertexArray( sf::Quads, numBushes * 4 );
	VertexArray &VA = *va;

	IntRect subRect = ts->GetSubRect( 0 );
	list<Vector2f>::iterator posIt;
	if( numBushes > 0 )
		posIt = pointList.begin();

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
}

GameSession::DecorExpression::~DecorExpression()
{
	delete va;
}

GameSession::DecorLayer::DecorLayer( Tileset *p_ts, int p_animLength, int p_animFactor, int p_startTile, int p_loopWait )
	:ts( p_ts ), frame( 0 ), animLength( p_animLength ), startTile( p_startTile ), animFactor( p_animFactor ),
	loopWait( p_loopWait )
{

}

void GameSession::DecorLayer::Update()
{
	++frame;
	if(frame == animLength * animFactor + loopWait )
	{
		frame = 0;
	}
}

void GameSession::DecorExpression::UpdateSprites()
{
	int numBushes = va->getVertexCount() / 4;

	Tileset *ts_bush = layer->ts;
	int frame = max( layer->frame - layer->loopWait, 0 );
	int animLength = layer->animLength;
	int animFactor = layer->animFactor;

	VertexArray &bVA = *va;

	IntRect subRect = ts_bush->GetSubRect( (layer->startTile + frame) / animFactor );

	for( int i = 0; i < numBushes; ++i )
	{
		bVA[i*4+0].texCoords = Vector2f( subRect.left, subRect.top );
		bVA[i*4+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
		bVA[i*4+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
		bVA[i*4+3].texCoords = Vector2f( subRect.left, subRect.top + subRect.height );
	}
}

GameSession::DecorExpression * GameSession::CreateDecorExpression(  DecorType dType,
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
		//int GameSession::TestVA::bushFrame = 0;
		//int GameSession::TestVA::bushAnimLength = 20;
		//int GameSession::TestVA::bushAnimFactor = 8;

		Tileset *ts_d = NULL;
		switch (dType)
		{
		case D_W1_BUSH_NORMAL:
			ts_d = GetTileset("bush_01_64x64.png", 64, 64);
			layer = new DecorLayer(ts_d, 20, 8);
			break;
		case D_W1_ROCK_1:
			ts_d = GetTileset("rock_1_01_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_ROCK_2:
			ts_d = GetTileset("rock_1_02_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_ROCK_3:
			ts_d = GetTileset("rock_1_03_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_PLANTROCK:
			ts_d = GetTileset("bush_1_01_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_GRASSYROCK:
			ts_d = GetTileset("bush_1_02_256x256.png", 256, 256);
			layer = new DecorLayer(ts_d, 1, 1);
			break;
		case D_W1_VEINS1:
			ts_d = GetTileset("veins_w1_1_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS2:
			ts_d = GetTileset("veins_w1_2_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS3:
			ts_d = GetTileset("veins_w1_3_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS4:
			ts_d = GetTileset("veins_w1_4_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS5:
			ts_d = GetTileset("veins_w1_5_512x512.png", 512, 512);
			layer = new DecorLayer(ts_d, 12, 5, 0, veinLoopWait);
			break;
		case D_W1_VEINS6:
			ts_d = GetTileset("veins_w1_6_512x512.png", 512, 512);
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


struct PlantInfo
{
	PlantInfo( Edge*e, double q, double w )
		:edge( e), quant( q), quadWidth(w)
	{
	}
	Edge *edge;
	double quant;
	double quadWidth;
};

sf::VertexArray * GameSession::SetupPlants( Edge *startEdge, Tileset *ts )//, int (*ValidEdge)(sf::Vector2<double> &) )
{
	list<PlantInfo> info;

	int tw = 32;
	int th = 32;

	Edge *te = startEdge;
	do
	{
		//V2d eNorm = te->Normal();
		//int valid = ValidEdge( eNorm );
		int valid = 0;
		if( valid != -1 )//eNorm.x == 0 )
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
	//cout << "number of divs: " << divs << endl;
	for( int i = 0; i < divs; ++i )//while( angle <= PI * 2 )
	//int i = 3;
	//int i = 23;
	{
		rayIgnoreEdge1 = NULL;
		rayIgnoreEdge = NULL;

		allInfo.push_back( list<pair<V2d,bool>>() );
		list<pair<V2d,bool>> &pointList = allInfo.back();
		//cout << "div " << i << endl;
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
				
				if( rcEdge->edgeType == Edge::BORDER )
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
						//cout << "failing here: " << i << endl;
						assert( 0 );
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

			rayOkay = length( (goalPos + rayDir * startRadius) - rayEnd ) <= 10000;
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

void GameSession::Fade( bool in, int frames, sf::Color c)
{
	if( in )
	{
		fadeLength = frames;
		fadingIn = true;
		fadingOut = false;
	}
	else
	{
		fadeLength = frames;
		fadingIn = false;
		fadingOut = true;
	}
	fadeRect.setFillColor( Color( c.r, c.g, c.b, 255 ) );
	
	fadeFrame = 0;
}

void GameSession::UpdateFade()
{
	if( !fadingIn && !fadingOut )
		return;
	//cout << "fade frame: " << fadeFrame << endl;

	++fadeFrame;

	if( fadeFrame > fadeLength )
	{
		fadingIn = false;
		fadingOut = false;
		return;
	}

	if( fadingIn )
	{
		fadeAlpha = floor( (255.0 - 255.0 * fadeFrame / (double)fadeLength) + .5 );
	}
	else if( fadingOut )
	{
		fadeAlpha = floor( 255.0 * fadeFrame / (double)fadeLength + .5 );
	}

	Color oldColor = fadeRect.getFillColor();
	fadeRect.setFillColor( Color( oldColor.r, oldColor.g, oldColor.b, fadeAlpha ) );
}

void GameSession::DrawFade( sf::RenderTarget *target )
{
	if( fadeAlpha > 0 )
	{
		target->draw( fadeRect );
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
		inactiveEffects->prev = NULL;
		inactiveEffects->next = NULL;
	}
	else
	{
		BasicEffect *b = new BasicEffect( this ) ;
		b->next = inactiveEffects;
		inactiveEffects->prev = b;
		inactiveEffects = b;
	}
}

void GameSession::AllocateLight()
{
	if( inactiveLights == NULL )
	{
		inactiveLights = new Light( this, Vector2i( 0, 0 ), Color( 255, 255, 255, 255 ), 1, 1 );
		inactiveLights->prev = NULL;
		inactiveLights->next = NULL;
	}
	else
	{
		Light *light= new Light( this, Vector2i( 0, 0 ), Color( 255, 255, 255, 255 ), 1, 1 );
		light->next = inactiveLights;
		inactiveLights->prev = light;
		inactiveLights = light;
	}
}

BasicEffect * GameSession::ActivateEffect( EffectLayer layer, Tileset *ts, V2d pos, bool pauseImmune, double angle, int frameCount,
	int animationFactor, bool right, float depth )
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

	if( b_crawler != NULL ) b_crawler->Reset();

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

Light * GameSession::ActivateLight( int radius,  int brightness, const Color color )
{
	if( inactiveLights == NULL )
	{
		return NULL;
	}
	else
	{
		Light *l = inactiveLights;

		if( inactiveEffects->next == NULL )
		{
			inactiveEffects = NULL;
		}
		else
		{
			inactiveLights = (Light*)(inactiveLights->next);
			inactiveLights->prev = NULL;
		}

		//assert( ts != NULL );
		l->next = NULL;
		
		if( activeLights != NULL )
		{
			activeLights->prev = l;
			l->next = activeLights;
			activeLights = l;
		}
		else
		{
			activeLights = l;
		}

		l->radius = radius;
		l->brightness = brightness;
		l->color = color;
		//cout << "activating: " << b << " blah: " << b->prev << endl;
		return l;
	}
}

void GameSession::DeactivateLight( Light *light )
{
	Light *prev = light->prev;
	Light *next = light->next;

	if( prev == NULL && next == NULL )
	{
		activeLights = NULL;
	}
	else
	{
		if( light == activeLights )
		{
			next->prev = NULL;
			activeLights = next;
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


	if( inactiveLights == NULL )
	{
		inactiveLights = light;
		light->next = NULL;
	}
	else
	{
		light->next = inactiveLights;
		inactiveLights->prev = light;
		inactiveLights = light;
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
	panelTex.loadFromFile( "powerbarmockup.png" );
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

void Grass::HandleQuery( QuadTreeCollider *qtc )
{
	qtc->HandleEntrant( this );
}

bool Grass::IsTouchingBox( const Rect<double> &r )
{
	return isQuadTouchingQuad( V2d( r.left, r.top ), V2d( r.left + r.width, r.top ), 
		V2d( r.left + r.width, r.top + r.height ), V2d( r.left, r.top + r.height ),
		A, B, C, D );


	/*double left = min( edge->v0.x, edge->v1.x );
	double right = max( edge->v0.x, edge->v1.x );
	double top = min( edge->v0.y, edge->v1.y );
	double bottom = max( edge->v0.y, edge->v1.y );

	Rect<double> er( left, top, right - left, bottom - top );

	if( er.intersects( r ) )
	{
		return true;
	}*/
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

void GameSession::SetParMountains( sf::RenderTarget *target )
{
	View vah = view;
	double zoomFactor = 6.0;
	double yChange = 100;
	double zoom = view.getSize().x / 960.0;
	double addZoom = (zoom - 1) / zoomFactor;
	double newZoom = 1 + addZoom;

	vah.setSize( 960 * newZoom, 540 * newZoom );
	//vah.setSize( 960 * zoom, 540 * zoom );
	vah.setCenter( vah.getCenter().x / zoomFactor, vah.getCenter().y / zoomFactor );

	sf::RectangleShape rs;
	rs.setSize( Vector2f( vah.getSize().x, 512 ) );
	rs.setFillColor( Color::Red );
	rs.setPosition( vah.getCenter().x - vah.getSize().x / 2, - 512 + yChange );

	mountainShader.setUniform( "Resolution", Vector2f( 1920, 1080 ) );
	mountainShader.setUniform( "zoom", (float)newZoom );
	mountainShader.setUniform( "size", Vector2f( 1920, 1024 ) );
	
	Vector2f trueBotLeft = Vector2f( view.getCenter().x - view.getSize().x / 2, view.getCenter().y + view.getSize().y / 2 );
	Vector2i tempPos = preScreenTex->mapCoordsToPixel( trueBotLeft );
	preScreenTex->setView( vah );
	trueBotLeft = preScreenTex->mapPixelToCoords( tempPos );
	trueBotLeft.y -= yChange;


	mountainShader.setUniform( "topLeft", trueBotLeft );

	preScreenTex->draw( rs, &mountainShader );

	preScreenTex->setView( view );
}

void GameSession::SetParMountains1( sf::RenderTarget *target )
{
	View vah = view;
	double zoomFactor = 4;
	double yChange = 200;
	double zoom = view.getSize().x / 960.0;
	double addZoom = (zoom - 1) / zoomFactor;
	double newZoom = 1 + addZoom;

	//vah.setSize( 960 * newZoom, 540 * newZoom );
	vah.setSize( 960 * newZoom, 540 * newZoom );
	vah.setCenter( vah.getCenter().x / zoomFactor, vah.getCenter().y / zoomFactor );
	
	sf::RectangleShape rs;
	rs.setSize( Vector2f( vah.getSize().x, 406 ) );
	//rs.setFillColor( Color::Red );
	rs.setPosition( vah.getCenter().x - vah.getSize().x / 2, -406 + yChange );//- 512 );

	mountainShader1.setUniform( "Resolution", Vector2f( 1920, 1080 ) );
	mountainShader1.setUniform( "zoom", (float)newZoom );
	mountainShader1.setUniform( "size", Vector2f( 1920, 812 ) );
	
	
	Vector2f trueBotLeft = Vector2f( view.getCenter().x - view.getSize().x / 2, view.getCenter().y + view.getSize().y / 2 );
	Vector2i tempPos = preScreenTex->mapCoordsToPixel( trueBotLeft );
	preScreenTex->setView( vah );
	trueBotLeft = preScreenTex->mapPixelToCoords( tempPos );
	trueBotLeft.y -= yChange;

	mountainShader1.setUniform( "topLeft", trueBotLeft );

	preScreenTex->draw( rs, &mountainShader1 );

	preScreenTex->setView( view );
}

void GameSession::SetParOnTop( sf::RenderTarget *target )
{
	//closeBack0.setPosition( -960, -400 );
	//closeBack0.setTextureRect( IntRect( 0, 0, 1920, 400 ) );
	//closeBack0.setColor( Color::Red );
	
	sf::RectangleShape rs;
	rs.setSize( Vector2f( view.getSize().x, 370 / 2 ) );
	rs.setFillColor( Color::White );
	rs.setPosition( view.getCenter().x - view.getSize().x / 2, - 370 / 2 );

	onTopShader.setUniform( "Resolution", Vector2f( 1920, 1080 ) );
	onTopShader.setUniform( "zoom", cam.GetZoom() );
	onTopShader.setUniform( "topLeft", Vector2f( view.getCenter().x - view.getSize().x / 2,
		view.getCenter().y + view.getSize().y / 2 ) );

	preScreenTex->draw( rs, &onTopShader );
	//preScreenTex->draw( rs );


	/*int tilesWide = 3;
	int totalWidth = 1920 * tilesWide;
	int camLeft = view.getCenter().x - view.getSize().x / 2;
	int camRight = view.getCenter().x + view.getSize().x / 2;
	int diff = camLeft / totalWidth;

	sf::RectangleShape r0( Vector2f( 1920, 400 ) );

	r0.setPosition( diff * totalWidth, -400 );
	r0.setFillColor( Color::Red );

	preScreenTex->draw( r0 );

	r0.setPosition( 1920 + diff * totalWidth, -400 );
	r0.setFillColor( Color::Green );

	preScreenTex->draw( r0 );

	r0.setPosition( 1920 * 2 + diff * totalWidth, -400 );
	r0.setFillColor( Color::Blue );

	
	preScreenTex->draw( r0 );*/


	
	/*int tilesWide = 6;
	int zoom = 1;
	int height = 400;
	int width = 1920;
	int totalWidth = width * tilesWide;
	for( int i = 0; i < tilesWide; ++i )
	{
		onTopPar[i*4].color = Color::Blue;
		onTopPar[i*4+1].color= Color::Red;
		onTopPar[i*4+2].color= Color::Red;
		onTopPar[i*4+3].color= Color::Blue;
	}

	Vector2f delta;
	delta.x = cam.pos.x / zoom;
	delta.y = cam.pos.y / zoom;

	cout << "blah: " << (int)cam.pos.x % totalWidth << endl;
	for( int i = 0; i < tilesWide; ++i )
	{
		int x;
		if( cam.pos.x >= 0 )
		{
			x = i * width + cam.pos.x - ;//- (int)cam.pos.x % totalWidth;

			onTopPar[i*4].position = Vector2f( x, -height );
			onTopPar[i*4+1].position = Vector2f( x + width, -height );
			onTopPar[i*4+2].position = Vector2f( x + width, 0 );
			onTopPar[i*4+3].position = Vector2f( x, 0 );
		}
		else
		{
			x = i * width + cam.pos.x - (int)cam.pos.x % totalWidth;
			onTopPar[i*4].position = Vector2f( x, -height );
			onTopPar[i*4+1].position = Vector2f( x + width, -height );
			onTopPar[i*4+2].position = Vector2f( x + width, 0 );
			onTopPar[i*4+3].position = Vector2f( x, 0 );
		}

		
		
	}

	target->draw( onTopPar );*/
	
}

bool GameSession::SetGroundPar()
{	
	Color undertransColor( 255, 50, 255, 255 );
	Color altTransColor( 0, 255, 255, 255 );
	int widthFactor = 1;
	int yView = view.getCenter().y / widthFactor;
	cloudView.setCenter( 960, 540 + yView );
	int cloudBot = cloudView.getCenter().y + cloudView.getSize().y / 2;

	//cout << "yView << " << yView << endl;
	int tileHeight = 1080 / 2;//540;
	int transTileHeight = 750;//650 / 2;

	if( yView > 1080 + transTileHeight || yView < -tileHeight )
	{
	//	return false;
	}
	Vector2f offset( 0, -transTileHeight );
	
	int width = 1920 * widthFactor;
	bool flipped = false;
	int a = ((int)view.getCenter().x) % width;
	double ratio = a / (double)width;
	if( ratio < 0 )
		ratio = 1 + ratio;

	int b = ((int)view.getCenter().x) % (width * 2);
	double ratiob = b / (double)(width);
	if( ratiob < 0 )
		ratiob = 2 + ratiob;

	if( ratiob > ratio + .001 )
	{
	//	cout << "flipped ratiob: " << ratiob << ", oldratio: " << ratio << endl;
		flipped = true;
	}
	

	int i = 0;
	if( flipped )
	{
		i = 1;
		
	}
	//preScreenTex->setView( cloudView );
	float screenBottom = view.getCenter().y + view.getSize().y / 2;
	int transBot;
	
	int groundBottom = 1080;
	int groundTop = groundBottom - tileHeight;
	preScreenTex->setView( view );
	

	transBot = groundBottom + transTileHeight + 1;

	if( screenBottom >= 0 )
	{
		//cout << "undergroundPos: " << 
		Vector2i po = preScreenTex->mapCoordsToPixel( Vector2f( 0, 0 ) );
		preScreenTex->setView( cloudView );
		Vector2f la = preScreenTex->mapPixelToCoords( po );
	//	cout << "under: " << la.y << endl;
		transBot = -offset.y + la.y + 1;
	//	Vector2f pix = preScreenTex->mapPixelToCoords( Vector2i( 1920, 1080 ) ).y;
		//underground is visible
		//transBot =  //1080 - screenBottom;;//cloudBot;//cloudBot; //
		//transBot = view.getCenter().y / 2 / cam.GetZoom();
	//	cout << "transbot: " << transBot << ", center: " << view.getCenter().y << endl;
	}
	//else
	{
		//underground isn't visible
	//	cout << "transtop: " << groundBottom << endl;
	//	cout << "transbot no underground: " << transBot << endl;
	}
	int transTop = groundBottom;
	//ratio = 1 - ratio;
	
	ratio = 1 - ratio;
	//cout << "ratio: " << ratio << endl;

	groundPar[i*4].position = Vector2f( 0, groundTop ) + offset;
	groundPar[i*4+1].position = Vector2f( 1920 * ratio, groundTop) + offset;
	groundPar[i*4+2].position = Vector2f( 1920 * ratio, groundBottom ) + offset;
	groundPar[i*4+3].position = Vector2f( 0, groundBottom ) + offset;

	

	groundPar[i*4].texCoords = Vector2f( 1920 * (1-ratio), tileHeight * i );
	groundPar[i*4 + 1].texCoords = Vector2f( 1920, tileHeight * i );
	groundPar[i*4 + 2].texCoords = Vector2f( 1920, tileHeight * (i + 1) );
	groundPar[i*4 + 3].texCoords = Vector2f( 1920 * (1-ratio), tileHeight * (i + 1) );

	//int what = transTileHeight;//tileHeight / 2 + 100;

//	int bottom = transTileHeight + (1080);
	
	//preScreenTex->setView( view );
	if( screenBottom >= 0 )
	{
	//	bottom = preScreenTex->mapCoordsToPixel( Vector2f( 0, 0 ) ).y;//transTileHeight + 1080 - screenBottom;
	}
	/*underTransPar[i*4].position = Vector2f( 0, transTop  ) + offset;
	underTransPar[i*4+1].position = Vector2f( 1920 * ratio, transTop ) + offset;
	underTransPar[i*4+2].position = Vector2f( 1920 * ratio, transBot ) + offset;
	underTransPar[i*4+3].position = Vector2f( 0, transBot ) + offset;

	underTransPar[i*4].color = undertransColor;
	underTransPar[i*4 + 1].color = undertransColor;
	underTransPar[i*4 + 2].color = altTransColor;
	underTransPar[i*4 + 3].color = altTransColor;*/

	if( flipped )
	{
		i = 0;
	}
	else
	{
		i = 1;
	}



	groundPar[i*4].position = Vector2f( 1920 * ratio , groundTop ) + offset;
	groundPar[i*4+ 1].position = Vector2f( 1920, groundTop ) + offset;
	groundPar[i*4+2].position = Vector2f( 1920, groundBottom ) + offset;
	groundPar[i*4+3].position = Vector2f( 1920 * ratio , groundBottom ) + offset;

	groundPar[i*4].texCoords = Vector2f( 0, tileHeight * i );
	groundPar[i*4+1].texCoords = Vector2f( 1920 * (1-ratio), tileHeight * i );
	groundPar[i*4+2].texCoords = Vector2f( 1920 * (1-ratio), tileHeight * (i+1) );
	groundPar[i*4+3].texCoords = Vector2f( 0, tileHeight * (i+1) );

	/*underTransPar[i*4].position = Vector2f( 1920 * ratio , transTop ) + offset;
	underTransPar[i*4+ 1].position = Vector2f( 1920, transTop ) + offset;
	underTransPar[i*4+2].position = Vector2f( 1920, transBot ) + offset;
	underTransPar[i*4+3].position = Vector2f( 1920 * ratio , transBot ) + offset;

	underTransPar[i*4].color = undertransColor;
	underTransPar[i*4 + 1].color = undertransColor;
	underTransPar[i*4 + 2].color = altTransColor;
	underTransPar[i*4 + 3].color = altTransColor;*/

	
	
	preScreenTex->setView( cloudView );

	return true;
}

void GameSession::SetupClouds()
{
	clouds[0].setTexture( *cloudTileset->texture );//"cloud01.png
	clouds[1].setTexture( *cloudTileset->texture );
	clouds[2].setTexture( *cloudTileset->texture );
	clouds[3].setTexture( *cloudTileset->texture );
	clouds[4].setTexture( *cloudTileset->texture );

	for( int i = 0; i < NUM_CLOUDS; ++i )
	{
		//clouds[i].setOrigin( clouds[i].getLocalBounds().width / 2, clouds[i].getLocalBounds().height / 2 );
	}

	clouds[0].setPosition( 0, 0 );
	clouds[1].setPosition( 100, 100 );
	clouds[2].setPosition( 200, 300 );
	clouds[3].setPosition( 300, 500 );
	clouds[4].setPosition( 400, 700 );
}

void GameSession::SetCloudParAndDraw()
{
	int depth = 8;
	Vector2f orig( originalPos.x, originalPos.y );
	int screenWidthFactor = 3;


	for( int i = 0; i < NUM_CLOUDS; ++i )
	{
		if( view.getCenter().x < 0 )
		{
			int x = (int)((orig.x + view.getCenter().x) / depth - .5);
			clouds[i].setPosition( -( x % ( 1920 * screenWidthFactor ) 
				+ ( (1920 * screenWidthFactor) / 2 )), clouds[i].getPosition().y );
		//	cout << "neg: " << clouds[i].getPosition().x << endl;
		}
		else
		{
			int x = (int)((orig.x + view.getCenter().x) / depth + .5);
			clouds[i].setPosition( -(x % ( 1920 * screenWidthFactor ) 
				- ( (1920 * screenWidthFactor) / 2 )), clouds[i].getPosition().y );
		//	cout << "pos: " << clouds[i].getPosition().x << endl;
		}

		if( view.getCenter().y < 0 )
		{
			clouds[i].setPosition( clouds[i].getPosition().x, -((int)((orig.y + view.getCenter().y) / depth - .5) % ( -1080 * 3 ) ));// - ( 1080 * 3 / 2 )));
		}
		else
		{
			clouds[i].setPosition( clouds[i].getPosition().x, -((int)((orig.y + view.getCenter().y) / depth + .5) % ( 1080 * 3 ) ));// - ( 1080 * 3 / 2 )));
		}
		//cout << "cloudpos: " << clouds[i].getPosition().x << ", " << clouds[i].getPosition().y << endl;
		
		preScreenTex->draw( clouds[i] );
	}

	//float depth = 3;
	//parTest.setPosition( orig / depth + ( cam.pos - orig ) / depth );
	//float scale = 1 + ( 1 - 1 / ( cam.GetZoom() * depth ) );
	//parTest.setScale( scale, scale );
}

void GameSession::SetUndergroundParAndDraw()
{
	Actor *player = GetPlayer( 0 );
	preScreenTex->setView( view );

	underShader.setUniform( "u_texture", *GetTileset( "underground01.png" , 128, 128 )->texture );
	//underShader.setUniform( "u_normals", *GetTileset( "underground01_NORMALS.png", 128, 128 )->texture );
	//underShader.setUniform( "u_pattern", *GetTileset( "terrainworld1_PATTERN.png", 16, 16 )->texture );

	underShader.setUniform( "AmbientColor", Glsl::Vec4( 1, 1, 1, 1 ) );
	underShader.setUniform( "Resolution", Vector2f( 1920, 1080 ) );//window->getSize().x, window->getSize().y);
	underShader.setUniform( "zoom", cam.GetZoom() );
	underShader.setUniform( "topLeft", Vector2f( view.getCenter().x - view.getSize().x / 2, 
		view.getCenter().y + view.getSize().y / 2 ) );

	lightsAtOnce = 0;
	tempLightLimit = 0;//9; //only for now

	sf::Rect<double> r( view.getCenter().x - view.getSize().x / 2, view.getCenter().y - view.getSize().y / 2, view.getSize().x, view.getSize().y );
	
	queryMode = "lights"; 
	lightTree->Query( this, r );

	Vector2i vi = Mouse::getPosition();
	Vector3f blahblah( vi.x / 1920.f,  -1 + vi.y / 1080.f, .015 );
	//polyShader.setUniform( "stuff", 10, 10, 10 );
	
/*	Vector3f pos0( vi0.x / 1920.f, (1080 - vi0.y) / 1080.f, .015 ); 
	pos0.y = 1 - pos0.y;
	Vector3f pos1( vi1.x / 1920.f, (1080 - vi1.y) / 1080.f, .015 ); 
	pos1.y = 1 - pos1.y;
	Vector3f pos2( vi2.x / 1920.f, (1080 - vi2.y) / 1080.f, .015 ); 
	pos2.y = 1 - pos2.y;*/
	
	bool on[9];
	for( int i = 0; i < 9; ++i )
	{
		on[i] = false;
	}

	float windowx = 1920;//window->getSize().x;
	float windowy = 1080;//window->getSize().y;
	
	if( lightsAtOnce > 0 )
	{
		float depth0 = touchedLights[0]->depth;
		Vector2i vi0 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[0]->pos.x, touchedLights[0]->pos.y ) );
		

		Vector3f pos0( vi0.x / windowx, -1 + vi0.y / windowy, depth0 ); 
		//Vector3f pos0( vi0.x / (float)window->getSize().x, ((float)window->getSize().y - vi0.y) / (float)window->getSize().y, depth0 ); 
		Color c0 = touchedLights[0]->color;
		
		//underShader.setUniform( "On0", true );
		on[0] = true;
		underShader.setUniform( "LightPos0", pos0 );//Vector3f( 0, -300, .075 ) );
		underShader.setUniform( "LightColor0", Glsl::Vec4(c0.r / 255.0, c0.g / 255.0, c0.b / 255.0, 1 ) );
		underShader.setUniform( "Radius0", touchedLights[0]->radius );
		underShader.setUniform( "Brightness0", touchedLights[0]->brightness);
		
	}
	if( lightsAtOnce > 1 )
	{
		float depth1 = touchedLights[1]->depth;
		Vector2i vi1 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[1]->pos.x, touchedLights[1]->pos.y ) ); 
		Vector3f pos1( vi1.x / windowx, -1 + vi1.y / windowy, depth1 ); 
		//Vector3f pos1( vi1.x / (float)window->getSize().x, ((float)window->getSize().y - vi1.y) / (float)window->getSize().y, depth1 ); 
		Color c1 = touchedLights[1]->color;
		
		on[1] = true;
		//underShader.setUniform( "On1", true );
		underShader.setUniform( "LightPos1", pos1 );//Vector3f( 0, -300, .075 ) );
		underShader.setUniform( "LightColor1", Glsl::Vec4( c1.r / 255.0, c1.g / 255.0, c1.b / 255.0, 1 ) );
		underShader.setUniform( "Radius1", touchedLights[1]->radius );
		underShader.setUniform( "Brightness1", touchedLights[1]->brightness);
	}
	if( lightsAtOnce > 2 )
	{
		float depth2 = touchedLights[2]->depth;
		Vector2i vi2 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[2]->pos.x, touchedLights[2]->pos.y ) );
		Vector3f pos2( vi2.x / windowx, -1 + vi2.y / windowy, depth2 ); 
		//Vector3f pos2( vi2.x / (float)window->getSize().x, ((float)window->getSize().y - vi2.y) / (float)window->getSize().y, depth2 ); 
		Color c2 = touchedLights[2]->color;
		
		on[2] = true;
		//underShader.setUniform( "On2", true );
		underShader.setUniform( "LightPos2", pos2 );//Vector3f( 0, -300, .075 ) );
		underShader.setUniform( "LightColor2", Glsl::Vec4( c2.r / 255.0, c2.g / 255.0, c2.b / 255.0, 1 ) );
		underShader.setUniform( "Radius2", touchedLights[2]->radius );
		underShader.setUniform( "Brightness2", touchedLights[2]->brightness);
	}
	if( lightsAtOnce > 3 )
	{
		float depth3 = touchedLights[3]->depth;
		Vector2i vi3 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[3]->pos.x, touchedLights[3]->pos.y ) );
		Vector3f pos3( vi3.x / windowx, -1 + vi3.y / windowy, depth3 ); 
		//Vector3f pos3( vi3.x / (float)window->getSize().x, ((float)window->getSize().y - vi3.y) / (float)window->getSize().y, depth3 ); 
		Color c3 = touchedLights[3]->color;
		
		on[3] = true;
		//underShader.setUniform( "On3", true );
		underShader.setUniform( "LightPos3", pos3 );
		underShader.setUniform( "LightColor3", Glsl::Vec4( c3.r / 255.0, c3.g / 255.0, c3.b / 255.0, 1 ) );
		underShader.setUniform( "Radius3", touchedLights[3]->radius );
		underShader.setUniform( "Brightness3", touchedLights[3]->brightness);
	}
	if( lightsAtOnce > 4 )
	{
		float depth4 = touchedLights[4]->depth;
		Vector2i vi4 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[4]->pos.x, touchedLights[4]->pos.y ) );
		Vector3f pos4( vi4.x / windowx, -1 + vi4.y / windowy, depth4 ); 
		//Vector3f pos4( vi4.x / (float)window->getSize().x, ((float)window->getSize().y - vi4.y) / (float)window->getSize().y, depth4 ); 
		Color c4 = touchedLights[4]->color;
		
		
		on[4] = true;
		underShader.setUniform( "LightPos4", pos4 );
		underShader.setUniform( "LightColor4", Glsl::Vec4( c4.r / 255.0, c4.g / 255.0, c4.b / 255.0, 1 ) );
		underShader.setUniform( "Radius4", touchedLights[4]->radius );
		underShader.setUniform( "Brightness4", touchedLights[4]->brightness);
	}
	if( lightsAtOnce > 5 )
	{
		float depth5 = touchedLights[5]->depth;
		Vector2i vi5 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[5]->pos.x, touchedLights[5]->pos.y ) );
		Vector3f pos5( vi5.x / windowx, -1 + vi5.y / windowy, depth5 ); 
		//Vector3f pos5( vi5.x / (float)window->getSize().x, ((float)window->getSize().y - vi5.y) / (float)window->getSize().y, depth5 ); 
		Color c5 = touchedLights[5]->color;
		
		
		on[5] = true;
		underShader.setUniform( "LightPos5", pos5 );
		underShader.setUniform( "LightColor5", Glsl::Vec4( c5.r / 255.0, c5.g / 255.0, c5.b / 255.0, 1 ) );
		underShader.setUniform( "Radius5", touchedLights[5]->radius );
		underShader.setUniform( "Brightness5", touchedLights[5]->brightness);
	}
	if( lightsAtOnce > 6 )
	{
		float depth6 = touchedLights[6]->depth;
		Vector2i vi6 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[6]->pos.x, touchedLights[6]->pos.y ) );
		Vector3f pos6( vi6.x / windowx, -1 + vi6.y / windowy, depth6 ); 
		//Vector3f pos6( vi6.x / (float)window->getSize().x, ((float)window->getSize().y - vi6.y) / (float)window->getSize().y, depth6 ); 
		Color c6 = touchedLights[6]->color;
		
		on[6] = true;
		underShader.setUniform( "LightPos6", pos6 );
		underShader.setUniform( "LightColor6", Glsl::Vec4(c6.r / 255.0, c6.g / 255.0, c6.b / 255.0, 1 ) );
		underShader.setUniform( "Radius6", touchedLights[0]->radius );
		underShader.setUniform( "Brightness6", touchedLights[0]->brightness);
	}
	if( lightsAtOnce > 7 )
	{
		float depth7 = touchedLights[7]->depth;
		Vector2i vi7 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[7]->pos.x, touchedLights[7]->pos.y ) );
		Vector3f pos7( vi7.x / windowx, -1 + vi7.y / windowy, depth7 ); 
		//Vector3f pos7( vi7.x / (float)window->getSize().x, ((float)window->getSize().y - vi7.y) / (float)window->getSize().y, depth7 ); 
		Color c7 = touchedLights[7]->color;
		
		on[7] = true;
		underShader.setUniform( "LightPos7", pos7 );
		underShader.setUniform( "LightColor7", Glsl::Vec4(c7.r / 255.0, c7.g / 255.0, c7.b / 255.0, 1 ) );
		underShader.setUniform( "Radius7", touchedLights[7]->radius );
		underShader.setUniform( "Brightness7", touchedLights[7]->brightness);
	}
	if( lightsAtOnce > 8 )
	{
		float depth8 = touchedLights[8]->depth;
		Vector2i vi8 = preScreenTex->mapCoordsToPixel( Vector2f( touchedLights[8]->pos.x, touchedLights[8]->pos.y ) );
		Vector3f pos8( vi8.x / windowx, -1 + vi8.y / windowy, depth8 ); 
		//Vector3f pos8( vi8.x / (float)window->getSize().x, ((float)window->getSize().y - vi8.y) / (float)window->getSize().y, depth8 ); 
		Color c8 = touchedLights[8]->color;
		
		on[8] = true;
		underShader.setUniform( "LightPos8", pos8 );
		underShader.setUniform( "LightColor8", Glsl::Vec4(c8.r / 255.0, c8.g / 255.0, c8.b / 255.0, 1 ) );
		underShader.setUniform( "Radius8", touchedLights[8]->radius );
		underShader.setUniform( "Brightness8", touchedLights[8]->brightness);
	}

	underShader.setUniform( "On0", on[0] );
	underShader.setUniform( "On1", on[1] );
	underShader.setUniform( "On2", on[2] );
	underShader.setUniform( "On3", on[3] );
	underShader.setUniform( "On4", on[4] );
	underShader.setUniform( "On5", on[5] );
	underShader.setUniform( "On6", on[6] );
	underShader.setUniform( "On7", on[7] );
	underShader.setUniform( "On8", on[8] );

	Color c = player->testLight->color;
	Vector2i vip = preScreenTex->mapCoordsToPixel( Vector2f( player->testLight->pos.x, player->testLight->pos.y ) );
	Vector3f posp( vip.x / windowx, -1 + vip.y / windowy, player->testLight->depth ); 
	underShader.setUniform( "LightPosPlayer", posp );
	underShader.setUniform( "LightColorPlayer", Glsl::Vec4( c.r / 255.0, c.g / 255.0, c.b / 255.0, 1 ) );
	underShader.setUniform( "RadiusPlayer", player->testLight->radius );
	underShader.setUniform( "BrightnessPlayer", player->testLight->brightness );

	/*undergroundPar[0].color = Color::Red;
	undergroundPar[1].color = Color::Red;
	undergroundPar[2].color = Color::Red;
	undergroundPar[3].color = Color::Red;*/

	Vector2f center = view.getCenter();

	float top = center.y - view.getSize().y / 2;
	float left = center.x - view.getSize().x / 2;
	float bottom = center.y + view.getSize().y / 2;
	float right = center.x + view.getSize().x / 2;
	
	
	//cout << preScreenTex->getView().getCenter().x << ", " << preScreenTex->getView().getCenter().y << endl;
//	cout << "zoom: " << cam.GetZoom() << ", dist: " << -center.y * cam.GetZoom() /  4  << endl;
	
	//int distFromTop = 
	//cout << "distfrom: " << distFromTop << endl;
	//if( distFromTop < 0 )
	//	distFromTop = 0;
	//if( distFromTop > 1080 )
	//{
		/*undergroundPar[0].position = Vector2f( 0, 0 );
		undergroundPar[1].position = Vector2f( 0, 0 );
		undergroundPar[2].position = Vector2f( 0, 0 );
		undergroundPar[3].position = Vector2f( 0, 0 );*/
	//}
	//else
	int blah = 0;
	//cout << "blah: " << blah << endl;
	if( bottom < blah )
	{
	}
	else
	{
		
		if( top < blah )
		{
			top = blah;
		}
		//preScreenTex->setView( view );
		//top = 0;
		undergroundPar[0].position = Vector2f( left, top );
		undergroundPar[1].position = Vector2f( right, top );
		undergroundPar[2].position = Vector2f( right, bottom );
		undergroundPar[3].position = Vector2f( left, bottom );
		preScreenTex->draw( undergroundPar, &underShader );
	}
	
		
	//else
	{
		
		//cout << "NOT normal" << endl;
	}

	//cloudView.setCenter( cloudView.getCenter().x, center.y );

	
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
		assert(0);
	}
	//z->SetShadowColor( Color( 0, 0, 255, 10 ) );
	if (currentZone != NULL)
	{
		currentZone->action = Zone::CLOSING;
		currentZone->frame = 0;
	}

	currentZone = z;
	keyMarker->SetStartKeysZone(currentZone);


	if (!instant)
	{
		int soundIndex = SoundType::S_KEY_ENTER_0 + (currentZone->requiredKeys);
		soundNodeList->ActivateSound(gameSoundBuffers[soundIndex]);
	}
}

void GameSession::UnlockGate( Gate *g )
{
	g->SetLocked( false );

	cout << "adding gate to unlock list: " << g << endl;
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

	if( currentZone != NULL )
	{
		list<Edge*> &gList = currentZone->gates;
		for( list<Edge*>::iterator it = gList.begin(); it != gList.end(); ++it )
		{
			Gate *gg = (Gate*)(*it)->info;
			if( gg == g || gg->gState == Gate::OPEN || gg->gState == Gate::DISSOLVE )
				continue;

			gg->gState = Gate::LOCKFOREVER;
			//g->SetLocked();
		}
	}
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
		activeSequence = b_crawler->crawlerFightSeq;
		activeSequence->frame = 0;

		assert( b_crawler != NULL );
		b_crawler->spawned = true;
		AddEnemy( b_crawler );

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
	//hadBlueKey = false;

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

	ts_scoreTest = owner->GetTileset( "score_menu_01.png", 1920, 1080 );
	scoreTestSprite.setTexture( *ts_scoreTest->texture );
	scoreTestSprite.setPosition( 0, 0 );

	Tileset *scoreTS = owner->GetTileset( "number_score_80x80.png", 80, 80 );
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

MomentumBar::MomentumBar(GameSession *owner)
{
	ts_bar = owner->GetTileset("momentumbar_105x105.png", 105, 105);
	ts_container = owner->GetTileset("momentumbar_115x115.png", 115, 115);
	ts_num = owner->GetTileset("momentumnum_48x48.png", 48, 48);

	levelNumSpr.setTexture(*ts_num->texture);
	levelNumSpr.setTextureRect(ts_bar->GetSubRect(0));

	teal.setTexture(*ts_bar->texture);
	teal.setTextureRect(ts_bar->GetSubRect(0));

	blue.setTexture(*ts_bar->texture);
	blue.setTextureRect(ts_bar->GetSubRect(1));

	purp.setTexture(*ts_bar->texture);
	purp.setTextureRect(ts_bar->GetSubRect(2));

	container.setTexture(*ts_container->texture);
	container.setTextureRect(ts_container->GetSubRect(0));

	levelNumSpr.setTexture(*ts_num->texture);
	levelNumSpr.setTextureRect(ts_num->GetSubRect(0));

	if (!partShader.loadFromFile("Shader/momentum_shader.frag", sf::Shader::Fragment))
	{
		cout << "momentum bar SHADER NOT LOADING CORRECTLY" << endl;
		assert(0);
	}

	partShader.setUniform("barTex", sf::Shader::CurrentTexture);
}

void MomentumBar::SetTopLeft(sf::Vector2f &pos)
{
	Vector2f extra(5, 5);
	teal.setPosition(pos + extra);
	blue.setPosition(pos + extra);
	purp.setPosition(pos + extra);
	container.setPosition(pos);
	levelNumSpr.setPosition(pos + Vector2f(76, -50));
}

void MomentumBar::SetMomentumInfo(int p_level, float p_part)
{
	level = p_level;
	part = p_part;
	partShader.setUniform("tile", (float)level);
	partShader.setUniform("factor", part);

	container.setTextureRect(ts_container->GetSubRect(level));
	levelNumSpr.setTextureRect(ts_num->GetSubRect(level));
}

void MomentumBar::Draw(sf::RenderTarget *target)
{
	target->draw(container);

	if (level == 0)
	{
		target->draw(teal, &partShader);
	}
	else if (level == 1)
	{
		//target->draw(teal);
		target->draw(blue, &partShader);
	}
	else if (level == 2)
	{
		//target->draw(blue);
		target->draw(purp, &partShader);
	}

	target->draw(levelNumSpr);
}