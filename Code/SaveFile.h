#ifndef __SAVEFILE_H__
#define __SAVEFILE_H__

#include <fstream>
#include <SFML/Graphics.hpp>
#include "ShardTypes.h"
#include <list>
#include <string>

struct Sector;
struct World;




struct BitField
{
	BitField(int p_numOptions);
	bool Load(std::ifstream &is);
	void Save(std::ofstream &of);
	~BitField();
	void SetBit(int index, bool val );
	bool GetBit(int index);
	void Reset();
	sf::Uint32 *optionField;
	int numOptions;
	int numFields;
};



//struct Shard
//{
//	enum ShardType
//	{
//		NEW_SHARDS, //the player has not viewed
//		//new shards
//		W1_TEACH_JUMP,
//	};
//
//
//	enum ShardGroupType
//	{
//		GT_TRI,
//		GT_QUAD,
//		GT_Count
//	};
//
//	Shard();
//	std::string animName;
//	ShardGroupType groupType;
//	int groupIndex;
//	bool Load(std::ifstream is);
//
//};

struct Level
{
	
	enum BitOption : int
	{
		COMPLETE,
		UNLOCKEDTOPBONUS,
		UNLOCKEDBOTTOMBONUS,
	};

	Level();
	Sector *sec;
	BitField optionField;
	std::string name;
	void SetComplete(bool comp);
	void UpdateShardNameList();
	float GetCapturedShardsPortion();
	std::list<std::string> shardNameList;
	bool shardsLoaded;

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
	int index;
	std::string GetFullName();
	float GetCompletionPercentage();
};

struct Sector
{
	Sector();
	~Sector();
	bool HasTopBonus(int index);
	bool hasBottomBonus(int index);
	int numLevels;
	Level *levels;
	int index;
	World *world;
	int numUnlockConditions;
	void UpdateShardNameList();
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
	int numSectors;
	Sector *sectors;
	SaveFile *sf;
	int index;
	bool Load(std::ifstream &is);
	bool Save(std::ofstream &of);
	int GetNumSectorTypeComplete(int sType);
	float GetCompletionPercentage();	
};

struct SaveFile
{
	SaveFile( const std::string &name );
	~SaveFile();
	void Save();
	void Load();
	float GetCompletionPercentage();
	int GetShardNum();
	int GetTotalShardNum();

	BitField shardField;
	BitField newShardField; //for unviewed shards
	std::string fileName;
	bool ShardIsCaptured(ShardType sType);
	World *worlds;
	int numWorlds;
};

#endif