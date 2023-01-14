#include "MusicSelector.h"
#include "SoundManager.h"
#include <sstream>
#include <assert.h>
#include <iostream>
#include "UIWindow.h"
#include "MainMenu.h"
#include "GameSession.h"
#include "MusicPlayer.h"
#include "MapHeader.h"

using namespace sf;
using namespace std;
using namespace boost::filesystem;

MusicManager::MusicManager(MainMenu *p_mainMenu)
	:mainMenu( p_mainMenu )
{
	LoadFolderPaths();
	//LoadMusicNames();
	//DebugLoadMusic();
}

MusicManager::~MusicManager()
{
	for (auto it = songMap.begin(); it != songMap.end(); ++it)
	{
		delete (*it).second;
		
	}
	//songs.clear();
}

bool MusicManager::LoadFolderPaths()
{
	ifstream is;
	is.open("Resources/Audio/Music/musicpaths.txt");
	if (is.is_open())
	{
		string s;
		while (getline(is, s))
		{
			cout << "reading directory: " << s << "\n";
			folderPaths.push_back(s);
		}
	}
	else
	{
		//assert(0 && "failed to open music directories sheet");
	}

	folderPaths.push_back(current_path().string() + "/Resources/Audio/Music/");
	return true;
}

bool MusicManager::rLoadMusicNames(const path &p)
{
	try
	{
		if (exists(p))    // does p actually exist?
		{
			if (is_regular_file(p))        // is p a regular file?   
			{
				if (p.extension().string() == ".ogg")
				{
					string fString = p.filename().stem().string();
					if (songMap.count(fString) == 0)
					{ 
						MusicInfo *mi = new MusicInfo;
						mi->songPath = p;
						mi->music = NULL;
						
						songMap[fString] = mi;
					}
					else
					{
						assert("probably don't want to have this multiple music");
					}

					//songPaths.push_back(p.string());
					//cout << "loading ogg: " << p.filename().string() << "\n";
					return true;
				}
			}
			else if (is_directory(p))      // is p a directory?
			{
				vector<path> v;
				////cout << p << " is a directory containing:\n";

				//TreeNode *newDir = new TreeNode;
				//newDir->parent = parentNode;
				//newDir->next = NULL;
				//newDir->name = p.filename().string();
				//newDir->filePath = p;

				copy(directory_iterator(p), directory_iterator(), back_inserter(v));

				//sort(v.begin(), v.end());             // sort, since directory iteration
				//									  // is not ordered on some file systems

				for (vector<path>::const_iterator it(v.begin()); it != v.end(); ++it)
				{
					//cout << "loading folder: " << (*it).filename().string() << "\n";
					rLoadMusicNames((*it));
					//cout << "   " << *it << '\n';
				}
			}
			else
				cout << p << " exists, but is neither a regular file nor a directory\n";
		}
		else
		{
			cout << p << " does not exist music\n";
			return false;
		}
	}
	catch (const filesystem_error& ex)
	{
		cout << ex.what() << '\n';
		return false;
	}

	return true;
}

MusicInfo::MusicInfo()
	:music( NULL )
{

}

MusicInfo::~MusicInfo()
{
	Cleanup();
}

void MusicInfo::Cleanup()
{
	if (music != NULL)
	{
		delete music;
		music = NULL;
	}
}

bool MusicManager::LoadMusicNames()
{
	for (auto it = folderPaths.begin(); it != folderPaths.end(); ++it)
	{
		rLoadMusicNames((*it));
	}

	return true;
}

bool MusicInfo::Load()
{
	if (music == NULL)
	{
		music = new sf::Music;
		bool res = music->openFromFile(songPath.string());
		music->setVolume(0);
		return res;
	}
	return true;
}

bool MusicManager::LoadSong(const std::string &name)
{
	if (songMap.count(name) > 0 )
	{
		return songMap[name]->Load();
	}
	else
	{
		assert(0);
		return false;
	}
	/*for (auto it = songMap.begin(); it != songMap.end(); ++it)
	{
		string fName = (*it).second->songPath.filename().stem().string();
		if (name == fName)
		{
			return (*it)->Load();
		}
	}*/
	assert(0);
	return false;
}

bool MusicManager::DebugLoadMusic()
{
	//for (auto it = songPaths.begin(); it != songPaths.end(); ++it)
	//{
	//	//MusicInfo *info = new MusicInfo;
	//	info->music = new sf::Music;
	//	info->music->openFromFile((*it).string());//mainMenu->soundManager.GetMusic((*it).string());
	//	//info->name = (*it).filename().stem().string();
	//	info->songPath = (*it);
	//}

	return true;
}

MusicInfo *MusicManager::GetMusicInfo(const std::string &name)
{
	if (songMap.count(name) > 0)
	{
		return songMap[name];
	}
	else
	{
		return NULL;
	}
}