#include "SaveMenuScreen.h"
#include "MainMenu.h"
#include "SaveFile.h"
#include "VectorMath.h"
#include "IntroMovie.h"

using namespace sf;
using namespace std;

SaveFileDisplay::SaveFileDisplay(sf::Font &f)
{
	completedShards.setFont(f);
	totalTime.setFont(f);
	totalPercentage.setFont(f);

	completedShards.setCharacterSize(60);
	totalTime.setCharacterSize(60);
	totalPercentage.setCharacterSize(60);

	Color fillColor = Color::White;
	Color lineColor = Color::Black;

	completedShards.setFillColor(fillColor);
	completedShards.setOutlineColor(lineColor);
	completedShards.setOutlineThickness(3);
	totalTime.setFillColor(fillColor);
	totalTime.setOutlineColor(lineColor);
	totalTime.setOutlineThickness(3);
	totalPercentage.setFillColor(fillColor);
	totalPercentage.setOutlineColor(lineColor);
	totalPercentage.setOutlineThickness(3);
}

void SaveFileDisplay::SetPosition(Vector2f &pos)
{
	Vector2f innerStart = pos + Vector2f(280, 30);

	totalPercentage.setPosition(innerStart);
	//totalTime.setPosition(innerStart + Vector2f(100, 0));
	completedShards.setPosition(innerStart + Vector2f(0, 80));
}

void SaveFileDisplay::Draw(sf::RenderTarget *target)
{
	target->draw(completedShards);
	target->draw(totalTime);
	target->draw(totalPercentage);
}

void SaveFileDisplay::SetValues(SaveFile *sf)
{
	stringstream ss;
	ss << sf->GetShardNum() << " / " << sf->GetTotalShardNum() << " Shards";
	completedShards.setString(ss.str());
	ss.str("");
	ss << sf->GetCompletionPercentage() << "%";
	totalPercentage.setString(ss.str());
}

SaveMenuScreen::SaveMenuScreen(MainMenu *p_mainMenu)
	:mainMenu(p_mainMenu)
{
	menuOffset = Vector2f(0, 0);

	TilesetManager &tsMan = mainMenu->tilesetManager;
	selectedSaveIndex = 0;
	frame = 0;
	kinFaceTurnLength = 15;
	selectedSaveIndex = 0;
	cloudLoopLength = 8;
	cloudLoopFactor = 5;

	saveJumpFactor = 5;
	saveJumpLength = 6;

	ts_background = tsMan.GetTileset("Menu/save_bg_1920x1080.png", 1920, 1080);
	ts_kinFace = tsMan.GetTileset("Menu/save_menu_kin_256x256.png", 256, 256);
	ts_selectSlot = tsMan.GetTileset("Menu/save_select_710x270.png", 710, 270);
	
	actionLength[WAIT] = 1;
	actionLength[SELECT] = 12 * 3 + 6 * 4 + 20;
	actionLength[TRANSITION] = 30;
	actionLength[TRANSITIONMOVIE] = 30;
	actionLength[FADEIN] = 30;

	files[0] = new SaveFile("blue");
	files[1] = new SaveFile("green");
	files[2] = new SaveFile("yellow");
	files[3] = new SaveFile("orange");
	files[4] = new SaveFile("red");
	files[5] = new SaveFile("magenta");



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
		fileDisplay[i]->SetValues(files[i]);
	}

	background.setTexture(*ts_background->texture);
	background.setPosition(menuOffset);


	kinFace.setTexture(*ts_kinFace->texture);
	kinFace.setTextureRect(ts_kinFace->GetSubRect(0));
	selectSlot.setTexture(*ts_selectSlot->texture);
	selectSlot.setTextureRect(ts_selectSlot->GetSubRect(0));

	ts_kinJump1 = tsMan.GetTileset("Menu/save_kin_jump1_500x1080.png", 500, 1080);
	ts_kinJump2 = tsMan.GetTileset("Menu/save_kin_jump2_500x1080.png", 500, 1080);
	ts_kinClouds = tsMan.GetTileset("Menu/save_kin_clouds_500x416.png", 500, 416);
	ts_kinWindow = tsMan.GetTileset("Menu/save_kin_window_500x1080.png", 500, 1080);
	//ts_saveKinWindow = tilesetManager.GetTileset( "Menu/save_kin_window_500x1080.png", 500, 1080 );
	ts_kinSky = tsMan.GetTileset("Menu/save_menu_sky_01_500x1080.png", 500, 1080);

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

	ts_starBackground = tsMan.GetTileset("WorldMap/map_z1_stars.png", 1920, 1080);
	starBackground.setTexture(*ts_starBackground->texture);
	starBackground.setPosition(menuOffset);

	ts_world = tsMan.GetTileset("WorldMap/map_z1_world.png", 1120, 1080);
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
	kinFace.setPosition(topLeftPos);


}

