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

const int RecordGhostMenu::BOX_WIDTH = 300;
const int RecordGhostMenu::BOX_HEIGHT = 100;
const int RecordGhostMenu::BOX_SPACING = 10;

ReplayGhost::ReplayGhost(Actor *p_player)
	:player(p_player), sprBuffer(NULL)
{
	frame = 0;
	init = false;
}

void ReplayGhost::Draw(RenderTarget *target)
{
	if (!init)
		return;

	if (frame >= 0 && frame < numTotalFrames)
		target->draw(replaySprite);
}

bool ReplayGhost::OpenGhost(const std::string &fileName)
{
	ifstream is;

	is.open(fileName);
	if (is.is_open())
	{
		is >> numTotalFrames;
		sprBuffer = new SprInfo[numTotalFrames];

		init = true;

		for (int i = 0; i < numTotalFrames; ++i)
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

		}

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

	SprInfo &info = sprBuffer[frame];
	Tileset *ts = player->tileset[(Actor::Action)info.action];
	replaySprite.setTexture(*ts->texture);

	Actor::Action a = (Actor::Action)info.action;
	if (a == Actor::JUMPSQUAT)
	{
		cout << "setting jumpsquat in ghost. frame: " << frame << endl;
	}
	IntRect ir = ts->GetSubRect(info.tileIndex);

	replaySprite.setRotation(info.rotation);

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

	replaySprite.setColor(Color(255, 255, 255, 200));

	++frame;
}

RecordGhost::RecordGhost(Actor *p_player)
	:player(p_player)
{
	numTotalFrames = -1;
	frame = -1;
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
}

