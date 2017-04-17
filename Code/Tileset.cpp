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
	int sx = (texture->getSize().x / tileWidth );
	int sy = (texture->getSize().y / tileHeight );

	xi = localID % sx;
	yi = localID / sx;
		
	if( localID < 0 || localID >= sx * sy )
	{
		cout << "texture error: " << this->sourceName << endl;
		cout << "localID: " << localID << ", sx: " << sx << ", sy: " << sy << endl;
		assert( 0 );
	}
	return IntRect( xi * tileWidth, yi * tileHeight, tileWidth, tileHeight ); 
}

int Tileset::GetNumTiles()
{
	int sx = (texture->getSize().x / tileWidth );
	int sy = (texture->getSize().y / tileHeight );

	return sx * sy;
}

Texture *TilesetManager::CreateAltColorTex( sf::Image &im,
		int numAltColors, 
		sf::Color *startColorBuf,
		sf::Color *endColorBuf )
{
	int imW = im.getSize().x;
	int imH = im.getSize().y;
	for( int x = 0; x < imW; ++x )
	{
		for( int y = 0; y < imH; ++y )
		{
			for( int i = 0; i < numAltColors; ++i )
			{
				if( startColorBuf[i] == im.getPixel( x, y ) )
				{
					assert( x != 0 || y != 0 );
					im.setPixel( x, y, endColorBuf[i] );
				}
			}
		}
	}

	Texture *tNew = new Texture;
	tNew->loadFromImage( im );

	return tNew;
}

Tileset * TilesetManager::GetTileset( const std::string & s, int tileWidth, int tileHeight, int altColorIndex, int numAltColors,
	sf::Color *startColorBuf, sf::Color *endColorBuf)
{
	//cout << "checking for string: " << s << endl;
	for( list<Tileset*>::iterator it = tilesetList.begin(); it != tilesetList.end(); ++it )
	{
		if( (*it)->sourceName == s && (*it)->altColorIndex == altColorIndex )
		{

			sf::Image img = (*it)->texture->copyToImage();
			Texture *tNew = CreateAltColorTex( img, numAltColors, startColorBuf, 
				endColorBuf );

			delete (*it)->texture;
			(*it)->texture = tNew;

			return (*it);
		}
	}

	//not already here. need to create it

	Tileset *t = new Tileset();
	//t->texture = new Texture();

	sf::Image im;
	
	if( !im.loadFromFile( s ) )
	{
		cout << "failed to load IM: " << s << endl;
		assert( false );
	}

	t->altColorIndex = altColorIndex;
	t->texture = CreateAltColorTex( im, numAltColors, startColorBuf, endColorBuf );

	t->tileWidth = tileWidth;
	t->tileHeight = tileHeight;
	t->sourceName = s;
	tilesetList.push_back( t );

	return t;
}

Tileset * TilesetManager::GetTileset( const std::string & s, int tileWidth, int tileHeight, int altColorIndex )
{
	//cout << "checking for string: " << s << endl;
	for( list<Tileset*>::iterator it = tilesetList.begin(); it != tilesetList.end(); ++it )
	{
		if( (*it)->sourceName == s && (*it)->altColorIndex == altColorIndex )
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

	t->altColorIndex = altColorIndex;
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