void SaveMenuScreen::Update()
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
				if (selectedSaveIndex == 0)
				{
					assert(0);
				}
				defaultFiles[selectedSaveIndex] = false;
				files[selectedSaveIndex]->CopyFromDefault();
				action = TRANSITIONMOVIE;
			}
			else
			{
				action = TRANSITION;
			}
			
			transparency = 0;
			fadeOut = 0;
			frame = 0;
			break;
		case TRANSITION:
		{
			mainMenu->menuMode = MainMenu::Mode::TRANS_SAVE_TO_WORLDMAP;
			mainMenu->transAlpha = 255;
			mainMenu->worldMap->state = WorldMap::PLANET;//WorldMap::PLANET_AND_SPACE;
			mainMenu->worldMap->frame = 0;
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundBuffers[MainMenu::S_SELECT]);
			return;
			break;
		}
		case TRANSITIONMOVIE:
		{
			mainMenu->PlayIntroMovie();
			return;
			break;
		}
		case FADEIN:
		{
			action = WAIT;
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


	int moveDelayFrames = 15;
	int moveDelayFramesSmall = 6;

	if (mainMenu->menuMode == MainMenu::SAVEMENU && action == WAIT )
	{
		if (menuCurrInput.B && !menuPrevInput.B )
		{
			mainMenu->SetMode(MainMenu::TRANS_SAVE_TO_MAIN);
			return;
		}
		else if (menuCurrInput.A && !menuPrevInput.A )
		{
			action = SELECT;
			frame = 0;
			return;
		}

		bool canMoveOther = ((moveDelayCounter - moveDelayFramesSmall) <= 0);
		bool canMoveSame = (moveDelayCounter == 0);
		if ((menuCurrInput.LDown() || menuCurrInput.PDown()) && (
			(!moveDown && canMoveOther) || (moveDown && canMoveSame)))
		{
			selectedSaveIndex += 2;
			//currentMenuSelect++;
			if (selectedSaveIndex > 5)
				selectedSaveIndex -= 6;
			moveDown = true;
			moveDelayCounter = moveDelayFrames;
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundBuffers[MainMenu::S_DOWN]);
		}
		else if ((menuCurrInput.LUp() || menuCurrInput.PUp()) && (
			(!moveUp && canMoveOther) || (moveUp && canMoveSame)))
		{
			selectedSaveIndex -= 2;
			if (selectedSaveIndex < 0)
				selectedSaveIndex += 6;
			moveUp = true;
			moveDelayCounter = moveDelayFrames;
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundBuffers[MainMenu::S_UP]);
		}

		if ((menuCurrInput.LRight() || menuCurrInput.PRight()) && (
			(!moveRight && canMoveOther) || (moveRight && canMoveSame)))
		{
			selectedSaveIndex++;
			//currentMenuSelect++;
			if (selectedSaveIndex % 2 == 0)
				selectedSaveIndex -= 2;
			moveRight = true;
			moveDelayCounter = moveDelayFrames;
		}
		else if ((menuCurrInput.LLeft() || menuCurrInput.PLeft()) && (
			(!moveLeft && canMoveOther) || (moveLeft && canMoveSame)))
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
		}

		if (moveDelayCounter > 0)
		{
			moveDelayCounter--;
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

		if (frame > 12 * 3)
		{
			int jFrame = (frame - 12 * 3);

			if (jFrame == 0)
			{
				kinJump.setTexture(*ts_kinJump1->texture);
			}
			else if (jFrame == 4 * 3)
			{
				kinJump.setTexture(*ts_kinJump2->texture);
			}

			if (jFrame < 3 * 4)
			{
				kinJump.setTextureRect(ts_kinJump1->GetSubRect(jFrame / 4));
			}
			else if( jFrame < 6 * 4 )
			{
				kinJump.setTextureRect(ts_kinJump2->GetSubRect(jFrame / 4 - 3));
			}
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
	}

	selectSlot.setTextureRect(ts_selectSlot->GetSubRect(selectedSaveIndex));
	//kinFace.setTextureRect(ts_kinFace->GetSubRect(0));

	Vector2f topLeftPos = GetTopLeftSaveSlot(selectedSaveIndex);

	selectSlot.setPosition(topLeftPos);
	kinFace.setPosition(topLeftPos);

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
}

Vector2f SaveMenuScreen::GetTopLeftSaveSlot(int index)
{
	Vector2f topLeftPos;
	topLeftPos.x += ts_selectSlot->tileWidth * (index % 2);
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

	int endDraw = 12 * 3 + 6 * 4;
	if (action == WAIT || (action == SELECT && frame < endDraw ) || action == FADEIN )
	{
		saveTexture->draw(kinJump);
	}
	
	saveTexture->draw(selectSlot);
	saveTexture->draw(kinFace);

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
	
}

void SaveMenuScreen::Reset()
{
	//doesnt reset the selected save index
	fadeOut = 0;
	transparency = 0;
	action = WAIT;
	frame = 0;

	for (int i = 0; i < 6; ++i)
	{
		fileDisplay[i]->SetValues(files[i]);
	}
	
	asteroidFrameBack = 0;
	asteroidFrameFront = 0;
	moveDelayCounter = 0;

	kinJump.setTexture(*ts_kinJump1->texture);
	kinJump.setTextureRect(ts_kinJump1->GetSubRect(0));
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