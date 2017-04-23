#ifndef __UIWINDOW_H__
#define __UIWINDOW_H__

#include "SFML/Graphics.hpp"
#include "Input.h"

struct Tileset;
struct GameSession;
struct TilesetManager;

struct UIControl
{
	UIControl( UIControl *p_parent );
	virtual void SetTopLeft( const sf::Vector2f &pos ) = 0;
	virtual void SetTopLeft( float x, float y ) = 0;
	virtual const sf::Vector2f &GetTopLeftGlobal() = 0;
	virtual const sf::Vector2f &GetTopLeftRel();
	virtual void Update( ControllerState &curr,
		ControllerState &prev ) = 0;
	virtual void Draw( sf::RenderTarget *target ) = 0;	
	bool focused;
	UIControl *parent;
	sf::Vector2f relTopLeft;
};

struct UIBar : UIControl
{
	enum Alignment
	{
		LEFT,
		MIDDLE,
		RIGHT
	};

	UIBar( UIControl *parent, TilesetManager *tsMan, sf::Font *f,
		int p_width );
	void Draw( sf::RenderTarget *target );
	void AssignTexture();
	void SetTopLeft( const sf::Vector2f &pos );
	void SetTopLeft( float x, float y );
	virtual void Update( ControllerState &curr,
		ControllerState &prev );
	const sf::Vector2f &GetTopLeftGlobal();
	void SetText( const std::string &text );
	void SetText( const std::string &text, 
		Alignment align );
	void SetTextAlignment( Alignment align );
	Tileset *ts_bar;
	int width;
	sf::Vertex barVA[3*4];
	sf::Text currText;
	Alignment alignment;
	sf::Vector2f textOffset;
};

//types of bars: horizchooser, button, checkbox,
//lightup



struct UIHorizChooser : UIControl
{
	enum Type
	{
		ERR,
		INT,
		STR
	};

	
	UIHorizChooser( UIControl *parent, TilesetManager *tsMan,
		sf::Font *f,
		int numOptions, std::string *names,
		Type type, bool p_loop, int p_defaultIndex,
		int chooserWidth );	
	void Draw( sf::RenderTarget *target );
	void Update( ControllerState &curr,
		ControllerState &prev );
	void UpdateSprite();
	void AssignArrowTexture();

	const sf::Vector2f &GetTopLeftGlobal();
	const sf::Vector2f &GetTopLeftRel();
	void SetTopLeft( const sf::Vector2f &pos );
	void SetTopLeft( float x, float y );

	UIBar *bar;
	sf::Vertex arrows[2*4];
	Tileset *ts_arrow;

	Type chooserType;
	std::string *names;
	int numOptions;
	int currIndex;
	bool loop;
	int defaultIndex;
	
};




struct UIHorizChooserInt : UIHorizChooser
{
	UIHorizChooserInt( UIControl *parent, 
		TilesetManager *tsMan,
		sf::Font *f,
		int numOptions, std::string *names,
		int *results,
		bool p_loop, int p_defaultIndex,
		int chooserWidth );
	int *results;
	int GetResult( int index );
};

struct UIHorizChooserStr : UIHorizChooser
{
	UIHorizChooserStr( UIControl *parent,
		TilesetManager *tsMan,
		sf::Font *f,
		int numOptions, std::string *names,
		std::string *results,
		bool p_loop, int p_defaultIndex,
		int chooserWidth );
	std::string *results;
	const std::string &GetResult( int index );
};

struct UIWindow : UIControl
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

	UIWindow( UIControl *p_parent, 
		Tileset *t,
		sf::Vector2f &p_windowSize );
	void AssignTextureToCorners();
	void AssignTextureToCornerEdges();
	void AssignTextureToEdges();
	void AssignTextureCenter();
	void Update();
	
	void SetTopLeft( const sf::Vector2f &pos );
	void SetTopLeft( float x, float y );
	const sf::Vector2f &GetTopLeftGlobal();
	void Draw( sf::RenderTarget *target );
	void SetCornerVertices( sf::Vector2f &topLeft, int index );
	void SetWallVertices( sf::Vector2f &topLeft, int index );
	void SetCornerEdgesHorizVertices( sf::Vector2f &topLeft, int index );
	void SetCornerEdgesVertVertices( sf::Vector2f &topLeft, int index );
	void SetFlatVertices( sf::Vector2f &topLeft, int index );
	void SetCenterVertices( sf::Vector2f &A,
		sf::Vector2f &B, sf::Vector2f &C,
		sf::Vector2f &D );
	void Resize( sf::Vector2f &size );
	void Resize( float x, float y );
	void Update( ControllerState &curr,
		ControllerState &prev );

	int minSize;
	int tileSize;

	//sf::Vector2f position;
	sf::Vertex borderVA[Count * 4];
	sf::Vertex centerVA[4];
	Tileset *ts_window;
	//GameSession *owner;
	sf::Vector2f windowSize;

	UIHorizChooserStr *test;
};

#endif