#include "SaveMenuScreen.h"
#include "MainMenu.h"
#include "SaveFile.h"
#include "VectorMath.h"
#include "IntroMovie.h"
#include "WorldMap.h"
#include "Fader.h"
#include "SkinMenu.h"
#include "MusicPlayer.h"
#include "AdventureManager.h"
#include "UIMouse.h"
#include "CustomCursor.h"
#include "GamePopup.h"
#include "MusicPlayer.h"
#include "MusicSelector.h"

using namespace sf;
using namespace std;

SaveFileDisplay::SaveFileDisplay(sf::Font &f)
	:font( f )
{
	blankMode = true;
	fillColor = Color::White;
	lineColor = Color::Black;

	InitText(capturedShardsText);
	InitText(capturedLogsText);
	InitText(totalTime);
	InitText(completeLevelsText);
	InitText(blankText);
	InitText(completedWorldsText);

	blankText.setString("OPEN SLOT");
}

void SaveFileDisplay::InitText( sf::Text &text)
{
	text.setFont(font);
	text.setFillColor(fillColor);
	text.setOutlineColor(lineColor);
	text.setOutlineThickness(3);
}

bool SaveFileDisplay::Contains(sf::Vector2f &checkPos)
{
	//710 x 270
	FloatRect fr(pos, Vector2f(710, 270));
	return fr.contains(checkPos);
}

void SaveFileDisplay::SetPosition(Vector2f &p_pos)
{
	pos = p_pos;
	Vector2f innerStart = pos + Vector2f(280, 30);


	blankText.setPosition(innerStart + Vector2f( 0, 50 ));
	//totalPercentage.setPosition(innerStart);
	//totalTime.setPosition(innerStart + Vector2f(0, 100));
	completedWorldsText.setPosition(innerStart);
	completeLevelsText.setPosition(innerStart + Vector2f(0, 50));
	capturedShardsText.setPosition(innerStart + Vector2f(0, 100));
	capturedLogsText.setPosition(innerStart + Vector2f(0, 150));
}

void SaveFileDisplay::Draw(sf::RenderTarget *target)
{
	if (blankMode)
	{
		target->draw(blankText);
	}
	else
	{
		target->draw(completedWorldsText);
		target->draw(completeLevelsText);
		target->draw(capturedShardsText);
		target->draw(capturedLogsText);
		//target->draw(totalTime);
		
	}
}

void SaveFileDisplay::SetValues(SaveFile *sf, AdventurePlanet *adventurePlanet)
{
	if (sf != NULL)
	{
		stringstream ss;

		int totalWorlds = adventurePlanet->numWorlds;
		int numCompleteWorlds = sf->GetNumCompleteWorlds(adventurePlanet);

		ss << numCompleteWorlds << " / " << totalWorlds << " Worlds completed";

		completedWorldsText.setString(ss.str());

		ss.str("");

		ss << sf->GetTotalMapsBeaten() << " / " << sf->GetTotalMaps() << " Levels completed";//sf->GetCompletionPercentage() << "% Complete";
		completeLevelsText.setString(ss.str());

		ss.str("");

		//ss << //sf->GetNumShardsCaptured() << " / " << sf->GetNumShardsTotal() << " Shards collected";
		//capturedShardsText.setString(ss.str());

		//ss.str("");

		ss << sf->GetNumLogsCollected() << " / " << sf->GetNumLogsTotal() << " Logs collected";
		capturedLogsText.setString(ss.str());

		/*ss.str("");
		ss << "Time: " << sf->GetBestTimeString();
		totalTime.setString(ss.str());*/
		blankMode = false;
	}
	else
	{
		blankMode = true;
	}
}

