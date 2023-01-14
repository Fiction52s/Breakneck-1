#ifndef __CONTROL_PROFILE__
#define __CONTROL_PROFILE__
#include "Input.h"
#include <string>
#include <fstream>
#include <list>
#include <SFML\Graphics.hpp>
#include "UIWindow.h"
#include "ItemSelector.h"

struct MainMenu;



struct ControlProfile
{
	//you must make a named profile to store
	//controls
	ControlProfile();
	
	
	std::string name;
	ControllerType tempCType;
	XBoxButton *GetCurrFilter();
	XBoxButton filter[ControllerSettings::BUTTONTYPE_Count];
	XBoxButton gccFilter[ControllerSettings::BUTTONTYPE_Count];
};

struct ControlProfileManager
{
	~ControlProfileManager();
	bool LoadProfiles();
	void DebugPrint();
	std::list<ControlProfile*> profiles;
	static XBoxButton GetButton( const std::string &str );
	static ControllerSettings::ButtonType 
		GetButtonTypeFromAction( const std::string &str );
	void WriteProfiles();
private:
	void WriteFilter( std::ofstream &of, XBoxButton *filter);
	void WriteInputType(std::ofstream &of, const std::string &inputType);
	bool MoveToNextSymbolText( char startSymbol,
		char endSymbol, std::string &outStr );
	bool MoveToPeekNextOpener( char &outChar );
	char MoveToEquals( std::string &outStr );
	bool LoadXBOXConfig( ControlProfile *profile );
	bool LoadGamecubeConfig(ControlProfile *profile);
	bool IsSymbol( char c );
	void DeleteProfile( std::list<ControlProfile*>::iterator &it );
	void ClearProfiles();

	//std::list<ControlProfile*> profiles;
	std::ifstream is;
};


//struct ScrollingVerticalSelector
//{
//	ScrollingVerticalSelector(MainMenu *mainMenu);
//	void Draw(sf::RenderTarget *target);
//	int numBoxes;
//	int boxWidth;
//	int boxHeight;
//	int boxSpacing;
//	void SetupBoxes();
//	sf::Vertex *boxes;// [NUM_BOXES * 4];
//	sf::Text *profileNames;// [NUM_BOXES];
//	sf::Vector2f topMid;
//	int topIndex;
//	int currIndex;
//	sf::Font &font;
//	MainMenu *mainMenu;
//};

struct ProfileSelector
{
	static const int NUM_BOXES = 7;

	static const int BOX_WIDTH;
	static const int BOX_HEIGHT;
	static const int BOX_SPACING;


	enum State
	{
		S_SELECTED,
		S_SHOWING_OPTIONS
	};

	State state;
	ProfileSelector( MainMenu *p_mainMenu,
		sf::Vector2f &topMid );
	~ProfileSelector();
	void UpdateNames();
	void UpdateButtonIcons();
	bool SetCurrProfileByName(const std::string &name);
	ControlProfileManager *cpm;
	bool SaveCurrConfig();
	XBoxButton tempFilter[ControllerSettings::BUTTONTYPE_Count];
	XBoxButton oldFilter[ControllerSettings::BUTTONTYPE_Count];
	
	void Draw(sf::RenderTarget *target);
	void SetupBoxes();
	sf::Vertex boxes[NUM_BOXES * 4];
	sf::Text profileNames[NUM_BOXES];
	//std::list<ControlProfile*> profiles;
	VertSlider vSlider;
	sf::Vector2f topMid;
	int topIndex;
	MainMenu *mainMenu;
	ControlProfile *currProfile;
	SingleAxisSelector *saSelector;
	void MoveUp();
	void MoveDown();
	void Update(ControllerState &currInput,
		ControllerState &prevInput);
	void UpdateBoxColor();
	sf::Text selectedProfileText;
	int oldCurrIndex;
};

struct MainMenu;
struct ControlProfileMenu : UIEventHandlerBase
{
	enum State
	{
		S_SELECTED,
		S_SHOWING_OPTIONS,
		S_EDIT_CONFIG,
		S_RECEIVE_BUTTON,
		S_MUSIC_SELECTOR,
		S_GHOST_SELECTOR,
		S_Count
	};

	int currReceiveFrame;
	int maxReceiveFrames;
	ControllerSettings::ButtonType editIndex;
	XBoxButton tempFilter[ControllerSettings::BUTTONTYPE_Count];
	ControllerType tempCType;

	//TODO scrollbar to show how far in to the names you are
	static const int NUM_BOXES = 7;
	static const int BOX_WIDTH;
	static const int BOX_HEIGHT;
	static const int BOX_SPACING;

	XBoxButton ReceiveInput( ControllerState &currInput, 
	ControllerState &prevInput );
	bool SaveCurrConfig();
	

	ControlProfileMenu( int playerIndex,
		std::list<ControlProfile*> &p_profiles,
		sf::Vector2f &p_topMid );
	~ControlProfileMenu();
	void SetTopMid(sf::Vector2f &tm);
	

	bool ButtonEvent( UIEvent eType,
		ButtonEventParams *param );
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

	//int currIndex;
	int oldCurrIndex;
	int topIndex;
	
	std::list<ControlProfile*> profiles;
	int playerIndex;
	sf::Font font;

	VertSlider vSlider;

	//int waitFrames[3];
	//int waitModeThresh[2];
	//int framesWaiting;
	//int currWaitLevel;
	//int flipCounterUp;
	//int flipCounterDown;
	ControlProfile *currProfile;
	SingleAxisSelector *saSelector;

	sf::Text selectedProfileText;
	UIControlGrid *editProfileGrid;
};

#endif