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

	bg = NULL;

	SetRectCenter(lockedOverlayQuad, 1920, 1080, Vector2f(960, 540));
	SetRectColor(lockedOverlayQuad, Color(100, 100, 100, 100));

	SetRectSubRect(levelBG, ms->ts_sectorLevelBG->GetSubRect(0));
	SetRectSubRect(statsBG, ms->ts_levelStatsBG->GetSubRect(0));
	SetRectSubRect(sectorStatsBG, ms->ts_sectorStatsBG->GetSubRect(0));

	ts_energyCircle = mainMenu->tilesetManager.GetTileset("WorldMap/node_energy_circle_80x80.png", 80, 80);
	ts_energyTri = mainMenu->tilesetManager.GetTileset("WorldMap/node_energy_tri_80x80.png", 80, 80);
	ts_energyMask = mainMenu->tilesetManager.GetTileset("WorldMap/node_energy_mask_80x80.png", 80, 80);
	ts_nodeExplode = mainMenu->tilesetManager.GetTileset("WorldMap/nodeexplode_288x288.png", 288, 288);

	nodeExplodeSpr.setTexture(*ts_nodeExplode->texture);
	nodeExplodeSpr.setTextureRect(ts_nodeExplode->GetSubRect(0));
	nodeExplodeSpr.setOrigin(nodeExplodeSpr.getLocalBounds().width / 2, nodeExplodeSpr.getLocalBounds().height / 2);

	endSpr.setTexture(*ms->ts_sectorOpen[0]->texture);
	endSpr.setTextureRect(ms->ts_sectorOpen[0]->GetSubRect(0));

	shardsCollectedText.setFont(mainMenu->arial);
	completionPercentText.setFont(mainMenu->arial);

	shardsCollectedText.setCharacterSize(40);
	shardsCollectedText.setFillColor(Color::White);

	completionPercentText.setCharacterSize(40);
	completionPercentText.setFillColor(Color::White);

	levelPercentCompleteText.setFillColor(Color::White);
	levelPercentCompleteText.setCharacterSize(40);
	levelPercentCompleteText.setFont(mainMenu->arial);

	
	int waitFrames[3] = { 30, 15, 10 };
	int waitModeThresh[2] = { 2, 2 };
	mapSASelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 8, 0);
	
	string worldStr = to_string(sec->worldIndex + 1);
	if (bg == NULL)
	{
		string secStr = to_string(index + 1);
		string bgStr = "w" + worldStr + "_0" + secStr;
		bg = Background::SetupFullBG(bgStr, &(mainMenu->tilesetManager));
	}

	stringstream ss;
	ss.str("");
	ss << "Shard/shards_w" << (index + 1) << "_48x48.png";
	ts_shards = mainMenu->tilesetManager.GetTileset(ss.str(), 48, 48);

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

	SetXCenter(960);
	//UpdateNodes();
	UpdateStats();
	UpdateLevelStats();
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