SaveMenuScreen::SaveMenuScreen()
{
	mainMenu = MainMenu::GetInstance();

	//confirmPopup = new ConfirmPopup;

	menuOffset = Vector2f(0, 0);

	startWithTutorial = false;

	//TilesetManager &tsMan = mainMenu->tilesetManager;
	
	skinMenu = new SkinMenu(this);

	decisionPopup = new GamePopup;
	
	frame = 0;
	kinFaceTurnLength = 15;
	cloudLoopLength = 8;
	cloudLoopFactor = 5;

	saveJumpFactor = 5;
	saveJumpLength = 6;

	ts_background = GetTileset("Menu/Save/save_bg_1920x1080.png", 1920, 1080);
	ts_kinFace = GetTileset("Menu/Save/save_menu_kin_256x256.png", 256, 256);
	ts_selectSlot = GetTileset("Menu/Save/save_select_710x270.png", 710, 270);
	ts_skinButton = GetSizedTileset("Menu/Save/skin_select_256x80.png");
	
	actionLength[WAIT] = 1;
	actionLength[SELECT] = 12 * 3 + 24 * 2 + 20;
	actionLength[TRANSITION] = 30;
	actionLength[TRANSITIONMOVIE] = 30;
	actionLength[TRANSITIONTUTORIAL] = 60;
	actionLength[FADEIN] = 30;

	ts_skinButton->SetSpriteTexture(skinButtonSpr);
	ts_skinButton->SetSubRect(skinButtonSpr, 0);
	skinButtonSpr.setPosition(1542, 0);


	Vector2f buttonOffset(192, 2);

	skinButtonIconSpr.setPosition(skinButtonSpr.getPosition() + buttonOffset);

	AdventureFile *af = &mainMenu->adventureManager->adventureFile;

	string currName;
	if (mainMenu->adventureManager->currSaveFile != NULL)
	{
		currName = mainMenu->adventureManager->currSaveFile->name;
	}

	for (int i = 0; i < 6; ++i)
	{
		defaultFiles[i] = false;
		fileDisplay[i] = new SaveFileDisplay(mainMenu->arial);
		fileDisplay[i]->SetPosition(GetTopLeftSaveSlot(i));
	}

	for (int i = 0; i < 6; ++i)
	{
		defaultFiles[i] = !(mainMenu->adventureManager->files[i]->Load());

		if( !defaultFiles[i] )
			fileDisplay[i]->SetValues(mainMenu->adventureManager->files[i], mainMenu->adventureManager->adventurePlanet);
		else
		{
			fileDisplay[i]->SetValues(NULL, NULL);
		}
	}

	background.setTexture(*ts_background->texture);
	background.setPosition(menuOffset);


	kinFace.setTexture(*ts_kinFace->texture);
	kinFace.setTextureRect(ts_kinFace->GetSubRect(0));
	maskPlayerSkinShader.SetSubRect(ts_kinFace, ts_kinFace->GetSubRect(0));
	selectSlot.setTexture(*ts_selectSlot->texture);
	selectSlot.setTextureRect(ts_selectSlot->GetSubRect(0));

	ts_kinJump[0] = GetTileset("Menu/Save/save_kin_jump1_500x1080.png", 500, 1080);
	ts_kinJump[1] = GetTileset("Menu/Save/save_kin_jump2_500x1080.png", 500, 1080);
	ts_kinJump[2] = GetTileset("Menu/Save/save_kin_jump3_500x1080.png", 500, 1080);
	ts_kinJump[3] = GetTileset("Menu/Save/save_kin_jump4_500x1080.png", 500, 1080);
	ts_kinJump[4] = GetTileset("Menu/Save/save_kin_jump5_500x1080.png", 500, 1080);
	ts_kinJump[5] = GetTileset("Menu/Save/save_kin_jump6_500x1080.png", 500, 1080);

	ts_kinClouds = GetTileset("Menu/Save/save_kin_clouds_500x416.png", 500, 416);
	ts_kinWindow = GetTileset("Menu/Save/save_kin_window_500x1080.png", 500, 1080);
	//ts_saveKinWindow = tilesetManager.GetTileset( "Menu/save_kin_window_500x1080.png", 500, 1080 );
	ts_kinSky = GetTileset("Menu/Save/save_menu_sky_01_500x1080.png", 500, 1080);

	kinClouds.setTexture(*ts_kinClouds->texture);
	kinClouds.setTextureRect(ts_kinClouds->GetSubRect(0));
	kinClouds.setOrigin(kinClouds.getLocalBounds().width, kinClouds.getLocalBounds().height);
	kinClouds.setPosition(Vector2f(1920, 1080) + menuOffset);

	kinWindow.setTexture(*ts_kinWindow->texture);
	kinWindow.setOrigin(kinWindow.getLocalBounds().width, 0);
	kinWindow.setPosition(Vector2f(1920, 0) + menuOffset);

	kinSky.setTexture(*ts_kinSky->texture);
	kinSky.setOrigin(kinSky.getLocalBounds().width, 0);
	kinSky.setPosition(Vector2f(1920, 0) + menuOffset);

	SetRectCenter(blackQuad, 1920, 1080, Vector2f(1920 / 2, 1080 / 2));
	SetRectColor(blackQuad, Color(Color::Black));
	//saveKinJump.setTexture( ts_saveKin

	cloudFrame = 0;

	ts_starBackground = GetTileset("Menu/WorldMap/map_z1_stars.png", 1920, 1080);
	starBackground.setTexture(*ts_starBackground->texture);
	starBackground.setPosition(menuOffset);

	ts_world = GetTileset("Menu/WorldMap/map_z1_world.png", 1120, 1080);
	world.setTexture(*ts_world->texture);
	world.setOrigin(world.getLocalBounds().width / 2, world.getLocalBounds().height / 2);
	world.setPosition(Vector2f(960, 540) + menuOffset);

	//Tileset *ts_asteroid0 = tsMan.GetTileset("Menu/w0_asteroid_01_960x1080.png", 960, 1080);
	//Tileset *ts_asteroid1 = tsMan.GetTileset("Menu/w0_asteroid_02_1920x1080.png", 1920, 1080);
	//Tileset *ts_asteroid2 = tsMan.GetTileset("Menu/w0_asteroid_03_1920x1080.png", 1920, 1080);

	//asteroid0.setTexture(*ts_asteroid0->texture);
	//asteroid1.setTexture(*ts_asteroid1->texture);
	//asteroid2.setTexture(*ts_asteroid2->texture);

	/*asteroid0.setPosition(Vector2f(0, 0) + menuOffset);
	asteroid1.setPosition(Vector2f(0, 0) + menuOffset);
	asteroid2.setPosition(Vector2f(0, 0) + menuOffset);*/

	a0start = Vector2f(-1920, 0) + menuOffset;
	a0end = Vector2f(1920, 0) + menuOffset;

	a1start = Vector2f(1920, 0) + menuOffset;
	a1end = Vector2f(-1920, 0) + menuOffset;

	a2start = Vector2f(-1920, 0) + menuOffset;
	a2end = Vector2f(1920, 0) + menuOffset;

	asteroidScrollFrames0 = 2000;
	asteroidScrollFrames1 = 500;
	asteroidScrollFrames2 = 120;

	asteroidFrameBack = asteroidScrollFrames0 / 2;
	asteroidFrameFront = asteroidScrollFrames1 / 2;

	Vector2f start(177.5, 987);
	//180, 535, 888, 1243
	//355 each
	//177.5
	
	for (int i = 0; i < 4; ++i)
	{
		SetRectCenter(actionButtonIcons + i * 4, 128, 128, start + Vector2f(355 * i, 0));
	}

	UpdateButtonIconsWhenControllerIsChanged();

	SetSelectedIndex(mainMenu->adventureManager->currSaveFileIndex);

	
	myMusic = mainMenu->musicManager->songMap["w0_Save_Menu"];
	if (myMusic != NULL)
	{
		myMusic->Load();
	}

	Reset();
	//kinFace.setPosition(topLeftPos + Vector2f( 50, 0 ));
}

