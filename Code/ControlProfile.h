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
	std::string name;
	XBoxButton filter[ControllerSettings::BUTTONTYPE_Count];

	ControlProfile();
	void SetFilterDefault();
};

struct ControlProfileManager
{
	std::list<ControlProfile*> profiles;

	~ControlProfileManager();
	bool LoadProfiles();
	void DebugPrint();
	static XBoxButton GetButton( const std::string &str );
	static ControllerSettings::ButtonType 
		GetButtonTypeFromAction( const std::string &str );
	void WriteProfiles();
private:
	std::ifstream is;

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
};

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
	ControlProfileManager *cpm;
	XBoxButton tempFilter[ControllerSettings::BUTTONTYPE_Count];
	XBoxButton oldFilter[ControllerSettings::BUTTONTYPE_Count];
	sf::Vertex boxes[NUM_BOXES * 4];
	sf::Text profileNames[NUM_BOXES];
	VertSlider vSlider;
	sf::Vector2f topMid;
	int topIndex;
	MainMenu *mainMenu;
	ControlProfile *currProfile;
	SingleAxisSelector *saSelector;


	ProfileSelector( MainMenu *p_mainMenu,
		sf::Vector2f &topMid );
	~ProfileSelector();
	void UpdateNames();
	bool SetCurrProfileByName(const std::string &name);
	bool SaveCurrConfig();
	void Draw(sf::RenderTarget *target);
	void SetupBoxes();
	void MoveUp();
	void MoveDown();
	void Update(ControllerDualStateQueue *controllerInput);
	void UpdateBoxColor();
	sf::Text selectedProfileText;
	int oldCurrIndex;
};



struct ActionButton
{
	sf::Text actionName;
	sf::Vertex *quad;

	sf::Vector2f position;
	sf::Vector2f quadCenter;

	float buttonSize;
	
	ActionButton( sf::Vertex *quad, const std::string &name, sf::Vector2f &pos);
	void SetButtonSubRect( sf::IntRect &ir );
	void Draw(sf::RenderTarget *target);
};

struct ControlProfileMenu;
struct ActionButtonGroup
{
	enum Action
	{
		A_SELECT_BUTTON,
		A_MODIFY_BUTTON,
	};

	const static int NUM_BUTTONS = 9;
	const static int COLS = 3;
	const static int ROWS = 3;

	sf::Vertex buttonQuads[NUM_BUTTONS * 4];
	sf::Vertex highlightQuad[4];
	
	ActionButton *actionButtons[NUM_BUTTONS];

	ControlProfileMenu *controlMenu;

	int selectedIndex;

	int action;

	sf::Vector2f topLeft;

	ActionButtonGroup(ControlProfileMenu *p_controlMenu, sf::Vector2f &p_topLeft );
	~ActionButtonGroup();

	void Reset();

	void SetSelectedIndex(int sel);

	void Update(ControllerDualStateQueue *controllerInput);
	

	void Draw(sf::RenderTarget *target);
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

	static const int NUM_BOXES = 7;
	static const int BOX_WIDTH;
	static const int BOX_HEIGHT;
	static const int BOX_SPACING;

	int currReceiveFrame;
	int maxReceiveFrames;
	ControllerSettings::ButtonType editIndex;
	XBoxButton tempFilter[ControllerSettings::BUTTONTYPE_Count];
	ControllerType tempCType;
	sf::Vertex boxes[NUM_BOXES * 4];
	sf::Text profileNames[NUM_BOXES];
	State state;
	sf::Vector2f topMid;
	int oldCurrIndex;
	int topIndex;

	std::list<ControlProfile*> profiles;
	sf::Font font;

	VertSlider vSlider;

	ControlProfile *currProfile;
	SingleAxisSelector *saSelector;

	int currControllerType;

	sf::Text selectedProfileText;


	ActionButtonGroup *actionButtonGroup;


	ControlProfileMenu(std::list<ControlProfile*> &p_profiles,
		sf::Vector2f &p_topMid);
	~ControlProfileMenu();
	XBoxButton ReceiveInput( ControllerState &currInput, 
	ControllerState &prevInput );
	bool SaveCurrConfig();
	void SetTopMid(sf::Vector2f &tm);
	bool ButtonEvent( UIEvent eType,
		ButtonEventParams *param );
	void SetupBoxes();
	void Update( ControllerDualStateQueue *controllerInput );
	void MoveUp();
	void MoveDown();
	void UpdateNames();
	void UpdateBoxesDebug();
	void Draw( sf::RenderTarget *target );
};

#endif