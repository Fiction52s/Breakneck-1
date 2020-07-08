#include "MainMenu.h"
#include <boost/filesystem.hpp>
#include "MapHeader.h"
#include "MusicSelector.h"
#include "PlayerRecord.h"
#include "GameSession.h"
#include "MusicPlayer.h"
#include "ControlProfile.h"

using namespace sf;
using namespace std;
using namespace boost::filesystem;

MapSelectionMenu::MapSelectionMenu(MainMenu *p_mainMenu, sf::Vector2f &p_pos)
	:mainMenu(p_mainMenu), font(p_mainMenu->arial), topIndex(0),
	oldCurrIndex(0)
{
	multiSelectorState = MS_NEUTRAL;

	toMultiTransLength = 60;
	fromMultiTransLength = 60;


	allItems = NULL;
	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 0, 0);

	menuOffset = Vector2f(0, 0);
	topMid = p_pos + Vector2f(BOX_WIDTH / 2, 0) + menuOffset;

	SetupBoxes();

	for (int i = 0; i < NUM_BOXES; ++i)
	{
		itemName[i].setFont(font);
		itemName[i].setCharacterSize(40);
		itemName[i].setFillColor(Color::White);
	}

	descriptionText.setFont(font);
	descriptionText.setCharacterSize(20);
	descriptionText.setFillColor(Color::White);
	descriptionText.setPosition(Vector2f(960 + 150, 680 + 40) + menuOffset);

	previewBlank = true;
	blankTest.setFillColor(Color::Blue);
	blankTest.setSize(Vector2f(960 + 24, 540 + 24));
	blankTest.setPosition(Vector2f(960, 0) + menuOffset);

	previewSprite.setPosition(menuOffset + Vector2f(960 + 24, 24));//blankTest.getPosition());

	ts_bg = mainMenu->tilesetManager.GetTileset("Menu/map_select_menu.png", 1920, 1080);
	bg.setTexture(*ts_bg->texture);
	bg.setPosition(menuOffset);

	//playerindex may change later
	singleSection = new MultiSelectionSection(mainMenu, NULL, 0, Vector2f(580 + 240, 680) + menuOffset);

	state = S_MAP_SELECTOR;

	gs = NULL;
	loadThread = NULL;

	LoadItems();

	UpdateItemText();

	UICheckbox *check = new UICheckbox(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, "testcheckbox", 300, 50);
	UICheckbox *check1 = new UICheckbox(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, "testcheckbox", 300, 50);
	UICheckbox *check2 = new UICheckbox(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, "testcheckbox", 300, 50);

	UIControl *testBlah[] = { check, check1, check2 };

	filterOptions = new UIVerticalControlList(NULL, sizeof(testBlah) / sizeof(UIControl*), testBlah, 20);
	Vector2f tLeft = Vector2f(600, 120) + menuOffset;
	filterOptions->SetTopLeft(tLeft.x, tLeft.y);

	progressDisplay = new LoadingMapProgressDisplay(mainMenu, menuOffset + Vector2f(960, 540));
	progressDisplay->SetProgressString("hello here i am");
	progressDisplay->SetProgressString("hello here i am", 2);

	musicSelector = new MusicSelector(mainMenu, this, Vector2f(topMid.x, topMid.y + 80), mainMenu->musicManager);
	musicSelector->UpdateNames();

	ghostSelector = new RecordGhostMenu(mainMenu, topMid);
	ghostSelector->UpdateItemText();

	//ts_multiProfileRow = mainMenu->tilesetManager.GetTileset("Menu/multiprofile.png", 1920, 400);
	//multiProfileRow.setTexture(*ts_multiProfileRow->texture);

	ts_multiSelect = mainMenu->tilesetManager.GetTileset("Menu/multiselector.png", 960, 680);
	multiSelect.setTexture(*ts_multiSelect->texture);

	multiRowOnPos = menuOffset + Vector2f(0, 1080 - 400);
	multiRowOffPos = menuOffset + Vector2f(0, 1080);

	//multiProfileRow.setPosition( multiRowOffPos );


	multiSelectOnPos = menuOffset + Vector2f(0, 0);
	multiSelectOffPos = menuOffset + Vector2f(0, -680);


	int quarter = 1920 / 4;
	//menuOffset = Vector2f(-1920 * 2, 0);
	for (int i = 0; i < 4; ++i)
	{
		Vector2f topMid = Vector2f(quarter * i + quarter / 2, 1080 - 400) + menuOffset;
		multiPlayerSection[i] = new MultiSelectionSection(mainMenu, this, i, topMid);
	}

	for (int i = 0; i < 4; ++i)
	{
		multiPlayerSection[i]->SetTopMid(menuOffset + Vector2f(480 * i + 240, multiRowOffPos.y));
	}

	//filterOptions = new UIVerticalControlList()
}

