#include "PlayerBox.h"
#include "Input.h"
#include "ControlProfile.h"
#include "MainMenu.h"
#include "PlayerSkinShader.h"
#include "Actor.h"

using namespace std;
using namespace sf;

PlayerBox::PlayerBox(PlayerBoxGroup *p_group, int p_index )
{
	index = p_index;

	boxGroup = p_group;

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

	pressText.setString("Press Start\nor ENTER");
	auto localB = pressText.getLocalBounds();
	pressText.setOrigin(localB.left + localB.width / 2, localB.top + localB.height / 2);

	boxGroup->ts_kin->SetSpriteTexture(kinSprite);
	boxGroup->ts_kin->SetSubRect(kinSprite, 0);
	kinSprite.setOrigin(kinSprite.getLocalBounds().width / 2, kinSprite.getLocalBounds().height / 2);
	kinSprite.setScale(4, 4);

	playerShader = new PlayerSkinShader("player");

	playerShader->SetTileset(boxGroup->ts_kin);

	playerShader->SetQuad(boxGroup->ts_kin, 0);
	playerShader->SetSkin(0);

	SetRectColor(bgQuad, Color::Red);

	SetTopLeft(Vector2f(0, 0));

	ClearInfo();

	Show();
}

PlayerBox::~PlayerBox()
{
	delete playerShader;

	delete controlMenu;
}

void PlayerBox::SetSkin(int index)
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

void PlayerBox::ClearInfo()
{
	controllerStates = NULL;
	SetName("");
	action = A_WAITING_FOR_JOIN;
	skinIndex = -1;
}

void PlayerBox::Show()
{
	show = true;
	SetTopLeft(topLeft);
}
void PlayerBox::Hide()
{
	show = false;
}

ControlProfile *PlayerBox::GetCurrProfile()
{
	return controlMenu->currProfile;
}

void PlayerBox::Update()
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
						boxGroup->PrevSkin( index );
					}
					else if (controllerStates->ButtonPressed_RightShoulder())
					{
						boxGroup->NextSkin( index );
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

void PlayerBox::SetMode(int m)
{
	if (mode == m)
	{
		return;
	}

	mode = m;

	SetTopLeft(topLeft);
}

bool PlayerBox::IsChangingControls()
{
	return action == A_CHANGING_CONTROLS;
}


void PlayerBox::SetTopLeft(sf::Vector2f &pos)
{
	topLeft = pos;

	Vector2f center(topLeft.x + boxGroup->playerBoxWidth / 2, topLeft.y + boxGroup->playerBoxHeight / 2);

	pressText.setPosition(center);
	//numberText->setPosition(Vector2f(center));

	Vector2i namePos(boxGroup->playerBoxWidth / 2, 0);

	SetRectTopLeft(bgQuad, boxGroup->playerBoxWidth, boxGroup->playerBoxHeight, Vector2f(topLeft));

	Vector2f controllerIconSize(boxGroup->ts_controllerIcons->tileWidth, boxGroup->ts_controllerIcons->tileHeight);
	Vector2f portIconSize(boxGroup->ts_portIcons->tileWidth, boxGroup->ts_portIcons->tileHeight);

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
		SetRectTopLeft(portIconQuad, portIconSize.x, portIconSize.y, portIconPos);
	}
	else
	{
		float scaleFactor = 1.5;

		controllerIconSize.x *= scaleFactor;
		controllerIconSize.y *= scaleFactor;
		portIconSize.x *= scaleFactor;
		portIconSize.y *= scaleFactor;

		Vector2f controlIconPos = Vector2f(boxGroup->playerBoxWidth - (controllerIconSize.x + border), border) + Vector2f(topLeft);
		Vector2f portIconPos = Vector2f(boxGroup->playerBoxWidth - (portIconSize.x + border), border + controllerIconSize.y - (controllerIconSize.y / 6)) + Vector2f(topLeft);

		SetRectTopLeft(controllerIconQuad, controllerIconSize.x, controllerIconSize.y, controlIconPos);
		SetRectTopLeft(portIconQuad, portIconSize.x, portIconSize.y, portIconPos);
	}

	controlMenu->SetTopLeft(topLeft);

	kinSprite.setPosition(Vector2f(center));

	Vector2f kinBottomCenter = kinSprite.getPosition() + Vector2f(0, kinSprite.getGlobalBounds().height / 2);

	skinNumberText.setPosition(kinBottomCenter + Vector2f(0, 30));
}

void PlayerBox::SetControllerStates(ControllerDualStateQueue *conStates, int p_skinIndex)
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

	boxGroup->ts_controllerIcons->SetQuadSubRect(controllerIconQuad, tileIndex);
	boxGroup->ts_portIcons->SetQuadSubRect(portIconQuad, controllerStates->GetIndex());

	SetSkin(p_skinIndex);
}

