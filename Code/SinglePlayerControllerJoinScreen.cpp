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
//#include "Input.h"

using namespace std;
using namespace sf;

SinglePlayerBox::SinglePlayerBox(SinglePlayerControllerJoinScreen *p_joinScreen )
{
	joinScreen = p_joinScreen;

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

	joinScreen->ts_kin->SetSpriteTexture(kinSprite);
	joinScreen->ts_kin->SetSubRect(kinSprite, 0);
	kinSprite.setOrigin(kinSprite.getLocalBounds().width / 2, kinSprite.getLocalBounds().height / 2);
	kinSprite.setScale(4, 4);

	//joinScreen->ts_kin->SetQuadSubRect(kinQuad, 0);

	playerShader = new PlayerSkinShader("player");

	playerShader->SetTileset(joinScreen->ts_kin);

	playerShader->SetQuad(joinScreen->ts_kin, 0);
	//playerShader->SetSubRect(joinScreen->ts_kin, joinScreen->ts_kin->GetSubRect(0));
	playerShader->SetSkin(0);

	SetRectColor(bgQuad, Color::Red);

	SetTopLeft(Vector2i(0, 0));//Vector2i(100, 700) + Vector2i(index * (spacing + width), 0));

	ClearInfo();

	Show();
}

SinglePlayerBox::~SinglePlayerBox()
{
	delete playerShader;
}

void SinglePlayerBox::SetSkin(int index)
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

void SinglePlayerBox::ClearInfo()
{
	controllerStates = NULL;
	SetName("");
	action = A_WAITING_FOR_JOIN;
	skinIndex = -1;
}

void SinglePlayerBox::Show()
{
	show = true;
	SetTopLeft(topLeft);
	//numberText->setString(to_string(index));
	//numberText->setOrigin(numberText->getLocalBounds().left + numberText->getLocalBounds().width / 2,
	//	numberText->getLocalBounds().top + numberText->getLocalBounds().height / 2);
}
void SinglePlayerBox::Hide()
{
	show = false;
	//playerNameText->setString("");
	//numberText->setString("");
}

void SinglePlayerBox::Update()
{
	if (controllerStates != NULL)
	{
		if (controllerStates->ButtonPressed_LeftShoulder())
		{
			joinScreen->PrevSkin();
		}
		else if (controllerStates->ButtonPressed_RightShoulder())
		{
			joinScreen->NextSkin();
		}
	}
}

void SinglePlayerBox::SetTopLeft(sf::Vector2i &pos)
{
	topLeft = pos;

	Vector2i center(topLeft.x + joinScreen->playerBoxWidth / 2, topLeft.y + joinScreen->playerBoxHeight / 2);

	pressText.setPosition(Vector2f(center));
	//numberText->setPosition(Vector2f(center));

	Vector2i namePos(joinScreen->playerBoxWidth / 2, 0);

	SetRectTopLeft(bgQuad, joinScreen->playerBoxWidth, joinScreen->playerBoxHeight, Vector2f(topLeft));

	Vector2f controllerIconSize(joinScreen->ts_controllerIcons->tileWidth, joinScreen->ts_controllerIcons->tileHeight);
	Vector2f portIconSize(joinScreen->ts_portIcons->tileWidth, joinScreen->ts_portIcons->tileHeight);

	int border = 10;

	SetRectTopLeft(controllerIconQuad, controllerIconSize.x, controllerIconSize.y, Vector2f(joinScreen->playerBoxWidth - (controllerIconSize.x + border), border) + Vector2f(topLeft));
	SetRectTopLeft(portIconQuad, portIconSize.x, portIconSize.y, Vector2f(joinScreen->playerBoxWidth - (portIconSize.x + border), controllerIconSize.y) + Vector2f(topLeft));

	SetRectCenter(kinQuad, joinScreen->ts_kin->tileWidth, joinScreen->ts_kin->tileHeight, Vector2f(center));

	kinSprite.setPosition(Vector2f(center));

	skinNumberText.setPosition(Vector2f(topLeft + Vector2i(210, 250)));


	//playerName->setPosition(Vector2f(pos + namePos));
	//auto &bounds = playerName->getLocalBounds();
	//playerName->setPosition(Vector2f(playerName->getPosition().x - (bounds.left + bounds.width / 2), playerName->getPosition().y));
}

void SinglePlayerBox::SetControllerStates(ControllerDualStateQueue *conStates, int p_skinIndex)
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

	joinScreen->ts_controllerIcons->SetQuadSubRect(controllerIconQuad, tileIndex);
	joinScreen->ts_portIcons->SetQuadSubRect(portIconQuad, controllerStates->GetIndex());

	SetSkin(p_skinIndex);
}

void SinglePlayerBox::SetName(const std::string &name)
{
	playerNameStr = name;

	if (show)
	{
		playerNameText.setString(playerNameStr);
		SetTopLeft(topLeft);
	}
}

