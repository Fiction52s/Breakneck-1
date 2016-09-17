#include "PauseMenu.h"
#include <sstream>
#include "GameSession.h"
#include "MainMenu.h"
#include <iostream>

using namespace sf;
using namespace std;


OptionSelector::OptionSelector( Vector2f &p_pos, int p_optionCount,
		std::string *p_options )
		:optionCount( p_optionCount ), options( NULL ), pos( p_pos ), accelBez( 0, 0, 1, 1 )
{
	currentText.setFont( *MainMenu::arial );
	currentText.setCharacterSize( 80 );
	currentText.setColor( Color::White );
	options = new string[optionCount];
	for( int i = 0; i < optionCount; ++i )
	{
		options[i] = p_options[i];
	}
	currentIndex = 0;

	currentText.setString( options[currentIndex] );
	currentText.setPosition( pos );

	maxWaitFrames = 30;
	currWaitFrames = maxWaitFrames;
	minWaitFrames = 4;
	framesWaiting = maxWaitFrames;

	momentum = 0;

	maxMomentum = 4;
}

void OptionSelector::Right()
{
	if( framesWaiting >= currWaitFrames )
	{
		currentIndex++;
		if( currentIndex == optionCount )
			currentIndex = 0;

		currentText.setString( options[currentIndex] );

		if( momentum <= 0 )
		{
			momentum = 1;
		}
		else if( momentum < maxMomentum )
		{
			momentum++;
		}

		//CubicBezier bez( 0, 0, 1, 1 );

		double v = accelBez.GetValue( momentum / (double)maxMomentum );
		framesWaiting = 0;
		currWaitFrames = floor( (maxWaitFrames * ( 1 - v ) + minWaitFrames * v) + .5 );
		//cout << "switch right" << endl;
	}
}


void OptionSelector::Left()
{
	if( framesWaiting >= currWaitFrames )
	{
		currentIndex--;
		if( currentIndex < 0 )
			currentIndex = optionCount - 1;

		currentText.setString( options[currentIndex] );

		if( momentum >= 0 )
		{
			momentum = -1;
		}
		else if( momentum > -maxMomentum )
		{
			momentum--;
		}

		//CubicBezier bez( 0, 0, 1, 1 );

		double v = accelBez.GetValue( (-momentum) / (double)maxMomentum );
		framesWaiting = 0;
		currWaitFrames = floor( (maxWaitFrames * ( 1 - v ) + minWaitFrames * v) + .5 );
	}
}

void OptionSelector::Stop()
{
	framesWaiting = maxWaitFrames;
	momentum = 0;
	//currWaitFrames = maxWaitFrames;
}

void OptionSelector::Update()
{
	framesWaiting++;
}

const std::string & OptionSelector::GetString()
{
	return options[currentIndex];
}

void OptionSelector::Draw( sf::RenderTarget *target )
{
	target->draw( currentText );
}

PauseMenu::PauseMenu( GameSession *p_owner )
	:owner( p_owner ), currentTab( Tab::MAP )
{
	ts_background[0] = owner->GetTileset( "Menu/pause_1_map_1820x980.png", 1820, 980 );
	ts_background[1] = owner->GetTileset( "Menu/pause_2_kin_1820x980.png", 1820, 980 );
	ts_background[2] = owner->GetTileset( "Menu/pause_3_shards_1820x980.png", 1820, 980 );
	ts_background[3] = owner->GetTileset( "Menu/pause_4_options_1820x980.png", 1820, 980 );
	ts_background[4] = owner->GetTileset( "Menu/pause_5_pause_1820x980.png", 1820, 980 );

	ts_select = owner->GetTileset( "Menu/menu_select_800x140.png", 800, 140 );
	selectSprite.setTexture( *ts_select->texture );

	bgSprite.setPosition( 0, 0 );

	pauseSelectIndex = 0;

	numVideoOptions = 2;
	videoSelectors = new OptionSelector*[numVideoOptions];


	string resolutions[] = { "1920 x 1080", "1600 x 900" };
	videoSelectors[0] = new OptionSelector( Vector2f( 100, 20 ), 2, resolutions );

	string test[] = { "0", "1" };
	videoSelectors[1] = new OptionSelector( Vector2f( 100, 100 ), 2, test );

	currentSelectors = videoSelectors;
	numCurrentSelectors = numVideoOptions;
	optionSelectorIndex = 0;

	/*maxWaitFrames = 30;
	currWaitFrames = maxWaitFrames;
	minWaitFrames = 4;
	framesWaiting = maxWaitFrames;

	momentum = 0;

	maxMomentum = 4;*/

	//bgSprite.setPosition( (1920 - 1820) / 2, (1080 - 980) / 2);

	//SetTab( MAP );
}

PauseMenu::~PauseMenu()
{
	for( int i = 0; i < numVideoOptions; ++i )
	{
		delete videoSelectors[i];
	}
	delete [] videoSelectors;
}

void PauseMenu::TabLeft()
{
	int index = (int)currentTab;
	index--;
	if( index < 0 )
		index = 4;

	currentTab = (Tab)index;
	SetTab( currentTab );
}

void PauseMenu::TabRight()
{
	int index = (int)currentTab;
	index++;
	if( index > 4 )
		index = 0;

	currentTab = (Tab)index;
	SetTab( currentTab );
}

