#ifndef __PAUSE_MENU_H__
#define __PAUSE_MENU_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "Input.h"
#include "Movement.h"

struct GameSession;
struct ShardMenu;
struct ControlSettingsMenu;
struct ColorShifter;
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
struct SingleAxisSelector;
struct OptionsMenu
{

	MainMenu *mainMenu;

	//left bar
	bool autoUseController;
	int controllerIconsIndex;
	int useIconsIndex;
	int keyboardSchemeIndex;
	int controllerSchemeIndex;
	int useKeyboardSchemeIndex;
	int useControllerSchemeIndex;


	//modify
	int modifyIndex;

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
	void SetButtonAssoc( int controlIndex, XBoxButton b );
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

	bool moveDown;
	bool moveUp;
	bool moveLeft;
	bool moveRight;

	int moveDelayCounter;
	int moveDelayFrames; 
	int moveDelayFramesSmall;
};

struct KinMenu
{
	SingleAxisSelector *xSelector;
	SingleAxisSelector *ySelector;
	ControlSettingsMenu *csm;
	int GetCurrIndex();
	~KinMenu();
	MainMenu *mainMenu;
	sf::Sprite selectorSpr;
	KinMenu(MainMenu *p_mainMenu,
		ControlSettingsMenu *csm);
	sf::Vertex powerQuads[10 * 4];
	sf::Vertex powerQuadsBG[10 * 4];
	ColorShifter *selectedShifter;
	ColorShifter *aura1AShifter;
	ColorShifter *aura1BShifter;
	ColorShifter *aura2AShifter;
	ColorShifter *aura2BShifter;
	ColorShifter *bgShifter;
	Tileset *ts_powers;
	Tileset *ts_kin;
	Tileset *ts_aura1A;
	Tileset *ts_aura1B;
	Tileset *ts_aura2A;
	Tileset *ts_aura2B;
	Tileset *ts_veins;
	int frame;
	sf::Shader scrollShader1;
	sf::Shader scrollShader2;
	//sf::Shader scrollShader1;
	sf::Color Get1AColor();
	sf::Color Get1BColor();
	sf::Color Get2AColor();
	sf::Color Get2BColor();
	sf::Sprite tutorialSpr;
	sf::Vertex descriptionBox[4];
	void UpdateCommandButton();
	Tileset *ts_xboxButtons;
	Tileset *ts_currentButtons;
	sf::Sprite commandSpr;
	Tileset *ts_tutorial[10];
	sf::Sprite kinSpr;
	sf::Sprite veinSpr;
	sf::Sprite aura1ASpr;
	sf::Sprite aura1BSpr;
	sf::Sprite aura2ASpr;
	sf::Sprite aura2BSpr;
	sf::Vertex kinBG[4];
	void Update(ControllerState &curr,
		ControllerState &prev);
	void UpdateDescription();
	void UpdateSelector();
	void UpdateTutorial();
	void UpdatePowerSprite();
	std::string powerDescriptions[10];
	void Draw(sf::RenderTarget *target);
	sf::Text description;
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

	PauseMenu( MainMenu *p_mainMenu );
	~PauseMenu();
	void Draw( sf::RenderTarget *target );
	void SetTab( Tab t );
	void TabLeft();
	void TabRight();
	void ApplyVideoSettings();
	void ApplySoundSettings( );
	
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

	UpdateResponse Update(
		ControllerState &currInput,
		ControllerState &prevInput );
	Tileset *ts_background[Count];
	Tileset *ts_select;
	GameSession *owner;

	ShardMenu *shardMenu;
	KinMenu *kinMenu;

	sf::Sprite bgSprite;
	sf::Sprite selectSprite;
	MainMenu *mainMenu;


	ControlSettingsMenu *controlSettingsMenu;

	
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