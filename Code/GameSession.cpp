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

#include "BonusHandler.h"
#include "GameMode.h"
#include "ggponet.h"
#include "GGPO.h"

#include "NetplayManager.h"

#include "MatchResultsScreen.h"

#include "MatchStats.h"
#include "CustomCursor.h"
#include "AdventureManager.h"
#include "globals.h"
#include "Leaderboard.h"
#include "UIMouse.h"
#include "md5.h"
#include "UIController.h"
#include "PracticeInviteDisplay.h"
#include "FeedbackForm.h"
#include "Leaderboard.h"
#include "ReplayHUD.h"
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
	if (IsParallelSession())
		return false;

	if (simulationMode)
		return false;

	Actor *p0 = GetPlayer(0);

	bool ggpoNetplay = netplayManager != NULL && !netplayManager->IsPracticeMode();// && ggpo != NULL;

	if (ggpoNetplay && matchParams.numPlayers > 1 && (gameModeType == MatchParams::GAME_MODE_FIGHT || gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE) )
	{
		ControllerState currInput;

		ControllerState prevInput;

		currInput = GetCurrInputFiltered(0);//GetPlayer(0)->currInput;
		prevInput = GetPrevInputFiltered(0);//GetPlayer(0)->prevInput;


		if (currInput.start && !prevInput.start)
		{
			onlinePauseMenuOn = !onlinePauseMenuOn;
			if (onlinePauseMenuOn)
			{
				assert(gameMode->onlinePauseMenu != NULL);
				gameMode->onlinePauseMenu->Reset();
			}
			//if (gameState == GameState::RUN)
			{
				//onlinePauseMenuOn = true;
				//gameState = GameState::ONLINE_PAUSE;
				//cout << "setting mode to online_pause" << "\n";
				//ActivatePauseSound(GetSound("pause_on"));
				//switchGameState = true;
				//UpdateControllers();
				//CONTROLLERS.Update();

			}
			//else if (gameState == GameState::ONLINE_PAUSE)
			//{
			//	gameState = GameState::RUN;
			//	cout << "setting mode to run" << "\n";
			//	//ActivatePauseSound(GetSound("pause_off"));
			//	//switchGameState = true;
			//	//CONTROLLERS.Update();
			//	//UpdateControllers();
			//}

			
			
		}

		return false;
	}

	//eventually add better logic for when its okay to pause in multiplayer etc
	if ( (matchParams.numPlayers == 1 || gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE ) && !p0->IsGoalKillAction(p0->action) && !p0->IsExitAction(p0->action))
	{
		ControllerState currInput;
		
		ControllerState prevInput;

		currInput = GetCurrInputFiltered(0);//GetPlayer(0)->currInput;
		prevInput = GetPrevInputFiltered(0);//GetPlayer(0)->prevInput;

		//cout << "currstart: " << currInput.start << "\n";

		/*if (controllerStates[0]->GetControllerType() == CTYPE_KEYBOARD)
		{
			
		}
		else
		{
			currInput = GetCurrInput(0);
			prevInput = GetPrevInput(0);
		}*/
		
		//if( IsKeyPressed( Keyboard ) )
		if (currInput.start && !prevInput.start)
		{
			gameState = PAUSE;
			ActivatePauseSound(GetSound("pause_on"));
			pauseMenu->SetTab(PauseMenu::PAUSE);
			if (soundNodeList != NULL)
			{
				soundNodeList->Pause(true);
			}
			
			return true;
		}
		else if (currInput.back && !prevInput.back) //|| CONTROLLERS.KeyboardButtonHeld(Keyboard::G))
		{
			gameState = PAUSE;
			pauseMenu->SetTab(PauseMenu::MAP);
			ActivatePauseSound(GetSound("pause_on"));
			if (soundNodeList != NULL)
			{
				soundNodeList->Pause(true);
			}
			
			return true;
		}
	}
	else if ((matchParams.numPlayers == 1 || gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE)
		&& mainMenu->gameRunType == MainMenu::GRT_ADVENTURE && scoreDisplay->waiting)
	{
		if (CONTROLLERS.KeyboardButtonPressed(Keyboard::F1))
		{
			gameState = FEEDBACK_FORM;
			mainMenu->adventureManager->feedbackForm->Activate(this);
			if (soundNodeList != NULL)
			{
				soundNodeList->Pause(true);
			}
			MOUSE.SetControllersOn(false);
			MOUSE.Show();
			return true;
		}
	}

	if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && !p0->IsGoalKillAction(p0->action) && !p0->IsExitAction(p0->action) && !IsReplayOn())
	{
		ControllerState currInput;

		ControllerState prevInput;

		currInput = GetCurrInputFiltered(0);//GetPlayer(0)->currInput;
		prevInput = GetPrevInputFiltered(0);//GetPlayer(0)->prevInput;

		if (currInput.PUp() && !prevInput.PUp())
		{
			gameState = PRACTICE_INVITE;

			ParallelPracticeMode *ppm = (ParallelPracticeMode*)gameMode;
			ppm->ResetInviteDisplay();

			if (soundNodeList != NULL)
			{
				soundNodeList->Pause(true);
			}
			return true;
		}
	}

	return false;
}

//void GameSession::StartPracticeInviteMode()
//{
//	gameState = PRACTICE_INVITE;
//
//	ParallelPracticeMode *ppm = (ParallelPracticeMode*)gameMode;
//	ppm->ResetInviteDisplay();
//
//	soundNodeList->Pause(true);
//}

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

	switch (gameModeType)
	{
	case MatchParams::GAME_MODE_PARALLEL_PRACTICE:
	case MatchParams::GAME_MODE_BASIC:
	{
		cam.SetCamType(Camera::CamType::BASIC);
		cam.playerIndex = 0;
		cam.Update();
		break;
	}
	case MatchParams::GAME_MODE_FIGHT:
	{
		cam.SetCamType(Camera::CamType::FIGHTING);
		cam.Update();
		//cam.playerIndex = 0;
		break;
	}
	case MatchParams::GAME_MODE_RACE:
	{
		cam.SetCamType(Camera::CamType::FIGHTING);
		cam.Update();
		break;
	}
	case MatchParams::GAME_MODE_PARALLEL_RACE:
	{
		cam.SetCamType(Camera::CamType::BASIC);
		cam.playerIndex = 0;
		cam.Update();
		/*cam.SetCamType(Camera::CamType::FIGHTING);
		cam.Update();*/
		break;
	}
	case MatchParams::GAME_MODE_COOP:
	{
		//eventually need another cam type which is fighting + enemies for coop
		cam.SetCamType(Camera::CamType::FIGHTING);
		cam.Update();
		break;
	}
	case MatchParams::GAME_MODE_EXPLORE:
	{
		if (matchParams.numPlayers == 1)
		{
			cam.SetCamType(Camera::CamType::BASIC);
			cam.playerIndex = 0;
			cam.Update();
		}
		else
		{
			cam.SetCamType(Camera::CamType::FIGHTING);
			cam.Update();
		}
		break;
	}
	}
	

	Vector2f camPos = cam.GetPos();
	double camWidth = 960 * cam.GetZoom();
	double camHeight = 540 * cam.GetZoom();

	//cout << "camPos: " << camPos.x << ", " << camPos.y << endl;

	screenRect = sf::Rect<double>(cam.GetRect());//sf::Rect<double>(camPos.x - camWidth / 2, camPos.y - camHeight / 2, camWidth, camHeight);

	view.setSize(Vector2f( 960 * cam.GetZoom(), 540 * cam.GetZoom()));
	//this is because kin's sprite is 2x size in the game as well as other stuff
	lastViewSize = view.getSize();
	view.setCenter(camPos.x, camPos.y);

	view.setRotation(cam.GetRotation());

	//lastViewCenter = view.getCenter(); //depreciated completely
}

void GameSession::DrawShockwaves(sf::RenderTarget *target)
{
	DrawSceneToPostProcess(postProcessTex2);
	/*for (auto it = shockwaves.begin(); it != shockwaves.end(); ++it)
	{
		(*it)->Draw(postProcessTex2, target);
	}*/
}


