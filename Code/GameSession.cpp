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
#include "ShardSequence.h"
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

int GameSession::GetGameSessionState()
{
	return state;
}

void GameSession::SetGameSessionState(int s)
{
	state = (State)s;
}

void GameSession::QueryToSpawnEnemies()
{
	queryMode = QUERY_ENEMY;
	sf::Rect<double> spawnRect = screenRect;
	double spawnExtra = 600;//800
	spawnRect.left -= spawnExtra;
	spawnRect.width += 2 * spawnExtra;
	spawnRect.top -= spawnExtra;
	spawnRect.height += 2 * spawnExtra;

	tempSpawnRect = spawnRect;
	enemyTree->Query(this, spawnRect);
}

void GameSession::UpdateBackAndStartButtons()
{
	Actor *p0 = GetPlayer(0);
	if (raceFight != NULL)
	{
		if (raceFight->gameOver || GetCurrInput(0).back)
		{
			state = RACEFIGHT_RESULTS;
			raceFight->raceFightResultsFrame = 0;
			raceFight->victoryScreen->Reset();
			raceFight->victoryScreen->SetupColumns();
		}
	}
	else if (!p0->IsGoalKillAction(p0->action) && !p0->IsExitAction(p0->action))
	{
		ControllerState &currInput = GetCurrInput(0);
		ControllerState &prevInput = GetPrevInput(0);
		//if( IsKeyPressed( Keyboard ) )
		if (currInput.start && !prevInput.start)
		{
			state = PAUSE;
			ActivatePauseSound(GetSound("pause_on"));
			pauseMenu->SetTab(PauseMenu::PAUSE);
			soundNodeList->Pause(true);
		}
		else if ((currInput.back && !prevInput.back) || IsKeyPressed(Keyboard::G))
		{
			state = PAUSE;
			pauseMenu->SetTab(PauseMenu::MAP);
			ActivatePauseSound(GetSound("pause_on"));
			soundNodeList->Pause(true);
		}
	}
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

	if (raceFight != NULL)
	{
		cam.UpdateVS(GetPlayer(0), GetPlayer(1));
	}
	else
	{
		cam.Update(GetPlayer(0));
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

void GameSession::DrawKinOverFader(sf::RenderTarget *target)
{
	Actor *p = NULL;
	if ((fader->fadeSkipKin && fader->fadeAlpha > 0) || (swiper->skipKin && swiper->IsSwiping()))//IsFading()) //adjust later?
	{
		DrawEffects(EffectLayer::IN_FRONT, preScreenTex);
		for (int i = 0; i < 4; ++i)
		{
			p = GetPlayer(i);
			if (p != NULL)
			{
				p->Draw(preScreenTex);
			}
		}
	}
}

void GameSession::DrawShockwaves(sf::RenderTarget *target)
{
	DrawSceneToPostProcess(postProcessTex2);
	/*for (auto it = shockwaves.begin(); it != shockwaves.end(); ++it)
	{
		(*it)->Draw(postProcessTex2, target);
	}*/
}

void GameSession::DrawGame(sf::RenderTexture *target )//sf::RenderTarget *target)
{
	target->setView(view);

	background->Draw(target);

	UpdateEnvShaders(); //move this into the update loop

	DrawTopClouds(target);

	DrawBlackBorderQuads(target);

	DrawStoryLayer(EffectLayer::BEHIND_TERRAIN, target);
	DrawActiveSequence(EffectLayer::BEHIND_TERRAIN, target );
	DrawEffects(EffectLayer::BEHIND_TERRAIN, target);
	DrawEmitters(EffectLayer::BEHIND_TERRAIN, target);
	DrawZones(target);

	DrawSpecialTerrain(target);

	DrawFlyTerrain(target);

	DrawTerrain(target);

	DrawGoalEnergy(target);

	DrawHUD(target);

	DrawGates(target);
	DrawRails(target);
	DrawDecorBetween(target);
	DrawStoryLayer(EffectLayer::BEHIND_ENEMIES, target);
	DrawActiveSequence(EffectLayer::BEHIND_ENEMIES, target);
	DrawEffects(EffectLayer::BEHIND_ENEMIES, target);
	DrawEmitters(EffectLayer::BEHIND_ENEMIES, target );

	DrawEnemies(target);

	DrawStoryLayer(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, target);
	DrawActiveSequence(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, target );
	DrawEffects(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, target);
	DrawEmitters(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, target );

	goalPulse->Draw(target);
	DrawPlayerWires(target);

	DrawHitEnemies(target); //whited out hit enemies

	absorbParticles->Draw(target);
	absorbDarkParticles->Draw(target);

	DrawPlayers(target);

	absorbShardParticles->Draw(target);

	DrawReplayGhosts();

	DrawEffects(EffectLayer::IN_FRONT, target);
	DrawEmitters(EffectLayer::IN_FRONT, target );
	DrawStoryLayer(EffectLayer::IN_FRONT, target);
	DrawActiveSequence(EffectLayer::IN_FRONT, target );

	DrawBullets(target);

	DrawRain(target);

	//DrawActiveEnvPlants();

	UpdateDebugModifiers();

	DebugDraw(target);

	DrawShockwaves(target); //not operational atm

	target->setView(uiView);

	if (raceFight != NULL)
	{
		raceFight->DrawScore(target);
	}

	/*if (adventureHUD != NULL)
	{
	adventureHUD->Draw(preScreenTex);
	}*/

	scoreDisplay->Draw(target);

	if (showFrameRate)
	{
		target->draw(frameRateText);
	}

	if (showRunningTimer && !scoreDisplay->active)
	{
		target->draw(runningTimerText);
	}

	if (inputVis != NULL)
		inputVis->Draw(target);

	if (gateMarkers != NULL)
		gateMarkers->Draw(target);

	target->setView(view);

	DrawDyingPlayers(target);

	UpdateTimeSlowShader();

	if (currBroadcast != NULL)
	{
		target->setView(uiView);
		currBroadcast->Draw(target);
	}

	DrawActiveSequence(EffectLayer::UI_FRONT, target );
	DrawEffects(EffectLayer::UI_FRONT, target);
	DrawEmitters(EffectLayer::UI_FRONT, target );

	target->setView(uiView);

	fader->Draw(target);
	swiper->Draw(target);

	mainMenu->DrawEffects(target);

	target->setView(view); //sets it back to normal for any world -> pixel calcs
	DrawKinOverFader(target);
}

void GameSession::DrawRain(sf::RenderTarget *target)
{
	rainView.setCenter((int)view.getCenter().x % 64, (int)view.getCenter().y % 64);
	rainView.setSize(view.getSize());
	preScreenTex->setView(rainView);
	if (rain != NULL)
		rain->Draw(preScreenTex);
	preScreenTex->setView(view);
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

void GameSession::UpdateBarriers()
{
	for (auto it = barriers.begin();
		it != barriers.end(); ++it)
	{
		bool trig = (*it)->Update();
		if (trig)
		{
			TriggerBarrier((*it));
		}
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

void GameSession::ActiveStorySequenceUpdate()
{
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
}

void GameSession::ActiveBroadcastUpdate()
{
	if (currBroadcast != NULL)
	{
		if (!currBroadcast->Update())
		{
			//currStorySequence->EndSequence();
			currBroadcast = NULL;
		}
		else
		{
		}
	}
}

void GameSession::ActiveSequenceUpdate()
{
	if (activeSequence != NULL)// && activeSequence == startSeq )
	{
		State oldState = state;
		if (!activeSequence->Update())
		{
			if (activeSequence->nextSeq != NULL)
			{
				activeSequence->nextSeq->Reset();
				SetActiveSequence(activeSequence->nextSeq);
			}
			else
			{
				if (activeSequence == preLevelScene)
				{
					FreezePlayerAndEnemies(false);
					SetPlayerInputOn(true);
					if (shipEnterScene != NULL)
					{
						shipEnterScene->Reset();
						SetActiveSequence(shipEnterScene);
						state = RUN;
						return;
					}
				}


				if (activeSequence == postLevelScene)
				{
					goalDestroyed = true;
				}

				state = RUN;
				activeSequence = NULL;
			}
		}
		else
		{
			if (state != oldState)
			{
				switchState = true;
				return;
				//goto starttest;
			}
		}

		if (state == SEQUENCE)
		{

		}
	}
}

void GameSession::ActiveDialogueUpdate()
{
	if (activeDialogue != NULL)
	{
		if (GetCurrInput(0).A && !GetPrevInput(0).A)
		{
			if (activeDialogue->ConfirmDialogue())
				activeDialogue = NULL;
		}
	}
}

void GameSession::UpdateFrameRateCounterText( double frameTime )
{
	if (showFrameRate)
	{
		if (frameRateCounter == frameRateCounterWait)
		{
			double blah = 1.0 / frameTime;
			frameRateTimeTotal += blah;
			frameRateText.setString(to_string(frameRateTimeTotal / (frameRateCounterWait + 1)));
			frameRateCounter = 0;
			frameRateTimeTotal = 0;
		}
		else
		{
			double blah = 1.0 / frameTime;
			frameRateTimeTotal += blah;
			++frameRateCounter;
		}
	}
}

void GameSession::UpdateRunningTimerText()
{
	if (showRunningTimer && !scoreDisplay->active)
	{
		int tFrames = totalGameFrames;
		if (totalFramesBeforeGoal >= 0)
		{
			tFrames = totalFramesBeforeGoal;
		}
		runningTimerText.setString(GetTimeStr(tFrames));

	}
}

void GameSession::DrawSceneToPostProcess(sf::RenderTexture *tex)
{
	Sprite blah;
	blah.setTexture(preScreenTex->getTexture());
	tex->draw(blah);
	tex->display();
}

bool GameSession::RunGameModeUpdate( double frameTime )
{
	Actor *p = NULL;
	Actor *p0 = GetPlayer(0);

	accumulator += frameTime;

	window->clear(Color::Red);

	preScreenTex->clear(Color::Red);
	postProcessTex2->clear(Color::Red);

	switch (mapHeader->bossFightType)
	{
	case 0:
		break;
	case 1:
		break;
	}

	collider.ClearDebug();

	while (accumulator >= TIMESTEP)
	{
		if (!OneFrameModeUpdate())
		{
			break;
		}

		if (IsKeyPressed(sf::Keyboard::Y))
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

		if (goalDestroyed)
		{
			quit = true;
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
			frameRateCounter = 0;
		}

		if (pauseFrames > 0)
		{
			HitlagUpdate(); //the full update while in hitlag
			continue;
		}

		UpdateControllers();

		if (repPlayer != NULL)
		{
			//currently only records 1 player replays. fix this later
			repPlayer->UpdateInput(GetCurrInput(0));
		}

		if (recPlayer != NULL)
		{
			recPlayer->RecordFrame();
		}

		UpdateAllPlayersInput();

		ActiveSequenceUpdate();
		if (switchState)
			break;

		ActiveDialogueUpdate();
		
		totalGameFrames++;

		UpdatePlayersPrePhysics();

		TryToActivateBonus();

		ActiveStorySequenceUpdate();

		ActiveBroadcastUpdate();

		if (inputVis != NULL)
			inputVis->Update(GetPlayer(0)->currInput);

		if (!playerAndEnemiesFrozen)
		{
			UpdateEnemiesPrePhysics();

			UpdatePhysics();
		}

		RecordReplayEnemies();

		if (!playerAndEnemiesFrozen)
		{
			UpdatePlayersPostPhysics();
		}

		if (recGhost != NULL)
			recGhost->RecordFrame();

		UpdateReplayGhostSprites();
		
		if (goalDestroyed)
		{
			quit = true;
			returnVal = resType;
			/*recGhost->StopRecording();
			recGhost->WriteToFile( "testghost.bghst" );*/
			break;
		}

		UpdatePlayerWireQuads();

		if (!playerAndEnemiesFrozen)
		{
			UpdateEnemiesPostPhysics();
		}

		UpdateGates();

		absorbParticles->Update();
		absorbDarkParticles->Update();
		absorbShardParticles->Update();

		UpdateEffects();
		UpdateEmitters();

		mainMenu->musicPlayer->Update();

		UpdateHUD();

		scoreDisplay->Update();

		soundNodeList->Update();

		pauseSoundNodeList->Update();

		goalPulse->Update();

		if (rain != NULL)
			rain->Update();

		UpdateBarriers();

		UpdateCamera();

		if (gateMarkers != NULL)
			gateMarkers->Update(&cam);

		fader->Update();
		swiper->Update();
		background->Update(cam.GetPos());
		if (topClouds != NULL)
			topClouds->Update();
		//rain.Update();

		mainMenu->UpdateEffects();

		if (raceFight != NULL)
		{
			raceFight->UpdateScore();
		}

		UpdateGoalFlow();

		QueryToSpawnEnemies();

		UpdateEnvPlants();

		QueryBorderTree(screenRect);

		specialPieceList = NULL;
		queryMode = QUERY_SPECIALTERRAIN;
		specialTerrainTree->Query(this, screenRect);

		flyTerrainList = NULL;
		queryMode = QUERY_FLYTERRAIN;
		flyTerrainTree->Query(this, screenRect);

		drawInversePoly = ScreenIntersectsInversePoly(screenRect);

		UpdateDecorSprites();
		UpdateDecorLayers();

		UpdateBackAndStartButtons();

		UpdateZones();

		/*if( player->record > 0 )
		{
		player->ghosts[player->record-1]->states[player->ghosts[player->record-1]->currFrame].screenRect =
		screenRect;
		}*/
		accumulator -= TIMESTEP;

		if (debugScreenRecorder != NULL)
		{
			break; //for recording stuff
		}
	}

	if (switchState && state != FROZEN)
	{
		return false;
	}

	if (debugScreenRecorder != NULL)
		if (IsKeyPressed(Keyboard::R))
		{
			debugScreenRecorder->StartRecording();
			//player->maxFallSpeedSlo += maxFallSpeedFactor;
			//cout << "maxFallSpeed : " << player->maxFallSpeed << endl;
		}

	sf::Event ev;
	while (window->pollEvent(ev))
	{
		if (ev.type == Event::LostFocus)
		{
			if (state == RUN)
				state = PAUSE;
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

	return true;
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
}

PoiInfo::PoiInfo( const std::string &pname, Edge *e, double q )
{
	name = pname;
	edge = e;
	edgeQuantity = q;
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

	for (auto it = cameraShotMap.begin(); it != cameraShotMap.end(); ++it)
	{
		delete (*it).second;
	}
	cameraShotMap.clear();

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

	//might be able to setup in setuptopclouds
	if (topClouds != NULL)
	{
		delete topClouds;
		topClouds = NULL;
	}

	CleanupGates();

	for (auto it = decorBetween.begin(); it != decorBetween.end(); ++it)
	{
		delete (*it);
	}
	decorBetween.clear();

	for (auto it = fullAirTriggerList.begin(); it != fullAirTriggerList.end(); ++it)
	{
		delete (*it);
	}
	fullAirTriggerList.clear();

	for (auto it = poiMap.begin(); it != poiMap.end(); ++it)
	{
		delete (*it).second;
	}
	poiMap.clear();

	if (shipExitScene != NULL)
	{
		delete shipExitScene;
		shipExitScene = NULL;
	}

	if (shipEnterScene != NULL)
	{
		delete shipEnterScene;
		shipEnterScene = NULL;
	}

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

	if (deathSeq != NULL)
	{
		delete deathSeq;
		deathSeq = NULL;
	}

	if (getShardSeq != NULL)
	{
		delete getShardSeq;
		getShardSeq = NULL;
	}

	if (gateMarkers != NULL)
	{
		delete gateMarkers;
		gateMarkers = NULL;
	}

	for( int i = 0; i < allPolysVec.size(); ++i)
	{
		delete allPolysVec[i];
	}
	allPolysVec.clear();

	if ( parentGame == NULL && fBubblePos != NULL)
	{
		delete[] fBubblePos;
		delete[] fBubbleRadiusSize;
		delete[] fBubbleFrame;

		fBubblePos = NULL;
		fBubbleRadiusSize = NULL;
		fBubbleFrame = NULL;
	}

	for (auto it = cameraShotMap.begin(); it != cameraShotMap.end(); ++it)
	{
		delete (*it).second;
	}
	cameraShotMap.clear();

	for (auto it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		delete (*it);
	}
	fullEnemyList.clear();

	if ( goalFlow != NULL)
	{
		delete goalFlow;
		goalFlow = NULL;
	}

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

	if ( parentGame == NULL && shardsCapturedField != NULL)
	{
		delete shardsCapturedField;
		shardsCapturedField = NULL;
	}

	if (shardPop != NULL)
	{
		delete shardPop;
		shardPop = NULL;
	}

	

	if (parentGame == NULL && goalPulse != NULL)
	{
		delete goalPulse;
		goalPulse = NULL;
	}

	if (topClouds != NULL)
	{
		delete topClouds;
		topClouds = NULL;
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

	if (enemyTree != NULL)
	{
		delete enemyTree;
		enemyTree = NULL;
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

	if ( parentGame == NULL && scoreDisplay != NULL)
	{
		delete scoreDisplay;
		scoreDisplay = NULL;
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

	for (auto it = decorBetween.begin(); it != decorBetween.end(); ++it)
	{
		delete (*it);
	}

	for (auto it = fullAirTriggerList.begin(); it != fullAirTriggerList.end(); ++it)
	{
		delete (*it);
	}

	for (auto it = poiMap.begin(); it != poiMap.end(); ++it)
	{
		delete (*it).second;
	}
	
	if (shipExitScene != NULL)
	{
		delete shipExitScene;
		shipExitScene = NULL;
	}

	if (shipEnterScene != NULL)
	{
		delete shipEnterScene;
		shipEnterScene = NULL;
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

void GameSession::UpdatePhysics()
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
		
		if( current->hasMonitor)
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
			current->keyShader.setUniform( "prop", fac );
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

void GameSession::UnlockPower(int pType)
{
	mainMenu->GetCurrentProgress()->UnlockUpgrade(pType);
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
	if (mapHeader->gameMode == MapHeader::MapType::T_RACEFIGHT)
	{
		assert(raceFight == NULL);
		raceFight = new RaceFight(this, 180);
	}
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

			decorBetween.push_back(new DecorDraw(betweenVerts,
				numBetweenLayer * 4, betweenList.front().ts));
		}
	}

	decorListMap.clear();
}

void GameSession::ProcessPlayerStartPos()
{
	Actor *p0 = GetPlayer(0);
	if (parentGame == NULL)
	{
		p0->position = V2d(playerOrigPos);
	}
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

	if (enemy != NULL)
	{
		fullEnemyList.push_back(enemy);
		enemyTree->Insert(enemy);

		enemy->AddToWorldTrees(); //adds static objects etc
	}
	else
	{
		const string &typeName = a->type->info.name;
		if (typeName == "xbarrier")
		{
			XBarrierParams *xbp = (XBarrierParams*)a;
			AddBarrier(xbp);
		}
		else if (typeName == "extrascene")
		{
			ExtraSceneParams *xp = (ExtraSceneParams*)a;
			BasicBossScene *scene = BasicBossScene::CreateScene(xp->GetName());
			if (xp->extraSceneType == 0)//prelevel
			{
				preLevelScene = scene;
			}
			else if (xp->extraSceneType == 1)//postlevel
			{
				postLevelScene = scene;
			}
		}
		else if (typeName == "camerashot")
		{
			CameraShotParams *csp = (CameraShotParams*)a;
			const std::string &cName = csp->GetName();
			CameraShot *shot = new CameraShot(csp->GetName(), csp->GetFloatPos(),csp->zoom);
			if (cameraShotMap.count(cName) > 0 )
			{
				assert(false);
			}
		
			cameraShotMap[cName] = shot;
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

	//create sequences for the barriers after all enemies have already been loaded
	for (auto it = barriers.begin(); it != barriers.end(); ++it)
	{
		(*it)->SetScene();
	}


	for (list<Enemy*>::iterator it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		(*it)->SetZoneSpritePosition();
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



void GameSession::CreateDeathSequence()
{
	if (parentGame != NULL)
	{
		deathSeq = parentGame->deathSeq;
	}
	else if (deathSeq == NULL)
	{
		deathSeq = new DeathSequence(this);
	}
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


	SetupHUD();

	bool blackBorder[2];
	bool topBorderOn = false;
	SetupGlobalBorderQuads(blackBorder, topBorderOn);
	adventureHUD->mini->SetupBorderQuads(blackBorder, topBorderOn, mapHeader);


	kinMapSpawnIcon.setTexture(*adventureHUD->mini->ts_miniIcons->texture);
	kinMapSpawnIcon.setTextureRect(adventureHUD->mini->ts_miniIcons->GetSubRect(1));
	kinMapSpawnIcon.setOrigin(kinMapSpawnIcon.getLocalBounds().width / 2,
		kinMapSpawnIcon.getLocalBounds().height / 2);


	if (topBorderOn)
	{
		topClouds = new TopClouds(this);
	}

	

	CreateZones();

	SetupGateMarkers();

	SetupZones();

	

	SetupBackground();
	

	//still too far


	cout << "done opening file" << endl;

	SetupPlayers();

	
	SetupTimeBubbles();


	SetPlayersGameMode();

	CreateDeathSequence();

	/*for (auto it = fullEnemyList.begin(); it != fullEnemyList.end(); ++it)
	{
		(*it)->Init();
	}*/

	
	//too far
	

	SetupRecGhost();
	

	SetupPauseMenu();

	SetupControlProfiles();

	SetupGoalPulse();

	GetPlayer(0)->SetupDrain();

	SetupGoalFlow();
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
		for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			players[i] = parentGame->players[i];
			//if( players[i] != NULL )
			//	players[i]->Respawn(); //need a special bonus respawn later
		}

		m_numActivePlayers = parentGame->m_numActivePlayers;
		return;
	}


	if (raceFight != NULL)
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
}

void GameSession::SetupShardsCapturedField()
{
	if (parentGame != NULL)
	{
		shardsCapturedField = parentGame->shardsCapturedField;
	}
	else if( shardsCapturedField == NULL )
		shardsCapturedField = new BitField(32 * 5);
	else
	{
		shardsCapturedField->Reset();
	}
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

void GameSession::SetupScoreDisplay()
{
	if (parentGame != NULL)
	{
		scoreDisplay = parentGame->scoreDisplay;
		scoreDisplay->Reset();
	}
	else if( scoreDisplay == NULL )
		scoreDisplay = new ScoreDisplay(this, Vector2f(1920, 0), mainMenu->arial);
	else
	{
		scoreDisplay->Reset();
	}
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
	else if (mapHeader->gameMode == MapHeader::MapType::T_STANDARD && recGhost == NULL)
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
	showFrameRate = true;
	showRunningTimer = true;

	ClearEmitters();
	bool oldMouseGrabbed = mainMenu->GetMouseGrabbed();
	bool oldMouseVisible = mainMenu->GetMouseVisible();

	mainMenu->SetMouseGrabbed(true);
	mainMenu->SetMouseVisible(false);

	currStorySequence = NULL;
	currBroadcast = NULL;

	View oldPreTexView = preScreenTex->getView();
	View oldWindowView = window->getView();

	preScreenTex->setView(view);
	
	frameRateText.setString("00");
	frameRateText.setFont(mainMenu->arial);
	frameRateText.setCharacterSize(30);
	frameRateText.setFillColor(Color::Red);

	runningTimerText.setString("---- : --");
	runningTimerText.setFont(mainMenu->arial);
	runningTimerText.setCharacterSize(30);
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
	
	frameRateCounter = 0;
	frameRateCounterWait = 20;
	frameRateTimeTotal = 0;

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
	rainView.setCenter(Vector2f(0, 0));
	rainView.setSize(Vector2f(1920, 1080));

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
	switchState = false;

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

	if (parentGame != NULL)
	{
		for (int i = 0; i < 4; ++i)
		{
			p = GetPlayer(i);
			if (p != NULL)
			{
				p->Respawn(); //need a special bonus respawn later
			}
		}
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

		UpdateFrameRateCounterText( frameTime );
		UpdateRunningTimerText();


		if( state == RUN )
		{
			if (!RunGameModeUpdate(frameTime))
			{
				continue;
			}
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

			Vector2i b1 = mapTex->mapCoordsToPixel( Vector2f(playerOrigPos.x, playerOrigPos.y ) );

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
				preScreenTex->draw(frameRateText);
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
				preScreenTex->draw(frameRateText);
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
					ActivatePauseSound(GetSound("pause_off"));
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
					frameRateCounter = 0;
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

				Vector2i b1 = mapTex->mapCoordsToPixel(Vector2f(playerOrigPos.x, playerOrigPos.y));

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
	deathSeq = NULL;
	gateMarkers = NULL;
	inversePolygon = NULL;
	fBubblePos = NULL;
	fBubbleRadiusSize = NULL;
	fBubbleFrame = NULL;
	preLevelScene = NULL;
	postLevelScene = NULL;
	shardPop = NULL;
	getShardSeq = NULL;
	shardsCapturedField = NULL;
	level = NULL;
	inputVis = NULL;
	goalPulse = NULL;
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
	
	rain = NULL;//new Rain(this);//NULL;
	va = NULL;
	activeEnemyList = NULL;
	activeEnemyListTail = NULL;
	raceFight = NULL;
	recPlayer = NULL;
	repPlayer = NULL;
	recGhost = NULL;
	//repGhost = NULL;
	shipExitScene = NULL;
	shipEnterScene = NULL;
	activeDialogue = NULL;
	explodingGravityGrass = NULL;
	polyQueryList = NULL;
	specialPieceList = NULL;
	flyTerrainList = NULL;
	absorbParticles = NULL;
	absorbDarkParticles = NULL;
	absorbShardParticles = NULL;
	goalFlow = NULL;
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
	drawInversePoly = true;
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

bool GameSession::ScreenIntersectsInversePoly( sf::Rect<double> &screenRect )
{
	double extra = 32; //for some buffer room

	screenRect.left -= extra;
	screenRect.top -= extra;
	screenRect.width += extra * 2;
	screenRect.height += extra * 2;

	inverseEdgeList = NULL;
	queryMode = QUERY_INVERSEBORDER;
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

void GameSession::SetStorySeq(StorySequence *storySeq)
{
	storySeq->Reset();
	currStorySequence = storySeq;
	state = GameSession::STORY;
}

void GameSession::DrawDyingPlayers(sf::RenderTarget *target)
{
	Actor *p = NULL;
	for (int i = 0; i < 4; ++i)
	{
		p = GetPlayer(i);
		if (p != NULL)
		{
			p->DeathDraw(target);
		}
	}
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

void GameSession::SetupGoalPulse()
{
	if (parentGame != NULL)
	{
		goalPulse = parentGame->goalPulse;
	}
	else if (goalPulse == NULL)
	{
		goalPulse = new GoalPulse(this);// , Vector2f(goalPos.x, goalPos.y));
	}

	goalPulse->SetPosition(Vector2f(goalPos));
}

void GameSession::EndLevel()
{
	if (postLevelScene != NULL)
	{
		SetActiveSequence(postLevelScene);
	}
	else
	{
		goalDestroyed = true;
	}
}

void GameSession::DrawStoryLayer(EffectLayer ef, sf::RenderTarget *target)
{
	if (currStorySequence != NULL)
	{
		sf::View oldV = target->getView();
		target->setView(uiView);
		currStorySequence->DrawLayer(target, ef);
		target->setView(oldV);
	}
}

void GameSession::DrawActiveSequence(EffectLayer layer, sf::RenderTarget *target)
{
	if (activeSequence != NULL)
	{
		sf::View oldView = target->getView();
		if (layer == UI_FRONT)
		{
			target->setView(uiView);
		}

		activeSequence->Draw(target, layer);

		if (layer == UI_FRONT)
		{
			target->setView(oldView);
		}
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

void GameSession::DrawBlackBorderQuads(sf::RenderTarget *target)
{
	bool narrowMap = mapHeader->boundsWidth < 1920 * 2;

	if (cam.manual || narrowMap)
	{
		target->draw(blackBorderQuads, 16, sf::Quads);
	}
	else
	{
		target->draw(blackBorderQuads, 8, sf::Quads);
	}
}

void GameSession::DrawTopClouds(sf::RenderTarget *target)
{
	if (topClouds != NULL)
		topClouds->Draw(target);
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

void GameSession::DrawDecorBetween(sf::RenderTarget *target)
{
	for (auto it = decorBetween.begin(); it != decorBetween.end(); ++it)
	{
		(*it)->Draw(target);
	}
}

void GameSession::DrawGoalEnergy(sf::RenderTarget *target)
{
	if (goalFlow != NULL)
	{
		goalFlow->Draw(target);
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

void GameSession::DrawEmitters(EffectLayer layer, sf::RenderTarget *target)
{
	ShapeEmitter *curr = emitterLists[layer];
	while (curr != NULL)
	{
		curr->Draw(target);
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
	if (goalFlow != NULL)
	{
		Vector2f topLeft(view.getCenter().x - view.getSize().x / 2,
			view.getCenter().y + view.getSize().y / 2);
		goalFlow->Update(cam.GetZoom(), topLeft);
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

void GameSession::DrawReplayGhosts()
{
	for (auto it = replayGhosts.begin(); it != replayGhosts.end(); ++it)
	{
		(*it)->Draw(preScreenTex);
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
		adventureHUD->keyMarker->Reset();
		
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

typedef pair<V2d, V2d> pairV2d;
void GameSession::SetupGoalFlow()
{
	//is still created in a bonus level

	if (goalFlow != NULL)
	{
		delete goalFlow;
		goalFlow = NULL;
	}

	if (!hasGoal)
	{
		return;
	}

	

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
	rayMode = "energy_flow";

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

		bool rayOkay = rayEnd.x >= mapHeader->leftBounds && rayEnd.y >= mapHeader->topBounds 
			&& rayEnd.x <= mapHeader->leftBounds + mapHeader->boundsWidth
			&& rayEnd.y <= mapHeader->topBounds + mapHeader->boundsHeight;
		
		
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
				if( rcEdge->IsInvisibleWall() || rcEdge->edgeType == Edge::CLOSED_GATE )
				{
				//	cout << "secret break" << endl;
					break;
				}
				
				rayIgnoreEdge1 = rayIgnoreEdge;
				rayIgnoreEdge = rcEdge;

				V2d rn = rcEdge->Normal();
				double d = dot( rn, rayDir );
				V2d hitPoint = rcEdge->GetPosition( rcQuantity );
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
		return;
	}

	goalFlow = new GoalFlow(Vector2f(goalPos), allInfo);
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
	else if( rayMode == "energy_flow" )
	{
		if( edge->edgeType == Edge::CLOSED_GATE || edge->edgeType == Edge::OPEN_GATE )
		{
			return;
		}

		if( edge != rayIgnoreEdge && edge != rayIgnoreEdge1 
			&& ( rcEdge == NULL || length( edge->GetPosition( edgeQuantity ) - rayStart ) < 
			length( rcEdge->GetPosition( rcQuantity ) - rayStart ) ) )
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

void GameSession::ResetEnemies()
{
	rResetEnemies( enemyTree->startNode );

	activeEnemyList = NULL;
	activeEnemyListTail = NULL;

	ClearEffects();
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

//please get rid of this soon lol
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

			if (e->spawned)
			{
				e->InitOnRespawn();
			}
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

			if (e->spawned)
			{
				e->InitOnRespawn();
			}

			e->Reset();
		}
		
	}
}

void GameSession::SetActiveSequence(Sequence *activeSeq)
{
	activeSequence = activeSeq;

	if (activeSequence == preLevelScene)
	{
		FreezePlayerAndEnemies(true);
		SetPlayerInputOn(false);
	}

	activeSequence->StartRunning();
}

void GameSession::TriggerBarrier( Barrier *b )
{
	if (b->triggerSeq != NULL)
	{
		SetActiveSequence(b->triggerSeq);
	}
	else
	{
		b->Trigger();
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
