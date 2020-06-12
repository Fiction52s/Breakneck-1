#ifndef __SAVEFILE_H__
#define __SAVEFILE_H__

#include <fstream>
#include <SFML/Graphics.hpp>
#include "ShardTypes.h"
#include <list>
#include <string>
#include "BitField.h"
//#include "Actor.h"

struct Sector;
struct World;
struct Actor;


struct AdventureMap
{
	std::string name;
	std::string path;
	void Load(std::ifstream &is, int copyMode );
	void Save(std::ofstream &of, int copyMode);
};

struct AdventureSector
{
	AdventureSector();
	int requirements[4];
	int numRequirements[4];
	AdventureMap maps[8];
	void Load(std::ifstream &is, int copyMode );
	void Save(std::ofstream &of, int copyMode );
};

struct AdventureWorld
{
	AdventureSector sectors[8];
	void Load(std::ifstream &is, int copyMode );
	void Save(std::ofstream &of, int copyMode );
};

struct AdventureFile
{
	enum CopyMode : int
	{
		PATH,
		COPY,
	};

	AdventureWorld worlds[8];
	bool Load(const std::string &p_path,
		const std::string &adventureName);
	void Save(const std::string &p_path,
		const std::string &adventureName, CopyMode cpy );
	CopyMode copyMode;
};



struct Level
{
	
	enum BitOption : int
	{
		COMPLETE,
		UNLOCKEDTOPBONUS,
		UNLOCKEDBOTTOMBONUS,
	/*	JUSTUNLOCKEDTOP,
		JUSTUNLOCKEDBOTTOM,
		JUSTUNLOCKED,*/
	};

	Level();
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
	void UnlockTopBonus();
	void UnlockBottomBonus();
	bool TopBonusUnlocked();
	bool BottomBonusUnlocked();
	std::string GetFullName();
	float GetCompletionPercentage();


	//void SetJustUnlocked(bool unlocked);
	//void SetJustUnlockedTop(bool unlocked);
	//void SetJustUnlockedBottom(bool unlocked);

	//bool GetJustUnlocked();
	//bool GetJustUnlockedTop();
	//bool GetJustUnlockedBottom();


	std::list<std::string> shardNameList;
	bool shardsLoaded;
	Sector *sec;
	BitField optionField;
	std::string name;
	bool justBeaten;
	int bossFightType;
	int bestTimeFrames;
	int index;
};

struct Sector
{
	Sector();
	~Sector();
	bool HasTopBonus(int index);
	bool hasBottomBonus(int index);
	int numLevels;
	Level *levels;
	std::string name;
	int index;
	World *world;
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
struct World
{
	World();
	~World();
	std::string name;
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

struct SaveFile
{
	SaveFile( const std::string &name );
	~SaveFile();
	void Save();
	bool Load();
	bool LoadInfo(std::ifstream &is );
	float GetCompletionPercentage();
	void CopyFromDefault();

	int GetTotalFrames();
	void UpdateShardNameList();
	int GetNumShardsCaptured();
	int GetNumTotalShards();
	bool HasNewShards();
	std::list<std::string> shardNameList;
	/*void SetJustUnlocked(int world,
		int sec, int lev);*/

	BitField powerField;
	bool HasPowerUnlocked(int pType);
	void UnlockPower(int pType);

	BitField momentaField;

	BitField shardField;
	BitField newShardField; //for unviewed shards
	std::string fileName;
	bool ShardIsCaptured(int sType);
	World *worlds;
	int numWorlds;

	std::string controlProfileName;

	const static int MAX_SECTORS = 8;
	const static int MAX_LEVELS_PER_SECTOR = 8;


};

#endif