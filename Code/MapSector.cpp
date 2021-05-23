#include "WorldMap.h"
#include "MainMenu.h"
#include "Background.h"
#include "VectorMath.h"
#include "Enemy_Shard.h"

using namespace std;
using namespace sf;

MapSector::MapSector( AdventureFile &p_adventureFile, Sector *p_sector, MapSelector *p_ms, int index)
	:saveFile( NULL ), numLevels(-1), ms(p_ms), sectorIndex(index), sec( p_sector ),
	adventureFile( p_adventureFile )
{
	unlockedIndex = -1;
	nodeSize = 128;
	pathLen = 16;
	frame = 0;
	nodes = NULL;

	MainMenu *mainMenu = ms->mainMenu;
	WorldMap *worldMap = ms->worldMap;
	bg = NULL;

	ms->ts_statIcons->SetSpriteTexture(mapShardIconSpr);
	ms->ts_statIcons->SetSubRect(mapShardIconSpr, 14);

	ms->ts_statIcons->SetSpriteTexture(mapBestTimeIconSpr);
	ms->ts_statIcons->SetSubRect(mapBestTimeIconSpr, 12);
	
	ms->ts_statIcons->SetSpriteTexture(sectorShardIconSpr);
	ms->ts_statIcons->SetSubRect(sectorShardIconSpr, 14);
	

	SetRectCenter(lockedOverlayQuad, 1920, 1080, Vector2f(960, 540));
	SetRectColor(lockedOverlayQuad, Color(100, 100, 100, 100));

	SetRectSubRect(levelBG, ms->ts_sectorLevelBG->GetSubRect(0));

	SetRectColor(levelStatsBG, Color(0, 0, 0, 100));
	//SetRectSubRect(levelStatsBG, ms->ts_levelStatsBG->GetSubRect(0));
	//SetRectSubRect(sectorStatsBG, ms->ts_sectorStatsBG->GetSubRect(0));

	ts_energyCircle = worldMap->GetTileset("WorldMap/node_energy_circle_80x80.png", 80, 80);
	ts_energyTri = worldMap->GetTileset("WorldMap/node_energy_tri_80x80.png", 80, 80);
	ts_energyMask = worldMap->GetTileset("WorldMap/node_energy_mask_80x80.png", 80, 80);
	ts_nodeExplode = worldMap->GetTileset("WorldMap/nodeexplode_288x288.png", 288, 288);
	ts_sectorArrows = worldMap->GetSizedTileset("Menu/LevelSelect/sector_arrows_64x64.png");
	ts_mapSelectOptions = worldMap->GetSizedTileset("Menu/LevelSelect/map_select_options_384x80.png");
	ts_mapOptionButtons = worldMap->GetSizedTileset("Menu/button_icon_128x128.png");

	ts_levelSelectNumbers = worldMap->GetSizedTileset("Menu/LevelSelect/level_select_number_32x32.png");

	ts_mapSelectOptions->SetQuadSubRect(levelSelectOptionQuads, 0);
	ts_mapSelectOptions->SetQuadSubRect(levelSelectOptionQuads + 4, 1);
	ts_mapSelectOptions->SetQuadSubRect(levelSelectOptionQuads + 8, 2);

	for (int i = 0; i < 8; ++i)
	{
		ts_levelSelectNumbers->SetQuadSubRect(levelNumberQuads + i * 4, i);
	}

	

	nodeExplodeSpr.setTexture(*ts_nodeExplode->texture);
	nodeExplodeSpr.setTextureRect(ts_nodeExplode->GetSubRect(0));
	nodeExplodeSpr.setOrigin(nodeExplodeSpr.getLocalBounds().width / 2, nodeExplodeSpr.getLocalBounds().height / 2);

	endSpr.setTexture(*ms->ts_sectorOpen[0]->texture);
	endSpr.setTextureRect(ms->ts_sectorOpen[0]->GetSubRect(0));

	ts_mapPreview = NULL;
	
	ts_menuSelector = worldMap->GetSizedTileset("Menu/menu_selector_64x64.png");

	selectorSprite.setTexture(*ts_menuSelector->texture);

	selectorAnimFrame = 0;
	sectorArrowFrame = 0;
	selectorAnimDuration = 21;
	selectorAnimFactor = 3;


	bestTimeText.setFont(mainMenu->arial);
	bestTimeText.setCharacterSize(40);
	bestTimeText.setFillColor(Color::White);
	

		
	shardsCollectedText.setFont(mainMenu->arial);
	shardsCollectedText.setCharacterSize(40);
	shardsCollectedText.setFillColor(Color::White);

	numLevelsBeatenText.setFont(mainMenu->arial);
	numLevelsBeatenText.setCharacterSize(40);
	numLevelsBeatenText.setFillColor(Color::White);

	sectorShardsCollectedText.setFont(mainMenu->arial);
	sectorShardsCollectedText.setCharacterSize(40);
	sectorShardsCollectedText.setFillColor(Color::White);

	

	//completionPercentText.setCharacterSize(40);
	//completionPercentText.setFillColor(Color::White);

	//levelPercentCompleteText.setFillColor(Color::White);
	//levelPercentCompleteText.setCharacterSize(40);
	//levelPercentCompleteText.setFont(mainMenu->arial);

	
	int waitFrames[3] = { 30, 15, 10 };
	int waitModeThresh[2] = { 2, 2 };
	mapSASelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 8, 0);
	
	string worldStr = to_string(sec->worldIndex + 1);
	string secStr = to_string(index + 1);
	bgName = "w" + worldStr + "_0" + secStr;

	//stringstream ss;
	//ss.str("");
	//ss << "Shard/shards_w" << (index + 1) << "_48x48.png";
	//ts_shards = worldMap->GetTileset(ss.str(), 48, 48);

	sectorNameText.setFillColor(Color::White);
	sectorNameText.setCharacterSize(60);
	sectorNameText.setFont(mainMenu->arial);
	sectorNameText.setString("Sector " + to_string( index + 1 ));
	sectorNameText.setOrigin(sectorNameText.getLocalBounds().left + sectorNameText.getLocalBounds().width / 2, 0);

	bool blank = mapSASelector == NULL;
	bool diffNumLevels = false;

	numLevels = sec->numLevels;

	Tileset *ts_node = ms->ts_node;
	nodes = new Sprite[numLevels];

	int counter = 0;
	Tileset *currTS = ts_node;
	for (int i = 0; i < numLevels; ++i)
	{
		nodes[i].setTexture(*currTS->texture);
		nodes[i].setTextureRect(currTS->GetSubRect(0));
		nodes[i].setOrigin(nodes[i].getLocalBounds().width / 2, nodes[i].getLocalBounds().height / 2);
	}

	//requirementText
	numRequiredRunes = adventureFile.GetRequiredRunes(sec);

	requirementText.setFont(mainMenu->arial);
	requirementText.setCharacterSize(40);
	requirementText.setFillColor(Color::White);
	requirementText.setString(to_string(numRequiredRunes));

	

	ts_sectorArrows->SetQuadSubRect(sectorArrowQuads, 0);
	ts_sectorArrows->SetQuadSubRect(sectorArrowQuads+4, 0, true );

	mapPreviewHeight = 500 + 50;

	//mapPreviewSpr.setPosition(960, 500);

	//ts_mapPreview = worldMap->GetSizedTileset()

	SetXCenter(960);
	//UpdateNodes();
	//UpdateStats();
	//UpdateLevelStats();

	//UpdateMapPreview();
}

