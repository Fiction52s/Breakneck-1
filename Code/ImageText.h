#ifndef __IMAGETEXT_H__
#define __IMAGETEXT_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct ImageText
{
	ImageText( int maxDigits, 
		Tileset *ts_tex );
	~ImageText();
	sf::Vector2f topRight;
	sf::Vector2f center;
	bool positionCenter;
	void SetCenter(sf::Vector2f &p_center);
	void SetTopRight(sf::Vector2f &p_topRight);
	virtual void UpdateSprite();
	void Draw( sf::RenderTarget *target );
	//in a timer this sets number of seconds
	void SetNumber( int num );
	
	void ShowZeroes( int numZeroes );

	int numShowZeroes;
	int maxDigits;
	int activeDigits;
	int value;
	sf::Vertex *vert;
	Tileset *ts;
};

struct TimerText : ImageText
{
	TimerText( Tileset *ts_tex );
	void UpdateSprite();
};

struct Script
{
	Script();
	~Script();
	void Load(const std::string &name);
	std::string fullMessage;
	std::string *sections;
	int numSections;
	//int numLinesPerSection;
	const std::string &GetSection(int index);
};

struct TextDisp;
struct GameSession;
struct Speech
{
	Speech(GameSession *owner);
	~Speech();
	std::string speaker;
	int speakerTile;
	TextDisp *disp;
	void SetupSprite();
	Tileset *GetTileset();
	Tileset *ts;
	sf::Sprite speakerSpr;
	GameSession *owner;
	void Draw(sf::RenderTarget *target);
};

struct Conversation
{
	~Conversation();
	Conversation( GameSession *owner );
	void Load(const std::string &name);
	std::string GetSpeakerName( const std::string &line,
		int &tIndex );
	void Draw(sf::RenderTarget *target);
	bool Update();
	void Reset();
	void Show();
	void Hide();
	void NextSection();
	void SetRate(int wait, int letterPer);


	GameSession *owner;
	int numSpeeches;
	Speech **speeches;
	sf::Sprite speakerSpr;
	int currSpeech;
};


struct TextDisp
{
	TextDisp( GameSession *owner,
		int width, int height, int charSize = 30, int frameLetterWait = 3,
		int p_letterPerShow = 1);
	~TextDisp();
	void SetTopLeft(sf::Vector2f &pos);
	void Load(const std::string &name);
	//static void LoadScript()
	void SetTextSize();
	void SetString(const std::string &str);
	void AddLineBreaks( const std::string &msg );
	void SetRate(int wait, int letterPer);
	sf::Vector2f GetTopLeft();
	void Reset();
	bool Update();
	bool NextSection();
	bool sectionWait;
	void Show();
	void Hide();
	void Draw(sf::RenderTarget *target);
	sf::Text text;
	sf::Vector2f rectSize;
	std::string * sections;
	int numSections;
	int currSection;
	int linesShownAtOnce;
	sf::RectangleShape bgRect;
	int nextLetterWait;
	int frame;
	bool show;
	int letterPerShow;
};


#endif