void PauseMenu::SetTab( Tab t )
{
	currentTab = t;
	bgSprite.setTexture( *ts_background[currentTab]->texture );

	switch( t )
	{
	case MAP:
		mapCenter.x = owner->player->position.x;
		mapCenter.y = owner->player->position.y;
		mapZoomFactor = 16;	
		break;
	case KIN:
		break;
	case SHARDS:
		break;
	case OPTIONS:
		break;
	case PAUSE:
		pauseSelectIndex = 0;
		break;
	}
}

void PauseMenu::Draw( sf::RenderTarget *target )
{
	target->draw( bgSprite );

	if( currentTab == PAUSE )
	{
		target->draw( selectSprite );
	}
	else if( currentTab == OPTIONS )
	{
		for( int i = 0; i < numVideoOptions; ++i )
		{
			videoSelectors[i]->Draw( target );
		}
	}
}

PauseMenu::UpdateResponse PauseMenu::Update( ControllerState &currInput,
		ControllerState &prevInput )
{
	if( (currInput.start && !prevInput.start) || (currInput.back && !prevInput.back ) )
	{
		owner->state = GameSession::State::RUN;
		owner->soundNodeList->Pause( false );
	}

	if( currInput.leftShoulder && !prevInput.leftShoulder )
	{
		TabLeft();
		return R_NONE;
	}
	else if( currInput.rightShoulder && !prevInput.rightShoulder )
	{
		TabRight();
		return R_NONE;
	}


	switch( currentTab )
	{
	case MAP:
		{
			float fac = .05;
			if( currInput.A )
			{
				mapZoomFactor -= fac * mapZoomFactor;
			}
			else if( currInput.B )
			{
				mapZoomFactor += fac * mapZoomFactor;
			}

			if( mapZoomFactor < 1.f )
			{
				mapZoomFactor = 1.f;
			}
			else if( mapZoomFactor > 128.f )
			{
				mapZoomFactor = 128.f;
			}

			float move = 20.0 * mapZoomFactor / 2.0;
			if( currInput.LLeft() )
			{
				mapCenter.x -= move;
			}
			else if( currInput.LRight() )
			{
				mapCenter.x += move;
			}

			if( currInput.LUp() )
			{
				mapCenter.y -= move;
			}
			else if( currInput.LDown() )
			{
				mapCenter.y += move;
			}

			if( mapCenter.x < owner->leftBounds )
			{
				mapCenter.x = owner->leftBounds;
			}
			else if( mapCenter.x > owner->leftBounds + owner->boundsWidth )
			{
				mapCenter.x = owner->leftBounds + owner->boundsWidth;
			}

			if( mapCenter.y < owner->topBounds )
			{
				mapCenter.y = owner->topBounds;
			}
			else if( mapCenter.y > owner->topBounds + owner->boundsHeight )
			{
				mapCenter.y = owner->topBounds + owner->boundsHeight;
			}
			break;
		}
	case KIN:
		{
			break;
		}
	case SHARDS:
		{
			break;
		}
	case OPTIONS:
		{	
		
			if( currInput.LDown() && !prevInput.LDown() )
			{
				optionSelectorIndex++;
				if( optionSelectorIndex == numCurrentSelectors )
				{
					optionSelectorIndex = 0;
				}		
			}
			else if( currInput.LUp() && !prevInput.LUp() )
			{
				optionSelectorIndex--;
				if( optionSelectorIndex == -1 )
				{
					optionSelectorIndex = numCurrentSelectors - 1;
				}
			}
			else if( currInput.LRight() )
			{
				//cout << "optionindex: " << optionSelectorIndex << endl;
				//cout << "num curr selcts: " << numCurrentSelectors << endl;
				currentSelectors[optionSelectorIndex]->Right();
			}
			else if( currInput.LLeft() )
			{
				//cout << "optionindex: " << optionSelectorIndex << endl;
				//cout << "num curr selcts: " << numCurrentSelectors << endl;
				currentSelectors[optionSelectorIndex]->Left();
			}
			else if( !currInput.LLeft() && !currInput.LRight() )
			{
				currentSelectors[optionSelectorIndex]->Stop();
			}
			currentSelectors[optionSelectorIndex]->Update();
			/*for( int i = 0; i < numCurrentSelectors; ++i )
			{
				currentSelectors[i]->Update();
			}*/
			
			break;
		}
	case PAUSE:
		{
			if( currInput.A && !prevInput.A )
			{
				UpdateResponse ur = (UpdateResponse)(pauseSelectIndex+1);
				return ur;
				//switch( pauseSelectIndex )
				//{
				//case 0: //resume
				//	{
				//		//owner->state = GameSession::RUN;
				//		//owner->soundNodeList->Pause( false );
				//		break;
				//	}
				//case 1: //restart
				//	{

				//		break;
				//	}
				//case 2: //exit level
				//	break;
				//case 3: //exit to title
				//	break;
				//case 4: //exit game
				//	break;
				//}
			}
			else if( currInput.LDown() && !prevInput.LDown() )
			{
				pauseSelectIndex++;
				if( pauseSelectIndex == 5 )
				{
					pauseSelectIndex = 0;
				}

				
			}
			else if( currInput.LUp() && !prevInput.LUp() )
			{
				pauseSelectIndex--;
				if( pauseSelectIndex == -1 )
				{
					pauseSelectIndex = 4;
				}
			}

			int h = 200;
			selectSprite.setPosition( 100, 100 + h * pauseSelectIndex );
			break;
		}
	}

	return R_NONE;
}
//using namespace std;
//using namespace sf;