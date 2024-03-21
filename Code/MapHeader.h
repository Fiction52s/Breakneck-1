#ifndef __MAPHEADER_H__
#define __MAPHEADER_H__

#include "ShardInfo.h"
#include "LogInfo.h"
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <boost/filesystem.hpp>
#include "steam\steamtypes.h"
#include "MatchParams.h"

struct MapHeader
{
	enum GameModeFlags
	{
		MI_HAS_GOAL,
		MI_HAS_BASES,
	};

	MapHeader();
	void Clear();
	bool CanRunAsMode(int gm);
	bool CanRunAsMode(int numP, int gm);
	bool CanRun(std::vector<int> & numPlayers, std::vector<int> &gameModes );
	bool Load(std::ifstream &is);
	void Save(std::ofstream &of);
	bool Replace(boost::filesystem::path &p);
	int GetLeft();
	int GetTop();
	int GetRight();
	int GetBot();
	//int GetNumPlayerPositions();
	//int GetNumPlayers();
	void ClearSongs();
	void AddSong(const std::string &songName,
		int songLevel);
	int GetNumSongs();
	int ver1;
	int ver2;
	std::string fullName;
	std::string description;
	
	int drainSeconds;
	int goldSeconds;
	int silverSeconds;
	int bronzeSeconds;

	ShardInfo goldRewardShardInfo;
	int silverRewardCategory;
	int silverRewardIndex;
	int bronzeRewardCategory;
	int bronzeRewardIndex;

	std::string envName;
	int envWorldType;
	int leftBounds;
	int topBounds;
	int boundsWidth;
	int boundsHeight;
	int numVertices;
	int numShards;
	int numLogs;
	int numPowers;
	int bossFightType;
	std::vector<ShardInfo> shardInfoVec;
	std::vector<LogInfo> logInfoVec;
	std::vector<int> powerVec;
	std::string preLevelSceneName;
	std::string postLevelSceneName;
	uint64 creatorID;
	std::string creatorName;
	

	std::map < std::string, int > songLevels;
	std::vector<std::string> songOrder;

	bool songLevelsModified;

	//int gameMode;
	int numPlayerSpawns;
	int possibleGameModeTypeFlags;
	int numGameObjects;
	int functionalWidth;
	int functionalHeight;
};

#endif