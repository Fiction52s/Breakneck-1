#include "MusicSelector.h"
#include "SoundManager.h"
#include <sstream>
#include <assert.h>
#include <iostream>
#include "UIWindow.h"
#include "MainMenu.h"


using namespace sf;
using namespace std;

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

		musicNames[i].setString((*lit)->name);
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