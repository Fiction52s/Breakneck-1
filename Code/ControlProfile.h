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

	void FilterState(ControllerState &state);
	XBoxButton Filter(ControllerSettings::ButtonType b);
	int GetControllerType();
	void CopyTo(ControlProfile *cp);

	void SetFilterDefault();
private:
	int cType;
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


	enum Action
	{
		A_SELECTED,
		A_SHOWING_OPTIONS
	};

	int action;
	ControlProfileManager *cpm;
	XBoxButton tempFilter[ControllerSettings::BUTTONTYPE_Count];
	XBoxButton oldFilter[ControllerSettings::BUTTONTYPE_Count];
	sf::Vertex boxes[NUM_BOXES * 4];
	sf::Text profileNames[NUM_BOXES];
	VertSlider vSlider;
	sf::Vector2f topLeft;
	int topIndex;
	MainMenu *mainMenu;
	ControlProfile *currProfile;
	SingleAxisSelector *saSelector;


	ProfileSelector();
	~ProfileSelector();
	void SetTopLeft(sf::Vector2f &topLeft);
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
	
	ActionButton(sf::Vertex *quad, const std::string &name);
	void SetPosition(sf::Vector2f &pos);
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

	//const static int NUM_BUTTONS = 9;
	//const static int COLS = 3;
	//const static int ROWS = 3;

	sf::Vertex *buttonQuads;
	sf::Vertex highlightQuad[4];
	
	std::vector<ActionButton*> actionButtons;

	ControlProfileMenu *controlMenu;

	int selectedIndex;

	int action;

	int numButtons;
	int rows;
	int cols;

	sf::Vector2f topLeft;

	ActionButtonGroup(ControlProfileMenu *p_controlMenu);
	~ActionButtonGroup();

	void Reset();

	void UpdateButtonIcons();

	void SetTopLeft(sf::Vector2f &pos);

	void SetSelectedIndex(int sel);

	void SetModifiedButton(XBoxButton button);

	void ModifySelectedButton();

	void Update();
	

	void Draw(sf::RenderTarget *target);
};

struct MainMenu;
struct ControlProfileMenu : UIEventHandlerBase
{
	enum Action
	{
		A_SELECTED,
		A_SHOWING_OPTIONS,
		A_EDIT_PROFILE,
		A_REPLACE_BUTTON,
		A_Count
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
	int action;
	sf::Vector2f topLeft;
	int oldCurrIndex;
	int topIndex;
	

	std::list<ControlProfile*> profiles;
	sf::Font font;

	VertSlider vSlider;


	SingleAxisSelector *saSelector;


	sf::Text selectedProfileText;


	ActionButtonGroup *actionButtonGroup;

	ControlProfile *currProfile;
	ControlProfile *tempProfile;
	ControllerDualStateQueue *controllerInput;


	ControlProfileMenu(std::list<ControlProfile*> &p_profiles);
	~ControlProfileMenu();
	XBoxButton ReceiveInput( ControllerState &currInput, 
	ControllerState &prevInput );
	bool SaveCurrConfig();
	bool ButtonEvent( UIEvent eType,
		ButtonEventParams *param );
	ControlProfile *GetProfileAtIndex(int ind);
	void SetControllerInput(ControllerDualStateQueue *controllerInput);
	void SetupBoxes();
	void SetTopLeft(sf::Vector2f &p_topLeft);
	void Update();
	void MoveUp();
	void MoveDown();
	void UpdateNames();
	void UpdateBoxesDebug();
	void BeginSelectingProfile();
	void Draw( sf::RenderTarget *target );
};

#endif