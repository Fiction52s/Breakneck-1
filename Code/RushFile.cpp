#include "SaveFile.h"
#include <sstream>
#include <assert.h>
#include <iostream>
#include "MapHeader.h"
#include "globals.h"
#include "Session.h"
#include "Actor.h"
#include "LogMenu.h"
#include "RushFile.h"

using namespace std;

RushMapHeaderInfo::RushMapHeaderInfo()
{
	Clear();
}

bool RushMapHeaderInfo::IsLoaded()
{
	return mapType >= 0;
}

void RushMapHeaderInfo::Clear()
{
	mapType = -1;

	mainSongName = "";

	goldSeconds = 0;
	silverSeconds = 0;
	bronzeSeconds = 0;
}

void RushMapHeaderInfo::Set(RushMapHeaderInfo &info)
{
	mapType = info.mapType;

	mainSongName = info.mainSongName;

	goldSeconds = info.goldSeconds;
	silverSeconds = info.silverSeconds;
	bronzeSeconds = info.bronzeSeconds;
}

RushMap::RushMap()
{
	Clear();
}

string RushMap::GetFilePath()
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

std::string RushMap::GetMapPath()
{
	return "Resources\\Maps\\" + path + MAP_EXT;//"Resources\\" + GetFilePath() + MAP_EXT;
}

void RushMap::Clear()
{
	name = "";
	path = "";
	headerInfo.Clear();
}

bool RushMap::Exists()
{
	return name != "";
}

void RushMap::Set(RushMap &am)
{
	name = am.name;
	path = am.path;
	headerInfo.Set(am.headerInfo);
}

bool RushMap::LoadHeaderInfo()
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
		headerInfo.specialItemInfoVec = mh.specialItemInfoVec;
		if (mh.GetNumSongs() > 0)
		{
			headerInfo.mainSongName = mh.songOrder[0];
		}
		else
		{
			headerInfo.mainSongName = "";
		}

		headerInfo.goldSeconds = mh.goldSeconds;
		headerInfo.silverSeconds = mh.silverSeconds;
		headerInfo.bronzeSeconds = mh.bronzeSeconds;

		is.close();
	}
	else
	{
		cout << "unable to open header for: " << filePath << "\n";
		assert(false);
	}

	return true;
}

void RushMap::Load(std::ifstream &is )
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
	path = p.string();
}

void RushMap::Save(std::ofstream &of )
{
	if (name == "")
	{
		of << "----" << "\n";
	}
	else
	{
		of << path << "\n";
	}
}

RushFile::RushFile()
{
	SetVer(1);
	Clear();
}

RushFile::~RushFile()
{
}

void RushFile::SetVer(int v)
{
	ver = v;
}

void RushFile::Clear()
{
	//numMaps = 0;
	numWorlds = 0;
	numMapsPerWorld = 0;
	ver = 0;
	worlds.clear();
}

bool RushFile::Load(const std::string &p_path,
	const std::string &rushName)
{
	string filePath = p_path + "/" + rushName + RUSH_EXT;

	string emptyStr;

	ifstream is;
	is.open(filePath);

	if (!is.is_open())
	{
		return false;
	}
	else
	{
		is >> ver;

		is >> numWorlds;

		is >> numMapsPerWorld;

		is.get();

		worlds.resize(numWorlds);
		for (int w = 0; w < numWorlds; ++w)
		{
			worlds[w].maps.resize(numMapsPerWorld);
			for (int i = 0; i < numMapsPerWorld; ++i)
			{
				worlds[w].maps[i].Load(is);
			}
			std::getline(is, emptyStr);
		}
		/*maps.resize(numMaps);
		for (int i = 0; i < numMaps; ++i)
		{
			maps[i].Load(is);
		}*/

		is.close();
	}

	return true;
}

void RushFile::Save(const std::string &p_path,
	const std::string &rushName )
{
	string ext = RUSH_EXT;
	ofstream of;

	if (p_path == "")
	{
		of.open(rushName + ext);
	}
	else
	{
		of.open("Resources/" + p_path + "/" + rushName + ext);
	}

	assert(of.is_open());

	of << ver << "\n";

	of << numWorlds << "\n";

	of << numMapsPerWorld << "\n";

	for (int w = 0; w < numWorlds; ++w)
	{
		for (int i = 0; i < numMapsPerWorld; ++i)
		{
			worlds[w].maps[i].Save(of);
		}
	}

	//for (int i = 0; i < numMaps; ++i)
	//{
	//	maps[i].Save( of );
	//}

	of.close();
}

RushMap &RushFile::GetMap(int index)
{
	return worlds[0].maps[index]; //not real
}

bool RushFile::LoadMapHeaders()
{
	/*for (int i = 0; i < numMaps; ++i)
	{
		if (maps[i].Exists())
		{
			maps[i].LoadHeaderInfo();
		}
	}*/

	return true;
}