void SinglePlayerBox::Draw(sf::RenderTarget *target)
{
	if (!show)
		return;

	target->draw(bgQuad, 4, sf::Quads);

	if (action == A_HAS_PLAYER)
	{
		target->draw(playerNameText);
		target->draw(numberText);

		target->draw(controllerIconQuad, 4, sf::Quads, joinScreen->ts_controllerIcons->texture);

		if (controllerStates->GetControllerType() != CTYPE_KEYBOARD)
		{
			target->draw(portIconQuad, 4, sf::Quads, joinScreen->ts_portIcons->texture);
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

SinglePlayerControllerJoinScreen::SinglePlayerControllerJoinScreen(MainMenu *mm)
{
	mainMenu = mm;

	panel = new Panel("SinglePlayerControllerJoinScreen", 1920, 1080, this, true);
	panel->SetColor(Color::Transparent);
	panel->SetCenterPos(Vector2i(960, 540));

	ts_controllerIcons = GetSizedTileset("Menu/controllers_64x64.png");
	ts_portIcons = GetSizedTileset("Menu/slots_64x32.png");
	ts_kin = GetSizedTileset("Kin/stand_64x64.png");

	//inviteButton = panel->AddButton("invite", Vector2i(20 + 200, panel->size.y - 200), Vector2f(270, 40), "INVITE FRIEND");

	SetRectColor(bgQuad, Color(100, 100, 100));
	SetRectCenter(bgQuad, 1920, 1080, Vector2f(960, 540));

	playerBox = new SinglePlayerBox(this);

	mapOptionsPopup = new MapOptionsPopup(MapOptionsPopup::MODE_FREEPLAY);

	int numBoxes = 4;
	playerBoxWidth = 300;
	playerBoxHeight = 300;
	playerBoxSpacing = 100;

	Vector2i center(960, 540 + 100);//50);//150
	playerBox->SetTopLeft(center + Vector2i( playerBoxWidth / 2, playerBoxHeight / 2));
	
	Start();
}

SinglePlayerControllerJoinScreen::~SinglePlayerControllerJoinScreen()
{
	delete panel;
	delete playerBox;

	delete mapOptionsPopup;
}

void SinglePlayerControllerJoinScreen::Start()
{
	SetAction(A_WAITING_FOR_PLAYER);
	playerBox->ClearInfo();
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

const MatchParams &SinglePlayerControllerJoinScreen::GetMatchParams()
{
	return currParams;
	//cout << "creating custom lobby test: " << mapOptionsPopup->currLobbyData->mapPath << endl;
	//cout << "hash: " << mapOptionsPopup->currLobbyData->fileHash << endl;
	//cout << "creatorID: " << mapOptionsPopup->currLobbyData->creatorId << endl;
}

void SinglePlayerControllerJoinScreen::SetFromMatchParams(MatchParams &mp)
{
	playerBox->SetControllerStates(mp.controllerStateVec[0], mp.playerSkins[0]);
}

void SinglePlayerControllerJoinScreen::NextSkin()
{
	int nextIndex = playerBox->skinIndex;

	nextIndex++;
	if (nextIndex >= Actor::SKIN_Count)
	{
		nextIndex = 0;
	}
	
	playerBox->SetSkin(nextIndex);
}

void SinglePlayerControllerJoinScreen::PrevSkin()
{
	int prevIndex = playerBox->skinIndex;

	prevIndex--;
	if (prevIndex < 0)
	{
		prevIndex = Actor::SKIN_Count - 1;
	}

	playerBox->SetSkin(prevIndex);
}

void SinglePlayerControllerJoinScreen::Update()
{
	switch (action)
	{
	case A_WAITING_FOR_PLAYER:
	{
		ControllerDualStateQueue *states = playerBox->controllerStates;

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
	case A_START:
		break;
	case A_BACK:
		break;
	}

	switch (action)
	{
	case A_WAITING_FOR_PLAYER:
	{
		playerBox->Update();
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
	case A_START:
		break;
	case A_BACK:
		break;
	}
}


void SinglePlayerControllerJoinScreen::TryControllerJoin(ControllerDualStateQueue *conStates)
{
	assert(playerBox->controllerStates == NULL);

	playerBox->SetControllerStates(conStates, 0);
	//cout << "added controller: " << conStates->GetControllerType() << ", index: " << conStates->GetIndex() << endl;
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
	{
		target->draw(bgQuad, 4, sf::Quads);

		playerBox->Draw(target);
		panel->Draw(target);
		break;
	}
	}
}

void SinglePlayerControllerJoinScreen::SetAction(int a)
{
	action = a;
	frame = 0;
}

void SinglePlayerControllerJoinScreen::CancelCallback(Panel *p)
{
	Quit();
}