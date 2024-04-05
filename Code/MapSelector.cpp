#include "MainMenu.h"
#include "WorldMap.h"
#include "Background.h"
#include "VectorMath.h"
#include "PlayerRecord.h"
#include "AdventureManager.h"
#include "Leaderboard.h"

using namespace std;
using namespace sf;

MapSelector::MapSelector( WorldMap *p_worldMap, World *p_world,
	MainMenu *mm, sf::Vector2f &pos )
	:centerPos(pos), world( p_world ), worldMap( p_worldMap )
{
	mainMenu = mm;

	for (int i = 1; i <= 5; ++i)
	{
		ts_kinJump[i - 1] = worldMap->GetSizedTileset("Menu/LevelSelect/Level_Teleport_0" + to_string(i) + "_512x512.png");
	}

	ts_statIcons = mm->adventureManager->GetSizedTileset("HUD/score_icons_128x96.png");

	/*ts_statIcons->SetSpriteTexture(bestTimeIconSpr);
	ts_statIcons->SetSpriteTexture(shardIconSpr);
	ts_statIcons->SetSpriteTexture(logIconSpr);

	ts_statIcons->SetSubRect(bestTimeIconSpr, 12);
	ts_statIcons->SetSubRect(shardIconSpr, 14);
	ts_statIcons->SetSubRect(logIconSpr, 14);*/

	ts_rock = worldMap->GetSizedTileset("Menu/LevelSelect/level_rock_256x300.png");


	rockSprite.setTexture(*ts_rock->texture);
	rockSprite.setOrigin(rockSprite.getLocalBounds().width / 2, 0);
	rockSprite.setPosition(960, 540 + 399 + 30);

	kinState = K_STAND;
	kinFrame = 0;
	kinSprite.setPosition(960, 540 + 300 + 30);

	//recordGhostMenu = new RecordGhostMenu(mainMenu, Vector2f(960, 540));
	//recordGhostMenu->UpdateItemText();
	//list< GhostEntry*> ghosts;
	//recordGhostMenu->GetActiveList(ghosts);
	//if (!ghosts.empty())
	//{
		//gs->SetupGhosts(ghosts);
		
	//}
	//ghostSelector->Update(mainMenu->menuCurrInput, mainMenu->menuPrevInput);
	//ghostSelector->UpdateLoadedFolders();

	
	
	


	string worldIndexStr = to_string(world->index + 1);

	string nodeFile = "Menu/WorldMap/LevelNode/node_w" + worldIndexStr + "_128x128.png";

	ts_node = worldMap->GetSizedTileset(nodeFile);

	ts_bossFight = new Tileset*[1];
	ts_bossFight[0] = worldMap->GetTileset("Menu/Worldmap/boss_w1_128x128.png", 128, 128);

	ts_sectorOpen = new Tileset*[1];
	ts_sectorOpen[0] = worldMap->GetTileset("Menu/Worldmap/sectorunlock_256x256.png", 256, 256);

	bottomBG.setPosition(624, 545);
	numSectors = 0;
	sectorCenter = Vector2f(960, 550);

	sectorSASelector = NULL;

	frame = 0;

	/*continueBGLoadingThread = true;
	bgLoadFinished = true;
	bgLoadThread = NULL;
	bgLoadThread = new boost::thread(MapSelector::sLoadBG, this);

	continueBGDestroyThread = true;
	bgDestroyFinished = true;
	bgDestroyThread = new boost::thread(MapSelector::sDestroyBG, this);*/

	
	numSectors = world->numSectors;
	sectors.resize(numSectors);
	
	for (int i = 0; i < numSectors; ++i)
	{
		sectors[i] = new MapSector(mainMenu->adventureManager->adventureFile,
			&(world->sectors[i]), this, i);
	}
	
	int waitFrames[3] = { 30, 15, 10 };
	int waitModeThresh[2] = { 2, 2 };

	sectorSASelector = new SingleAxisSelector(3, waitFrames, 2, 
		waitModeThresh, numSectors, 0);
	//sectorSASelector->SetTotalSize()

	if (world->numSectors == 1)
	{
		state = S_MAPSELECT;
		//FocusedSector()->UpdateMapPreview();
	}
	else
	{
		state = S_SECTORSELECT;
	}
}

