#include "SaveFile.h"
#include <sstream>
#include <assert.h>
#include <iostream>
#include "Session.h"
//#include "MainMenu.h"
#include "Enemy_Shard.h"
#include "MapHeader.h"
#include "MainMenu.h"
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;




SaveLevel::SaveLevel()
	:optionField(32)
{
	bossFightType = 0;
	SetComplete(false);
	optionField.Reset();
	shardsLoaded = false;
	justBeaten = false;
}

bool SaveLevel::IsLastInSector()
{
	return (index == sec->numLevels - 1);
}

void SaveLevel::UpdateFromMapHeader()
{
	if (shardsLoaded) return;

	shardsLoaded = true;

	string worldName = sec->world->name;

	ifstream is;

	string file = "Resources/Maps/" + worldName + "/" + name + ".brknk";
	is.open(file);

	if (!is.is_open())
	{
		//check the general directory if its not in the folder
		is = ifstream();
		file = "Resources/Maps/" + name + ".brknk";
		is.open(file);
	}

	if (is.is_open())
	{
		int capCount = 0;
		float percent = 0;

		MapHeader *mh = MainMenu::ReadMapHeader(is);

		shardNameList.clear();
		for (auto it = mh->shardNameList.begin(); it != mh->shardNameList.end(); ++it)
		{
			shardNameList.push_back((*it));
		}

		bossFightType = mh->bossFightType;

		delete mh;
	}
	else
	{

		assert(0);
	}
}

int SaveLevel::GetNumShardsCaptured()
{
	int capCount = 0;
	for (auto it = shardNameList.begin(); it != shardNameList.end(); ++it)
	{
		int st = Shard::GetShardType((*it));
		if (sec->world->sf->ShardIsCaptured(st))
		{
			++capCount;
		}
	}
	return capCount;
}

float SaveLevel::GetCapturedShardsPortion()
{
	int capCount = GetNumShardsCaptured();

	if (capCount == 0)
		return 0;

	return (float)capCount / shardNameList.size();
}

int SaveLevel::GetNumTotalShards()
{
	return shardNameList.size();
}

float SaveLevel::GetCompletionPercentage()
{
	float portion = 0;
	if (GetComplete())
	{
		if (shardNameList.size() == 0)
		{
			portion = 1.0;
		}
		else
		{
			portion += .5f;
			portion += GetCapturedShardsPortion() * .5f;
		}
	}
	else
	{
		if (shardNameList.size() == 0)
		{
			portion = 0.0;
		}
		else
		{
			portion += GetCapturedShardsPortion() * .5f;
		}
	}



	return portion * 100.f;
	//MapSelectionMenu::ReadMapHeader()
}

void SaveLevel::Reset()
{
	SetComplete(false);
	optionField = 0;
	shardsLoaded = false;
	justBeaten = false;
}

bool SaveLevel::Load(std::ifstream &is)
{
	is >> name;
	optionField.Load(is);
	is >> bestTimeFrames;

	UpdateFromMapHeader();

	return true;
}

void SaveLevel::Save(std::ofstream &of)
{
	of << name << endl;
	optionField.Save(of);
	of << bestTimeFrames << endl;
	//cout << "save level: " << name << endl;
}

void SaveLevel::SetComplete(bool comp)
{
	optionField.SetBit(COMPLETE, comp);
}

bool SaveLevel::TrySetRecord(int numFrames)
{
	if (bestTimeFrames < 0 || numFrames < bestTimeFrames)
	{
		bestTimeFrames = numFrames;
		return true;
	}

	return false;
}

bool SaveLevel::GetComplete()
{
	return optionField.GetBit(COMPLETE);
}

bool SaveLevel::IsOneHundredPercent()
{
	return(GetCompletionPercentage() == 100.f);
}

std::string SaveLevel::GetFullName()
{
	return "Resources/Maps/" + sec->world->name + "/" + name + ".brknk";
}





