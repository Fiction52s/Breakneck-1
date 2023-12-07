#include "WorldMap.h"

#include "GameSession.h"
#include <sstream>
#include <iostream>
#include "MainMenu.h"
#include "SaveFile.h"
#include "Enemy_Shard.h"
#include "SaveMenuScreen.h"

#include "Background.h"
#include "KinBoostScreen.h"
#include "LoadingBackpack.h"
#include "UIMouse.h"
#include "CustomCursor.h"
#include "WorldMapShip.h"
#include "AdventureManager.h"
#include "NetplayManager.h"
#include "Leaderboard.h"
#include "Config.h"

using namespace boost::filesystem;
using namespace sf;
using namespace std;

WorldMap::WorldMap()
	:font( MainMenu::GetInstance()->arial )
{
	mainMenu = MainMenu::GetInstance();
	adventureManager = mainMenu->adventureManager;

	adventureManager->parallelPracticeMode = mainMenu->config->GetData().parallelPlayOn;
	
	allUnlocked = true;

	UpdateButtonIconsWhenControllerIsChanged();

	ship = new WorldMapShip(this);

	worldSelector = new WorldSelector(mainMenu);

	SetRectColor(infoQuadBG, Color(0, 0, 0, 100));
	SetRectColor(infoNameBG, Color(0, 0, 0, 200));

	Vector2f infoQuadPos(1416, 292);
	SetRectTopLeft(infoQuadBG, 256, 192, infoQuadPos);
	SetRectTopLeft(infoNameBG, 256, 64, infoQuadPos + Vector2f( 0, -64 ));

	sectorsCompleteText.setFillColor(Color::White);
	sectorsCompleteText.setCharacterSize(36);
	sectorsCompleteText.setFont(mainMenu->arial);
	sectorsCompleteText.setPosition( infoQuadPos + Vector2f(10, 10));

	shardsCapturedText.setFillColor(Color::White);
	shardsCapturedText.setCharacterSize(36);
	shardsCapturedText.setFont(mainMenu->arial);
	shardsCapturedText.setPosition(infoQuadPos + Vector2f(10, 10 + 60));

	logsCapturedText.setFillColor(Color::White);
	logsCapturedText.setCharacterSize(36);
	logsCapturedText.setFont(mainMenu->arial);
	logsCapturedText.setPosition(infoQuadPos + Vector2f(10, 10 + 60 + 60));
	

	worldNameText.setFillColor(Color::White);
	worldNameText.setCharacterSize(40);
	worldNameText.setFont(mainMenu->arial);
	worldNameText.setPosition(infoQuadPos + Vector2f(256/2, 10) + Vector2f( 0, -64 ));

	for (int i = 0; i < ADVENTURE_MAX_NUM_WORLDS; ++i)
	{
		numCurrentPlayersText[i].setFillColor(Color::White);
		numCurrentPlayersText[i].setCharacterSize(40);
		numCurrentPlayersText[i].setFont(mainMenu->arial);
	}

	
	ts_parallelPlayMarker = GetSizedTileset("Menu/parallel_play_384x128.png");
	ts_parallelPlayMarker->SetSpriteTexture(parallelPlayMarkerSpr);

	Vector2f parallelPlayMarkerPos(1450, 850);

	parallelPlayMarkerSpr.setPosition(parallelPlayMarkerPos);

	Vector2f buttonIconOffset(320, 2);
	SetRectTopLeft(parallelPlayButtonQuad, 64, 64, parallelPlayMarkerPos + buttonIconOffset);

	if (adventureManager->parallelPracticeMode)
	{
		ts_parallelPlayMarker->SetSubRect(parallelPlayMarkerSpr, 0);
	}
	else
	{
		ts_parallelPlayMarker->SetSubRect(parallelPlayMarkerSpr, 1);
	}
	
	ts_colonyActive[0] = GetTileset("Menu/WorldMap/w1_select.png", 1920, 1080);
	ts_colonyActive[1] = GetTileset("Menu/WorldMap/w1_select.png", 1920, 1080);
	ts_colonyActiveZoomed[0] = GetTileset("Menu/WorldMap/map_w1_vein.png", 1920, 1080);
	ts_colonyActiveZoomed[1] = GetTileset("Menu/WorldMap/map_w2_vein.png", 1920, 1080);

	ts_space = GetTileset("Menu/WorldMap/worldmap_bg.png", 1920, 1080);
	spaceSpr.setTexture(*ts_space->texture);
	
	ts_planet = GetTileset("Menu/WorldMap/worldmap.png", 1920, 1080); 
	planetSpr.setTexture(*ts_planet->texture);
	planetSpr.setOrigin(planetSpr.getLocalBounds().width / 2, planetSpr.getLocalBounds().height / 2);
	planetSpr.setPosition(960, 540);
	
	ts_colony[0] = GetTileset("Menu/WorldMap/map_w1.png", 1920, 1080);
	ts_colony[1] = GetTileset("Menu/WorldMap/map_w2.png", 1920, 1080);
	ts_colony[2] = GetTileset("Menu/WorldMap/map_w3.png", 1920, 1080);
	ts_colony[3] = GetTileset("Menu/WorldMap/map_w4.png", 1920, 1080);
	ts_colony[4] = GetTileset("Menu/WorldMap/map_w5.png", 1920, 1080);
	ts_colony[5] = GetTileset("Menu/WorldMap/map_w6.png", 1920, 1080);
	ts_colony[6] = GetTileset("Menu/WorldMap/map_w7.png", 1920, 1080);
	ts_colony[7] = GetTileset("Menu/WorldMap/map_w7.png", 1920, 1080);

	ts_selectableRing = GetSizedTileset("Menu/WorldMap/world_select_ring_192x192.png");
	
	SetupAsteroids();

	zoomView.setCenter(960, 540);
	zoomView.setSize(1920, 1080);

	colonySpr[0].setPosition(1087, 331);
	colonySpr[1].setPosition(1087, 614);
	colonySpr[2].setPosition(842, 756);
	colonySpr[3].setPosition(595, 614);
	colonySpr[4].setPosition(595, 331);
	colonySpr[5].setPosition(841, 189);
	colonySpr[6].setPosition(841, 473);
	colonySpr[7].setPosition(841, 600);

	for (int i = 0; i < 2; ++i)
	{
		SetRectSubRect(worldActiveQuads + i * 4, ts_colonyActive[i]->GetSubRect(0));
		SetRectCenter(worldActiveQuads + i * 4, 1920, 1080, Vector2f(960, 540));
		SetRectSubRect(worldActiveQuadsZoomed + i * 4, ts_colonyActiveZoomed[i]->GetSubRect(0));
		SetRectCenter(worldActiveQuadsZoomed + i * 4, 1920 / 8.f, 1080 / 8.f, Vector2f(colonySpr[i].getPosition() + Vector2f(960/8.f, 540/8.f)));
	}

	for (int i = 0; i < ADVENTURE_MAX_NUM_WORLDS; ++i)
	{
		colonySpr[i].setTexture(*ts_colony[i]->texture);
		colonySpr[i].setScale(1.f / 8.f, 1.f / 8.f);

		SetRectSubRectGL(worldSelectableQuads + i * 4, ts_selectableRing->GetSubRect(0), Vector2f(ts_selectableRing->texture->getSize()));

		numCurrentPlayersText[i].setPosition(colonySpr[i].getPosition() + Vector2f(960 / 8.f, 0) + Vector2f(0, -80));
	}

	colonyRadius = 192 / 2;

	colonySelectSprZoomed.setScale(1.f / 8.f, 1.f / 8.f );
	
	if (!zoomShader.loadFromFile("Resources/Shader/zoomblur.frag", sf::Shader::Fragment))
	{
		cout << "zoom blur SHADER NOT LOADING CORRECTLY" << endl;
		assert(0);
	}
	zoomShader.setUniform("texSize", Vector2f(1920, 1080));
	zoomShader.setUniform("radial_blur", 1.f);
	zoomShader.setUniform("radial_bright", .1f);
	zoomShader.setUniform("radial_origin", Vector2f( .5, .5 ) );
	zoomShader.setUniform("radial_size", Vector2f( 1.f / 1920, 1.f / 1080 ));

	if (!selectableRingShader.loadFromFile("Resources/Shader/colonyselectable.frag", sf::Shader::Fragment ))
	{
		cout << "selectable ring shader not loading correctly" << endl;
		assert(0);
	}
	selectableRingShader.setUniform("u_texture", *ts_selectableRing->texture);
	selectableRingShader.setUniform("prop", 0.f);

	int width = 1920;
	int height = 1080;
	uiView = View(sf::Vector2f(width / 2, height / 2), sf::Vector2f(width, height));

	fontHeight = 24;
	menuPos = Vector2f( 300, 300 );
	yspacing = 40;
	entries = NULL;
	text = NULL;
	dirNode = NULL;
	localPaths = NULL;
	leftBorder = 20;

	currLevelTimeText.setFillColor(Color::White);
	currLevelTimeText.setCharacterSize(40);
	currLevelTimeText.setFont(mainMenu->arial);
	currLevelTimeText.setOrigin(currLevelTimeText.getLocalBounds().left + currLevelTimeText.getLocalBounds().width / 2, 0);
	currLevelTimeText.setPosition(1300, 200);
	currLevelTimeText.setString("");

	Reset(NULL);

	int numWorlds = adventureManager->adventurePlanet->numWorlds;
	selectors = new MapSelector*[numWorlds];

	for (int i = 0; i < numWorlds; ++i)
	{
		selectors[i] = new MapSelector(this, &(adventureManager->adventurePlanet->worlds[i]),
			mainMenu, Vector2f(960, 540));
	}
}