void GameSession::DrawTerrain(sf::RenderTarget *target)
{
	//kind of messy. its when you have the camera not seeing the edges
	//of the map, 
	bool inverseInList = false;
	PolyPtr poly = polyQueryList;
	while (poly != NULL)
	{
		if (poly->inverse)
		{
			inverseInList = true;
		}
		poly = poly->queryNext;
	}


	DrawQueriedTerrain(target);

	if ( inversePolygon != NULL && !inverseInList)
	{
		inversePolygon->Draw(target);
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
	DrawQueriedSpecialTerrain(target);
}

//returns gameresult of the bonus
int GameSession::TryToActivateBonus()
{
	Actor *p = NULL;

	int bonusReturnVal = GR_BONUS_RETURN;
	if (bonusGame != NULL)
	{
		if (activateBonus)
		{
			activateBonus = false;
			currSession = bonusGame;
			for (int i = 0; i < MAX_PLAYERS; ++i)
			{
				p = GetPlayer(i);
				if (p != NULL)
				{
					p->SetSession(bonusGame, bonusGame, NULL);
				}
			}

			pauseMenu->game = bonusGame;

			bonusGame->oneFrameMode = oneFrameMode;

			bonusGame->myBestReplayManager = myBestReplayManager;

			bonusGame->activePlayerReplayManagers = activePlayerReplayManagers;

			bonusGame->bestReplayOn = bestReplayOn;

			bonusGame->bestTimeGhostOn = bestTimeGhostOn;

			bonusGame->replayGhosts = replayGhosts;

			bonusGame->RestartLevel();

			Actor *p;
			for (int i = 0; i < MAX_PLAYERS; ++i)
			{
				p = GetPlayer(i);
				if (p != NULL)
				{
					p->position = V2d(bonusGame->playerOrigPos[i]);
				}
			}


			bonusGame->Run();

			
			
			pauseMenu->game = this;
			currSession = this;
			for (int i = 0; i < MAX_PLAYERS; ++i)
			{
				p = GetPlayer(i);
				if (p != NULL)
				{
					p->SetSession(this, this, NULL);
					p->position = bonusReturnPos;
					//p->SetAirPos(pi->pos, p->facingRight);
					//p->Respawn(); //special respawn for leaving bonus later
				}
			}

			//might have to do recursive resetting here

			//assert(parentGame == NULL);
			//cout << "Restarting clock" << "\n";
			//gameClock.restart();
			currentTime = gameClock.getElapsedTime().asSeconds();

			if (bonusHandler != NULL)
			{
				bonusHandler->ReturnFromBonus();
			}

			bonusType = BONUSTYPE_NONE;
			

			bonusHandler = NULL;


			if (bonusGame->returnVal == GR_BONUS_RETURN)
			{
				bonusReturnVal = GR_BONUS_RETURN;
			}
			else if (bonusGame->returnVal == GR_BONUS_RESPAWN)
			{
				if (parentGame != NULL)
				{
					quit = true;
					returnVal = GR_BONUS_RESPAWN;
					bonusReturnVal = GR_BONUS_RESPAWN;
				}
				else
				{
					RestartGame();
					bonusReturnVal = GR_BONUS_RESPAWN;
				}
			}
			else// if (bonusGame->returnVal == GR_EXITLEVEL)
			{
				quit = true;
				returnVal = bonusGame->returnVal;//GR_EXITLEVEL;
				bonusReturnVal = GR_EXITLEVEL;
			}
			//cout << "setting current time to: " << currentTime << "\n";

			//cout << "restarted time: " << gameClock.getElapsedTime().asSeconds() << "\n";
			//accumulator = TIMESTEP + .1;

			//currentTime = gameClock.getElapsedTime().asSeconds();
			//accumulator = TIMESTEP + .01;

			//GetCurrInput(0) = bonusGame->GetCurrInput(0);
			//GetPrevInput(0) = bonusGame->GetPrevInput(0);

		}
	}

	return bonusReturnVal;
}


//DEPRECIATED
string GameSession::GetBestTimeGhostPath()
{
	assert(saveFile != NULL);
	if( saveFile != NULL )
	{
		stringstream ss;

		ss << "Resources/Adventure/SaveData/" << saveFile->name << "/" << level->index << "_besttime" << GHOST_EXT;

		return ss.str();
	}
	else
	{
		return "";
	}
}

std::string GameSession::GetBestReplayPath()
{
	assert(saveFile != NULL);
	if (saveFile != NULL)
	{
		stringstream ss;

		int w, s, m;
		AdventureFile::GetMapIndexes(level->index, w, s, m);

		
		//ss << "Resources/Adventure/SaveData/" << saveFile->name << "/" << level->index << "_best" << REPLAY_EXT;
		//ss << "Resources/Adventure/SaveData/" << saveFile->name << "/" << saveFile->adventureFile->GetMap(level->index).name << "_" << myHash << "_best" << REPLAY_EXT;//<< w+1 << "_" << s+1 << "_" << m+1 << "_best" << REPLAY_EXT;
		ss << saveFile->replayFolderName << saveFile->adventureFile->GetMap(level->index).name << "_" << myHash << "_best" << REPLAY_EXT;//<< w+1 << "_" << s+1 << "_" << m+1 << "_best" << REPLAY_EXT;
		return ss.str();
	}
	else
	{
		return "";
	}
}

GameSession * GameSession::CreateBonus(const std::string &bonusName, int p_bonusType )
{
	boost::filesystem::path p("Resources/Maps/" + bonusName + MAP_EXT);
	
	MatchParams mp = matchParams;
	//mp.saveFile = saveFile;
	mp.mapPath = p;
	//mp.gameModeType = gameModeType;

	GameSession *newBonus = new GameSession(&mp);
	newBonus->bonusType = p_bonusType;
	newBonus->SetParentGame(this);
	newBonus->Load();

	numSimulatedFramesRequired = max(numSimulatedFramesRequired,
		newBonus->numSimulatedFramesRequired );

	currSession = this;
	pauseMenu->game = this;

	return newBonus;
}

GameSession *GameSession::CreateParallelSession( int parIndex )
{
	MatchParams mp;
	/*mp.saveFile = saveFile;
	mp.mapPath = matchParams.mapPath;
	mp.gameModeType = matchParams.gameModeType;*/
	mp = matchParams;

	GameSession *parallelGame = new GameSession(&mp);

	parallelGame->SetParentTilesetManager(this);

	parallelGame->parallelSessionIndex = parIndex;

	parallelGame->level = level;

	parallelGame->Load();

	parallelGame->mapHeader->envWorldType = background->envWorld;

	parallelGame->RestartLevel();

	currSession = this;
	pauseMenu->game = this;

	return parallelGame;
}


void GameSession::SetBonus(GameSession *bonus,
	V2d &returnPos, BonusHandler *bHandler)
{
	assert(bonus != NULL);

	activateBonus = true;
	bonusReturnPos = returnPos;
	bonusHandler = bHandler;
	bonusGame = bonus;
	bonusGame->cam.offset = cam.offset;
	bonusGame->cam.zoomFactor = cam.zoomFactor;
}

void GameSession::ReturnFromBonus()
{
	if (parentGame != NULL)
	{
		quit = true;
		returnVal = GameSession::GR_BONUS_RETURN;
		parentGame->cam.offset = cam.offset;
		parentGame->cam.zoomFactor = cam.zoomFactor;
	}
}

void GameSession::DrawSceneToPostProcess(sf::RenderTexture *tex)
{
	Sprite blah;
	blah.setTexture(preScreenTex->getTexture());
	tex->draw(blah);
	tex->display();
}

void GameSession::QuitGame()
{
	quit = true;
	returnVal = resType;
}

bool GameSession::RunPreUpdate()
{
	/*if (IsKeyPressed(sf::Keyboard::Y))
	{
		quit = true;
		return false;
	}*/

	UpdateDebugModifiers();

	//if you get a message to play but you're not in the menu already
	if (netplayManager != NULL && gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && !IsParallelSession())
	{
		if (netplayManager->HasBeenInvitedToPracticeRace())
		{
			gameState = PRACTICE_INVITE;

			ParallelPracticeMode *ppm = (ParallelPracticeMode*)gameMode;
			ppm->SetInviteDisplayPrepareToLeave();

			if (soundNodeList != NULL)
			{
				soundNodeList->Pause(true);
			}
		}
		/*ParallelPracticeMode *ppm = (ParallelPracticeMode*)gameMode;
		if (ppm->practiceInviteDisplay->IsTryingToStartMatch())
		{
			StartRaceFromPractice();
		}*/
	}

	/*if (goalDestroyed)
	{
		quit = true;
		returnVal = resType;
		return false;
	}*/



	if (nextFrameRestartGame)
	{
		RestartLevel();
	}

	//if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && IsParallelSession())
	//{
	//	PracticePlayer &prac = netplayManager->practicePlayers[parallelSessionIndex];
	//	if (prac.action == PracticePlayer::A_RUNNING && prac.syncStateBuf != NULL)
	//	{
	//		LoadState(prac.syncStateBuf, prac.syncStateBufSize);
	//		prac.ClearSyncStateBuf();
	//		prac.ClearMessages();
	//		prac.waitingForFrame = totalGameFrames;
	//		prac.nextFrameToRead = totalGameFrames;
	//		//nextFrameRestartGame = true;
	//		//netplayManager->practicePlayers[parallelSessionIndex].action = PracticePlayer::A_RUNNING;
	//	}
	//}

	return true;
}

bool GameSession::RunPostUpdate()
{
	if (goalDestroyed)
	{
		quit = true;
		returnVal = resType;
		return false;
	}

	return true;
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



void GameSession::Reload(const boost::filesystem::path &p_filePath)
{
	//partial cleanup
	originalMusic = NULL;

	for (int i = 0; i < allPolysVec.size(); ++i)
	{
		delete allPolysVec[i];
	}
	allPolysVec.clear();

	allComboObjectsVec.clear();

	CleanupZones();

	CleanupBarriers();

	CleanupCameraShots();

	for (auto it = allSpawnableEnemiesVec.begin(); it != allSpawnableEnemiesVec.end(); ++it)
	{
		delete (*it);
	}
	allSpawnableEnemiesVec.clear();


	allEnemiesVec.clear();

	/*for (auto it = allEnemiesVec.begin(); it != allEnemiesVec.end(); ++it)
	{
		delete (*it);
	}
	allEnemiesVec.clear();*/

	CleanupGlobalBorders();

	for (auto it = decorLayerMap.begin(); it != decorLayerMap.end(); ++it)
	{
		delete (*it).second;
	}
	decorLayerMap.clear();

	for (auto it = allSpecialPolysVec.begin(); it != allSpecialPolysVec.end(); ++it)
	{
		delete (*it);
	}
	allSpecialPolysVec.clear();

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

	allSequencesVec.clear();

	allEmittersVec.clear();
	

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

GameSession::GameSession(MatchParams *mp )
	:Session( Session::SESS_GAME, mp->mapPath)
{
	originalProgressionCompatible = false;

	isDefaultKeyboardInputOn = false;

	SetMatchParams(*mp);
	currSession = this;
	Init();
}

void GameSession::Cleanup()
{
	/*if (bonusGame != NULL)
	{
		delete bonusGame;
		bonusGame = NULL;
	}*/

	if (mainMenu->gameRunType == MainMenu::GRT_ADVENTURE && mainMenu->adventureManager != NULL)
	{
	}
	else
	{
		assert(pauseMenu != NULL);
		delete pauseMenu;
		pauseMenu = NULL;
	}


	for( int i = 0; i < allPolysVec.size(); ++i)
	{
		delete allPolysVec[i];
	}
	allPolysVec.clear();

	/*for (auto it = allEnemiesVec.begin(); it != allEnemiesVec.end(); ++it)
	{
		delete (*it);
	}
	allEnemiesVec.clear();*/
	for (auto it = allSpawnableEnemiesVec.begin(); it != allSpawnableEnemiesVec.end(); ++it)
	{
		delete (*it);
	}
	allSpawnableEnemiesVec.clear();

	allEnemiesVec.clear();


	CleanupGoalFlow();

	for (auto it = decorLayerMap.begin(); it != decorLayerMap.end(); ++it)
	{
		delete (*it).second;
	}
	decorLayerMap.clear();

	for (auto it = allSpecialPolysVec.begin(); it != allSpecialPolysVec.end(); ++it)
	{
		delete (*it);
	}
	allSpecialPolysVec.clear();

	for (auto it = allRailsVec.begin(); it != allRailsVec.end(); ++it)
	{
		delete (*it);
	}
	allRailsVec.clear();

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

	

	CleanupGoalPulse();

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

	CleanupMyBestPlayerReplayManager();

	ClearReplayGhosts();

	if (parentGame == NULL && playerRecordingManager != NULL)
	{
		delete playerRecordingManager;
		playerRecordingManager = NULL;
	}

	CleanupPopup();
}

GameSession::~GameSession()
{
	Cleanup();

	if (currSession == this)
	{
		currSession = NULL;
	}
}

GameSession *GameSession::GetSession()
{
	return currSession;
}

void GameSession::SetMatchParams(MatchParams &mp)
{
	matchParams = mp;


	if (matchParams.HasControllerStates())
	{
		for (int i = 0; i < 4; ++i)
		{
			SetControllerStates(i, matchParams.controllerStateVec[i]);
		}
	}

	for (int i = 0; i < 4; ++i)
	{
		controlProfiles[i] = matchParams.controlProfiles[i];
	}

	for (int i = 0; i < 4; ++i)
	{
		if (matchParams.playerSkins[i] == -1)
		{
			matchParams.playerSkins[i] = i;
		}
	}

	SeedRand(matchParams.randSeed);
	saveFile = matchParams.saveFile;
	netplayManager = matchParams.netplayManager;

	gameModeType = matchParams.gameModeType;

	/*assert(netplayManager != NULL);
	cout << "test queue 1" << "\n";
	for (auto it = netplayManager->ggpoMessageQueue.begin(); it != netplayManager->ggpoMessageQueue.end(); ++it )
	{
		cout << (*it) << "\n";
	}*/
}

void GameSession::CheckSinglePlayerInputDefaultKeyboard()
{
	if (matchParams.numPlayers > 1)
		return;

	
	ControllerDualStateQueue *paramStates = matchParams.controllerStateVec[0];
	if (paramStates->GetControllerType() == CTYPE_KEYBOARD)
	{
		return;
	}

	ControllerDualStateQueue *keyboardStates = CONTROLLERS.GetStateQueue(CTYPE_KEYBOARD, 0);

	bool paramStatesDoingAnything = paramStates->IsDoingAnything();

	ControlProfile *defaultKeyboardProfile = mainMenu->cpm->profiles[CTYPE_KEYBOARD].front();
	ControllerState currTestInput;
	CONTROLLERS.UpdateFilteredKeyboardState(defaultKeyboardProfile, currTestInput, GetPlayer(0)->prevInput);

	bool keyboardStatesDoingAnything = keyboardStates->IsDoingAnything();

	if (!currTestInput.IsLeftNeutral())
	{
		keyboardStatesDoingAnything = true;
	}

	if (isDefaultKeyboardInputOn)
	{
		if (paramStatesDoingAnything && !keyboardStatesDoingAnything)
		{
			//update all icons here
			SetControllerStates(0, paramStates);
			controlProfiles[0] = matchParams.controlProfiles[0];
			isDefaultKeyboardInputOn = false;

			//eventually probably just have a function that updates all the icons when either happens, since its basically the same code
			pauseMenu->UpdateButtonIconsWhenControllerIsChanged();
		}
	}
	else
	{
		if (keyboardStatesDoingAnything && !paramStatesDoingAnything)
		{
			//update all icons here
			SetControllerStates(0, keyboardStates);
			controlProfiles[0] = defaultKeyboardProfile;
			isDefaultKeyboardInputOn = true;

			pauseMenu->UpdateButtonIconsWhenControllerIsChanged();
		}
	}
}

void GameSession::UnlockUpgrade(int upgradeType, int playerIndex )
{
	if (IsReplayOn())
	{
		//replay on
		//currUpgradeField.SetBit(upgradeType, true);
		//GetPlayer(playerIndex)->SetStartUpgrade(upgradeType, true);
		//do I need to set the currUpgradeField here?

		return;
	}
	else if (saveFile != NULL && !IsParallelSession())
	{
		saveFile->UnlockUpgrade(upgradeType);
	}

	currUpgradeField.SetBit(upgradeType, true);
	GetPlayer(playerIndex)->SetStartUpgrade(upgradeType, true);

	//GetPlayer(playerIndex)->SetStartUpgrade(upgradeType, true);
}

bool GameSession::TrySaveCurrentSaveFile()
{
	if (IsReplayOn())
	{
		//replay on
		return false;
	}
	else if (IsParallelSession())
	{
		return false;
	}
	else if (saveFile != NULL )
	{
		saveFile->Save();
		return true;
	}

	return false;
}


void GameSession::UnlockLog(int lType, int playerIndex )
{
	if (IsReplayOn())
	{
		//replay on
		return;
	}

	if (saveFile != NULL)
	{
		saveFile->UnlockLog(lType);
	}

	currLogField.SetBit(lType, true);
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
		allSpecialPolysVec.push_back(poly);
		specialTerrainTree->Insert(poly);
		waterMatSet.insert(poly->waterType);
	}
	else if (specialType == 2)
	{
		poly->AddFliesToWorldTrees();
		poly->AddFliesToQuadTree(enemyTree);
		allSpecialPolysVec.push_back(poly);
		flyTerrainTree->Insert(poly);
	}
	//matSet.insert(make_pair(poly->terrainWorldType, poly->terrainVariation));
	
}

void GameSession::ProcessAllSpecialTerrain()
{
	int numMats = waterMatSet.size();

	waterTypeIndexesUsedVec.reserve(numMats);
	for (auto it = waterMatSet.begin(); it != waterMatSet.end(); ++it)
	{
		waterTypeIndexesUsedVec.push_back((*it));
	}
}

void GameSession::ProcessGate(int gCat,
	int gVar,
	int numToOpen,
	int poly0Index, int vertexIndex0, int poly1Index,
	int vertexIndex1, int shardWorld,
	int shardIndex, int seconds)
{
	if (inversePolygon != NULL)
	{
		poly0Index++;
		poly1Index++;
	}

	GateInfo gi;
	gi.category = gCat;
	gi.variation = gVar;
	gi.numToOpen = numToOpen;
	gi.poly0 = allPolysVec[poly0Index];
	gi.poly1 = allPolysVec[poly1Index];
	gi.vertexIndex0 = vertexIndex0;
	gi.vertexIndex1 = vertexIndex1;
	gi.point0 = gi.poly0->GetPoint(vertexIndex0);
	gi.point1 = gi.poly1->GetPoint(vertexIndex1);
	gi.shardWorld = shardWorld;
	gi.shardIndex = shardIndex;
	gi.seconds = seconds;

	Gate * gate = new Gate(this, gCat, gVar);
	gate->Setup(&gi);
}

void GameSession::ProcessRail(RailPtr rail)
{
	if (rail->IsEnemyType())
	{
		ProcessActor(rail->enemyParams);
		rail->enemyParams = NULL;
		delete rail;
	}
	else
	{
		rail->railIndex = totalRails;
		totalRails++; //is this really even needed?
		allRailsVec.push_back(rail);
		rail->AddEdgesToQuadTree(railEdgeTree);
		railDrawTree->Insert(rail);
	}
}

void GameSession::ProcessHeader()
{
	/*if (mapHeader->gameMode == MapHeader::MapType::T_RACEFIGHT)
	{
		assert(raceFight == NULL);
		raceFight = new RaceFight(this, 180);
	}*/
}


void GameSession::ProcessDecorSpr(const std::string &name, int dTile, int dLayer, sf::Vector2f &centerPos,
	float rotation, sf::Vector2f &scale)
{
	Sprite dSpr;
	dSpr.setScale(scale);
	dSpr.setRotation(rotation);
	dSpr.setPosition(centerPos);

	Tileset *d_ts = GetSizedTileset(name + ".png");
	//assumes decor tilesets are sized already

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

	int numMats = matSet.size();

	terrainTypeIndexesUsedVec.reserve(numMats);
	for (auto it = matSet.begin(); it != matSet.end(); ++it)
	{
		terrainTypeIndexesUsedVec.push_back((*it).first * 8 + (*it).second);
	}

	int index = 0;
	for (set<pair<int, int>>::iterator it = matSet.begin(); it != matSet.end(); ++it)
	{
		//load decor here somehow? matindices might be needed for that?

		/*if (!LoadPolyShader(index, (*it).first, (*it).second))
		{
			assert(0);
		}

		int texInd = (*it).first * Session::MAX_TERRAINTEX_PER_WORLD + (*it).second;
		matIndices[texInd] = index;

		++index;*/

		
	}

	PolyPtr poly;
	allPolysVec.reserve(allPolygonsList.size());
	int polyIndex = 0;
	for (auto it = allPolygonsList.begin(); it != allPolygonsList.end(); ++it)
	{
		poly = (*it);
		poly->polyIndex = polyIndex;
		poly->Finalize();
		poly->grassBufferForAABBOn = true; //so that the quadtree can get a bigger AABB for this
		poly->AddEdgesToQuadTree(terrainTree);
		//poly->AddGrassToQuadTree(grassTree);

		if (poly->inverse)
		{
			poly->AddEdgesToQuadTree(inverseEdgeTree);
			inversePolygon = poly;
		}
		borderTree->Insert(poly);

		allPolysVec.push_back((*it));

		polyIndex++;
	}
	allPolygonsList.clear();
}

void GameSession::ProcessActor(ActorPtr a)
{
	Enemy *enemy = a->myEnemy; //enemy already created during loading from file

	const string &typeName = a->type->info.name;
	if (enemy != NULL)
	{
		numSimulatedFramesRequired = max(numSimulatedFramesRequired,
			enemy->GetNumSimulationFramesRequired());

		enemy->AddToGame();

		if (typeName == "shippickup")
		{
			if (shipExitScene == NULL)
			{
				shipExitScene = new ShipExitScene;
				shipExitScene->Init();
				shipExitScene->SetIDAndAddToAllSequencesVec();
			}
		}
	}
	else
	{
		if (typeName == "xbarrier")
		{
			XBarrierParams *xbp = (XBarrierParams*)a;
			AddBarrier(xbp, false);
		}
		else if ( typeName == "xbarrierwarp")
		{
			XBarrierParams *xbp = (XBarrierParams*)a;
			AddBarrier(xbp, true);
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
		else if (typeName == "crawlernode"
			|| typeName == "birdnode" || typeName == "coyotenode" 
			|| typeName == "tigernode" || typeName == "gatornode"
			|| typeName == "skeletonnode" 
			|| typeName == "greyskeletonnode"
			|| typeName == "bearnode" )
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
				shipEnterScene->SetIDAndAddToAllSequencesVec();
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

	a->myEnemy = NULL;
	delete a; //eventually probably delete these all at once or something
}

void GameSession::ProcessAllActors()
{
	//how does this know the right number of bullets?
	//CreateBulletQuads();


	if (mapHeader->preLevelSceneName != "NONE")
	{
		preLevelScene = Sequence::CreateScene(mapHeader->preLevelSceneName);
	}

	if (mapHeader->postLevelSceneName != "NONE")
	{
		postLevelScene = Sequence::CreateScene(mapHeader->postLevelSceneName);
	}

	////create sequences for the barriers after all enemies have already been loaded
	//SetupBarrierScenes();

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
	if (mainMenu->gameRunType == MainMenu::GRT_ADVENTURE && mainMenu->adventureManager != NULL)
	{
		ts_key = mainMenu->adventureManager->ts_key;
		ts_keyExplode = mainMenu->adventureManager->ts_keyExplode;
		ts_goal = mainMenu->adventureManager->ts_goal;
		ts_goalCrack = mainMenu->adventureManager->ts_goalCrack;
		ts_goalExplode = mainMenu->adventureManager->ts_goalExplode;
	}
	else
	{
		UpdateWorldDependentTileset(0);
	}

	if (mainMenu->gameRunType == MainMenu::GRT_ADVENTURE && mainMenu->adventureManager != NULL)
	{
		originalProgressionModeOn = mainMenu->adventureManager->originalProgressionMode;//false; //testing
	}
	

	if (saveFile != NULL && mainMenu->gameRunType == MainMenu::GRT_ADVENTURE )
	{
		saveFile->adventureFile->GetOriginalProgressionUpgradeField(level->index, originalProgressionPlayerOptionsField);
		saveFile->adventureFile->GetOriginalProgressionLogField(level->index, originalProgressionLogField);
	}


	
	mapNameText.setFont(mainMenu->arial);
	mapNameText.setCharacterSize(24);
	mapNameText.setFillColor(Color::White);
	mapNameText.setOutlineColor(Color::Black);
	mapNameText.setOutlineThickness(3);
	mapNameText.setPosition(20, 20);


	replayText.setFont(mainMenu->arial);
	replayText.setCharacterSize(24);
	replayText.setFillColor(Color(255, 0, 0, 150));
	replayText.setOutlineColor(Color::Black);
	replayText.setOutlineThickness(3);
	replayText.setString("REPLAY");
	//replayText.setOrigin(replayText.getLocalBounds().width / 2 + replayText.getLocalBounds().left, 0);
	replayText.setPosition(1920 - ( replayText.getGlobalBounds().width + 10 ), 1080 - (replayText.getGlobalBounds().height + 20 ));

	SetupPopup();

	RegisterAllEnemies();
	SetupEnemyTypes();
	SetupSoundLists();

	//old pause menu init

	if (!ShouldContinueLoading())
	{
		cout << "cleanup 0" << endl;
		Cleanup();
		return false;
	}

	//return true;

	//inputVis = new InputVisualizer;

	SetupPokeTriangleScreenGroup();



	const ConfigData &cd = mainMenu->config->GetData();

	if (soundNodeList != NULL)
	{
		soundNodeList->SetSoundVolume(cd.soundVolume);
	}
	if (pauseSoundNodeList != NULL )
	{
		pauseSoundNodeList->SetSoundVolume(cd.soundVolume);
	}

	//SetupScoreDisplay();

	SetupShaders();

	if (!ShouldContinueLoading())
	{
		cout << "cleanup blabhagbo4a" << endl;
		Cleanup();
		return false;
	}

	SetupQuadTrees();

	//cout << "weird timing 1" << endl;

	AllocateEffects();

	if (!ShouldContinueLoading())
	{
		cout << "cleanup A" << endl;
		Cleanup();

		return false;
	}

	//cout << "weird timing 2" << endl;

	//blah 2


	//these sounds are probably just unused
	/*gameSoundInfos[S_KEY_COMPLETE_W1] = GetSound("key_complete_w1");
	gameSoundInfos[S_KEY_COMPLETE_W2] = GetSound("key_complete_w2");
	gameSoundInfos[S_KEY_COMPLETE_W3] = GetSound("key_complete_w2");
	gameSoundInfos[S_KEY_COMPLETE_W4] = GetSound("key_complete_w2");
	gameSoundInfos[S_KEY_COMPLETE_W5] = GetSound("key_complete_w2");
	gameSoundInfos[S_KEY_COMPLETE_W6] = GetSound("key_complete_w6");
	gameSoundInfos[S_KEY_ENTER_0] = GetSound("key_enter_1");
	gameSoundInfos[S_KEY_ENTER_1] = GetSound("key_enter_1");
	gameSoundInfos[S_KEY_ENTER_2] = GetSound("key_enter_2");
	gameSoundInfos[S_KEY_ENTER_3] = GetSound("key_enter_3");
	gameSoundInfos[S_KEY_ENTER_4] = GetSound("key_enter_4");
	gameSoundInfos[S_KEY_ENTER_5] = GetSound("key_enter_5");
	gameSoundInfos[S_KEY_ENTER_6] = GetSound("key_enter_6");*/

	//blah 3

	cutPlayerInput = false;
	activeEnvPlants = NULL;
	totalGameFrames = 0;
	totalGameFramesIncludingRespawns = 0;
	totalFramesBeforeGoal = -1;
	originalZone = NULL;

	unlockedGateList = NULL;

	debugScreenRecorder = NULL; //debugScreenRecorder = new ScreenRecorder("BACKWARDS_DASH_JUMP");

	activeSequence = NULL;

	//view = View( Vector2f( 300, 300 ), sf::Vector2f( 960 * 2, 540 * 2 ) );

	//repGhost = new ReplayGhost( player );

	//cout << "weird timing 3" << endl;
	if (!ShouldContinueLoading())
	{
		cout << "cleanup B" << endl;
		Cleanup();

		return false;
	}
	//cout << "weird timing 4" << endl;

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

	matSet.clear();

	SetupControlProfiles();

	SetupScoreDisplay();

	myHash = md5file(filePathStr); //need the hash before you set up leaderboards

	if (!IsParallelSession())
	{
		if (mainMenu->gameRunType == MainMenu::GRT_ADVENTURE && mainMenu->adventureManager != NULL)
		{
			if (IsParallelSession())
			{
				//shardMenu = mainMenu->adventureManager->pauseMenu->shardMenu; //dont need the shard menu but do I need the log menu?
				logMenu = mainMenu->adventureManager->pauseMenu->logMenu;
			}
			else
			{
				mainMenu->adventureManager->SetBoards(this);

				pauseMenu = mainMenu->adventureManager->pauseMenu;
				pauseMenu->SetGame(this);


				shardMenu = pauseMenu->shardMenu;
				logMenu = pauseMenu->logMenu;
			}
		}
		else
		{
			if (!IsParallelSession())
			{
				pauseMenu = new PauseMenu(this);
				pauseMenu->SetGame(this);

				shardMenu = pauseMenu->shardMenu;
				logMenu = pauseMenu->logMenu;
			}
			else
			{
				//this should be cleaned up because the parallel session doesn't need either of these. not relevant right now outside of freeplay though which doesn't have parallel play
				assert(0);
				pauseMenu = new PauseMenu(this);
				pauseMenu->SetGame(this);

				shardMenu = pauseMenu->shardMenu;
				logMenu = pauseMenu->logMenu;
			}
		}
	}
	else
	{
		//dont set game to this, but I still need to access the log menu
		pauseMenu = mainMenu->adventureManager->pauseMenu;
		logMenu = pauseMenu->logMenu;
	}
	

	ReadFile();

	//myHash = md5file(filePathStr);

	mapNameText.setString(mapHeader->fullName);

	/*if (mainMenu->gameRunType == MainMenu::GRT_ADVENTURE && mainMenu->adventureManager != NULL)
	{
		mainMenu->adventureManager->SetBoards(this);

		pauseMenu = mainMenu->adventureManager->pauseMenu;
		pauseMenu->SetGame(this);

		shardMenu = pauseMenu->shardMenu;
		logMenu = pauseMenu->logMenu;
	}*/

	SetupBackground(); //new location above SetupGameMode for parallel backgrounds

	SetupAbsorbParticles();

	SetupGameMode();
	gameMode->Setup();

	/*if (gameModeType == MatchParams::GAME_MODE_FIGHT
		|| gameModeType == MatchParams::GAME_MODE_RACE)
	{
		matchParams.numPlayers = 2;
		cout << "setting numplayers to 2 for testing only" << endl;
	}*/

	SetupPlayers();


	

	

	//original location
	/*{
		Actor *p = NULL;
		for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			p = GetPlayer(i);
			if (p != NULL)
			{
				p->UpdateNumFuturePositions();
			}
		}
	}*/

	
	

	//for (int i = 1; i < mapHeader->GetNumPlayers(); ++i)
	//{
	//	players[i] = new Actor(this, NULL, i);
	//}
	//m_numActivePlayers = mapHeader->GetNumPlayers(); //not really used

	

	//create bullet quads after game mode because game mode might make new enemies with bullets
	CreateBulletQuads();

	

	SetupHUD();

	bool blackBorder[2];
	bool topBorderOn = false;
	SetupGlobalBorderQuads(blackBorder, topBorderOn);
	if (hud != NULL && hud->mini != NULL && !IsParallelSession())
	{
		hud->mini->SetupBorderQuads(blackBorder, topBorderOn, mapHeader);
		kinMapSpawnIcon.setTexture(*hud->mini->ts_miniIcons->texture);
		kinMapSpawnIcon.setTextureRect(hud->mini->ts_miniIcons->GetSubRect(1));
		kinMapSpawnIcon.setOrigin(kinMapSpawnIcon.getLocalBounds().width / 2,
			kinMapSpawnIcon.getLocalBounds().height / 2);
	}
	if (pauseMenu != NULL)
	{
		pauseMenu->SetupMapBorderQuads(blackBorder, topBorderOn, mapHeader);
	}


	


	if (topBorderOn)
	{
		topClouds = new TopClouds;
		topClouds->SetToHeader();
	}

	

	CreateZones();

	SetupGateMarkers();

	SetupZones();

	

	//SetupBackground(); //old location
	

	//still too far


	//cout << "done opening file" << endl;

	//SetupPlayers();

	
	SetupTimeBubbles();


	SetPlayersGameMode();

	SetupDeathSequence();

	if (deathSeq != NULL && parentGame == NULL)
	{
		deathSeq->SetIDAndAddToAllSequencesVec();
	}
	/*for (auto it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		(*it)->Init();
	}*/

	
	//too far
	

	SetupPlayerRecordingManager();

	//playerRecordingManager = new PlayerRecordingManager(m_numActivePlayers);
	//GhostHeader *gh = new GhostHeader;
	if (bestReplayOn || bestTimeGhostOn )
	{
		SetupMyBestPlayerReplayManager();
	}
	//SetupPlayerReplayerManagers();

	SetupGoalPulse();

	if (hasGoal)
	{
		if (!IsParallelSession())
		{
			SetupGoalFlow();
		}
	}
	else
	{
		//CleanupGoalPulse();
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
	for( auto it = allEnemiesVec.begin(); it != allEnemiesVec.end(); ++it )
	{
		(*it)->Setup();
		(*it)->SetExtraIDsAndAddToVectors();
	}

	//need this after the whole level is setup in case of warp barriers
	//create sequences for the barriers after all enemies have already been loaded
	SetupBarrierScenes();

	cout << "done loading" << endl;

	//make sure this is after all bonuses have been loaded
	{
		Actor *p = NULL;
		for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			p = GetPlayer(i);
			if (p != NULL)
			{
				p->UpdateNumFuturePositions();
			}
		}
	}


	oneFrameMode = false;
	skipped = false;
	quit = false;
	returnVal = GR_EXITLEVEL;
	firstUpdateHasHappened = false;
	gameState = RUN;
	SeedRand(matchParams.randSeed);

	oldCamAngle = 0;
	oldShaderZoom = -1;
	goalDestroyed = false;
	const ConfigData &configData = mainMenu->config->GetData();
	frameRateDisplay.showFrameRate = configData.showFPS;
	runningTimerDisplay.showRunningTimer = configData.showRunningTimer;
	goalDestroyed = false;

	if (saveFile == NULL)
	{
		for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			if (GetPlayer(i) != NULL)
			{
				SetPlayerOptionField(i);
			}
		}
	}

	/*assert(netplayManager != NULL);
	cout << "test queue 2" << "\n";
	for (auto it = netplayManager->ggpoMessageQueue.begin(); it != netplayManager->ggpoMessageQueue.end(); ++it)
	{
		cout << (*it) << "\n";
	}*/

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
		m_numActivePlayers = matchParams.numPlayers; //m_numActivePlayers depreciated I think
		//if (gameModeType != MatchParams::GAME_MODE_PARALLEL_RACE)
		if( !IsParallelGameModeType())
		{
			for (int i = 1; i < matchParams.numPlayers; ++i)//mapHeader->GetNumPlayerPositions(); ++i)
			{
				players[i] = new Actor(this, NULL, i);
				//if( players[i] != NULL )
				//	players[i]->Respawn(); //need a special bonus respawn later
			}
		}
	}

	Actor *p;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->position = V2d(playerOrigPos[i]);

			p->InitEmitters();
		}
	}
}