SaveSector::SaveSector()
{
	numLevels = 0;
	levels = NULL;
	conditions = NULL;
	numTypesNeeded = NULL;
	sectorType = 100;
	numUnlockConditions = 99;
}

bool SaveSector::IsLevelUnlocked(int index)
{
	for (int i = 0; i < index; ++i )
	{
		if (!levels[i].GetComplete())
		{
			return false;
		}
	}
	return true;
}

void SaveSector::UpdateShardNameList()
{
	std::map<std::string, int> shardNameMap;
	for (int i = 0; i < numLevels; ++i)
	{
		for (auto it = levels[i].shardNameList.begin(); it != levels[i].shardNameList.end(); ++it)
		{
			if (shardNameMap.count((*it)) == 0)
			{
				shardNameMap[(*it)] = 0;
			}
			else
			{
				shardNameMap[(*it)]++;
			}
		}
	}

	for (auto it = shardNameMap.begin(); it != shardNameMap.end(); ++it)
	{
		shardNameList.push_back((*it).first);
	}
}

int SaveSector::GetNumTotalShards()
{
	return shardNameList.size();
}

int SaveSector::GetNumShardsCaptured()
{
	int capCount = 0;
	for (auto it = shardNameList.begin(); it != shardNameList.end(); ++it)
	{
		int st = Shard::GetShardType((*it));
		if (world->sf->ShardIsCaptured(st))
		{
			++capCount;
		}
	}
	return capCount;
}

float SaveSector::GetCompletionPercentage()
{
	float completePercent = 0;
	float possiblePercent = 0;
	for (int i = 0; i < numLevels; ++i)
	{
		completePercent += levels[i].GetCompletionPercentage();
		possiblePercent += 100;
	}

	return (completePercent / possiblePercent) * 100.f;
}

bool SaveSector::HasTopBonus(int index)
{
	return (topBonuses.count(index) > 0);
}
bool SaveSector::hasBottomBonus(int index)
{
	return (bottomBonuses.count(index) > 0);
}

int SaveSector::GetTotalFrames()
{
	int total = 0;
	int temp;
	for (int i = 0; i < numLevels; ++i)
	{
		temp = levels[i].bestTimeFrames;

		if (temp == -1)
		{
			return -1;
		}

		total += temp;
	}

	return total;
}

bool SaveSector::IsComplete()
{
	for (int i = 0; i < numLevels; ++i)
	{
		if (!levels[i].GetComplete())
		{
			return false;
		}
	}
	return true;
}

bool SaveSector::Load(std::ifstream &is)
{
	is.get();
	std::getline(is, name);
	//is >> name;
	is >> sectorType;
	is >> numUnlockConditions;

	if (numUnlockConditions > 0)
	{
		conditions= new int[numUnlockConditions];
		numTypesNeeded = new int[numUnlockConditions];

		int conditionType = -1;
		int numOfTypeNeeded = -1;
		for (int i = 0; i < numUnlockConditions; ++i)
		{
			is >> conditionType;
			is >> numOfTypeNeeded;

			conditions[i] = conditionType;
			numTypesNeeded[i] = numOfTypeNeeded;
		}
	}

	is >> numLevels;
	levels = new Level[numLevels];
	
	bool res = true;
	for (int i = 0; i < numLevels; ++i)
	{
		levels[i].sec = this;
		levels[i].index = i;
		res = levels[i].Load(is);

		assert(res);
	}

	int numTopBonuses = 0;
	int numBottomBonuses = 0;

	
	is >> numTopBonuses;
	for (int j = 0; j < numTopBonuses; ++j) //0 - 2
	{
		int bonusIndex;
		is >> bonusIndex;

		Level &lev = topBonuses[bonusIndex];
		lev.sec = this;
		lev.index = bonusIndex;
		res = lev.Load(is);

		assert(res);
	}

	is >> numBottomBonuses;
	for (int j = 0; j < numBottomBonuses; ++j) //0 - 2
	{
		int bonusIndex;
		is >> bonusIndex;

		Level &lev = bottomBonuses[bonusIndex];
		lev.sec = this;
		lev.index = bonusIndex;
		res = lev.Load(is);

		assert(res);
	}
	
	assert(is.good());

	UpdateShardNameList();

	return true;
}

