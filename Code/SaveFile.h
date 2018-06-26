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
	/*	JUSTUNLOCKEDTOP,
		JUSTUNLOCKEDBOTTOM,
		JUSTUNLOCKED,*/
	};

	Level();
	Sector *sec;
	BitField optionField;
	std::string name;
	bool justBeaten;
	int bossFightType;
	void SetComplete(bool comp);
	//void SetJustUnlocked(bool unlocked);
	//void SetJustUnlockedTop(bool unlocked);
	//void SetJustUnlockedBottom(bool unlocked);

	//bool GetJustUnlocked();
	//bool GetJustUnlockedTop();
	//bool GetJustUnlockedBottom();
	void UpdateFromMapHeader();
	float GetCapturedShardsPortion();
	std::list<std::string> shardNameList;
	bool shardsLoaded;

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
	bool IsLevelUnlocked(int index);
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

	void UpdateShardNameList();
	int GetNumShardsCaptured();
	int GetNumTotalShards();
	std::list<std::string> shardNameList;
	/*void SetJustUnlocked(int world,
		int sec, int lev);*/

	BitField shardField;
	BitField newShardField; //for unviewed shards
	std::string fileName;
	bool ShardIsCaptured(ShardType sType);
	World *worlds;
	int numWorlds;
};

#endif