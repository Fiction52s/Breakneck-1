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
//#include "Input.h"

using namespace std;
using namespace sf;

FreeplayPlayerBox::FreeplayPlayerBox(FreeplayScreen *p_fps, int p_index)
{
	index = p_index;

	fps = p_fps;

	Font &f = MainMenu::GetInstance()->arial;

	skinIndex = -1;

	playerNameText.setCharacterSize(30);
	playerNameText.setFont(f);
	playerNameText.setFillColor(Color::White);

	numberText.setCharacterSize(30);
	numberText.setFont(f);
	numberText.setFillColor(Color::White);

	pressText.setCharacterSize(30);
	pressText.setFont(f);
	pressText.setFillColor(Color::White);

	
	skinNumberText.setCharacterSize(16);
	skinNumberText.setFont(f);
	skinNumberText.setFillColor(Color::White);

	

	pressText.setString("Press Start");
	auto localB = pressText.getLocalBounds();
	pressText.setOrigin(localB.left + localB.width / 2, localB.top + localB.height / 2);

	fps->ts_kin->SetSpriteTexture(kinSprite);
	fps->ts_kin->SetSubRect(kinSprite, 0);
	kinSprite.setOrigin(kinSprite.getLocalBounds().width / 2, kinSprite.getLocalBounds().height / 2);
	kinSprite.setScale(4, 4);

	//fps->ts_kin->SetQuadSubRect(kinQuad, 0);

	playerShader = new PlayerSkinShader("player");

	playerShader->SetTileset(fps->ts_kin);

	playerShader->SetQuad(fps->ts_kin, 0);
	//playerShader->SetSubRect(fps->ts_kin, fps->ts_kin->GetSubRect(0));
	playerShader->SetSkin(0);

	SetRectColor(bgQuad, Color::Red);

	SetTopLeft(Vector2i(0, 0));//Vector2i(100, 700) + Vector2i(index * (spacing + width), 0));

	ClearInfo();

	Show();
}

FreeplayPlayerBox::~FreeplayPlayerBox()
{
	delete playerShader;
}

void FreeplayPlayerBox::SetSkin(int index)
{
	skinIndex = index;
	playerShader->SetSkin(skinIndex);

	if (skinIndex < 10)
	{
		skinNumberText.setString("S-KIN #0" + to_string(skinIndex));
	}
	else
	{
		skinNumberText.setString("S-KIN #" + to_string(skinIndex));
	}
	
}

void FreeplayPlayerBox::ClearInfo()
{
	controllerStates = NULL;
	SetName("");
	action = A_WAITING_FOR_JOIN;
	skinIndex = -1;
}

void FreeplayPlayerBox::Show()
{
	show = true;
	SetTopLeft(topLeft);
	//numberText->setString(to_string(index));
	//numberText->setOrigin(numberText->getLocalBounds().left + numberText->getLocalBounds().width / 2,
	//	numberText->getLocalBounds().top + numberText->getLocalBounds().height / 2);
}
void FreeplayPlayerBox::Hide()
{
	show = false;
	//playerNameText->setString("");
	//numberText->setString("");
}

void FreeplayPlayerBox::Update()
{
	if (controllerStates != NULL)
	{
		if (controllerStates->ButtonPressed_LeftShoulder())
		{
			fps->PrevSkin(index);
		}
		else if (controllerStates->ButtonPressed_RightShoulder())
		{
			fps->NextSkin(index);
		}
	}
}

