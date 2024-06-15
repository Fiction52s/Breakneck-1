#ifndef __SCOREDISPLAY_H__
#define __SCOREDISPLAY_H__

#include "SFML/Graphics.hpp"

struct Tileset;
struct Session;

struct ScoreDisplay;
struct GameSession;
struct FeedbackInputBox;
struct Medal;
struct MedalSequence;
struct TilesetManager;

struct ScoreDisplay
{

	FeedbackInputBox *feedbackInputBox;
	int action;
	int frame;
	bool madeRecord;
	bool gotGold;
	bool gotSilver;
	bool gotBronze;
	sf::Font &font;
	Session *sess;
	GameSession *game;
	MedalSequence *medalSeq;

	ScoreDisplay( TilesetManager *tm, sf::Font &f);
	virtual ~ScoreDisplay();
	virtual void Reset();
	virtual void Activate() = 0;
	virtual void Deactivate() = 0;
	virtual void Update() = 0;
	
	
	virtual bool IsActive() = 0;
	virtual bool IsWaiting() = 0;
	virtual bool IsIncludingExtraOptions() = 0;

	virtual void Draw(sf::RenderTarget *target) = 0;
};

struct DefaultScoreDisplay : ScoreDisplay
{
	enum Action
	{
		A_IDLE,
		A_ENTER,
		A_WAIT,
		A_GIVE_GOLD,
		A_GIVE_SILVER,
		A_GIVE_BRONZE,
		A_Count
	};

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
		DefaultScoreDisplay *parent;

		sf::Sprite barSprite;
		sf::Sprite symbolSprite;
		sf::Text text;

		sf::Sprite extraSymbolSprite;
		sf::Text extraText;

		sf::Color textColor;
		int row;

		ScoreBar(int row, DefaultScoreDisplay *parent);
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
		DefaultScoreDisplay *parent;
		sf::Text extraText;

		sf::Sprite barSprite;
		sf::Sprite buttonIconSprite;

		SelectBar(int row, DefaultScoreDisplay *parent);
		void SetBarPos(float xDiff);
		void PopOut();
		void Retract();
		void Update();
		void Reset();
		void Draw(sf::RenderTarget *target);
	};

	sf::Vertex nameBGQuad[4];
	sf::Text levelNameText;

	//GameSession *game;
	sf::Sprite score;
	static const int NUM_BARS = 3;
	ScoreBar *bars[NUM_BARS];
	static const int NUM_SELECT_BARS = 6;
	SelectBar *selectBars[NUM_SELECT_BARS];
	int numShownSelectBars;
	int includeExtraSelectBars;
	Tileset *ts_score;
	Tileset *ts_scoreIcons;
	sf::Vector2f basePos;
	float selectOffset;
	int numEnemiesTotal;
	int numEnemiesKilled;
	double numSeconds;

	DefaultScoreDisplay(sf::Vector2f &position,
		sf::Font & testFont);
	~DefaultScoreDisplay();

	void Reset();
	void Activate();
	void Deactivate();
	void Update();

	
	bool IsActive();
	bool IsWaiting();
	bool IsIncludingExtraOptions();

	void Draw(sf::RenderTarget *target);

	void PopOutBar(int row);
	void PopOutSelectBars();
	int GetNumSelectBars();
};

#endif