void GameSession::SetupPopup()
{
	if (parentGame != NULL)
	{
		gamePopup = parentGame->gamePopup;
	}
	else if( gamePopup == NULL )
	{
		gamePopup = new GamePopup;
		currPopupType = -1;
		gameStatePrePopup = -1;
	}
}

void GameSession::CleanupPopup()
{
	if (parentGame == NULL && gamePopup != NULL )
	{
		delete gamePopup;
		gamePopup = NULL;
		currPopupType = -1;
		gameStatePrePopup = -1;
	}
}

void GameSession::UpdatePopup()
{
	int res = gamePopup->Update(controllerStates[0]);

	if (gamePopup->numOptions == 1)
	{
		if (res != GamePopup::OPTION_NOTHING)
		{
			gameState = (Session::GameState)gameStatePrePopup;
		}
	}

	switch (currPopupType)
	{
	case POPUPTYPE_NO_REPLAY_FOUND:
	{
		if (res == GamePopup::OPTION_YES)
		{
			
		}
	}
	}
}

void GameSession::OpenPopup(int popType)
{
	gameStatePrePopup = gameState;
	gameState = POPUP;
	currPopupType = popType;
	switch (currPopupType)
	{
	case POPUPTYPE_NO_REPLAY_FOUND:
	{
		gamePopup->SetInfo("Error: Replay not found", 1);
		break;
	}
	case POPUPTYPE_NO_GHOST_FOUND:
	{
		gamePopup->SetInfo("Error: Ghost not found", 1);
		break;
	}
	}
}

