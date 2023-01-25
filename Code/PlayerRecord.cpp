#include "PlayerRecord.h"
#include "Actor.h"
#include <string>
#include <fstream>
#include <assert.h>
#include "Buf.h"
#include <iostream>
#include "GameSession.h"
#include "MainMenu.h"
#include <vector>

using namespace sf;
using namespace std;

const int RecordGhostMenu::BOX_WIDTH = 580;
const int RecordGhostMenu::BOX_HEIGHT = 40;
const int RecordGhostMenu::BOX_SPACING = 0;

ReplayGhost::ReplayGhost(Actor *p_player)
	:player(p_player), sprBuffer(NULL), playerSkinShader( "player")
{
	frame = 0;
	action = 0;
	init = false;

	cs.setFillColor(Color::Red);
	cs.setRadius(50);
	cs.setOrigin(cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2);


	playerSkinShader.SetSkin(Actor::SKIN_GHOST);

	playerSkinShader.pShader.setUniform("u_invincible", 0.f);
	playerSkinShader.pShader.setUniform("u_super", 0.f);
	playerSkinShader.pShader.setUniform("u_slide", 0.f);
}

ReplayGhost::~ReplayGhost()
{
	if (sprBuffer != NULL)
	{
		delete[] sprBuffer;
	}
}

void ReplayGhost::Draw(RenderTarget *target)
{
	if (!init || frame == numTotalFrames)
		return;

	if (!player->IsVisibleAction(action))
	{
		return;
	}

	if (frame >= 0 && frame < numTotalFrames)
	{
		//target->draw(cs);
		target->draw(replaySprite, &playerSkinShader.pShader);
	}
}

void ReplayGhost::Reset()
{
	frame = 0;
}

bool ReplayGhost::OpenGhost(const boost::filesystem::path &filePath)
{
	ifstream is;

	is.open(filePath.string(), ios::binary );
	if (is.is_open())
	{
		header.Read(is);
		//is.read((char*)&header, sizeof(header);

		is.read((char*)&numTotalFrames, sizeof(int));
		
		sprBuffer = new SprInfo[numTotalFrames];

		init = true;

		is.read((char*)sprBuffer, sizeof(SprInfo) * numTotalFrames);

		/*for (int i = 0; i < numTotalFrames; ++i)
		{
			SprInfo &info = sprBuffer[i];
			is >> info.position.x;
			is >> info.position.y;

			is >> info.origin.x;
			is >> info.origin.y;

			is >> info.rotation;

			int fx;
			int fy;

			is >> fx;
			is >> fy;

			info.flipX = (bool)fx;
			info.flipY = (bool)fy;

			is >> info.action;
			is >> info.tileIndex;

			is >> info.speedLevel;

		}*/

		is.close();

		frame = 0;
		return true;
	}
	//return false on failure
	return false;

}

void ReplayGhost::UpdateReplaySprite()
{
	if (!init || frame == numTotalFrames)
		return;

	if (player->action == Actor::SPAWNWAIT)
		return;

	

	SprInfo &info = sprBuffer[frame];
	Tileset *ts = player->tileset[(Actor::Action)info.action];
	replaySprite.setTexture(*ts->texture);

	action = info.action;
	Actor::Action a = (Actor::Action)info.action;
	/*if (a == Actor::JUMPSQUAT)
	{
		cout << "setting jumpsquat in ghost. frame: " << frame << endl;
	}*/
	IntRect ir = ts->GetSubRect(info.tileIndex);

	replaySprite.setRotation(info.rotation);


	float width = ts->texture->getSize().x;
	float height = ts->texture->getSize().y;

	playerSkinShader.pShader.setUniform("u_quad", Glsl::Vec4(ir.left / width, ir.top / height,
		(ir.left + ir.width) / width, (ir.top + ir.height) / height));

	if (info.flipX)
	{
		ir.left += ir.width;
		ir.width = -ir.width;
	}
	if (info.flipY)
	{
		ir.top += ir.height;
		ir.height = -ir.height;
	}
	

	replaySprite.setTextureRect(ir);
	replaySprite.setOrigin(info.origin.x, info.origin.y);
	replaySprite.setRotation(info.rotation);
	replaySprite.setPosition(info.position.x, info.position.y);

	cs.setPosition(replaySprite.getPosition());

	//replaySprite.setColor(Color(255, 255, 255, 200));

	++frame;
}

