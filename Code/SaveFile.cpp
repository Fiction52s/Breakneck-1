#include "SaveFile.h"
#include <sstream>
#include <assert.h>
#include <iostream>

using namespace std;

BitField::BitField(int p_numOptions)
{
	numOptions = p_numOptions;
	numFields = numOptions / 32;
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
	int test = (1 << trueIndex);
	if (optionField[fieldIndex] & test)
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
	:shardField( 32 * 4 )
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
	}
	else
	{
		cout << "error loading save file: " << fileName << endl;
		assert( false );
	}

	is.close();
}

void SaveFile::Save()
{
	ofstream of;

	of.open( fileName );

	if( of.is_open() )
	{
		of << numWorlds << endl;
		//save worlds, then save shards
		for (int i = 0; i < numWorlds; ++i)
		{
			worlds[i].Save( of );
		}

		shardField.Save(of);
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
}

bool Sector::Load(std::ifstream &is)
{
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

	//levels[4].completed = true;
	//levels[2].completed = true;
	
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
	of << numLevels << endl;
	for (int i = 0; i < numLevels; ++i)
	{
		levels[i].Save(of);
	}
}

Level::Level( )
	:optionField(32)
{
	SetComplete(false);
	optionField.Reset();
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
	//SetComplete(true);
	//optionField = 0;
	//name = "testgates33";
	return true;
}

void Level::Save(std::ofstream &of)
{
	of << name << endl;
	optionField.Save(of);
}

void Level::SetComplete(bool comp)
{
	optionField.SetBit(0, comp );
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
	for (int i = 0; i < numSectors; ++i)
	{
		sectors[i].Save( of );
	}
	return true;
}