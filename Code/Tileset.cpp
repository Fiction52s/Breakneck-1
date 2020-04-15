#include "Tileset.h"	
#include <iostream>
#include <assert.h>
//#include "Kin"
//#include "globals.h"

using namespace std;
using namespace sf;

Tileset::~Tileset()
{
	delete texture;
}

IntRect Tileset::GetSubRect( int localID )
{
	int xi,yi;
	Vector2i size(texture->getSize());
	int sx = size.x / tileWidth;
	int sy = size.y / tileHeight;

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
	sf::Color tempColor;
	for( int x = 0; x < imW; ++x )
	{
		for( int y = 0; y < imH; ++y )
		{
			for( int i = 0; i < numAltColors; ++i )
			{
				tempColor = im.getPixel(x, y);
				if( startColorBuf[i].r == tempColor.r &&
					startColorBuf[i].g == tempColor.g &&
					startColorBuf[i].b == tempColor.b )
				{
					tempColor.r = endColorBuf[i].r;
					tempColor.g = endColorBuf[i].g;
					tempColor.b = endColorBuf[i].b;

					if (tempColor.a != 255)
					{
						int xxxx = 6;
					}
					assert( x != 0 || y != 0 );
					im.setPixel( x, y, tempColor );
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
	
	string s2 = string("Resources/") + s;

	if( !im.loadFromFile( s2 ) )
	{
		cout << "failed to load IM: " << s << endl;
		//assert( false );

		return NULL;
	}

	t->altColorIndex = altColorIndex;
	t->texture = CreateAltColorTex( im, numAltColors, startColorBuf, endColorBuf );

	t->tileWidth = tileWidth;
	t->tileHeight = tileHeight;
	t->sourceName = s;
	tilesetList.push_back( t );

	return t;
}


#include <boost/filesystem.hpp>
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

	string s2 = string("Resources/") + s;
	//not found

	//if (!boost::filesystem::exists(s2))
	//{
	//	//take this out later. its not always an error to get this
	//	cout << "NO FILE EXISTS: " << s << endl; 
	//	return NULL;
	//}

	Texture *tex = new Texture();
	if( !tex->loadFromFile( s2 ) )
	{
		delete tex;
		cout << "failed to load: " << s << endl;
		//assert( false );

		return NULL;
	}

	cout << "created texture for: " << s2 << endl;

	Tileset *t = new Tileset();
	t->texture = tex;

	if (tileWidth == 0 )
	{
		tileWidth = tex->getSize().x;
		
	}
	if (tileHeight == 0)
	{
		tileHeight = tex->getSize().y;
	}
	

	t->altColorIndex = altColorIndex;
	t->tileWidth = tileWidth;
	t->tileHeight = tileHeight;
	t->sourceName = s;
	tilesetList.push_back( t );
	

	return t;
}

Tileset * TilesetManager::GetTileset(const std::string & s, int tileWidth, int tileHeight, Skin *skin)
{
	if (skin != NULL)
	{
		return GetTileset(s, tileWidth, tileHeight, skin->index, skin->numChanges, skin->startColors, skin->endColors);
	}
	else
	{
		return GetTileset(s, tileWidth, tileHeight);
	}
}


Tileset *TilesetManager::GetUpdatedTileset(
	const std::string & s, int tileWidth, int tileHeight, int altColorIndex)
{
	for (list<Tileset*>::iterator it = tilesetList.begin(); it != tilesetList.end(); ++it)
	{
		if ((*it)->sourceName == s && (*it)->altColorIndex == altColorIndex)
		{
			DestroyTileset((*it));
			break;
		}
	}

	string s2 = string("Resources/") + s;

	Texture *tex = new Texture();
	if (!tex->loadFromFile(s2))
	{
		delete tex;
		return NULL;
	}

	Tileset *t = new Tileset();
	t->texture = tex;

	if (tileWidth == 0)
	{
		tileWidth = tex->getSize().x;

	}
	if (tileHeight == 0)
	{
		tileHeight = tex->getSize().y;
	}

	t->altColorIndex = altColorIndex;
	t->tileWidth = tileWidth;
	t->tileHeight = tileHeight;
	t->sourceName = s;
	tilesetList.push_back(t);

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

void TilesetManager::DestroyTileset(Tileset * t)
{
	for (auto it = tilesetList.begin(); it != tilesetList.end(); ++it)
	{
		if (t == (*it))
		{
			tilesetList.remove((*it));
			break;
		}
	}

	delete t;
}

TilesetManager::~TilesetManager()
{
	ClearTilesets();
}