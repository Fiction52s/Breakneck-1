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

Tileset *TilesetManager::Find(TilesetCategory cat, const std::string &s, int altColorIndex)
{
	Tileset *currTS;
	auto & currMap = tilesetMaps[cat];
	auto it = currMap.find(s);
	if (it != currMap.end())
	{
		auto &currList = (*it).second;
		for (auto listIt = currList.begin(); listIt != currList.end(); ++listIt)
		{
			if ((*listIt).first->altColorIndex == altColorIndex)
			{
				(*listIt).second++;
				return (*listIt).first;
			}
		}
	}

	return NULL;
}

void TilesetManager::Add(TilesetCategory cat, const std::string &s, Tileset *ts)
{
	tilesetMaps[cat][s].push_back(make_pair(ts,1));
}

Tileset *TilesetManager::Create(TilesetCategory cat, const std::string &s, int tileWidth, int tileHeight,
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

	Add(cat, s, t);

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


TilesetManager::TilesetCategory TilesetManager::GetCategory(const std::string &s)
{
	string folderString = s.substr(0, s.find('/'));
	//cout << "folder: " << folderString << endl;
	if (folderString == "Kin" || folderString == "Sword")
	{
		return C_PLAYER;
	}
	else if (folderString == "Enemies")
	{
		return C_ENEMY;
	}
	else if (folderString == "Momenta")
	{
		return C_MOMENTA;
	}
	else if (folderString == "Goal")
	{
		return C_GOAL;
	}
	else if (folderString == "Shard")
	{
		return C_SHARD;
	}
	else if (folderString == "Borders" || folderString == "Terrain" )
	{
		return C_TERRAIN;
	}
	else if (folderString == "Story")
	{
		return C_STORY;
	}
	else if (folderString == "Zone")
	{
		return C_ZONE;
	}
	else if (folderString == "Backgrounds" || folderString == "Parallax")
	{
		return C_BACKGROUND;
	}
	else if (folderString == "Env")
	{
		return C_ENV;
	}
	else if (folderString == "FX")
	{
		return C_FX;
	}
	else if (folderString == "HUD")
	{
		return C_HUD;
	}
	else
	{
		return C_DEFAULT;
	}
}

Tileset * TilesetManager::GetTileset( const std::string & s, int tileWidth, int tileHeight, int altColorIndex, int numAltColors,
	sf::Color *startColorBuf, sf::Color *endColorBuf)
{
	TilesetCategory cat = GetCategory(s);

	Tileset *alreadyExistsTS = Find(cat, s, altColorIndex);
	if (alreadyExistsTS != NULL)
		return alreadyExistsTS;
	
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

	Add(cat, s, t);

	return t;
}


#include <boost/filesystem.hpp>

Tileset * TilesetManager::GetSizedTileset(const std::string & s, int altColorIndex)
{
	TilesetCategory cat = GetCategory(s);

	Tileset *alreadyExistsTS = Find(cat, s, altColorIndex);
	if (alreadyExistsTS != NULL)
		return alreadyExistsTS;

	std::size_t finalUnderScore = s.find_last_of("_");
	std::size_t finalX = s.find_last_of("x");
	std::size_t finalDot = s.find('.');
	int tileWidth = std::stoi(s.substr(finalUnderScore + 1, finalX - finalUnderScore - 1));
	int tileHeight = std::stoi(s.substr(finalX + 1, finalDot - finalX - 1));

	return Create(cat, s, tileWidth, tileHeight, altColorIndex);
}

Tileset * TilesetManager::GetSizedTileset(const std::string &folder, const std::string & s, int altColorIndex)
{
	string fullString = folder + s;
	return GetSizedTileset(fullString, altColorIndex);
}

Tileset * TilesetManager::GetSizedTileset(const std::string &folder, const std::string & s, Skin *skin)
{
	string fullString = folder + s;

	if (skin != NULL)
	{
		return GetSizedTileset(fullString, skin->index, skin->numChanges, skin->startColors, skin->endColors);
	}
	else
	{
		return GetSizedTileset(fullString);
	}
}

Tileset * TilesetManager::GetSizedTileset(const std::string & s, int altColorIndex, int numColorChanges,
	sf::Color *startColorBuf, sf::Color *endColorBuf)
{
	std::size_t finalUnderScore = s.find_last_of("_");
	std::size_t finalX = s.find_last_of("x");
	std::size_t finalDot = s.find('.');
	int tileWidth = std::stoi(s.substr(finalUnderScore + 1, finalX - finalUnderScore - 1));
	int tileHeight = std::stoi(s.substr(finalX + 1, finalDot - finalX - 1));

	return GetTileset(s, tileWidth, tileHeight, altColorIndex, numColorChanges, startColorBuf, endColorBuf );
}


Tileset * TilesetManager::GetTileset( const std::string & s, int tileWidth, int tileHeight, int altColorIndex )
{
	TilesetCategory cat = GetCategory(s);

	Tileset *alreadyExistsTS = Find(cat, s, altColorIndex);
	if (alreadyExistsTS != NULL)
		return alreadyExistsTS;

	return Create(cat, s, tileWidth, tileHeight, altColorIndex);
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
	TilesetCategory cat = GetCategory(s);

	Tileset *alreadyExistsTS = Find(cat, s, altColorIndex);
	if (alreadyExistsTS != NULL)
	{
		DestroyTileset(alreadyExistsTS);
	}

	return Create(cat, s, tileWidth, tileHeight, altColorIndex);
}

void TilesetManager::ClearTilesets()
{
	list<pair<Tileset*,int>>::iterator listIt;
	list<pair<Tileset*, int>>::iterator listItEnd;
	for (int i = 0; i < C_Count; ++i)
	{
		auto & currMap = tilesetMaps[i];
		for (auto it = currMap.begin(); it != currMap.end(); ++it)
		{
			listIt = (*it).second.begin();
			listItEnd = (*it).second.end();
			for (; listIt != listItEnd; ++listIt)
			{
				delete (*listIt).first;

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
		currMap.clear();
	}
}

void TilesetManager::DestroyTileset(Tileset * t)
{
	TilesetCategory cat = GetCategory(t->sourceName);

	auto &currMap = tilesetMaps[cat];
	auto it = currMap.find(t->sourceName);
	if (it != currMap.end())
	{
		list<pair<Tileset*,int>> &currList = (*it).second;
		for (auto lit = currList.begin(); lit != currList.end(); ++lit)
		{
			if ((*lit).first == t)
			{
				currList.erase(lit);
				delete t;

				if (currList.empty())
				{
					currMap.erase(it);
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

void TilesetManager::ResetTilesetAccessCount()
{
	//ignore DEFAULT and PLAYER tilesets, since they are always used
	for (int i = C_ENEMY; i < C_Count; ++i)
	{
		auto &currMap = tilesetMaps[i];
		for (auto it = currMap.begin(); it != currMap.end(); ++it)
		{
			auto &currList = (*it).second;
			for (auto lit = currList.begin(); lit != currList.end(); ++lit)
			{
				(*lit).second = 0;
			}
		}
	}
}

void TilesetManager::CleanupUnusedTilests()
{
	for (int i = C_ENEMY; i < C_Count; ++i)
	{
		auto &currMap = tilesetMaps[i];
		for (auto it = currMap.begin(); it != currMap.end(); ++it)
		{
			auto &currList = (*it).second;
			for (auto lit = currList.begin(); lit != currList.end(); )
			{
				if ((*lit).second == 0) //not used
				{
					cout << "cleaning up unused: " << i << " called: " << (*lit).first->sourceName << endl;
					lit = currList.erase(lit);
				}
				else
				{
					++lit;
				}
			}
		}
	}
}