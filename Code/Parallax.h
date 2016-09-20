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
		sf::Vector2f &offset, int repeatWidth );
	sf::Sprite spr;
	void Update( sf::Vector2f &camPos );
	sf::Vector2f relPos;
	int repeatWidth;
	Parallax *parent;
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
	Parallax( GameSession *owner, int depthLevel,
		Arrange arrange );
	~Parallax();
	//std::list<Tileset*> tilesets;
	void SetImage( Tileset *ts, int index );
	void Update();
	void Draw( sf::RenderTarget *target );
	GameSession *owner;
	sf::Sprite *vaArray;
	RepeatingSprite *testRepeat;
	RepeatingSprite *testRepeat1;
	//sf::Sprite testSprite;
	//sf::Sprite testSprite1;
	Tileset **tilesets;
	int depthLevel;
	float depth;
	sf::View oldView;
	sf::View newView;

	Arrange arrange;
};

#endif