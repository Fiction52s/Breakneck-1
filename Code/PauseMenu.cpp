#include "PauseMenu.h"
#include <sstream>
#include "GameSession.h"
#include "MainMenu.h"
#include <iostream>
#include "SaveFile.h"
#include <fstream>
#include "ShardMenu.h"
#include "MusicSelector.h"
#include "ControlSettingsMenu.h"
#include "ControlProfile.h"
#include "ColorShifter.h"

using namespace sf;
using namespace std;


OptionSelector::OptionSelector( Vector2f &p_pos, int p_optionCount,
		std::string *p_options )
		:optionCount( p_optionCount ), options( NULL ), pos( p_pos ), accelBez( 0, 0, 1, 1 )
{
	selected = false;
	currentText.setFont( MainMenu::arial );
	currentText.setCharacterSize( 80 );
	currentText.setFillColor( Color::White );
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

	//cout << "end selector init" << endl;
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
		currentText.setFillColor( Color::Red );
	else
		currentText.setFillColor( Color::White );
	target->draw( currentText );
}

OptionsMenu::OptionsMenu( PauseMenu *pauseMenu )
{
	state = CHOOSESTATE;
	mainMenu = pauseMenu->mainMenu;
	//temporary init here
	basePos = Vector2f( 100, 100 );

	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	optionModeSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 4, 0);

	ts_optionMode = pauseMenu->mainMenu->tilesetManager.GetTileset("Menu/optionsptions_768x128.png", 768, 128);

	//Vector2f startOffset(1820 / 2, 100);
	Vector2f startOffset(1820 / 2, 128/2 + 150);
	int spacing = 30;
	for (int i = 0; i < 4; ++i)
	{
		
		SetRectCenter(optionModeQuads + i * 4, 768, 128, startOffset + Vector2f(0, (128 + spacing) * i));
	}

	csm = pauseMenu->controlSettingsMenu;
}

void OptionsMenu::UpdateOptionModeQuads()
{
	for (int i = 0; i < 4; ++i)
	{
		if (optionModeSelector->currIndex == i)
		{
			SetRectSubRect(optionModeQuads + i * 4, ts_optionMode->GetSubRect(i + 4));
		}
		else
		{
			SetRectSubRect(optionModeQuads + i * 4, ts_optionMode->GetSubRect(i));
		}
	}
}

void OptionsMenu::Update( ControllerState &currInput,
		ControllerState &prevInput )
{
	switch (state)
	{
	case CHOOSESTATE:
	{
		int res = optionModeSelector->UpdateIndex(currInput.LUp(), currInput.LDown());
		if (res != 0)
		{
			if (mainMenu->pauseMenu->owner != NULL)
			{
				GameSession *owner = mainMenu->pauseMenu->owner;
				mainMenu->pauseMenu->owner->pauseSoundNodeList->ActivateSound(mainMenu->pauseMenu->owner->soundManager->GetSound("pause_change"));
			}
			//owner->pauseSoundNodeList->ActivateSound(owner->soundManager->GetSound("pause_change"));
		}
		UpdateOptionModeQuads();

		if (currInput.A && !prevInput.A)
		{
			state = (State)(optionModeSelector->currIndex + 1);
			switch (state)
			{
			case SOUND:
				mainMenu->optionsMenu->Load();
				break;
			case VISUAL:
				state = SOUND;
				mainMenu->optionsMenu->Load();
				break;
			case GAMEPLAY:
				state = CHOOSESTATE;
				break;
			}
		}
		break;
	}
	case CONTROL:
		if (csm->currButtonState != ControlSettingsMenu::S_SELECTED)
		{
			if (currInput.B && !prevInput.B)
			{
				state = CHOOSESTATE;
				break;
			}
		}
		csm->Update(currInput, prevInput);
		break;
	case SOUND:
		if (currInput.B && !prevInput.B)
		{
			state = CHOOSESTATE;
			break;
		}
		//config->WaitForLoad();
		mainMenu->optionsMenu->Update( currInput, prevInput );
		break;
	}
}

void OptionsMenu::Draw( sf::RenderTarget *target )
{
	switch (state)
	{
	case CHOOSESTATE:
		target->draw(optionModeQuads, 4 * 4, sf::Quads, ts_optionMode->texture);
		break;
	case CONTROL:
		csm->Draw(target);
		break;
	case SOUND:
		mainMenu->optionsMenu->Draw(target);
		break;
	}
}

PauseMenu::PauseMenu(MainMenu *p_mainMenu )
	:mainMenu( p_mainMenu ), currentTab( Tab::MAP ),  accelBez( 0, 0, 1, 1 )
	