WorldMap::~WorldMap()
{
	delete worldSelector;
	for (int i = 0; i < adventureManager->adventurePlanet->numWorlds; ++i)
	{
		delete selectors[i];
	}
	delete[] selectors;

	delete ship;

	ClearEntries();
}

void WorldMap::SetupAsteroids()
{
	ts_asteroids[0] = GetTileset("Menu/WorldMap/asteroid_1_1920x1080.png", 1920, 1080);
	ts_asteroids[1] = GetTileset("Menu/WorldMap/asteroid_2_1920x1080.png", 1920, 1080);
	ts_asteroids[2] = GetTileset("Menu/WorldMap/asteroid_3_1920x1080.png", 1920, 1080);
	ts_asteroids[3] = GetTileset("Menu/WorldMap/asteroid_4_1920x1080.png", 1920, 1080);

	for (int i = 0; i < 4; ++i)
	{
		IntRect ir = ts_asteroids[i]->GetSubRect(0);
		ir.width *= 3;
		SetRectSubRect(asteroidQuads + i * 4, ir);
		SetRectCenter(asteroidQuads + i * 4, ir.width, ir.height, Vector2f(960, 540));
	}

	if (!asteroidShader.loadFromFile("Resources/Shader/menuasteroid.frag", sf::Shader::Fragment))
	{
		cout << "asteroid SHADER NOT LOADING CORRECTLY" << endl;
		assert(0);
	}
	asteroidShader.setUniform("u_texture", sf::Shader::CurrentTexture);
}



