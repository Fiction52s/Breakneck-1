#include "UIWindow.h"
#include "GameSession.h"
#include "Tileset.h"
#include <string>

using namespace sf;
using namespace std;

UIVerticalControlList::UIVerticalControlList( UIControl *p_parent,
	int p_numControls, UIControl **p_controls, int p_spacing )
	:UIControl( p_parent, NULL, UI_VERTICAL_CONTROL_LIST ), spacing( p_spacing ), numControls( p_numControls ), 
	controls( p_controls ), focusedIndex( 0 )
{
	controls = new UIControl*[numControls];
	for( int i = 0; i < numControls; ++i )
	{
		controls[i] = p_controls[i];
		controls[i]->parent = this;
	}

	SetTopLeft( 0, 0 );

	waitFrames[0] = 10;
	waitFrames[1] = 5;
	waitFrames[2] = 2;

	waitModeThresh[0] = 2;
	waitModeThresh[1] = 2;

	currWaitLevel = 0;
	flipCounterUp = 0;
	flipCounterDown = 0;
	framesWaiting = 0;
}

UIVerticalControlList::~UIVerticalControlList()
{
	for( int i = 0; i < numControls; ++i )
	{
		delete controls[i];
	}
	delete [] controls;
}

void UIVerticalControlList::SetTopLeft( float x, float y )
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

const sf::Vector2f &UIVerticalControlList::GetTopLeftGlobal()
{
	return globalTopLeft;
}

bool UIVerticalControlList::Update( ControllerState &curr,
	ControllerState &prev )
{
	bool up = curr.LUp();
	bool down = curr.LDown();

	int oldIndex = focusedIndex;
	bool consumed = controls[focusedIndex]->Update( curr, prev );

	if( down )
	{
		if( flipCounterDown == 0 
			|| ( flipCounterDown > 0 && framesWaiting == waitFrames[currWaitLevel] )
			)
		{
			if( flipCounterDown == 0 )
			{
				currWaitLevel = 0;
			}

			++flipCounterDown;

			if( flipCounterDown == waitModeThresh[currWaitLevel] && currWaitLevel < 2 )
			{
				currWaitLevel++;
			}

			flipCounterUp = 0;
			framesWaiting = 0;

			if( focusedIndex < numControls - 1 )
			{
				focusedIndex++;
			}
			else
			{
				focusedIndex = 0;
			}
		}
		else
		{
			++framesWaiting;
		}
		
	}
	else if( up )
	{
		if( flipCounterUp == 0 
			|| ( flipCounterUp > 0 && framesWaiting == waitFrames[currWaitLevel] )
			)
		{
			if( flipCounterUp == 0 )
			{
				currWaitLevel = 0;
			}

			++flipCounterUp;

			if( flipCounterUp == waitModeThresh[currWaitLevel] && currWaitLevel < 2 )
			{
				currWaitLevel++;
			}

			flipCounterDown = 0;
			framesWaiting = 0;
			if( focusedIndex > 0 )
			{
				focusedIndex--;
			}
			else
			{
				focusedIndex = numControls - 1;	
			}
		}
		else
		{
			++framesWaiting;
		}
		
	}
	else
	{
		flipCounterUp = 0;
		flipCounterDown = 0;
		currWaitLevel = 0;
		framesWaiting = 0;
	}

	if( focusedIndex != oldIndex )
	{
		controls[oldIndex]->Unfocus();
		controls[focusedIndex]->Focus();
	}

	return false;
}

void UIVerticalControlList::Draw( sf::RenderTarget *target )
{
	for( int i = 0; i < numControls; ++i )
	{
		controls[i]->Draw( target );
	}
}