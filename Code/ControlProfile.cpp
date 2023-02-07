#include "ControlProfile.h"
#include <sstream>
#include <assert.h>
#include <iostream>
#include "MainMenu.h"
#include "VectorMath.h"

using namespace sf;
using namespace std;

#define PROFILE_START_CHAR '<'
#define PROFILE_END_CHAR '>'
#define INPUT_TYPE_START_CHAR '{'
#define INPUT_TYPE_END_CHAR '}'
#define INPUT_TYPE_XBOX "XBOX"
#define INPUT_TYPE_KEYBOARD "KEYBOARD"
#define INPUT_TYPE_GAMECUBE "GAMECUBE"
#define COLON ':'

//const int ControlProfileMenu::NUM_BOXES = 8;
const int ControlProfileMenu::BOX_WIDTH = 300;
const int ControlProfileMenu::BOX_HEIGHT = 40;//50; //40
const int ControlProfileMenu::BOX_SPACING = 0;//10;

//waitFrames[0] = 10;
//waitFrames[1] = 5;
//waitFrames[2] = 2;

//waitModeThresh[0] = 2;
//waitModeThresh[1] = 2;

ControlProfile::ControlProfile()
{
	SetControllerType(CTYPE_XBOX);
	SetFilterDefault();
	editable = true;
}

//void ControlProfile::SetFilterKeyboardMenuDefault()
//{
//	for (int i = 0; i < ControllerSettings::BUTTONTYPE_Count; ++i)
//	{
//		filter[i] = XBOX_BLANK;
//	}
//
//	filter[ControllerSettings::BUTTONTYPE_JUMP] = Keyboard::Key::Z;
//	filter[ControllerSettings::BUTTONTYPE_DASH] = Keyboard::Key::X;
//	filter[ControllerSettings::BUTTONTYPE_ATTACK] = Keyboard::Key::C;
//	filter[ControllerSettings::BUTTONTYPE_SHIELD] = Keyboard::Key::V;
//	filter[ControllerSettings::BUTTONTYPE_SPECIAL] = Keyboard::Key::B;
//	filter[ControllerSettings::BUTTONTYPE_LEFTWIRE] = Keyboard::Key::LControl;
//	filter[ControllerSettings::BUTTONTYPE_RIGHTWIRE] = Keyboard::Key::Space;
//
//	filter[ControllerSettings::BUTTONTYPE_PAUSE] = Keyboard::Key::Delete;
//
//	filter[ControllerSettings::BUTTONTYPE_LLEFT] = Keyboard::Key::Left;
//	filter[ControllerSettings::BUTTONTYPE_LRIGHT] = Keyboard::Key::Right;
//	filter[ControllerSettings::BUTTONTYPE_LUP] = Keyboard::Key::Up;
//	filter[ControllerSettings::BUTTONTYPE_LDOWN] = Keyboard::Key::Down;
//
//	filter[ControllerSettings::BUTTONTYPE_RLEFT] = Keyboard::Key::U;
//	filter[ControllerSettings::BUTTONTYPE_RRIGHT] = Keyboard::Key::O;
//	filter[ControllerSettings::BUTTONTYPE_RUP] = Keyboard::Key::P;
//	filter[ControllerSettings::BUTTONTYPE_RDOWN] = Keyboard::Key::SemiColon;
//}

void ControlProfile::SetFilterDefault()
{
	for (int i = 0; i < ControllerSettings::BUTTONTYPE_Count; ++i)
	{
		filter[i] = XBOX_BLANK;
	}

	switch (cType)
	{
	case CTYPE_XBOX:
	{
		filter[ControllerSettings::BUTTONTYPE_JUMP] = XBOX_A;
		filter[ControllerSettings::BUTTONTYPE_DASH] = XBOX_X;
		filter[ControllerSettings::BUTTONTYPE_ATTACK] = XBOX_R1;
		filter[ControllerSettings::BUTTONTYPE_SHIELD] = XBOX_L1;
		filter[ControllerSettings::BUTTONTYPE_SPECIAL] = XBOX_B;
		filter[ControllerSettings::BUTTONTYPE_LEFTWIRE] = XBOX_L2;
		filter[ControllerSettings::BUTTONTYPE_RIGHTWIRE] = XBOX_R2;

		filter[ControllerSettings::BUTTONTYPE_PAUSE] = XBOX_START;
		break;
	}
	case CTYPE_GAMECUBE:
	{
		filter[ControllerSettings::BUTTONTYPE_JUMP] = XBOX_Y;
		filter[ControllerSettings::BUTTONTYPE_DASH] = XBOX_X;
		filter[ControllerSettings::BUTTONTYPE_ATTACK] = XBOX_A;
		filter[ControllerSettings::BUTTONTYPE_SHIELD] = XBOX_R1;
		filter[ControllerSettings::BUTTONTYPE_SPECIAL] = XBOX_B;
		filter[ControllerSettings::BUTTONTYPE_LEFTWIRE] = XBOX_L2;
		filter[ControllerSettings::BUTTONTYPE_RIGHTWIRE] = XBOX_R2;

		filter[ControllerSettings::BUTTONTYPE_PAUSE] = XBOX_START;
		break;
	}
	case CTYPE_KEYBOARD:
	{
		filter[ControllerSettings::BUTTONTYPE_JUMP] = Keyboard::Key::Z;
		filter[ControllerSettings::BUTTONTYPE_DASH] = Keyboard::Key::X;
		filter[ControllerSettings::BUTTONTYPE_ATTACK] = Keyboard::Key::C;
		filter[ControllerSettings::BUTTONTYPE_SHIELD] = Keyboard::Key::V;
		filter[ControllerSettings::BUTTONTYPE_SPECIAL] = Keyboard::Key::B;
		filter[ControllerSettings::BUTTONTYPE_LEFTWIRE] = Keyboard::Key::LControl;
		filter[ControllerSettings::BUTTONTYPE_RIGHTWIRE] = Keyboard::Key::Space;

		filter[ControllerSettings::BUTTONTYPE_PAUSE] = Keyboard::Key::Delete;

		filter[ControllerSettings::BUTTONTYPE_LLEFT] = Keyboard::Key::J;
		filter[ControllerSettings::BUTTONTYPE_LRIGHT] = Keyboard::Key::L;
		filter[ControllerSettings::BUTTONTYPE_LUP] = Keyboard::Key::I;
		filter[ControllerSettings::BUTTONTYPE_LDOWN] = Keyboard::Key::K;

		filter[ControllerSettings::BUTTONTYPE_RLEFT] = Keyboard::Key::U;
		filter[ControllerSettings::BUTTONTYPE_RRIGHT] = Keyboard::Key::O;
		filter[ControllerSettings::BUTTONTYPE_RUP] = Keyboard::Key::P;
		filter[ControllerSettings::BUTTONTYPE_RDOWN] = Keyboard::Key::SemiColon;

		break;
	}
		
	}
}

