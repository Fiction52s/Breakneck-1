#include "ControlProfile.h"
#include <sstream>
#include <assert.h>
#include <iostream>
#include "UIWindow.h"
#include "MainMenu.h"
#include "VectorMath.h"

using namespace sf;
using namespace std;

#define PROFILE_START_CHAR '<'
#define PROFILE_END_CHAR '>'
#define INPUT_TYPE_START_CHAR '['
#define INPUT_TYPE_END_CHAR ']'
#define INPUT_TYPE_XBOX "XBOX"
#define INPUT_TYPE_KEYBOARD "KEYBOARD"
#define INPUT_TYPE_GAMECUBE "GAMECUBE"
#define EQUALS '='

//const int ControlProfileMenu::NUM_BOXES = 8;
const int ControlProfileMenu::BOX_WIDTH = 300;
const int ControlProfileMenu::BOX_HEIGHT = 40;
const int ControlProfileMenu::BOX_SPACING = 10;

const int ProfileSelector::BOX_WIDTH = 300;
const int ProfileSelector::BOX_HEIGHT = 40;
const int ProfileSelector::BOX_SPACING = 10;

//waitFrames[0] = 10;
//waitFrames[1] = 5;
//waitFrames[2] = 2;

//waitModeThresh[0] = 2;
//waitModeThresh[1] = 2;

ControlProfile::ControlProfile()
{
	SetFilterDefault();
}

void ControlProfile::SetFilterDefault()
{
	for (int i = 0; i < ControllerSettings::BUTTONTYPE_Count; ++i)
	{
		filter[i] = XBOX_BLANK;
	}

	filter[ControllerSettings::BUTTONTYPE_JUMP] = XBOX_A;
	filter[ControllerSettings::BUTTONTYPE_DASH] = XBOX_X;
	filter[ControllerSettings::BUTTONTYPE_ATTACK] = XBOX_R1;
	filter[ControllerSettings::BUTTONTYPE_SHIELD] = XBOX_L1;
	filter[ControllerSettings::BUTTONTYPE_LEFTWIRE] = XBOX_L2;
	filter[ControllerSettings::BUTTONTYPE_RIGHTWIRE] = XBOX_R2;
	filter[ControllerSettings::BUTTONTYPE_SPECIAL] = XBOX_B;
	filter[ControllerSettings::BUTTONTYPE_MAP] = XBOX_BACK;
	filter[ControllerSettings::BUTTONTYPE_PAUSE] = XBOX_START;

	//gamecube controller

	/*filter[ControllerSettings::BUTTONTYPE_JUMP] = XBOX_Y;
	filter[ControllerSettings::BUTTONTYPE_DASH] = XBOX_X;
	filter[ControllerSettings::BUTTONTYPE_ATTACK] = XBOX_A;
	filter[ControllerSettings::BUTTONTYPE_SHIELD] = XBOX_R1;
	filter[ControllerSettings::BUTTONTYPE_LEFTWIRE] = XBOX_L2;
	filter[ControllerSettings::BUTTONTYPE_RIGHTWIRE] = XBOX_R2;
	filter[ControllerSettings::BUTTONTYPE_SPECIAL] = XBOX_B;
	filter[ControllerSettings::BUTTONTYPE_MAP] = XBOX_BACK;
	filter[ControllerSettings::BUTTONTYPE_PAUSE] = XBOX_START;*/
}