void GameSession::SetupShaders()
{
	//since these are not pointers, cannot transfer them from the parentGame. Might want to change that?

	if (shadersLoaded)
		return;

	shadersLoaded = true;

	if (!timeSlowShader.loadFromFile("Resources/Shader/clone.frag", sf::Shader::Fragment))
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

	if (IsParallelSession())
	{
		//mapHeader->envWorldType = background->envWorld;
		return;
	}
	
	CleanupBackground();

	//for when you have the same BG as a parent and don't want to 
	//delete the tilesets on deletion
	GameSession *currGame = parentGame;
	bool bgAlreadyInUse = false;
	while (currGame != NULL)
	{
		if (currGame->background->name == mapHeader->envName)
		{
			bgAlreadyInUse = true;
			break;
		}
		currGame = currGame->parentGame;
	}

	if (bgAlreadyInUse)
	{
		background = currGame->background;
		ownsBG = false;
	}
	else
	{
		background = Background::SetupFullBG(mapHeader->envName);
		ownsBG = true;
	}

	mapHeader->envWorldType = background->envWorld;
	UpdateWorldDependentTileset(mapHeader->envWorldType);
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

void GameSession::SetupPlayerRecordingManager()
{
	if (IsParallelSession())
		return;


	if (parentGame != NULL)
	{
		playerRecordingManager = parentGame->playerRecordingManager;
	}
	else if (( gameModeType == MatchParams::GAME_MODE_BASIC || gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE )
		&& playerRecordingManager == NULL)//mapHeader->gameMode == MapHeader::MapType::T_BASIC && recGhost == NULL)
	{
		if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE)
		{
			playerRecordingManager = new PlayerRecordingManager(1);
		}
		else
		{
			playerRecordingManager = new PlayerRecordingManager(matchParams.numPlayers);
		}
		
	}
}

bool GameSession::SetupControlProfiles()
{
	if (parentGame != NULL)
		return true;

	


	//ControlProfile *currProfile;
	//SaveFile *currFile = saveFile;//mainMenu->GetCurrSaveFile();
	//if (currFile != NULL)
	//{
	//	bool set = false;
	//	if (mainMenu->adventureManager != NULL)
	//	{
	//		set = pauseMenu->controlSettingsMenu->pSel->SetCurrProfileByName(mainMenu->adventureManager->currProfile);
	//	}
	//	if (!set)
	//	{
	//		//error. profile does not exist
	//		currFile->controlProfileName = "Default";
	//		currFile->Save();
	//	}
	//}
	//for (int i = 0; i < 1; ++i)
	//{
	//	//temporary
	//	//mainMenu->GetController(i).SetFilter( pauseMenu->cOptions->xboxInputAssoc[0] );
	//	currProfile = pauseMenu->GetCurrSelectedProfile();


	//	//controllerStates[i]->con->SetFilter(currProfile->GetCurrFilter());



	//	//GameController *con = GetController(i);
	//	//currProfile->tempCType = con->GetCType();
	//	//con->SetFilter(currProfile->GetCurrFilter());//mainMenu->cpm->profiles.front()->filter );
	//}

	return true;
}

void GameSession::SetupGhosts(std::list<GhostEntry*> &ghostEntries)
{
	/*for( auto it = ghostEntries.begin(); it != ghostEntries.end(); ++it )
	{
		boost::filesystem::path &p = (*it)->gPath;
		ReplayGhost *rg = new ReplayGhost(players[0]);
		rg->OpenGhost(p);
		replayGhosts.push_back(rg);

		rg->frame = 0;
	}*/
}

void GameSession::SetBestReplayOn(bool on)
{
	bestReplayOn = on;
	if (!activePlayerReplayManagers.empty())
	{
		activePlayerReplayManagers[0]->replaysActive = on;
	}

	/*if (myBestReplayManager != NULL)
	{
		myBestReplayManager->replaysActive = on;
	}*/
}

void GameSession::SetBestGhostOn(bool on)
{
	bestTimeGhostOn = on;
	if (!activePlayerReplayManagers.empty())
	{
		activePlayerReplayManagers[0]->ghostsActive = on;
	}
	/*if (playerReplayManager != NULL)
	{
		playerReplayManager->ghostsActive = on;
	}*/
}

void GameSession::CleanupReplaysAndGhosts()
{
	ClearReplayGhosts();

	CleanupMyBestPlayerReplayManager();

	activePlayerReplayManagers.clear();
}

bool GameSession::SetupMyBestPlayerReplayManager()
{
	CleanupMyBestPlayerReplayManager();

	string replayPath = GetBestReplayPath();
	if (saveFile == NULL)
		return false;

	if (saveFile->GetBestFramesLevel(level->index) > 0 && boost::filesystem::exists(replayPath))
	{
		myBestReplayManager = new PlayerReplayManager;

		if (myBestReplayManager->LoadFromFile(replayPath))
		{
			activePlayerReplayManagers.push_back(myBestReplayManager);
			myBestReplayManager->ghostsActive = bestTimeGhostOn;
			myBestReplayManager->replaysActive = bestReplayOn;

			//actual frames of run is 1 less than replay count, because its arbitrary which order to count it in
			int replayFramesBeforeGoal = myBestReplayManager->GetReplayer(0)->GetFramesBeforeGoal();
			myBestReplayManager->GetReplayer(0)->SetDisplayName(GetTimeStr(replayFramesBeforeGoal));


			bool usePlayerSkins = false;
			if (mainMenu->adventureManager != NULL && mainMenu->adventureManager->leaderboard->IsUsingPlayerGhostSkins())
			{
				usePlayerSkins = true;
			}

			if (bestTimeGhostOn)
			{
				myBestReplayManager->AddGhostsToVec(replayGhosts, usePlayerSkins);
			}
			return true;
		}
		else
		{
			CleanupMyBestPlayerReplayManager();
		}
	}

	return false;
}


