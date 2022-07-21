#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

UIMouse::UIMouse()
{
	currWindow = NULL;
	ResetMouse();
}

void UIMouse::Update(sf::Vector2i &mPos)
{
	if (!IsWindowFocused())
	{
		return;
	}

	bool mouseDownL = Mouse::isButtonPressed(Mouse::Left);
	bool mouseDownR = Mouse::isButtonPressed(Mouse::Right);

	lastMouseDownLeft = isMouseDownLeft;
	isMouseDownLeft = mouseDownL;

	lastMouseDownRight = isMouseDownRight;
	isMouseDownRight = mouseDownR;

	mousePos = mPos;

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