SaveSector::~SaveSector()
{
	if (levels != NULL)
	{
		delete[] levels;
	}

	if (conditions != NULL)
		delete[] conditions;
	if (numTypesNeeded != NULL)
		delete[] numTypesNeeded;
}

bool SaveSector::IsConditionFulfilled(int index)
{
	int numTypeComplete = world->GetNumSectorTypeComplete(conditions[index]);
	return (numTypeComplete >= numTypesNeeded[index]);
}

bool SaveSector::IsUnlocked()
{
	int numTypeComplete = -1;
	for (int i = 0; i < numUnlockConditions; ++i)
	{
		if (!IsConditionFulfilled(i))
		{
			return false;
		}
	}

	return true;
}

void SaveSector::Save(std::ofstream &of)
{
	of << name << endl;
	of << sectorType << endl;
	of << numUnlockConditions << endl;
	if (numUnlockConditions > 0)
	{
		for (int i = 0; i < numUnlockConditions; ++i)
		{
			of << conditions[i] << endl;
			of << numTypesNeeded[i] << endl;
		}

	}

	of << numLevels << endl;
	for (int i = 0; i < numLevels; ++i)
	{
		levels[i].Save(of);
	}

	int numTopBonuses = topBonuses.size();
	int numBottomBonuses = bottomBonuses.size();
	of << numTopBonuses << endl;	
	for (auto it = topBonuses.begin(); it != topBonuses.end(); ++it)
	{
		of << (*it).first << endl;
		(*it).second.Save(of);
	}

	of << numBottomBonuses << endl;
	for (auto it = bottomBonuses.begin(); it != bottomBonuses.end(); ++it)
	{
		of << (*it).first << endl;
		(*it).second.Save(of);
	}
	
}






SaveWorld::SaveWorld()
{
	sectors = NULL;
	numSectors = 0;
}

SaveWorld::~SaveWorld()
{
	if (sectors != NULL)
	{
		delete[] sectors;
	}
}

int SaveWorld::GetTotalFrames()
{
	int total = 0;
	int temp;
	for (int i = 0; i < numSectors; ++i)
	{
		temp = sectors[i].GetTotalFrames();
		if (temp == -1)
		{
			return -1;
		}

		total += temp;
	}

	return total;
}

float SaveWorld::GetCompletionPercentage()
{
	float totalComplete = 0;
	float totalPossible = 0;

	for (int i = 0; i < numSectors; ++i)
	{
		totalComplete += sectors[i].GetCompletionPercentage();
		totalPossible += 100.f;
	}

	return (totalComplete / totalPossible) * 100.f;
}

bool SaveWorld::Load(std::ifstream &is)
{
	is.get();
	std::getline(is, name);
	is >> numSectors;
	assert(numSectors > 0);
	sectors = new Sector[numSectors];

	for (int i = 0; i < numSectors; ++i)
	{
		sectors[i].world = this;
		sectors[i].index = i;
		sectors[i].Load(is);
	}

	UpdateShardNameList();
	return true;
}

bool SaveWorld::Save(std::ofstream &of)
{
	of << name << endl;
	of << numSectors << endl;
	cout << "numSectors: " << numSectors << endl;
	for (int i = 0; i < numSectors; ++i)
	{
		sectors[i].Save(of);
	}
	return true;
}

void SaveWorld::UpdateShardNameList()
{
	std::map<std::string, int> shardNameMap;
	for (int i = 0; i < numSectors; ++i)
	{
		for (auto it = sectors[i].shardNameList.begin(); it != sectors[i].shardNameList.end(); ++it)
		{
			if (shardNameMap.count((*it)) == 0)
			{
				shardNameMap[(*it)] = 0;
			}
			else
			{
				shardNameMap[(*it)]++;
			}
		}
	}

	for (auto it = shardNameMap.begin(); it != shardNameMap.end(); ++it)
	{
		shardNameList.push_back((*it).first);
	}
}

