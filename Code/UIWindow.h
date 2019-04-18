#ifndef __UIWINDOW_H__
#define __UIWINDOW_H__

#include "SFML/Graphics.hpp"
#include "Input.h"
#include <list>

struct Tileset;
struct GameSession;
struct TilesetManager;

struct UIButton;
template <typename T>
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
	void *selector; //needs to be cast to the correct type
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
		UI_HORIZ_SELECTOR,
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
		BAR_ALT3,
		BAR_Count
	};


	enum Alignment
	{
		LEFT,
		MIDDLE,
		RIGHT
	};

	UIBar( UIControl *parent, TilesetManager *tsMan, sf::Font *f,
		int p_width, int p_height = 80, int p_textHeight = 40 );
	void Draw( sf::RenderTarget *target );
	void AssignTexture( int tileIndex );
	void SetTopLeft( float x, float y );
	virtual bool Update( ControllerState &curr,
		ControllerState &prev );
	const sf::Vector2f &GetTopLeftGlobal();
	void SetText( const std::string &text );
	void SetText( const std::string &text,
		sf::Vector2i offset,
		Alignment align);
	void SetTextHeight( int height );
	void SetTextAlignment( Alignment align, sf::Vector2i &offset );
	void SetState( BarState state );
	void Focus();
	void Unfocus();
	void UpdateSprite();
	Tileset *ts_bar;
	const sf::String &GetString();
	int width;
	sf::Vertex barVA[3*4];
	sf::Text currText;
	Alignment alignment;
	sf::Vector2i textOffset;
	BarState bState;
	int sideWidth;
};

//types of bars: horizchooser, button, checkbox,
//lightup


template <typename T>
struct UIHorizSelector : UIControl
{	
	/*UIHorizSelector( UIControl *parent, 
		UIEventHandlerBase *p_handler,
		TilesetManager *tsMan,
		sf::Font *f,
		int numOptions, std::string *names,
		const std::string &label, int p_labelWidth,
		T *results, bool p_loop, int p_defaultIndex,
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
	const T & GetResult(int index);*/

	UIBar *bar;
	UIBar *nameBar;
	sf::Vertex arrows[2*4];
	Tileset *ts_arrow;

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
	T *results;
	

	UIHorizSelector(UIControl *p_parent, UIEventHandlerBase *p_handler,
		TilesetManager *tsMan, sf::Font *f, int p_numOptions,
		std::string *p_names, const std::string &label, int p_labelWidth, T *p_results,
		bool p_loop, int p_defaultIndex, int p_chooserWidth)
		:UIControl(p_parent, p_handler, UIControl::UIControlType::UI_HORIZ_SELECTOR), 
		numOptions(p_numOptions), loop(p_loop),
		defaultIndex(p_defaultIndex)
	{
		results = new T[p_numOptions];
		for (int i = 0; i < p_numOptions; ++i)
		{
			results[i] = p_results[i];
		}
		//ts_arrows = tsMan->GetTileset( "arrows_" );
		names = new std::string[numOptions];
		for (int i = 0; i < numOptions; ++i)
		{
			names[i] = p_names[i];
		}
		currIndex = defaultIndex;

		AssignArrowTexture();

		bar = new UIBar(p_parent, tsMan, f, p_chooserWidth);

		nameBar = new UIBar(p_parent, tsMan, f, p_labelWidth);
		nameBar->SetText(label, Vector2i(0, 0), UIBar::Alignment::MIDDLE);

		dimensions = bar->dimensions;

		/*currIndexText.setFont( *f );
		currIndexText.setCharacterSize( 40 );
		currIndexText.setColor( Color::White );*/
		bar->SetText(names[defaultIndex]);

		waitFrames[0] = 10;
		waitFrames[1] = 5;
		waitFrames[2] = 2;

		waitModeThresh[0] = 2;
		waitModeThresh[1] = 2;

		currWaitLevel = 0;
		flipCounterLeft = 0;
		flipCounterRight = 0;
		framesWaiting = 0;
	}

	virtual ~UIHorizSelector()
	{
		delete bar;
		delete nameBar;
		delete[] names;
		delete[] results;
	}

	//returns true on success
	bool SetCurrAsResult( const T &param )
	{
		for (int i = 0; i < numOptions; ++i)
		{
			if (results[i] == param)
			{
				currIndex = i;
				UpdateOption();
				return true;
			}
		}

		return false;
	}

	const sf::Vector2f &GetTopLeftRel()
	{
		return bar->GetTopLeftRel();
	}

	const sf::Vector2f &GetTopLeftGlobal()
	{
		return bar->GetTopLeftGlobal();
	}

	void Focus()
	{
		UIControl::Focus();
		bar->Focus();
		nameBar->Focus();
	}

	void Unfocus()
	{
		UIControl::Unfocus();
		bar->Unfocus();
		nameBar->Unfocus();
	}