MapSector::~MapSector()
{
	delete mapSASelector;

	if (nodes != NULL)
	{
		delete[] nodes;
		nodes = NULL;
	}

	delete bg;
}

void MapSector::DestroyBG()
{
	assert(bg != NULL);
	delete bg;
	bg = NULL;
}

void MapSector::CreateBG()
{
	assert(bg == NULL);

	bg = Background::SetupFullBG(bgName, ms->worldMap, true);
}

void MapSector::UpdateUnlockedLevelCount()
{
	unlockedLevelCount = numLevels;

	if (ms->worldMap->IsInAllUnlockedMode())
	{
		return;
	}

	Level *level;
	for (int i = 0; i < numLevels - 1; ++i)
	{
		level = sec->GetLevel(i);
		if( !saveFile->IsCompleteLevel( level ))
		{
			unlockedLevelCount = max(1, i + 1);
			break;
		}
	}
}

bool MapSector::IsFocused()
{
	if (ms->sectorSASelector == NULL)
		return false;

	return ms->sectorSASelector->currIndex == sectorIndex;
}

void MapSector::Draw(sf::RenderTarget *target)
{
	bg->Draw(target);

	//if (!saveFile->adventureFile->IsUnlocked()) //later, store this variable instead of 
		//calling the function every frame
	bool unlocked = saveFile->IsUnlockedSector(sec);

	if( !unlocked)
	{
		target->draw(lockedOverlayQuad, 4, sf::Quads);
	}
	
	bool focused = IsFocused();

	if (focused)
	{
		target->draw(sectorNameText);
		if (unlocked)
		{
			target->draw(endSpr);
			if (ms->state == MapSelector::S_SECTORSELECT
				|| ms->state == MapSelector::S_CHANGINGSECTORS)
			{
				DrawStats(target);
				target->draw(sectorArrowQuads, 2 * 4, sf::Quads, ts_sectorArrows->texture);
			}
			else if (ms->state == MapSelector::S_MAPSELECT)
			{
				for (int i = 0; i < numLevels; ++i)
				{
					target->draw(nodes[i]);
				}

				target->draw(nodeHighlight);

				target->draw(selectorSprite);

				

				if (state == LEVELJUSTCOMPLETE)
				{
					target->draw(nodeExplodeSpr);
				}

				DrawLevelStats(target);

				target->draw(mapPreviewSpr);

				target->draw(levelStatsBG, 4, sf::Quads);
				target->draw(mapBestTimeIconSpr);
				target->draw(bestTimeText);
				target->draw(mapShardIconSpr);
				target->draw(shardsCollectedText);

				int numOptionsShown = 0;
				if (ghostAndReplayOn)
				{
					numOptionsShown = 3;
				}
				else
				{
					numOptionsShown = 1;
				}
				target->draw(levelSelectOptionQuads, numOptionsShown * 4, sf::Quads, ts_mapSelectOptions->texture);
				target->draw(levelSelectOptionButtonQuads, numOptionsShown * 4, sf::Quads, ts_mapOptionButtons->texture);

				target->draw(levelNumberQuads, numLevels * 4, sf::Quads, ts_levelSelectNumbers->texture);
			}

			
		}
		else
		{
			DrawRequirement(target);
		}
	}

	
}