{
	owner = NULL;
	optionType = OptionType::O_INPUT;
	cOptions = NULL;//new OptionsMenu( this );
	ts_background[0] = mainMenu->tilesetManager.GetTileset( "Menu/pause_1_pause_1820x980.png", 1820, 980 );
	ts_background[1] = mainMenu->tilesetManager.GetTileset( "Menu/pause_2_map_1820x980.png", 1820, 980 );
	ts_background[2] = mainMenu->tilesetManager.GetTileset( "Menu/pause_3_shards_1820x980.png", 1820, 980 );
	ts_background[3] = mainMenu->tilesetManager.GetTileset( "Menu/pause_4_options_1820x980.png", 1820, 980 );
	ts_background[4] = mainMenu->tilesetManager.GetTileset( "Menu/pause_5_kin_1820x980.png", 1820, 980 );

	ts_select = mainMenu->tilesetManager.GetTileset( "Menu/menu_select_800x140.png", 800, 140 );
	
	ts_pauseOptions = mainMenu->tilesetManager.GetTileset("Menu/pauseoptions_768x128.png", 768, 128);

	Vector2f startOffset(1820 / 2, 128/2 + 100);
	int spacing = 20;
	for (int i = 0; i < 5; ++i)
	{
		SetRectCenter(pauseOptionQuads + i * 4, 768, 128, startOffset + Vector2f(0, (spacing + 128) * i));
	}

	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	pauseSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 5, 0);
	
	controlSettingsMenu = new ControlSettingsMenu(mainMenu);
	
	optionsMenu = new OptionsMenu(this);

	selectSprite.setTexture( *ts_select->texture );

	bgSprite.setPosition( 0, 0 );

	//pauseSelectIndex = 0;

	numVideoOptions = 3;
	videoSelectors = new OptionSelector*[numVideoOptions];

	shardMenu = new ShardMenu( mainMenu );
	kinMenu = new KinMenu(mainMenu, controlSettingsMenu);
	//resolution
	//fullscreen
	//vsync

	//video
	string resolutions[] = { "1920 x 1080", "1600 x 900" };
	videoSelectors[0] = new OptionSelector( Vector2f( 100, 20 ), 2, resolutions );

	string windowType[] = { "Fullscreen", "Window", "Borderless Window" };
	videoSelectors[1] = new OptionSelector( Vector2f( 100, 100 ), 3, windowType );
	//
	string vsync[] = { "on", "off" };
	videoSelectors[2] = new OptionSelector( Vector2f( 100, 180 ), 2, vsync );

	////cout << "YYYYY this initialization" << endl;
	string masterVolume[101];
	string musicVolume[101];
	string soundVolume[101];

	////cout << "WATTTT" << endl;

	//
	for( int i = 0; i < 101; ++i )
	{
		//cout << "starting loop: " << i << endl;

		stringstream ss;	
		ss << i;

		
		masterVolume[i] = ss.str();
		musicVolume[i] = ss.str();
		soundVolume[i] = ss.str();
		//cout << "end loop: " << i << endl;
	}
	//cout << "WATTTT" << endl;
	numSoundOptions = 5;
	soundSelectors = new OptionSelector*[numSoundOptions];

	string enable[] = {"on", "off"};

	//cout << "awfhaweioph ihewhtf a" << endl;
	soundSelectors[0] = new OptionSelector( Vector2f( 100, 20 ), 101, masterVolume );
	soundSelectors[1] = new OptionSelector( Vector2f( 100, 180 ), 2, enable );
	soundSelectors[2] = new OptionSelector( Vector2f( 100, 260 ), 101, musicVolume );
	soundSelectors[3] = new OptionSelector( Vector2f( 100, 340 ), 2, enable );
	soundSelectors[4] = new OptionSelector( Vector2f( 100, 420 ), 101, soundVolume );

	//cout << "ZZZZ this initialization" << endl;

	currentSelectors = soundSelectors;
	numCurrentSelectors = numSoundOptions;
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
	//SetAssocSymbols();
	//cout << "end this initialization" << endl;
}

void PauseMenu::UpdatePauseOptions()
{
	for (int i = 0; i < 5; ++i)
	{
		if (i == pauseSelector->currIndex)
		{
			SetRectSubRect(pauseOptionQuads + i * 4, ts_pauseOptions->GetSubRect(i + 5));
		}
		else
		{
			SetRectSubRect(pauseOptionQuads + i * 4, ts_pauseOptions->GetSubRect(i));
		}
	}
}

PauseMenu::~PauseMenu()
{
	/*for( int i = 0; i < numVideoOptions; ++i )
	{
		delete videoSelectors[i];
	}
	delete [] videoSelectors;*/
}





