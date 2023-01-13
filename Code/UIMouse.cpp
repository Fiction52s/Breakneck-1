#include "UIMouse.h"
#include "Input.h"
#include <iostream>
#include "CustomCursor.h"

using namespace sf;
using namespace std;

UIMouse::UIMouse()
{
	currWindow = NULL;
	customCursor = NULL;
	ResetMouse();
}

void UIMouse::Update(sf::Vector2i &p_mousePos)
{
	if (!IsWindowFocused())
	{
		return;
	}

	bool mouseDownL = Mouse::isButtonPressed(Mouse::Left);
	bool mouseDownR = Mouse::isButtonPressed(Mouse::Right);

	bool cursorOn = customCursor == NULL || customCursor->visible;

	if (cursorOn)
	{
		if (CONTROLLERS.ButtonHeld_A())
		{
			mouseDownL = true;
		}
		if (CONTROLLERS.ButtonHeld_X())
		{
			mouseDownR = true;
		}
	}

	lastMouseDownLeft = isMouseDownLeft;
	isMouseDownLeft = mouseDownL;

	lastMouseDownRight = isMouseDownRight;
	isMouseDownRight = mouseDownR;

	mousePos = p_mousePos;


	if (cursorOn)
	{


		ControllerDualStateQueue *nonNeutralStates = NULL;
		ControllerDualStateQueue *currStates = NULL;
		double mag = 0;
		for (int i = 0; i < CTYPE_NONE; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				currStates = CONTROLLERS.GetStateQueue(i, j);
				if (currStates != NULL)
				{
					mag = currStates->GetCurrState().leftStickMagnitude;
					if (mag > 0)
					{
						nonNeutralStates = currStates;
						break;
					}
				}
			}
		}

		if (nonNeutralStates == NULL)
		{
			myPos = mousePos;
		}
		else
		{
			ControllerState currState = nonNeutralStates->GetCurrState();
			if (currState.leftStickMagnitude > 0)
			{
				float x = cos(currState.leftStickRadians) * currState.leftStickMagnitude;
				float y = -sin(currState.leftStickRadians) * currState.leftStickMagnitude;
				float maxSpeed = 20;
				sf::Vector2i movement(round(x * maxSpeed), round(y * maxSpeed));

				//cout << "old mypos: " << myPos.x << ", " << myPos.y << endl;
				myPos += movement;
			}
			else
			{
				if (currState.LLeft())
				{
					myPos.x -= 10;
				}
				else if (currState.LRight())
				{
					myPos.x += 10;
				}

				if (currState.LUp())
				{
					myPos.y -= 10;
				}
				else if (currState.LDown())
				{
					myPos.y += 10;
				}
			}

			Vector2f pos(myPos);
			//turn back into screen coords
			pos.x *= currWindow->getSize().x / 1920.f;
			pos.y *= currWindow->getSize().y / 1080.f;

			sf::Mouse::setPosition(Vector2i(pos), *currWindow);
		}
	}

	consumed = false;
}

const sf::Vector2i &UIMouse::GetPos()
{
	return myPos;
}

sf::Vector2f UIMouse::GetFloatPos()
{
	return sf::Vector2f(myPos);
}

bool UIMouse::IsMouseDownLeft()
{
	return isMouseDownLeft;
}

bool UIMouse::IsMouseDownRight()
{
	return isMouseDownRight;
}

bool UIMouse::IsMouseLeftClicked()
{
	return isMouseDownLeft && !lastMouseDownLeft;
}

bool UIMouse::IsMouseLeftReleased()
{
	return !isMouseDownLeft && lastMouseDownLeft;
}

bool UIMouse::IsMouseRightClicked()
{
	return isMouseDownRight && !lastMouseDownRight;
}

bool UIMouse::IsMouseRightReleased()
{
	return !isMouseDownRight && lastMouseDownRight;
}

void UIMouse::ResetMouse()
{
	isMouseDownLeft = false;
	lastMouseDownLeft = false;
	isMouseDownRight = false;
	lastMouseDownRight = false;
}

void UIMouse::SetRenderWindow(sf::RenderWindow *rw)
{
	currWindow = rw;
}

void UIMouse::SetCustomCursor(CustomCursor *cc)
{
	customCursor = cc;
}

bool UIMouse::IsWindowFocused()
{
	if (currWindow == NULL)
		return false;

	if (currWindow->hasFocus())
	{
		return true;
	}

	return false;
}

void UIMouse::SetPosition(sf::Vector2i &pos)
{
	mousePos = pos;
	myPos = pos;
	sf::Mouse::setPosition(myPos, *currWindow);
}