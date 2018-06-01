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


ControlProfileMenu::ControlProfileMenu( MultiSelectionSection *p_section,
	int p_playerIndex, list<ControlProfile*> &p_profiles, Vector2f &p_topMid )
	:profiles( p_profiles ), playerIndex( p_playerIndex ), 
	font( p_section->parent->mainMenu->arial ),
	topIndex( 0 ), state( S_SELECTED ), oldCurrIndex( 0 ), 
	section( p_section ), topMid( p_topMid )
{
	assert(!p_profiles.empty());
	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	saSelector = new SingleAxisSelector( 3, waitFrames, 2, waitModeThresh, 0, 0 );

	currProfile = p_profiles.front(); //KIN 

	selectedProfileText.setFont(section->mainMenu->arial);
	selectedProfileText.setCharacterSize(30);
	selectedProfileText.setFillColor(Color::Black);
	
	selectedProfileText.setString(currProfile->name);

	selectedProfileText.setOrigin(selectedProfileText.getLocalBounds().width / 2, 
		selectedProfileText.getLocalBounds().height / 2);
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
			currButton = new UIButton( NULL, this, &section->mainMenu->tilesetManager,
				&section->mainMenu->arial, buttonTexts[i*4+j], 220, 80 );
			currButton->bar->SetTextHeight( 20 );
			currButton->bar->SetTextAlignment( UIBar::Alignment::MIDDLE, Vector2i( 0, 3 ) );
			controls[i*4+j] = currButton;
		}
	}
	
	int quarter = 1920 / 4;
	editProfileGrid = new UIControlGrid( NULL, 2, 4, controls, 20, 20, true );
	editProfileGrid->SetTopLeft( topMid.x - quarter/2 + 10, topMid.y + 10 );

	maxReceiveFrames = 240;

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
		editProfileGrid->Draw( target );
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

void ControlProfileMenu::Update( ControllerState &currInput,
		ControllerState &prevInput )
{
	if( currInput.A && !prevInput.A )
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
				selectedProfileText.setOrigin(selectedProfileText.getLocalBounds().width / 2,
					selectedProfileText.getLocalBounds().height / 2);
				//selectedProfileText.setPosition(selectedProfileText.getPosition());
				break;
			}
		case S_EDIT_CONFIG:
			{
				break;
			}
		}
	}
	else if( currInput.rightShoulder && !prevInput.rightShoulder)
	{
		switch( state )
		{
		case S_SELECTED:
			{
				if (section->parent->state == MapSelectionMenu::State::S_MULTI_SCREEN)
				{
					if (section->parent->multiSelectorState == MapSelectionMenu::MS_NEUTRAL
						|| section->parent->multiSelectorState == MapSelectionMenu::MS_GHOST)
					{
						state = S_GHOST_SELECTOR;
					}
				}
				break;
			}
		case S_SHOWING_OPTIONS:
			//state = S_SELECTED;
			state = S_EDIT_CONFIG;
			for (int i = 0; i < ControllerSettings::Count; ++i)
			{
				tempFilter[i] = currProfile->filter[i];
			}
			break;
		case S_EDIT_CONFIG:
			{
				//save
				bool res = SaveCurrConfig();
				assert( res ); //if failed, file write failed
				break;
			}
		}
	}
	else if (currInput.Y && !prevInput.Y)
	{
		switch (state)
		{
		case S_SELECTED:
			if (section->parent->state == MapSelectionMenu::State::S_MULTI_SCREEN)
			{
				if (section->parent->multiSelectorState == MapSelectionMenu::MS_NEUTRAL
					|| section->parent->multiSelectorState == MapSelectionMenu::MS_MUSIC )
				{
					state = S_MUSIC_SELECTOR;
				}
			}
			else
			{

			}
			
			break;
		}
	}
	if( currInput.B && !prevInput.B )
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
		editProfileGrid->Update( currInput, prevInput );
	}
	else if( state == S_RECEIVE_BUTTON )
	{
		XBoxButton but = ReceiveInput( currInput, prevInput );
		if( but == XBoxButton::XBOX_BLANK )
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
		}
	}
}