RecordGhost::RecordGhost(Actor *p_player)
	:player(p_player)
{
	numTotalFrames = -1;
	frame = -1;
	header.playerInfo = new GhostHeader::PlayerInfo[1];
	header.playerInfo[0].skinIndex = 0;
}

void RecordGhost::StartRecording()
{
	frame = 0;
	numTotalFrames = -1;
}

void RecordGhost::StopRecording()
{
	//cout << "stop recording: " << frame << endl;
	numTotalFrames = frame;
	frame = -1;
	header.gType = GhostHeader::G_SINGLE_LEVEL_COMPLETE;
	header.numberOfPlayers = 1;
}

void RecordGhost::RecordFrame()
{
	if (frame < 0)
		return;
	//assert( frame >= 0 );
	if (frame >= MAX_RECORD)
		return;

	if (player->action != Actor::SPAWNWAIT)
	{
		SprInfo &info = sprBuffer[frame];
		info.position = player->sprite->getPosition();
		info.origin = player->sprite->getOrigin();
		info.rotation = player->sprite->getRotation();
		info.flipX = player->flipTileX;
		info.flipY = player->flipTileY;
		info.action = (int)player->spriteAction;
		info.tileIndex = player->currTileIndex;
		info.speedLevel = player->speedLevel;

		//cout << "record frame: " << frame << ",action: " << info.action << ", t: " << info.tileIndex << endl;
		++frame;
	}
}

void RecordGhost::WriteToFile(const std::string &fileName)
{
	if (numTotalFrames == 0)
		return;
	assert(numTotalFrames > 0);

	ofstream of;
	of.open(fileName, ios::binary );

	//header
	if (of.is_open())
	{
		header.Write(of);

		of.write((char*)&numTotalFrames, sizeof(int));
		of.write((char*)sprBuffer, numTotalFrames * sizeof(SprInfo));

		of.close();
		/*of << numTotalFrames << endl;
		for (int i = 0; i < numTotalFrames; ++i)
		{
			SprInfo &info = sprBuffer[i];
			of << info.position.x << " " << info.position.y << endl;
			of << info.origin.x << " " << info.origin.y << endl;
			of << info.rotation << endl;
			of << (int)info.flipX << " " << (int)info.flipY << endl;
			of << info.action << " " << info.tileIndex << endl;
			of << info.speedLevel << endl;
		}

		of.close();*/
	}
	else
	{
		cout << "recordghost fileName: " << fileName << endl;
		cout << "RECORD FAILED" << endl;
		//assert(false && "failed to open file to write to");
	}
}

ReplayHeader::ReplayHeader()
	:bUpgradeField(Session::PLAYER_OPTION_BIT_COUNT)
{
	SetVer(1);
}

void ReplayHeader::Read(ifstream &is)
{
	is.read((char*)&ver, sizeof(ver)); //read in the basic vars
	is.read((char*)&startPowerMode, sizeof(startPowerMode));
	bUpgradeField.LoadBinary(is);
}

void ReplayHeader::Write(ofstream &of)
{
	of.write((char*)&ver, sizeof(ver));
	of.write((char*)&startPowerMode, sizeof(startPowerMode));
	bUpgradeField.SaveBinary(of);
}

void ReplayHeader::SetVer(int v )
{
	ver = v;
}


RecordPlayer::RecordPlayer(Actor *p_player)
	:player(p_player)
{
	numTotalFrames = -1;
	frame = -1;
}

void RecordPlayer::StartRecording()
{
	frame = 0;
	numTotalFrames = -1;
	header.bUpgradeField.Set(player->bStartHasUpgradeField);
	header.startPowerMode = player->currPowerMode;
}

void RecordPlayer::StopRecording()
{
	//cout << "stop recording: " << frame << endl;
	numTotalFrames = frame;
	frame = -1;
}

