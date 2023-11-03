#ifndef __SCOREDISPLAY_H__
#define __SCOREDISPLAY_H__

#include "SFML/Graphics.hpp"

struct Tileset;
struct Session;

struct ScoreDisplay;
struct GameSession;
struct FeedbackInputBox;

struct ScoreBar
{
	enum State
	{
		NONE,
		POP_OUT,
		SHEET_APPEAR,
		SYMBOL_DISPLAY,
		SHEET_DISPLAY,
		RETRACT,
		S_Count
	};

	int stateLength[S_Count];
	State state;
	int frame;
	float xDiffPos;
	ScoreDisplay *parent;

	sf::Sprite barSprite;
	sf::Sprite symbolSprite;
	sf::Text text;

	sf::Sprite extraSymbolSprite;
	sf::Text extraText;

	sf::Color textColor;
	int row;

	ScoreBar(int row, ScoreDisplay *parent);
	void SetBarPos(float xDiff);
	void PopOut();
	void Retract();
	void Update();
	void Reset();
	void Draw(sf::RenderTarget *target);
	void SetText(const std::string &str,
		sf::Color c);
	
	
	//sf::Text info;

};

struct SelectBar
{
	enum State
	{
		NONE,
		POP_OUT,
		DISPLAY,
		RETRACT,
		S_Count
	};

	int stateLength[S_Count];
	State state;
	int frame;
	int row;
	float xDiffPos;
	ScoreDisplay *parent;
	sf::Text extraText;

	sf::Sprite barSprite;
	sf::Sprite buttonIconSprite;

	SelectBar(int row, ScoreDisplay *parent);
	void SetBarPos(float xDiff);
	void PopOut();
	void Retract();
	void Update();
	void Reset();
	void Draw(sf::RenderTarget *target);
};

struct ScoreDisplay
{
	bool active;
	bool waiting;

	FeedbackInputBox *feedbackInputBox;

	sf::Vertex nameBGQuad[4];
	sf::Text levelNameText;

	GameSession *game;
	sf::Sprite score;
	static const int NUM_BARS = 3;
	ScoreBar *bars[NUM_BARS];

	static const int NUM_SELECT_BARS = 6;
	SelectBar *selectBars[NUM_SELECT_BARS];

	int numShownSelectBars;

	bool madeRecord;

	int includeExtraSelectBars;

	Session *sess;
	Tileset *ts_score;
	Tileset *ts_scoreIcons;

	sf::Vector2f basePos;
	float selectOffset;

	sf::Font &font;

	ScoreDisplay(sf::Vector2f &position,
		sf::Font & testFont);
	~ScoreDisplay();

	int GetNumSelectBars();
	void Draw(sf::RenderTarget *target);
	int numEnemiesTotal;
	int numEnemiesKilled;
	double numSeconds;
	void Reset();
	void Activate();
	void Deactivate();
	void PopOutBar(int row);
	void PopOutSelectBars();
	void Update();
};

#endif