SaveMenuScreen::~SaveMenuScreen()
{
	delete decisionPopup;

	delete skinMenu;
	for (int i = 0; i < 6; ++i)
	{
		delete fileDisplay[i];
	}

	//delete confirmPopup;
}

void SaveMenuScreen::UpdateButtonIconsWhenControllerIsChanged()
{
	int cType = mainMenu->adventureManager->controllerInput->GetControllerType();

	ts_buttons = mainMenu->GetButtonIconTileset(cType);

	auto button = XBoxButton::XBOX_A;
	IntRect ir = mainMenu->GetButtonIconTileForMenu(cType, button);
	SetRectSubRect(actionButtonIcons, ir);

	button = XBoxButton::XBOX_Y;
	ir = mainMenu->GetButtonIconTileForMenu(cType, button);
	SetRectSubRect(actionButtonIcons + 4 * 1, ir);

	button = XBoxButton::XBOX_X;
	ir = mainMenu->GetButtonIconTileForMenu(cType, button);
	SetRectSubRect(actionButtonIcons + 4 * 2, ir);

	button = XBoxButton::XBOX_B;
	ir = mainMenu->GetButtonIconTileForMenu(cType, button);
	SetRectSubRect(actionButtonIcons + 4 * 3, ir);

	button = XBoxButton::XBOX_R1;
	ts_buttons->SetSpriteTexture(skinButtonIconSpr);
	skinButtonIconSpr.setTextureRect(mainMenu->GetButtonIconTileForMenu(cType, button));

	if (ts_buttons == mainMenu->ts_buttonIcons)
	{
		skinButtonIconSpr.setScale(.5, .5);
	}
	else
	{
		skinButtonIconSpr.setScale(1.0, 1.0);
	}
}

