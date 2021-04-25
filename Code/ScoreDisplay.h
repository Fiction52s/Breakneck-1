#ifndef __SCOREDISPLAY_H__
#define __SCOREDISPLAY_H__

#include "SFML/Graphics.hpp"

struct Tileset;
struct Session;

struct ScoreDisplay;

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

	sf::Sprite score;
	static const int NUM_BARS = 3;
	ScoreBar *bars[NUM_BARS];

	static const int NUM_SELECT_BARS = 3;
	SelectBar *selectBars[NUM_SELECT_BARS];

	Session *sess;
	Tileset *ts_score;

	sf::Vector2f basePos;
	float selectOffset;

	sf::Font &font;

	ScoreDisplay(sf::Vector2f &position,
		sf::Font & testFont);
	~ScoreDisplay();

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