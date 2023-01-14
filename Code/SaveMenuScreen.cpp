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

using namespace sf;
using namespace std;

SaveFileDisplay::SaveFileDisplay(sf::Font &f)
	:font( f )
{
	blankMode = true;
	fillColor = Color::White;
	lineColor = Color::Black;

	InitText(capturedShardsText);
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

		ss << sf->GetNumShardsCaptured() << " / " << sf->GetNumShardsTotal() << " Shards collected";
		capturedShardsText.setString(ss.str());
		
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
	:playerSkinShader( "player" ),maskPlayerSkinShader( "player" )
{
	mainMenu = MainMenu::GetInstance();

	messagePopup = new MessagePopup;
	confirmPopup = new ConfirmPopup;

	menuOffset = Vector2f(0, 0);

	startWithTutorial = false;

	//TilesetManager &tsMan = mainMenu->tilesetManager;
	
	skinMenu = new SkinMenu(this);
	
	frame = 0;
	kinFaceTurnLength = 15;
	cloudLoopLength = 8;
	cloudLoopFactor = 5;

	saveJumpFactor = 5;
	saveJumpLength = 6;

	ts_background = GetTileset("Menu/save_bg_1920x1080.png", 1920, 1080);
	ts_kinFace = GetTileset("Menu/save_menu_kin_256x256.png", 256, 256);
	ts_selectSlot = GetTileset("Menu/save_select_710x270.png", 710, 270);
	ts_skinButton = GetSizedTileset("Menu/skin_select_256x80.png");
	
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

	ts_kinJump[0] = GetTileset("Menu/save_kin_jump1_500x1080.png", 500, 1080);
	ts_kinJump[1] = GetTileset("Menu/save_kin_jump2_500x1080.png", 500, 1080);
	ts_kinJump[2] = GetTileset("Menu/save_kin_jump3_500x1080.png", 500, 1080);
	ts_kinJump[3] = GetTileset("Menu/save_kin_jump4_500x1080.png", 500, 1080);
	ts_kinJump[4] = GetTileset("Menu/save_kin_jump5_500x1080.png", 500, 1080);
	ts_kinJump[5] = GetTileset("Menu/save_kin_jump6_500x1080.png", 500, 1080);

	ts_kinClouds = GetTileset("Menu/save_kin_clouds_500x416.png", 500, 416);
	ts_kinWindow = GetTileset("Menu/save_kin_window_500x1080.png", 500, 1080);
	//ts_saveKinWindow = tilesetManager.GetTileset( "Menu/save_kin_window_500x1080.png", 500, 1080 );
	ts_kinSky = GetTileset("Menu/save_menu_sky_01_500x1080.png", 500, 1080);

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

	ts_starBackground = GetTileset("WorldMap/map_z1_stars.png", 1920, 1080);
	starBackground.setTexture(*ts_starBackground->texture);
	starBackground.setPosition(menuOffset);

	ts_world = GetTileset("WorldMap/map_z1_world.png", 1120, 1080);
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

	//Reset();

	//selectSlot.setTextureRect(ts_selectSlot->GetSubRect(selectedSaveIndex));
	//kinFace.setTextureRect(ts_kinFace->GetSubRect(0));

	//Vector2f topLeftPos = GetTopLeftSaveSlot(selectedSaveIndex);

	//selectSlot.setPosition(topLeftPos);

	SetSelectedIndex(0);

	Reset();
	//kinFace.setPosition(topLeftPos + Vector2f( 50, 0 ));
}

SaveMenuScreen::~SaveMenuScreen()
{
	delete skinMenu;
	for (int i = 0; i < 6; ++i)
	{
		delete fileDisplay[i];
	}

	delete messagePopup;
	delete confirmPopup;
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
	mainMenu->adventureManager->currSaveFile->defaultSkinIndex = currSkin;
	SaveSelectedFile();
}

bool SaveMenuScreen::HandleEvent(sf::Event ev)
{
	switch (action)
	{
	case ASKTUTORIAL:
	case CONFIRMDELETE:
		return confirmPopup->HandleEvent(ev);
	}

	return true;
}

bool SaveMenuScreen::Update()
{
	Tileset * ts_buttons = mainMenu->GetButtonIconTileset(0);
	IntRect ir = mainMenu->GetButtonIconTile(0, 4);
	ts_buttons->SetSpriteTexture(skinButtonIconSpr);
	skinButtonIconSpr.setTextureRect(ir);

	if (ts_buttons == mainMenu->ts_buttonIcons)
	{
		skinButtonIconSpr.setScale(.5, .5);
	}
	else
	{
		skinButtonIconSpr.setScale(1.0, 1.0);
	}

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
					mainMenu->adventureManager->worldMap->SetDefaultSelections();
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
					confirmPopup->Pop(ConfirmPopup::BACK_ALLOWED);
					confirmPopup->SetQuestion("You are starting a new file.\nDo you want a tutorial?");
					//decisionPopup.SetOption(SaveMenuDecisionPopup::OPTION_YES);
				}
				else
				{
					action = SELECT;
					frame = 0;
					mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("save_Select"));
					return true;
				}
			}
			else if (controllerInput->ButtonPressed_B())
			{
				mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("main_menu_back"));
				return false;
			}
			else if (controllerInput->ButtonPressed_RightShoulder())
			{
				action = SKINMENU;
				frame = 0;
				skinMenu->SetSelectedIndex(mainMenu->adventureManager->currSaveFile->defaultSkinIndex);
				changedToSkin = true; //so you dont exit the same frame you open
			}
			else if (controllerInput->ButtonPressed_X())
			{
				if (!defaultFiles[mainMenu->adventureManager->currSaveFileIndex])
				{
					action = CONFIRMDELETE;
					confirmPopup->SetQuestion("Are you sure you want\nto delete this save file?");
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
				ChangeIndex(controllerInput->DirPressed_Down(), controllerInput->DirPressed_Up(), controllerInput->DirPressed_Left(), controllerInput->DirPressed_Right());
			}
		}
		else if (action == CONFIRMDELETE)
		{
			confirmPopup->Update();
			if (confirmPopup->action == ConfirmPopup::A_YES )
			{
				action = CONFIRMDELETE2;
				confirmPopup->Pop();
				confirmPopup->SetQuestion("Save file will be permanently\ndeleted. Continue?");
				frame = 0;
			}
			else if (confirmPopup->action == ConfirmPopup::A_NO)
			{
				action = WAIT;
				frame = 0;
			}
		}
		else if (action == CONFIRMDELETE2)
		{
			if (confirmPopup->action == ConfirmPopup::A_YES)
			{
				action = INFOPOP;
				frame = 0;
				messagePopup->Pop("Deleted save file");
				//infoPopup->SetText("Deleted save file");

				defaultFiles[mainMenu->adventureManager->currSaveFileIndex] = true;
				mainMenu->adventureManager->files[mainMenu->adventureManager->currSaveFileIndex]->Delete();
				fileDisplay[mainMenu->adventureManager->currSaveFileIndex]->SetValues(NULL, NULL);
				
			}
			else if (confirmPopup->action == ConfirmPopup::A_NO)
			{
				action = WAIT;
				frame = 0;
			}
		}
		else if (action == INFOPOP)
		{
			if (messagePopup->action == MessagePopup::A_INACTIVE )
			{
				action = WAIT;
				frame = 0;
			}
		}
		else if (action == ASKTUTORIAL)
		{
			confirmPopup->Update();
			if (confirmPopup->action == ConfirmPopup::A_YES)
			{
				action = SELECT;
				frame = 0;
				mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("save_Select"));
				startWithTutorial = true;
			}
			else if (confirmPopup->action == ConfirmPopup::A_NO)
			{
				action = SELECT;
				frame = 0;
				mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("save_Select"));
				startWithTutorial = false;
			}
			else if (confirmPopup->action == ConfirmPopup::A_BACK)
			{
				action = WAIT;
				frame = 0;
			}
		}
		else if (action == COPY)
		{
			if (controllerInput->ButtonPressed_A())
			{
				if (defaultFiles[mainMenu->adventureManager->currSaveFileIndex])
				{
					action = CONFIRMCOPY;
					frame = 0;
					confirmPopup->SetQuestion("Are you sure you want to\ncopy the save file here?");
				}
				else
				{
					action = INFOPOP;
					frame = 0;
					messagePopup->Pop("Cannot overwrite existing file");
				}
			}
			else if (controllerInput->ButtonPressed_B())
			{
				action = WAIT;
				frame = 0;
			}
			else
			{
				//ChangeIndex(down, up, left, right);
			}
		}
		else if (action == CONFIRMCOPY)
		{
			confirmPopup->Update();
			if (confirmPopup->action == ConfirmPopup::A_YES)
			{
				action = INFOPOP;
				frame = 0;
				messagePopup->Pop("Copied save file successfully");

				mainMenu->adventureManager->files[copiedIndex]->CopyTo(mainMenu->adventureManager->files[mainMenu->adventureManager->currSaveFileIndex]);
				mainMenu->adventureManager->files[mainMenu->adventureManager->currSaveFileIndex]->Save();
				defaultFiles[mainMenu->adventureManager->currSaveFileIndex] = false;
				fileDisplay[mainMenu->adventureManager->currSaveFileIndex]->SetValues(mainMenu->adventureManager->files[mainMenu->adventureManager->currSaveFileIndex], mainMenu->adventureManager->adventurePlanet);
			}
			else if (confirmPopup->action == ConfirmPopup::A_NO || confirmPopup->action == ConfirmPopup::A_BACK )
			{
				action = WAIT;
				frame = 0;
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
				SetSkin(mainMenu->adventureManager->currSaveFile->defaultSkinIndex);
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

	
	//if( kinFaceFrame < saveKinFaceTurnLength * 3 )
	//{
	//	saveKinFace.setTextureRect( ts_saveMenuKinFace->GetSubRect( kinFaceFrame / 3 ) );
	//}
	//else
	//{
	//	saveKinFace.setTextureRect( ts_saveMenuKinFace->GetSubRect( saveKinFaceTurnLength - 1 ) );
	//}

	//if( kinFaceFrame < saveJumpLength * saveJumpFactor )
	//{
	//	if( kinFaceFrame == 0 )
	//	{
	//		saveKinJump.setTexture( *ts_saveKinJump1->texture );
	//	}
	//	else if( kinFaceFrame == 3 * saveJumpFactor )
	//	{
	//		saveKinJump.setTexture( *ts_saveKinJump2->texture );
	//	}

	//	int f = kinFaceFrame / saveJumpFactor;
	//	if( kinFaceFrame < 3 * saveJumpFactor )
	//	{
	//		saveKinJump.setTextureRect( ts_saveKinJump1->GetSubRect( f ) );
	//	}
	//	else
	//	{
	//		saveKinJump.setTextureRect( ts_saveKinJump2->GetSubRect( f - 3 ) );
	//	}

	//	saveKinJump.setOrigin( saveKinJump.getLocalBounds().width, 0);
	//}

	++frame;
	
	return true;
}

void SaveMenuScreen::SetSelectedIndex(int index)
{
	mainMenu->adventureManager->SetCurrSaveFile(index);
	if (action != COPY)
	{
		SetSkin(mainMenu->adventureManager->currSaveFile->defaultSkinIndex);
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
		confirmPopup->Draw(target);
	}
	else if (action == ASKTUTORIAL)
	{
		confirmPopup->Draw(target);
	}
	else if (action == INFOPOP)
	{
		messagePopup->Draw(target);
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

SaveMenuConfirmPopup::SaveMenuConfirmPopup( MainMenu *mainMenu )
{
	size = Vector2f(500, 300);
	ts_buttons = mainMenu->tilesetManager.GetSizedTileset("Menu/button_icon_128x128.png");
	SetRectColor(popupBGQuad, Color::Black);

	ts_buttons->SetQuadSubRect(buttonQuads, 0);
	ts_buttons->SetQuadSubRect(buttonQuads+4, 1);

	confirmText.setFont(mainMenu->arial);
	confirmText.setCharacterSize(40);
	confirmText.setFillColor(Color::White);
	confirmText.setString("HELLO");

	SetPos(Vector2f(960, 540));
}

void SaveMenuConfirmPopup::SetPos(sf::Vector2f &pos)
{
	position = pos;
	SetRectCenter(popupBGQuad, size.x, size.y, pos);
	SetRectCenter(buttonQuads, 128, 128, pos + Vector2f(-100, 50));
	SetRectCenter(buttonQuads+4, 128, 128, pos + Vector2f(100, 50));
	confirmText.setPosition(Vector2f( pos.x, (pos.y - size.y / 2) + 10));
}

int SaveMenuConfirmPopup::Update(ControllerState &currInput,
	ControllerState &prevInput)
{
	if (currInput.A && !prevInput.A)
	{
		return OPTION_CONFIRM;
	}
	else if (currInput.B && !prevInput.B)
	{
		return OPTION_BACK;
	}

	return OPTION_NOTHING;
}

void SaveMenuConfirmPopup::Draw(sf::RenderTarget *target)
{
	target->draw(popupBGQuad, 4, sf::Quads);
	target->draw(confirmText);
	target->draw(buttonQuads, 4 * 2, sf::Quads, ts_buttons->texture);
}

void SaveMenuConfirmPopup::SetText(const std::string &str)
{
	confirmText.setString(str);
	confirmText.setOrigin(confirmText.getLocalBounds().left
		+ confirmText.getLocalBounds().width / 2, 0);
	size = Vector2f(max(confirmText.getGlobalBounds().width + 40.f, 500.f ), 300);
	SetPos(position);
}

SaveMenuDecisionPopup::SaveMenuDecisionPopup(MainMenu *mainMenu)
{
	size = Vector2f(500, 300);
	
	SetRectColor(popupBGQuad, Color::Black);

	optionText.setFont(mainMenu->arial);
	optionText.setCharacterSize(40);
	optionText.setFillColor(Color::White);
	optionText.setString("HELLO");

	yesText.setFont(mainMenu->arial);
	yesText.setCharacterSize(40);
	yesText.setFillColor(Color::White);
	yesText.setString("YES");
	yesText.setOrigin(yesText.getLocalBounds().left + yesText.getLocalBounds().width / 2,
		yesText.getLocalBounds().top + yesText.getLocalBounds().height / 2);

	noText.setFont(mainMenu->arial);
	noText.setCharacterSize(40);
	noText.setFillColor(Color::White);
	noText.setString("NO");
	noText.setOrigin(noText.getLocalBounds().left + noText.getLocalBounds().width / 2,
		noText.getLocalBounds().top + noText.getLocalBounds().height / 2);

	SetOption(OPTION_YES);
	//SetRectColor(yesSelectedQuad, Color::Red);
	//SetRectColor(noSelectedQuad, Color::Blue);

	//currentlySelectedOption = OPTION_YES;

	SetPos(Vector2f(960, 540));
}

void SaveMenuDecisionPopup::SetPos(sf::Vector2f &pos)
{
	position = pos;
	SetRectCenter(popupBGQuad, size.x, size.y, pos);

	Vector2f yesOffset = Vector2f(-100, 50);
	Vector2f noOffset = Vector2f(100, 50);

	SetRectCenter(yesSelectedQuad, 128, 128, pos + yesOffset);
	yesText.setPosition(pos + yesOffset);
	SetRectCenter(noSelectedQuad, 128, 128, pos + Vector2f(100, 50));
	noText.setPosition(pos + noOffset);


	optionText.setPosition(Vector2f(pos.x, (pos.y - size.y / 2) + 10));
}

int SaveMenuDecisionPopup::Update(ControllerState &currInput,
	ControllerState &prevInput)
{
	if (currInput.LLeft())
	{
		SetOption(OPTION_YES);
	}
	else if (currInput.LRight())
	{
		SetOption(OPTION_NO);
	}


	if (currInput.A && !prevInput.A)
	{
		return currentlySelectedOption;
	}
	else if (currInput.B && !prevInput.B)
	{
		return OPTION_BACK;
	}

	
	

	return OPTION_NOTHING;
}

void SaveMenuDecisionPopup::SetOption(int op)
{
	Color yesSelectedColor = Color( 29, 208, 9 );
	Color noSelectedColor = Color::Red;
	Color unselectedColor = Color(93, 93, 93);//Color::Green;
	currentlySelectedOption = op;
	if (op == OPTION_YES)
	{
		SetRectColor( yesSelectedQuad, yesSelectedColor);
		SetRectColor( noSelectedQuad, unselectedColor);
	}
	else if (op == OPTION_NO)
	{
		SetRectColor(yesSelectedQuad, unselectedColor);
		SetRectColor(noSelectedQuad, noSelectedColor);
	}
}

void SaveMenuDecisionPopup::Draw(sf::RenderTarget *target)
{
	target->draw(popupBGQuad, 4, sf::Quads);
	target->draw(optionText);

	target->draw(yesSelectedQuad, 4, sf::Quads);
	target->draw(noSelectedQuad, 4, sf::Quads);
	target->draw(yesText);
	target->draw(noText);
	
}

void SaveMenuDecisionPopup::SetText(const std::string &str)
{
	optionText.setString(str);
	optionText.setOrigin(optionText.getLocalBounds().left
		+ optionText.getLocalBounds().width / 2, 0);
	size = Vector2f(max(optionText.getGlobalBounds().width + 40.f, 500.f), 300);
	SetPos(position);
}