#include "Tileset.h"	
#include <iostream>
#include <assert.h>
#include <boost/filesystem.hpp>
#include "globals.h"

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

int Tileset::GetMemoryUsage()
{
	return texture->getSize().x * texture->getSize().y * 4;
}

TilesetManager::TilesetManager()
{
	gameResourcesMode = true;
	lastQueriedTilesetWasDuplicate = false;
	parentManager = NULL;
}

void TilesetManager::SetParentTilesetManager(TilesetManager *man)
{
	parentManager = man;
}

Tileset *TilesetManager::Find(TilesetCategory cat, const std::string &s)
{
	if (parentManager != NULL)
	{
		Tileset *parentRes = parentManager->Find(cat, s);
		if (parentRes != NULL)
			return parentRes;

		//if parent doesn't have it, search my own stuff
	}

	Tileset *currTS;
	auto & currMap = tilesetMaps[cat];
	auto it = currMap.find(s);
	if (it != currMap.end())
	{
		auto &tPair = (*it).second;
		tPair.second++;
		return tPair.first;
	}

	return NULL;
}

void TilesetManager::Add(TilesetCategory cat, const std::string &s, Tileset *ts)
{
	tilesetMaps[cat][s] = make_pair(ts, 1);//.push_back(make_pair(ts,1));
}

void TilesetManager::SetGameResourcesMode(bool on)
{
	gameResourcesMode = on;
}

bool TilesetManager::IsResourceMode()
{
	return gameResourcesMode;
}

int TilesetManager::GetMemoryUsage()
{
	list<pair<Tileset*, int>>::iterator listIt;
	list<pair<Tileset*, int>>::iterator listItEnd;

	int counted = 0;

	for (int i = 0; i < C_Count; ++i)
	{
		auto & currMap = tilesetMaps[i];
		for (auto it = currMap.begin(); it != currMap.end(); ++it)
		{
			auto &tPair = (*it).second;
			counted += tPair.first->GetMemoryUsage();
		}
	}

	return counted;
}

Tileset *TilesetManager::Create(TilesetCategory cat, const std::string &s, int tileWidth, int tileHeight)
{
	string s2;
	if (gameResourcesMode)
	{
		s2 = string("Resources/") + s;
	}
	else
	{
		s2 = s;
	}
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
		cout << "failed to load: " << s << "\n";
		//assert( false );

		return NULL;
	}

	cout << "created texture for: " << s2 << "\n";

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

	t->tileWidth = tileWidth;
	t->tileHeight = tileHeight;
	t->sourceName = s;

	Add(cat, s, t);

	return t;
}

//Texture *TilesetManager::CreateAltColorTex( sf::Image &im,
//		int numAltColors, 
//		sf::Color *startColorBuf,
//		sf::Color *endColorBuf )
//{
//	int imW = im.getSize().x;
//	int imH = im.getSize().y;
//	sf::Color tempColor;
//	for( int x = 0; x < imW; ++x )
//	{
//		for( int y = 0; y < imH; ++y )
//		{
//			for( int i = 0; i < numAltColors; ++i )
//			{
//				tempColor = im.getPixel(x, y);
//				if( startColorBuf[i].r == tempColor.r &&
//					startColorBuf[i].g == tempColor.g &&
//					startColorBuf[i].b == tempColor.b )
//				{
//					tempColor.r = endColorBuf[i].r;
//					tempColor.g = endColorBuf[i].g;
//					tempColor.b = endColorBuf[i].b;
//
//					if (tempColor.a != 255)
//					{
//						int xxxx = 6;
//					}
//					assert( x != 0 || y != 0 );
//					im.setPixel( x, y, tempColor );
//				}
//			}
//		}
//	}
//
//	Texture *tNew = new Texture;
//	tNew->loadFromImage( im );
//
//	return tNew;
//}


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

