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
	~Script();
	void Load(const std::string &name);
	std::string *sections;
	int numSections;
	//int numLinesPerSection;
	const std::string &GetSection(int index);
};

struct GameSession;
struct TextDisp
{
	TextDisp( GameSession *owner );
	void SetTopLeft(sf::Vector2f &pos);
	//static void LoadScript()
	void SetTextSize();
	void SetString(const std::string &str);
	void Reset();
	bool Update();
	void Show();
	void Hide();
	void Draw(sf::RenderTarget *target);
	sf::Text text;
	sf::Vector2f rectSize;
	std::string message;
	sf::RectangleShape bgRect;
	int nextLetterWait;
	int frame;
	bool show;
};


#endif