void PauseMenu::TabLeft()
{
	int index = (int)currentTab;
	index--;
	if( index < 0 )
		index = 4;	
	SetTab((Tab)index);
	owner->pauseSoundNodeList->ActivateSound(owner->soundManager->GetSound("tab_left"));
}

void PauseMenu::TabRight()
{
	int index = (int)currentTab;
	index++;
	if( index > 4 )
		index = 0;
	SetTab((Tab)index );
	owner->pauseSoundNodeList->ActivateSound(owner->soundManager->GetSound("tab_right"));
}

void PauseMenu::SetTab( Tab t )
{
	switch (currentTab)
	{
	case SHARDS:
		shardMenu->StopMusic();
		break;
	}

	currentTab = t;
	bgSprite.setTexture( *ts_background[currentTab]->texture );

	switch( t )
	{
	case MAP:
		mapCenter.x = owner->GetPlayer( 0 )->position.x;
		mapCenter.y = owner->GetPlayer( 0 )->position.y;
		mapZoomFactor = 16;	
		break;
	case KIN:
		break;
	case SHARDS:
		//shardMenu->SetCurrSequence();
		shardMenu->state = ShardMenu::WAIT;
		shardMenu->SetCurrShard();
		break;
	case OPTIONS:
		//LoadControlOptions();
		//UpdateButtonIcons();
		break;
	case PAUSE:
		pauseSelector->currIndex = 0;
		break;
	}
}

