#include "MainMenu.h"
#include "WorldMap.h"
#include "Background.h"
#include "VectorMath.h"


using namespace std;
using namespace sf;

MapSelector::MapSelector(MainMenu *mm, sf::Vector2f &pos, int wIndex)
	:centerPos(pos)
{
	Tileset *ts_testNewSelect = mm->tilesetManager.GetTileset(
		"selectortest2.png", 1920, 1080);
	ts_testNewSelect->SetSpriteTexture(newSelectTestSpr);
	//bg->Set(Vector2f(0, 0), 1.f);


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

	string scrollEnergyFile = string("WorldMap/sector_aura_w") + worldIndexStr + string("_1200x300.png");

	ts_scrollingEnergy = mainMenu->tilesetManager.GetTileset(scrollEnergyFile, 1200, 300);
	//Tileset *ts_bottom = mm->tilesetManager.GetTileset("Worldmap/levelselect_672x256.png", 672, 256);
	SetRectSubRect(frontScrollEnergy, ts_scrollingEnergy->GetSubRect(0));
	SetRectSubRect(backScrollEnergy, ts_scrollingEnergy->GetSubRect(1));

	if (!horizScrollShader1.loadFromFile("Resources/Shader/horizslider.frag", sf::Shader::Fragment))
	{
		cout << "horizslider SHADER NOT LOADING CORRECTLY" << endl;
	}
	horizScrollShader1.setUniform("u_texture", sf::Shader::CurrentTexture);


	if (!horizScrollShader2.loadFromFile("Resources/Shader/horizslider.frag", sf::Shader::Fragment))//("Shader/horizslider.frag", sf::Shader::Fragment))
	{
		cout << "horizslider SHADER NOT LOADING CORRECTLY" << endl;
	}
	horizScrollShader2.setUniform("u_texture", sf::Shader::CurrentTexture);


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
	sectors = NULL;
	sectorCenter = Vector2f(960, 550);
	sectorSelector = NULL;
	mapSelector = NULL;

	int waitFrames[3] = { 30, 15, 10 };
	int waitModeThresh[2] = { 2, 2 };
	mapSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 8, 0);

	//bottomBG.setOrigin(bottomBG.getLocalBounds().width / 2, bottomBG.getLocalBounds().height / 2);
	//shardBG.setOrigin(shardBG.getLocalBounds().width / 2, shardBG.getLocalBounds().height / 2);


	frame = 0;
}

MapSelector::~MapSelector()
{
	delete[] ts_bossFight;
	delete[] ts_sectorOpen;

	if (sectors != NULL)
	{
		for (int i = 0; i < numSectors; ++i)
		{
			delete sectors[i];
		}
		delete[] sectors;
		sectors = NULL;

		delete sectorSelector;

	}
	delete mapSelector;
}

void MapSelector::UpdateSprites()
{

}

void MapSelector::RunSelectedMap()
{
	sectors[sectorSelector->currIndex]->RunSelectedMap();
	ReturnFromMap();
}

void MapSelector::Draw(sf::RenderTarget *target)
{
	for (int i = 0; i < numSectors; ++i)
	{
		//sectors[saSelector->currIndex]->Draw(target);
		sectors[sectorSelector->currIndex]->Draw(target);
	}

	if (kinState != K_HIDE)
	{
		target->draw(kinSprite);
	}

	target->draw(nodeHighlight);

	//return;

	/*sf::RenderStates rs;
	rs.texture = ts_scrollingEnergy->texture;
	rs.shader = &horizScrollShader1;

	target->draw(backScrollEnergy, 4, sf::Quads, rs);

	rs.shader = &horizScrollShader2;

	target->draw(frontScrollEnergy, 4, sf::Quads, rs);*/

}

void MapSelector::ReturnFromMap()
{
	kinState = K_STAND;
	frame = 0;
}

MapSector * MapSelector::GetFocusedSector()
{
	return sectors[sectorSelector->currIndex];
}

bool MapSelector::Update(ControllerState &curr,
	ControllerState &prev)
{
	ControllerState empty;
	MapSector::State currSectorState = GetFocusedSector()->state;

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
		int changed = sectorSelector->UpdateIndex(curr.LLeft(), curr.LRight());
		int numCurrLevels = GetFocusedSector()->unlockedLevelCount;
		if (changed != 0)
		{
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("level_change"));
			mapSelector->SetTotalSize(numCurrLevels);
			GetFocusedSector()->UpdateLevelStats();
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

			if (!sectors[sectorSelector->currIndex]->Update(curr, prev))
			{
				kinState = K_JUMP;
				frame = 0;
			}

		}
		break;
	}
	}

	sectors[sectorSelector->currIndex]->UpdateBG();

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

void MapSelector::UpdateAllInfo(int index)
{
	SaveFile *sf = mainMenu->GetCurrentProgress();
	World & w = sf->worlds[index];

	MapSector *mSector;

	int waitFrames[3] = { 30, 15, 10 };
	int waitModeThresh[2] = { 2, 2 };


	if ((sectors != NULL && numSectors != w.numSectors))
	{
		for (int i = 0; i < numSectors; ++i)
		{
			delete sectors[i];
		}
		delete[] sectors;
		sectors = NULL;

		delete sectorSelector;
		sectorSelector = NULL;
	}

	if (sectors == NULL)
	{
		numSectors = w.numSectors;
		sectors = new MapSector*[numSectors];

		for (int i = 0; i < numSectors; ++i)
		{
			mSector = new MapSector(this, i);
			//load sectors externally
			sectors[i] = mSector;
			mSector->Init(&(w.sectors[i]));
		}

		sectorSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, numSectors, 0);
	}
	else
	{
		for (int i = 0; i < numSectors; ++i)
		{
			sectors[i]->Init(&(w.sectors[i]));
		}
	}

	int scrollHeight = 400 + sectorSelector->currIndex * 120;
	SetRectCenter(backScrollEnergy, 1200, 200, Vector2f(960, scrollHeight));
	SetRectCenter(frontScrollEnergy, 1200, 200, Vector2f(960, scrollHeight));

	mapSelector->SetTotalSize(GetFocusedSector()->unlockedLevelCount);
}

void MapSelector::UpdateHighlight()
{
	MapSector *currSec = sectors[sectorSelector->currIndex];
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