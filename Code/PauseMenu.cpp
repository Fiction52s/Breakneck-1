#include "PauseMenu.h"
#include <sstream>
#include "GameSession.h"
#include "MainMenu.h"
#include <iostream>
#include <sstream>
#include "SaveFile.h"

using namespace sf;
using namespace std;


OptionSelector::OptionSelector( Vector2f &p_pos, int p_optionCount,
		std::string *p_options )
		:optionCount( p_optionCount ), options( NULL ), pos( p_pos ), accelBez( 0, 0, 1, 1 )
{
	selected = false;
	currentText.setFont( MainMenu::arial );
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
	if( framesWaiting >= currWaitFrames || momentum <= 0 )
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
	if( framesWaiting >= currWaitFrames || momentum >= 0 )
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
	if( selected )
		currentText.setColor( Color::Red );
	else
		currentText.setColor( Color::White );
	target->draw( currentText );
}

PauseMenu::PauseMenu( GameSession *p_owner )
	:owner( p_owner ), currentTab( Tab::MAP ),  accelBez( 0, 0, 1, 1 ),
	assocSymbols( sf::Quads, ( ControllerSettings::ButtonType::Count + 4 ) * 4 )
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

	string inputTypes[] = { "Xbox", "Keyboard", "PS4", "Gamecube" };
	inputSelectors = new OptionSelector*[3];

	inputSelectors[0] = new OptionSelector( Vector2f( 100, 100 ), 4, inputTypes );

	string settings[] = {"Setting 1", "Setting 2", "Setting 3" };

	string thumbSticks[] = { "left analog", "right analog" };

	inputSelectors[1] = new OptionSelector( Vector2f( 100, 180 ), 3, settings );

	inputSelectors[2] = new OptionSelector( Vector2f( 300, 180 ), 2, thumbSticks );

	string bounceSpecial = "bounce";
	string grindSpecial = "grind";
	string timeslowSpecial = "time slow";
	string wireLeftSpecial = "left wire";
	string wireRightSpecial = "right wire";
	string toggleBounce = "toggle bounce";
	string toggleGrind = "toggle grind";
	string toggleTimeSlow = "toggle time slow";

	if( owner->saveFile != NULL )
	{
		SaveFile *sf = owner->saveFile;

		//if they dont have the power yet, rename them to specials 1-5 

		//if 
		//sf->
		
	}
	possibleControllerActions = new string[12];
	possibleControllerActions[0] = "move";
	possibleControllerActions[1] = "jump";
	possibleControllerActions[2] = "dash";
	possibleControllerActions[3] = "attack";
	possibleControllerActions[4] = bounceSpecial;
	possibleControllerActions[5] = grindSpecial;
	possibleControllerActions[6] = timeslowSpecial;
	possibleControllerActions[7] = wireLeftSpecial;
	possibleControllerActions[8] = wireRightSpecial;
	possibleControllerActions[9] = toggleBounce;
	possibleControllerActions[10] = toggleGrind;
	possibleControllerActions[11] = toggleTimeSlow;
	/*{ 
		"move", "jump", "dash", "attack", bounceSpecial, grindSpecial,
		timeslowSpecial, wireLeftSpecial, wireRightSpecial, 
		toggleBounce, toggleGrind, toggleTimeSlow };*/

	possibleKeyboardActions = new string[15];
	possibleKeyboardActions[0] = "up";
	possibleKeyboardActions[1] = "left";
	possibleKeyboardActions[2] = "down";
	possibleKeyboardActions[3] = "right";
	possibleKeyboardActions[4] = "jump";
	possibleKeyboardActions[5] = "dash";
	possibleKeyboardActions[6] = "attack";
	possibleKeyboardActions[7] = bounceSpecial;
	possibleKeyboardActions[8] = grindSpecial;
	possibleKeyboardActions[9] = timeslowSpecial;
	possibleKeyboardActions[10] = wireLeftSpecial;
	possibleKeyboardActions[11] = wireRightSpecial;
	possibleKeyboardActions[12] = toggleBounce;
	possibleKeyboardActions[13] = toggleGrind;
	possibleKeyboardActions[14] = toggleTimeSlow;

	/*string possibleKeyboardActions[15] = { 
	"up", "left", "down", "right", "jump", "dash", "attack", bounceSpecial, grindSpecial,
	timeslowSpecial, wireLeftSpecial, wireRightSpecial, 
	toggleBounce, toggleGrind, toggleTimeSlow };*/


	

	//resolution
	//fullscreen
	//vsync

	//video
	string resolutions[] = { "1920 x 1080", "1600 x 900" };
	videoSelectors[0] = new OptionSelector( Vector2f( 100, 20 ), 2, resolutions );

	string windowType[] = { "Fullscreen", "Window", "Borderless Window" };
	videoSelectors[1] = new OptionSelector( Vector2f( 100, 100 ), 3, windowType );
	
	string vsync[] = { "on", "off" };
	videoSelectors[2] = new OptionSelector( Vector2f( 100, 180 ), 2, vsync );

	string masterVolume[101];
	string musicVolume[101];
	string soundVolume[101];
	for( int i = 0; i < 101; ++i )
	{
		stringstream ss;
		ss << i;

		masterVolume[i] = ss.str();
		musicVolume[i] = ss.str();
		soundVolume[i] = ss.str();
	}

	numSoundOptions = 5;
	soundSelectors = new OptionSelector*[numSoundOptions];

	string enable[] = {"on", "off"};

	soundSelectors[0] = new OptionSelector( Vector2f( 100, 20 ), 101, masterVolume );
	soundSelectors[1] = new OptionSelector( Vector2f( 100, 180 ), 2, enable );
	soundSelectors[2] = new OptionSelector( Vector2f( 100, 260 ), 101, musicVolume );
	soundSelectors[3] = new OptionSelector( Vector2f( 100, 340 ), 2, enable );
	soundSelectors[4] = new OptionSelector( Vector2f( 100, 420 ), 101, soundVolume );

	

	currentSelectors = inputSelectors;//soundSelectors;
	numCurrentSelectors = 3;//numSoundOptions;
	optionSelectorIndex = 0;
	maxWaitFrames = 30;
	currWaitFrames = maxWaitFrames;
	minWaitFrames = 4;
	momentum = 0;
	maxMomentum = 4;
	/*maxWaitFrames = 30;
	currWaitFrames = maxWaitFrames;
	minWaitFrames = 4;
	framesWaiting = maxWaitFrames;
	momentum = 0;
	maxMomentum = 4;*/

	//bgSprite.setPosition( (1920 - 1820) / 2, (1080 - 980) / 2);

	//SetTab( MAP );
	SetAssocSymbols();

}