void PauseMenu::Draw( sf::RenderTarget *target )
{
	target->draw( bgSprite );

	if( currentTab == PAUSE )
	{
		target->draw(pauseOptionQuads, 4 * 5, sf::Quads, ts_pauseOptions->texture);
		//target->draw( selectSprite );
	}
	else if( currentTab == OPTIONS )
	{
		optionsMenu->Draw(target);
		/*string inputType = inputSelectors[0]->GetString();
		int num = numCurrentSelectors;
		if( inputType == "Keyboard" )
		{
			num = num - 1;
		}
		for( int i = 0; i < num; ++i )
		{
			currentSelectors[i]->Draw( target );
		}*/
		

		//switch( optionType )
		//{
		//case O_VIDEO:
		//	{
		//		//return UpdateVideoOptions( currInput, prevInput );
		//		break;
		//	}
		//case O_AUDIO:
		//	{
		//		//return UpdateAudioOptions( currInput, prevInput );
		//		break;
		//	}
		//case O_INPUT:
		//	{
		//		
		//		cOptions->Draw( target );
		//		break;
		//	}
		//}
		//if( currentSelectors == inputSelectors )
		//{
		//	target->draw( assocSymbols, ts_actionIcons->texture );

		//	
		//	int cap;
		//	if( inputType == "Xbox" )
		//	{
		//		cap = 12;
		//	}
		//	else if( inputType == "Keyboard" )
		//	{
		//		cap = 15;
		//	}
		//	else
		//	{
		//		//test
		//		cap = 8;
		//	}
		//	for( int i = 0; i < cap; ++i )
		//	{
		//		target->draw( actionText[i] );
		//	}

		//	target->draw( buttonVA, ts_currentButtons->texture );
		//}

		
	}
	else if (currentTab == SHARDS)
	{
		shardMenu->Draw(target);
	}
	else if (currentTab == KIN)
	{
		kinMenu->Draw(target);
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
	mainMenu->ResizeWindow( width, height, style );


	if( vsync == "on" )
	{
		mainMenu->window->setVerticalSyncEnabled( true );
	}
	else if( vsync == "off" )
	{
		mainMenu->window->setVerticalSyncEnabled( false );
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
	mainMenu->soundNodeList->SetGlobalVolume( master );

	owner->soundNodeList->SetSoundsEnable( enSounds );
	mainMenu->soundNodeList->SetSoundsEnable( enSounds );

	//owner->soundNodeList->SetMusicEnable( enMusic );
	//owner->mainMenu->soundNodeList->SetMusicEnable( enMusic );

	owner->soundNodeList->SetRelativeSoundVolume( sound );
	mainMenu->soundNodeList->SetRelativeSoundVolume( sound );

	//owner->soundNodeList->SetRelativeMusicVolume( music );
	//owner->mainMenu->soundNodeList->SetRelativeMusicVolume( music );
}

//run this in a loop
void PauseMenu::ResetCounters()
{
	pauseSelector->ResetCounters();
}

PauseMenu::UpdateResponse PauseMenu::Update( ControllerState &currInput,
		ControllerState &prevInput )
{
	if( (currInput.start && !prevInput.start))// || (currInput.back && !prevInput.back ) )
	{
		/*if (currentTab == OPTIONS && controlSettingsMenu->currButtonState == ControlSettingsMenu::S_SELECTED)
		{

		}*/
		//else
		{
			ResetCounters();
			

			if (currentTab == OPTIONS)
			{
				controlSettingsMenu->SetButtonAssoc();
				optionsMenu->optionModeSelector->currIndex = 0;
			}

			owner->pauseSoundNodeList->ActivateSound(owner->soundManager->GetSound("pause_off"));
			owner->state = GameSession::State::RUN;
			owner->soundNodeList->Pause(false);
		}
	}

	if( currInput.leftShoulder && !prevInput.leftShoulder )
	{
		ResetCounters();
		if (currentTab == OPTIONS)
		{
			controlSettingsMenu->SetButtonAssoc();
			optionsMenu->state = OptionsMenu::CHOOSESTATE;
			optionsMenu->optionModeSelector->currIndex = 0;
		}
		TabLeft();

		if (currentTab == OPTIONS)
		{
			optionsMenu->state = OptionsMenu::CHOOSESTATE;
			mainMenu->optionsMenu->Center(Vector2f(1820, 980));
		}
		else if (currentTab == SHARDS)
		{
			shardMenu->SetShardTab();
		}
		return R_NONE;
	}
	else if( currInput.rightShoulder && !prevInput.rightShoulder )
	{
		ResetCounters();
		if (currentTab == OPTIONS)
		{
			controlSettingsMenu->SetButtonAssoc();
			optionsMenu->state = OptionsMenu::CHOOSESTATE;
			optionsMenu->optionModeSelector->currIndex = 0;
		}
		TabRight();

		if (currentTab == OPTIONS)
		{
			optionsMenu->state = OptionsMenu::CHOOSESTATE;
			mainMenu->optionsMenu->Center(Vector2f(1820, 980));
		}
		else if (currentTab == SHARDS)
		{
			shardMenu->SetShardTab();
		}
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

			if( mapCenter.x < owner->mh->leftBounds )
			{
				mapCenter.x = owner->mh->leftBounds;
			}
			else if( mapCenter.x > owner->mh->leftBounds + owner->mh->boundsWidth )
			{
				mapCenter.x = owner->mh->leftBounds + owner->mh->boundsWidth;
			}

			if( mapCenter.y < owner->mh->topBounds )
			{
				mapCenter.y = owner->mh->topBounds;
			}
			else if( mapCenter.y > owner->mh->topBounds + owner->mh->boundsHeight )
			{
				mapCenter.y = owner->mh->topBounds + owner->mh->boundsHeight;
			}
			break;
		}
	case KIN:
		{
			kinMenu->Update(currInput,prevInput);
			break;
		}
	case SHARDS:
		{
			shardMenu->Update( currInput, prevInput );
			break;
		}
	case OPTIONS:
		{	
			
			//return UpdateOptions( currInput, prevInput );
			
			optionsMenu->Update( currInput, prevInput );
			
			
			break;
		}
	case PAUSE:
		{
			if( currInput.A && !prevInput.A )
			{
				UpdateResponse ur = (UpdateResponse)(pauseSelector->currIndex+1);
				//owner->pauseSoundNodeList->ActivateSound(owner->soundManager->GetSound("pause_select"));
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

			int res = pauseSelector->UpdateIndex(currInput.LUp(), currInput.LDown());

			if (res != 0)
			{
				owner->pauseSoundNodeList->ActivateSound(owner->soundManager->GetSound("pause_change"));
			}
			int h = 200;
			//selectSprite.setPosition( 100, 100 + h * pauseSelector->currIndex);
			UpdatePauseOptions();
			break;
		}
	}

	return R_NONE;
}


PauseMenu::UpdateResponse PauseMenu::UpdateOptions( ControllerState &currInput,
	ControllerState &prevInput )
{
	switch( optionType )
	{
	case O_VIDEO:
		return UpdateVideoOptions( currInput, prevInput );
		break;
	case O_AUDIO:
		return UpdateAudioOptions( currInput, prevInput );
		break;
	case O_INPUT:
		
		cOptions->Update( currInput, prevInput );
		//return UpdateResponse::R_NONE;
		break;
	}
	/*if( currentSelectors == inputSelectors )
	{
		return UpdateInputOptions( currInput, prevInput );
	}
	else if( currentSelectors == videoSelectors )
	{
		
	}
	else if( currentSelectors == soundSelectors )
	{
		
	}
	else
	{
		assert( 0 );
		return R_NONE;
	}*/
	

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
		//if( currentSelectors == inputSelectors )
		//{
		//	if( selectingProfile )
		//	{
		//		//go back to selecting option type
		//	}
		//	else
		//	{
		//		selectingProfile = true;
		//	}
		//}
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


	/*if( selectingProfile )
	{

	}*/
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


	//if( selectingProfile )
	//{

	//}
}

PauseMenu::UpdateResponse PauseMenu::UpdateInputOptions(
	ControllerState &currInput, ControllerState &prevInput)
{
	return UpdateResponse::R_NONE;
}
//using namespace std;
//using namespace sf;


KinMenu::KinMenu(MainMenu *p_mainMenu, ControlSettingsMenu *p_csm)
	:mainMenu(p_mainMenu), csm( p_csm )
{
	Vector2f powersOffset(512, 495);
	Vector2f powerPos(0, 0);
	Vector2f powerSpacing(30, 20);
	ts_powers = mainMenu->tilesetManager.GetTileset("Menu/power_icon_128x128.png", 128, 128);

	description.setFont(mainMenu->arial);
	description.setCharacterSize(24);
	description.setFillColor(Color::White);

	int tutWidth = 1220;
	int tutHeight = 320;
	ts_tutorial[0] = mainMenu->tilesetManager.GetTileset("Menu/tut_jump.png", tutWidth, tutHeight);
	ts_tutorial[1] = mainMenu->tilesetManager.GetTileset("Menu/tut_attack.png", tutWidth, tutHeight);
	ts_tutorial[2] = mainMenu->tilesetManager.GetTileset("Menu/tut_sprint.png", tutWidth, tutHeight);
	ts_tutorial[3] = mainMenu->tilesetManager.GetTileset("Menu/tut_dash.png", tutWidth, tutHeight);
	ts_tutorial[4] = mainMenu->tilesetManager.GetTileset("Menu/tut_walljump.png", tutWidth, tutHeight);
	ts_tutorial[5] = mainMenu->tilesetManager.GetTileset("Menu/tut_speed.png", tutWidth, tutHeight);
	ts_tutorial[6] = mainMenu->tilesetManager.GetTileset("Menu/tut_health.png", tutWidth, tutHeight);
	ts_tutorial[7] = mainMenu->tilesetManager.GetTileset("Menu/tut_survival.png", tutWidth, tutHeight);
	ts_tutorial[8] = mainMenu->tilesetManager.GetTileset("Menu/tut_key.png", tutWidth, tutHeight);
	ts_tutorial[9] = mainMenu->tilesetManager.GetTileset("Menu/tut_airdash.png", tutWidth, tutHeight);
	tutorialSpr.setPosition(512, 74);

	
	ts_xboxButtons = mainMenu->tilesetManager.GetTileset("Menu/xbox_button_icons_128x128.png", 128, 128);

	SetRectColor(descriptionBox, Color(0, 0, 0, 255));
	SetRectCenter(descriptionBox, 1220, 90, Vector2f(1122, 439));//topleft is 512,394

	description.setPosition(512 + 10, 394 + 10);

	commandSpr.setPosition(512, 394);
	commandSpr.setTexture(*ts_xboxButtons->texture);
	commandSpr.setScale(.4, .4);

	for (int i = 0; i < 10; ++i)
	{
		SetRectSubRect(powerQuads + i * 4, ts_powers->GetSubRect(i));
		SetRectSubRect(powerQuadsBG + i * 4, ts_powers->GetSubRect(i));
		//powerPos.x = powersOffset.x + (128 + powerSpacing.x) * (i%9) + 64;
		powerPos.x = powersOffset.x + (128 + powerSpacing.x) * (i%8) + 64;
		powerPos.y = powersOffset.y + (128 + powerSpacing.y) * (i / 8) + 64;
		SetRectCenter(powerQuads + i * 4, 128, 128, powerPos);
		SetRectCenter(powerQuadsBG + i * 4, 128, 128, powerPos);
		
	}

	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	xSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 8, 0);
	ySelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 2, 0);

	Tileset *ts_selector  = mainMenu->tilesetManager.GetTileset("Menu/power_icon_border_160x160.png", 160, 160);
	selectorSpr.setTexture(*ts_selector->texture);
	selectorSpr.setOrigin(selectorSpr.getLocalBounds().width / 2, selectorSpr.getLocalBounds().height / 2);

	ts_kin = mainMenu->tilesetManager.GetTileset("Menu/pause_kin_400x836.png", 400, 836);
	ts_aura1A = mainMenu->tilesetManager.GetTileset("Menu/pause_kin_aura_1a_400x836.png", 400, 836);
	ts_aura1B = mainMenu->tilesetManager.GetTileset("Menu/pause_kin_aura_1b_400x836.png", 400, 836);
	ts_aura2A = mainMenu->tilesetManager.GetTileset("Menu/pause_kin_aura_2a_400x836.png", 400, 836);
	ts_aura2B = mainMenu->tilesetManager.GetTileset("Menu/pause_kin_aura_2b_400x836.png", 400, 836);
	ts_veins = mainMenu->tilesetManager.GetTileset("Menu/pause_kin_veins_400x836.png", 400, 836);

	if (!scrollShader1.loadFromFile("Resources/Shader/menuauraslide.frag", sf::Shader::Fragment))
	{
		cout << "menu aura sliding SHADER NOT LOADING CORRECTLY" << endl;
		assert(0);
	}
	scrollShader1.setUniform("u_texture", sf::Shader::CurrentTexture);
	scrollShader1.setUniform("blendColor", ColorGL( Color::White ));

	if (!scrollShader2.loadFromFile("Resources/Shader/menuauraslide.frag", sf::Shader::Fragment))
	{
		cout << "menu aura sliding SHADER NOT LOADING CORRECTLY" << endl;
		assert(0);
	}
	scrollShader2.setUniform("u_texture", sf::Shader::CurrentTexture);
	scrollShader2.setUniform("blendColor", ColorGL(Color::White));
	Vector2f offset(72, 74);

	kinSpr.setTexture(*ts_kin->texture);
	aura1ASpr.setTexture(*ts_aura1A->texture);
	aura1BSpr.setTexture(*ts_aura1B->texture);
	aura2ASpr.setTexture(*ts_aura2A->texture);
	aura2BSpr.setTexture(*ts_aura2B->texture);
	veinSpr.setTexture(*ts_veins->texture);

	/*aura1ASpr.setColor(Color::Red);
	aura1BSpr.setColor(Color::Green);
	aura2ASpr.setColor(Color::Yellow);
	aura2BSpr.setColor(Color::Magenta);*/
	Image palette;
	bool loadPalette = palette.loadFromFile("Resources/Menu/pause_kin_aura_color.png");
	assert(loadPalette);

	Image powerPalette;
	bool loadPowerPalette = powerPalette.loadFromFile("Resources/Menu/power_icons_palette.png");

	aura1AShifter = new ColorShifter(ColorShifter::ShifterType::SEQUENTIAL, 60, 2);
	aura1BShifter = new ColorShifter(ColorShifter::ShifterType::SEQUENTIAL, 60, 2);
	aura2AShifter = new ColorShifter(ColorShifter::ShifterType::SEQUENTIAL, 60, 2);
	aura2BShifter = new ColorShifter(ColorShifter::ShifterType::SEQUENTIAL, 60, 2);
	bgShifter = new ColorShifter(ColorShifter::ShifterType::SEQUENTIAL, 60, 2);
	selectedShifter = new ColorShifter(ColorShifter::ShifterType::SEQUENTIAL, 60, 3);
	//selectedShifter->SetColors(palette, 0);
	//selectedShifter->SetColorIndex(0, Color::Cyan);
	//selectedShifter->SetColorIndex(1, Color::Cyan);
	//selectedShifter->SetColorIndex(0, Color::Black);

	selectedShifter->SetColors(powerPalette, 0);
	
	bgShifter->SetColors(palette, 0);
	aura1AShifter->SetColors(palette, 1);
	aura1BShifter->SetColors(palette, 2);
	aura2AShifter->SetColors(palette, 3);
	aura2BShifter->SetColors(palette, 4);
	/*aura1AShifter->SetColorIndex(0, Color::Red);
	aura1AShifter->SetColorIndex(1, Color::Cyan);
	aura1BShifter->SetColorIndex(0, Color::Yellow);
	aura1BShifter->SetColorIndex(1, Color::White);
	aura2AShifter->SetColorIndex(0, Color::Blue);
	aura2AShifter->SetColorIndex(1, Color::Black );
	aura2BShifter->SetColorIndex(0, Color::Green);
	aura2BShifter->SetColorIndex(1, Color::Red);
	bgShifter->SetColorIndex(0, Color::Magenta);
	bgShifter->SetColorIndex(1, Color::Black);*/

	aura1AShifter->Reset();
	aura1BShifter->Reset();
	aura2AShifter->Reset();
	aura2BShifter->Reset();
	bgShifter->Reset();
	selectedShifter->Reset();

	kinSpr.setPosition(offset);
	aura1ASpr.setPosition(offset);
	aura1BSpr.setPosition(offset);
	aura2ASpr.setPosition(offset);
	aura2BSpr.setPosition(offset);
	veinSpr.setPosition(offset);
	veinSpr.setColor(Color::Transparent);
	
	SetRectCenter(kinBG, aura1ASpr.getGlobalBounds().width, aura1ASpr.getGlobalBounds().height,
		Vector2f(offset.x + 200, offset.y + 418));
	//SetRectColor(kinBG, Color(Color::Cyan));

	frame = 0;

	powerDescriptions[0] = "      = JUMP     Press JUMP to launch yourself into the air, or press JUMP while aerial to double jump.\n"
		"Hold the JUMP button down longer for extra height!";
	powerDescriptions[1] = "      = ATTACK     Press ATTACK to release an energy burst in front of you capable of destroying enemies.\n"
		"Hold UP or DOWN while in the air to do a directional attack.";
	powerDescriptions[2] = "When moving on a slope, hold diagonally UP/DOWN and FORWARD to accelerate.\n" 
		"Hold UP or DOWN to slide with low friction. On steep slopes hold DOWN to slide down even faster.";
	powerDescriptions[3] = "      = DASH     Press DASH to quickly start moving in the direction you are facing while grounded.\n"
		"Tap DASH quickly while ascending a steep slope to climb your way up.";
	powerDescriptions[4] = "Hold towards a wall to wall cling and descend slowly. Tap away from a wall to wall jump."
		"\nHold DOWN and away from the wall to drift away from the wall without wall jumping";
	powerDescriptions[5] = "Move fast and kill enemies to build up your speed meter. There are 3 different meter levels."
		"\nWith each speed level, your attacks get bigger and your dash is more powerful.";
	powerDescriptions[6] = "The TRANSCEND absorbs all energy within its territory, meaning even Kin's energy drains at a constant"
		"\nrate. Kill enemies and steal their energy to bolster your health while you search for the NODE.";
	powerDescriptions[7] = "When Kin runs out of energy, the core of the BREAKNECK suit becomes unstable and enters SURVIVAL"
		"\nMODE. You have 5 seconds to kill an enemy or destroy the NODE before the BREAKNECK self destructs.";
	powerDescriptions[8] = "Certain enemies have a special ABSORPTION HEART which supports the NODE and VEINS. When you"
		"\nclear enough of them from an area, the nearby VEINS will weaken, allowing you to break through them.";
	powerDescriptions[9] = "      = DASH     Press DASH while in the air to HOVER for a short period. Hold a direction to"
		"\nAIRDASH in 1 of 8 directions. You can change your AIRDASH direction at any time. Kin gets 1 AIRDASH per air time.";
	UpdateDescription();
	UpdateSelector();
	UpdatePowerSprite();
	UpdateTutorial();
	UpdateCommandButton();
}