void ControlProfile::FilterState(ControllerState &state)
{
	ControllerState origState = state;

	state.A = origState.CheckControllerButton(filter[ControllerSettings::BUTTONTYPE_JUMP]);
	state.B = origState.CheckControllerButton(filter[ControllerSettings::BUTTONTYPE_SPECIAL]);
	state.X = origState.CheckControllerButton(filter[ControllerSettings::BUTTONTYPE_DASH]);
	state.Y = false;

	state.leftShoulder = origState.CheckControllerButton(filter[ControllerSettings::BUTTONTYPE_SHIELD]);
	state.rightShoulder = origState.CheckControllerButton(filter[ControllerSettings::BUTTONTYPE_ATTACK]);
	
	state.leftTrigger = origState.CheckControllerButton(filter[ControllerSettings::BUTTONTYPE_LEFTWIRE]);
	state.rightTrigger = origState.CheckControllerButton(filter[ControllerSettings::BUTTONTYPE_RIGHTWIRE]);

	state.start = origState.CheckControllerButton(filter[ControllerSettings::BUTTONTYPE_PAUSE]);
}

int ControlProfile::Filter(ControllerSettings::ButtonType b)
{
	return filter[b];
}

int ControlProfile::GetControllerType()
{
	return cType;
}

void ControlProfile::SetControllerType(int c)
{
	cType = c;
}

void ControlProfile::Save(ofstream &of)
{
	if (cType == CTYPE_KEYBOARD)
	{
		of << "JUMP:" << GetKeyboardButtonString(filter[ControllerSettings::BUTTONTYPE_JUMP]) << "\n";
		of << "DASH:" << GetKeyboardButtonString(filter[ControllerSettings::BUTTONTYPE_DASH]) << "\n";
		of << "ATTACK:" << GetKeyboardButtonString(filter[ControllerSettings::BUTTONTYPE_ATTACK]) << "\n";
		of << "SHIELD:" << GetKeyboardButtonString(filter[ControllerSettings::BUTTONTYPE_SHIELD]) << "\n";
		of << "SPECIAL:" << GetKeyboardButtonString(filter[ControllerSettings::BUTTONTYPE_SPECIAL]) << "\n";
		of << "LEFTWIRE:" << GetKeyboardButtonString(filter[ControllerSettings::BUTTONTYPE_LEFTWIRE]) << "\n";
		of << "RIGHTWIRE:" << GetKeyboardButtonString(filter[ControllerSettings::BUTTONTYPE_RIGHTWIRE]) << "\n";

		of << "LLEFT:" << GetKeyboardButtonString(filter[ControllerSettings::BUTTONTYPE_LLEFT]) << "\n";
		of << "LRIGHT:" << GetKeyboardButtonString(filter[ControllerSettings::BUTTONTYPE_LRIGHT]) << "\n";
		of << "LUP:" << GetKeyboardButtonString(filter[ControllerSettings::BUTTONTYPE_LUP]) << "\n";
		of << "LDOWN:" << GetKeyboardButtonString(filter[ControllerSettings::BUTTONTYPE_LDOWN]) << "\n";

		of << "RLEFT:" << GetKeyboardButtonString(filter[ControllerSettings::BUTTONTYPE_RLEFT]) << "\n";
		of << "RRIGHT:" << GetKeyboardButtonString(filter[ControllerSettings::BUTTONTYPE_RRIGHT]) << "\n";
		of << "RUP:" << GetKeyboardButtonString(filter[ControllerSettings::BUTTONTYPE_RUP]) << "\n";
		of << "RDOWN:" << GetKeyboardButtonString(filter[ControllerSettings::BUTTONTYPE_RDOWN]);// << "\n";
	}
	else
	{
		of << "JUMP:" << GetXBoxButtonString(filter[ControllerSettings::BUTTONTYPE_JUMP]) << "\n";
		of << "DASH:" << GetXBoxButtonString(filter[ControllerSettings::BUTTONTYPE_DASH]) << "\n";
		of << "ATTACK:" << GetXBoxButtonString(filter[ControllerSettings::BUTTONTYPE_ATTACK]) << "\n";
		of << "SHIELD:" << GetXBoxButtonString(filter[ControllerSettings::BUTTONTYPE_SHIELD]) << "\n";
		of << "SPECIAL:" << GetXBoxButtonString(filter[ControllerSettings::BUTTONTYPE_SPECIAL]) << "\n";
		of << "LEFTWIRE:" << GetXBoxButtonString(filter[ControllerSettings::BUTTONTYPE_LEFTWIRE]) << "\n";
		of << "RIGHTWIRE:" << GetXBoxButtonString(filter[ControllerSettings::BUTTONTYPE_RIGHTWIRE]);// << "\n";
	}
}

void ControlProfile::CopyTo(ControlProfile *cp)
{
	cp->cType = cType;
	cp->name = name;
	cp->editable = editable;
	for (int i = 0; i < ControllerSettings::BUTTONTYPE_Count; ++i)
	{
		cp->filter[i] = filter[i];
	}
}

ControlProfileMenu::ControlProfileMenu()
	:font( MainMenu::GetInstance()->arial ), topIndex( 0 ), action( A_SELECTED ), oldCurrIndex( 0 )
{
	actionButtonGroup = new ActionButtonGroup(this);

	tempProfile = new ControlProfile;

	//assert(!p_profiles.empty());
	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	saSelector = new SingleAxisSelector( 3, waitFrames, 2, waitModeThresh, 0, 0 );

	currProfile = NULL;// p_profiles.front(); //KIN 

	int textSize = 30;

	selectedProfileText.setFont(MainMenu::GetInstance()->arial);
	selectedProfileText.setCharacterSize(textSize);
	selectedProfileText.setFillColor(Color::Black);

	

	editingProfileText.setFont(MainMenu::GetInstance()->arial);
	editingProfileText.setCharacterSize(textSize);
	editingProfileText.setFillColor(Color::Black);
	
	

	//selectedProfileText.setOrigin(selectedProfileText.getLocalBounds().left + selectedProfileText.getLocalBounds().width / 2,
	//	selectedProfileText.getLocalBounds().top + selectedProfileText.getLocalBounds().height / 2);
	
	for( int i = 0; i < NUM_BOXES; ++i )
	{
		profileNames[i].setFont( font );
		profileNames[i].setCharacterSize(textSize);
		profileNames[i].setFillColor( Color::White );
	}

	/*waitFrames[0] = 10;
	waitFrames[1] = 5;
	waitFrames[2] = 2;

	waitModeThresh[0] = 2;
	waitModeThresh[1] = 2;

	currWaitLevel = 0;
	flipCounterUp = 0;
	flipCounterDown = 0;
	framesWaiting = 0;*/
	
	int quarter = 1920 / 4;
	//editProfileGrid = new UIControlGrid( NULL, 2, 4, controls, 20, 20, true );
	//editProfileGrid->SetTopLeft( topMid.x - quarter/2 + 10, topMid.y + 10 );

	maxReceiveFrames = 240;

	controllerInput = NULL;
}

