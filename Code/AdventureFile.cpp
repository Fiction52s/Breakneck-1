#include "SaveFile.h"
#include <sstream>
#include <assert.h>
#include <iostream>
#include "MapHeader.h"
#include "globals.h"

using namespace std;

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

string AdventureMap::GetFilePath()
{
	if (path == "")
		return name;
	else
	{
		return path + "\\" + name;
	}
}

std::string AdventureMap::GetMapPath()
{
	return "Resources\\" + GetFilePath() + MAP_EXT;
}

bool AdventureMap::Exists()
{
	return name != "";
}

bool AdventureMap::LoadHeaderInfo()
{
	assert(Exists());

	ifstream is;

	if (path == "")
	{
		is.open( "Resources\\" + name + MAP_EXT);
	}
	else
	{
		is.open( "Resources\\" + path + "\\" + name + MAP_EXT);
	}

	if (is.is_open())
	{
		MapHeader mh;
		mh.Load(is);

		headerInfo.mapType = mh.bossFightType;
		headerInfo.shardInfoVec = mh.shardInfoVec;
		if (mh.GetNumSongs() > 0)
		{
			headerInfo.mainSongName = mh.songOrder[0];
		}
		

		for (auto it = headerInfo.shardInfoVec.begin();
			it != headerInfo.shardInfoVec.end(); ++it)
		{
			headerInfo.hasShardField.SetBit((*it).GetTrueIndex(), true);
		}
	}
	else
	{
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
		string fullPath;
		std::getline(is, fullPath);

		if (fullPath == "----")
		{
			name = "";
			path = "";
			return;
		}

		auto lastSlash = fullPath.find_last_of('\\');
		if (lastSlash == std::string::npos)
		{
			name = fullPath;
			path = "";
		}
		else
		{
			name = fullPath.substr(lastSlash + 1);
			path = fullPath.substr(0, lastSlash);
		}
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
			of << path << "\\" << name << "\n";
		}
	}
}

AdventureSector::AdventureSector()
	:hasShardField(ShardInfo::MAX_SHARDS)
{
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
	:hasShardField( 32 * 4 )
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
	int maxLevelsPerWorld = (ADVENTURE_MAX_NUM_SECTORS_PER_WORLD * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR);
	int w = index / maxLevelsPerWorld;
	int s = (index % maxLevelsPerWorld) / ADVENTURE_MAX_NUM_SECTORS_PER_WORLD;
	int m = (index % ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR);

	return worlds[w].sectors[s].maps[m];
}

void AdventureFile::GetMapIndexes( int index, int &w, int &s, int &m)
{
	int maxLevelsPerWorld = (ADVENTURE_MAX_NUM_SECTORS_PER_WORLD * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR);
	w = index / maxLevelsPerWorld;
	s = (index % maxLevelsPerWorld) / ADVENTURE_MAX_NUM_SECTORS_PER_WORLD;
	m = (index % ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR);
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
				}
			}
			worlds[w].hasShardField.Or(worlds[w].sectors[s].hasShardField);
		}
		hasShardField.Or(worlds[w].hasShardField);
	}

	return true;
}