KinMenu::~KinMenu()
{
	delete aura1AShifter;
	delete aura1BShifter;
	delete aura2AShifter;
	delete aura2BShifter;

	delete bgShifter;
}

void KinMenu::UpdateSelector()
{
	selectorSpr.setPosition(powerQuads[GetCurrIndex() * 4].position + Vector2f(64, 64));
	//SetRectCenter(powerQuads + i * 4, 128, 128, powerPos);
}

void KinMenu::UpdateTutorial()
{
	tutorialSpr.setTexture(*ts_tutorial[GetCurrIndex()]->texture);
}

void KinMenu::UpdatePowers( Actor *player )
{
	if (player->hasPowerAirDash)
	{
		secondRowMax = 1;
	}
	else
	{
		secondRowMax = 0;
	}
}

void KinMenu::UpdateCommandButton()
{
	ts_currentButtons = ts_xboxButtons;

	int index = GetCurrIndex();
	IntRect sub;
	if (index == 0)
		sub = ts_xboxButtons->GetSubRect(csm->GetFilteredButton(ControllerSettings::JUMP)-1);
	else if( index == 1)
		sub = ts_xboxButtons->GetSubRect(csm->GetFilteredButton(ControllerSettings::ATTACK)-1);
	else if (index == 3 || index == 9)
	{
		sub = ts_xboxButtons->GetSubRect(csm->GetFilteredButton(ControllerSettings::DASH)-1);
	}

	commandSpr.setTexture(*ts_currentButtons->texture);
	commandSpr.setTextureRect(sub);
}

