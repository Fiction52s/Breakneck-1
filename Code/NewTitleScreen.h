#ifndef __NEW_TITLE_SCREEN_H__
#define __NEW_TITLE_SCREEN_H__

#include <SFML/Graphics.hpp>
#include <list>
#include "Tileset.h"
#include "GUI.h"
#include <string>

struct MainMenu;
struct Background;
struct ScrollingBackground;
struct MusicInfo;
struct CreditsMenuScreen;

struct FeedbackForm;
struct PostPracticeMatchMenu;
struct SingleAxisSelector;
struct RecordedAnimation;
struct Movie;

struct NewTitleScreen : TilesetManager, GUIHandler
{
	enum Action
	{
		A_IDLE,
		A_CHANGE_DOWN,
		A_CHANGE_UP,
		A_Count
	};

	Tileset *ts_backpack;
	const static int NUM_BUTTONS = 7;

	sf::Vertex highlightQuads[2 * 4];

	sf::Vertex backpackQuad[4];

	std::vector<Movie*> movies;
	Movie *currMovie;

	int actionLength[A_Count];

	bool isCursorModeOn;

	Panel *panel;
	
	MainMenu *mainMenu;
	CreditsMenuScreen *creditsMenuScreen;

	int action;
	int frame;
	MusicInfo *titleMusic;

	sf::Shader scrollShader;

	Tileset *ts_socials;

	Tileset *ts_bg;
	sf::Vertex bgQuad[4];

	sf::Vertex buttonQuads[4 * NUM_BUTTONS];
	Tileset *ts_buttons;

	sf::Color tintColors[NUM_BUTTONS];
	sf::Color oldColor;
	sf::Color currTint;
	int colorFadeFrame;
	int colorFadeTotalFrames;

	sf::Vertex extraButtonQuad[4];

	float xRate;
	float yRate;
	float quantX;
	float quantY;

	sf::Vector2f buttonPositions[NUM_BUTTONS];
	float buttonScales[NUM_BUTTONS];

	float offScreenButtonScale;
	sf::Vector2f offScreenTopButtonPos;
	sf::Vector2f offScreenBottomButtonPos;
	sf::Vector2f baseButtonSize;


	SingleAxisSelector *saSelector;
	bool isMusicStarted;

	int pressedIndex;
	

	NewTitleScreen(MainMenu *p_mainMenu);
	~NewTitleScreen();
	void ChooseRectEvent(ChooseRect *cr, int eventType);
	void ButtonCallback(Button *b, const std::string & e);
	void Draw(sf::RenderTarget *target);
	int GetSelectedIndex();
	int GetPressedIndex();
	void UpdateBGColor();
	void SetBGColor(sf::Color c);
	void Reset();
	void Update();
};

#endif