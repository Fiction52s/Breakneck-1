#include "ControlSettingsMenu.h"
#include "MainMenu.h"
#include "Tileset.h"

using namespace std;
using namespace sf;

ControlSettingsMenu::ControlSettingsMenu( MainMenu *p_mm)
	:mainMenu( p_mm )
{
	ts_xboxButtons = mainMenu->tilesetManager.GetTileset("Menu/xbox_button_icons_128x128.png", 128, 128);
	ts_actionIcons = mainMenu->tilesetManager.GetTileset("Menu/power_icon_128x128.png", 128, 128);
}

void ControlSettingsMenu::Update()
{
	ControllerState & currInput = mainMenu->menuCurrInput;
	ControllerState & prevInput = mainMenu->menuPrevInput;

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