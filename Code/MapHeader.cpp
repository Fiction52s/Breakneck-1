#include "MapHeader.h"
#include <fstream>
#include "SFML\Graphics.hpp"
#include <assert.h>
#include <sstream>
#include <iostream>

using namespace std;
using namespace sf;
using namespace boost::filesystem;

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

	//get description
	char last = 0;
	char curr = 0;
	stringstream ss;
	string tempStr;
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
				ss << last;
			}
		}
	}

	is >> numShards;
	shardInfoVec.reserve(16);
	int w, li;
	for (int i = 0; i < numShards; ++i)
	{
		is >> w;
		is >> li;
		shardInfoVec.push_back(ShardInfo(w, li));
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

	string collectionName;
	is >> collectionName;

	if (collectionName == "")
	{
		string test;
		is >> test;
		int bb = 65;
		assert(0);
	}

	ver1 = part1Num;
	ver2 = part2Num;

	is >> gameMode;
	is >> envWorldType;
	is >> envName;
	is >> leftBounds;
	is >> topBounds;
	is >> boundsWidth;
	is >> boundsHeight;
	is >> drainSeconds;
	is >> bossFightType;
	is >> numVertices;

	if (ver1 >= 2 && ver2 >= 2)
	{
		is >> preLevelSceneName;
		is >> postLevelSceneName;
	}

	collectionName = collectionName;
	
	description = ss.str();

	if (ver1 < 2 || ( ver1 == 2 && ver2 < 3) )
	{
		gameMode = T_BASIC;
	}

	return true;
}

void MapHeader::Save(std::ofstream &of)
{
	of << ver1 << "." << ver2 << "\n";
	of << description << "<>\n";

	of << numShards << "\n";
	for (auto it = shardInfoVec.begin(); it != shardInfoVec.end(); ++it)
	{
		of << (*it).world << " " << (*it).localIndex << "\n";
	}

	of << songOrder.size() << "\n";
	for (auto it = songOrder.begin(); it != songOrder.end(); ++it)
	{
		of << (*it) << "\n" << songLevels[(*it)] << "\n";
	}

	/*of << songLevels.size() << "\n";
	for (auto it = songLevels.begin(); it != songLevels.end(); ++it)
	{
		of << (*it).first << "\n" << (*it).second << "\n";
	}*/

	of << collectionName << "\n";
	of << gameMode << "\n";

	//of << (int)envType << " " << envLevel << endl;
	of << envWorldType << " ";
	of << envName << endl;

	of << leftBounds << " " << topBounds << " " << boundsWidth << " " << boundsHeight << endl;

	of << drainSeconds << endl;

	of << bossFightType << endl;

	of << numVertices << "\n";

	of << preLevelSceneName << "\n";

	of << postLevelSceneName << "\n";
}

bool MapHeader::Replace(boost::filesystem::path &p )
{
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

	//mh->songLevelsModified = false;

	return true;
}

int MapHeader::GetNumPlayers()
{
	switch (gameMode)
	{
	case T_BASIC:
		return 1;
	case T_REACHENEMYBASE:
		return 2;
	case T_FIGHT:
		return 2;
	default:
		return 1;
	}
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
