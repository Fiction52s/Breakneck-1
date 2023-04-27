#include "UIMouse.h"
#include "Input.h"
#include <iostream>
#include "CustomCursor.h"
#include <assert.h>

using namespace sf;
using namespace std;

const float UIMouse::stickDeadZone = .1;

UIMouse::UIMouse()
{
	currWindow = NULL;
	customCursor = NULL;
	controllersOn = true;
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

	if (cursorOn && controllersOn )
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


	if (cursorOn && controllersOn)
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
					if (mag > stickDeadZone)
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
			//we already know its out of the deadzone because of the previous calculation
			ControllerState currState = nonNeutralStates->GetCurrState();

			Vector2i origSize(1920, 1080);
			Vector2f origSizeF(origSize);

			float x = cos(currState.leftStickRadians) * currState.leftStickMagnitude;
			float y = -sin(currState.leftStickRadians) * currState.leftStickMagnitude;
			float maxSpeed = 20;
			sf::Vector2i movement(round(x * maxSpeed), round(y * maxSpeed));

			myPos += movement;

			if (myPos.x < 0)
				myPos.x = 0;
			else if (myPos.x > origSize.x)
				myPos.x = origSize.x;

			if (myPos.y < 0)
				myPos.y = 0;
			else if (myPos.y > origSize.y)
				myPos.y = origSize.y;

			Vector2f windowSize(currWindow->getSize());
			Vector2f pos(myPos);

			pos.x *= windowSize.x / origSizeF.x;
			pos.y *= windowSize.y / origSizeF.y;

			sf::Mouse::setPosition(Vector2i(pos), *currWindow);
		}
	}
	else if (cursorOn)
	{
		myPos = mousePos;
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

void UIMouse::SetControllersOn(bool on)
{
	controllersOn = on;
}

bool UIMouse::IsControllerModeOn()
{
	return controllersOn;
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

sf::Vector2i UIMouse::GetRealPixelPos()
{
	Vector2i pPos = Mouse::getPosition(*currWindow);
	pPos.x *= 1920.f / currWindow->getSize().x;
	pPos.y *= 1080.f / currWindow->getSize().y;

	return pPos;
}

//cannot be called from non-main thread
void UIMouse::Hide()
{
	if (customCursor != NULL)
	{
		customCursor->Hide();
	}
}

//cannot be called from non-main thread
void UIMouse::Show()
{
	if (customCursor != NULL)
	{
		customCursor->Show();
	}
}

void UIMouse::Grab()
{
	if (customCursor != NULL)
	{
		customCursor->Grab();
	}
}

void UIMouse::Release()
{
	if (customCursor != NULL)
	{
		customCursor->Release();
	}
}