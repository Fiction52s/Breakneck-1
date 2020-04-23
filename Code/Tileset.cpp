#include "Tileset.h"	
#include <iostream>
#include <assert.h>
//#include "Kin"
//#include "globals.h"

using namespace std;
using namespace sf;


void Tileset::SetSpriteTexture(sf::Sprite &spr)
{
	spr.setTexture(*texture);
}

void Tileset::SetSubRect(sf::Sprite &spr, int index, bool flipX, bool flipY )
{
	IntRect sub = GetSubRect(index);
	if (flipX)
	{
		sub.left += sub.width;
		sub.width = -sub.width;
	}
	if (flipY)
	{
		sub.top += sub.height;
		sub.height = -sub.height;
	}

		
	spr.setTextureRect(sub);
}

void Tileset::SetQuadSubRect(sf::Vertex *v, int index, bool flipX, bool flipY)
{
	IntRect sub = GetSubRect(index);
	if (flipX)
	{
		sub.left += sub.width;
		sub.width = -sub.width;
	}
	if (flipY)
	{
		sub.top += sub.height;
		sub.height = -sub.height;
	}

	v[0].texCoords = Vector2f(sub.left, sub.top);
	v[1].texCoords = Vector2f(sub.left + sub.width, sub.top);
	v[2].texCoords = Vector2f(sub.left + sub.width, sub.top + sub.height);
	v[3].texCoords = Vector2f(sub.left, sub.top + sub.height);
}

Tileset::~Tileset()
{
	//ideally make this not a pointer soon as possible to avoid allocation
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

Tileset *TilesetManager::Find(const std::string &s, int altColorIndex)
{
	Tileset *currTS;
	auto it = tilesetMap.find(s);
	if (it != tilesetMap.end())
	{
		std::list<Tileset*> &currList = (*it).second;
		for (auto listIt = currList.begin(); listIt != currList.end(); ++listIt)
		{
			if ((*listIt)->altColorIndex == altColorIndex)
			{
				return (*listIt);
			}
		}
	}

	return NULL;
}

void TilesetManager::Add(const std::string &s, Tileset *ts)
{
	tilesetMap[s].push_back(ts);
}

Tileset *TilesetManager::Create(const std::string &s, int tileWidth, int tileHeight,
	int altColorIndex )
{
	string s2 = string("Resources/") + s;
	//not found

	//if (!boost::filesystem::exists(s2))
	//{
	//	//take this out later. its not always an error to get this
	//	cout << "NO FILE EXISTS: " << s << endl; 
	//	return NULL;
	//}

	Texture *tex = new Texture();
	if (!tex->loadFromFile(s2))
	{
		delete tex;
		cout << "failed to load: " << s << endl;
		//assert( false );

		return NULL;
	}

	cout << "created texture for: " << s2 << endl;

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

	Add(s, t);

	return t;
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


	Tileset *alreadyExistsTS = Find(s, altColorIndex);
	if (alreadyExistsTS != NULL)
		return alreadyExistsTS;
	//if (alreadyExistsTS != NULL)
	//	return alreadyExistsTS;
	/*if ( alreadyExistsTS != NULL && alreadyExistsTS->sourceName == s && alreadyExistsTS->altColorIndex == altColorIndex)
	{
		sf::Image img = alreadyExistsTS->texture->copyToImage();
		Texture *tNew = CreateAltColorTex(img, numAltColors, startColorBuf,
			endColorBuf);

		delete alreadyExistsTS->texture;
		alreadyExistsTS->texture = tNew;

		return alreadyExistsTS;
	}*/

	//not already here. need to create it

	
	//t->texture = new Texture();

	
	
	string s2 = string("Resources/") + s;

	sf::Image im;
	if( !im.loadFromFile( s2 ) )
	{
		cout << "failed to load IM: " << s << endl;
		//assert( false );

		return NULL;
	}

	Tileset *t = new Tileset();

	t->altColorIndex = altColorIndex;
	t->texture = CreateAltColorTex( im, numAltColors, startColorBuf, endColorBuf );

	t->tileWidth = tileWidth;
	t->tileHeight = tileHeight;
	t->sourceName = s;

	Add(s, t);
	//tilesetList.push_back( t );

	return t;
}


#include <boost/filesystem.hpp>

Tileset * TilesetManager::GetSizedTileset(const std::string & s, int altColorIndex )
{
	Tileset *alreadyExistsTS = Find(s, altColorIndex);
	if (alreadyExistsTS != NULL)
		return alreadyExistsTS;

	std::size_t finalUnderScore = s.find_last_of("_");
	std::size_t finalX = s.find_last_of("x");
	std::size_t finalDot = s.find('.');
	int tileWidth = std::stoi(s.substr(finalUnderScore + 1, finalX - finalUnderScore - 1));
	int tileHeight = std::stoi(s.substr(finalX + 1, finalDot - finalX - 1));

	return Create(s, tileWidth, tileHeight, altColorIndex);
}

Tileset * TilesetManager::GetTileset( const std::string & s, int tileWidth, int tileHeight, int altColorIndex )
{
	Tileset *alreadyExistsTS = Find(s, altColorIndex);
	if (alreadyExistsTS != NULL)
		return alreadyExistsTS;

	return Create(s, tileWidth, tileHeight, altColorIndex);
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
	Tileset *alreadyExistsTS = Find(s, altColorIndex);
	if (alreadyExistsTS != NULL)
	{
		DestroyTileset(alreadyExistsTS);
	}

	return Create(s, tileWidth, tileHeight, altColorIndex);
}

void TilesetManager::ClearTilesets()
{
	list<Tileset*>::iterator listIt;
	list<Tileset*>::iterator listItEnd;
	for (auto it = tilesetMap.begin(); it != tilesetMap.end(); ++it)
	{
		listIt = (*it).second.begin();
		listItEnd = (*it).second.end();
		for (; listIt != listItEnd; ++listIt)
		{
			delete (*listIt);

			/*if ((*listIt) == NULL)
			{
				cout << "should never have a NULL tileset here" << endl;
				assert( false );
				continue;

			}

			cout << "About to delete: " << (*listIt)->sourceName << ", "
				<< (*listIt)->tileWidth << ", " << (*listIt)->tileWidth << endl;*/
			
		}

	}

	tilesetMap.clear();
}

void TilesetManager::DestroyTileset(Tileset * t)
{
	auto it = tilesetMap.find(t->sourceName);
	if (it != tilesetMap.end())
	{
		list<Tileset*> &currList = (*it).second;
		for (auto lit = currList.begin(); lit != currList.end(); ++lit)
		{
			if ((*lit) == t)
			{
				currList.erase(lit);
				delete t;

				if (currList.empty())
				{
					tilesetMap.erase(it);
				}
				return;
			}
		}
	}
}

TilesetManager::~TilesetManager()
{
	ClearTilesets();
}