ControlProfileMenu::ControlProfileMenu( list<ControlProfile*> &p_profiles, Vector2f &p_topMid )
	:profiles( p_profiles ),
	font( MainMenu::GetInstance()->arial ), topIndex( 0 ), state( S_SELECTED ), oldCurrIndex( 0 ), topMid( p_topMid )
{
	actionButtonGroup = new ActionButtonGroup(this, topMid);

	assert(!p_profiles.empty());
	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	saSelector = new SingleAxisSelector( 3, waitFrames, 2, waitModeThresh, 0, 0 );

	currProfile = p_profiles.front(); //KIN 

	selectedProfileText.setFont(MainMenu::GetInstance()->arial);
	selectedProfileText.setCharacterSize(30);
	selectedProfileText.setFillColor(Color::Black);
	
	selectedProfileText.setString(currProfile->name);

	selectedProfileText.setOrigin(selectedProfileText.getLocalBounds().left + selectedProfileText.getLocalBounds().width / 2,
		selectedProfileText.getLocalBounds().top + selectedProfileText.getLocalBounds().height / 2);
	selectedProfileText.setPosition(p_topMid.x, p_topMid.y + 90);

	SetupBoxes();

	for( int i = 0; i < NUM_BOXES; ++i )
	{
		profileNames[i].setFont( font );
		profileNames[i].setCharacterSize( 40 );
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

	std::string buttonTexts[] = { "JUMP", "DASH", "ATTACK", "POWER3", "POWER4",
		"POWER5", "POWER6LEFT", "POWER6RIGHT" };
	UIControl **controls = new UIControl*[2*4];
	UIButton *currButton;
	for( int i = 0; i < 2; ++i  )
	{
		for( int j = 0; j < 4; ++j )
		{
			currButton = new UIButton( NULL, this, &MainMenu::GetInstance()->tilesetManager,
				&MainMenu::GetInstance()->arial, buttonTexts[i*4+j], 220, 80 );
			currButton->bar->SetTextHeight( 20 );
			currButton->bar->SetTextAlignment( UIBar::Alignment::MIDDLE, Vector2i( 0, 3 ) );
			controls[i*4+j] = currButton;
		}
	}
	
	int quarter = 1920 / 4;
	//editProfileGrid = new UIControlGrid( NULL, 2, 4, controls, 20, 20, true );
	//editProfileGrid->SetTopLeft( topMid.x - quarter/2 + 10, topMid.y + 10 );

	maxReceiveFrames = 240;
}

ControlProfileMenu::~ControlProfileMenu()
{
	delete actionButtonGroup;
	delete saSelector;
}

bool ControlProfileMenu::ButtonEvent( UIEvent eType,
		ButtonEventParams *param )
{
	if( eType == UIEvent::E_BUTTON_PRESSED )
	{
		string s = param->button->bar->GetString();
		state = S_RECEIVE_BUTTON;
		currReceiveFrame = 0;

		editIndex = ControlProfileManager::GetButtonTypeFromAction( s );
	}

	return true;
}

XBoxButton ControlProfileMenu::ReceiveInput( ControllerState &currInput, 
	ControllerState &prevInput )
{
	if( currReceiveFrame < maxReceiveFrames )
	{
		if( currInput.A && !prevInput.A )
		{
			return XBoxButton::XBOX_A;
		}
		else if( currInput.B && !prevInput.B )
		{
			return XBoxButton::XBOX_B;
		}
		else if( currInput.X && !prevInput.X )
		{
			return XBoxButton::XBOX_X;
		}
		else if( currInput.Y && !prevInput.Y )
		{
			return XBoxButton::XBOX_Y;
		}
		else if( currInput.leftShoulder && !prevInput.leftShoulder )
		{
			return XBoxButton::XBOX_L1;
		}
		else if( currInput.rightShoulder && !prevInput.rightShoulder )
		{
			return XBoxButton::XBOX_R1;
		}
		else if( currInput.LeftTriggerPressed() && !prevInput.LeftTriggerPressed() )
		{
			return XBoxButton::XBOX_L2;
		}
		else if( currInput.RightTriggerPressed() && !prevInput.RightTriggerPressed() )
		{
			return XBoxButton::XBOX_R2;
		}
		
		++currReceiveFrame;
		return XBoxButton::XBOX_BLANK;
		
	}
	else
	{
		return XBoxButton::XBOX_BLANK;
		//assert( 0 );
	}
}

void ControlProfileMenu::Draw( sf::RenderTarget *target )
{
	if( state == S_SHOWING_OPTIONS )
	{
		target->draw( boxes, NUM_BOXES*4,sf::Quads );
		for( int i = 0; i < NUM_BOXES; ++i )
		{
			//cout << "drawing: " << profileNames[i].getString().toAnsiString() << "\n";
			target->draw( profileNames[i] );
		}
		vSlider.Draw(target);
	}
	else if( state == S_EDIT_CONFIG )
	{
		actionButtonGroup->Draw(target);
		//editProfileGrid->Draw( target );
	}
	else if (state == State::S_SELECTED)
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


	list<ControlProfile*>::iterator lit = profiles.begin();
	if( topIndex > profiles.size() )
	{
		topIndex = profiles.size() - 1;
	}

	for( int i = 0; i < topIndex; ++i )
	{
		++lit;
	}

	int trueI;
	int i = 0;
	int numProfiles = profiles.size();
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

		profileNames[i].setString( (*lit)->name );
		profileNames[i].setOrigin( profileNames[i].getLocalBounds().width / 2, 0 );
			//profileNames[i].getLocalBounds().height / 2 );
		//profileNames[i].setPosition( topMid.x, topMid.y + (BOX_HEIGHT+BOX_SPACING) * i );
		profileNames[i].setPosition( topMid.x, topMid.y + (BOX_HEIGHT+BOX_SPACING) * i );

		++lit;
	}

	saSelector->totalItems = numProfiles;

	Vector2f offset(20, 0);
	vSlider.Setup(Vector2f(topMid.x + BOX_WIDTH / 2 + offset.x, topMid.y + offset.y), 
		Vector2f(vSlider.barSize.x, max((vSlider.selectorSize.y / numProfiles), 5.f) ), vSlider.selectorSize);

	vSlider.SetSlider((float)saSelector->currIndex / (saSelector->totalItems - 1));
}

