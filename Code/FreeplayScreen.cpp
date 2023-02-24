#include <iostream>
#include "MainMenu.h"
#include "FreeplayScreen.h"
#include "UIMouse.h"
#include "MapBrowserScreen.h"
#include "MapBrowser.h"
#include "MapOptionsPopup.h"
#include "LobbyManager.h"
#include "PlayerSkinShader.h"
#include "Actor.h"
#include "MainMenu.h"
//#include "Input.h"

using namespace std;
using namespace sf;



FreeplayScreen::FreeplayScreen()
{
	mainMenu = MainMenu::GetInstance();

	panel = new Panel("freeplayscreen", 1920, 1080, this, true);
	panel->SetColor(Color::Transparent);
	panel->SetCenterPos(Vector2i(960, 540));

	mapBrowserScreen = MainMenu::GetInstance()->mapBrowserScreen;
	assert(mapBrowserScreen != NULL);

	//inviteButton = panel->AddButton("invite", Vector2i(20 + 200, panel->size.y - 200), Vector2f(270, 40), "INVITE FRIEND");

	SetRectColor(bgQuad, Color( 100, 100, 100 ));
	SetRectCenter(bgQuad, 1920, 1080, Vector2f(960, 540));

	

	mapOptionsPopup = new MapOptionsPopup(MapOptionsPopup::MODE_FREEPLAY);

	int numBoxes = 4;
	int playerBoxWidth = 450;
	int playerBoxHeight = 450;
	int playerBoxSpacing = 20;

	playerBoxGroup = new PlayerBoxGroup(this, numBoxes, playerBoxWidth, playerBoxHeight, playerBoxSpacing);
	Vector2f left(960, 540 );//50);//150
	if (numBoxes % 2 == 0)
	{
		left.x -= playerBoxSpacing / 2 + playerBoxWidth + (playerBoxSpacing + playerBoxWidth) * (numBoxes / 2 - 1);
	}
	else
	{
		left.x -= playerBoxWidth / 2 + (playerBoxSpacing + playerBoxWidth) * (numBoxes / 2);
	}

	//left + Vector2f(nodeSize / 2, 0) + Vector2f((pathLen + nodeSize) * node, 0);
	for (int i = 0; i < numBoxes; ++i)
	{
		playerBoxGroup->SetBoxTopLeft( i, left + Vector2f((playerBoxSpacing + playerBoxWidth) * i, 0));
	}

	//Start();
}

FreeplayScreen::~FreeplayScreen()
{
	delete panel;

	delete playerBoxGroup;

	delete mapOptionsPopup;
}

void FreeplayScreen::Start()
{
	SetAction(A_WAITING_FOR_PLAYERS);
	//MOUSE.Hide();
	playerBoxGroup->ClearInfo();
}

void FreeplayScreen::Quit()
{
	action = A_BACK;
	mapBrowserScreen->browserHandler->Clear();
}

bool FreeplayScreen::HandleEvent(sf::Event ev)
{
	switch (action)
	{
	case A_WAITING_FOR_PLAYERS:
		return panel->HandleEvent(ev);
	case A_CHOOSE_MAP:
		return mapBrowserScreen->HandleEvent(ev);
	case A_CHOOSE_MAP_OPTIONS:
		return mapOptionsPopup->HandleEvent(ev);
	}
	
	return false;
}

void FreeplayScreen::StartBrowsing()
{
	MOUSE.Show();
	action = A_CHOOSE_MAP;
	frame = 0;
	mapBrowserScreen->browserHandler->ClearSelection();
	//mapBrowserScreen->browserHandler->chooser->ClearNodes();
	//would clear nodes, but everyone is supposed to clear their own nodes so this would be redundant
	//actually all 3 of these would be redundant

	mapBrowserScreen->browserHandler->chooser->ClearFilters();
	//NumActivePlayers()

	int numActivePlayers = playerBoxGroup->GetNumFullBoxes();
	std::vector<int> numPlayersVec;
	numPlayersVec.push_back(numActivePlayers);

	std::vector<int> gameModesVec;
	gameModesVec.reserve(MatchParams::GAME_MODE_Count);

	bool modeFound = false;
	for (int i = 0; i < MatchParams::GAME_MODE_Count; ++i)
	{
		modeFound = false;
		//4 is max players
		for (int j = 0; j < 4; ++j)
		{
			auto &pVec = MatchParams::GetNumPlayerOptions(i, j);
			for (auto pIt = pVec.begin(); pIt != pVec.end(); ++pIt)
			{
				if ((*pIt) == numActivePlayers )
				{
					modeFound = true;
					gameModesVec.push_back(i);
					break;
				}
			}

			if (modeFound)
				break;
		}
	}
	
	mapBrowserScreen->browserHandler->chooser->UpdateNumPlayersCriteria(numPlayersVec);
	mapBrowserScreen->browserHandler->chooser->UpdateGameModeCriteria(gameModesVec);


	mapBrowserScreen->StartLocalBrowsing(MapBrowser::FREEPLAY, true);
}

void FreeplayScreen::TryActivateOptionsPanel(MapNode *mp)
{
	if (mapOptionsPopup->Activate(mp, playerBoxGroup->GetNumFullBoxes() ) )
	{
		action = A_CHOOSE_MAP_OPTIONS;
	}
	else
	{
		//messagePopup->Pop("ERROR: Map choice not valid.");
		//preErrorAction = action;
		//action = A_ERROR_MESSAGE;
		//selectedMap = NULL;
		//mapBrowserScreen->browserHandler->ClearSelection();
	}
}

