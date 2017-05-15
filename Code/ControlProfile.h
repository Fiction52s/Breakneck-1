#ifndef __CONTROL_PROFILE__
#define __CONTROL_PROFILE__
#include "Input.h"
#include <string>
#include <fstream>
#include <list>

struct ControlProfile
{
	//you must make a named profile to store
	//controls
	ControlProfile();
	
	std::string name;
	XBoxButton filter[ControllerSettings::Count];
	bool hasXBoxFilter;
};

struct ControlProfileManager
{

	bool LoadProfiles();
	void DebugPrint();
	std::list<ControlProfile*> profiles;
private:
	bool MoveToNextSymbolText( char startSymbol,
		char endSymbol, std::string &outStr );
	bool MoveToPeekNextOpener( char &outChar );
	char MoveToEquals( std::string &outStr );
	bool LoadXBOXConfig( ControlProfile *profile );
	XBoxButton GetButton( const std::string &str );
	std::string GetString( XBoxButton button );
	bool IsSymbol( char c );
	void DeleteProfile( std::list<ControlProfile*>::iterator &it );
	void WriteProfiles();

	//std::list<ControlProfile*> profiles;
	std::ifstream is;
};

#endif