void FreeplayPlayerBox::SetTopLeft(sf::Vector2i &pos)
{
	topLeft = pos;

	Vector2i center(topLeft.x + fps->playerBoxWidth / 2, topLeft.y + fps->playerBoxHeight / 2);

	pressText.setPosition(Vector2f(center));
	//numberText->setPosition(Vector2f(center));

	Vector2i namePos(fps->playerBoxWidth / 2, 0);

	SetRectTopLeft(bgQuad, fps->playerBoxWidth, fps->playerBoxHeight, Vector2f(topLeft));

	Vector2f controllerIconSize(fps->ts_controllerIcons->tileWidth, fps->ts_controllerIcons->tileHeight);
	Vector2f portIconSize(fps->ts_portIcons->tileWidth, fps->ts_portIcons->tileHeight);

	int border = 10;

	SetRectTopLeft(controllerIconQuad, controllerIconSize.x, controllerIconSize.y, Vector2f(fps->playerBoxWidth - (controllerIconSize.x + border), border) + Vector2f(topLeft));
	SetRectTopLeft(portIconQuad, portIconSize.x, portIconSize.y, Vector2f(fps->playerBoxWidth - (portIconSize.x + border), controllerIconSize.y) + Vector2f(topLeft));

	SetRectCenter(kinQuad, fps->ts_kin->tileWidth, fps->ts_kin->tileHeight, Vector2f(center));

	kinSprite.setPosition(Vector2f(center));

	skinNumberText.setPosition(Vector2f(topLeft + Vector2i(210, 250)));


	//playerName->setPosition(Vector2f(pos + namePos));
	//auto &bounds = playerName->getLocalBounds();
	//playerName->setPosition(Vector2f(playerName->getPosition().x - (bounds.left + bounds.width / 2), playerName->getPosition().y));
}

void FreeplayPlayerBox::SetControllerStates(ControllerDualStateQueue *conStates, int p_skinIndex)
{
	if (conStates == NULL)
		return;

	controllerStates = conStates;
	action = A_HAS_PLAYER;

	int tileIndex = 0;
	switch (controllerStates->GetControllerType())
	{
	case CTYPE_XBOX:
	{
		tileIndex = 1;
		break;
	}
	case CTYPE_GAMECUBE:
	{
		tileIndex = 2;
		break;
	}
	case CTYPE_PS5:
	{
		tileIndex = 0;
		break;
	}
	case CTYPE_KEYBOARD:
	{
		tileIndex = 3;
		break;
	}
	}

	fps->ts_controllerIcons->SetQuadSubRect(controllerIconQuad, tileIndex);
	fps->ts_portIcons->SetQuadSubRect(portIconQuad, controllerStates->GetIndex() );

	SetSkin(p_skinIndex);
}

void FreeplayPlayerBox::SetName(const std::string &name)
{
	playerNameStr = name;

	if (show)
	{
		playerNameText.setString(playerNameStr);
		SetTopLeft(topLeft);
	}
}

void FreeplayPlayerBox::Draw(sf::RenderTarget *target)
{
	if (!show)
		return;

	target->draw(bgQuad, 4, sf::Quads);

	if (action == A_HAS_PLAYER)
	{
		target->draw(playerNameText);
		target->draw(numberText);

		target->draw(controllerIconQuad, 4, sf::Quads, fps->ts_controllerIcons->texture);

		if (controllerStates->GetControllerType() != CTYPE_KEYBOARD)
		{
			target->draw(portIconQuad, 4, sf::Quads, fps->ts_portIcons->texture);
		}

		//target->draw(kinQuad, 4, sf::Quads, &(playerShader->pShader));
		target->draw(kinSprite, &(playerShader->pShader));

		target->draw(skinNumberText);
	}
	else
	{
		target->draw(pressText);
	}
	
}