void WorldMap::UpdateWorldStats()
{
	assert(selectedColony >= 0);

	int sel = selectedColony + 1;

	worldNameText.setString("World " + to_string(sel));

	FloatRect worldNameTextLocalBounds = worldNameText.getLocalBounds();
	worldNameText.setOrigin(worldNameTextLocalBounds.width / 2
		+ worldNameTextLocalBounds.left, 0);

	SaveFile *saveFile = adventureManager->currSaveFile;

	AdventureWorld &aw = adventureManager->adventureFile.GetWorld(selectedColony);

	World &world = adventureManager->adventurePlanet->worlds[selectedColony];
	int numTotalSectors = world.numSectors;
	int numCompletedSectors = 0;
	for (int i = 0; i < numTotalSectors; ++i)
	{
		if (saveFile->IsCompleteSector(&world.sectors[i]))
		{
			++numCompletedSectors;
		}
	}

	stringstream ss;

	ss << "Sectors: " << numCompletedSectors << "/" << numTotalSectors;

	sectorsCompleteText.setString(ss.str());

	int numTotalShards = 0;
	int totalShardsCaptured = 0;
	int numLevels = 0;
	int levIndex = 0;
	int numShardsInLevel = 0;

	int numTotalLogs = 0;
	int totalLogsCaptured = 0;
	int numLogsInLevel = 0;

	for (int i = 0; i < numTotalSectors; ++i)
	{
		numLevels = world.sectors[i].numLevels;
		for (int j = 0; j < numLevels; ++j)
		{
			levIndex = world.sectors[i].GetLevelIndex(j);
			AdventureMapHeaderInfo &amhi = adventureManager->adventureFile.GetMapHeaderInfo(levIndex);

			numShardsInLevel = amhi.shardInfoVec.size();
			numTotalShards += numShardsInLevel;
			for (int k = 0; k < numShardsInLevel; ++k)
			{
				if (saveFile->IsShardCaptured(amhi.shardInfoVec[k].GetTrueIndex()))
				{
					++totalShardsCaptured;
				}
			}

			numLogsInLevel = amhi.logInfoVec.size();
			numTotalLogs += numLogsInLevel;
			for (int k = 0; k < numLogsInLevel; ++k)
			{
				if (saveFile->HasLog(amhi.logInfoVec[k].GetTrueIndex()))
				{
					++totalLogsCaptured;
				}
			}

		}
	}

	ss.str("");
	ss.clear();

	ss << "Shards: " << totalShardsCaptured << "/" << numTotalShards;

	shardsCapturedText.setString(ss.str());

	ss.str("");
	ss.clear();

	ss << "Logs: " << totalLogsCaptured << "/" << numTotalLogs;

	logsCapturedText.setString(ss.str());
}

void WorldMap::UpdateColonySelect()
{
	if (selectedColony >= 0)
	{
		worldSelector->SetPosition(Vector2f(colonySpr[selectedColony].getPosition() + Vector2f(960 / 8.f, 540 / 8.f)));
	}	
}