PauseMenu::~PauseMenu()
{
	/*for( int i = 0; i < numVideoOptions; ++i )
	{
		delete videoSelectors[i];
	}
	delete [] videoSelectors;*/
}

void PauseMenu::SetAssocSymbols()
{
	int symbolX = 200;
	int symbolSize = 50;
	int spacing = 16;
	string inputType = inputSelectors[0]->GetString();
	if( inputType == "Xbox" )
	{
		int numAssocSymbols = ControllerSettings::ButtonType::Count + 1;
		
		//move comes first
		for( int i = 0; i < numAssocSymbols; ++i )
		{
			assocSymbols[i*4+0].color = Color::Red;
			assocSymbols[i*4+1].color = Color::Green;
			assocSymbols[i*4+2].color = Color::Blue;
			assocSymbols[i*4+3].color = Color::Magenta;

			assocSymbols[i*4+0].position = Vector2f( symbolX, (symbolSize + spacing) * i );
			assocSymbols[i*4+1].position = Vector2f( symbolX + symbolSize, (symbolSize + spacing) * i );
			assocSymbols[i*4+2].position = Vector2f( symbolX + symbolSize, (symbolSize + spacing) * i + symbolSize );
			assocSymbols[i*4+3].position = Vector2f( symbolX, (symbolSize + spacing) * i + symbolSize );
		}
		for( int i = numAssocSymbols; i < numAssocSymbols + 3; ++i )
		{
			assocSymbols[i*4+0].position = Vector2f( 0, 0 );
			assocSymbols[i*4+1].position = Vector2f( 0, 0 );
			assocSymbols[i*4+2].position = Vector2f( 0, 0 );
			assocSymbols[i*4+3].position = Vector2f( 0, 0 );
		}

		for( int i = 0; i < numAssocSymbols; ++i )
		{
			actionText[i].setFont( owner->arial );
			actionText[i].setCharacterSize( 24 );
			actionText[i].setPosition( 100, 50 * i );
			actionText[i].setColor( Color::White );
			actionText[i].setString( possibleControllerActions[i] );
		}
	}
	else if( inputType == "Keyboard" )
	{
		int numAssocSymbols = ControllerSettings::ButtonType::Count + 4;
		
		//move comes first
		for( int i = 0; i < numAssocSymbols; ++i )
		{
			assocSymbols[i*4+0].color = Color::Red;
			assocSymbols[i*4+1].color = Color::Green;
			assocSymbols[i*4+2].color = Color::Blue;
			assocSymbols[i*4+3].color = Color::Magenta;

			assocSymbols[i*4+0].position = Vector2f( symbolX, (symbolSize + spacing) * i );
			assocSymbols[i*4+1].position = Vector2f( symbolX + symbolSize, (symbolSize + spacing) * i );
			assocSymbols[i*4+2].position = Vector2f( symbolX + symbolSize, (symbolSize + spacing) * i + symbolSize );
			assocSymbols[i*4+3].position = Vector2f( symbolX, (symbolSize + spacing) * i + symbolSize );
		}

		for( int i = 0; i < numAssocSymbols; ++i )
		{
			actionText[i].setFont( owner->arial );
			actionText[i].setCharacterSize( 24 );
			actionText[i].setPosition( 100, 50 * i );
			actionText[i].setColor( Color::White );
			actionText[i].setString( possibleKeyboardActions[i] );
		}
	}
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
		string inputType = inputSelectors[0]->GetString();
		int num = numCurrentSelectors;
		if( inputType == "Keyboard" )
		{
			num = num - 1;
		}
		for( int i = 0; i < num; ++i )
		{
			currentSelectors[i]->Draw( target );
		}
		
		if( currentSelectors == inputSelectors )
		{
			target->draw( assocSymbols );

			
			int cap;
			if( inputType == "Xbox" )
			{
				cap = 12;
			}
			else if( inputType == "Keyboard" )
			{
				cap = 15;
			}
			else
			{
				//test
				cap = 8;
			}
			for( int i = 0; i < cap; ++i )
			{
				target->draw( actionText[i] );
			}
		}

		
	}
}