void SaveMenuScreen::Start()
{
	mainMenu->musicPlayer->PlayMusic(myMusic);
}

void SaveMenuScreen::SaveSelectedFile()
{
	mainMenu->adventureManager->currSaveFile->Save();
}

void SaveMenuScreen::SetSkin(int index)
{
	currSkin = index;
	playerSkinShader.SetSkin(currSkin);
	maskPlayerSkinShader.SetSkin(currSkin);
}

void SaveMenuScreen::SaveCurrSkin()
{
	mainMenu->adventureManager->currSaveFile->visualInfo.skinIndex = currSkin;
	SaveSelectedFile();
}

bool SaveMenuScreen::HandleEvent(sf::Event ev)
{
	switch (action)
	{
	case ASKTUTORIAL:
	case CONFIRMDELETE:
		break;
		//return confirmPopup->HandleEvent(ev);
	}

	return true;
}

bool SaveMenuScreen::Update()
{
	if (frame == actionLength[action])
	{
		switch (action)
		{
		case WAIT:
			frame = 0;
			break;
		case SELECT:
			if (defaultFiles[mainMenu->adventureManager->currSaveFileIndex])
			{
				defaultFiles[mainMenu->adventureManager->currSaveFileIndex] = false;
				mainMenu->adventureManager->StartDefaultSaveFile(mainMenu->adventureManager->currSaveFileIndex);
				//action = TRANSITIONMOVIE;
				if (startWithTutorial)
				{
					//mainMenu->musicPlayer->FadeOutCurrentMusic(30);
					//mainMenu->LoadMode(MainMenu::ADVENTURETUTORIAL);
					action = TRANSITIONTUTORIAL;
				}
				else
				{
					action = TRANSITION;
					mainMenu->adventureManager->worldMap->InitSelectors();
					//mainMenu->adventureManager->worldMap->SetDefaultSelections();
				}
			}
			else
			{
				//mainMenu->customCursor->Show();
				//mainMenu->customCursor->SetMode(CustomCursor::M_SHIP);
				//MOUSE.SetPosition(Vector2i(mainMenu->worldMap->worldSelector->position));

				action = TRANSITION;
			}

			mainMenu->adventureManager->worldMap->UpdateWorldStats();

			transparency = 0;
			fadeOut = 0;
			frame = 0;
			break;
		case TRANSITION:
		{
			mainMenu->SetMode(MainMenu::Mode::TRANS_SAVE_TO_WORLDMAP);
			mainMenu->transAlpha = 255;
			mainMenu->adventureManager->worldMap->state = WorldMap::PLANET;//WorldMap::PLANET_AND_SPACE;
			mainMenu->adventureManager->worldMap->frame = 0;
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundInfos[MainMenu::S_SELECT]);

			mainMenu->adventureManager->worldMap->InitSelectors();
			return true;
			break;
		}
		case TRANSITIONTUTORIAL:
		{
			//mainMenu->musicPlayer->FadeOutCurrentMusic(30);
			//mainMenu->LoadMode(MainMenu::TUTORIAL);
			return true;

			//mainMenu->SetMode(MainMenu::Mode::TRANS_SAVE_TO_WORLDMAP);
			//mainMenu->transAlpha = 255;
			//mainMenu->worldMap->state = WorldMap::PLANET;//WorldMap::PLANET_AND_SPACE;
			//mainMenu->worldMap->frame = 0;
			//mainMenu->soundNodeList->ActivateSound(mainMenu->soundBuffers[MainMenu::S_SELECT]);

			//mainMenu->worldMap->InitSelectors();
			break;
		}
		case TRANSITIONMOVIE:
		{
			mainMenu->PlayIntroMovie();
			return true;
			break;
		}
		case FADEIN:
		{
			action = WAIT;
			break;
		}
		case SKINMENU:
		{
			frame = 0;
			break;
		}
		case CONFIRMCOPY:
		case CONFIRMDELETE:
		case CONFIRMDELETE2:
		case COPY:
		{
			frame = 0;
			break;
		}
		}
	}

	ControllerDualStateQueue *controllerInput = mainMenu->adventureManager->controllerInput;

	int moveDelayFrames = 60;
	int moveDelayFramesSmall = 40;

	bool changedToSkin = false;

	bool keyboardBack = controllerInput->GetControllerType() != CTYPE_KEYBOARD && (CONTROLLERS.KeyboardButtonPressed(Keyboard::BackSpace)
		|| CONTROLLERS.KeyboardButtonPressed(Keyboard::Escape));

	if (mainMenu->menuMode == MainMenu::SAVEMENU )
	{
		if( action == WAIT )
		{
			if (controllerInput->ButtonPressed_A() )//(MouseIsOverSelectedFile() && MOUSE.IsMouseLeftClicked()) )
			{
				if (defaultFiles[mainMenu->adventureManager->currSaveFileIndex])
				{
					action = ASKTUTORIAL;
					frame = 0;

					decisionPopup->SetInfo("You are starting a new file.\nDo you want a tutorial?", 3, GamePopup::OPTION_YES);
				}
				else
				{
					action = SELECT;
					frame = 0;
					mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("save_Select"));
					return true;
				}
			}
			else if (controllerInput->ButtonPressed_B() || keyboardBack)
			{
				mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("main_menu_back"));
				return false;
			}
			else if (controllerInput->ButtonPressed_RightShoulder())
			{
				//if (!defaultFiles[mainMenu->adventureManager->currSaveFileIndex])
				{
					action = SKINMENU;
					frame = 0;
					skinMenu->SetSelectedIndex(mainMenu->adventureManager->currSaveFile->visualInfo.skinIndex);
					changedToSkin = true; //so you dont exit the same frame you open
				}
			}
			else if (controllerInput->ButtonPressed_X())
			{
				if (!defaultFiles[mainMenu->adventureManager->currSaveFileIndex])
				{
					action = CONFIRMDELETE;

					decisionPopup->SetInfo("Are you sure you want\nto delete this save file?", 2, GamePopup::OPTION_NO);
					frame = 0;
				}
			}
			else if (controllerInput->ButtonPressed_Y())
			{
				if (!defaultFiles[mainMenu->adventureManager->currSaveFileIndex])
				{
					action = COPY;
					frame = 0;
					copiedIndex = mainMenu->adventureManager->currSaveFileIndex;
				}
			}
			else
			{
				ChangeIndex(controllerInput->MenuDirPressed_Down(), controllerInput->MenuDirPressed_Up(), 
					controllerInput->MenuDirPressed_Left(), controllerInput->MenuDirPressed_Right());
			}
		}
		else if (action == CONFIRMDELETE)
		{
			
			int res = decisionPopup->Update( mainMenu->adventureManager->controllerInput);

			if (res == GamePopup::OPTION_BACK || res == GamePopup::OPTION_NO)
			{
				action = WAIT;
				frame = 0;
			}
			else if (res == GamePopup::OPTION_YES)
			{
				action = CONFIRMDELETE2;
				//confirmPopup->Pop();
				//confirmPopup->SetQuestion("Save file will be permanently\ndeleted. Continue?");

				decisionPopup->SetInfo("Save file will be permanently\ndeleted. Continue?", 2, GamePopup::OPTION_NO);
				frame = 0;
			}

			/*if (confirmPopup->action == ConfirmPopup::A_YES )
			{
				
			}
			else if (confirmPopup->action == ConfirmPopup::A_NO)
			{
				
			}*/
		}
		else if (action == CONFIRMDELETE2)
		{
			int res = decisionPopup->Update(mainMenu->adventureManager->controllerInput);

			if (res == GamePopup::OPTION_BACK || res == GamePopup::OPTION_NO)
			{
				action = WAIT;
				frame = 0;
			}
			else if (res == GamePopup::OPTION_YES)
			{
				action = INFOPOP;
				frame = 0;

				SetSkin(0);

				mainMenu->adventureManager->files[mainMenu->adventureManager->currSaveFileIndex]->Delete();
				fileDisplay[mainMenu->adventureManager->currSaveFileIndex]->SetValues(NULL, NULL);

				decisionPopup->SetInfo("Deleted save file", 1);

				defaultFiles[mainMenu->adventureManager->currSaveFileIndex] = true;
				
			}
		}
		else if (action == INFOPOP)
		{
			int res = decisionPopup->Update(mainMenu->adventureManager->controllerInput);

			if (res == GamePopup::OPTION_YES)
			{
				action = WAIT;
				frame = 0;
			}
		}
		else if (action == ASKTUTORIAL)
		{
			int res = decisionPopup->Update(mainMenu->adventureManager->controllerInput);

			if (res == GamePopup::OPTION_NO)
			{
				action = SELECT;
				frame = 0;
				mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("save_Select"));
				startWithTutorial = false;

				mainMenu->adventureManager->worldMap->SetShipToColony(0);
			}
			else if (res == GamePopup::OPTION_YES)
			{
				action = SELECT;
				frame = 0;
				mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("save_Select"));
				startWithTutorial = true;
			}
			else if (res == GamePopup::OPTION_BACK)
			{
				action = WAIT;
				frame = 0;
			}

			/*confirmPopup->Update();
			if (confirmPopup->action == ConfirmPopup::A_YES)
			{
				action = SELECT;
				frame = 0;
				mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("save_Select"));
				startWithTutorial = true;
			}
			else if (confirmPopup->action == ConfirmPopup::A_NO)
			{
				
			}
			else if (confirmPopup->action == ConfirmPopup::A_BACK)
			{
				action = WAIT;
				frame = 0;
			}*/
		}
		else if (action == COPY)
		{
			if (controllerInput->ButtonPressed_A())
			{
				if (defaultFiles[mainMenu->adventureManager->currSaveFileIndex])
				{
					action = CONFIRMCOPY;
					frame = 0;
					//confirmPopup->SetQuestion("Are you sure you want to\ncopy the save file here?");

					decisionPopup->SetInfo("Are you sure you want to\ncopy the save file here?", 2, GamePopup::OPTION_NO);
				}
				else
				{
					action = INFOPOP;
					frame = 0;

					decisionPopup->SetInfo("Cannot overwrite existing file", 1);
					//messagePopup->Pop("Cannot overwrite existing file");
				}
			}
			else if (controllerInput->ButtonPressed_B() || keyboardBack )
			{
				action = WAIT;
				frame = 0;
			}
			else
			{
				ChangeIndex(controllerInput->MenuDirPressed_Down(), controllerInput->MenuDirPressed_Up(), controllerInput->MenuDirPressed_Left(), controllerInput->MenuDirPressed_Right());
			}
		}
		else if (action == CONFIRMCOPY)
		{
			int res = decisionPopup->Update(mainMenu->adventureManager->controllerInput);

			if (res == GamePopup::OPTION_NO || res == GamePopup::OPTION_BACK)
			{
				action = WAIT;
				frame = 0;
			}
			else if (res == GamePopup::OPTION_YES)
			{
				action = INFOPOP;
				frame = 0;

				decisionPopup->SetInfo("Copied save file successfully", 1);

				mainMenu->adventureManager->files[mainMenu->adventureManager->currSaveFileIndex]->SetAndSave(mainMenu->adventureManager->files[copiedIndex]);
				defaultFiles[mainMenu->adventureManager->currSaveFileIndex] = false;
				fileDisplay[mainMenu->adventureManager->currSaveFileIndex]->SetValues(mainMenu->adventureManager->files[mainMenu->adventureManager->currSaveFileIndex], mainMenu->adventureManager->adventurePlanet);
			}
		}
	}

	if (controllerInput->ButtonHeld_RightShoulder())
	{
		ts_skinButton->SetSubRect(skinButtonSpr, 1);
	}
	else
	{
		ts_skinButton->SetSubRect(skinButtonSpr, 0);
	}

	switch (action)
	{
	case WAIT:
		transparency = 0;
		fadeOut = 0;
		break;
	case SELECT:
	{
		if (frame < 15 * 3)
		{
			kinFace.setTextureRect(ts_kinFace->GetSubRect(frame / 3));
			maskPlayerSkinShader.SetSubRect(ts_kinFace, ts_kinFace->GetSubRect(frame / 3));
		}
			

		if (frame >= 12 * 3)
		{
			int jFrame = (frame - 12 * 3);

			if (jFrame >= 6 * 4 * 2)
			{
				break;
			}

			int texIndex = jFrame / (4 * 2);
			int rem = jFrame % (4 * 2);
			if ( rem == 0 )
			{
				kinJump.setTexture(*ts_kinJump[texIndex]->texture);
			}

			Tileset *ts_currKinJump = ts_kinJump[texIndex];
			IntRect ir = ts_currKinJump->GetSubRect(rem / 2);
			playerSkinShader.SetSubRect(ts_currKinJump, ir);
			kinJump.setTextureRect(ir);
		}

		break;
	}
	case TRANSITION:
		transparency = (float)(frame%actionLength[TRANSITION]) / actionLength[TRANSITION];
		break;
	case TRANSITIONMOVIE:
		fadeOut = (float)(frame%actionLength[TRANSITIONMOVIE]) / actionLength[TRANSITIONMOVIE];
		break;
	case FADEIN:
		transparency = 1.f - ((float)(frame%actionLength[FADEIN]) / actionLength[FADEIN]);
		break;
	case SKINMENU:
	{
		if (!changedToSkin)
		{
			if (!skinMenu->Update(controllerInput))
			{
				action = WAIT;
				frame = 0;
				SetSkin(mainMenu->adventureManager->currSaveFile->visualInfo.skinIndex);
			}
		}
		break;
	}
	}

	//selectSlot.setTextureRect(ts_selectSlot->GetSubRect(selectedSaveIndex));
	////kinFace.setTextureRect(ts_kinFace->GetSubRect(0));

	//Vector2f topLeftPos = GetTopLeftSaveSlot(selectedSaveIndex);

	//selectSlot.setPosition(topLeftPos);
	//kinFace.setPosition(topLeftPos + Vector2f( 15, -6 ));

	

	UpdateClouds();
	++asteroidFrameBack;
	++asteroidFrameFront;

	int r0 = asteroidFrameBack % asteroidScrollFrames0;
	int r1 = asteroidFrameFront % asteroidScrollFrames1;
	//int r2 = asteroidFrame % asteroidScrollFrames2;

	Vector2f offset0(0, 0);
	Vector2f offset1(0, 0);
	CubicBezier bez(0, 0, 1, 1);
	double v = bez.GetValue(r0 / (double)asteroidScrollFrames0);
	double v1 = bez.GetValue(r1 / (double)asteroidScrollFrames1);
	offset0.x = 1920 * 3 * v;
	offset1.x = -1920 * v1;

	//cout << "asteroidframe: " << asteroidFrame << ", offset0: " << offset0.x << ", offset1: " << offset1.x << endl;


	asteroid0.setPosition(a0start * (float)(1.0 - v1) + a0end * (float)v1);
	asteroid1.setPosition(a1start * (float)(1.0 - v) + a1end * (float)v);
	asteroid2.setPosition(a2start * (float)(1.0 - v1) + a2end * (float)v1);


	++frame;
	
	return true;
}

