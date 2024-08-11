#include "MapHeader.h"
#include <fstream>
#include "SFML\Graphics.hpp"
#include <assert.h>
#include <sstream>
#include <iostream>
#include "steam\steam_api.h"


using namespace std;
using namespace sf;
using namespace boost::filesystem;


//early access launched on version 8
//added medals in version 9

//current version: 9


//enum GameModeType
//{
//	GAME_MODE_BASIC,
//	GAME_MODE_REACHENEMYBASE,
//	GAME_MODE_FIGHT,
//	GAME_MODE_RACE,
//	GAME_MODE_PARALLEL_RACE,
//};

SpecialItemInfo::SpecialItemInfo()
	:itemType(-1), itemIndex0(-1), itemIndex1(-1)
{

}

SpecialItemInfo::SpecialItemInfo(int it, int iv0, int iv1)
	:itemType(it), itemIndex0(iv0), itemIndex1(iv1)
{

}

void SpecialItemInfo::Load(std::ifstream &is)
{
	is >> itemType;
	is >> itemIndex0;
	is >> itemIndex1;
}

void SpecialItemInfo::Save(std::ofstream &of)
{
	of << itemType << " " << itemIndex0 << " " << itemIndex1 << "\n";
}


MapHeader::MapHeader()
{
	Clear();
}

void MapHeader::Clear()
{
	specialMapType = MAPTYPE_NORMAL;
	ver1 = 0;
	ver2 = 0;
	description = "no description";
	numPlayerSpawns = 1;
	leftBounds = 0;
	topBounds = 0;
	boundsWidth = 0;
	boundsHeight = 0;
	numVertices = -1;
	drainSeconds = 60;
	goldSeconds = 0;
	silverSeconds = 0;
	bronzeSeconds = 0;
	bossFightType = 0;
	envName = "w1_01";
	envWorldType = -1;
	preLevelSceneName = "NONE";
	postLevelSceneName = "NONE";
	creatorID = 0;
	possibleGameModeTypeFlags = 0;
	numGameObjects = -1;
	functionalWidth = -1;
	functionalHeight = -1;
	specialItemInfoVec.clear();
}

int MapHeader::GetSpecialMapTypeFromString(const std::string &s)
{
	if (s == "normal")
	{
		return MAPTYPE_NORMAL;
	}
	else if (s == "ship")
	{
		return MAPTYPE_SHIP;
	}

	return -1;
}

std::string MapHeader::GetSpecialMapStringFromType(int t)
{
	if (t == MAPTYPE_NORMAL)
	{
		return "normal";
	}
	else if (t == MAPTYPE_SHIP)
	{
		return "ship";
	}

	return "maptype_error";
}

bool MapHeader::CanRunAsMode(int gm )
{
	bool hasGoal = possibleGameModeTypeFlags & (1 << MapHeader::GameModeFlags::MI_HAS_GOAL);

	switch (gm)
	{
	case MatchParams::GAME_MODE_BASIC:
	{
		if (hasGoal)
		{
			return true;
		}
		break;
	}
	case MatchParams::GAME_MODE_REACHENEMYBASE:
	{
		return false;
		break;
	}
	case MatchParams::GAME_MODE_FIGHT:
	{
		if (numPlayerSpawns >= 2)
		{
			return true;
		}
		break;
	}
	case MatchParams::GAME_MODE_RACE:
	{
		if (numPlayerSpawns >= 2 && hasGoal )
		{
			return true;
		}
		break;
	}
	case MatchParams::GAME_MODE_PARALLEL_RACE:
	{
		if (hasGoal)
		{
			return true;
		}
		break;
	}
	case MatchParams::GAME_MODE_EXPLORE:
	{
		return true;
		break;
	}
	}

	return false;
}

bool MapHeader::CanRunAsMode(int numP, int gm)
{
	if (!CanRunAsMode(gm))
		return false;

	auto playerVec = MatchParams::GetNumPlayerOptions(gm, numPlayerSpawns);
	for (auto possiblePlayersIt = playerVec.begin(); possiblePlayersIt != playerVec.end(); ++possiblePlayersIt)
	{
		if ((*possiblePlayersIt) == numP)
			return true;
	}

	return false;
}

