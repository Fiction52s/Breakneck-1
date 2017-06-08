#include "MusicSelector.h"
#include "SoundManager.h"
#include <sstream>
#include <assert.h>
#include <iostream>
#include "UIWindow.h"
#include "MainMenu.h"


using namespace sf;
using namespace std;
using namespace boost::filesystem;

//const int MusicSelector::NUM_BOXES = 8;
const int MusicSelector::BOX_WIDTH = 300;
const int MusicSelector::BOX_HEIGHT = 40;
const int MusicSelector::BOX_SPACING = 10;

MusicSelector::MusicSelector(MainMenu *p_mainMenu, Vector2f &p_topMid,
	list<MusicInfo*> &p_songs)
	:font(p_mainMenu->arial),topIndex(0), oldCurrIndex(0), topMid(p_topMid),
	mainMenu( p_mainMenu ), rawSongs( p_songs )
{
	//assert(!p_profiles.empty());
	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 0, 0);

	currSong = songs.front();//p_profiles.front(); //KIN 

	SetupBoxes();

	for (int i = 0; i < NUM_BOXES; ++i)
	{
		musicNames[i].setFont(font);
		musicNames[i].setCharacterSize(40);
		musicNames[i].setFillColor(Color::White);
	}
}

void MusicSelector::Draw(sf::RenderTarget *target)
{
	target->draw(boxes, NUM_BOXES * 4, sf::Quads);
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		//cout << "drawing: " << profileNames[i].getString().toAnsiString() << "\n";
		target->draw(musicNames[i]);
	}
}


void MusicSelector::UpdateNames()
{
	if (songs.size() == 0)
	{

		return;
	}

	auto lit = songs.begin();
	if (topIndex > songs.size())
	{
		topIndex = songs.size() - 1;
	}

	for (int i = 0; i < topIndex; ++i)
	{
		++lit;
	}

	int trueI;
	int i = 0;
	int numSongs = songs.size();
	for (; i < NUM_BOXES; ++i)
	{
		trueI = (topIndex + i) % NUM_BOXES;
		if (i == numSongs)
		{
			for (; i < NUM_BOXES; ++i)
			{
				musicNames[i].setString("");
			}
			break;
		}

		if (lit == songs.end())
			lit = songs.begin();

		musicNames[i].setString((*lit)->songPath.string());
		musicNames[i].setOrigin(musicNames[i].getLocalBounds().width / 2, 0);
		musicNames[i].setPosition(topMid.x, topMid.y + (BOX_HEIGHT + BOX_SPACING) * i);

		++lit;
	}

	saSelector->totalItems = numSongs;
}

void MusicSelector::Update()
{
	ControllerState &currInput = mainMenu->menuCurrInput;
	ControllerState &prevInput = mainMenu->menuPrevInput;

	if (currInput.A && !prevInput.A)
	{
		
		
	}
	else if (currInput.X && !prevInput.X)
	{
		
	}
	if (currInput.B && !prevInput.B)
	{
		
	}

	//tomorrow: set up the edit profile grid to draw in a separate state from a selected
	//profile. then make a popup window where you input a button to change your controls.
	//editProfileGrid->Update( currInput, prevInput );


	bool up = currInput.LUp();
	bool down = currInput.LDown();

	int changed = saSelector->UpdateIndex(up, down);
	int cIndex = saSelector->currIndex;

	bool inc = changed > 0;
	bool dec = changed < 0;

	if (inc)
	{
		if (cIndex - topIndex == NUM_BOXES)
		{
			topIndex = cIndex - (NUM_BOXES - 1);
		}
		else if (cIndex == 0)
		{
			topIndex = 0;
		}
	}
	else if (dec)
	{
		/*if (currIndex > 0)
		{
		currIndex--;

		if (currIndex < topIndex)
		{
		topIndex = currIndex;
		}
		}
		else
		{
		currIndex = profiles.size() - 1;
		topIndex = profiles.size() - NUM_BOXES;
		}*/

		if (cIndex == saSelector->totalItems - 1)
			topIndex = saSelector->totalItems - NUM_BOXES;
		else if (cIndex < topIndex)
			topIndex = cIndex;
	}

	if (changed != 0)
	{
		UpdateNames();
		//cout << "currIndex: " << cIndex << ", topIndex: " << topIndex << endl;
		//controls[oldIndex]->Unfocus();
		//controls[focusedIndex]->Focus();
	}

	//cout << "currIndex : " << currIndex << endl;
	UpdateBoxesDebug();

	
}

