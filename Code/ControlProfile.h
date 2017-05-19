#ifndef __CONTROL_PROFILE__
#define __CONTROL_PROFILE__
#include "Input.h"
#include <string>
#include <fstream>
#include <list>
#include <SFML\Graphics.hpp>

struct MainMenu;

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
	~ControlProfileManager();
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
	void ClearProfiles();

	//std::list<ControlProfile*> profiles;
	std::ifstream is;
};

struct UIControlGrid;
struct MultiSelectionSection;
struct ControlProfileMenu
{
	enum State
	{
		S_SELECTED,
		S_SHOWING_OPTIONS,
		S_EDIT_CONFIG,
		S_Count
	};

	
	//TODO scrollbar to show how far in to the names you are
	static const int NUM_BOXES = 7;
	static const int BOX_WIDTH;
	static const int BOX_HEIGHT;
	static const int BOX_SPACING;
	

	ControlProfileMenu( MultiSelectionSection *section, 
		int playerIndex,
		std::list<ControlProfile*> &p_profiles );
	void SetupBoxes();
	void Update( ControllerState &currInput,
		ControllerState &prevInput );
	void MoveUp();
	void MoveDown();
	void UpdateNames();
	void UpdateBoxesDebug();
	void Draw( sf::RenderTarget *target );

	sf::Vertex boxes[NUM_BOXES*4];
	sf::Text profileNames[NUM_BOXES];
	State state;
	sf::Vector2f topMid;

	int currIndex;
	int oldCurrIndex;
	MultiSelectionSection *section;
	int topIndex;
	
	std::list<ControlProfile*> profiles;
	int playerIndex;
	sf::Font font;

	int waitFrames[3];
	int waitModeThresh[2];
	int framesWaiting;
	int currWaitLevel;
	int flipCounterUp;
	int flipCounterDown;

	UIControlGrid *editProfileGrid;
};

#endif