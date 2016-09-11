#include "Tileset.h"	
#include <iostream>
#include <assert.h>

using namespace std;
using namespace sf;

Tileset::~Tileset()
{
	delete texture;
}

IntRect Tileset::GetSubRect( int localID )
{
	int xi,yi;
	xi = localID % (texture->getSize().x / tileWidth );
	yi = localID / (texture->getSize().x / tileWidth );
		

	return IntRect( xi * tileWidth, yi * tileHeight, tileWidth, tileHeight ); 
}

Tileset * TilesetManager::GetTileset( const std::string & s, int tileWidth, int tileHeight )
{
	//cout << "checking for string: " << s << endl;
	for( list<Tileset*>::iterator it = tilesetList.begin(); it != tilesetList.end(); ++it )
	{
		if( (*it)->sourceName == s )
		{
			return (*it);
		}
	}


	//not found


	Tileset *t = new Tileset();
	t->texture = new Texture();
	if( !t->texture->loadFromFile( s ) )
	{
		cout << "failed to load: " << s << endl;
		assert( false );
	}
	t->tileWidth = tileWidth;
	t->tileHeight = tileHeight;
	t->sourceName = s;
	tilesetList.push_back( t );
	

	return t;
}

void TilesetManager::ClearTilesets()
{
	for( list<Tileset*>::iterator it = tilesetList.begin(); it != tilesetList.end(); ++it )
	{
		if( (*it) == NULL )
		{
			cout <<"WHAT FAIL" << endl;
			//assert( false );
			continue;

		}

		cout << "About to delete: " << (*it)->sourceName << ", "
			<< (*it)->tileWidth << ", " << (*it)->tileWidth << endl;
		delete (*it);
	}
}

TilesetManager::~TilesetManager()
{
	ClearTilesets();
}