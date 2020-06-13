#include "SaveFile.h"
#include <sstream>
#include <assert.h>
#include <iostream>
#include "MapHeader.h"

using namespace std;

Planet::Planet(AdventureFile &adventureFile)
{
	int numSectors;
	int numLevels;
	World *sw;
	Sector *ss;
	Level *sl;
	int counter;
	int levelCounter;
	int levelIndex;
	
	numWorlds = adventureFile.GetNumActiveWorlds();
	if (numWorlds == 0)
		assert(0);

	worlds = new World[numWorlds];
	int worldCounter = 0;

	for (int w = 0; w < 8; ++w)
	{
		numSectors = adventureFile.worlds[w].GetNumActiveSectors();
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
		for (int s = 0; s < 8; ++s)
		{
			numLevels = adventureFile.GetSector(w, s).GetNumActiveMaps();
			if (numLevels > 0)
			{
				ss = &(sw->sectors[counter]);
				ss->worldIndex = w;
				ss->numLevels = numLevels;
				ss->levels = new Level[numLevels];
				ss->index = counter;
				levelCounter = 0;
				for (int m = 0; m < 8; ++m)
				{
					levelIndex = w * 64 + s * 8 + m;
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

Planet::~Planet()
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
	return "Resources\\" + GetFilePath() + ".brknk";
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
		is.open( "Resources\\" + name + ".brknk");
	}
	else
	{
		is.open( "Resources\\" + path + "\\" + name + ".brknk");
	}



	if (is.is_open())
	{
		MapHeader mh;
		mh.Load(is);

		headerInfo.mapType = mh.bossFightType;
		headerInfo.shardInfoVec = mh.shardInfoVec;

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
	requiredRunes = 0;
}

int AdventureSector::GetNumActiveMaps()
{
	int activeCounter = 0;
	for (int i = 0; i < 8; ++i)
	{
		if (maps[i].Exists())
		{
			++activeCounter;
		}
	}

	return activeCounter;
}

void AdventureSector::Load(std::ifstream &is, int copyMode)
{
	is >> requiredRunes;

	is.get();//goes to next line

	for (int i = 0; i < 8; ++i)
	{
		maps[i].Load(is, copyMode);
	}
}

void AdventureSector::Save(std::ofstream &of, int copyMode)
{
	of << requiredRunes << "\n";

	for (int i = 0; i < 8; ++i)
	{
		maps[i].Save(of, copyMode);
	}
}

int AdventureWorld::GetNumActiveSectors()
{
	int activeCounter = 0;
	for (int i = 0; i < 8; ++i)
	{
		if (sectors[i].GetNumActiveMaps() > 0)
		{
			++activeCounter;
		}
	}

	return activeCounter;
}

void AdventureWorld::Load(std::ifstream &is, int copyMode)
{
	for (int i = 0; i < 8; ++i)
	{
		sectors[i].Load(is, copyMode);
	}
}

void AdventureWorld::Save(std::ofstream &of, int copyMode)
{
	for (int i = 0; i < 8; ++i)
	{
		sectors[i].Save(of, copyMode);
	}
}

int AdventureFile::GetNumActiveWorlds()
{
	int activeCounter = 0;
	for (int i = 0; i < 8; ++i)
	{
		if (worlds[i].GetNumActiveSectors() > 0)
		{
			activeCounter++;
		}
	}

	return activeCounter;
}

AdventureFile::AdventureFile()
	:hasShardField( 32 * 4 )
{

}

AdventureFile::~AdventureFile()
{
}

bool AdventureFile::Load(const std::string &p_path,
	const std::string &adventureName)
{
	string filePath = p_path + "/" + adventureName + ".adventure";

	ifstream is;
	is.open(filePath);

	if (!is.is_open())
	{
		return false;
	}
	else
	{
		int cMode;
		is >> cMode;

		copyMode = (CopyMode)cMode;

		for (int i = 0; i < 8; ++i)
		{
			worlds[i].Load(is, copyMode);
		}

		is.close();
	}

	return true;
}

void AdventureFile::Save(const std::string &p_path,
	const std::string &adventureName, CopyMode cpy)
{
	string ext = ".adventure";
	ofstream of;

	if (p_path == "")
	{
		of.open(adventureName + ext);
	}
	else
	{
		of.open(p_path + "/" + adventureName + ext);
	}

	assert(of.is_open());

	of << (int)cpy << "\n";

	for (int i = 0; i < 8; ++i)
	{
		worlds[i].Save(of, (int)cpy);
	}

	of.close();
}

int AdventureFile::GetRequiredRunes(Sector *sec)
{
	return GetSector(sec->worldIndex, sec->index).requiredRunes;
}


AdventureMap &AdventureFile::GetMap(int index)
{
	int w = index / 64;
	int s = (index % 64) / 8;
	int m = (index % 8);

	return worlds[w].sectors[s].maps[m];
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
	for (int w = 0; w < 8; ++w)
	{
		for (int s = 0; s < 8; ++s)
		{
			for (int m = 0; m < 8; ++m)
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