void KinMenu::Update(ControllerState &curr, ControllerState &prev)
{
	int xchanged;
	int ychanged;


	xchanged = xSelector->UpdateIndex(curr.LLeft(), curr.LRight());
	ychanged = ySelector->UpdateIndex(curr.LUp(), curr.LDown());


	if (xchanged != 0 || ychanged != 0)
	{
		if (mainMenu->pauseMenu->owner != NULL)
		{
			GameSession *owner = mainMenu->pauseMenu->owner;
			mainMenu->pauseMenu->owner->pauseSoundNodeList->ActivateSound(mainMenu->pauseMenu->owner->soundManager->GetSound("pause_change"));
		}
		
		if ( ySelector->currIndex == 1 && xSelector->currIndex > secondRowMax )
		{
			if (ychanged != 0)
			{
				ySelector->currIndex = 0;
			}
			else if (xchanged != 0)
			{
				xSelector->currIndex = 0;
			}

		}

		UpdateDescription();
		UpdateSelector();
		 
		UpdateTutorial();
		UpdateCommandButton();
	}

	UpdatePowerSprite();
	

	int scrollFrames1 = 120;
	int scrollFrames2 = 240;
	float portion1 = ((float)(frame % scrollFrames1)) / scrollFrames1;
	float portion2 = ((float)(frame % scrollFrames2)) / scrollFrames2;
	scrollShader1.setUniform("quant", portion1);
	scrollShader2.setUniform("quant", portion2);

	int breatheFrames = 180;
	int breatheWaitFrames = 120;
	int bTotal = breatheFrames + breatheWaitFrames;
	float halfBreathe = breatheFrames / 2;
	int f = frame % (bTotal);
	float alpha;
	if (f <= halfBreathe)
	{
		alpha = f / halfBreathe;
	}
	else if( f <= breatheFrames )
	{
		f -= halfBreathe;
		alpha = 1.f - f / halfBreathe;
	}
	else
	{
		alpha = 0;
	}
	//cout << "alpha: " << alpha << endl;
	veinSpr.setColor(Color(255, 255, 255, alpha * 100));

	aura1AShifter->Update();
	aura1BShifter->Update();
	aura2AShifter->Update();
	aura2BShifter->Update();
	bgShifter->Update();
	selectedShifter->Update();
	//Color c = aura1AShifter->GetCurrColor();
	//cout << "c: " << c.a << ", " << c.g << ", " << c.b << endl;
	

	++frame;
}