void RecordPlayer::RecordFrame()
{

	if (frame < 0)
		return;
	//assert( frame >= 0 );
	if (frame >= MAX_RECORD)
		return;

	//of << state.leftStickMagnitude << " " << state.leftStickRadians << " " << state.leftTrigger << " "
	//			<< state.rightTrigger 
	//			<< " " // << (int)state.start << " " << (int)state.back << " "
	//			<< (int)state.leftShoulder
	//			<< " " << (int)state.rightShoulder << " " << (int)state.A << " " << (int)state.B << " " 
	//			<< (int)state.X << " " << (int)state.Y << endl;
	ControllerState &s = player->owner->GetCurrInput(0);//currInput;
														//temporary hack^
														//ControllerState &state = inputBuffer[frame];
														//state = ;




	//Buf & b = player->owner->testBuf;

	int compressedInputs = s.GetCompressedState();

	//b.Send(compressedInputs);

	inputBuffer[frame] = compressedInputs;




	/*b.Send(s.leftStickMagnitude);
	b.Send(s.leftStickRadians);
	b.Send(s.leftTrigger);
	b.Send(s.rightTrigger);

	b.Send(s.leftShoulder);
	b.Send(s.rightShoulder);
	b.Send(s.A);
	b.Send(s.B);
	b.Send(s.X);
	b.Send(s.Y);*/

	//cout << "record frame: " << frame << " buttons A: " << (int)s.A << ", B: " << (int)s.B << endl;

	//if( state.A )
	//	cout << "record frame: " << frame << " jump" << endl;
	/*info.position = player->sprite->getPosition();
	info.origin = player->sprite->getOrigin();
	info.rotation = player->sprite->getRotation();
	info.flipX = player->flipTileX;
	info.flipY = player->flipTileY;
	info.action = (int)player->spriteAction;
	info.tileIndex = player->currTileIndex;
	info.speedLevel = player->speedLevel;*/

	//cout << "record frame: " << frame << ",action: " << info.action << ", t: " << info.tileIndex << endl;
	++frame;
}

void RecordPlayer::WriteToFile(const std::string &fileName)
{
	assert(numTotalFrames > 0);

	ofstream of;
	of.open(fileName, ios::binary | ios::out );
	if (of.is_open())
	{
		header.Write(of);

		of.write((char*)&numTotalFrames, sizeof(numTotalFrames));
		of.write((char*)inputBuffer, numTotalFrames * sizeof(int));

		of.close();
	}
	else
	{
		cout << "recordghost fileName: " << fileName << endl;
		assert(false && "failed to open file to write to");
	}
}

ReplayPlayer::ReplayPlayer(Actor *p_player)
	:player(p_player), inputBuffer(NULL)
{
	frame = 0;
	init = false;
}

ReplayPlayer::~ReplayPlayer()
{
	delete[] inputBuffer;
}

bool ReplayPlayer::OpenReplay(const std::string &fileName)
{
	ifstream is;

	is.open(fileName, ios::binary | ios::in);
	if (is.is_open())
	{
		init = true;

		header.Read(is);

		is.read((char*)&numTotalFrames, sizeof(numTotalFrames));
		cout << "reading num frames: " << numTotalFrames << endl;

		inputBuffer = new int[numTotalFrames];

		is.read((char*)inputBuffer, numTotalFrames * sizeof(int));

		is.close();
		return true;
	}
	//return false on failure
	return false;

}

void ReplayPlayer::Reset()
{
	frame = 0;
	player->SetAllUpgrades(header.bUpgradeField);
	player->currPowerMode = header.startPowerMode;
}

void ReplayPlayer::UpdateInput(ControllerDualStateQueue *controllerInput)
{
	if (!init || frame == numTotalFrames)
		return;

	bool start = controllerInput->GetCurrState().start;
	bool back = controllerInput->GetCurrState().back;

	controllerInput->states[0].SetFromCompressedState(inputBuffer[frame]);
	//state.SetFromCompressedState(inputBuffer[frame]);

	controllerInput->states[0].start = start;
	controllerInput->states[0].back = back;

	++frame;
}