ControlProfileMenu::~ControlProfileMenu()
{
	delete actionButtonGroup;
	delete saSelector;
	delete tempProfile;
}

void ControlProfileMenu::SetProfiles(std::list<ControlProfile*> &p_profiles)
{
	profiles = p_profiles;

	assert(!profiles.empty());

	currProfile = NULL;
	//currProfile = profiles.front();
	
	//if (profiles.empty())
	//{
	//	//cant be empty because I always add the default
	//}
}

void ControlProfileMenu::SetCurrProfile(ControlProfile *profile)
{
	currProfile = profile;
	selectedProfileText.setString("Controls: " + currProfile->name);
}

void ControlProfileMenu::SetControllerInput(ControllerDualStateQueue *p_controllerInput)
{
	controllerInput = p_controllerInput;

	auto &managedProfiles = MainMenu::GetInstance()->cpm->profiles[controllerInput->GetControllerType()];

	SetProfiles(managedProfiles);

	SetCurrProfile(managedProfiles.front());

	//currProfile = managedProfiles.front();//controlMenu = new ControlProfileMenu;

	//selectedProfileText.setString("Controls: " + currProfile->name);

	//currProfile = MainMenu::GetInstance()->cpm->profiles.front();

	currProfile->CopyTo(tempProfile);
	actionButtonGroup->UpdateButtonIcons();
}

ControlProfile *ControlProfileMenu::GetProfileAtIndex(int ind)
{
	int i = 0;
	for (auto it = profiles.begin(); it != profiles.end(); ++it)
	{
		if (i == ind)
		{
			return (*it);
		}
		++i;
	}

	return NULL;
}

void ControlProfileMenu::Draw( sf::RenderTarget *target )
{
	if( action == A_SHOWING_OPTIONS )
	{
		target->draw( boxes, NUM_BOXES*4,sf::Quads );
		for( int i = 0; i < NUM_BOXES; ++i )
		{
			//cout << "drawing: " << profileNames[i].getString().toAnsiString() << "\n";
			target->draw( profileNames[i] );
		}
		vSlider.Draw(target);
	}
	else if( action == A_EDIT_PROFILE || action == A_REPLACE_BUTTON )
	{
		actionButtonGroup->Draw(target);
		target->draw(editingProfileText);
		//editProfileGrid->Draw( target );
	}
	else if (action == A_SELECTED)
	{
		target->draw(selectedProfileText);
	}
}


void ControlProfileMenu::UpdateNames()
{
	if( profiles.size() == 0 )
	{

		return;
	}


	auto lit = profiles.begin();
	if( topIndex > profiles.size() )
	{
		topIndex = profiles.size() - 1;
	}

	for( int i = 0; i < topIndex; ++i )
	{
		++lit;
	}
	
	Vector2f topMid = topLeft + Vector2f(BOX_WIDTH / 2, 0);

	int trueI;
	int i = 0;
	int numProfiles = profiles.size();
	ControlProfile *tempProfile = NULL;
	for( ; i < NUM_BOXES; ++i )
	{
		trueI = (topIndex + i) % NUM_BOXES;
		if( i == numProfiles )
		{
			for( ; i < NUM_BOXES; ++i )
			{
				profileNames[i].setString( "" );
			}
			break;
		}

		if( lit == profiles.end() )
			lit = profiles.begin();

		tempProfile = GetProfileAtIndex(trueI);

		if (tempProfile->editable)
		{
			profileNames[i].setFillColor(Color::White);
		}
		else
		{
			profileNames[i].setFillColor(Color(150, 150, 150));
		}
		profileNames[i].setString( (*lit)->name );

		auto &lb = profileNames[i].getLocalBounds();
		profileNames[i].setOrigin( lb.left + lb.width / 2, lb.top + lb.height / 2);
			//profileNames[i].getLocalBounds().height / 2 );
		//profileNames[i].setPosition( topMid.x, topMid.y + (BOX_HEIGHT+BOX_SPACING) * i );
		profileNames[i].setPosition( topMid.x, topMid.y + (BOX_HEIGHT+BOX_SPACING) * i + BOX_HEIGHT / 2 );

		++lit;
	}

	saSelector->totalItems = numProfiles;

	Vector2f offset(20, 0);
	vSlider.Setup(Vector2f(topLeft.x + BOX_WIDTH + offset.x, topLeft.y + offset.y), 
		Vector2f(vSlider.barSize.x, max((vSlider.selectorSize.y / numProfiles), 5.f) ), vSlider.selectorSize);

	vSlider.SetSlider((float)saSelector->currIndex / (saSelector->totalItems - 1));
}

void ControlProfileMenu::SetTopLeft(sf::Vector2f &p_topLeft)
{
	topLeft = p_topLeft;
	//SetupBoxes(); //only need this if i was going to be moving the boxes while the selector isup
	selectedProfileText.setPosition(topLeft.x + 20, topLeft.y + 20);//topLeft.x, topLeft.y + 90);
	editingProfileText.setPosition(topLeft.x + 20, topLeft.y + 5);

	actionButtonGroup->SetTopLeft(topLeft + Vector2f( 0, 40 ));

	SetupBoxes();

	/*int numProfiles = profiles.size();
	Vector2f offset(20, 0);
	vSlider.Setup(Vector2f(topLeft.x + BOX_WIDTH + offset.x, topLeft.y + offset.y),
		Vector2f(vSlider.barSize.x, max((vSlider.selectorSize.y / numProfiles), 5.f)), vSlider.selectorSize);*/
}

void ControlProfileMenu::BeginSelectingProfile()
{
	action = A_SHOWING_OPTIONS;
	UpdateNames();
	UpdateBoxesDebug();
	oldCurrIndex = saSelector->currIndex;
}