void MapSector::DrawLevelStats(sf::RenderTarget *target)
{
	//target->draw(levelCollectedShardsBG, numTotalShards * 4, sf::Quads);
	//target->draw(levelCollectedShards, 16 * 4, sf::Quads, ts_shards->texture);
	//target->draw(levelPercentCompleteText);
}

void MapSector::SetXCenter(float x)
{
	xCenter = x;

	//mapPreviewSpr.setPosition(960, 500+50);

	//left = Vector2f(xCenter - 600, 400);
	left = Vector2f(xCenter, 170 + 50);//150
	int numLevelsPlus = numLevels + 0;
	if (numLevelsPlus % 2 == 0)
	{
		left.x -= pathLen / 2 + nodeSize + (pathLen + nodeSize) * (numLevelsPlus / 2 - 1);
	}
	else
	{
		left.x -= nodeSize / 2 + (pathLen + nodeSize) * (numLevelsPlus / 2);
	}

	


	//sectorNameText.setPosition(Vector2f(x - 150, ms->sectorCenter.y - 370));
	sectorNameText.setPosition(x, 20);
	Vector2f sectorStatsSize(386, 192);
	Vector2f sectorStatsCenter = Vector2f(x, mapPreviewHeight 
		- sectorStatsSize.y/2);//ms->sectorCenter.y - 370);
	
	
	



	



	sf::FloatRect sectorNameGlobalBounds = sectorNameText.getGlobalBounds();

	Vector2f sectorNameTopRight(sectorNameGlobalBounds.left + sectorNameGlobalBounds.width, sectorNameGlobalBounds.top + sectorNameGlobalBounds.height / 2);

	endSpr.setOrigin(endSpr.getLocalBounds().width / 2, endSpr.getLocalBounds().height / 2);
	endSpr.setPosition(sectorNameTopRight + Vector2f( 70 + 15, 0 ) );

	FloatRect endGB = endSpr.getGlobalBounds();
	float arrowSpacing = 64;
	float left = sectorNameGlobalBounds.left - arrowSpacing;
	float right = endGB.left + endGB.width
		+ arrowSpacing - 100;
	float y = sectorNameGlobalBounds.top + sectorNameGlobalBounds.height / 2;

	float testCenter = (left + right) / 2.f;
	float diff = 960 - testCenter;

	Vector2f vDiff(diff, 0);

	sectorNameText.move(vDiff);
	endSpr.move(vDiff);

	SetRectCenter(sectorArrowQuads, 64, 64, Vector2f(left + diff, y));
	SetRectCenter(sectorArrowQuads + 4, 64, 64, Vector2f(right + diff, y));


	Vector2f levelStatsSize(256, 192);

	Vector2f sectorStatsTopLeft = sectorStatsCenter - sectorStatsSize / 2.f;
	numLevelsBeatenText.setPosition(sectorStatsTopLeft + Vector2f(10, 10));
	sectorShardIconSpr.setPosition(sectorStatsTopLeft + Vector2f(-30, 96 - 20));
	sectorShardsCollectedText.setPosition(sectorStatsTopLeft + Vector2f(100, 96));

	SetRectCenter(sectorStatsBG, sectorStatsSize.x, sectorStatsSize.y, sectorStatsCenter);
	SetRectColor(sectorStatsBG, Color(0, 0, 0, 200));


	SetRectCenter(levelBG, 1200, 400, Vector2f(x, ms->sectorCenter.y));

	//top right of mappreview
	Vector2f levelStatsTopLeft = Vector2f( 960, 550 ) + Vector2f(912 / 2, -492 / 2) + Vector2f( 20, 0 );

	Vector2f levelSelectOptionsTopLeft = levelStatsTopLeft + Vector2f(0, levelStatsSize.y + 28);
	Vector2f buttonIconOffset(320, 2);
	SetRectTopLeft(levelSelectOptionQuads, 384, 80, levelSelectOptionsTopLeft);
	SetRectTopLeft(levelSelectOptionButtonQuads, 64, 64, levelSelectOptionsTopLeft + buttonIconOffset);

	SetRectTopLeft(levelSelectOptionQuads + 4, 384, 80, levelSelectOptionsTopLeft + Vector2f( 0, 100 ));
	SetRectTopLeft(levelSelectOptionButtonQuads + 4, 64, 64, levelSelectOptionsTopLeft + Vector2f(0, 100) + buttonIconOffset);

	SetRectTopLeft(levelSelectOptionQuads + 8, 384, 80, levelSelectOptionsTopLeft + Vector2f( 0, 200 ));
	SetRectTopLeft(levelSelectOptionButtonQuads + 8, 64, 64, levelSelectOptionsTopLeft + Vector2f(0, 200) + buttonIconOffset);

	

	SetRectTopLeft(levelStatsBG, 256 + 48, 192, levelStatsTopLeft); 

	mapShardIconSpr.setPosition(levelStatsTopLeft + Vector2f( -15, 96 ));
	mapBestTimeIconSpr.setPosition(levelStatsTopLeft + Vector2f( -20, 0 ) );

	bestTimeText.setPosition(mapBestTimeIconSpr.getPosition() + Vector2f(96 + 60, 20));
	shardsCollectedText.setPosition(mapShardIconSpr.getPosition() + Vector2f(96 + 60, 20));

	requirementText.setPosition(sectorStatsTopLeft.x + 30, sectorStatsTopLeft.y + 30 + 50);

	//shardsCollectedText.setPosition(sectorStatsTopLeft.x + 30, sectorStatsTopLeft.y + 30 + 50 * 0);
	//completionPercentText.setPosition(sectorStatsTopLeft.x + 30, sectorStatsTopLeft.y + 30 + 50 * 1);

	//Vector2f shardGridOffset = Vector2f(20, 20);
	//Vector2f gridTopLeft = levelStatsTopLeft + shardGridOffset;
	//float gridSpacing = 30;

	//float gridTotalRight = gridTopLeft.x + (ts_shards->tileWidth * 4) + (gridSpacing * 3);

	/*for (int i = 0; i < numTotalShards; ++i)
	{
		SetRectCenter(levelCollectedShards + i * 4, 48, 48, Vector2f(gridTopLeft.x + (ts_shards->tileWidth + gridSpacing) * (i % 4) + ts_shards->tileWidth / 2.f,
			gridTopLeft.y + (ts_shards->tileHeight + gridSpacing) * (i / 4) + ts_shards->tileHeight / 2.f));
	}*/

	//Vector2f levelStatsActualTopLeft(gridTotalRight, gridTopLeft.y);

	//levelPercentCompleteText.setPosition(levelStatsActualTopLeft + Vector2f(50, 50));

	UpdateNodePosition();

	for (int i = 0; i < numLevels; ++i)
	{
		SetRectCenter(levelNumberQuads + i * 4, 32, 32, 
			GetNodePos(i) + Vector2f( 0, -68));
	}
	
}