Tileset * TilesetManager::GetSizedTileset(const std::string & s)
{
	TilesetCategory cat = GetCategory(s);

	Tileset *alreadyExistsTS = Find(cat, s);
	if (alreadyExistsTS != NULL)
	{
		lastQueriedTilesetWasDuplicate = true;
		return alreadyExistsTS;
	}

	lastQueriedTilesetWasDuplicate = false;
	std::size_t finalUnderScore = s.find_last_of("_");
	std::size_t finalX = s.find_last_of("x");
	std::size_t finalDot = s.find('.');
	int tileWidth = std::stoi(s.substr(finalUnderScore + 1, finalX - finalUnderScore - 1));
	int tileHeight = std::stoi(s.substr(finalX + 1, finalDot - finalX - 1));

	return Create(cat, s, tileWidth, tileHeight);
}

Tileset * TilesetManager::GetSizedTileset(const std::string &folder, const std::string & s)
{
	string fullString = folder + s;
	return GetSizedTileset(fullString);
}

Tileset * TilesetManager::GetTileset( const std::string & s, int tileWidth, int tileHeight )
{
	TilesetCategory cat = GetCategory(s);

	Tileset *alreadyExistsTS = Find(cat, s);
	if (alreadyExistsTS != NULL)
	{
		lastQueriedTilesetWasDuplicate = true;
		return alreadyExistsTS;
	}

	lastQueriedTilesetWasDuplicate = false;

	return Create(cat, s, tileWidth, tileHeight);
}

//when the texture has already been created and you want to give ownership to this tilesetmanager and produce a tileset
Tileset *TilesetManager::GetTileset(const std::string & s, sf::Texture *tex)
{
	TilesetCategory cat = C_DEFAULT;

	Tileset *alreadyExistsTS = Find(cat, s);
	if (alreadyExistsTS != NULL)
	{
		lastQueriedTilesetWasDuplicate = true;
		return alreadyExistsTS;
	}
		
	lastQueriedTilesetWasDuplicate = false;

	Tileset *t = new Tileset();
	t->texture = tex;

	t->tileWidth = tex->getSize().x;
	t->tileHeight = tex->getSize().y;
	t->sourceName = s;

	Add(cat, s, t);

	return t;
}


Tileset *TilesetManager::GetUpdatedTileset(
	const std::string & s, int tileWidth, int tileHeight)
{
	TilesetCategory cat = GetCategory(s);

	Tileset *alreadyExistsTS = Find(cat, s);
	if (alreadyExistsTS != NULL)
	{
		DestroyTileset(alreadyExistsTS);
	}

	return Create(cat, s, tileWidth, tileHeight);
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
			auto &tPair = (*it).second;
			delete tPair.first;
		}
		currMap.clear();
	}
}

//only deletes it if it exists
void TilesetManager::DestroyTilesetIfExists(const std::string &sourceName,
	int altIndex)
{
	TilesetCategory cat = GetCategory(sourceName);

	auto &currMap = tilesetMaps[cat];
	auto it = currMap.find(sourceName);
	if (it != currMap.end())
	{
		auto &tPair = (*it).second;

		delete tPair.first;
		
		currMap.erase(it);
	}
}

void TilesetManager::DestroyTileset(Tileset * t)
{
	TilesetCategory cat = GetCategory(t->sourceName);

	auto &currMap = tilesetMaps[cat];
	auto it = currMap.find(t->sourceName);
	if (it != currMap.end())
	{
		delete t;

		currMap.erase(it);
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
			auto &tPair = (*it).second;

			tPair.second = 0;
		}
	}
}

void TilesetManager::CleanupUnusedTilests()
{
	for (int i = C_ENEMY; i < C_Count; ++i)
	{
		auto &currMap = tilesetMaps[i];
		for (auto it = currMap.begin(); it != currMap.end();)
		{
			auto &tPair = (*it).second;

			if (tPair.second == 0) //not used
			{
				cout << "cleaning up unused: " << i << " called: " << tPair.first->sourceName << endl;
				delete tPair.first;
				currMap.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
}

void TilesetManager::DestroyTilesetCategory(TilesetCategory tc)
{
	auto & currMap = tilesetMaps[tc];
	for (auto it = currMap.begin(); it != currMap.end(); ++it)
	{
		auto & tPair = (*it).second;
		delete tPair.first;;
	}
	currMap.clear();
}