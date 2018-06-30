#include "SaveFile.h"
#include <sstream>
#include <assert.h>
#include <iostream>
#include "MainMenu.h"
#include "Enemy_Shard.h"

using namespace std;
using namespace boost::filesystem;

BitField::BitField(int p_numOptions)
{
	numOptions = p_numOptions;
	numFields = numOptions / 32;
	if (numFields == 0)
	{
		int x = 5;
	}
	numFields = max(numFields, 1);
	optionField = new sf::Uint32[numFields];
	Reset();
}

BitField::~BitField()
{
	delete[] optionField;
}

void BitField::SetBit(int index, bool val)
{
	int trueIndex = index % 32;
	int fieldIndex = index / 32;
	sf::Uint32 check = (1 << trueIndex);
	if (val)
	{
		optionField[fieldIndex] |= check;
	}
	else
	{
		optionField[fieldIndex] &= ~check;
	}
}

bool BitField::GetBit(int index)
{
	int trueIndex = index % 32;
	int fieldIndex = index / 32;
	sf::Uint32 test = (1 << trueIndex);
	sf::Uint32 field = optionField[fieldIndex];
	if (field & test)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void BitField::Reset()
{
	for (int i = 0; i < numFields; ++i)
	{
		optionField[i] = 0;
	}
}

bool BitField::Load(std::ifstream &is)
{
	//Reset();
	for (int i = 0; i < numFields; ++i)
	{
		is >> optionField[i];
	}
	//change this to binary later
	//add error checking later

	return true;
}

void BitField::Save(std::ofstream &of)
{
	for (int i = 0; i < numFields; ++i)
	{
		of << optionField[i] << endl;
	}
}

//bool Shard::Load(std::ifstream is)
//{
//	is >> animName;
//	int gType;
//	is >> gType;
//	is >> groupIndex;
//
//}

SaveFile::SaveFile( const std::string &name )
	:shardField( 32 * 5 ), newShardField( 32 * 5 )
{
	stringstream ss;
	ss << "Data/" << name << ".kin";

	fileName = ss.str();
}

float SaveFile::GetCompletionPercentage()
{
	float totalComplete = 0;
	float totalPossible = 0;

	for (int i = 0; i < numWorlds; ++i)
	{
		totalComplete += worlds[i].GetCompletionPercentage();
		totalPossible += 100.f;
	}

	return (totalComplete / totalPossible) * 100.f;
}

SaveFile::~SaveFile()
{
	/*for( int i = 0; i < 6; ++i )
	{
		if( worlds[i] != NULL )
		{
			delete [] worlds[i];
		}
	}*/
}

bool SaveFile::ShardIsCaptured(ShardType sType)
{
	return shardField.GetBit(sType);
}

void SaveFile::CopyFromDefault()
{
	path from("Data/default.kin");

	path to(fileName);

	boost::filesystem::copy_file(from, to, copy_option::fail_if_exists);
}

bool SaveFile::LoadInfo(ifstream &is)
{
	if (is.is_open())
	{
		is >> numWorlds;
		worlds = new World[numWorlds];
		for (int i = 0; i < numWorlds; ++i)
		{
			worlds[i].sf = this;
			worlds[i].index = i;
			worlds[i].Load(is);
		}

		shardField.Load(is);
		newShardField.Load(is);

		UpdateShardNameList();

		is.close();
		return true;
	}
	else
	{
		/*if (fileName == "Data/blue.kin")
		{
			int xxx = 54;
		}*/
		return false;
	}
}

void SaveFile::UpdateShardNameList()
{
	std::map<std::string, int> shardNameMap;
	for (int i = 0; i < numWorlds; ++i)
	{
		for (auto it = worlds[i].shardNameList.begin(); it != worlds[i].shardNameList.end(); ++it)
		{
			if (shardNameMap.count((*it)) == 0)
			{
				shardNameMap[(*it)] = 0;
			}
			else
			{
				shardNameMap[(*it)]++;
			}
		}
	}

	for (auto it = shardNameMap.begin(); it != shardNameMap.end(); ++it)
	{
		shardNameList.push_back((*it).first);
	}
}

int SaveFile::GetNumShardsCaptured()
{
	int capCount = 0;
	for (auto it = shardNameList.begin(); it != shardNameList.end(); ++it)
	{
		ShardType st = Shard::GetShardType((*it));
		if (ShardIsCaptured(st))
		{
			++capCount;
		}
	}
	return capCount;
}

int SaveFile::GetNumTotalShards()
{
	return shardNameList.size();
}

//return true on normal loading, and false if you need to make a default
bool SaveFile::Load()
{
	ifstream is;
	
	is.open( fileName );

	if( !LoadInfo(is) )
	{
		ifstream defIs;
		defIs.open("Data/default.kin");
		bool test = LoadInfo(defIs);
		assert(test);
		return false;
	}
	else
	{
		return true;
	}
}

//void SaveFile::CreateSaveThread(SaveFile *sf)
//{
//	if( sf->thr )
//	sf->thr = new boost::thread(&(Buf::ThreadedBufferWrite), &testBuf, &of);
//}

void SaveFile::Save()
{
	ofstream of;

	of.open( fileName );

	cout << "saving save file" << endl;

	if( of.is_open() )
	{
		of << numWorlds << endl;
		cout << "numworlds: " << numWorlds << endl;
		//save worlds, then save shards
		for (int i = 0; i < numWorlds; ++i)
		{
			worlds[i].Save( of );
		}

		shardField.Save(of);
		newShardField.Save(of);

		of.close();
	}
	else
	{
		cout << "error saving file: " << fileName << endl;
		assert( false );
	}
}

//void SaveFile::SetJustUnlocked(int world, int sec, int lev)
//{
//	if (lev == worlds[world].sectors[sec].numLevels - 1)
//	{
//		return;
//	}
//	else
//	{
//		worlds[world].sectors[sec].levels[lev + 1].justUnlocked = true;
//	}
//	/*Level &level = worlds[world].sectors[sec].levels[lev];
//	World &w = worlds[world];
//
//	for (int i = 0; i < w.numSectors; ++i)
//	{
//		Sector &sector = w.sectors[sec];
//		if (sector.IsUnlocked())
//		{
//
//		}
//	}*/
//}

World::World()
{
	sectors = NULL;
	numSectors = 0;
}

World::~World()
{
	if (sectors != NULL)
	{
		delete[] sectors;
	}
}

float World::GetCompletionPercentage()
{
	float totalComplete = 0;
	float totalPossible = 0;

	for (int i = 0; i < numSectors; ++i)
	{
		totalComplete += sectors[i].GetCompletionPercentage();
		totalPossible += 100.f;
	}

	return (totalComplete / totalPossible) * 100.f;
}

Sector::Sector()
{
	numLevels = 0;
	levels = NULL;
	conditions = NULL;
	numTypesNeeded = NULL;
	sectorType = 100;
	numUnlockConditions = 99;
}

bool Sector::IsLevelUnlocked(int index)
{
	for (int i = 0; i < index; ++i )
	{
		if (!levels[i].GetComplete())
		{
			return false;
		}
	}
	return true;
}

void Sector::UpdateShardNameList()
{
	std::map<std::string, int> shardNameMap;
	for (int i = 0; i < numLevels; ++i)
	{
		for (auto it = levels[i].shardNameList.begin(); it != levels[i].shardNameList.end(); ++it)
		{
			if (shardNameMap.count((*it)) == 0)
			{
				shardNameMap[(*it)] = 0;
			}
			else
			{
				shardNameMap[(*it)]++;
			}
		}
	}

	for (auto it = shardNameMap.begin(); it != shardNameMap.end(); ++it)
	{
		shardNameList.push_back((*it).first);
	}
}

int Sector::GetNumTotalShards()
{
	return shardNameList.size();
}

int Sector::GetNumShardsCaptured()
{
	int capCount = 0;
	for (auto it = shardNameList.begin(); it != shardNameList.end(); ++it)
	{
		ShardType st = Shard::GetShardType((*it));
		if (world->sf->ShardIsCaptured(st))
		{
			++capCount;
		}
	}
	return capCount;
}

float Sector::GetCompletionPercentage()
{
	float completePercent = 0;
	float possiblePercent = 0;
	for (int i = 0; i < numLevels; ++i)
	{
		completePercent += levels[i].GetCompletionPercentage();
		possiblePercent += 100;
	}

	return (completePercent / possiblePercent) * 100.f;
}

bool Sector::HasTopBonus(int index)
{
	return (topBonuses.count(index) > 0);
}
bool Sector::hasBottomBonus(int index)
{
	return (bottomBonuses.count(index) > 0);
}

bool Sector::IsComplete()
{
	for (int i = 0; i < numLevels; ++i)
	{
		if (!levels[i].GetComplete())
		{
			return false;
		}
	}
	return true;
}

bool Sector::Load(std::ifstream &is)
{
	is >> sectorType;
	is >> numUnlockConditions;

	if (numUnlockConditions > 0)
	{
		conditions= new int[numUnlockConditions];
		numTypesNeeded = new int[numUnlockConditions];

		int conditionType = -1;
		int numOfTypeNeeded = -1;
		for (int i = 0; i < numUnlockConditions; ++i)
		{
			is >> conditionType;
			is >> numOfTypeNeeded;

			conditions[i] = conditionType;
			numTypesNeeded[i] = numOfTypeNeeded;
		}

	}

	is >> numLevels;
	levels = new Level[numLevels];
	
	bool res = true;
	for (int i = 0; i < numLevels; ++i)
	{
		levels[i].sec = this;
		levels[i].index = i;
		res = levels[i].Load(is);

		assert(res);
	}

	int numTopBonuses = 0;
	int numBottomBonuses = 0;

	
	is >> numTopBonuses;
	for (int j = 0; j < numTopBonuses; ++j) //0 - 2
	{
		int bonusIndex;
		is >> bonusIndex;

		Level &lev = topBonuses[bonusIndex];
		lev.sec = this;
		lev.index = bonusIndex;
		res = lev.Load(is);

		assert(res);
	}

	is >> numBottomBonuses;
	for (int j = 0; j < numBottomBonuses; ++j) //0 - 2
	{
		int bonusIndex;
		is >> bonusIndex;

		Level &lev = bottomBonuses[bonusIndex];
		lev.sec = this;
		lev.index = bonusIndex;
		res = lev.Load(is);

		assert(res);
	}
	
	assert(is.good());

	UpdateShardNameList();

	return true;
}

Sector::~Sector()
{
	if (levels != NULL)
	{
		delete[] levels;
	}
}


bool Sector::IsConditionFulfilled(int index)
{
	int numTypeComplete = world->GetNumSectorTypeComplete(conditions[index]);
	return (numTypeComplete >= numTypesNeeded[index]);
}

bool Sector::IsUnlocked()
{
	int numTypeComplete = -1;
	for (int i = 0; i < numUnlockConditions; ++i)
	{
		if (!IsConditionFulfilled(i))
		{
			return false;
		}
	}

	return true;
}

void Sector::Save(std::ofstream &of)
{
	of << sectorType << endl;
	of << numUnlockConditions << endl;
	if (numUnlockConditions > 0)
	{
		for (int i = 0; i < numUnlockConditions; ++i)
		{
			of << conditions[i] << endl;
			of << numTypesNeeded[i] << endl;
		}

	}

	of << numLevels << endl;
	for (int i = 0; i < numLevels; ++i)
	{
		levels[i].Save(of);
	}

	int numTopBonuses = topBonuses.size();
	int numBottomBonuses = bottomBonuses.size();
	of << numTopBonuses << endl;	
	for (auto it = topBonuses.begin(); it != topBonuses.end(); ++it)
	{
		of << (*it).first << endl;
		(*it).second.Save(of);
	}

	of << numBottomBonuses << endl;
	for (auto it = bottomBonuses.begin(); it != bottomBonuses.end(); ++it)
	{
		of << (*it).first << endl;
		(*it).second.Save(of);
	}
	
}

Level::Level( )
	:optionField(32)
{
	bossFightType = 0;
	SetComplete(false);
	optionField.Reset();
	shardsLoaded = false;
	justBeaten = false;
}

void Level::UpdateFromMapHeader()
{
	if (shardsLoaded) return;

	shardsLoaded = true;
	ifstream is;
	string file = "Maps/" + name + ".brknk";
	is.open(file);
	if (is.is_open())
	{
		int capCount = 0;
		float percent = 0;

		MapHeader *mh = MapSelectionMenu::ReadMapHeader(is);

		shardNameList.clear();
		for (auto it = mh->shardNameList.begin(); it != mh->shardNameList.end(); ++it)
		{
			shardNameList.push_back((*it));
		}

		bossFightType = mh->bossFightType;

		delete mh;
	}
	else
	{
		assert(0);
	}
}

int Level::GetNumShardsCaptured()
{
	int capCount = 0;
	for (auto it = shardNameList.begin(); it != shardNameList.end(); ++it)
	{
		ShardType st = Shard::GetShardType((*it));
		if (sec->world->sf->ShardIsCaptured(st))
		{
			++capCount;
		}
	}
	return capCount;
}

float Level::GetCapturedShardsPortion()
{
	int capCount = GetNumShardsCaptured();

	if (capCount == 0)
		return 0;

	return (float)capCount / shardNameList.size();
}

int Level::GetNumTotalShards()
{
	return shardNameList.size();
}

float Level::GetCompletionPercentage()
{
	float portion = 0;
	if (GetComplete())
	{
		if (shardNameList.size() == 0)
		{
			portion = 1.0;
		}
		else
		{
			portion += .5f;
			portion += GetCapturedShardsPortion() * .5f;
		}
	}
	else
	{
		if (shardNameList.size() == 0)
		{
			portion = 0.0;
		}
		else
		{
			portion += GetCapturedShardsPortion() * .5f;
		}
	}

	

	return portion * 100.f;
	//MapSelectionMenu::ReadMapHeader()
}

bool Level::TopBonusUnlocked()
{
	return optionField.GetBit(UNLOCKEDTOPBONUS);
}

bool Level::BottomBonusUnlocked()
{
	return optionField.GetBit(UNLOCKEDBOTTOMBONUS);
}

void Level::UnlockBottomBonus()
{
	optionField.SetBit(UNLOCKEDBOTTOMBONUS, true);
}

void Level::UnlockTopBonus()
{
	optionField.SetBit(UNLOCKEDTOPBONUS, true);
}

void Level::Reset()
{
	SetComplete(false);
	optionField = 0;
	shardsLoaded = false;
	justBeaten = false;
}

bool Level::Load(std::ifstream &is)
{
	is >> name;
	optionField.Load(is);

	UpdateFromMapHeader();

	return true;
}

void Level::Save(std::ofstream &of)
{
	of << name << endl;
	optionField.Save(of);

	//cout << "save level: " << name << endl;
}

void Level::SetComplete(bool comp)
{
	optionField.SetBit(COMPLETE, comp );
}

//void Level::SetJustUnlocked(bool unlocked)
//{
//	optionField.SetBit(JUSTUNLOCKED, unlocked);
//}
//
//void Level::SetJustUnlockedTop(bool unlocked)
//{
//	optionField.SetBit(JUSTUNLOCKEDTOP, unlocked);
//}
//
//void Level::SetJustUnlockedBottom(bool unlocked)
//{
//	optionField.SetBit(JUSTUNLOCKEDBOTTOM, unlocked);
//}

bool Level::GetComplete()
{
	return optionField.GetBit(COMPLETE);
}

//bool Level::GetJustUnlocked()
//{
//	return optionField.GetBit(JUSTUNLOCKED);
//}
//bool Level::GetJustUnlockedTop()
//{
//	return optionField.GetBit(JUSTUNLOCKEDTOP);
//}
//bool Level::GetJustUnlockedBottom()
//{
//	return optionField.GetBit(JUSTUNLOCKEDBOTTOM);
//}

bool Level::IsOneHundredPercent()
{
	return(GetCompletionPercentage() == 100.f);
}

std::string Level::GetFullName()
{
	return "maps/" + name + ".brknk";
}

bool World::Load(std::ifstream &is)
{
	is >> numSectors;
	assert(numSectors > 0);
	sectors = new Sector[numSectors];

	for (int i = 0; i < numSectors; ++i)
	{
		sectors[i].world = this;
		sectors[i].index = i;
		sectors[i].Load(is);
	}

	UpdateShardNameList();
	return true;
}

void World::UpdateShardNameList()
{
	std::map<std::string, int> shardNameMap;
	for (int i = 0; i < numSectors; ++i)
	{
		for (auto it = sectors[i].shardNameList.begin(); it != sectors[i].shardNameList.end(); ++it)
		{
			if (shardNameMap.count((*it)) == 0)
			{
				shardNameMap[(*it)] = 0;
			}
			else
			{
				shardNameMap[(*it)]++;
			}
		}
	}

	for (auto it = shardNameMap.begin(); it != shardNameMap.end(); ++it)
	{
		shardNameList.push_back((*it).first);
	}
}

int World::GetNumTotalShards()
{
	return shardNameList.size();
}

int World::GetNumShardsCaptured()
{
	int capCount = 0;
	for (auto it = shardNameList.begin(); it != shardNameList.end(); ++it)
	{
		ShardType st = Shard::GetShardType((*it));
		if (sf->ShardIsCaptured(st))
		{
			++capCount;
		}
	}
	return capCount;
}

int World::GetNumSectorTypeComplete(int sType)
{
	int count = 0;
	for (int i = 0; i < numSectors; ++i)
	{
		if (sectors[i].sectorType == sType)
		{
			if (sectors[i].IsComplete())
			{
				++count;
			}
		}
	}

	return count;
}

bool World::Save(std::ofstream &of)
{
	of << numSectors << endl;
	cout << "numSectors: " << numSectors << endl;
	for (int i = 0; i < numSectors; ++i)
	{
		sectors[i].Save( of );
	}
	return true;
}