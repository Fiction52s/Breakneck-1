#include "UIMouse.h"
#include "Input.h"

using namespace sf;
using namespace std;

UIMouse::UIMouse()
{
	currWindow = NULL;
	ResetMouse();
	
}

void UIMouse::Update(sf::Vector2i &mPos)
{
	vector<ControllerState> states;
	Update(mPos, states);
}

const sf::Vector2i &UIMouse::GetPos()
{
	return myPos;
}

sf::Vector2f UIMouse::GetFloatPos()
{
	return sf::Vector2f(myPos);
}

void UIMouse::Update(sf::Vector2i &p_mousePos, std::vector<ControllerState> & states)
{
	if (!IsWindowFocused())
	{
		return;
	}

	bool mouseDownL = Mouse::isButtonPressed(Mouse::Left);
	bool mouseDownR = Mouse::isButtonPressed(Mouse::Right);

	for (auto it = states.begin(); it != states.end(); ++it)
	{
		if ((*it).JumpButtonDown())
		{
			mouseDownL = true;
		}
		if ((*it).DashButtonDown())
		{
			mouseDownR = true;
		}
	}

	lastMouseDownLeft = isMouseDownLeft;
	isMouseDownLeft = mouseDownL;

	lastMouseDownRight = isMouseDownRight;
	isMouseDownRight = mouseDownR;

	mousePos = p_mousePos;

	ControllerState *nonNeutralState = NULL;
	for (auto it = states.begin(); it != states.end(); ++it)
	{
		//if (!(*it).IsLeftNeutral())
		if( (*it).leftStickMagnitude > 0 )
		{
			nonNeutralState = &(*it);
			break;
		}
	}

	if (nonNeutralState == NULL )
	{
		myPos = mousePos;
	}
	else
	{
		if (nonNeutralState->leftStickMagnitude > 0)
		{
			float x = cos(nonNeutralState->leftStickRadians) * nonNeutralState->leftStickMagnitude;
			float y = -sin(nonNeutralState->leftStickRadians) * nonNeutralState->leftStickMagnitude;
			float maxSpeed = 20;
			sf::Vector2i movement(round(x * maxSpeed), round(y * maxSpeed));
			myPos += movement;
		}
		else
		{
			if (nonNeutralState->LLeft())
			{
				myPos.x -= 10;
			}
			else if (nonNeutralState->LRight())
			{
				myPos.x += 10;
			}

			if (nonNeutralState->LUp())
			{
				myPos.y -= 10;
			}
			else if (nonNeutralState->LDown())
			{
				myPos.y += 10;
			}
		}

		sf::Mouse::setPosition(myPos);
	}

	consumed = false;
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
	controllerMode = false;
}

void UIMouse::SetRenderWindow(sf::RenderWindow *rw)
{
	currWindow = rw;
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
	sf::Mouse::setPosition(myPos);
}