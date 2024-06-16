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

Sector::Sector()
{
	index = -1;
	numLevels = 0;
	levels = NULL;
	worldIndex = -1;
}

Level *Sector::GetLevel(int ind)
{
	return &levels[ind];
}
int Sector::GetLevelIndex(int ind)
{
	return levels[ind].index;
}
Sector::~Sector()
{
	if (levels != NULL)
	{
		delete[] levels;
	}
}

World::World()
{
	index = -1;
	numSectors = 0;
	sectors = NULL;
}

World::~World()
{
	if (sectors != NULL)
	{
		delete []sectors;
	}
}

LevelData::LevelData()
{
	Reset();
}

bool LevelData::IsBeaten()
{
	return levelState > 0;
}

void LevelData::BeatLevel()
{
	levelState = 1;
}

void LevelData::Reset()
{
	levelState = 0; //unbeaten
	bestFrames = 0;
	medalLevel = 0; //no medals
	specialUnlockedItemsVec.clear();
}

void LevelData::Save(ofstream &of)
{
	of << levelState << " " << bestFrames << " " << medalLevel << "\n";

	of << specialUnlockedItemsVec.size() << "\n";
	for (auto it = specialUnlockedItemsVec.begin(); it != specialUnlockedItemsVec.end(); ++it)
	{
		(*it).Save(of);
	}
}

void LevelData::Load(ifstream &is)
{
	is >> levelState;
	is >> bestFrames;
	is >> medalLevel;

	int numSpecialItems;
	is >> numSpecialItems;
	specialUnlockedItemsVec.clear();
	specialUnlockedItemsVec.resize(numSpecialItems);
	for (int i = 0; i < numSpecialItems; ++i)
	{
		specialUnlockedItemsVec[i].Load(is);
	}
}

bool LevelData::HasBronze()
{
	return medalLevel >= 1;
}

bool LevelData::HasSilver()
{
	return medalLevel >= 2;
}

bool LevelData::HasGold()
{
	return medalLevel >= 3;
}


SaveFile::SaveFile(const std::string &p_name, AdventureFile *p_adventure)
	:kinOptionField(Session::PLAYER_OPTION_BIT_COUNT),
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
			total += levelData[i].bestFrames;
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
			total += levelData[i].bestFrames;
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
			total += levelData[i].bestFrames;
		}
	}
	
	return total;
}

int SaveFile::GetBestFramesLevel(int w, int s, int m)
{
	int i = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD + s * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR + m;
	if (adventureFile->GetMap(i).Exists())
	{
		return levelData[i].bestFrames;
	}

	return -1;
}

int SaveFile::GetBestFramesLevel(int index)
{
	if (adventureFile->GetMap(index).Exists())
	{
		return levelData[index].bestFrames;
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
			if (levelData[i].HasGold() )
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
		if (adventureFile->GetMap(i).Exists() && levelData[i].HasGold())
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
		if (adventureFile->GetMap(i).Exists() && levelData[i].HasGold())
		{
			total++;
		}
	}

	return total;
}

bool SaveFile::HasGoldForLevel(int w, int s, int m)
{
	int i = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD + s * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR + m;
	if (adventureFile->GetMap(i).Exists() && levelData[i].HasGold())
	{
		return true;
	}

	return false;
}

bool SaveFile::HasGoldForLevel(int index)
{
	if (adventureFile->GetMap(index).Exists() && levelData[index].HasGold() )
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
			if (levelData[i].HasSilver() )
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
		if (adventureFile->GetMap(i).Exists() && levelData[i].HasSilver())
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
		if (adventureFile->GetMap(i).Exists() && levelData[i].HasSilver())
		{
			total++;
		}
	}

	return total;
}

bool SaveFile::HasSilverForLevel(int w, int s, int m)
{
	int i = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD + s * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR + m;
	if (adventureFile->GetMap(i).Exists() && levelData[i].HasSilver())
	{
		return true;
	}

	return false;
}

bool SaveFile::HasSilverForLevel(int index)
{
	if (adventureFile->GetMap(index).Exists() && levelData[index].HasSilver())
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
			if (levelData[i].HasBronze() )
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
		if (adventureFile->GetMap(i).Exists() && levelData[i].HasBronze())
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
		if (adventureFile->GetMap(i).Exists() && levelData[i].HasBronze())
		{
			total++;
		}
	}

	return total;
}

