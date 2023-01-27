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

SinglePlayerBox::SinglePlayerBox(SinglePlayerControllerJoinScreen *p_joinScreen )
{
	joinScreen = p_joinScreen;

	controlMenu = new ControlProfileMenu;

	mode = MODE_DEFAULT;

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


	skinNumberText.setCharacterSize(30);
	skinNumberText.setFont(f);
	skinNumberText.setFillColor(Color::White);

	pressText.setString("Press Start");
	auto localB = pressText.getLocalBounds();
	pressText.setOrigin(localB.left + localB.width / 2, localB.top + localB.height / 2);

	joinScreen->ts_kin->SetSpriteTexture(kinSprite);
	joinScreen->ts_kin->SetSubRect(kinSprite, 0);
	kinSprite.setOrigin(kinSprite.getLocalBounds().width / 2, kinSprite.getLocalBounds().height / 2);
	kinSprite.setScale(4, 4);

	playerShader = new PlayerSkinShader("player");

	playerShader->SetTileset(joinScreen->ts_kin);

	playerShader->SetQuad(joinScreen->ts_kin, 0);
	//playerShader->SetSubRect(joinScreen->ts_kin, joinScreen->ts_kin->GetSubRect(0));
	playerShader->SetSkin(0);

	SetRectColor(bgQuad, Color::Red);

	SetTopLeft(Vector2f(0, 0));//Vector2i(100, 700) + Vector2i(index * (spacing + width), 0));

	

	ClearInfo();

	Show();
}

SinglePlayerBox::~SinglePlayerBox()
{
	delete playerShader;

	delete controlMenu;
}