MapSelector::~MapSelector()
{
//	delete recordGhostMenu;

	delete[] ts_bossFight;
	delete[] ts_sectorOpen;

	for (int i = 0; i < numSectors; ++i)
	{
		delete sectors[i];
	}
	delete sectorSASelector;
}

void MapSelector::Init()
{
	playerSkinShader.SetSkin(mainMenu->adventureManager->currSaveFile->visualInfo.skinIndex);
	for (int i = 0; i < numSectors; ++i)
	{
		sectors[i]->Init(mainMenu->adventureManager->currSaveFile);
	}
}

void MapSelector::UpdateSprites()
{

}

void MapSelector::RunSelectedMap()
{
	FocusedSector()->RunSelectedMap();
	//ReturnFromMap();
}

void MapSelector::Draw(sf::RenderTarget *target)
{
	FocusedSector()->Draw(target);

	

	//recordGhostMenu->Draw(target);
}

void MapSelector::ReturnFromMap()
{
	kinState = K_STAND;
	kinFrame = 0;
	frame = 0;
}

MapSector * MapSelector::FocusedSector()
{
	return sectors[sectorSASelector->currIndex];
}

void MapSelector::CreateBGs()
{
	//cout << "creating bgs for world " << world->index + 1 << endl;
	for (int i = 0; i < numSectors; ++i)
	{
		sectors[i]->CreateBG();
	}
	//cout << "creating bgs for world " << world->index + 1 << endl;
}

void MapSelector::DestroyBGs()
{
	//cout << "destroying bgs for world " << world->index + 1 << endl;
	for (int i = 0; i < numSectors; ++i)
	{
		sectors[i]->DestroyBG();
	}
}

bool MapSelector::UpdateSectorIndex(bool left, bool right)
{
	int changed = sectorSASelector->UpdateIndex(left, right);
	int numCurrLevels = FocusedSector()->unlockedLevelCount;
	if (changed != 0)
	{
		mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("level_change"));

		//mapSelector->SetTotalSize(numCurrLevels);

		/*loadIndex = sectorSASelector->currIndex;
		sectorSASelector->currIndex = oldIndex;
		bgLoadFinished = false;
		state = S_CHANGINGSECTORS;*/

		FocusedSector()->UpdateLevelStats();

		FocusedSector()->mapSASelector->SetIndex(0);

		return true;
	}

	return false;
}