bool SaveFile::HasBronzeForLevel(int w, int s, int m)
{
	int i = w * ADVENTURE_MAX_NUM_LEVELS_PER_WORLD + s * ADVENTURE_MAX_NUM_LEVELS_PER_SECTOR + m;
	if (adventureFile->GetMap(i).Exists() && levelData[i].HasBronze())
	{
		return true;
	}

	return false;
}

bool SaveFile::HasBronzeForLevel(int index)
{
	if (adventureFile->GetMap(index).Exists() && levelData[index].HasBronze())
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

			if (levelData[i].IsBeaten())
			{
				++totalBeaten;
			}
		}
	}
}

void SaveFile::CalcLogProgress(float &totalLogs,
	float &totalCaptured)
{
	totalLogs = 0;
	totalCaptured = 0;

	/*for (int i = 0; i < LogDetailedInfo::MAX_LOGS; ++i)
	{
		if (b.GetBit(i))
		{
			++totalLogs;
			if (HasLog(i))
			{
				++totalCaptured;
			}
		}
	}*/
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
			if (levelData[i].IsBeaten())
			{
				++totalMapsBeaten;
			}
		}
	}
	return totalMapsBeaten;
}

float SaveFile::CalcCompletionPercentage(int start, int end )
{
	float totalMaps, totalBeaten;

	CalcProgress(start, end, totalMaps, totalBeaten);

	float portion = totalBeaten / totalMaps;

	return portion * 100.f;

	/*CalcShardProgress(b, totalShards, totalCaptured);

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

	return totalPortion * 100.f;*/
}

float SaveFile::GetCompletionPercentage()
{
	return CalcCompletionPercentage(0, ADVENTURE_MAX_NUM_LEVELS );
}

float SaveFile::GetCompletionPercentageWorld(int w)
{
	return CalcCompletionPercentage(GetWorldStart(w), GetWorldEnd(w) );
}

float SaveFile::GetCompletionPercentageSector(int w, int s)
{
	return CalcCompletionPercentage(GetSectorStart(w,s), GetSectorEnd(w,s) );
}

bool SaveFile::IsRangeComplete(int start, int end)
{
	bool complete = true;
	for (int i = start; i < end; ++i)
	{
		AdventureMap &am = adventureFile->GetMap(i);
		if (am.Exists() && !levelData[i].IsBeaten() )
		{
			complete = false;
			break;
		}
	}

	return complete;
}

int SaveFile::GetNumLogsCollected()
{
	std::vector<SpecialItemInfo> collectedLogVec; //excluding reps
	collectedLogVec.reserve(256);

	bool found = false;

	for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS; ++i)
	{
		AdventureMap &am = adventureFile->GetMap(i);
		if (am.Exists())
		{
			std::vector<SpecialItemInfo> &vec = levelData[i].specialUnlockedItemsVec;
			for (auto it = vec.begin(); it != vec.end(); ++it)
			{
				if ((*it).itemType == SpecialItemInfo::SI_LOG)
				{
					found = false;
					for (auto logIt = collectedLogVec.begin(); logIt != collectedLogVec.end(); ++logIt)
					{
						if ((*logIt).itemIndex0 == (*it).itemIndex0 && (*logIt).itemIndex1 == (*it).itemIndex1)
						{
							found = true;
							break;
						}
					}

					if (!found)
					{
						collectedLogVec.push_back((*it));
					}
				}
			}

			break;
		}
	}

	return collectedLogVec.size();
}

int SaveFile::GetNumLogsTotal()
{
	std::vector<SpecialItemInfo> allLogVec; //excluding reps
	allLogVec.reserve(256);

	bool found = false;

	for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS; ++i)
	{
		AdventureMap &am = adventureFile->GetMap(i);
		if (am.Exists())
		{
			std::vector<SpecialItemInfo> &vec = am.headerInfo.specialItemInfoVec; //use the adventure map vector rather than the save data
			for (auto it = vec.begin(); it != vec.end(); ++it)
			{
				if ((*it).itemType == SpecialItemInfo::SI_LOG)
				{
					found = false;
					for (auto logIt = allLogVec.begin(); logIt != allLogVec.end(); ++logIt)
					{
						if ((*logIt).itemIndex0 == (*it).itemIndex0 && (*logIt).itemIndex1 == (*it).itemIndex1)
						{
							found = true;
							break;
						}
					}

					if (!found)
					{
						allLogVec.push_back((*it));
					}
				}
			}
			
			break;
		}
	}

	return allLogVec.size();
}