FreeplayScreen::FreeplayScreen(MainMenu *mm)
{
	mainMenu = mm;

	panel = new Panel("freeplayscreen", 1920, 1080, this, true);
	panel->SetColor(Color::Transparent);
	panel->SetCenterPos(Vector2i(960, 540));

	mapBrowserScreen = MainMenu::GetInstance()->mapBrowserScreen;
	assert(mapBrowserScreen != NULL);
	//startButton = panel->AddButton("start", Vector2i(20, panel->size.y - 100), Vector2f(100, 40), "START");
	//leaveButton = panel->AddButton("leave", Vector2i(20 + 200, panel->size.y - 100), Vector2f(100, 40), "LEAVE");

	ts_controllerIcons = GetSizedTileset( "Menu/controllers_64x64.png" );
	ts_portIcons = GetSizedTileset("Menu/slots_64x32.png");
	ts_kin = GetSizedTileset("Kin/stand_64x64.png");

	//inviteButton = panel->AddButton("invite", Vector2i(20 + 200, panel->size.y - 200), Vector2f(270, 40), "INVITE FRIEND");

	SetRectColor(bgQuad, Color( 100, 100, 100 ));
	SetRectCenter(bgQuad, 1920, 1080, Vector2f(960, 540));

	for (int i = 0; i < 4; ++i)
	{
		playerBoxes[i] = new FreeplayPlayerBox(this, i);
	}

	mapOptionsPopup = new MapOptionsPopup(MapOptionsPopup::MODE_FREEPLAY);

	int numBoxes = 4;
	playerBoxWidth = 300;
	playerBoxHeight = 300;
	playerBoxSpacing = 100;

	Vector2i left(960, 540 + 100);//50);//150
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
		playerBoxes[i]->SetTopLeft(left + Vector2i((playerBoxSpacing + playerBoxWidth) * i, 0));
	}

	Start();
}

FreeplayScreen::~FreeplayScreen()
{
	delete panel;
	for (int i = 0; i < 4; ++i)
	{
		delete playerBoxes[i];
	}

	delete mapOptionsPopup;
}

void FreeplayScreen::Start()
{
	SetAction(A_WAITING_FOR_PLAYERS);
	for (int i = 0; i < 4; ++i)
	{
		playerBoxes[i]->ClearInfo();
	}
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
	action = A_CHOOSE_MAP;
	frame = 0;
	mapBrowserScreen->browserHandler->ClearSelection();
	//mapBrowserScreen->browserHandler->chooser->ClearNodes();
	//would clear nodes, but everyone is supposed to clear their own nodes so this would be redundant
	//actually all 3 of these would be redundant

	mapBrowserScreen->browserHandler->chooser->ClearFilters();
	//NumActivePlayers()

	int numActivePlayers = NumActivePlayers();
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


	mapBrowserScreen->StartLocalBrowsing();
}

void FreeplayScreen::TryActivateOptionsPanel(MapNode *mp)
{
	if (mapOptionsPopup->Activate(mp, NumActivePlayers() ) )
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
		playerBoxes[i]->SetControllerStates(mp.controllerStateVec[i], mp.playerSkins[i]);
		//playerBoxes[i]->skinIndex = mp.playerSkins[i];
	}
}

int FreeplayScreen::GetFirstAvailableSkinIndex()
{
	int testSkin = 0;

	bool found = false;
	while (true)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (playerBoxes[i]->controllerStates != NULL)
			{
				if (playerBoxes[i]->skinIndex == testSkin)
				{
					testSkin++;
					break;
				}
			}
		}

		break;
	}

	return testSkin;
}

bool FreeplayScreen::IsSkinAvailable(int p_skinIndex)
{
	for (int i = 0; i < 4; ++i)
	{
		if (playerBoxes[i]->controllerStates != NULL)
		{
			if (playerBoxes[i]->skinIndex == p_skinIndex)
			{
				return false;
			}
		}
	}

	return true;
}

void FreeplayScreen::NextSkin(int playerBoxIndex)
{
	int nextIndex = playerBoxes[playerBoxIndex]->skinIndex;

	do
	{
		nextIndex++;
		if (nextIndex >= Actor::SKIN_Count)
		{
			nextIndex = 0;
		}
	} 
	while (!IsSkinAvailable(nextIndex));

	playerBoxes[playerBoxIndex]->SetSkin(nextIndex);
}

void FreeplayScreen::PrevSkin(int playerBoxIndex)
{
	int prevIndex = playerBoxes[playerBoxIndex]->skinIndex;

	do
	{
		prevIndex--;
		if (prevIndex < 0)
		{
			prevIndex = Actor::SKIN_Count - 1;
		}
	} while (!IsSkinAvailable(prevIndex));

	playerBoxes[playerBoxIndex]->SetSkin(prevIndex);
}

