#include "MainMenu.h"
#include "WorldMap.h"
#include "Background.h"
#include "VectorMath.h"


using namespace std;
using namespace sf;

MapSelector::MapSelector( AdventureWorld &adventureWorld, 
	MainMenu *mm, sf::Vector2f &pos, int wIndex)
	:centerPos(pos)
{
	Tileset *ts_testNewSelect = mm->tilesetManager.GetTileset(
		"selectortest2.png", 1920, 1080);
	ts_testNewSelect->SetSpriteTexture(newSelectTestSpr);

	for (int i = 1; i <= 5; ++i)
	{
		ts_kinJump[i - 1] = mm->tilesetManager.GetSizedTileset("Menu/LevelSelect/Level_Teleport_0" + to_string(i) + "_512x512.png");
	}

	kinState = K_STAND;
	kinFrame = 0;
	kinSprite.setPosition(960, 540 + 300);

	worldIndex = wIndex;
	state = S_SECTORSELECT;
	mainMenu = mm;

	string worldIndexStr = to_string(1);//to_string(worldIndex+1);
	string nodeFile = string("WorldMap/node_w") + worldIndexStr + string("_128x128.png");

	ts_node = mm->tilesetManager.GetTileset(nodeFile, 128, 128);

	ts_bossFight = new Tileset*[1];
	ts_bossFight[0] = mm->tilesetManager.GetTileset("Worldmap/boss_w1_128x128.png", 128, 128);

	ts_sectorLevelBG = mm->tilesetManager.GetTileset("Worldmap/sector_levelbg_1200x400.png", 1200, 400);
	ts_levelStatsBG = mm->tilesetManager.GetTileset("Worldmap/level_stats_512x256.png", 512, 256);
	ts_sectorStatsBG = mm->tilesetManager.GetTileset("Worldmap/sector_box_256x256.png", 256, 256);

	ts_sectorKey = mm->tilesetManager.GetTileset("Worldmap/sectorkey_80x80.png", 80, 80);
	ts_sectorOpen = new Tileset*[1];
	ts_sectorOpen[0] = mm->tilesetManager.GetTileset("Worldmap/sectorunlock_256x256.png", 256, 256);

	bottomBG.setPosition(624, 545);
	numSectors = 0;
	sectorCenter = Vector2f(960, 550);

	sectorSASelector = NULL;

	frame = 0;

	int waitFrames[3] = { 30, 15, 10 };
	int waitModeThresh[2] = { 2, 2 };

	numSectors = adventureWorld.GetNumActiveSectors();

	sectors.resize(numSectors);

	int counter = 0;
	for (int i = 0; i < 8; ++i)
	{
		if (adventureWorld.sectors[i].GetNumActiveMaps() > 0)
		{
			sectors[counter] = new MapSector(adventureWorld.sectors[i], this, counter);
			++counter;
		}
	}

	sectorSASelector = new SingleAxisSelector(3, waitFrames, 2, 
		waitModeThresh, numSectors, 0);
}

MapSelector::~MapSelector()
{
	delete[] ts_bossFight;
	delete[] ts_sectorOpen;

	for (int i = 0; i < numSectors; ++i)
	{
		delete sectors[i];
	}
	delete sectorSASelector;
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

	if (kinState != K_HIDE)
	{
		target->draw(kinSprite);
	}

	target->draw(nodeHighlight);
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
				state = S_SECTORSELECT;
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

	UpdateHighlight();


	if (kinState == K_STAND)
	{
		kinFrame = 0;
		ts_kinJump[0]->SetSpriteTexture(kinSprite);
		ts_kinJump[0]->SetSubRect(kinSprite, 0);
		kinSprite.setOrigin(kinSprite.getLocalBounds().width / 2,
			kinSprite.getLocalBounds().height / 2);
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
		}
	}


	++kinFrame;
	++frame;

	return true;
}

void MapSelector::UpdateHighlight()
{
	MapSector *currSec = FocusedSector();
	nodeHighlight.setPosition(currSec->GetSelectedNodePos());
	int n = currSec->GetSelectedNodeSubIndex();


	int bossFightT = currSec->GetSelectedNodeBossFightType();//sec->levels[saSelector->currIndex].bossFightType;

	switch (bossFightT)
	{
	case 0:
		nodeHighlight.setTexture(*ts_node->texture);
		nodeHighlight.setTextureRect(ts_node->GetSubRect(9 + (n % 3)));
		break;
	case 1:
		nodeHighlight.setTexture(*ts_bossFight[0]->texture);
		nodeHighlight.setTextureRect(ts_bossFight[0]->GetSubRect(6));
		break;
	}

	nodeHighlight.setOrigin(nodeHighlight.getLocalBounds().width / 2, nodeHighlight.getLocalBounds().height / 2);

	int breathe = 60;
	float trans = (float)(frame%breathe) / (breathe / 2);
	if (trans > 1)
	{
		trans = 2.f - trans;

	}

	nodeHighlight.setColor(Color(255, 255, 255, 255 * trans));
}