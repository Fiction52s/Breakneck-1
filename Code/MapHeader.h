#ifndef __MAPHEADER_H__
#define __MAPHEADER_H__

#include "ShardInfo.h"
#include <fstream>
#include <string>
#include <map>
#include <vector>

struct MapHeader
{
	enum MapType
	{
		T_RACEFIGHT,
		T_STANDARD,
	};

	MapHeader()
		:ver1(0), ver2(0), collectionName("nothing"),
		description("no description"), gameMode(T_STANDARD),
		leftBounds(0), topBounds(0), boundsWidth(0), boundsHeight(0),
		numVertices(-1), songLevelsModified(false), numShards(0),
		drainSeconds(60), bossFightType(0), envName("w1_01"),
		envWorldType(0)
	{

	}

	bool Load(std::ifstream &is);
	void Save(std::ofstream &of);
	int GetLeft();
	int GetTop();
	int GetRight();
	int GetBot();
	int ver1;
	int ver2;
	std::string collectionName;
	std::string description;
	MapType gameMode;
	int drainSeconds;
	std::string envName;
	int envWorldType;
	int leftBounds;
	int topBounds;
	int boundsWidth;
	int boundsHeight;
	int numVertices;
	int numShards;
	int bossFightType;
	std::vector<ShardInfo> shardInfoVec;

	std::map < std::string, int > songLevels;

	bool songLevelsModified;
};

#endif