void ControlProfileMenu::Update()
{	
	switch(action)
	{
	case A_SELECTED:
	{
		if (controllerInput->ButtonPressed_A())
		{
			BeginSelectingProfile();
		}
		break;
	}
	case A_SHOWING_OPTIONS:
	{
		if (controllerInput->ButtonPressed_A())
		{
			int test = 0;
			action = A_SELECTED;

			SetCurrProfile(GetProfileAtIndex(saSelector->currIndex));
			//selectedProfileText.setOrigin(selectedProfileText.getLocalBounds().left + selectedProfileText.getLocalBounds().width / 2,
			//	selectedProfileText.getLocalBounds().top + selectedProfileText.getLocalBounds().height / 2);
		}
		else if (controllerInput->ButtonPressed_B())
		{
			action = A_SELECTED;
			saSelector->currIndex = oldCurrIndex;
		}
		else if (controllerInput->ButtonPressed_X())
		{
			ControlProfile *focusedProfile = GetProfileAtIndex(saSelector->currIndex);
			action = A_EDIT_PROFILE;
			editingProfileText.setString(focusedProfile->name);
			focusedProfile->CopyTo(tempProfile);

			bool keyboardMode = (tempProfile->GetControllerType() == CTYPE_KEYBOARD);
			actionButtonGroup->SetKeyboardMode(keyboardMode);

			actionButtonGroup->SetSelectedIndex(0);
			actionButtonGroup->UpdateButtonIcons();
		}
		break;
	}
	case A_EDIT_PROFILE:
	{
		if (tempProfile->editable)
		{
			if (controllerInput->ButtonPressed_A())
			{
				actionButtonGroup->ModifySelectedButton();
				action = A_REPLACE_BUTTON;
			}
			else if (controllerInput->ButtonPressed_B())
			{
				if (actionButtonGroup->AllButtonsAssigned())
				{
					action = A_SHOWING_OPTIONS;
				}
			}
			else if (controllerInput->ButtonPressed_X())
			{
				if (actionButtonGroup->AllButtonsAssigned())
				{
					action = A_SHOWING_OPTIONS;
					tempProfile->CopyTo(GetProfileAtIndex(saSelector->currIndex));
					MainMenu::GetInstance()->cpm->WriteProfiles();
				}
			}
		}
		else
		{
			if (controllerInput->ButtonPressed_A() || controllerInput->ButtonPressed_B() || controllerInput->ButtonPressed_X())
			{
				action = A_SHOWING_OPTIONS;
			}
		}
		
		break;
	}
	case A_REPLACE_BUTTON:
	{
		if (controllerInput->GetControllerType() == CTYPE_KEYBOARD)
		{
			int key = Keyboard::Unknown;

			for (int i = 0; i < Keyboard::KeyCount; ++i)
			{
				if( IsKeyValidForInput(i) )
				{
					if (CONTROLLERS.KeyboardButtonPressed(i))
					{
						key = i;
						break;
					}
				}	
			}

			if (key != Keyboard::Unknown)
			{
				actionButtonGroup->SetModifiedButton(key);
				action = A_EDIT_PROFILE;
			}
			
			//if( controllerInput->GetCurrState())
		}
		else
		{
			XBoxButton button = XBOX_BLANK;
			if (controllerInput->ButtonPressed_A())
			{
				button = XBOX_A;
			}
			else if (controllerInput->ButtonPressed_B())
			{
				button = XBOX_B;
			}
			else if (controllerInput->ButtonPressed_X())
			{
				button = XBOX_X;
			}
			else if (controllerInput->ButtonPressed_Y())
			{
				button = XBOX_Y;
			}
			else if (controllerInput->ButtonPressed_LeftShoulder())
			{
				button = XBOX_L1;
			}
			else if (controllerInput->ButtonPressed_RightShoulder())
			{
				button = XBOX_R1;
			}
			else if (controllerInput->ButtonPressed_LeftTrigger())
			{
				button = XBOX_L2;
			}
			else if (controllerInput->ButtonPressed_RightTrigger())
			{
				button = XBOX_R2;
			}

			//dont forget triggers

			if (button != XBOX_BLANK)
			{
				actionButtonGroup->SetModifiedButton(button);
				action = A_EDIT_PROFILE;
			}
		}
		
		break;
	}
	}

	if( action == A_SHOWING_OPTIONS )
	{

		bool up = controllerInput->GetCurrState().LUp();
		bool down = controllerInput->GetCurrState().LDown();

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

		if( changed != 0 )
		{
			

			UpdateNames();
			//cout << "currIndex: " << cIndex << ", topIndex: " << topIndex << endl;
			//controls[oldIndex]->Unfocus();
			//controls[focusedIndex]->Focus();

			vSlider.SetSlider((float)saSelector->currIndex / (saSelector->totalItems - 1));

		}

		//cout << "currIndex : " << currIndex << endl;
		UpdateBoxesDebug();
		
	}
	else if( action == A_EDIT_PROFILE )
	{
		actionButtonGroup->Update();
		//editProfileGrid->Update( currInput, prevInput );
	}
	else if( action == A_REPLACE_BUTTON )
	{
		actionButtonGroup->Update();
		//XBoxButton but = ReceiveInput( currInput, prevInput );
		/*if( but == XBoxButton::XBOX_BLANK )
		{
			if( currReceiveFrame == maxReceiveFrames )
			{
				action = A_EDIT_PROFILE;
			}
		}
		else
		{
			tempFilter[editIndex] = but;
			action = A_EDIT_PROFILE;
		}*/
	}
}

bool ControlProfileMenu::SaveCurrConfig()
{
	bool different = false;
	//XBoxButton *fil = currProfile->GetCurrFilter();
	/*for( int i = 0; i < ControllerSettings::BUTTONTYPE_Count; ++i )
	{
		if(fil[i] != tempFilter[i] )
		{
			fil[i] = tempFilter[i];
			different = true;
		}
	}*/

	//commented out for now because using mapselectionmenu or related stuff
	//if( different ) section->mainMenu->cpm->WriteProfiles();
	MainMenu::GetInstance()->cpm->WriteProfiles();

	action = A_SELECTED;
	return true;
}

void ControlProfileMenu::SetupBoxes()
{
	sf::Vector2f currTopLeft;
	int extraHeight = 0;
	
	for( int i = 0; i < NUM_BOXES; ++i )
	{
		currTopLeft = topLeft + Vector2f( 0, extraHeight );

		boxes[i*4+0].position = Vector2f(currTopLeft.x, currTopLeft.y );
		boxes[i*4+1].position = Vector2f(currTopLeft.x + BOX_WIDTH, currTopLeft.y );
		boxes[i*4+2].position = Vector2f(currTopLeft.x + BOX_WIDTH, currTopLeft.y + BOX_HEIGHT );
		boxes[i*4+3].position = Vector2f(currTopLeft.x, currTopLeft.y + BOX_HEIGHT );

		boxes[i*4+0].color = Color::Red;
		boxes[i*4+1].color = Color::Red;
		boxes[i*4+2].color = Color::Red;
		boxes[i*4+3].color = Color::Red;

		extraHeight += BOX_HEIGHT + BOX_SPACING;
	}

	Vector2f offset(20, 0);
	vSlider.Setup(Vector2f(topLeft.x + BOX_WIDTH + offset.x, topLeft.y + offset.y), Vector2f(30, 0),
		Vector2f(30, NUM_BOXES * (BOX_HEIGHT + BOX_SPACING)));
}

void ControlProfileMenu::MoveUp()
{
	topIndex++;
	if( topIndex == profiles.size() )
	{
		topIndex = 0;
	}
}