void ControlProfileMenu::SetTopMid(Vector2f &tm)
{
	topMid = tm;
	//SetupBoxes(); //only need this if i was going to be moving the boxes while the selector isup
	selectedProfileText.setPosition(topMid.x, topMid.y + 90);
}

void ControlProfileMenu::Update( ControllerDualStateQueue *controllerInput )
{	
	if(controllerInput->ButtonPressed_A())
	{
		switch( state )
		{
		case S_SELECTED:
			state = S_SHOWING_OPTIONS;
			UpdateNames();
			oldCurrIndex = saSelector->currIndex;

			break;
		case S_SHOWING_OPTIONS:
			{
				int test = 0;
				state = S_SELECTED;

				for( list<ControlProfile*>::iterator it = profiles.begin(); 
					it != profiles.end(); ++it )
				{
					if( test == saSelector->currIndex )
					{
						currProfile = (*it);
						break;
					}
					++test;
				}

				selectedProfileText.setString(currProfile->name);
				/*selectedProfileText.setOrigin(  
					selectedProfileText.getLocalBounds().width / 2 
					- selectedProfileText.getLocalBounds().left, 
					selectedProfileText.getLocalBounds().height );*/
				selectedProfileText.setOrigin(selectedProfileText.getLocalBounds().left + selectedProfileText.getLocalBounds().width / 2,
					selectedProfileText.getLocalBounds().top + selectedProfileText.getLocalBounds().height / 2);
				//selectedProfileText.setPosition(selectedProfileText.getPosition());
				break;
			}
		case S_EDIT_CONFIG:
			{
				break;
			}
		}
	}
	else if( controllerInput->ButtonPressed_RightShoulder())
	{
		switch( state )
		{
		case S_SELECTED:
			{
			//commented out for now because using mapselectionmenu
				/*if (section->parent->state == MapSelectionMenu::State::S_MULTI_SCREEN)
				{
					if (section->parent->multiSelectorState == MapSelectionMenu::MS_NEUTRAL
						|| section->parent->multiSelectorState == MapSelectionMenu::MS_GHOST)
					{
						state = S_GHOST_SELECTOR;
					}
				}*/
				break;
			}
		case S_SHOWING_OPTIONS:
		{
			//state = S_SELECTED;
			state = S_EDIT_CONFIG;
			
			/*XBoxButton *fil = currProfile->GetCurrFilter();
			for (int i = 0; i < ControllerSettings::BUTTONTYPE_Count; ++i)
			{
				tempFilter[i] = fil[i];
			}*/
			break;
		}
		case S_EDIT_CONFIG:
			{
				//save
				bool res = SaveCurrConfig();
				assert( res ); //if failed, file write failed
				break;
			}
		}
	}
	else if (controllerInput->ButtonPressed_Y())
	{
		switch (state)
		{
		case S_SELECTED:

			//commented out for now because using mapselectionmenu
			/*if (section->parent->state == MapSelectionMenu::State::S_MULTI_SCREEN)
			{
				if (section->parent->multiSelectorState == MapSelectionMenu::MS_NEUTRAL
					|| section->parent->multiSelectorState == MapSelectionMenu::MS_MUSIC )
				{
					state = S_MUSIC_SELECTOR;
				}
			}
			else
			{

			}*/
			
			break;
		}
	}
	if( controllerInput->ButtonPressed_B() )
	{
		switch (state)
		{
		case S_SHOWING_OPTIONS:
		{
			saSelector->currIndex = oldCurrIndex;
			state = S_SELECTED;
			break;
		}
		case S_MUSIC_SELECTOR:
			state = S_SELECTED;
			break;
		case S_GHOST_SELECTOR:
			state = S_SELECTED;
			break;
		}
		
			
		
	}

	//tomorrow: set up the edit profile grid to draw in a separate state from a selected
	//profile. then make a popup window where you input a button to change your controls.
	//editProfileGrid->Update( currInput, prevInput );

	if( state == S_SHOWING_OPTIONS )
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
	else if( state == S_EDIT_CONFIG )
	{
		actionButtonGroup->Update(controllerInput);
		//editProfileGrid->Update( currInput, prevInput );
	}
	else if( state == S_RECEIVE_BUTTON )
	{
		//XBoxButton but = ReceiveInput( currInput, prevInput );
		/*if( but == XBoxButton::XBOX_BLANK )
		{
			if( currReceiveFrame == maxReceiveFrames )
			{
				state = S_EDIT_CONFIG;
			}
		}
		else
		{
			tempFilter[editIndex] = but;
			state = S_EDIT_CONFIG;
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


	state = S_SELECTED;
	return true;
}

void ControlProfileMenu::SetupBoxes()
{
	sf::Vector2f currTopMid;
	int extraHeight = 0;
	
	for( int i = 0; i < NUM_BOXES; ++i )
	{
		currTopMid = topMid + Vector2f( 0, extraHeight );

		boxes[i*4+0].position = Vector2f( currTopMid.x - BOX_WIDTH / 2, currTopMid.y );
		boxes[i*4+1].position = Vector2f( currTopMid.x + BOX_WIDTH / 2, currTopMid.y );
		boxes[i*4+2].position = Vector2f( currTopMid.x + BOX_WIDTH / 2, currTopMid.y + BOX_HEIGHT );
		boxes[i*4+3].position = Vector2f( currTopMid.x - BOX_WIDTH / 2, currTopMid.y + BOX_HEIGHT );

		boxes[i*4+0].color = Color::Red;
		boxes[i*4+1].color = Color::Red;
		boxes[i*4+2].color = Color::Red;
		boxes[i*4+3].color = Color::Red;

		extraHeight += BOX_HEIGHT + BOX_SPACING;
	}

	Vector2f offset(20, 0);
	vSlider.Setup(Vector2f(topMid.x + BOX_WIDTH / 2 + offset.x, topMid.y + offset.y), Vector2f(30, 0),
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
	for( list<ControlProfile*>::iterator it = profiles.begin(); it != profiles.end(); ++it )
	{
		delete (*it);
	}
	profiles.clear();
}

bool ControlProfileManager::LoadProfiles()
{
	ClearProfiles();

	ControlProfile *def = new ControlProfile;
	def->name = "KIN_Default";
	profiles.push_front(def);

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
			profiles.push_back( newProfile );

			while (true)
			{

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
						res = LoadXBOXConfig(newProfile);
						if (!res)
						{
							newProfile->SetFilterDefault();
							//SetFilterDefault(newProfile->filter);
							assert(0);
							return false;
						}
					}
					else if (inputTypeName == INPUT_TYPE_KEYBOARD)
					{
						//TODO
					}
					else if (inputTypeName == INPUT_TYPE_GAMECUBE)
					{
						res = LoadGamecubeConfig(newProfile);
						if (!res)
						{
							//SetFilterDefaultGCC(newProfile->gccFilter);
							assert(0);
							return false;
						}
					}
				}
				else
				{
					return true;
				}
			}
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
	for( list<ControlProfile*>::iterator it = profiles.begin(); it != profiles.end(); ++it )
	{
		cout << "profile: " << (*it)->name << endl;
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
char ControlProfileManager::MoveToEquals( std::string &outStr )
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

		if( c == EQUALS )
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

bool ControlProfileManager::LoadXBOXConfig( ControlProfile *profile ) 
{
	string inputName;
	string buttonStr;

	while( true )
	{

	char ec = MoveToEquals( inputName );
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

	
	char mod = 0;
	switch( c )
	{
	case 'a':
	case 'A':
		buttonStr = "A";
		break;
	case 'b':
	case 'B':
		buttonStr = "B";
		break;
	case 'x':
	case 'X':
		buttonStr = "X";
		break;
	case 'y':
	case 'Y':
		buttonStr = "Y";
		break;
	case 'r':
	case 'R':

		if( !is.get( mod ) )
			return false;
		if( mod == '1' )
			buttonStr = "R1";
		else if( mod == '2' )
			buttonStr = "R2";
		else
		{
			assert( 0 );
			return false;
		}
		break;
	case 'l':
	case 'L':
		if( !is.get( mod ) )
			return false;
		if( mod == '1' )
			buttonStr = "L1";
		else if( mod == '2' )
			buttonStr = "L2";
		break;
	}

	XBoxButton b = GetButton( buttonStr );
	ControllerSettings::ButtonType buttonType = GetButtonTypeFromAction( inputName );

	assert( buttonType < ControllerSettings::BUTTONTYPE_Count );

	profile->filter[buttonType] = b;

	}
}

bool ControlProfileManager::LoadGamecubeConfig(ControlProfile *profile)
{
	string inputName;
	string buttonStr;

	while (true)
	{

		char ec = MoveToEquals(inputName);
		char c = 0;

		if (ec == 0)
			return true;

		if (IsSymbol(ec))
		{
			return true;
		}


		if (!is.get(c))
		{
			return false;
		}


		char mod = 0;
		switch (c)
		{
		case 'a':
		case 'A':
			buttonStr = "A";
			break;
		case 'b':
		case 'B':
			buttonStr = "B";
			break;
		case 'x':
		case 'X':
			buttonStr = "X";
			break;
		case 'y':
		case 'Y':
			buttonStr = "Y";
			break;
		case 'r':
		case 'R':

			if (!is.get(mod))
				return false;
			if (mod == '1')
				buttonStr = "R1";
			else if (mod == '2')
				buttonStr = "R2";
			else
			{
				assert(0);
				return false;
			}
			break;
		case 'l':
		case 'L':

			if (!is.get(mod))
				return false;
			if (mod == '1')
				buttonStr = "L1";
			else if (mod == '2')
				buttonStr = "L2";
			else
			{
				assert(0);
				return false;
			}
			break;
		}

		XBoxButton b = GetButton(buttonStr);
		ControllerSettings::ButtonType buttonType = GetButtonTypeFromAction(inputName);

		assert(buttonType < ControllerSettings::BUTTONTYPE_Count);

		//profile->gccFilter[buttonType] = b;
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
	else if( inputName == "POWER6RIGHT" )
	{
		buttonType = ControllerSettings::BUTTONTYPE_RIGHTWIRE;
	}
	else if( inputName == "POWER6LEFT" )
	{
		buttonType = ControllerSettings::BUTTONTYPE_LEFTWIRE;
	}
	else
	{
		assert( 0 );
	}

	return buttonType;
}

XBoxButton ControlProfileManager::GetButton( const std::string &str )
{
	if( str == "A" )
	{
		return XBoxButton::XBOX_A;
	}
	else if( str == "B" )
	{
		return XBoxButton::XBOX_B;
	}
	else if( str == "X" )
	{
		return XBoxButton::XBOX_X;
	}
	else if( str == "Y" )
	{
		return XBoxButton::XBOX_Y;
	}
	else if( str == "R1" )
	{
		return XBoxButton::XBOX_R1;
	}
	else if( str == "L1" )
	{
		return XBoxButton::XBOX_L1;
	}
	else if( str == "R2" )
	{
		return XBoxButton::XBOX_R2;
	}
	else if( str == "L2" )
	{
		return XBoxButton::XBOX_L2;
	}
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
	profiles.erase( it );

	WriteProfiles();
}

//true if found, false if not found
bool ControlProfileManager::MoveToNextSymbolText( char startSymbol, char endSymbol, std::string &outStr )
{
	const int MAX_NAME_LENGTH = 8;
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



void ControlProfileManager::WriteFilter( ofstream &of, XBoxButton *filter)
{
	of << "JUMP=" << GetXBoxButtonString(filter[ControllerSettings::BUTTONTYPE_JUMP]) << "\n";
	of << "DASH=" << GetXBoxButtonString(filter[ControllerSettings::BUTTONTYPE_DASH]) << "\n";
	of << "ATTACK=" << GetXBoxButtonString(filter[ControllerSettings::BUTTONTYPE_ATTACK]) << "\n";
	of << "SHIELD=" << GetXBoxButtonString(filter[ControllerSettings::BUTTONTYPE_SHIELD]) << "\n";
	of << "POWER6LEFT=" << GetXBoxButtonString(filter[ControllerSettings::BUTTONTYPE_LEFTWIRE]) << "\n";
	of << "POWER6RIGHT=" << GetXBoxButtonString(filter[ControllerSettings::BUTTONTYPE_RIGHTWIRE]) << "\n";
	of << "SPECIAL=" << GetXBoxButtonString(filter[ControllerSettings::BUTTONTYPE_SPECIAL]) << "\n";
	of << "PAUSE=" << GetXBoxButtonString(filter[ControllerSettings::BUTTONTYPE_PAUSE]) << "\n";

	of << "\n";
}

void ControlProfileManager::WriteInputType(ofstream &of, const std::string &inputType)
{
	of << INPUT_TYPE_START_CHAR << inputType << INPUT_TYPE_END_CHAR << "\n";
}

void ControlProfileManager::WriteProfiles()
{
	ofstream of;
	of.open( "Resources/controlprofiles.txt" );

	list<ControlProfile*>::iterator it = profiles.begin();
	++it; //always skip KIN

	for( ; it != profiles.end(); ++it )
	{
		of << PROFILE_START_CHAR << (*it)->name << PROFILE_END_CHAR << "\n";
		
		WriteInputType(of, INPUT_TYPE_XBOX);

		WriteFilter(of, (*it)->filter );

		WriteInputType(of, INPUT_TYPE_GAMECUBE);

		//WriteFilter(of, (*it)->gccFilter);

		of << "\n";
	}
}

ControlProfileManager::~ControlProfileManager()
{
	ClearProfiles();
}

ProfileSelector::ProfileSelector(MainMenu *p_mainMenu,
	sf::Vector2f &p_topMid)
	:topMid(p_topMid), cpm(p_mainMenu->cpm), mainMenu(p_mainMenu )
{
	state = S_SELECTED;
	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 0, 0);
	topIndex = 0;
	
	currProfile = cpm->profiles.front(); //KIN 

	selectedProfileText.setFont(p_mainMenu->arial);
	selectedProfileText.setCharacterSize(40);
	selectedProfileText.setFillColor(Color::White);

	selectedProfileText.setString(currProfile->name);

	selectedProfileText.setOrigin(selectedProfileText.getLocalBounds().left + selectedProfileText.getLocalBounds().width / 2,
		0 );
	selectedProfileText.setPosition(topMid.x, topMid.y);

	SetupBoxes();

	for (int i = 0; i < NUM_BOXES; ++i)
	{
		profileNames[i].setFont(p_mainMenu->arial);
		profileNames[i].setCharacterSize(40);
		profileNames[i].setFillColor(Color::White);
	}

	UpdateNames();
}

ProfileSelector::~ProfileSelector()
{
	delete saSelector;
}

bool ProfileSelector::SetCurrProfileByName(const string &name)
{
	int i = 0;
	for (auto it = cpm->profiles.begin(); it != cpm->profiles.end(); ++it)
	{
		if (name == (*it)->name)
		{
			currProfile = (*it);
			saSelector->currIndex = i;
			topIndex = i;
			oldCurrIndex = i;
			UpdateNames();
			selectedProfileText.setString(currProfile->name);
			return true;
		}
		++i;
	}

	return false;
	//set the current profile to the profile named name.
	//then in gamesession we use the same code on the current profile that we did before
}

void ProfileSelector::UpdateNames()
{
	list<ControlProfile*>::iterator lit = cpm->profiles.begin();
	if (topIndex > cpm->profiles.size())
	{
		topIndex = cpm->profiles.size() - 1;
	}

	for (int i = 0; i < topIndex; ++i)
	{
		++lit;
	}

	int trueI;
	int i = 0;
	int numProfiles = cpm->profiles.size();
	for (; i < NUM_BOXES; ++i)
	{
		trueI = (topIndex + i) % NUM_BOXES;
		if (i == numProfiles)
		{
			for (; i < NUM_BOXES; ++i)
			{
				profileNames[i].setString("");
			}
			break;
		}

		if (lit == cpm->profiles.end())
			lit = cpm->profiles.begin();

		profileNames[i].setString((*lit)->name);
		profileNames[i].setOrigin(profileNames[i].getLocalBounds().width / 2, 0);
		//profileNames[i].getLocalBounds().height / 2 );
		//profileNames[i].setPosition( topMid.x, topMid.y + (BOX_HEIGHT+BOX_SPACING) * i );
		profileNames[i].setPosition(topMid.x, topMid.y + (BOX_HEIGHT + BOX_SPACING) * i);

		++lit;
	}

	saSelector->totalItems = numProfiles;

	Vector2f offset(20, 0);
	vSlider.Setup(Vector2f(topMid.x + BOX_WIDTH / 2 + offset.x, topMid.y + offset.y),
		Vector2f(vSlider.barSize.x, max((vSlider.selectorSize.y / numProfiles), 5.f)), vSlider.selectorSize);

	vSlider.SetSlider((float)saSelector->currIndex / (saSelector->totalItems - 1));
}

void ProfileSelector::Draw(sf::RenderTarget *target)
{
	if (state == S_SHOWING_OPTIONS)
	{
		target->draw(boxes, NUM_BOXES * 4, sf::Quads);
		for (int i = 0; i < NUM_BOXES; ++i)
		{
			target->draw(profileNames[i]);
		}
		vSlider.Draw(target);
	}
	else if (state == State::S_SELECTED)
	{
		target->draw(selectedProfileText);
	}
}

void ProfileSelector::MoveUp()
{
	topIndex++;
	if (topIndex == cpm->profiles.size())
	{
		topIndex = 0;
	}
}

void ProfileSelector::MoveDown()
{
	topIndex--;
	if (topIndex == -1)
	{
		topIndex = cpm->profiles.size() - 1;
	}
}

void ProfileSelector::SetupBoxes()
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

	Vector2f offset(20, 0);
	vSlider.Setup(Vector2f(topMid.x + BOX_WIDTH / 2 + offset.x, topMid.y + offset.y), Vector2f(30, 0),
		Vector2f(30, NUM_BOXES * (BOX_HEIGHT + BOX_SPACING)));
}


void ProfileSelector::Update(ControllerDualStateQueue *controllerInput)
{
	switch (state)
	{
	case S_SELECTED:
	{
		if (controllerInput->ButtonPressed_A())
		{
			state = S_SHOWING_OPTIONS;
			UpdateNames();
			oldCurrIndex = saSelector->currIndex;
		}
		break;
	}
	case S_SHOWING_OPTIONS:
	{
		if (controllerInput->ButtonPressed_A())
		{
			int test = 0;
			state = S_SELECTED;

			for (list<ControlProfile*>::iterator it = cpm->profiles.begin();
				it != cpm->profiles.end(); ++it)
			{
				if (test == saSelector->currIndex)
				{
					currProfile = (*it);
					break;
				}
				++test;
			}

			selectedProfileText.setString(currProfile->name);
			selectedProfileText.setOrigin(selectedProfileText.getLocalBounds().left
				+ selectedProfileText.getLocalBounds().width / 2, 0);
		}

		break;
	}
	}

	if (state == S_SHOWING_OPTIONS)
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
			if (cIndex == saSelector->totalItems - 1)
				topIndex = saSelector->totalItems - NUM_BOXES;
			else if (cIndex < topIndex)
				topIndex = cIndex;
		}

		if (changed != 0)
		{
			int test = 0;
			for (list<ControlProfile*>::iterator it = cpm->profiles.begin();
				it != cpm->profiles.end(); ++it)
			{
				if (test == saSelector->currIndex)
				{
					currProfile = (*it);
					break;
				}
				++test;
			}

			UpdateNames();

			vSlider.SetSlider((float)saSelector->currIndex / (saSelector->totalItems - 1));
		}
		UpdateBoxColor();
	}
}

void ProfileSelector::UpdateBoxColor()
{
	Color c;
	int trueI = (saSelector->currIndex - topIndex);// % NUM_BOXES;
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
		SetRectColor(boxes + i * 4, c);
	}
}

bool ProfileSelector::SaveCurrConfig()
{
	bool different = false;
	
	//XBoxButton *fil = currProfile->GetCurrFilter();
	/*for (int i = 0; i < ControllerSettings::BUTTONTYPE_Count; ++i)
	{
		if (oldFilter[i] != tempFilter[i])
		{
			fil[i] = tempFilter[i];	
			different = true;
		}
		else
		{		
			fil[i] = oldFilter[i];
		}
	}*/

	if (different)
	{
		mainMenu->cpm->WriteProfiles();
	}
		

	state = S_SELECTED;
	return true;
}

ActionButtonGroup::ActionButtonGroup(ControlProfileMenu *p_controlMenu, sf::Vector2f &p_topLeft )
{
	controlMenu = p_controlMenu;

	topLeft = p_topLeft;

	selectedIndex = 0;

	std::string buttonTexts[] = { "JUMP", "DASH", "ATTACK", "POWER3", "POWER4",
		"SHIELD", "POWER5", "POWER6LEFT", "POWER6RIGHT" };

	int xSpacing = 20;
	int ySpacing = 20;
	int squareSize = 64;
	int x, y;
	for (int i = 0; i < NUM_BUTTONS; ++i)
	{
		x = i % COLS;
		y = i / COLS;
		actionButtons[i] = new ActionButton(buttonQuads + 4 * i, buttonTexts[i], topLeft + Vector2f(x * (squareSize + xSpacing), y * (squareSize + ySpacing)));
	}

	Reset();
}

ActionButtonGroup::~ActionButtonGroup()
{
	for (int i = 0; i < NUM_BUTTONS; ++i)
	{
		delete actionButtons[i];
	}
}

void ActionButtonGroup::Reset()
{
	action = A_SELECT_BUTTON;

	SetSelectedIndex(0);
}

void ActionButtonGroup::SetSelectedIndex(int sel)
{
	selectedIndex = sel;

	SetRectColor(highlightQuad, Color::Red);
	float bSize = actionButtons[selectedIndex]->buttonSize;
	float border = 16;
	SetRectCenter(highlightQuad, bSize + border, bSize + border, actionButtons[selectedIndex]->quadCenter );
}

void ActionButtonGroup::Update(ControllerDualStateQueue *controllerInput)
{
	switch (action)
	{
	case A_SELECT_BUTTON:
	{
		int x = selectedIndex % COLS;
		int y = selectedIndex / COLS;

		if (controllerInput->GetPrevState().IsLeftNeutral())
		{


			if (controllerInput->DirPressed_Left())
			{
				--x;
				if (x < 0)
				{
					x = COLS - 1;
				}
			}
			else if (controllerInput->DirPressed_Right())
			{
				++x;
				if (x == COLS)
				{
					x = 0;
				}
			}

			if (controllerInput->DirPressed_Up())
			{
				--y;
				if (y < 0)
				{
					y = ROWS - 1;
				}
			}
			else if (controllerInput->DirPressed_Down())
			{
				++y;
				if (y == ROWS)
				{
					y = 0;
				}
			}
		}

		int tempIndex = y * COLS + x;

		if (tempIndex != selectedIndex)
		{
			SetSelectedIndex(tempIndex);
		}
		break;
	}
	case A_MODIFY_BUTTON:
	{
		break;
	}
	}
}

void ActionButtonGroup::Draw(sf::RenderTarget *target)
{
	MainMenu *mm = MainMenu::GetInstance();
	
	Tileset *ts_buttons = mm->GetButtonIconTileset(controlMenu->currControllerType);

	target->draw(highlightQuad, 4, sf::Quads);

	target->draw(buttonQuads, 4 * NUM_BUTTONS, sf::Quads, ts_buttons->texture);

	for (int i = 0; i < NUM_BUTTONS; ++i)
	{
		actionButtons[i]->Draw(target);
	}
}

ActionButton::ActionButton(sf::Vertex *p_quad, const std::string &name, sf::Vector2f &pos)
{
	position = pos;
	quad = p_quad;

	buttonSize = 64;

	MainMenu *mm = MainMenu::GetInstance();

	actionName.setFont(mm->arial);
	actionName.setCharacterSize(20);
	actionName.setFillColor(Color::White);
	actionName.setString(name);
	auto lb = actionName.getLocalBounds();
	actionName.setOrigin(lb.left + lb.width / 2, 0);
	actionName.setPosition(pos);

	quadCenter = pos + Vector2f(0, 25 + buttonSize / 2);
	SetRectCenter(quad, buttonSize, buttonSize, quadCenter);
	
}

void ActionButton::SetButtonSubRect(sf::IntRect &ir)
{
	SetRectSubRect(quad, ir);
}

void ActionButton::Draw(sf::RenderTarget *target)
{
	target->draw(actionName);
}