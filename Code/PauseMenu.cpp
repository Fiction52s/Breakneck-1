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
#include "MapHeader.h"
#include "Config.h"
#include "KinMenu.h"
#include "LogMenu.h"
#include "PaletteShader.h"
//#include "Actor.h"

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

OptionSelector::~OptionSelector()
{
	delete[] options;
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
	game = pauseMenu->game;
	mainMenu = pauseMenu->mainMenu;
	//temporary init here
	basePos = Vector2f( 100, 100 );

	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	optionModeSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 4, 0);

	ts_optionMode = game->GetSizedTileset("Menu/Pause/optionsptions_768x128.png");

	//Vector2f startOffset(1820 / 2, 100);
	Vector2f startOffset(1820 / 2, 128/2 + 150);
	int spacing = 30;
	for (int i = 0; i < 4; ++i)
	{
		
		SetRectCenter(optionModeQuads + i * 4, 768, 128, startOffset + Vector2f(0, (128 + spacing) * i));
	}

	csm = mainMenu->controlSettingsMenu;
}

OptionsMenu::~OptionsMenu()
{
	delete optionModeSelector;
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
	ControllerType cType = mainMenu->GetController(0).GetCType();
	switch (state)
	{
	case CHOOSESTATE:
	{
		int res = optionModeSelector->UpdateIndex(currInput.LUp(), currInput.LDown());
		if (res != 0)
		{
			game->pauseSoundNodeList->ActivateSound(game->soundManager->GetSound("pause_change"));
			//owner->pauseSoundNodeList->ActivateSound(owner->soundManager->GetSound("pause_change"));
		}
		UpdateOptionModeQuads();

		if (currInput.A && !prevInput.A)
		{
			state = (State)(optionModeSelector->currIndex + 1);
			switch (state)
			{
			case CONTROL:
				csm->UpdateXboxButtonIcons();
				break;
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
	{
		if (csm->currButtonState != ControlSettingsMenu::S_SELECTED)
		{
			if (currInput.B && !prevInput.B)
			{
				state = CHOOSESTATE;
				break;
			}
		}

		csm->currCType = cType;
		ControlSettingsMenu::UpdateState uState = csm->Update(currInput, prevInput);
		if (uState == ControlSettingsMenu::CONFIRM)
		{
			//GameSession *owner = mainMenu->pauseMenu->owner;
			//owner->GetController(0).SetFilter(csm->pSel->currProfile->filter);
		}
		break;
	}
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

bool OptionsMenu::CanChangeTab()
{
	if (state == CONTROL)
	{
		if (csm->IsEditingButtons())
		{
			return false;
		}
	}

	return true;
}

PauseMenu::PauseMenu( GameSession *p_game)
	:currentTab( Tab::MAP ),  accelBez( 0, 0, 1, 1 )
{
	game = p_game;
	mainMenu = game->mainMenu;
	

	debugText.setFont(game->mainMenu->arial);
	debugText.setCharacterSize(28);
	debugText.setFillColor(Color::White);
//	debugText.setOutlineColor(Color::Black);
	debugText.setString(
		"Hold B and press Dpad Down\n"
		"while paused to store a debug\n"
		"replay. The game is currently a\n"
		"work in progress.\nThanks for testing\n\nPress Y to restart w/ ghost");
	debugText.setPosition(100, 100);

	optionType = OptionType::O_INPUT;
	cOptions = NULL;//new OptionsMenu( this );
	/*ts_background[0] = game->GetSizedTileset("Menu/Pause/pause_1_pause_910x490.png");
	ts_background[1] = game->GetSizedTileset("Menu/Pause/pause_2_map_910x490.png");
	ts_background[2] = game->GetSizedTileset("Menu/Pause/pause_3_shards_910x490.png");
	ts_background[3] = game->GetSizedTileset("Menu/Pause/pause_4_logs_910x490.png");
	ts_background[4] = game->GetSizedTileset("Menu/Pause/pause_5_options_910x490.png");
	ts_background[5] = game->GetSizedTileset("Menu/Pause/pause_6_kin_910x490.png");*/

	ts_background = game->GetSizedTileset("Menu/Pause/pause_BG_1720x880.png");
	bgSprite.setTexture(*ts_background->texture);

	ts_tabs = game->GetSizedTileset("Menu/Pause/pause_tabs_1326x50.png");
	tabSprite.setTexture(*ts_tabs->texture);
	

	bgPaletteShader = new PaletteShader("pause", 
		"Resources/Menu/Pause/pause_palette_16x6.png");

	ts_select = game->GetSizedTileset("Menu/menu_select_800x140.png");
	
	ts_pauseOptions = game->GetSizedTileset("Menu/Pause/pauseoptions_768x128.png");

	controlSettingsMenu = game->mainMenu->controlSettingsMenu;

	Vector2f startOffset(1820 / 2, 128/2 + 100);
	int spacing = 20;
	for (int i = 0; i < 5; ++i)
	{
		SetRectCenter(pauseOptionQuads + i * 4, 768, 128, startOffset + Vector2f(0, (spacing + 128) * i));
	}

	int waitFrames[3] = { 60, 30, 30 };
	int waitModeThresh[2] = { 2, 2 };
	pauseSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 5, 0, false);
	
	optionsMenu = new OptionsMenu(this);

	selectSprite.setTexture( *ts_select->texture );

	bgSprite.setPosition( 50, 50 );
	tabSprite.setPosition(0, 0);

	//pauseSelectIndex = 0;

	numVideoOptions = 3;
	videoSelectors = new OptionSelector*[numVideoOptions];

	shardMenu = game->shardMenu;
	logMenu = game->logMenu;
	kinMenu = new KinMenu(game);
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

	SetTab(PAUSE);
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
	delete pauseSelector;
	
	delete optionsMenu;
	for (int i = 0; i < numVideoOptions; ++i)
	{
		delete videoSelectors[i];
	}
	delete[] videoSelectors;

	delete kinMenu;

	for (int i = 0; i < numSoundOptions; ++i)
	{
		delete soundSelectors[i];
	}
	delete[] soundSelectors;

	delete bgPaletteShader;
}


ControlProfile * PauseMenu::GetCurrSelectedProfile()
{
	return game->mainMenu->GetCurrSelectedProfile();;
}

bool PauseMenu::SetCurrProfileByName(const std::string &name)
{
	return game->mainMenu->SetCurrProfileByName(name);
}

void PauseMenu::TabLeft()
{
	int index = (int)currentTab;
	index--;
	if( index < 0 )
		index = 5;	
	SetTab((Tab)index);
	game->pauseSoundNodeList->ActivateSound(game->soundManager->GetSound("tab_left"));
}

void PauseMenu::TabRight()
{
	int index = (int)currentTab;
	index++;
	if( index > 5 )
		index = 0;
	SetTab((Tab)index );
	game->pauseSoundNodeList->ActivateSound(game->soundManager->GetSound("tab_right"));
}

void PauseMenu::SetTab( Tab t )
{
	switch (currentTab)
	{
	case SHARDS:
	//	shardMenu->StopMusic();
		break;
	case LOGS:
		logMenu->StopMusic();
		break;
	}

	currentTab = t;

	bgPaletteShader->SetPaletteIndex(currentTab);

	tabSprite.setTextureRect(ts_tabs->GetSubRect(currentTab));

	switch( t )
	{
	case MAP:
		mapCenter.x = game->GetPlayerPos( 0 ).x;
		mapCenter.y = game->GetPlayerPos( 0 ).y;
		mapZoomFactor = 16;	
		break;
	case KIN:
	{
		kinMenu->UpdateCommandButton();
		SaveFile *sf = game->GetCurrentProgress();
		int skinInd = 0;
		if (sf != NULL)
		{
			skinInd = sf->defaultSkinIndex;
		}

		kinMenu->playerSkinShader.SetSkin(skinInd);
		kinMenu->UpdatePowers(game->GetPlayer(0));
		
		break;
	}
		
		
	case SHARDS:
		//shardMenu->SetCurrSequence();
		shardMenu->SetShardTab();
		
		
		break;
	case LOGS:
		logMenu->SetLogTab();
		//shardMenu->SetCurrSequence();
		
		break;
	case OPTIONS:
		optionsMenu->state = OptionsMenu::CHOOSESTATE;
		game->mainMenu->optionsMenu->Center(Vector2f(1820, 980));
		//LoadControlOptions();
		//UpdateButtonIcons();
		break;
	case PAUSE:
		//pauseSelector->currIndex = 0;
		pauseSelector->Reset();
		break;
	}
}

bool PauseMenu::CanChangeTab()
{
	if (currentTab == OPTIONS)
	{
		if (!optionsMenu->CanChangeTab())
		{
			return false;
		}
	}


	return true;
}

void PauseMenu::Draw( sf::RenderTarget *target )
{
	target->draw( bgSprite, &bgPaletteShader->pShader);
	target->draw(tabSprite);

	if( currentTab == PAUSE )
	{
		target->draw(pauseOptionQuads, 4 * 5, sf::Quads, ts_pauseOptions->texture);
		target->draw(debugText);
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
	else if (currentTab == LOGS)
	{
		logMenu->Draw(target);
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
	}
	else if( resolution == "1600 x 900" )
	{
		width = 1600;
		height = 900;
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
	game->mainMenu->ResizeWindow( width, height, style );


	if( vsync == "on" )
	{
		game->mainMenu->window->setVerticalSyncEnabled( true );
	}
	else if( vsync == "off" )
	{
		game->mainMenu->window->setVerticalSyncEnabled( false );
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


	//owner->soundNodeList->SetGlobalVolume( master );
	//mainMenu->soundNodeList->SetGlobalVolume( master );

	//owner->soundNodeList->SetSoundsEnable( enSounds );
	//mainMenu->soundNodeList->SetSoundsEnable( enSounds );

	//owner->soundNodeList->SetMusicEnable( enMusic );
	//owner->mainMenu->soundNodeList->SetMusicEnable( enMusic );

	//owner->soundNodeList->SetRelativeSoundVolume( sound );
	//mainMenu->soundNodeList->SetRelativeSoundVolume( sound );

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
		if (controlSettingsMenu->IsEditingButtons() )
		{

		}
		else
		{
			ResetCounters();
			

			if (currentTab == OPTIONS)
			{
				int sVol = game->mainMenu->config->GetData().soundVolume;
				game->soundNodeList->SetSoundVolume(sVol);
				game->pauseSoundNodeList->SetSoundVolume(sVol);
				controlSettingsMenu->SetButtonAssoc();
				optionsMenu->optionModeSelector->currIndex = 0;
			}

			game->pauseSoundNodeList->ActivateSound(game->soundManager->GetSound("pause_off"));
			game->SetGameSessionState(GameSession::RUN);
			game->soundNodeList->Pause(false);
		}
	}

	if (CanChangeTab())
	{
		if ((currInput.leftShoulder && !prevInput.leftShoulder)
			|| (currInput.LeftTriggerPressed() && !prevInput.LeftTriggerPressed()))
		{
			ResetCounters();
			if (currentTab == OPTIONS)
			{
				controlSettingsMenu->SetButtonAssoc();
				optionsMenu->state = OptionsMenu::CHOOSESTATE;
				optionsMenu->optionModeSelector->currIndex = 0;

				int sVol = game->mainMenu->config->GetData().soundVolume;
				game->soundNodeList->SetSoundVolume(sVol);
				game->pauseSoundNodeList->SetSoundVolume(sVol);
			}
			
			/*if (currentTab == OPTIONS)
			{
				optionsMenu->state = OptionsMenu::CHOOSESTATE;
				game->mainMenu->optionsMenu->Center(Vector2f(1820, 980));
			}
			else if (currentTab == SHARDS)
			{
				shardMenu->SetShardTab();
			}
			else if (currentTab == LOGS)
			{
				logMenu->SetLogTab();
			}
			else if (currentTab == KIN)
			{
				kinMenu->UpdateCommandButton();
			}*/

			TabLeft();

			return R_NONE;
		}
		else if ( (currInput.rightShoulder && !prevInput.rightShoulder)
			|| (currInput.RightTriggerPressed() && !prevInput.RightTriggerPressed()))
		{
			ResetCounters();
			if (currentTab == OPTIONS)
			{
				controlSettingsMenu->SetButtonAssoc();
				optionsMenu->state = OptionsMenu::CHOOSESTATE;
				optionsMenu->optionModeSelector->currIndex = 0;

				int sVol = game->mainMenu->config->GetData().soundVolume;
				game->soundNodeList->SetSoundVolume(sVol);
				game->pauseSoundNodeList->SetSoundVolume(sVol);
			}
			

			

			TabRight();

			return R_NONE;
		}
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

			if( mapCenter.x < game->mapHeader->leftBounds )
			{
				mapCenter.x = game->mapHeader->leftBounds;
			}
			else if( mapCenter.x > game->mapHeader->leftBounds + game->mapHeader->boundsWidth )
			{
				mapCenter.x = game->mapHeader->leftBounds + game->mapHeader->boundsWidth;
			}

			if( mapCenter.y < game->mapHeader->topBounds )
			{
				mapCenter.y = game->mapHeader->topBounds;
			}
			else if( mapCenter.y > game->mapHeader->topBounds + game->mapHeader->boundsHeight )
			{
				mapCenter.y = game->mapHeader->topBounds + game->mapHeader->boundsHeight;
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
	case LOGS:
	{
		logMenu->Update(currInput, prevInput);
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

			}

			int res = pauseSelector->UpdateIndex(currInput.LUp() && !prevInput.LUp(), currInput.LDown() && !prevInput.LDown());

			if (res != 0)
			{
				game->pauseSoundNodeList->ActivateSound(game->soundManager->GetSound("pause_change"));
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