bool MapSelector::Update(ControllerDualStateQueue *controllerInput)
{
	ControllerState empty;
	MapSector::State currSectorState = FocusedSector()->state;

	bool keyboardBack = controllerInput->GetControllerType() != CTYPE_KEYBOARD && (CONTROLLERS.KeyboardButtonPressed(Keyboard::BackSpace)
		|| CONTROLLERS.KeyboardButtonPressed(Keyboard::Escape));

	switch (state)
	{
	case S_SECTORSELECT:
	{
		if (controllerInput->ButtonPressed_A())
		{
			if (FocusedSector()->IsUnlocked())
			{
				state = S_MAPSELECT;
				FocusedSector()->UpdateMapPreview();
			}
			break;
		}
		else if (controllerInput->ButtonPressed_B() || keyboardBack)
		{
			//FocusedSector()->DestroyMapPreview();
			//FocusedSector()->DestroyBG();
			return false;
		}
		else
		{
			bool left = controllerInput->GetCurrState().LLeft() || controllerInput->ButtonHeld_PadLeft() || controllerInput->GetCurrState().LeftTriggerPressed();
			bool right = controllerInput->GetCurrState().LRight() || controllerInput->ButtonHeld_PadRight() || controllerInput->GetCurrState().RightTriggerPressed();

			UpdateSectorIndex(left, right);
		}		
		break;
	}
	case S_MAPSELECT:
	{
		if (kinState == K_STAND)
		{
			if (controllerInput->ButtonPressed_B() || keyboardBack)
			{
				if (FocusedSector()->state == MapSector::LEADERBOARD || FocusedSector()->state == MapSector::LEADERBOARD_STARTING)
				{
					FocusedSector()->state = MapSector::NORMAL;
					mainMenu->adventureManager->leaderboard->Hide();
					break;
				}

				if (world->numSectors == 1)
				{
					return false;
					//state = S_MAPSELECT;
				}
				else
				{
					state = S_SECTORSELECT;
					FocusedSector()->UpdateStats();
					FocusedSector()->SetSelectedIndexToFurthestProgress();
					FocusedSector()->UpdateLevelStats();
					FocusedSector()->UpdateMapPreview();
					//FocusedSector()->DestroyMapPreview();
				}
				break;
			}
			
			int oldIndex = sectorSASelector->currIndex;

			bool left = controllerInput->GetCurrState().LeftTriggerPressed();
			bool right = controllerInput->GetCurrState().RightTriggerPressed();
			bool sectorChanged = UpdateSectorIndex(left, right);

			if (sectorChanged)
			{
				FocusedSector()->UpdateLevelStats();
				FocusedSector()->UpdateMapPreview();
				break;
			}


			if (!FocusedSector()->Update(controllerInput))
			{
				kinState = K_JUMP;
				frame = 0;
			}

		}
		break;
	}
	case S_CHANGINGSECTORS:
	{
		/*if (bgLoadFinished)
		{
			state = S_SECTORSELECT;

			destroyIndex = sectorSASelector->currIndex;

			sectorSASelector->currIndex = loadIndex;

			bgDestroyFinished = false;
		}*/
		break;
	}
	}

	FocusedSector()->UpdateBG();


	if (kinState == K_STAND)
	{
		kinFrame = 0;
		ts_kinJump[0]->SetSpriteTexture(kinSprite);
		ts_kinJump[0]->SetSubRect(kinSprite, 0);

		IntRect ir = ts_kinJump[0]->GetSubRect(0);

		playerSkinShader.SetSubRect(ts_kinJump[0], ir);

		kinSprite.setOrigin(kinSprite.getLocalBounds().width / 2,
			kinSprite.getLocalBounds().height / 2);

		sf::FloatRect ksGlobalBounds = kinSprite.getGlobalBounds();
		//rockSprite.setPosition(ksGlobalBounds.left + ksGlobalBounds.width / 2.0,
		//	ksGlobalBounds.top + ksGlobalBounds.height + 40);
	}
	else if (kinState == K_JUMP)
	{
		if (kinFrame == 66 * 2)
		{
			mainMenu->SetMode(MainMenu::TRANS_WORLDMAP_TO_LOADING);
			kinState = K_HIDE;
			frame = 0;
			//load map
		}
		else
		{
			int kinMult = 2;
			int kinTex = (kinFrame / kinMult) / 16;
			int realKinFrame = (kinFrame / kinMult) % 16;

			ts_kinJump[kinTex]->SetSpriteTexture(kinSprite);
			ts_kinJump[kinTex]->SetSubRect(kinSprite, realKinFrame);
			kinSprite.setOrigin(kinSprite.getLocalBounds().width / 2,
				kinSprite.getLocalBounds().height / 2);

			IntRect ir = ts_kinJump[kinTex]->GetSubRect(realKinFrame);

			playerSkinShader.SetSubRect(ts_kinJump[kinTex], ir);

			sf::FloatRect ksGlobalBounds = kinSprite.getGlobalBounds();
			//rockSprite.setPosition(ksGlobalBounds.left + ksGlobalBounds.width / 2.0,
			//	ksGlobalBounds.top + ksGlobalBounds.height + 40);
		}

		

	}


	//keep selector spinning during jump
	FocusedSector()->UpdateSelectorSprite();
	FocusedSector()->UpdateSectorArrows();
	FocusedSector()->UpdateOptionButtons();

	++kinFrame;
	++frame;

	return true;
}

//void MapSelector::sLoadBG(MapSelector* ms )
//{
//	while (ms->continueBGLoadingThread)
//	{
//		if (!ms->bgLoadFinished)
//		{
//			ms->sectors[ms->loadIndex]->CreateBG();
//			ms->bgLoadFinished = true;
//		}
//	}
//	//ms->sectors[oldIndex]->DestroyBG();
//}
//
//void MapSelector::sDestroyBG(MapSelector *ms)
//{
//	while (ms->continueBGDestroyThread)
//	{
//		if (!ms->bgDestroyFinished)
//		{
//			ms->sectors[ms->destroyIndex]->DestroyBG();
//			ms->bgDestroyFinished = true;
//		}
//	}
//}