//return false means continue/go again
bool GameSession::RunMainLoopOnce()
{
	if (CONTROLLERS.AltF4())
	{
		quit = true;
		returnVal = GR_EXITGAME;
		mainMenu->musicPlayer->StopCurrentMusic();
		return true;
	}

	//if (netplayManager != NULL && netplayManager->IsPracticeMode())
	//{
	//	if (netplayManager->HasPracticePlayerStartedRace())
	//	{
	//		cout << "quitting because someone is hosting a race\n";
	//		quit = true;
	//		returnVal = GR_EXIT_PRACTICE_TO_RACE;
	//		return true;
	//		//mainMenu->musicPlayer->StopCurrentMusic();
	//	}
	//}

	//if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE)
	//{
	//	ParallelPracticeMode *ppm = (ParallelPracticeMode*)gameMode;
	//	ppm->ClearUpdateFlags();
	//}

	CheckSinglePlayerInputDefaultKeyboard();

	//cout << this << " << run loop once" << endl;
	switchGameState = false;
	double newTime = gameClock.getElapsedTime().asSeconds();
	double frameTime = newTime - currentTime;

	if (frameTime > 0.25)
	{
		/*if (parentGame != NULL)
		{
		cout << "child game" << "\n";
		}
		else
		{
		cout << "yep thats my clock" << "\n";
		}
		cout << "new time: " << newTime << "\n";
		cout << "big frametime: " << frameTime << "\n";*/
		frameTime = 0.25;
	}
	//frameTime = 0.167;//0.25;	
	currentTime = newTime;

	frameRateDisplay.Update(frameTime);
	//UpdateRunningTimerText();

	accumulator += frameTime;

	bool ggpoNetplay = netplayManager != NULL && !netplayManager->IsPracticeMode();// && ggpo != NULL;

	
	
	if (ggpoNetplay)
	{
		/*if (CONTROLLERS.KeyboardButtonHeld(Keyboard::Escape))
		{
			cout << "esc is pressed. ending match." << endl;
			quit = true;
			returnVal = GR_EXITLEVEL;

			netplayManager->DumpDesyncInfo();
			return true;
		}*/

		if (netplayManager->action == NetplayManager::A_DISCONNECT)
		{
			quit = true;
			returnVal = GR_EXITLEVEL;

			//dont dump for now
			//netplayManager->DumpDesyncInfo();
			return true;
		}


		ggpo_idle(ggpo, 5);
		SteamAPI_RunCallbacks();

		if (accumulator >= TIMESTEP && timeSyncFrames > 0)
		{
			//turn these back on later
			//ggpo_idle(ggpo, 5);

			--timeSyncFrames;
			accumulator -= TIMESTEP;
		}
		else
		{
			while (accumulator >= TIMESTEP)
			{
				//ggpo_idle(ggpo, 5);
				//SteamAPI_RunCallbacks();
				GGPORunFrame();
				accumulator -= TIMESTEP;

				/*if (switchGameState)
				{
				break;
				}*/
			}
		}

		sf::Event ev;
		while (window->pollEvent(ev))
		{
		}

		window->clear(Color::Red);
		extraScreenTex->clear(Color::Transparent);
		preScreenTex->clear(Color::Red);
		postProcessTex2->clear(Color::Red);

		DrawGame(preScreenTex); //draw game differently if you are in a diff mode. i dont mind drawing it in frozen mode tho

		if (IsReplayOn())
		{
			/*preScreenTex->setView(uiView);
			preScreenTex->draw(replayText);
			preScreenTex->setView(view);*/
		}

		preScreenTex->display();

		const Texture &preTex0 = preScreenTex->getTexture();
		Sprite preTexSprite(preTex0);
		preTexSprite.setPosition(-960 / 2, -540 / 2);
		preTexSprite.setScale(.5, .5);
		preTexSprite.setTexture(preTex0);

		if (debugScreenRecorder != NULL)
			debugScreenRecorder->Update(preTex0);

		window->draw(preTexSprite);//, &timeSlowShader );

		if (ggpoNetplay && netplayManager->desyncDetected)
		{
			sf::Vector2u windowSize = window->getSize();
			sf::Texture texture;
			texture.create(windowSize.x, windowSize.y);
			texture.update(*window);
			sf::Image screenshot = texture.copyToImage();
			screenshot.saveToFile("Resources/Recordings/Debug/desync.png");

			quit = true;
			returnVal = GR_EXITLEVEL;

			netplayManager->Abort();
		}
	}
	else if (gameState == RUN)
	{
		window->clear(Color::Red);
		preScreenTex->clear(Color::Red);
		extraScreenTex->clear(Color::Transparent);
		postProcessTex2->clear(Color::Red);

		if (ggpoNetplay)
		{
			if (CONTROLLERS.KeyboardButtonHeld(Keyboard::Escape))
			{
				cout << "esc is pressed. ending match." << endl;
				quit = true;
				returnVal = GR_EXITLEVEL;

				netplayManager->DumpDesyncInfo();
				return true;
			}

			if (netplayManager->action == NetplayManager::A_DISCONNECT)
			{
				quit = true;
				returnVal = GR_EXITLEVEL;

				netplayManager->DumpDesyncInfo();
				return true;
			}


			ggpo_idle(ggpo, 5);
			SteamAPI_RunCallbacks();

			if (accumulator >= TIMESTEP && timeSyncFrames > 0)
			{
				//turn these back on later
				//ggpo_idle(ggpo, 5);

				--timeSyncFrames;
				accumulator -= TIMESTEP;
			}
			else
			{
				while (accumulator >= TIMESTEP)
				{
					//ggpo_idle(ggpo, 5);
					//SteamAPI_RunCallbacks();
					GGPORunFrame();
					accumulator -= TIMESTEP;

					if (switchGameState)
					{
						break;
					}
				}
			}
		}
		else
		{
			if (!RunGameModeUpdate())
			{
				return false;
			}
		}

		sf::Event ev;
		while (window->pollEvent(ev))
		{
			if (ev.type == Event::LostFocus)
			{
				if (gameState == RUN)
				{
					//temporarily remove this to test netplay
					/*if (!p0->IsGoalKillAction(p0->action) && !p0->IsExitAction(p0->action))
					{
					gameState = PAUSE;
					}*/
				}

			}
			else if (ev.type == sf::Event::GainedFocus)
			{
				//if( state == PAUSE )
				//	state = RUN;
			}
		}


		DrawGame(preScreenTex);
		
		if (IsReplayOn())
		{
			SetView(uiView);
			preScreenTex->draw(replayText);
			SetView(view);
		}

		/*SetView(uiView);
		preScreenTex->draw(mapNameText);
		SetView(view);*/

		preScreenTex->display();

		const Texture &preTex0 = preScreenTex->getTexture();
		Sprite preTexSprite(preTex0);
		preTexSprite.setPosition(-960 / 2, -540 / 2);
		preTexSprite.setScale(.5, .5);
		preTexSprite.setTexture(preTex0);

		if (debugScreenRecorder != NULL)
			debugScreenRecorder->Update(preTex0);

		window->draw(preTexSprite);//, &timeSlowShader );

		if (ggpoNetplay && netplayManager->desyncDetected)
		{
			sf::Vector2u windowSize = window->getSize();
			sf::Texture texture;
			texture.create(windowSize.x, windowSize.y);
			texture.update(*window);
			sf::Image screenshot = texture.copyToImage();
			screenshot.saveToFile("Resources/Recordings/Debug/desync.png");

			quit = true;
			returnVal = GR_EXITLEVEL;

			netplayManager->Abort();
		}
	}
	else if (gameState == FROZEN)
	{
		window->clear();

		sf::Event ev;
		while (window->pollEvent(ev))
		{
		}

		if (!FrozenGameModeUpdate())
		{
			return false;
		}

		

		preScreenTex->clear(Color::Red);
		extraScreenTex->clear(Color::Transparent);
		postProcessTex2->clear(Color::Red);
		DrawGame(preScreenTex); //draw game differently if you are in a diff mode. i dont mind drawing it in frozen mode tho

		
		Sprite preTexSprite;
		preTexSprite.setTexture(preScreenTex->getTexture());
		preTexSprite.setPosition(-960 / 2, -540 / 2);
		preTexSprite.setScale(.5, .5);
		window->draw(preTexSprite);

		pauseTex->clear(Color::Transparent);

		DrawGameSequence(pauseTex);

		pauseTex->display();
		Sprite pauseMenuSprite;
		pauseMenuSprite.setTexture(pauseTex->getTexture());
		pauseMenuSprite.setPosition(-960 / 2, -540 / 2);
		pauseMenuSprite.setScale(.5, .5);
		window->draw(pauseMenuSprite);
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

		//accumulator += frameTime;
		Sprite preTexSprite;

		View v;
		v.setCenter(0, 0);
		v.setSize(1920 / 2, 1080 / 2);

		if (accumulator >= TIMESTEP)
		{
			window->clear();
			window->setView(v);
			preScreenTex->clear();
			extraScreenTex->clear(Color::Transparent);

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
			SetView(uiView);
			currStorySequence->Draw(preScreenTex);
		}

		SetView(uiView);
		fader->Draw(EffectLayer::IN_FRONT_OF_UI, preScreenTex);
		//swiper->Draw(preScreenTex);

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

		window->clear();
		preScreenTex->clear();
		extraScreenTex->clear(Color::Transparent);

		if (!SequenceGameModeUpdate())
		{
			return false;
		}

		DrawGameSequence(preScreenTex);

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
			pauseMenu->HandleEvent(ev);
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

		//accumulator += frameTime;

		bool instantQuitForBonus = false;
		//cout << "frames: " << (int)(accumulator / TIMESTEP) << endl;
		while (accumulator >= TIMESTEP)
		{
			UpdateControllers();

			ControllerState &curr = GetCurrInput(0);
			ControllerState &prev = GetPrevInput(0);

			RunFrameForParallelPractice();

			SteamAPI_RunCallbacks();

			if (netplayManager != NULL && netplayManager->IsPracticeMode() && !IsParallelSession())
			{
				netplayManager->SendPracticeInitMessageToAllNewPeers();

				SendPracticeStartMessageToAllNewPeers();

				netplayManager->Update();
			}

			//if (pauseMenu->currentTab == PauseMenu::PAUSE)
			//{
			//	if (curr.Y && !prev.Y && !bestTimeGhostOn)
			//	{
			//		if (parentGame != NULL)
			//		{
			//			/*quit = true;
			//			returnVal = GR_BONUS_RESPAWN;
			//			break;*/
			//		}
			//		else
			//		{
			//			//turn this back on eventually!
			//			/*bestTimeGhostOn = true;
			//			bestReplayOn = false;
			//			if (repPlayer != NULL)
			//			{
			//				delete repPlayer;
			//				repPlayer = NULL;
			//			}
			//			SetupBestTimeGhost();
			//			RestartLevel();*/
			//		}
			//	}
			//	else if (curr.X && !prev.X)
			//	{
			//		if (parentGame != NULL)
			//		{
			//			quit = true;
			//			returnVal = GR_BONUS_RESPAWN;
			//			break;
			//		}
			//		else
			//		{
			//			RestartLevel();
			//		}
			//	}
			//	else if (curr.B && curr.PDown() && !prev.PDown())
			//	{
			//		/*if (recPlayer != NULL)
			//		{
			//			ActivatePauseSound(GetSound("pause_off"));
			//			recPlayer->numTotalFrames = recPlayer->frame;
			//			recPlayer->WriteToFile("Resources/Debug/debugreplay" + string(REPLAY_EXT));
			//		}*/
			//	}
			//}

			PauseMenu::UpdateResponse ur = pauseMenu->Update(curr, prev);
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

				if (soundNodeList != NULL)
				{
					soundNodeList->Pause(false);
				}

				//UpdateControllers();
				if (IsReplayHUDOn())
				{
					UpdateControllers();
				}
				
				//pauseMenu->shardMenu->StopMusic();
				break;
			}
			case PauseMenu::R_P_RESPAWN:
			{
				if (parentGame != NULL)
				{
					//parentGame->RestartGame();
					quit = true;
					returnVal = GR_BONUS_RESPAWN;
					break;
				}
				else
				{

					RestartLevel();

					if (IsReplayHUDOn())
					{
						oneFrameMode = false;
						UpdateControllers();
					}

					//moved this stuff into restart level
					/*gameState = GameSession::RUN;
					gameClock.restart();
					currentTime = 0;
					accumulator = TIMESTEP + .1;
					frameRateDisplay.Reset();*/
				}


				//soundNodeList->Pause( false );
				//kill sounds on respawn
				break;
			}
			case PauseMenu::R_P_EXITLEVEL:
			{
				quit = true;
				returnVal = GR_EXITLEVEL;
				mainMenu->musicPlayer->StopCurrentMusic();
				break;
			}
			case PauseMenu::R_P_EXITTITLE:
			{
				quit = true;
				returnVal = GR_EXITTITLE;
				mainMenu->musicPlayer->StopCurrentMusic();
				break;
			}
			case PauseMenu::R_P_EXITGAME:
			{
				quit = true;
				returnVal = GR_EXITGAME;
				mainMenu->musicPlayer->StopCurrentMusic();
				break;
			}

			}

			if (gameState != PAUSE)
			{
				break;
			}

			accumulator -= TIMESTEP;
		}



		if (instantQuitForBonus)
		{
			return false;
		}

		if (gameState != PAUSE)
		{
			/*if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && !IsParallelSession())
			{
				PracticeStateChangeMsg pm;
				pm.state = (int)gameState;
				pm.frame = totalGameFrames;
				netplayManager->SendPracticeStateChangeMessageToAllPeers(pm);
			}*/
			return false;
		}

		pauseTex->clear(Color::Transparent);
		window->clear();
		Sprite preTexSprite;
		preTexSprite.setTexture(preScreenTex->getTexture());
		preTexSprite.setPosition(-960 / 2, -540 / 2);
		preTexSprite.setScale(.5, .5);
		window->draw(preTexSprite);

		
		sf::Vertex pauseMenuBG[4];
		SetRectColor(pauseMenuBG, Color( 0, 0, 0, 180 ));
		SetRectCenter(pauseMenuBG, 1920, 1080, Vector2f(0, 0));
		window->draw(pauseMenuBG, 4, sf::Quads );

		pauseMenu->Draw(pauseTex);

		pauseTex->display();
		Sprite pauseMenuSprite;
		pauseMenuSprite.setTexture(pauseTex->getTexture());
		//bgSprite.setPosition( );
		pauseMenuSprite.setPosition(-960 / 2, -540 / 2);//(1920 - 1820) / 4 - 960 / 2, (1080 - 980) / 4 - 540 / 2);
		pauseMenuSprite.setScale(.5, .5);
		window->draw(pauseMenuSprite);
	}
	else if (gameState == PRACTICE_INVITE)
	{
		window->clear();

		sf::Event ev;
		while (window->pollEvent(ev))
		{
		}

		while (accumulator >= TIMESTEP)
		{
			UpdateControllers();

			RunFrameForParallelPractice();

			ControllerState &curr = GetCurrInputFiltered(0);
			ControllerState &prev = GetPrevInputFiltered(0);

			ParallelPracticeMode *ppm = (ParallelPracticeMode*)gameMode;

			if (ppm->IsInviteDisplayReadyToRun())
			{
				cout << "quitting because invite display is ready to run\n";
				quit = true;
				returnVal = GR_EXIT_PRACTICE_TO_RACE;
				return true;
			}
			
			bool result = ppm->UpdateInviteDisplay(curr, prev);

			if (!result)
			{
				gameState = GameSession::RUN;

				if (soundNodeList != NULL)
				{
					soundNodeList->Pause(false);
				}
			}

			//RunFrameForParallelPractice(); //i guess I had double this here before? should only be one time obviously. shouldn't change anything though.

			SteamAPI_RunCallbacks();

			if (netplayManager != NULL && netplayManager->IsPracticeMode() && !IsParallelSession())
			{
				netplayManager->SendPracticeInitMessageToAllNewPeers();

				SendPracticeStartMessageToAllNewPeers();

				netplayManager->Update();
			}

			if (gameState != PRACTICE_INVITE)
			{
				break;
			}

			accumulator -= TIMESTEP;
		}


		if (gameState != PRACTICE_INVITE)
		{
			return false;
		}

		preScreenTex->clear(Color::Red);
		extraScreenTex->clear(Color::Transparent);
		postProcessTex2->clear(Color::Red);
		DrawGame(preScreenTex); //draw game differently if you are in a diff mode. i dont mind drawing it in frozen mode tho

		//just noticed that I wasn't displaying the texture so I'm doing that now..not sure what it'll affect
		preScreenTex->display();

		Sprite preTexSprite;
		preTexSprite.setTexture(preScreenTex->getTexture());
		preTexSprite.setPosition(-960 / 2, -540 / 2);
		preTexSprite.setScale(.5, .5);
		window->draw(preTexSprite);
	}
	else if (gameState == FEEDBACK_FORM )
	{
		window->clear();

		assert(mainMenu->adventureManager != NULL);

		FeedbackForm *feedbackForm = mainMenu->adventureManager->feedbackForm;

		sf::Event ev;
		while (window->pollEvent(ev))
		{
			feedbackForm->HandleEvent(ev);
		}

		while (accumulator >= TIMESTEP)
		{
			MOUSE.Update(MOUSE.GetRealPixelPos());

			UpdateControllers();

			RunFrameForParallelPractice();

			ControllerState &curr = GetCurrInputFiltered(0);
			ControllerState &prev = GetPrevInputFiltered(0);

			feedbackForm->Update();

			if (feedbackForm->action == FeedbackForm::A_CANCEL || feedbackForm->action == FeedbackForm::A_CONFIRM)
			{
				bool confirmed = feedbackForm->action == FeedbackForm::A_CONFIRM;

				MOUSE.Hide();

				gameState = GameSession::RUN;

				if (soundNodeList != NULL)
				{
					soundNodeList->Pause(false);
				}
			}

			//RunFrameForParallelPractice();

			SteamAPI_RunCallbacks();

			if (netplayManager != NULL && netplayManager->IsPracticeMode() && !IsParallelSession())
			{
				netplayManager->SendPracticeInitMessageToAllNewPeers();

				SendPracticeStartMessageToAllNewPeers();

				netplayManager->Update();
			}

			if (gameState != FEEDBACK_FORM)
			{
				break;
			}

			accumulator -= TIMESTEP;
		}


		if (gameState != FEEDBACK_FORM)
		{
			return false;
		}

		preScreenTex->clear(Color::Red);
		extraScreenTex->clear(Color::Transparent);
		postProcessTex2->clear(Color::Red);
		DrawGame(preScreenTex); //draw game differently if you are in a diff mode. i dont mind drawing it in frozen mode tho

		preScreenTex->setView(uiView);
		feedbackForm->Draw(preScreenTex);

		preScreenTex->display();

		Sprite preTexSprite;
		preTexSprite.setTexture(preScreenTex->getTexture());
		preTexSprite.setPosition(-960 / 2, -540 / 2);
		preTexSprite.setScale(.5, .5);
		window->draw(preTexSprite);
	}
	else if (gameState == POPUP)
	{
		//clear window here or under the update? not sure yet
		window->clear();
		pauseTex->clear(Color::Transparent);

		sf::Event ev;
		while (window->pollEvent(ev))
		{
		}

		if (!PopupGameModeUpdate())
		{
			return false;
		}

		//could clear the window here!
		//window->clear();

		
		Sprite preTexSprite;
		preTexSprite.setTexture(preScreenTex->getTexture());
		preTexSprite.setPosition(-960 / 2, -540 / 2);
		preTexSprite.setScale(.5, .5);
		window->draw(preTexSprite);

		gamePopup->Draw(pauseTex);

		pauseTex->display();
		Sprite pauseMenuSprite;
		pauseMenuSprite.setTexture(pauseTex->getTexture());
		pauseMenuSprite.setPosition((1920 - 1820) / 4 - 960 / 2, (1080 - 980) / 4 - 540 / 2);
		pauseMenuSprite.setScale(.5, .5);
		window->draw(pauseMenuSprite);
	}
	else if (gameState == LEADERBOARD)
	{
		//clear window here or under the update? not sure yet
		window->clear();
		pauseTex->clear(Color::Transparent);

		sf::Event ev;
		while (window->pollEvent(ev))
		{
			if (mainMenu->adventureManager != NULL)
			{
				mainMenu->adventureManager->leaderboard->HandleEvent(ev);
			}
		}

		if (!LeaderboardGameModeUpdate())
		{
			return false;
		}

		//could clear the window here!
		//window->clear();


		Sprite preTexSprite;
		preTexSprite.setTexture(preScreenTex->getTexture());
		preTexSprite.setPosition(-960 / 2, -540 / 2);
		preTexSprite.setScale(.5, .5);
		window->draw(preTexSprite);


		DrawLeaderboard(pauseTex);

		pauseTex->display();
		Sprite pauseMenuSprite;
		pauseMenuSprite.setTexture(pauseTex->getTexture());
		pauseMenuSprite.setPosition(-960 / 2, -540 / 2);//(1920 - 1820) / 4 - 960 / 2, (1080 - 980) / 4 - 540 / 2);
		pauseMenuSprite.setScale(.5, .5);
		window->draw(pauseMenuSprite);
	}

	if (!IsParallelSession())
	{
		//if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE)
		//{
		//	ParallelMode *pm = (ParallelMode*)gameMode;

		//	for (int i = 0; i < pm->MAX_PARALLEL_SESSIONS; ++i)
		//	{
		//		if (pm->parallelGames[i] != NULL)
		//		{
		//			if (netplayManager->practicePlayers[i].HasNextInput())
		//			{
		//				pm->parallelGames[i]->OnlineRunGameModeUpdate();//RunMainLoopOnce();
		//			}
		//		}
		//	}
		//	//pm->RunParallelGameModeUpdates();
		//	//pm->RunParallelMainLoopsOnce();
		//}

		window->display();
	}
	

	return true;
}

