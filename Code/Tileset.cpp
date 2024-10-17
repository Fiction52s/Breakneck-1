#include "Tileset.h"	
#include <iostream>
#include <assert.h>
#include <boost/filesystem.hpp>
#include "globals.h"
#include <fstream>
#include "nlohmann\json.hpp"

using namespace std;
using namespace sf;
using namespace nlohmann;


std::map<std::string, CompressedTilesetInfo> TilesetManager::compressedTilesetInfoMap;

void CompressedTilesetInfo::Set(const std::string &p_name, sf::Vector2i p_origin, sf::Vector2i p_originalTexSize, const std::string &p_realTexName)
{
	originalTexName = p_name;
	origin = p_origin;
	originalTexSize = p_originalTexSize;
	realTexName = p_realTexName;
}

void TilesetManager::LoadCompressedTilesetJSON()
{
	compressedTilesetInfoMap.clear();

	ifstream is;
	is.open("Resources/compressedtextureinfo.json");
	if (is.is_open() )
	{
		json j;
		is >> j;

		string originalTexName;
		Vector2i origin;
		Vector2i originalTexSize;
		string realTexName;
		for (auto it = j.begin(); it != j.end(); ++it)
		{	
			origin.x = (*it)["origin"][0];
			origin.y = (*it)["origin"][1];
			originalTexName = (*it)["originalTexName"];
			originalTexSize.x = (*it)["originalTexSize"][0];
			originalTexSize.y = (*it)["originalTexSize"][1];
			realTexName = (*it)["texName"];

			assert(compressedTilesetInfoMap.count(originalTexName) == 0);
			CompressedTilesetInfo &cti = compressedTilesetInfoMap[originalTexName];
			cti.Set(originalTexName, origin, originalTexSize, realTexName);
		}
	}
	else
	{
		cout << "unable to open compressedtextureinfo.json" << endl;
		assert(0);
	}
}


Tileset::Tileset()
{
	isChild = false;
	tileWidth = 0;
	tileHeight = 0;
	texture = NULL;
}

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
	for (auto it = childTSMap.begin(); it != childTSMap.end(); ++it)
	{
		delete (*it).second;
	}

	if (!isChild)
	{
		delete texture;
	}
}

IntRect Tileset::GetSubRect( int localID )
{
	int xi = localID % gridSize.x;
	int yi = localID / gridSize.x;
		
	if( localID < 0 || localID >= gridSize.x * gridSize.y )
	{
		cout << "texture error: " << this->sourceName << endl;
		cout << "localID: " << localID << ", sx: " << gridSize.x << ", sy: " << gridSize.y << endl;
		assert( 0 );
	}
	return IntRect( origin.x + xi * tileWidth, origin.y + yi * tileHeight, tileWidth, tileHeight );
}


sf::IntRect Tileset::GetCustomSubRect(sf::Vector2i p_tileSize, sf::Vector2i p_origin, sf::Vector2i p_gridSize, int p_tileIndex)
{
	int xi = p_tileIndex % p_gridSize.x;
	int yi = p_tileIndex / p_gridSize.x;

	if (p_tileIndex < 0 || p_tileIndex >= p_gridSize.x * p_gridSize.y)
	{
		cout << "issue in special GetSubRect" << endl;
		cout << "texture error: " << this->sourceName << endl;
		cout << "tileIndex: " << p_tileIndex << ", sx: " << p_gridSize.x << ", sy: " << p_gridSize.y << endl;
		assert(0);
	}
	return IntRect(p_origin.x + xi * p_tileSize.x, p_origin.y + yi * p_tileSize.y, p_tileSize.x, p_tileSize.y);
}

int Tileset::GetNumTiles()
{
	return gridSize.x * gridSize.y;
}

int Tileset::GetMemoryTextureSize()
{
	Vector2u size = texture->getSize();
	int larger = max(size.x, size.y);
	int realSize = 0;

	for (int i = 1; i <= 20; ++i)
	{
		if (pow(2, i) >= larger)
		{
			realSize = pow(2, i);
			break;
		}
	}

	return realSize;
}

int Tileset::GetMemoryUsage()
{
	int realSize = GetMemoryTextureSize();
	return realSize * realSize * 4;
}

