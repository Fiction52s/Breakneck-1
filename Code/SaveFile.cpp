#include "SaveFile.h"
#include <sstream>
#include <assert.h>
#include <iostream>
#include "Session.h"
//#include "MainMenu.h"
#include "Enemy_Shard.h"
#include "MapHeader.h"
#include "MainMenu.h"
#include <boost/filesystem.hpp>
#include "globals.h"
#include "Actor.h"
#include "LogMenu.h"

//appdata
//#include <windows.h>
//#include <KnownFolders.h>




using namespace std;
using namespace boost::filesystem;

LevelScore::LevelScore()
{
	Reset();
}

void LevelScore::Reset()
{
	bestFramesToBeat = 0;
	medalLevel = 0; //no medals
}

void LevelScore::Save(ofstream &of)
{
	of << bestFramesToBeat << " " << medalLevel << "\n";
}

void LevelScore::Load(ifstream &is)
{
	is >> bestFramesToBeat;
	is >> medalLevel;
}

bool LevelScore::HasBronze()
{
	return medalLevel >= 1;
}

bool LevelScore::HasSilver()
{
	return medalLevel >= 2;
}

bool LevelScore::HasGold()
{
	return medalLevel >= 3;
}


SaveFile::SaveFile(const std::string &p_name, AdventureFile *p_adventure)
	:levelsJustBeatenField(ADVENTURE_MAX_NUM_LEVELS), 
	levelsBeatenField(ADVENTURE_MAX_NUM_LEVELS),
	upgradeField(Session::PLAYER_OPTION_BIT_COUNT),
	upgradesTurnedOnField(Session::PLAYER_OPTION_BIT_COUNT),
	visualRewardsField(ADVENTURE_MAX_NUM_LEVELS),
	logField(LogDetailedInfo::MAX_LOGS),
	adventureFile( p_adventure ),
	name( p_name ),
	mostRecentWorldSelected( 0 )
{
	SetAsDefault();

	string dataFolder = MainMenu::GetInstance()->appDataPath + "SaveData\\";

	myFolderName = dataFolder + name + "\\";
	fileName = myFolderName + name + SAVE_EXT;
	replayFolderName = myFolderName + name + "_bestreplay\\";
}

SaveFile::~SaveFile()
{
}

void SaveFile::CreateSaveWorlds()
{
	
}

int SaveFile::GetBestFrames()
{
	int total = 0;
	int temp;

	for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS; ++i)
	{
		if (adventureFile->GetMap(i).Exists())
		{
			total += levelScores[i].bestFramesToBeat;
		}
	}

	return total;
}

std::string SaveFile::GetBestTimeString()
{
	int totalFrames = GetBestFrames();

	int allFrames = totalFrames;

	int framesInSecond = 60;
	int framesInMinute = framesInSecond * 60;
	int framesInHour = framesInMinute * 60;
	int numHours = totalFrames / framesInHour;

	totalFrames = totalFrames - numHours * framesInHour;

	int numMinutes = totalFrames / framesInMinute;

	totalFrames = totalFrames - numMinutes * framesInMinute;

	int numSeconds = totalFrames / framesInSecond;

	totalFrames = totalFrames - numSeconds * framesInSecond;

	stringstream ss;
	ss << numHours << " hr " << numMinutes << " min " << numSeconds << " sec\n(Frames: " << allFrames << ")";

	return ss.str();
}

int SaveFile::GetBestFramesWorld(int w)
{
	int total = 0;
	int wStart = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD;
	int wEnd = (w + 1) * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD;
	for (int i = wStart; i < wEnd; ++i)
	{
		if (adventureFile->GetMap(i).Exists())
		{
			total += levelScores[i].bestFramesToBeat;
		}
	}

	return total;
}

int SaveFile::GetBestFramesSector(int w, int s)
{
	int total = 0;
	int sStart = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD + s * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR;
	int sEnd = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD + (s + 1) * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR;

	for (int i = sStart; i < sEnd; ++i)
	{
		if (adventureFile->GetMap(i).Exists())
		{
			total += levelScores[i].bestFramesToBeat;
		}
	}
	
	return total;
}

int SaveFile::GetBestFramesLevel(int w, int s, int m)
{
	int i = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD + s * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR + m;
	if (adventureFile->GetMap(i).Exists())
	{
		return levelScores[i].bestFramesToBeat;
	}

	return -1;
}

