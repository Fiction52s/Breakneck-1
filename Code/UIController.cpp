#include "UIController.h"
#include "Input.h"
#include "MainMenu.h"

using namespace sf;
using namespace std;

UIController::UIController()
{
	currWindow = NULL;
	maxScroll = 5.f;
	Reset();
}

void UIController::Update()
{
	if (!IsWindowFocused())
	{
		return;
	}

	MainMenu *mm = MainMenu::GetInstance();

	confirmPressed = false;
	cancelPressed = false;
	confirmHeld = false;
	cancelHeld = false;

	if (CONTROLLERS.ButtonHeld_Start())
	{
		confirmHeld = true;
		if (CONTROLLERS.ButtonPressed_Start())
		{
			confirmPressed = true;
		}
	}

	if (CONTROLLERS.ButtonHeld_B())
	{
		cancelHeld = true;
		if (CONTROLLERS.ButtonPressed_B())
		{
			cancelPressed = true;
		}
	}

	ControllerDualStateQueue *foundStates = NULL;
	for (int i = 0; i < CTYPE_NONE; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			auto *states = CONTROLLERS.GetStateQueue(i, j);
			if ( states != NULL && states->GetCurrState().rightStickMagnitude > 0)
			{
				foundStates = states;
				break;
			}
		}
	}

	if (foundStates == NULL )
	{
		scrollCounter = 0;
	}
	else
	{
		ControllerState currState = foundStates->GetCurrState();
		float y = sin(currState.rightStickRadians) * currState.rightStickMagnitude;

		y /= 4;

		if ((y > 0 && scrollCounter < 0) || (y < 0 && scrollCounter > 0 ) )
		{
			scrollCounter = 0;
		}
		else
		{
			if (scrollCounter == 0)
			{
				if (y > 0)
				{
					scrollCounter = 1;
				}
				else if (y < 0)
				{
					scrollCounter = -1;
				}
			}

			scrollCounter += y;
			/*if (scrollCounter > maxScroll)
			{
				scrollCounter = maxScroll;
			}
			else if (scrollCounter < -maxScroll)
			{
				scrollCounter = -maxScroll;
			}*/
		}
	}
}

bool UIController::IsConfirmHeld()
{
	return confirmHeld;
}

bool UIController::IsConfirmPressed()
{
	return confirmPressed;
}

bool UIController::IsCancelHeld()
{
	return cancelHeld;
}

bool UIController::IsCancelPressed()
{
	return cancelPressed;
}

int UIController::ConsumeScroll()
{
	int s = scrollCounter;  //cut off decimal
	scrollCounter -= s;
	return s;
}

void UIController::Reset()
{
	confirmPressed = false;
	confirmHeld = false;

	cancelPressed = false;
	cancelHeld = false;
	
	scrollCounter = 0;
}

void UIController::SetRenderWindow(sf::RenderWindow *rw)
{
	currWindow = rw;
}

bool UIController::IsWindowFocused()
{
	if (currWindow == NULL)
		return false;

	if (currWindow->hasFocus())
	{
		return true;
	}

	return false;
}