bool WorldMap::IsInAllUnlockedMode()
{
	return allUnlocked;
}

void WorldMap::RunSelectedMap()
{
	MapSelector *ms = CurrSelector();
	ms->RunSelectedMap();

}

void WorldMap::Reset( SaveFile *sf )
{
	fontHeight = 24;
	state = PLANET_VISUAL_ONLY;
	frame = 0;
	asteroidFrame = 0;
	selectedColony = -1;
	selectedLevel = 0;

	ClearEntries();
	moveDown = false;
	moveUp = false;

	UpdateColonySelect();
}

const std::string & WorldMap::GetSelected()
{
	return localPaths[selectedLevel];
}



void WorldMap::ClearEntries()
{
	numTotalEntries = 0;
	selectedLevel = 0;
	//mouseOverIndex = -1;
	numTotalEntries = 0;
	if( entries != NULL )
	{
		delete entries;
		//ClearEntries( entries );
		entries = NULL;
	}
	if( text != NULL )
	{
		delete [] text;
	}

	if( localPaths != NULL )
	{
		delete [] localPaths;
	}

	if (dirNode != NULL)
	{

		delete[] dirNode;
	}
		
}

MapSelector *WorldMap::CurrSelector()
{
	return selectors[selectedColony];
}

void WorldMap::SetDefaultSelections()
{
	selectedColony = -1;
}

void WorldMap::InitSelectors()
{
	for (int i = 0; i < adventureManager->adventurePlanet->numWorlds; ++i)
	{
		selectors[i]->Init();
		//selectors[i]->UpdateAllInfo(i);
	}
}

void WorldMap::UpdateSelectedColony()
{
	Vector2f shipPos = ship->GetPosition();
	Vector2f colMiddle;

	int numCompletedWorlds;
	int numUnlockedWorlds = -1;
	if (allUnlocked)
	{
		numCompletedWorlds = adventureManager->adventurePlanet->numWorlds;
	}
	else
	{
		SaveFile *saveFile = adventureManager->currSaveFile;
		numCompletedWorlds = saveFile->GetNumCompleteWorlds(adventureManager->adventurePlanet);
	}

	if (numCompletedWorlds == adventureManager->adventurePlanet->numWorlds)
	{
		numUnlockedWorlds = numCompletedWorlds;
	}
	else
	{
		numUnlockedWorlds = numCompletedWorlds + 1;
	}

	//for (int i = 0; i < adventureManager->adventurePlanet->numWorlds; ++i)
	for (int i = 0; i < numUnlockedWorlds; ++i)
	{
		colMiddle = colonySpr[i].getPosition() + Vector2f(colonySpr[i].getGlobalBounds().width / 2,
			colonySpr[i].getGlobalBounds().height / 2);

		if (length(shipPos - colMiddle) < colonyRadius)
		{
			selectedColony = i;
			return;
		}
	}
	
	selectedColony = -1;
}

void WorldMap::SetShipToColony(int index)
{
	selectedColony = index;
	UpdateColonySelect();
	UpdateWorldStats();

	ship->SetPosition(GetColonyCenter(selectedColony));
}

sf::Vector2f WorldMap::GetColonyCenter(int index)
{
	return colonySpr[index].getPosition() + Vector2f(colonySpr[index].getGlobalBounds().width / 2,
		colonySpr[index].getGlobalBounds().height / 2);
}

void WorldMap::SetToColony(int selColony)
{
	state = WorldMap::COLONY;
	selectedColony = selColony;
	selectedLevel = 0;
}

void WorldMap::SetToLevel(int selColony, int sec, int m)
{
	state = WorldMap::COLONY;
	selectedColony = selColony;

	SaveFile *saveFile = adventureManager->currSaveFile;
	if (selectedColony != saveFile->mostRecentWorldSelected)
	{
		saveFile->mostRecentWorldSelected = adventureManager->worldMap->selectedColony;
		saveFile->Save();
	}

	CurrSelector()->sectorSASelector->currIndex = sec;
	CurrSelector()->FocusedSector()->mapSASelector->currIndex = m;

	CurrSelector()->CreateBGs();

	CurrSelector()->FocusedSector()->UpdateLevelStats();
	CurrSelector()->FocusedSector()->UpdateStats();
	CurrSelector()->FocusedSector()->UpdateMapPreview();

	CurrSelector()->state = MapSelector::S_MAPSELECT;
	CurrSelector()->FocusedSector()->UpdateMapPreview();

	

	Update();

	//CurrSelector()->FocusedSector();
}

void WorldMap::HandleEvent(sf::Event ev)
{
	if (state == COLONY)
	{
		MapSector *currSector = CurrSelector()->FocusedSector();
		if (currSector->state == MapSector::LEADERBOARD)
		{
			adventureManager->leaderboard->HandleEvent(ev);
		}
	}
}