int SaveFile::GetBestFramesLevel(int index)
{
	if (adventureFile->GetMap(index).Exists())
	{
		return levelScores[index].bestFramesToBeat;
	}

	return -1;
}


int SaveFile::GetNumGolds()
{
	int total = 0;
	int temp;

	for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS; ++i)
	{
		if (adventureFile->GetMap(i).Exists())
		{
			if (levelScores[i].HasGold() )
			{
				total++;
			}
			
		}
	}

	return total;
}

int SaveFile::GetNumGoldsWorld(int w)
{
	int total = 0;
	int wStart = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD;
	int wEnd = (w + 1) * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD;
	for (int i = wStart; i < wEnd; ++i)
	{
		if (adventureFile->GetMap(i).Exists() && levelScores[i].HasGold())
		{
			total++;
		}
	}

	return total;
}

int SaveFile::GetNumGoldsSector(int w, int s)
{
	int total = 0;
	int sStart = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD + s * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR;
	int sEnd = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD + (s + 1) * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR;

	for (int i = sStart; i < sEnd; ++i)
	{
		if (adventureFile->GetMap(i).Exists() && levelScores[i].HasGold())
		{
			total++;
		}
	}

	return total;
}

bool SaveFile::HasGoldForLevel(int w, int s, int m)
{
	int i = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD + s * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR + m;
	if (adventureFile->GetMap(i).Exists() && levelScores[i].HasGold())
	{
		return true;
	}

	return false;
}

bool SaveFile::HasGoldForLevel(int index)
{
	if (adventureFile->GetMap(index).Exists() && levelScores[index].HasGold() )
	{
		return true;
	}

	return false;
}

int SaveFile::GetNumSilvers()
{
	int total = 0;
	int temp;

	for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS; ++i)
	{
		if (adventureFile->GetMap(i).Exists())
		{
			if (levelScores[i].HasSilver() )
			{
				total++;
			}

		}
	}

	return total;
}

int SaveFile::GetNumSilversWorld(int w)
{
	int total = 0;
	int wStart = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD;
	int wEnd = (w + 1) * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD;
	for (int i = wStart; i < wEnd; ++i)
	{
		if (adventureFile->GetMap(i).Exists() && levelScores[i].HasSilver())
		{
			total++;
		}
	}

	return total;
}

int SaveFile::GetNumSilversSector(int w, int s)
{
	int total = 0;
	int sStart = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD + s * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR;
	int sEnd = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD + (s + 1) * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR;

	for (int i = sStart; i < sEnd; ++i)
	{
		if (adventureFile->GetMap(i).Exists() && levelScores[i].HasSilver())
		{
			total++;
		}
	}

	return total;
}

bool SaveFile::HasSilverForLevel(int w, int s, int m)
{
	int i = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD + s * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR + m;
	if (adventureFile->GetMap(i).Exists() && levelScores[i].HasSilver())
	{
		return true;
	}

	return false;
}

bool SaveFile::HasSilverForLevel(int index)
{
	if (adventureFile->GetMap(index).Exists() && levelScores[index].HasSilver())
	{
		return true;
	}

	return false;
}

int SaveFile::GetNumBronzes()
{
	int total = 0;
	int temp;

	for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS; ++i)
	{
		if (adventureFile->GetMap(i).Exists())
		{
			if (levelScores[i].HasBronze() )
			{
				total++;
			}

		}
	}

	return total;
}

int SaveFile::GetNumBronzesWorld(int w)
{
	int total = 0;
	int wStart = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD;
	int wEnd = (w + 1) * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD;
	for (int i = wStart; i < wEnd; ++i)
	{
		if (adventureFile->GetMap(i).Exists() && levelScores[i].HasBronze())
		{
			total++;
		}
	}

	return total;
}

int SaveFile::GetNumBronzesSector(int w, int s)
{
	int total = 0;
	int sStart = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD + s * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR;
	int sEnd = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD + (s + 1) * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR;

	for (int i = sStart; i < sEnd; ++i)
	{
		if (adventureFile->GetMap(i).Exists() && levelScores[i].HasBronze())
		{
			total++;
		}
	}

	return total;
}

bool SaveFile::HasBronzeForLevel(int w, int s, int m)
{
	int i = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD + s * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR + m;
	if (adventureFile->GetMap(i).Exists() && levelScores[i].HasBronze())
	{
		return true;
	}

	return false;
}