void SaveMenuScreen::SetSelectedIndex(int index)
{
	mainMenu->adventureManager->SetCurrSaveFile(index);
	if (action != COPY)
	{
		SetSkin(mainMenu->adventureManager->currSaveFile->visualInfo.skinIndex);
	}

	int currColonyIndex = 0;
	if (defaultFiles[index])
	{
		currColonyIndex = 0;
	}
	else
	{
		currColonyIndex = mainMenu->adventureManager->currSaveFile->mostRecentWorldSelected;
	}

	selectSlot.setTextureRect(ts_selectSlot->GetSubRect(index));
	//kinFace.setTextureRect(ts_kinFace->GetSubRect(0));

	Vector2f topLeftPos = GetTopLeftSaveSlot(index);

	selectSlot.setPosition(topLeftPos);
	kinFace.setPosition(topLeftPos + Vector2f(15, -6));

	mainMenu->adventureManager->worldMap->SetShipToColony(currColonyIndex);
}

void SaveMenuScreen::SelectedIndexChanged()
{
	mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("save_change"));

	//SetSelectedIndex(mainMenu->adventureManager->currSaveFileIndex);
}

void SaveMenuScreen::UnlockSkin(int skinIndex)
{
	mainMenu->UnlockSkin(skinIndex);
}

