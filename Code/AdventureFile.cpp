#include "SaveFile.h"
#include <sstream>
#include <assert.h>
#include <iostream>
#include "MapHeader.h"
#include "globals.h"
#include "Session.h"
#include "Actor.h"
#include "LogMenu.h"

using namespace std;

AdventureMapHeaderInfo::AdventureMapHeaderInfo()
	:hasShardField(ShardInfo::MAX_SHARDS),
	hasLogField(LogDetailedInfo::MAX_LOGS)
{
	Clear();
}

bool AdventureMapHeaderInfo::IsLoaded()
{ 
	return mapType >= 0; 
}

void AdventureMapHeaderInfo::Clear()
{
	mapType = -1;
	
	shardInfoVec.clear();
	mainSongName = "";
	powerVec.clear();
	

	hasShardField.Reset();
	hasLogField.Reset();

	goldSeconds = 0;
	silverSeconds = 0;
	bronzeSeconds = 0;

	goldRewardShardInfo.Clear();
	silverRewardCategory = -1;
	silverRewardIndex = -1;
}

void AdventureMapHeaderInfo::Set(AdventureMapHeaderInfo &info)
{
	mapType = info.mapType;
	
	shardInfoVec = info.shardInfoVec;
	mainSongName = info.mainSongName;

	hasShardField.Set(info.hasShardField);
	hasLogField.Set(info.hasLogField);

	powerVec = info.powerVec;

	goldSeconds = info.goldSeconds;
	silverSeconds = info.silverSeconds;
	bronzeSeconds = info.bronzeSeconds;

	goldRewardShardInfo = info.goldRewardShardInfo;
	silverRewardCategory = info.silverRewardCategory;
	silverRewardIndex = info.silverRewardIndex;
}

AdventurePlanet::AdventurePlanet(AdventureFile &adventureFile)
{
	int numSectors;
	int numLevels;
	World *sw;
	Sector *ss;
	Level *sl;
	int counter;
	int levelCounter;
	int levelIndex;
	
	numWorlds = adventureFile.GetNumExistingWorlds();
	if (numWorlds == 0)
		assert(0);

	worlds = new World[numWorlds];
	int worldCounter = 0;

	for (int w = 0; w < ADVENTURE_MAX_NUM_WORLDS; ++w)
	{
		numSectors = adventureFile.worlds[w].GetNumExistingSectors();
		if (numSectors == 0)
		{
			continue;
		}

		sw = &worlds[worldCounter];
		sw->index = w;
		++worldCounter;

		sw->numSectors = numSectors;

		if (numSectors == 0)
		{
			sw->sectors = NULL;
			continue;
		}

		sw->sectors = new Sector[numSectors];
		counter = 0;
		for (int s = 0; s < ADVENTURE_MAX_NUM_SECTORS_PER_WORLD; ++s)
		{
			numLevels = adventureFile.GetSector(w, s).GetNumExistingMaps();
			if (numLevels > 0)
			{
				ss = &(sw->sectors[counter]);
				ss->worldIndex = w;
				ss->numLevels = numLevels;
				ss->levels = new Level[numLevels];
				ss->index = counter;
				levelCounter = 0;
				for (int m = 0; m < ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR; ++m)
				{
					levelIndex = w * (ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR * ADVENTURE_MAX_NUM_SECTORS_PER_WORLD) + s * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR + m;
					if (adventureFile.GetMap(levelIndex).Exists())
					{
						sl = &(ss->levels[levelCounter]);
						sl->index = levelIndex;

						++levelCounter;
					}
				}
				++counter;
			}
		}
	}
}

AdventurePlanet::~AdventurePlanet()
{
	delete[] worlds;
}

AdventureMap::AdventureMap()
{
	Clear();
}

string AdventureMap::GetFilePath()
{
	return path;

	//if (path == "")
	//	return name;
	//else
	//{
	//	return path;
	//	//return path + "\\" + name;
	//}
}

std::string AdventureMap::GetMapPath()
{
	return "Resources\\Maps\\" + path + MAP_EXT;//"Resources\\" + GetFilePath() + MAP_EXT;
}

void AdventureMap::Clear()
{
	name = "";
	path = "";
	headerInfo.Clear();
}