bool SaveFile::HasBronzeForLevel(int index)
{
	if (adventureFile->GetMap(index).Exists() && levelScores[index].HasBronze())
	{
		return true;
	}

	return false;
}

int SaveFile::GetWorldStart(int w)
{
	return ADVENTURE_MAX_NUM_LEVELS_PER_WORLD * w;
}
int SaveFile::GetWorldEnd(int w)
{
	return ADVENTURE_MAX_NUM_LEVELS_PER_WORLD * (w + 1);
}

int SaveFile::GetSectorStart(int w, int s)
{
	return ADVENTURE_MAX_NUM_LEVELS_PER_WORLD * w + ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR * s;
}
int SaveFile::GetSectorEnd(int w, int s)
{
	return ADVENTURE_MAX_NUM_LEVELS_PER_WORLD * w + ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR * (s + 1);
}

int SaveFile::GetMapIndex(int w, int s, int m)
{
	return GetSectorStart(w, s) + m;
}

void SaveFile::CalcProgress(int start, int end, float &totalMaps,
	float &totalBeaten)
{
	totalMaps = 0;
	totalBeaten = 0;
	for (int i = start; i < end; ++i)
	{
		if (adventureFile->GetMap(i).Exists())
		{
			++totalMaps;
			if (levelsBeatenField.GetBit(i))
			{
				++totalBeaten;
			}
		}
	}
}

void SaveFile::CalcShardProgress(BitField &b, float &totalShards,
	float &totalCaptured)
{
	totalShards = 0;
	totalCaptured = 0;
	for (int i = 0; i < ShardInfo::MAX_SHARDS; ++i)
	{
		if (b.GetBit(i))
		{
			++totalShards;
			if (IsShardCaptured(i))
			{
				++totalCaptured;
			}
		}
	}
}

void SaveFile::CalcLogProgress(BitField &b, float &totalLogs,
	float &totalCaptured)
{
	totalLogs = 0;
	totalCaptured = 0;
	for (int i = 0; i < LogDetailedInfo::MAX_LOGS; ++i)
	{
		if (b.GetBit(i))
		{
			++totalLogs;
			if (HasLog(i))
			{
				++totalCaptured;
			}
		}
	}
}

int SaveFile::GetTotalMaps()
{
	int totalMaps = 0;
	for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS; ++i)
	{
		if (adventureFile->GetMap(i).Exists())
		{
			++totalMaps;
		}
	}
	return totalMaps;
}
int SaveFile::GetTotalMapsBeaten()
{
	int totalMapsBeaten = 0;
	for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS; ++i)
	{
		if (adventureFile->GetMap(i).Exists())
		{
			if (levelsBeatenField.GetBit(i))
			{
				++totalMapsBeaten;
			}
		}
	}
	return totalMapsBeaten;
}

float SaveFile::CalcCompletionPercentage(int start, int end, BitField & b)
{
	float totalMaps, totalBeaten, totalShards, totalCaptured;

	CalcProgress(start, end, totalMaps, totalBeaten);
	CalcShardProgress(b, totalShards, totalCaptured);

	float shardPortion = 0;
	if (totalShards > 0)
	{
		shardPortion = totalCaptured / totalShards;
	}
	
	float portion = totalBeaten / totalMaps;

	float portionShardsWorth = .1;

	float totalPortion = shardPortion * portionShardsWorth + portion * (1.0 - portionShardsWorth);

	if (totalPortion > .99f)
		totalPortion = 1.0f;

	return totalPortion * 100.f;
}

float SaveFile::GetCompletionPercentage()
{
	return CalcCompletionPercentage(0, ADVENTURE_MAX_NUM_LEVELS, adventureFile->hasShardField);
}

float SaveFile::GetCompletionPercentageWorld(int w)
{
	return CalcCompletionPercentage(GetWorldStart(w), GetWorldEnd(w), 
		adventureFile->worlds[w].hasShardField);
}

float SaveFile::GetCompletionPercentageSector(int w, int s)
{
	return CalcCompletionPercentage(GetSectorStart(w,s), GetSectorEnd(w,s), 
		adventureFile->worlds[w].sectors[s].hasShardField );
}

