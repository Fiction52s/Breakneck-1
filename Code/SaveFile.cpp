#include "SaveFile.h"
#include <sstream>
#include <assert.h>
#include <iostream>

using namespace std;

SaveFile::SaveFile( const std::string &name )
	:iconVA( sf::Quads, 4 )
{
	stringstream ss;
	ss << "Data/" << name << ".kin";

	stringstream ss1;
	ss1 << "Data/" << "default" << name << ".kin";

	fileName = ss.str();
	defaultFileName = ss1.str();
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