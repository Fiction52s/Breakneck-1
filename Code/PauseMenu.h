#ifndef __PAUSE_MENU_H__
#define __PAUSE_MENU_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "Input.h"
#include "Movement.h"

struct GameSettingsScreen;


struct GameSession;
struct ShardMenu;
struct LogMenu;
struct ColorShifter;
struct ControlProfile;
struct PaletteShader;
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
struct PlayerBoxGroup;

struct OptionsMenu
{
	enum State
	{
		CHOOSESTATE,
		CONTROL,
		SOUND,
		VISUAL,
		GAMEPLAY,
	};
	State state;
	GameSession *game;

	PlayerBoxGroup *playerBoxGroup;
	GameSettingsScreen *gameSettingsMenu;
	TilesetManager *tm;
	Tileset *ts_optionMode;
	sf::Vertex optionModeQuads[4*4];
	SingleAxisSelector *optionModeSelector;
	sf::Vector2f basePos;
	OptionsMenu( TilesetManager *tm );
	~OptionsMenu();
	void SetGame(GameSession *game);
	void HandleEvent(sf::Event ev);
	void Start();
	void Update( ControllerState &currInput,
		ControllerState &prevInput );
	void Draw( sf::RenderTarget *target );
	void UpdateOptionModeQuads();
	bool CanChangeTab();
};

struct GameSession;
struct KinMenu;
struct PauseMap;

struct PauseMenu
{
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

	PauseMenu(TilesetManager *tm);
	~PauseMenu();
	void SetGame(GameSession *p_game);
	void Draw( sf::RenderTarget *target );
	void ResetCounters();
	void SetTab( Tab t );
	void TabLeft();
	void TabRight();
	void UpdateButtonIconsWhenControllerIsChanged();
	void ApplyVideoSettings();
	void ApplySoundSettings( );
	bool CanChangeTab();
	void HandleEvent(sf::Event ev);
	void SetTopLeft(sf::Vector2f &pos);

	TilesetManager *tm;
	sf::Text debugText;
	sf::Text mapNameText;

	Tileset *ts_buttons;

	MainMenu *mainMenu;
	Tileset *ts_pauseOptions;
	sf::Vertex pauseOptionQuads[5 * 4];
	sf::Vertex respawnButtonIconQuad[4];

	
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
	Tileset *ts_background;
	

	PaletteShader *bgPaletteShader;

	Tileset *ts_select;

	ShardMenu *shardMenu;
	LogMenu *logMenu;
	KinMenu *kinMenu;

	sf::Sprite bgSprite;
	sf::Sprite tabSprite;
	Tileset *ts_tabs;
	sf::Sprite selectSprite;
	GameSession *game;

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
	PauseMap *pauseMap;

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
	
	/*int maxWaitFrames;
	int currWaitFrames;
	int minWaitFrames;
	int framesWaiting;
	int momentum;
	int maxMomentum;*/
};

#endif