void MapSector::DrawStats(sf::RenderTarget *target)
{
	target->draw(sectorStatsBG, 4, sf::Quads);
	target->draw(numLevelsBeatenText);
	target->draw(sectorShardIconSpr);
	target->draw(sectorShardsCollectedText);
	//target->draw(completionPercentText);
	//target->draw(shardsCollectedText);
}

void MapSector::DrawRequirement(sf::RenderTarget *target)
{
	target->draw(requirementText);
}

void MapSector::UpdateStats()
{
	//numLevelsBeatenText
	int numLevels = sec->numLevels;
	int numLevelsCompleted = 0;
	for (int i = 0; i < numLevels; ++i)
	{
		if (saveFile->IsCompleteLevel(sec->GetLevel(i)))
		{
			++numLevelsCompleted;
		}
	}

	stringstream ss;
	ss << "Levels: " << numLevelsCompleted << "/" << numLevels;

	numLevelsBeatenText.setString(ss.str());

	int totalShards = 0;
	int numCaptured = 0;

	int numShardsPerLevel = 0;
	for (int i = 0; i < numLevels; ++i)
	{
		AdventureMapHeaderInfo &amhi =
			adventureFile.GetMapHeaderInfo(sec->GetLevelIndex(i));
		numShardsPerLevel = amhi.shardInfoVec.size();
		totalShards += numShardsPerLevel;
		for (int j = 0; j < numShardsPerLevel; ++j)
		{
			if (saveFile->ShardIsCaptured(amhi.shardInfoVec[j].GetTrueIndex()))
			{
				++numCaptured;
			}
		}
	}
	
	ss.str("");
	ss.clear();

	ss << numCaptured << "/" << totalShards;

	sectorShardsCollectedText.setString(ss.str());

	/*int numTotalShards = adventureFile.GetAdventureSector(sec).hasShardField.GetOnCount();
	int numShardsCaptured = saveSector->GetNumShardsCaptured();

	ss << numShardsCaptured << " / " << numTotalShards;

	shardsCollectedText.setString(ss.str());

	ss.str("");

	int percent = floor(saveSector->GetCompletionPercentage());

	ss << percent << "%";

	completionPercentText.setString(ss.str());*/
}

