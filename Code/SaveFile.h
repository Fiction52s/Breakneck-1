#ifndef __SAVEFILE_H__
#define __SAVEFILE_H__

#include <fstream>
#include <SFML/Graphics.hpp>
#include "ShardTypes.h"
#include <list>
#include <string>
#include "BitField.h"
#include "ShardInfo.h"
//#include "Actor.h"

struct Sector;
struct World;
struct Actor;


struct AdventureMapHeaderInfo
{
	int mapType;
	std::vector<ShardInfo> shardInfoVec;
	bool IsLoaded() { return mapType >= 0; }
	/*bool HasShard(int w, int li)
	{
		for (auto it = shardInfoVec.begin();
			it != shardInfoVec.end(); ++it)
		{
			if ((*it).world == w && (*it).localIndex == li)
				return true;
		}

		return false;
	}*/

	AdventureMapHeaderInfo()
		:mapType(-1)
	{

	}
};

struct AdventureMap
{
	std::string name;
	std::string path;
	AdventureMapHeaderInfo headerInfo;

	bool Exists();
	void Load(std::ifstream &is, int copyMode );
	void Save(std::ofstream &of, int copyMode);
	bool LoadHeaderInfo();
};

struct AdventureSector
{	
	AdventureSector();
	int requiredRunes;
	AdventureMap maps[8];
	void Load(std::ifstream &is, int copyMode );
	void Save(std::ofstream &of, int copyMode );
	int GetNumActiveMaps();
};

struct AdventureWorld
{
	AdventureSector sectors[8];
	void Load(std::ifstream &is, int copyMode );
	void Save(std::ofstream &of, int copyMode );
	int GetNumActiveSectors();
};

struct AdventureFile
{
	enum CopyMode : int
	{
		PATH,
		COPY,
	};

	AdventureFile();
	AdventureWorld worlds[8];
	bool Load(const std::string &p_path,
		const std::string &adventureName);
	void Save(const std::string &p_path,
		const std::string &adventureName, CopyMode cpy );
	AdventureMap &GetMap(int index);
	CopyMode copyMode;
	BitField hasShardField;
	int GetNumActiveWorlds();
	bool LoadMapHeaders();
};


struct SaveLevel
{
	
	enum BitOption : int
	{
		COMPLETE,
	};

	SaveLevel();
	void SetComplete(bool comp);
	bool TrySetRecord(int numFrames);
	bool IsLastInSector();
	void UpdateFromMapHeader();
	float GetCapturedShardsPortion();
	bool IsOneHundredPercent();
	bool GetComplete();
	int GetNumTotalShards();
	int GetNumShardsCaptured();
	bool Load(std::ifstream &is);
	void Save(std::ofstream &of);
	void Reset();
	std::string GetFullName();
	float GetCompletionPercentage();

	std::list<std::string> shardNameList;
	bool shardsLoaded;
	SaveSector *sec;
	BitField optionField;
	std::string name;
	bool justBeaten;
	int bossFightType;
	int bestTimeFrames;
	int index;
};

struct SaveSector
{
	SaveSector();
	~SaveSector();
	bool HasTopBonus(int index);
	bool hasBottomBonus(int index);
	int numLevels;
	Level *levels;
	std::string name;
	int index;
	SaveWorld *world;
	bool IsLevelUnlocked(int index);
	int numUnlockConditions;
	void UpdateShardNameList();
	int GetTotalFrames();
	int GetNumTotalShards();
	int GetNumShardsCaptured();
	std::list<std::string> shardNameList;
	int *conditions;
	int sectorType;
	int *numTypesNeeded;
	void Save(std::ofstream &of);
	bool Load(std::ifstream &is);
	bool IsConditionFulfilled(int i);
	bool IsComplete();
	bool IsUnlocked();
	float GetCompletionPercentage();
	std::map<int,Level> topBonuses;
	std::map<int, Level> bottomBonuses;
};

struct SaveFile;
struct SaveWorld
{
	SaveWorld();
	~SaveWorld();
	int numSectors;
	Sector *sectors;
	SaveFile *sf;
	int index;
	int GetTotalFrames();
	void UpdateShardNameList();
	int GetNumTotalShards();
	bool Load(std::ifstream &is);
	bool Save(std::ofstream &of);
	int GetNumSectorTypeComplete(int sType);
	float GetCompletionPercentage();
	int GetNumShardsCaptured();
	std::list<std::string> shardNameList;
};

struct LevelScore
{
	LevelScore();
	void Save(std::ofstream &of);
	void Load(std::ifstream &is);
	int bestFramesToBeat;
	std::vector<std::string> shardNames;
	int GetNumTotalShards();
	int GetNumShardsCaptured();
};

struct SaveFile
{
	SaveFile( const std::string &name,
		AdventureFile &adventureFile );
	~SaveFile();

	//functionality
	void Save();
	bool Load();
	bool LoadInfo(std::ifstream &is );
	void CopyFromDefault();

	//queries
	float GetCompletionPercentage();

	int GetBestFrames();
	int GetBestFramesWorld(int w);
	int GetBestFramesSector(int w, int s);
	int GetBestFramesLevel(int w, int s, int m);

	void UpdateShardNameList();
	int GetNumShardsCaptured();
	int GetNumTotalShards();
	bool HasNewShards();
	bool HasUpgrade(int pType);
	void UnlockUpgrade(int pType);
	bool ShardIsCaptured(int sType);

	//info
	std::string fileName;
	//std::vector<ShardInfo> shardInfoVec[8];
	
	//state
	BitField levelsBeatenField;
	LevelScore levelScores[512];
	BitField upgradeField;
	BitField momentaField;
	BitField shardField;
	BitField newShardField; //for unviewed shards
	std::string controlProfileName;

	AdventureFile &adventureFile;

	const static int MAX_SECTORS = 8;
	const static int MAX_LEVELS_PER_SECTOR = 8;


};

#endif