bool MapHeader::CanRun(std::vector<int> & numP, std::vector<int> &gameModes)
{
	//test all combos of players and game mode and see if it works
	for (auto gameIt = gameModes.begin(); gameIt != gameModes.end(); ++gameIt)
	{
		if (!CanRunAsMode((*gameIt)))
		{
			continue;
		}

		auto playerVec = MatchParams::GetNumPlayerOptions((*gameIt), numPlayerSpawns);

		for (auto possiblePlayersIt = playerVec.begin(); possiblePlayersIt != playerVec.end(); ++possiblePlayersIt)
		{
			for (auto paramPlayerOptionsIt = numP.begin(); paramPlayerOptionsIt != numP.end(); ++paramPlayerOptionsIt)
			{
				if ((*possiblePlayersIt) == (*paramPlayerOptionsIt))
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool MapHeader::Load(std::ifstream &is)
{
	assert(is.is_open());

	int part1Num = -1;
	int part2Num = -1;

	string versionString;
	is >> versionString;

	int sepIndex = versionString.find('.');
	string part1 = versionString.substr(0, sepIndex);
	string part2 = versionString.substr(sepIndex + 1);
	try
	{
		part1Num = stoi(part1);
	}
	catch (std::exception & e)
	{
		assert(0);
	}

	try
	{
		part2Num = stoi(part2);
	}
	catch (std::exception & e)
	{
		assert(0);
	}

	ver1 = part1Num;

	if (ver1 >= 7)
	{
		ver2 = 0;
	}
	else
	{
		ver2 = part2Num;
	}

	//get description
	char last = 0;
	char curr = 0;
	stringstream descriptionSS;
	stringstream nameSS;
	string tempStr;

	if (ver1 >= 5)
	{
		int numNameChars;
		is >> numNameChars;

		if (!is.get()) //get newline char out of the way
		{
			assert(0);
		}

		for (int i = 0; i < numNameChars; ++i)
		{
			if (!is.get(curr))
			{
				assert(0);
			}

			nameSS << curr;
		}

		fullName = nameSS.str();
	}
	else
	{
		fullName = "----";
	}

	if (ver1 >= 4)
	{
		int numDescriptionChars;

		is >> numDescriptionChars;

		if (!is.get()) //get newline char out of the way
		{
			assert(0);
		}

		for (int i = 0; i < numDescriptionChars; ++i)
		{
			if (!is.get(curr))
			{
				assert(0);
			}

			descriptionSS << curr;
		}

	}
	else
	{
		if (!is.get()) //get newline char out of the way
		{
			assert(0);
		}
		while (true)
		{
			last = curr;
			if (!is.get(curr))
			{
				assert(0);
			}

			if (last == '<' && curr == '>')
			{
				break;
			}
			else
			{
				if (last != 0)
				{
					descriptionSS << last;
				}
			}
		}
	}

	description = descriptionSS.str();

	if (ver1 > 11)
	{
		string specialMapStr;
		is >> specialMapStr;
		specialMapType = GetSpecialMapTypeFromString(specialMapStr);
	}
	else if (ver1 == 11)
	{
		int blank;
		is >> blank;
		specialMapType = MAPTYPE_NORMAL;
	}
	else
	{
		specialMapType = MAPTYPE_NORMAL;
	}


	if (ver1 <= 8)
	{
		std::vector<ShardInfo> shardInfoVec;
		std::vector<LogInfo> logInfoVec;
		std::vector<int> powerVec;

		int numShards;
		int numLogs;
		int numPowers;

		is >> numShards;
		shardInfoVec.reserve(16);
		int w, li;
		for (int i = 0; i < numShards; ++i)
		{
			is >> w;
			is >> li;
			shardInfoVec.push_back(ShardInfo(w, li));
		}

		if (ver1 > 2 || (ver1 == 2 && ver2 >= 7))
		{
			is >> numLogs;
			logInfoVec.reserve(16);
			for (int i = 0; i < numLogs; ++i)
			{
				is >> w;
				is >> li;
				logInfoVec.push_back(LogInfo(w, li));
			}
		}

		if (ver1 == 8)
		{
			is >> numPowers;
			powerVec.reserve(7);

			int powerIndex;
			for (int i = 0; i < numPowers; ++i)
			{
				is >> powerIndex;
				powerVec.push_back(powerIndex);
			}
		}
	}
	else
	{
		int numSpecialItems;
		is >> numSpecialItems;
		specialItemInfoVec.resize(numSpecialItems);
		for (int i = 0; i < numSpecialItems; ++i)
		{
			is >> specialItemInfoVec[i].itemType;
			is >> specialItemInfoVec[i].itemIndex0;
			is >> specialItemInfoVec[i].itemIndex1;
		}
	}


	int numSongValues;
	is >> numSongValues;

	int oftenLevel;
	string tempSongStr;
	is.get();
	for (int i = 0; i < numSongValues; ++i)
	{
		getline(is, tempSongStr); //this allows spaces in names
		is >> oftenLevel;

		songOrder.push_back(tempSongStr);
		songLevels[tempSongStr] = oftenLevel;
		is.get();
	}

	if (ver1 >= 6)
	{
		getline(is, creatorName);
		//is.get();
		//is >> creatorName;
	}

	if (ver1 < 2 || ( ver1 == 2 && ver2 <= 8 ) )
	{
		string collectionName;
		is >> collectionName;

		if (collectionName == "")
		{
			string test;
			is >> test;
			int bb = 65;
			assert(0);
		}
	}
	else
	{
		is >> creatorID;
		/*string creatorIDStr;
		is >> creatorIDStr;

		creatorID = stoll(creatorIDStr);*/

		//is >> creatorID;
	}

	if (ver1 >= 3)
	{
		is >> numPlayerSpawns;
		is >> possibleGameModeTypeFlags;
	}
	else
	{
		int oldGameMode;
		is >> oldGameMode;

		if ((ver1 == 2 && ver2 <=1) || ver1 < 2 )
		{
			oldGameMode = MatchParams::GAME_MODE_BASIC;
		}

		switch (oldGameMode)
		{
		case MatchParams::GAME_MODE_BASIC:
		{
			numPlayerSpawns = 1;
			break;
		}
		case MatchParams::GAME_MODE_FIGHT:
		{
			numPlayerSpawns = 2;
			break;
		}
		case MatchParams::GAME_MODE_RACE:
		{
			numPlayerSpawns = 2;
			break;
		}
		case MatchParams::GAME_MODE_REACHENEMYBASE:
		{
			numPlayerSpawns = 2;
			break;
		}
		case MatchParams::GAME_MODE_EXPLORE:
		{
			numPlayerSpawns = 1;
			break;
		}
		}
	}
	

	if (ver1 >= 7)
	{
		is >> numGameObjects;
		is >> functionalWidth;
		is >> functionalHeight;
	}
	else
	{
		numGameObjects = -1;
		functionalWidth = -1;
		functionalHeight = -1;
	}

	if (ver1 < 2 || (ver1 == 2 && ver2 <= 7 ) )
	{
		is >> envWorldType;
	}
	
	is >> envName;
	is >> leftBounds;
	is >> topBounds;
	is >> boundsWidth;
	is >> boundsHeight;
	is >> drainSeconds;

	if (ver1 >= 9)
	{
		is >> goldSeconds;
		is >> silverSeconds;
		is >> bronzeSeconds;
	}
	else
	{
		goldSeconds = 0;
		silverSeconds = 0;
		bronzeSeconds = 0;
	}

	is >> bossFightType;
	is >> numVertices;

	if (ver1 > 2 || ( ver1 ==2 && ver2 >= 2))
	{
		is >> preLevelSceneName;
		is >> postLevelSceneName;
	}
	return true;
}

void MapHeader::Save(std::ofstream &of)
{
	//current version is 4, go up by an integer every time from now on
	of << ver1 << "\n"; // << "." << ver2 << "\n";

	of << fullName.size() << "\n";

	if (fullName.size() > 0)
	{
		of << fullName << "\n";
	}

	of << description.size() << "\n";

	of << description << "\n";
	//of << description << "<>\n";

	string specialStr = GetSpecialMapStringFromType(specialMapType);

	of << specialStr << "\n";

	of << specialItemInfoVec.size() << "\n";
	for (auto it = specialItemInfoVec.begin(); it != specialItemInfoVec.end(); ++it)
	{
		of << (*it).itemType << " " << (*it).itemIndex0 << " " << (*it).itemIndex1 << "\n";
	}

	of << songOrder.size() << "\n";
	for (auto it = songOrder.begin(); it != songOrder.end(); ++it)
	{
		of << (*it) << "\n" << songLevels[(*it)] << "\n";
	}

	of << creatorName << "\n";

	of << creatorID << "\n";

	of << numPlayerSpawns << "\n";
	of << possibleGameModeTypeFlags << "\n";

	of << numGameObjects << "\n";
	of << functionalWidth << "\n";
	of << functionalHeight << "\n";

	of << envName << endl;

	of << leftBounds << " " << topBounds << " " << boundsWidth << " " << boundsHeight << endl;

	of << drainSeconds << endl;

	of << goldSeconds << " " << "\n";

	of << silverSeconds << " " << "\n";

	of << bronzeSeconds << " " << "\n";

	of << bossFightType << endl;

	of << numVertices << "\n";

	of << preLevelSceneName << "\n";

	of << postLevelSceneName << "\n";
}

bool MapHeader::Replace(boost::filesystem::path &p )
{
	assert(0);
	return false;
	//depreciated

	ifstream is;
	is.open(p.string());

	path from("Resources/map.tmp");
	ofstream of;
	of.open(from.string());
	assert(of.is_open());

	Save(of);

	if (is.is_open())
	{
		MapHeader *oldHeader = new MapHeader;
		oldHeader->Load(is);// MainMenu::ReadMapHeader(is);

		is.get(); //gets rid of the extra newline char

		char c;
		while (is.get(c))
		{
			of.put(c);
		}

		of.close();
		is.close();

		delete oldHeader;
	}
	else
	{
		assert(0);
	}

	try
	{
		//assert(boost::filesystem::exists(from) && boost::filesystem::exists(p));
		boost::filesystem::copy_file(from, p, copy_option::overwrite_if_exists);
	}
	catch (const boost::system::system_error &err)
	{
		cout << "file already exists!" << endl;
		assert(0);
	}

	return true;
}

void MapHeader::ClearSongs()
{
	songLevels.clear();
	songOrder.clear();
}

void MapHeader::AddSong(const std::string &songName,
	int songLevel)
{
	songOrder.push_back(songName);
	songLevels[songName] = songLevel;
}

int MapHeader::GetNumSongs()
{
	return songOrder.size();
}

int MapHeader::GetLeft()
{
	return leftBounds;
}

int MapHeader::GetTop()
{
	return topBounds;
}

int MapHeader::GetRight()
{
	return leftBounds + boundsWidth;
}

int MapHeader::GetBot()
{
	return topBounds + boundsHeight;
}