bool ControlProfileMenu::SaveCurrConfig()
{
	bool different = false;
	for( int i = 0; i < ControllerSettings::Count; ++i )
	{
		if( currProfile->filter[i] != tempFilter[i] )
		{
			currProfile->filter[i] = tempFilter[i];
			different = true;
		}
	}

	if( different ) section->mainMenu->cpm->WriteProfiles();


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

	


	is.open( "controlprofiles.txt" );

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

			char opener = 0;
			bool res = MoveToPeekNextOpener( opener );

			char test = is.peek();

			if( opener == PROFILE_START_CHAR )
			{
				cout << "done with input types\n";
				continue;
			}
			else if( opener == INPUT_TYPE_START_CHAR )
			{
				res = MoveToNextSymbolText( INPUT_TYPE_START_CHAR, INPUT_TYPE_END_CHAR, inputTypeName );
				if( !res )
				{
					assert( 0 );
					return false;
				}
				cout << "input type: " << inputTypeName << "\n";
				if( inputTypeName == INPUT_TYPE_XBOX )
				{
					res = LoadXBOXConfig( newProfile );
					if( !res )
					{
						SetFilterDefault(newProfile->filter);
						assert( 0 );
						return false;
					}
				}
				else if( inputTypeName == INPUT_TYPE_KEYBOARD )
				{
					//TODO
				}
			}
			else
			{
				return true;
			}
		}
	}
	else
	{
	}

	//WriteProfiles(); //debug, will be in other functions
	//DeleteProfile( profiles.begin() );

	ControlProfile *def= new ControlProfile;
	SetFilterDefault( def->filter );
	def->hasXBoxFilter = true;
	def->name = "KIN Default";
	profiles.push_front( def );
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
	profile->hasXBoxFilter = true;
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

	assert( buttonType < ControllerSettings::Count );

	profile->filter[buttonType] = b;

	}
}

