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
	:mainMenu(p_mainMenu), playerSkinShader( "basicplayerskin" )
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
	
	actionLength[WAIT] = 1;
	actionLength[SELECT] = 12 * 3 + 24 * 2 + 20;
	actionLength[TRANSITION] = 30;
	actionLength[TRANSITIONMOVIE] = 30;
	actionLength[FADEIN] = 30;

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
	mainMenu->currSaveFile->defaultSkinIndex = index;
	SaveSelectedFile();
	playerSkinShader.SetSkin(index);
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
				files[selectedSaveIndex]->SetAsDefault();
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
		}
	}

	

	ControllerState &menuCurrInput = mainMenu->menuCurrInput;
	ControllerState &menuPrevInput = mainMenu->menuPrevInput;

	bool moveDown = false;
	bool moveUp = false;
	bool moveLeft = false;
	bool moveRight = false;


	int moveDelayFrames = 60;
	int moveDelayFramesSmall = 40;

	if (mainMenu->menuMode == MainMenu::SAVEMENU && action == WAIT )
	{
		if (menuCurrInput.B && !menuPrevInput.B )
		{
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("main_menu_back"));
			return false;
			/*mainMenu->LoadMode(SAVEMENU);
			mainMenu->SetMode(MainMenu::TRANS_SAVE_TO_MAIN);
			mainMenu->fader->CrossFade(30, 0, 30, Color::Black);*/
		}
		else if (menuCurrInput.A && !menuPrevInput.A )
		{
			action = SELECT;
			frame = 0;
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("save_Select"));
			return true;
		}
		else if (menuCurrInput.Y && !menuPrevInput.Y)
		{
			action = SKINMENU;
			frame = 0;
			skinMenu->SetSelectedIndex(mainMenu->currSaveFile->defaultSkinIndex);
		}

		//bool canMoveOther = ((moveDelayCounter - moveDelayFramesSmall) <= 0);
		//bool canMoveSame = (moveDelayCounter == 0);


		bool down = (menuCurrInput.LDown() && !menuPrevInput.LDown())
			|| ( menuCurrInput.PDown() && !menuPrevInput.PDown() );
		bool left = (menuCurrInput.LLeft() && !menuPrevInput.LLeft())
			|| (menuCurrInput.PLeft() && !menuPrevInput.PLeft());
		bool up = (menuCurrInput.LUp() && !menuPrevInput.LUp())
			|| (menuCurrInput.PUp() && !menuPrevInput.PUp());
		bool right = (menuCurrInput.LRight() && !menuPrevInput.LRight())
			|| (menuCurrInput.PRight() && !menuPrevInput.PRight());

		if (down )
		{
			selectedSaveIndex += 2;
			//currentMenuSelect++;
			if (selectedSaveIndex > 5)
				selectedSaveIndex -= 6;
			moveDown = true;
			moveDelayCounter = moveDelayFrames;
			//mainMenu->soundNodeList->ActivateSound(mainMenu->soundBuffers[MainMenu::S_DOWN]);
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("save_change"));
		}
		else if (up)
		{
			selectedSaveIndex -= 2;
			if (selectedSaveIndex < 0)
				selectedSaveIndex += 6;
			moveUp = true;
			moveDelayCounter = moveDelayFrames;
			//mainMenu->soundNodeList->ActivateSound(mainMenu->soundBuffers[MainMenu::S_UP]);
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("save_change"));
		}

		if (right)
		{
			selectedSaveIndex++;
			//currentMenuSelect++;
			if (selectedSaveIndex % 2 == 0)
				selectedSaveIndex -= 2;
			moveRight = true;
			moveDelayCounter = moveDelayFrames;
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("save_change"));
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
			moveLeft = true;

			moveDelayCounter = moveDelayFrames;
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("save_change"));
		}

		if (moveDelayCounter > 0)
		{
			//should have to press a direction each time to get the save file
			//clean up later?
			//moveDelayCounter--;
		}


		if (!(menuCurrInput.LDown() || menuCurrInput.PDown()))
		{
			moveDown = false;
		}
		if (!(menuCurrInput.LUp() || menuCurrInput.PUp()))
		{
			moveUp = false;
		}

		if (!(menuCurrInput.LRight() || menuCurrInput.PRight()))
		{
			moveRight = false;
		}
		if (!(menuCurrInput.LLeft() || menuCurrInput.PLeft()))
		{
			moveLeft = false;
		}
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
			kinFace.setTextureRect(ts_kinFace->GetSubRect(frame / 3));

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

			kinJump.setTextureRect(ts_kinJump[texIndex]->GetSubRect(rem / 2));
		}
		//if( kinFaceFrame == saveKinFaceTurnLength * 3 + 40 )
		//{
		//	menuMode = WORLDMAP;
		//	break;
		//	//kinFaceFrame = 0;
		//}

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
		if (!skinMenu->Update(menuCurrInput, menuPrevInput))
		{
			action = WAIT;
			frame = 0;
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

	mainMenu->currSaveFile = files[selectedSaveIndex];
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
	if (action == WAIT || (action == SELECT && frame < endDraw ) || action == FADEIN )
	{
		saveTexture->draw(kinJump, &playerSkinShader.pShader);
	}
	
	saveTexture->draw(selectSlot);
	saveTexture->draw(kinFace, &playerSkinShader.pShader);

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

	if (action == SKINMENU)
	{
		skinMenu->Draw(target);
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
	moveDelayCounter = 0;

	kinJump.setTexture(*ts_kinJump[0]->texture);
	kinJump.setTextureRect(ts_kinJump[0]->GetSubRect(0));
	kinJump.setOrigin(kinJump.getLocalBounds().width, 0);
	kinJump.setPosition(Vector2f(1920, 0) + menuOffset);

	kinFace.setTextureRect(ts_kinFace->GetSubRect(0));
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