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
#include "EditorDecorInfo.h"
#include "EditorPlayerTracker.h"

#include "clipper.hpp"

#include "TransformTools.h"

#include "CircleGroup.h"

#include "ItemSelector.h"
#include "EnemyChain.h"

#include "GateMarker.h"
#include "BrushManager.h"
#include "FileChooser.h"
#include "AdventureCreator.h"
#include "AbsorbParticles.h"
#include "HUD.h"
#include "KeyMarker.h"

#include "Fader.h"
#include "Minimap.h"
#include "Sequence.h"
#include "MusicPlayer.h"
#include "Barrier.h"
#include "TopClouds.h"
#include "Enemy_Goal.h"
#include "GGPO.h"
#include "GameMode.h"

//#define GGPO_ON

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

bool EditSession::IsShardCaptured(int sType)
{
	return shardsCapturedField->GetBit(sType);
}

void EditSession::SetupGates()
{
	if (gates.size() > 0)
	{
		for (auto it = gates.begin(); it != gates.end(); ++it)
		{
			(*it)->SetLocked(false);
			delete (*it);
		}
		gates.clear();
	}

	SetNumGates(gateInfoList.size());
	Gate *g;
	for (list<GateInfoPtr>::iterator it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
	{
		g = new Gate(this, (*it)->category, (*it)->variation);
		g->Setup((*it)); //adds gate to gates
	}
}

void EditSession::SetTrackingEnemy(ActorType *type, int level)
{
	if (trackingEnemyParams == NULL)
	{
		//cout << "copy of level : " << level << endl;
		trackingEnemyParams = type->defaultParamsVec[level-1]->Copy();
		trackingEnemyParams->group = groups["--"];
		//GetPolygon((0);

		//trackingEnemyParams->AnchorToGround();
		trackingEnemyParams->CreateMyEnemy();
		grabbedActor = trackingEnemyParams;
		SelectObject(grabbedActor);

		trackingEnemyParams->MoveTo(Vector2i(worldPos));
		//extraDelta = Vector2i(worldPos) - Vector2i(grabCenter);
		if (grabbedActor->myEnemy != NULL)
		{
			grabbedActor->myEnemy->UpdateFromEditParams(0);
		}
		

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
		assert(0);
	}
}

void EditSession::SetTrackingDecor(DecorPtr dec)
{
	if (trackingDecor == NULL)
	{
		ClearSelectedBrush();
		//cout << "copy of level : " << level << endl;
		trackingDecor = dec;

		//trackingEnemyParams->group = groups["--"];
		//GetPolygon((0);

		//trackingEnemyParams->AnchorToGround();
		//trackingEnemyParams->CreateMyEnemy();
		grabbedImage = trackingDecor;
		SelectObject(grabbedImage);

		grabbedImage->MoveTo(Vector2i(worldPos));

		editMouseGrabPos = Vector2i(worldPos.x, worldPos.y);
		pointGrabPos = Vector2i(worldPos.x, worldPos.y);
		editMouseOrigPos = editMouseGrabPos;

		editMouseDownMove = true;
		editStartMove = false;
		editMouseDownBox = false;

		createDecorModeUI->SetShown(false);
	}
	else
	{
		int x = 5;
		assert(0);
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

V2d EditSession::GetPlayerSpawnPos( int i )
{
	if (mapHeader == NULL || mapHeader->GetNumPlayers() == 1)
	{
		if (HoldingControl())
		{
			return worldPos;
		}
		else
		{
			return playerMarkers[0]->GetPosition();
		}
	}
	else
	{
		return playerMarkers[i]->GetPosition();
	}
}

void EditSession::ClearSelectedBrush()
{
	editModeUI->SetEnemyPanel(NULL);
	selectedBrush->SetSelected(false);
	selectedBrush->Clear();
	ClearSelectedPoints();
	grabbedActor = NULL;
	grabbedObject = NULL;
	grabbedPoint = NULL;
	grabbedImage = NULL;
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

void EditSession::DeselectObjectType(ISelectable::ISelectableType sType)
{
	for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end();)
	{
		if ((*it)->selectableType == sType)
		{
			(*it)->SetSelected(false);
			it = selectedBrush->objects.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void EditSession::DeselectActorType(const std::string &typeName)
{
	for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end();)
	{
		if ((*it)->selectableType == ISelectable::ACTOR)
		{
			ActorPtr a = (*it)->GetAsActor();
			if (a->GetTypeName() == typeName )
			{
				(*it)->SetSelected(false);
				it = selectedBrush->objects.erase(it);
			}
		}
		else
		{
			++it;
		}
	}
}

bool EditSession::UpdateRunModeBackAndStartButtons()
{
	Actor *pTemp;
	if (GetCurrInput(0).start && !GetPrevInput(0).start)
	{
		SetMode(EDIT);
		/*if (gameCam)
		{
		SetZoom(2);
		}*/

		pTemp = GetPlayer(0);

		if (pTemp->ground != NULL) //doesn't work with bounce or grind
		{
			playerTracker->SetOldTrackPos(pTemp->ground->GetPosition(pTemp->edgeQuantity),
				pTemp->position);
		}
		//bounce and grind also later

		playerTracker->SetOn(true);
		playerTracker->CalcShownCircles();
		return true;
	}

	return false;
}

void EditSession::UpdateCamera()
{
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
		break;
	}
	}
	/*if ( != NULL)
	{
	cam.UpdateVS(GetPlayer(0), GetPlayer(1));
	}
	else*/
	//{
	//	if (ggpo == NULL)
	//	{
	//		cam.SetCamType(Camera::CamType::BASIC);
	//		cam.playerIndex = 0;
	//		cam.Update();
	//	}
	//	else
	//	{
	//		cam.SetCamType(Camera::CamType::FIGHTING);
	//		int handle = ngs->local_player_handle - 1;
	//		if (handle < 0)
	//			handle = 0; //sync test
	//		cam.playerIndex = handle;
	//		cam.Update();
	//	}
	//}

	Vector2f camPos = cam.GetPos();
	double camWidth = 960 * cam.GetZoom();
	double camHeight = 540 * cam.GetZoom();
	screenRect = sf::Rect<double>(camPos.x - camWidth / 2, camPos.y - camHeight / 2, camWidth, camHeight);

	if (gameCam)
	{

		view.setSize(Vector2f(960 * cam.GetZoom(), 540 * cam.GetZoom()));

		//this is because kin's sprite is 2x size in the game as well as other stuff
		//lastViewSize = view.getSize();
		view.setCenter(camPos.x, camPos.y);
	}
}

bool EditSession::RunPostUpdate()
{
	if (mode != TEST_PLAYER)
		return false;
	else
	{
		if (totalGameFrames % 3 == 0)
		{
			playerTracker->TryAddTrackPoint(GetPlayerPos(0));
		}
	}

	return true;
}

void EditSession::SequenceGameModeRespondToGoalDestroyed()
{
	EndTestMode();
}

bool EditSession::GGPOTestPlayerModeUpdate()
{
	switchGameState = false;
	double newTime = gameClock.getElapsedTime().asSeconds();
	double frameTime = newTime - currentTime;
	
	//if (frameTime < TIMESTEP)
	{
		ggpo_idle(ggpo, 5);
		//int fTime = (TIMESTEP - frameTime);
		//ggpo_idle(ggpo, fTime- .001);
	}

	currentTime = newTime;

	frameRateDisplay.Update(frameTime);
	UpdateRunningTimerText();

	accumulator += frameTime;

	if ( accumulator >= TIMESTEP && timeSyncFrames > 0)
	{
		--timeSyncFrames;
		accumulator -= TIMESTEP;
	}
	else
	{
		while (accumulator >= TIMESTEP)
		{
			GGPORunFrame();
			UpdateNetworkStats();
			accumulator -= TIMESTEP;
		}
	}

	//now = timeGetTime();
	//ggpo_idle(ggpo, 5);//max(0, next - now - 1));
	//if (now >= next)
	{
		//GGPORunFrame();
		//next = now + (1000 / 60);
	}
	//accumulator += frameTime;
	//while (accumulator >= frameTime)
	{
		
	}
	
	//GGPORunGameModeUpdate();

	return true;
}

bool EditSession::TestPlayerModeUpdate()
{
	switchGameState = false;
	double newTime = gameClock.getElapsedTime().asSeconds();
	double frameTime = newTime - currentTime;

	if (frameTime > 0.25)
	{
		frameTime = 0.25;
	}
	currentTime = newTime;

	frameRateDisplay.Update(frameTime);
	UpdateRunningTimerText();

	accumulator += frameTime;

	switch (gameState)
	{
	case RUN:
		if (!RunGameModeUpdate())
		{
			return false; //loop the function again
		}
		break;
	case FROZEN:
		if (!FrozenGameModeUpdate())
		{
			return false; //loop the function again
		}
		break;
	case SEQUENCE:
		if (!SequenceGameModeUpdate())
		{
			return false;
		}
		break;
	}

	return true;
}

void EditSession::SetupGGPOStatsPanel()
{
	ggpoStatsPanel = new Panel("stats", 500, 300, this, false);

	ggpoStatsPanel->SetAutoSpacing(false, true, Vector2i(10, 10), Vector2i(0, 20));
	ggpoStatsPanel->AddLabel("kbps_sent", Vector2i(), 28, "kbps_sent:");
	ggpoStatsPanel->AddLabel("ping", Vector2i(), 28, "ping:");
	ggpoStatsPanel->AddLabel("recv", Vector2i(), 28, "recv_queue_len:");
	ggpoStatsPanel->AddLabel("send", Vector2i(), 28, "send_queue_len:");
	ggpoStatsPanel->AddLabel("local", Vector2i(), 28, "local_frames_behind:");
	ggpoStatsPanel->AddLabel("remote", Vector2i(), 28, "remote_frames_behind:");
}

void EditSession::UpdateNetworkStats()
{
	if (ggpo != NULL)
	{
		int remoteIndex = 0;
		if( ngs->local_player_handle == 1 )
		{ 
			remoteIndex = 1;
		}
		else if (ngs->local_player_handle == 2)
		{
			remoteIndex = 0;
		}
		GGPONetworkStats ns;
		ggpo_get_network_stats(ggpo, ngs->playerInfo[remoteIndex].handle, &ns);

		ggpoStatsPanel->labels["kbps_sent"]->setString( "kbps_sent: " + to_string(ns.network.kbps_sent));
		ggpoStatsPanel->labels["ping"]->setString( "ping: " + to_string(ns.network.ping));
		ggpoStatsPanel->labels["recv"]->setString( "recv_queue_len: " + to_string(ns.network.recv_queue_len));
		ggpoStatsPanel->labels["send"]->setString( "send_queue_len: " + to_string(ns.network.send_queue_len));
		ggpoStatsPanel->labels["local"]->setString( "local_frames_behind: " + to_string(ns.timesync.local_frames_behind));
		ggpoStatsPanel->labels["remote"]->setString("remote_frames_behind: " + to_string(ns.timesync.remote_frames_behind));
	}
}

void EditSession::InitGGPO()
{
	timeSyncFrames = 0;
	//srand(400);
	srand(time(0));
	WSADATA wd = { 0 };
	WSAStartup(MAKEWORD(2, 2), &wd);

	GGPOSessionCallbacks cb = { 0 };
	cb.begin_game = begin_game_callback;
	cb.advance_frame = advance_frame_callback;
	cb.load_game_state = load_game_state_callback;
	cb.save_game_state = save_game_state_callback;
	cb.free_buffer = free_buffer;
	cb.on_event = on_event_callback;
	cb.log_game_state = log_game_state;

	/*for (int i = 0; i < 10; ++i)
	{
		saveStates[i] = new SaveGameState;
		usedSaveState[i] = false;
	}*/

	currSaveState = new SaveGameState;
	ngs = new GGPONonGameState;
	ggpoPlayers = new GGPOPlayer[4];

	GGPOErrorCode result;

	unsigned short localPort = 7000;
	unsigned short otherPort = 7001;

	bool shift = HoldingShift();
	if (shift)
	{
		localPort = 7001;
		otherPort = 7000;
	}

	int frameDelay = 0;
	string ipStr;// = "127.0.0.1";

	ifstream is;
	is.open("Resources/ggpotest.txt");
	is >> frameDelay;
	is >> ipStr;

	int sync;
	is >> sync;


	//int offset = 1, local_player = 0;
	int num_players = 2;
	ngs->num_players = num_players;

	
	if (sync)
	{
		result = ggpo_start_synctest(&ggpo, &cb, "bn", num_players,
			sizeof(int), 1);
	}
	else
	{
		result = ggpo_start_session(&ggpo, &cb, "vectorwar", num_players,
			sizeof(int), localPort);
	}
	
	

	//ggpo_log(ggpo, "test\n");
	//result = ggpo_start_session(&ggpo, &cb, "vectorwar", num_players, sizeof(int), localport);
	ggpo_set_disconnect_timeout(ggpo, 0); //3000
	ggpo_set_disconnect_notify_start(ggpo, 1000);
	int myIndex = 0;
	int otherIndex = 1;
	if (shift)
	{
		myIndex = 1;
		otherIndex = 0;
	}

	ggpoPlayers[myIndex].size = sizeof(ggpoPlayers[myIndex]);
	ggpoPlayers[myIndex].player_num = myIndex + 1;
	ggpoPlayers[otherIndex].size = sizeof(ggpoPlayers[otherIndex]);
	ggpoPlayers[otherIndex].player_num = otherIndex + 1;
	ggpoPlayers[myIndex].type = GGPO_PLAYERTYPE_LOCAL;
	ggpoPlayers[otherIndex].type = GGPO_PLAYERTYPE_REMOTE;
//	local_player = myIndex;
	
	int ipLen = ipStr.length();
	for (int i = 0; i < ipLen; ++i)
	{
		ggpoPlayers[otherIndex].u.remote.ip_address[i] = ipStr[i];
	}
	ggpoPlayers[otherIndex].u.remote.ip_address[ipLen] = '\0';

	//ggpoPlayers[otherIndex].u.remote.ip_address = ipStr.c_str();
	ggpoPlayers[otherIndex].u.remote.port = otherPort;

	int i;
	for (i = 0; i < num_players; i++) {
		GGPOPlayerHandle handle;
		result = ggpo_add_player(ggpo, ggpoPlayers + i, &handle);
		ngs->playerInfo[i].handle = handle;
		ngs->playerInfo[i].type = ggpoPlayers[i].type;
		if (ggpoPlayers[i].type == GGPO_PLAYERTYPE_LOCAL) {
			ngs->playerInfo[i].connect_progress = 100;
			ngs->local_player_handle = handle;
			ngs->SetConnectState(handle, Connecting);
			ggpo_set_frame_delay(ggpo, handle, frameDelay);
		}
		else {
			ngs->playerInfo[i].connect_progress = 0;
		}
	}
}

void EditSession::TestPlayerMode()
{
	if (mode != TEST_PLAYER)
	{
#ifdef GGPO_ON
		InitGGPO();
#endif
		
		gameMode->Setup();
	}
	
	//----------------------------------------
	
	for (int i = 0; i < 3; ++i)
	{
		auto & pList = GetCorrectPolygonList(i);
		for (auto it = pList.begin(); it != pList.end(); ++it)
		{
			if ((*it)->renderMode == TerrainPolygon::RENDERMODE_TRANSFORM)
			{
				(*it)->CancelTransformation();
			}
			
		}
	}

	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		(*it)->CancelTransformation();
	}

	scoreDisplay->Reset();

	gameState = Session::RUN;
	cam.Reset();
	currStorySequence = NULL;
	frameRateDisplay.Reset();
	currSuperPlayer = NULL;
	soundNodeList->Reset();

	SetPlayerInputOn(true);

	fader->Reset();
	swiper->Reset();
	
	ClearEmitters();
	ClearEffects();
	ResetAbsorbParticles();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (GetPlayer(i) != NULL)
		{
			SetPlayerOptionField(i);
		}
	}
	skipped = false;
	oneFrameMode = false;

	accumulator = TIMESTEP + .1;

	activeSequence = NULL;

	totalGameFrames = 0;
	totalFramesBeforeGoal = -1;

	Actor *p;

	bool continueTracking = (GetCurrInput(0).start && playerTracker->IsTrackStarted() && mode != TEST_PLAYER);

	if (continueTracking)
	{
		//eventually have a flag that the terrain has been modified, which you change when pushing
		//done actions onto the stack. if they haven't been modified, theres no reason to 
		//try to attach the player or even rebuild the trees.

		p = GetPlayer(0);
		if (p->ground != NULL)
		{
			if (!TryAttachPlayerToPolys(playerTracker->playerOldGroundTrackPos, p->offsetX))
			{
				p->SetAirPos(playerTracker->playerOldTrackPos, p->facingRight);
			}
		}
	}

	

	
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			if (!continueTracking)
			{
				p->Respawn();
			}
		}
	}

	

	if (mode == TEST_PLAYER)
	{	
		for (auto it = barriers.begin(); it != barriers.end(); ++it)
		{
			(*it)->Reset();
		}

		//GetPlayer(0)->Respawn();
		Actor *p;
		//gameClock.restart();

		/*for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			p = GetPlayer(i);
			if (p != NULL)
			{
				
				p->Respawn();
				p->velocity = worldPos - oldWorldPosTest;
			}
			
		}*/

		auto testPolys = GetCorrectPolygonList(0);
		for (auto it = testPolys.begin(); it != testPolys.end(); ++it)
		{
			(*it)->ResetTouchGrass();
			(*it)->ResetState();
		}

		
		Enemy *curr = activeEnemyList;
		while (curr != NULL)
		{
			Enemy *next = curr->next;
			RemoveEnemy(curr);
			curr = next;
		}
		
		ResetZones();

		ResetGates();

		currentZone = NULL;
		if (originalZone != NULL)
		{
			currentZoneNode = zoneTree;
			ActivateZone(originalZone, true);
			//gateMarkers->SetToZone(currentZone);
			
			AdventureHUD *ah = GetAdventureHUD();
			if( ah != NULL ) ah->keyMarker->Reset();
			
		}

		/*for (auto it = groups.begin(); it != groups.end(); ++it)
		{
			for (auto enit = (*it).second->actors.begin(); enit != (*it).second->actors.end(); ++enit)
			{
				if ((*enit)->myEnemy != NULL)
				{
					RemoveEnemy((*enit)->myEnemy);
				}
			}
		}*/

		for (auto it = groups.begin(); it != groups.end(); ++it)
		{
			for (auto enit = (*it).second->actors.begin(); 
				enit != (*it).second->actors.end(); ++enit)
			{
				if ((*enit)->myEnemy != NULL)
				{
					(*enit)->myEnemy->Reset();
					//AddEnemy((*enit)->myEnemy);
				}
			}
		}

		for (auto it = rails.begin(); it != rails.end(); ++it)
		{
			if ((*it)->enemyChain != NULL)
			{
				(*it)->enemyChain->Reset();
				//AddEnemy((*it)->enemyChain);
			}
			else
			{
				(*it)->ResetState();
			}
		}

		if (preLevelScene != NULL)
		{
			SetActiveSequence(preLevelScene);
		}
		else if (shipEnterScene != NULL)
		{
			shipEnterScene->Reset();
			SetActiveSequence(shipEnterScene);
		}

		gameMode->StartGame();
		
		return;
	}

	SetMode(TEST_PLAYER);
	ClearSelectedPoints();
	ClearSelectedBrush();
	
	pauseFrames = 0;

	

	if (terrainTree != NULL)
	{
		terrainTree->Clear();
		specialTerrainTree->Clear();
		borderTree->Clear();
		grassTree->Clear();
		activeItemTree->Clear();
		railEdgeTree->Clear();
		gateTree->Clear();
		enemyTree->Clear();
		railDrawTree->Clear();
		specterTree->Clear();
		

		Enemy *currEnemy;
		for (auto it = groups.begin(); it != groups.end(); ++it)
		{
			for (auto enit = (*it).second->actors.begin(); enit != (*it).second->actors.end(); 
				++enit)
			{
				currEnemy = (*enit)->myEnemy;
				if ( currEnemy != NULL)
				{
					currEnemy->Reset();
					RemoveEnemy(currEnemy);
				}
			}
		}

		for (auto it = rails.begin(); it != rails.end(); ++it)
		{
			if ((*it)->enemyChain != NULL)
			{
				(*it)->enemyChain->Reset();
				RemoveEnemy((*it)->enemyChain);
			}
		}
		
		//reset enemies
	}
	else
	{
		terrainTree = new QuadTree(1000000, 1000000);
		specialTerrainTree = new QuadTree(1000000, 1000000);
		borderTree = new QuadTree(1000000, 1000000);
		grassTree = new QuadTree(1000000, 1000000);

		railEdgeTree = new QuadTree(1000000, 1000000);
		barrierTree = new QuadTree(1000000, 1000000);

		staticItemTree = new QuadTree(1000000, 1000000);

		activeItemTree = new QuadTree(1000000, 1000000);

		gateTree = new QuadTree(1000000, 1000000);

		enemyTree = new QuadTree(1000000, 1000000);

		railDrawTree = new QuadTree(1000000, 1000000);

		specterTree = new QuadTree(1000000, 1000000);
		//Actor *p;
		
		/*for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			p = GetPlayer(i);
			if (p != NULL)
				p->SetToOriginalPos();
		}*/

		
	}

	if (bigBulletVA != NULL)
	{
		delete[] bigBulletVA;
		bigBulletVA = NULL;
	}

	totalNumberBullets = 0;
	Enemy *currEnemy;
	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		for (auto enit = (*it).second->actors.begin(); enit != (*it).second->actors.end(); ++enit)
		{
			currEnemy = (*enit)->myEnemy;
			if (currEnemy != NULL)
			{
				totalNumberBullets = currEnemy->SetLaunchersStartIndex(totalNumberBullets);
			}
		}
	}

	numSimulatedFramesRequired = 0;
	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		for (auto enit = (*it).second->actors.begin(); enit != (*it).second->actors.end(); ++enit)
		{
			currEnemy = (*enit)->myEnemy;
			if (currEnemy != NULL)
			{
				numSimulatedFramesRequired = max(
					numSimulatedFramesRequired, currEnemy->GetNumSimulationFramesRequired());
			}
		}
	}

	CreateBulletQuads();
	/*for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		for (auto enit = (*it).second->actors.begin(); enit != (*it).second->actors.end(); ++enit)
		{
			currEnemy = (*enit)->myEnemy;
			if (currEnemy != NULL)
			{
				totalNumberBullets = currEnemy->SetLaunchersStartIndex(totalNumberBullets);
			}
		}
	}*/
	
	auto testPolys = GetCorrectPolygonList(0);
	for (auto it = testPolys.begin(); it != testPolys.end(); ++it)
	{
		(*it)->ResetTouchGrass();
		(*it)->ResetState();
	}

	//auto &testPolys = GetCorrectPolygonList(0);
	for (auto it = testPolys.begin(); it != testPolys.end(); ++it)
	{
		borderTree->Insert((*it));
		(*it)->AddEdgesToQuadTree(terrainTree);
		(*it)->AddGrassToQuadTree(grassTree);
	}

	auto &testPolys1 = GetCorrectPolygonList(1);
	for (auto it = testPolys1.begin(); it != testPolys1.end(); ++it)
	{
		specialTerrainTree->Insert((*it));
	}

	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		
		if ((*it)->enemyChain != NULL)
		{
			(*it)->AddEnemyChainToWorldTrees();
			enemyTree->Insert((*it)->enemyChain);
		}
		else
		{
			(*it)->ResetState();
			(*it)->AddEdgesToQuadTree(railEdgeTree);
			railDrawTree->Insert((*it));
		}
	}

	fullEnemyList.clear();
	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		for (auto enit = (*it).second->actors.begin(); enit != (*it).second->actors.end(); ++enit)
		{
			currEnemy = (*enit)->myEnemy;
			if (currEnemy != NULL)
			{
				currEnemy->AddToGame();
			}
		}
	}

	

	/*for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		for (auto enit = (*it).second->actors.begin(); enit != (*it).second->actors.end(); ++enit)
		{
			currEnemy = (*enit)->myEnemy;
			if (currEnemy != NULL)
			{
				AddEnemy(currEnemy);
			}
		}
	}

	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		if ((*it)->enemyChain != NULL)
		{
			AddEnemy((*it)->enemyChain);
		}
	}*/

	if (continueTracking)
	{
		currentTime = gameClock.getElapsedTime().asSeconds();
	}

	SetupGates();

	CleanupZones();
	CreateZones();

	int setupZoneStatus = SetupZones();
	

	SetupGateMarkers();

	if (originalZone != NULL)
	{
		if (gateMarkers != NULL)
		{
			gateMarkers->SetToZone(originalZone);
		}
	}

	hasGoal = false;

	bool foundShipEnter = false;
	bool foundShipExit = false;

	CleanupCameraShots();
	CleanupPoi();
	CleanupBossNodes();
	CleanupBarriers();

	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		for (auto enit = (*it).second->actors.begin(); enit != (*it).second->actors.end(); ++enit)
		{
			if ((*enit)->type == types["xbarrier"])
			{
				XBarrierParams *xbp = (XBarrierParams*)(*enit);
				AddBarrier(xbp, false);
			}
			else if ((*enit)->type == types["xbarrierwarp"])
			{
				XBarrierParams *xbp = (XBarrierParams*)(*enit);
				AddBarrier(xbp, true);
			}
			else if ((*enit)->type == types["camerashot"])
			{
				CameraShotParams *csp = (CameraShotParams*)(*enit);
				//tempCameraShots.push_back(csp);
				AddCameraShot(csp);
			}
			else if ((*enit)->type == types["poi"])
			{
				PoiParams *pp = (PoiParams*)(*enit);
				//tempPoiParams.push_back(pp);
				AddPoi(pp);
			}
			else if ( (*enit)->type == types["crawlernode"]
				|| (*enit)->type == types["birdnode"]
				|| (*enit)->type == types["coyotenode"]
				|| (*enit)->type == types["tigernode"]
				|| (*enit)->type == types["gatornode"] 
				|| (*enit)->type == types["skeletonnode"]
				|| (*enit)->type == types["greyskeletonnode"]
				|| (*enit)->type == types["bearnode"])
			{
				PoiParams *pp = (PoiParams*)(*enit);
				AddBossNode((*enit)->GetTypeName(), pp);
			}
			else if ((*enit)->type == types["ship"])
			{
				if (shipEnterScene == NULL)
				{
					shipEnterScene = new ShipEnterScene;
					shipEnterScene->Init();
				}
				foundShipEnter = true;
				shipEnterScene->shipEntrancePos = (*enit)->GetPosition();

			}
			else if (((*enit)->type == types["shippickup"]))
			{
				if (shipExitScene == NULL)
				{
					shipExitScene = new ShipExitScene;
					shipExitScene->Init();
				}
				foundShipExit = true;
			}
			else if ((*enit)->type == types["goal"] && !hasGoal )
			{
				Goal *g = (Goal*)((*enit)->myEnemy);
				g->SetMapGoalPos();
			}
		}
	}

	for (auto it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		(*it)->Setup();
	}


	/*for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		for (auto enit = (*it).second->actors.begin(); enit != (*it).second->actors.end(); ++enit)
		{
			currEnemy = (*enit)->myEnemy;
			if (currEnemy != NULL)
			{
				currEnemy->Setup();

			}
		}
	}*/

	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		if ((*it)->enemyChain != NULL)
		{
			//(*it)->enemyChain->Reset();
			(*it)->enemyChain->Setup();
		}
	}

	if (mapHeader->preLevelSceneName != "NONE")
	{
		if (preLevelSceneName != mapHeader->preLevelSceneName)
		{
			CleanupPreLevelScene();
			preLevelScene = Sequence::CreateScene(mapHeader->preLevelSceneName);
		}
	}
	else
	{
		CleanupPreLevelScene();
	}

	if (!foundShipEnter && shipEnterScene != NULL)
	{
		CleanupShipEntrance();
	}

	if (!foundShipExit && shipExitScene != NULL)
	{
		CleanupShipExit();
	}

	//an issue right now is that these load tilesets that do not get deleted
	//might not get deleted ever! need to at least get rid of them when reloading.
	SetupBarrierScenes();

	SetupEnemyZoneSprites();

	CleanupGlobalBorders();

	bool blackBorder[2];
	bool topBorderOn = false;

	realLeftBounds = mapHeader->leftBounds;
	realTopBounds = mapHeader->topBounds;
	realBoundsWidth = mapHeader->boundsWidth;
	realBoundsHeight = mapHeader->boundsHeight;

	SetupGlobalBorderQuads(blackBorder, topBorderOn);

	CleanupTopClouds();
	if (topBorderOn)
	{
		topClouds = new TopClouds;
		topClouds->SetToHeader();
	}

	if (hasGoal)
	{
		SetupGoalFlow();
		SetupGoalPulse();
	}


	if( hud != NULL )
		hud->mini->SetupBorderQuads(blackBorder, topBorderOn, mapHeader);

	GetPlayer(0)->SetupDrain();

	
	if (preLevelScene != NULL)
	{
		SetActiveSequence(preLevelScene);
	}
	else if (shipEnterScene != NULL)
	{
		shipEnterScene->Reset();
		SetActiveSequence(shipEnterScene);
	}

	gameMode->StartGame();

	if (!continueTracking)
	{
		playerTracker->Reset();
		totalGameFrames = 0;
		totalFramesBeforeGoal = -1;
		currentTime = 0;
		gameClock.restart();
		//accumulator = TIMESTEP + .1;
	}
	else
	{
		int x = 5;
	}

	if (setupZoneStatus == -1) //couldnt find goal in a zone based map
	{
		SetMode(EDIT);
		CreateError(ERR_CANT_MAKE_ZONE_STRUCTURE_WITHOUT_GOAL);
		ShowMostRecentError();
		return;
	}
}