void PlayerBox::SetCurrProfile(ControlProfile *cp)
{
	controlMenu->SetCurrProfile(cp);
}

void PlayerBox::SetName(const std::string &name)
{
	playerNameStr = name;

	if (show)
	{
		playerNameText.setString(playerNameStr);
		SetTopLeft(topLeft);
	}
}

void PlayerBox::Draw(sf::RenderTarget *target)
{
	if (!show)
		return;

	target->draw(bgQuad, 4, sf::Quads);

	if (action == A_HAS_PLAYER)
	{
		target->draw(playerNameText);
		target->draw(numberText);

		target->draw(controllerIconQuad, 4, sf::Quads, boxGroup->ts_controllerIcons->texture);

		if (controllerStates->GetControllerType() != CTYPE_KEYBOARD)
		{
			target->draw(portIconQuad, 4, sf::Quads, boxGroup->ts_portIcons->texture);
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
	else if (action == A_WAITING_FOR_JOIN)
	{
		target->draw(pressText);
	}
}


PlayerBoxGroup::PlayerBoxGroup( TilesetManager *tm, int numBoxes, int p_playerBoxWidth, int p_playerBoxHeight, int p_playerBoxSpacing )
{
	ts_controllerIcons = tm->GetSizedTileset("Menu/controllers_64x64.png");
	ts_portIcons = tm->GetSizedTileset("Menu/slots_64x32.png");
	ts_kin = tm->GetSizedTileset("Kin/stand_64x64.png");

	playerBoxWidth = p_playerBoxWidth;
	playerBoxHeight = p_playerBoxHeight;
	playerBoxSpacing = p_playerBoxSpacing;
	playerBoxes.resize(numBoxes);
	for (int i = 0; i < numBoxes; ++i)
	{
		playerBoxes[i] = new PlayerBox(this, i);
	}
}


PlayerBoxGroup::~PlayerBoxGroup()
{
	for (auto it = playerBoxes.begin(); it != playerBoxes.end(); ++it)
	{
		delete (*it);
	}
	playerBoxes.clear();
}

void PlayerBoxGroup::Update()
{
	for (auto it = playerBoxes.begin(); it != playerBoxes.end(); ++it)
	{
		(*it)->Update();
	}	
}

void PlayerBoxGroup::SetControlProfile(int index, ControlProfile *cp)
{
	playerBoxes[index]->SetCurrProfile(cp);
}

bool PlayerBoxGroup::CheckControllerJoins()
{
	ControllerDualStateQueue *states = NULL;
	for (int i = 0; i < 4; ++i)
	{
	states = CONTROLLERS.GetStateQueue(CTYPE_XBOX, i);
	if (states->ButtonPressed_Start())
	{
		if (TryControllerJoin(states))
			return true;
	}

	states = CONTROLLERS.GetStateQueue(CTYPE_GAMECUBE, i);
	if (states->ButtonPressed_Start())
	{
		if (TryControllerJoin(states))
		{
			return true;
		}
		
	}
	}

	states = CONTROLLERS.GetStateQueue(CTYPE_KEYBOARD, 0);
	if (states->ButtonPressed_Start())
	{
		if (TryControllerJoin(states))
		{
			return true;
		}
	}

	return false;
}

void PlayerBoxGroup::NextSkin(int playerBoxIndex)
{
	int nextIndex = playerBoxes[playerBoxIndex]->skinIndex;

	do
	{
		nextIndex++;
		if (nextIndex >= Actor::SKIN_Count)
		{
			nextIndex = 0;
		}
	} while (!IsSkinAvailable(nextIndex));

	playerBoxes[playerBoxIndex]->SetSkin(nextIndex);
}

void PlayerBoxGroup::PrevSkin(int playerBoxIndex)
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

int PlayerBoxGroup::GetFirstAvailableSkinIndex()
{
	for (int i = 0; i < Actor::SKIN_Count; ++i)
	{
		if (IsSkinAvailable(i))
		{
			return i;
		}
	}

	return -1; //should never get hit
}

bool PlayerBoxGroup::IsSkinAvailable(int p_skinIndex)
{
	for (auto it = playerBoxes.begin(); it != playerBoxes.end(); ++it)
	{
		if ((*it)->controllerStates != NULL && (*it)->skinIndex == p_skinIndex)
		{
			return false;
		}
	}

	return true;
}

ControllerDualStateQueue * PlayerBoxGroup::GetControllerStates(int ind)
{
	return playerBoxes[ind]->controllerStates;
}

ControlProfile *PlayerBoxGroup::GetControlProfile(int ind)
{
	return playerBoxes[ind]->GetCurrProfile();
}

int PlayerBoxGroup::GetSkinIndex(int ind)
{
	return playerBoxes[ind]->skinIndex;
}

bool PlayerBoxGroup::IsBoxChangingControls(int ind)
{
	if (playerBoxes[ind]->action == PlayerBox::A_CHANGING_CONTROLS)
	{
		return true;
	}
	
	return false;
}

void PlayerBoxGroup::SetBoxTopLeft(int ind, sf::Vector2f &pos)
{
	playerBoxes[ind]->SetTopLeft(pos);
}

void PlayerBoxGroup::SetBoxCenter(int ind, sf::Vector2f &pos)
{
	Vector2f tLeft = pos - Vector2f(playerBoxWidth / 2, playerBoxHeight / 2);
	playerBoxes[ind]->SetTopLeft(tLeft);
}

void PlayerBoxGroup::ClearInfo()
{
	for (auto it = playerBoxes.begin(); it != playerBoxes.end(); ++it)
	{
		(*it)->ClearInfo();
	}
}

void PlayerBoxGroup::ClearInfo( int ind )
{
	playerBoxes[ind]->ClearInfo();
}

void PlayerBoxGroup::SetMode(int m)
{
	for (auto it = playerBoxes.begin(); it != playerBoxes.end(); ++it)
	{
		(*it)->SetMode(m);
	}
}

void PlayerBoxGroup::SetControllerStates(int index, ControllerDualStateQueue *conStates, int p_skinIndex)
{
	playerBoxes[index]->SetControllerStates(conStates, p_skinIndex);
}

bool PlayerBoxGroup::IsFull()
{
	for (auto it = playerBoxes.begin(); it != playerBoxes.end(); ++it)
	{
		if ((*it)->controllerStates == NULL)
		{
			return false;
		}
	}

	return true;
}

bool PlayerBoxGroup::BackButtonPressed()
{
	ControllerDualStateQueue *states = NULL;
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			states = CONTROLLERS.GetStateQueue(i, j);
			if (states != NULL && states->ButtonPressed_B() && !IsStateChangingControls(states) )
			{
				return true;
			}
		}
	}

	return false;


	/*ControllerDualStateQueue *states = NULL;
	int numBoxes = playerBoxes.size();
	for (int i = 0; i < numBoxes; ++i)
	{
		states = playerBoxes[i]->controllerStates;
		if (states != NULL && !IsBoxChangingControls(i))
		{
			if (states->ButtonPressed_B())
			{
				return true;
			}
		}
	}*/
}