void WorldMap::UpdateButtonIconsWhenControllerIsChanged()
{
	ts_buttons = mainMenu->GetButtonIconTileset(mainMenu->adventureManager->controllerInput->GetControllerType());

	int cType = mainMenu->adventureManager->controllerInput->GetControllerType();

	auto button = XBOX_START;
	IntRect ir = mainMenu->GetButtonIconTileForMenu(cType, button);
	SetRectSubRect(parallelPlayButtonQuad, ir);
}

void WorldMap::Update()
{
	auto *controllerInput = adventureManager->controllerInput;
	assert(controllerInput != NULL);

	bool keyboardBack = controllerInput->GetControllerType() != CTYPE_KEYBOARD && (CONTROLLERS.KeyboardButtonPressed(Keyboard::BackSpace)
		|| CONTROLLERS.KeyboardButtonPressed(Keyboard::Escape));

	bool keyboardAllLevelsUnlockedCheat = CONTROLLERS.KeyboardButtonPressed(Keyboard::F1);

	int trans = 20;
	switch( state )
	{
	case PLANET_VISUAL_ONLY:
		break;
	case PLANET:
	{
		if (keyboardAllLevelsUnlockedCheat)
		{
			allUnlocked = !allUnlocked;
			InitSelectors();
		}

		UpdateSelectedColony();

		UpdateColonySelect();

		ship->Update(controllerInput);

		if ( selectedColony >= 0 && controllerInput->ButtonPressed_A() )//currInput.A && !prevInput.A)
		{
			state = PlANET_TO_COLONY;
			frame = 0;
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("world_zoom_in"));

			MapSelector *currSelector = CurrSelector();

			int startSector = 0;
			for (int i = 0; i < currSelector->numSectors; ++i)
			{
				if (!currSelector->sectors[i]->state == MapSector::COMPLETE)
				{
					startSector = i;
					break;
				}
			}

			currSelector->sectorSASelector->currIndex = startSector;

			//currSelector->sectors[startSector]->CreateBG();

			for (int se = 0; se < currSelector->numSectors; ++se)
			{
				int numLevels = currSelector->sectors[startSector]->numLevels;
				int startLevel = 0;
				for (int i = 0; i < numLevels; ++i)
				{
					/*if (!currSelector->sectors[startSector]->sec->levels[i].GetComplete())
					{
						startLevel = i;
						break;
					}*/
				}
				//currSelector->mapSelector->currIndex = startLevel;
			}
			
			break;
		}
		else if (controllerInput->ButtonPressed_B() || keyboardBack)
		{
			state = PLANET_VISUAL_ONLY;
			frame = 0;

			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("main_menu_back"));
			mainMenu->adventureManager->FadeInSaveMenu();
			

			
			//mainMenu->customCursor->SetMode(CustomCursor::M_REGULAR);
			//mainMenu->customCursor->Hide();
			//mainMenu->saveMenu->kinFace.setTextureRect(mainMenu->saveMenu->ts_kinFace->GetSubRect(0));
			//mainMenu->saveMenu->kinJump.setTextureRect(mainMenu->saveMenu->ts_kinJump1->GetSubRect(0));

			//action = SPACE;
			//transition back up later instead of just turning off
			break;
		}
		else if (controllerInput->ButtonPressed_Start())
		{
			if (MainMenu::GetInstance()->steamOn)
			{
				adventureManager->parallelPracticeMode = !adventureManager->parallelPracticeMode;

				ConfigData data = mainMenu->config->GetData();
				data.parallelPlayOn = adventureManager->parallelPracticeMode;
				mainMenu->config->SetData(data);
				mainMenu->config->Save();
			}
			
		}

		if (!MainMenu::GetInstance()->steamOn)
		{
			adventureManager->parallelPracticeMode = false;
		}

		if (adventureManager->parallelPracticeMode)
		{
			ts_parallelPlayMarker->SetSubRect(parallelPlayMarkerSpr, 0);
		}
		else
		{
			ts_parallelPlayMarker->SetSubRect(parallelPlayMarkerSpr, 1);
		}

		int numUnlockedWorlds = -1;
		int numCompletedWorlds = -1;
		if (allUnlocked)
		{
			numCompletedWorlds = adventureManager->adventurePlanet->numWorlds;
		}
		else
		{
			SaveFile *saveFile = adventureManager->currSaveFile;
			numCompletedWorlds = saveFile->GetNumCompleteWorlds(adventureManager->adventurePlanet);
		}

		if (numCompletedWorlds == adventureManager->adventurePlanet->numWorlds)
		{
			numUnlockedWorlds = numCompletedWorlds;
		}
		else
		{
			numUnlockedWorlds = numCompletedWorlds + 1;
		}

		for (int i = 0; i < ADVENTURE_MAX_NUM_WORLDS; ++i)
		{
			if ( i < numUnlockedWorlds )
			{
				SetRectCenter(worldSelectableQuads + i * 4, ts_selectableRing->tileWidth, ts_selectableRing->tileHeight, GetColonyCenter(i));
			}
			else
			{
				SetRectCenter(worldSelectableQuads + i * 4, 0, 0, GetColonyCenter(i));
			}
		}

		int tempSelected = -1;
		Vector2f shipPos = ship->GetPosition();
		for (int i = 0; i < numUnlockedWorlds; ++i)
		{
			if (length(shipPos - GetColonyCenter(i)) < colonyRadius)
			{
				tempSelected = i;
				break;
			}
		}

		if (tempSelected >= 0 && tempSelected != selectedColony)
		{
			selectedColony = tempSelected;
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundManager.GetSound("world_change"));
			UpdateWorldStats();
		}


	

		//a little messy for now until we get a menu
		if (adventureManager->parallelPracticeMode )
		{
			

			NetplayManager *netplayManager = mainMenu->netplayManager;
			assert(netplayManager != NULL);

			//has a 20 second timeout call
			/*if (frame % (60 * 5) == 0)
			{
				netplayManager->QueryPracticeMatches();
			}*/

			for (int i = 0; i < ADVENTURE_MAX_NUM_WORLDS; ++i)
			{
				if (i < numUnlockedWorlds)
				{
					numCurrentPlayersText[i].setString(to_string( netplayManager->numPracticeUsersPerWorld[i] ));
					auto lb = numCurrentPlayersText[i].getLocalBounds();
					numCurrentPlayersText[i].setOrigin(lb.left + lb.width / 2, 0);
				}
				else
				{
					numCurrentPlayersText[i].setString("");
				}
			}

			netplayManager->Update();
		}


		UpdateColonySelect();

		break;
	}	
	case PlANET_TO_COLONY:
	{
		int limit = 120 / 2;
		if (frame == limit)
		{
			//state = COLONY;
			//frame = 0;
			//state = PLANET;
			//frame = 0;
			mainMenu->LoadMode(MainMenu::WORLDMAP_COLONY);
			return;
			//worldSelector->SetAlpha(1.f - a);
		}
		else
		{
			

			float a = frame / (float)(limit - 1);

			worldSelector->SetAlpha(1.f - a * 2);
			ship->SetAlpha(1.f - a * 2);

			int mLimit = 50 / 2;
			int mFrame = min(frame, mLimit);

			float aMove = mFrame / (float)(mLimit - 1);

			CubicBezier cb(.83, .27, .06, .63);//(0, 0, 1, 1);
			CubicBezier cbMove(0, 0, 1, 1);

			oldZoomCurvePos = zoomCurvePos;

			zoomCurvePos = cb.GetValue(a);


			float f = zoomCurvePos;

			float fz = cbMove.GetValue(aMove);

			//float test = f * 5.f;


			if (frame == 0)
				zoomShader.setUniform("sampleStrength", 0.f);


			Vector2f endPos = GetColonyCenter(selectedColony);
			float endScale = colonySpr[selectedColony].getScale().x * .2f;

			Vector2f startPos(960, 540);
			float startScale = 1.f;


			float oldA = currScale;



			currScale = startScale * (1.f - f) + endScale * f;

			currCenter = startPos * (1.f - fz) + endPos * fz;

			zoomView.setCenter(currCenter);
			zoomView.setSize(Vector2f(1920, 1080) * currScale);

			if (frame > 0)
			{
				float diff = zoomCurvePos - oldZoomCurvePos;//abs(currScale - oldScale);

															//float diffFactor = diff / abs(endScale - startScale) / limit;
				float multiple = limit;
				diff *= multiple;
				diff += 1.0;

				zoomShader.setUniform("sampleStrength", diff);
			}
		}
		break;
	}
	case COLONY:
	{
		worldSelector->SetAlpha(0);
		ship->SetAlpha(0);
		Vector2f endPos = GetColonyCenter(selectedColony);
		
		zoomView.setCenter(endPos);
		zoomView.setSize(Vector2f(1920, 1080) * colonySpr[selectedColony].getScale().x * .2f);
		break;
	}
	case COLONY_TO_PLANET:
	{
		int limit = 60;//120 / 2;
		if (frame == limit)
		{
			state = PLANET;
			frame = 0;
			currScale = 1.f;
			zoomView.setSize(Vector2f(1920, 1080) * currScale);
			zoomView.setCenter(960, 540);
			worldSelector->SetAlpha(1.f);
			ship->SetAlpha(1.f);
		}
		else
		{
			
			float a = frame / (float)(limit - 1);
			worldSelector->SetAlpha(a);
			ship->SetAlpha(a);
			int mLimit = 60;//16 / 2;
			int mFrame = min(frame, mLimit);

			float aMove = mFrame / (float)(mLimit - 1);

			CubicBezier cb(0, 0, 1, 1);//(.12, .66, .85, .4);//.83, .27, .06, .63);//(0, 0, 1, 1);
			CubicBezier cbMove(0, 0, 1, 1);

			oldZoomCurvePos = zoomCurvePos;

			zoomCurvePos = cb.GetValue(a);


			float f = zoomCurvePos;

			float fz = cbMove.GetValue(aMove);

			//float test = f * 5.f;


			if (frame == 0)
				zoomShader.setUniform("sampleStrength", 0.f);

			Vector2f startPos = GetColonyCenter(selectedColony);
			float startScale = colonySpr[selectedColony].getScale().x;//.2f;

			Vector2f endPos(960, 540);
			float endScale = 1.f;


			float oldA = currScale;



			currScale = startScale * (1.f - f) + endScale * f;

			currCenter = startPos * (1.f - fz) + endPos * fz;

			zoomView.setCenter(currCenter);
			zoomView.setSize(Vector2f(1920, 1080) * currScale);

			if (frame > 0)
			{
				float diff = zoomCurvePos - oldZoomCurvePos;
				float multiple = limit;
				diff *= multiple;
				diff += 1.0;

				//zoomShader.setUniform("sampleStrength", diff);
			}
		}
		break;
	}
		
	}

	

	switch( state )
	{
	case PLANET:
		{
			worldSelector->SetAlpha(1.f);//need every frame?
			ship->SetAlpha(1.f);
			//if( frame == 0 )
			{
				//back.setTexture( *planetTex );
				/*back.setTexture( *ts_planet->texture );
				back.setColor( Color( 255, 255, 255, 255 ) );*/
			}
			break;
		}
	case PlANET_TO_COLONY:
		{
			if( frame == 0 )
			{/*
				UpdateMapList();
				front.setTexture( *ts_colony[selectedColony]->texture );
				front.setColor( Color( 255, 255, 255, 255 ) );*/
			}
			break;
		}
	case COLONY:
		{
			if (!CurrSelector()->Update(controllerInput) )
			{
				mainMenu->LoadMode(MainMenu::WORLDMAP);
				//mainMenu->SetMode(MainMenu::WORLDMAP);

			}
			break;
		}
	case COLONY_TO_PLANET:
	{
		/*if (frame == trans)
		{
			state = PLANET;
			frame = 0;
			currScale = 1.f;
			zoomView.setSize(Vector2f(1920, 1080) * currScale);
			zoomView.setCenter(960, 540);
		}*/
		break;
	}
	}

	worldSelector->Update();

	++frame;
	++asteroidFrame;
}