void MapSector::UpdateUnlockedLevelCount()
{
	unlockedLevelCount = numLevels;
	Level *level;
	for (int i = 0; i < numLevels - 1; ++i)
	{
		level = sec->GetLevel(i);
		if( saveFile->IsCompleteLevel( level ))
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
			DrawStats(target);
			DrawLevelStats(target);
		}
		else
		{
			DrawRequirement(target);
		}
	}

	if (unlocked)
	{
		for (int i = 0; i < numLevels; ++i)
		{
			target->draw(nodes[i]);
		}

		target->draw(endSpr);

		if (state == LEVELJUSTCOMPLETE)
		{
			target->draw(nodeExplodeSpr);
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

	//left = Vector2f(xCenter - 600, 400);
	left = Vector2f(xCenter, 400);
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
	sectorNameText.setPosition(x, 0);
	Vector2f sectorStatsCenter = Vector2f(x + 150, ms->sectorCenter.y - 370);
	Vector2f sectorStatsSize(256, 256);

	endSpr.setPosition(sectorStatsCenter + Vector2f(-72, -74));

	Vector2f levelStatsSize(512, 256);
	Vector2f levelStatsCenter = Vector2f(x, ms->sectorCenter.y + 370);
	Vector2f levelStatsTopLeft = levelStatsCenter - Vector2f(levelStatsSize.x / 2, levelStatsSize.y / 2);
	SetRectCenter(sectorStatsBG, sectorStatsSize.x, sectorStatsSize.y, sectorStatsCenter);
	SetRectCenter(levelBG, 1200, 400, Vector2f(x, ms->sectorCenter.y));
	SetRectCenter(statsBG, levelStatsSize.x, levelStatsSize.y, levelStatsCenter);



	Vector2f sectorStatsTopLeft(sectorStatsCenter.x - sectorStatsSize.x / 2, sectorStatsCenter.y - sectorStatsSize.y / 2);

	requirementText.setPosition(sectorStatsTopLeft.x + 30, sectorStatsTopLeft.y + 30 + 50);

	shardsCollectedText.setPosition(sectorStatsTopLeft.x + 30, sectorStatsTopLeft.y + 30 + 50 * 0);
	completionPercentText.setPosition(sectorStatsTopLeft.x + 30, sectorStatsTopLeft.y + 30 + 50 * 1);

	Vector2f shardGridOffset = Vector2f(20, 20);
	Vector2f gridTopLeft = levelStatsTopLeft + shardGridOffset;
	float gridSpacing = 30;

	float gridTotalRight = gridTopLeft.x + (ts_shards->tileWidth * 4) + (gridSpacing * 3);

	/*for (int i = 0; i < numTotalShards; ++i)
	{
		SetRectCenter(levelCollectedShards + i * 4, 48, 48, Vector2f(gridTopLeft.x + (ts_shards->tileWidth + gridSpacing) * (i % 4) + ts_shards->tileWidth / 2.f,
			gridTopLeft.y + (ts_shards->tileHeight + gridSpacing) * (i / 4) + ts_shards->tileHeight / 2.f));
	}*/

	Vector2f levelStatsActualTopLeft(gridTotalRight, gridTopLeft.y);

	levelPercentCompleteText.setPosition(levelStatsActualTopLeft + Vector2f(50, 50));

	UpdateNodePosition();
}

void MapSector::DrawStats(sf::RenderTarget *target)
{
	target->draw(completionPercentText);
	target->draw(shardsCollectedText);
}

void MapSector::DrawRequirement(sf::RenderTarget *target)
{
	target->draw(requirementText);
}

void MapSector::UpdateStats()
{
	stringstream ss;

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
	ms->mainMenu->AdventureLoadLevel( ms->world->index, GetSelectedAdventureMap(),
		GetSelectedLevel());
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



	if (state == NORMAL || state == COMPLETE || (state == LEVELJUSTCOMPLETE && stateFrame >= 3 * 7))
	{
		int old = GetSelectedIndex();

		bool left = curr.LLeft();
		bool right = curr.LRight();

		int changed = mapSASelector->UpdateIndex(left, right);

		if (changed != 0)
		{
			UpdateLevelStats();
		}

		if (changed != 0)
		{
			ms->mainMenu->soundNodeList->ActivateSound(ms->mainMenu->soundManager.GetSound("level_change"));
			UpdateNodes();
		}

		if (curr.A && !prev.A && saveFile->IsUnlockedSector( sec ))
		{
			if (saveFile->IsCompleteSector(sec) )
			{
				state = COMPLETE;
			}
			else
			{
				state = NORMAL;
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
				//saSelector->currIndex = unlockedIndex + 1;
			}
		}
		else
		{
			//paths[unlockedIndex].setTextureRect(ms->ts_path->GetSubRect(ff));
			if (stateFrame == 0)
			{
				nodeExplodeSpr.setPosition(ms->nodeHighlight.getPosition());
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
	return 0;
}

int MapSector::GetSelectedNodeSubIndex()
{
	return GetNodeSubIndex(GetSelectedIndex());
}

int MapSector::GetSelectedNodeBossFightType()
{
	return 0; //sec->levels[GetSelectedIndex()].bossFightType;
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

	mapSASelector->SetTotalSize(unlockedLevelCount);
}