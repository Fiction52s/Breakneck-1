#ifndef __SAVEFILE_H__
#define __SAVEFILE_H__

#include <fstream>
#include <SFML/Graphics.hpp>

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
	Level();
	Sector *sec;
	BitField optionField;
	std::string name;
	void SetComplete(bool comp);
	bool GetComplete();
	bool Load(std::ifstream &is);
	void Save(std::ofstream &of);
	void Reset();
	int index;
	std::string GetFullName();
};

struct Sector
{
	Sector();
	~Sector();
	int numLevels;
	Level *levels;
	int index;
	World *world;
	void Save(std::ofstream &of);
	bool Load(std::ifstream &is);
};

struct World
{
	World();
	~World();
	int numSectors;
	Sector *sectors;
	int index;
	bool Load(std::ifstream &is);
	bool Save(std::ofstream &of);
};

struct SaveFile
{
	SaveFile( const std::string &name );
	~SaveFile();
	void Save();
	void Load();
	BitField shardField;
	std::string fileName;
	World *worlds;
	int numWorlds;
};

#endif