MapSelectionMenu::~MapSelectionMenu()
{
	for (auto it = collections.begin(); it != collections.end(); ++it)
	{
		delete (*it);
	}

	delete saSelector;
	delete singleSection;
	delete filterOptions;
	delete progressDisplay;
	delete musicSelector;
	delete ghostSelector;
	for (int i = 0; i < 4; ++i)
	{
		delete multiPlayerSection[i];
	}

	if (allItems != NULL)
	{
		delete[] allItems;
	}
}

int MapSelectionMenu::NumPlayersReady()
{
	return 4;
}

void MapSelectionMenu::SetupBoxes()
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



MapSelectionItem::~MapSelectionItem()
{
	delete headerInfo;
}

void MapSelectionMenu::LoadPath(boost::filesystem::path &p)
{
	vector<path> v;
	try
	{
		if (exists(p))    // does p actually exist?
		{
			if (is_regular_file(p))        // is p a regular file?   
			{
				if (p.extension().string() == ".brknk")
				{
					items.push_back(p);
				}
			}
			else if (is_directory(p))      // is p a directory?
			{
				copy(directory_iterator(p), directory_iterator(), back_inserter(v));

				sort(v.begin(), v.end());

				for (vector<path>::iterator it(v.begin()); it != v.end(); ++it)
				{
					LoadPath((*it));
				}
			}
			else
				cout << p << " exists, but is neither a regular file nor a directory\n";
		}
		else
		{
			cout << p << " does not exist\n";
			assert(0);
		}
	}
	catch (const filesystem_error& ex)
	{
		cout << ex.what() << '\n';
		assert(0);
	}
}

bool MapSelectionMenu::AllPlayersReady()
{
	for (int i = 0; i < 4; ++i)
	{
		if (multiPlayerSection[i]->active)
		{
			if (!multiPlayerSection[i]->IsReady())
				return false;
		}
	}



	return true;
}



void MapSelectionMenu::LoadItems()
{
	path p(current_path() / "/Resources/Maps/");
	LoadPath(p);

	map<string, MapCollection*> collectionMap;
	for (auto it = items.begin(); it != items.end(); ++it)
	{
		ifstream is;
		is.open((*it).string());
		if (is.is_open())
		{
			MapHeader *mh = new MapHeader;
			mh->Load(is);
			//MapHeader *mh = MainMenu::ReadMapHeader(is);
			string pFile = string("Maps/Previews/") + (*it).relative_path().stem().string() + string("_preview_912x492.png");//string("Maps/") + (*it).filename().stem().string() + string("_preview_960x540.png");
																															 //string defaultFile = "Menu/nopreview_960x540.png";

			if (collectionMap.find(mh->collectionName) != collectionMap.end())
			{
				MapSelectionItem *item = new MapSelectionItem((*it), mh);

				//load textures in one at a time. loading all textures like this is insane.
				item->ts_preview = NULL; //mainMenu->tilesetManager.GetTileset(pFile, 960, 540);
				if (item->ts_preview == NULL)
				{
					//item->ts_preview = mainMenu->tilesetManager.GetTileset(defaultFile, 960, 540);
				}

				MapCollection *temp = collectionMap[mh->collectionName];

				item->collection = temp;

				temp->collectionName = mh->collectionName;
				temp->maps.push_back(item);
			}
			else
			{
				MapSelectionItem *item = new MapSelectionItem((*it), mh);

				item->ts_preview = mainMenu->tilesetManager.GetTileset(pFile, 960, 540);

				MapCollection *coll = new MapCollection;
				collectionMap[mh->collectionName] = coll;

				item->collection = coll;

				coll->collectionName = mh->collectionName;
				coll->maps.push_back(item);


			}
		}
		else
		{
			assert(0);
		}
	}

	for (auto it = collectionMap.begin(); it != collectionMap.end(); ++it)
	{
		collections.push_back((*it).second);
	}

	numTotalItems = 0;
	for (auto it = collections.begin(); it != collections.end(); ++it)
	{
		++numTotalItems;
		numTotalItems += (*it)->maps.size();
	}

	if (allItems != NULL)
	{
		delete[] allItems;
	}

	allItems = new pair<string, MapIndexInfo>[numTotalItems];

	int ind = 0;
	for (auto it = collections.begin(); it != collections.end(); ++it)
	{
		MapIndexInfo mi;
		mi.coll = (*it);
		mi.item = NULL;

		allItems[ind] = pair<string, MapIndexInfo>((*it)->collectionName, mi);
		++ind;
		assert((*it)->maps.size() > 0);
		for (auto it2 = (*it)->maps.begin(); it2 != (*it)->maps.end(); ++it2)
		{
			mi.coll = NULL;
			mi.item = (*it2);

			allItems[ind] = pair<string, MapIndexInfo>((*it2)->path.filename().stem().string(), mi);
			++ind;
		}
	}
}