bool PlayerBoxGroup::IsStateChangingControls(ControllerDualStateQueue *conStates)
{
	for (auto it = playerBoxes.begin(); it != playerBoxes.end(); ++it)
	{
		if ((*it)->controllerStates == conStates)
		{
			if ((*it)->IsChangingControls() )
			{
				return true;
			}
		}
	}

	return false;
}

bool PlayerBoxGroup::IsReadyAndStartPressed()
{
	if (IsReady())
	{
		ControllerDualStateQueue *states = NULL;
		for (auto it = playerBoxes.begin(); it != playerBoxes.end(); ++it)
		{
			states = (*it)->controllerStates;
			if (states != NULL)
			{
				if (states->ButtonPressed_Start())
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool PlayerBoxGroup::IsReady()
{
	int numBoxes = playerBoxes.size();
	for (int i = 0; i < numBoxes; ++i)
	{
		if (IsBoxChangingControls(i))
		{
			return false;
		}
	}

	for (int i = 0; i < numBoxes; ++i)
	{
		if (playerBoxes[i]->controllerStates != NULL)
		{
			return true;
		}
	}

	return false;
}

bool PlayerBoxGroup::AlreadyJoined(ControllerDualStateQueue *conStates)
{
	assert(conStates != NULL);
	for (auto it = playerBoxes.begin(); it != playerBoxes.end(); ++it)
	{
		if ((*it)->controllerStates == conStates)
		{
			return true;
		}
	}

	return false;
}

bool PlayerBoxGroup::TryControllerJoin(ControllerDualStateQueue *conStates)
{
	if (IsFull())
		return false;

	if (!AlreadyJoined(conStates))
	{
		int numBoxes = playerBoxes.size();
		for (int i = 0; i < numBoxes; ++i)
		{
			if (playerBoxes[i]->controllerStates == NULL)
			{
				SetControllerStates(i, conStates, GetFirstAvailableSkinIndex());
				cout << "added controller: " << conStates->GetControllerType() << ", index: " << conStates->GetIndex() << endl;
				return true;
			}
		}
		
	}

	return false;
}

int PlayerBoxGroup::GetNumFullBoxes()
{
	int numFull = 0;
	for (auto it = playerBoxes.begin(); it != playerBoxes.end(); ++it)
	{
		if ((*it)->controllerStates != NULL)
			++numFull;
	}

	return numFull;
}

void PlayerBoxGroup::Draw(sf::RenderTarget *target)
{
	for( auto it = playerBoxes.begin(); it != playerBoxes.end(); ++it )
	{
		(*it)->Draw(target);
	}
}

