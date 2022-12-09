#ifndef __GAME_SETTINGS_SCREEN_H__
#define __GAME_SETTINGS_SCREEN_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "MatchParams.h"
#include "GUI.h"

struct MainMenu;

struct GameSettingsScreen : TilesetManager, GUIHandler
{
	enum Action
	{
		A_ACTIVE,
		A_CONFIRM,
		A_CANCEL,
	};

	int action;
	int frame;
	MainMenu *mainMenu;
	sf::Vertex bgQuad[4];

	Panel *panel;	

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
	void ApplyConfig();
	void ButtonCallback(Button *b,
		const std::string &e);

private:
	void CreateResolutionDropdown();
	void CreateWindowModeDropdown();
};

#endif