RecordGhostMenu::RecordGhostMenu(MainMenu *p_mainMenu, sf::Vector2f &p_pos)
	:mainMenu(p_mainMenu), font(p_mainMenu->arial), topIndex(0),
	oldCurrIndex(0)
{
	//allItems = NULL;
	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 0, 0);

	//Vector2f menuOffset(-1920, 0);
	topMid = p_pos;//p_pos + Vector2f(BOX_WIDTH / 2, 0) + menuOffset;

	SetupBoxes();

	for (int i = 0; i < NUM_BOXES; ++i)
	{
		itemName[i].setFont(font);
		itemName[i].setCharacterSize(40);
		itemName[i].setFillColor(Color::White);
	}

	LoadItems();

	UpdateItemText();

	UICheckbox *check = new UICheckbox(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, "testcheckbox", 300, 50);
	UICheckbox *check1 = new UICheckbox(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, "testcheckbox", 300, 50);
	UICheckbox *check2 = new UICheckbox(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, "testcheckbox", 300, 50);

	UIControl *testBlah[] = { check, check1, check2 };

	ghostOptions = new UIVerticalControlList(NULL, sizeof(testBlah) / sizeof(UIControl*), testBlah, 20);
	//Vector2f tLeft = Vector2f(600, 120) + menuOffset;
	//ghostOptions->SetTopLeft(tLeft.x, tLeft.y);

	//autoFolder = new GhostFolder("auto");
	//saveFolder = new GhostFolder("save");

	//folders.push_back(autoFolder);
	//folders.push_back(saveFolder);
}

RecordGhostMenu::~RecordGhostMenu()
{
	for (auto it = ghostFolders.begin(); it != ghostFolders.end(); ++it)
	{
		delete (*it).second;
	}

	delete saSelector;
	delete ghostOptions;
}

void RecordGhostMenu::SetupBoxes()
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

#include <boost/filesystem.hpp>
using namespace boost::filesystem;

void RecordGhostMenu::LoadDir(boost::filesystem::path &p, list<GhostEntry*> &entries )
{
	for (auto it = entries.begin(); it != entries.end(); ++it)
	{
		delete (*it);
	}
	entries.clear();

	try
	{
		if (exists(p))    // does p actually exist?
		{
			assert(is_directory(p));
			vector<path> v;

			copy(directory_iterator(p), directory_iterator(), back_inserter(v));

			sort(v.begin(), v.end());

			for (vector<path>::iterator it(v.begin()); it != v.end(); ++it)
			{
				if (is_regular_file((*it)))        // is p a regular file?   
				{
					if ((*it).extension().string() == ".bghst")
					{
						//items.push_back(p);
						//entries.push_back( )
						ifstream is;
						is.open((*it).string(), std::ios::binary );
						assert(is.is_open());

						GhostEntry *ge = new GhostEntry((*it), ReadGhostHeader(is));

						entries.push_back(ge);
					}
				}
			}
		}
		else
		{
			//folder doesn't exist
			create_directory(p);
		}
	}
	catch (const filesystem_error& ex)
	{
		cout << ex.what() << '\n';
		assert(0);
	}
}

void RecordGhostMenu::LoadFolders()
{
	path pFolder(current_path() / "/Resources/Recordings/Ghost/");

	ghostFolders.clear();
	try
	{
		if (exists(pFolder))    // does p actually exist?
		{
			assert(is_directory(pFolder));
			vector<path> v;

			copy(directory_iterator(pFolder), directory_iterator(), back_inserter(v));

			sort(v.begin(), v.end());

			for (vector<path>::iterator it(v.begin()); it != v.end(); ++it)
			{
				if ( is_directory((*it) ) )
				{
					ghostFolders[(*it).filename().stem().string()] = new GhostFolder((*it));
				}
			}
		}
		else
		{
			//folder doesn't exist
			create_directory(pFolder);
		}
	}
	catch (const filesystem_error& ex)
	{
		cout << ex.what() << '\n';
		assert(0);
	}
}

void RecordGhostMenu::SetupInds( int &ind, GhostFolder *gf )
{
	//GhostIndexInfo gi;
	//gi.folder = gf;
	//gi.entry = NULL;

	//allItems[ind] = pair<string, GhostIndexInfo>(gf->folderName, gi);
	//++ind;
	////assert(gf->.size() > 0);
	//for (auto it2 = gf->ghosts.begin(); it2 != gf->ghosts.end(); ++it2)
	//{
	//	gi.folder = NULL;
	//	gi.entry = (*it2);

	//	allItems[ind] = pair<string, GhostIndexInfo>((*it2)->gPath.filename().stem().string(), gi);
	//	++ind;
	//}
}

