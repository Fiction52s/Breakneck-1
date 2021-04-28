#include "MainMenu.h"
#include "WorldMap.h"
#include "Background.h"
#include "VectorMath.h"
#include "PlayerRecord.h"

using namespace std;
using namespace sf;

MapSelector::MapSelector( WorldMap *p_worldMap, World *p_world,
	MainMenu *mm, sf::Vector2f &pos )
	:centerPos(pos), world( p_world ), worldMap( p_worldMap )
{
	for (int i = 1; i <= 5; ++i)
	{
		ts_kinJump[i - 1] = mm->tilesetManager.GetSizedTileset("Menu/LevelSelect/Level_Teleport_0" + to_string(i) + "_512x512.png");
	}

	ts_statIcons = mm->tilesetManager.GetSizedTileset("HUD/score_384x96.png");

	ts_statIcons->SetSpriteTexture(bestTimeIconSpr);
	ts_statIcons->SetSpriteTexture(shardIconSpr);

	ts_statIcons->SetSubRect(bestTimeIconSpr, 12);
	ts_statIcons->SetSubRect(shardIconSpr, 14);

	ts_rock = mm->tilesetManager.GetSizedTileset("Menu/LevelSelect/level_rock_256x300.png");


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

	if (world->numSectors == 1)
	{
		state = S_MAPSELECT;
	}
	else
	{
		state = S_SECTORSELECT;
	}
	
	mainMenu = mm;


	string worldIndexStr = to_string(1);//to_string(worldIndex+1);
	string nodeFile = "WorldMap/node_w1_128x128.png";//string("WorldMap/node_w") + worldIndexStr + string("_128x128.png");

	ts_node = worldMap->GetTileset(nodeFile, 128, 128);

	ts_bossFight = new Tileset*[1];
	ts_bossFight[0] = worldMap->GetTileset("Worldmap/boss_w1_128x128.png", 128, 128);

	ts_sectorLevelBG = worldMap->GetTileset("Worldmap/sector_levelbg_1200x400.png", 1200, 400);
	ts_levelStatsBG = worldMap->GetTileset("Worldmap/level_stats_512x256.png", 512, 256);
	ts_sectorStatsBG = worldMap->GetTileset("Worldmap/sector_box_256x256.png", 256, 256);

	ts_sectorKey = worldMap->GetTileset("Worldmap/sectorkey_80x80.png", 80, 80);
	ts_sectorOpen = new Tileset*[1];
	ts_sectorOpen[0] = worldMap->GetTileset("Worldmap/sectorunlock_256x256.png", 256, 256);

	bottomBG.setPosition(624, 545);
	numSectors = 0;
	sectorCenter = Vector2f(960, 550);

	sectorSASelector = NULL;

	frame = 0;

	

	
	numSectors = world->numSectors;
	sectors.resize(numSectors);
	
	for (int i = 0; i < numSectors; ++i)
	{
		sectors[i] = new MapSector(worldMap->adventureFile,
			&(world->sectors[i]), this, i);
	}
	
	int waitFrames[3] = { 30, 15, 10 };
	int waitModeThresh[2] = { 2, 2 };

	sectorSASelector = new SingleAxisSelector(3, waitFrames, 2, 
		waitModeThresh, numSectors, 0);
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
	for (int i = 0; i < numSectors; ++i)
	{
		sectors[i]->Init(mainMenu->GetCurrentProgress());
	}
}

void MapSelector::UpdateSprites()
{

}

void MapSelector::RunSelectedMap()
{
	FocusedSector()->RunSelectedMap();
	ReturnFromMap();
}

void MapSelector::Draw(sf::RenderTarget *target)
{
	FocusedSector()->Draw(target);

	target->draw(rockSprite);

	if (kinState != K_HIDE)
	{
		target->draw(kinSprite);
	}

	//recordGhostMenu->Draw(target);
}

void MapSelector::ReturnFromMap()
{
	kinState = K_STAND;
	frame = 0;
}

MapSector * MapSelector::FocusedSector()
{
	return sectors[sectorSASelector->currIndex];
}

bool MapSelector::Update(ControllerState &curr,
	ControllerState &prev)
{
	ControllerState empty;
	MapSector::State currSectorState = FocusedSector()->state;

	bool left = curr.LLeft();
	bool right = curr.LRight();

	switch (state)
	{
	case S_SECTORSELECT:
	{
		if (curr.A && !prev.A)
		{
			state = S_MAPSELECT;
			break;
		}
		else if (curr.B && !prev.B)
		{
			return false;
		}
		//(currSectorState == MapSector::NORMAL || currSectorState == MapSector::COMPLETE)
		int changed = sectorSASelector->UpdateIndex(curr.LLeft(), curr.LRight());
		int numCurrLevels = FocusedSector()->unlockedLevelCount;
		if (changed != 0)
		{
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("level_change"));
			//mapSelector->SetTotalSize(numCurrLevels);
			FocusedSector()->UpdateLevelStats();
		}
		
		break;
	}
	case S_MAPSELECT:
	{
		if (kinState == K_STAND)
		{
			
			if (curr.B && !prev.B)
			{
				if (world->numSectors == 1)
				{
					return false;
					//state = S_MAPSELECT;
				}
				else
				{
					state = S_SECTORSELECT;
					FocusedSector()->UpdateStats();
				}
				break;
			}

			if (!FocusedSector()->Update(curr, prev))
			{
				kinState = K_JUMP;
				frame = 0;
			}

		}
		break;
	}
	}

	FocusedSector()->UpdateBG();


	if (kinState == K_STAND)
	{
		kinFrame = 0;
		ts_kinJump[0]->SetSpriteTexture(kinSprite);
		ts_kinJump[0]->SetSubRect(kinSprite, 0);
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

			sf::FloatRect ksGlobalBounds = kinSprite.getGlobalBounds();
			//rockSprite.setPosition(ksGlobalBounds.left + ksGlobalBounds.width / 2.0,
			//	ksGlobalBounds.top + ksGlobalBounds.height + 40);
		}

		

	}


	//keep selector spinning during jump
	FocusedSector()->UpdateSelectorSprite();
	FocusedSector()->UpdateSectorArrows();

	++kinFrame;
	++frame;

	return true;
}
