#include "UIWindow.h"
#include "GameSession.h"
#include "Tileset.h"
#include <string>

using namespace sf;
using namespace std;


UICheckbox::UICheckbox( UIControl *p_parent, UIEventHandlerBase *p_handler, int p_numControls, UIControl **p_controls, int p_spacing )
	:UIControl( p_parent, p_handler, UI_BUTTON ), spacing( p_spacing ), numControls( p_numControls ), 
	controls( p_controls ), focusedIndex( 0 )
{
	controls = new UIControl*[numControls];
	for( int i = 0; i < numControls; ++i )
	{
		controls[i] = p_controls[i];
		controls[i]->parent = this;
	}

	SetTopLeft( 0, 0 );
}

UICheckbox::~UICheckbox()
{
	for( int i = 0; i < numControls; ++i )
	{
		delete controls[i];
	}
	delete [] controls;
}

void UICheckbox::Unfocus()
{
	UIControl::Focus();
	pressedDown = false;
}

void UICheckbox::SetTopLeft( float x, float y )
{
	relTopLeft = Vector2f( x, y );
	if( parent != NULL )
	{
		globalTopLeft = relTopLeft + parent->GetTopLeftGlobal();
	}
	else
	{
		globalTopLeft = relTopLeft;
	}

	int spaceCounter = 0;
	for( int i = 0; i < numControls; ++i )
	{
		controls[i]->SetTopLeft( globalTopLeft.x, spaceCounter );
		spaceCounter += controls[i]->GetHeight() + spacing;
	}
}

const sf::Vector2f &UICheckbox::GetTopLeftGlobal()
{
	return globalTopLeft;
}

bool UICheckbox::Update( ControllerState &curr,
	ControllerState &prev )
{
	bool up = curr.LUp();
	bool down = curr.LDown();

	int oldIndex = focusedIndex;
	bool consumed = controls[focusedIndex]->Update( curr, prev );

	if( down )
	{
		if( focusedIndex < numControls - 1 )
		{
			focusedIndex++;
		}
		else
		{
			focusedIndex = 0;
		}
	}
	else if( up )
	{
		if( focusedIndex > 0 )
		{
			focusedIndex--;
		}
		else
		{
			focusedIndex = numControls - 1;	
		}
	}

	if( focusedIndex != oldIndex )
	{
		controls[oldIndex]->focused = false;
		controls[focusedIndex]->focused = true; //might be unnecessary
	}

	return false;
}

void UICheckbox::Draw( sf::RenderTarget *target )
{
	for( int i = 0; i < numControls; ++i )
	{
		controls[i]->Draw( target );
	}
}