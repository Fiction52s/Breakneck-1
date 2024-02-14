#include <iostream>
#include "MainMenu.h"
#include "SinglePlayerControllerJoinScreen.h"
#include "UIMouse.h"
#include "MapBrowserScreen.h"
#include "MapBrowser.h"
#include "MapOptionsPopup.h"
#include "LobbyManager.h"
#include "PlayerSkinShader.h"
#include "Actor.h"
#include "ControlProfile.h"
//#include "Input.h"

using namespace std;
using namespace sf;

StartBox::StartBox()
{
	mm = MainMenu::GetInstance();

	startText.setFont(mm->arial);
	startText.setFillColor(Color::White);
	startText.setOutlineColor(Color::Black);
	startText.setOutlineThickness(1);
	startText.setCharacterSize(48);
	startText.setString("GO!");
	startText.setOrigin(startText.getLocalBounds().left + startText.getLocalBounds().width / 2, 0);

	SetRectColor(bgQuad, Color::Green);

	width = 400;
	height = 200;

	ts_buttons = NULL;
}

void StartBox::Update()
{

}

void StartBox::SetControllerType(int cType)
{
	ts_buttons = mm->GetButtonIconTileset(cType);

	auto button = XBOX_START;
	IntRect ir = mm->GetButtonIconTileForMenu(cType, button);
	SetRectSubRect(startButtonQuad, ir);
}

void StartBox::SetTopLeft(sf::Vector2f pos)
{
	SetRectTopLeft(bgQuad, width, height, pos);

	startText.setPosition(pos + Vector2f(width / 2, 0));

	float size = 128;
	SetRectTopLeft(startButtonQuad, size, size, pos + Vector2f(width / 2 - size / 2, height - (size + 10)));
}

void StartBox::SetCenter(sf::Vector2f pos)
{
	SetTopLeft(pos - Vector2f(width / 2, height / 2));
}

void StartBox::Draw(sf::RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads);
	target->draw(startText);

	assert(ts_buttons != NULL);
	target->draw(startButtonQuad, 4, sf::Quads, ts_buttons->texture);
}

SinglePlayerControllerJoinScreen::SinglePlayerControllerJoinScreen(MainMenu *mm)
{
	mainMenu = mm;

	ts_bg = mainMenu->GetTileset("Menu/Load/load_w1.png", 1920, 1080);

	panel = new Panel("SinglePlayerControllerJoinScreen", 1920, 1080, this, true);
	panel->SetColor(Color::Transparent);
	panel->SetCenterPos(Vector2i(960, 540));

	startBox.SetCenter(Vector2f(960, 150));

	selectedMap = NULL;

	//playerBoxGroup = new PlayerBoxGroup(this, 1, 400, 400, 100);
	playerBoxGroup = new PlayerBoxGroup(this, 1, 450, 450, 100);

	//inviteButton = panel->AddButton("invite", Vector2i(20 + 200, panel->size.y - 200), Vector2f(270, 40), "INVITE FRIEND");

	//SetRectColor(bgQuad, Color(100, 100, 100));
	SetRectCenter(bgQuad, 1920, 1080, Vector2f(960, 540));

	mapOptionsPopup = new MapOptionsPopup(MapOptionsPopup::MODE_FREEPLAY);

	Vector2f center(960, 540 + 100);//50);//150

	ts_bg->SetQuadSubRect(bgQuad, 0);

	playerBoxGroup->SetBoxCenter( 0, center);
	
	Start();
}

SinglePlayerControllerJoinScreen::~SinglePlayerControllerJoinScreen()
{
	delete panel;
	delete playerBoxGroup;

	delete mapOptionsPopup;
}

void SinglePlayerControllerJoinScreen::Start()
{
	SetAction(A_WAITING_FOR_PLAYER);
	//SetRectColor(bgQuad, Color(100, 100, 100));
	playerBoxGroup->ClearInfo();
}

void SinglePlayerControllerJoinScreen::Quit()
{
	action = A_BACK;
}

bool SinglePlayerControllerJoinScreen::HandleEvent(sf::Event ev)
{
	switch (action)
	{
	case A_WAITING_FOR_PLAYER:
		return panel->HandleEvent(ev);
	}

	return false;
}