bool SaveFile::IsRangeComplete(int start, int end)
{
	bool complete = true;
	for (int i = start; i < end; ++i)
	{
		AdventureMap &am = adventureFile->GetMap(i);
		if (am.Exists() && !levelsBeatenField.GetBit( i ) )
		{
			complete = false;
			break;
		}
	}

	return complete;
}

int SaveFile::GetNumShardsCaptured()
{
	return upgradeField.GetOnCount(Actor::SHARD_START_INDEX);
}

int SaveFile::GetNumShardsTotal()
{
	return adventureFile->hasShardField.GetOnCount();
}

int SaveFile::GetNumLogsCaptured()
{
	return logField.GetOnCount();
}

int SaveFile::GetNumLogsTotal()
{
	return adventureFile->hasLogField.GetOnCount();
}

bool SaveFile::IsLevelJustBeaten(Level *lev)
{
	return levelsJustBeatenField.GetBit(lev->index);
}

void SaveFile::SetLevelNotJustBeaten(Level *lev)
{
	levelsJustBeatenField.SetBit(lev->index, false);
}

bool SaveFile::TrySetRecordTime(int totalFrames,
	Level *lev )
{
	int index = lev->index;//GetMapIndex(w, s, m);
	int toBeat = levelScores[index].bestFramesToBeat;
	if ( toBeat == 0 || totalFrames < toBeat)
	{
		levelScores[index].bestFramesToBeat = totalFrames;
		return true;
	}

	return false;
}

bool SaveFile::TryUnlockGoldMedal(int totalFrames, Level *lev)
{
	int index = lev->index;
	
	if (!HasGoldForLevel(index))
	{
		int goldFrames = adventureFile->GetMap(index).headerInfo.goldSeconds * 60;
		if (totalFrames <= goldFrames)
		{
			levelScores[index].medalLevel = 3;
			return true;
		}
	}

	return false;
}

bool SaveFile::TryUnlockSilverMedal(int totalFrames, Level *lev)
{
	int index = lev->index;

	if (!HasSilverForLevel(index))
	{
		int silverFrames = adventureFile->GetMap(index).headerInfo.silverSeconds * 60;
		if (totalFrames <= silverFrames)
		{
			levelScores[index].medalLevel = 2;
			return true;
		}
	}

	return false;
}

bool SaveFile::TryUnlockBronzeMedal(int totalFrames, Level *lev)
{
	int index = lev->index;

	if (!HasBronzeForLevel(index))
	{
		int bronzeFrames = adventureFile->GetMap(index).headerInfo.bronzeSeconds * 60;
		if (totalFrames <= bronzeFrames)
		{
			levelScores[index].medalLevel = 1;
			return true;
		}
	}

	return false;
}

bool SaveFile::IsLevelLastInSector( Level *lev )
{
	int levIndex = lev->index;
	int world = levIndex / ADVENTURE_MAX_NUM_LEVELS_PER_WORLD;
	int sector = (levIndex % ADVENTURE_MAX_NUM_LEVELS_PER_WORLD) / ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR;
	int ind = (levIndex % 8);

	AdventureSector &as = adventureFile->GetSector(world, sector);
	for (int i = ind+1; i < ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR; ++i)
	{
		if (as.maps[i].Exists())
		{
			return false;
		}
	}

	return true;
}

int SaveFile::GetNumCompleteWorlds( AdventurePlanet *planet )
{
	int numComplete = 0;
	for (int i = 0; i < planet->numWorlds; ++i)
	{
		if (IsCompleteWorld(&planet->worlds[i]))
		{
			numComplete++;
		}
	}

	return numComplete;
}

void SaveFile::CompleteLevel(Level *lev)
{
	int index = lev->index;
	levelsJustBeatenField.SetBit(index, true);
	levelsBeatenField.SetBit(index, true);
}

bool SaveFile::IsCompleteWorld( World *world )
{
	int w = world->index;
	return IsRangeComplete(GetWorldStart(w), GetWorldEnd(w));
}

bool SaveFile::IsCompleteSector(Sector *sector)
{
	int first = sector->GetLevelIndex(0);
	int last = sector->GetLevelIndex(sector->numLevels - 1);
	
	return IsRangeComplete(first, last+1);
}

bool SaveFile::IsCompleteLevel(Level *lev)
{
	int index = lev->index;
	AdventureMap &am = adventureFile->GetMap(index);
	if (am.Exists() && levelsBeatenField.GetBit(index))
	{
		return true;
	}

	assert(am.Exists());

	return false;
}

