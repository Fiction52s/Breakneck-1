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

	kinJumpFrame = 0;
	kinJumpSprite.setPosition(960, 540 + 300);

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
	//numSectors = 7;
	//MapSector *ms;
	sectorCenter = Vector2f(960, 550);/*bottomBG.getPosition()
									  + Vector2f(bottomBG.getLocalBounds().width / 2,
									  bottomBG.getLocalBounds().height / 2);*/

	sectorSelector = NULL;
	mapSelector = NULL;
	//slideDuration = 50;
	////bottomCenter.x = 0;
	//sectors = new MapSector*[numSectors];
	//for (int i = 0; i < numSectors; ++i)
	//{
	//	ms = new MapSector( this, 6 );
	//	//load sectors externally
	//	sectors[i] = ms;
	//	ms->SetCenter(bottomCenter);
	//	//ms->ts_thumb 
	//	
	//}

	//int waitFrames[3] = { 10, 5, 2 };
	int waitFrames[3] = { 30, 15, 10 };
	int waitModeThresh[2] = { 2, 2 };
	mapSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 8, 0);

	//bottomBG.setOrigin(bottomBG.getLocalBounds().width / 2, bottomBG.getLocalBounds().height / 2);
	//thumbnailBG.setOrigin(thumbnailBG.getLocalBounds().width / 2, thumbnailBG.getLocalBounds().height / 2);
	//shardBG.setOrigin(shardBG.getLocalBounds().width / 2, shardBG.getLocalBounds().height / 2);


	frame = 0;



	//saSelectorLevel = new SingleAxisSelector(numLevelsInWorld, waitFrames, 2, waitModeThresh, 0, 0);
	//Tileset *ts_thumb = mm->tilesetManager.GetTileset("WorldMap/thumbnail01.png", 256, 256);
	//thumbnailSpr.setTexture(ts_thumb);

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
}

void MapSelector::Draw(sf::RenderTarget *target)
{
	for (int i = 0; i < numSectors; ++i)
	{
		//sectors[saSelector->currIndex]->Draw(target);
		sectors[sectorSelector->currIndex]->Draw(target);
	}

	target->draw(kinJumpSprite);

	target->draw(nodeHighlight);

	//return;

	/*sf::RenderStates rs;
	rs.texture = ts_scrollingEnergy->texture;
	rs.shader = &horizScrollShader1;

	target->draw(backScrollEnergy, 4, sf::Quads, rs);

	rs.shader = &horizScrollShader2;

	target->draw(frontScrollEnergy, 4, sf::Quads, rs);*/

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
		if (curr.B && !prev.B)
		{
			state = S_SECTORSELECT;
			break;
		}

		sectors[sectorSelector->currIndex]->Update(curr, prev);
		break;
	}
	}

	sectors[sectorSelector->currIndex]->UpdateBG();

	/*for (int i = 0; i < numSectors; ++i)
	{
		if (sectorSelector->currIndex == i)
			continue;

		sectors[i]->UpdateNodes();
	}*/

	UpdateHighlight();
	//else if (state == S_SLIDINGLEFT)
	//{
	//	if (frame == slideDuration+1)
	//	{
	//		state = S_IDLE;
	//		frame = 0;
	//		//sectors[saSelector->currIndex]->SetXCenter(sectorCenter.x);
	//	}
	//	else
	//	{
	//		float diff = 1920.f * (float)frame / slideDuration;
	//		float oldCenter = sectorCenter.x + diff;
	//		float newCenter = -960.f + diff;
	//		sectors[saSelector->oldCurrIndex]->SetXCenter(oldCenter);
	//		sectors[saSelector->currIndex]->SetXCenter(newCenter);
	//	}
	//	
	//}
	//else if (state == S_SLIDINGRIGHT)
	//{
	//	if (frame == slideDuration+1)
	//	{
	//		state = S_IDLE;
	//		frame = 0;
	//		//sectors[saSelector->currIndex]->SetXCenter(sectorCenter.x);
	//	}
	//	else
	//	{
	//		float diff = 1920.f * (float)frame / slideDuration;
	//		float oldCenter = sectorCenter.x - diff;
	//		float newCenter = (1920.f + 960.f) - diff;
	//		sectors[saSelector->oldCurrIndex]->SetXCenter(oldCenter);
	//		sectors[saSelector->currIndex]->SetXCenter(newCenter);
	//	}
	//	
	//}

	int scrollHeight = 400 + sectorSelector->currIndex * 120;
	SetRectCenter(backScrollEnergy, 1200, 100, Vector2f(960, scrollHeight));
	SetRectCenter(frontScrollEnergy, 1200, 100, Vector2f(960, scrollHeight));

	int scrollFramesBack = 180;
	int scrollFramesFront = 360;
	float fBack = frame % scrollFramesBack;
	float fFront = frame % scrollFramesFront;

	int mult = frame / scrollFramesBack;
	int multFront = frame / scrollFramesFront;

	float facBack = fBack / (scrollFramesBack);
	float facFront = fFront / (scrollFramesFront);

	horizScrollShader1.setUniform("quant", -facBack);
	horizScrollShader2.setUniform("quant", -facFront);

	++kinJumpFrame;
	if (kinJumpFrame == 66 * 2)
	{
		kinJumpFrame = 0;
	}

	int kinMult = 2;
	int kinTex = (kinJumpFrame / kinMult) / 16;
	int realKinFrame = (kinJumpFrame / kinMult) % 16;

	ts_kinJump[kinTex]->SetSpriteTexture(kinJumpSprite);
	ts_kinJump[kinTex]->SetSubRect(kinJumpSprite, realKinFrame);
	kinJumpSprite.setOrigin(kinJumpSprite.getLocalBounds().width / 2,
		kinJumpSprite.getLocalBounds().height / 2);

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