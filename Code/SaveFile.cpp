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

using namespace std;
using namespace boost::filesystem;

LevelScore::LevelScore()
{
	Reset();
}

void LevelScore::Reset()
{
	bestFramesToBeat = 0;
}

void LevelScore::Save(ofstream &of)
{
	of << bestFramesToBeat << "\n";
}

void LevelScore::Load(ifstream &is)
{
	is >> bestFramesToBeat;
}

SaveFile::SaveFile(const std::string &p_name, AdventureFile *p_adventure)
	:levelsBeatenField(512),
	upgradeField(Session::PLAYER_OPTION_BIT_COUNT),
	upgradesTurnedOnField(Session::PLAYER_OPTION_BIT_COUNT),
	logField(256),
	levelsJustBeatenField( 512 ),
	adventureFile( p_adventure ),
	name( p_name ),
	defaultSkinIndex( 0 ),
	mostRecentWorldSelected( 0 )
{
	SetVer(1);

	string dataFolder = "Resources\\Adventure\\SaveData\\";

	fileName = dataFolder + name + SAVE_EXT;
	replayFolderName = dataFolder + name + "\\";
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

	for (int i = 0; i < 512; ++i)
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
	int wStart = w * 64;
	int wEnd = (w + 1) * 64;
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
	int sStart = w * 64 + s * 8;
	int sEnd = w * 64 + (s + 1) * 8;

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
	int i = w * 64 + s * 8 + m;
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


int SaveFile::GetWorldStart(int w)
{
	return 64 * w;
}
int SaveFile::GetWorldEnd(int w)
{
	return 64 * (w + 1);
}

int SaveFile::GetSectorStart(int w, int s)
{
	return 64 * w + 8 * s;
}
int SaveFile::GetSectorEnd(int w, int s)
{
	return 64 * w + 8 * (s + 1);
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

int SaveFile::GetTotalMaps()
{
	int totalMaps = 0;
	for (int i = 0; i < 512; ++i)
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
	for (int i = 0; i < 512; ++i)
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
	return CalcCompletionPercentage(0, 512, adventureFile->hasShardField);
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
	return shardField.GetOnCount();
}

int SaveFile::GetNumShardsTotal()
{
	return adventureFile->hasShardField.GetOnCount();
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

bool SaveFile::IsLevelLastInSector( Level *lev )
{
	int levIndex = lev->index;
	int world = levIndex / 64;
	int sector = (levIndex % 64) / 8;
	int ind = (levIndex % 8);

	AdventureSector &as = adventureFile->GetSector(world, sector);
	for (int i = ind+1; i < 8; ++i)
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
	return upgradeField.GetBit(pow);
}

void SaveFile::UnlockUpgrade(int pow)
{
	upgradeField.SetBit(pow, true);
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
		is >> ver;
		
		is >> defaultSkinIndex;
		is >> mostRecentWorldSelected;

		levelsBeatenField.Load(is);

		for (int i = 0; i < 512; ++i)
		{
			levelScores[i].Load(is);
		}
		
		upgradeField.Load(is);
		upgradesTurnedOnField.Load(is);
		logField.Load(is);
		is.close();

		if (!boost::filesystem::exists(replayFolderName))
		{
			boost::filesystem::create_directory(replayFolderName);
		}
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
		float totalShards, totalCaptured;
		CalcShardProgress(am.headerInfo.hasShardField, totalShards, totalCaptured);

		if (totalShards == totalCaptured)
		{
			return true;
		}
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
	ofstream of;

	of.open(fileName);

	cout << "saving save file" << endl;

	if (of.is_open())
	{
		of << ver << endl;

		of << defaultSkinIndex << endl;

		of << mostRecentWorldSelected << endl;

		levelsBeatenField.Save(of);

		for (int i = 0; i < 512; ++i)
		{
			levelScores[i].Save(of);
		}

		upgradeField.Save(of);
		upgradesTurnedOnField.Save(of);
		logField.Save(of);

		of.close();
	}
	else
	{
		cout << "error saving file: " << fileName << endl;
		assert(false);
	}

	if (!boost::filesystem::exists(replayFolderName))
	{
		boost::filesystem::create_directory(replayFolderName);
	}
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

	for (int i = 0; i < 512; ++i)
	{
		levelScores[i] = saveFile->levelScores[i];
	}
	upgradeField.Set(saveFile->upgradeField);
	upgradesTurnedOnField.Set(saveFile->upgradesTurnedOnField);
	logField.Set(saveFile->logField);

	defaultSkinIndex = saveFile->defaultSkinIndex;

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
	levelsBeatenField.Reset();

	mostRecentWorldSelected = 0;
	
	for (int i = 0; i < 512; ++i)
	{
		levelScores[i].Reset();
	}

	upgradeField.Reset();
	upgradesTurnedOnField.Reset();
	logField.Reset();

	defaultSkinIndex = 0;
}

string GlobalSaveFile::fileName = "Resources/Adventure/SaveData/globalsave" + string(GLOBAL_SAVE_EXT);

GlobalSaveFile::GlobalSaveFile()
	:skinField(64)
{
	SetToDefaults();
}

bool GlobalSaveFile::Load()
{
	ifstream is;

	is.open(fileName);

	if (is.is_open())
	{
		is >> ver;

		skinField.Load(is);

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

		skinField.Save(of);

		of.close();
	}
	else
	{
		cout << "error saving global savefile: " << fileName << endl;
		assert(false);
	}
}

void GlobalSaveFile::UnlockSkin(int skinIndex)
{
	if (!skinField.GetBit(skinIndex))
	{
		skinField.SetBit(skinIndex, true);
		Save();
	}
}

bool GlobalSaveFile::IsSkinUnlocked(int skinIndex)
{
	return skinField.GetBit(skinIndex);
}

void GlobalSaveFile::SetToDefaults()
{
	SetVer(1);
	skinField.Reset();
	skinField.SetBit(0, true);
}

RemoteStorageManager::RemoteStorageManager()
{
	m_pSteamRemoteStorage = SteamRemoteStorage();

	destPath = "Resources\\hello\\";

	GetFileStats();
}

void RemoteStorageManager::GetFileStats()
{
	m_ulBytesQuota = 0;
	m_ulAvailableBytes = 0;
	m_nNumFilesInCloud = m_pSteamRemoteStorage->GetFileCount();
	m_pSteamRemoteStorage->GetQuota(&m_ulBytesQuota, &m_ulAvailableBytes);
}

void RemoteStorageManager::Test()
{
	Upload("globalsave.kingsave");
}

void RemoteStorageManager::LoadAll()
{
	if (boost::filesystem::exists(destPath))
	{
		boost::filesystem::remove_all(destPath);
	}
	boost::filesystem::create_directory(destPath);

	DownloadAndSave("globalsave" + string(GLOBAL_SAVE_EXT));

	/*string saveFileName = "blue";

	DownloadAndSave(saveFileName + string(SAVE_EXT));

	boost::filesystem::path saveFileReplayPath = path + saveFileName;

	if (boost::filesystem::exists(saveFileReplayPath))
	{
		boost::filesystem::remove_all(saveFileReplayPath);
	}
	boost::filesystem::create_directory(saveFileReplayPath);*/




	
	//	return;
}

bool RemoteStorageManager::DownloadAndSave(const std::string &file)
{
	if (!m_pSteamRemoteStorage->FileExists(file.c_str()))
		return false;

	int32 fileSize = m_pSteamRemoteStorage->GetFileSize(file.c_str());

	char *bytes = new char[fileSize];
	int32 bytesRead = m_pSteamRemoteStorage->FileRead(file.c_str(), bytes, sizeof(char) * fileSize - 1);

	string fullPath = destPath + file;

	FILE *filePtr = fopen(fullPath.c_str(), "wb");
	fwrite(bytes, 1, bytesRead, filePtr);
	fclose(filePtr);

	delete[] bytes;

	if (bytesRead > 0 )
	{
		cout << "successfully downloaded: " << file << endl;
		return true;
	}
	else
	{
		cout << "failed to download " << file << endl;
		return false;
	}
}

bool RemoteStorageManager::Upload(const std::string &file)
{
	std::ifstream is;
	is.open(file);

	assert(is.is_open());
	std::string content((std::istreambuf_iterator<char>(is)),
		(std::istreambuf_iterator<char>()));
	is.close();

	bool bRet = m_pSteamRemoteStorage->FileWrite(file.c_str(), content.c_str(), content.size());

	GetFileStats();

	if (bRet)
	{
		cout << "successfully uploaded: " << file << endl;
		return true;
	}
	else
	{
		cout << "failed to upload " << file << endl;
		return false;
	}
}

bool RemoteStorageManager::Upload(SaveFile *saveFile)
{
	return Upload(saveFile->fileName);
}