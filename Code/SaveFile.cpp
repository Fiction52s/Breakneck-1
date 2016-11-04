#include "SaveFile.h"
#include <sstream>
#include <assert.h>
#include <iostream>

using namespace std;

Level::Level()
	:completed( false )
{
	for( int i = 0; i < 3; ++i )
	{
		shards[i] = false;
	}
}

SaveFile::SaveFile( const std::string &name )
	:iconVA( sf::Quads, 4 )
{
	worlds[0] = new Level[6];
	for( int i = 1; i < 6; ++i )
	{
		worlds[i] = NULL;
	}

	stringstream ss;
	ss << "Data/" << name << ".kin";

	fileName = ss.str();
}

SaveFile::~SaveFile()
{
	for( int i = 0; i < 6; ++i )
	{
		if( worlds[i] != NULL )
		{
			delete [] worlds[i];
		}
	}
}

void SaveFile::LoadFromFile()
{
	ifstream is;
	
	
	is.open( fileName );

	if( is.is_open() )
	{
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