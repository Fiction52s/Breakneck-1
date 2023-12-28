#ifndef __PARALLEL_PRACTICE_SETTINGS_MENU_H__
#define __PARALLEL_PRACTICE_SETTINGS_MENU_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
//#include "MatchParams.h"
#include "GUI.h"

struct MainMenu;

struct ParallelPracticeSettingsMenu : TilesetManager, GUIHandler
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

	CheckBox *showKinsOnMinimapCheckBox;
	CheckBox *showKinsOnPauseMapCheckBox;
	CheckBox *showLobbyInCornerCheckBox;

	Button *defaultButton;
	Button *applyButton;
	Button *backButton;

	ParallelPracticeSettingsMenu(MainMenu *mm);
	~ParallelPracticeSettingsMenu();

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
};

#endif