int MapSector::GetSelectedIndex()
{
	return mapSASelector->currIndex;
}

Level * MapSector::GetSelectedLevel()
{
	return sec->GetLevel(GetSelectedIndex());
}

AdventureMap *MapSector::GetSelectedAdventureMap()
{
	return &adventureFile.GetMap(sec->GetLevelIndex(GetSelectedIndex()));
}

void MapSector::UpdateLevelStats()
{
	Level *level = GetSelectedLevel();

	int recordScore =
		saveFile->GetBestFramesLevel(level->index);

	if (recordScore > 0)
	{
		bestTimeText.setString(GetTimeStr(recordScore));

		ghostAndReplayOn = true;
	}
	else
	{
		bestTimeText.setString("-----");

		ghostAndReplayOn = false;
	}

	AdventureMapHeaderInfo &headerInfo = 
		saveFile->adventureFile->GetMapHeaderInfo(level->index);

	

	int totalNumShards = headerInfo.shardInfoVec.size();


	int numCollected = 0;
	for (auto it = headerInfo.shardInfoVec.begin(); it !=
		headerInfo.shardInfoVec.end(); ++it)
	{
		if (saveFile->ShardIsCaptured((*it).GetTrueIndex()))
		{
			numCollected++;
		}
	}

	stringstream ss;
	ss << numCollected << "/" << totalNumShards;

	shardsCollectedText.setString(ss.str());



	//int gridIndex = 0;
	//int uncaptured = 0;
	//Level &lev = GetSelectedLevel();
	//auto &snList = lev.shardNameList;
	////numTotalShards = snList.size();
	//for (int i = 0; i < numTotalShards; ++i)
	//{
	//	//SetRectColor(levelCollectedShardsBG + i * 4, Color(Color::Transparent));
	//	SetRectSubRect(levelCollectedShards + i * 4, IntRect());
	//}

	//for (auto it = snList.begin(); it != snList.end(); ++it)
	//{
	//	int sType = Shard::GetShardType((*it));
	//	int subRectIndex = sType % 22;
	//	if (saveSector->world->sf->ShardIsCaptured(sType))
	//	{
	//		SetRectSubRect(levelCollectedShards + gridIndex * 4, ts_shards->GetSubRect(subRectIndex));
	//	}
	//	else
	//	{
	//		SetRectSubRect(levelCollectedShards + gridIndex * 4, IntRect());//ts_shards->GetSubRect(44));
	//	}

	//	//SetRectColor(levelCollectedShardsBG + gridIndex * 4, Color(Color::Black));
	//	++gridIndex;
	//}

	//stringstream ss;
	//int percent = floor(GetSelectedLevel().GetCompletionPercentage());

	//ss << percent << "%";

	//Text &currLevelTimeText = ms->mainMenu->worldMap->currLevelTimeText;

	//string levelTimeStr = "Fastest Level Time  ";
	//if (lev.bestTimeFrames < 0)
	//{
	//	currLevelTimeText.setString(levelTimeStr + string("-- : --"));
	//}
	//else
	//{
	//	currLevelTimeText.setString(levelTimeStr + GetTimeStr(lev.bestTimeFrames));
	//}
}

sf::Vector2f MapSector::GetNodePos(int node)
{
	return left + Vector2f(nodeSize / 2, 0) + Vector2f((pathLen + nodeSize) * node, 0);
}