bool SaveFile::TrySetRecordTime(int totalFrames,
	Level *lev )
{
	int index = lev->index;//GetMapIndex(w, s, m);
	int toBeat = levelData[index].bestFrames;
	if ( toBeat == 0 || totalFrames < toBeat)
	{
		levelData[index].bestFrames = totalFrames;
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
			levelData[index].medalLevel = 3;
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
			levelData[index].medalLevel = 2;
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
			levelData[index].medalLevel = 1;
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

	levelData[index].BeatLevel();
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
	if (am.Exists() && levelData[index].IsBeaten())
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

bool SaveFile::IsKinOptionOn(int i)
{
	return kinOptionField.GetBit(i);
}

void SaveFile::SetKinOption(int i, bool b)
{
	kinOptionField.SetBit(i, b);
}

bool SaveFile::HasLog(int lType)
{
	return false;//logField.GetBit(lType);
}

void SaveFile::UnlockLog(int lType)
{
	//logField.SetBit(lType, true);
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
		
		is >> mostRecentWorldSelected;

		visualInfo.Load(is);
		kinOptionField.Load(is);
		is >> exp;
		is >> currency;
		
		for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS; ++i)
		{
			levelData[i].Load(is);
		}

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

		float totalLogs, totalLogsCaptured;
		CalcLogProgress(totalLogs, totalLogsCaptured);

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

		of << mostRecentWorldSelected << endl;

		visualInfo.Save(of);
		kinOptionField.Save(of);
		of << exp << " " << currency << endl;

		for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS; ++i)
		{
			levelData[i].Save(of);
		}

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
	mostRecentWorldSelected = saveFile->mostRecentWorldSelected;
	visualInfo = saveFile->visualInfo;
	kinOptionField.Set(saveFile->kinOptionField);

	for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS; ++i)
	{
		levelData[i] = saveFile->levelData[i];
	}

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

	mostRecentWorldSelected = 0;

	visualInfo.Reset();

	kinOptionField.Reset();

	exp = 0;
	currency = 0;
	
	for (int i = 0; i < ADVENTURE_MAX_NUM_LEVELS; ++i)
	{
		levelData[i].Reset();
	}
}

int SaveFile::GetRelativeExp()
{
	int totalExp = exp;
	int relExp = 0;
	int currKinLevel = 1;
	int nextLevelExp;

	while (true)
	{
		nextLevelExp = GetExpForNextKinLevel(currKinLevel);

		if (totalExp > nextLevelExp)
		{
			currKinLevel += 1;
			totalExp -= nextLevelExp;
		}
		else
		{
			break;
		}
	}

	return totalExp;
}

int SaveFile::GetKinLevelFromExp()
{
	int totalExp = exp;
	int relExp = 0;
	int currKinLevel = 1;
	int nextLevelExp;

	while( true )
	{
		nextLevelExp = GetExpForNextKinLevel(currKinLevel);

		if (totalExp > nextLevelExp)
		{
			currKinLevel += 1;
			totalExp -= nextLevelExp;
		}
		else
		{
			break;
		}
	}
	
	return currKinLevel;
}

int SaveFile::GetExpForNextKinLevel(int lev)
{
	switch (lev)
	{
	case 1:
		return 50;
	case 2:
		return 50;
	default:
		return 50;
	}
}

GlobalSaveFile::GlobalSaveFile()
	:cosmeticsField(MAX_COSMETICS)
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

		cosmeticsField.Load(is);

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

		cosmeticsField.Save(of);

		of.close();
	}
	else
	{
		cout << "error saving global savefile: " << fileName << endl;
		assert(false);
	}
}

void GlobalSaveFile::UnlockCosmetic(int visIndex)
{
	if (!cosmeticsField.GetBit(visIndex))
	{
		cosmeticsField.SetBit(visIndex, true);
		Save();
	}
}

bool GlobalSaveFile::IsCosmeticUnlocked(int visIndex)
{
	return cosmeticsField.GetBit(visIndex);
}

void GlobalSaveFile::SetToDefaults()
{
	SetVer(2);
	cosmeticsField.Reset();

}