void MapSelectionMenu::LoadMap()
{
	//filePath = p_path;

	//SetPreview();

	/*if (stopThread != NULL)
	{
	stopThread->join();
	stopThread = NULL;
	}*/

	//if (loadThread != NULL)
	//{
	//	//assert(loadThread->joinable());
	//	//cout << "joining111" << endl;
	//	//loadThread->join();

	//	delete loadThread;
	//}
	//
	//if (gs != NULL)
	//{
	//	delete gs;
	//}


	int cIndex = saSelector->currIndex;
	int pIndex = GetPairIndex(cIndex);



	gs = new GameSession(NULL, allItems[pIndex].second.item->path.string());

	gs->progressDisplay = progressDisplay;

	loadThread = new boost::thread(GameSession::sLoad, gs);
}

bool MapSelectionMenu::WriteMapHeader(std::ofstream &of, MapHeader *mh)
{
	mh->Save(of);
	//of << mh->ver1 << "." << mh->ver2 << "\n";
	//of << mh->description << "<>\n";

	//of << mh->numShards << "\n";
	//for (auto it = mh->shardNameList.begin(); it != mh->shardNameList.end(); ++it)
	//{
	//	of << (*it) << "\n";
	//}

	//of << mh->songLevels.size() << "\n";
	//for (auto it = mh->songLevels.begin(); it != mh->songLevels.end(); ++it)
	//{
	//	of << (*it).first << "\n";
	//	of << (*it).second << "\n";
	//}

	//of << mh->collectionName << "\n";
	//of << mh->gameMode << "\n";

	////of << (int)mh->envType << " " << mh->envLevel << endl;
	//of << mh->envWorldType << " ";
	//of << mh->envName << endl;

	//of << mh->leftBounds << " " << mh->topBounds << " " << mh->boundsWidth << " " << mh->boundsHeight << endl;


	//of << mh->drainSeconds << endl;

	//of << mh->bossFightType << endl;

	//of << mh->numVertices << endl;

	return true;
}

bool MapSelectionMenu::ReplaceHeader(boost::filesystem::path &p, MapHeader *mh)
{
	ifstream is;
	is.open(p.string());

	path from("Resources/map.tmp");
	ofstream of;
	of.open(from.string());
	assert(of.is_open());
	WriteMapHeader(of, mh);

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

	mh->songLevelsModified = false;

	return true;
}

