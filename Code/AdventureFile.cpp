#include "SaveFile.h"
#include <sstream>
#include <assert.h>
#include <iostream>

using namespace std;

bool AdventureMap::Exists()
{
	return name != "";
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