void PauseMenu::ApplyVideoSettings()
{
	string resolution = videoSelectors[0]->GetString();
	string windowType = videoSelectors[1]->GetString();
	string vsync = videoSelectors[2]->GetString();

	int width, height;
	int style;
	if( resolution == "1920 x 1080" )
	{
		width = 1920;
		height = 1080;
		//owner->mainMenu->ResizeWindow( 1920, 1080 );
		/*sf::VideoMode( windowWidth, windowHeight ), "Breakneck", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 0, 0 ) );*/
		//owner->window->create( VideoMode( 1920, 1080 ), "Breakneck",sf::Style::None, sf::ContextSettings( 0, 0, 0, 0, 0 )  );

		//owner->updatewin
		//owner->sets
	}
	else if( resolution == "1600 x 900" )
	{
		width = 1600;
		height = 900;
		//owner->mainMenu->ResizeWindow( 1600, 900 );
	}
	else
	{
		assert( 0 );
	}
	

	if( windowType == "Fullscreen" )
	{
		style = sf::Style::Fullscreen;
	}
	else if( windowType == "Window" )
	{
		style = sf::Style::Default;
	}
	else if( windowType == "Borderless Window" )
	{
		style = sf::Style::None;
	}
	else
	{
		assert( 0 );
	}
	owner->mainMenu->ResizeWindow( width, height, style );


	if( vsync == "on" )
	{
		owner->window->setVerticalSyncEnabled( true );
	}
	else if( vsync == "off" )
	{
		owner->window->setVerticalSyncEnabled( false );
	}
	else
	{
		assert( 0 );
	}
	
}

