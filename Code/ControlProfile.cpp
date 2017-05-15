#include "ControlProfile.h"
#include <sstream>
#include <assert.h>
#include <iostream>

using namespace std;

#define PROFILE_START_CHAR '<'
#define PROFILE_END_CHAR '>'
#define INPUT_TYPE_START_CHAR '['
#define INPUT_TYPE_END_CHAR ']'
#define INPUT_TYPE_XBOX "XBOX"
#define INPUT_TYPE_KEYBOARD "KEYBOARD"
#define EQUALS '='


bool ControlProfileManager::LoadProfiles()
{
	is.open( "controlprofiles.txt" );

	if( is.is_open() )
	{
		string profileName;
		string inputTypeName;
		while( MoveToNextSymbolText( PROFILE_START_CHAR, PROFILE_END_CHAR, profileName ) )
		{
			cout << "new profile: " << profileName << "\n";
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

	WriteProfiles(); //debug
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
	ControllerSettings::ButtonType buttonType = ControllerSettings::Count;
	cout << "inputName: " << inputName << endl;
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

	assert( buttonType < ControllerSettings::Count );

	profile->filter[buttonType] = b;

	}
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



