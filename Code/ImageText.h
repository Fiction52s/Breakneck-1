#ifndef __IMAGETEXT_H__
#define __IMAGETEXT_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct ImageText
{
	ImageText( int maxDigits, 
		Tileset *ts_tex );
	sf::Vector2f topRight;
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

#endif