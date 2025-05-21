#ifndef __GAME_SETTINGS_SCREEN_H__
#define __GAME_SETTINGS_SCREEN_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "MatchParams.h"
#include "GUI.h"

struct MainMenu;
struct NewTitleScreen;
struct SingleAxisSelector;

struct SettingsModule
{
	sf::Vector2f pos;

	bool selected;
	sf::Text text;

	SettingsModule();
	virtual ~SettingsModule(){}
	virtual void Reset() = 0;
	virtual void Update() = 0;
	virtual void Draw(sf::RenderTarget* target) = 0;
	virtual void SetTopLeft(sf::Vector2f pos) = 0;
	virtual void SetCenter(sf::Vector2f pos) = 0;
	virtual void CheckLeftRight() {}
	virtual void Press(){}
};

struct SettingsSlider : SettingsModule
{
	sf::Vertex bodyQuad[4];
	Tileset *ts_sliderBody;
	Tileset *ts_marker;

	int minValue;
	int maxValue;
	int defaultValue;

	SingleAxisSelector *saSelector;

	sf::Text valueText;

	sf::Vertex underQuads[2 * 4];

	SettingsSlider(TilesetManager *tm, const std::string &name, int minVal, int maxVal, int defaultVal);
	~SettingsSlider();
	void Reset();
	void SetTopLeft(sf::Vector2f pos);
	void SetCenter(sf::Vector2f pos);
	void CheckLeftRight();
	void Update();
	void Draw(sf::RenderTarget *target);
};

struct SettingsSwitch : SettingsModule
{
	sf::Vertex bodyQuad[4];
	sf::Vertex onOffQuad[4];
	Tileset *ts_switchBody;
	Tileset *ts_switchOnOff;
	bool on;
	bool hasBody;

	SettingsSwitch(TilesetManager *tm, const std::string &name, bool hasBody = true );
	void Reset();
	void SetTopLeft(sf::Vector2f pos);
	void SetCenter(sf::Vector2f pos);
	void Update();
	void Press();
	void Swap();
	void Draw(sf::RenderTarget *target);
};

struct SettingsSelector : SettingsModule
{
	Tileset *ts_switchBody;
	Tileset *ts_arrow;
	sf::Vertex arrowQuads[4 * 2];
	sf::Vertex selectorQuad[4];
	std::vector<std::string> options;
	SingleAxisSelector *saSelector;

	SettingsSelector(TilesetManager *tm, std::vector<std::string> &p_options);
	~SettingsSelector();
	void Reset();
	void SetTopLeft(sf::Vector2f pos);
	void SetCenter(sf::Vector2f p_pos);
	void Update();
	void CheckLeftRight();
	void Draw(sf::RenderTarget *target);
};

struct VideoSettingsPanel
{
	enum SelectorTypes
	{
		ST_DISPLAY,
		ST_RESOLUTION,
		ST_Count
	};

	Panel *panel;
	SingleAxisSelector *saSelector;
	MainMenu *mainMenu;

	sf::Text displayText;
	sf::Text resolutionText;
	sf::Text vSyncText;

	std::vector<SettingsModule*> modules;

	VideoSettingsPanel();
	~VideoSettingsPanel();

	void Start();
	void Update();
	void Draw(sf::RenderTarget *target);
};

struct GameSettingsPanel
{
	Panel *panel;
	SingleAxisSelector *saSelector;
	MainMenu *mainMenu;

	std::vector<SettingsModule*> modules;
	const static int NUM_SWITCHES = 4;
	SettingsSwitch *switches[NUM_SWITCHES];

	GameSettingsPanel();
	~GameSettingsPanel();
	void Start();
	void Update();
	void UpdateSwitches();
	void Draw(sf::RenderTarget *target);
};

struct AudioSettingsPanel
{
	Panel *panel;
	SingleAxisSelector *saSelector;
	MainMenu *mainMenu;

	std::vector<SettingsModule*> modules;
	sf::Text volumeText;


	AudioSettingsPanel();
	~AudioSettingsPanel();

	void Start();
	void Update();
	void Draw(sf::RenderTarget *target);
};

struct GameSettingsScreen : TilesetManager, GUIHandler
{
	enum Action
	{
		A_ACTIVE,
		A_CONFIRM,
		A_CANCEL,
	};

	GameSettingsPanel *gsPanel;
	VideoSettingsPanel *vsPanel;
	AudioSettingsPanel *asPanel;

	Tileset *ts_frame;
	sf::Vertex frameQuad[4];

	int action;
	int frame;
	MainMenu *mainMenu;

	Panel *panel;	
	NewTitleScreen *nts;

	Dropdown *resolutionDropdown;
	Dropdown *windowModeDropdown;
	Slider *musicVolumeSlider;
	Slider *soundVolumeSlider;
	Button *defaultButton;
	Button *applyButton;
	Button *backButton;
	Button *checkForControllerButton;
	Label *resolutionLabel;
	Label *windowModeLabel;
	CheckBox *showFPSCheckBox;
	CheckBox *showRunningTimerCheckBox;
	CheckBox *showTerrainLinesCheckBox;

	std::vector<sf::Vector2i> resolutions;
	std::vector<int> windowModes;

	GameSettingsScreen(MainMenu *mm);
	~GameSettingsScreen();

	void UpdateFromConfig();
	void Start();
	void Quit();
	bool HandleEvent(sf::Event ev);
	void DrawPopupBG(sf::RenderTarget *target);
	void Update();
	void Draw(sf::RenderTarget *target);
	void CancelCallback(Panel *p);
	void ConfirmCallback(Panel *p);
	void SetAction(int a);
	void ButtonCallback(Button *b,
		const std::string &e);

private:
	void CreateResolutionDropdown();
	void CreateWindowModeDropdown();
};

#endif