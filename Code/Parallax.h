#ifndef __PARALLAX_H__
#define __PARALLAX_H__

#include "Tileset.h"
#include "SFML/Graphics.hpp"
#include <list>

struct GameSession;
struct Parallax;

struct RepeatingSprite
{
	RepeatingSprite( Parallax *parent, 
		Tileset *ts, int index,
		sf::Vector2f &offset, int repeatWidth,
		int depthLevel,
		float p_scrollSpeedX = 0);
	sf::Sprite spr;
	void Update( sf::Vector2f &camPos );
	sf::Vector2f relPos;
	int repeatWidth;
	float depth;// depthLevel;
	Parallax *parent;
	float scrollSpeedX;
	void Draw( sf::RenderTarget *target );
};

struct Parallax
{
	enum Arrange
	{
		ABAB_2,
		AABB_2,
		ABCABC_3,
		Count
	};

	//how do i do animated parallax?
	Parallax();
	~Parallax();
	//std::list<Tileset*> tilesets;
	void SetImage( Tileset *ts, int index );
	void AddRepeatingSprite( Tileset *ts, int index,
		sf::Vector2f &offset, int repeatWidth,
		int depthLevel );
	void Update( sf::Vector2f &camPos );
	void Draw( sf::RenderTarget *target );
	GameSession *owner;
	sf::Sprite *vaArray;
	std::list<RepeatingSprite*> repeatingSprites;
	//sf::Sprite testSprite;
	//sf::Sprite testSprite1;
	Tileset **tilesets;
	int depthLevel;
	float depth;
	sf::View oldView;
	sf::View newView;

	Arrange arrange;
};

struct ScrollingBackground
{
	ScrollingBackground( Tileset *ts, int index,
		int depthLevel, float p_scrollSpeedX = 0.f );
	Tileset *ts;
	int tsIndex;
	sf::Vector2f relPos;
	sf::VertexArray va;
	float xPos;
	int depthLevel;
	sf::View oldView;
	sf::View newView;
	float depth;
	float scrollSpeedX;
	float scrollOffset;
	void Update( sf::Vector2f &camPos );
	void SetTileIndex( int index );
	void Draw( sf::RenderTarget *target );
	void SetLeftPos( sf::Vector2f &pos );
};

#endif