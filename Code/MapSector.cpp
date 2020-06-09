#include "WorldMap.h"
#include "MainMenu.h"
#include "Background.h"
#include "VectorMath.h"
#include "Enemy_Shard.h"

using namespace std;
using namespace sf;

MapSector::MapSector(MapSelector *p_ms, int index)
	:numLevels(-1), ms(p_ms), sectorIndex(index)
{
	unlockedIndex = -1;
	numUnlockConditions = -1;
	nodeSize = 128;
	pathLen = 16;
	frame = 0;
	nodes = NULL;
	unlockCondText = NULL;

	MainMenu *mainMenu = ms->mainMenu;

	bg = NULL;

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

	sectorNameText.setFillColor(Color::White);
	sectorNameText.setCharacterSize(40);
	sectorNameText.setFont(mainMenu->arial);
	sectorNameText.setOrigin(sectorNameText.getLocalBounds().left + sectorNameText.getLocalBounds().width / 2, 0);
}

MapSector::~MapSector()
{
	delete[] levelCollectedShards;
	delete[] levelCollectedShardsBG;


	if (nodes != NULL)
	{
		delete[] nodes;
		nodes = NULL;
	}

	if (topBonusNodes != NULL)
	{
		delete[] topBonusNodes;
		topBonusNodes = NULL;
	}

	if (botBonusNodes != NULL)
	{
		delete[] botBonusNodes;
		botBonusNodes = NULL;
	}

	if (unlockCondText != NULL)
	{
		delete[]unlockCondText;
	}

	delete bg;
}

void MapSector::UpdateUnlockedLevelCount()
{
	unlockedLevelCount = numLevels;
	for (int i = 0; i < numLevels - 1; ++i)
	{
		if (!sec->levels[i].GetComplete())
		{
			unlockedLevelCount = max(1, i + 1);
			break;
		}
	}
}

bool MapSector::IsFocused()
{
	if (ms->sectorSelector == NULL)
		return false;

	return ms->sectorSelector->currIndex == sectorIndex;
}

