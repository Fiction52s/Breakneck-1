#ifndef __UIWINDOW_H__
#define __UIWINDOW_H__

#include "SFML/Graphics.hpp"
#include "Input.h"

struct Tileset;
struct GameSession;

struct UIWindow
{
	enum Quads
	{
		CORNER_TOPLEFT_TOP,
		CORNER_TOPLEFT_LEFT,
		CORNER_TOPRIGHT_TOP,
		CORNER_TOPRIGHT_RIGHT,
		CORNER_BOTRIGHT_BOT,
		CORNER_BOTRIGHT_RIGHT,
		CORNER_BOTLEFT_BOT,
		CORNER_BOTLEFT_LEFT,
		CORNER_TOPLEFT,
		CORNER_TOPRIGHT,
		CORNER_BOTRIGHT,
		CORNER_BOTLEFT,
		WALL_LEFT,
		WALL_RIGHT,
		FLOOR,
		CEILING,
		Count
	};

	UIWindow( GameSession *owner, Tileset *t,
		sf::Vector2f &p_windowSize );
	void AssignTextureToCorners();
	void AssignTextureToCornerEdges();
	void AssignTextureToEdges();
	void Update();
	
	void SetTopLeft( sf::Vector2f &pos );
	const sf::Vector2f &GetTopLeft();
	void Draw( sf::RenderTarget *target );
	void SetCornerVertices( sf::Vector2f &topLeft, int index );
	void SetWallVertices( sf::Vector2f &topLeft, int index );
	void SetCornerEdgesHorizVertices( sf::Vector2f &topLeft, int index );
	void SetCornerEdgesVertVertices( sf::Vector2f &topLeft, int index );
	void SetFlatVertices( sf::Vector2f &topLeft, int index );

	int minSize;
	int tileSize;

	//sf::Vector2f position;
	sf::Vertex borderVA[Count * 4];
	Tileset *ts_window;
	GameSession *owner;
	sf::Vector2f windowSize;
};

#endif