sf::Vector2f MapSector::GetSelectedNodePos()
{
	return GetNodePos(GetSelectedIndex());
}

void MapSector::UpdateNodePosition()
{
	for (int i = 0; i < numLevels; ++i)
	{
		nodes[i].setPosition(GetNodePos(i));
	}
}

void MapSector::RunSelectedMap()
{
	ms->mainMenu->gameRunType = MainMenu::GRT_ADVENTURE;

	LevelLoadParams loadParams;
	loadParams.adventureMap = GetSelectedAdventureMap();
	loadParams.bestTimeGhostOn = bestTimeGhostOn;
	loadParams.bestReplayOn = bestReplayOn;
	loadParams.level = GetSelectedLevel();
	loadParams.loadingScreenOn = true;
	loadParams.world = ms->world->index;

	ms->mainMenu->AdventureLoadLevel( loadParams );
}

int MapSector::GetNumLevels()
{
	return numLevels;
}

void MapSector::UpdateBG()
{
	bg->Update(Vector2f(960, 540));
}

bool MapSector::Update(ControllerState &curr,
	ControllerState &prev)
{
	UpdateUnlockedLevelCount();

	if (unlockedLevelCount != GetNumLevels())
	{
		//saSelector->SetTotalSize(unlockedLevelCount);
	}

	if (state == LEVELCOMPLETEDWAIT)
	{
		if (stateFrame == 30)
		{
			state = LEVELJUSTCOMPLETE;
			stateFrame = 0;
		}
	}

	if (state == NORMAL)
	{
		int lastBeaten = -1;
		for (int i = 0; i < numLevels; ++i)
		{
			if (saveFile->IsLevelJustBeaten( sec->GetLevel( i ) ) )
			{
				state = LEVELCOMPLETEDWAIT;

				stateFrame = 0;
				unlockedIndex = i;//saSelector->currIndex;
								  //unlockFrame = frame;
				saveFile->SetLevelNotJustBeaten(sec->GetLevel(i));
				lastBeaten = i;
				//break;
			}
		}
		if (lastBeaten >= 0)
		{
			/*if (lastBeaten < saSelector->totalItems - 1)
			++lastBeaten;
			saSelector->currIndex = lastBeaten;*/
		}
	}

	UpdateNodes();

	if (state == NORMAL || state == COMPLETE 
		|| (state == LEVELJUSTCOMPLETE && stateFrame >= 3 * 7))
	{
		int old = GetSelectedIndex();

		bool left = curr.LLeft();
		bool right = curr.LRight();

		int changed = mapSASelector->UpdateIndex(left, right);

		if (changed != 0)
		{

			UpdateLevelStats();
			UpdateMapPreview();

			
		}

		if (changed != 0)
		{
			ms->mainMenu->soundNodeList->ActivateSound(ms->mainMenu->soundManager.GetSound("level_change"));
			UpdateNodes();
		}

		bool aPress = (curr.A && !prev.A);
		bool yPress = (curr.Y && !prev.Y) && ghostAndReplayOn;
		bool r1Press = (curr.rightShoulder && !prev.rightShoulder) && ghostAndReplayOn;

		if ( (aPress || yPress || r1Press) && saveFile->IsUnlockedSector( sec ))
		{
			//no idea wtf this does
			if (saveFile->IsCompleteSector(sec) )
			{
				state = COMPLETE;
			}
			else
			{
				state = NORMAL;
			}

			bestTimeGhostOn = false;
			bestReplayOn = false;

			if (aPress)
			{

			}
			else if (yPress)
			{
				bestTimeGhostOn = true;
			}
			else if (r1Press)
			{
				bestReplayOn = true;
			}

			

			ms->mainMenu->soundNodeList->ActivateSound(ms->mainMenu->soundManager.GetSound("level_select"));

			return false;
		}
	}

	//UpdateHighlight();

	if (state == LEVELJUSTCOMPLETE)//unlockedIndex != -1)
	{
		int ff = stateFrame / 7;
		if (ff == 13)
		{
			bool secOver = saveFile->IsCompleteSector(sec);

			if (secOver)
			{
				state = EXPLODECOMPLETE;
			}
			else
			{
				state = NORMAL;
				//unlockedIndex
			}

			stateFrame = 0;
		}
		else if (ff == 3)
		{
			nodeExplodeSpr.setTextureRect(ts_nodeExplode->GetSubRect(ff));
			if (unlockedLevelCount < numLevels - 1)
			{
				mapSASelector->currIndex = unlockedLevelCount - 1;
				UpdateLevelStats();
				UpdateMapPreview();
				//saSelector->currIndex = unlockedIndex + 1;
			}
		}
		else
		{
			//paths[unlockedIndex].setTextureRect(ms->ts_path->GetSubRect(ff));
			if (stateFrame == 0)
			{
				nodeExplodeSpr.setPosition(nodeHighlight.getPosition());
			}
			nodeExplodeSpr.setTextureRect(ts_nodeExplode->GetSubRect(ff));
		}

	}



	if (state == EXPLODECOMPLETE)
	{
		int explodeFactor = 4;
		if (stateFrame == 16 * explodeFactor)
		{
			state = COMPLETE;
			stateFrame = 0;
			endSpr.setTextureRect(ms->ts_sectorOpen[0]->GetSubRect(15));
			//endSpr.setOrigin(endSpr.getLocalBounds().width / 2, endSpr.getLocalBounds().width / 2);
		}
		else
		{
			endSpr.setTextureRect(ms->ts_sectorOpen[0]->GetSubRect(stateFrame / explodeFactor));
			//endSpr.setOrigin(endSpr.getLocalBounds().width / 2, endSpr.getLocalBounds().width / 2);
		}
	}

	UpdateHighlight();
	//UpdateSelectorSprite();

	++frame;
	++stateFrame;

	return true;
}

