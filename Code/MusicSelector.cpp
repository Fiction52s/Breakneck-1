#include "MusicSelector.h"
#include "SoundManager.h"
#include <sstream>
#include <assert.h>
#include <iostream>
#include "UIWindow.h"
#include "MainMenu.h"
#include "GameSession.h"


using namespace sf;
using namespace std;
using namespace boost::filesystem;

//const int MusicSelector::NUM_BOXES = 8;
const int MusicSelector::NUM_SLIDER_OPTIONS = 30;
const int MusicSelector::BOX_WIDTH = 580;
const int MusicSelector::BOX_HEIGHT = 80;
const int MusicSelector::BOX_SPACING = 0;



MusicSelector::MusicSelector(MainMenu *p_mainMenu, 
	MapSelectionMenu *p_mapMenu, Vector2f &p_topMid,
	MusicManager *p_musicMan )
	:font(p_mainMenu->arial),topIndex(0), oldCurrIndex(0), topMid(p_topMid),
	mainMenu( p_mainMenu ), musicMan( p_musicMan ), mapMenu( p_mapMenu ), modifiedValues( false )
{
	multiMode = false;
	previewSong = NULL;
	songs = NULL;
	//assert(!p_profiles.empty());
	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 0, 0);

	SetupBoxes();

	LoadNames();

	mapName.setFont(font);
	mapName.setCharacterSize(40);
	mapName.setFillColor(Color::Black);
	//mapName.setOrigin( )
	mapName.setPosition(Vector2f(topMid.x, topMid.y - 40));

	for (int i = 0; i < NUM_BOXES; ++i)
	{
		musicNames[i].setFont(font);
		musicNames[i].setCharacterSize(40);
		musicNames[i].setFillColor(Color::White);
	}

	mapNameRect.setFillColor(Color::White);
	mapNameRect.setSize(Vector2f(BOX_WIDTH, 80));
	mapNameRect.setOrigin(mapNameRect.getLocalBounds().width / 2, mapNameRect.getLocalBounds().height);
	mapNameRect.setPosition(topMid);
}

void MusicSelector::SetMapName(const std::string &mName)
{
	mapName.setString(mName);
	mapName.setOrigin(mapName.getLocalBounds().width / 2, mapName.getLocalBounds().height / 2);
}

void MusicSelector::Draw(sf::RenderTarget *target)
{
	target->draw(boxes, GetNumBoxes() * 4, sf::Quads);
	for (int i = 0; i < GetNumBoxes(); ++i)
	{
		//cout << "drawing: " << profileNames[i].getString().toAnsiString() << "\n";
		target->draw(musicNames[i]);
		oftenSlider[i]->Draw(target);
	}

	vSlider.Draw(target);
	target->draw(mapNameRect);
	target->draw(mapName);
}

void MusicSelector::LoadNames()
{
	int numSongs = musicMan->songMap.size();//rawSongs.size();

	if( songs != NULL )
		delete[]songs;

	songs = new MusicInfo*[numSongs];

	int ind = 0;
	for (auto it = musicMan->songMap.begin(); it != musicMan->songMap.end(); ++it)
	{
		songs[ind] = (*it).second;
		ind++;
	}

	saSelector->totalItems = numSongs;

	Vector2f offset(20, 0);
	vSlider.Setup(Vector2f(topMid.x + BOX_WIDTH / 2 + offset.x, topMid.y + offset.y), 
		Vector2f( vSlider.barSize.x, 
			max((float)GetSelectorHeight() / numSongs, 5.f)), Vector2f( 30, GetSelectorHeight() ) );
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

	string nameStr;
	for (; i < GetNumBoxes(); ++i)
	{
		trueI = (topIndex + i) % GetNumBoxes();
		if (i == numTotalSongs)
		{
			for (; i < GetNumBoxes(); ++i)
			{
				musicNames[i].setString("");
			}
			break;
		}

		if (ind == numTotalSongs)
			ind = 0;

		nameStr = songs[ind]->songPath.filename().stem().string();
		musicNames[i].setString(nameStr);
		musicNames[i].setOrigin(musicNames[i].getLocalBounds().width / 2, 0);
		musicNames[i].setPosition(topMid.x, topMid.y + (BOX_HEIGHT + BOX_SPACING) * i);

		int cIndex = mapMenu->saSelector->currIndex;
		int pIndex = mapMenu->GetPairIndex(cIndex);

		MapSelectionItem *mi = mapMenu->allItems[pIndex].second.item;

		if (mi != NULL)
		{
			auto &songLevels = mi->headerInfo->songLevels;
			if (songLevels.count(nameStr) == 0)
			{
				oftenSlider[i]->saSelector->currIndex = 0;
			}
			else
			{
				oftenSlider[i]->saSelector->currIndex = songLevels[nameStr];
			}

			oftenSlider[i]->UpdateSliderPos();
		}
		

		//oftenSlider[i]->saSelector->currIndex = songLevels.count( musicNames[i]. )

		++ind;
	}

}

