#include "EditSession.h"
#include "Action.h"
#include "EditorModes.h"

//#include "GUI.h"
//#include "EditSession.h"
//#include <fstream>
//#include <assert.h>
//#include <iostream>
//#include "poly2tri/poly2tri.h"
//#include <sstream>
//#include <boost/lexical_cast.hpp>
//#include "Physics.h"

//#include <set>
//#include "MainMenu.h"
//#include "Background.h"
//#include "Parallax.h"
//#include "Enemy_Shard.h"
//#include "ActorParams.h"

using namespace sf;
using namespace std;

EditorMode::EditorMode()
{
	edit = EditSession::GetSession();
}

void CreateTerrainMode::HandleEvent(sf::Event ev)
{
	Panel *showPanel = edit->showPanel;

	switch (ev.type)
	{
	case Event::MouseButtonPressed:
	{
		if (ev.mouseButton.button == Mouse::Left)
		{
			if (showPanel != NULL)
			{
				showPanel->Update(true, uiMouse.x, uiMouse.y);
				break;
			}

			if (cutChoose)
			{
				bool c0 = cutPoly0->ContainsPoint(testPoint);
				bool c1 = cutPoly1->ContainsPoint(testPoint);
				if (cutPoly0->inverse)
					c0 = !c0;
				if (cutPoly1->inverse)
					c1 = !c1;

				//contains point returns the opposite when you're inverse
				if (c0)
				{
					ChooseCutPoly(cutPoly0);
				}
				else if (c1)
				{
					ChooseCutPoly(cutPoly1);
				}

				extendingPolygon = NULL;
				extendingPoint = NULL;
			}

		}

		break;
	}
	case Event::MouseButtonReleased:
	{
		if (showPanel != NULL)
		{
			showPanel->Update(false, uiMouse.x, uiMouse.y);
		}
		break;
	}
	case Event::MouseWheelMoved:
	{
		break;
	}
	case Event::KeyPressed:
	{
		if (showPanel != NULL)
		{
			showPanel->SendKey(ev.key.code, ev.key.shift);
			break;
		}

		if (ev.key.code == Keyboard::Space)
		{
			ExecuteTerrainCompletion();	
		}
		else if (ev.key.code == sf::Keyboard::X || ev.key.code == sf::Keyboard::Delete)
		{
			if (polygonInProgress->numPoints > 0)
			{
				polygonInProgress->RemovePoint(polygonInProgress->pointEnd);
			}
		}
		else if (ev.key.code == sf::Keyboard::E)
		{
		}
		else if (ev.key.code == sf::Keyboard::Z && ev.key.control)
		{
			if (doneActionStack.size() > 0)
			{
				Action *action = doneActionStack.back();
				doneActionStack.pop_back();

				//cout << "undoing an action" << endl;
				action->Undo();

				undoneActionStack.push_back(action);
			}
		}
		else if (ev.key.code == sf::Keyboard::Y && ev.key.control)
		{
			if (undoneActionStack.size() > 0)
			{
				Action *action = undoneActionStack.back();
				undoneActionStack.pop_back();

				action->Perform();

				doneActionStack.push_back(action);
			}
		}

		break;
	}
	case Event::KeyReleased:
	{
		if (ev.key.code == sf::Keyboard::E)
		{
			if (false) //only for this build
			{

				showPoints = false;
				extendingPolygon = NULL;
				extendingPoint = NULL;
				polygonInProgress->ClearPoints();
			}
		}
		break;
	}
	case Event::LostFocus:
	{
		break;
	}
	case Event::GainedFocus:
	{
		break;
	}
	}
	break;
}