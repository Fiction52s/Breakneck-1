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

enum UIEvent
{
	E_BUTTON_DOWN,
	E_BUTTON_HOLD_DOWN,
	E_BUTTON_PRESSED,
	E_CHECKBOX_CHANGED,
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

struct UIEventHandlerBase
{
	virtual bool ButtonEvent( UIEvent eType,
		ButtonEventParams *param ){return false;}
	virtual bool CheckboxEvent( UIEvent eType,
		CheckboxEventParams *param ){return false;}
	virtual bool SelectorEvent( UIEvent eType,
		SelectorEventParams *param ){return false;}
};

struct UIControl
{
	enum UIControlType
	{
		UI_BAR,
		UI_HORIZ_SELECTOR_INT,
		UI_HORIZ_SELECTOR_STR,
		UI_VERTICAL_CONTROL_LIST,
		UI_BUTTON,
		UI_WINDOW,
		UI_CHECKBOX,
		UI_CONTROLGRID,
		UI_Count
	};

	UIControl( UIControl *p_parent, UIEventHandlerBase *p_handler,
		UIControlType p_cType );
	virtual ~UIControl();
	void SetTopLeftVec( const sf::Vector2f &pos );
	virtual void SetTopLeft( float x, float y ) = 0;
	virtual const sf::Vector2f &GetTopLeftGlobal() = 0;
	virtual const sf::Vector2f &GetTopLeftRel();
	virtual bool Update( ControllerState &curr,
		ControllerState &prev ) = 0;
	virtual void Draw( sf::RenderTarget *target ) = 0;	
	virtual void Focus();
	virtual void Unfocus();
	bool IsFocused();
	UIControl *parent;
	sf::Vector2f relTopLeft;
	sf::Vector2f dimensions;
	float GetWidth();
	float GetHeight();
	UIControl *GetOwner();
	UIControlType GetType();
private:
	UIControlType cType;
	bool focused;
protected:
	UIEventHandlerBase *handler;
};

struct UIBar : UIControl
{
	enum BarState
	{
		BAR_UNFOCUSED,
		BAR_FOCUSED,
		BAR_ALT0,
		BAR_ALT1,
		BAR_ALT2,
		BAR_Count
	};


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
	void SetState( BarState state );
	void Focus();
	void Unfocus();
	void UpdateSprite();
	Tileset *ts_bar;
	int width;
	sf::Vertex barVA[3*4];
	sf::Text currText;
	Alignment alignment;
	sf::Vector2f textOffset;
	BarState bState;
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
		UIEventHandlerBase *p_handler,
		UIControlType p_cType,
		TilesetManager *tsMan,
		sf::Font *f,
		int numOptions, std::string *names,
		const std::string &label, int p_labelWidth,
		Type type, bool p_loop, int p_defaultIndex,
		int chooserWidth );	
	virtual ~UIHorizSelector();
	void Draw( sf::RenderTarget *target );
	bool Update( ControllerState &curr,
		ControllerState &prev );
	void Focus();
	void Unfocus();
	void AssignArrowTexture();

	const sf::Vector2f &GetTopLeftGlobal();
	const sf::Vector2f &GetTopLeftRel();
	void SetTopLeft( float x, float y );

	UIBar *bar;
	UIBar *nameBar;
	sf::Vertex arrows[2*4];
	Tileset *ts_arrow;

	Type chooserType;
	std::string *names;
	int numOptions;
	int currIndex;
	bool loop;
	int defaultIndex;

	int waitFrames[3];
	int waitModeThresh[2];
	int framesWaiting;
	int currWaitLevel;
	int flipCounterLeft;
	int flipCounterRight;
	
};




struct UIHorizSelectorInt : UIHorizSelector
{
	UIHorizSelectorInt( UIControl *parent, 
		UIEventHandlerBase *p_handler,
		TilesetManager *tsMan,
		sf::Font *f,
		int numOptions, std::string *names,
		const std::string &label, int p_labelWidth,
		int *results,
		bool p_loop, int p_defaultIndex,
		int chooserWidth );
	int *results;
	int GetResult( int index );
};

struct UIHorizSelectorStr : UIHorizSelector
{
	UIHorizSelectorStr( UIControl *parent,
		UIEventHandlerBase *p_handler,
		TilesetManager *tsMan,
		sf::Font *f,
		int numOptions, std::string *names,
		const std::string &label, int p_labelWidth,
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

	int waitFrames[3];
	int waitModeThresh[2];
	int framesWaiting;
	int currWaitLevel;
	int flipCounterUp;
	int flipCounterDown;
};

struct UIControlGrid : UIControl
{
	UIControlGrid( UIControl *p_parent,
		int p_numControlsX, 
		int p_numControlsY,
		UIControl **p_controls,
		int p_spacingX, int p_spacingY,
		bool horizontalDominant );
	~UIControlGrid();
	void SetTopLeft( float x, float y );
	const sf::Vector2f &GetTopLeftGlobal();
	bool Update( ControllerState &curr,
		ControllerState &prev );
	void Draw( sf::RenderTarget *target );	
	int spacingX;
	int spacingY;
	int numControlsX;
	int numControlsY;
	int focusedIndexX;
	int focusedIndexY;
	sf::Vector2f globalTopLeft;
	UIControl **controls;

	int waitFrames[3];
	int waitModeThresh[2];
	int framesWaiting;
	int currWaitLevel;
	int flipCounterUp;
	int flipCounterDown;
	int flipCounterLeft;
	int flipCounterRight;
};



struct UIButton : UIControl
{
	UIButton( UIControl *p_parent,  
		UIEventHandlerBase *p_handler,
		TilesetManager *tsMan,
		sf::Font *f,
		const std::string &text,
		int p_width );
	~UIButton();
	void Focus();
	void Unfocus();
	void SetTopLeft( float x, float y );
	const sf::Vector2f &GetTopLeftGlobal();
	const sf::Vector2f &GetTopLeftRel();
	bool Update( ControllerState &curr,
		ControllerState &prev );
	void Draw( sf::RenderTarget *target );	

	int spacing;
	int numControls;
	int focusedIndex;
	UIBar *bar;
	bool pressedDown;
};

struct UICheckbox : UIControl
{
	UICheckbox( UIControl *p_parent,  
		UIEventHandlerBase *p_handler,
		TilesetManager *tsMan,
		sf::Font *f,
		const std::string &text,
		int p_width );
	~UICheckbox();
	void Focus();
	void Unfocus();
	void SetTopLeft( float x, float y );
	const sf::Vector2f &GetTopLeftGlobal();
	const sf::Vector2f &GetTopLeftRel();
	bool Update( ControllerState &curr,
		ControllerState &prev );
	void Draw( sf::RenderTarget *target );	

	bool checked;
	int spacing;
	int numControls;
	int focusedIndex;
	UIBar *bar;
	bool pressedDown;
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