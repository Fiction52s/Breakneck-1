#include "ControlSettingsMenu.h"
#include "MainMenu.h"
#include "Tileset.h"
#include "VectorMath.h"
#include <SFML/Graphics.hpp>
#include "ControlProfile.h"

using namespace std;
using namespace sf;

ControlSettingsMenu::ControlSettingsMenu( MainMenu *p_mm)
	:mainMenu( p_mm )
{
	pSel = new ProfileSelector(mainMenu, Vector2f( 200, 200 ));

	ts_xboxButtons = mainMenu->tilesetManager.GetTileset("Menu/xbox_button_icons_128x128.png", 128, 128);
	ts_actionIcons = mainMenu->tilesetManager.GetTileset("Menu/power_icon_128x128.png", 128, 128);

	int numActions = 10;

	actionQuads = new Vertex[numActions * 4];
	buttonQuads = new Vertex[numActions * 4];
	actionText = new Text[numActions];

	std::string buttonTexts[10] = { "JUMP", "DASH", "ATTACK", "POWER3", "POWER4",
		"POWER5", "POWER6LEFT", "POWER6RIGHT", "MAP", "PAUSE" };

	for (int i = 0; i < numActions; ++i)
	{
		actionText[i].setFont(mainMenu->arial);
		actionText[i].setCharacterSize(20);
		actionText[i].setFillColor(Color::White);
		actionText[i].setString(buttonTexts[i]);
		actionText[i].setOrigin(actionText[i].getLocalBounds().width / 2, actionText[i].getLocalBounds().height);
	}

	ts_currentButtons = NULL;

	SetActionTile(0, 0);
	SetActionTile(1, 1);
	SetActionTile(2, 2);
	SetActionTile(3, 5);
	SetActionTile(4, 6);
	SetActionTile(5, 7);
	SetActionTile(6, 8);
	SetActionTile(7, 8);
	SetActionTile(8, 3);
	SetActionTile(9, 3);

	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	xSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 4, 0);
	ySelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 2, 0);

	Vector2f originPos = Vector2f(100, 400);
	Vector2f spacing(50, 50);
	Vector2f actionSize(ts_actionIcons->tileWidth, ts_actionIcons->tileHeight);
	Vector2f buttonSize(ts_xboxButtons->tileWidth, ts_xboxButtons->tileHeight);

	

	for (int i = 0; i < numActions; ++i)
	{
		Vector2f index(i % (numActions/2), i / (numActions/2));
		Vector2f quadCenter = originPos + Vector2f(actionSize.x / 2.f, actionSize.y / 2.f)
			+ Vector2f((actionSize.x + buttonSize.x + spacing.x) * index.x, (actionSize.y + spacing.y) * index.y);
		SetRectCenter( actionQuads + i * 4, actionSize.x, actionSize.y, quadCenter);
		
	}

	for (int i = 0; i < numActions; ++i)
	{
		Vector2f index(i % (numActions/2), i / (numActions/2));
		Vector2f quadCenter = originPos + Vector2f(buttonSize.x / 2.f, buttonSize.y / 2.f) + Vector2f( actionSize.x, 0 )
			+ Vector2f((actionSize.x + buttonSize.x + spacing.x) * index.x, (actionSize.y + spacing.y) * index.y);
		SetRectCenter(buttonQuads + i * 4, buttonSize.x, buttonSize.y, quadCenter);
		actionText[i].setPosition(Vector2f(quadCenter.x - buttonSize.x / 2, quadCenter.y - buttonSize.y / 2 - 10));
	}

	UpdateXboxButtonIcons();
}

void ControlSettingsMenu::SetActionTile(int actionIndex, int actionType)
{
	SetRectSubRect(actionQuads + actionIndex * 4, ts_actionIcons->GetSubRect(actionType));
}

ControlSettingsMenu::~ControlSettingsMenu()
{
	delete[] actionQuads;
	delete[] buttonQuads;

	delete xSelector;
	delete ySelector;
}