void FreeplayScreen::Update()
{
	switch (action)
	{
	case A_WAITING_FOR_PLAYERS:
	{
		ControllerDualStateQueue *states = NULL;
		for (int i = 0; i < 4; ++i)
		{
			states = playerBoxes[i]->controllerStates;
			if ( states != NULL)
			{
				if (states->ButtonPressed_Start())
				{
					StartBrowsing();
					return; //don't use the same controller inputs for the next menu, gotta skip a frame
				}
			}
		}
		break;
	}
	case A_CHOOSE_MAP:

		if (mapBrowserScreen->browserHandler->chooser->selectedRect != NULL)
		{
			selectedMap = (MapNode*)mapBrowserScreen->browserHandler->chooser->selectedRect->info;

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

		if (mapBrowserScreen->browserHandler->chooser->action == MapBrowser::A_CANCELLED)
		{
			SetAction(A_WAITING_FOR_PLAYERS);
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

			SetAction(A_START);

			currParams.gameModeType = mapOptionsPopup->currLobbyData->gameModeType;//MatchParams::GAME_MODE_BASIC;//mapOptionsPopup->currLobbyData->gameModeType;
			currParams.mapPath = mapOptionsPopup->currLobbyData->mapPath;
			currParams.numPlayers = NumActivePlayers();
			currParams.randSeed = time(0);

			for (int i = 0; i < 4; ++i)
			{
				currParams.controllerStateVec[i] = playerBoxes[i]->controllerStates;
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
		for (int i = 0; i < 4; ++i)
		{
			playerBoxes[i]->Update();
		}

		panel->MouseUpdate();

		ControllerDualStateQueue *states = NULL;
		for (int i = 0; i < 4; ++i)
		{
			states = CONTROLLERS.GetStateQueue(CTYPE_XBOX, i);
			if (states->ButtonPressed_Start())
			{
				TryControllerJoin(states);
			}

			states = CONTROLLERS.GetStateQueue(CTYPE_GAMECUBE, i);
			if (states->ButtonPressed_Start())
			{
				TryControllerJoin(states);
			}
		}

		states = CONTROLLERS.GetStateQueue(CTYPE_KEYBOARD, 0);
		if (states->ButtonPressed_Start())
		{
			TryControllerJoin(states);
		}
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

bool FreeplayScreen::IsFull()
{
	for (int i = 0; i < 4; ++i)
	{
		if (playerBoxes[i]->controllerStates == NULL)
		{
			return false;
		}
	}

	return true;
}

bool FreeplayScreen::AlreadyJoined(ControllerDualStateQueue *conStates)
{
	for (int i = 0; i < 4; ++i)
	{
		if (playerBoxes[i]->controllerStates == conStates)
		{
			return true;
		}
	}

	return false;
}

FreeplayPlayerBox *FreeplayScreen::GetNextOpenBox()
{
	for (int i = 0; i < 4; ++i)
	{
		if (playerBoxes[i]->controllerStates == NULL)
		{
			return playerBoxes[i];
		}
	}

	return NULL;
}

void FreeplayScreen::TryControllerJoin(ControllerDualStateQueue *conStates)
{
	if( IsFull() )
		return;

	if (!AlreadyJoined(conStates))
	{
		FreeplayPlayerBox *next = GetNextOpenBox();
		assert(next != NULL);

		next->SetControllerStates(conStates, GetFirstAvailableSkinIndex());
		cout << "added controller: " << conStates->GetControllerType() << ", index: " << conStates->GetIndex() << endl;
	}
}

int FreeplayScreen::NumActivePlayers()
{
	int numActive = 0;
	for (int i = 0; i < 4; ++i)
	{
		if (playerBoxes[i]->controllerStates != NULL)
			++numActive;
	}

	return numActive;
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

		for (int i = 0; i < 4; ++i)
		{
			playerBoxes[i]->Draw(target);
		}

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