ControllerSettings::ButtonType ControlProfileManager::GetButtonTypeFromAction( 
	const std::string &inputName )
{
	ControllerSettings::ButtonType buttonType;
	if( inputName == "JUMP" )
	{
		buttonType = ControllerSettings::JUMP;
	}
	else if( inputName == "DASH" )
	{
		buttonType = ControllerSettings::DASH;
	}
	else if( inputName == "ATTACK" )
	{
		buttonType = ControllerSettings::ATTACK;
	}
	else if( inputName == "POWER3" )
	{
		buttonType = ControllerSettings::BOUNCE;
	}
	else if( inputName == "POWER4" )
	{
		buttonType = ControllerSettings::GRIND;
	}
	else if( inputName == "POWER5" )
	{
		buttonType = ControllerSettings::TIMESLOW;
	}
	else if( inputName == "POWER6RIGHT" )
	{
		buttonType = ControllerSettings::RIGHTWIRE;
	}
	else if( inputName == "POWER6LEFT" )
	{
		buttonType = ControllerSettings::LEFTWIRE;
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

ControlProfile::ControlProfile()
	:hasXBoxFilter( false )
{
	SetFilterDefault(filter);
}

void ControlProfileManager::WriteProfiles()
{
	ofstream of;
	of.open( "controlprofiles.txt" );

	XBoxButton *filter = NULL;
	list<ControlProfile*>::iterator it = profiles.begin();
	++it; //always skip KIN

	for( ; it != profiles.end(); ++it )
	{
		of << PROFILE_START_CHAR << (*it)->name << PROFILE_END_CHAR << "\n";
		if( (*it)->hasXBoxFilter )
		{
			of << INPUT_TYPE_START_CHAR << INPUT_TYPE_XBOX << INPUT_TYPE_END_CHAR << "\n";

			filter = (*it)->filter;

			of << "JUMP=" << GetXBoxButtonString( filter[ControllerSettings::JUMP] ) << "\n";
			of << "DASH=" << GetXBoxButtonString( filter[ControllerSettings::DASH] ) << "\n";
			of << "ATTACK=" << GetXBoxButtonString( filter[ControllerSettings::ATTACK] ) << "\n";
			of << "POWER3=" << GetXBoxButtonString( filter[ControllerSettings::BOUNCE] ) << "\n";
			of << "POWER4=" << GetXBoxButtonString( filter[ControllerSettings::GRIND] ) << "\n";
			of << "POWER5=" << GetXBoxButtonString( filter[ControllerSettings::TIMESLOW] ) << "\n";
			of << "POWER6LEFT=" << GetXBoxButtonString( filter[ControllerSettings::LEFTWIRE] ) << "\n";
			of << "POWER6RIGHT=" << GetXBoxButtonString( filter[ControllerSettings::RIGHTWIRE] ) << "\n\n";
		}

		of << "\n";
	}
}

ControlProfileManager::~ControlProfileManager()
{
	for( list<ControlProfile*>::iterator it = profiles.begin(); it != profiles.end(); ++it )
	{
		delete (*it);
	}
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

	selectedProfileText.setOrigin(selectedProfileText.getLocalBounds().width / 2,
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

void ProfileSelector::UpdateButtonIcons()
{

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


void ProfileSelector::Update(ControllerState &currInput,
	ControllerState &prevInput)
{
	if (currInput.A && !prevInput.A)
	{
		switch (state)
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
			selectedProfileText.setOrigin(selectedProfileText.getLocalBounds().width / 2,
				0 );
			break;
		}
		}
	}
	//else if (currInput.rightShoulder && !prevInput.rightShoulder)
	//{
	//	switch (state)
	//	{
	//	case S_SELECTED:
	//	{
	//		if (section->parent->state == MapSelectionMenu::State::S_MULTI_SCREEN)
	//		{
	//			if (section->parent->multiSelectorState == MapSelectionMenu::MS_NEUTRAL
	//				|| section->parent->multiSelectorState == MapSelectionMenu::MS_GHOST)
	//			{
	//				state = S_GHOST_SELECTOR;
	//			}
	//		}
	//		break;
	//	}
	//	case S_SHOWING_OPTIONS:
	//		//state = S_SELECTED;
	//		state = S_EDIT_CONFIG;
	//		for (int i = 0; i < ControllerSettings::Count; ++i)
	//		{
	//			tempFilter[i] = currProfile->filter[i];
	//		}
	//		break;
	//	case S_EDIT_CONFIG:
	//	{
	//		//save
	//		bool res = SaveCurrConfig();
	//		assert(res); //if failed, file write failed
	//		break;
	//	}
	//	}
	//}
	//else if (currInput.Y && !prevInput.Y)
	//{
	//	switch (state)
	//	{
	//	case S_SELECTED:
	//		if (section->parent->state == MapSelectionMenu::State::S_MULTI_SCREEN)
	//		{
	//			if (section->parent->multiSelectorState == MapSelectionMenu::MS_NEUTRAL
	//				|| section->parent->multiSelectorState == MapSelectionMenu::MS_MUSIC)
	//			{
	//				state = S_MUSIC_SELECTOR;
	//			}
	//		}
	//		else
	//		{

	//		}

	//		break;
	//	}
	//}
	//if (currInput.B && !prevInput.B)
	//{
	//	switch (state)
	//	{
	//	case S_SHOWING_OPTIONS:
	//	{
	//		saSelector->currIndex = oldCurrIndex;
	//		state = S_SELECTED;
	//		break;
	//	}
	//	case S_MUSIC_SELECTOR:
	//		state = S_SELECTED;
	//		break;
	//	case S_GHOST_SELECTOR:
	//		state = S_SELECTED;
	//		break;
	//	}



	

	//tomorrow: set up the edit profile grid to draw in a separate state from a selected
	//profile. then make a popup window where you input a button to change your controls.
	//editProfileGrid->Update( currInput, prevInput );

	if (state == S_SHOWING_OPTIONS)
	{

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
			//cout << "currIndex: " << cIndex << ", topIndex: " << topIndex << endl;
			//controls[oldIndex]->Unfocus();
			//controls[focusedIndex]->Focus();

			vSlider.SetSlider((float)saSelector->currIndex / (saSelector->totalItems - 1));

		}
		UpdateBoxColor();
		//UpdateBoxesDebug();

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
	
	for (int i = 0; i < ControllerSettings::Count; ++i)
	{
		if (oldFilter[i] != tempFilter[i])
		{
			currProfile->filter[i] = tempFilter[i];
			different = true;
		}
		else
		{
			currProfile->filter[i] = oldFilter[i];
		}
	}

	if (different) mainMenu->cpm->WriteProfiles();

	state = S_SELECTED;
	return true;
}