void ControlProfileMenu::MoveDown()
{
	topIndex--;
	if( topIndex == -1 )
	{
		topIndex = profiles.size() - 1;
	}
}

void ControlProfileMenu::UpdateBoxesDebug()
{
	Color c;
	int trueI = ( saSelector->currIndex - topIndex );// % NUM_BOXES;
	for( int i = 0; i < NUM_BOXES; ++i )
	{
		if( i == trueI )
		{
			c = Color::Blue;
		}
		else
		{
			c = Color::Red;
		}
		boxes[i*4+0].color = c;
		boxes[i*4+1].color = c;
		boxes[i*4+2].color = c;
		boxes[i*4+3].color = c;
	}
}

void ControlProfileManager::ClearProfiles()
{
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (auto it = profiles[i].begin(); it != profiles[i].end(); ++it)
		{
			delete (*it);
		}
		profiles[i].clear();
	}
}

bool ControlProfileManager::LoadProfiles()
{
	ClearProfiles();

	ControlProfile *defXBOX = new ControlProfile;
	defXBOX->name = "Default";
	defXBOX->editable = false;
	defXBOX->SetControllerType(CTYPE_XBOX);
	defXBOX->SetFilterDefault();

	profiles[CTYPE_XBOX].push_back(defXBOX);

	ControlProfile *defGCC = new ControlProfile;
	defGCC->name = "Default";
	defGCC->editable = false;
	defGCC->SetControllerType(CTYPE_GAMECUBE);
	defGCC->SetFilterDefault();

	profiles[CTYPE_GAMECUBE].push_back(defGCC);

	ControlProfile *defKeyboard = new ControlProfile;
	defKeyboard->name = "Default";
	defKeyboard->editable = false;
	defKeyboard->SetControllerType(CTYPE_KEYBOARD);
	defKeyboard->SetFilterDefault();

	profiles[CTYPE_KEYBOARD].push_back(defKeyboard);

	is.open( "Resources/controlprofiles.txt" );

	if( is.is_open() )
	{
		string profileName;
		string inputTypeName;
		while( MoveToNextSymbolText( PROFILE_START_CHAR, PROFILE_END_CHAR, profileName ) )
		{
			//cout << "new profile: " << profileName << "\n";
			ControlProfile *newProfile = new ControlProfile;
			newProfile->name = profileName;
			

			//while (true)
			//{

			char opener = 0;
			bool res = MoveToPeekNextOpener(opener);

			char test = is.peek();

			if (opener == PROFILE_START_CHAR)
			{
				cout << "done with input types\n";
				break;
			}
			else if (opener == INPUT_TYPE_START_CHAR)
			{
				res = MoveToNextSymbolText(INPUT_TYPE_START_CHAR, INPUT_TYPE_END_CHAR, inputTypeName);
				if (!res)
				{
					assert(0);
					return false;
				}
				//cout << "input type: " << inputTypeName << "\n";
				if (inputTypeName == INPUT_TYPE_XBOX)
				{
					newProfile->SetControllerType(CTYPE_XBOX);
					res = LoadControllerConfig(newProfile);
					if (!res)
					{
						newProfile->SetFilterDefault();
						//SetFilterDefault(newProfile->filter);
						assert(0);
						return false;
					}
					profiles[CTYPE_XBOX].push_back(newProfile);
				}
				else if (inputTypeName == INPUT_TYPE_KEYBOARD)
				{
					newProfile->SetControllerType(CTYPE_KEYBOARD);
					res = LoadKeyboardConfig(newProfile);
					if (!res)
					{
						newProfile->SetFilterDefault();
						//SetFilterDefaultGCC(newProfile->gccFilter);
						assert(0);
						return false;
					}
					profiles[CTYPE_KEYBOARD].push_back(newProfile);
					//TODO
				}
				else if (inputTypeName == INPUT_TYPE_GAMECUBE)
				{
					newProfile->SetControllerType(CTYPE_GAMECUBE);
					res = LoadControllerConfig(newProfile);
					if (!res)
					{
						newProfile->SetFilterDefault();
						//SetFilterDefaultGCC(newProfile->gccFilter);
						assert(0);
						return false;
					}
					profiles[CTYPE_GAMECUBE].push_back(newProfile);
				}
			}
			else
			{
				return true;
			}
			//}
		}
	}
	else
	{
	}

	//WriteProfiles(); //debug, will be in other functions
	//DeleteProfile( profiles.begin() );

	
}

void ControlProfileManager::DebugPrint()
{
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (auto it = profiles[i].begin(); it != profiles[i].end(); ++it)
		{
			cout << "profile: " << (*it)->name << endl;
		}
	}
	
}

bool ControlProfileManager::IsSymbol( char c )
{
	if( c == PROFILE_START_CHAR || c == PROFILE_END_CHAR || c == INPUT_TYPE_START_CHAR
			|| c == INPUT_TYPE_END_CHAR )
			return true;

	return false;
}

//char is 0 for fail, or a symbol if it hits a symbol
char ControlProfileManager::MoveToColon( std::string &outStr )
{
	stringstream ss;
	char c = 0;
	while( true )
	{
		c = is.peek();
		if( c == EOF )
			return 0;

		if( IsSymbol( c ) )
		{
			return c;
		}

		if( c == COLON )
		{
			outStr = ss.str();
			
			if( !is.get() )
			{
				return 0;
			}

			return 1;
		}
		else
		{
			if( c >= 48 && c <= 122 )
				ss << c;
		}

		if( !is.get() )
		{
			return 0;
		}
	}
}

bool ControlProfileManager::LoadControllerConfig( ControlProfile *profile ) 
{
	string inputName;
	string buttonStr;

	while( true )
	{
		char ec = MoveToColon( inputName );
		char c = 0;

		if( ec == 0 )
			return true;

		if( IsSymbol( ec ) )
		{
			return true;
		}


		if( !is.get( c ) )
		{
			return false;
		}

		buttonStr.clear();
		buttonStr.push_back(c);
		char mod = 0;

		if (c == 'R' || c == 'L')
		{
			if (!is.get(mod))
				return false;

			buttonStr.push_back(mod);
		}

		XBoxButton b = GetButton( buttonStr );
		ControllerSettings::ButtonType buttonType = GetButtonTypeFromAction( inputName );

		assert( buttonType < ControllerSettings::BUTTONTYPE_Count );

		profile->filter[buttonType] = b;

	}
}

bool ControlProfileManager::LoadKeyboardConfig(ControlProfile *profile)
{
	string inputName;
	string buttonStr;

	while (true)
	{
		char ec = MoveToColon(inputName);
		char c = 0;

		if (ec == 0)
			return true;

		if (IsSymbol(ec))
		{
			return true;
		}

		getline(is, buttonStr);

		sf::Keyboard::Key k = GetKey(buttonStr);
		ControllerSettings::ButtonType buttonType = GetButtonTypeFromAction(inputName);

		assert(buttonType < ControllerSettings::BUTTONTYPE_Count);

		profile->filter[buttonType] = k;
	}
}