void MusicSelector::SetupBoxes()
{
	sf::Vector2f currTopMid;
	int extraHeight = 0;

	for (int i = 0; i < NUM_BOXES; ++i)
	{
		currTopMid = topMid + Vector2f(0, extraHeight);

		boxes[i * 4 + 0].position = Vector2f(currTopMid.x - BOX_WIDTH / 2, currTopMid.y);
		boxes[i * 4 + 1].position = Vector2f(currTopMid.x + BOX_WIDTH / 2, currTopMid.y);
		boxes[i * 4 + 2].position = Vector2f(currTopMid.x + BOX_WIDTH / 2, currTopMid.y + BOX_HEIGHT);
		boxes[i * 4 + 3].position = Vector2f(currTopMid.x - BOX_WIDTH / 2, currTopMid.y + BOX_HEIGHT);

		boxes[i * 4 + 0].color = Color::Red;
		boxes[i * 4 + 1].color = Color::Red;
		boxes[i * 4 + 2].color = Color::Red;
		boxes[i * 4 + 3].color = Color::Red;

		extraHeight += BOX_HEIGHT + BOX_SPACING;
	}
}

void MusicSelector::MoveUp()
{
	topIndex++;
	if (topIndex == songs.size())
	{
		topIndex = 0;
	}
}

void MusicSelector::MoveDown()
{
	topIndex--;
	if (topIndex == -1)
	{
		topIndex = songs.size() - 1;
	}
}

void MusicSelector::UpdateBoxesDebug()
{
	Color c;
	int trueI = (saSelector->currIndex - topIndex);
	

	for (int i = 0; i < NUM_BOXES; ++i)
	{
		if (i == trueI)
		{
			c = Color::Blue;
		}
		else
		{
			c = Color::Red;
		}
		boxes[i * 4 + 0].color = c;
		boxes[i * 4 + 1].color = c;
		boxes[i * 4 + 2].color = c;
		boxes[i * 4 + 3].color = c;
	}
}


//use the options section of single player, and then make the right side of multiplayer an options section
//people can press a button (y) to switch their control to that window and influence the single menu input that is built from them
//need a function that can combine controlelrs into a menu input even if there are not all of them present

MusicManager::MusicManager(MainMenu *p_mainMenu)
	:mainMenu( p_mainMenu )
{
	LoadFolderPaths();
	LoadMusicNames();
	DebugLoadMusic();
}

MusicManager::~MusicManager()
{
	for (auto it = songs.begin(); it != songs.end(); ++it)
	{
		delete (*it);
	}
	songs.clear();
}

bool MusicManager::LoadFolderPaths()
{
	ifstream is;
	is.open("Audio/Music/musicpaths.txt");
	if (is.is_open())
	{
		string s;
		while (getline(is, s))
		{
			cout << "reading directory: " << s << "\n";
		}

		folderPaths.push_back(s);
	}
	else
	{
		assert(0 && "failed to open music directories sheet");
	}

	folderPaths.push_back(current_path().string() + "/Audio/Music/");
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
					songPaths.push_back(p.string());
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
	for (auto it = songs.begin(); it != songs.end(); ++it)
	{
		string fName = (*it)->songPath.filename().stem().string();
		if (name == fName)
		{
			return (*it)->Load();
		}
	}
	assert(0);
	return false;
}

bool MusicManager::DebugLoadMusic()
{
	for (auto it = songPaths.begin(); it != songPaths.end(); ++it)
	{
		MusicInfo *info = new MusicInfo;
		info->music = new sf::Music;
		info->music->openFromFile((*it).string());//mainMenu->soundManager.GetMusic((*it).string());
		//info->name = (*it).filename().stem().string();
		info->songPath = (*it);
		songs.push_back(info);
	}

	return true;
}