void EditSession::EndTestMode()
{
	
	SetMode(EDIT);
}

void EditSession::ProcessDecorFromFile(const std::string &name,
	int tile)
{
	decorTileIndexMap[name].push_back(tile);
}

EditSession *EditSession::GetSession()
{
	return currSession;
}

void EditSession::HandleEventFunc(int m)
{
	if (handleEventFunctions.find(m) != handleEventFunctions.end())
	{
		(this->*handleEventFunctions[m])();
	}
}

void EditSession::LoseFocusFunc(int m)
{
	if (loseFocusFunctions.find(m) != loseFocusFunctions.end())
	{
		(this->*loseFocusFunctions[m])();
	}
}

void EditSession::UpdateModeFunc(int m)
{
	if (m == TEST_PLAYER)
	{
		while (true )
		{

#ifdef GGPO_ON
			if (GGPOTestPlayerModeUpdate())
			{
				break;
			}
#else
			if (TestPlayerModeUpdate())
			{
				break;
			}
#endif

			
		}
		return;
	}

	if (updateModeFunctions.find(m) != updateModeFunctions.end())
	{
		(this->*updateModeFunctions[m])();
	}
}

EditSession::EditSession( MainMenu *p_mainMenu, const boost::filesystem::path &p_filePath)
	:Session( Session::SESS_EDIT, p_filePath ), mainMenu( p_mainMenu ), arial( p_mainMenu->arial ),
	errorBar(p_mainMenu->arial)
{	
	currSession = this;

	handleEventFunctions[CREATE_TERRAIN] = &EditSession::CreateTerrainModeHandleEvent;
	handleEventFunctions[EDIT] = &EditSession::EditModeHandleEvent;
	handleEventFunctions[SELECT_MODE] = &EditSession::SelectModeHandleEvent;
	handleEventFunctions[CREATE_PATROL_PATH] = &EditSession::CreatePatrolPathModeHandleEvent;
	handleEventFunctions[CREATE_RECT] = &EditSession::CreateRectModeHandleEvent;
	handleEventFunctions[SET_DIRECTION] = &EditSession::SetDirectionModeHandleEvent;
	handleEventFunctions[PASTE] = &EditSession::PasteModeHandleEvent;
	//handleEventFunctions[PAUSED] = &EditSession::PausedModeHandleEvent;
	handleEventFunctions[CREATE_ENEMY] = &EditSession::CreateEnemyModeHandleEvent;
	handleEventFunctions[CREATE_GATES] = &EditSession::CreateGatesModeHandleEvent;
	handleEventFunctions[CREATE_IMAGES] = &EditSession::CreateImagesModeHandleEvent;
	handleEventFunctions[CREATE_RAILS] = &EditSession::CreateRailsModeHandleEvent;
	handleEventFunctions[SET_CAM_ZOOM] = &EditSession::SetCamZoomModeHandleEvent;
	handleEventFunctions[TEST_PLAYER] = &EditSession::TestPlayerModeHandleEvent;
	handleEventFunctions[TRANSFORM] = &EditSession::TransformModeHandleEvent;
	handleEventFunctions[MOVE_BORDER] = &EditSession::MoveBorderModeHandleEvent;

	updateModeFunctions[CREATE_TERRAIN] = &EditSession::CreateTerrainModeUpdate;
	updateModeFunctions[EDIT] = &EditSession::EditModeUpdate;
	updateModeFunctions[SELECT_MODE] = &EditSession::SelectModeUpdate;
	updateModeFunctions[CREATE_PATROL_PATH] = &EditSession::CreatePatrolPathModeUpdate;
	updateModeFunctions[CREATE_RECT] = &EditSession::CreateRectModeUpdate;
	updateModeFunctions[SET_DIRECTION] = &EditSession::SetDirectionModeUpdate;
	updateModeFunctions[PASTE] = &EditSession::PasteModeUpdate;
	//updateModeFunctions[PAUSED] = &EditSession::PauseModeUpdate;
	updateModeFunctions[CREATE_ENEMY] = &EditSession::CreateEnemyModeUpdate;
	updateModeFunctions[CREATE_GATES] = &EditSession::CreateGatesModeUpdate;
	updateModeFunctions[CREATE_IMAGES] = &EditSession::CreateImagesModeUpdate;
	updateModeFunctions[CREATE_RAILS] = &EditSession::CreateRailsModeUpdate;
	updateModeFunctions[SET_CAM_ZOOM] = &EditSession::SetCamZoomModeUpdate;
	//updateModeFunctions[TEST_PLAYER] = &EditSession::TestPlayerModeUpdate;
	updateModeFunctions[TRANSFORM] = &EditSession::TransformModeUpdate;
	updateModeFunctions[MOVE_BORDER] = &EditSession::MoveBorderModeUpdate;

	ggpoStatsPanel = NULL;
	currGrassType = 0;

	int waitFrames[] = { 30, 2 };
	int waitModeThresh[] = { 1 };
	removeProgressPointWaiter = new FrameWaiter(2, waitFrames, 1, waitModeThresh );
	
	variationSelector = new EnemyVariationSelector( false );
	playerTracker = new PlayerTracker();
	runToResave = false;

	gameCam = false;
	brushManager = NULL;

	boxToolColor = Color(255, 0, 0, 50);
	SetRectColor(boxToolQuad, boxToolColor);

	createEnemyModeUI = NULL;
	createDecorModeUI = NULL;
	createTerrainModeUI = NULL;
	enemyEdgePolygon = NULL;
	moveAction = NULL;
	LoadPolyShader();

	SetupWaterShaders();
	
	

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

	mainMenu->SetCurrProfileByName("Custom 1");
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
	
	SetupScoreDisplay();

	minZoom = .25 / 16.0;//.25;
	maxZoom = 65536;

	copiedBrush = NULL;
	freeActorCopiedBrush = NULL;
	
	//arialFont.loadFromFile( "Breakneck_Font_01.ttf" );
	cursorLocationText.setFont( mainMenu->arial );
	cursorLocationText.setCharacterSize( 16 );
	cursorLocationText.setFillColor( Color::White );
	cursorLocationText.setPosition( 1920 - 100, 1080 - 100 );
	
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
	brushPreviewTex = MainMenu::brushPreviewTexture;


	for( int i = 0; i < 12; ++i )
	{
		fullBounds[i].color = COLOR_ORANGE;
		fullBounds[i].position = Vector2f( 0, 0 );
	}
	grabbedObject = NULL;
	grabbedActor = NULL;
	grabbedPoint = NULL;
	grabbedImage = NULL;
	zoomMultiple = 1;
	editMouseDownBox = false;
	editMouseDownMove = false;
	editMoveThresh = 5;
	editStartMove = false;
	//adding 5 for random distance buffer
	
	//minAngle = .99;
	progressBrush = new Brush();
	selectedBrush = new Brush();
	mapStartBrush = new Brush();
	moveActive = false;

	

	
	

	grassSize = 128;//64;
	grassSpacing = -60;//-40;//-20;//-10;

	//extras

	//monitor,level, path, loop, 

	RegisterAllEnemies();

	Init();
}

PolyPtr EditSession::GetPolygon(int index )
{
	Brush *currLoadingBrush = brushManager->currLoadingBrush;
	if (currLoadingBrush != NULL)
	{
		int counter = 0;
		for (auto it = currLoadingBrush->objects.begin(); it != currLoadingBrush->objects.end(); ++it)
		{
			if( (*it)->selectableType == ISelectable::TERRAIN )
			{
				if (counter == index)
				{
					return (*it)->GetAsTerrain();
				}
				else
				{
					++counter;
				}
			}
		}
		return NULL;
	}


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

void EditSession::UpdateTerrainStates()
{
	auto testPolys = GetCorrectPolygonList(0);
	for (auto it = testPolys.begin(); it != testPolys.end(); ++it)
	{
		(*it)->UpdateState();
	}
}

void EditSession::UpdateRailStates()
{
	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		if ((*it)->enemyChain == NULL)
		{
			(*it)->UpdateState();
		}
	}
}


void EditSession::CleanupForReload()
{
	if (mode == TEST_PLAYER)
	{
		CleanupTestPlayerMode();
	}

	for (auto it = doneActionStack.begin(); it != doneActionStack.end(); ++it)
	{
		delete (*it);
	}
	doneActionStack.clear();

	for (auto it = undoneActionStack.begin(); it != undoneActionStack.end(); ++it)
	{
		delete (*it);
	}
	undoneActionStack.clear();

	DestroyCopiedBrushes();

	mapStartBrush->Destroy();

	gateInfoList.clear();

	inversePolygon = NULL;
	polygons.clear();
	waterPolygons.clear();
	flyPolygons.clear();

	if (background != NULL)
	{
		delete background;
		background = NULL;
	}

	ClearEffects();

	CleanupTopClouds();

	DestroyTilesetCategory(TilesetCategory::C_STORY);
}

EditSession::~EditSession()
{
	for (int i = 0; i < 4; ++i)
	{
		players[i] = allPlayers[i];
	}

	delete removeProgressPointWaiter;

	delete variationSelector;

	delete playerTracker;

	delete transformTools;

	delete graph;

	delete confirmPopup;

	delete brushManager;

	delete fileChooser;

	delete adventureCreator;

	delete polygonInProgress;
	delete railInProgress;

	delete progressBrush;
	delete selectedBrush;

	for (auto it = doneActionStack.begin(); it != doneActionStack.end(); ++it)
	{
		delete (*it);
	}
	doneActionStack.clear();

	for (auto it = undoneActionStack.begin(); it != undoneActionStack.end(); ++it)
	{
		delete (*it);
	}
	undoneActionStack.clear();

	DestroyCopiedBrushes();

	mapStartBrush->Destroy();

	if (createEnemyModeUI != NULL)
	{
		delete createEnemyModeUI;
	}

	if (createDecorModeUI != NULL)
	{
		delete createDecorModeUI;
	}


	if (createGatesModeUI != NULL)
	{
		delete createGatesModeUI;
	}

	if (createTerrainModeUI != NULL)
	{
		delete createTerrainModeUI;
	}
	
	if (createRailModeUI != NULL)
	{
		delete createRailModeUI;
	}

	if (editModeUI != NULL)
	{
		delete editModeUI;
	}

	if (mapOptionsUI != NULL)
	{
		delete mapOptionsUI;
	}

	if (generalUI != NULL)
	{
		delete generalUI;
	}

	delete matTypePanel;
	delete shardTypePanel;
	delete nameBrushPanel;
	delete newMapPanel;

	//mapStartBrush->Destroy();

	delete mapStartBrush;

	for (auto it = allPopups.begin(); it != allPopups.end(); ++it)
	{
		delete (*it);
	}

	if (ggpoStatsPanel != NULL)
		delete ggpoStatsPanel;

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
	if (IsDrawMode(Emode::TEST_PLAYER))
	{
		if (gameState == RUN)
		{
			preScreenTex->clear();
			DrawGame(preScreenTex);
		}
		else if (gameState == SEQUENCE)
		{
			preScreenTex->clear();
			DrawGameSequence(preScreenTex);
		}
		else if (gameState == FROZEN)
		{
			preScreenTex->clear();
			DrawGame(preScreenTex);
			//redraws the previous frame
		}
		
		DrawUI();

		if (ggpo != NULL && showNetStats)
		{
			preScreenTex->setView(uiView);
			ggpoStatsPanel->Draw(preScreenTex);
		}

		return;
	}

	preScreenTex->clear();

	preScreenTex->setView(view);

	if( background != NULL )
		background->Draw(preScreenTex);

	preScreenTex->draw(border, 8, sf::Lines);

	DrawDecor(EffectLayer::BEHIND_TERRAIN, preScreenTex );

	DrawGateInfos();

	DrawSpecialTerrain(preScreenTex);

	DrawFlyTerrain(preScreenTex);

	DrawTerrain(preScreenTex);

	DrawRails(preScreenTex);

	DrawDecor(EffectLayer::BEHIND_ENEMIES, preScreenTex);

	if (!IsDrawMode(Emode::TEST_PLAYER))
	{
		DrawActors();
	}

	DrawGraph();

	DrawMode();

	//DebugDraw(preScreenTex);

	preScreenTex->setView(view);

	TempMoveSelectedBrush();

	DrawDecor(EffectLayer::IN_FRONT, preScreenTex);

	if (zoomMultiple > 7 && (!gameCam || mode != TEST_PLAYER))
	{
		playerZoomIcon.setPosition(playerMarkers[0]->GetFloatPos());
		playerZoomIcon.setScale(zoomMultiple * 1.8, zoomMultiple * 1.8);
		preScreenTex->draw(playerZoomIcon);
	}

	preScreenTex->draw(fullBounds, 4*3, sf::Quads);

	if (mode == CREATE_IMAGES)
	{
		//is this depreciated?
		preScreenTex->draw(tempDecorSprite);
	}

	DrawUI();

	//mainMenu->fader->Draw(preScreenTex);
}