void SinglePlayerBox::SetSkin(int index)
{
	skinIndex = index;
	playerShader->SetSkin(skinIndex);

	if (skinIndex < 10)
	{
		skinNumberText.setString("S-KIN #0" + to_string(skinIndex));
		auto lb = skinNumberText.getLocalBounds();
		skinNumberText.setOrigin(lb.left + lb.width / 2, lb.top);
	}
	else
	{
		skinNumberText.setString("S-KIN #" + to_string(skinIndex));
		auto lb = skinNumberText.getLocalBounds();
		skinNumberText.setOrigin(lb.left + lb.width / 2, lb.top);
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

ControlProfile *SinglePlayerBox::GetCurrProfile()
{
	return controlMenu->currProfile;
}

void SinglePlayerBox::Update()
{
	if (controllerStates != NULL)
	{
		switch (action)
		{
		case A_HAS_PLAYER:
		{
			if (controllerStates->ButtonPressed_A())
			{
				controlMenu->BeginSelectingProfile();
				action = A_CHANGING_CONTROLS;
			}
			else
			{
				if (mode != MODE_CONTROLLER_ONLY)
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
			break;
		}
		case A_CHANGING_CONTROLS:
		{
			controlMenu->Update();
			if (controlMenu->action == ControlProfileMenu::A_SELECTED)
			{
				action = A_HAS_PLAYER;
			}
			break;
		}
			
		}
	}
}

void SinglePlayerBox::SetMode(int m)
{
	if (mode == m)
	{
		return;
	}

	mode = m;

	SetTopLeft(topLeft);
}


void SinglePlayerBox::SetTopLeft(sf::Vector2f &pos)
{
	topLeft = pos;

	Vector2f center(topLeft.x + joinScreen->playerBoxWidth / 2, topLeft.y + joinScreen->playerBoxHeight / 2);

	pressText.setPosition(center);
	//numberText->setPosition(Vector2f(center));

	Vector2i namePos(joinScreen->playerBoxWidth / 2, 0);

	SetRectTopLeft(bgQuad, joinScreen->playerBoxWidth, joinScreen->playerBoxHeight, Vector2f(topLeft));

	Vector2f controllerIconSize(joinScreen->ts_controllerIcons->tileWidth, joinScreen->ts_controllerIcons->tileHeight);
	Vector2f portIconSize(joinScreen->ts_portIcons->tileWidth, joinScreen->ts_portIcons->tileHeight);

	int border = 0;

	if (mode == MODE_CONTROLLER_ONLY)
	{
		border = 0;

		float scaleFactor = 3;

		controllerIconSize.x *= scaleFactor;
		controllerIconSize.y *= scaleFactor;
		portIconSize.x *= scaleFactor;
		portIconSize.y *= scaleFactor;

		
		Vector2f controlIconPos = Vector2f(center.x - (controllerIconSize.x / 2), center.y - (controllerIconSize.y / 2));
		Vector2f portIconPos = Vector2f(center.x - (portIconSize.x / 2), controlIconPos.y + controllerIconSize.y - (controllerIconSize.y / 6));

		SetRectTopLeft(controllerIconQuad, controllerIconSize.x, controllerIconSize.y, controlIconPos);
		SetRectTopLeft(portIconQuad, portIconSize.x, portIconSize.y, portIconPos );
	}
	else
	{
		float scaleFactor = 1.5;

		controllerIconSize.x *= scaleFactor;
		controllerIconSize.y *= scaleFactor;
		portIconSize.x *= scaleFactor;
		portIconSize.y *= scaleFactor;

		Vector2f controlIconPos = Vector2f(joinScreen->playerBoxWidth - (controllerIconSize.x + border), border) + Vector2f(topLeft);
		Vector2f portIconPos = Vector2f(joinScreen->playerBoxWidth - (portIconSize.x + border), border + controllerIconSize.y - (controllerIconSize.y / 6)) + Vector2f(topLeft);

		SetRectTopLeft(controllerIconQuad, controllerIconSize.x, controllerIconSize.y, controlIconPos);
		SetRectTopLeft(portIconQuad, portIconSize.x, portIconSize.y, portIconPos);
	}

	controlMenu->SetTopLeft(topLeft); //+ Vector2f(joinScreen->playerBoxWidth / 2, 0 ));

	kinSprite.setPosition(Vector2f(center));

	Vector2f kinBottomCenter = kinSprite.getPosition() + Vector2f(0, kinSprite.getGlobalBounds().height / 2);

	skinNumberText.setPosition(kinBottomCenter + Vector2f(0, 30));


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

	controlMenu->SetControllerInput(controllerStates);

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

		if (mode != MODE_CONTROLLER_ONLY)
		{
			target->draw(kinSprite, &(playerShader->pShader));

			target->draw(skinNumberText);
		}

		controlMenu->Draw(target);
	}
	else if (action == A_CHANGING_CONTROLS)
	{
		controlMenu->Draw(target);
	}
	else if( action == A_WAITING_FOR_JOIN )
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
	playerBoxWidth = 400;//300
	playerBoxHeight = 400;//300
	playerBoxSpacing = 100;

	Vector2f center(960, 540 + 100);//50);//150
	playerBox->SetTopLeft(center - Vector2f( playerBoxWidth / 2, playerBoxHeight / 2));
	
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

void SinglePlayerControllerJoinScreen::SetMode(int m)
{
	playerBox->SetMode(m);
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
		break;
	}
	case A_READY:
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
	case A_CONTROL_PROFILE:
	{
		if (playerBox->action != SinglePlayerBox::A_CHANGING_CONTROLS)
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

		SetRectColor(bgQuad, Color(100, 100, 100));
		break;
	}
	case A_READY:
	{
		playerBox->Update();
		if (playerBox->action != SinglePlayerBox::A_CHANGING_CONTROLS)
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
		playerBox->Update();
		break;
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

	SetAction(A_READY);
	SetRectColor(bgQuad, Color(83, 102, 188));
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
	case A_READY:
	case A_CONTROL_PROFILE:
	case A_START:
	{
		target->draw(bgQuad, 4, sf::Quads);

		playerBox->Draw(target);
		panel->Draw(target);
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
	Quit();
}