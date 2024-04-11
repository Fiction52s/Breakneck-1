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
#include "PlayerVisualInfo.h"
#include "MapHeader.h"
//#include "Actor.h"

struct Actor;
struct AdventureFile;

const static int ADVENTURE_MAX_NUM_WORLDS = 8;
const static int ADVENTURE_MAX_NUM_SECTORS_PER_WORLD = 8;
const static int ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR = 8;
const static int ADVENTURE_MAX_NUM_LEVELS_PER_WORLD = ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR * ADVENTURE_MAX_NUM_SECTORS_PER_WORLD;
const static int ADVENTURE_MAX_NUM_LEVELS = ADVENTURE_MAX_NUM_WORLDS * ADVENTURE_MAX_NUM_SECTORS_PER_WORLD * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR;

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

	std::vector<SpecialItemInfo> specialItemInfoVec;

	std::string mainSongName;
	
	int goldSeconds;
	int silverSeconds;
	int bronzeSeconds;

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
	BitField hasLogField;
	AdventureMap maps[ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR];
	void Clear();
	void Load(std::ifstream &is, int ver, int copyMode );
	void Save(std::ofstream &of, int ver, int copyMode );
	int GetNumExistingMaps();
};

struct AdventureWorld
{
	BitField hasShardField;
	BitField hasLogField;
	AdventureSector sectors[ADVENTURE_MAX_NUM_SECTORS_PER_WORLD];

	AdventureWorld();
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
	BitField hasLogField;
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
	static void GetMapIndexes( int mIndex, int &w, int &s, int &m);
	std::string GetLeaderboardName(int mIndex);
	std::string GetLeaderboardDisplayName(int mIndex);

	AdventureSector &GetSector(int w, int s);
	AdventureSector &GetAdventureSector(Sector *sec);
	AdventureWorld &GetWorld(int w);
	
	
	int GetNumExistingWorlds();
	bool LoadMapHeaders();
};


struct LevelScore
{
	int bestFramesToBeat;
	int medalLevel;

	LevelScore();
	void Reset();
	void Save(std::ofstream &of);
	void Load(std::ifstream &is);
	bool HasBronze();
	bool HasSilver();
	bool HasGold();
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

	int GetNumGolds();
	int GetNumGoldsWorld(int w);
	int GetNumGoldsSector(int w, int s);
	bool HasGoldForLevel(int w, int s, int m);
	bool HasGoldForLevel(int index);

	int GetNumSilvers();
	int GetNumSilversWorld(int w);
	int GetNumSilversSector(int w, int s);
	bool HasSilverForLevel(int w, int s, int m);
	bool HasSilverForLevel(int index);

	int GetNumBronzes();
	int GetNumBronzesWorld(int w);
	int GetNumBronzesSector(int w, int s);
	bool HasBronzeForLevel(int w, int s, int m);
	bool HasBronzeForLevel(int index);

	bool HasVisualReward(int pType);
	void UnlockVisualReward(int pType);

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
	bool TryUnlockGoldMedal(int totalFrames,
		Level *lev);
	bool TryUnlockSilverMedal(int totalFrames,
		Level *lev);
	bool TryUnlockBronzeMedal(int totalFrames,
		Level *lev);

	int GetNumShardsCaptured();
	int GetNumShardsTotal();

	int GetNumLogsCaptured();
	int GetNumLogsTotal();

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
	std::string myFolderName;
	AdventureFile *adventureFile;
	int mostRecentWorldSelected;
	
	//state
	BitField levelsJustBeatenField;

	//state that gets saved out
	
	BitField levelsBeatenField;
	LevelScore levelScores[512];

	PlayerVisualInfo visualInfo; //skin, etc.

	BitField upgradeField; //then 32 for power + 512 for shards
	BitField upgradesTurnedOnField;
	BitField visualRewardsField;
	
	BitField logField;
	int warpCharge;

	int ver;

private:
	void CalcProgress(int start, int end, float &totalMaps,
		float &totalBeaten);
	void CalcShardProgress(BitField &b, float &totalShards,
		float &totalCaptured);
	void CalcLogProgress(BitField &b, float &totalLogs,
		float &totalLogsCaptured);
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
	BitField visualRewardsField;
	std::string fileName;
	int ver;

	GlobalSaveFile();
	void SetToDefaults();
	void Save();
	bool Load();
	void SetVer(int v);
	bool IsVisualRewardUnlocked(int skinIndex);
	void UnlockVisual(int skinIndex);
};



#endif