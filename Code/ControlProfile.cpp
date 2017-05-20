#include "ControlProfile.h"
#include <sstream>
#include <assert.h>
#include <iostream>
#include "UIWindow.h"
#include "MainMenu.h"
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




ControlProfileMenu::ControlProfileMenu( MultiSelectionSection *p_section,
	int p_playerIndex, list<ControlProfile*> &p_profiles )
	:profiles( p_profiles ), playerIndex( p_playerIndex ), 
	font( p_section->parent->mainMenu->arial ),
	topIndex( 0 ), state( S_SELECTED ), currIndex( 0 ), oldCurrIndex( 0 ), 
	section( p_section )
{
	currProfile = p_profiles.front(); //KIN 

	int quarter = 1920 / 4;
	topMid = Vector2f( quarter * playerIndex + quarter / 2, 1080-400 );

	SetupBoxes();

	for( int i = 0; i < NUM_BOXES; ++i )
	{
		profileNames[i].setFont( font );
		profileNames[i].setCharacterSize( 40 );
		profileNames[i].setColor( Color::White );
	}

	waitFrames[0] = 10;
	waitFrames[1] = 5;
	waitFrames[2] = 2;

	waitModeThresh[0] = 2;
	waitModeThresh[1] = 2;

	currWaitLevel = 0;
	flipCounterUp = 0;
	flipCounterDown = 0;
	framesWaiting = 0;

	std::string buttonTexts[] = { "JUMP", "DASH", "ATTACK", "POWER3", "POWER4",
		"POWER5", "POWER6LEFT", "POWER6RIGHT" };
	UIControl **controls = new UIControl*[2*4];
	UIButton *currButton;
	for( int i = 0; i < 2; ++i  )
	{
		for( int j = 0; j < 4; ++j )
		{
			currButton = new UIButton( NULL, this, &section->parent->mainMenu->tilesetManager,
				&section->parent->mainMenu->arial, buttonTexts[i*4+j], 220, 80 );
			currButton->bar->SetTextHeight( 20 );
			currButton->bar->SetTextAlignment( UIBar::Alignment::MIDDLE, Vector2i( 0, 3 ) );
			controls[i*4+j] = currButton;
		}
	}

	editProfileGrid = new UIControlGrid( NULL, 2, 4, controls, 20, 20, true );
	editProfileGrid->SetTopLeft( topMid.x - quarter/2 + 10, topMid.y + 10 );

	maxReceiveFrames = 240;
}

bool ControlProfileMenu::ButtonEvent( UIEvent eType,
		ButtonEventParams *param )
{
	string s = param->button->bar->GetString();
	state = S_RECEIVE_BUTTON;
	currReceiveFrame = 0;

	editIndex = ControlProfileManager::GetButtonTypeFromAction( s );

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
	}
	else if( state == S_EDIT_CONFIG )
	{
		editProfileGrid->Draw( target );
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
			oldCurrIndex = currIndex;
			break;
		case S_SHOWING_OPTIONS:
			{
				int test = 0;
				state = S_SELECTED;
				for( list<ControlProfile*>::iterator it = profiles.begin(); 
					it != profiles.end(); ++it )
				{
					if( test == currIndex )
					{
						currProfile = (*it);
						break;
					}
					++test;
				}
				break;
			}
		case S_EDIT_CONFIG:
			{
				break;
			}
		}
	}
	else if( currInput.X && !prevInput.X )
	{
		switch( state )
		{
		case S_SELECTED:
			state = S_EDIT_CONFIG;
			break;
		case S_SHOWING_OPTIONS:
			state = S_SELECTED;
			
			break;
		case S_EDIT_CONFIG:
			{
				break;
			}
		}
	}
	if( currInput.B && !prevInput.B )
	{
		if( state == S_SHOWING_OPTIONS )
		{
			currIndex = oldCurrIndex;
			state = S_SELECTED;
		}
	}

	//tomorrow: set up the edit profile grid to draw in a separate state from a selected
	//profile. then make a popup window where you input a button to change your controls.
	//editProfileGrid->Update( currInput, prevInput );

	if( state == S_SHOWING_OPTIONS )
	{

		bool up = currInput.LUp();
		bool down = currInput.LDown();

		int oldIndex = currIndex;
		//bool consumed = controls[focusedIndex]->Update( curr, prev );

		if( down )
		{
			if( flipCounterDown == 0 
				|| ( flipCounterDown > 0 && framesWaiting == waitFrames[currWaitLevel] )
				)
			{
				if( flipCounterDown == 0 )
				{
					currWaitLevel = 0;
				}

				++flipCounterDown;

				if( flipCounterDown == waitModeThresh[currWaitLevel] && currWaitLevel < 2 )
				{
					currWaitLevel++;
				}

				flipCounterUp = 0;
				framesWaiting = 0;

				if( currIndex < profiles.size() - 1 )
				{
					currIndex++;

					if( currIndex - topIndex == NUM_BOXES )
						topIndex = currIndex - (NUM_BOXES-1);
				}
				else
				{
					currIndex = 0;
					topIndex = 0;
				}
			}
			else
			{
				++framesWaiting;
			}
		
		}
		else if( up )
		{
			if( flipCounterUp == 0 
				|| ( flipCounterUp > 0 && framesWaiting == waitFrames[currWaitLevel] )
				)
			{
				if( flipCounterUp == 0 )
				{
					currWaitLevel = 0;
				}

				++flipCounterUp;

				if( flipCounterUp == waitModeThresh[currWaitLevel] && currWaitLevel < 2 )
				{
					currWaitLevel++;
				}

				flipCounterDown = 0;
				framesWaiting = 0;
				if( currIndex > 0 )
				{
					currIndex--;

					if( currIndex < topIndex )
					{
						topIndex = currIndex;
					}
				}
				else
				{
					currIndex = profiles.size() - 1;
					topIndex = profiles.size() - NUM_BOXES;
				}
			}
			else
			{
				++framesWaiting;
			}
		
		}
		else
		{
			flipCounterUp = 0;
			flipCounterDown = 0;
			currWaitLevel = 0;
			framesWaiting = 0;
		}

		if( currIndex != oldIndex )
		{
			UpdateNames();
			cout << "currIndex: " << currIndex << ", topIndex: " << topIndex << endl;
			//controls[oldIndex]->Unfocus();
			//controls[focusedIndex]->Focus();
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
			currProfile->filter[editIndex] = but;
		}
	}
	
	
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
	int trueI = ( currIndex - topIndex );// % NUM_BOXES;
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

	WriteProfiles(); //debug, will be in other functions
	//DeleteProfile( profiles.begin() );

	ControlProfile *def= new ControlProfile;
	SetFilterDefault( def->filter );
	def->hasXBoxFilter = true;
	def->name = "KIN";
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
	for( list<ControlProfile*>::iterator it = profiles.begin(); it != profiles.end(); ++it )
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

