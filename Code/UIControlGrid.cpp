#include "UIWindow.h"
#include "GameSession.h"
#include "Tileset.h"
#include <string>

using namespace sf;
using namespace std;

UIControlGrid::UIControlGrid( UIControl *p_parent, int p_numControlsX, int p_numControlsY,
		UIControl **p_controls, int p_spacingX, int p_spacingY, bool horizontalDominant )
		:UIControl( p_parent, NULL, UI_CONTROLGRID ), spacingX( p_spacingX ), spacingY( p_spacingY ),
			numControlsX( p_numControlsX ), numControlsY( p_numControlsY ),
			focusedIndexX( 0 ), focusedIndexY( 0 )
{
	controls = p_controls;//new UIControl*[numControlsX * numControlsY];
	int index;
	for( int i = 0; i < numControlsX; ++i )
	{
		for( int j = 0; j < numControlsY; ++j )
		{
			index = i*numControlsY + j;
			controls[index]->parent = this;
		}
	}

	waitFrames[0] = 20;
	waitFrames[1] = 10;
	waitFrames[2] = 5;

	waitModeThresh[0] = 2;
	waitModeThresh[1] = 2;

	currWaitLevelHoriz = 0;
	currWaitLevelVert = 0;
	flipCounterLeft = 0;
	flipCounterRight = 0;
	flipCounterLeft = 0;
	flipCounterRight = 0;
	framesWaitingHoriz = 0;
	framesWaitingVert = 0;

	controls[focusedIndexX * numControlsY + focusedIndexY]->Focus();
}

UIControlGrid::~UIControlGrid()
{
	
	//delete [][] controls;
}

bool UIControlGrid::Update( ControllerState &curr, ControllerState &prev )
{
	bool up = curr.LUp();
	bool down = curr.LDown();
	bool left = curr.LLeft();
	bool right = curr.LRight();

	int oldIndexX = focusedIndexX;
	int oldIndexY = focusedIndexY;
	bool consumed = controls[focusedIndexX * numControlsY + focusedIndexY]
		->Update( curr, prev );

	if( down )
	{
		if( flipCounterDown == 0 
			|| ( flipCounterDown > 0 && framesWaitingVert == waitFrames[currWaitLevelVert] )
			)
		{
			if( flipCounterDown == 0 )
			{
				currWaitLevelVert = 0;
			}

			++flipCounterDown;

			if( flipCounterDown == waitModeThresh[currWaitLevelVert] && currWaitLevelVert < 2 )
			{
				currWaitLevelVert++;
			}

			flipCounterUp = 0;
			//flipCounterLeft = 0;
			//flipCounterRight = 0;
			framesWaitingVert = 0;

			if( focusedIndexY < numControlsY - 1 )
			{
				focusedIndexY++;
			}
			else
			{
				focusedIndexY = 0;
			}
		}
		else
		{
			++framesWaitingVert;
		}
		
	}
	else if( up )
	{
		if( flipCounterUp == 0 
			|| ( flipCounterUp > 0 && framesWaitingVert == waitFrames[currWaitLevelVert] )
			)
		{
			if( flipCounterUp == 0 )
			{
				currWaitLevelVert = 0;
			}

			++flipCounterUp;

			if( flipCounterUp == waitModeThresh[currWaitLevelVert] && currWaitLevelVert < 2 )
			{
				currWaitLevelVert++;
			}

			flipCounterDown = 0;
			//flipCounterRight = 0;
			//flipCounterLeft = 0;
			framesWaitingVert = 0;
			if( focusedIndexY > 0 )
			{
				focusedIndexY--;
			}
			else
			{
				focusedIndexY = numControlsY - 1;	
			}
		}
		else
		{
			++framesWaitingVert;
		}
		
	}
	else
	{
		flipCounterUp = 0;
		flipCounterDown = 0;
		currWaitLevelVert = 0;
		framesWaitingVert = 0;
	}

	if( right )
	{
		if( flipCounterRight == 0 
			|| ( flipCounterRight > 0 && framesWaitingHoriz == waitFrames[currWaitLevelHoriz] )
			)
		{
			if( flipCounterRight == 0 )
			{
				currWaitLevelHoriz = 0;
			}

			++flipCounterRight;

			if( flipCounterRight == waitModeThresh[currWaitLevelHoriz] && currWaitLevelHoriz < 2 )
			{
				currWaitLevelHoriz++;
			}

			//flipCounterDown = 0;
			flipCounterLeft = 0;
			//flipCounterUp = 0;
			framesWaitingHoriz = 0;

			if( focusedIndexX < numControlsX - 1 )
			{
				focusedIndexX++;
			}
			else
			{
				focusedIndexX = 0;
			}
		}
		else
		{
			++framesWaitingHoriz;
		}
		
	}
	else if( left )
	{
		if( flipCounterLeft == 0 
			|| ( flipCounterLeft > 0 && framesWaitingHoriz == waitFrames[currWaitLevelHoriz] )
			)
		{
			if( flipCounterLeft == 0 )
			{
				currWaitLevelHoriz = 0;
			}

			++flipCounterLeft;

			if( flipCounterLeft == waitModeThresh[currWaitLevelHoriz] && currWaitLevelHoriz < 2 )
			{
				currWaitLevelHoriz++;
			}

			//flipCounterUp = 0;
			flipCounterRight = 0;
			//flipCounterDown = 0;
			framesWaitingHoriz = 0;
			if( focusedIndexX > 0 )
			{
				focusedIndexX--;
			}
			else
			{
				focusedIndexX = numControlsX - 1;	
			}
		}
		else
		{
			++framesWaitingHoriz;
		}
		
	}
	else
	{
		flipCounterLeft = 0;
		flipCounterRight = 0;
		currWaitLevelHoriz = 0;
		framesWaitingHoriz = 0;
	}

	if( focusedIndexX != oldIndexX || focusedIndexY !=  oldIndexY )
	{
		controls[oldIndexX * numControlsY + oldIndexY]->Unfocus();
		controls[focusedIndexX * numControlsY + focusedIndexY]->Focus();
	}

	return false;
}

const sf::Vector2f &UIControlGrid::GetTopLeftGlobal()
{
	return globalTopLeft;
}

void UIControlGrid::SetTopLeft( float x, float y )
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

	//int spaceCounterY = 0;
	//int spaceCounterX = 0;
	for( int i = 0; i < numControlsX; ++i )
	{
		for( int j = 0; j < numControlsY; ++j )
		{
			controls[i*numControlsY+j]->SetTopLeft( globalTopLeft.x + i * (controls[i*numControlsY+j]->GetWidth() + spacingX ), 
				globalTopLeft.y + j * (controls[i*numControlsY+j]->GetHeight() + spacingY ) );
			//spaceCounter += controls[i]->GetHeight() + spacing;
		}
	}
	/*for( int i = 0; i < numControls; ++i )
	{
		controls[i]->SetTopLeft( globalTopLeft.x, spaceCounter );
		spaceCounter += controls[i]->GetHeight() + spacing;
	}*/
}

void UIControlGrid::Draw( sf::RenderTarget *target )
{
	for( int i = 0; i < numControlsX * numControlsY; ++i )
	{
		controls[i]->Draw( target );
	}
}