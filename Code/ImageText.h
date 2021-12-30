#ifndef __IMAGETEXT_H__
#define __IMAGETEXT_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct Session;

struct ImageText
{
	enum PositionType
	{
		TOP_LEFT,
		CENTER,
		TOP_RIGHT
	};

	PositionType posType;

	ImageText( int maxDigits, 
		Tileset *ts_tex );
	~ImageText();
	sf::Vector2f anchor;
	void SetCenter(sf::Vector2f &p_center);
	void SetTopRight(sf::Vector2f &p_topRight);
	void SetTopLeft(sf::Vector2f &p_topLeft);
	virtual void UpdateSprite();
	void Draw( sf::RenderTarget *target );
	//in a timer this sets number of seconds
	void SetNumber( int num );
	void SetScale(float s);
	
	void ShowZeroes( int numZeroes );

	float scale;
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


struct GameSession;

struct TextDisp
{

	TextDisp(int width, int height, int charSize = 30, 
		int frameLetterWait = 3, int p_letterPerShow = 1);
	~TextDisp();
	void SetTopLeft(sf::Vector2f &pos);
	void Load(const std::string &name);
	//static void LoadScript()
	void SetTextSize();
	void SetString(const std::string &str);
	void AddLineBreaks(const std::string &msg);
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


struct Speech
{
	enum TextMode
	{
		TOPTEXT,
		BOTTOMTEXT,
	};

	Speech();
	~Speech();
	std::string speaker;
	int speakerTile;
	TextDisp *disp;
	void SetupSprite();
	Tileset *GetTileset();
	void SetTextMode(TextMode tMode);
	Tileset *ts;
	sf::Sprite speakerSpr;
	Session *sess;
	void Draw(sf::RenderTarget *target);
};

struct Conversation;

struct ConversationGroup
{
	ConversationGroup();
	~ConversationGroup();
	void Load(const std::string &name);
	void Reset();
	Conversation* GetConv(int index);
	int numConvs;
	Conversation **convs;

	std::string sceneName;
};

struct Conversation
{
	

	~Conversation();
	Conversation();
	void Load(const std::string &name);
	bool Load(std::ifstream &is);
	std::string GetSpeakerName( const std::string &line,
		int &tIndex );
	void Draw(sf::RenderTarget *target);
	bool Update();
	void Reset();
	void Show();
	void Hide();
	void NextSection();
	void SetRate(int wait, int letterPer);

	
	void SetTextMode(Speech::TextMode tMode);

	bool show;
	int numSpeeches;
	Speech **speeches;
	sf::Sprite speakerSpr;
	int currSpeech;
};





#endif