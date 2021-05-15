#include "SaveMenuScreen.h"
#include "MainMenu.h"
#include "SaveFile.h"
#include "VectorMath.h"
#include "IntroMovie.h"
#include "WorldMap.h"
#include "Fader.h"
#include "SkinMenu.h"

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

void SaveFileDisplay::SetPosition(Vector2f &pos)
{
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

void SaveFileDisplay::SetValues(SaveFile *sf, WorldMap *wm)
{
	if (sf != NULL)
	{
		stringstream ss;

		int totalWorlds = wm->planet->numWorlds;
		int numCompleteWorlds = sf->GetNumCompleteWorlds(wm->planet);

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

SaveMenuScreen::SaveMenuScreen(MainMenu *p_mainMenu)
	:mainMenu(p_mainMenu),
	playerSkinShader( "player" ),
	maskPlayerSkinShader( "player" ),
	confirmPopup( p_mainMenu ), infoPopup( p_mainMenu )
{
	menuOffset = Vector2f(0, 0);

	//TilesetManager &tsMan = mainMenu->tilesetManager;
	selectedSaveIndex = 0;
	skinMenu = new SkinMenu(this);
	
	frame = 0;
	kinFaceTurnLength = 15;
	selectedSaveIndex = 0;
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
	actionLength[FADEIN] = 30;

	ts_skinButton->SetSpriteTexture(skinButtonSpr);
	ts_skinButton->SetSubRect(skinButtonSpr, 0);
	skinButtonSpr.setPosition(1542, 0);

	mainMenu->ts_buttonIcons->SetSpriteTexture(skinButtonIconSpr);

	ControllerType ct = mainMenu->GetController(0).GetCType();
	int tileOffset = 0;
	switch (ct)
	{
	case ControllerType::CTYPE_XBOX:
		tileOffset = 0;
		break;
	case ControllerType::CTYPE_GAMECUBE:
		tileOffset = 16 * 2;
		break;
	}

	int buttonIndex = 4;

	mainMenu->ts_buttonIcons->SetSubRect(skinButtonIconSpr, tileOffset + buttonIndex);
	skinButtonIconSpr.setScale(.5, .5);

	Vector2f buttonOffset(192, 2);

	skinButtonIconSpr.setPosition(skinButtonSpr.getPosition() + buttonOffset);

	AdventureFile *af = &mainMenu->worldMap->adventureFile;

	string currName;
	if (mainMenu->currSaveFile != NULL)
	{
		currName = mainMenu->currSaveFile->name;
	}

	

	std::vector<string> saveNames = { "blue", "green", "yellow", "orange", "red", "magenta" };
	for (int i = 0; i < 6; ++i)
	{
		files[i] = new SaveFile(saveNames[i], af);
		/*if (saveNames[i] == currName)
		{
			files[i] = mainMenu->currSaveFile;
		}
		else
		{
			files[i] = new SaveFile(saveNames[i], af);
		}*/
	}

	mainMenu->currSaveFile = files[selectedSaveIndex];
	/*files[0] = new SaveFile("blue",af );
	files[1] = new SaveFile("green", af);
	files[2] = new SaveFile("yellow", af);
	files[3] = new SaveFile("orange", af);
	files[4] = new SaveFile("red", af);
	files[5] = new SaveFile("magenta", af);*/



	for (int i = 0; i < 6; ++i)
	{
		defaultFiles[i] = false;
		fileDisplay[i] = new SaveFileDisplay(mainMenu->arial);
		fileDisplay[i]->SetPosition(GetTopLeftSaveSlot(i));
	}

	for (int i = 0; i < 6; ++i)
	{
		defaultFiles[i] = !(files[i]->Load());
		/*if (i == 0 && defaultFiles[i] == true)
		{
			assert(0);
		}*/
		if( !defaultFiles[i] )
			fileDisplay[i]->SetValues(files[i], mainMenu->worldMap);
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

	selectSlot.setTextureRect(ts_selectSlot->GetSubRect(selectedSaveIndex));
	//kinFace.setTextureRect(ts_kinFace->GetSubRect(0));

	Vector2f topLeftPos = GetTopLeftSaveSlot(selectedSaveIndex);

	selectSlot.setPosition(topLeftPos);
	//kinFace.setPosition(topLeftPos + Vector2f( 50, 0 ));


}

SaveMenuScreen::~SaveMenuScreen()
{
	delete skinMenu;
	for (int i = 0; i < 6; ++i)
	{
		if (files[i] != mainMenu->currSaveFile)
		{
			delete files[i];
		}
		delete fileDisplay[i];
	}
}

void SaveMenuScreen::SaveSelectedFile()
{
	mainMenu->currSaveFile->Save();
}

void SaveMenuScreen::SetSkin(int index)
{
	currSkin = index;
	playerSkinShader.SetSkin(currSkin);
	maskPlayerSkinShader.SetSkin(currSkin);
}

void SaveMenuScreen::SaveCurrSkin()
{
	mainMenu->currSaveFile->defaultSkinIndex = currSkin;
	SaveSelectedFile();
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
			if (defaultFiles[selectedSaveIndex])
			{
				/*if (selectedSaveIndex == 0)
				{
					assert(0);
				}*/
				defaultFiles[selectedSaveIndex] = false;
				int savedSkin = files[selectedSaveIndex]->defaultSkinIndex;
				files[selectedSaveIndex]->SetAsDefault();
				files[selectedSaveIndex]->defaultSkinIndex = savedSkin;
				files[selectedSaveIndex]->Save();
				mainMenu->worldMap->InitSelectors();
				mainMenu->worldMap->SetDefaultSelections();
				//action = TRANSITIONMOVIE;
				action = TRANSITION;
			}
			else
			{
				action = TRANSITION;
			}

			mainMenu->worldMap->UpdateWorldStats();

			transparency = 0;
			fadeOut = 0;
			frame = 0;
			break;
		case TRANSITION:
		{

			mainMenu->SetMode(MainMenu::Mode::TRANS_SAVE_TO_WORLDMAP);
			mainMenu->transAlpha = 255;
			mainMenu->worldMap->state = WorldMap::PLANET;//WorldMap::PLANET_AND_SPACE;
			mainMenu->worldMap->frame = 0;
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundBuffers[MainMenu::S_SELECT]);

			mainMenu->worldMap->InitSelectors();
			return true;
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
		case COPY:
		{
			frame = 0;
			break;
		}
		}
	}



	ControllerState &menuCurrInput = mainMenu->menuCurrInput;
	ControllerState &menuPrevInput = mainMenu->menuPrevInput;


	bool moveDown = false;
	bool moveUp = false;
	bool moveLeft = false;
	bool moveRight = false;


	bool down = (menuCurrInput.LDown() && !menuPrevInput.LDown())
		|| (menuCurrInput.PDown() && !menuPrevInput.PDown());
	bool left = (menuCurrInput.LLeft() && !menuPrevInput.LLeft())
		|| (menuCurrInput.PLeft() && !menuPrevInput.PLeft());
	bool up = (menuCurrInput.LUp() && !menuPrevInput.LUp())
		|| (menuCurrInput.PUp() && !menuPrevInput.PUp());
	bool right = (menuCurrInput.LRight() && !menuPrevInput.LRight())
		|| (menuCurrInput.PRight() && !menuPrevInput.PRight());

	int moveDelayFrames = 60;
	int moveDelayFramesSmall = 40;

	bool changedToSkin = false;

	if (mainMenu->menuMode == MainMenu::SAVEMENU )
	{
		if( action == WAIT )
		{
			if (menuCurrInput.B && !menuPrevInput.B)
			{
				mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("main_menu_back"));
				return false;
				/*mainMenu->LoadMode(SAVEMENU);
				mainMenu->SetMode(MainMenu::TRANS_SAVE_TO_MAIN);
				mainMenu->fader->CrossFade(30, 0, 30, Color::Black);*/
			}
			else if (menuCurrInput.A && !menuPrevInput.A)
			{
				action = SELECT;
				frame = 0;
				mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("save_Select"));
				return true;
			}
			else if (menuCurrInput.rightShoulder && !menuPrevInput.rightShoulder)
			{
				action = SKINMENU;
				frame = 0;
				skinMenu->SetSelectedIndex(mainMenu->currSaveFile->defaultSkinIndex);
				changedToSkin = true; //so you dont exit the same frame you open
				
			}
			else if (menuCurrInput.X && !menuPrevInput.X)
			{
				if (!defaultFiles[selectedSaveIndex])
				{
					action = CONFIRMDELETE;
					frame = 0;
				}
			}
			else if (menuCurrInput.Y && !menuPrevInput.Y)
			{
				if (!defaultFiles[selectedSaveIndex])
				{
					action = COPY;
					frame = 0;
					copiedIndex = selectedSaveIndex;
				}
			}
			else
			{
				ChangeIndex(down, up, left, right);
			}
		}
		else if (action == CONFIRMDELETE)
		{
			int res = confirmPopup.Update(menuCurrInput, menuPrevInput);
			if ( res == SaveMenuConfirmPopup::OPTION_CONFIRM)
			{
				action = INFOPOP;
				frame = 0;
				infoPopup.SetText("Deleted save file");

				defaultFiles[selectedSaveIndex] = true;
				files[selectedSaveIndex]->Delete();
				fileDisplay[selectedSaveIndex]->SetValues(NULL, NULL);
				
			}
			else if( res == SaveMenuConfirmPopup::OPTION_BACK)
			{
				action = WAIT;
				frame = 0;
			}
		}
		else if (action == INFOPOP)
		{
			bool res = infoPopup.Update(menuCurrInput, menuPrevInput);
			if (res)
			{
				action = WAIT;
				frame = 0;
			}
		}
		else if (action == COPY)
		{
			if (menuCurrInput.A && !menuPrevInput.A)
			{
				if (defaultFiles[selectedSaveIndex])
				{
					action = CONFIRMCOPY;
					frame = 0;
				}
				else
				{
					action = INFOPOP;
					frame = 0;
					infoPopup.SetText("Cannot overwrite existing file");
				}
			}
			else if (menuCurrInput.B && !menuPrevInput.B)
			{
				action = WAIT;
				frame = 0;
			}
			else
			{
				ChangeIndex(down, up, left, right);
			}
		}
		else if (action == CONFIRMCOPY)
		{
			int res = confirmPopup.Update(menuCurrInput, menuPrevInput);
			if (res == SaveMenuConfirmPopup::OPTION_CONFIRM)
			{
				action = INFOPOP;
				frame = 0;
				infoPopup.SetText("Copied save file successfully");

				files[copiedIndex]->CopyTo(files[selectedSaveIndex]);
				files[selectedSaveIndex]->Save();
				defaultFiles[selectedSaveIndex] = false;
				fileDisplay[selectedSaveIndex]->SetValues(files[selectedSaveIndex], mainMenu->worldMap);
			}
			else if (res == SaveMenuConfirmPopup::OPTION_BACK)
			{
				action = WAIT;
				frame = 0;
			}
		}
	}

	if (mainMenu->currInputUnfiltered[0].rightShoulder)
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
			if (!skinMenu->Update(menuCurrInput, menuPrevInput))
			{
				action = WAIT;
				frame = 0;
				SetSkin(mainMenu->currSaveFile->defaultSkinIndex);
			}
		}
		break;
	}
	}

	selectSlot.setTextureRect(ts_selectSlot->GetSubRect(selectedSaveIndex));
	//kinFace.setTextureRect(ts_kinFace->GetSubRect(0));

	Vector2f topLeftPos = GetTopLeftSaveSlot(selectedSaveIndex);

	selectSlot.setPosition(topLeftPos);
	kinFace.setPosition(topLeftPos + Vector2f( 15, -6 ));

	

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

