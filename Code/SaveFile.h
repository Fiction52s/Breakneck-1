#ifndef __SAVEFILE_H__
#define __SAVEFILE_H__

#include <fstream>
#include <SFML/Graphics.hpp>
#include <list>
#include <string>
#include "BitField.h"
#include "ShardInfo.h"
#include "steam/steam_api.h"
#include "LogInfo.h"
//#include "Actor.h"

struct Actor;
struct AdventureFile;

const static int ADVENTURE_MAX_NUM_WORLDS = 8;
const static int ADVENTURE_MAX_NUM_SECTORS_PER_WORLD = 8;
const static int ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR = 8;

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

struct AdventurePlanet
{
	AdventurePlanet(AdventureFile &af);
	~AdventurePlanet();
	int numWorlds;
	World *worlds;	
};

struct AdventureMapHeaderInfo
{
	int mapType;
	std::vector<ShardInfo> shardInfoVec;
	std::string mainSongName;
	BitField hasShardField;
	BitField hasLogField;
	std::vector<LogInfo> logInfoVec;
	std::vector<int> powerVec;

	AdventureMapHeaderInfo();
	void Set(AdventureMapHeaderInfo &inf);
	void Clear();
	bool IsLoaded();
};

struct AdventureMap
{
	std::string name;
	std::string path;
	std::string GetFilePath();
	std::string GetMapPath();
	AdventureMapHeaderInfo headerInfo;

	AdventureMap();
	bool Exists();
	void Set(AdventureMap &am);
	void Clear();
	void Load(std::ifstream &is, int copyMode );
	void Save(std::ofstream &of, int copyMode);
	bool LoadHeaderInfo();

};

struct AdventureSector
{	
	AdventureSector();
	BitField hasShardField;
	AdventureMap maps[ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR];
	void Clear();
	void Load(std::ifstream &is, int ver, int copyMode );
	void Save(std::ofstream &of, int ver, int copyMode );
	int GetNumExistingMaps();
};

struct AdventureWorld
{
	BitField hasShardField;
	AdventureSector sectors[ADVENTURE_MAX_NUM_SECTORS_PER_WORLD];

	AdventureWorld()
		:hasShardField(ShardInfo::MAX_SHARDS)
	{

	}
	
	void Clear();
	void Load(std::ifstream &is, int ver, int copyMode );
	void Save(std::ofstream &of, int ver, int copyMode );
	int GetNumExistingSectors();
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
	AdventureWorld worlds[ADVENTURE_MAX_NUM_WORLDS];
	int ver;

	AdventureFile();
	~AdventureFile();
	void Clear();
	void SetVer(int v);
	bool Load(const std::string &p_path,
		const std::string &adventureName);
	void Save(const std::string &p_path,
		const std::string &adventureName, CopyMode cpy );
	void GetOriginalProgressionUpgradeField( int mapIndex, BitField &bf);
	void GetOriginalProgressionLogField(int mapIndex, BitField &bf);
	AdventureMap &GetMap(int index);
	AdventureMapHeaderInfo &GetMapHeaderInfo(int index);
	void GetMapIndexes( int mIndex, int &w, int &s, int &m);
	std::string GetLeaderboardName(int mIndex);

	AdventureSector &GetSector(int w, int s);
	AdventureSector &GetAdventureSector(Sector *sec);
	AdventureWorld &GetWorld(int w);
	
	
	int GetNumExistingWorlds();
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
	void SetAndSave(SaveFile *saveFile);

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

	bool HasUpgrade(int pType);
	void UnlockUpgrade(int pType);
	bool HasLog(int lType);
	void UnlockLog(int lType);
	bool IsShardCaptured(int sType);

	bool IsCompleteSector(Sector *sector);
	bool IsCompleteWorld(World *world);
	bool IsCompleteLevel(Level *lev);
	void CompleteLevel(Level *lev);

	bool IsUnlockedSector( World *world, Sector *sector);
	bool TrySetRecordTime(int totalFrames,
		Level *lev );

	int GetNumShardsCaptured();
	int GetNumShardsTotal();

	int GetNumCompleteWorlds( AdventurePlanet *planet );

	bool IsLevelLastInSector( Level *lev );

	bool IsLevelJustBeaten(Level *lev);
	void SetLevelNotJustBeaten(Level *lev);

	bool IsUnlockedLevel(Sector *sec, int index );
	bool IsFullyCompleteLevel(Level *lev);
	void SetVer(int v);

	const static int MAX_SECTORS = 8;
	const static int MAX_LEVELS_PER_SECTOR = 8;

	//info
	std::string name;
	std::string fileName;
	std::string replayFolderName;
	AdventureFile *adventureFile;
	int mostRecentWorldSelected;
	
	//state
	BitField levelsJustBeatenField;

	//state that gets saved out
	
	BitField levelsBeatenField;
	LevelScore levelScores[512];
	BitField upgradeField;
	BitField upgradesTurnedOnField;
	BitField logField;

	int defaultSkinIndex;

	int ver;

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

	int ver;

	GlobalSaveFile();
	void SetToDefaults();
	void Save();
	bool Load();
	void SetVer(int v);
	bool IsSkinUnlocked(int skinIndex);
	void UnlockSkin(int skinIndex);
};



#endif