ControllerSettings::ButtonType ControlProfileManager::GetButtonTypeFromAction( 
	const std::string &inputName )
{
	ControllerSettings::ButtonType buttonType;
	if( inputName == "JUMP" )
	{
		buttonType = ControllerSettings::BUTTONTYPE_JUMP;
	}
	else if( inputName == "DASH" )
	{
		buttonType = ControllerSettings::BUTTONTYPE_DASH;
	}
	else if( inputName == "ATTACK" )
	{
		buttonType = ControllerSettings::BUTTONTYPE_ATTACK;
	}
	else if( inputName == "SHIELD" )
	{
		buttonType = ControllerSettings::BUTTONTYPE_SHIELD;
	}
	else if (inputName == "SPECIAL")
	{
		buttonType = ControllerSettings::BUTTONTYPE_SPECIAL;
	}
	else if (inputName == "LEFTWIRE")
	{
		buttonType = ControllerSettings::BUTTONTYPE_LEFTWIRE;
	}
	else if( inputName == "RIGHTWIRE" )
	{
		buttonType = ControllerSettings::BUTTONTYPE_RIGHTWIRE;
	}
	else if (inputName == "PAUSE")
	{
		buttonType = ControllerSettings::BUTTONTYPE_PAUSE;
	}
	else if (inputName == "LLEFT")
	{
		buttonType = ControllerSettings::BUTTONTYPE_LLEFT;
	}
	else if (inputName == "LRIGHT")
	{
		buttonType = ControllerSettings::BUTTONTYPE_LRIGHT;
	}
	else if (inputName == "LUP")
	{
		buttonType = ControllerSettings::BUTTONTYPE_LUP;
	}
	else if (inputName == "LDOWN")
	{
		buttonType = ControllerSettings::BUTTONTYPE_LDOWN;
	}
	else if (inputName == "RLEFT")
	{
		buttonType = ControllerSettings::BUTTONTYPE_RLEFT;
	}
	else if (inputName == "RRIGHT")
	{
		buttonType = ControllerSettings::BUTTONTYPE_RRIGHT;
	}
	else if (inputName == "RUP")
	{
		buttonType = ControllerSettings::BUTTONTYPE_RUP;
	}
	else if (inputName == "RDOWN")
	{
		buttonType = ControllerSettings::BUTTONTYPE_RDOWN;
	}
	else
	{
		assert( 0 );
	}

	return buttonType;
}

XBoxButton ControlProfileManager::GetButton( const std::string &str )
{
	if( str == "A" || str == "a" )
	{
		return XBoxButton::XBOX_A;
	}
	else if( str == "B" || str == "b")
	{
		return XBoxButton::XBOX_B;
	}
	else if( str == "X" || str == "x")
	{
		return XBoxButton::XBOX_X;
	}
	else if( str == "Y" || str == "y")
	{
		return XBoxButton::XBOX_Y;
	}
	else if( str == "R1"  || str == "r1" )
	{
		return XBoxButton::XBOX_R1;
	}
	else if( str == "L1"  || str == "l1")
	{
		return XBoxButton::XBOX_L1;
	}
	else if( str == "R2" || str == "r2")
	{
		return XBoxButton::XBOX_R2;
	}
	else if( str == "L2" || str == "l2")
	{
		return XBoxButton::XBOX_L2;
	}

	return XBOX_BLANK;
}

sf::Keyboard::Key ControlProfileManager::GetKey(const std::string &str)
{
	if (str.length() == 1)
	{
		//assume its a letter

		char c = str[0];

		if (c >= 'A' && c <= 'Z')
		{
			return (Keyboard::Key)(c - 'A');
		}
		else if( c >= '0' && c <= '9')
		{
			return (Keyboard::Key)(c - '0');
		}
		else
		{
			switch (c)
			{
			case '[':
				return Keyboard::LBracket;
			case ']':
				return Keyboard::RBracket;
			case ';':
				return Keyboard::Semicolon;
			case ',':
				return Keyboard::Comma;
			case '.':
				return Keyboard::Period;
			case '\'':
				return Keyboard::Quote;
			case '/':
				return Keyboard::Slash;
			case '\\':
				return Keyboard::Backslash;
			case '`':
				return Keyboard::Tilde;
			case '=':
				return Keyboard::Equal;
			case '-':
				return Keyboard::Hyphen;
			}
		}
	}
	else
	{
		if (str == "LCONTROL")
		{
			return Keyboard::LControl;
		}
		else if (str == "LSHIFT")
		{
			return Keyboard::LShift;
		}
		else if (str == "LALT")
		{
			return Keyboard::LAlt;
		}
		else if (str == "RCONTROL")
		{
			return Keyboard::RControl;
		}
		else if (str == "RSHIFT")
		{
			return Keyboard::RShift;
		}
		else if (str == "RALT")
		{
			return Keyboard::RAlt;
		}
		else if (str == "SPACE")
		{
			return Keyboard::Space;
		}
		else if (str == "ENTER")
		{
			return Keyboard::Enter;
		}
		else if (str == "BACKSPACE")
		{
			return Keyboard::Backspace;
		}
		else if (str == "TAB")
		{
			return Keyboard::Tab;
		}
		else if (str == "PAGEUP")
		{
			return Keyboard::PageUp;
		}
		else if (str == "PAGEDOWN")
		{
			return Keyboard::PageDown;
		}
		else if (str == "END")
		{
			return Keyboard::End;
		}
		else if (str == "HOME")
		{
			return Keyboard::Home;
		}
		else if (str == "INSERT")
		{
			return Keyboard::Insert;
		}
		else if (str == "DELETE")
		{
			return Keyboard::Delete;
		}
		else if (str == "LEFT")
		{
			return Keyboard::Left;
		}
		else if (str == "RIGHT")
		{
			return Keyboard::Right;
		}
		else if (str == "UP")
		{
			return Keyboard::Up;
		}
		else if (str == "DOWN")
		{
			return Keyboard::Down;
		}
		/*else if (str == "NUMPAD0")
		{
			return Keyboard::Numpad0;
		}
		else if (str == "NUMPAD1")
		{
			return Keyboard::Numpad1;
		}
		else if (str == "NUMPAD2")
		{
			return Keyboard::Numpad2;
		}
		else if (str == "NUMPAD3")
		{
			return Keyboard::Numpad3;
		}
		else if (str == "NUMPAD4")
		{
			return Keyboard::Numpad4;
		}
		else if (str == "NUMPAD5")
		{
			return Keyboard::Numpad5;
		}
		else if (str == "NUMPAD6")
		{
			return Keyboard::Numpad6;
		}
		else if (str == "NUMPAD7")
		{
			return Keyboard::Numpad7;
		}
		else if (str == "NUMPAD8")
		{
			return Keyboard::Numpad8;
		}
		else if (str == "NUMPAD9")
		{
			return Keyboard::Numpad9;
		}
		else if (str == "F1")
		{
			return Keyboard::F1;
		}
		else if (str == "F2")
		{
			return Keyboard::F2;
		}
		else if (str == "F3")
		{
			return Keyboard::F3;
		}
		else if (str == "F4")
		{
			return Keyboard::F4;
		}
		else if (str == "F5")
		{
			return Keyboard::F5;
		}
		else if (str == "F6")
		{
			return Keyboard::F6;
		}
		else if (str == "F7")
		{
			return Keyboard::F7;
		}
		else if (str == "F8")
		{
			return Keyboard::F8;
		}
		else if (str == "F9")
		{
			return Keyboard::F9;
		}
		else if (str == "F10")
		{
			return Keyboard::F10;
		}
		else if (str == "F11")
		{
			return Keyboard::F11;
		}
		else if (str == "F12")
		{
			return Keyboard::F12;
		}*/
	}

	return Keyboard::Key::Unknown;
}