int KinMenu::GetCurrIndex()
{
	return ySelector->currIndex * 8 + xSelector->currIndex % 8;
}

void KinMenu::UpdateDescription()
{
	description.setString(powerDescriptions[GetCurrIndex()]);
	
	/*sf::FloatRect textRect = description.getLocalBounds();
	description.setOrigin(textRect.left + textRect.width / 2.0f,
		textRect.top + textRect.height / 2.0f);
	description.setPosition(Vector2f(1122, 439));*/
	//text.setPosition(sf::Vector2f(SCRWIDTH / 2.0f, SCRHEIGHT / 2.0f));
}

void KinMenu::UpdatePowerSprite()
{
	int maxShowIndex = 8 + secondRowMax;
	int currIndex = GetCurrIndex();
	for (int i = 0; i < 10; ++i)
	{
		if (i > maxShowIndex)
		{
			SetRectSubRect(powerQuads + i * 4, FloatRect( 0, 0, 0, 0 ));
			SetRectColor(powerQuadsBG + i * 4, Color(Color::Transparent));
		}
		else
		{
			SetRectSubRect(powerQuads + i * 4, ts_powers->GetSubRect(i + 0));
			if (currIndex == i)
			{
				//SetRectSubRect(powerQuads + i * 4, ts_powers->GetSubRect(i + 16));
				SetRectColor(powerQuadsBG + i * 4, selectedShifter->GetCurrColor());//Color(0, 0, 0, 255));
			}
			else
			{
				//SetRectSubRect(powerQuads + i * 4, ts_powers->GetSubRect(i + 0));
				SetRectColor(powerQuadsBG + i * 4, Color(0, 0, 0, 255));
			}
		}
	}
}