void MapSector::Draw(sf::RenderTarget *target)
{
	bg->Draw(target);

	bool focused = IsFocused();
	bool unlocked = sec->IsUnlocked();

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
			DrawUnlockConditions(target);
		}



	}

	if (unlocked)
	{
		for (int i = 0; i < numLevels; ++i)
		{
			target->draw(topBonusNodes[i]);
			target->draw(nodes[i]);
			target->draw(botBonusNodes[i]);
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
	target->draw(levelCollectedShardsBG, numTotalShards * 4, sf::Quads);
	target->draw(levelCollectedShards, numTotalShards * 4, sf::Quads, ts_shards->texture);
	target->draw(levelPercentCompleteText);
}

void MapSector::SetXCenter(float x)
{
	xCenter = x;

	left = Vector2f(xCenter - 600, 400);
	int numLevelsPlus = numLevels + 0;
	if (numLevelsPlus % 2 == 0)
	{
		//left.x -= pathLen / 2 + nodeSize + (pathLen + nodeSize) * (numLevelsPlus / 2 - 1);
	}
	else
	{
		//left.x -= nodeSize / 2 + (pathLen + nodeSize) * (numLevelsPlus / 2);
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

	int numUnlock = sec->numUnlockConditions;
	for (int i = 0; i < numUnlock; ++i)
	{
		unlockCondText[i].setPosition(sectorStatsTopLeft.x + 30, sectorStatsTopLeft.y + 30 + 50 * i);
	}

	shardsCollectedText.setPosition(sectorStatsTopLeft.x + 30, sectorStatsTopLeft.y + 30 + 50 * 0);
	completionPercentText.setPosition(sectorStatsTopLeft.x + 30, sectorStatsTopLeft.y + 30 + 50 * 1);



	Vector2f shardGridOffset = Vector2f(20, 20);
	Vector2f gridTopLeft = levelStatsTopLeft + shardGridOffset;
	float gridSpacing = 30;

	float gridTotalRight = gridTopLeft.x + (ts_shards->tileWidth * 4) + (gridSpacing * 3);

	for (int i = 0; i < numTotalShards; ++i)
	{
		//shardSpr[i].setPosition();
		SetRectCenter(levelCollectedShards + i * 4, 48, 48, Vector2f(gridTopLeft.x + (ts_shards->tileWidth + gridSpacing) * (i % 4) + ts_shards->tileWidth / 2.f,
			gridTopLeft.y + (ts_shards->tileHeight + gridSpacing) * (i / 4) + ts_shards->tileHeight / 2.f));
		SetRectCenter(levelCollectedShardsBG + i * 4, 48, 48, Vector2f(gridTopLeft.x + (ts_shards->tileWidth + gridSpacing) * (i % 4) + ts_shards->tileWidth / 2.f,
			gridTopLeft.y + (ts_shards->tileHeight + gridSpacing) * (i / 4) + ts_shards->tileHeight / 2.f));
	}

	Vector2f levelStatsActualTopLeft(gridTotalRight, gridTopLeft.y);

	levelPercentCompleteText.setPosition(levelStatsActualTopLeft + Vector2f(50, 50));


	UpdateNodePosition();
}

void MapSector::DrawStats(sf::RenderTarget *target)
{
	target->draw(completionPercentText);
	target->draw(shardsCollectedText);
}

void MapSector::DrawUnlockConditions(sf::RenderTarget *target)
{
	int numUnlock = sec->numUnlockConditions;
	for (int i = 0; i < numUnlock; ++i)
	{
		target->draw(unlockCondText[i]);
	}
}

void MapSector::UpdateStats()
{
	stringstream ss;

	int numTotalShards = sec->GetNumTotalShards();
	int numShardsCaptured = sec->GetNumShardsCaptured();

	ss << numShardsCaptured << " / " << numTotalShards;

	shardsCollectedText.setString(ss.str());

	ss.str("");

	int percent = floor(sec->GetCompletionPercentage());

	ss << percent << "%";

	completionPercentText.setString(ss.str());
}

int MapSector::GetSelectedIndex()
{
	return ms->mapSelector->currIndex;
}

Level &MapSector::GetSelectedLevel()
{
	int selectedIndex = GetSelectedIndex();
	if (selectedIndex >= sec->numLevels)
	{
		cout << "this is a bug in getselectedlevel()" << endl;
		//assert(0);
		selectedIndex = 0;
	}
	return sec->levels[selectedIndex];
}

void MapSector::UpdateLevelStats()
{
	int gridIndex = 0;
	int uncaptured = 0;
	Level &lev = GetSelectedLevel();
	auto &snList = lev.shardNameList;
	//numTotalShards = snList.size();
	for (int i = 0; i < numTotalShards; ++i)
	{
		SetRectColor(levelCollectedShardsBG + i * 4, Color(Color::Transparent));
		SetRectSubRect(levelCollectedShards + i * 4, IntRect());
	}

	for (auto it = snList.begin(); it != snList.end(); ++it)
	{
		int sType = Shard::GetShardType((*it));
		int subRectIndex = sType % 22;
		if (sec->world->sf->ShardIsCaptured(sType))
		{
			SetRectSubRect(levelCollectedShards + gridIndex * 4, ts_shards->GetSubRect(subRectIndex));
		}
		else
		{
			SetRectSubRect(levelCollectedShards + gridIndex * 4, IntRect());//ts_shards->GetSubRect(44));
		}

		SetRectColor(levelCollectedShardsBG + gridIndex * 4, Color(Color::Black));
		++gridIndex;
	}

	stringstream ss;
	int percent = floor(GetSelectedLevel().GetCompletionPercentage());

	ss << percent << "%";

	Text &currLevelTimeText = ms->mainMenu->worldMap->currLevelTimeText;

	string levelTimeStr = "Fastest Level Time  ";
	if (lev.bestTimeFrames < 0)
	{
		currLevelTimeText.setString(levelTimeStr + string("-- : --"));
	}
	else
	{
		currLevelTimeText.setString(levelTimeStr + GetTimeStr(lev.bestTimeFrames));
	}
}

sf::Vector2f MapSector::GetNodePos(int node)
{
	return left + Vector2f(nodeSize / 2, 0) + Vector2f((pathLen + nodeSize) * node, 0);
}

sf::Vector2f MapSector::GetSelectedNodePos()
{
	return GetNodePos(GetSelectedIndex());
}

sf::Vector2f MapSector::GetTopNodePos(int n)
{
	Vector2f res = GetNodePos(n);
	res.y -= pathLen + nodeSize;
	return res;
}

sf::Vector2f MapSector::GetBotNodePos(int n)
{
	Vector2f res = GetNodePos(n);
	res.y += pathLen + nodeSize;
	return res;
}

void MapSector::UpdateNodePosition()
{
	for (int i = 0; i < numLevels; ++i)
	{
		topBonusNodes[i].setPosition(GetTopNodePos(i));
		nodes[i].setPosition(GetNodePos(i));
		botBonusNodes[i].setPosition(GetBotNodePos(i));
	}
}

void MapSector::RunSelectedMap()
{
	ms->mainMenu->gameRunType = MainMenu::GRT_ADVENTURE;
	ms->mainMenu->AdventureLoadLevel(&GetSelectedLevel());
}

int MapSector::GetNumLevels()
{
	return sec->numLevels;
}

void MapSector::UpdateBG()
{
	bg->Update(Vector2f(960, 540));
}

void MapSector::Update(ControllerState &curr,
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
			if (sec->levels[i].justBeaten)
			{
				state = LEVELCOMPLETEDWAIT;

				stateFrame = 0;
				unlockedIndex = i;//saSelector->currIndex;
								  //unlockFrame = frame;
				sec->levels[i].justBeaten = false;
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
		int old = GetSelectedIndex();//saSelector->currIndex;

		bool left = curr.LLeft();
		bool right = curr.LRight();

		int changed = ms->mapSelector->UpdateIndex(left, right);

		if (changed != 0)
		{
			UpdateLevelStats();
		}

		if (changed != 0)//|| oldYIndex != selectedYIndex)
		{
			ms->mainMenu->soundNodeList->ActivateSound(ms->mainMenu->soundManager.GetSound("level_change"));
			UpdateNodes();
		}

		if (curr.A && !prev.A && sec->IsUnlocked())
		{
			if (selectedYIndex == 1)
			{

				if (sec->IsComplete())
				{
					state = COMPLETE;
				}
				else
				{
					state = NORMAL;
				}

				ms->mainMenu->soundNodeList->ActivateSound(ms->mainMenu->soundManager.GetSound("level_select"));
				ms->mainMenu->SetMode(MainMenu::TRANS_WORLDMAP_TO_LOADING);
			}
		}
	}

	//UpdateHighlight();

	if (state == LEVELJUSTCOMPLETE)//unlockedIndex != -1)
	{
		int ff = stateFrame / 7;
		if (ff == 13)
		{
			bool secOver = sec->IsComplete();

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
				ms->mapSelector->currIndex = unlockedLevelCount - 1;
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
}

bool MapSector::HasTopBonus(int node)
{
	return sec->HasTopBonus(node);
}
bool MapSector::HasBotBonus(int node)
{
	return sec->HasTopBonus(node);
}

void MapSector::UpdateNodes()
{
	Tileset *ts_node = ms->ts_node;
	for (int i = 0; i < numLevels; ++i)
	{
		//Vertex *n = (nodes + 1 * numLevels * 4 + i * 4);
		//Vertex *nTop = (nodes + 0 * numLevels * 4 + i * 4);
		//Vertex *nBot = (nodes + 2 * numLevels * 4 + i * 4);
		if (HasTopBonus(i) && sec->levels[i].TopBonusUnlocked())
		{
			//SetRectSubRect(nTop, ms->ts_node->GetSubRect(GetNodeBonusIndexTop(i)));
			//SetRectColor(nTop, Color(Color::White));
			topBonusNodes[i].setTextureRect(ts_node->GetSubRect(GetNodeBonusIndexTop(i)));
			topBonusNodes[i].setColor(Color::White);
		}
		else
		{
			topBonusNodes[i].setColor(Color::Transparent);
		}

		if (HasBotBonus(i) && sec->levels[i].BottomBonusUnlocked())
		{
			botBonusNodes[i].setTextureRect(ts_node->GetSubRect(GetNodeBonusIndexBot(i)));

			botBonusNodes[i].setColor(Color::White);
		}
		else
		{
			botBonusNodes[i].setColor(Color::Transparent);
		}

		Tileset *currTS = NULL;
		int bFType = sec->levels[i].bossFightType;
		if (bFType == 0)
		{
			currTS = ts_node;
		}
		else
		{
			currTS = ms->ts_bossFight[bFType - 1];
		}

		nodes[i].setTextureRect(currTS->GetSubRect(GetNodeSubIndex(i)));
		//SetRectSubRect(n, ms->ts_node->GetSubRect(GetNodeSubIndex(i)));
	}
}


void MapSector::Load()
{

}

int MapSector::GetNodeSubIndex(int node)
{
	int bType = sec->levels[node].bossFightType;
	int res;
	if (bType == 0)
	{
		if (!sec->IsLevelUnlocked(node))
		{
			res = 0;
		}
		else
		{
			if (IsFocused() && selectedYIndex == 1 && GetSelectedIndex() == node)
			{
				if (sec->levels[node].IsOneHundredPercent())
				{
					res = 8;
				}
				else if (sec->levels[node].GetComplete())
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
				if (sec->levels[node].IsOneHundredPercent())
				{
					res = 5;
				}
				else if (sec->levels[node].GetComplete())
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
		if (!sec->IsLevelUnlocked(node))
		{
			res = 0;
		}
		else
		{
			if (IsFocused() && selectedYIndex == 1 && GetSelectedIndex() == node)
			{
				if (sec->levels[node].IsOneHundredPercent())
				{
					res = 2;
				}
				else if (sec->levels[node].GetComplete())
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
				if (sec->levels[node].IsOneHundredPercent())
				{
					res = 1;
				}
				else if (sec->levels[node].GetComplete())
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
	return res;
}

int MapSector::GetSelectedNodeSubIndex()
{
	return GetNodeSubIndex(GetSelectedIndex());
}

int MapSector::GetSelectedNodeBossFightType()
{
	return sec->levels[GetSelectedIndex()].bossFightType;
}

void MapSector::UpdateUnlockConditions()
{
	int numUnlock = sec->numUnlockConditions;
	stringstream ss;
	for (int i = 0; i < numUnlock; ++i)
	{
		ss.str("");
		int completeOfType = sec->world->GetNumSectorTypeComplete(sec->conditions[i]);
		int numNeededOfType = sec->numTypesNeeded[i];
		ss << completeOfType << " / " << numNeededOfType;
		unlockCondText[i].setString(ss.str());
	}
}

int MapSector::GetNodeBonusIndexTop(int node)
{
	if (selectedYIndex == 0 && GetSelectedIndex() == node)
	{
		return 0;
	}
	else
	{
		return 0;
	}
	/*if (sec->levels[node].completed)
	{
	return 64;
	}
	else
	{
	return 65;
	}*/
}

int MapSector::GetNodeBonusIndexBot(int node)
{
	if (selectedYIndex == 2 && GetSelectedIndex() == node)
	{
		return 0;
	}
	else
	{
		return 0;
	}
}

void MapSector::Init(Sector *m_sec)
{
	unlockedIndex = -1;
	sec = m_sec;
	int oldNumLevels = numLevels;
	numLevels = sec->numLevels;

	string worldStr = to_string(sec->world->index + 1);
	
	if (bg == NULL)
	{
		string secStr = to_string(sec->index + 1);
		string bgStr = "w" + worldStr + "_0" + secStr;
		bg = Background::SetupFullBG(bgStr, &(ms->mainMenu->tilesetManager));
	}
		
	stringstream ss;

	ss.str("");
	ss << "Shard/shards_w" << (sec->world->index + 1) << "_48x48.png";

	ts_shards = ms->mainMenu->tilesetManager.GetTileset(ss.str(), 48, 48);

	sectorNameText.setString(sec->name);

	int waitFrames[3] = { 30, 10, 5 };
	int waitModeThresh[2] = { 2, 2 };

	bool blank = ms->mapSelector == NULL;
	bool diffNumLevels = false;

	selectedYIndex = 1;

	if (oldNumLevels > 0 && oldNumLevels != numLevels)
	{
		delete[] nodes;
		nodes = NULL;

		delete[] topBonusNodes;
		topBonusNodes = NULL;

		delete[] botBonusNodes;
		botBonusNodes = NULL;
	}
	if (nodes == NULL)
	{

		Tileset *ts_node = ms->ts_node;
		nodes = new Sprite[numLevels];
		topBonusNodes = new Sprite[numLevels];
		botBonusNodes = new Sprite[numLevels];

		for (int i = 0; i < numLevels; ++i)
		{
			Tileset *currTS = NULL;
			int bFType = sec->levels[i].bossFightType;
			if (bFType == 0)
			{
				currTS = ts_node;
			}
			else
			{
				currTS = ms->ts_bossFight[bFType - 1];
			}

			assert(currTS != NULL);
			nodes[i].setTexture(*currTS->texture);

			nodes[i].setTextureRect(currTS->GetSubRect(0));
			nodes[i].setOrigin(nodes[i].getLocalBounds().width / 2, nodes[i].getLocalBounds().height / 2);

			topBonusNodes[i].setTexture(*ts_node->texture);
			topBonusNodes[i].setTextureRect(ts_node->GetSubRect(0));
			topBonusNodes[i].setOrigin(topBonusNodes[i].getLocalBounds().width / 2, topBonusNodes[i].getLocalBounds().height / 2);
			botBonusNodes[i].setTexture(*ts_node->texture);
			botBonusNodes[i].setTextureRect(ts_node->GetSubRect(0));
			botBonusNodes[i].setOrigin(botBonusNodes[i].getLocalBounds().width / 2, botBonusNodes[i].getLocalBounds().height / 2);
		}
	}

	if (numUnlockConditions != -1 && sec->numUnlockConditions != numUnlockConditions)
	{
		delete[] unlockCondText;
		unlockCondText = NULL;
	}

	if (unlockCondText == NULL)
	{
		numUnlockConditions = sec->numUnlockConditions;
		unlockCondText = new Text[numUnlockConditions];

		for (int i = 0; i < numUnlockConditions; ++i)
		{
			unlockCondText[i].setFont(ms->mainMenu->arial);
			unlockCondText[i].setCharacterSize(40);
			unlockCondText[i].setFillColor(Color::White);
		}
	}

	if (sec->IsComplete())
	{
		state = COMPLETE;
		endSpr.setTextureRect(ms->ts_sectorOpen[0]->GetSubRect(15));
	}
	else
	{
		state = NORMAL;
	}

	auto &snList = GetSelectedLevel().shardNameList;
	numTotalShards = snList.size();

	levelCollectedShards = new Vertex[numTotalShards * 4];
	levelCollectedShardsBG = new Vertex[numTotalShards * 4];

	SetXCenter(960);

	UpdateNodes();
	UpdateUnlockConditions();
	UpdateStats();
	UpdateLevelStats();
	UpdateUnlockedLevelCount();
	//UpdateHighlight();
}