void MapSelectionMenu::UpdateMultiInput()
{
	if (multiSelectorState == MS_NEUTRAL)
	{

	}
	else if (multiSelectorState == MS_MUSIC)
	{
		multiMusicPrev = multiMusicCurr;
		multiMusicCurr = ControllerState();
		for (int i = 0; i < 4; ++i)
		{



			if (multiPlayerSection[i]->profileSelect->state != ControlProfileMenu::S_MUSIC_SELECTOR)
			{
				continue;
			}

			ControllerState &pInput = mainMenu->GetPrevInput(i);
			ControllerState &cInput = mainMenu->GetCurrInput(i);
			GameController &c = mainMenu->GetController(i);

			//pInput = cInput;
			bool active = c.UpdateState();

			if (active)
			{
				cInput = c.GetState();
				multiMusicCurr.A |= (cInput.A && !pInput.A);
				multiMusicCurr.B |= (cInput.B && !pInput.B);
				multiMusicCurr.X |= (cInput.X && !pInput.X);
				multiMusicCurr.Y |= (cInput.Y && !pInput.Y);
				multiMusicCurr.rightShoulder |= (cInput.rightShoulder && !pInput.rightShoulder);
				multiMusicCurr.leftShoulder |= (cInput.leftShoulder && !pInput.leftShoulder);
				multiMusicCurr.start |= (cInput.start && !pInput.start);
				multiMusicCurr.leftTrigger = max(multiMusicCurr.leftTrigger, cInput.leftTrigger);
				multiMusicCurr.rightTrigger = max(multiMusicCurr.rightTrigger, cInput.rightTrigger);
				multiMusicCurr.back |= (cInput.back && !pInput.back);
				multiMusicCurr.leftStickPad |= cInput.leftStickPad;
			}
			else
			{
				cInput.Set(ControllerState());
			}
		}
	}
	else if (multiSelectorState == MS_GHOST)
	{
		//setup ghost curr
		multiGhostPrev = multiGhostCurr;
		for (int i = 0; i < 4; ++i)
		{
			if (multiPlayerSection[i]->profileSelect->state != ControlProfileMenu::S_GHOST_SELECTOR)
			{
				continue;
			}

			ControllerState &pInput = mainMenu->GetPrevInput(i);
			ControllerState &cInput = mainMenu->GetCurrInput(i);
			GameController &c = mainMenu->GetController(i);

			//pInput = cInput;
			bool active = c.UpdateState();

			if (active)
			{
				cInput = c.GetState();
				multiGhostCurr.A |= (cInput.A && !pInput.A);
				multiGhostCurr.B |= (cInput.B && !pInput.B);
				multiGhostCurr.X |= (cInput.X && !pInput.X);
				multiGhostCurr.Y |= (cInput.Y && !pInput.Y);
				multiGhostCurr.rightShoulder |= (cInput.rightShoulder && !pInput.rightShoulder);
				multiGhostCurr.leftShoulder |= (cInput.leftShoulder && !pInput.leftShoulder);
				multiGhostCurr.start |= (cInput.start && !pInput.start);
				multiGhostCurr.leftTrigger = max(multiGhostCurr.leftTrigger, cInput.leftTrigger);
				multiGhostCurr.rightTrigger = max(multiGhostCurr.rightTrigger, cInput.rightTrigger);
				multiGhostCurr.back |= (cInput.back && !pInput.back);
				multiGhostCurr.leftStickPad |= cInput.leftStickPad;
			}
			else
			{
				cInput.Set(ControllerState());
			}
		}
	}
}

void MapSelectionMenu::sStopLoadThread(MapSelectionMenu *mapMenu, TInfo &ti)
{
	mapMenu->StopLoadThread(ti);
}

void MapSelectionMenu::StopLoadThread(TInfo &ti)
{
	ti.gsession->SetContinueLoading(false);
	ti.loadThread->join();
	delete ti.loadThread;
	delete ti.gsession;
}