void ControlSettingsMenu::Update( ControllerState &currInput, ControllerState &prevInput )
{

	pSel->Update( currInput, prevInput);

	return;
	int xchanged = xSelector->UpdateIndex(currInput.LLeft(), currInput.LRight());
	int ychanged = ySelector->UpdateIndex(currInput.LUp(), currInput.LDown());

	if (xchanged != 0 || ychanged != 0)
	{
		
	}

	bool A = currInput.A;
	switch (currButtonState)
	{
	case S_NEUTRAL:
		if (A)
		{
			currButtonState = S_PRESSED;
		}
		break;
	case S_PRESSED:
		if (!A)
		{
			currButtonState = S_SELECTED;
		}
		break;
	case S_SELECTED:
		if (A)
		{
			currButtonState = S_UNPRESSED;
		}
		break;
	case S_UNPRESSED:
		if (!A)
		{
			currButtonState = S_NEUTRAL;
		}
		break;
	}

	if (currButtonState == S_SELECTED)
	{
		XBoxButton b;
		do
		{
			b = CheckXBoxInput(currInput);
		} while (b != XBoxButton::XBOX_BLANK);

		//xboxInputAssoc[useControllerSchemeIndex][ControllerSettings::JUMP] = b;

		//UpdateXboxButtonIcons(useControllerSchemeIndex);
	}
}

void ControlSettingsMenu::Draw(sf::RenderTarget *target )
{
	target->draw(actionQuads, 10 * 4, sf::Quads, ts_actionIcons->texture);
	target->draw(buttonQuads, 10 * 4, sf::Quads, ts_currentButtons->texture);
	for (int i = 0; i < 10; ++i)
	{
		target->draw(actionText[i]);
	}

	pSel->Draw(target);
}

void ControlSettingsMenu::UpdateXboxButtonIcons()
{
	ts_currentButtons = ts_xboxButtons;
	for (int i = 0; i < ControllerSettings::ButtonType::Count; ++i)
	{
		int ind = 0;//(int)xboxInputAssoc[controlSetIndex][i] - 1;
		IntRect sub = ts_xboxButtons->GetSubRect(ind);
		SetRectSubRect(buttonQuads + i * 4, sub);
	}
}

XBoxButton ControlSettingsMenu::CheckXBoxInput(ControllerState &currInput)
{
	bool leftMovement = true;
	//bool leftMovement = (inputSelectors[2]->GetString() == "left analog");
	if (currInput.A)
	{
		return XBOX_A;
	}
	else if (currInput.B)
	{
		return XBOX_B;
	}
	else if (currInput.X)
	{
		return XBOX_X;
	}
	else if (currInput.Y)
	{
		return XBOX_Y;
	}
	else if (currInput.leftShoulder)
	{
		return XBOX_L1;
	}
	else if (currInput.rightShoulder)
	{
		return XBOX_R1;
	}
	else if (currInput.LeftTriggerPressed())
	{
		return XBOX_L2;
	}
	else if (currInput.RightTriggerPressed())
	{
		return XBOX_R2;
	}
	else if (currInput.PLeft())
	{
		return XBOX_PLEFT;
	}
	else if (currInput.PUp())
	{
		return XBOX_PUP;
	}
	else if (currInput.PRight())
	{
		return XBOX_PRIGHT;
	}
	else if (currInput.PDown())
	{
		return XBOX_PDOWN;
	}
	else if (leftMovement && currInput.RLeft())
	{
		return XBOX_RLEFT;
	}
	else if (leftMovement && currInput.RUp())
	{
		return XBOX_RUP;
	}
	else if (leftMovement && currInput.RRight())
	{
		return XBOX_RRIGHT;
	}
	else if (leftMovement && currInput.RDown())
	{
		return XBOX_RDOWN;
	}
	else if (!leftMovement && currInput.LLeft())
	{
		return XBOX_LLEFT;
	}
	else if (!leftMovement && currInput.LUp())
	{
		return XBOX_LUP;
	}
	else if (!leftMovement && currInput.LRight())
	{
		return XBOX_LRIGHT;
	}
	else if (!leftMovement && currInput.LDown())
	{
		return XBOX_LDOWN;
	}
	else if (currInput.back)
	{
		return XBOX_BACK;
	}
	else if (currInput.start)
	{
		return XBOX_START;
	}
	else
	{
		return XBOX_BLANK;
	}
}