bool AdventureMap::Exists()
{
	return name != "";
}

void AdventureMap::Set(AdventureMap &am)
{
	name = am.name;
	path = am.path;
	headerInfo.Set(am.headerInfo);
}

bool AdventureMap::LoadHeaderInfo()
{
	assert(Exists());

	ifstream is;

	//path is just correct already! probably ruined opening world map!
	/*if (path == "")
	{
		is.open( "Resources\\Maps\\" + name + MAP_EXT);
	}
	else
	{
		is.open( "Resources\\" + path + "\\" + name + MAP_EXT);
	}*/

	string filePath = GetMapPath();
	is.open(filePath);

	if (is.is_open())
	{
		MapHeader mh;
		mh.Load(is);

		headerInfo.mapType = mh.bossFightType;
		headerInfo.shardInfoVec = mh.shardInfoVec;
		headerInfo.logInfoVec = mh.logInfoVec;
		if (mh.GetNumSongs() > 0)
		{
			headerInfo.mainSongName = mh.songOrder[0];
		}
		else
		{
			headerInfo.mainSongName = "";
		}

		for (auto it = headerInfo.shardInfoVec.begin();
			it != headerInfo.shardInfoVec.end(); ++it)
		{
			headerInfo.hasShardField.SetBit((*it).GetTrueIndex(), true);
		}

		for (auto it = headerInfo.logInfoVec.begin();
			it != headerInfo.logInfoVec.end(); ++it)
		{
			headerInfo.hasLogField.SetBit((*it).GetTrueIndex(), true);
		}

		headerInfo.powerVec = mh.powerVec;

		headerInfo.goldSeconds = mh.goldSeconds;
		headerInfo.silverSeconds = mh.silverSeconds;
		headerInfo.bronzeSeconds = mh.bronzeSeconds;

		headerInfo.goldRewardShardInfo = mh.goldRewardShardInfo;
		headerInfo.silverRewardCategory = mh.silverRewardCategory;

		is.close();
	}
	else
	{
		cout << "unable to open header for: " << filePath << "\n";
		assert(false);
	}

	return true;
}

void AdventureMap::Load(std::ifstream &is, int copyMode)
{
	if (copyMode == AdventureFile::COPY)
	{
		std::getline(is, name);
		if (name == "----")
			name = "";
	}
	else if (copyMode == AdventureFile::PATH)
	{
		string pathStr;
		std::getline(is, pathStr);

		if (pathStr == "----")
		{
			name = "";
			path = "";
			return;
		}

		boost::filesystem::path p(pathStr);

		name = p.filename().string();
		path = p.string();// +MAP_EXT;
		/*auto lastSlash = fullPath.find_last_of('\\');
		if (lastSlash == std::string::npos)
		{
			name = fullPath;
			path = "";
		}
		else
		{
			name = fullPath.substr(lastSlash + 1);
			path = fullPath.substr(0, lastSlash);
		}*/
	}
}

void AdventureMap::Save(std::ofstream &of, int copyMode)
{
	if (name == "")
	{
		of << "----" << "\n";
	}
	else
	{
		if (copyMode == AdventureFile::COPY)
		{
			of << name << "\n";
		}
		else if (copyMode == AdventureFile::PATH)
		{
			of << path << "\n";
			//of << path << "\\" << name << "\n";
		}
	}
}

AdventureSector::AdventureSector()
	:hasShardField(ShardInfo::MAX_SHARDS),
	hasLogField( LogDetailedInfo::MAX_LOGS )
{
}

void AdventureSector::Clear()
{
	hasShardField.Reset();
	hasLogField.Reset();
	for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR; ++i)
	{
		maps[i].Clear();
	}
}

int AdventureSector::GetNumExistingMaps()
{
	int activeCounter = 0;
	for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR; ++i)
	{
		if (maps[i].Exists())
		{
			++activeCounter;
		}
	}

	return activeCounter;
}

void AdventureSector::Load(std::ifstream &is, int ver, int copyMode)
{
	for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR; ++i)
	{
		maps[i].Load(is, copyMode);
	}
}