const MatchParams &FreeplayScreen::GetMatchParams()
{
	return currParams;
	//cout << "creating custom lobby test: " << mapOptionsPopup->currLobbyData->mapPath << endl;
	//cout << "hash: " << mapOptionsPopup->currLobbyData->fileHash << endl;
	//cout << "creatorID: " << mapOptionsPopup->currLobbyData->creatorId << endl;
}

void FreeplayScreen::SetFromMatchParams(MatchParams &mp)
{
	for (int i = 0; i < 4; ++i)
	{
		playerBoxGroup->SetControllerStates( i, mp.controllerStateVec[i], mp.playerSkins[i]);
	}
}

void FreeplayScreen::Update()
{
	switch (action)
	{
	case A_WAITING_FOR_PLAYERS:
	{
		if (playerBoxGroup->BackButtonPressed())
		{
			Quit();
			return;
		}


		if (playerBoxGroup->IsReadyAndStartPressed())
		{
			StartBrowsing();
			return; //don't use the same controller inputs for the next menu, gotta skip a frame
		}
		break;
	}
	case A_CHOOSE_MAP:
		if (mapBrowserScreen->browserHandler->chooser->selectedNode != NULL)
		{
			selectedMap = mapBrowserScreen->browserHandler->chooser->selectedNode;

			if (mapBrowserScreen->browserHandler->CheckIfSelectedItemInstalled())
			{
				//mapOptionsPopup->Activate(boost::filesystem::relative(selectedMap->filePath).string());
				TryActivateOptionsPanel(selectedMap);
			}
			else
			{
				action = A_DOWNLOADING_WORKSHOP_MAP;
				selectedMap->Subscribe();
			}
		}

		if (mapBrowserScreen->IsCancelled())
		{
			mapBrowserScreen->TurnOff();
			SetAction(A_WAITING_FOR_PLAYERS);
			MOUSE.Hide();
			//Quit();
			return;
		}
		break;
	case A_CHOOSE_MAP_OPTIONS:
	{
		if (mapOptionsPopup->action == MapOptionsPopup::A_HOST)
		{
			//cout << "creating custom lobby test: " << mapOptionsPopup->currLobbyData->mapPath << endl;
			//cout << "hash: " << mapOptionsPopup->currLobbyData->fileHash << endl;
			//cout << "creatorID: " << mapOptionsPopup->currLobbyData->creatorId << endl;

			mapBrowserScreen->TurnOff();

			MOUSE.Hide();

			SetAction(A_START);

			currParams.gameModeType = mapOptionsPopup->currLobbyData->gameModeType;//MatchParams::GAME_MODE_BASIC;//mapOptionsPopup->currLobbyData->gameModeType;
			currParams.mapPath = mapOptionsPopup->currLobbyData->mapPath;
			currParams.numPlayers = playerBoxGroup->GetNumFullBoxes();
			currParams.randSeed = time(0);

			for (int i = 0; i < 4; ++i)
			{
				currParams.controllerStateVec[i] = playerBoxGroup->GetControllerStates(i);
				currParams.controlProfiles[i] = playerBoxGroup->GetControlProfile(i);
				currParams.playerSkins[i] = playerBoxGroup->GetSkinIndex(i);
			}

			//cout << "waiting room" << endl;
		}
		else if (mapOptionsPopup->action == MapOptionsPopup::A_CANCELLED)
		{
			SetAction(A_CHOOSE_MAP);
			selectedMap = NULL;
			mapBrowserScreen->browserHandler->ClearSelection();

		}
		break;
	}
	case A_START:
		break;
	case A_BACK:
		break;
	}

	switch (action)
	{
	case A_WAITING_FOR_PLAYERS:
	{
		playerBoxGroup->Update();

		//panel->MouseUpdate();

		playerBoxGroup->CheckControllerJoins();
		break;
	}
	case A_CHOOSE_MAP:
		mapBrowserScreen->Update();
		break;
	case A_CHOOSE_MAP_OPTIONS:
		mapOptionsPopup->Update();
		break;
	case A_START:
		break;
	case A_BACK:
		break;
	}

	//if (MOUSE.IsMouseRightClicked())
	//{
	//	SetAction(A_READY);
	////mainMenu->SetMode(MainMenu::Mode::TRANS_CREDITS_TO_MAIN);
	//}


	/*if (MOUSE.IsMouseRightClicked())
	{
		mainMenu->SetMode(MainMenu::Mode::TRANS_CREDITS_TO_MAIN);
	}*/
}

void FreeplayScreen::DrawPopupBG(sf::RenderTarget *target)
{
	sf::RectangleShape rect;
	rect.setFillColor(Color(0, 0, 0, 100));
	rect.setSize(Vector2f(1920, 1080));
	rect.setPosition(0, 0);
	target->draw(rect);
}

void FreeplayScreen::Draw(sf::RenderTarget *target)
{
	switch (action)
	{
	case A_BACK:
	case A_WAITING_FOR_PLAYERS:
	{
		target->draw(bgQuad, 4, sf::Quads);

		playerBoxGroup->Draw(target);

		panel->Draw(target);
		break;
	}
	case A_CHOOSE_MAP:
	{
		mapBrowserScreen->Draw(target);
		break;
	}
	case A_DOWNLOADING_WORKSHOP_MAP:
	{
		mapBrowserScreen->Draw(target);
		break;
	}
	case A_CHOOSE_MAP_OPTIONS:
	{
		mapBrowserScreen->Draw(target);
		DrawPopupBG(target);
		mapOptionsPopup->Draw(target);
		break;
	}
	}
}

void FreeplayScreen::SetAction(int a)
{
	action = a;
	frame = 0;
}

void FreeplayScreen::CancelCallback(Panel *p)
{
	Quit();
}