TilesetManager::TilesetManager()
{
	gameResourcesMode = true;
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

Tileset *TilesetManager::Create(TilesetCategory cat, const std::string &s, CompressedTilesetInfo *cti, int tileWidth, int tileHeight)
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

	

	Tileset *t = new Tileset();
	t->texture = tex;

	cout << "created texture for: " << s2 << ", mem: " << t->GetMemoryTextureSize() << "\n";


	if (tileWidth == 0)
	{
		tileWidth = tex->getSize().x;
	}
	if (tileHeight == 0)
	{
		tileHeight = tex->getSize().y;
	}

	t->gridSize.x = tex->getSize().x / tileWidth;
	t->gridSize.y = tex->getSize().y / tileHeight;
	t->origin = Vector2i(0, 0);
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

Tileset * TilesetManager::FindOrCreate(const std::string &s, int tw, int th)
{
	string textureStr = s;
	CompressedTilesetInfo *myCompressedInfo = NULL;
	if (compressedTilesetInfoMap.count(s) > 0)
	{
		myCompressedInfo = &compressedTilesetInfoMap[s];
		textureStr = myCompressedInfo->realTexName;
	}

	TilesetCategory cat = GetCategory(textureStr);

	Tileset *alreadyExistsTS = Find(cat, textureStr);
	if (alreadyExistsTS != NULL)
	{
		if (myCompressedInfo != NULL)
		{
			if (alreadyExistsTS->childTSMap.count(s) > 0)
			{
				return alreadyExistsTS->childTSMap[s];
			}
			else
			{
				Tileset *newChild = new Tileset;

				newChild->isChild = true;
				newChild->texture = alreadyExistsTS->texture;
				newChild->tileWidth = tw;//myCompressedInfo->tileSize.x;
				newChild->tileHeight = th;//myCompressedInfo->tileSize.y;
				newChild->origin = myCompressedInfo->origin;

				Vector2i gridSize(myCompressedInfo->originalTexSize.x / tw, myCompressedInfo->originalTexSize.y / th);

				newChild->gridSize = gridSize;
				newChild->sourceName = s;

				alreadyExistsTS->childTSMap[s] = newChild;

				return newChild;
			}
		}


		return alreadyExistsTS;
	}

	Tileset *createdTS = Create(cat, textureStr, myCompressedInfo, tw, th);

	if (myCompressedInfo != NULL)
	{
		if (createdTS->childTSMap.count(s) > 0)
		{
			return createdTS->childTSMap[s];
		}
		else
		{
			Tileset *newChild = new Tileset;

			newChild->isChild = true;
			newChild->texture = createdTS->texture;
			newChild->tileWidth = tw;//myCompressedInfo->tileSize.x;
			newChild->tileHeight = th;//myCompressedInfo->tileSize.y;
			newChild->origin = myCompressedInfo->origin;

			Vector2i gridSize(myCompressedInfo->originalTexSize.x / tw, myCompressedInfo->originalTexSize.y / th);

			newChild->gridSize = gridSize;
			newChild->sourceName = s;

			createdTS->childTSMap[s] = newChild;

			return newChild;
		}
	}

	return createdTS;
}

Tileset * TilesetManager::GetSizedTileset(const std::string & s)
{
	std::size_t finalUnderScore = s.find_last_of("_");
	std::size_t finalX = s.find_last_of("x");
	std::size_t finalDot = s.find('.');
	int tileWidth = std::stoi(s.substr(finalUnderScore + 1, finalX - finalUnderScore - 1));
	int tileHeight = std::stoi(s.substr(finalX + 1, finalDot - finalX - 1));

	return FindOrCreate(s, tileWidth, tileHeight);
}

Tileset * TilesetManager::GetSizedTileset(const std::string &folder, const std::string & s)
{
	string fullString = folder + s;
	return GetSizedTileset(fullString);
}

Tileset * TilesetManager::GetTileset( const std::string & s, int tileWidth, int tileHeight )
{
	return FindOrCreate(s, tileWidth, tileHeight);
}

//when the texture has already been created and you want to give ownership to this tilesetmanager and produce a tileset
Tileset *TilesetManager::GetTileset(const std::string & s, sf::Texture *tex)
{
	//hasnt been updated to the new stuff yet, hopefully doesn't cause any weird issues, but I doubt it

	TilesetCategory cat = C_DEFAULT;

	Tileset *alreadyExistsTS = Find(cat, s);
	if (alreadyExistsTS != NULL)
	{
		return alreadyExistsTS;
	}

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
	//hasn't been updated to the new system, could cause issues but I doubt it

	TilesetCategory cat = GetCategory(s);

	Tileset *alreadyExistsTS = Find(cat, s);
	if (alreadyExistsTS != NULL)
	{
		DestroyTileset(alreadyExistsTS);
	}

	return Create(cat, s, NULL, tileWidth, tileHeight);
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
//unused in the code
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
	//shouldn't need to be updated to the new system if I check for this to be sure
	assert(!t->isChild);


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