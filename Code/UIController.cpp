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

	confirm = false;
	cancel = false;

	for (int i = 0; i < 4; ++i)
	{
		ControllerState &prevState = mm->GetPrevInputUnfiltered(i);
		ControllerState &currState = mm->GetCurrInputUnfiltered(i);

		if (!prevState.start && currState.start)
		{
			confirm = true;
		}
		else if (!prevState.B && currState.B)
		{
			cancel = true;
		}
	}

	int foundIndex = -1;
	for (int i = 0; i < 4; ++i)
	{
		ControllerState &currState = mm->GetCurrInputUnfiltered(i);

		if (currState.rightStickMagnitude > 0)
		{
			foundIndex = i;
			break;
			
		}
	}

	if (foundIndex == -1)
	{
		scrollCounter = 0;
	}
	else
	{
		ControllerState &currState = mm->GetCurrInputUnfiltered(foundIndex);
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

int UIController::ConsumeScroll()
{
	int s = scrollCounter;  //cut off decimal
	scrollCounter -= s;
	return s;
}

void UIController::Reset()
{
	confirm = false;
	cancel = false;
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