void WorldMap::DrawAsteroids(RenderTarget *target, bool back)
{
	int scrollSeconds[] = { 400, 300, 140, 120 };
	float astFactor[] = { .1f, .3f, 1.5f, 2.f };

	float z = zoomView.getSize().x / 1920.f;
	View vvv = zoomView;

	RenderStates rs;

	rs.shader = &asteroidShader;

	int startIndex = 0;
	int endIndex = 2;

	Vector2f viewSize(1920, 1080);

	if (!back)
	{
		startIndex = 2;
		endIndex = 4;
	}

	for (int i = startIndex; i < endIndex; ++i)
	{
		rs.texture = ts_asteroids[i]->texture;
		float aZ = 1.f - z;
		aZ *= astFactor[i];
		aZ = 1.f - aZ;

		assert(aZ <= 1.f);
		if (aZ > 0)
		{
			float xDiff = zoomView.getCenter().x - 960;
			float yDiff = zoomView.getCenter().y - 540;
			xDiff *= 1.f - aZ;
			yDiff *= 1.f - aZ;
			vvv.setSize(aZ * 1920.f, aZ * 1080.f);
			vvv.setCenter(960 + xDiff, 540 + yDiff);
			target->setView(vvv);
			asteroidShader.setUniform("quant", -(asteroidFrame % (scrollSeconds[i] * 60)) / (float)(scrollSeconds[i] * 60));
			target->draw(asteroidQuads + i * 4, 4, sf::Quads, rs);
		}
	}
}

