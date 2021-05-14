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

struct Actor;
struct AdventureFile;

struct Level
{
	int index;
};

struct Sector
{
	int index;
	int numLevels;
	Level * levels;
	int worldIndex;
	Level *GetLevel(int ind)
	{
		return &levels[ind];
	}
	int GetLevelIndex(int ind)
	{
		return levels[ind].index;
	}
	~Sector()
	{
		delete[] levels;
	}

};

struct World
{
	int index;
	int numSectors;
	Sector *sectors;

	~World()
	{
		delete[] sectors;
	}
};

struct Planet
{
	Planet(AdventureFile &af);
	~Planet();
	int numWorlds;
	World *worlds;	
};

struct AdventureMapHeaderInfo
{
	int mapType;
	std::vector<ShardInfo> shardInfoVec;
	bool IsLoaded() { return mapType >= 0; }
	BitField hasShardField;
	std::string mainSongName;

	AdventureMapHeaderInfo()
		:mapType(-1), hasShardField( ShardInfo::MAX_SHARDS )
	{

	}
};

struct AdventureMap
{
	std::string name;
	std::string path;
	std::string GetFilePath();
	std::string GetMapPath();
	AdventureMapHeaderInfo headerInfo;
	bool Exists();
	void Load(std::ifstream &is, int copyMode );
	void Save(std::ofstream &of, int copyMode);
	bool LoadHeaderInfo();
};

struct AdventureSector
{	
	AdventureSector();
	BitField hasShardField;
	int requiredRunes;
	AdventureMap maps[8];
	void Load(std::ifstream &is, int copyMode );
	void Save(std::ofstream &of, int copyMode );
	int GetNumActiveMaps();
};

struct AdventureWorld
{
	BitField hasShardField;
	AdventureSector sectors[8];

	AdventureWorld()
		:hasShardField(ShardInfo::MAX_SHARDS)
	{

	}
	
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

	CopyMode copyMode;
	BitField hasShardField;
	AdventureWorld worlds[8];


	AdventureFile();
	~AdventureFile();
	bool Load(const std::string &p_path,
		const std::string &adventureName);
	void Save(const std::string &p_path,
		const std::string &adventureName, CopyMode cpy );
	int GetRequiredRunes(Sector *sec);
	AdventureMap &GetMap(int index);
	AdventureMapHeaderInfo &GetMapHeaderInfo(int index);

	AdventureSector &GetSector(int w, int s);
	AdventureSector &GetAdventureSector(Sector *sec);
	AdventureWorld &GetWorld(int w);
	
	
	int GetNumActiveWorlds();
	bool LoadMapHeaders();
};


struct LevelScore
{
	int bestFramesToBeat;

	LevelScore();
	void Reset();
	void Save(std::ofstream &of);
	void Load(std::ifstream &is);
};

struct SaveFile
{
	SaveFile( const std::string &name,
		AdventureFile *adventureFile );
	~SaveFile();

	//functionality
	void Save();
	bool Load();
	bool LoadInfo(std::ifstream &is );
	void Delete();
	void CopyTo(SaveFile *saveFile);

	void SetAsDefault();

	int GetTotalMaps();
	int GetTotalMapsBeaten();

	void CreateSaveWorlds();
	//queries
	
	float GetCompletionPercentage();
	float GetCompletionPercentageWorld( int w);
	float GetCompletionPercentageSector(int w, int s);


	std::string GetBestTimeString();
	int GetBestFrames();
	int GetBestFramesWorld(int w);
	int GetBestFramesSector(int w, int s);
	int GetBestFramesLevel(int w, int s, int m);
	int GetBestFramesLevel(int index);

	bool HasNewShards();
	bool HasUpgrade(int pType);
	void UnlockUpgrade(int pType);
	bool ShardIsCaptured(int sType);

	bool IsCompleteSector(Sector *sector);
	bool IsCompleteWorld(World *world);
	bool IsCompleteLevel(Level *lev);
	void CompleteLevel(Level *lev);

	bool IsUnlockedSector(Sector *sector);
	bool TrySetRecordTime(int totalFrames,
		Level *lev );

	int GetNumShardsCaptured();
	int GetNumShardsTotal();

	int GetNumCompleteWorlds( Planet *planet );

	bool IsLevelLastInSector( Level *lev );

	bool IsLevelJustBeaten(Level *lev);
	void SetLevelNotJustBeaten(Level *lev);

	bool IsUnlockedLevel(Sector *sec, int index );
	bool IsFullyCompleteLevel(Level *lev);

	const static int MAX_SECTORS = 8;
	const static int MAX_LEVELS_PER_SECTOR = 8;

	//info
	std::string name;
	std::string fileName;
	AdventureFile *adventureFile;
	int mostRecentWorldSelected;
	
	//state
	BitField levelsJustBeatenField;

	//state that gets saved out
	std::string controlProfileName;
	BitField levelsBeatenField;
	LevelScore levelScores[512];
	BitField upgradeField;
	BitField momentaField;
	BitField shardField;
	BitField newShardField; //for unviewed shards	
	int defaultSkinIndex;

private:
	void CalcProgress(int start, int end, float &totalMaps,
		float &totalBeaten);
	void CalcShardProgress(BitField &b, float &totalShards,
		float &totalCaptured);
	float CalcCompletionPercentage(
		int start, int end, BitField &b);
	bool IsRangeComplete(int start, int end);
	int GetWorldStart(int w);
	int GetWorldEnd(int w);
	int GetSectorStart(int w, int s);
	int GetSectorEnd(int w, int s);
	int GetMapIndex(int w, int s, int m);

};

struct GlobalSaveFile
{
	BitField skinField;
	static std::string fileName;

	GlobalSaveFile();
	void SetToDefaults();
	void Save();
	bool Load();
	bool IsSkinUnlocked(int skinIndex);
	void UnlockSkin(int skinIndex);
};

#endif