bool SaveFile::IsUnlockedSector( World *world, Sector *sector)
{
	int secIndex = sector->index;
	for (int i = 0; i < secIndex; ++i)
	{
		if (!IsCompleteSector(&world->sectors[i]))
		{
			return false;
		}
	}

	return true;
}

bool SaveFile::HasUpgrade(int pow)
{
	assert(pow < ADVENTURE_MAX_NUM_LEVELS);
	return upgradeField.GetBit(pow);
}

void SaveFile::UnlockUpgrade(int pow)
{
	assert(pow < ADVENTURE_MAX_NUM_LEVELS);
	upgradeField.SetBit(pow, true);
}

bool SaveFile::HasVisualReward(int pType)
{
	return visualRewardsField.GetBit(pType);
}

void SaveFile::UnlockVisualReward(int pType)
{
	visualRewardsField.SetBit(pType, true);
}

bool SaveFile::HasLog(int lType)
{
	return logField.GetBit(lType);
}

void SaveFile::UnlockLog(int lType)
{
	logField.SetBit(lType, true);
}

bool SaveFile::IsShardCaptured(int sType)
{
	return upgradeField.GetBit( Actor::SHARD_START_INDEX + sType  );
}

void SaveFile::SetVer(int v)
{
	ver = v;
}

bool SaveFile::LoadInfo(ifstream &is)
{
	if (is.is_open())
	{
		int v;

		is >> v;

		if (v < ver)
		{
			if (v == 1)
			{
				SetAsDefault();
				//this should be the only possibility for now

				int defaultSkinIndex;
				is >> defaultSkinIndex;

				visualInfo.skinIndex = defaultSkinIndex;

				is >> mostRecentWorldSelected;

				levelsBeatenField.Load(is);

				for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS; ++i)
				{
					is >> levelScores[i].bestFramesToBeat; //since its just this int here
				}

				BitField oldUpgradeField(256);
				BitField oldUpgradeTurnedOnField(256);

				oldUpgradeField.Load(is);
				oldUpgradeTurnedOnField.Load(is);
				//never converts this to the new shards, so your upgrades are reset


				logField.Load(is);

				is.close();

				Save();
				return Load();

			}
			else
			{
				assert(0);
				return false;
			}
		}
		
		visualInfo.Load(is);
		
		is >> mostRecentWorldSelected;

		is >> warpCharge;

		levelsBeatenField.Load(is);

		for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS; ++i)
		{
			levelScores[i].Load(is);
		}
		
		upgradeField.Load(is);
		upgradesTurnedOnField.Load(is);

		visualRewardsField.Load(is);

		logField.Load(is);

		is.close();
		return true;
	}
	else
	{
		return false;
	}
}

bool SaveFile::IsUnlockedLevel(Sector *sec, int index )
{
	for (int i = 0; i < sec->numLevels; ++i)
	{
		if (i == index)
		{
			return true;
		}
		else
		{
			if (!IsCompleteLevel(sec->GetLevel(i)))
			{
				return false;
			}
		}
	}

	return true;
}

bool SaveFile::IsFullyCompleteLevel(Level *lev)
{
	int ind = lev->index;
	AdventureMap &am = adventureFile->GetMap(ind);
	
	if (IsCompleteLevel(lev))
	{
		bool hasAllShards = false;
		bool hasAllLogs = false;

		float totalShards, totalCaptured;
		CalcShardProgress(am.headerInfo.hasShardField, totalShards, totalCaptured);

		if (totalShards == totalCaptured)
		{
			hasAllShards = true;
		}

		float totalLogs, totalLogsCaptured;
		CalcLogProgress(am.headerInfo.hasLogField, totalLogs, totalLogsCaptured);

		if (totalLogs == totalLogsCaptured)
		{
			hasAllLogs = true;
		}

		if (hasAllShards && hasAllLogs)
			return true;
	}
	return false;
}

//return true on normal loading, and false if you need to make a default
bool SaveFile::Load()
{
	ifstream is;

	is.open(fileName);

	if (!LoadInfo(is))
	{
		SetAsDefault();
		return false;
	}
	else
	{
		return true;
	}
}

