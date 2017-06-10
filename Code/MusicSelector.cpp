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

SingleAxisSlider::SingleAxisSlider(Vector2f &p_topMid, int numOptions, int startIndex,
	int width, int height )
	:topMid(p_topMid)
{
	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, numOptions, startIndex, false);
	leftPos = topMid.x - (float)width / 2;

	size = Vector2f(width, height);

	scopeRect.setSize(Vector2f(width, height));
	scopeRect.setOrigin(scopeRect.getLocalBounds().width / 2, 0);
	scopeRect.setPosition(topMid);
	scopeRect.setFillColor(Color::Red);

	sliderRect.setSize(Vector2f(max(2.f, (float)numOptions / width), height) );
	sliderRect.setOrigin(sliderRect.getLocalBounds().width / 2, 0);
	sliderRect.setFillColor(Color::Blue);

	float sectionWidth = (float)size.x / saSelector->totalItems;

	sliderRect.setPosition(Vector2f(leftPos + saSelector->currIndex * sectionWidth, topMid.y));
}

void SingleAxisSlider::Update(ControllerState &currInput, ControllerState &prevInput)
{
	int currIndex = saSelector->currIndex;

	bool left = currInput.LLeft();
	bool right = currInput.LRight();

	int changed = saSelector->UpdateIndex(left, right);
	int cIndex = saSelector->currIndex;

	bool inc = changed > 0;
	bool dec = changed < 0;

	float sectionWidth = (float)size.x / saSelector->totalItems;

	if (changed != 0)
	{
		sliderRect.setPosition(Vector2f(leftPos + saSelector->currIndex * sectionWidth, topMid.y));
	}
}

void SingleAxisSlider::Draw(sf::RenderTarget *target)
{
	target->draw(scopeRect);
	target->draw(sliderRect);
}


MusicSelector::MusicSelector(MainMenu *p_mainMenu, Vector2f &p_topMid,
	list<MusicInfo*> &p_songs)
	:font(p_mainMenu->arial),topIndex(0), oldCurrIndex(0), topMid(p_topMid),
	mainMenu( p_mainMenu ), rawSongs( p_songs )
{
	previewSong = NULL;
	songs = NULL;
	//assert(!p_profiles.empty());
	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 0, 0);

	LoadNames();

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
		oftenSlider[i]->Draw(target);
	}
}

void MusicSelector::LoadNames()
{
	int numSongs = rawSongs.size();

	if( songs != NULL )
		delete[]songs;


	songs = new MusicInfo*[numSongs];

	int ind = 0;
	for (auto it = rawSongs.begin(); it != rawSongs.end(); ++it)
	{
		songs[ind] = (*it);
		ind++;
	}

	saSelector->totalItems = numSongs;
}

void MusicSelector::UpdateNames()
{
	int numTotalSongs = saSelector->totalItems;

	if (numTotalSongs == 0)
	{

		return;
	}

	//auto lit = songs.begin();
	if (topIndex > numTotalSongs)
	{
		topIndex = numTotalSongs - 1;
	}

	int ind = topIndex;
	//for (int i = 0; i < topIndex; ++i)
	//{
	//	//++lit;
	//	++ind;
	//}

	int trueI;
	int i = 0;

	for (; i < NUM_BOXES; ++i)
	{
		trueI = (topIndex + i) % NUM_BOXES;
		if (i == numTotalSongs)
		{
			for (; i < NUM_BOXES; ++i)
			{
				musicNames[i].setString("");
			}
			break;
		}

		if (ind == numTotalSongs)
			ind = 0;

		musicNames[i].setString(songs[ind]->songPath.filename().stem().string() );
		musicNames[i].setOrigin(musicNames[i].getLocalBounds().width / 2, 0);
		musicNames[i].setPosition(topMid.x, topMid.y + (BOX_HEIGHT + BOX_SPACING) * i);

		++ind;
	}

	
}

void MusicSelector::Update()
{
	ControllerState &currInput = mainMenu->menuCurrInput;
	ControllerState &prevInput = mainMenu->menuPrevInput;

	MusicInfo *mi = songs[saSelector->currIndex];

	if (currInput.A && !prevInput.A)
	{
		if ( previewSong != NULL )//mi->music != NULL && mi->music->getStatus() == sf::Music::Status::Playing)
		{
			if (previewSong == mi)
			{
				if (mi->music->getStatus() == sf::Music::Status::Playing)
				{
					mi->music->pause();
				}
				else
				{
					mi->music->setVolume(100);
					mi->music->setLoop(true);
					mi->music->play();
				}
				
				//previewSong = NULL;
				//previewSong->music->pause();
				//previewSong = NULL;
			}
			else
			{
				previewSong->music->stop();
				mi->Load();
				previewSong = mi;
				mi->music->setLoop(true);
				mi->music->setVolume(100);
				previewSong->music->play();
			}
		}
		else
		{
			mi->Load();
			mi->music->setVolume(100);
			mi->music->play();
			previewSong = mi;
		}
	}
	else if (currInput.X && !prevInput.X)
	{
		
	}
	if (currInput.B && !prevInput.B)
	{
		
	}

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
		{
			topIndex = saSelector->totalItems - NUM_BOXES;
		}
		else if (cIndex < topIndex)
		{
			topIndex = cIndex;
		}
	}

	if (changed != 0)
	{
		if (previewSong != NULL && previewSong->music->getStatus() == Music::Status::Paused)
		{
			previewSong->music->stop();
			previewSong = NULL;
		}

		UpdateNames();
		//cout << "currIndex: " << cIndex << ", topIndex: " << topIndex << endl;
		//controls[oldIndex]->Unfocus();
		//controls[focusedIndex]->Focus();
	}
	else
	{
		oftenSlider[cIndex-topIndex]->Update(currInput, prevInput);
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

		
		oftenSlider[i] = new SingleAxisSlider(Vector2f(currTopMid.x, currTopMid.y + 20), 30, 15, 300, 24 );
		

		extraHeight += BOX_HEIGHT + BOX_SPACING;
	}
}

void MusicSelector::MoveUp()
{
	topIndex++;
	if (topIndex == saSelector->totalItems )
	{
		topIndex = 0;
	}
}

void MusicSelector::MoveDown()
{
	topIndex--;
	if (topIndex == -1)
	{
		topIndex = saSelector->totalItems - 1;
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
	//LoadMusicNames();
	//DebugLoadMusic();
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
					MusicInfo *mi = new MusicInfo;
					mi->songPath = p;
					mi->music = NULL;
					songs.push_back(mi);
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
	delete music;
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
	//for (auto it = songPaths.begin(); it != songPaths.end(); ++it)
	//{
	//	//MusicInfo *info = new MusicInfo;
	//	info->music = new sf::Music;
	//	info->music->openFromFile((*it).string());//mainMenu->soundManager.GetMusic((*it).string());
	//	//info->name = (*it).filename().stem().string();
	//	info->songPath = (*it);
	//	songs.push_back(info);
	//}

	return true;
}