void SaveMenuScreen::SelectedIndexChanged()
{
	mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("save_change"));
	mainMenu->currSaveFile = files[selectedSaveIndex];

	if (action != COPY)
	{
		SetSkin(mainMenu->currSaveFile->defaultSkinIndex);
	}

	int currColonyIndex = 0;
	if (defaultFiles[selectedSaveIndex])
	{
		currColonyIndex = 0;
	}
	else
	{
		currColonyIndex = mainMenu->currSaveFile->mostRecentWorldSelected;
	}

	mainMenu->worldMap->selectedColony = currColonyIndex;
	mainMenu->worldMap->UpdateColonySelect();
}

void SaveMenuScreen::UnlockSkin(int skinIndex)
{
	mainMenu->UnlockSkin(skinIndex);
}

bool SaveMenuScreen::IsSkinUnlocked(int skinIndex)
{
	return mainMenu->IsSkinUnlocked(skinIndex);
}

void SaveMenuScreen::ChangeIndex(bool down, bool up, bool left, bool right)
{
	if (down)
	{
		selectedSaveIndex += 2;
		//currentMenuSelect++;
		if (selectedSaveIndex > 5)
			selectedSaveIndex -= 6;
	
		//mainMenu->soundNodeList->ActivateSound(mainMenu->soundBuffers[MainMenu::S_DOWN]);
		SelectedIndexChanged();

	}
	else if (up)
	{
		selectedSaveIndex -= 2;
		if (selectedSaveIndex < 0)
			selectedSaveIndex += 6;
		
		//mainMenu->soundNodeList->ActivateSound(mainMenu->soundBuffers[MainMenu::S_UP]);
		SelectedIndexChanged();
	}

	if (right)
	{
		selectedSaveIndex++;
		//currentMenuSelect++;
		if (selectedSaveIndex % 2 == 0)
			selectedSaveIndex -= 2;

		SelectedIndexChanged();
	}
	else if (left)
	{
		selectedSaveIndex--;
		if (selectedSaveIndex % 2 == 1)
			selectedSaveIndex += 2;
		else if (selectedSaveIndex < 0)
		{
			selectedSaveIndex += 2;
		}
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
	if (action == WAIT || (action == SELECT && frame < endDraw ) || action == FADEIN || action == SKINMENU 
		|| action == CONFIRMDELETE || action == CONFIRMCOPY || action == COPY || action == INFOPOP )
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
	
	saveTexture->display();
	const Texture &saveTex = saveTexture->getTexture();
	sf::Sprite saveSpr(saveTex);

	saveSpr.setColor(Color(255, 255, 255, 255 * (1.f - transparency)));
	
	target->draw(saveSpr);

	target->draw(skinButtonSpr);
	target->draw(skinButtonIconSpr);

	if (action == SKINMENU)
	{
		skinMenu->Draw(target);
	}
	else if (action == CONFIRMDELETE || action == CONFIRMCOPY)
	{
		confirmPopup.Draw(target);
	}
	else if (action == INFOPOP)
	{
		infoPopup.Draw(target);
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
			fileDisplay[i]->SetValues(files[i], mainMenu->worldMap);
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
	size = Vector2f(500, 200);
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
	SetRectCenter(popupBGQuad, size.x, size.y, pos);
	SetRectCenter(buttonQuads, 128, 128, pos + Vector2f(-100, 100));
	SetRectCenter(buttonQuads+4, 128, 128, pos + Vector2f(100, 100));
	confirmText.setPosition(pos);
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
}

SaveMenuInfoPopup::SaveMenuInfoPopup(MainMenu *mainMenu)
{
	size = Vector2f(300, 100);
	SetRectColor(popupBGQuad, Color::Black);

	text.setFont(mainMenu->arial);
	text.setCharacterSize(40);
	text.setFillColor(Color::White);
	text.setString("HELLO");

	SetPos(Vector2f(960, 540));
}

bool SaveMenuInfoPopup::Update(ControllerState &currInput,
	ControllerState &prevInput)
{
	if( (currInput.A && !prevInput.A)
		|| (currInput.B && !prevInput.B)
		|| (currInput.Y && !prevInput.Y)
		|| (currInput.X && !prevInput.X)
		|| (currInput.rightShoulder && !prevInput.rightShoulder) )
	{
		return true;
	}

	return false;
}

void SaveMenuInfoPopup::SetPos(sf::Vector2f &pos)
{
	SetRectCenter(popupBGQuad, size.x, size.y, pos);
	text.setPosition(pos);
}

void SaveMenuInfoPopup::SetText(const std::string &str)
{
	text.setString(str);
}

void SaveMenuInfoPopup::Draw(sf::RenderTarget *target)
{
	target->draw(popupBGQuad, 4, sf::Quads);
	target->draw(text);
}