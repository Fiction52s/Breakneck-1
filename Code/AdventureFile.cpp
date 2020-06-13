#include "SaveFile.h"
#include <sstream>
#include <assert.h>
#include <iostream>
#include "MapHeader.h"

using namespace std;

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
		is.open(name);
	}
	else
	{
		is.open(path + "\\" + name);
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

	of << (int)cpy << "\n";

	for (int i = 0; i < 8; ++i)
	{
		worlds[i].Save(of, (int)cpy);
	}

	of.close();
}

AdventureMap &AdventureFile::GetMap(int index)
{
	int w = index / 64;
	int s = (index % 64) / 8;
	int m = (index % 8);

	return worlds[w].sectors[s].maps[m];
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