void RecordGhostMenu::UpdateLoadedFolders()
{
	for (auto it = ghostFolders.begin(); it != ghostFolders.end(); ++it)
	{
		if ((*it).second->autoLoaded)
		{
			LoadMapGhosts(false, (*it).second->folderPath);
		}
		if ((*it).second->saveLoaded)
		{
			for (auto lit = (*it).second->autoGhosts.begin(); lit != (*it).second->autoGhosts.end(); ++lit)
			{
				delete (*lit);
			}
			(*it).second->autoGhosts.clear();

			LoadMapGhosts(true, (*it).second->folderPath);
		}
	}

	UpdateItemText();
}

void RecordGhostMenu::LoadMapGhosts( bool save, const boost::filesystem::path &p) //save compared to auto
{
	string mapName = p.filename().stem().string();
	GhostFolder *gf = NULL;
	gf = ghostFolders[mapName];
	assert(gf != NULL);
	if (save)
	{
		path pSave(current_path() / "/Resources/Recordings/Ghost/" / mapName / "/save/");
		LoadDir(pSave, gf->saveGhosts);
	}
	else
	{
		path pAuto(current_path() / "/Resources/Recordings/Ghost/" / mapName / "/auto/");
		LoadDir(pAuto, gf->autoGhosts);
	}
}

void RecordGhostMenu::LoadItems()
{
	LoadFolders();

	sortedFolders.clear();
	for (auto it = ghostFolders.begin(); it != ghostFolders.end(); ++it)
	{
		sortedFolders.push_back((*it).second);
	}
	
	//numTotalItems = 0;


	////numTotalItems += autoFolder->ghosts.size() + 1;
	////numTotalItems += saveFolder->ghosts.size() + 1;

	//if (allItems != NULL)
	//{
	//	delete[] allItems;
	//}

	//allItems = new pair<string, GhostIndexInfo>[numTotalItems];

	//int ind = 0;

	//SetupInds(ind, autoFolder);
	//SetupInds(ind, saveFolder);
}

GhostHeader * RecordGhostMenu::ReadGhostHeader(std::ifstream &is)
{
	GhostHeader *gh = new GhostHeader;
	//memset(gh, 0, sizeof(GhostHeader));

	assert(is.is_open());

	/*if (!is.read((char*)gh, sizeof(GhostHeader)))
	{
		assert(0);
	}*/

	return gh;
}

bool RecordGhostMenu::WriteGhostHeader(std::ofstream &of, GhostHeader *gh)
{
	of.write((char*)gh, sizeof(gh));
	return true;
}