void MapSelectionMenu::Update(ControllerState &currInput,
	ControllerState &prevInput)
{
	progressDisplay->UpdateText();
	if (state == S_MAP_SELECTOR)
	{
		if (currInput.B && !prevInput.B)
		{
			mainMenu->SetMode(MainMenu::Mode::TRANS_MAPSELECT_TO_MAIN);
			return;
		}
		else if (currInput.X && !prevInput.X)
		{
			state = S_MAP_OPTIONS;
			return;
		}
		else if (currInput.Y && !prevInput.Y)
		{
			int cIndex = saSelector->currIndex;
			int pIndex = GetPairIndex(cIndex);
			MapSelectionItem *item = allItems[pIndex].second.item;
			if (item != NULL)
			{
				oldState = state;
				state = S_MUSIC_SELECTOR;
				musicSelector->UpdateNames();
				musicSelector->modifiedValues = false;
				musicSelector->SetMapName(item->path.filename().stem().string());

				return;
			}
		}


		int cIndex = saSelector->currIndex;
		int pIndex = GetPairIndex(cIndex);
		if (currInput.A && !prevInput.A)
		{
			MapCollection *mc = allItems[pIndex].second.coll;
			if (mc != NULL)
			{
				mc->expanded = !mc->expanded;
				UpdateItemText();
			}
			else
			{

				MapSelectionItem *item = allItems[pIndex].second.item;
				if (item->headerInfo->gameMode == MapHeader::T_REACHENEMYBASE)
				{
					//mainMenu->multiLoadingScreen->Reset(item->path);
					//mainMenu->SetMode(MainMenu::Mode::TRANS_MAPSELECT_TO_MULTIPREVIEW);

					state = S_TO_MULTI_TRANS;
					multiTransFrame = 0;
					//multiProfileRow.setPosition(multiRowOffPos);
					multiSelect.setPosition(multiSelectOffPos);
					for (int i = 0; i < 4; ++i)
					{
						multiPlayerSection[i]->SetTopMid(menuOffset + Vector2f(480 * i + 240, multiRowOffPos.y));
					}

					LoadMap();
					return;
				}
				else
				{
					//single player
					state = S_SELECTING_SKIN;

					LoadMap();
				}


			}
		}

		bool up = currInput.LUp();
		bool down = currInput.LDown();

		if (saSelector->totalItems > 1)
		{
			int changed = saSelector->UpdateIndex(up, down);
			cIndex = saSelector->currIndex;

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
				if (cIndex == saSelector->totalItems - 1)
					topIndex = max(saSelector->totalItems - NUM_BOXES, 0);
				else if (cIndex < topIndex)
					topIndex = cIndex;
			}

			if (changed != 0)
			{
				UpdateItemText();

				int pIndex = GetPairIndex(cIndex);
				if (allItems[pIndex].second.item == NULL)
				{
					descriptionText.setString("");
					previewBlank = true;
				}
				else
				{
					descriptionText.setString(allItems[pIndex].second.item->headerInfo->description);
					if (allItems[pIndex].second.item->ts_preview != NULL)
					{
						previewSprite.setTexture(*allItems[pIndex].second.item->ts_preview->texture);
						previewBlank = false;
					}
				}

			}
		}


		UpdateBoxesDebug();
	}
	else if (state == S_SELECTING_SKIN)
	{
		if (singleSection->ShouldGoBack())
		{
			//kill loading map!

			state = S_MAP_SELECTOR;

			//assert(stopThread == NULL);

			CleanupStopThreads();

			TInfo ti;
			ti.gsession = gs;
			ti.loadThread = loadThread;
			stopThreads.push_back(new boost::thread(&MapSelectionMenu::sStopLoadThread, this, ti));
			return;
		}


		if (currInput.Y && !prevInput.Y)
		{
			int cIndex = saSelector->currIndex;
			int pIndex = GetPairIndex(cIndex);

			MapIndexInfo &mi = allItems[pIndex].second;

			oldState = state;
			state = S_MUSIC_SELECTOR;
			musicSelector->UpdateNames();
			musicSelector->modifiedValues = false;
			musicSelector->SetMapName(mi.item->path.filename().stem().string());
			return;
		}
		else if (currInput.X && !prevInput.X)
		{
			state = S_GHOST_SELECTOR;

			return;
		}
		else
		{
			singleSection->Update();
		}

		if (singleSection->IsReady())
		{
			//loadThread->join();
			//boost::chrono::steady_clock::now()
			if (loadThread->try_join_for(boost::chrono::milliseconds(0)))
			{
				int cIndex = saSelector->currIndex;
				int pIndex = GetPairIndex(cIndex);

				//mh->songLevelsModified = true;
				MapSelectionItem *mi = allItems[pIndex].second.item;
				if (mi->headerInfo->songLevelsModified)
				{
					/*auto &songLevels = mi->headerInfo->songLevels;
					for (auto it = songLevels.begin(); it != songLevels.end();)
					{
					if (mainMenu->musicManager->songMap.count((*it).first) == 0)
					{
					songLevels.erase(it++);
					}
					else
					{
					++it;
					}
					}*/

					ReplaceHeader(mi->path, mi->headerInfo);
				}

				mainMenu->GetController(singleSection->playerIndex).SetFilter(singleSection->profileSelect->currProfile->filter);

				list< GhostEntry*> ghosts;
				ghostSelector->GetActiveList(ghosts);
				if (!ghosts.empty())
				{
					gs->SetupGhosts(ghosts);
					//load ghosts
					//into list
				}

				mainMenu->gameRunType = MainMenu::GRT_FREEPLAY;
				int res = gs->Run();

				XBoxButton filter[ControllerSettings::Count];
				SetFilterDefault(filter);

				for (int i = 0; i < 4; ++i)
				{
					mainMenu->GetController(i).SetFilter(filter);
				}

				ghostSelector->UpdateLoadedFolders();



				TInfo ti;
				ti.gsession = gs;
				ti.loadThread = loadThread;
				stopThreads.push_back(new boost::thread(&MapSelectionMenu::sStopLoadThread, this, ti));

				/*delete loadThread;
				loadThread = NULL;
				delete gs;
				gs = NULL;*/

				View vv;
				vv.setCenter(960, 540);
				vv.setSize(1920, 1080);
				mainMenu->window->setView(vv);

				mainMenu->v.setCenter(mainMenu->leftCenter);
				mainMenu->v.setSize(Vector2f(1920, 1080));
				mainMenu->preScreenTexture->setView(mainMenu->v);

				//singleSection->isReady = false;
				state = S_MAP_SELECTOR;


			}
		}
	}
	else if (state == S_MAP_OPTIONS)
	{
		if ((currInput.B && !prevInput.B))//|| (currInput.B && !prevInput.B ) )
		{
			state = S_MAP_SELECTOR;
			return;
		}

		filterOptions->Update(currInput, prevInput);
	}
	else if (state == S_MUSIC_SELECTOR)
	{
		if ((currInput.B && !prevInput.B))
		{
			if (musicSelector->previewSong != NULL)
			{
				musicSelector->previewSong->music->stop();
				musicSelector->previewSong = NULL;
			}
			if (oldState == S_MAP_SELECTOR)
			{
				int cIndex = saSelector->currIndex;
				int pIndex = GetPairIndex(cIndex);

				MapSelectionItem *mi = allItems[pIndex].second.item;

				auto &songLevels = mi->headerInfo->songLevels;
				for (auto it = songLevels.begin(); it != songLevels.end();)
				{
					if (mainMenu->musicManager->songMap.count((*it).first) == 0)
					{
						songLevels.erase(it++);
					}
					else
					{
						++it;
					}
				}

				//guaranteed to be a file not a folder
				ReplaceHeader(mi->path, mi->headerInfo);

				musicSelector->modifiedValues = false;
				//save
			}
			else if (oldState == S_SELECTING_SKIN || oldState == S_MAP_OPTIONS)
			{

				//save after starting the level or when someone cancels the load //this might be causing a bug atm
			}
			if (musicSelector->modifiedValues)
			{

			}
			state = oldState;
		}

		musicSelector->Update(mainMenu->menuCurrInput, mainMenu->menuPrevInput);
	}
	else if (state == S_MUSIC_OPTIONS)
	{
		if (currInput.B && !prevInput.B)
		{
			state = S_MUSIC_SELECTOR;
		}
	}
	else if (state == S_GHOST_SELECTOR)
	{
		if (currInput.B && !prevInput.B)
		{
			state = S_SELECTING_SKIN;
			return;
			//state = S_MUSIC_SELECTOR;
		}

		ghostSelector->Update(mainMenu->menuCurrInput, mainMenu->menuPrevInput);
	}
	else if (state == S_GHOST_OPTIONS)
	{
		if (currInput.B && !prevInput.B)
		{
			state = S_GHOST_SELECTOR;
			return;
		}
	}
	else if (state == S_TO_MULTI_TRANS)
	{
		if (multiTransFrame > toMultiTransLength)
		{
			state = S_MULTI_SCREEN;
		}
		else
		{
			float r = (float)multiTransFrame / toMultiTransLength;
			multiProfileRow.setPosition(multiRowOffPos * (1 - r) + multiRowOnPos * r);

			multiSelect.setPosition(multiSelectOffPos * (1 - r) + multiSelectOnPos * r);

			for (int i = 0; i < 4; ++i)
			{
				multiPlayerSection[i]->SetTopMid(menuOffset + Vector2f(480 * i + 240, multiProfileRow.getPosition().y));
			}

			++multiTransFrame;
		}
	}
	else if (state == S_FROM_MULTI_TRANS)
	{
		if (multiTransFrame > fromMultiTransLength)
		{
			state = S_MAP_SELECTOR;
		}
		else
		{
			float r = (float)multiTransFrame / fromMultiTransLength;
			multiProfileRow.setPosition(multiRowOnPos * (1 - r) + multiRowOffPos * r);

			multiSelect.setPosition(multiSelectOnPos * (1 - r) + multiSelectOffPos * r);

			for (int i = 0; i < 4; ++i)
			{
				multiPlayerSection[i]->SetTopMid(menuOffset + Vector2f(480 * i + 240, multiProfileRow.getPosition().y));
			}

			++multiTransFrame;
		}
	}
	else if (state == S_MULTI_SCREEN)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (multiPlayerSection[i]->ShouldGoBack())
			{
				//kill loading map!
				state = S_FROM_MULTI_TRANS;
				multiTransFrame = 0;

				//assert(stopThread == NULL);

				CleanupStopThreads();

				TInfo ti;
				ti.gsession = gs;
				ti.loadThread = loadThread;
				stopThreads.push_back(new boost::thread(&MapSelectionMenu::sStopLoadThread, this, ti));
				return;
			}
		}

		for (int i = 0; i < 4; ++i)
		{
			multiPlayerSection[i]->Update();
		}

		bool ghostOn = false;
		bool musicOn = false;
		bool allNeutral = true;

		for (int i = 0; i < 4; ++i)
		{
			if (multiPlayerSection[i]->profileSelect->state == ControlProfileMenu::State::S_MUSIC_SELECTOR)
			{
				musicOn = true;
				allNeutral = false;
				break;
			}
		}

		for (int i = 0; i < 4; ++i)
		{
			if (multiPlayerSection[i]->profileSelect->state == ControlProfileMenu::State::S_GHOST_SELECTOR)
			{
				if (musicOn)
				{
					multiPlayerSection[i]->profileSelect->state = ControlProfileMenu::State::S_SELECTED;
				}
				else
				{
					ghostOn = true;
					allNeutral = false;
					break;
				}
			}
		}



		if (multiSelectorState == MS_NEUTRAL)
		{
			if (musicOn)
			{
				multiSelectorState = MS_MUSIC;
				musicSelector->SetMultiOn(true);
				return;
			}
			else if (ghostOn)
			{
				multiSelectorState = MS_GHOST;
				return;
			}
		}
		else if ((multiSelectorState == MS_MUSIC || multiSelectorState == MS_GHOST) && allNeutral)
		{
			multiSelectorState = MS_NEUTRAL;
			musicSelector->SetMultiOn(false);
			return;
		}



		if (!musicOn && !ghostOn && mainMenu->menuCurrInput.start && !mainMenu->menuPrevInput.start)
		{
			if (AllPlayersReady() && NumPlayersReady() > 1) //replace the numplayersready based on the map specifics
			{
				//loadThread->join();
				//boost::chrono::steady_clock::now()
				if (loadThread->try_join_for(boost::chrono::milliseconds(0)))
				{
					for (int i = 0; i < 4; ++i)
					{
						mainMenu->GetController(i).SetFilter(multiPlayerSection[i]->profileSelect->currProfile->filter);
					}
					mainMenu->gameRunType = MainMenu::GRT_FREEPLAY;
					int res = gs->Run();

					XBoxButton filter[ControllerSettings::Count];
					SetFilterDefault(filter);

					for (int i = 0; i < 4; ++i)
					{
						mainMenu->GetController(i).SetFilter(filter);
					}

					/*delete loadThread;
					loadThread = NULL;
					delete gs;
					gs = NULL;*/
					CleanupStopThreads();

					TInfo ti;
					ti.gsession = gs;
					ti.loadThread = loadThread;
					stopThreads.push_back(new boost::thread(&MapSelectionMenu::sStopLoadThread, this, ti));

					View vv;
					vv.setCenter(960, 540);
					vv.setSize(1920, 1080);
					mainMenu->window->setView(vv);

					mainMenu->v.setCenter(mainMenu->leftCenter);
					mainMenu->v.setSize(Vector2f(1920, 1080));
					mainMenu->preScreenTexture->setView(mainMenu->v);

					mainMenu->SetMode(MainMenu::Mode::MAPSELECT);
					mainMenu->mapSelectionMenu->state = MapSelectionMenu::State::S_MAP_SELECTOR;
					mainMenu->v.setCenter(mainMenu->leftCenter);
					mainMenu->preScreenTexture->setView(mainMenu->v);
				}
			}
		}


		UpdateMultiInput();


		if (musicOn)
		{
			musicSelector->Update(multiMusicCurr, multiMusicPrev);
		}
		if (musicOn && (multiMusicCurr.B && !multiMusicPrev.B))
		{
			if (musicSelector->previewSong != NULL)
			{
				musicSelector->previewSong->music->stop();
				musicSelector->previewSong = NULL;
			}

			if (multiSelectorState == MS_MUSIC_OPTIONS)
			{
				multiSelectorState == MS_MUSIC;
				return;
			}

			//save after starting the level or when someone cancels the load //this might be causing a bug atm

			if (musicSelector->modifiedValues)
			{

			}
		}


		if (ghostOn)
		{
			ghostSelector->Update(multiMusicCurr, multiMusicPrev);
		}
		if (ghostOn && (multiGhostCurr.B && !multiGhostPrev.B))
		{
			if (multiSelectorState == MS_GHOST_OPTIONS)
			{
				multiSelectorState = MS_GHOST;
				return;
			}
		}
	}
	else if (state == S_TO_MULTI_TRANS)
	{
		//hold B to go back
	}
}

