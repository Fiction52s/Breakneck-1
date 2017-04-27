#ifndef __UIWINDOW_H__
#define __UIWINDOW_H__

#include "SFML/Graphics.hpp"
#include "Input.h"

struct Tileset;
struct GameSession;
struct TilesetManager;

struct UIButton;
struct UIHorizSelector;
struct UICheckbox;
struct UIWindow;

struct UIEventHandlerBase
{
	enum UIEvent
	{
		E_BUTTON_PRESSED,
		E_CHECKBOX_CHECKED,
		E_SELECTOR_CHANGED,
		E_Count
	};

	struct SelectorEventParams
	{
		int oldSelectorIndex;
		int newSelectorIndex;
		UIHorizSelector *selector;
	};

	struct ButtonEventParams
	{
		UIButton *button;
	};

	struct CheckboxEventParams
	{
		UICheckbox *checkbox;
	};

	virtual bool ButtonEvent( UIEvent eType,
		ButtonEventParams *param ) = 0;
	virtual bool CheckboxEvent( UIEvent eType,
		CheckboxEventParams *param ) = 0;
	virtual bool SelectorEvent( UIEvent eType,
		SelectorEventParams *param ) = 0;

};

struct UIControl
{
	enum UIControlType
	{
		UI_BAR,
		UI_HORIZSELECTOR,
		UI_HORIZSELECTOR_INT,
		UI_HORIZSELECTOR_STR,
		UI_VERTICAL_CONTROL_LIST,
		UI_BUTTON,
		UI_WINDOW,
		UI_Count
	};

	UIControl( UIControl *p_parent, UIControlType p_cType );
	virtual ~UIControl();
	void SetTopLeftVec( const sf::Vector2f &pos );
	virtual void SetTopLeft( float x, float y ) = 0;
	virtual const sf::Vector2f &GetTopLeftGlobal() = 0;
	virtual const sf::Vector2f &GetTopLeftRel();
	virtual bool Update( ControllerState &curr,
		ControllerState &prev ) = 0;
	virtual void Draw( sf::RenderTarget *target ) = 0;	
	bool focused;
	UIControl *parent;
	sf::Vector2f relTopLeft;
	sf::Vector2f dimensions;
	float GetWidth();
	float GetHeight();
	UIControl *GetOwner();
	UIControlType GetType();
private:
	UIControlType cType;
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
	void SetTopLeft( float x, float y );
	virtual bool Update( ControllerState &curr,
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



struct UIHorizSelector : UIControl
{
	enum Type
	{
		ERR,
		INT,
		STR
	};

	
	UIHorizSelector( UIControl *parent, 
		UIControlType p_cType,
		TilesetManager *tsMan,
		sf::Font *f,
		int numOptions, std::string *names,
		Type type, bool p_loop, int p_defaultIndex,
		int chooserWidth );	
	void Draw( sf::RenderTarget *target );
	bool Update( ControllerState &curr,
		ControllerState &prev );
	void UpdateSprite();
	void AssignArrowTexture();

	const sf::Vector2f &GetTopLeftGlobal();
	const sf::Vector2f &GetTopLeftRel();
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




struct UIHorizSelectorInt : UIHorizSelector
{
	UIHorizSelectorInt( UIControl *parent, 
		TilesetManager *tsMan,
		sf::Font *f,
		int numOptions, std::string *names,
		int *results,
		bool p_loop, int p_defaultIndex,
		int chooserWidth );
	int *results;
	int GetResult( int index );
};

struct UIHorizSelectorStr : UIHorizSelector
{
	UIHorizSelectorStr( UIControl *parent,
		TilesetManager *tsMan,
		sf::Font *f,
		int numOptions, std::string *names,
		std::string *results,
		bool p_loop, int p_defaultIndex,
		int chooserWidth );
	std::string *results;
	const std::string &GetResult( int index );
};

struct UIVerticalControlList : UIControl
{
	UIVerticalControlList( UIControl *p_parent,
		int p_numControls, UIControl **p_controls,
		int p_spacing );
	~UIVerticalControlList();
	void SetTopLeft( float x, float y );
	const sf::Vector2f &GetTopLeftGlobal();
	bool Update( ControllerState &curr,
		ControllerState &prev );
	void Draw( sf::RenderTarget *target );	
	int spacing;
	int numControls;
	int focusedIndex;
	sf::Vector2f globalTopLeft;
	UIControl **controls;
};

struct UIButton : UIControl
{
	UIButton( UIControl *p_parent,
		int p_numControls, UIControl **p_controls,
		int p_spacing );
	~UIButton();
	void SetTopLeft( float x, float y );
	const sf::Vector2f &GetTopLeftGlobal();
	bool Update( ControllerState &curr,
		ControllerState &prev );
	void Draw( sf::RenderTarget *target );	
	int spacing;
	int numControls;
	int focusedIndex;
	sf::Vector2f globalTopLeft;
	UIControl **controls;
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
	bool Update();
	
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
	bool Update( ControllerState &curr,
		ControllerState &prev );

	int minSize;
	int tileSize;

	//sf::Vector2f position;
	sf::Vertex borderVA[Count * 4];
	sf::Vertex centerVA[4];
	Tileset *ts_window;
	//GameSession *owner;

	UIVerticalControlList *controlList;
};

#endif