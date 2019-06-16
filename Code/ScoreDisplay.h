#ifndef __SCOREDISPLAY_H__
#define __SCOREDISPLAY_H__

#include "SFML/Graphics.hpp"

struct Tileset;
struct GameSession;

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
		RETRACT
	};

	ScoreBar(int row, ScoreDisplay *parent,
		bool contBar = false );
	void SetBarPos(float xDiff);
	void SetSheetFrame(int frame);
	void SetSymbolTransparency(float f);
	void ClearSheet();
	void Update();
	void Reset();
	int row;
	State state;
	int frame;
	bool contBar;
	float xDiffPos;
	ScoreDisplay *parent;
	//sf::Text info;

};

struct ScoreDisplay
{
	ScoreDisplay(GameSession *owner,
		sf::Vector2f &position,
		sf::Font & testFont);
	~ScoreDisplay();
	sf::Sprite score;
	void Draw(sf::RenderTarget *target);
	int numEnemiesTotal;
	int numEnemiesKilled;
	double numSeconds;
	void Reset();
	void Activate();
	void Deactivate();
	bool active;
	bool waiting;
	//void Set

	

	void Update();


	static const int NUM_BARS = 3;
	ScoreBar *bars[NUM_BARS+1];

	GameSession *owner;
	Tileset *ts_scoreBar;
	Tileset *ts_scoreSheet;
	Tileset *ts_scoreSymbols;

	sf::VertexArray scoreBarVA;
	sf::Sprite scoreContinue;
	sf::VertexArray scoreSymbolsVA;
	sf::VertexArray scoreSheetVA;

	sf::Vector2f basePos;
	sf::Shader colorSwapShader;

	sf::Text time;
	sf::Text keys;
	sf::Font &font;
	//sf::Sprite scoreBar;
};

#endif