	void AssignArrowTexture()
	{
		IntRect sub;
		for (int i = 0; i < 2; ++i)
		{
			/*sub = ts_arrow->GetSubRect( i );
			arrows[i*4+0].texCoords = Vector2f( sub.left, sub.top );
			arrows[i*4+1].texCoords = Vector2f( sub.left + sub.width, sub.top );
			arrows[i*4+2].texCoords = Vector2f( sub.left + sub.width, sub.top + sub.height );
			arrows[i*4+3].texCoords = Vector2f( sub.left, sub.top + sub.height );*/

			arrows[i * 4 + 0].color = Color::Blue;
			arrows[i * 4 + 1].color = Color::Cyan;
			arrows[i * 4 + 2].color = Color::Yellow;
			arrows[i * 4 + 3].color = Color::Red;
		}
	}

	void UpdateOption()
	{
		bar->SetText(names[currIndex]);
	}

	bool Update(ControllerState &curr, ControllerState &prev)
	{
		bool upOrDown = curr.LUp() || curr.LDown();
		bool left = curr.LLeft() && !upOrDown;
		bool right = curr.LRight() && !upOrDown;

		if (right)
		{
			if (flipCounterRight == 0
				|| (flipCounterRight > 0 && framesWaiting == waitFrames[currWaitLevel])
				)
			{
				if (flipCounterRight == 0)
				{
					currWaitLevel = 0;
				}

				++flipCounterRight;

				if (flipCounterRight == waitModeThresh[currWaitLevel] && currWaitLevel < 2)
				{
					currWaitLevel++;
				}

				flipCounterLeft = 0;
				framesWaiting = 0;

				if (currIndex < numOptions - 1)
				{
					currIndex++;
				}
				else
				{
					if (loop)
					{
						currIndex = 0;
					}
				}
			}
			else
			{
				++framesWaiting;
			}

		}
		else if (left)
		{
			if (flipCounterLeft == 0
				|| (flipCounterLeft > 0 && framesWaiting == waitFrames[currWaitLevel])
				)
			{
				if (flipCounterLeft == 0)
				{
					currWaitLevel = 0;
				}

				++flipCounterLeft;

				if (flipCounterLeft == waitModeThresh[currWaitLevel] && currWaitLevel < 2)
				{
					currWaitLevel++;
				}

				flipCounterRight = 0;
				framesWaiting = 0;
				if (currIndex > 0)
				{
					currIndex--;
				}
				else
				{
					if (loop)
					{
						currIndex = numOptions - 1;
					}
				}
			}
			else
			{
				++framesWaiting;
			}

		}
		else
		{
			flipCounterLeft = 0;
			flipCounterRight = 0;
			currWaitLevel = 0;
			framesWaiting = 0;
		}

		
		UpdateOption();

		return false;
	}

	void SetTopLeft(float x, float y)
	{
		int extraSpacing = 20;
		relTopLeft = Vector2f(x, y);
		nameBar->SetTopLeft(x, y);
		bar->SetTopLeft(x + nameBar->GetWidth() + extraSpacing, y);

		//set arrows
		//currIndexText.setPosition( x, y );
	}

	void Draw(sf::RenderTarget *target)
	{
		nameBar->Draw(target);
		bar->Draw(target);
	}

	const T & GetResult(int index)
	{
		return results[index];
	}
};


//struct UIHorizSelectorInt : UIHorizSelector
//{
//	UIHorizSelectorInt( UIControl *parent, 
//		UIEventHandlerBase *p_handler,
//		TilesetManager *tsMan,
//		sf::Font *f,
//		int numOptions, std::string *names,
//		const std::string &label, int p_labelWidth,
//		int *results,
//		bool p_loop, int p_defaultIndex,
//		int chooserWidth );
//	int *results;
//	int GetResult( int index );
//};
//
//struct UIHorizSelectorStr : UIHorizSelector
//{
//	UIHorizSelectorStr( UIControl *parent,
//		UIEventHandlerBase *p_handler,
//		TilesetManager *tsMan,
//		sf::Font *f,
//		int numOptions, std::string *names,
//		const std::string &label, int p_labelWidth,
//		std::string *results,
//		bool p_loop, int p_defaultIndex,
//		int chooserWidth );
//	std::string *results;
//	const std::string &GetResult( int index );
//};

struct UIVerticalControlList : UIControl
{
	UIVerticalControlList( UIControl *p_parent,
		int p_numControls, UIControl **p_controls,
		int p_spacing );
	virtual ~UIVerticalControlList();
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
	virtual ~UIControlGrid();
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
	int framesWaitingHoriz;
	int framesWaitingVert;
	int currWaitLevelHoriz;
	int currWaitLevelVert;
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
		int p_width,
		int p_height = 80 );
	virtual ~UIButton();
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
		int p_width,
		int p_height = 80 );
	virtual ~UICheckbox();
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
	virtual ~UIWindow();
	void AssignTextureToCorners();
	void AssignTextureToCornerEdges();
	void AssignTextureToEdges();
	void AssignTextureCenter();
	
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

	std::list<UIControl*> controls;
	//UIVerticalControlList *controlList;
};

#endif