void MapSector::UpdateNodes()
{
	Tileset *ts_node = ms->ts_node;
	for (int i = 0; i < numLevels; ++i)
	{
		Tileset *currTS = NULL;
		
		int bFType = adventureFile.GetMapHeaderInfo(sec->GetLevelIndex(i)).mapType;
		if (bFType == 0)
		{
			currTS = ts_node;
		}
		else
		{
			currTS = ms->ts_bossFight[bFType - 1];
		}
		currTS->SetSpriteTexture(nodes[i]);
		nodes[i].setTextureRect(currTS->GetSubRect(GetNodeSubIndex(i)));
		//SetRectSubRect(n, ms->ts_node->GetSubRect(GetNodeSubIndex(i)));
	}
}

void MapSector::Load()
{

}

void MapSector::DestroyMapPreview()
{
	//cout << "destroyed preview" << endl;
	if (ts_mapPreview != NULL)
	{
		ms->worldMap->DestroyTileset(ts_mapPreview);
		ts_mapPreview = NULL;
	}
}

void MapSector::UpdateMapPreview()
{
	DestroyMapPreview();

	string fPath = adventureFile.GetAdventureSector(sec).maps[GetSelectedIndex()].GetFilePath();
	string previewPath = fPath + ".png";

	ts_mapPreview = ms->worldMap->GetTileset(previewPath, 912, 492);
	mapPreviewSpr.setTexture(*ts_mapPreview->texture);
	mapPreviewSpr.setOrigin(mapPreviewSpr.getLocalBounds().width / 2, mapPreviewSpr.getLocalBounds().height / 2);
	mapPreviewSpr.setPosition(960, mapPreviewHeight);
}

int MapSector::GetNodeSubIndex(int node)
{
	Level *lev = sec->GetLevel(node);

	int bType = adventureFile.GetMapHeaderInfo(lev->index).mapType;
	int res;

	bool isUnlocked = saveFile->IsUnlockedLevel(sec, node );
	bool isComplete = saveFile->IsCompleteLevel(lev);
	bool isFullyComplete = saveFile->IsFullyCompleteLevel(lev);

	if (bType == 0)
	{
		if (!isUnlocked)
		{
			res = 0;
		}
		else
		{
			if (IsFocused() && GetSelectedIndex() == node)
			{
				if (isFullyComplete)
				{
					res = 8;
				}
				else if (isComplete)
				{
					res = 7;
				}
				else
				{
					res = 6;
				}

				if (state == LEVELCOMPLETEDWAIT || (state == LEVELJUSTCOMPLETE && stateFrame < 3 * 7))
				{
					res = 6;//--res;
				}
			}
			else
			{
				if (isFullyComplete)
				{
					res = 5;
				}
				else if( isComplete)
				{
					res = 4;
				}
				else
				{
					res = 3;
				}
			}
		}
	}
	else
	{
		if (!isUnlocked)
		{
			res = 0;
		}
		else
		{
			if (IsFocused() && GetSelectedIndex() == node)
			{
				if (isFullyComplete)
				{
					res = 2;
				}
				else if (isComplete)
				{
					res = 2;
				}
				else
				{
					res = 5;
				}

				if (state == LEVELCOMPLETEDWAIT || (state == LEVELJUSTCOMPLETE && stateFrame < 3 * 7))
				{
					res = 5;
					//--res;
				}
			}
			else
			{
				if (isFullyComplete)
				{
					res = 1;
				}
				else if (isComplete)
				{
					res = 1;
				}
				else
				{
					res = 4;
				}
			}
		}
	}
	//return res;
	return res;
}