void MapSelectionMenu::MoveUp()
{
	topIndex++;
	if (topIndex == items.size())
	{
		topIndex = 0;
	}
}

void MapSelectionMenu::MoveDown()
{
	topIndex--;
	if (topIndex == -1)
	{
		topIndex = items.size() - 1;
	}
}

void MapSelectionMenu::UpdateItemText()
{
	int totalShownItems = 0;
	for (auto it = collections.begin(); it != collections.end(); ++it)
	{
		++totalShownItems;
		if ((*it)->expanded)
		{
			totalShownItems += (*it)->maps.size();
		}
	}

	saSelector->totalItems = totalShownItems;

	if (numTotalItems == 0)
	{
		return;
	}

	if (topIndex > totalShownItems)
	{
		topIndex = totalShownItems - 1;
	}

	int ind = topIndex;

	int trueI;
	int i = 0;
	Color col;
	for (; i < NUM_BOXES; ++i)
	{
		trueI = (topIndex + i) % NUM_BOXES;
		if (i == totalShownItems)
		{
			for (; i < NUM_BOXES; ++i)
			{
				itemName[i].setString("");
			}
			break;
		}

		if (ind == totalShownItems)
			ind = 0;

		pair<string, MapIndexInfo> &p = allItems[GetPairIndex(ind)];
		string printStr = p.first;
		itemName[i].setString(printStr);
		itemName[i].setOrigin(0, 0);
		if (p.second.item != NULL)
		{
			MapSelectionItem *item = p.second.item;
			switch (item->headerInfo->gameMode)
			{
			case MapHeader::T_BASIC:
			{
				col = Color::Cyan;
				break;
			}
			case MapHeader::T_REACHENEMYBASE:
			{
				col = Color::White;
				break;
			}
			default:
			{
				col = Color::Yellow;
			}
			}
			itemName[i].setFillColor(col);
		}
		else if (p.second.coll != NULL)
		{
			itemName[i].setFillColor(Color::White);
		}


		int xVal = topMid.x - BOX_WIDTH / 2;
		if (p.second.coll == NULL) //its just a file
		{
			MapSelectionItem *mi = p.second.item;

			xVal += 60;
		}
		else
		{
			xVal += 10;
			//descriptionText.setString("");
		}
		itemName[i].setPosition(xVal, topMid.y + (BOX_HEIGHT + BOX_SPACING) * i);

		++ind;
	}
}