void KinMenu::Draw(sf::RenderTarget *target)
{
	SetRectColor(kinBG, bgShifter->GetCurrColor());
	target->draw(kinBG, 4, sf::Quads );
	scrollShader1.setUniform("blendColor", ColorGL(aura1AShifter->GetCurrColor()));
	target->draw(aura1ASpr, &scrollShader1);
	scrollShader1.setUniform("blendColor", ColorGL(aura1BShifter->GetCurrColor()));
	target->draw(aura1BSpr, &scrollShader1);
	scrollShader2.setUniform("blendColor", ColorGL(aura2AShifter->GetCurrColor()));
	target->draw(aura2ASpr, &scrollShader2);
	scrollShader2.setUniform("blendColor", ColorGL(aura2BShifter->GetCurrColor()));
	target->draw(aura2BSpr, &scrollShader2);
	target->draw(kinSpr);
	target->draw(veinSpr);

	target->draw(powerQuadsBG, 10 * 4, sf::Quads);
	target->draw(powerQuads, 10 * 4, sf::Quads, ts_powers->texture);
	

	target->draw(tutorialSpr);
	target->draw(descriptionBox, 4, sf::Quads );

	int index = GetCurrIndex();
	if (index == 0 || index == 1 || index == 3 || index == 9)
	{
		target->draw(commandSpr);
	}

	target->draw(description);

	target->draw(selectorSpr);
}