void AdventureSector::Save(std::ofstream &of, int ver, int copyMode)
{
	//of << requiredRunes << "\n";

	for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR; ++i)
	{
		maps[i].Save(of, copyMode);
	}
}

int AdventureWorld::GetNumExistingSectors()
{
	int activeCounter = 0;
	for (int i = 0; i < ADVENTURE_MAX_NUM_SECTORS_PER_WORLD; ++i)
	{
		if (sectors[i].GetNumExistingMaps() > 0)
		{
			++activeCounter;
		}
	}

	return activeCounter;
}

AdventureWorld::AdventureWorld()
	:hasShardField(ShardInfo::MAX_SHARDS),
	hasLogField(LogDetailedInfo::MAX_LOGS)
{

}

void AdventureWorld::Clear()
{
	hasShardField.Reset();
	hasLogField.Reset();
	for (int i = 0; i < ADVENTURE_MAX_NUM_SECTORS_PER_WORLD; ++i)
	{
		sectors[i].Clear();
	}
}

void AdventureWorld::Load(std::ifstream &is, int ver,
	int copyMode)
{
	for (int i = 0; i < ADVENTURE_MAX_NUM_SECTORS_PER_WORLD; ++i)
	{
		sectors[i].Load(is, ver, copyMode);
	}
}

void AdventureWorld::Save(std::ofstream &of, int ver,
	int copyMode)
{
	for (int i = 0; i < ADVENTURE_MAX_NUM_SECTORS_PER_WORLD; ++i)
	{
		sectors[i].Save(of, ver, copyMode);
	}
}

int AdventureFile::GetNumExistingWorlds()
{
	int activeCounter = 0;
	for (int i = 0; i < ADVENTURE_MAX_NUM_WORLDS; ++i)
	{
		if (worlds[i].GetNumExistingSectors() > 0)
		{
			activeCounter++;
		}
	}

	return activeCounter;
}

AdventureFile::AdventureFile()
	:hasShardField(ShardInfo::MAX_SHARDS), hasLogField(LogDetailedInfo::MAX_LOGS)
{
	SetVer(1);
}

AdventureFile::~AdventureFile()
{
}

void AdventureFile::SetVer(int v )
{
	ver = v;
}

void AdventureFile::Clear()
{
	hasShardField.Reset();
	hasLogField.Reset();
	for (int i = 0; i < ADVENTURE_MAX_NUM_WORLDS; ++i)
	{
		worlds[i].Clear();
	}
}

bool AdventureFile::Load(const std::string &p_path,
	const std::string &adventureName)
{
	string filePath = p_path + "/" + adventureName + ADVENTURE_EXT;

	ifstream is;
	is.open(filePath);

	if (!is.is_open())
	{
		return false;
	}
	else
	{
		is >> ver;

		int cMode;
		is >> cMode;

		copyMode = (CopyMode)cMode;

		is.get();

		for (int i = 0; i < ADVENTURE_MAX_NUM_WORLDS; ++i)
		{
			worlds[i].Load(is, ver, copyMode);
		}

		is.close();
	}

	return true;
}

void AdventureFile::Save(const std::string &p_path,
	const std::string &adventureName, CopyMode cpy)
{
	string ext = ADVENTURE_EXT;
	ofstream of;

	if (p_path == "")
	{
		of.open(adventureName + ext);
	}
	else
	{
		of.open("Resources/" + p_path + "/" + adventureName + ext);
	}

	assert(of.is_open());

	of << ver << endl;

	of << (int)cpy << "\n";

	for (int i = 0; i < ADVENTURE_MAX_NUM_WORLDS; ++i)
	{
		worlds[i].Save(of, ver, (int)cpy);
	}

	of.close();
}

AdventureMap &AdventureFile::GetMap(int index)
{
	int w, s, m;
	GetMapIndexes(index, w, s, m);

	return worlds[w].sectors[s].maps[m];
}

void AdventureFile::GetMapIndexes( int index, int &w, int &s, int &m)
{
	int maxLevelsPerWorld = (ADVENTURE_MAX_NUM_SECTORS_PER_WORLD * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR);
	w = index / maxLevelsPerWorld;
	s = (index % maxLevelsPerWorld) / ADVENTURE_MAX_NUM_SECTORS_PER_WORLD;
	m = (index % ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR);
}

