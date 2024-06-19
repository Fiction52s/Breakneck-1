#ifndef __RUSHFILE_H__
#define __RUSHFILE_H__

#include <fstream>
#include <SFML/Graphics.hpp>
#include <list>
#include <string>
#include "BitField.h"
#include "ShardInfo.h"
#include "steam/steam_api.h"
#include "LogInfo.h"
#include "PlayerVisualInfo.h"
#include "MapHeader.h"

struct RushMapHeaderInfo
{
	int mapType;

	std::vector<SpecialItemInfo> specialItemInfoVec;
	std::string mainSongName;

	int goldSeconds;
	int silverSeconds;
	int bronzeSeconds;

	RushMapHeaderInfo();
	void Set(RushMapHeaderInfo &inf);
	void Clear();
	bool IsLoaded();
};

struct RushMap
{
	std::string name;
	std::string path;
	std::string GetFilePath();
	std::string GetMapPath();
	AdventureMapHeaderInfo headerInfo;

	RushMap();
	bool Exists();
	void Set(RushMap &rm);
	void Clear();
	void Load(std::ifstream &is);
	void Save(std::ofstream &of);
	bool LoadHeaderInfo();

};

struct RushFile
{
	int ver;
	int numMaps;
	std::vector<RushMap> maps;
	//int numActiveWorlds;

	RushFile();
	~RushFile();
	void Clear();
	void SetVer(int v);
	bool Load(const std::string &p_path,
		const std::string &adventureName);
	void Save(const std::string &p_path,
		const std::string &adventureName);
	RushMap &GetMap(int index);
	bool LoadMapHeaders();
};

#endif