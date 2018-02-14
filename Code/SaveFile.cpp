#include "SaveFile.h"
#include <sstream>
#include <assert.h>
#include <iostream>

using namespace std;

SaveFile::SaveFile( const std::string &name )
{
	for (int i = 0; i < 8; ++i)
	{
		worlds[i].index = i;
	}
	//worlds = new World[8];
	/*for( int i = 1; i < 6; ++i )
	{
		worlds[i] = NULL;
	}*/

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

void SaveFile::LoadFromFile()
{
	ifstream is;
	
	is.open( fileName );

	if( is.is_open() )
	{
		for (int i = 0; i < NUM_WORLDS; ++i)
		{
			worlds[i].Load(is);
		}
	}
	else
	{
		cout << "error loading save file: " << fileName << endl;
		assert( false );
	}

	is.close();
}


void SaveFile::SaveCurrent()
{
	ofstream of;

	of.open( fileName );

	if( of.is_open() )
	{
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
	numLevels = 6;
	levels = new Level[numLevels];

	bool res = true;
	for (int i = 0; i < numLevels; ++i)
	{
		levels[i].sec = this;
		levels[i].index = i;
		res = levels[i].Load(is);
		assert(res);
	}

	levels[4].completed = true;
	levels[2].completed = true;
	
	return true;
}

Sector::~Sector()
{
	if (levels != NULL)
	{
		delete[] levels;
	}
}

Level::Level( )
{
	completed = false;
	optionField = 0;
}

void Level::SetOption(int index, bool value)
{
	unsigned int check = (1 << index);
	if (value)
	{
		optionField |= check;
	}
	else
	{
		optionField &= ~check;
	}
}
void Level::SetCompleted( bool comp )
{
	completed = comp;
}
void Level::Reset()
{
	completed = false;
	optionField = 0;
}

bool Level::Load(std::ifstream &is)
{
	completed = false;
	optionField = 0;
	name = "testgates33";
	return true;
}

std::string Level::GetFullName()
{
	return "maps/" + name + ".brknk";
}

bool World::Load(std::ifstream &is)
{
	numSectors = 7;
	sectors = new Sector[numSectors];

	for (int i = 0; i < numSectors; ++i)
	{
		sectors[i].world = this;
		sectors[i].index = i;
		sectors[i].Load(is);
	}
	return true;
}