bool SaveMenuScreen::IsSkinUnlocked(int skinIndex)
{
	return mainMenu->IsSkinUnlocked(skinIndex);
}

void SaveMenuScreen::UpdateSelectedIndex()
{
	Vector2f mousePos = MOUSE.GetFloatPos();

	bool isOnSomething = false;

	int foundIndex = -1;
	for (int i = 0; i < 6; ++i)
	{
		if (fileDisplay[i]->Contains(mousePos))
		{
			foundIndex = i;
			break;
		}
	}

	if (foundIndex != mainMenu->adventureManager->currSaveFileIndex)
	{
		SetSelectedIndex(foundIndex);
		SelectedIndexChanged();
	}
}

void SaveMenuScreen::ChangeIndex(bool down, bool up, bool left, bool right)
{
	int currInd = mainMenu->adventureManager->currSaveFileIndex;
	int oldInd = currInd;
	if (down)
	{
		currInd += 2;
		if (currInd > 5)
			currInd -= 6;
	}
	else if (up)
	{
		currInd -= 2;
		if (currInd < 0)
			currInd += 6;
	}

	if (right)
	{
		currInd++;
		if (currInd % 2 == 0)
			currInd -= 2;
	}
	else if (left)
	{
		currInd--;
		if (currInd % 2 == 1)
			currInd += 2;
		else if (currInd < 0)
		{
			currInd += 2;
		}
	}

	if (currInd != oldInd)
	{
		SetSelectedIndex(currInd);
		SelectedIndexChanged();
	}
}