void EditSession::UpdateFullBounds()
{
	int boundRectWidth = 5 * zoomMultiple;

	int leftBound = mapHeader->leftBounds;
	int topBound = mapHeader->topBounds;
	int boundWidth = mapHeader->boundsWidth;

	int boundsBot = mapHeader->topBounds + mapHeader->boundsHeight;

	if (inversePolygon != NULL)
	{
		auto aabb = inversePolygon->GetAABB();
		boundsBot = aabb.top + aabb.height;
	}

	//0 top rect
	fullBounds[0].position = Vector2f( leftBound, topBound - boundRectWidth );
	fullBounds[1].position = Vector2f( leftBound + boundWidth, topBound - boundRectWidth );
	fullBounds[2].position = Vector2f( leftBound + boundWidth, topBound + boundRectWidth );
	fullBounds[3].position = Vector2f( leftBound, topBound + boundRectWidth );

	//1 left rect
	fullBounds[4].position = Vector2f(leftBound - boundRectWidth, topBound);
	fullBounds[5].position = Vector2f(leftBound + boundRectWidth, topBound);
	fullBounds[6].position = Vector2f(leftBound + boundRectWidth, boundsBot);
	fullBounds[7].position = Vector2f(leftBound - boundRectWidth, boundsBot);

	//2 right rect
	fullBounds[8].position = Vector2f( ( leftBound + boundWidth ) - boundRectWidth, topBound );
	fullBounds[9].position = Vector2f( ( leftBound + boundWidth ) + boundRectWidth, topBound );
	fullBounds[10].position = Vector2f( ( leftBound + boundWidth ) + boundRectWidth, boundsBot);
	fullBounds[11].position = Vector2f( ( leftBound + boundWidth ) - boundRectWidth, boundsBot);

	////bottom rect
	//fullBounds[8].position = Vector2f(0, 0);//Vector2f( leftBound, ( topBound + boundHeight ) - boundRectWidth );
	//fullBounds[9].position = Vector2f(0, 0);//Vector2f( leftBound + boundWidth, ( topBound + boundHeight ) - boundRectWidth );
	//fullBounds[10].position = Vector2f(0, 0);//Vector2f( leftBound + boundWidth, ( topBound + boundHeight ) + boundRectWidth );
	//fullBounds[11].position = Vector2f(0, 0);//Vector2f( leftBound, ( topBound + boundHeight ) + boundRectWidth );

	//left rect
	
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

void EditSession::ProcessDecorSpr(const std::string &name,
	Tileset *d_ts, int dTile, int dLayer, sf::Vector2f &centerPos,
	float rotation, sf::Vector2f &scale)
{
	DecorPtr dec = new EditorDecorInfo(name, d_ts, dTile, dLayer, centerPos,
		rotation, scale );
	if (dLayer > 0)
	{
		dec->myList = &decorImages[BEHIND_TERRAIN];//&decorImagesBehindTerrain;
		//decorImagesBehindTerrain.sort(CompareDecorInfo);
		//decorImagesBehindTerrain.push_back(dec);
	}
	else if (dLayer < 0)
	{
		dec->myList = &decorImages[BEHIND_ENEMIES];//&decorImagesFrontTerrain;
		//decorImagesFrontTerrain.push_back(dec);
	}
	else if (dLayer == 0)
	{
		dec->myList = &decorImages[EffectLayer::BETWEEN_PLAYER_AND_ENEMIES];//&decorImagesBetween;
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

void EditSession::ProcessGate(int gCat, int gVar, int numToOpen,
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
	gi->category = gCat;
	gi->variation = gVar;
	gi->SetNumToOpen(numToOpen);
	gi->edit = this;

	if (gCat == Gate::SHARD)
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
	gateInfoList.push_back(gi);

	mapStartBrush->AddObject(gi);
}

void EditSession::ProcessHeader()
{
	double memD = GetMemoryUsage();
	double megs = memD / 1000000.0;
	cout << "memory usage: " << megs << endl;
	double memDMM = MainMenu::GetInstance()->tilesetManager.GetMemoryUsage();
	double megsMM = memDMM / 1000000.0;
	cout << "mm usage: " << megsMM << endl;
	background = Background::SetupFullBG(mapHeader->envName, this, true);
	//background->Hide();

	UpdateFullBounds();
}

void EditSession::WriteMapHeader(ofstream &of)
{
	mapHeader->ver1 = 2;
	mapHeader->ver2 = 6;

	ShardParams *sp = NULL;

	auto &shardVec = mapHeader->shardInfoVec;
	shardVec.reserve(16);//unlikely to be more than 16 types
	shardVec.clear();
	bool foundShard;
	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		std::list<ActorPtr> &aList = (*it).second->actors;
		for (auto ait = aList.begin(); ait != aList.end(); ++ait)
		{
			if ((*ait)->type->info.name == "shard")
			{
				sp = (ShardParams*)(*ait);
				foundShard = false;
				for (auto sit = shardVec.begin(); sit != shardVec.end(); ++sit)
				{
					if ((*sit).world == sp->shInfo.world
						&& (*sit).localIndex == sp->shInfo.localIndex)
					{
						foundShard = true;
						break;
					}
				}

				if (!foundShard)
				{
					shardVec.push_back(sp->shInfo);
				}
			}
		}
	}

	mapHeader->numShards = shardVec.size();
	mapHeader->Save(of);
}

void EditSession::WriteDecor(ofstream &of)
{
	int totalDecor = 0;

	for (int i = 0; i < EffectLayer::EFFECTLAYER_Count; ++i)
	{
		totalDecor += decorImages[i].size();
	}

	of << totalDecor << endl;

	for (int i = 0; i < EffectLayer::EFFECTLAYER_Count; ++i)
	{
		auto &dList = decorImages[i];
		for (auto it = dList.begin(); it != dList.end(); ++it)
		{
			(*it)->WriteFile(of);
		}
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
	int numSpecialPolys = waterPolygons.size() + flyPolygons.size();
	of << numSpecialPolys << endl;

	for (auto it = waterPolygons.begin(); it != waterPolygons.end(); ++it)
	{
		(*it)->WriteFile(of);
	}

	for (auto it = flyPolygons.begin(); it != flyPolygons.end(); ++it)
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
	of << gateInfoList.size() << endl;
	for (list<GateInfoPtr>::iterator it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
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

void EditSession::WritePlayerOptions(std::ofstream &of)
{
	playerOptionsField.Save(of);
}

void EditSession::WriteFile(string fileName)
{
	saveUpdated = true;

	bool hGoal = false;
	for( map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it )
	{
		ActorGroup *group = (*it).second;
		for( list<ActorPtr>::iterator it2 = group->actors.begin(); it2 != group->actors.end(); ++it2 )
		{
			if( (*it2)->type->IsGoalType() )
			{
				hGoal = true;
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
	//boost::filesystem::copy_file()
	string tempMap = "tempmap";

	ofstream of;
	of.open(tempMap);


	int pointCount = 0;
	int bgPlatCount0 = 0;

	for (list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end(); ++it)
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
	//of << pointCount << endl;
	mapHeader->numVertices = pointCount;


	int tempTop = mapHeader->topBounds;
	int tempLeft = mapHeader->leftBounds;
	int tempWidth = mapHeader->boundsWidth;
	int tempHeight = mapHeader->boundsHeight;

	if (mode == TEST_PLAYER)
	{
		mapHeader->topBounds = realTopBounds;
		mapHeader->leftBounds = realLeftBounds;
		mapHeader->boundsWidth = realBoundsWidth;
		mapHeader->boundsHeight = realBoundsHeight;
	}

	WriteMapHeader(of);

	if (mode == TEST_PLAYER)
	{
		mapHeader->topBounds = tempTop;
		mapHeader->leftBounds = tempLeft;
		mapHeader->boundsWidth = tempWidth;
		mapHeader->boundsHeight = tempHeight;
	}

	WriteDecor(of);

	int numPlayers = mapHeader->GetNumPlayers();
	for (int i = 0; i < numPlayers; ++i)
	{
		Vector2i playerIntPos(playerMarkers[i]->GetIntPos());
		of << playerIntPos.x << " " << playerIntPos.y << endl;
	}

	WritePlayerOptions(of);

	WriteInversePoly(of);

	WritePolygons(of, bgPlatCount0);

	WriteRails(of);
	//going to use this for number of rails
	//of << "0" << endl; //writing the number of static lights for consistency. Remove this when possible.

	WriteActors(of);

	WriteGates(of);
	
	of.close();

	string from = tempMap;
	string to = fileName;
	boost::filesystem::copy_file(from, to, boost::filesystem::copy_option::overwrite_if_exists);
	boost::filesystem::remove(from);

	CreatePreview(Vector2i( 1920 / 2 - 48, 1080 / 2 - 48 ));
	//CreatePreview(Vector2i(960 * 1.25f, 540 * ));

	//enemies here
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

	ClearMostRecentError();
	for (list<PolyPtr>::iterator it = polygons.begin(); it != polygons.end() && !found; ++it)
	{
		//extended aabb 
		TerrainPoint *closePoint = (*it)->GetClosePoint(8 * zoomMultiple, pos);

		if (closePoint != NULL)
		{
			if (gatePoints == 0)
			{
				bool onpoint0, onpoint1;

				for (list<GateInfoPtr>::iterator git = gateInfoList.begin(); git != gateInfoList.end(); ++git)
				{
					onpoint0 = (*git)->point0 == closePoint;
					onpoint1 = (*git)->point1 == closePoint;
					if ( onpoint0 || onpoint1 )
					{
						GateInfoPtr gi = (*git);

						//modifyGate = gi;

						testGateInfo = *gi;

						if (onpoint0)
						{
							testGateInfo.poly0 = gi->poly1;
							testGateInfo.point0 = gi->point1;
							testGateInfo.vertexIndex0 = gi->point1->GetIndex();
						}
						else
						{
							testGateInfo.poly0 = gi->poly0;
							testGateInfo.point0 = gi->point0;
							testGateInfo.vertexIndex0 = gi->point0->GetIndex();
						}

						createGatesModeUI->SetFromGateInfo(gi);

						Action * action = new DeleteGateAction(gi, mapStartBrush);
						action->Perform();
						AddDoneAction(action);

						gatePoints = 1;

						found = true;
						break;
					}
				}

				if (!found)
				{
					createGatesModeUI->CompleteEditingGate();
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
		else
		{
			if (gatePoints == 1)
			{
				CreateError(ERR_GATE_NEEDS_BOTH_POINTS);
				ShowMostRecentError();
			}
		}
	}

	if (!found && gatePoints == 0 )
	{
		bool foundOn = false;
		for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
		{
			if ((*it)->ContainsPoint(pos))
			{
				modifyGate = (*it);
				createGatesModeUI->SetEditGate(modifyGate);
				foundOn = true;
				break;
			}
		}

		if (!foundOn)
		{
			createGatesModeUI->CompleteEditingGate();
			createGatesModeUI->modifyGate = NULL;
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
			if (actor->myEnemy != NULL)
			{
				newActor->CreateMyEnemy();
			}
			newActor->AnchorToGround(gi); //might be unnecessary

			assert(newActor != NULL);
			return newActor;
		}

	}

	return NULL;
	//return false;
}

ActorParams * EditSession::AttachActorToRail(ActorPtr actor, RailPtr rail)
{
	TerrainPoint *next;
	V2d currPos, nextPos;
	V2d aCurr, aNext;
	V2d actorPos;

	int numP = rail->GetNumPoints();
	TerrainPoint *railCurr, *railNext;
	TerrainPoint *nextActorPoint;
	for (int i = 0; i < numP - 1; ++i)
	{
		railCurr = rail->GetPoint(i);
		railNext = rail->GetPoint(i+1);

		currPos.x = railCurr->pos.x;
		currPos.y = railCurr->pos.y;

		nextPos.x = railNext->pos.x;
		nextPos.y = railNext->pos.y;

		assert(actor->posInfo.ground != NULL);

		actorPos = actor->posInfo.GetPosition();//aCurr + normalize(aNext - aCurr) * actorQuant;//V2d( actor->image.getPosition() );//
		bool onLine = PointOnLine(actorPos, currPos, nextPos);

		double finalQuant = dot(actorPos - currPos, normalize(nextPos - currPos));


		if (onLine)
		{
			cout << "actorPos: " << actorPos.x << ", " << actorPos.y << ", currPos: "
				<< currPos.x << ", " << currPos.y << endl;
			PositionInfo gi;

			gi.SetRail(rail, i, finalQuant);
			//might need to make sure it CAN be grounded

			ActorParams *newActor = actor->Copy();
			if (actor->myEnemy != NULL)
			{
				newActor->CreateMyEnemy();
			}
			newActor->AnchorToGround(gi); //might be unnecessary

			assert(newActor != NULL);
			return newActor;
		}

	}

	return NULL;
}

void EditSession::AddDoneAction( Action *a )
{
	saveUpdated = false;
	if (!doneActionStack.empty() && doneActionStack.back() == a)
	{
		int xxxxx = 5;
		assert(0);
	}
	doneActionStack.push_back(a);
}

void EditSession::UndoMostRecentAction()
{
	saveUpdated = false;
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
	LineIntersection li;
	lineIntersection( li, V2d(a.x, a.y), V2d(b.x, b.y),
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
	LineIntersection li;
	lineIntersection( li, V2d(a.x, a.y), V2d(b.x, b.y),
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


void EditSession::LoadAndResave()
{
	runToResave = true;
	Run();
	runToResave = false;
}

void EditSession::SetMatTypePanelLayer(int layer)
{
	if (layer == matTypeRectsCurrLayer)
	{
		return;
	}

	matTypeRectsCurrLayer = layer;
	for (int i = 0; i < TERRAINLAYER_Count; ++i)
	{
		if (i == layer)
		{
			auto &mtr = matTypeRects[i];
			for (auto it = mtr.begin(); it != mtr.end(); ++it)
			{
				if ((*it)->ts != NULL)
				{
					(*it)->SetShown(true);
				}
			}
		}
		else
		{
			auto &mtr = matTypeRects[i];
			for (auto it = mtr.begin(); it != mtr.end(); ++it)
			{
				if ((*it)->ts != NULL)
				{
					(*it)->SetShown(false);
				}
			}
		}
	}
	
}

void EditSession::ModifySelectedTerrainMat(
	int world, int var)
{
	Action *modifyAction = new ModifyTerrainTypeAction(selectedBrush, world, var);
	modifyAction->Perform();

	AddDoneAction(modifyAction);
}

void EditSession::SetBackground(const std::string &bgName)
{
	if (bgName != mapHeader->envName)
	{
		if (background != NULL)
		{
			delete background;
			background = NULL;
		}

		mapHeader->envName = bgName;
		background = Background::SetupFullBG(bgName, this, true );
	}
}

void EditSession::SetupTerrainSelectPanel()
{
	matTypeRectsCurrLayer = -1;
	terrainGridSize = 64;
	matTypePanel = new Panel("mattype", 600, 600, this, true);
	Color c(100, 100, 100);
	c.a = 180;
	matTypePanel->SetColor(c);

	//terrainSelectPanel->SetPosition(Vector2i(currMatRectPos.x, currMatRectPos.y + 100 + 10));
	int maxTerrainVarPerWorld = EditSession::MAX_TERRAIN_VARIATION_PER_WORLD;
	int numTypeRects = 8 * maxTerrainVarPerWorld;

	int numWaterTypeRects = TerrainPolygon::WATER_Count;
	int numPickupTypeRects = 1 * maxTerrainVarPerWorld;

	int totalRects = numTypeRects + numWaterTypeRects + numPickupTypeRects;

	matTypePanel->ReserveImageRects(totalRects);

	matTypeRects[TERRAINLAYER_NORMAL].resize(numTypeRects);

	for (int worldI = 0; worldI < 8; ++worldI)
	{
		int ind;
		for (int i = 0; i < maxTerrainVarPerWorld; ++i)
		{
			ind = worldI * maxTerrainVarPerWorld + i;

			matTypeRects[TERRAINLAYER_NORMAL][ind] = matTypePanel->AddImageRect(
				ChooseRect::ChooseRectIdentity::I_TERRAINLIBRARY,
				Vector2f(worldI * terrainGridSize, i * terrainGridSize),
				ts_terrain, ts_terrain->GetSubRect( worldI * maxTerrainVarPerWorld + i ),
				terrainGridSize);

			matTypeRects[TERRAINLAYER_NORMAL][ind]->Init();
			if (matTypeRects[TERRAINLAYER_NORMAL][ind]->ts != NULL)
			{
				//matTypeRects[TERRAINLAYER_NORMAL][ind]->SetShown(false);
			}
		}
	}

	matTypeRects[TERRAINLAYER_WATER].resize(numWaterTypeRects);

	for (int i = 0; i < TerrainPolygon::WATER_Count; ++i)
	{
		matTypeRects[TERRAINLAYER_WATER][i] = matTypePanel->AddImageRect(
			ChooseRect::ChooseRectIdentity::I_TERRAINLIBRARY,
			Vector2f(TerrainPolygon::GetWaterWorld(i) * terrainGridSize, 
				TerrainPolygon::GetWaterIndexInWorld(i) * terrainGridSize),
			ts_water, ts_water->GetSubRect(i * 2),
			terrainGridSize);
		matTypeRects[TERRAINLAYER_WATER][i]->Init();
		//matTypeRects[TERRAINLAYER_WATER][ind]->SetShown(true);
	}

	//int startWorldWater = 8;
	//for (int worldI = startWorldWater; worldI < startWorldWater + 8; ++worldI)
	//{
	//	int ind;
	//	int trueWorld = worldI - startWorldWater;
	//	for (int i = 0; i < maxTexPerWorld; ++i)
	//	{
	//		ind = trueWorld * maxTexPerWorld + i;
	//		//GetMatTileset(0, 0), is the line to change to get the right textures
	//		matTypeRects[TERRAINLAYER_WATER][ind] = matTypePanel->AddImageRect(
	//			ChooseRect::ChooseRectIdentity::I_TERRAINLIBRARY,
	//			Vector2f(trueWorld * terrainGridSize, i * terrainGridSize),
	//			ts_water,
	//			ts_water->GetSubRect( ind * 2 ),
	//			terrainGridSize);
	//		matTypeRects[TERRAINLAYER_WATER][ind]->Init();
	//		if (matTypeRects[TERRAINLAYER_WATER][ind]->ts != NULL)
	//		{
	//			//matTypeRects[TERRAINLAYER_WATER][ind]->SetShown(true);
	//		}
	//	}
	//}

	matTypeRects[TERRAINLAYER_FLY].resize(numPickupTypeRects);

	int startWorldPickup = 16;
	for (int worldI = startWorldPickup; worldI < startWorldPickup + 1; ++worldI)
	{
		int ind;
		int trueWorld = worldI - startWorldPickup;
		for (int i = 0; i < maxTerrainVarPerWorld; ++i)
		{
			ind = trueWorld * maxTerrainVarPerWorld + i;

			matTypeRects[TERRAINLAYER_FLY][ind] = matTypePanel->AddImageRect(
				ChooseRect::ChooseRectIdentity::I_TERRAINLIBRARY,
				Vector2f(trueWorld * terrainGridSize, i * terrainGridSize),
				ts_terrain,
				IntRect(0, 0, 128, 128),
				terrainGridSize);
			matTypeRects[TERRAINLAYER_FLY][ind]->Init();
			if (matTypeRects[TERRAINLAYER_FLY][ind]->ts != NULL)
			{
				//matTypeRects[TERRAINLAYER_WATER][ind]->SetShown(true);
			}
		}
	}

}

void EditSession::SetCurrSelectedShardType(int w, int li)
{
	if (selectedBrush->IsSingleActor())
	{
		ActorPtr a = selectedBrush->GetFirst()->GetAsActor();
		ShardParams *sp = (ShardParams*)a;
		sp->SetShard(w, li);
	}
	else
	{
		assert(0);
	}
}

void EditSession::SetupShardSelectPanel()
{
	shardNumX = 11;
	shardNumY = 2;

	shardGridSize = 64;

	shardTypePanel = new Panel("shardtype", 600, 600, this, true);
	Color c(100, 100, 100);
	c.a = 180;
	shardTypePanel->SetColor(c);

	int numWorlds = 7;
	for (int i = 0; i < numWorlds; ++i)
	{
		ts_shards[i] = GetSizedTileset("Shard/shards_w" + to_string(i + 1) + "_48x48.png");
	}

	int totalShards = shardNumX * shardNumY * 7;

	Tileset *ts_currShards;
	int sInd = 0;

	shardTypePanel->ReserveImageRects(totalShards);
	shardTypeRects.resize(totalShards);

	for (int w = 0; w < numWorlds; ++w)
	{
		ts_currShards = ts_shards[w];
		if (ts_currShards == NULL)
			continue;

		for (int y = 0; y < shardNumY; ++y)
		{
			for (int x = 0; x < shardNumX; ++x)
			{
				sInd = y * shardNumX + x;
				int shardT = (sInd + (shardNumX * shardNumY) * w);
				if (shardT >= SHARD_Count)
				{
					shardTypeRects[shardT] = NULL;
				}
				else
				{
					shardTypeRects[shardT] =
						shardTypePanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_SHARDLIBRARY,
							Vector2f(x * shardGridSize, y * shardGridSize + w * 2 * shardGridSize),
							ts_currShards, sInd, shardGridSize);
					shardTypeRects[shardT]->Init();
					shardTypeRects[shardT]->SetShown(true);
				}
			}
		}
	}
}

void EditSession::SetupNewMapPanel()
{
	newMapPanel = new Panel("newmap", 500, 500, this, true);
	newMapPanel->SetPosition(Vector2i(960 - newMapPanel->size.x/2, 540 - newMapPanel->size.y/2));
	
	newMapPanel->AddLabel("mapnamelabel", Vector2i( 10, 10 ), 28, "Map Name:");
	newMapPanel->AddTextBox("mapname", Vector2i(200, 10), 200, 30, "");

	newMapPanel->AddLabel("pathlabel", Vector2i(100, 60), 28, "Resources\\Maps\\CustomMaps");
	newMapPanel->AddButton("pathbutton", Vector2i(10, 60), sf::Vector2f(30, 30), "");

	newMapPanel->AddLabel("timetolivelabel", Vector2i(10, 120), 28, "Time to Live\n(seconds):");
	TextBox *ttlBox = newMapPanel->AddTextBox("timetolive", Vector2i(200, 140), 200, 30, "60");

	Button *OK = newMapPanel->AddButton("ok", Vector2i(10, 300),
		Vector2f(50, 30), "OK");
	newMapPanel->SetConfirmButton(OK);
	Button *cancel = newMapPanel->AddButton("skip", Vector2i(10, 360),
		Vector2f(50, 30), "Skip");
	newMapPanel->SetCancelButton(cancel);
}

void EditSession::ActivateNewMapPanel()
{
	AddActivePanel(newMapPanel);
	newMapPanel->textBoxes["mapname"]->SetString("");
	newMapPanel->textBoxes["timetolive"]->SetString("60");
}

void EditSession::SetupBrushPanels()
{
	nameBrushPanel = new Panel("namebrush", 300, 150, this, true);
	nameBrushPanel->AddLabel("name", Vector2i(10, 10), 28, "Name the Brush:");
	nameBrushPanel->AddTextBox("text", Vector2i(40, 10), 200, 20, "");
	Button *OK = nameBrushPanel->AddButton("ok", Vector2i(250, 10),
		Vector2f(30, 30), "OK");
	nameBrushPanel->SetConfirmButton(OK);
	Button *cancel = nameBrushPanel->AddButton("cancel", Vector2i(250, 50),
		Vector2f(30, 30), "Cancel");
	nameBrushPanel->SetConfirmButton(cancel);
}

void EditSession::Init()
{
	frameRateDisplay.showFrameRate = true;
	runningTimerDisplay.showRunningTimer = true;

	saveUpdated = true;
	reload = false;
	reloadNew = false;

	Tileset *ts_playerZoomIcon = GetTileset("Editor/playerzoomicon.png");
	playerZoomIcon.setTexture(*ts_playerZoomIcon->texture);
	playerZoomIcon.setOrigin(playerZoomIcon.getLocalBounds().width / 2, playerZoomIcon.getLocalBounds().height / 2);

	mainMenu->SetMouseGrabbed(true);
	mainMenu->SetMouseVisible(true);

	

	playerType = NULL;

	graph = NULL;

	currTerrainWorld[TERRAINLAYER_NORMAL] = 0;
	currTerrainVar[TERRAINLAYER_NORMAL] = 0;

	currTerrainWorld[TERRAINLAYER_WATER] = 8;
	currTerrainVar[TERRAINLAYER_WATER] = 0;

	currTerrainWorld[TERRAINLAYER_FLY] = 9;
	currTerrainVar[TERRAINLAYER_FLY] = 0;

	SetupSoundManager();
	SetupSoundLists();

	SetupSuperSequence();

	SetupHitboxManager();
	
	

	SetupShardsCapturedField();

	assert(players[0] == NULL);

	players[0] = new Actor(NULL, this, 0);
	allPlayers[0] = players[0];
	for (int i = 1; i < MAX_PLAYERS; ++i)
	{
		players[i] = new Actor(NULL, this, i);
		allPlayers[i] = players[i];
	}

	SetupEnemyTypes();

	brushManager = new BrushManager;
	fileChooser = new DefaultFileSelector;
	adventureCreator = new AdventureCreator;

	for (auto it = types.begin(); it != types.end(); ++it)
	{
		//cout << "creating default for: " << (*it).first << endl;
		(*it).second->CreateDefaultEnemy();
	}

	ReadDecorImagesFile();

	SetupGGPOStatsPanel();
	SetupTerrainSelectPanel();
	SetupShardSelectPanel();
	SetupBrushPanels();
	SetupNewMapPanel();

	confirmPopup = new ConfirmPopup();

	graph = new EditorGraph;

	generalUI = new GeneralUI();

	createEnemyModeUI = new CreateEnemyModeUI();
	createDecorModeUI = new CreateDecorModeUI();
	createTerrainModeUI = new CreateTerrainModeUI();
	createRailModeUI = new CreateRailModeUI();
	createGatesModeUI = new CreateGatesModeUI();
	editModeUI = new EditModeUI();

	mapOptionsUI = new MapOptionsUI();


	polygonInProgress = new TerrainPolygon();
	railInProgress = new TerrainRail();

	AllocateEffects();

	SetupAbsorbParticles();

	SetupDeathSequence();

	if (filePathStr == "")
	{
		DefaultInit();
	}
}

void EditSession::RestartGame()
{
	TestPlayerMode();
}

void EditSession::ReloadNew()
{
	reload = true;
	reloadNew = true;
	quit = true;
	filePathStr = "";
	filePath = "";
}

void EditSession::Reload(
	const boost::filesystem::path &p_filePath)
{
	reload = true;
	quit = true;
	//ClearActivePanels();
	filePath = p_filePath;
	filePathStr = filePath.string();
}

void EditSession::Load()
{

}

void EditSession::DefaultInit()
{
	mapHeader = new MapHeader;
	mapHeader->description = "no description";
	mapHeader->collectionName = "default";
	mapHeader->gameMode = MapHeader::T_BASIC;

	mapHeader->envName = "w1_01";//newMapInfo.envName;//"";//"w1_01";

	mapHeader->envWorldType = 0;//newMapInfo.envWorldType;
	
	mapHeader->leftBounds = -1500;
	mapHeader->topBounds = -1500;
	mapHeader->boundsWidth = 3000;
	mapHeader->boundsHeight = 3000;

	mapHeader->drainSeconds = 60;//newMapInfo.drainSeconds;//60;

	background = Background::SetupFullBG(mapHeader->envName, this, true);

	mapHeader->bossFightType = 0;

	for (int i = 0; i < 4; ++i)
	{
		playerOrigPos[i] = Vector2i(0, 0);
	}
	

	UpdateFullBounds();

	currentFile = "";
}

void EditSession::UpdateNumPlayers()
{
	int numPlayers = mapHeader->GetNumPlayers();
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != NULL)
		{
			if (i >= numPlayers)
			{
				playerMarkers[i]->Deactivate();
				players[i] = NULL;
			}
		}
		else
		{
			if (i < numPlayers)
			{
				if (players[i] == NULL)
				{
					players[i] = allPlayers[i];
					playerMarkers[i]->Activate();

					if (i == 1)
					{
						playerMarkers[i]->MoveTo(
							playerMarkers[0]->GetIntPos() + Vector2i(100, 0));
					}
					else if (i == 2)
					{
						playerMarkers[i]->MoveTo(
							playerMarkers[0]->GetIntPos() + Vector2i(100, 100));
					}
					else if (i == 3)
					{
						playerMarkers[i]->MoveTo(
							playerMarkers[0]->GetIntPos() + Vector2i(0, 100));
					}
				}
			}
		}

	}
}

void EditSession::SetGameMode(int newMode)
{
	if (newMode == mapHeader->gameMode)
	{
		return;
	}

	if (gameMode != NULL)
	{
		delete gameMode;
		gameMode = NULL;
	}

	mapHeader->gameMode = newMode;

	SetupGameMode();

	if (hud != NULL)
	{
		delete hud;
		hud = NULL;
	}

	SetupHUD();


	UpdateNumPlayers();
}

int EditSession::EditRun()
{
	saveUpdated = true;
	reload = false;
	mapStartBrush->Clear();
	totalGameFrames = 0;
	grassChanges = NULL;
	focusedPanel = NULL;
	grabbedBorderIndex = -1;

	oldShaderZoom = -1;
	complexPaste = NULL;

	testGateInfo.edit = EditSession::GetSession();
	bool oldMouseGrabbed = mainMenu->GetMouseGrabbed();
	bool oldMouseVis = mainMenu->GetMouseVisible();

	sf::View oldPreTexView = preScreenTex->getView();//mainMenu->preScreenTexture->
	sf::View oldWindowView = window->getView();

	tempActor = NULL;
	v.setCenter(0, 0);
	v.setSize(1920 / 2, 1080 / 2);
	window->setView(v);

	validityRadius = 4;

	modifyGate = NULL;

	showGraph = false;

	justCompletedPolyWithClick = false;
	justCompletedRailWithClick = false;

	trackingEnemyParams = NULL;
	trackingDecor = NULL;

	currTerrainTypeSpr.setPosition(0, 160);
	UpdateCurrTerrainType();

	returnVal = 0;
	Color testColor(0x75, 0x70, 0x90);

	preScreenTex->setView(view);

	currentFile = filePath.string();

	mode = EDIT;
	SetMode(EDIT);
	stored = mode;

	if (reloadNew )
	{
		//clear groups on my own in case I don't load the file
		for (auto it = groups.begin(); it != groups.end(); ++it)
		{
			delete(*it).second;
		}
		groups.clear();

		DefaultInit();

		ActivateNewMapPanel();

		
	}
	else if( filePathStr != "" )
	{
		ReadFile();
	}

	

	//this needs to be after readfile because reading enemies deletes actorgroup

	
	string basicGroupName = "--";

	if (groups[basicGroupName] == NULL)
	{
		ActorGroup *basicGroup = new ActorGroup(basicGroupName);
		groups[basicGroupName] = basicGroup;
	}

	ActorGroup *playerGroup = new ActorGroup("player");
	groups["player"] = playerGroup;

	ParamsInfo playerPI("player", NULL, NULL,
		Vector2i(), Vector2i(22, 42), false, false, false, false, 1, 0,
		GetTileset("Kin/jump_64x64.png", 64, 64));

	if (playerType == NULL)
	{
		playerType = new ActorType(playerPI);
		types["player"] = playerType;
	}

	//need to make a new one each time because they get destroyed when I load actors in session
	PlayerParams *currPlayerMarker;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		currPlayerMarker = new PlayerParams(playerType, Vector2i(0, 0));
		currPlayerMarker->group = groups["player"];
		//if( )
		groups["player"]->actors.push_back(currPlayerMarker);
		currPlayerMarker->SetPosition(playerOrigPos[i]);
		currPlayerMarker->image.setPosition(currPlayerMarker->GetFloatPos());
		currPlayerMarker->SetBoundingQuad();
		mapStartBrush->AddObject(currPlayerMarker);

		playerMarkers[i] = currPlayerMarker;
		/*if (i == 0)
		{
			playerMarkers[i]->Activate();
		}*/
	}


	int gm = mapHeader->gameMode;
	mapHeader->gameMode = -1;
	SetGameMode(gm);

	//UpdateNumPlayers();

	reloadNew = false;

	//SetupHUD();

	
	
	
	
	//-------------------------

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != NULL)
		{
			players[i]->SetGameMode();
		}
	}

	cam.Init(GetPlayerPos(0));

	if (!initialViewSet)
	{
		view.setSize(1920, 1080);
		view.setCenter(playerMarkers[0]->GetFloatPos());
	}

	quit = false;

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

	Tileset *ts_guiMenu = GetSizedTileset("Editor/orbmodeselector_384x384.png");
	ts_guiMenu->SetSpriteTexture(guiMenuSprite);
	guiMenuSprite.setOrigin(guiMenuSprite.getLocalBounds().width / 2, guiMenuSprite.getLocalBounds().height / 2);

	
	bool canCreatePoint = true;

	menuCircleDist = 128;
	menuCircleRadius = 64;

	circleTopPos = V2d(0, -1) * menuCircleDist;

	circleUpperRightPos = V2d(sqrt(3.0) / 2, -.5) * menuCircleDist;
	circleLowerRightPos = V2d(sqrt(3.0) / 2, .5) * menuCircleDist;

	circleUpperLeftPos = V2d(-sqrt(3.0) / 2, -.5) * menuCircleDist;
	circleLowerLeftPos = V2d(-sqrt(3.0) / 2, .5) * menuCircleDist;

	circleBottomPos = V2d(0, 1) * menuCircleDist;

	menuSelection = "";

	borderMove = 100;

	Vector2f uiMouse;

	editClock.restart();

	while (window->pollEvent(ev))
	{
	}

	while (!quit)
	{
		if (runToResave)
		{
			cout << "run to resave: writing to file: " << currentFile << endl;
			WriteFile(currentFile);
			break;
		}

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

		preScreenTex->setView(uiView);
		uiMouse = preScreenTex->mapPixelToCoords(pixelPos);
		uiMousePos = uiMouse;

		preScreenTex->setView(view);

		testPoint.x = worldPos.x;
		testPoint.y = worldPos.y;

		if (mode == CREATE_PATROL_PATH || mode == SET_DIRECTION)
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

		MOUSE.Update(pixelPos);

		if (IsKeyPressed(Keyboard::Num5))
		{
			Vector2f halfSize(scaleSprite.getGlobalBounds().width / 2.f,
				scaleSprite.getGlobalBounds().height / 2.f);
			scaleSprite.setPosition(Vector2f(pixelPos) - halfSize);
			scaleSpriteBGRect.setPosition(Vector2f(pixelPos) - halfSize);
		}
		else
		{
			scaleSprite.setPosition(0, 80);
			scaleSpriteBGRect.setPosition(0, 80);
		}

		//showGraph = false;

		if (quit)
			break;


		if (mode == PAUSED)
		{
			while (window->pollEvent(ev))
			{
				PausedModeHandleEvent();
			}
		}
		else
		{
			HandleEvents();

			if (mode != PAUSED)
				UpdateMode();
		}

		UpdatePanning();

		if (background != NULL )
		{
			background->Update(view.getCenter(), spriteUpdateFrames);
		}
		
		UpdateFullBounds();

		UpdatePolyShaders();

		//ShowMostRecentError();

		/*int testSize = 0;
		for (auto it = groups.begin(); it != groups.end(); ++it)
		{
		auto aList = (*it).second->actors;
		testSize += aList.size();
		}
		cout << "testsize: " << testSize << endl;*/

		Draw();

		

		preScreenTex->setView(view);

		Display();
	}

	preScreenTex->setView(oldPreTexView);
	window->setView(oldWindowView);

	mainMenu->SetMouseGrabbed(oldMouseGrabbed);
	mainMenu->SetMouseVisible(oldMouseVis);

	return returnVal;
}

int EditSession::Run()
{
	soundNodeList->SetSoundVolume(10);
	int result;
	while( true )
	{
		result = EditRun();
		if (reload)
		{
			CleanupForReload();
			
		}
		else
		{
			break;
		}
	}

	fader->Reset();
	swiper->Reset();
	soundNodeList->Reset();

	return result;
}

//THIS IS ALSO DEFINED IN ACTORPARAMS NEED TO GET RID OF THE DUPLICATE
//helper function to assign monitor types

void EditSession::ButtonCallback( Button *b, const std::string & e )
{
	Panel *p = b->panel;

	if (p == newMapPanel)
	{
		if (b == newMapPanel->confirmButton)
		{
			string mapName = newMapPanel->textBoxes["mapname"]->GetString();
			if ( mapName != "")
			{
				mapHeader->envName = "w1_01";
				mapHeader->envWorldType = 0;
				background = Background::SetupFullBG(mapHeader->envName, this, true);

				stringstream ss;
				ss << newMapPanel->textBoxes["timetolive"]->GetString();
				int d;
				ss >> d;
				mapHeader->drainSeconds = d;

				string pathStr = newMapPanel->labels["pathlabel"]->getString().toAnsiString()
					+ "\\" + mapName + ".brknk";
				filePathStr = pathStr;
				filePath = pathStr;
				currentFile = filePathStr;

				RemoveActivePanel(newMapPanel);
			}
		}
		else if (b == newMapPanel->cancelButton)
		{
			RemoveActivePanel(newMapPanel);
		}
	}
	else if (p == editDecorPanel)
	{
		if (b->name == "ok")
		{
			RemoveActivePanel(p);
		}
	}
	else if (p->name == "airtrigger_options")
	{
		if (b->name == "ok")
		{
			RegularOKButton(p);
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

			RemoveActivePanel(p);
			
			SetMode(CREATE_RECT);
			drawingCreateRect = false;
		}
	}
	else if (p->name == "camerashot_options")
	{
		if (b->name == "ok")
		{
			RegularOKButton(p);
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

			RemoveActivePanel(p);
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

				//MessagePop("minimum edge length too low.\n Set to minimum of 8");

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
				mapHeader->drainSeconds = dSecs;
			}

			ss.clear();

			ss << bossTypeStr;

			int bType;
			ss >> bType;

			if (!ss.fail())
			{
				//mapHeader->bossType = bType;
			}


			RemoveActivePanel(p);
		}
		else if (b->name == "envtype")
		{
			//GridSelectPop("bg");
		}
	}
	else if( p->name == "terrain_options" )
	{
		if (b->name == "ok")
		{
			RemoveActivePanel(p);
		}
	}
	//else if (p->name == "rail_options")
	//{
	//	SelectPtr select = selectedBrush->objects.front();
	//	TerrainRail *tr = (TerrainRail*)select;
	//	if (b->name == "ok")
	//	{
	//		tr->SetParams(railOptionsPanel);
	//		RemoveActivePanel(p);
	//	}
	//	else if (b->name == "reverse")
	//	{
	//		tr->SwitchDirection();
	//		//reverse single rail
	//	}
	//}
	else if( p->name == "error_popup" )
	{
		if (b->name == "ok")
		{
			RemoveActivePanel(p);
		}
	}
	/*else if( p == gateSelectorPopup )
	{
		tempGridResult = "delete";
	}*/
	else
	{
		if (b->name == "ok")
		{
			RegularOKButton(p);
		}
		/*else if (b->name == "createpath" || b->name == "createrail")
		{
			RegularCreatePathButton(p);
		}
		else if (b->name == "setdirection")
		{
			RegularCreatePathButton(p);
			SetMode(SET_DIRECTION);
		}*/
	}
}

void EditSession::TextBoxCallback( TextBox *tb, const std::string & e )
{
	//to be able to show previews in real time
	Panel *p = tb->panel;
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
	Panel *panel = gs->panel;
	if (panel == decorPanel)
	{
		if (name != "not set")
		{
			//cout << "real result: " << name << endl;
			currDecorName = name;
			ts_currDecor = decorTSMap[currDecorName];
			int ind = gs->selectedY * gs->xSize + gs->selectedX;
			//currDecorTile = decorTileIndexes[ind];


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

			RemoveActivePanel(panel);

		}
		else
		{
			//	cout << "not set" << endl;
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
	Panel *p = cb->panel;

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

void EditSession::SliderCallback(Slider *slider)
{

}

void EditSession::DropdownCallback(Dropdown *dropdown, const std::string & e)
{

}

void EditSession::ClearUndoneActions()
{
	HideErrorBar();

	for( list<Action*>::iterator it = undoneActionStack.begin(); it != undoneActionStack.end(); ++it )
	{
		delete (*it);
	}
	undoneActionStack.clear();
}

TerrainPoint * EditSession::TrySnapPosToPoint(sf::Vector2f &p, SelectPtr &obj, double radius)
{
	auto & currPolyList = GetCorrectPolygonList();

	for (auto it = currPolyList.begin(); it != currPolyList.end(); ++it)
	{
		TerrainPoint *closePoint = (*it)->GetClosePoint(radius, V2d(p));
		if (closePoint != NULL)
		{
			p = Vector2f(closePoint->pos);
			obj = (*it);
			return closePoint;
		}
	}

	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		TerrainPoint *closePoint = (*it)->GetClosePoint(radius, V2d(p));
		if (closePoint != NULL)
		{
			p = Vector2f(closePoint->pos);
			obj = (*it);
			return closePoint;
		}
	}

	return false;
}

TerrainPoint * EditSession::TrySnapPosToPoint(V2d &p, SelectPtr &obj, double radius)
{
	auto & currPolyList = GetCorrectPolygonList();

	for (auto it = currPolyList.begin(); it != currPolyList.end(); ++it)
	{
		TerrainPoint *closePoint = (*it)->GetClosePoint(radius, V2d(p));
		if (closePoint != NULL)
		{
			p = V2d(closePoint->pos);
			obj = (*it);
			return closePoint;
		}
	}

	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		TerrainPoint *closePoint = (*it)->GetClosePoint(radius, V2d(p));
		if (closePoint != NULL)
		{
			p = V2d(closePoint->pos);
			obj = (*it);
			return closePoint;
		}
	}

	return NULL;
}

void EditSession::ChooseFileOpen(FileChooser *fc,
	const std::string &fileName)
{
	if (fc->ext == ".bnbrush")
	{
		Brush *loadedBrush = brushManager->LoadBrush(fc->currPath.string(),
			fileName);

		createTerrainModeUI->UpdateBrushHotbar();

		DestroyCopiedBrushes();

		copiedBrush = loadedBrush->CopyTerrainAndAttachedActors();
		freeActorCopiedBrush = loadedBrush->CopyFreeActors();

		EditModePaste();
	}
	else if (fc->ext == ".adventure")
	{
		adventureCreator->LoadAdventure(fc->currPath.string(), fileName);
		adventureCreator->Open();
	}
	else if (fc->ext == ".brknk")
	{
		Reload(fc->currPath.string() + "\\" + fileName + ".brknk");
	}
}

void EditSession::ChooseFileSave(FileChooser *fc,
	const std::string &fileName)
{
	if (fc->ext == ".bnbrush" )
	{
		brushManager->SaveBrush(selectedBrush, fc->currPath.string(),
			fileName );

		createTerrainModeUI->UpdateBrushHotbar();
	}
	else if (fc->ext == ".brknk")
	{
		string fp = fc->currPath.string() + "\\" + fileName + ".brknk";
		filePath = fp;
		filePathStr = fp;
		currentFile = fp;
		WriteFile(fp);
	}
}

void EditSession::ShowMostRecentError()
{
	if (mostRecentError != ERR_NO_ERROR)
	{
		errorBar.ShowError(mostRecentError);
		mostRecentError = ERR_NO_ERROR;
	}
}

void EditSession::ClearMostRecentError()
{
	mostRecentError = ERR_NO_ERROR;
	HideErrorBar();
}

void EditSession::CreateError(ErrorType er)
{
	mostRecentError = er;
}

void EditSession::HideErrorBar()
{
	errorBar.SetShown(false);
}

int EditSession::GetSpecialTerrainMode()
{
	if (mode == CREATE_TERRAIN)
	{
		return createTerrainModeUI->GetTerrainLayer();
	}
	else
	{
		if (currTerrainWorld[0] >= 8 && currTerrainWorld[0] < 16) //all waters
		{
			return 1;
		}
		if (currTerrainWorld[0] == 16 ) //all pickups for now adjust later
		{
			return 2;
		}
		else
		{
			return 0;
		}
	}
	
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
	//decorTileIndexes = new int[w*h];
	

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
			//decorTileIndexes[ind] = (*tit);
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
	for( auto it = selectedPoints.begin(); it != selectedPoints.end(); ++it )
	{
		count += (*it).second.size();
	}

	for (auto it = selectedRailPoints.begin(); it != selectedRailPoints.end(); ++it)
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

void EditSession::ClearPolygonInProgress()
{
	polygonInProgress->ClearPoints();
}

void EditSession::ClearRailInProgress()
{
	railInProgress->ClearPoints();
}

void EditSession::RemovePointFromRailInProgress()
{
	int numP = railInProgress->GetNumPoints();
	if ( numP > 0)
	{
		railInProgress->RemoveLastPoint();
		if (numP == 1 )
		{
			if (trackingEnemyParams != NULL)
			{
				CancelEnemyCreation();
				SetMode(CREATE_ENEMY);
			}
			else if (editModeUI->currParams != NULL)
			{
				EditModeDelete();
				SetMode(EDIT);
			}
		}
	}
}

void EditSession::TryAttachActorsToRails(
	std::list<RailPtr> & origRails,
	std::list<RailPtr> & newRails,
	Brush *b)
{
	for (auto it = origRails.begin(); it != origRails.end(); ++it)
	{
		TryAttachActorsToRail((*it), newRails, b);
	}
}

void EditSession::TryAttachActorsToRail(
	RailPtr orig,
	std::list<RailPtr> & newRails,
	Brush *b)
{
	for (auto mit = orig->enemies.begin(); mit != orig->enemies.end(); ++mit)
	{
		for (auto bit = (*mit).second.begin(); bit != (*mit).second.end(); ++bit)
		{
			for (auto rit = newRails.begin();
				rit != newRails.end(); ++rit)
			{
				ActorParams *ac = AttachActorToRail((*bit), (*rit));
				if (ac != NULL)
				{
					b->AddObject(ac);
				}
			}
		}
	}
}

void EditSession::TryAttachActorsToPolys(
	std::list<PolyPtr> &origRails,
	std::list<PolyPtr> &newRails,
	Brush *b)
{
	for (auto it = origRails.begin(); it != origRails.end(); ++it)
	{
		TryAttachActorsToPoly((*it), newRails, b);
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

bool EditSession::TryAttachPlayerToPolys(V2d &groundPosition, double xoff)
{
	Actor *player = GetPlayer(0);
	auto &polyList = GetCorrectPolygonList(0);
	Edge *pEdge;
	double quant;
	for (auto it = polyList.begin(); it != polyList.end(); ++it)
	{
		pEdge = (*it)->CheckPlayerOnLine(groundPosition, quant);
		if (pEdge != NULL)
		{
			player->SetGroundedPos(pEdge, quant, xoff );
			return true;
		}
	}

	return false;


	//TerrainPoint *next;
	//V2d currPos, nextPos;
	//V2d aCurr, aNext;
	//V2d actorPos;

	//int numP = poly->GetNumPoints();
	//TerrainPoint *polyCurr, *polyNext;
	//TerrainPoint *nextActorPoint;
	//for (int i = 0; i < numP; ++i)
	//{
	//	polyCurr = poly->GetPoint(i);
	//	polyNext = poly->GetNextPoint(i);

	//	currPos.x = polyCurr->pos.x;
	//	currPos.y = polyCurr->pos.y;

	//	nextPos.x = polyNext->pos.x;
	//	nextPos.y = polyNext->pos.y;

	//	assert(actor->posInfo.ground != NULL);

	//	actorPos = actor->posInfo.GetPosition();//aCurr + normalize(aNext - aCurr) * actorQuant;//V2d( actor->image.getPosition() );//
	//	bool onLine = PointOnLine(actorPos, currPos, nextPos);

	//	double finalQuant = dot(actorPos - currPos, normalize(nextPos - currPos));


	//	if (onLine)
	//	{
	//		cout << "actorPos: " << actorPos.x << ", " << actorPos.y << ", currPos: "
	//			<< currPos.x << ", " << currPos.y << endl;
	//		PositionInfo gi;

	//		gi.SetGround(poly, i, finalQuant);
	//		//might need to make sure it CAN be grounded

	//		ActorParams *newActor = actor->Copy();
	//		if (actor->myEnemy != NULL)
	//		{
	//			newActor->CreateMyEnemy();
	//		}
	//		newActor->AnchorToGround(gi); //might be unnecessary

	//		assert(newActor != NULL);
	//		return newActor;
	//	}

	//}

	//return NULL;
}

void EditSession::TryKeepGrass(std::list<PolyPtr> & origPolys,
	std::list<PolyPtr> & newPolys)
{
	for (auto origIt = origPolys.begin(); origIt != origPolys.end(); ++origIt)
	{
		for (auto newIt = newPolys.begin(); newIt != newPolys.end(); ++newIt)
		{
			(*origIt)->CopyMyGrass((*newIt));
		}
	}
}

void EditSession::TryKeepGrass(PolyPtr origPoly,
	std::list<PolyPtr> & newPolys)
{
	for (auto newIt = newPolys.begin(); newIt != newPolys.end(); ++newIt)
	{
		origPoly->CopyMyGrass((*newIt));
	}
}

void EditSession::TryKeepGrass(std::set<PolyPtr> & origPolys,
	std::list<PolyPtr> & newPolys)
{
	for (auto origIt = origPolys.begin(); origIt != origPolys.end(); ++origIt)
	{
		for (auto newIt = newPolys.begin(); newIt != newPolys.end(); ++newIt)
		{
			(*origIt)->CopyMyGrass((*newIt));
		}
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

void EditSession::AddFullRailsToBrush(
	std::list<RailPtr> &railList,
	Brush *b)
{
	for (auto it = railList.begin(); it != railList.end(); ++it)
	{
		AddFullRailToBrush((*it),b);
	}
}

void EditSession::AddFullRailToBrush(RailPtr rail, Brush *b)
{
	b->AddObject(rail);
	rail->AddEnemiesToBrush(b);
}

void EditSession::TryRemoveSelectedPoints()
{
	//need to make this into an undoable action soon

	//int removeSuccess = IsRemovePointsOkay();

	//if (removeSuccess == 1)
	if( !selectedPoints.empty() )
	{
		Brush orig;
		Brush result;

		list<GateInfoPtr> gateInfoList;
		list<PolyPtr> affectedPolys;
		list<PolyPtr> newPolys;
		bool valid = true;

		for (auto it = selectedPoints.begin(); it != selectedPoints.end(); ++it)
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
					ShowMostRecentError();
				}
			}

			if (!valid)
			{
				delete newPoly;
				for (auto pit = newPolys.begin(); pit != newPolys.end(); ++pit)
				{
					delete (*pit);
				}
				//MessagePop("problem removing points");
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
		TryKeepGrass(affectedPolys, newPolys);

		TryKeepGates(gateInfoList, newPolys, &result);
		selectedPoints.clear();

		ClearUndoneActions();

		Action * action = new ReplaceBrushAction(&orig, &result, mapStartBrush);

		action->Perform();
		AddDoneAction(action);
	}


	if (!selectedRailPoints.empty())
	{
		Brush orig;
		Brush result;

		list<RailPtr> affectedRails;
		list<RailPtr> newRails;
		bool valid = true;
		for (auto it = selectedRailPoints.begin(); it != selectedRailPoints.end(); ++it)
		{
			RailPtr rp = (*it).first;
			affectedRails.push_back(rp);

			rp->CreateNewRailsWithSelectedPointsRemoved(newRails);
			/*PolyPtr newPoly(tp->CreateCopyWithSelectedPointsRemoved());

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

			newPolys.push_back(newPoly);*/
		}

		for (auto it = newRails.begin(); it != newRails.end(); ++it)
		{
			(*it)->Finalize();
			result.AddObject((*it));
		}

		ClearSelectedPoints();

		AddFullRailsToBrush(affectedRails, &orig);

		TryAttachActorsToRails(affectedRails, newRails, &result);

		selectedRailPoints.clear();		

		ClearUndoneActions();

		Action * action = new ReplaceBrushAction(&orig, &result, mapStartBrush);

		action->Perform();
		AddDoneAction(action);
	}
}

bool EditSession::PointSelectActor( V2d &pos )
{
	if (!editModeUI->IsLayerActionable(LAYER_ACTOR))
	{
		return false;
	}

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
					//ClearSelectedPolys();
				}
				else
				{
					if (!HoldingShift())
					{
						ClearSelectedBrush();
					}

					
					
					grabbedActor = (*ait);

					//if( grabbedActor->type->panel != NULL )
					editModeUI->SetEnemyPanel(grabbedActor );

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
	if (!editModeUI->IsLayerActionable(LAYER_IMAGE))
	{
		return false;
	}

	for (int i = EffectLayer::EFFECTLAYER_Count - 1; i >= 0; --i)
	{
		auto &dList = decorImages[i];

		for (auto it = dList.begin(); it != dList.end(); ++it)
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
	}
	return false;
}

bool EditSession::AnchorSelectedEnemies()
{
	ActorPtr actor;
	PolyPtr poly;
	for( auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it )
	{
		actor = (*it)->GetAsActor();
		if (actor == NULL)
			continue;

		//enemy goes back to idle frame when you set it down
		if (actor->myEnemy != NULL)
		{
			actor->myEnemy->SetActionEditLoop();
			actor->myEnemy->UpdateFromEditParams(0);
		}

		if (actor->posInfo.ground != NULL) //might need a thing here for rails too
		{
			poly = actor->posInfo.ground;
			if (poly->selected)
			{
				continue;
			}

			if (!selectedPoints.empty())
				continue;
			//if (selectedPoints.find(poly) != selectedPoints.end())
			//	continue;

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
				Action *action = new MoveBrushAction(selectedBrush, delta, false, NULL);

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
		

		createEnemyModeUI->SetShown(true);

		ClearSelectedBrush();
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

void EditSession::FinishEnemyCreation()
{
	AddRecentEnemy(trackingEnemyParams);
	trackingEnemyParams = NULL;
	createEnemyModeUI->SetLibraryShown(false);
}

void EditSession::CancelEnemyCreation()
{
	trackingEnemyParams = NULL;

	Action * action = doneActionStack.back();
	doneActionStack.pop_back();

	action->Undo();

	delete action;
}

void EditSession::TryCompleteEnemyCreation()
{
	bool validMove = false;

	assert(mode == CREATE_ENEMY);

	ClearMostRecentError();
	if (trackingEnemyParams->CanApply())
	{
		validMove = true;
	}
	else
	{
		ShowMostRecentError();
	}

	if (validMove)
	{
		ClearUndoneActions();
		trackingEnemyParams->OnCreate();
		if (mode == CREATE_ENEMY) //might not be because of OnCreate such as rail enemies
		{
			FinishEnemyCreation();
		}
		
	}
	else
	{
		CancelEnemyCreation();
	}
}

void EditSession::TryCompleteSelectedMove()
{
	bool validMove = false;

	if (mode == CREATE_IMAGES)
	{
		assert(grabbedImage != NULL);
		if (grabbedImage->layer > 0)
		{
			grabbedImage->myList = &decorImages[BEHIND_TERRAIN];//&decorImagesBehindTerrain;
		}
		else if (grabbedImage->layer < 0)
		{
			grabbedImage->myList = &decorImages[BEHIND_ENEMIES];//&decorImagesFrontTerrain;
		}
		else if (grabbedImage->layer == 0)
		{
			grabbedImage->myList = &decorImages[BETWEEN_PLAYER_AND_ENEMIES];//&decorImagesBetween;
		}

		Action *apply = new ApplyBrushAction(selectedBrush);
		createDecorModeUI->SetShown(true);
		apply->Perform();
		AddDoneAction(apply);
		trackingDecor = NULL;
		ClearSelectedBrush();
		createDecorModeUI->SetLibraryShown(false);
		return;
		//apply->performed = true;
	}


	//check if valid
	ClearMostRecentError();
	if (selectedBrush->CanApply())
	{
		validMove = true;
	}
	else
	{
		ShowMostRecentError();
	}

	if (validMove)
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

void EditSession::SetEnemyGridIndex( GridSelector *gs, int x, int y, const std::string &eName)
{
	gs->Set(x, y, types[eName]->GetSprite(gs->tileSizeX, gs->tileSizeY), eName);
}


void EditSession::RegularOKButton( Panel *p)
{
	if (mode == EDIT)
	{
		ISelectable *select = selectedBrush->objects.front();
		ActorParams *ap = (ActorParams*)select;
		ap->SetParams();
		if (ap->myEnemy != NULL)
		{
			ap->myEnemy->UpdateParamsSettings();
		}
	}
	else if (mode == CREATE_ENEMY)
	{
		ActorPtr ac(tempActor);
		ac->SetParams();
		ac->group = groups["--"];

		CreateActor(ac);

		tempActor = NULL;
	}
	RemoveActivePanel(p);
}

void EditSession::CreatePathButton(ActorParams *ap)
{
	if (ap == NULL)
	{
		assert(selectedBrush->IsSingleActor());
		ap = selectedBrush->objects.front()->GetAsActor();
	}

	SetMode(CREATE_PATROL_PATH);
	patrolPath.clear();
	patrolPath.push_back(ap->GetIntPos());
	patrolPathLengthSize = 0;
}

void EditSession::CreateChainButton(ActorParams *ap)
{
	if (ap == NULL)
	{
		assert(selectedBrush->IsSingleActor());
		ap = selectedBrush->objects.front()->GetAsActor();
	}

	SetMode(EditSession::CREATE_RAILS);
	justCompletedRailWithClick = true;
	railInProgress->SetRailToActorType(ap);
	railInProgress->ClearPoints();
	railInProgress->AddPoint(ap->GetIntPos(), false);
}

void EditSession::SetDirectionButton( ActorParams *ap )
{
	if (ap == NULL)
	{
		assert(selectedBrush->IsSingleActor());
		ap = selectedBrush->objects.front()->GetAsActor();
	}

	SetMode(SET_DIRECTION);
	patrolPath.clear();
	patrolPath.push_back(ap->GetIntPos());
	patrolPathLengthSize = 0;
}

void EditSession::SetZoomButton(ActorParams *ap)
{
	if (ap == NULL)
	{
		assert(selectedBrush->IsSingleActor());
		ap = selectedBrush->objects.front()->GetAsActor();
	}

	SetMode(SET_CAM_ZOOM);
	CameraShotParams *camShot = (CameraShotParams*)ap;
	currentCameraShot = camShot;
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




bool EditSession::PerformMovePointsAction()
{
	Vector2i delta = Vector2i(worldPos.x, worldPos.y) - editMouseOrigPos;
	//here the delta being subtracted is the points original positionv

	//commented the grabbedPoint thing out because it was crashing. but it maybe shouldn't be?

	PointMover *pm = new PointMover;//need to worry about alignextremes but for now just use the actual moving points.

	PolyPtr poly;
	std::list<PointMoveInfo> adjList;
	for (auto mit = selectedPoints.begin(); mit != selectedPoints.end(); ++mit)
	{
		adjList.clear();
		poly = (*mit).first;

		 //adjust this later!!! need to take this into account

		poly->AlignExtremes((*mit).second, adjList);

		vector<PointMoveInfo> &pmVec = pm->movePoints[poly];

		

		pmVec.reserve((*mit).second.size() + adjList.size() );

		for (auto it = (*mit).second.begin(); it != (*mit).second.end(); ++it)
		{
			(*it).newPos = (*it).GetPolyPoint()->pos;
			pmVec.push_back((*it));
		}

		for (auto it = adjList.begin(); it != adjList.end(); ++it)
		{
			(*it).newPos = (*it).GetPolyPoint()->pos;
			pmVec.push_back((*it));
		}

		pm->oldEnemyPosInfo.insert(pm->oldEnemyPosInfo.end(),
			poly->enemyPosBackups.begin(), poly->enemyPosBackups.end());

		poly->StoreEnemyPositions(pm->newEnemyPosInfo);

		poly->SoftReset();
		poly->Finalize();

		/*for (auto pit = pmVec.begin(); pit != pmVec.end(); ++pit)
		{
			(*pit).poly->SetGrassVecOn((*pit).pointIndex, (*pit).grassVec);
		}*/

		poly->SetGrassFromPointMoveInfoVectors(pmVec);

		poly->SetRenderMode(TerrainPolygon::RENDERMODE_NORMAL);

		for (auto it = pm->newEnemyPosInfo.begin(); it != pm->newEnemyPosInfo.end(); ++it)
		{
			(*it).first->posInfo = (*it).second;

			if ((*it).first->myEnemy != NULL)
				(*it).first->myEnemy->UpdateOnEditPlacement();

			(*it).first->UpdateGroundedSprite();
			(*it).first->SetBoundingQuad();
		}
	}

	RailPtr rail;
	for (auto mit = selectedRailPoints.begin(); mit != selectedRailPoints.end(); ++mit)
	{
		rail = (*mit).first;

		rail->AlignExtremes(); //adjust this later!!! need to take this into account

		vector<PointMoveInfo> &pmVec = pm->railMovePoints[rail];
		pmVec.reserve((*mit).second.size());
		for (auto it = (*mit).second.begin(); it != (*mit).second.end(); ++it)
		{
			(*it).newPos = (*it).GetRailPoint()->pos;
			pmVec.push_back((*it));
		}
		pm->oldEnemyPosInfo.insert(pm->oldEnemyPosInfo.end(),
			rail->enemyPosBackups.begin(), rail->enemyPosBackups.end());

		rail->StoreEnemyPositions(pm->newEnemyPosInfo);

		rail->SoftReset();
		rail->Finalize();
		rail->SetRenderMode(TerrainRail::RENDERMODE_NORMAL);
		//rail->UpdateBounds();
		/*rail->SoftReset();
		rail->Finalize();
		rail->SetRenderMode(TerrainPolygon::RENDERMODE_NORMAL);*/

		for (auto it = pm->newEnemyPosInfo.begin(); it != pm->newEnemyPosInfo.end(); ++it)
		{
			(*it).first->posInfo = (*it).second;

			if ((*it).first->myEnemy != NULL)
				(*it).first->myEnemy->UpdateOnEditPlacement();

			(*it).first->UpdateGroundedSprite();
			(*it).first->SetBoundingQuad();
		}
	}

	MoveBrushAction *action = new MoveBrushAction(selectedBrush, delta, false, pm);
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

	Brush * testBrush = pm->MakeBrush();

	bool validMove = false;
	ClearMostRecentError();
	if (testBrush->CanApply() && selectedBrush->CanApply() )
	{
		validMove = true;
	}
	else
	{
		ShowMostRecentError();
	}
	delete testBrush;
	//might want to pass pm to IsGateAttachedToAffectedPoint etc

	//check for validity
	if (validMove )//action->moveValid)
	{
		int gateActionsAdded = 0;
		for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
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

						return false;
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

		ClearUndoneActions();

		return true;
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

		return false;
	}
}



void EditSession::StartMoveSelectedPoints()
{
	PolyPtr poly;
	for (auto it = selectedPoints.begin(); it != selectedPoints.end(); ++it)
	{
		poly = (*it).first;

		poly->BackupEnemyPositions();
		TerrainPoint *point;
		bool rem;
		int numGrass = 0;
		for (auto pit = (*it).second.begin(); pit != (*it).second.end(); ++pit)
		{
			point = (*pit).GetPolyPoint();
			(*pit).origPos = point->pos;

			poly->FillGrassVec(point, (*pit).grassVec);
			poly->FillGrassVec(poly->GetPrevPoint(point->index), (*pit).prevGrassVec);
		}
	}

	RailPtr rail;
	for (auto it = selectedRailPoints.begin(); it != selectedRailPoints.end(); ++it)
	{
		rail = (*it).first;

		rail->BackupEnemyPositions();
		for (auto pit = (*it).second.begin(); pit != (*it).second.end(); ++pit)
		{
			(*pit).origPos = (*pit).GetRailPoint()->pos;
		}
	}
}

void EditSession::RevertMovedPoints(PointMap::iterator it)
{
	PolyPtr poly;
	TerrainPoint *curr, *prev;
	for (auto pit = (*it).second.begin(); pit != (*it).second.end(); ++pit)
	{
		poly = (*it).first;
		poly->SetPointPos((*pit).pointIndex, (*pit).oldPos);
		curr = poly->GetPoint((*pit).pointIndex);
		prev = poly->GetPrevPoint((*pit).pointIndex);

		auto currIt = poly->enemies.find(curr);
		if (currIt != poly->enemies.end())
		{
			list<ActorPtr> &currList = (*currIt).second;
			for (auto it = currList.begin(); it != currList.end(); ++it)
			{
				if ((*it)->myEnemy != NULL)
					(*it)->myEnemy->UpdateOnEditPlacement();

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
				(*it)->posInfo.groundQuantity = (*it)->oldQuant;
				//this is only on prev because the ground quant is not changed on curr
				if ((*it)->myEnemy != NULL)
				{
					(*it)->myEnemy->UpdateOnEditPlacement();
				}
				(*it)->UpdateGroundedSprite();
				(*it)->SetBoundingQuad();
			}
		}
	}
}

void EditSession::RevertMovedPoints(RailPointMap::iterator it)
{
	RailPtr rail;
	TerrainPoint *curr, *prev;
	for (auto pit = (*it).second.begin(); pit != (*it).second.end(); ++pit)
	{
		rail = (*it).first;
		rail->SetPointPos((*pit).pointIndex, (*pit).oldPos);
		curr = rail->GetPoint((*pit).pointIndex);
		prev = rail->GetPoint((*pit).pointIndex - 1);

		auto currIt = rail->enemies.find(curr);
		if (currIt != rail->enemies.end())
		{
			list<ActorPtr> &currList = (*currIt).second;
			for (auto it = currList.begin(); it != currList.end(); ++it)
			{
				if ((*it)->myEnemy != NULL)
					(*it)->myEnemy->UpdateOnEditPlacement();

				(*it)->UpdateGroundedSprite();
				(*it)->SetBoundingQuad();
			}
		}

		if (prev != NULL)
		{
			currIt = rail->enemies.find(prev);
			if (currIt != rail->enemies.end())
			{
				list<ActorPtr> &currList = (*currIt).second;
				for (auto it = currList.begin(); it != currList.end(); ++it)
				{
					(*it)->posInfo.groundQuantity = (*it)->oldQuant;
					//this is only on prev because the ground quant is not changed on curr
					if ((*it)->myEnemy != NULL)
					{
						(*it)->myEnemy->UpdateOnEditPlacement();
					}
					(*it)->UpdateGroundedSprite();
					(*it)->SetBoundingQuad();
				}
			}
		}
	}
}

void EditSession::MoveSelectedPoints()
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

	bool revert = false;
	
	for (auto it = selectedPoints.begin(); it != selectedPoints.end(); ++it)
	{
		poly = (*it).first;
		for (auto pit = (*it).second.begin(); pit != (*it).second.end(); ++pit)
		{
			i = (*pit).pointIndex;

			curr = poly->GetPoint(i);
			prev = poly->GetPrevPoint(i);

			(*pit).oldPos = curr->pos;
			edge = poly->GetEdge(i);
			prevEdge = poly->GetPrevEdge(i);

			oldPrevLength = prevEdge->GetLength();

			auto enemyIt = poly->enemies.find(prev);
			if (enemyIt != poly->enemies.end())
			{
				list<ActorPtr> &enemies = (*enemyIt).second;
				for (list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait)
				{
					(*ait)->oldQuant = (*ait)->posInfo.groundQuantity;
				}
			}
		}
	}

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

			edge = poly->GetEdge(i);
			prevEdge = poly->GetPrevEdge(i);

			oldPrevLength = prevEdge->GetLength();

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
				//double along = dot(-V2d(pointGrabDelta), edge->Along());
				//poly->MovePoint(i, -pointGrabDelta);
				revert = true;
				break;
				//poly->MovePoint(i, Vector2i( edge->Along() * along ));
				//poly->SetPointPos(i, Vector2i(edge->v1 - edge->Along() * maxQuant)); //works!
			}

			double minQuant;
			ActorPtr closest = poly->GetClosestEnemy(prev->GetIndex(), minQuant);
			if (closest != NULL && minQuant < 0)
			{
				revert = true;
				break;
				//poly->MovePoint(i, -pointGrabDelta);
				
				//poly->SetPointPos(i, Vector2i(prevEdge->v0 + prevEdge->Along() * (prevEdgeLen - minQuant)));
				//cout << "oldlen: " << oldPrevLength << ", prevEdgeLen: " << prevEdgeLen << "minQuant: " << minQuant << endl;
				if (prevEdgeLen != oldPrevLength)
				{
					auto enemyIt = poly->enemies.find(prev);
					if (enemyIt != poly->enemies.end())
					{
						list<ActorPtr> &enemies = (*enemyIt).second;
						for (list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait)
						{
							(*ait)->posInfo.groundQuantity = (*ait)->oldQuant;//-minQuant;
						}
					}
				}

				
			}

			poly->UpdateLineColor(prev->index);
			poly->UpdateLineColor(i);

			affected = true;

			auto currIt = poly->enemies.find(curr);
			if (currIt != poly->enemies.end())
			{
				list<ActorPtr> &currList = (*currIt).second;
				for (auto it = currList.begin(); it != currList.end(); ++it)
				{
					if ((*it)->myEnemy != NULL)
						(*it)->myEnemy->UpdateOnEditPlacement();

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
					//this is only on prev because the ground quant is not changed on curr
					if ((*it)->myEnemy != NULL)
					{
						(*it)->myEnemy->UpdateOnEditPlacement();
					}
					(*it)->UpdateGroundedSprite();
					(*it)->SetBoundingQuad();
				}
			}

		}

		//not sure why this comment exists
		//not sure why this is needed, but the edges are colliding w/ the enemies again

		if (!poly->IsInternallyValid() || revert )
		{
			revert = true;
			break;
		}
		else if (affected)
		{
			poly->SetRenderMode(TerrainPolygon::RENDERMODE_MOVING_POINTS);
			poly->UpdateLinePositions();
			poly->UpdateBounds();
		}
	}

	if (revert)
	{
		for (auto it = selectedPoints.begin(); it != selectedPoints.end(); ++it)
		{
			RevertMovedPoints(it);
		}
	}
}

void EditSession::MoveSelectedRailPoints(V2d worldPos)
{
	bool affected;
	TerrainPoint *curr, *prev;
	RailPtr rail;

	Edge *edge; //use prev edge also
	Edge *prevEdge;

	double edgeLen;
	double prevEdgeLen;

	bool failMove;
	double oldPrevLength;

	int i;

	bool revert = false;

	for (auto it = selectedRailPoints.begin(); it != selectedRailPoints.end(); ++it)
	{
		rail = (*it).first;
		for (auto pit = (*it).second.begin(); pit != (*it).second.end(); ++pit)
		{
			i = (*pit).pointIndex;

			curr = rail->GetPoint(i);
			prev = rail->GetPoint(i - 1);

			(*pit).oldPos = curr->pos;

			if (prev != NULL)
			{
				auto enemyIt = rail->enemies.find(prev);
				if (enemyIt != rail->enemies.end())
				{
					list<ActorPtr> &enemies = (*enemyIt).second;
					for (list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait)
					{
						(*ait)->oldQuant = (*ait)->posInfo.groundQuantity;
					}
				}
			}
		}
	}

	for (auto it = selectedRailPoints.begin(); it != selectedRailPoints.end(); ++it)
	{
		rail = (*it).first;

		if (rail->selected)
		{
			DeselectObject(rail);
		}

		affected = false;

		failMove = false;

		for (auto pit = (*it).second.begin(); pit != (*it).second.end(); ++pit)
		{
			i = (*pit).pointIndex;

			curr = rail->GetPoint(i);
			prev = rail->GetPoint(i - 1);


			prevEdge = rail->GetEdge(i - 1);

			if (prevEdge != NULL)
				oldPrevLength = prevEdge->GetLength();

			rail->MovePoint(i, pointGrabDelta);





			if (prevEdge != NULL)
				prevEdgeLen = prevEdge->GetLength();

			if (prevEdge != NULL && prevEdgeLen != oldPrevLength)
			{
				auto enemyIt = rail->enemies.find(prev);
				if (enemyIt != rail->enemies.end())
				{
					list<ActorPtr> &enemies = (*enemyIt).second;
					for (list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait)
					{
						(*ait)->posInfo.groundQuantity -= (oldPrevLength - prevEdgeLen);
					}
				}
			}

			affected = true;

			edge = rail->GetEdge(i);

			if (edge != NULL)
			{


				edgeLen = edge->GetLength();

				//doesnt yet cover both sides at once
				double maxQuant;
				ActorPtr furthest = rail->GetFurthestEnemy(i, maxQuant);
				if (furthest != NULL && maxQuant > edgeLen)
				{
					revert = true;
					break;
				}


				if (prev != NULL)
				{
					double minQuant;
					ActorPtr closest = rail->GetClosestEnemy(prev->GetIndex(), minQuant);
					if (closest != NULL && minQuant < 0)
					{
						revert = true;
						break;
						if (prevEdgeLen != oldPrevLength)
						{
							auto enemyIt = rail->enemies.find(prev);
							if (enemyIt != rail->enemies.end())
							{
								list<ActorPtr> &enemies = (*enemyIt).second;
								for (list<ActorPtr>::iterator ait = enemies.begin(); ait != enemies.end(); ++ait)
								{
									(*ait)->posInfo.groundQuantity = (*ait)->oldQuant;//-minQuant;
								}
							}
						}
					}
				}

				//rail->UpdateLineColor(prev->index);
				//rail->UpdateLineColor(i);

				auto currIt = rail->enemies.find(curr);
				if (currIt != rail->enemies.end())
				{
					list<ActorPtr> &currList = (*currIt).second;
					for (auto it = currList.begin(); it != currList.end(); ++it)
					{
						if ((*it)->myEnemy != NULL)
							(*it)->myEnemy->UpdateOnEditPlacement();

						(*it)->UpdateGroundedSprite();
						(*it)->SetBoundingQuad();
					}
				}
			}

			if (prev != NULL)
			{
				auto currIt = rail->enemies.find(prev);
				if (currIt != rail->enemies.end())
				{
					list<ActorPtr> &currList = (*currIt).second;
					for (auto it = currList.begin(); it != currList.end(); ++it)
					{
						//this is only on prev because the ground quant is not changed on curr
						if ((*it)->myEnemy != NULL)
						{
							(*it)->myEnemy->UpdateOnEditPlacement();
						}
						(*it)->UpdateGroundedSprite();
						(*it)->SetBoundingQuad();
					}
				}
			}

		}

		//not sure why this comment exists
		//not sure why this is needed, but the edges are colliding w/ the enemies again

		if (!rail->IsInternallyValid() || revert)
		{
			revert = true;
			break;
		}
		else if (affected)
		{
			rail->UpdateEnemyChain();
			rail->SetRenderMode(TerrainRail::RENDERMODE_MOVING_POINTS);
			rail->UpdateLines();
			rail->UpdateBounds();
		}
	}

	if (revert)
	{
		for (auto it = selectedRailPoints.begin(); it != selectedRailPoints.end(); ++it)
		{
			RevertMovedPoints(it);
		}
	}
}

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

	return false;
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

	return false;
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
	for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
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
	for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
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

	for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
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
	{
		CreateError(ERR_GATE_SAME_POINT);
		return false;
	}
		

	if (GateMakesSliverAngles(gi))
	{
		CreateError(ERR_GATE_CREATES_SLIVER);
		return false;
	}

	if (GateIntersectsPolys(gi))
	{
		CreateError(ERR_GATE_INSTERSECTS_ENEMY);
		return false;
	}

	if (GateIntersectsGates(gi))
	{
		CreateError(ERR_GATE_INTERSECTS_GATE);
		return false;
	}

	for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
	{
		if ((*it)->point0 == gi->point0 || (*it)->point0 == gi->point1
			|| (*it)->point1 == gi->point0 || (*it)->point1 == gi->point1)
		{
			CreateError(ERR_GATE_POINT_ALREADY_OCCUPIED);
			return false;
		}
	}

	if (GateIsTouchingEnemies(gi))
	{
		CreateError(ERR_GATE_INSTERSECTS_ENEMY);
		return false;
	}

	Vector2f center(Vector2f(gi->point0->pos + gi->point1->pos) / 2.f);
	if (gi->poly0 == gi->poly1)
	{
		if (gi->poly0->ContainsPoint(center))
		{
			CreateError(ERR_GATE_INSTERSECTS_ENEMY);
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
	for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
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

	MoveBrushAction *action = new MoveBrushAction(&b, adjust, true, NULL);
	action->Perform();

	//check for validity
	if (true)//action->moveValid)
	{
		compound->subActions.push_back(action);

		Brush attachedPolys;
		PolyPtr p0, p1;
		for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
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


		for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
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
	PointMover * pmap = new PointMover;
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

	poly->BackupEnemyPositions();

	pmap->oldEnemyPosInfo.insert(pmap->oldEnemyPosInfo.end(),
		poly->enemyPosBackups.begin(), poly->enemyPosBackups.end());

	point->pos = pi.newPos;

	list<PointMoveInfo> piList;
	piList.push_back(pi);


	list<PointMoveInfo> adjList;
	poly->AlignExtremes(piList, adjList );

	pmap->movePoints[poly].reserve(piList.size() + adjList.size() );
	for (auto it = piList.begin(); it != piList.end(); ++it)
	{
		pmap->movePoints[poly].push_back((*it));
	}

	for (auto it = adjList.begin(); it != adjList.end(); ++it)
	{
		pmap->movePoints[poly].push_back((*it));
	}

	poly->StoreEnemyPositions(pmap->newEnemyPosInfo);
	
	MoveBrushAction * action = new MoveBrushAction(selectedBrush, Vector2i(), true, pmap);
	//action->performed = true;
	action->Perform();

	//check validity here

	if (true)//action->moveValid)
	{
		compound->subActions.push_back(action);

		for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
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
	{
		CreateError(ERR_POLY_INTERSECTS_POLY);
		return false;
	}
		

	if (PolyContainsPolys(poly, ignore ))
	{
		CreateError(ERR_POLY_CONTAINS_POLY);
		return false;
	}

	if (PolyIsContainedByPolys(poly, ignore ))
	{
		CreateError(ERR_POLY_CONTAINS_POLY);
		return false;
	}

	if (PolyIsTouchingEnemiesOrBeingTouched(poly, ignore ))
	{
		CreateError(ERR_POLY_INTERSECTS_ENEMY);
		return false;
	}

	if (PolyIntersectsGates(poly))
	{
		CreateError(ERR_POLY_INTERSECTS_GATE);
		return false;
	}

	if (PolyGatesIntersectOthers(poly))
	{
		CreateError(ERR_POLY_INTERSECTS_GATE);
		return false;
	}

	if (PolyGatesMakeSliverAngles(poly))
	{
		CreateError(ERR_GATE_CREATES_SLIVER);
		return false;
	}
	return true;
}

bool EditSession::IsPolygonValid( PolyPtr poly, PolyPtr ignore )
{
	bool a = IsPolygonExternallyValid(poly, ignore);
	bool b = poly->IsInternallyValid();
	//cout << "a: " << a << " b: " << b << endl;

	return a && b;
}

Panel * EditSession::CreatePopupPanel( const std::string &type )
{
	Panel *p = NULL;
	if( type == "message" )
	{
		p = new Panel( "message_popup", 400, 100, this, true );
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
	else if( type == "gateselector" )
	{
		p = new Panel( "gate_popup", 200, 500, this );
	}
	else if( type == "terrainselector" )
	{
		p = new Panel( "terrain_popup", 100, 100, this, true );
	}
	else if (type == "bg")
	{
		p = new Panel("bg_popup", 1500, 600, this);
	}
	/*else if (type == "shardselector")
	{
		p = new Panel("shardselector", 700, 1080, this);
		p->AddLabel("shardtype", Vector2i(20, 900), 24, "SHARD_W1_TEACH_JUMP");
		CreateShardGridSelector(p, Vector2i(0, 0));
		p->AddButton("ok", Vector2i(100, 1000), Vector2f(100, 50), "OK");
	}*/

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
	//p->handler = ap;

	ap->SetPanelInfo();

	AddActivePanel(p);
}

void EditSession::SetDecorEditPanel()
{
	SelectPtr sp = selectedBrush->objects.front();
	assert(sp->selectableType == ISelectable::IMAGE);
	EditorDecorInfo *di = (EditorDecorInfo*)sp;

	/*editDecorPanel->textBoxes["xpos"]->text.setString(boost::lexical_cast<string>(di->spr.getPosition().x));
	editDecorPanel->textBoxes["ypos"]->text.setString(boost::lexical_cast<string>(di->spr.getPosition().y));

	editDecorPanel->textBoxes["rotation"]->text.setString(boost::lexical_cast<string>(di->spr.getRotation()));
	editDecorPanel->textBoxes["xscale"]->text.setString(boost::lexical_cast<string>(di->spr.getScale().x));
	editDecorPanel->textBoxes["yscale"]->text.setString(boost::lexical_cast<string>(di->spr.getScale().y));
	
	editDecorPanel->textBoxes["layer"]->text.setString(boost::lexical_cast<string>(di->layer));*/
}

void EditSession::SetDecorParams()
{
	/*SelectPtr sp = selectedBrush->objects.front();
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
	}*/
}

bool EditSession::CanCreateGate( GateInfo &testGate )
{
	//if (testGate.poly0 == NULL || testGate.poly1 == NULL)
	//	return false;
	//this function can later be moved into IsGateValid and cleaned up

	Vector2i v0 = testGate.point0->pos;
	Vector2i v1 = testGate.point1->pos;

	//no duplicate points
	for( list<GateInfoPtr>::iterator it = gateInfoList.begin(); it != gateInfoList.end(); ++it )
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
	int left, top, right, bot;
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
		int pLeft = 0;
		int pTop = 0;
		int pRight = 0;
		int pBot = 0;
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
		top = mapHeader->topBounds;
		right = pRight;
		bot = pBot;
	}

		
	if (left < mapHeader->leftBounds)
	{
		left = mapHeader->leftBounds;
	}
	else
	{
		left -= extraBound;
	}

	if (top < mapHeader->topBounds)
	{
		top = mapHeader->topBounds;
	}
	else
	{
		top -= extraBound;
	}
	int bRight = mapHeader->leftBounds + mapHeader->boundsWidth;
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
	
	terrainShader.setUniform("zoom", zoom);
	terrainShader.setUniform("topLeft", botLeft);

	oldShaderZoom = -1; //updates the shader back to normal after this is over

	mapPreviewTex->clear(Color::Black);

	mapPreviewTex->setView( pView );

	//DrawTopClouds(mapPreviewTex);

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

	bool oldSelected;


	for (auto it = waterPolygons.begin(); it != waterPolygons.end(); ++it)
	{
		oldSelected = (*it)->selected;
		(*it)->SetSelected(false);
		(*it)->Draw(false, 1, mapPreviewTex, false, NULL);
		(*it)->SetSelected(oldSelected);
	}

	for (auto it = flyPolygons.begin(); it != flyPolygons.end(); ++it)
	{
		oldSelected = (*it)->selected;
		(*it)->SetSelected(false);
		(*it)->Draw(false, 1, mapPreviewTex, false, NULL);
		(*it)->SetSelected(oldSelected);
	}

	for( auto it = polygons.begin(); it != polygons.end(); ++it )
	{
		oldSelected = (*it)->selected;
		(*it)->SetSelected(false);
		(*it)->Draw( false, 1, mapPreviewTex, false, NULL );
		(*it)->SetSelected(oldSelected);
	}

	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		(*it)->Draw(mapPreviewTex);
	}

	for (auto it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
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

	cs.setPosition(playerMarkers[0]->GetFloatPos());
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
	//ssPrev << filePath.parent_path().relative_path().string() << "/Previews/" << filePath.stem().string() << "_preview_" << imageSize.x << "x" << imageSize.y << ".png";


	//ssPrev << filePath.parent_path().relative_path().string() << "\\" << filePath.stem().string() << ".png";
	ssPrev << filePath.parent_path().string() << "\\" << filePath.stem().string() << ".png";
	std::string previewFile = ssPrev.str();
	img.saveToFile( previewFile );
	//currentFile
}

//needs cleanup badly
PositionInfo EditSession::ConvertPointToGround( sf::Vector2i testPoint, ActorPtr a, Brush * brush )
{
	

	PositionInfo gi;

	
	
	//PolyPtr poly = NULL;
	gi.ground = NULL;
	gi.railGround = NULL;

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
	double edgeLen;

	double testRadius = actorSize.y * ( 2.0 / 3.0 );//actorAABB.width / 3;//a->type->info.size.y /2;//actorAABB.height;//200
	//testPoint = a->GetIntPos();

	double minQuant = 0;//actorSize.x / 2;
	//double extra = actorSize.x;

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
				edge = (*it)->GetPrevEdge(i);

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

				//atm you can now place actors anywhere since they are all spheres. maybe some special
				//stuff later where you dont want this..can make another variable for it later

				edgeLen = edge->GetLength();
				if( brush->IsSingleActor() )
				{
					/*if (testQuantity >= 0 && testQuantity < minQuant)
						testQuantity = minQuant;
					else if (testQuantity > length(cu - pr) - minQuant && testQuantity <= length(cu - pr))
					{
						testQuantity = floor(length(cu - pr) - minQuant);
					}*/
					if (testQuantity > -actorSize.x / 2.0 && testQuantity < 0)
						testQuantity = 0;
					else if (testQuantity > edgeLen && testQuantity <= edgeLen + actorSize.x / 2.0)
					{
						testQuantity = floor(edgeLen);
					}
				}

				

				V2d newPoint(pr.x + (cu.x - pr.x) * (testQuantity / edgeLen), pr.y + (cu.y - pr.y) *
					(testQuantity / edgeLen));

				V2d norm = edge->Normal();
				//if( cross( worldPos - edge->v0,edge->Along()))

				/*if (!(*it)->CheckOtherSideRay(worldPos, newPoint, edge))
				{
					continue;
				}*/


				if (((dist >= 0 && dist < testRadius) || (pointInPoly && dist < 0 && dist > - 200 )) 
					&& testQuantity >= minQuant && testQuantity <= edgeLen - minQuant
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
					next = rail->GetPoint(i+1);

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
	return GetCorrectPolygonList(t->GetSpecialPolyIndex());
}

list<PolyPtr> & EditSession::GetCorrectPolygonList(int ind)
{
	switch (ind)
	{
	case 0:
		return polygons;
	case 1:
		return waterPolygons;
	case 2:
		return flyPolygons;
	default:
		assert(0);
		return polygons;
	}
}

list<PolyPtr> & EditSession::GetCorrectPolygonList()
{
	int specialTerrainMode = GetSpecialTerrainMode();
	return GetCorrectPolygonList(specialTerrainMode);
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

int EditSession::GetCurrTerrainWorld()
{
	return currTerrainWorld[GetSpecialTerrainMode()];
}

int EditSession::GetCurrTerrainVariation()
{
	return currTerrainVar[GetSpecialTerrainMode()];
}

bool EditSession::ExecuteTerrainCompletion()
{	
	ClearMostRecentError();

	if (!polygonInProgress->IsCompletionValid())
	{
		ShowMostRecentError();
		return false;
	}

	polygonInProgress->SetMaterialType(GetCurrTerrainWorld(),
		GetCurrTerrainVariation());

	polygonInProgress->UpdateBounds();

	bool applyOkay = true;

	int liRes;

	bool tryMakeInverse = 
		createTerrainModeUI->GetCurrTerrainTool() == TERRAINTOOL_SETINVERSE;//IsKeyPressed(Keyboard::LAlt);

	auto &testPolygons = GetCorrectPolygonList(polygonInProgress);
	ClearMostRecentError();
	for (auto it = testPolygons.begin(); it != testPolygons.end(); ++it)
	{
		if (!(*it)->inverse && (*it)->Contains(polygonInProgress))
		{
			applyOkay = false;
			polygonInProgress->ClearPoints();
			break;
		}

		if (tryMakeInverse && !(*it)->inverse )
		{
			liRes = polygonInProgress->LinesIntersect((*it));
			if (liRes > 0)
			{
				CreateError(ERR_INVERSE_CANT_INTERSECT_NORMAL_POLYS);
				applyOkay = false;
				polygonInProgress->ClearPoints();
				break;
			}
		}
	}

	if (!applyOkay)
	{
		ShowMostRecentError();
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

			bool add = 
				createTerrainModeUI->GetCurrTerrainTool() == TERRAINTOOL_ADD;//!(IsKeyPressed(Keyboard::LShift) || IsKeyPressed(Keyboard::RShift));
			bool success = false;
			if (add)
			{
				success = ExecuteTerrainMultiAdd(inProgress, orig, result, GetSpecialTerrainMode() );
			}
			else
			{
				success = ExecuteTerrainMultiSubtract(inProgress, orig, result, GetSpecialTerrainMode());
			}

			bool changesMade = !orig.IsEmpty() || !result.IsEmpty();
			if (success && changesMade )
			{
				ClearUndoneActions(); //critical to have this before the deactivation

				Action *replaceAction = new ReplaceBrushAction(&orig, &result, mapStartBrush);
				replaceAction->Perform();

				AddDoneAction(replaceAction);

				polygonInProgress->ClearPoints();
			}
			else
			{
				if (TOOL_BOX)
				{
					cout << "box made no changes" << endl;
					polygonInProgress->ClearPoints();
				}
			}
		}
		return true;
	}

	return false;
}

void EditSession::ExecuteRailCompletion()
{
	int numP = railInProgress->GetNumPoints();
	if (numP >= 2)
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
			//MessagePop("polygon is invalid!!! new message");
		}
		else
		{
			if (railAttachStart == NULL && railAttachEnd == NULL )
			{
				bool empty = true;
				//eventually be able to combine rails by putting your start/end points at their starts/ends
				if (empty)
				{
					if (trackingEnemyParams == NULL && editModeUI->currParams == NULL)
					{
						railInProgress->SetRailType(createRailModeUI->GetRailType());
					}
					else if (editModeUI->currParams != NULL)
					{
						ActorPtr testParams = editModeUI->currParams->Copy();
						railInProgress->enemyParams = testParams;
					}

					railInProgress->Finalize();

					progressBrush->Clear();
					
					

					progressBrush->AddObject(railInProgress);

					ClearUndoneActions();

					Brush orig;
					if (trackingEnemyParams != NULL)
					{
						orig.AddObject(trackingEnemyParams);
						FinishEnemyCreation();
						SetMode(CREATE_ENEMY);
					}
					else if (editModeUI->currParams != NULL)
					{
						
						if (editModeUI->currRail != NULL)
						{
							orig.AddObject(editModeUI->currRail);
						}
						else
						{
							orig.AddObject(editModeUI->currParams);
						}
						ClearSelectedBrush();

						SetMode(EDIT);

						SelectObject(railInProgress);
						editModeUI->SetCurrRailPanel(railInProgress);
					}

					Action *action = new ReplaceBrushAction(&orig, progressBrush, mapStartBrush);
					//Action *action = new ApplyBrushAction(progressBrush);

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
			else
			{
				RailPtr newRail = new TerrainRail;

				Brush oldBrush;

				if (railAttachStart != NULL && railAttachEnd == NULL)
				{
					int numPointsOld = railAttachStart->GetNumPoints();
					int numPointsProgress = railInProgress->GetNumPoints();
					int total = numPointsOld + numPointsProgress;
					newRail->Reserve(total - 1);
					//-1 here because theres a duplicate where they meet

					bool attachedAtOldStart = (railAttachStartPoint->index == 0);

					if (attachedAtOldStart)
					{
						for (int i = numPointsProgress - 1; i >= 1; --i)
						{
							newRail->AddPoint(railInProgress->GetPoint(i)->pos, false);
						}

						for (int i = 0; i < numPointsOld; ++i)
						{
							newRail->AddPoint(railAttachStart->GetPoint(i)->pos, false);
						}
					}
					else
					{
						for (int i = 0; i < numPointsOld; ++i)
						{
							newRail->AddPoint(railAttachStart->GetPoint(i)->pos, false);
						}

						for (int i = 1; i < numPointsProgress; ++i)
						{
							newRail->AddPoint(railInProgress->GetPoint(i)->pos, false);
						}
					}

					oldBrush.AddObject(railAttachStart);
				}
				else if (railAttachEnd != NULL && railAttachStart == NULL)
				{
					int numPointsOld = railAttachEnd->GetNumPoints();
					int numPointsProgress = railInProgress->GetNumPoints();
					int total = numPointsOld + numPointsProgress;
					newRail->Reserve(total - 1);

					bool attachedAtOldStart = (railAttachEndPoint->index == 0);

					if (attachedAtOldStart)
					{
						for (int i = 0; i < numPointsProgress - 1; ++i )
						{
							newRail->AddPoint(railInProgress->GetPoint(i)->pos, false);
						}

						for (int i = 0; i < numPointsOld; ++i)
						{
							newRail->AddPoint(railAttachEnd->GetPoint(i)->pos, false);
						}
					}
					else
					{
						for (int i = 0; i < numPointsOld; ++i)
						{
							newRail->AddPoint(railAttachEnd->GetPoint(i)->pos, false);
						}

						for (int i = numPointsProgress - 2; i >= 0; --i)
						{
							newRail->AddPoint(railInProgress->GetPoint(i)->pos, false);
						}
					}

					oldBrush.AddObject(railAttachEnd);
				}
				else
				{
					//both
					int numPointsOldStart = railAttachStart->GetNumPoints();
					int numPointsOldEnd = railAttachEnd->GetNumPoints();
					int numPointsProgress = railInProgress->GetNumPoints();
					int total = numPointsOldStart + numPointsOldEnd + numPointsProgress;
					newRail->Reserve( total - 2 );
					//2 duplicate points

					bool attachedAtOldStartStart = (railAttachStartPoint->index == 0);
					bool attachedAtOldEndStart = (railAttachEndPoint->index == 0);

					if (attachedAtOldStartStart && attachedAtOldEndStart)
					{
						//if attached to both heads 

						for (int i = numPointsOldStart - 1; i >= 0; --i)
						{
							newRail->AddPoint(railAttachStart->GetPoint(i)->pos, false);
						}

						for (int i = 1; i < numPointsProgress - 1; ++i)
						{
							newRail->AddPoint(railInProgress->GetPoint(i)->pos, false);
						}

						for (int i = 0; i < numPointsOldEnd; ++i)
						{
							newRail->AddPoint(railAttachEnd->GetPoint(i)->pos, false);
						}
					}
					else if (!attachedAtOldStartStart && !attachedAtOldEndStart)
					{

						//both tails
						for (int i = 0; i < numPointsOldStart; ++i)
						{
							newRail->AddPoint(railAttachStart->GetPoint(i)->pos, false);
						}

						for (int i = 1; i < numPointsProgress - 1; ++i)
						{
							newRail->AddPoint(railInProgress->GetPoint(i)->pos, false);
						}

						for (int i = numPointsOldEnd-1; i >= 0; --i)
						{
							newRail->AddPoint(railAttachEnd->GetPoint(i)->pos, false);
						}
					}
					else if (attachedAtOldStartStart && !attachedAtOldEndStart)
					{
						for (int i = 0; i < numPointsOldEnd; ++i)
						{
							newRail->AddPoint(railAttachEnd->GetPoint(i)->pos, false);
						}

						for (int i = numPointsProgress - 2; i >= 1; --i)
						{
							newRail->AddPoint(railInProgress->GetPoint(i)->pos, false);
						}

						for (int i = 0; i < numPointsOldStart; ++i)
						{
							newRail->AddPoint(railAttachStart->GetPoint(i)->pos, false);
						}
					}
					else if (!attachedAtOldStartStart && attachedAtOldEndStart)
					{
						for (int i = 0; i < numPointsOldStart; ++i)
						{
							newRail->AddPoint(railAttachStart->GetPoint(i)->pos, false);
						}

						for (int i = 1; i < numPointsProgress - 1; ++i)
						{
							newRail->AddPoint(railInProgress->GetPoint(i)->pos, false);
						}

						for (int i = 0; i < numPointsOldEnd; ++i)
						{
							newRail->AddPoint(railAttachEnd->GetPoint(i)->pos, false);
						}
					}
					oldBrush.AddObject(railAttachStart);
					oldBrush.AddObject(railAttachEnd);
				}

				if (trackingEnemyParams == NULL && editModeUI->currParams == NULL)
					newRail->SetRailType(createRailModeUI->GetRailType());
				else
					newRail->SetRailType(railInProgress->GetRailType());

				newRail->Finalize();

				railInProgress->ClearPoints();
				
				Brush newBrush;

				newBrush.AddObject(newRail);

				ReplaceBrushAction *action = new ReplaceBrushAction(&oldBrush,
					&newBrush, mapStartBrush);

				action->Perform();

				AddDoneAction(action);

				ClearUndoneActions();

				railAttachStart = NULL;
				railAttachEnd = NULL;
				railAttachStartPoint = NULL;
				railAttachEndPoint = NULL;
			}
		}
	}
	else if (numP < 2 && numP > 0)
	{
		if (trackingEnemyParams != NULL)
		{
			railInProgress->ClearPoints();
			FinishEnemyCreation();
			SetMode(CREATE_ENEMY);
		}
		else if (editModeUI->currParams != NULL)
		{
			railInProgress->ClearPoints();

			if (editModeUI->currRail != NULL)
			{
				ActorPtr testParams = editModeUI->currParams->Copy();
				testParams->SetPath(vector<Vector2i>());
				testParams->CreateMyEnemy();

				Brush orig;
				Brush result;

				orig.AddObject(editModeUI->currRail);
				ClearSelectedBrush();

				SetMode(EDIT);

				result.AddObject(testParams);

				Action *action = new ReplaceBrushAction(&orig, &result, mapStartBrush);

				action->Perform();
				AddDoneAction(action);

				SelectObject(testParams);
				editModeUI->SetEnemyPanel(testParams);
			}

			SetMode(EDIT);
		}
		else
		{
			cout << "cant finalize. cant make rail" << endl;
			railInProgress->ClearPoints();
		}
		
	}
}

void EditSession::SetInversePoly()
{
	polygonInProgress->FixWinding();
	//polygonInProgress->Finalize();

	polygonInProgress->FinalizeInverse();
	

	Brush orig;
	list<GateInfoPtr> gateList;
	if( inversePolygon != NULL )
	{
		AddFullPolyToBrush(inversePolygon, gateList, &orig);
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

Vector2i EditSession::GetCopiedCenter()
{
	if (copiedBrush != NULL && freeActorCopiedBrush != NULL)
	{
		IntRect copiedRect = copiedBrush->GetAABB();
		IntRect freeRect = freeActorCopiedBrush->GetAABB();

		int left = min(freeRect.left, copiedRect.left);
		int right = max(freeRect.left + freeRect.width, copiedRect.left + copiedRect.width );
		int top = min(freeRect.top, copiedRect.top);
		int bot = max(freeRect.top + freeRect.height, copiedRect.top + copiedRect.height);

		return Vector2i((right + left) / 2, (top + bot) / 2);
	}
	else if (copiedBrush != NULL)
	{
		return copiedBrush->GetCenter();
	}
	else if (freeActorCopiedBrush != NULL)
	{
		return freeActorCopiedBrush->GetCenter();
	}
}

bool EditSession::PasteInverseTerrain(Brush *cBrush)
{
	list<PolyPtr> inverseCopiedList;

	PolyPtr poly;
	if (cBrush != NULL)
	{
		for (auto bit = cBrush->objects.begin(); bit != cBrush->objects.end(); ++bit)
		{
			poly = (*bit)->GetAsTerrain();
			if (poly != NULL)
			{
				if (poly->copiedInverse)
				{
					inverseCopiedList.push_back(poly);
					break;
				}
			}
		}
	}

	Brush orig;
	Brush result;
	bool success = true;

	if (!inverseCopiedList.empty())
	{
		//opposite of normal
		if (!HoldingControl())
		{
			success = ExecuteTerrainMultiSubtract(inverseCopiedList, orig, result, 0);
		}
		else
		{
			success = ExecuteTerrainMultiAdd(inverseCopiedList, orig, result, 0);
		}
	}
	else
	{
		return true;
	}

	if (success && (!orig.IsEmpty() || !result.IsEmpty()))
	{
		ClearUndoneActions(); //critical to have this before the deactivation

		orig.Deactivate();
		result.Activate();

		if (complexPaste == NULL)
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

	return success;
}

void EditSession::PasteTerrain(Brush *cBrush, Brush *freeActorBrush)
{
	if (!PasteInverseTerrain(cBrush))
	{
		return;
	}

	std::vector<list<PolyPtr>> brushPolyLists;
	brushPolyLists.resize(TERRAINLAYER_Count);
	PolyPtr poly;

	bool terrainEmpty = true;

	if (cBrush != NULL)
	{
		for (auto bit = cBrush->objects.begin(); bit != cBrush->objects.end(); ++bit)
		{
			poly = (*bit)->GetAsTerrain();
			if (poly != NULL )
			{
				if (poly->copiedInverse)
				{     
				}
				else
				{
					brushPolyLists[poly->GetSpecialPolyIndex()].push_back(poly);
					terrainEmpty = false;
				}
					
			}
		}
	}

	Brush orig;
	Brush result;
	
	bool success = true;

	for (int i = TERRAINLAYER_Count - 1; i >= 0; --i)
	{
		list<PolyPtr> &currList = brushPolyLists[i];
		if (!currList.empty())
		{
			if (HoldingControl())
			{
				success = ExecuteTerrainMultiSubtract(currList, orig, result, i);
			}
			else
			{
				success = ExecuteTerrainMultiAdd(currList, orig, result, i);
			}

			if (!success)
			{
				//full resultbrush is destroyed by both functions when they return false
				break;
			}
		}
	}

	//if( terrainEmpty )
	//freeactors are bugged right now
	{
		if ( freeActorBrush != NULL && freeActorBrush->CanApply())
		{
			ActorPtr actor;
			ActorPtr newActor;
			for (auto it = freeActorBrush->objects.begin(); it != freeActorBrush->objects.end(); ++it)
			{
				actor = (*it)->GetAsActor();
				if (actor != NULL)
				{
					newActor = actor->Copy();
					if (actor->myEnemy != NULL)
					{
						newActor->CreateMyEnemy();
					}

					result.AddObject(newActor);
				}
			}
		}
	}

	RailPtr rail;
	if (cBrush != NULL)
	{
		for (auto bit = cBrush->objects.begin(); bit != cBrush->objects.end(); ++bit)
		{
			rail = (*bit)->GetAsRail();
			if (rail != NULL)
			{
				result.AddObject(rail->Copy());
				//rail enemies go here?
			}
		}
	}

	DecorPtr dec;
	if (cBrush != NULL)
	{
		for (auto bit = cBrush->objects.begin(); bit != cBrush->objects.end(); ++bit)
		{
			dec = (*bit)->GetAsDecor();
			if (dec != NULL)
			{
				result.AddObject(dec->Copy());
				//decorList.push_back(dec);
			}
		}
	}

	if( success && (!orig.IsEmpty() || !result.IsEmpty() ))
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
	Brush &orig, Brush &resultBrush, int terrainLayer )
{
	//change this eventually to reflect the actual layer. maybe pass in which layer im on?
	auto &testPolygons = GetCorrectPolygonList(terrainLayer);
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
		bool madeInverse = false;

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
				madeInverse = true;
				(*it)->MakeInverse();
				break;
			}
		}

		if (!madeInverse)
		{
			//when you've subtracted and you have a polygon touching a point on the inverse.
			resultBrush.Destroy();
			return false;
			
		}

		

		for (auto it = inverseResults.begin(); it != inverseResults.end(); ++it)
		{
			(*it)->Finalize();
		}

		
	}


	AddFullPolysToBrush(containedPolys, gateInfoList, &orig);
	//AddFullPolysToBrush(nonInverseIntersList, gateInfoList, &orig);
	
	
	if (inversePolygon != NULL)
	{
		TryAttachActorsToPoly(inversePolygon, attachList, &resultBrush);
		TryKeepGrass(inversePolygon, attachList);
	}
	

	for (auto it = brushPolys.begin(); it != brushPolys.end(); ++it)
	{
		if ((*it)->copiedInverse)
		{
			TryAttachActorsToPoly((*it), attachList, &resultBrush);
			break;
		}
	}

	TryAttachActorsToPolys(nonInverseIntersList, attachList, &resultBrush);
	TryKeepGrass(nonInverseIntersList, attachList);
	//TryAttachActorsToPolys(inverseConnectedInters, attachList, &resultBrush);

	TryKeepGates(gateInfoList, attachList, &resultBrush);
	
	return true;
}

bool EditSession::ExecuteTerrainMultiAdd(list<PolyPtr> &brushPolys,
	Brush &orig, Brush &resultBrush, int terrainLayer )
{
	//change this eventually to reflect the actual layer. maybe pass in which layer im on?
	auto &testPolygons = GetCorrectPolygonList(terrainLayer);//GetCorrectPolygonList(polygonInProgress);
	
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
				//for (auto tempIt = tempContained.begin(); tempIt != tempContained.end(); ++tempIt)
				//{
				//	//cant be a duplicate only because brushes won't be on top of one another.
				//	containedPolys.push_back((*tempIt));
				//}

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
			
			//i think this should be more general, so it goes here instead of in one case.
			for (auto tempIt = tempContained.begin(); tempIt != tempContained.end(); ++tempIt)
			{
				//cant be a duplicate only because brushes won't be on top of one another.
				containedPolys.push_back((*tempIt));
			}
			++brushIt;
		}
	}

	if (inversePolygon != NULL)
	{


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
	}

	
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

	bool allSameType = true;
	int sameWorld = -1;
	int sameVar = -1;
	for (auto it = nonInverseInters.begin(); it != nonInverseInters.end(); ++it)
	{
		if (sameWorld == -1)
		{
			sameWorld = (*it)->terrainWorldType;
			sameVar = (*it)->terrainVariation;
		}
		else
		{
			int currWorld = (*it)->terrainWorldType;
			int currVar = (*it)->terrainVariation;
			if (currWorld != sameWorld || currVar != sameVar)
			{
				allSameType = false;
				break;
			}
		}
	}

	if (allSameType && intersectsInverse)
	{
		if (sameWorld == -1)
		{
			sameWorld = inversePolygon->terrainWorldType;
			sameVar = inversePolygon->terrainVariation;
		}
		else
		{
			int currWorld = inversePolygon->terrainWorldType;
			int currVar = inversePolygon->terrainVariation;
			if (currWorld != sameWorld || currVar != sameVar)
			{
				allSameType = false;
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
				delete newPoly;
				continue;
				//newPoly->TryFixPointsTouchingLines();
			}
			
			
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
				if (allSameType)
				{
					finalCheckVec[i].first->SetMaterialType(sameWorld, sameVar);
				}
				else
				{
					finalCheckVec[i].first->SetMaterialType(GetCurrTerrainWorld(), GetCurrTerrainVariation());
				}
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
			Vector2i playerIntPos(playerMarkers[i]->GetIntPos());
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

			if (allSameType)
			{
				newInverse->SetMaterialType(sameWorld, sameVar);
			}
			else
			{
				newInverse->SetMaterialType(GetCurrTerrainWorld(), GetCurrTerrainVariation());
			}

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

		//AddFullPolyToBrush(inversePolygon, gateInfoList, &orig);
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

	//if (complexPaste == NULL)
	{
		TryAttachActorsToPolys(brushPolys, attachList, &resultBrush);
		//TryKeepGrass(brushPolys, attachList);
	}
	
	TryAttachActorsToPolys(nonInverseInters, attachList, &resultBrush);
	TryKeepGrass(nonInverseInters, attachList);
	TryAttachActorsToPolys(inverseConnectedInters, attachList, &resultBrush);
	TryKeepGrass(inverseConnectedInters, attachList);

	if (inverseConnectedPolys.size() > 1 || inverseOnlyBrushes.size() > 0)
	{
		TryAttachActorsToPoly(inversePolygon, attachList, &resultBrush);
		TryKeepGrass(inversePolygon, attachList);
	}
	
	TryKeepGates(gateInfoList, attachList, &resultBrush);
	return true;
}

bool EditSession::PointSelectTerrain(V2d &pos, int terrainLayer )
{
	if (editModeUI->IsEditPointsOn())
	{
		if (PointSelectPolyPoint(pos, terrainLayer ))
		{
			return true;
		}
	}
	else
	{
		if (PointSelectPoly(pos, terrainLayer ))
		{
			return true;
		}
	}

	return false;
}

bool EditSession::PointSelectPolyPoint( V2d &pos, int terrainLayer )
{
	bool shift = IsKeyPressed(Keyboard::LShift) || IsKeyPressed(Keyboard::RShift);

	auto & currPolyList = GetCorrectPolygonList(terrainLayer);

	TerrainPoint *foundPoint = NULL;
	for (auto it = currPolyList.begin(); it != currPolyList.end(); ++it)
	{
		foundPoint = (*it)->GetClosePoint( 8 * zoomMultiple, pos);
		if (foundPoint != NULL)
		{
			//if (shift && foundPoint->selected )
			//{
			//	grabbedPoint = foundPoint;
			//	worldPos = V2d(grabbedPoint->pos);
			//	//DeselectPoint((*it), foundPoint);
			//}
			//else
			{	
				if (!foundPoint->selected)
				{
					if (!shift)
						ClearSelectedPoints();

					SelectPoint((*it), foundPoint);
					grabbedPoint = foundPoint;
					worldPos = V2d(grabbedPoint->pos);

				}
				else
				{
					grabbedPoint = foundPoint;
					worldPos = V2d(grabbedPoint->pos);
				}
			}
			return true;
		}
	}
	return false;
}

bool EditSession::PointSelectRail(V2d &pos)
{
	if (editModeUI->IsEditPointsOn())
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
			if (!foundPoint->selected)
			{
				if (!shift)
					ClearSelectedPoints();

				SelectPoint((*it), foundPoint);
			}

			grabbedPoint = foundPoint;
			worldPos = V2d(grabbedPoint->pos);
			return true;
		}
	}
	return false;
}

bool EditSession::PointSelectPoly(V2d &pos, int terrainLayer )
{
	auto & currPolyList = GetCorrectPolygonList(terrainLayer);
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
	double radius, int terrainLayer )
{
	if (r.width == 0 || r.height == 0)
		return false;

	auto & currPolyList = GetCorrectPolygonList(terrainLayer);

	bool specialMode = GetSpecialTerrainMode() != 0;

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
	if (rect.width == 0 || rect.height == 0)
		return false;

	if (!editModeUI->IsLayerActionable(LAYER_ACTOR))
	{
		return false;
	}

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
	if (rect.width == 0 || rect.height == 0)
		return false;

	if (!editModeUI->IsLayerActionable(LAYER_IMAGE))
	{
		return false;
	}

	bool found = false;

	for (int i = EffectLayer::EFFECTLAYER_Count - 1; i >= 0; --i)
	{
		auto &dList = decorImages[i];

		for (auto it = dList.begin(); it != dList.end(); ++it)
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
	}

	return found;
}

bool EditSession::BoxSelectPolys(sf::IntRect &rect, int terrainLayer )
{
	bool found = false;

	if (rect.width == 0 || rect.height == 0)
		return false;

	auto & currPolyList = GetCorrectPolygonList(terrainLayer);

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

	return found;
}

bool EditSession::BoxSelectRails(sf::IntRect &rect)
{
	if (rect.width == 0 || rect.height == 0)
		return false;

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

	bool specialMode = GetSpecialTerrainMode() != 0;

	if (!HoldingShift())
	{
		//clear everything
		ClearSelectedBrush();
	}

	if ( !specialMode && BoxSelectActors(r))
	{
		selectionEmpty = false;
	}

	if ( !specialMode && BoxSelectDecor(r))
	{
		selectionEmpty = false;
	}

	if (editModeUI->IsEditPointsOn()) //always use point selection for now
	{
		if (HoldingShift())
		{
			//ClearSelectedPoints();
		}

		for (int i = TERRAINLAYER_Count - 1; i >= 0; --i)
		{
			if (editModeUI->IsLayerActionable(editModeUI->terrainEditLayerMap[i])
				&& BoxSelectPoints(r, 8 * zoomMultiple,
				i))
			{
				selectionEmpty = false;
			}
		}
	}
	else if (!editModeUI->IsEditPointsOn())//polygon selection. don't use it for a little bit
	{
		for (int i = TERRAINLAYER_Count - 1; i >= 0; --i)
		{
			if (editModeUI->IsLayerActionable(editModeUI->terrainEditLayerMap[i])
				&& BoxSelectPolys(r,i))
			{
				selectionEmpty = false;
			}
		}

		if (!specialMode && BoxSelectRails(r))
		{
			selectionEmpty = false;
		}
	}

	if (selectionEmpty)
	{
		ClearSelectedBrush();
	}
	else
	{
		if (selectedBrush->IsSingleActor())
		{
			ActorPtr a = selectedBrush->GetFirst()->GetAsActor();
			assert(a != NULL);
			if (a->type->panel != NULL)
			{
				a->SetPanelInfo();
				editModeUI->SetEnemyPanel(a);
			}
			
		}
		else if (selectedBrush->IsSingleRail())
		{
			RailPtr r = selectedBrush->GetFirst()->GetAsRail();
			editModeUI->SetCurrRailPanel(r);
		}
		//if( selectedBrush->issingl)
	}
}

double EditSession::GetZoomedMinEdgeLength()
{
	 return minimumEdgeLength * std::max(zoomMultiple, 1.0);
}

void EditSession::UpdateGrass()
{
	//if (showGrass)
	//{
	//	for (auto it = polygons.begin(); it != polygons.end(); ++it)
	//	{
	//		(*it)->UpdateGrass();
	//	}
	//	/*PolyPtr tp;
	//	for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
	//	{
	//		tp = (*it)->GetAsTerrain();
	//		if (tp != NULL)
	//			tp->UpdateGrass();
	//	}*/
	//}
}

void EditSession::ModifyGrass()
{
	if (editModeUI->IsShowGrassOn() && IsMousePressed(Mouse::Left)
		&& !justCompletedPolyWithClick )
	{
		for (auto it = polygons.begin(); it != polygons.end(); ++it)
		{
			(*it)->SwitchGrass(worldPos, !HoldingShift(), true, currGrassType );
		}
	}
}

void EditSession::ClearActivePanels()
{
	if (focusedPanel != NULL && focusedPanel != generalUI->mainPanel)
	{
		focusedPanel->Deactivate();
		focusedPanel = NULL;
	}

	activePanels.clear();
}

void EditSession::AddActivePanel(Panel *p)
{
	for (auto it = activePanels.begin(); it != activePanels.end(); ++it)
	{
		if ((*it) == p)
		{
			return;
		}
	}
	activePanels.push_back(p);
}

void EditSession::RemoveActivePanel(Panel *p)
{
	if (p == NULL)
		return;

	bool found = (find(activePanels.begin(), activePanels.end(), p)) != activePanels.end();
	if (found)
	{
		activePanels.remove(p);
		if (focusedPanel == p)
		{
			p->Deactivate();
			focusedPanel = NULL;
		}
	}
}

void EditSession::CleanupTestPlayerMode()
{
	fader->Reset();
	swiper->Reset();

	ResetEnemies();

	Enemy *curr = activeEnemyList;
	while (curr != NULL)
	{
		Enemy *next = curr->next;
		RemoveEnemy(curr);
		curr = next;
	}

	CleanupGoalFlow();
	CleanupGoalPulse();

	playerTracker->HideAll();

	mapHeader->leftBounds = realLeftBounds;
	mapHeader->topBounds = realTopBounds;
	mapHeader->boundsWidth = realBoundsWidth;
	mapHeader->boundsHeight = realBoundsHeight;

	auto testPolys = GetCorrectPolygonList(0);
	for (auto it = testPolys.begin(); it != testPolys.end(); ++it)
	{
		(*it)->ResetState();
	}

	for (auto it = rails.begin(); it != rails.end(); ++it)
	{
		(*it)->ResetState();
	}

	if (ggpo != NULL)
	{


		ggpo_close_session(ggpo);
		ggpo = NULL;

		WSACleanup();

		delete currSaveState;
		delete ngs;
		delete[] ggpoPlayers;
	}
}

void EditSession::SetMode(Emode m)
{


	Emode oldMode = mode;
	if (oldMode == SELECT_MODE)
	{
		oldMode = menuDownStored;
	}

	errorBar.SetShown(false);

	mode = m;

	switch (oldMode)
	{
	case CREATE_RAILS:
		if (trackingEnemyParams != NULL)
		{
			railInProgress->ClearPoints();
			FinishEnemyCreation();
		}
		break;
	case EDIT:
	{
		if (editModeUI->IsShowGrassOn())
		{
			editModeUI->FlipShowGrass();
			ShowGrass(false);
		}
		break;
	}
		
	case CREATE_TERRAIN:
		break;
	case TEST_PLAYER:
	{
		CleanupTestPlayerMode();
		//playerTracker->SetOn(false);
		break;
	}
	case CREATE_GATES:
	{
		createGatesModeUI->CompleteEditingGate();
		createGatesModeUI->modifyGate = NULL;
		break;
	}
	}

	ClearActivePanels();

	switch (mode)
	{
	case CREATE_TERRAIN:
		justCompletedPolyWithClick = false;
		createTerrainModeUI->SetShown(true);
		createTerrainModeUI->SetDrawTool(TOOL_DRAW);
		createTerrainModeUI->SetGridSize(graph->GetSpacing());
		break;
	case CREATE_RAILS:
		createRailModeUI->SetShown(true);
		railAttachStart = NULL;
		railAttachStartPoint = NULL;
		railAttachEnd = NULL;
		railAttachEndPoint = NULL;
		justCompletedRailWithClick = false;
		createRailModeUI->SetGridSize(graph->GetSpacing());
		break;
	case CREATE_GATES:
		gatePoints = 0;
		createGatesModeUI->SetShown(true);
		break;
	case CREATE_ENEMY:
		trackingEnemyParams = NULL;
		ClearSelectedBrush();
		createEnemyModeUI->showLibrary = false;
		createEnemyModeUI->SetShown(true);
		//AddActivePanel(createEnemyModeUI->topbarPanel);
		lastLeftMouseDown = false;//IsMousePressed( Mouse::)
		//grabbedActor = NULL;
		editClock.restart();
		editCurrentTime = 0;
		editAccumulator = TIMESTEP + .1;
		break;
	case CREATE_IMAGES:
		ClearSelectedBrush();
		createDecorModeUI->showLibrary = false;
		createDecorModeUI->SetShown(true);
		grabbedImage = NULL;
		selectedBrush->Clear();
		break;
	case EDIT:
	{
		editModeUI->SetShown(true);
		editModeUI->SetGridSize(graph->GetSpacing());
		editClock.restart();
		editCurrentTime = 0;
		editAccumulator = TIMESTEP + .1;
		justCompletedPolyWithClick = false;
		break;
	}
		
	}
}

void EditSession::UpdateCurrEnemyParamsFromPanel()
{
	assert(selectedBrush->IsSingleActor());
	ActorPtr a = selectedBrush->objects.front()->GetAsActor();
	a->SetParams();
	if (a->myEnemy != NULL)
	{
		a->myEnemy->UpdateParamsSettings();
	}
}

bool EditSession::IsOnlyPlayerSelected()
{
	if (selectedBrush->objects.size() == 1)
	{
		if (playerMarkers[0] == selectedBrush->objects.front())
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

	if (selectedBrush->objects.size() > 0)
	{
		ClearUndoneActions();

		Action *remove = new RemoveBrushAction(selectedBrush, mapStartBrush);

		remove->Perform();
		AddDoneAction(remove);


		ClearSelectedBrush();
	}
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

void EditSession::MoveActors(sf::Vector2i &delta, V2d &grabCenter, Brush *brush )
{
	ActorPtr actor;
	Vector2i extraDelta = Vector2i(0, 0);
	bool unanchored = false;

	PositionInfo pi;

	Vector2i diffPerActor;

	std::vector<PositionInfo> piVec;

	Brush validActorBrush;

	PolyPtr poly;
	for (auto it = brush->objects.begin(); it != brush->objects.end(); ++it)
	{
		actor = (*it)->GetAsActor();
		if (actor == NULL)
			continue;

		poly = actor->posInfo.ground;
		if (poly != NULL && (poly->selected
			|| !selectedPoints.empty()))//selectedPoints.find(poly) != selectedPoints.end()))
			continue;

		validActorBrush.AddObject(actor);
	}

	if (validActorBrush.IsEmpty())
		return;

	//piVec.resize(brush->GetNumActors());//objects.size());
	piVec.resize(validActorBrush.GetNumActors());//objects.size());

	int piIndex = 0;
	
	int numCanBeAnchored = 0;
	int numWillBeAnchored = 0;

	//PositionInfo grabbedActorInfo;

	//V2d grabCenter = V2d(grabbedActor->GetGrabAABBCenter());
	extraDelta = Vector2i(worldPos) - Vector2i(grabCenter);

	V2d actorRealignDiff;
	for (auto it = validActorBrush.objects.begin(); it != validActorBrush.objects.end(); ++it )
	{
		actor = (*it)->GetAsActor();

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

		++piIndex;
	}
	
	piIndex = 0;
	for (auto it = validActorBrush.objects.begin(); it != validActorBrush.objects.end(); ++it)
	{
		actor = (*it)->GetAsActor();

		//needed to comment the typecanbeaerial out for the purpose of POI and other versatile actors
		if (actor->posInfo.ground == NULL && actor->type->CanBeGrounded() )//&& !actor->type->CanBeAerial())
		{
			numCanBeAnchored++;
			piVec[piIndex] = ConvertPointToGround(Vector2i(worldPos + actor->diffFromGrabbed), actor, brush);

			if (piVec[piIndex].ground == NULL)
				break;

			/*if (actor == grabbedActor)
			{
				grabbedActorInfo = piVec[piIndex];
			}*/
			numWillBeAnchored++;

		}
		++piIndex;
	}

	if ( numCanBeAnchored == 0 || numCanBeAnchored != numWillBeAnchored)
	{
		for (auto it = validActorBrush.objects.begin(); it != validActorBrush.objects.end(); ++it)
		{
			actor = (*it)->GetAsActor();

			actor->Move(delta + extraDelta);
			//brush->Move(delta + extraDelta);
		}
		
	}
	else
	{
		bool alignmentMaintained = true;
		/*V2d currDiff;
		double currDiffAngle;
		piIndex = 0;
		for (auto it = brush->objects.begin(); it != brush->objects.end(); ++it, ++piIndex)
		{
			if (piVec[piIndex].ground == NULL)
			{
				continue;
			}

			actor = (*it)->GetAsActor();
			currDiff = piVec[piIndex].GetPosition() - grabbedActorInfo.GetPosition();
			currDiffAngle = piVec[piIndex].GetEdge()->GetNormalAngleRadians() - grabbedActorInfo.GetEdge()->GetNormalAngleRadians();
		}*/

		if (alignmentMaintained)
		{
			piIndex = 0;
			for (auto it = validActorBrush.objects.begin(); it != validActorBrush.objects.end(); ++it, ++piIndex)
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
			for (auto it = validActorBrush.objects.begin(); it != validActorBrush.objects.end(); ++it)
			{
				actor = (*it)->GetAsActor();

				actor->Move(delta + extraDelta);
				//brush->Move(delta + extraDelta);
			}
			//brush->Move(delta + extraDelta);
		}
	}
	
	//doesnt SEEM like this is needed but im not convinced yet
	for (auto it = validActorBrush.objects.begin(); it != validActorBrush.objects.end(); ++it, ++piIndex)
	{
		actor = (*it)->GetAsActor();
		if (actor->myEnemy != NULL)
			actor->myEnemy->UpdateFromEditParams(spriteUpdateFrames);
	}

	bool canApply = brush->CanApply();
	for (auto it = validActorBrush.objects.begin(); it != validActorBrush.objects.end(); ++it, ++piIndex)
	{
		actor = (*it)->GetAsActor();
		
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
	if (mode == EDIT && !editModeUI->IsMoveOn())
		return;

	editStartMove = true;
	Vector2i pos(worldPos.x, worldPos.y);
	Vector2i delta = pos - editMouseGrabPos;

	//assumption that all are grounded atm
	if( selectedPoints.empty() )
	//if (grabbedActor != NULL && selectedBrush->GetNumTerrain() == 0 ) //need to figure out how to separate terrain selection from enemies
	{
		moveAction = selectedBrush->UnAnchor();
		if (moveAction != NULL)
		{
			ActorPtr actor;
			V2d grabbedPos;

			if (grabbedActor != NULL)
			{
				grabbedActor->diffFromGrabbed = V2d(0, 0);

				grabbedPos = grabbedActor->posInfo.GetPosition();
			}
			else
			{
				grabbedPos = worldPos;
			}
			

			for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
			{
				actor = (*it)->GetAsActor();
				if (actor == NULL || actor == grabbedActor)
					continue;

				if (actor->posInfo.ground != NULL)
				{
					actor->diffFromGrabbed = actor->posInfo.GetPosition() - grabbedPos;
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
	//if (HoldingShift())
	//{
	//	worldPos.x = editMouseGrabPos.x;
	//}
	
	if (HoldingShift())
	{
		if (pasteAxis < 0)
		{
			editMouseOrigPos = Vector2i(worldPos);
			pasteAxis = 0;
		}
		else// if (pasteAxis == 0)
		{
			Vector2i test = Vector2i(worldPos) - editMouseOrigPos;
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

	if (pasteAxis == 1)
	{
		worldPos.y = editMouseOrigPos.y;
	}
	else if (pasteAxis == 2)
	{
		worldPos.x = editMouseOrigPos.x;
	}


	Vector2i pos(worldPos.x, worldPos.y);
	Vector2i delta = pos - editMouseGrabPos;
	
	
	//if (IsSingleActorSelected() && selectedPoints.empty())
	//if( selectedBrush->num)
	if (/*selectedPoints.empty() && */grabbedActor != NULL && selectedBrush->GetNumTerrain() == 0 )
	{
		MoveActors(delta, V2d(grabbedActor->GetGrabAABBCenter()), selectedBrush );
	}
	else
	{
		MoveActors(delta, worldPos, selectedBrush);
	}
	/*else if(selectedPoints.empty())
	{
		
	}*/
	//else
	{
		for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
		{
			if ((*it)->GetAsActor() == NULL)
			{
				(*it)->Move(delta);
			}
		}
		//selectedBrush->Move(delta);

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
	bool mouseDown = IsMousePressed(Mouse::Left);
	
	bool justClicked = mouseDown && !lastLeftMouseDown;
	if (!panning && mouseDown)
	{
		Vector2i worldi(round(testPoint.x), round(testPoint.y));

		if (justClicked)
		{
			ClearMostRecentError();
		}
		
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
		else
		{
			if (justClicked)
			{
				ShowMostRecentError();
			}
		}
	}

	lastLeftMouseDown = mouseDown;
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
			
			if (numP == 0)
			{
				railInProgress->AddPoint(worldi, false);

				if (potentialRailAttachPoint != NULL)
				{
					railAttachStartPoint = potentialRailAttachPoint;
					railAttachStart = potentialRailAttach;
				}
			}
			else
			{
				TerrainPoint *end = railInProgress->GetEndPoint();
				double distFromLastPoint = length(V2d(testPoint.x, testPoint.y)
					- V2d(end->pos.x, end->pos.y));
				bool beyondMinLength = distFromLastPoint >= minimumEdgeLength * std::max(zoomMultiple, 1.0);
				if (beyondMinLength)
				{
					railInProgress->AddPoint(worldi, false);
				}

				if (potentialRailAttach != NULL)
				{
					railAttachEnd = potentialRailAttach;
					railAttachEndPoint = potentialRailAttachPoint;

					ExecuteRailCompletion();
					justCompletedRailWithClick = true;
				}
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
	mapHeader->topBounds += amount;
	mapHeader->boundsHeight -= amount;
	UpdateFullBounds();
}

void EditSession::MoveLeftBorder(int amount)
{
	mapHeader->leftBounds += amount;
	mapHeader->boundsWidth -= amount;
	UpdateFullBounds();
}

void EditSession::MoveRightBorder(int amount)
{
	mapHeader->boundsWidth += amount;
	UpdateFullBounds();
}

void EditSession::BackupGrass()
{
	for (auto it = polygons.begin(); it != polygons.end(); ++it)
	{
		(*it)->BackupGrass();
	}
}

void EditSession::ChangeGrassAction()
{
	int changedCounter = 0;
	for (auto it = polygons.begin(); it != polygons.end(); ++it)
	{
		changedCounter += (*it)->GetNumGrassChanges();
	}
	assert(grassChanges == NULL);

	if (changedCounter > 0)
	{
		grassChanges = new GrassDiff[changedCounter];

		int startIndex = 0;
		for (auto it = polygons.begin(); it != polygons.end(); ++it)
		{
			(*it)->isGrassBackedUp = false;
			startIndex += (*it)->AddGrassChanges(grassChanges + startIndex);
			(*it)->grassChanged = false;
		}

		GrassAction *gAction = new GrassAction(grassChanges, changedCounter);
		gAction->performed = true;

		AddDoneAction(gAction);

		grassChanges = NULL;

	}
}

void EditSession::ShowGrass(bool s)
{
	if (s)
	{
		for (auto it = polygons.begin(); it != polygons.end(); ++it)
		{
			(*it)->ShowGrass(true);
		}
	}
	else
	{
		for (auto it = polygons.begin(); it != polygons.end(); ++it)
		{
			(*it)->ShowGrass(false);
		}
		
	}
}

void EditSession::ModifyZoom(double factor)
{
	SetZoom(zoomMultiple * factor);
}

void EditSession::SetZoom(double z)
{
	double old = zoomMultiple;

	zoomMultiple = z;

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
	int ind = currTerrainWorld[TERRAINLAYER_NORMAL] 
		* MAX_TERRAIN_VARIATION_PER_WORLD + currTerrainVar[TERRAINLAYER_NORMAL];
	currTerrainTypeSpr.setTexture(*ts_terrain->texture);//*ts_polyShaders[ind]->texture);
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

		terrainShader.setUniform("zoom", zoom);

		for (int i = 0; i < TerrainPolygon::WATER_Count; ++i)
		{
			waterShaders[i].setUniform("zoom", zoom);
		}
	}

	if (first || oldShaderBotLeft != botLeft)
	{
		oldShaderBotLeft = botLeft;

		terrainShader.setUniform("topLeft", botLeft);


		for (int i = 0; i < TerrainPolygon::WATER_Count; ++i)
		{
			waterShaders[i].setUniform("topLeft", botLeft);
		}
	}

	for (int i = 0; i < TerrainPolygon::WATER_Count; ++i)
	{
		waterShaders[i].setUniform("u_slide", waterShaderCounter);
	}
	waterShaderCounter += .01f;
	
	
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

bool EditSession::IsGridOn()
{
	switch (mode)
	{
	case CREATE_TERRAIN:
		return createTerrainModeUI->IsGridOn();
	case EDIT:
		return editModeUI->IsGridOn();
	case CREATE_RAILS:
		return createRailModeUI->IsGridOn();
	default:
		return false;
	}
}

bool EditSession::IsSnapPointsOn()
{
	switch (mode)
	{
	case CREATE_TERRAIN:
		return createTerrainModeUI->IsSnapPointsOn();
	case CREATE_RAILS:
		return createRailModeUI->IsSnapPointsOn();
		break;
	}
}

void EditSession::DrawGraph()
{
	if (IsGridOn())
	{
		graph->SetCenterAbsolute(Vector2f(worldPos), zoomMultiple);//view.getCenter());
		graph->Draw(preScreenTex);
	}
}

bool EditSession::IsShowingPoints()
{
	bool showPoints = false;
	switch (mode)
	{
	case CREATE_TERRAIN:
		showPoints = createTerrainModeUI->IsSnapPointsOn() && !createTerrainModeUI->IsGridOn();
		break;
	case CREATE_GATES:
		showPoints = true;
		break;
	case EDIT:
		showPoints = editModeUI->IsEditPointsOn();
		break;
	case CREATE_RAILS:
		showPoints = createRailModeUI->IsSnapPointsOn();
		break;
	}
	return showPoints;
}

void EditSession::DrawSpecialTerrain(sf::RenderTarget *target)
{
	if (!editModeUI->IsLayerShowing(LAYER_WATER))
	{
		return;
	}

	bool showPoints = IsShowingPoints();

	auto & currPolyList = GetCorrectPolygonList(1);
	for (auto it = currPolyList.begin(); it != currPolyList.end(); ++it)
	{
		(*it)->Draw(false, zoomMultiple, preScreenTex, showPoints, NULL);
	}
}

void EditSession::DrawTerrain(sf::RenderTarget *target)
{
	if (!editModeUI->IsLayerShowing(LAYER_TERRAIN))
	{
		return;
	}

	bool showPoints = IsShowingPoints();

	if (inversePolygon != NULL)
	{
		inversePolygon->Draw(false, zoomMultiple, preScreenTex, showPoints, NULL);
	}

	auto & currPolyList = GetCorrectPolygonList(0);
	for (auto it = currPolyList.begin(); it != currPolyList.end(); ++it)
	{
		if ((*it)->inverse)
			continue;

		(*it)->Draw(false, zoomMultiple, preScreenTex, showPoints, NULL);
	}
}

void EditSession::DrawFlyTerrain(sf::RenderTarget *target)
{
	bool showPoints = IsShowingPoints();

	auto & currPolyList = GetCorrectPolygonList(2);
	for (auto it = currPolyList.begin(); it != currPolyList.end(); ++it)
	{
		(*it)->Draw(false, zoomMultiple, preScreenTex, showPoints, NULL);
	}
}

void EditSession::DrawRails( sf::RenderTarget *target )
{
	bool showPoints = IsShowingPoints();

	for (list<RailPtr>::iterator it = rails.begin(); it != rails.end(); ++it)
	{
		if (mode != TEST_PLAYER || (*it)->enemyChain == NULL)
		{
			(*it)->Draw(zoomMultiple, showPoints, target);
		}
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
	if (editModeUI->IsLayerShowing(LAYER_ACTOR))
	{
		for (map<string, ActorGroup*>::iterator it = groups.begin(); it != groups.end(); ++it)
		{
			(*it).second->Draw(preScreenTex);
		}
	}
}

void EditSession::DrawGateInfos()
{
	for (list<GateInfoPtr>::iterator it = gateInfoList.begin(); it != gateInfoList.end(); ++it)
	{
		(*it)->Draw(preScreenTex);
	}
}

void EditSession::DrawDecor(EffectLayer ef, sf::RenderTarget *target)
{
	if (mode != TEST_PLAYER && !editModeUI->IsLayerShowing(LAYER_IMAGE))
	{
		return;
	}

	auto &dList = decorImages[ef];

	for (auto it = dList.begin(); it != dList.end(); ++it)
	{
		(*it)->Draw(target);
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
				c = Color::Green;
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

void EditSession::DrawMode()
{
	Emode dMode = mode;
	if (mode == SELECT_MODE)
	{
		dMode = menuDownStored;
	}
	else if (mode == PAUSED)
	{
		dMode = stored;
	}
	/*return ((mode == em) || (menuDownStored == em && mode == SELECT_MODE)
		|| (stored == em && mode == PAUSED));*/

	switch (dMode)
	{
	case TRANSFORM:
	{
		transformTools->Draw(preScreenTex);

		if (grabbedImage != NULL)
			grabbedImage->Draw(preScreenTex);

		DrawPlayerTracker(preScreenTex);
		break;
	}
	case PASTE:
	{
		if (copiedBrush != NULL)
			copiedBrush->Draw(preScreenTex);
		if (freeActorCopiedBrush != NULL)
			freeActorCopiedBrush->Draw(preScreenTex);


		DrawPlayerTracker(preScreenTex);
		break;
	}
	case TEST_PLAYER:
	{
		//DrawGame(preScreenTex);
		break;
	}
	case CREATE_TERRAIN:
	{
		int currTool = createTerrainModeUI->GetCurrDrawTool();
		if (currTool == TOOL_DRAW)
		{
			DrawPolygonInProgress();
		}
		else if( currTool == TOOL_BOX && boxDrawStarted)
		{
			SetRectCenter(boxToolQuad, abs(startBoxPos.x - testPoint.x),
				abs(startBoxPos.y - testPoint.y), (startBoxPos + testPoint) / 2.f);
			preScreenTex->draw(boxToolQuad, 4, sf::Quads);
		}

		DrawPlayerTracker(preScreenTex);
		break;
	}
	case CREATE_RAILS:
	{
		DrawRailInProgress();

		DrawPlayerTracker(preScreenTex);
		break;
	}
	case EDIT:
	{
		DrawBoxSelection();
		DrawPlayerTracker(preScreenTex);
		
		break;
	}
	case CREATE_ENEMY:
	{
		//DrawTrackingEnemy();

		if (grabbedActor != NULL)
			grabbedActor->Draw(preScreenTex);

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
	case CREATE_GATES:
	{
		DrawGateInProgress();
		createGatesModeUI->Draw(preScreenTex);
		break;
	}
	case CREATE_IMAGES:
	{
		//createDecorModeUI->Draw(preScreenTex);

		if (grabbedImage != NULL)
		{
			grabbedImage->Draw(preScreenTex);
		}
		break;
	}
	}
}

void EditSession::DrawPlayerTracker(sf::RenderTarget *target)
{
	if (playerTracker->IsOn())
	{
		playerTracker->Draw(preScreenTex);
		GetPlayer(0)->Draw(preScreenTex);
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


		//needs a cleanup later
		Color c;

		int outlineThickness = 3;
		int fontSize = 18;

		sf::Text textblue;
		textblue.setCharacterSize(fontSize);
		textblue.setFont(arial);
		textblue.setString("CREATE\nTERRAIN");
		textblue.setFillColor(sf::Color::White);
		textblue.setOutlineColor(sf::Color::Black);
		textblue.setOutlineThickness(outlineThickness);
		textblue.setOrigin(textblue.getLocalBounds().width / 2, textblue.getLocalBounds().height / 2);
		textblue.setPosition((menuDownPos + circleUpperRightPos).x, (menuDownPos + circleUpperRightPos).y);
		preScreenTex->draw(textblue);

		sf::Text textgreen;
		textgreen.setCharacterSize(fontSize);
		textgreen.setFont(arial);
		textgreen.setString("CREATE\nRAILS");
		textgreen.setFillColor(sf::Color::White);
		textgreen.setOutlineColor(sf::Color::Black);
		textgreen.setOutlineThickness(outlineThickness);
		textgreen.setOrigin(textgreen.getLocalBounds().width / 2, textgreen.getLocalBounds().height / 2);
		textgreen.setPosition((menuDownPos + circleLowerRightPos).x, (menuDownPos + circleLowerRightPos).y);
		preScreenTex->draw(textgreen);

		textgreen.setString("CREATE\nDECOR");
		textgreen.setPosition(Vector2f(menuDownPos + circleBottomPos));
		preScreenTex->draw(textgreen);

		sf::Text textorange;
		textorange.setString("GATES");
		textorange.setFont(arial);
		textorange.setCharacterSize(fontSize);
		textorange.setFillColor(sf::Color::White);
		textorange.setOutlineColor(sf::Color::Black);
		textorange.setOutlineThickness(outlineThickness);
		textorange.setOrigin(textorange.getLocalBounds().width / 2, textorange.getLocalBounds().height / 2);
		textorange.setPosition((menuDownPos + circleLowerLeftPos).x, (menuDownPos + circleLowerLeftPos).y);
		preScreenTex->draw(textorange);

		sf::Text textred;
		textred.setString("CREATE\nENEMIES");
		textred.setFont(arial);
		textred.setCharacterSize(fontSize);
		textred.setFillColor(sf::Color::White);
		textred.setOutlineColor(sf::Color::Black);
		textred.setOutlineThickness(outlineThickness);
		textred.setOrigin(textred.getLocalBounds().width / 2, textred.getLocalBounds().height / 2);
		textred.setPosition((menuDownPos + circleUpperLeftPos).x, (menuDownPos + circleUpperLeftPos).y);
		preScreenTex->draw(textred);

		sf::Text textmag;

		bool singleObj, singleActor, singleImage, singleRail, onlyPoly; 
		
		singleObj = selectedBrush->objects.size() == 1 && selectedPoints.size() == 0;

		singleActor = singleObj && selectedBrush->objects.front()->selectableType == ISelectable::ACTOR;
		singleImage = singleObj && selectedBrush->objects.front()->selectableType == ISelectable::IMAGE;
		singleRail = singleObj && selectedBrush->objects.front()->selectableType == ISelectable::RAIL;
		onlyPoly = selectedBrush != NULL && !selectedBrush->objects.empty() && selectedBrush->terrainOnly;

		textmag.setString("EDIT");

		textmag.setFont(arial);
		textmag.setCharacterSize(fontSize);
		textmag.setFillColor(sf::Color::White);
		textmag.setOutlineColor(sf::Color::Black);
		textmag.setOutlineThickness(outlineThickness);
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

	errorBar.Draw(preScreenTex);

	for (auto it = activePanels.begin(); it != activePanels.end(); ++it)
	{
		(*it)->Draw(preScreenTex);
	}

	if( mode != TEST_PLAYER )
		generalUI->Draw(preScreenTex);
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

void EditSession::GeneralMouseUpdate()
{
	if (mode != PAUSED && mode != SELECT_MODE)
	{
		if (MOUSE.IsMouseRightClicked())
		{
			if (mode != PASTE && focusedPanel == NULL)
			{ 
				menuDownStored = mode;
				mode = SELECT_MODE;
				menuDownPos = V2d(uiMousePos.x, uiMousePos.y);
				guiMenuSprite.setPosition(uiMousePos.x, uiMousePos.y);
			}
		}
	}
}

void EditSession::SaveMapDialog()
{
	fileChooser->chooser->StartRelative(".brknk", FileChooser::SAVE, "Resources\\Maps\\CustomMaps");
}

void EditSession::OpenMapDialog()
{
	fileChooser->chooser->StartRelative(".brknk", FileChooser::OPEN, "Resources\\Maps\\CustomMaps");
}

void EditSession::TryReloadNew()
{
	if (saveUpdated)
	{
		ReloadNew();
	}
	else
	{
		confirmPopup->Pop(ConfirmPopup::ConfirmType::SAVE_CURRENT);
	}
}

void EditSession::TrySaveMap()
{
	if (currentFile == "")
	{
		SaveMapDialog();
	}
	else
	{
		cout << "writing to file: " << currentFile << endl;
		WriteFile(currentFile);
	}
}

void EditSession::TryExitEditor()
{
	if (!saveUpdated)
		confirmPopup->Pop(ConfirmPopup::ConfirmType::SAVE_CURRENT_EXIT);
	else
	{
		quit = true;
		returnVal = 1;
	}
}

void EditSession::GeneralEventHandler()
{
	if (mode != PAUSED && mode != SELECT_MODE)
	{
		switch (ev.type)
		{
		case Event::MouseButtonPressed:
		{
			if (ev.mouseButton.button == Mouse::Button::Middle)
			{
				gameCam = false;
				ClearMostRecentError();
				panning = true;
				panAnchor = worldPos;
			}
			else if (ev.mouseButton.button == Mouse::Button::Right)
			{
				
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
			if (IsGridOn() && HoldingControl())
			{
				if (ev.mouseWheel.delta > 0)
				{
					graph->ModifyGraphSpacing(.5);
					createTerrainModeUI->SetGridSize(graph->GetSpacing());
				}
				else if (ev.mouseWheel.delta < 0)
				{
					graph->ModifyGraphSpacing(2.0);
					createTerrainModeUI->SetGridSize(graph->GetSpacing());
				}
			}
			else
			{
				gameCam = false;
				if (ev.mouseWheel.delta > 0)
				{
					if (zoomMultiple > 32)
					{
						ModifyZoom(.5);
					}
					else if (zoomMultiple > 8)
					{
						SetZoom(zoomMultiple - 8);
						//zoomMultiple += 10;
					}
					else if (zoomMultiple > 1)
					{
						SetZoom(zoomMultiple - 1);
					}
					else
					{
						ModifyZoom(.5);
					}

					//ModifyZoom(.5);
					//ModifyZoom(.5);
				}
				else if (ev.mouseWheel.delta < 0)
				{
					if (zoomMultiple >= 32)
					{
						ModifyZoom(2);
					}
					else if (zoomMultiple >= 8)
					{
						SetZoom(zoomMultiple + 8);
						//ModifyZoom(2.0);
						//zoomMultiple += 10;
					}
					else if (zoomMultiple >= 1)
					{
						SetZoom(zoomMultiple + 1);
					}
					else
					{
						ModifyZoom(2.0);
					}
					//ModifyZoom(2);
					//ModifyZoom(2.0);
				}
			}

			break;
		}
		case Event::KeyPressed:
		{
			if (focusedPanel == NULL)
			{
				if (ev.key.code == Keyboard::S && ev.key.control)
				{
					if (ev.key.shift)
					{
						SaveMapDialog();
					}
					else
					{
						//polygonInProgress->ClearPoints();
						TrySaveMap();
					}
					
				}
				else if (ev.key.code == Keyboard::O && ev.key.control)
				{
					OpenMapDialog();
				}
				else if (ev.key.code == Keyboard::N && ev.key.control)
				{
					TryReloadNew();
				}
				else if (ev.key.code == Keyboard::T )
				{
					//make this only to some modes later
					TestPlayerMode();
					//quit = true;
				}
				else if (ev.key.code == Keyboard::Escape)
				{
					TryExitEditor();
				}
				else if (ev.key.code == sf::Keyboard::Equal || ev.key.code == sf::Keyboard::Dash)
				{
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
			double newTime = editClock.getElapsedTime().asSeconds();
			editCurrentTime = newTime;
			editAccumulator = 0;
			break;
		}
		}
	}
	/*else if (mode == PAUSED)
	{
		PausedModeHandleEvent();
	}*/
}

void EditSession::HandleEvents()
{
	mainWindowLostFocus = false;
	//if (MOUSE.IsMouseLeftClicked() || MOUSE.IsMouseRightClicked())
	{
		Vector2i mousePos = MOUSE.GetPos();
		bool found = false;

		bool noFocusedPanel = (focusedPanel == NULL);

		if ( mode != TEST_PLAYER && generalUI->mainPanel->ContainsPoint(mousePos))
		{
			if (focusedPanel != generalUI->mainPanel)
			{
				if( focusedPanel != NULL )
					focusedPanel->Deactivate();
				focusedPanel = generalUI->mainPanel;
			}
			found = true;
		}

		if (!found)
		{
			for (auto it = activePanels.rbegin(); it != activePanels.rend(); ++it)
			{
				if ((*it)->IsPopup() || (*it)->ContainsPoint(mousePos))
				{
					if (focusedPanel != (*it))
					{
						if (focusedPanel != NULL)
							focusedPanel->Deactivate();
						focusedPanel = (*it);
					}
					found = true;
					break;
				}
			}
		}

		if (!found)
		{
			if (focusedPanel != NULL)
			{
				focusedPanel->Deactivate();
			}
			focusedPanel = NULL;
		}

		if (noFocusedPanel && focusedPanel != NULL )
		{
			mainWindowLostFocus = true;
			//went from main window to focused panel
		}
	}

	

	while (window->pollEvent(ev))
	{
		if (ev.type == Event::KeyPressed)
		{
			if (ev.key.code == Keyboard::F5)
			{
				if( background != NULL )
					background->FlipShown();
				continue;
			}
		}

		bool focuseHandled = false;
		if (focusedPanel != NULL)
		{
			focuseHandled = focusedPanel->HandleEvent(ev);
		}

		if( !focuseHandled )
		{
			HandleEventFunc(mode);

			GeneralEventHandler();
		}

		
		//GeneralEventHandler();		
	}
}

void EditSession::SetBoxTool()
{
	polygonInProgress->ClearPoints();
	boxDrawStarted = false;
}

void EditSession::CreateTerrainModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::KeyPressed:
	{
		if (ev.key.code == Keyboard::Space)
		{
			ExecuteTerrainCompletion();
		}
		else if (ev.key.code == sf::Keyboard::X || ev.key.code == sf::Keyboard::Delete)
		{
			//if( createTerrainModeUI->)
			if (createTerrainModeUI->GetCurrDrawTool() == TOOL_DRAW)
			{
				removeProgressPointWaiter->Reset();
				RemovePointFromPolygonInProgress();
			}
			else
			{
				boxDrawStarted = false;
			}
			
		}
		//else if (ev.key.code == sf::Keyboard::E)
		//{
		//	AddActivePanel(terrainSelectorPopup);
		//	tempGridResult = "not set";
		//	//GridSelectPop("terraintypeselect");
		//}
		/*else if (ev.key.code == sf::Keyboard::R)
		{
			SetMode(CREATE_RAILS);
			railInProgress->CopyPointsFromPoly(polygonInProgress);
			polygonInProgress->ClearPoints();
		}*/
		else if (ev.key.code == sf::Keyboard::Z && ev.key.control)
		{
			UndoMostRecentAction();
		}
		else if (ev.key.code == sf::Keyboard::Y && ev.key.control)
		{
			RedoMostRecentUndoneAction();
		}
		else if (ev.key.code == sf::Keyboard::B)
		{
			createTerrainModeUI->SetDrawTool(TOOL_BOX);
		}
		else if (ev.key.code == sf::Keyboard::D)
		{
			createTerrainModeUI->SetDrawTool(TOOL_DRAW);
		}
		else if (ev.key.code == sf::Keyboard::R)
		{
			createTerrainModeUI->SetDrawTool(TOOL_BRUSH);
		}
		else if (ev.key.code == Keyboard::T )
		{
			//eventually something telling the create mode that you can here from create terrain
			TestPlayerMode();
			
		}
		else if (ev.key.code == Keyboard::G)
		{
			createTerrainModeUI->FlipGrid();
		}
		else if (ev.key.code == Keyboard::F)
		{
			createTerrainModeUI->FlipSnapPoints();
		}
		else if (ev.key.code == Keyboard::A)
		{
			createTerrainModeUI->SetTerrainTool(TERRAINTOOL_ADD);
		}
		else if (ev.key.code == Keyboard::S && !ev.key.control)
		{
			createTerrainModeUI->SetTerrainTool(TERRAINTOOL_SUBTRACT);
		}
		else if (ev.key.code == Keyboard::I)
		{
			createTerrainModeUI->SetTerrainTool(TERRAINTOOL_SETINVERSE);
		}
		else if (ev.key.code == Keyboard::E)
		{
			createTerrainModeUI->ExpandTerrainLibrary();
		}
		else if (ev.key.code == Keyboard::H)
		{
			playerTracker->SwitchOnOff();
		}
		else if( ev.key.code == Keyboard::W )
		{
			createTerrainModeUI->SetLayerWater();
		}
		else if (ev.key.code == Keyboard::Q)
		{
			createTerrainModeUI->SetLayerTerrain();
		}
		break;
	}
	case Event::KeyReleased:
	{
		break;
	}
	}
}

void EditSession::CreateRailsModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::KeyPressed:
	{
		if (ev.key.code == Keyboard::Space)
		{
			ExecuteRailCompletion();
		}
		else if (ev.key.code == sf::Keyboard::X || ev.key.code == sf::Keyboard::Delete)
		{
			removeProgressPointWaiter->Reset();
			RemovePointFromRailInProgress();
		}
		else if (ev.key.code == sf::Keyboard::G)
		{
			createRailModeUI->FlipGrid();
		}
		else if (ev.key.code == Keyboard::F)
		{
			createRailModeUI->FlipSnapPoints();
		}
		else if (ev.key.code == Keyboard::E)
		{
			createRailModeUI->ExpandLibrary();
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

void EditSession::EditModeDelete()
{
	if (!editStartMove)
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
	else
	{
		ClearMostRecentError();
		CreateError(ERR_CANT_DELETE_WHILE_MOVING);
		ShowMostRecentError();
	}
}

void EditSession::EditModeTransform()
{
	if (selectedBrush->IsEmpty())
		return;

	SetMode(TRANSFORM);

	if (selectedBrush->IsSingleDecor())
	{
		//transformTools->Reset(selectedBrush->GetCenterF(),
		//	selectedBrush->GetTerrainSize());
		DecorPtr sDec = selectedBrush->objects.front()->GetAsDecor();
		//FloatRect localBounds = sDec->spr.getLocalBounds();
		//Vector2f size(localBounds.width * sDec->spr.getScale().x,
		//	localBounds.height * sDec->spr.getScale().y);
		transformTools->Reset(sDec->center, Vector2f(sDec->tileSize.x * sDec->scale.x,
			sDec->tileSize.y * sDec->scale.y), sDec->rotation);
	}
	//else if (selectedBrush->IsSingleFlyPoly())
	//{

	//	
	//}
	else
	{
		transformTools->Reset(selectedBrush->GetCenterF(),
			selectedBrush->GetTerrainSize());
	}

	//selectedBrush->Scale(1.05f);
	PolyPtr p;
	DecorPtr dec;
	RailPtr r;
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

		r = (*it)->GetAsRail();
		if (r != NULL)
		{
			r->SetRenderMode(TerrainRail::RENDERMODE_TRANSFORM);
		}


		dec = (*it)->GetAsDecor();
		if (dec != NULL)
		{
			dec->transformOffset = dec->center - transformTools->GetCenter();
			dec->StartTransformation();

		}

	}
}

void EditSession::DestroyCopiedBrushes()
{
	if (copiedBrush != NULL)
	{
		copiedBrush->Destroy();
		delete copiedBrush;
		copiedBrush = NULL;
	}
	if (freeActorCopiedBrush != NULL)
	{
		freeActorCopiedBrush->Destroy();
		delete freeActorCopiedBrush;
		freeActorCopiedBrush = NULL;
	}
}

void EditSession::EditModeCopy()
{
	//copiedBrush = selectedBrush->Copy();
	DestroyCopiedBrushes();

	copiedBrush = selectedBrush->CopyTerrainAndAttachedActors();
	freeActorCopiedBrush = selectedBrush->CopyFreeActors();
}

void EditSession::EditModePaste()
{
	if (copiedBrush != NULL || freeActorCopiedBrush != NULL)
	{
		Vector2i pos = Vector2i(worldPos.x, worldPos.y);

		if (copiedBrush != NULL)
		{
			//copiedBrush->CenterOnPoint(pos);
		}

		if (freeActorCopiedBrush != NULL)
		{
			ActorPtr actor;
			for (auto it = freeActorCopiedBrush->objects.begin(); it != freeActorCopiedBrush->objects.end(); ++it)
			{
				actor = (*it)->GetAsActor();
				if (actor == NULL)
					continue;

				actor->diffFromGrabbed = actor->posInfo.GetPosition() - V2d(freeActorCopiedBrush->GetCenter());//worldPos;
			}
		}

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

void EditSession::EditModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::KeyPressed:
	{
		if (ev.key.code == Keyboard::C && ev.key.control)
		{
			EditModeCopy();
		}
		else if (ev.key.code == Keyboard::V && ev.key.control)
		{
			EditModePaste();
		}
		else if (ev.key.code == sf::Keyboard::Z && ev.key.control)
		{
			if (!MOUSE.IsMouseDownLeft() && !MOUSE.IsMouseDownRight())
				UndoMostRecentAction();
		}
		else if (ev.key.code == sf::Keyboard::Y && ev.key.control)
		{
			if( !MOUSE.IsMouseDownLeft() && !MOUSE.IsMouseDownRight())
				RedoMostRecentUndoneAction();
		}
		
		else if (ev.key.code == Keyboard::X || ev.key.code == Keyboard::Delete)
		{
			EditModeDelete();
		}
		else if (ev.key.code == Keyboard::N)
		{
			EditModeTransform();
		}
		else if (ev.key.code == Keyboard::R)
		{
			if (!MOUSE.IsMouseDownLeft() && !MOUSE.IsMouseDownRight())
			{
				editModeUI->FlipShowGrass();
				ShowGrass(editModeUI->IsShowGrassOn());
				if (!editModeUI->IsShowGrassOn())
				{
					ChangeGrassAction();
				}
			}
		}
		else if (ev.key.code == Keyboard::Q)
		{
			editModeUI->FlipMove();
		}
		else if (ev.key.code == Keyboard::G)
		{
			editModeUI->FlipGrid();
		}
		else if (ev.key.code == Keyboard::B)
		{
			editModeUI->FlipEditPoints();
			if (!editModeUI->IsEditPointsOn())
			{
				ClearSelectedPoints();
			}
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
			if (editModeUI->IsShowGrassOn())
			{
				editModeUI->ExpandGrassLibrary();
			}
			else if (selectedBrush->GetNumTerrain() > 0)
			{
				int layer = selectedBrush->GetTerrainLayer();

				ClearMostRecentError();
				if (layer < 0 )
				{
					CreateError(ERR_SELECTED_TERRAIN_MULTIPLE_LAYERS);
					ShowMostRecentError();
				}
				else
				{
					editModeUI->ExpandTerrainLibrary(layer);
				}
			}
			else if( selectedBrush->IsSingleActor() )
			{
				ActorPtr a = selectedBrush->GetFirst()->GetAsActor();
				if (a->type->info.name == "player")
				{
					editModeUI->ToggleKinOptionsPanel();
				}
				else if (a->type->info.name == "shard")
				{
					editModeUI->ExpandShardLibrary();
				}
			}
			
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
		else if (ev.key.code == Keyboard::Num1)
		{
			gameCam = !gameCam;
			if (!gameCam)
			{
				//why is this here?
				panning = false;
			}
		}
		else if (ev.key.code == Keyboard::H)
		{
			playerTracker->SwitchOnOff();
		}
		else if (ev.key.code == Keyboard::Num9)
		{
			fileChooser->chooser->StartRelative(".bnbrush",
				FileChooser::OPEN, "Resources/Brushes");
		}
		else if (ev.key.code == sf::Keyboard::Num8)
		{
			fileChooser->chooser->StartRelative(".bnbrush",
				FileChooser::SAVE, "Resources/Brushes");
			/*brushManager->SaveBrush(selectedBrush, "Resources/Brushes/", 
				"testbrush");*/
		}
		break;
	}
	case Event::KeyReleased:
	{
		/*if (ev.key.code == Keyboard::R)
		{
			ShowGrass(false);
		}*/
		break;
	}
	}
}

void EditSession::PasteModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::KeyPressed:
	{
		if (ev.key.code == Keyboard::X)
		{
			if (freeActorCopiedBrush != NULL)
			{
				ActorPtr a;
				for (auto it = freeActorCopiedBrush->objects.begin();
					it != freeActorCopiedBrush->objects.end(); ++it)
				{
					a = (*it)->GetAsActor();
					if (a->posInfo.ground != NULL )
					{
						a->UnAnchor();
					}
				}
			}

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
		/*else if (ev.key.code == sf::Keyboard::B)
		{
			SaveBrush(copiedBrush);
		}*/
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
		break;
	}
	case Event::KeyPressed:
	{
		if (ev.key.code == Keyboard::X || ev.key.code == Keyboard::Delete)
		{
			if (trackingEnemyParams != NULL)
			{
				ClearSelectedBrush();
				delete trackingEnemyParams;
				trackingEnemyParams = NULL;
				grabbedActor = NULL;
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
	case Event::GainedFocus:
	{
		mode = stored;
		//SetMode(stored);
		break;
	}
	}
}

void EditSession::SelectModeHandleEvent()
{
	switch (ev.type)
	{
	}
}

void EditSession::CreatePatrolPathModeHandleEvent()
{
	minimumPathEdgeLength = 16;
	switch (ev.type)
	{
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
				editModeUI->SetEnemyPanel(actor);
				actor->SetPath(patrolPath);
				SetMode(EDIT);
			}
			else
			{
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
	}
}

void EditSession::CreateRectModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::KeyPressed:
	{
		if (ev.key.code == Keyboard::Space)
		{
			if (selectedBrush->objects.size() == 1) //EDIT
			{
				SelectPtr select = selectedBrush->objects.front();
				AirTriggerParams *actor = (AirTriggerParams*)select;
				AddActivePanel(actor->type->panel);
				SetMode(EDIT);
			}
			else
			{
				SetMode(CREATE_ENEMY);
			}
		}
		break;
	}
	}
}

void EditSession::SetCamZoomModeHandleEvent()
{
	//minimumPathEdgeLength = 16;

	switch (ev.type)
	{
	
	}

}

void EditSession::SetDirectionModeHandleEvent()
{
}

void EditSession::CreateGatesModeHandleEvent()
{
	switch (ev.type)
	{
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
		else if (ev.key.code == sf::Keyboard::X || ev.key.code == sf::Keyboard::Delete)
		{
			gatePoints = 0;
		}
		else if (ev.key.code == sf::Keyboard::E)
		{
			createGatesModeUI->ExpandLibrary();
		}
		break;
	}
	case Event::KeyReleased:
	{

		break;
	}
	}
}

void EditSession::CreateImagesModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::KeyPressed:
	{
		if (ev.key.code == Keyboard::X || ev.key.code == Keyboard::Delete)
		{
			
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

void EditSession::TransformModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::KeyPressed:
	{
		if (ev.key.code == sf::Keyboard::Space)
		{
			SetMode(EDIT);
			PolyPtr p;
			Brush origBrush;
			Brush resultBrush;
			PolyPtr temp;
			DecorPtr dec;
			DecorPtr tempDec;
			RailPtr r;
			RailPtr tempRail;
			for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
			{
				p = (*it)->GetAsTerrain();
				if (p != NULL)
				{
					temp = p->CompleteTransformation(transformTools);
					if (temp != NULL)
					{
						resultBrush.AddObject(temp);
					}
					origBrush.AddObject((*it));
				}

				r = (*it)->GetAsRail();
				if (r != NULL)
				{
					tempRail = r->CompleteTransformation(transformTools);
					if (tempRail != NULL)
					{
						resultBrush.AddObject(tempRail);
					}
					origBrush.AddObject((*it));
				}

				dec = (*it)->GetAsDecor();
				if (dec != NULL)
				{
					tempDec = dec->CompleteTransformation(transformTools);
					if (tempDec != NULL)
					{
						resultBrush.AddObject(tempDec);
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
			DecorPtr dec;
			RailPtr r;
			for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
			{
				p = (*it)->GetAsTerrain();
				if (p != NULL)
				{
					p->CancelTransformation();
				}

				r = (*it)->GetAsRail();
				if (r != NULL)
				{
					r->CancelTransformation();
				}

				dec = (*it)->GetAsDecor();
				if (dec != NULL)
				{
					dec->CancelTransformation();
				}
			}
		}
		else if (ev.key.code == sf::Keyboard::Num3) //use better buttons for this at some point
		{
			transformTools->FlipX();
		}
		else if (ev.key.code == sf::Keyboard::Num4)
		{
			transformTools->FlipY();
		}
		else if (ev.key.code == sf::Keyboard::Num5)
		{
			transformTools->CWRotate45();
		}
		else if (ev.key.code == sf::Keyboard::Num6)
		{
			transformTools->CCWRotate45();
		}
		break;
	}
	case Event::KeyReleased:
	{
		break;
	}
	}
}

void EditSession::MoveBorderModeHandleEvent()
{

}

void EditSession::MoveBorderModeUpdate()
{

}

void EditSession::TestPlayerModeHandleEvent()
{
	switch (ev.type)
	{
	case Event::KeyPressed:
	{
		if (ev.key.code == sf::Keyboard::Num1)
		{
			gameCam = !gameCam;
			if (!gameCam)
			{
				SetZoom(4);
			}
		}
		else if (ev.key.code == sf::Keyboard::Num2)
		{
			showDebugDraw = !showDebugDraw;
		}
		else if (ev.key.code == sf::Keyboard::Num3)
		{
			showNetStats = !showNetStats;
		}
	}
	}
}

void EditSession::UpdateMode()
{
	bool focusedUpdate = false;

	for (auto it = activePanels.begin(); it != activePanels.end(); ++it)
	{
		(*it)->UpdateSlide(spriteUpdateFrames);
	}


	if (focusedPanel != NULL)
	{
		if (focusedPanel->MouseUpdate())
		{
			focusedUpdate = true;
		}
		if (focusedPanel != NULL)
		{
			focusedPanel->UpdateSprites(spriteUpdateFrames);
		}
	}

	if (!focusedUpdate)
	{
		UpdateModeFunc(mode);

		GeneralMouseUpdate();
	}
	else
	{
		if (mainWindowLostFocus)
		{
			UpdateModeFunc(mode);
		}
	}
}

void EditSession::CreateTerrainModeUpdate()
{
	if (MOUSE.IsMouseLeftClicked())
	{
		justCompletedPolyWithClick = false;
	}
	
	UpdateInputNonGame();

	if (GetCurrInput(0).start && !GetPrevInput(0).start)
	{
		TestPlayerMode();
		return;
	}

	if (IsKeyPressed(sf::Keyboard::X) || IsKeyPressed(sf::Keyboard::Delete))
	{
		if (!focusedPanel)
		{
			if (removeProgressPointWaiter->Hold())
			{
				RemovePointFromPolygonInProgress();
			}
		}
		else
		{
			removeProgressPointWaiter->Reset();
		}
	}
	else
	{
		removeProgressPointWaiter->Reset();
	}

	if (!focusedPanel)
	{
		if (IsGridOn())
		{
			SnapPointToGraph(testPoint, graph->graphSpacing);
		}
		else if (IsSnapPointsOn())
		{
			SelectPtr obj;
			TerrainPoint *pPoint = TrySnapPosToPoint(testPoint, obj, 8 * zoomMultiple);
		}

		/*if (IsKeyPressed(sf::Keyboard::LAlt))
		{
			createTerrainModeUI->SetTempTerrainTool(TERRAINTOOL_SETINVERSE);
		}
		else if (IsKeyPressed(sf::Keyboard::LControl))
		{
			createTerrainModeUI->SetTempTerrainTool(TERRAINTOOL_SUBTRACT);
		}
		else
		{
			createTerrainModeUI->RevertTerrainTool();
		}*/
	}

	int currTool = createTerrainModeUI->GetCurrDrawTool();

	if (currTool == TOOL_BOX)
	{
		if (!boxDrawStarted && MOUSE.IsMouseLeftClicked())
		{
			startBoxPos = testPoint;
			boxDrawStarted = true;
		}
		else if( boxDrawStarted && MOUSE.IsMouseLeftReleased() )
		{
			boxDrawStarted = false;
			if (testPoint.x - startBoxPos.x != 0 && testPoint.y - startBoxPos.y != 0)
			{
				assert(polygonInProgress->GetNumPoints() == 0);

				polygonInProgress->AddPoint(Vector2i(startBoxPos.x, startBoxPos.y), false);
				polygonInProgress->AddPoint(Vector2i(testPoint.x, startBoxPos.y), false);
				polygonInProgress->AddPoint(Vector2i(testPoint.x, testPoint.y), false);
				polygonInProgress->AddPoint(Vector2i(startBoxPos.x, testPoint.y), false);

				ExecuteTerrainCompletion();
			}
		}
	}
	


	PreventNearPrimaryAnglesOnPolygonInProgress();

	if (currTool == TOOL_DRAW)
	{
		if (!justCompletedPolyWithClick)
		{
			TryAddPointToPolygonInProgress();
		}
	}
	else if (currTool == TOOL_BOX)
	{

	}

	
}

void EditSession::CreateRailsModeUpdate()
{
	if (MOUSE.IsMouseLeftClicked())
	{
		justCompletedRailWithClick = false;
	}

	potentialRailAttachPoint = NULL;
	potentialRailAttach = NULL;

	if (IsKeyPressed(sf::Keyboard::X) || IsKeyPressed(sf::Keyboard::Delete))
	{
		if (!focusedPanel)
		{
			if (removeProgressPointWaiter->Hold())
			{
				RemovePointFromRailInProgress();
			}
		}
		else
		{
			removeProgressPointWaiter->Reset();
		}
	}
	else
	{
		removeProgressPointWaiter->Reset();
	}

	if (!focusedPanel)
	{
		if (IsGridOn())
		{
			SnapPointToGraph(testPoint, graph->graphSpacing);
		}
		else if (IsSnapPointsOn())
		{
			SelectPtr obj = NULL;
			TerrainPoint *pPoint = TrySnapPosToPoint(testPoint, obj, 8 * zoomMultiple);

			if (obj != NULL)
			{
				RailPtr r = obj->GetAsRail();
				if (r != NULL)
				{
					if (pPoint->index == 0 || pPoint->index == r->GetNumPoints() - 1)
					{
						if (railAttachStart != r)
						{
							potentialRailAttachPoint = pPoint;
							potentialRailAttach = r;
						}
					}
				}
			}
		}
	}
	else
	{
		//showPoints = false;
	}


	PreventNearPrimaryAnglesOnRailInProgress();

	if (!justCompletedRailWithClick)
	{
		TryAddPointToRailInProgress();
	}
}

void EditSession::UpdateInputNonGame()
{
	//set curr inputs to prev inputs
	/*for (int i = 0; i < 4; ++i)
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
	}*/
	//------------------------

	UpdateControllers();

	//UpdateAllPlayersInput();
	//-------------------------
}

void EditSession::SelectModeUpdate()
{
	if (MOUSE.IsMouseRightReleased())
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
			SetMode(EDIT);
		}
		else if (menuSelection == "upperleft")
		{
			SetMode(CREATE_ENEMY);
		}
		else if (menuSelection == "upperright")
		{
			SetMode(CREATE_TERRAIN);
		}
		else if (menuSelection == "lowerleft")
		{
			SetMode(CREATE_GATES);
			
		}
		else if (menuSelection == "lowerright")
		{
			SetMode(CREATE_RAILS);
			//AddActivePanel(mapOptionsPanel);
			//mapOptionsPanel->textBoxes["draintime"]->text.setString(to_string(drainSeconds));
			//mapOptionsPanel->textBoxes["bosstype"]->text.setString(to_string(bossType));
			//mode = menuDownStored;
			//			SetMode(menuDownStored);
		}
		else if (menuSelection == "bottom")
		{
			SetMode(CREATE_IMAGES);
			currImageTool = ITOOL_EDIT;
		}
	}
}

int EditSession::GetMouseOnBorderIndex()
{
	//0 top rect
	//1 left rect
	//2 right rectS

	for (int i = 0; i < 3; ++i)
	{
		if (QuadContainsPoint(fullBounds + i * 4, Vector2f(worldPos)))
		{
			return i;
		}
	}

	return -1;
}

void EditSession::EditModeUpdate()
{
	if (MOUSE.IsMouseLeftClicked())
	{
		justCompletedPolyWithClick = false;
	}

	if (MOUSE.IsMouseLeftClicked() && !mainWindowLostFocus)
	{
		if (!editModeUI->IsShowGrassOn() && !(editMouseDownMove || editMouseDownBox))
		{
			int borderIndex = GetMouseOnBorderIndex();
			grabbedBorderIndex = borderIndex;



			if (grabbedBorderIndex >= 0)
			{
				origLeftBounds = mapHeader->leftBounds;
				origTopBounds = mapHeader->topBounds;
				origBoundsRight = mapHeader->leftBounds + mapHeader->boundsWidth;
				origBoundsBot = mapHeader->topBounds + mapHeader->boundsHeight;
				return;
			}
			

			bool emptysp = true;
			//bool specialMode = GetSpecialTerrainMode() != 0;

			if (emptysp && PointSelectActor(worldPos))
			{
				emptysp = false;
			}

			if (emptysp && PointSelectRail(worldPos))
			{
				emptysp = false;
			}

			for (int i = TERRAINLAYER_Count - 1; i >= 0; --i)
			{
				if (editModeUI->IsLayerActionable(editModeUI->terrainEditLayerMap[i])
					&& emptysp && PointSelectTerrain(worldPos, i))
				{
					emptysp = false;
					break;
				}
			}

			if (emptysp && PointSelectDecor(worldPos))
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
				grabbedImage = NULL;
				grabbedBorderIndex = -1;
			}
			else
			{
				editMouseDownMove = true;
				editStartMove = false;
				editMouseDownBox = false;
			}
		}
		/*else if (editModeUI->IsShowGrassOn())
		{
			BackupGrass();
		}*/
	}
	else if (MOUSE.IsMouseLeftReleased() || mainWindowLostFocus)
	{
		if (grabbedBorderIndex >= 0)
		{
			int curr;
			int orig;
			switch (grabbedBorderIndex)
			{
			case 0:
			{
				curr = mapHeader->topBounds;
				orig = origTopBounds;	
				break;
			}
			case 1:
			{
				curr = mapHeader->leftBounds;
				orig = origLeftBounds;
				break;
			}
			case 2:
			{
				curr = mapHeader->leftBounds + mapHeader->boundsWidth;
				orig = origBoundsRight;
				break;
			}
				
			}

			if ( curr != orig )
			{
				ModifyBorderAction *mba = new ModifyBorderAction(grabbedBorderIndex,
					orig, curr);
				mba->performed = true;
				AddDoneAction(mba);
			}

			grabbedBorderIndex = -1;
		}

		if (editModeUI->IsShowGrassOn())
		{
			ChangeGrassAction();
			return;
		}

		if (editStartMove)
		{
			bool done = false;
			if (AnchorSelectedEnemies())
			{
				done = true;
				TryCompleteSelectedMove();
			}

			bool pointSuccess = true;
			if (!done)
			{
				PerformMovePointsAction();
			}

			//TryCompleteSelectedMove();
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
		grabbedImage = NULL;
		grabbedBorderIndex = -1;

		UpdateGrass();
	}

	bool rightClicked = MOUSE.IsMouseRightClicked();
	bool sizeOne = selectedBrush->objects.size() == 1;
	bool oneActor = selectedBrush->GetNumActors() == 1;
	if ( rightClicked && sizeOne &&
		oneActor )
	{
		ActorPtr a = selectedBrush->objects.front()->GetAsActor();
		if (a->type->info.name != "player")
		{
			if (a->ContainsPoint(testPoint))
			{
				Vector2i pixel = preScreenTex->mapCoordsToPixel(a->GetFloatPos());
				variationSelector->SetPosition(Vector2f(pixel));
				variationSelector->SetType(a->type);
				AddActivePanel(variationSelector->panel);
				focusedPanel = variationSelector->panel;
				return;
			}
		}
	}

	UpdateInputNonGame();

	if (GetCurrInput(0).start && !GetPrevInput(0).start)
	{
		TestPlayerMode();
		return;
	}

	if (focusedPanel == NULL)
	{
		if (IsGridOn())
		{
			SnapPointToGraph(worldPos, graph->graphSpacing);
		}
	}

	if (grabbedBorderIndex >= 0)
	{
		Vector2i wPos(worldPos);
		switch (grabbedBorderIndex)
		{
		case 0:
			{
				int bot = mapHeader->topBounds + mapHeader->boundsHeight;
				mapHeader->topBounds = wPos.y;
				mapHeader->boundsHeight = bot - wPos.y;
				break;
			}
		case 1:
		{
			int right = mapHeader->leftBounds + mapHeader->boundsWidth;
			mapHeader->leftBounds = wPos.x;
			mapHeader->boundsWidth = right - wPos.x;
			break;
		}
		case 2:
		{
			mapHeader->boundsWidth = wPos.x - mapHeader->leftBounds;
			break;
		}

		}

		UpdateFullBounds();
		
	}
		
	TrySelectedMove();

	ModifyGrass();
}

void EditSession::ChooseRectEvent(ChooseRect *cr, int eventType )
{
	
	if (mode == CREATE_ENEMY)
	{
		if (eventType == ChooseRect::E_LEFTCLICKED)
		{
			EnemyChooseRect *ceRect = cr->GetAsEnemyChooseRect();
			if (ceRect != NULL)
			{
				SetTrackingEnemy(ceRect->actorType, ceRect->level);
			}
			else
			{
				ImageChooseRect *icRect = cr->GetAsImageChooseRect();
				if (icRect != NULL && icRect->rectIdentity == ChooseRect::I_SEARCHENEMYLIBRARY)
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
		else if (eventType == ChooseRect::E_RIGHTCLICKED)
		{
			EnemyChooseRect *ceRect = cr->GetAsEnemyChooseRect();
			if (ceRect != NULL && ceRect->rectIdentity == ChooseRect::I_ENEMYLIBRARY)
			{
				createEnemyModeUI->ExpandVariation(ceRect);
			}
		}
	}
	else if (mode == CREATE_IMAGES)
	{
		if (eventType == ChooseRect::E_LEFTCLICKED)
		{
			ImageChooseRect *icRect = cr->GetAsImageChooseRect();	
			if (icRect != NULL && ( icRect->rectIdentity == ChooseRect::I_DECORLIBRARY
				|| icRect->rectIdentity == ChooseRect::I_DECORHOTBAR ))
			{
				SetTrackingDecor(icRect->CreateDecor());
				//SetTrackingDecor( new EditorDecorInfo( icRect->spr, EffectLayer::BETWEEN_PLAYER_AND_ENEMIES,
				//	icRect->))
				//settrackingImage
			}
			else if (icRect != NULL && icRect->rectIdentity == ChooseRect::I_SEARCHDECORLIBRARY)
			{
				createDecorModeUI->FlipLibraryShown();
			}
		}
		else if (eventType == ChooseRect::E_FOCUSED)
		{
			ImageChooseRect *icRect = cr->GetAsImageChooseRect();
			if (icRect != NULL && icRect->rectIdentity == ChooseRect::I_WORLDCHOOSER)
			{
				createDecorModeUI->SetActiveLibraryWorld(icRect->tileIndex);
			}
		}
	}
	else if (mode == EDIT)
	{
		if (eventType == ChooseRect::E_RIGHTRELEASED)
		{
			EnemyChooseRect *ceRect = cr->GetAsEnemyChooseRect();
			if (ceRect != NULL && ceRect->rectIdentity == ChooseRect::I_CHANGEENEMYVAR)
			{
				ActorPtr a = selectedBrush->objects.front()->GetAsActor();

				if (a->GetLevel() != ceRect->enemyParams->GetLevel())
				{
					//replace with different variation copy
					//need to do error checking here after its working.
					//an enemy might be too big or otherwise not work in this position

					Brush orig;
					Brush result;

					orig.AddObject(a);
					DeselectObject(a);

					ActorPtr newParams = a->Copy();
					newParams->SetLevel(ceRect->enemyParams->GetLevel());
					newParams->CreateMyEnemy();
					
					result.AddObject(newParams);
					SelectObject(newParams);
					ReplaceBrushAction *action = new ReplaceBrushAction(
						&orig, &result, mapStartBrush);
					action->Perform();
					AddDoneAction(action);
				}
			}
		}
	}
}

void EditSession::PasteModeUpdate()
{
	if (MOUSE.IsMouseLeftClicked())
	{
		PasteTerrain(copiedBrush, freeActorCopiedBrush);
	}
	else if (MOUSE.IsMouseLeftReleased())
	{
		if (complexPaste != NULL)
		{
			complexPaste = NULL;
		}
	}

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

	Vector2i centerPoint;
	if (pasteAxis <= 0)
	{
		centerPoint = pos;
	}
	else if (pasteAxis == 1)
	{
		centerPoint = Vector2i(pos.x, editMouseOrigPos.y);
	}
	else if (pasteAxis == 2)
	{
		centerPoint = Vector2i(editMouseOrigPos.x, pos.y);
	}

	Vector2i cent = GetCopiedCenter();

	if (freeActorCopiedBrush != NULL)
	{
		Vector2i diff = freeActorCopiedBrush->GetCenter() - cent;
		MoveActors(( centerPoint + diff ) - freeActorCopiedBrush->GetCenter(), worldPos, freeActorCopiedBrush);
		//freeActorCopiedBrush->CenterOnPoint(centerPoint + diff);
	}

	if (copiedBrush != NULL)
	{
		Vector2i test = centerPoint + ( copiedBrush->GetCenter() - cent );
		copiedBrush->CenterOnPoint(test );
	}

	editMouseGrabPos = pos;

	if (!panning && IsMousePressed(Mouse::Left) && (delta.x != 0 || delta.y != 0)
		&& length(lastBrushPastePos - worldPos ) >= brushRepeatDist )
	{
		PasteTerrain(copiedBrush, freeActorCopiedBrush);
	}
}

void EditSession::CreateEnemyModeUpdate()
{
	if (MOUSE.IsMouseLeftReleased())
	{
		if (grabbedActor != NULL)
		{
			bool done = false;
			if (AnchorSelectedEnemies())
			{
				done = true;
			}

			TryCompleteEnemyCreation();
		}

		editMouseDownBox = false;
		editMouseDownMove = false;
		editStartMove = false;
		grabbedActor = NULL;
		grabbedImage = NULL;

	}


	if (grabbedActor != NULL)
	{
		TrySelectedMove();
	}
}

void EditSession::CreatePatrolPathModeUpdate()
{
	TryAddToPatrolPath();
}

void EditSession::CreateRectModeUpdate()
{
	if (MOUSE.IsMouseLeftClicked())
	{
		if (!drawingCreateRect)
		{
			drawingCreateRect = true;
			createRectStartPoint = Vector2i(worldPos);
			createRectCurrPoint = Vector2i(worldPos);
		}
	}
	else if (MOUSE.IsMouseLeftReleased())
	{
		if (drawingCreateRect)
		{
			drawingCreateRect = false;

			createRectCurrPoint = Vector2i(worldPos);

			Vector2i rc = (createRectStartPoint + createRectCurrPoint) / 2;
			float width = abs(createRectCurrPoint.x - createRectStartPoint.x);
			float height = abs(createRectCurrPoint.y - createRectStartPoint.y);
			rectCreatingTrigger->SetRect(width, height, rc);
		}
	}
	
	if (!panning && MOUSE.IsMouseDownLeft())
	{
		createRectCurrPoint = Vector2i(worldPos);

		Vector2i rc = (createRectStartPoint + createRectCurrPoint) / 2;
		float width = abs(createRectCurrPoint.x - createRectStartPoint.x);
		float height = abs(createRectCurrPoint.y - createRectStartPoint.y);
		rectCreatingTrigger->SetRect(width, height, rc);

		if (grabbedImage != NULL)
		{
			TrySelectedMove();
		}
	}
}

void EditSession::SetCamZoomModeUpdate()
{
	if (MOUSE.IsMouseLeftClicked())
	{
		currentCameraShot->SetZoom(Vector2i(testPoint),HoldingControl() );

		if (tempActor != NULL) //is tempactor still a thing?
		{
			SetMode(CREATE_ENEMY);
		}
		else
		{
			SetMode(EDIT);
		}

		AddActivePanel(currentCameraShot->type->panel);
	}

	currentCameraShot->SetZoom(Vector2i(testPoint), HoldingControl());
}

void EditSession::SetDirectionModeUpdate()
{
	minimumPathEdgeLength = 16;

	if( MOUSE.IsMouseLeftClicked() )
	{
		Vector2i worldi(testPoint.x, testPoint.y);
		patrolPath.push_back(worldi);

		if (trackingEnemyParams != NULL)
		{
			trackingEnemyParams->SetPath(patrolPath);
			FinishEnemyCreation();
			SetMode(CREATE_ENEMY);
			return;
		}

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

		//AddActivePanel(actor->type->panel);
		actor->SetPath(patrolPath);
	}
}

void EditSession::CreateGatesModeUpdate()
{
	if (MOUSE.IsMouseLeftClicked())
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			TryPlaceGatePoint(worldPos);
		}
	}

	PolyPtr p = NULL;
	TerrainPoint *pPoint = NULL;
	if (gatePoints == 1)
	{
		SelectPtr obj = NULL;
		pPoint = TrySnapPosToPoint(worldPos, obj, 8 * zoomMultiple);
		if( obj != NULL )
			p = obj->GetAsTerrain();
	}

	gateInProgressTestPoly = p;
	gateInProgressTestPoint = pPoint;

	if (modifyGate != NULL)
	{
		/*GridSelectPop("gateselect");

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
		return;*/
	}

	//DrawGateInProgress();

	if (gatePoints > 1)
	{
		ClearMostRecentError();
		bool result = IsGateValid(&testGateInfo);//CanCreateGate(testGateInfo);

		if (result)
		{
			//GridSelectPop("gateselect");
			//string gateResult = tempGridResult;

			

				/*if (gateResult == "shard")
				{
					GridSelectPop("shardselector");
					int sw, si;
					GetShardWorldAndIndex(tempGridX, tempGridY, sw, si);
					testGateInfo.SetShard(sw, si);
				}*/

			Vector2i adjust(0, 0);
				
			createGatesModeUI->SetGateInfo(&testGateInfo);

			Action *action = new CreateGateAction(testGateInfo);
			action->Perform();

			if (GetPrimaryAdjustment(testGateInfo.point0->pos, testGateInfo.point1->pos, adjust))
			{
				CompoundAction *testAction = new CompoundAction;
				testAction->subActions.push_back(action);

				if (!TryGateAdjustAction(GATEADJUST_POINT_B, &testGateInfo, adjust, testAction))
				{
					action->Undo();
					delete action;
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

			modifyGate = NULL;
			gatePoints = 0;
		}
		else
		{
			ShowMostRecentError();
			gatePoints = 1;
			testGateInfo.poly1 = NULL;
			testGateInfo.point1 = NULL;
			//MessagePop("gate would intersect some terrain");
		}

		
	}
}

void EditSession::CreateImagesModeUpdate()
{
	if (MOUSE.IsMouseLeftReleased())
	{
		if (grabbedImage != NULL)
		{
			TryCompleteSelectedMove();
		}

		editMouseDownBox = false;
		editMouseDownMove = false;
		editStartMove = false;
		grabbedImage = NULL;
	}

	//createDecorModeUI->Update(IsMousePressed(Mouse::Left), IsMousePressed(Mouse::Right), Vector2i(uiMousePos.x, uiMousePos.y));
	//createDecorModeUI->UpdateSprites(spriteUpdateFrames);

	/*if (grabbedActor != NULL)
	{
		grabbedActor->myEnemy->UpdateFromEditParams(spriteUpdateFrames);
		TrySelectedMove();
	}*/

	if (grabbedImage != NULL)
	{
		TrySelectedMove();
	}
}

void EditSession::TransformModeUpdate()
{
	if (IsKeyPressed(Keyboard::G))
	{
		SnapPointToGraph(testPoint, graph->graphSpacing);
		showGraph = true;
	}


	Vector2f fWorldPos(worldPos);
	if (transformTools->mode == TransformTools::SCALE && showGraph)
	{
		fWorldPos = testPoint;
	}
	transformTools->Update(fWorldPos, IsMousePressed(Mouse::Left));
	

	PolyPtr p;
	DecorPtr dec;
	RailPtr r;
	for (auto it = selectedBrush->objects.begin(); it != selectedBrush->objects.end(); ++it)
	{
		p = (*it)->GetAsTerrain();
		if (p != NULL)
		{
			p->UpdateTransformation(transformTools);
		}

		r = (*it)->GetAsRail();
		if (r != NULL)
		{
			r->UpdateTransformation(transformTools);
		}

		dec = (*it)->GetAsDecor();
		if (dec != NULL)
		{
			dec->UpdateTransformation(transformTools);
		}
	}
}
