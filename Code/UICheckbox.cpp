#include "UIWindow.h"

#include "GameSession.h"
#include "Tileset.h"
#include <string>

using namespace sf;
using namespace std;

UICheckbox::UICheckbox( UIControl *p_parent, UIEventHandlerBase *p_handler, 
	TilesetManager *tsMan, Font *f, const std::string &text, int p_width, int p_height )
	:UIControl( p_parent, p_handler, UI_CHECKBOX )
{
	bar = new UIBar( p_parent, tsMan, f, p_width, p_height );
	SetTopLeft( 0, 0 );
	dimensions = bar->dimensions;
	checked = false;

	if( checked )
	{
		bar->SetState( UIBar::BAR_ALT0 );
	}
	else
	{
		bar->SetState( UIBar::BAR_ALT1 );
	}
}

UICheckbox::~UICheckbox()
{
	delete bar;
}

void UICheckbox::Unfocus()
{
	UIControl::Unfocus();
	pressedDown = false;
	bar->Unfocus();

	if( checked )
	{
		bar->SetState( UIBar::BAR_ALT0 );
	}
	else
	{
		bar->SetState( UIBar::BAR_ALT1 );
	}
	//bar->bState = UIBar::BAR_UNFOCUSED;
}

void UICheckbox::Focus()
{
	UIControl::Focus();
	pressedDown = false;
	bar->Focus();

	if( checked )
	{
		bar->SetState( UIBar::BAR_ALT0 );
	}
	else
	{
		bar->SetState( UIBar::BAR_ALT1 );
	}
	
	//bar->bState = UIBar::BAR_FOCUSED;
}

void UICheckbox::SetTopLeft( float x, float y )
{
	relTopLeft = Vector2f( x, y );
	bar->SetTopLeft( x, y );
}

const sf::Vector2f &UICheckbox::GetTopLeftRel()
{
	return bar->GetTopLeftRel();
}

const sf::Vector2f &UICheckbox::GetTopLeftGlobal()
{
	return bar->GetTopLeftGlobal();
}


bool UICheckbox::Update( ControllerState &curr,
	ControllerState &prev )
{
	CheckboxEventParams params;
	params.checkbox = this;

	if( curr.A && !prev.A && !pressedDown )
	{
		pressedDown = true;
		
	}
	else if( !curr.A && prev.A && pressedDown )
	{
		pressedDown = false;
		checked = !checked;
		if( checked )
		{
			bar->SetState( UIBar::BAR_ALT0 );
		}
		else
		{
			bar->SetState( UIBar::BAR_ALT1 );
		}
		
		if( handler != NULL )
		{
			return handler->CheckboxEvent( UIEvent::E_CHECKBOX_CHANGED, &params );
		}
	}

	return false;
}

void UICheckbox::Draw( sf::RenderTarget *target )
{
	bar->Draw( target );
}