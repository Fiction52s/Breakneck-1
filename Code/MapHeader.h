#ifndef __MAPHEADER_H__
#define __MAPHEADER_H__

#include "ShardInfo.h"
#include "LogInfo.h"
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <boost/filesystem.hpp>

struct MapHeader
{
	enum MapType
	{
		T_BASIC,
		T_REACHENEMYBASE,
		T_FIGHT,
		T_RACE,
	};

	MapHeader()
		:ver1(0), ver2(0), collectionName("nothing"),
		description("no description"), gameMode(T_BASIC),
		leftBounds(0), topBounds(0), boundsWidth(0), boundsHeight(0),
		numVertices(-1), songLevelsModified(false), numShards(0),
		numLogs(0),drainSeconds(60), bossFightType(0), envName("w1_01"),
		envWorldType(0), preLevelSceneName("NONE"),
		postLevelSceneName("NONE")
	{

	}

	bool Load(std::ifstream &is);
	void Save(std::ofstream &of);
	bool Replace(boost::filesystem::path &p);
	int GetLeft();
	int GetTop();
	int GetRight();
	int GetBot();
	int GetNumPlayerPositions();
	int GetNumPlayers();
	void ClearSongs();
	void AddSong(const std::string &songName,
		int songLevel);
	int GetNumSongs();
	int ver1;
	int ver2;
	std::string collectionName;
	std::string description;
	int gameMode;
	int drainSeconds;
	std::string envName;
	int envWorldType;
	int leftBounds;
	int topBounds;
	int boundsWidth;
	int boundsHeight;
	int numVertices;
	int numShards;
	int numLogs;
	int bossFightType;
	std::vector<ShardInfo> shardInfoVec;
	std::vector<LogInfo> logInfoVec;
	std::string preLevelSceneName;
	std::string postLevelSceneName;

	std::map < std::string, int > songLevels;
	std::vector<std::string> songOrder;

	bool songLevelsModified;
};

#endif