int MapSelectionMenu::GetPairIndex(int index)
{
	int i = 0;
	for (int it = 0; it < index; ++it)
	{
		if (allItems[i].second.coll != NULL && !allItems[i].second.coll->expanded)
		{
			i += allItems[i].second.coll->maps.size();
		}

		++i;
	}

	return i;
}

void MapSelectionMenu::UpdateBoxesDebug()
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

void MapSelectionMenu::CleanupStopThreads()
{
	for (auto it = stopThreads.begin(); it != stopThreads.end();)
	{
		if ((*it)->try_join_for(boost::chrono::milliseconds(0)))
		{
			(*it)->join();
			delete (*it);
			stopThreads.erase(it++);
		}
		else
		{
			++it;
		}
	}
}

bool MapSelectionMenu::IsMusicSelectorVisible()
{
	return state == S_MUSIC_SELECTOR || (state == S_MULTI_SCREEN && IsMultiMusicOn());
}

bool MapSelectionMenu::IsGhostSelectorVisible()
{
	return state == S_GHOST_SELECTOR || (state == S_MULTI_SCREEN && IsMultiGhostOn());
}

bool MapSelectionMenu::IsMultiMusicOn()
{
	for (int i = 0; i < 4; ++i)
	{
		if (multiPlayerSection[i]->profileSelect->state == ControlProfileMenu::S_MUSIC_SELECTOR)
		{
			return true;
		}
	}

	return false;
}