std::string AdventureFile::GetLeaderboardName(int mIndex)
{
	//int w, s, m;
	//GetMapIndexes(mIndex, w, s, m);
	return GetMap(mIndex).name;//to_string(w+1) + "_" + to_string(s+1) + "_" + to_string(m+1);
}

std::string AdventureFile::GetLeaderboardDisplayName(int mIndex)
{
	/*int w, s, m;
	GetMapIndexes(mIndex, w, s, m);

	return to_string(w + 1) + "-" + to_string(s + 1) + "-" + to_string(m + 1);*/
	return GetMap(mIndex).name;
}

AdventureMapHeaderInfo &AdventureFile::GetMapHeaderInfo(int index)
{
	return GetMap(index).headerInfo;
}

AdventureSector &AdventureFile::GetSector(int w, int s)
{
	return worlds[w].sectors[s];
}

AdventureSector &AdventureFile::GetAdventureSector(Sector *sec)
{
	return worlds[sec->worldIndex].sectors[sec->index];
}

AdventureWorld &AdventureFile::GetWorld(int w)
{
	return worlds[w];
}

bool AdventureFile::LoadMapHeaders()
{
	for (int w = 0; w < ADVENTURE_MAX_NUM_WORLDS; ++w)
	{
		for (int s = 0; s < ADVENTURE_MAX_NUM_SECTORS_PER_WORLD; ++s)
		{
			for (int m = 0; m < ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR; ++m)
			{
				AdventureMap &am = worlds[w].sectors[s].maps[m];
				if (am.Exists())
				{
					am.LoadHeaderInfo();
					worlds[w].sectors[s].hasShardField.Or(am.headerInfo.hasShardField);
					worlds[w].sectors[s].hasLogField.Or(am.headerInfo.hasLogField);
				}
			}
			worlds[w].hasShardField.Or(worlds[w].sectors[s].hasShardField);
			worlds[w].hasLogField.Or(worlds[w].sectors[s].hasLogField);
		}
		hasShardField.Or(worlds[w].hasShardField);
		hasLogField.Or(worlds[w].hasLogField);
	}

	return true;
}

void AdventureFile::GetOriginalProgressionUpgradeField(int mapIndex, BitField &bf)
{
	assert(bf.numOptions == Session::PLAYER_OPTION_BIT_COUNT);

	bf.Reset();

	for (int i = 0; i < mapIndex; ++i)
	{
		auto &am = GetMap(i);

		if (!am.Exists())
		{
			continue;
		}

		auto &mhi = GetMapHeaderInfo(i);
		assert(mhi.IsLoaded());

		for (int j = 0; j < mhi.hasShardField.numOptions; ++j)
		{
			bf.SetBit(Actor::SHARD_START_INDEX + j, bf.GetBit(Actor::SHARD_START_INDEX + j) || mhi.hasShardField.GetBit(j)); //Or(mhi.hasShardField);
		}

		for (auto it = mhi.powerVec.begin(); it != mhi.powerVec.end(); ++it)
		{
			if ((*it) == Actor::UPGRADE_POWER_RWIRE)
			{
				bf.SetBit((*it), true);
				bf.SetBit((*it) + 1, true);
			}
			else
			{
				bf.SetBit((*it), true);
			}
		}

		//for (int j = 0; j < mhi.po.numOptions; ++j)
	}

	/*int w, s, m;
	GetMapIndexes(mapIndex, w, s, m);*/
}

void AdventureFile::GetOriginalProgressionLogField(int mapIndex, BitField &bf)
{
	assert(bf.numOptions == LogDetailedInfo::MAX_LOGS);

	bf.Reset();

	for (int i = 0; i < mapIndex; ++i)
	{
		auto &am = GetMap(i);

		if (!am.Exists())
		{
			continue;
		}

		auto &mhi = GetMapHeaderInfo(i);
		assert(mhi.IsLoaded());

		for (int j = 0; j < mhi.hasLogField.numOptions; ++j)
		{
			bf.SetBit(j, bf.GetBit(j) || mhi.hasLogField.GetBit(j));
		}
	}
}