void MusicSelector::SetMultiOn(bool on)
{
	multiMode = on;
	LoadNames();
	//Vector2f offset(20, 0);
	//Vector2f offset(20, 0);

	//vSlider.Setup(Vector2f(topMid.x + BOX_WIDTH / 2 + offset.x, topMid.y + offset.y),
	//	Vector2f(vSlider.barSize.x, max(vSlider.selectorSize.y / numSongs, 5.f)), Vector2f(30, GetSelectorHeight()));
	//vSlider.SetSlider((float)saSelector->currIndex / (saSelector->totalItems - 1));
}

int MusicSelector::GetNumBoxes()
{
	if (multiMode)
	{
		return 5;
	}
	else
	{
		return NUM_BOXES;
	}
}

void MusicSelector::Update(ControllerState &currInput, ControllerState &prevInput)
{
	//ControllerState &currInput = mainMenu->menuCurrInput;
	//ControllerState &prevInput = mainMenu->menuPrevInput;

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
		if (mapMenu->gs != NULL)
		{
			mapMenu->gs->levelMusic = songs[saSelector->currIndex];
		}
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
		if (cIndex - topIndex == GetNumBoxes())
		{
			topIndex = cIndex - (GetNumBoxes() - 1);
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
			topIndex = saSelector->totalItems - GetNumBoxes();
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

		vSlider.SetSlider((float)cIndex / (saSelector->totalItems-1));
		//cout << "currIndex: " << cIndex << ", topIndex: " << topIndex << endl;
		//controls[oldIndex]->Unfocus();
		//controls[focusedIndex]->Focus();
	}
	else
	{
		if (oftenSlider[cIndex - topIndex]->Update(currInput, prevInput) != 0)
		{

			int mcIndex = mapMenu->saSelector->currIndex;
			int mpIndex = mapMenu->GetPairIndex(mcIndex);

			MapSelectionItem *mi = mapMenu->allItems[mpIndex].second.item;

			MapHeader *mh = mi->headerInfo;
			auto &songLevels = mh->songLevels; //from the map
			string nameStr = songs[saSelector->currIndex]->songPath.filename().stem().string();
			if (songLevels.count(nameStr) == 0) //i dont have the song im looking for
			{
				int sliderCurr = oftenSlider[cIndex - topIndex]->saSelector->currIndex;
				if (sliderCurr == 0)
				{

				}
				else
				{
					songLevels[nameStr] = sliderCurr;
					oftenSlider[cIndex - topIndex]->UpdateSliderPos();
					mh->songLevelsModified = true;
				}
					//headerInfo->songLevels[nameStr] = headerInfo->
			}
			else
			{
				int sliderCurr = oftenSlider[cIndex - topIndex]->saSelector->currIndex;
				if (sliderCurr == 0)
				{
					songLevels.erase(nameStr);
				}
				else
				{
					songLevels[nameStr] = sliderCurr;
					oftenSlider[cIndex - topIndex]->UpdateSliderPos();
					mh->songLevelsModified = true;
				}
			}
		}
	}

	UpdateBoxesDebug();	
}

void MusicSelector::SetupBoxes()
{
	sf::Vector2f currTopMid;
	int extraHeight = 0;

	for (int i = 0; i < GetNumBoxes(); ++i)
	{
		currTopMid = topMid + Vector2f(0, extraHeight);

		boxes[i * 4 + 0].position = Vector2f(currTopMid.x - BOX_WIDTH / 2, currTopMid.y);
		boxes[i * 4 + 1].position = Vector2f(currTopMid.x + BOX_WIDTH / 2, currTopMid.y);
		boxes[i * 4 + 2].position = Vector2f(currTopMid.x + BOX_WIDTH / 2, currTopMid.y + BOX_HEIGHT);
		boxes[i * 4 + 3].position = Vector2f(currTopMid.x - BOX_WIDTH / 2, currTopMid.y + BOX_HEIGHT);

		boxes[i * 4 + 0].color = Color::Yellow;
		boxes[i * 4 + 1].color = Color::Yellow;
		boxes[i * 4 + 2].color = Color::Yellow;
		boxes[i * 4 + 3].color = Color::Yellow;

		
		oftenSlider[i] = new SingleAxisSlider(Vector2f(currTopMid.x, currTopMid.y + 50), NUM_SLIDER_OPTIONS, 15, 300, 24 );
		

		extraHeight += BOX_HEIGHT + BOX_SPACING;
	}

	Vector2f offset(20, 0);
	vSlider.Setup(Vector2f(topMid.x + BOX_WIDTH / 2 + offset.x, topMid.y + offset.y), Vector2f(30, 0),
		Vector2f(30, GetSelectorHeight()) );
}

int MusicSelector::GetSelectorHeight()
{
	return GetNumBoxes() * (BOX_HEIGHT + BOX_SPACING);
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
	

	for (int i = 0; i < GetNumBoxes(); ++i)
	{
		if (i == trueI)
		{
			c = Color::Blue;
		}
		else
		{
			c = Color::Black;
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

						if (fString == "w01_Cavern")
						{
							int x = 5;
						}
						
						songMap[fString] = mi;
					}
					else
					{
						assert("probably don't want to have this multiple music");
					}

					//songs.push_back(mi);
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