void WorldMap::Draw( RenderTarget *target )
{
	sf::RenderTexture *rt = MainMenu::extraScreenTexture;
	rt->clear(sf::Color::Transparent);
	
	rt->setView(uiView);

	if (state == COLONY)
	{
		rt->clear(Color::Transparent);
		rt->setView(uiView);
		CurrSelector()->Draw(rt);
		rt->draw(currLevelTimeText);
		rt->display();
		const sf::Texture &ttex = rt->getTexture();
		selectorExtraPass.setTexture(ttex);

		float dur = 30;
		if (frame <= dur)
		{
			selectorExtraPass.setColor(Color(255, 255, 255, 255.f * (frame / dur)));
		}
		else
		{
			//selectorExtraPass.setColor(Color(255, 255, 255, 40));
			selectorExtraPass.setColor(Color::White);
		}
		target->draw(selectorExtraPass);

		return;
	}

	rt->draw(spaceSpr);

	DrawAsteroids(rt, true);

	rt->setView(zoomView);

	rt->draw(planetSpr);

	for (int i = 0; i < 7; ++i)
	{
		rt->draw(colonySpr[i]);
	}

	int energyBreathe = 240;
	Color selectColor = Color::White;
	int ff = asteroidFrame % (energyBreathe);
	float energyFactor = 0;
	if (ff < energyBreathe / 2)
	{
		energyFactor = (float)ff / (energyBreathe / 2);
		selectColor.a = 150 * energyFactor;
		//c.a = std::max(20.f, (float)c.a);
	}
	else
	{
		energyFactor = 1.f - (float)(ff - energyBreathe / 2) / (energyBreathe / 2);
		selectColor.a = 150 * energyFactor;
		//c.a = std::max(20.f, (float)c.a);
	}

	

	for (int i = 0; i < 2; ++i)
	{
		SetRectColor(worldActiveQuads + i * 4, selectColor);
		SetRectColor(worldActiveQuadsZoomed + i * 4, selectColor);
		rt->draw(worldActiveQuads + i * 4, 4, sf::Quads, ts_colonyActive[i]->texture);
		rt->draw(worldActiveQuadsZoomed + i * 4, 4, sf::Quads, ts_colonyActiveZoomed[i]->texture);
	}

	int selectableBreathe = 120;
	ff = asteroidFrame % (selectableBreathe);
	float selectableFactor = 0;
	if (ff < selectableBreathe / 2)
	{
		selectableFactor = (float)ff / (selectableBreathe / 2);
		//c.a = std::max(20.f, (float)c.a);
	}
	else
	{
		selectableFactor = 1.f - (float)(ff - selectableBreathe / 2) / (selectableBreathe / 2);
		//c.a = std::max(20.f, (float)c.a);
	}

	selectableRingShader.setUniform("prop", selectableFactor);
	rt->draw(worldSelectableQuads, 4 * ADVENTURE_MAX_NUM_WORLDS, sf::Quads, &selectableRingShader);
	

	if (state != COLONY && selectedColony >= 0)
	{
		worldSelector->Draw(rt);
	}

	ship->Draw(rt);

	DrawAsteroids(rt, false);

	if (state == PLANET || state == PLANET_VISUAL_ONLY )
	{
		if (MainMenu::GetInstance()->steamOn)
		{
			rt->draw(parallelPlayMarkerSpr);
			rt->draw(parallelPlayButtonQuad, 4, sf::Quads, ts_buttons->texture);
		}
	}

	if (state != COLONY && state != PlANET_TO_COLONY
		&& state != COLONY_TO_PLANET && selectedColony >= 0 )
	{
		rt->draw(infoQuadBG, 4, sf::Quads);
		rt->draw(infoNameBG, 4, sf::Quads);
		rt->draw(sectorsCompleteText);
		rt->draw(shardsCapturedText);
		rt->draw(logsCapturedText);
		rt->draw(worldNameText);

		//if (adventureManager->parallelPracticeMode)
		//{
		//	for (int i = 0; i < ADVENTURE_MAX_NUM_WORLDS; ++i)
		//	{
		//		//enable again later maybe
		//		rt->draw(numCurrentPlayersText[i]);
		//	}
		//}
	}

	rt->display();
	const sf::Texture &tex = rt->getTexture();
	extraPassSpr.setTexture(tex);
	
	zoomShader.setUniform("zoomTex", sf::Shader::CurrentTexture );
	extraPassSpr.setPosition(0, 0);

	if ((state == PlANET_TO_COLONY /*|| state == COLONY_TO_PLANET*/ ) && frame > 20 )
	{
		target->draw(extraPassSpr, &zoomShader);
	}
	else
	{
		target->draw(extraPassSpr);
	}
}

Sector & WorldMap::GetCurrSector()
{
	//SaveFile * currFile = mainMenu->GetCurrSaveFile();
	//World & world = currFile->worlds[selectedColony];
	int secIndex = selectors[selectedColony]->sectorSASelector->currIndex;
	return adventureManager->adventurePlanet->worlds[selectedColony].sectors[secIndex];
	//return currFile->adventureFile.GetSector(selectedColony, secIndex);
}

int WorldMap::GetCurrSectorNumLevels()
{
	MapSelector *currSelector = selectors[selectedColony];
	int secIndex = currSelector->sectorSASelector->currIndex;
	return currSelector->sectors[secIndex]->numLevels;
}