void SinglePlayerControllerJoinScreen::SetMode(int m)
{
	playerBoxGroup->SetMode(m);
}

const MatchParams &SinglePlayerControllerJoinScreen::GetMatchParams()
{
	return currParams;
	//cout << "creating custom lobby test: " << mapOptionsPopup->currLobbyData->mapPath << endl;
	//cout << "hash: " << mapOptionsPopup->currLobbyData->fileHash << endl;
	//cout << "creatorID: " << mapOptionsPopup->currLobbyData->creatorId << endl;
}

void SinglePlayerControllerJoinScreen::SetFromMatchParams(MatchParams &mp)
{
	playerBoxGroup->SetControllerStates( 0, mp.controllerStateVec[0], mp.playerSkins[0]);
}

void SinglePlayerControllerJoinScreen::Update()
{
	switch (action)
	{
	case A_WAITING_FOR_PLAYER:
	{
		if (CONTROLLERS.ButtonPressed_B())
		{
			Quit();
			return;
		}

		if (playerBoxGroup->CheckControllerJoins())
		{
			//mainMenu->soundNodeList->ActivateSound(mainMenu->soundInfos[MainMenu::S_PLAYER_JOIN]);
			SetAction(A_READY);
			startBox.SetControllerType(playerBoxGroup->GetControllerStates(0)->GetControllerType());
			//SetRectColor(bgQuad, Color(83, 102, 188));
		}

		break;
	}
	case A_READY:
	{
		if (CONTROLLERS.ButtonPressed_B())
		{
			mainMenu->soundNodeList->ActivateSound(mainMenu->soundInfos[MainMenu::S_PLAYER_UNJOIN]);
			Start();
			break;
		}

		ControllerDualStateQueue *states = playerBoxGroup->GetControllerStates(0);

		if (states != NULL)
		{
			if (states->ButtonPressed_Start())
			{
				SetAction(A_START);
				return;
			}
		}
		break;
	}
	case A_CONTROL_PROFILE:
	{
		if (!playerBoxGroup->IsBoxChangingControls( 0 ) )
		{
			SetAction(A_READY);
			return;
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
	case A_WAITING_FOR_PLAYER:
	{
		playerBoxGroup->Update();
		panel->MouseUpdate();
		break;
	}
	case A_READY:
	{
		playerBoxGroup->Update();
		if (!playerBoxGroup->IsBoxChangingControls( 0 ) )
		{
			panel->MouseUpdate();
		}
		else
		{
			SetAction(A_CONTROL_PROFILE);
		}
		break;
	}
	case A_CONTROL_PROFILE:
		playerBoxGroup->Update();
		break;
	case A_START:
		break;
	case A_BACK:
		break;
	}
}


void SinglePlayerControllerJoinScreen::TryControllerJoin(ControllerDualStateQueue *conStates)
{
	assert(playerBoxGroup->playerBoxes[0]->controllerStates == NULL);

	playerBoxGroup->SetControllerStates( 0, conStates, 0);

	SetAction(A_READY);
	//SetRectColor(bgQuad, Color(83, 102, 188));
}

void SinglePlayerControllerJoinScreen::DrawPopupBG(sf::RenderTarget *target)
{
	sf::RectangleShape rect;
	rect.setFillColor(Color(0, 0, 0, 100));
	rect.setSize(Vector2f(1920, 1080));
	rect.setPosition(0, 0);
	target->draw(rect);
}

void SinglePlayerControllerJoinScreen::Draw(sf::RenderTarget *target)
{
	switch (action)
	{
	case A_BACK:
	case A_WAITING_FOR_PLAYER:
	case A_READY:
	case A_CONTROL_PROFILE:
	case A_START:
	{
		target->draw(bgQuad, 4, sf::Quads, ts_bg->texture);

		playerBoxGroup->Draw(target);
		panel->Draw(target);

		if (action == A_READY)
		{
			startBox.Draw(target);
		}
		break;
	}
	/*case A_CONTROL_PROFILE:
	{
		playerBox->Draw(target);
		panel->Draw(target);
		break;
	}*/
	}
}

void SinglePlayerControllerJoinScreen::SetAction(int a)
{
	action = a;
	frame = 0;
}

void SinglePlayerControllerJoinScreen::CancelCallback(Panel *p)
{
	//Quit();
}