void PauseMenu::ApplySoundSettings()
{
	/* masterVolume 
	enable );
	, musicVolume 
	enable );
	, soundVolume */


	string masterVolume = soundSelectors[0]->GetString();
	string enableMusic = soundSelectors[1]->GetString();
	string musicVolume = soundSelectors[2]->GetString();
	string enableSounds= soundSelectors[3]->GetString();
	string soundVolume = soundSelectors[4]->GetString();

	int width, height;
	int style;
	int master,music,sound;
	stringstream ss;

	master = stoi( masterVolume );
	music = stoi( musicVolume );
	sound = stoi( soundVolume );

	cout << "apply master: " << master << ", music: " << music << ", sound: " << sound << endl;

	bool enSounds;
	if( enableSounds == "on" )
		enSounds = true;
	else if( enableSounds == "off" )
		enSounds = false;

	bool enMusic;
	if( enableMusic == "on" )
		enMusic = true;
	else if( enableMusic == "off" )
		enMusic = false;


	owner->soundNodeList->SetGlobalVolume( master );
	owner->mainMenu->soundNodeList->SetGlobalVolume( master );

	owner->soundNodeList->SetSoundsEnable( enSounds );
	owner->mainMenu->soundNodeList->SetSoundsEnable( enSounds );

	//owner->soundNodeList->SetMusicEnable( enMusic );
	//owner->mainMenu->soundNodeList->SetMusicEnable( enMusic );

	owner->soundNodeList->SetRelativeSoundVolume( sound );
	owner->mainMenu->soundNodeList->SetRelativeSoundVolume( sound );

	//owner->soundNodeList->SetRelativeMusicVolume( music );
	//owner->mainMenu->soundNodeList->SetRelativeMusicVolume( music );
}

