#include "UIWindow.h"
#include "GameSession.h"
#include "Tileset.h"
#include <string>

using namespace sf;
using namespace std;

UIButton::UIButton( UIControl *p_parent, UIEventHandlerBase *p_handler, 
	TilesetManager *tsMan, Font *f, const std::string &text, int p_width, int p_height )
	:UIControl( p_parent, p_handler, UI_BUTTON )
{
	bar = new UIBar( p_parent, tsMan, f, p_width, p_height );
	SetTopLeft( 0, 0 );
	bar->SetText( text );
	dimensions = bar->dimensions;
}

UIButton::~UIButton()
{
	delete bar;
}

void UIButton::Unfocus()
{
	UIControl::Unfocus();
	pressedDown = false;
	bar->Unfocus();
	//bar->bState = UIBar::BAR_UNFOCUSED;
}

void UIButton::Focus()
{
	UIControl::Focus();
	pressedDown = false;
	bar->Focus();
	//bar->bState = UIBar::BAR_FOCUSED;
}

void UIButton::SetTopLeft( float x, float y )
{
	relTopLeft = Vector2f( x, y );
	bar->SetTopLeft( x, y );
}

const sf::Vector2f &UIButton::GetTopLeftRel()
{
	return bar->GetTopLeftRel();
}

const sf::Vector2f &UIButton::GetTopLeftGlobal()
{
	return bar->GetTopLeftGlobal();
}


bool UIButton::Update( ControllerState &curr,
	ControllerState &prev )
{
	ButtonEventParams params;
	params.button = this;

	if( curr.A && !prev.A && !pressedDown )
	{
		pressedDown = true;
		bar->SetState( UIBar::BAR_ALT0 );
		if( handler != NULL )
		{
			return handler->ButtonEvent( UIEvent::E_BUTTON_DOWN, &params );
		}
	}
	else if( !curr.A && prev.A && pressedDown )
	{
		pressedDown = false;
		bar->SetState( UIBar::BAR_FOCUSED );
		if( handler != NULL )
		{
			return handler->ButtonEvent( UIEvent::E_BUTTON_PRESSED, &params );
		}
	}
	else if( curr.A )
	{
		bar->SetState( UIBar::BAR_ALT1 );
		if( handler != NULL )
			return handler->ButtonEvent( UIEvent::E_BUTTON_HOLD_DOWN, &params );
	}

	return false;
}

void UIButton::Draw( sf::RenderTarget *target )
{
	bar->Draw( target );
}