bool ControlProfileManager::MoveToPeekNextOpener( char &outChar )
{
	char c = 0;
	while( true )
	{
		c = is.peek();

		if( c == EOF )
			return false;

		if( c == PROFILE_START_CHAR || c == INPUT_TYPE_START_CHAR )
		{
			outChar = c;
			return true;
		}

		if( !is.get() )
		{
			return false;
		}
	}
}

void ControlProfileManager::DeleteProfile( std::list<ControlProfile*>::iterator &it )
{
	//profiles.erase( it );

	//WriteProfiles();
}

//true if found, false if not found
bool ControlProfileManager::MoveToNextSymbolText( char startSymbol, char endSymbol, std::string &outStr )
{
	const int MAX_NAME_LENGTH = 24;
	char cStart = 0;
	char cEnd = 0;
	stringstream ss;

	while( true )
	{
		if( !is.get( cStart ) )
		{
			return false;
		}
		
		if( cStart == startSymbol )
		{
			for( int i = 0; i < MAX_NAME_LENGTH + 1; ++i )
			{
				if( !is.get( cEnd ) )
				{
					outStr = ss.str();
					return false;
				}

				if( cEnd == endSymbol )
				{
					outStr = ss.str();
					return true;
				}
				else
				{
					ss << cEnd;
				}
			}
			outStr = ss.str();
			return false;
		}
	}
}

void ControlProfileManager::WriteInputType(ofstream &of, const std::string &inputType)
{
	of << INPUT_TYPE_START_CHAR << inputType << INPUT_TYPE_END_CHAR << "\n";
}

void ControlProfileManager::WriteProfiles()
{
	ofstream of;
	of.open( "Resources/controlprofiles.txt" );

	int editableProfiles;

	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		editableProfiles = 0;
		for (auto it = profiles[i].begin(); it != profiles[i].end(); ++it)
		{
			if ((*it)->editable)
			{
				editableProfiles++;
			}
		}

		if (editableProfiles == 0)
		{
			//empty
			continue;
		}
		int controllerType;

		for (auto it = profiles[i].begin(); it != profiles[i].end(); ++it)
		{
			if (!(*it)->editable)
			{
				continue;
			}

			of << PROFILE_START_CHAR << (*it)->name << PROFILE_END_CHAR << "\n";

			controllerType = (*it)->GetControllerType();
			switch (controllerType)
			{
			case CTYPE_XBOX:
			{
				WriteInputType(of, INPUT_TYPE_XBOX);
				break;
			}
			case CTYPE_GAMECUBE:
			{
				WriteInputType(of, INPUT_TYPE_GAMECUBE);
				break;
			}
			case CTYPE_KEYBOARD:
			{
				WriteInputType(of, INPUT_TYPE_KEYBOARD);
				break;
			}
			}

			(*it)->Save(of);

			of << "\n\n";
		}
	}
}

ControlProfileManager::~ControlProfileManager()
{
	ClearProfiles();
}

ActionButtonGroup::ActionButtonGroup(ControlProfileMenu *p_controlMenu)
{
	controlMenu = p_controlMenu;

	topLeft = Vector2f(0, 0);

	numButtons = 0;

	selectedIndex = 0;

	buttonQuads = NULL;

	keyboardMode = true;
	SetKeyboardMode(false);

	Reset();
}

ActionButtonGroup::~ActionButtonGroup()
{
	for (int i = 0; i < actionButtons.size(); ++i)
	{
		delete actionButtons[i];
	}

	delete[] buttonQuads;
}

void ActionButtonGroup::Reset()
{
	action = A_SELECT_BUTTON;

	SetSelectedIndex(0);
}

void ActionButtonGroup::SetTopLeft(sf::Vector2f &pos)
{
	topLeft = pos;

	Vector2f start = Vector2f(spacing.x / 2, 10);
	
	int x, y;
	for (int i = 0; i < numButtons; ++i)
	{
		x = i % cols;
		y = i / cols;
		actionButtons[i]->SetPosition(topLeft + start + Vector2f(x * spacing.x, y * spacing.y));
	}
	SetSelectedIndex(selectedIndex);
}

int ActionButtonGroup::GetDefaultButton()
{
	int defaultButton = XBOX_BLANK;

	if (controlMenu->tempProfile->GetControllerType() == CTYPE_KEYBOARD)
	{
		defaultButton = Keyboard::Unknown; //keyboard blank??
	}

	return defaultButton;
}

bool ActionButtonGroup::AllButtonsAssigned()
{
	int defaultButton = GetDefaultButton();

	for (int i = 0; i < numButtons; ++i)
	{
		if (controlMenu->tempProfile->filter[i] == defaultButton)
		{
			return false;
		}
	}

	return true;
}