int SaveWorld::GetNumTotalShards()
{
	return shardNameList.size();
}

int SaveWorld::GetNumShardsCaptured()
{
	int capCount = 0;
	for (auto it = shardNameList.begin(); it != shardNameList.end(); ++it)
	{
		int st = Shard::GetShardType((*it));
		if (sf->ShardIsCaptured(st))
		{
			++capCount;
		}
	}
	return capCount;
}

int SaveWorld::GetNumSectorTypeComplete(int sType)
{
	int count = 0;
	for (int i = 0; i < numSectors; ++i)
	{
		if (sectors[i].sectorType == sType)
		{
			if (sectors[i].IsComplete())
			{
				++count;
			}
		}
	}

	return count;
}




LevelScore::LevelScore()
{
	bestFramesToBeat = 0;
}

void LevelScore::Save(ofstream &of)
{
	of << bestFramesToBeat << "\n";
}

void LevelScore::Load(ifstream &is)
{
	is >> bestFramesToBeat;
}

int LevelScore::GetNumTotalShards()
{

}

int LevelScore::GetNumShardsCaptured()
{

}


SaveFile::SaveFile(const std::string &name, AdventureFile &p_adventure)
	:levelsBeatenField( 512 ), 
	shardField(ShardInfo::MAX_SHARDS), 
	newShardField(ShardInfo::MAX_SHARDS),
	upgradeField(Session::PLAYER_OPTION_BIT_COUNT),
	momentaField(4 * 32),
	adventureFile( p_adventure )
{
	stringstream ss;
	ss << "Resources/Data/" << name << ".kin";

	fileName = ss.str();
}

SaveFile::~SaveFile()
{
}

int SaveFile::GetBestFrames()
{
	int total = 0;
	int temp;

	for (int i = 0; i < 512; ++i)
	{
		if (adventureFile.GetMap(i).Exists())
		{
			total += levelScores[i].bestFramesToBeat;
		}
	}

	return total;
}

int SaveFile::GetBestFramesWorld(int w)
{
	int total = 0;
	int wStart = w * 64;
	int wEnd = (w + 1) * 64;
	for (int i = wStart; i < wEnd; ++i)
	{
		if (adventureFile.GetMap(i).Exists())
		{
			total += levelScores[i].bestFramesToBeat;
		}
	}

	return total;
}

int SaveFile::GetBestFramesSector(int w, int s)
{
	int total = 0;
	int sStart = w * 64 + s * 8;
	int sEnd = w * 64 + (s + 1) * 8;

	for (int i = sStart; i < sEnd; ++i)
	{
		if (adventureFile.GetMap(i).Exists())
		{
			total += levelScores[i].bestFramesToBeat;
		}
	}
	
	return total;
}

int SaveFile::GetBestFramesLevel(int w, int s, int m)
{
	int i = w * 64 + s * 8 + m;
	if (adventureFile.GetMap(i).Exists())
	{
		return levelScores[i].bestFramesToBeat;
	}

	return -1;
}

float SaveFile::GetCompletionPercentage()
{
	float totalComplete = 0;
	float totalPossible = 0;

	float totalMaps = 0;
	float totalBeaten = 0;
	for (int i = 0; i < 512; ++i)
	{
		if (adventureFile.GetMap(i).Exists())
		{
			++totalMaps;
			if (levelsBeatenField.GetBit(i))
			{
				++totalBeaten;
			}
		}
	}

	float totalShards = 0;
	float totalCaptured = 0;
	for (int i = 0; i < ShardInfo::MAX_SHARDS; ++i)
	{
		if (adventureFile.hasShardField.GetBit(i))
		{
			++totalShards;
			if (shardField.GetBit(i))
			{
				++totalCaptured;
			}
		}
	}

	float shardPortion = totalCaptured / totalShards;

	float portion = totalBeaten / totalMaps;

	float totalPortion = shardPortion * .5 + portion * .5;

	if (totalPortion > .99f)
		totalPortion = 1.0f;

	return totalPortion * 100.f;
}

