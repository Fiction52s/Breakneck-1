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
	ResetMouse();
}

void UIMouse::Update(sf::Vector2i &mPos)
{
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