void RecordGhost::RecordFrame()
{
	if (frame < 0)
		return;
	//assert( frame >= 0 );
	if (frame >= MAX_RECORD)
		return;

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

void RecordGhost::WriteToFile(const std::string &fileName)
{
	assert(numTotalFrames > 0);

	ofstream of;
	of.open(fileName);
	if (of.is_open())
	{
		of << numTotalFrames << endl;
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

		of.close();
	}
	else
	{
		cout << "recordghost fileName: " << fileName << endl;
		assert(false && "failed to open file to write to");
	}
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

	Buf & b = player->owner->testBuf;

	//b.Send( player->owner->totalGameFrames );
	b.Send(s.leftStickMagnitude);
	b.Send(s.leftStickRadians);
	b.Send(s.leftTrigger);
	b.Send(s.rightTrigger);

	b.Send(s.leftShoulder);
	b.Send(s.rightShoulder);
	b.Send(s.A);
	b.Send(s.B);
	b.Send(s.X);
	b.Send(s.Y);

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
	return;
	assert(numTotalFrames > 0);

	ofstream of;
	of.open(fileName);
	if (of.is_open())
	{
		of << numTotalFrames << endl;
		for (int i = 0; i < numTotalFrames; ++i)
		{
			ControllerState &state = inputBuffer[i];
			of << state.leftStickMagnitude << " " << state.leftStickRadians << " " << state.leftTrigger << " "
				<< state.rightTrigger
				<< " " // << (int)state.start << " " << (int)state.back << " "
				<< (int)state.leftShoulder
				<< " " << (int)state.rightShoulder << " " << (int)state.A << " " << (int)state.B << " "
				<< (int)state.X << " " << (int)state.Y << endl;
			/*SprInfo &info = sprBuffer[i];
			of << info.position.x << " " << info.position.y << endl;
			of << info.origin.x << " " << info.origin.y << endl;
			of << info.rotation << endl;
			of << (int)info.flipX << " " << (int)info.flipY << endl;
			of << info.action << " " << info.tileIndex << endl;
			of << info.speedLevel << endl;*/
		}

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

bool ReplayPlayer::OpenReplay(const std::string &fileName)
{
	//ifstream is;

	//is.open( fileName );
	//if( is.is_open() )
	//{
	//	init = true;

	//	is >> numTotalFrames;
	//	inputBuffer = new ControllerState[numTotalFrames];

	//	for( int i = 0; i < numTotalFrames; ++i )
	//	{
	//		ControllerState &state = inputBuffer[i];
	//		
	//		is >> state.leftStickMagnitude;
	//		is >> state.leftStickRadians;
	//		is >> state.leftTrigger;
	//		is >> state.rightTrigger;
	//		int start, back, leftShoulder, rightShoulder, A,B,X,Y;
	//		//is >> start;
	//		//is >> back;
	//		is >> leftShoulder;
	//		is >> rightShoulder;
	//		is >> A >> B >> X >> Y;

	//		//state.start = start;
	//		//state.back = back;
	//		state.leftShoulder = leftShoulder;
	//		state.rightShoulder = rightShoulder;
	//		state.A = A;
	//		state.B = B;
	//		state.X = X;
	//		state.Y = Y;
	//		/*SprInfo &info = sprBuffer[i];
	//		is >> info.position.x;
	//		is >> info.position.y;

	//		is >> info.origin.x;
	//		is >> info.origin.y;

	//		is >> info.rotation;

	//		int fx;
	//		int fy;

	//		is >> fx;
	//		is >> fy;

	//		info.flipX = (bool)fx;
	//		info.flipY = (bool)fy;

	//		is >> info.action;
	//		is >> info.tileIndex;

	//		is >> info.speedLevel;*/
	//	}

	//	is.close();

	//	frame = 0;
	//	return true;
	//}
	////return false on failure
	//return false;

	ifstream is;

	is.open(fileName, ios::binary | ios::in);
	if (is.is_open())
	{
		init = true;

		is.read((char*)&numTotalFrames, sizeof(numTotalFrames));
		cout << "reading num frames: " << numTotalFrames << endl;

		inputBuffer = new ControllerState[numTotalFrames];

		for (int i = 0; i < numTotalFrames; ++i)
		{
			ControllerState &state = inputBuffer[i];

			//int gameFrames = -1;

			//assert( !is.eof() );
			//is.read( (char*)&gameFrames, sizeof( int ) );


			//cout << "testing replay " << i << " : " << gameFrames << endl;
			is.read((char*)&state.leftStickMagnitude, sizeof(state.leftStickMagnitude));
			is.read((char*)&state.leftStickRadians, sizeof(state.leftStickRadians));
			is.read((char*)&state.leftTrigger, sizeof(state.leftTrigger));
			is.read((char*)&state.rightTrigger, sizeof(state.rightTrigger));

			int start, back, leftShoulder, rightShoulder, A, B, X, Y;

			is.read((char*)&state.leftShoulder, sizeof(bool));
			is.read((char*)&state.rightShoulder, sizeof(bool));
			is.read((char*)&state.A, sizeof(bool));
			is.read((char*)&state.B, sizeof(bool));
			is.read((char*)&state.X, sizeof(bool));
			is.read((char*)&state.Y, sizeof(bool));

			//cout << "replay frame: " << i << " buttons A: " << (int)state.A << ", B: " << (int)state.B << endl;
			//cout << "replay frame: " << i << " buttons A: " << (int)state.A << ", B: " << (int)state.B << endl;
			//cout << "replay frame: " << i << ", leftstickmag: " << state.leftStickMagnitude << endl;
			//is >> start;
			//is >> back;
			//is >> leftShoulder;
			//is >> rightShoulder;
			//is >> A >> B >> X >> Y;

			////state.start = start;
			////state.back = back;
			//state.leftShoulder = leftShoulder;
			//state.rightShoulder = rightShoulder;
			//state.A = A;
			//state.B = B;
			//state.X = X;
			//state.Y = Y;
		}

		is.close();

		frame = 0;
		return true;
	}
	//return false on failure
	return false;

}

void ReplayPlayer::UpdateInput(ControllerState &state)
{
	if (!init || frame == numTotalFrames)
		return;

	ControllerState &cs = inputBuffer[frame];

	state.leftStickMagnitude = cs.leftStickMagnitude;
	state.leftStickRadians = cs.leftStickRadians;

	float stickThresh = GameController::stickThresh;
	state.leftStickPad = 0;
	if (state.leftStickMagnitude > stickThresh)
	{
		//cout << "left stick radians: " << currInput.leftStickRadians << endl;
		float x = cos(state.leftStickRadians);
		float y = sin(state.leftStickRadians);

		if (x > stickThresh)
			state.leftStickPad += 1 << 3;
		if (x < -stickThresh)
			state.leftStickPad += 1 << 2;
		if (y > stickThresh)
			state.leftStickPad += 1;
		if (y < -stickThresh)
			state.leftStickPad += 1 << 1;
	}

	state.leftTrigger = cs.leftTrigger;
	state.rightTrigger = cs.rightTrigger;
	//state.start = cs.start;
	//state.back = cs.back;
	state.leftShoulder = cs.leftShoulder;
	state.rightShoulder = cs.rightShoulder;
	state.A = cs.A;
	state.B = cs.B;
	state.X = cs.X;
	state.Y = cs.Y;

	if (state.A)
		cout << "replay frame: " << frame << " pressing A" << endl;

	/*SprInfo &info = sprBuffer[frame];
	Tileset *ts = player->tileset[(Actor::Action)info.action];
	replaySprite.setTexture( *ts->texture );

	IntRect ir = ts->GetSubRect( info.tileIndex );

	replaySprite.setRotation( info.rotation );

	if( info.flipX )
	{
	ir.left += ir.width;
	ir.width = -ir.width;
	}
	if( info.flipY )
	{
	ir.top += ir.height;
	ir.height = -ir.height;
	}

	replaySprite.setTextureRect( ir );
	replaySprite.setOrigin( info.origin.x, info.origin.y );
	replaySprite.setRotation( info.rotation );
	replaySprite.setPosition( info.position.x, info.position.y );

	replaySprite.setColor( Color( 255, 255, 255, 150 ) );*/

	++frame;
}

RecordGhostMenu::RecordGhostMenu(MainMenu *p_mainMenu, sf::Vector2f &p_pos)
	:mainMenu(p_mainMenu), font(p_mainMenu->arial), topIndex(0),
	oldCurrIndex(0)
{
	allItems = NULL;
	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 0, 0);

	Vector2f menuOffset(-1920, 0);
	topMid = p_pos + Vector2f(BOX_WIDTH / 2, 0) + menuOffset;

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
	Vector2f tLeft = Vector2f(600, 120) + menuOffset;
	ghostOptions->SetTopLeft(tLeft.x, tLeft.y);

	autoFolder = new GhostFolder("auto");
	saveFolder = new GhostFolder("save");

	folders.push_back(autoFolder);
	folders.push_back(saveFolder);
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
						is.open(p.string(), std::ios::binary );
						assert(is.is_open());

						GhostEntry *ge = new GhostEntry((*it), ReadGhostHeader(is));
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
	path pFolder(current_path() / "/Recordings/Ghost/");

	ghostFolderPaths.clear();
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
					ghostFolderPaths.push_back((*it));
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

void RecordGhostMenu::SetupInds( int &ind, GhostFolder *gf )
{
	GhostIndexInfo gi;
	gi.folder = gf;
	gi.entry = NULL;

	allItems[ind] = pair<string, GhostIndexInfo>(gf->folderName, gi);
	++ind;
	//assert(gf->.size() > 0);
	for (auto it2 = gf->ghosts.begin(); it2 != gf->ghosts.end(); ++it2)
	{
		gi.folder = NULL;
		gi.entry = (*it2);

		allItems[ind] = pair<string, GhostIndexInfo>((*it2)->gPath.filename().stem().string(), gi);
		++ind;
	}
}

void RecordGhostMenu::LoadMapReplays(const boost::filesystem::path &p)
{

}

void RecordGhostMenu::LoadItems()
{
	LoadFolders();
	path pAuto(current_path() / "/Recordings/Ghost/auto");
	path pSave(current_path() / "/Recordings/Ghost/save");
	LoadDir(pAuto, autoFolder->ghosts);
	LoadDir(pSave, saveFolder->ghosts );

	numTotalItems = 0;

	numTotalItems += autoFolder->ghosts.size() + 1;
	numTotalItems += saveFolder->ghosts.size() + 1;

	if (allItems != NULL)
	{
		delete[] allItems;
	}

	allItems = new pair<string, GhostIndexInfo>[numTotalItems];

	int ind = 0;

	SetupInds(ind, autoFolder);
	SetupInds(ind, saveFolder);
}

GhostHeader * RecordGhostMenu::ReadGhostHeader(std::ifstream &is)
{
	GhostHeader *gh = new GhostHeader;
	memset(gh, 0, sizeof(GhostHeader));

	assert(is.is_open());

	if (!is.read((char*)gh, sizeof(GhostHeader)))
	{
		assert(0);
	}

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

void RecordGhostMenu::Update(ControllerState &currInput,
	ControllerState &prevInput)
{	
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
	int pIndex = GetPairIndex(cIndex);
	if (currInput.A && !prevInput.A)
	{
		GhostFolder *gf = allItems[pIndex].second.folder;
		if (gf != NULL)
		{
			gf->expanded = !gf->expanded;
			UpdateItemText();
		}
		else
		{
			GhostEntry *entry = allItems[pIndex].second.entry;
			
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

			int pIndex = GetPairIndex(cIndex);
			//if (allItems[pIndex].second.info == NULL)
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

void RecordGhostMenu::UpdateItemText()
{
	int totalShownItems = 0;

	for (auto it = folders.begin(); it != folders.end(); ++it)
	{
		++totalShownItems;
		if ((*it)->expanded)
		{
			totalShownItems += (*it)->ghosts.size();
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

		pair<string, GhostIndexInfo> &p = allItems[GetPairIndex(ind)];
		string printStr = p.first;
		itemName[i].setString(printStr);
		itemName[i].setOrigin(0, 0);

		int xVal = topMid.x - BOX_WIDTH / 2;
		if (p.second.folder == NULL) //its just a file
		{
			GhostEntry *ge = p.second.entry;

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

int RecordGhostMenu::GetPairIndex(int index)
{
	int i = 0;
	for (int it = 0; it < index; ++it)
	{
		if (allItems[i].second.folder != NULL && !allItems[i].second.folder->expanded)
		{
			i += allItems[i].second.folder->ghosts.size();
		}

		++i;
	}

	return i;
}

void RecordGhostMenu::UpdateBoxesDebug()
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

void RecordGhostMenu::Draw(sf::RenderTarget *target)
{
	target->draw(boxes, NUM_BOXES * 4, sf::Quads);
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		target->draw(itemName[i]);
	}
}