void ControlSettingsMenu::UpdateControlIcons()
{
	Color unselectedColor = Color::Blue;
	Color selectedColor = Color::Magenta;

	for (int i = 0; i < ControllerTypes::Count; ++i)
	{
	}
}

void ControlSettingsMenu::InitAssocSymbols()
{
	int symbolX = 288;
	int symbolY = 100;
	int textX = 288;
	int textY = 70;
	int extraX;
	int extraY;
	int symbolSize = 128;//140;//128;
	int spacing = 32;//16;
	int buttonIconSpacing = 150;

	string bounceSpecial = "bounce";
	string grindSpecial = "grind";
	string timeslowSpecial = "time slow";
	string wireLeftSpecial = "left wire";
	string wireRightSpecial = "right wire";
	string toggleBounce = "toggle bounce";
	string toggleGrind = "toggle grind";
	string toggleTimeSlow = "toggle time slow";
	//these will be turned off when you dont have the powers
	string possibleActions[ControllerSettings::Count] = { "jump", "dash", "attack", bounceSpecial,
		grindSpecial, timeslowSpecial, wireLeftSpecial, wireRightSpecial,
		"map", "pause" };
	int count = ControllerSettings::Count;
	for (int i = 0; i < count; ++i)
	{
		if (i < count / 2)
		{
			extraX = 0;
			extraY = 0;
		}
		else
		{
			extraX = 500;
			extraY = -(symbolSize + spacing) * count / 2;
		}

		//assocSymbols[i * 4 + 0].position = basePos + Vector2f(extraX + symbolX, (symbolSize + spacing) * i + extraY + symbolY);
		//assocSymbols[i * 4 + 1].position = basePos + Vector2f(extraX + symbolX + symbolSize, (symbolSize + spacing) * i + extraY + symbolY);
		//assocSymbols[i * 4 + 2].position = basePos + Vector2f(extraX + symbolX + symbolSize, (symbolSize + spacing) * i + symbolSize + extraY + symbolY);
		//assocSymbols[i * 4 + 3].position = basePos + Vector2f(extraX + symbolX, (symbolSize + spacing) * i + symbolSize + extraY + symbolY);

		//IntRect sub = ts_actionIcons->GetSubRect(min(i, 8));

		//assocSymbols[i * 4 + 0].texCoords = Vector2f(sub.left, sub.top);
		//assocSymbols[i * 4 + 1].texCoords = Vector2f(sub.left + sub.width, sub.top);
		//assocSymbols[i * 4 + 2].texCoords = Vector2f(sub.left + sub.width, sub.top + sub.height);
		//assocSymbols[i * 4 + 3].texCoords = Vector2f(sub.left, sub.top + sub.height);

		//buttonVA[i * 4 + 0].position = basePos + Vector2f(buttonIconSpacing, 0) + Vector2f(extraX + symbolX, (symbolSize + spacing) * i + extraY + symbolY);
		//buttonVA[i * 4 + 1].position = basePos + Vector2f(buttonIconSpacing, 0) + Vector2f(extraX + symbolX + symbolSize, (symbolSize + spacing) * i + extraY + symbolY);
		//buttonVA[i * 4 + 2].position = basePos + Vector2f(buttonIconSpacing, 0) + Vector2f(extraX + symbolX + symbolSize, (symbolSize + spacing) * i + symbolSize + extraY + symbolY);
		//buttonVA[i * 4 + 3].position = basePos + Vector2f(buttonIconSpacing, 0) + Vector2f(extraX + symbolX, (symbolSize + spacing) * i + symbolSize + extraY + symbolY);

		//actionText[i].setFont(mainMenu->arial);
		//actionText[i].setCharacterSize(24);
		//actionText[i].setPosition(basePos + Vector2f(extraX + textX, (symbolSize + spacing) * i + extraY + textY));//textX, 50 * i );
		//actionText[i].setFillColor(Color::White);
		//actionText[i].setString(possibleActions[i]);

	}
}