bool SaveFile::HasUpgrade(int pow)
{
	return upgradeField.GetBit(pow);
}

void SaveFile::UnlockUpgrade(int pow)
{
	upgradeField.SetBit(pow, true);
}



bool SaveFile::ShardIsCaptured(int sType)
{
	return shardField.GetBit(sType);
}

void SaveFile::CopyFromDefault()
{
	path from("Resources/Data/default.kin");

	path to(fileName);

	boost::filesystem::copy_file(from, to, copy_option::fail_if_exists);
}

bool SaveFile::LoadInfo(ifstream &is)
{
	if (is.is_open())
	{
		getline(is, controlProfileName);

		levelsBeatenField.Load(is);

		for (int i = 0; i < 512; ++i)
		{
			levelScores[i].Load(is);
		}
		
		upgradeField.Load(is);
		momentaField.Load(is);

		shardField.Load(is);
		newShardField.Load(is);

		UpdateShardNameList();

		is.close();
		return true;
	}
	else
	{
		/*if (fileName == "Data/blue.kin")
		{
		int xxx = 54;
		}*/
		return false;
	}
}

void SaveFile::UpdateShardNameList()
{
	/*AdventureMap *am;
	bool found;

	shardInfoVec.reserve(128);
	for (int i = 0; i < 512; ++i)
	{
		am = &adventureFile.GetMap(i);
		if (am->Exists())
		{
			for (auto it = am->headerInfo.shardInfoVec.begin();
				it != am->headerInfo.shardInfoVec.end(); ++it)
			{
				found = false;
				for (auto myIt = shardInfoVec.begin(); myIt != shardInfoVec.end(); ++myIt)
				{
					if ((*it).world == (*myIt).world 
						&& (*it).localIndex == (*myIt).localIndex)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					shardInfoVec.push_back( )
				}
			}
		}
	}

	std::map<std::string, int> shardNameMap;
	for (int i = 0; i < numWorlds; ++i)
	{
		for (auto it = worlds[i].shardNameList.begin(); it != worlds[i].shardNameList.end(); ++it)
		{
			if (shardNameMap.count((*it)) == 0)
			{
				shardNameMap[(*it)] = 0;
			}
			else
			{
				shardNameMap[(*it)]++;
			}
		}
	}

	for (auto it = shardNameMap.begin(); it != shardNameMap.end(); ++it)
	{
		shardNameList.push_back((*it).first);
	}*/
}

int SaveFile::GetNumShardsCaptured()
{
	int capCount = 0;
	for (auto it = shardNameList.begin(); it != shardNameList.end(); ++it)
	{
		int st = Shard::GetShardType((*it));
		if (ShardIsCaptured(st))
		{
			++capCount;
		}
	}
	return capCount;
}

int SaveFile::GetNumTotalShards()
{
	return shardNameList.size();
}

bool SaveFile::HasNewShards()
{
	return newShardField.IsNonZero();
}

//return true on normal loading, and false if you need to make a default
bool SaveFile::Load()
{
	ifstream is;

	is.open(fileName);

	if (!LoadInfo(is))
	{
		ifstream defIs;
		defIs.open("Resources/Data/default.kin");
		bool test = LoadInfo(defIs);
		assert(test);
		return false;
	}
	else
	{
		return true;
	}
}

void SaveFile::Save()
{
	ofstream of;

	of.open(fileName);

	cout << "saving save file" << endl;

	if (of.is_open())
	{
		of << controlProfileName << endl;

		levelsBeatenField.Save(of);

		for (int i = 0; i < 512; ++i)
		{
			levelScores[i].Save(of);
		}

		upgradeField.Save(of);
		momentaField.Save(of);

		shardField.Save(of);
		newShardField.Save(of);

		of.close();
	}
	else
	{
		cout << "error saving file: " << fileName << endl;
		assert(false);
	}
}