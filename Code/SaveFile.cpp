#include "SaveFile.h"
#include <sstream>
#include <assert.h>
#include <iostream>

using namespace std;

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
	:shardField( 32 * 4 ), newShardField( 32 * 4 )
{
	stringstream ss;
	ss << "Data/" << name << ".kin";

	fileName = ss.str();
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

void SaveFile::Load()
{
	ifstream is;
	
	is.open( fileName );

	if( is.is_open() )
	{
		is >> numWorlds;
		worlds = new World[numWorlds];
		for (int i = 0; i < numWorlds; ++i)
		{
			worlds[i].index = i;
			worlds[i].Load(is);
		}

		shardField.Load(is);
		newShardField.Load(is);

		is.close();
	}
	else
	{
		cout << "error loading save file: " << fileName << endl;
		assert( false );
	}

	
}

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

Sector::Sector()
{
	numLevels = 0;
	levels = NULL;
	conditions = NULL;
	numTypesNeeded = NULL;
	sectorType = 100;
	numUnlockConditions = 99;
	
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

	return true;
}

Sector::~Sector()
{
	if (levels != NULL)
	{
		delete[] levels;
	}
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
	SetComplete(false);
	optionField.Reset();
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
}

bool Level::Load(std::ifstream &is)
{
	is >> name;
	optionField.Load(is);
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

bool Level::GetComplete()
{
	return optionField.GetBit(0);
}

std::string Level::GetFullName()
{
	return "maps/" + name + ".brknk";
}

bool World::Load(std::ifstream &is)
{
	is >> numSectors;
	sectors = new Sector[numSectors];

	for (int i = 0; i < numSectors; ++i)
	{
		sectors[i].world = this;
		sectors[i].index = i;
		sectors[i].Load(is);
	}
	return true;
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