int GameSession::Run()
{
	/*oneFrameMode = false;
	skipped = false;
	quit = false;
	returnVal = GR_EXITLEVEL;
	firstUpdateHasHappened = false;
	gameState = RUN;
	SeedRand(matchParams.randSeed);

	oldShaderZoom = -1;
	goalDestroyed = false;
	frameRateDisplay.showFrameRate = true;
	runningTimerDisplay.showRunningTimer = true;
	goalDestroyed = false;

	if (saveFile == NULL)
	{
		for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			if (GetPlayer(i) != NULL)
			{
				SetPlayerOptionField(i);
			}
		}
	}*/

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		matchPlacings[i] = -1;
	}


	View oldPreTexView = preScreenTex->getView();
	View oldWindowView = window->getView();

	SetView(view);

	Actor *p0 = GetPlayer(0);
	Actor *p = NULL;

	View v;
	v.setCenter(0, 0);
	v.setSize(1920 / 2, 1080 / 2);
	window->setView(v);

	//might move replay stuff later
	cout << "loop about to start" << endl;

	if (playerRecordingManager != NULL && parentGame == NULL )
	{
		playerRecordingManager->StartRecording();
	}

	SetOriginalMusic();

	std::stringstream ss;
	switchGameState = false;

	if (GetPlayer(0)->action == Actor::INTROBOOST)
	{
		//Fade(true, 60, Color::Black, true);
	}
	
	/*if (preLevelScene != NULL)
	{
		preLevelScene->Reset();
		SetActiveSequence(preLevelScene);
	}
	else if( shipEnterScene != NULL )
	{
		shipEnterScene->Reset();
		SetActiveSequence(shipEnterScene);
	}*/


	if (netplayManager != NULL && netplayManager->action == NetplayManager::A_PRACTICE_SETUP_ERROR)
	{
		cout << "turning off parallel practice due to connection error" << "\n";
		matchParams.netplayManager = NULL;
		netplayManager = NULL;
		matchParams.gameModeType = MatchParams::GAME_MODE_BASIC;
		gameModeType = matchParams.gameModeType;
		matchParams.numPlayers = 1;
		m_numActivePlayers = 1;
		SetupGameMode();
	}

	currUpgradeField.Reset();
	currLogField.Reset();
	if( saveFile != NULL && !originalProgressionModeOn )
	{
		currLogField.Set(saveFile->logField);
		currUpgradeField.Set(saveFile->upgradeField);
	}


	RestartLevel();

	if (IsParallelGameModeType())
	{
		ParallelMode *pm = (ParallelMode*)gameMode;
		pm->RestartParallelSessions();
		//trying to restart parallel races
		cout << "restarting parallel sessions" << "\n";
	}

	

	bool ggpoNetplay = netplayManager != NULL && !netplayManager->IsPracticeMode();

	if (ggpoNetplay) //testing!
	{
		fader->Fade(true, 60, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);
	}
	else
	{
		fader->Fade(true, 30, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);
	}

	if (parentGame != NULL && parentGame->bonusHandler != NULL)
	{
		parentGame->bonusHandler->InitBonus();
	}

	if (mainMenu->adventureManager != NULL)
	{
		if (mainMenu->adventureManager->leaderboard->IsTryingToStartReplay())
		{
			PlayerReplayManager *prm = mainMenu->adventureManager->leaderboard->replayChosen;
			//TryStartLeaderboardReplay(mainMenu->adventureManager->leaderboard->replayChosen);
			CleanupReplaysAndGhosts();

			bestTimeGhostOn = true; //for desyncs
			bestReplayOn = true;

			activePlayerReplayManagers.push_back(prm);
			prm->replayHUD->SetSession();

			prm->SetPlayers();

			prm->AddGhostsToVec(replayGhosts, mainMenu->adventureManager->leaderboard->IsUsingPlayerGhostSkins());
			prm->replaysActive = bestReplayOn;
			prm->ghostsActive = bestTimeGhostOn;

			prm->Reset();

			bool res = AddGhostsForReplay(prm);

			mainMenu->adventureManager->leaderboard->Hide();
		}
		else if (mainMenu->adventureManager->leaderboard->IsTryingToRaceGhosts())
		{
			CleanupReplaysAndGhosts();

			AddGhosts();

			mainMenu->adventureManager->leaderboard->Hide();
		}
	}

	quit = false;

	while (!quit)
	{
		double oldAccumulator = accumulator;
		while (!RunMainLoopOnce()) {}
	}


	if (parentGame == NULL)
	{
		if (false)
		{
			//if (recGhost != NULL)
			//{
			//	recGhost->StopRecording();

			//	//string fName = fileName
			//	//1. get folder. if it doesn't exist, make it.
			//	//2. include map name in ghost name
			//	//3. tag ghost with timestamp
			//	//4. 

			//	time_t t = time(0);
			//	struct tm now;
			//	localtime_s(&now, &t);





			//	stringstream fss;
			//	string mName = filePath.filename().stem().string();
			//	fss << "Resources/Recordings/Ghost/" << mName << "/auto/" << mName << "_ghost_"
			//		<< now.tm_year << "_" << now.tm_mon << "_" << now.tm_mday << "_" << now.tm_hour << "_"
			//		<< now.tm_min << "_" << now.tm_sec << GHOST_EXT;

			//	//recGhost->WriteToFile(fss.str());
			//}
		}
	}

	if (soundNodeList != NULL)
	{
		soundNodeList->Reset();
	}

	if (pauseSoundNodeList != NULL)
	{
		pauseSoundNodeList->Reset();
	}
	
	
	if (parentGame == NULL)
	{
		for (int i = 0; i < 4; ++i)
		{
			//fix this soon
			//if(GetController(i) != NULL )
			//	SetFilterDefault(GetController(i)->filter);
		}
	}

	if (parentGame != NULL)
	{
		pauseMenu->game = parentGame;
	}
	else
	{
		pauseMenu->game = NULL;
	}

	if (ggpoNetplay)
	{
		cout << "cleaning up ggpo" << endl;
		ggpo_close_session(ggpo);
	}

	//also cleans up savestate in other cases
	CleanupGGPO();

	//fader->Clear();

	SetView(oldPreTexView);
	window->setView(oldWindowView);
	

	return returnVal;
}



void GameSession::Init()
{
	nexus = NULL;

	resType = GR_BONUS_RETURN; //better than being unset, just makes it 0

	continueLoading = true;
	bestTimeGhostOn = false;
	bestReplayOn = false;

	boostEntrance = false;

	activateBonus = false;
	bonusType = BONUSTYPE_NONE;

	currPopupType = -1;
	gameStatePrePopup = -1;
	gamePopup = NULL;
	bonusGame = NULL;
	bonusHandler = NULL;
	gateMarkers = NULL;
	inversePolygon = NULL;

	postLevelScene = NULL;
	level = NULL;
	inputVis = NULL;
	
	pauseMenu = NULL;
	topClouds = NULL;
	specterTree = NULL;
	envPlantTree = NULL;
	itemTree = NULL;
	gateTree = NULL;
	enemyTree = NULL;
	staticItemTree = NULL;
	terrainBGTree = NULL;
	activeEnemyItemTree = NULL;
	airTriggerTree = NULL;
	inverseEdgeTree = NULL;
	scoreDisplay = NULL;
	va = NULL;
	activeEnemyList = NULL;
	activeEnemyListTail = NULL;
	myBestReplayManager = NULL;
	playerRecordingManager = NULL;

	explodingGravityGrass = NULL;
	polyQueryList = NULL;
	specialPieceList = NULL;
	flyTerrainList = NULL;
	absorbParticles = NULL;
	absorbDarkParticles = NULL;
	for (int i = 0; i < 4; ++i)
	{
		players[i] = NULL;
	}

	shadersLoaded = false;

	hasGoal = false;
	boostIntro = false;
	nextFrameRestartGame = false;
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
	//ReadDecorImagesFile();
}

void GameSession::SetStorySeq(StorySequence *storySeq)
{
	storySeq->Reset();
	currStorySequence = storySeq;
	gameState = GameSession::STORY;
}

//currently not used at all
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
	timeSlowShader.setUniform("totalBubbles", p0->MAX_BUBBLES * m_numActivePlayers);
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
	for (int i = 0; i < numGates; ++i)
	{
		if (gates[i]->category == gCat)
		{
			gates[i]->Soften();
		}
	}
}

void GameSession::ReformGates(int gCat)
{
	for (int i = 0; i < numGates; ++i)
	{
		if (gates[i]->category == gCat)
		{
			gates[i]->Reform();
		}
	}
}

void GameSession::CloseGates(int gCat)
{
	for (int i = 0; i < numGates; ++i)
	{
		if (gates[i]->category == gCat)
		{
			gates[i]->Close();
		}
	}
}

void GameSession::OpenGates(int gCat)
{
	for (int i = 0; i < numGates; ++i)
	{
		if (gates[i]->category == gCat)
		{
			gates[i]->Open();
		}
	}
}


void GameSession::UpdateEnvShaders()
{
	if (IsParallelSession())
	{
		return;
	}

	//sometimes gets called like 8 times in a frame wtf?
	//Vector2f botLeft(view.getCenter().x - view.getSize().x / 2,
	//	view.getCenter().y + view.getSize().y / 2);
	Vector2f vSize = view.getSize();
	Vector2f botLeft(view.getCenter().x - vSize.x / 2, view.getCenter().y + vSize.y / 2);

	//depreciated variable!!!
	Vector2f playertest = (botLeft - oldCamBotLeft) / 5.f;

	float camAngle = (float)(view.getRotation() * PI / 180.0);

	Vector2f botLeftTest(-vSize.x / 2, vSize.y / 2);
	RotateCW(botLeftTest, camAngle);

	botLeftTest += view.getCenter();

	botLeft = botLeftTest;


	UpdatePolyShaders(botLeft, playertest, cam.GetZoom() );

	/*for (auto it = zones.begin(); it != zones.end(); ++it)
	{
		(*it)->Update(cam.GetZoom(), botLeft, playertest);
	}*/
}