void SaveFile::Save()
{
	if (!boost::filesystem::exists(myFolderName))
	{
		boost::filesystem::create_directory(myFolderName);
	}

	if (!boost::filesystem::exists(replayFolderName))
	{
		boost::filesystem::create_directory(replayFolderName);
	}

	ofstream of;

	of.open(fileName);

	cout << "saving save file" << endl;

	if (of.is_open())
	{
		of << ver << endl;

		visualInfo.Save(of);

		of << mostRecentWorldSelected << endl;

		of << warpCharge << endl;

		levelsBeatenField.Save(of);

		for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS; ++i)
		{
			levelScores[i].Save(of);
		}

		upgradeField.Save(of);
		upgradesTurnedOnField.Save(of);

		visualRewardsField.Save(of);

		logField.Save(of);

		of.close();
	}
	else
	{
		cout << "error saving file: " << fileName << endl;
		assert(false);
	}

	/*if (!boost::filesystem::exists(replayFolderName))
	{
		boost::filesystem::create_directory(replayFolderName);
	}*/
}

void SaveFile::Delete()
{
	boost::filesystem::remove(fileName);
	if (boost::filesystem::exists(replayFolderName))
	{
		boost::filesystem::remove_all(replayFolderName);
	}

	//boost::filesystem::directory
	//boost::filesystem::create_directory
	SetAsDefault();
}

void SaveFile::SetAndSave(SaveFile *saveFile)
{
	levelsBeatenField.Set(saveFile->levelsBeatenField);

	for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS; ++i)
	{
		levelScores[i] = saveFile->levelScores[i];
	}

	upgradeField.Set(saveFile->upgradeField);
	upgradesTurnedOnField.Set(saveFile->upgradesTurnedOnField);

	visualRewardsField.Set(saveFile->visualRewardsField);

	logField.Set(saveFile->logField);

	visualInfo = saveFile->visualInfo;

	Save();

	if (boost::filesystem::exists(replayFolderName))
	{
		boost::filesystem::remove(replayFolderName);
	}
	//boost::filesystem::create_directory(replayFolderName);

	MainMenu::copyDirectoryRecursively(saveFile->replayFolderName, replayFolderName);
}

void SaveFile::SetAsDefault()
{
	//version 1 is early access launch
	//version 2 is post-medals
	SetVer(2);

	levelsBeatenField.Reset();

	mostRecentWorldSelected = 0;
	
	for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS; ++i)
	{
		levelScores[i].Reset();
	}

	upgradeField.Reset();
	upgradesTurnedOnField.Reset();

	visualRewardsField.Reset();

	logField.Reset();

	visualInfo.Reset();

	warpCharge = 0;
}

GlobalSaveFile::GlobalSaveFile()
	:visualRewardsField(ADVENTURE_MAX_NUM_LEVELS)
{
	
	fileName = MainMenu::GetInstance()->appDataPath + "globalsave" + string(GLOBAL_SAVE_EXT);
	SetToDefaults();
}

bool GlobalSaveFile::Load()
{
	ifstream is;

	is.open(fileName);

	if (is.is_open())
	{
		int v;
		is >> v;

		if (v < ver)
		{
			cout << "global save file is out of date. updating" << endl;
			is.close();
			SetToDefaults();
			Save();
			return Load();
		}
		else
		{
			//cout << "global save file is newer than the version I'm trying to use" << endl;
			//assert(ver == v);
		}

		visualRewardsField.Load(is);

		//add more later
		is.close();
		return true;
	}
	else
	{
		return false;
	}
}

void GlobalSaveFile::SetVer(int v )
{
	ver = v;
}

void GlobalSaveFile::Save()
{
	ofstream of;

	of.open(fileName);

	//cout << "saving global savefile" << endl;

	if (of.is_open())
	{
		of << ver << endl;

		visualRewardsField.Save(of);

		of.close();
	}
	else
	{
		cout << "error saving global savefile: " << fileName << endl;
		assert(false);
	}
}

void GlobalSaveFile::UnlockVisual(int visIndex)
{
	if (!visualRewardsField.GetBit(visIndex))
	{
		visualRewardsField.SetBit(visIndex, true);
		Save();
	}
}

bool GlobalSaveFile::IsVisualRewardUnlocked(int visIndex)
{
	return visualRewardsField.GetBit(visIndex);
}

void GlobalSaveFile::SetToDefaults()
{
	SetVer(2);
	visualRewardsField.Reset();

}