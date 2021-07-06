#ifndef __PAUSE_MENU_H__
#define __PAUSE_MENU_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "Input.h"
#include "Movement.h"

struct GameSession;
struct ShardMenu;
struct LogMenu;
struct ControlSettingsMenu;
struct ColorShifter;
struct ControlProfile;
struct OptionSelector
{
	static sf::Font *font;	
	OptionSelector( sf::Vector2f &pos,
		int optionCount,
		std::string *options ); 
	~OptionSelector();
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
struct GameSession;
struct OptionsMenu
{

	MainMenu *mainMenu;
	GameSession *game;

	enum State
	{
		CHOOSESTATE,
		CONTROL,
		SOUND,
		VISUAL,
		GAMEPLAY,
	};
	State state;

	Tileset *ts_optionMode;
	sf::Vertex optionModeQuads[4*4];
	SingleAxisSelector *optionModeSelector;
	sf::Vector2f basePos;
	OptionsMenu( PauseMenu *pauseMenu );
	~OptionsMenu();
	void Update( ControllerState &currInput,
		ControllerState &prevInput );
	void Draw( sf::RenderTarget *target );
	void UpdateOptionModeQuads();
	bool CanChangeTab();
	ControlSettingsMenu *csm;
};

struct GameSession;
struct KinMenu;
struct PauseMenu
{
	OptionsMenu *cOptions;
	enum Tab
	{
		PAUSE,
		MAP,
		SHARDS,
		LOGS,
		OPTIONS,
		KIN,
		Count
	};

	PauseMenu(GameSession *game);
	~PauseMenu();
	void Draw( sf::RenderTarget *target );
	void ResetCounters();
	void SetTab( Tab t );
	void TabLeft();
	void TabRight();
	void ApplyVideoSettings();
	void ApplySoundSettings( );
	bool CanChangeTab();

	sf::Text debugText;

	MainMenu *mainMenu;
	Tileset *ts_pauseOptions;
	sf::Vertex pauseOptionQuads[5 * 4];
	void UpdatePauseOptions();
	
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

	ShardMenu *shardMenu;
	LogMenu *logMenu;
	KinMenu *kinMenu;

	sf::Sprite bgSprite;
	sf::Sprite selectSprite;
	GameSession *game;
	ControlSettingsMenu *controlSettingsMenu;

	ControlProfile *GetCurrSelectedProfile();
	bool SetCurrProfileByName(const std::string &name);
	
	OptionsMenu *optionsMenu;
	
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

	SingleAxisSelector *pauseSelector;
	//int pauseSelectIndex;

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