Vector2f SaveMenuScreen::GetTopLeftSaveSlot(int index)
{
	Vector2f topLeftPos;
	topLeftPos.x += ts_selectSlot->tileWidth * (index %  2);
	topLeftPos.y += ts_selectSlot->tileHeight * (index / 2);
	//topLeftPos += menuOffset;

	return topLeftPos;
}

void SaveMenuScreen::Draw(sf::RenderTarget *target)
{

	RenderTexture *saveTexture = mainMenu->saveTexture;
	saveTexture->clear(Color::Transparent);
	
	saveTexture->setView(mainMenu->v);

	SetRectColor(blackQuad, Color( 0, 0, 0, fadeOut * 255));

	saveTexture->draw(background);
	saveTexture->draw(kinSky);
	saveTexture->draw(kinClouds);
	saveTexture->draw(kinWindow);

	int endDraw = 12 * 3 + 24 * 2;
	if (action == WAIT || (action == SELECT && frame < endDraw) || action == FADEIN || action == SKINMENU
		|| action == CONFIRMDELETE || action == CONFIRMDELETE2 || action == CONFIRMCOPY || action == COPY || action == INFOPOP
		|| action == ASKTUTORIAL || action == TRANSITIONTUTORIAL)
	{
		saveTexture->draw(kinJump, &playerSkinShader.pShader);
	}

	saveTexture->draw(selectSlot);
	saveTexture->draw(kinFace, &maskPlayerSkinShader.pShader);
	
	
	for (int i = 0; i < 6; ++i)
	{
		fileDisplay[i]->Draw(saveTexture);
	}

	saveTexture->draw(blackQuad, 4, sf::Quads);
	
	saveTexture->draw(skinButtonSpr);
	saveTexture->draw(skinButtonIconSpr);

	saveTexture->draw(actionButtonIcons, 4 * 4, sf::Quads, ts_buttons->texture);

	saveTexture->display();
	const Texture &saveTex = saveTexture->getTexture();
	sf::Sprite saveSpr(saveTex);

	saveSpr.setColor(Color(255, 255, 255, 255 * (1.f - transparency)));
	
	target->draw(saveSpr);

	

	if (action == SKINMENU)
	{
		skinMenu->Draw(target);
	}
	else if (action == CONFIRMDELETE || action == CONFIRMDELETE2 || action == CONFIRMCOPY)
	{
		decisionPopup->Draw(target);
		//confirmPopup->Draw(target);
	}
	else if (action == ASKTUTORIAL)
	{
		decisionPopup->Draw(target);
		//confirmPopup->Draw(target);
	}
	else if (action == INFOPOP)
	{
		decisionPopup->Draw(target);
		//messagePopup->Draw(target);
		//infoPopup->Draw(target);
	}
}