bool RecordGhostMenu::ReplaceGhostHeader(boost::filesystem::path &p, GhostHeader *gh)
{
	ifstream is;
	is.open(p.string());

	path from("tempghost.tmp");
	ofstream of;
	of.open(from.string());
	assert(of.is_open());
	WriteGhostHeader(of, gh);

	if (is.is_open())
	{
		GhostHeader *oldHeader = ReadGhostHeader(is);

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

void RecordGhostMenu::Update(ControllerState &currInput, ControllerState &prevInput)
{	
	 //= mainMenu->menuCurrInput;
	// = mainMenu->menuPrevInput;

	if (currInput.B && !prevInput.B)
	{
		return;
	}
	else if (currInput.X && !prevInput.X)
	{
		return;
	}
	else if (currInput.Y && !prevInput.Y)
	{
	}

	int cIndex = saSelector->currIndex;

	GhostIndexInfo inf = GetIndexInfo(cIndex);

	if (currInput.A && !prevInput.A)
	{
		GhostFolder *gf = inf.folder;
		if (gf != NULL)
		{
			if (inf.onAuto)
			{
				if (!gf->autoExpanded && !gf->autoLoaded )
				{
					LoadMapGhosts(false, gf->folderPath);
					gf->autoLoaded = true;
				}
				if (gf->autoGhosts.size() > 0)
				{
					gf->autoExpanded = !gf->autoExpanded;
				}
			}
			else if (inf.onSave)
			{
				if (!gf->saveExpanded && !gf->saveLoaded )
				{
					LoadMapGhosts(true, gf->folderPath);
					gf->saveLoaded = true;
				}

				if (gf->saveGhosts.size() > 0)
				{
					gf->saveExpanded = !gf->saveExpanded;	
				}
			}
			else
			{
				gf->expanded = !gf->expanded;
			}
			
			UpdateItemText();
		}
		else
		{
			GhostEntry *entry = inf.entry;
			
			entry->activeForMap = !entry->activeForMap;

			//add it to the list of active ones!
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
		}
	}


	UpdateBoxesDebug();
}

void RecordGhostMenu::MoveUp()
{
	topIndex++;
	if (topIndex == items.size())
	{
		topIndex = 0;
	}
}

void RecordGhostMenu::MoveDown()
{
	topIndex--;
	if (topIndex == -1)
	{
		topIndex = items.size() - 1;
	}
}

GhostIndexInfo RecordGhostMenu::GetIndexInfo(int index)
{
	int checkerIndex = 0;
	GhostIndexInfo info;
	info.entry = NULL;
	info.folder = NULL;
	info.onAuto = false;
	info.onSave = false;
	
	for (auto it = sortedFolders.begin(); it != sortedFolders.end(); ++it)
	{
		if (checkerIndex == index)
		{
			info.folder = (*it);
			return info;
		}

		GhostFolder *gf = (*it);
		++checkerIndex;

		if (gf->expanded)
		{
			//checkerIndex += 2; //save and auto folders
			if (checkerIndex == index)
			{
				info.folder = (*it);
				info.onAuto = true;
				return info;
			}
			++checkerIndex;

			if (gf->autoExpanded)
			{
				for (auto ai = gf->autoGhosts.begin(); ai != gf->autoGhosts.end(); ++ai )
				{
					if (checkerIndex == index)
					{
						info.entry = (*ai);
						return info;
					}
					++checkerIndex;
				}
			}

			if (checkerIndex == index)
			{
				info.folder = (*it);
				info.onSave = true;
				return info;
			}
			++checkerIndex;

			if (gf->saveExpanded)
			{
				for (auto ai = gf->saveGhosts.begin(); ai != gf->saveGhosts.end(); ++ai)
				{
					if (checkerIndex == index)
					{
						info.entry = (*ai);
						return info;
					}
					++checkerIndex;
				}
			}
		}
	}

	assert(0);

	return info;
}

void RecordGhostMenu::UpdateItemText()
{
	int totalShownItems = 0;

	for (auto it = sortedFolders.begin(); it != sortedFolders.end(); ++it)
	{
		GhostFolder *gf = (*it);
		++totalShownItems;
		if (gf->expanded)
		{
			totalShownItems += 2; //save and auto folders
			if (gf->autoExpanded)
			{
				totalShownItems += gf->autoGhosts.size();
			}
			if (gf->saveExpanded)
			{
				totalShownItems += gf->saveGhosts.size();
			}
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


		GhostIndexInfo inf = GetIndexInfo(ind);

		itemName[i].setString(inf.GetName());
		itemName[i].setOrigin(0, 0);

		int xVal = topMid.x - BOX_WIDTH / 2;
		if (inf.folder == NULL) //its just a file
		{
			GhostEntry *ge = inf.entry;

			xVal += 80;
		}
		else
		{
			if (inf.onAuto || inf.onSave)
			{
				xVal += 40;
			}

			xVal += 10;
		}
		itemName[i].setPosition(xVal, topMid.y + (BOX_HEIGHT + BOX_SPACING) * i);

		++ind;
	}
}


void RecordGhostMenu::UpdateBoxesDebug()
{
	Color c;
	int trueI = (saSelector->currIndex - topIndex);



	for (int i = 0; i < NUM_BOXES; ++i)
	{
		if (topIndex + i < saSelector->totalItems)
		{
			GhostIndexInfo inf = GetIndexInfo(topIndex + i);

			if (i == trueI)
			{
				if (inf.entry != NULL && inf.entry->activeForMap)
				{
					c = Color::Magenta;
				}
				else
				{
					if (inf.folder != NULL 
						&& (
						( inf.onAuto && inf.folder->IsAutoActive())
						|| ( inf.onSave && inf.folder->IsSaveActive() ) 
						|| ( !inf.onAuto && !inf.onSave && inf.folder->IsActive() ) 
							) 
						)
					{
						c = Color::Green;
					}
					else
					{
						c = Color::Blue;
					}
					
				}
			}
			else
			{
				if (inf.entry != NULL && inf.entry->activeForMap)
				{
					c = Color::Green;
				}
				else
				{
					if (inf.folder != NULL
						&& (
						(inf.onAuto && inf.folder->IsAutoActive())
							|| (inf.onSave && inf.folder->IsSaveActive())
							|| (!inf.onAuto && !inf.onSave && inf.folder->IsActive())
							)
						)
					{
						c = Color::Cyan;
					}
					else
					{
						c = Color::Red;
					}
				}
			}
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

void RecordGhostMenu::Draw(sf::RenderTarget *target)
{
	target->draw(boxes, NUM_BOXES * 4, sf::Quads);
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		target->draw(itemName[i]);
	}
}

void RecordGhostMenu::GetActiveList(std::list<GhostEntry*> &entries)
{
	assert(entries.empty());
	for (auto it = sortedFolders.begin(); it != sortedFolders.end(); ++it)
	{
		if ((*it)->IsAutoActive())
		{
			for (auto ait = (*it)->autoGhosts.begin(); ait != (*it)->autoGhosts.end(); ++ait)
			{
				if ((*ait)->activeForMap)
				{
					entries.push_back((*ait));
				}
			}
		}
		if ((*it)->IsSaveActive())
		{
			for (auto sit = (*it)->autoGhosts.begin(); sit != (*it)->autoGhosts.end(); ++sit)
			{
				if ((*sit)->activeForMap)
				{
					entries.push_back((*sit));
				}
			}
		}
	}
}

std::string GhostIndexInfo::GetName()
{
	if (folder == NULL)
	{
		assert(entry != NULL);
		return entry->gPath.filename().stem().string();
	}
	else if (entry == NULL)
	{
		assert(folder != NULL);
		if (onAuto)
		{
			return "auto";
		}
		else if (onSave)
		{
			return "save";
		}
		else
		{
			return folder->folderPath.filename().stem().string();
		}
	}

	assert(0);
}

GhostFolder::~GhostFolder()
{
	DestroyAutoGhosts();
	DestroySaveGhosts();
//	for( auto it = autoGhosts.begin(); it != autoGhosts.end();)
}

void GhostFolder::DestroyAutoGhosts()
{
	for (auto it = autoGhosts.begin(); it != autoGhosts.end(); ++it)
	{
		delete (*it);
	}
	autoGhosts.clear();
}

void GhostFolder::DestroySaveGhosts()
{
	for (auto it = saveGhosts.begin(); it != saveGhosts.end(); ++it)
	{
		delete (*it);
	}
	saveGhosts.clear();
}

bool GhostFolder::IsSaveActive()
{
	for (auto it = saveGhosts.begin(); it != saveGhosts.end(); ++it)
	{
		if ((*it)->activeForMap)
		{
			return true;
		}
	}

	return false;
}

bool GhostFolder::IsAutoActive()
{
	for (auto it = autoGhosts.begin(); it != autoGhosts.end(); ++it)
	{
		if ((*it)->activeForMap)
		{
			return true;
		}
	}

	return false;
}

GhostHeader::GhostHeader()
	:playerInfo(NULL), numberOfPlayers(0)
{
	SetVer(1);
}

GhostHeader::~GhostHeader()
{
	if (playerInfo != NULL)
	{
		delete[] playerInfo;
	}
}

void GhostHeader::SetVer(int v )
{
	ver = v;
}

void GhostHeader::Read(std::ifstream &is)
{
	assert(playerInfo == NULL);

	is.read((char*)&ver, sizeof(ver)); //read in the basic vars
	is.read((char*)&numberOfPlayers, sizeof(numberOfPlayers));
	assert(numberOfPlayers > 0 && numberOfPlayers < 5);
	playerInfo = new PlayerInfo[numberOfPlayers];
	is.read((char*)playerInfo, sizeof(PlayerInfo) * numberOfPlayers);
}

void GhostHeader::Write(std::ofstream &of)
{
	of.write((char*)&ver, sizeof(ver));
	of.write((char*)&numberOfPlayers, sizeof(numberOfPlayers));
	for (int i = 0; i < numberOfPlayers; ++i)
	{
		playerInfo[i].Write(of);
	}
}

void GhostHeader::PlayerInfo::Write(std::ofstream &of)
{
	of.write((char*)&skinIndex, sizeof(int));
}