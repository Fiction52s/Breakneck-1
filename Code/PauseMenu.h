#ifndef __PAUSE_MENU_H__
#define __PAUSE_MENU_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "Input.h"
#include "Movement.h"

struct GameSession;

struct OptionSelector
{
	static sf::Font *font;	
	OptionSelector( sf::Vector2f &pos,
		int optionCount,
		std::string *options ); 

	void Right();
	void Left();
	void Stop();
	const std::string & GetString();
	void Update();
	void Draw( sf::RenderTarget *target );
	bool selected;

	std::string *options;
	int optionCount;
	int currentIndex;
	sf::Text currentText;
	sf::Vector2f pos;

	int maxWaitFrames;
	int minWaitFrames;
	int framesWaiting;
	int maxMomentum;
	int currWaitFrames;
	CubicBezier accelBez;
	int momentum; //left is -1
	//right is 1, none is 0
};


struct PauseMenu;
struct MainMenu;
struct OptionsMenu
{
	MainMenu *mainMenu;
	bool autoUseController;
	int controllerIconsIndex;
	int useIconsIndex;
	int keyboardSchemeIndex;
	int controllerSchemeIndex;
	int useKeyboardSchemeIndex;
	int useControllerSchemeIndex;
	//enum checkboxtypes
	//CheckBox cb[checkboxtypes::count];

	enum LeftBarOptions
	{
		L_AUTOCONTROLLER,
		L_CONTROLLER_ICONS,
		L_KB_OPTIONS,
		L_CONTROLLER_OPTIONS,
		L_Count
	};

	int leftBarCurr;

	enum Mode
	{
		LEFTBAR,
		MODIFY_KEYBOARD_CONTROLS,
		MODIFY_XBOX_CONTROLS,
		Count
	};

	enum InputOptions
	{
		INPUT_TYPE,
		SETTINGS
	};

	
	sf::Vector2f basePos;
	Mode mode;
	OptionsMenu( MainMenu *mainMenu );
	OptionsMenu( PauseMenu *pauseMenu );
	void LoadControlOptions();
	void SetAssocSymbols( bool kb );

	void InitAssocSymbols();
	

	bool Update( ControllerState &currInput,
		ControllerState &prevInput );

	void SaveControlOptions();
	void UpdateXboxButtonIcons(
		int controlSetIndex );
	void UpdateButtonIcons();

	void Draw( sf::RenderTarget *target );
	XBoxButton CheckXBoxInput( ControllerState &currInput );
	Tileset *ts_xboxButtons;
	sf::VertexArray buttonVA;
	sf::VertexArray controlIconVA;
	sf::VertexArray schemeKeyboardVA;
	sf::VertexArray schemeControllerVA;
	void UpdateControlIcons();
	void UpdateSchemeVA( bool kb );
	ControllerTypes::Type controllerType;
	Tileset *ts_currentButtons;
	
	Tileset *ts_actionIcons;

	sf::Text actionText[15];
	XBoxButton xboxInputAssoc[3][ControllerSettings::ButtonType::Count];
	int actionIndex;
	bool selectingProfile;

	
	//more of these for diff controller types
	sf::VertexArray assocSymbols;

	
	int selectedIndex;
	
	std::string *possibleControllerActions;
	std::string *possibleKeyboardActions;

	OptionSelector **inputSelectors;
	OptionSelector **currentSelectors;
	int currInputIndex;

	int optionSelectorIndex;

	int maxWaitFrames;
	int currWaitFrames;
	int minWaitFrames;
	int framesWaiting;
	int momentum;
	int maxMomentum;
	CubicBezier accelBez;
};

struct PauseMenu
{
	OptionsMenu *cOptions;
	enum Tab
	{
		MAP,
		KIN,
		SHARDS,
		OPTIONS,
		PAUSE,
		Count
	};

	PauseMenu( GameSession *owner );
	~PauseMenu();
	void Draw( sf::RenderTarget *target );
	void SetTab( Tab t );
	void TabLeft();
	void TabRight();
	void ApplyVideoSettings();
	void ApplySoundSettings();
	
	enum OptionType
	{
		O_VIDEO,
		O_AUDIO,
		O_INPUT,
		O_Count
	};
	OptionType optionType;

	enum UpdateResponse
	{
		R_NONE,
		R_P_RESUME,
		R_P_RESPAWN,
		R_P_EXITLEVEL,
		R_P_EXITTITLE,
		R_P_EXITGAME
	};

	UpdateResponse Update( ControllerState &currInput,
		ControllerState &prevInput );
	Tileset *ts_background[Count];
	Tileset *ts_select;
	
	

	sf::Sprite bgSprite;
	sf::Sprite selectSprite;
	GameSession *owner;

	
	Tab currentTab;

	//bool show;
	//if you tab away it resets

	OptionSelector **videoSelectors;
	OptionSelector **soundSelectors;
	int numVideoOptions;
	int numSoundOptions;
	//map tab
	sf::Vector2f mapCenter;
	float mapZoomFactor;

	int pauseSelectIndex;

	int optionSelectorIndex;

	OptionSelector **currentSelectors;
	int numCurrentSelectors;

	int maxWaitFrames;
	int currWaitFrames;
	int minWaitFrames;
	int framesWaiting;
	int momentum;
	int maxMomentum;
	CubicBezier accelBez;

	

	
	
	

	
	UpdateResponse UpdateOptions(
		ControllerState &currInput,
	ControllerState &prevInput );
	UpdateResponse UpdateInputOptions(
		ControllerState &currInput,
		ControllerState &prevInput);
	UpdateResponse UpdateVideoOptions(
		ControllerState &currInput,
		ControllerState &prevInput);
	UpdateResponse UpdateAudioOptions(
		ControllerState &currInput,
		ControllerState &prevInput);
	
	
	/*int maxWaitFrames;
	int currWaitFrames;
	int minWaitFrames;
	int framesWaiting;
	int momentum;
	int maxMomentum;*/
};

#endif