void GameSession::DrawRails(sf::RenderTarget *target)
{
	QueryRailDrawTree(screenRect);
	DrawQueriedRails(target);
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
	if (CONTROLLERS.KeyboardButtonHeld(Keyboard::Num1))
	{
		showDebugDraw = true;
	}
	else if (CONTROLLERS.KeyboardButtonHeld(Keyboard::Num2))
	{
		showDebugDraw = false;
	}
	else if (CONTROLLERS.KeyboardButtonHeld(Keyboard::Num3))
	{
		showTerrainDecor = false;
	}
	else if (CONTROLLERS.KeyboardButtonHeld(Keyboard::Num4))
	{
		showTerrainDecor = true;
	}
	else if (CONTROLLERS.KeyboardButtonHeld(Keyboard::Num9))
	{
		runningTimerDisplay.showRunningTimer = true;
	}
	else if (CONTROLLERS.KeyboardButtonHeld(Keyboard::Num0))
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

void GameSession::UpdatePolyShaders( Vector2f &botLeft, Vector2f &playertest, float zoom )
{
	//oldShaderZoom is only used in editor. Need to optimize this in GameSession as well
	bool first = oldShaderZoom < 0;

	//float zoom = cam.GetZoom();

	int numTerrainMats = terrainTypeIndexesUsedVec.size();
	int numWaterMats = waterTypeIndexesUsedVec.size();

	float camAngle = (float)(view.getRotation() * PI / 180.0);

	bool shouldUpdateZoom = first || oldShaderZoom != zoom;
	bool shouldUpdatePos = first || oldShaderBotLeft != botLeft;
	bool shouldUpdateRotation = first || oldCamAngle != camAngle;

	/*if (first || oldCamAngle != camAngle)
	{
		oldCamAngle = camAngle;

		for (int i = 0; i < TerrainPolygon::TOTAL_TERRAIN_TYPES; ++i)
		{
			mainMenu->terrainShaders[i].setUniform("u_cameraAngle", camAngle);
		}
	}*/

	Shader *currTerrainShader = NULL;
	Shader *currWaterShader = NULL;

	if (shouldUpdateZoom)
	{
		oldShaderZoom = zoom;

		for (int i = 0; i < numTerrainMats; ++i)
		{
			currTerrainShader = &mainMenu->terrainShaders[terrainTypeIndexesUsedVec[i]];
			currTerrainShader->setUniform("zoom", zoom);
		}

		for (int i = 0; i < numWaterMats; ++i)
		{
			currWaterShader = &mainMenu->waterShaders[waterTypeIndexesUsedVec[i]];
			currWaterShader->setUniform("zoom", zoom);
		}
	}

	if (shouldUpdatePos)
	{
		oldShaderBotLeft = botLeft;

		for (int i = 0; i < numTerrainMats; ++i)
		{
			currTerrainShader = &mainMenu->terrainShaders[terrainTypeIndexesUsedVec[i]];
			currTerrainShader->setUniform("topLeft", botLeft);
		}

		for (int i = 0; i < numWaterMats; ++i)
		{
			currWaterShader = &mainMenu->waterShaders[waterTypeIndexesUsedVec[i]];
			currWaterShader->setUniform("topLeft", botLeft);
		}
	}

	if (shouldUpdateRotation)
	{
		oldCamAngle = camAngle;

		for (int i = 0; i < numTerrainMats; ++i)
		{
			currTerrainShader = &mainMenu->terrainShaders[terrainTypeIndexesUsedVec[i]];
			currTerrainShader->setUniform("u_cameraAngle", camAngle);
		}

		for (int i = 0; i < numWaterMats; ++i)
		{
			currWaterShader = &mainMenu->waterShaders[waterTypeIndexesUsedVec[i]];
			currWaterShader->setUniform("u_cameraAngle", camAngle);
		}
	}
	//mainMenu->terrainShader.setUniform("playertest", playertest);

	if (background != NULL)
	{
		for (int i = 0; i < numTerrainMats; ++i)
		{
			currTerrainShader = &mainMenu->terrainShaders[terrainTypeIndexesUsedVec[i]];
			currTerrainShader->setUniform("skyColor", ColorGL(background->GetSkyColor()));
		}
	}
	else
	{
		for (int i = 0; i < numTerrainMats; ++i)
		{
			currTerrainShader = &mainMenu->terrainShaders[terrainTypeIndexesUsedVec[i]];
			currTerrainShader->setUniform("skyColor", ColorGL(Color::White));
		}
	}

	for (int i = 0; i < numWaterMats; ++i)
	{
		currWaterShader = &mainMenu->waterShaders[waterTypeIndexesUsedVec[i]];
		currWaterShader->setUniform("u_slide", waterShaderCounter);
	}

	waterShaderCounter += .01f;
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
	//srand(time(0));

	if (pointsTotal > 0)
	{
		int r = GetRand() % (pointsTotal);

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
					cout << "couldn't load music for: " << (*it).first << endl;
					return;
				}
				//musicMap[(*it).first] = originalMusic;
				originalMusic->Load();
				if (originalMusic == mainMenu->musicPlayer->currMusic)
				{

				}
				else
				{
					mainMenu->musicPlayer->PlayMusic(originalMusic);
					//mainMenu->musicPlayer->TransitionMusic(originalMusic, 60);
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

SaveFile *GameSession::GetCurrSaveFile()
{
	return saveFile;
	/*if (mainMenu->gameRunType == MainMenu::GRT_ADVENTURE)
	{
		return mainMenu->GetCurrSaveFile();
	}
	else
	{
		return NULL;
	}*/
}


void GameSession::NextFrameRestartLevel()
{
	if (parentGame != NULL)
	{
		parentGame->nextFrameRestartGame = true;
		quit = true;
		returnVal = GR_BONUS_RESPAWN;
	}
	else
	{
		nextFrameRestartGame = true;
	}
}


void GameSession::RestartGame()
{
	NextFrameRestartLevel(); //for virtual function
}

void GameSession::RestartLevel()
{
	if (saveFile == NULL && !IsParallelSession())
	{
		//currUpgradeField.Reset();
		currUpgradeField.Set(defaultStartingPlayerOptionsField);
		currLogField.Reset();
	}

	if ( saveFile != NULL && originalProgressionModeOn)
	{
		//if orig progression on, set the log field to the orig progression, otherwise, let it stack up.
		currLogField.Set(saveFile->logField);
		currLogField.And(originalProgressionLogField);

		currUpgradeField.Set(saveFile->upgradeField);
		currUpgradeField.And(originalProgressionPlayerOptionsField);
	}

	phaseOn = false;
	skipOneReplayFrame = false;

	turnTimerOnCounter = -1;
	timerOn = true;

	onlinePauseMenuOn = false;
	gameState = GameSession::RUN;
	gameClock.restart();
	currentTime = 0;
	accumulator = TIMESTEP + .1;
	frameRateDisplay.Reset();

	activateBonus = false;

	if( gateMarkers != NULL)
		gateMarkers->Reset();

	if (gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE && !IsParallelSession())
	{
		netplayManager->SetPracticePlayersToNeedStartMessage();
	}

	//OpenGates(Gate::CRAWLER_UNLOCK);

	ClearEmitters();

	originalProgressionCompatible = false;
	if (saveFile != NULL && mainMenu->gameRunType == MainMenu::GRT_ADVENTURE)
	{
		originalProgressionCompatible = true;


		if (!originalProgressionModeOn)
		{
			//if original progression is not being forced, check to see if we are compatible with it.
			for (int i = 0; i < saveFile->upgradeField.numOptions; ++i)
			{
				if (saveFile->upgradeField.GetBit(i) && !originalProgressionPlayerOptionsField.GetBit(i))
				{
					originalProgressionCompatible = false;
					break;
				}
			}

			if (originalProgressionCompatible)
			{
				for (int i = 0; i < saveFile->logField.numOptions; ++i)
				{
					if (saveFile->logField.GetBit(i) && !originalProgressionLogField.GetBit(i))
					{
						originalProgressionCompatible = false;
						break;
					}
				}
			}
		}
	}

	if (originalProgressionCompatible)
	{
		cout << "compatible" << "\n";
	}
	else
	{
		cout << "not compatible \n";
	}

	//cout << "restarting the level on frame: " << totalGameFramesIncludingRespawns << "\n";

	//AddEmitter(testEmit, EffectLayer::IN_FRONT);
	//testEmit->Reset();

	for (auto it = allPolysVec.begin(); it != allPolysVec.end(); ++it)
	{
		(*it)->ResetTouchGrass();
		(*it)->ResetState();
	}

	for (auto it = allRailsVec.begin(); it != allRailsVec.end(); ++it)
	{
		(*it)->ResetState();
	}

	SetDrainOn(true);
	

	nextFrameRestartGame = false;
	//accumulator = TIMESTEP + .1;
	currStorySequence = NULL;
	currSuperPlayer = NULL;

	if( background != NULL )
		background->Reset();
	//keeps lighting the same when going into bonuses with the same bg
	if (parentGame != NULL)
	{
		if (parentGame->background != NULL)
		{
			background->frame = parentGame->background->frame;
		}
	}

	if (parentGame == NULL)
	{
		if (soundNodeList != NULL)
		{
			soundNodeList->Clear();
		}
		
	}

	//DONT RESET totalGameFramesIncludingRespawns
	totalGameFrames = 0;
	totalFramesBeforeGoal = -1;

	if( goalPulse != NULL )
		goalPulse->Reset();
	//f->Reset();


	if (parentGame == NULL)
	{
		fader->Reset();
	}
	
	numKeysCollected = 0;

	if (hud != NULL && !IsParallelSession())
	{
		hud->Reset();
	}
	
	//soundNodeList->Reset(); //already done using Clear
	scoreDisplay->Reset();

	if (playerRecordingManager != NULL && parentGame == NULL )
	{
		playerRecordingManager->RestartRecording();
	}

	//unneeded i think. I do the same thing a few lines down while calling reset
	/*if (parentGame == NULL)
	{
		for (auto it = activePlayerReplayManagers.begin(); it != activePlayerReplayManagers.end(); ++it)
		{
			(*it)->SetToStart();
		}
	}*/

	/*if (playerReplayManager != NULL && parentGame == NULL )
		playerReplayManager->SetToStart();*/

	if (parentGame == NULL)
	{
		for (auto it = replayGhosts.begin(); it != replayGhosts.end(); ++it)
		{
			(*it)->Reset();
		}
	}

	for (auto it = fullAirTriggerList.begin(); it != fullAirTriggerList.end(); ++it)
	{
		(*it)->Reset();
	}


	cutPlayerInput = false;

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		filteredPrevInput[i].Clear();
	}

	if (parentGame == NULL)
	{
		for (auto it = activePlayerReplayManagers.begin(); it != activePlayerReplayManagers.end(); ++it)
		{
			(*it)->Reset();
		}


		Actor *p;

		for (int i = 0; i < 4; ++i)
		{
			Actor *player = GetPlayer(i);
			if (player != NULL)
				player->Respawn();
		}
	}
	/*else
	{
		Actor *p;

		for (int i = 0; i < 4; ++i)
		{
			Actor *player = GetPlayer(i);
			
		}
	}*/

	cam.Reset();

	//cam.Update();

	/*if( playerReplayManager != NULL && parentGame == NULL )
	{
		playerReplayManager->Reset();
	}*/

	//orig pos below player respawn
	/*if (parentGame == NULL)
	{
		for (auto it = activePlayerReplayManagers.begin(); it != activePlayerReplayManagers.end(); ++it)
		{
			(*it)->Reset();
		}
	}*/

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
		ActivateZone(originalZone, true);
		gateMarkers->SetToZone(currentZone);

		SetKeyMarkerToCurrentZone();
	}
	//	originalZone->active = true;
	//
	//later don't relock gates in a level unless there is a "level reset"
	

	inactiveEnemyList = NULL;

	ResetBarriers();

	//cam.SetManual( false );

	activeSequence = NULL;

	

	gameMode->StartGame();
	//later can have a setting for this if needed
	if (preLevelScene != NULL)
	{
		preLevelScene->Reset();
		SetActiveSequence(preLevelScene);
	}
	else
	{
		activeSequence = NULL;
		if (shipEnterScene != NULL)
		{
			shipEnterScene->Reset();
			SetActiveSequence(shipEnterScene);
		}
	}

	pokeTriangleScreenGroup->Reset();
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
	if (IsReplayOn())
	{
		return activePlayerReplayManagers[0]->header.IsShardCaptured(shardType);
	}

	return currUpgradeField.GetBit(shardType + Actor::SHARD_START_INDEX);
}

bool GameSession::PopupGameModeUpdate()
{
	while (accumulator >= TIMESTEP)
	{
		if (!OneFrameModeUpdate())
		{
			break;
		}

		UpdateControllers();

		UpdatePopup();

		SteamAPI_RunCallbacks();

		if (gameState != POPUP)
		{
			break;
		}

		accumulator -= TIMESTEP;
	}

	if (gameState != POPUP)
	{
		return false;
	}

	return true;
}

bool GameSession::LeaderboardGameModeUpdate()
{
	while (accumulator >= TIMESTEP)
	{
		/*if (!OneFrameModeUpdate())
		{
			break;
		}*/

		MOUSE.Update(MOUSE.GetRealPixelPos());
		UpdateControllers();
		UICONTROLLER.Update();
		

		if (mainMenu->adventureManager != NULL)
		{
			Session *sess = Session::GetSession();

			mainMenu->adventureManager->leaderboard->Update(GetPrevInput(0), GetCurrInput(0));

			if (mainMenu->adventureManager->leaderboard->IsHidden())
			{
				gameState = RUN;
			}
			else if (mainMenu->adventureManager->leaderboard->IsTryingToStartReplay())
			{
				TryStartLeaderboardReplay(mainMenu->adventureManager->leaderboard->replayChosen);
				gameState = RUN;
				mainMenu->adventureManager->leaderboard->Hide();
			}
			else if (mainMenu->adventureManager->leaderboard->IsTryingToRaceGhosts())
			{
				TryStartGhosts();
				gameState = RUN;
				mainMenu->adventureManager->leaderboard->Hide();
			}
		}

		SteamAPI_RunCallbacks();

		if (gameState != LEADERBOARD)
		{
			break;
		}

		accumulator -= TIMESTEP;
	}

	if (gameState != LEADERBOARD)
	{
		return false;
	}

	return true;
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
					int r = GetRand() % 2;
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
		r = GetRand() % diffApartMax;
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
		rayCastInfo.rcEdge = NULL;
		rayCastInfo.rayStart = curr->GetPosition( quant );
		rayCastInfo.rayEnd = rayCastInfo.rayStart - cn * (double)maxPen;
		rayIgnoreEdge = curr;

		RayCast( this, qt->startNode, rayCastInfo.rayStart, rayCastInfo.rayEnd );

		if( rayCastInfo.rcEdge != NULL )
		{
			penLimit = length(rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcQuant ) - rayCastInfo.rayStart );
			diffPenMax = (int)penLimit - minDistanceApart;
		}
		if (diffPenMax == 0)
			rPen = 0;
		else
		{
			rPen = GetRand() % diffPenMax;
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

void GameSession::UpdateTerrainStates()
{
	for (auto it = allPolysVec.begin(); it != allPolysVec.end(); ++it)
	{
		(*it)->UpdateState();
	}
}

void GameSession::UpdateRailStates()
{
	for (auto it = allRailsVec.begin(); it != allRailsVec.end(); ++it)
	{
		(*it)->UpdateState();
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

		if( edge != rayIgnoreEdge && (rayCastInfo.rcEdge == NULL || length( edge->GetPosition( edgeQuantity ) - rayCastInfo.rayStart ) <
			length(rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcQuant ) - rayCastInfo.rayStart ) ) )
		{
			rayCastInfo.rcEdge = edge;
			rayCastInfo.rcQuant = edgeQuantity;
		}
		}
		else
		{
			rayCastInfo.rcEdge = edge;
			rayCastInfo.rcQuant = edgeQuantity;
		}
	}
	else if( rayMode == "decor" )
	{
		if( edge == rayIgnoreEdge )
			return; 

		if(rayCastInfo.rcEdge == NULL )
		{
			rayCastInfo.rcEdge = edge;
			rayCastInfo.rcQuant = edgeQuantity;
		}
		else
		{
			V2d rc = rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcQuant );
			if( length(rayCastInfo.rayStart - edge->GetPosition( edgeQuantity ) ) < length(rayCastInfo.rayStart - rc ) )
			{
				rayCastInfo.rcEdge = edge;
				rayCastInfo.rcQuant = edgeQuantity;
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

		int extraChildrenSize = n->extraChildren.size();
		for( int i = 0; i < extraChildrenSize; ++i )
		{
			EnvPlant *ev = (EnvPlant *)(n->extraChildren[i]);
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
	for (int i = 0; i < EffectLayer::EFFECTLAYER_Count; ++i)
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
	if (soundNodeList != NULL)
	{
		soundNodeList->Update();
	}
	
	if (pauseSoundNodeList != NULL)
	{
		pauseSoundNodeList->Update();
	}
}

void GameSession::RecGhostRecordFrame()
{
	if (playerRecordingManager != NULL)
	{
		playerRecordingManager->RecordGhostFrames();
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

bool GameSession::HasLog(int logIndex)
{
	if (IsReplayOn())
	{
		return activePlayerReplayManagers[0]->header.IsLogCaptured(logIndex);
	}

	return currLogField.GetBit(logIndex);
	/*if (originalProgressionModeOn)
	{
		return originalProgressionLogField.GetBit(logIndex);
	}

	if (saveFile != NULL)
	{
		return saveFile->HasLog(logIndex);
	}*/
	return false;
}

int GameSession::GetBonusType()
{
	if (parentGame == NULL)
	{
		return BONUSTYPE_NONE;
	}
	else
	{
		return bonusType;
	}
}

//int GameSession::GetPlayerNormalSkin(int index)
//{
//	if (saveFile != NULL)
//	{
//		return saveFile->defaultSkinIndex;
//	}
//	else if (netplayManager != NULL)
//	{
//		int realIndex = index;
//		if (gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE)
//		{
//			int adjustedIndex = index;
//			if (IsParallelSession())
//			{
//				assert(index == 0); //each parallel session can only have 1 player so far
//
//				adjustedIndex = parallelSessionIndex + 1;
//			}
//
//			realIndex = adjustedIndex;
//			if (adjustedIndex == 0)
//			{
//				realIndex = netplayManager->playerIndex;
//			}
//			else if (adjustedIndex <= netplayManager->playerIndex)
//			{
//				realIndex = adjustedIndex - 1;
//			}
//		}
//
//		return netplayManager->netplayPlayers[realIndex].skinIndex;
//	}
//	else
//	{
//		return matchParams.playerSkins[index];
//	}
//
//	return Actor::SKIN_NORMAL;
//}

MatchResultsScreen *GameSession::CreateResultsScreen()
{
	MatchStats *stats = new MatchStats;

	bool ggpoNetplay = netplayManager != NULL && !netplayManager->IsPracticeMode();

	assert(ggpoNetplay);

	if (netplayManager != NULL)
	{
		stats->netplay = true;
		if (netplayManager->currNetplayType == NetplayManager::NETPLAY_TYPE_QUICKPLAY )//mainMenu->gameRunType == MainMenu::GRT_QUICKPLAY)
		{
			stats->quickplay = true;
		}
	}

	stats->gameModeType = gameModeType;

	Actor *p = NULL;

	int playerParIndex = -1;
	int myPlayerIndex = netplayManager->playerIndex;

	for (int i = 0; i < 4; ++i)
	{
		p = NULL;
		if (matchParams.gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE)
		{
			//int parallelIndex = i - 1;
			if (i == myPlayerIndex)
			{
				playerParIndex = 0;
			}
			else if (i > myPlayerIndex)
			{
				playerParIndex = i;
			}
			else
			{
				playerParIndex = i + 1;
			}

			if (playerParIndex == 0)
			{
				p = GetPlayer(0);
			}
			else
			{
				ParallelRaceMode *prm = (ParallelRaceMode*)gameMode;

				if (prm->parallelGames[playerParIndex-1] != NULL)
				{
					p = prm->parallelGames[playerParIndex-1]->GetPlayer(0);
				}
			}

			/*if (i > 0)
			{
				ParallelRaceMode *prm = (ParallelRaceMode*)gameMode;

				if (prm->parallelGames[parallelIndex] != NULL)
				{
					p = prm->parallelGames[parallelIndex]->GetPlayer(0);
				}
				else
				{
					p = NULL;
				}
			}
			else
			{
				p = GetPlayer(0);
			}

			int adjustedIndex = 0;
			if (i > 0)
			{
				adjustedIndex = parallelIndex + 1;
			}
			
			realIndex = adjustedIndex;
			if (adjustedIndex == 0)
			{
				realIndex = netplayManager->playerIndex;
			}
			else if (adjustedIndex <= netplayManager->playerIndex)
			{
				realIndex = adjustedIndex - 1;
			}*/
		}
		else
		{
			p = GetPlayer(i);
		}
		
		if (p != NULL)
		{
			stats->playerStats[i] = new PlayerStats( p );
			stats->playerStats[i]->name = netplayManager->netplayPlayers[i].name;
			stats->playerStats[i]->skinIndex = netplayManager->netplayPlayers[i].skinIndex;
			stats->playerStats[i]->placing = matchPlacings[i];

			switch (matchParams.gameModeType)
			{
			case MatchParams::GAME_MODE_FIGHT:
			{
				FightMode *fm = (FightMode*)gameMode;
				stats->playerStats[i]->kills = fm->data.killCounter[i];
				break;
			}
			}
		}
	}

	switch (matchParams.gameModeType)
	{
	default:
		return new VictoryScreen4Player( stats );
	}

	return NULL;
}


void GameSession::DrawLeaderboard(sf::RenderTarget *target)
{
	AdventureManager *adventureManager = mainMenu->adventureManager;

	if (adventureManager != NULL)
	{
		adventureManager->leaderboard->Draw(target);
	}
}

void GameSession::StartLeaderboard()
{
	AdventureManager *adventureManager = mainMenu->adventureManager;
	CleanupReplaysAndGhosts();


	if (adventureManager != NULL)
	{
		gameState = LEADERBOARD;
		if (originalProgressionCompatible)
		{
			adventureManager->leaderboard->SetAnyPowersMode(false);
		}
		else
		{
			adventureManager->leaderboard->SetAnyPowersMode(true);
		}

		adventureManager->leaderboard->Start();//adventureManager->GetLeaderboardNameOriginalPowers(this), 
			//adventureManager->GetLeaderboardNameAnyPowers(this));
	}
}

bool GameSession::TryStartMyBestReplay()
{
	CleanupReplaysAndGhosts();

	bool oldGhostOn = bestTimeGhostOn;
	bool oldReplayOn = bestReplayOn;

	bestTimeGhostOn = true; //on to provide debug for the replays
	//bestTimeGhostOn = false;
	bestReplayOn = true;

	if (SetupMyBestPlayerReplayManager())
	{

		bool res = AddGhostsForReplay(myBestReplayManager);

		if (res)
		{
			RestartGame();
			return true;
		}
		else
		{
			OpenPopup(GameSession::POPUPTYPE_NO_REPLAY_FOUND);
			bestTimeGhostOn = oldGhostOn;
			bestTimeGhostOn = oldReplayOn;

			CleanupReplaysAndGhosts();
			return false;
		}
	}
	else
	{
		OpenPopup(GameSession::POPUPTYPE_NO_REPLAY_FOUND);
		bestTimeGhostOn = oldGhostOn;
		bestTimeGhostOn = oldReplayOn;

		CleanupReplaysAndGhosts();
		return false;
	}

	return false;
}

bool GameSession::TryStartLeaderboardReplay(PlayerReplayManager *prm)
{
	assert(prm != NULL);

	CleanupReplaysAndGhosts();

	bestTimeGhostOn = true; //for desyncs
	bestReplayOn = true;

	activePlayerReplayManagers.push_back(prm);
	prm->replayHUD->SetSession();

	prm->SetPlayers();

	prm->AddGhostsToVec(replayGhosts, mainMenu->adventureManager->leaderboard->IsUsingPlayerGhostSkins());
	prm->replaysActive = bestReplayOn;
	prm->ghostsActive = bestTimeGhostOn;

	bool res = AddGhostsForReplay(prm);

	if (!res)
	{
		return false;
	}

	RestartGame();

	return true;
}

int GameSession::GetNumPotentialGhosts()
{
	if (mainMenu->adventureManager != NULL )
	{
		int numGhosts = 0;

		numGhosts += mainMenu->adventureManager->leaderboard->GetNumActiveLeaderboardGhosts();

		if (numGhosts == 0 || mainMenu->adventureManager->leaderboard->IsDefaultGhostOn())
		{
			numGhosts += 1;
		}

		return numGhosts;
	}
	else
	{
		return 1;
	}
}

bool GameSession::AddGhostsForReplay(PlayerReplayManager *prm)
{
	if (mainMenu->adventureManager != NULL && mainMenu->adventureManager->leaderboard->ShouldShowGhostsWithReplay())
	{
		bool useDefaultGhost = true;
		bool useLeaderboardGhosts = true;

		if (mainMenu->gameRunType == MainMenu::GRT_ADVENTURE)
		{
			assert(mainMenu->adventureManager != NULL);

			if (bestTimeGhostOn)
			{
				if (mainMenu->adventureManager->leaderboard->GetNumActiveLeaderboardGhosts() > 0)
				{
					useLeaderboardGhosts = true;
				}

				useDefaultGhost = mainMenu->adventureManager->leaderboard->IsDefaultGhostOn();
			}
		}

		if (useDefaultGhost && prm != myBestReplayManager)
		{
			if (!SetupMyBestPlayerReplayManager())
			{
				OpenPopup(GameSession::POPUPTYPE_NO_GHOST_FOUND);
				bestTimeGhostOn = false;
				bestTimeGhostOn = false;

				CleanupReplaysAndGhosts();

				return false;
			}

			myBestReplayManager->ghostsActive = true;
			myBestReplayManager->replaysActive = false;
		}

		if (useLeaderboardGhosts)
		{
			mainMenu->adventureManager->leaderboard->AddGhostsToVec(replayGhosts, prm);
			mainMenu->adventureManager->leaderboard->AddPlayerReplayManagersToVec(activePlayerReplayManagers, prm);
			mainMenu->adventureManager->leaderboard->SetActive(false, bestTimeGhostOn, prm);
		}
	}

	return true;
}

bool GameSession::AddGhosts()
{
	bool oldGhostOn = bestTimeGhostOn;
	bool oldReplayOn = bestReplayOn;

	bestTimeGhostOn = true;
	bestReplayOn = false;

	bool useDefaultGhost = true;
	bool useLeaderboardGhosts = true;

	if (mainMenu->gameRunType == MainMenu::GRT_ADVENTURE)
	{
		assert(mainMenu->adventureManager != NULL);

		if (bestTimeGhostOn)
		{
			if (mainMenu->adventureManager->leaderboard->GetNumActiveLeaderboardGhosts() > 0)
			{
				useLeaderboardGhosts = true;
			}

			useDefaultGhost = mainMenu->adventureManager->leaderboard->IsDefaultGhostOn();
		}
	}

	if (!useDefaultGhost && !useLeaderboardGhosts)
	{
		useDefaultGhost = true;
	}

	if (useDefaultGhost)
	{
		if (!SetupMyBestPlayerReplayManager())
		{
			OpenPopup(GameSession::POPUPTYPE_NO_GHOST_FOUND);
			bestTimeGhostOn = oldGhostOn;
			bestTimeGhostOn = oldReplayOn;

			CleanupReplaysAndGhosts();

			return false;
		}
	}

	if (useLeaderboardGhosts)
	{
		/*for (auto it = activePlayerReplayManagers.begin(); it != activePlayerReplayManagers.end(); ++it)
		{
			(*it)->SetPlayers();
		}*/

		mainMenu->adventureManager->leaderboard->AddGhostsToVec(replayGhosts);
		mainMenu->adventureManager->leaderboard->AddPlayerReplayManagersToVec(activePlayerReplayManagers);
		mainMenu->adventureManager->leaderboard->SetActive(false, bestTimeGhostOn);
	}

	return true;
}

bool GameSession::TryStartGhosts()
{
	CleanupReplaysAndGhosts();

	bool res = AddGhosts();

	if (!res)
	{
		return false;
	}

	RestartGame();

	return true;
}

void GameSession::RestartWithNoReplayOrGhosts()
{
	bestReplayOn = false;
	bestTimeGhostOn = false;

	CleanupReplaysAndGhosts();

	RestartGame();
}

void GameSession::UpdateMatchParams(MatchParams &mp)
{
	assert(!IsParallelSession());

	int oldGameModeType = gameModeType;

	assert(currSession = this);
	SetMatchParams(mp);

	/*if (gameModeType == oldGameModeType)
	{
		return;
	}*/

	if (oldGameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE
		&& gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE)
	{
		if (!IsParallelSession())
		{
			ParallelPracticeMode *ppm = (ParallelPracticeMode*)gameMode;
			ppm->ownsParallelSessions = false;

			ParallelRaceMode *prm = new ParallelRaceMode;

			gameMode = prm;

			for (int i = 0; i < NetplayManager::MAX_PRACTICE_PLAYERS; ++i)
			{
				prm->parallelGames[i] = ppm->parallelGames[i];
				prm->parallelGames[i]->SetMatchParams(mp);
				currSession = prm->parallelGames[i];

				prm->parallelGames[i]->SetupGameMode();
				prm->parallelGames[i]->gameMode->Setup();
			}
			delete ppm;

			currSession = this;

			for (int i = matchParams.numPlayers - 1; i < ParallelMode::MAX_PARALLEL_SESSIONS; ++i)
			{
				delete prm->parallelGames[i];
				prm->parallelGames[i] = NULL;
			}

			gameMode->Setup();

			m_numActivePlayers = matchParams.numPlayers;

			/*for (int i = m_numActivePlayers; i < MAX_PLAYERS; ++i)
			{
				delete players[i];
			}*/
		}
	}
}

void GameSession::StartRaceFromPractice()
{
	ParallelPracticeMode *ppm = (ParallelPracticeMode*)gameMode;
	

	PracticePlayer &pp = netplayManager->practicePlayers[ppm->practiceInviteDisplay->selectedIndex];

	//netplayManager->TrySignalPracticePlayerToRace(pp);
	//netplayManager->TestNewRaceSystem();
	quit = true;
	returnVal = GameSession::GR_EXIT_PRACTICE_TO_RACE;
}