int MapSector::GetSelectedNodeSubIndex()
{
	return GetNodeSubIndex(GetSelectedIndex());
}

int MapSector::GetSelectedNodeBossFightType()
{
	return 0; //sec->levels[GetSelectedIndex()].bossFightType;
}

void MapSector::UpdateSelectorSprite()
{
	ts_menuSelector->SetSubRect(selectorSprite, selectorAnimFrame / selectorAnimFactor, true);
	//selectorSprite.setTextureRect(ts_menuSelector->GetSubRect(selectorAnimFrame / selectorAnimFactor));
	selectorSprite.setOrigin(selectorSprite.getLocalBounds().width / 2, selectorSprite.getLocalBounds().height / 2);
	selectorSprite.setRotation(-90);

	selectorAnimFrame++;
	if (selectorAnimFrame == selectorAnimDuration * selectorAnimFactor)
	{
		selectorAnimFrame = 0;
	}

	selectorSprite.setPosition(GetSelectedNodePos()
		+ Vector2f(0, 50));
}

void MapSector::UpdateOptionButtons()
{
	ControllerType ct = ms->mainMenu->GetController(0).GetCType();
	int tileOffset = 0;
	switch (ct)
	{
	case ControllerType::CTYPE_XBOX:
		tileOffset = 0;
		break;
	case ControllerType::CTYPE_PS5:
		tileOffset = 16 * 1;
		break;
	case ControllerType::CTYPE_GAMECUBE:
		tileOffset = 16 * 2;
		break;
	}

	int buttonIndex = 0;

	ts_mapOptionButtons->SetQuadSubRect(levelSelectOptionButtonQuads, tileOffset + 0);
	ts_mapOptionButtons->SetQuadSubRect(levelSelectOptionButtonQuads+4, tileOffset + 4);
	ts_mapOptionButtons->SetQuadSubRect(levelSelectOptionButtonQuads+8, tileOffset + 3);

	
}

void MapSector::UpdateSectorArrows()
{
	int tile = (sectorArrowFrame / 60) % 2;

	if (tile == 1)
	{
		tile = 2;
	}

	if (ms->mainMenu->GetCurrInputUnfiltered(0).LLeft())
	{
		ts_sectorArrows->SetQuadSubRect(sectorArrowQuads, tile + 1);
	}
	else
	{
		ts_sectorArrows->SetQuadSubRect(sectorArrowQuads, tile);
	}

	if (ms->mainMenu->GetCurrInputUnfiltered(0).LRight())
	{
		ts_sectorArrows->SetQuadSubRect(sectorArrowQuads + 4, tile + 1, true);
	}
	else
	{
		ts_sectorArrows->SetQuadSubRect(sectorArrowQuads + 4, tile, true);
	}
	/*if (tile == 1)
	{
		tile = 2;
	}*/

	
	

	sectorArrowFrame++;
}

void MapSector::Init(SaveFile *p_saveFile)
{
	saveFile = p_saveFile;
	if (saveFile->IsCompleteSector(sec) )
	{
		state = COMPLETE;
		endSpr.setTextureRect(ms->ts_sectorOpen[0]->GetSubRect(15));
	}
	else
	{
		state = NORMAL;
	}

	//auto &snList = GetSelectedLevel().shardNameList;
	//numTotalShards = snList.size();

	//levelCollectedShards = new Vertex[numTotalShards * 4];
	//levelCollectedShardsBG = new Vertex[numTotalShards * 4];

	SetXCenter(960);

	UpdateNodes();
	UpdateStats();
	UpdateLevelStats();
	UpdateUnlockedLevelCount();
	

	//selectorSprite.setPosition(GetSelectedNodePos()
	//	+ Vector2f(0, 50));

	mapSASelector->SetTotalSize(unlockedLevelCount);
}

void MapSector::UpdateHighlight()
{
	nodeHighlight.setPosition(GetSelectedNodePos());
	int n = GetSelectedNodeSubIndex();


	int bossFightT = GetSelectedNodeBossFightType();//sec->levels[saSelector->currIndex].bossFightType;

	switch (bossFightT)
	{
	case 0:
		nodeHighlight.setTexture(*ms->ts_node->texture);
		nodeHighlight.setTextureRect(ms->ts_node->GetSubRect(9 + (n % 3)));
		break;
	case 1:
		nodeHighlight.setTexture(*ms->ts_bossFight[0]->texture);
		nodeHighlight.setTextureRect(ms->ts_bossFight[0]->GetSubRect(6));
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