//run this in a loop
XBoxButton PauseMenu::CheckXBoxInput( ControllerState &currInput )
{
	bool leftMovement = (inputSelectors[2]->GetString() == "left analog");
	if( currInput.A )
	{
		return XBOX_A;
	}
	else if( currInput.B )
	{
		return XBOX_B;
	}
	else if( currInput.X )
	{
		return XBOX_X;
	}
	else if( currInput.Y )
	{
		return XBOX_Y;
	}
	else if( currInput.leftShoulder )
	{
		return XBOX_L1;
	}
	else if( currInput.rightShoulder )
	{
		return XBOX_R1;
	}
	else if( currInput.LeftTriggerPressed() )
	{
		return XBOX_L2;
	}
	else if( currInput.RightTriggerPressed() )
	{
		return XBOX_R2;
	}
	else if( currInput.PLeft() )
	{
		return XBOX_PLEFT;
	}
	else if( currInput.PUp() )
	{
		return XBOX_PUP;
	}
	else if( currInput.PRight() )
	{
		return XBOX_PRIGHT;
	}
	else if( currInput.PDown() )
	{
		return XBOX_PDOWN;
	}
	else if( leftMovement && currInput.RLeft() )
	{
		return XBOX_RLEFT;
	}
	else if( leftMovement && currInput.RUp() )
	{
		return XBOX_RUP;
	}
	else if( leftMovement && currInput.RRight() )
	{
		return XBOX_RRIGHT;
	}
	else if( leftMovement && currInput.RDown() )
	{
		return XBOX_RDOWN;
	}
	else if( !leftMovement && currInput.LLeft() )
	{
		return XBOX_LLEFT;
	}
	else if( !leftMovement && currInput.LUp() )
	{
		return XBOX_LUP;
	}
	else if( !leftMovement && currInput.LRight() )
	{
		return XBOX_LRIGHT;
	}
	else if( !leftMovement && currInput.LDown() )
	{
		return XBOX_LDOWN;
	}
	else if( currInput.back )
	{
		return XBOX_BACK;
	}
	else if( currInput.start )
	{
		return XBOX_START;
	}
	else
	{
		return XBOX_BLANK;
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
			return UpdateOptions( currInput, prevInput );

		
			
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


PauseMenu::UpdateResponse PauseMenu::UpdateOptions( ControllerState &currInput,
	ControllerState &prevInput )
{
	if( currentSelectors == inputSelectors )
	{
		return UpdateInputOptions( currInput, prevInput );
	}
	else if( currentSelectors == videoSelectors )
	{
		return UpdateVideoOptions( currInput, prevInput );
	}
	else if( currentSelectors == soundSelectors )
	{
		return UpdateAudioOptions( currInput, prevInput );
	}
	else
	{
		assert( 0 );
		return R_NONE;
	}
	

	return R_NONE;
}

PauseMenu::UpdateResponse PauseMenu::UpdateVideoOptions(
	ControllerState &currInput, ControllerState &prevInput)
{
	if( currInput.A && !prevInput.A )
	{
		ApplyVideoSettings();
				
		return R_NONE;
	}
	else if( currInput.B && !prevInput.B )
	{
		if( currentSelectors == inputSelectors )
		{
			if( selectingProfile )
			{
				//go back to selecting option type
			}
			else
			{
				selectingProfile = true;
			}
		}
	}
	if( currInput.LDown() && ( framesWaiting >= currWaitFrames || momentum <= 0 ))
	{
		//cout << "DOWN" << endl;
		currentSelectors[optionSelectorIndex]->selected = false;

		optionSelectorIndex++;
		if( optionSelectorIndex == numCurrentSelectors )
		{
			optionSelectorIndex = 0;
		}

		currentSelectors[optionSelectorIndex]->selected = true;

		if( momentum <= 0 )
		{
			momentum = 1;
		}
		else if( momentum < maxMomentum )
		{
			momentum++;
		}

		//CubicBezier bez( 0, 0, 1, 1 );
		currentSelectors[optionSelectorIndex]->Stop();
		double v = accelBez.GetValue( momentum / (double)maxMomentum );
		framesWaiting = 0;
		currWaitFrames = floor( (maxWaitFrames * ( 1 - v ) + minWaitFrames * v) + .5 );
	}
	else if( currInput.LUp() && ( framesWaiting >= currWaitFrames || momentum >= 0 ))
	{
		//cout << "up! " << framesWaiting << ", " << momentum << endl;
		currentSelectors[optionSelectorIndex]->selected = false;

		optionSelectorIndex--;
		if( optionSelectorIndex == -1 )
		{
			optionSelectorIndex = numCurrentSelectors - 1;
		}

		currentSelectors[optionSelectorIndex]->selected = true;

		if( momentum >= 0 )
		{
			momentum = -1;
		}
		else if( momentum > -maxMomentum )
		{
			momentum--;
		}

		//CubicBezier bez( 0, 0, 1, 1 );
		currentSelectors[optionSelectorIndex]->Stop();
		double v = accelBez.GetValue( (-momentum) / (double)maxMomentum );
		framesWaiting = 0;
		currWaitFrames = floor( (maxWaitFrames * ( 1 - v ) + minWaitFrames * v) + .5 );

				
	}
	else if( currInput.LRight() )
	{
		//cout << "optionindex: " << optionSelectorIndex << endl;
		//cout << "num curr selcts: " << numCurrentSelectors << endl;
		//cout << "Attempted right" << endl;
		currentSelectors[optionSelectorIndex]->Right();
	}
	else if( currInput.LLeft() )
	{
		//cout << "optionindex: " << optionSelectorIndex << endl;
		//cout << "num curr selcts: " << numCurrentSelectors << endl;
		//cout << "Attempted left" << endl;
		currentSelectors[optionSelectorIndex]->Left();
	}
			
	if( !currInput.LUp() && !currInput.LDown() )
	{
		momentum = 0;
		framesWaiting = maxWaitFrames;
		//currWaitFrames = maxWaitFrames;
	}

	if( !currInput.LLeft() && !currInput.LRight() )
	{
		currentSelectors[optionSelectorIndex]->Stop();
	}
	currentSelectors[optionSelectorIndex]->Update();
	++framesWaiting;


	if( selectingProfile )
	{

	}
}

PauseMenu::UpdateResponse PauseMenu::UpdateAudioOptions(
	ControllerState &currInput, ControllerState &prevInput)
{
	//cout <<  framesWaiting << ", " << momentum << ", curr: " << currWaitFrames << endl;
	if( currInput.A && !prevInput.A )
	{
		ApplySoundSettings();
					
		return R_NONE;
	}
	else if( currInput.B && !prevInput.B )
	{
		
	}
	if( currInput.LDown() && ( framesWaiting >= currWaitFrames || momentum <= 0 ))
	{
		//cout << "DOWN" << endl;
		currentSelectors[optionSelectorIndex]->selected = false;

		optionSelectorIndex++;
		if( optionSelectorIndex == numCurrentSelectors )
		{
			optionSelectorIndex = 0;
		}

		currentSelectors[optionSelectorIndex]->selected = true;

		if( momentum <= 0 )
		{
			momentum = 1;
		}
		else if( momentum < maxMomentum )
		{
			momentum++;
		}

		//CubicBezier bez( 0, 0, 1, 1 );
		currentSelectors[optionSelectorIndex]->Stop();
		double v = accelBez.GetValue( momentum / (double)maxMomentum );
		framesWaiting = 0;
		currWaitFrames = floor( (maxWaitFrames * ( 1 - v ) + minWaitFrames * v) + .5 );
	}
	else if( currInput.LUp() && ( framesWaiting >= currWaitFrames || momentum >= 0 ))
	{
		//cout << "up! " << framesWaiting << ", " << momentum << endl;
		currentSelectors[optionSelectorIndex]->selected = false;

		optionSelectorIndex--;
		if( optionSelectorIndex == -1 )
		{
			optionSelectorIndex = numCurrentSelectors - 1;
		}

		currentSelectors[optionSelectorIndex]->selected = true;

		if( momentum >= 0 )
		{
			momentum = -1;
		}
		else if( momentum > -maxMomentum )
		{
			momentum--;
		}

		//CubicBezier bez( 0, 0, 1, 1 );
		currentSelectors[optionSelectorIndex]->Stop();
		double v = accelBez.GetValue( (-momentum) / (double)maxMomentum );
		framesWaiting = 0;
		currWaitFrames = floor( (maxWaitFrames * ( 1 - v ) + minWaitFrames * v) + .5 );

				
	}
	else if( currInput.LRight() )
	{
		//cout << "optionindex: " << optionSelectorIndex << endl;
		//cout << "num curr selcts: " << numCurrentSelectors << endl;
		//cout << "Attempted right" << endl;
		currentSelectors[optionSelectorIndex]->Right();

		
	}
	else if( currInput.LLeft() )
	{
		//cout << "optionindex: " << optionSelectorIndex << endl;
		//cout << "num curr selcts: " << numCurrentSelectors << endl;
		//cout << "Attempted left" << endl;
		currentSelectors[optionSelectorIndex]->Left();

		
	}
			
	if( !currInput.LUp() && !currInput.LDown() )
	{
		momentum = 0;
		framesWaiting = maxWaitFrames;
		//currWaitFrames = maxWaitFrames;
	}

	if( !currInput.LLeft() && !currInput.LRight() )
	{
		currentSelectors[optionSelectorIndex]->Stop();
	}
	currentSelectors[optionSelectorIndex]->Update();
	++framesWaiting;


	if( selectingProfile )
	{

	}
}

PauseMenu::UpdateResponse PauseMenu::UpdateInputOptions(
	ControllerState &currInput, ControllerState &prevInput)
{
	string inputType = inputSelectors[0]->GetString();
	bool keyboardInput = inputType == "Keyboard" && currentSelectors == inputSelectors;
	bool xboxInput = inputType == "Xbox" && currentSelectors == inputSelectors;
	//cout <<  framesWaiting << ", " << momentum << ", curr: " << currWaitFrames << endl;
	if( currInput.A && !prevInput.A )
	{	
		XBoxButton b = XBoxButton::XBOX_BLANK;
		while( b == XBoxButton::XBOX_BLANK )
		{
			b = CheckXBoxInput( currInput );
		}
				
		return R_NONE;
	}
	else if( currInput.B && !prevInput.B )
	{
		if( selectingProfile )
		{
			//go back to selecting option type
		}
		else
		{
			selectingProfile = true;
		}
	}
	if( currInput.LDown() && ( framesWaiting >= currWaitFrames || momentum <= 0 ))
	{
		//cout << "DOWN" << endl;
		currentSelectors[optionSelectorIndex]->selected = false;

		optionSelectorIndex++;
		if( optionSelectorIndex == numCurrentSelectors )
		{
			optionSelectorIndex = 0;
		}
		else if( keyboardInput && optionSelectorIndex == numCurrentSelectors )
		{
			optionSelectorIndex = 0;
		}

		currentSelectors[optionSelectorIndex]->selected = true;

		if( momentum <= 0 )
		{
			momentum = 1;
		}
		else if( momentum < maxMomentum )
		{
			momentum++;
		}

		//CubicBezier bez( 0, 0, 1, 1 );
		currentSelectors[optionSelectorIndex]->Stop();
		double v = accelBez.GetValue( momentum / (double)maxMomentum );
		framesWaiting = 0;
		currWaitFrames = floor( (maxWaitFrames * ( 1 - v ) + minWaitFrames * v) + .5 );
	}
	else if( currInput.LUp() && ( framesWaiting >= currWaitFrames || momentum >= 0 ))
	{
		//cout << "up! " << framesWaiting << ", " << momentum << endl;
		currentSelectors[optionSelectorIndex]->selected = false;

		optionSelectorIndex--;
		if( optionSelectorIndex == -1 )
		{
			optionSelectorIndex = numCurrentSelectors - 1;
		}

		currentSelectors[optionSelectorIndex]->selected = true;

		if( momentum >= 0 )
		{
			momentum = -1;
		}
		else if( momentum > -maxMomentum )
		{
			momentum--;
		}

		//CubicBezier bez( 0, 0, 1, 1 );
		currentSelectors[optionSelectorIndex]->Stop();
		double v = accelBez.GetValue( (-momentum) / (double)maxMomentum );
		framesWaiting = 0;
		currWaitFrames = floor( (maxWaitFrames * ( 1 - v ) + minWaitFrames * v) + .5 );

				
	}
	else if( currInput.LRight() )
	{
		//cout << "optionindex: " << optionSelectorIndex << endl;
		//cout << "num curr selcts: " << numCurrentSelectors << endl;
		//cout << "Attempted right" << endl;
		currentSelectors[optionSelectorIndex]->Right();

		if( currentSelectors == inputSelectors && optionSelectorIndex == 0 )
		{
			SetAssocSymbols();
		}
	}
	else if( currInput.LLeft() )
	{
		//cout << "optionindex: " << optionSelectorIndex << endl;
		//cout << "num curr selcts: " << numCurrentSelectors << endl;
		//cout << "Attempted left" << endl;
		currentSelectors[optionSelectorIndex]->Left();

		if( currentSelectors == inputSelectors && optionSelectorIndex == 0 )
		{
			SetAssocSymbols();
		}
	}
			
	if( !currInput.LUp() && !currInput.LDown() )
	{
		momentum = 0;
		framesWaiting = maxWaitFrames;
		//currWaitFrames = maxWaitFrames;
	}

	if( !currInput.LLeft() && !currInput.LRight() )
	{
		currentSelectors[optionSelectorIndex]->Stop();
	}
	currentSelectors[optionSelectorIndex]->Update();
	++framesWaiting;


	if( selectingProfile )
	{

	}
}
//using namespace std;
//using namespace sf;