void SaveMenuScreen::Reset()
{
	skinMenu->Reset();
	//doesnt reset the selected save index
	fadeOut = 0;
	transparency = 0;
	action = WAIT;
	frame = 0;

	for (int i = 0; i < 6; ++i)
	{
		if (!defaultFiles[i])
		{
			fileDisplay[i]->SetValues(mainMenu->adventureManager->files[i], mainMenu->adventureManager->adventurePlanet);
		}
		else
		{
			fileDisplay[i]->SetValues(NULL, NULL);
		}
	}
	
	asteroidFrameBack = 0;
	asteroidFrameFront = 0;

	kinJump.setTexture(*ts_kinJump[0]->texture);
	kinJump.setTextureRect(ts_kinJump[0]->GetSubRect(0));
	kinJump.setOrigin(kinJump.getLocalBounds().width, 0);
	kinJump.setPosition(Vector2f(1920, 0) + menuOffset);

	playerSkinShader.SetSubRect(ts_kinJump[0], ts_kinJump[0]->GetSubRect(0));

	kinFace.setTextureRect(ts_kinFace->GetSubRect(0));
	maskPlayerSkinShader.SetSubRect(ts_kinFace, ts_kinFace->GetSubRect(0));
}

void SaveMenuScreen::UpdateClouds()
{
	if (cloudFrame == cloudLoopLength * cloudLoopFactor)
	{
		cloudFrame = 0;
	}

	int f = cloudFrame / cloudLoopFactor;

	//cout << "cloud frame: " << f << endl;
	kinClouds.setTextureRect(ts_kinClouds->GetSubRect(f));

	cloudFrame++;
}