void ActionButtonGroup::SetKeyboardMode(bool p_keyboardMode)
{
	if (keyboardMode != p_keyboardMode)
	{
		keyboardMode = p_keyboardMode;

		if (keyboardMode)
		{
			float textSize = 16;
			squareSize = 40;
			spacing.x = 56 + squareSize;
			spacing.y = 48 + squareSize;

			std::vector<std::string> buttonTexts = {
				"JUMP",
				"DASH",
				"ATTACK",
				"SHIELD",
				"SPECIAL",
				"B-WIRE",
				"R-WIRE",
				"PAUSE",
				"L-LEFT",
				"L-RIGHT",
				"L-UP",
				"L-DOWN",
				"R-LEFT",
				"R-RIGHT",
				"R-UP",
				"R-DOWN" };

			numButtons = buttonTexts.size();

			cols = 4;
			rows = ceil(numButtons / (float)cols);

			if (buttonQuads != NULL)
			{
				delete[]buttonQuads;
			}

			buttonQuads = new Vertex[numButtons * 4];

			for (int i = 0; i < actionButtons.size(); ++i)
			{
				delete actionButtons[i];
			}

			actionButtons.resize(numButtons);
			for (int i = 0; i < numButtons; ++i)
			{
				actionButtons[i] = new ActionButton(buttonQuads + 4 * i, buttonTexts[i]);

				actionButtons[i]->SetSizes(squareSize, textSize);
			}

			SetTopLeft(topLeft);
		}
		else
		{
			float textSize = 20;
			squareSize = 64;
			spacing.x = 66 + squareSize;
			spacing.y = 56 + squareSize;

			std::vector<std::string> buttonTexts = {
				"JUMP",
				"DASH",
				"ATTACK",
				"SHIELD",
				"SPECIAL",
				"B-WIRE",
				"R-WIRE" };

			numButtons = buttonTexts.size();

			cols = 3;
			rows = ceil(numButtons / (float)cols);

			if (buttonQuads != NULL)
			{
				delete[]buttonQuads;
			}

			buttonQuads = new Vertex[numButtons * 4];

			for (int i = 0; i < actionButtons.size(); ++i)
			{
				delete actionButtons[i];
			}

			actionButtons.resize(numButtons);
			for (int i = 0; i < numButtons; ++i)
			{
				actionButtons[i] = new ActionButton(buttonQuads + 4 * i, buttonTexts[i]);
				actionButtons[i]->SetSizes(squareSize, textSize);
			}

			SetTopLeft(topLeft);
		}

	}
	
}

void ActionButtonGroup::SetSelectedIndex(int sel)
{
	if (selectedIndex != sel)
	{
		//play noise
	}
	selectedIndex = sel;

	SetRectColor(highlightQuad, Color::White);
	float bSize = actionButtons[selectedIndex]->buttonSize;
	float border = 8;//bSize / 4;//16;
	SetRectCenter(highlightQuad, bSize + border, bSize + border, actionButtons[selectedIndex]->quadCenter );
}

void ActionButtonGroup::SetModifiedButton(int button)
{
	//replace the button here
	action = A_SELECT_BUTTON;
	SetRectColor(highlightQuad, Color::White);

	int defaultButton = GetDefaultButton();

	for (int i = 0; i < numButtons; ++i)
	{
		if (controlMenu->tempProfile->filter[i] == button)
		{
			controlMenu->tempProfile->filter[i] = defaultButton;
		}
	}

	controlMenu->tempProfile->filter[selectedIndex] = button;
	UpdateButtonIcons();
}

void ActionButtonGroup::ModifySelectedButton()
{
	action = A_MODIFY_BUTTON;
	
	//controlMenu->tempProfile->filter[i]
	SetRectColor(highlightQuad, Color::Magenta);
}

void ActionButtonGroup::UpdateButtonIcons()
{
	assert(controlMenu->tempProfile != NULL);
	ControllerDualStateQueue *controllerInput = controlMenu->controllerInput;

	MainMenu *mm = MainMenu::GetInstance();
	for (int i = 0; i < numButtons; ++i)
	{
		actionButtons[i]->SetButtonSubRect(mm->GetButtonIconTile((ControllerSettings::ButtonType)i, controlMenu->tempProfile));
	}
}

void ActionButtonGroup::Update()
{
	ControllerDualStateQueue *controllerInput = controlMenu->controllerInput;

	switch (action)
	{
	case A_SELECT_BUTTON:
	{
		if (!controlMenu->tempProfile->editable)
			break;

		int x = selectedIndex % cols;
		int y = selectedIndex / cols;

		//int oldX = x;
		//int oldY = y;

		int tempIndex = selectedIndex;

		if (controllerInput->GetPrevState().IsLeftNeutral())
		{
			if (controllerInput->DirPressed_Left())
			{
				do
				{
					--x;
					if (x < 0)
					{
						x = cols - 1;
					}

					tempIndex = y * cols + x;
				} 
				while (tempIndex >= numButtons);
			}
			else if (controllerInput->DirPressed_Right())
			{
				do
				{
					++x;
					if (x == cols)
					{
						x = 0;
					}
					tempIndex = y * cols + x;
				}
				while (tempIndex >= numButtons);
			}

			if (controllerInput->DirPressed_Up())
			{
				do
				{
					--y;
					if (y < 0)
					{
						y = cols - 1;
					}
					tempIndex = y * cols + x;
				}
				while (tempIndex >= numButtons);
			}
			else if (controllerInput->DirPressed_Down())
			{
				do
				{
					++y;
					if (y == cols)
					{
						y = 0;
					}
					tempIndex = y * cols + x;
				}
				while (tempIndex >= numButtons);
			}
		}

		//tempIndex = y * cols + x;

		if (tempIndex != selectedIndex)
		{
			SetSelectedIndex(tempIndex);
		}
		break;
	}
	}
}

void ActionButtonGroup::Draw(sf::RenderTarget *target)
{
	MainMenu *mm = MainMenu::GetInstance();
	
	int currProfileControllerType = controlMenu->currProfile->GetControllerType();

	Tileset *ts_buttons = mm->GetButtonIconTileset(currProfileControllerType);

	if (controlMenu->tempProfile->editable )
	{
		target->draw(highlightQuad, 4, sf::Quads);
	}

	target->draw(buttonQuads, 4 * numButtons, sf::Quads, ts_buttons->texture);

	for (int i = 0; i < numButtons; ++i)
	{
		actionButtons[i]->Draw(target);
	}
}

ActionButton::ActionButton(sf::Vertex *p_quad, const std::string &name )
{
	quad = p_quad;

	MainMenu *mm = MainMenu::GetInstance();

	actionName.setFont(mm->arial);
	actionName.setFillColor(Color::White);
	actionName.setString(name);

	SetSizes(64, 20);
}

void ActionButton::SetSizes(float bSize, float p_textSize )
{
	buttonSize = bSize;

	textSize = p_textSize;
	actionName.setCharacterSize(textSize);
	auto lb = actionName.getLocalBounds();
	actionName.setOrigin(lb.left + lb.width / 2, 0);

	SetPosition(position);
}

void ActionButton::SetButtonSubRect(sf::IntRect &ir)
{
	SetRectSubRect(quad, ir);
}

void ActionButton::SetPosition(sf::Vector2f &pos)
{
	position = pos;
	actionName.setPosition(position);

	float border = 8;//buttonSize / 4;
	float extra = 5;

	quadCenter = position + Vector2f(0, textSize + border + buttonSize / 2 + extra);
	SetRectCenter(quad, buttonSize, buttonSize, quadCenter);
}

void ActionButton::Draw(sf::RenderTarget *target)
{
	target->draw(actionName);
}