bool MapSelectionMenu::IsMultiGhostOn()
{
	for (int i = 0; i < 4; ++i)
	{
		if (multiPlayerSection[i]->profileSelect->state == ControlProfileMenu::S_GHOST_SELECTOR)
		{
			return true;
		}
	}

	return false;
}

void MapSelectionMenu::Draw(sf::RenderTarget *target)
{
	target->draw(bg);

	target->draw(boxes, NUM_BOXES * 4, sf::Quads);
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		target->draw(itemName[i]);
	}

	if (!previewBlank)
	{
		target->draw(previewSprite);
	}

	target->draw(descriptionText);




	if (state == S_SELECTING_SKIN)
	{
		singleSection->Draw(target);
		//target->draw(playerSprite);
		//profileSelect->Draw(target);
	}
	if (state == S_MULTI_SCREEN || state == S_TO_MULTI_TRANS
		|| state == S_FROM_MULTI_TRANS)
	{
		//target->draw(multiProfileRow);
		target->draw(multiSelect);
		for (int i = 0; i < 4; ++i)
		{
			multiPlayerSection[i]->Draw(target);
		}

		if (multiSelectorState == MS_MUSIC)
		{
			musicSelector->Draw(target);
		}
		else if (multiSelectorState == MS_GHOST)
		{
			ghostSelector->Draw(target);
		}
	}

	filterOptions->Draw(target);
	progressDisplay->Draw(target);

	if (IsMusicSelectorVisible())
	{
		musicSelector->Draw(target);
	}

	if (IsGhostSelectorVisible())
	{
		ghostSelector->Draw(target);
	}


}